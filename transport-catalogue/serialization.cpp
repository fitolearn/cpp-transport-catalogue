#include "serialization.h"
#include "map_renderer.h"
#include "transport_router.h"

tc_serialize::Stop SerializeStop(const transport::Stop& stop) {
    tc_serialize::Stop result;
    result.set_id_stop(stop.id);
    result.set_name(stop.stop_name);
    *result.mutable_coords() = SerializeCoordinates(stop.coordinates);
    return result;
}

tc_serialize::Coordinates SerializeCoordinates(const geo::Coordinates& coords) {
    tc_serialize::Coordinates result;
    result.set_latitude(coords.latitude);
    result.set_longitude(coords.longitude);
    return result;
}

transport::Stop DeserializeStop(const tc_serialize::Stop& stop) {
    return {stop.id_stop(), stop.name(), DeserializeCoordinates(stop.coords())};
}

geo::Coordinates DeserializeCoordinates(const tc_serialize::Coordinates& coords) {
    return {coords.latitude(), coords.longitude()};
}

tc_serialize::DistanceBetweenStops SerializeDistance(uint32_t from, uint32_t to, int distance) {
    tc_serialize::DistanceBetweenStops dbs;
    dbs.set_from_id(from);
    dbs.set_to_id(to);
    dbs.set_distance(distance);
    return dbs;
}

tc_serialize::Point SerializePoint(const svg::Point& p) {
    tc_serialize::Point result;
    result.set_x(p.x);
    result.set_y(p.y);
    return result;
}
svg::Point DeserializePoint(const tc_serialize::Point& p) {
    svg::Point result;
    result.x = p.x();
    result.y = p.y();
    return result;
}

tc_serialize::RGB SerializeRgb(const svg::Rgb& rgb) {
    tc_serialize::RGB result;
    result.set_red(rgb.red);
    result.set_green(rgb.green);
    result.set_blue(rgb.blue);
    return result;
}
svg::Rgb DeserializeRgb(const tc_serialize::RGB& rgb) {
    svg::Rgb result(rgb.red(), rgb.green(), rgb.blue());
    return result;
}

tc_serialize::RGBA SerializeRgba(const svg::Rgba& rgba) {
    tc_serialize::RGBA result;
    result.set_red(rgba.red);
    result.set_green(rgba.green);
    result.set_blue(rgba.blue);
    result.set_opacity(rgba.opacity);
    return result;
}
svg::Rgba DeserializeRgba(const tc_serialize::RGBA& rgba) {
    svg::Rgba result(rgba.red(), rgba.green(), rgba.blue(), rgba.opacity());
    return result;
}

tc_serialize::Color SerializeColor(const svg::Color& color) {
    tc_serialize::Color result;
    if (std::holds_alternative<std::string>(color)) {
        *result.mutable_color_string() = std::get<std::string>(color);
    } else if (std::holds_alternative<svg::Rgb>(color)) {
        *result.mutable_color_rgb() = std::move(SerializeRgb(std::get<svg::Rgb>(color)));
    } else if (std::holds_alternative<svg::Rgba>(color)) {
        *result.mutable_color_rgba() = std::move(SerializeRgba(std::get<svg::Rgba>(color)));
    } else {
        return {};
    }
    return result;
}
svg::Color DeserializeColor(const tc_serialize::Color& color) {
    svg::Color result;
    if (color.has_color_rgb()) {
        result = DeserializeRgb(color.color_rgb());
    } else if (color.has_color_rgba()) {
        result = DeserializeRgba(color.color_rgba());
    } else if (!color.color_string().empty()) {
        result = color.color_string();
    } else {
        return {};
    }
    return result;
}

tc_serialize::RenderSettings SerializeRendererSettings(const RendererSettings& settings) {
    tc_serialize::RenderSettings result;
    result.set_width(settings.width);
    result.set_height(settings.height);
    result.set_padding(settings.padding);
    result.set_line_width(settings.line_width);
    result.set_stop_radius(settings.stop_radius);
    result.set_bus_label_font_size(settings.bus_label_font_size);
    *result.mutable_bus_label_offset() = std::move(SerializePoint(settings.bus_label_offset));
    result.set_stop_label_font_size(settings.stop_label_font_size);
    *result.mutable_stop_label_offset() = std::move(SerializePoint(settings.stop_label_offset));
    *result.mutable_underlayer_color() = std::move(SerializeColor(settings.underlayer_color));
    result.set_underlayer_width(settings.underlayer_width);
    for (const svg::Color& color: settings.color_palette) {
        *result.add_color_palette() = std::move(SerializeColor(color));
    }
    return std::move(result);
}
RendererSettings DeserializeRenderSetting(const tc_serialize::RenderSettings& settings) {
    RendererSettings result;
    result.width = settings.width();
    result.height = settings.height();
    result.padding = settings.padding();
    result.line_width = settings.line_width();
    result.stop_radius = settings.stop_radius();
    result.bus_label_font_size = settings.bus_label_font_size();
    result.bus_label_offset = DeserializePoint(settings.bus_label_offset());
    result.stop_label_font_size = settings.stop_label_font_size();
    result.stop_label_offset = DeserializePoint(settings.stop_label_offset());
    result.underlayer_color = std::move(DeserializeColor(settings.underlayer_color()));
    result.underlayer_width = settings.underlayer_width();
    for (int i = 0; i < settings.color_palette_size(); ++i) {
        result.color_palette.emplace_back(std::move(DeserializeColor(settings.color_palette(i))));
    }
    return std::move(result);
}

tc_serialize::RoutingSettings SerializeRouting(const RoutingSettings& settings) {
    tc_serialize::RoutingSettings result;
    result.set_bus_wait_time(settings.bus_wait_time);
    result.set_bus_velocity(settings.bus_velocity);
    return std::move(result);
}
RoutingSettings DeserializeRouting(const tc_serialize::RoutingSettings& settings) {
    RoutingSettings result{};
    result.bus_wait_time = settings.bus_wait_time();
    result.bus_velocity = settings.bus_velocity();
    return result;
}