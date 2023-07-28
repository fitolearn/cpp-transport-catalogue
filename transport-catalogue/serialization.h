#pragma once

#include <string>
#include "domain.h"
#include "transport_catalogue.pb.h"
#include "svg.h"

struct SerializationSettings {
    std::string file_name;
};

struct RendererSettings;
struct RoutingSettings;

geo::Coordinates DeserializeCoordinates(const tc_serialize::Coordinates& coords);
svg::Point DeserializePoint(const tc_serialize::Point& p);
svg::Rgb DeserializeRgb(const tc_serialize::RGB& rgb);
svg::Rgba DeserializeRgba(const tc_serialize::RGBA& rgba);
svg::Color DeserializeColor(const tc_serialize::Color& color);
tc_serialize::Stop SerializeStop(const transport::Stop& stop);
tc_serialize::Coordinates SerializeCoordinates(const geo::Coordinates& coords);
tc_serialize::DistanceBetweenStops SerializeDistance(uint32_t from, uint32_t to, int distance);
tc_serialize::Point SerializePoint(const svg::Point& p);
tc_serialize::RGB SerializeRgb(const svg::Rgb& rgb);
tc_serialize::RGBA SerializeRgba(const svg::Rgba& rgba);
tc_serialize::Color SerializeColor(const svg::Color& color);
tc_serialize::RenderSettings SerializeRendererSettings(const RendererSettings& settings);
tc_serialize::RoutingSettings SerializeRouting(const RoutingSettings& settings);
RendererSettings DeserializeRenderSetting(const tc_serialize::RenderSettings& settings);
RoutingSettings DeserializeRouting(const tc_serialize::RoutingSettings& settings);
transport::Stop DeserializeStop(const tc_serialize::Stop& stop);
