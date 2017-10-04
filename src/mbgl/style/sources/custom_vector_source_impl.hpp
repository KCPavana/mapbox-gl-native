#pragma once

#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/sources/custom_vector_source.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/actor/actor_ref.hpp>

namespace mbgl {
namespace style {

class CustomVectorSource::Impl : public Source::Impl {
public:
    Impl(std::string id, GeoJSONOptions options);
    Impl(const Impl&, ActorRef<CustomTileLoader>);

    optional<std::string> getAttribution() const final;

    GeoJSONOptions getOptions() const;
    optional<ActorRef<CustomTileLoader>> getTileLoader() const;

private:
    GeoJSONOptions options;
    optional<ActorRef<CustomTileLoader>> loaderRef;
};

} // namespace style
} // namespace mbgl