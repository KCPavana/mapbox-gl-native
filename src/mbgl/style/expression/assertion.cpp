#include <mbgl/style/expression/assertion.hpp>
#include <mbgl/style/expression/check_subtype.hpp>

namespace mbgl {
namespace style {
namespace expression {

ParseResult Assertion::parse(const mbgl::style::conversion::Convertible& value, ParsingContext ctx) {
    using namespace mbgl::style::conversion;
    static std::unordered_map<std::string, type::Type> types {
        {"string", type::String},
        {"number", type::Number},
        {"boolean", type::Boolean},
        {"object", type::Object}
    };

    std::size_t length = arrayLength(value);

    if (length < 2) {
        ctx.error("Expected at least one argument.");
        return ParseResult();
    }

    auto it = types.find(*toString(arrayMember(value, 0)));
    assert(it != types.end());
    
    std::vector<std::unique_ptr<Expression>> parsed;
    for (std::size_t i = 1; i < length; i++) {
        ParseResult input = ctx.concat(i, {type::Value}).parse(arrayMember(value, i));
        if (!input) return ParseResult();
        parsed.push_back(std::move(*input));
    }

    return ParseResult(std::make_unique<Assertion>(it->second, std::move(parsed)));
}

EvaluationResult Assertion::evaluate(const EvaluationParameters& params) const {
    for (std::size_t i = 0; i < inputs.size(); i++) {
        EvaluationResult value = inputs[i]->evaluate(params);
        if (!value) return value;
        if (!type::checkSubtype(getType(), typeOf(*value))) {
            return value;
        } else if (i == inputs.size() - 1) {
            return EvaluationError {
                "Expected value to be of type " + toString(getType()) +
                ", but found " + toString(typeOf(*value)) + " instead."
            };
        }
    }

    assert(false);
    return EvaluationResult();
};

void Assertion::eachChild(std::function<void(const Expression*)> visit) const {
    for(const std::unique_ptr<Expression>& input : inputs) {
        visit(input.get());
    }
};

} // namespace expression
} // namespace style
} // namespace mbgl

