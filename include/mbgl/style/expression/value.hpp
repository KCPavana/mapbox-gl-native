#pragma once

#include <array>
#include <mbgl/style/expression/type.hpp>
#include <mbgl/style/position.hpp>
#include <mbgl/style/types.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/enum.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/variant.hpp>


namespace mbgl {
namespace style {
namespace expression {

struct Value;

using ValueBase = variant<
    NullValue,
    bool,
    double,
    std::string,
    mbgl::Color,
    mapbox::util::recursive_wrapper<std::vector<Value>>,
    mapbox::util::recursive_wrapper<std::unordered_map<std::string, Value>>>;
struct Value : ValueBase {
    using ValueBase::ValueBase;

    // Javascript's Number.MAX_SAFE_INTEGER
    static uint64_t max() { return 9007199254740991ULL; }
    
    static bool isSafeNumericValue(uint64_t x) { return x <= max(); };
    static bool isSafeNumericValue(int64_t x) {
        return static_cast<uint64_t>(x > 0 ? x : -x) <= max();
    }
    static bool isSafeNumericValue(double x) {
        return static_cast<uint64_t>(x > 0 ? x : -x) <= max();
    }
    
};

constexpr NullValue Null = NullValue();

type::Type typeOf(const Value& value);
std::string stringify(const Value& value);

/*
  Returns a Type object representing the expression type that corresponds to
  the value type T.  (Specialized for primitives and specific array types in
  the .cpp.)
*/
template <typename T>
type::Type valueTypeToExpressionType();

/*
  Conversions between style value types and expression::Value
*/

// no-op overloads for case where the target type is a member of expression::Value
Value toExpressionValue(const Value&);

template <typename T>
std::enable_if_t< std::is_convertible<T, Value>::value,
optional<T>> fromExpressionValue(const Value& v)
{
    return v.template is<T>() ? v.template get<T>() : optional<T>();
}

// real conversions
template <typename T, typename Enable = std::enable_if_t< !std::is_convertible<T, Value>::value >>
Value toExpressionValue(const T& value);

template <typename T>
std::enable_if_t< !std::is_convertible<T, Value>::value,
optional<T>> fromExpressionValue(const Value& v);



template <class T, class Enable = void>
struct ValueConverter {
    using ExpressionType = T;
    
    static Value toExpressionValue(const T& value) {
        return Value(value);
    }
    static optional<T> fromExpressionValue(const Value& value) {
        return value.template is<T>() ? value.template get<T>() : optional<T>();
    }
};

template <>
struct ValueConverter<float> {
    using ExpressionType = double;
    static type::Type expressionType() { return type::Number; }
    static Value toExpressionValue(const float& value);
    static optional<float> fromExpressionValue(const Value& value);
};

template<>
struct ValueConverter<mbgl::Value> {
    static Value toExpressionValue(const mbgl::Value& value);
};

template <typename T, std::size_t N>
struct ValueConverter<std::array<T, N>> {
    using ExpressionType = std::vector<Value>;
    static type::Type expressionType() {
        return type::Array(valueTypeToExpressionType<T>(), N);
    }
    static Value toExpressionValue(const std::array<T, N>& value);
    static optional<std::array<T, N>> fromExpressionValue(const Value& value);
};

template <typename T>
struct ValueConverter<std::vector<T>> {
    using ExpressionType = std::vector<Value>;
    static type::Type expressionType() {
        return type::Array(valueTypeToExpressionType<T>());
    }
    static Value toExpressionValue(const std::vector<T>& value);
    static optional<std::vector<T>> fromExpressionValue(const Value& value);
};

template <>
struct ValueConverter<Position> {
    using ExpressionType = std::vector<Value>;
    static type::Type expressionType() { return type::Array(type::Number, 3); }
    static Value toExpressionValue(const mbgl::style::Position& value);
    static optional<Position> fromExpressionValue(const Value& v);
};

template <typename T>
struct ValueConverter<T, std::enable_if_t< std::is_enum<T>::value >> {
    using ExpressionType = std::string;
    static type::Type expressionType() { return type::String; }
    static Value toExpressionValue(const T& value);
    static optional<T> fromExpressionValue(const Value& value);
};

} // namespace expression
} // namespace style
} // namespace mbgl