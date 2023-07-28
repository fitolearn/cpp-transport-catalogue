#include "map_renderer.h"
#include "domain.h"

using namespace std::literals;

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.longitude - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.latitude) * zoom_coeff_ + padding_
    };
}

void MapRenderer::RenderSvgMap(const transport::TransportCatalogue &tc, svg::Document& svg_doc) {
    const std::map<std::string_view, const transport::Stop*> stops = tc.GetAllStopsIndex();
    stops_ = &stops;
    std::vector<geo::Coordinates> all_route_stops_coordinates;
    for (const auto& stop : stops) {
        if (tc.GetBusesForStop(stop.first).empty()) { continue; }
        all_route_stops_coordinates.emplace_back(stop.second->coordinates);
    }
    SphereProjector projector(all_route_stops_coordinates.begin(), all_route_stops_coordinates.end(),settings_.width, settings_.height, settings_.padding);
    projector_ = &projector;
    const std::map<std::string_view, const transport::BusRoute*> routes = tc.GetAllRoutesIndex();
    routes_ = &routes;
    RenderLines(svg_doc);
    RenderRouteNames(svg_doc);
    RenderStopCircles(tc, svg_doc);
    RenderStopNames(tc, svg_doc);
    // to avoid dangling references
    stops_ = nullptr;
    routes_ = nullptr;
    projector_ = nullptr;
}

void MapRenderer::RenderSvgMap(const transport::TransportCatalogue &tc, std::ostream& out) {
    svg::Document svg_doc;
    RenderSvgMap(tc, svg_doc);
    svg_doc.Render(out);
}

svg::Color MapRenderer::GetNextColor(size_t &color_count) const {
    if (color_count >= settings_.color_palette.size()) { color_count = 0; }
    return settings_.color_palette[color_count++];
}

void MapRenderer::RenderLines(svg::Document &svg_doc) const {
    size_t color_count = 0;
    auto projector = *projector_;
    for (const auto route : *routes_) {
        if (route.second->route_stops.empty()) { continue; }
        svg::Color palette_color = GetNextColor(color_count);
        svg::Polyline line;
        line.SetStrokeColor(palette_color).SetFillColor({}).SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (auto route_stop : route.second->route_stops) {
            line.AddPoint(projector( route_stop->coordinates ));
        }
        if (route.second->type == transport::RouteType::RETURN_ROUTE) {
            for (auto back_iter = std::next(route.second->route_stops.rbegin()); back_iter != route.second->route_stops.rend(); ++back_iter) {
                line.AddPoint(projector( (*back_iter)->coordinates ));
            }
        }
        svg_doc.Add(std::move(line));
    }
}

void MapRenderer::RenderRouteNames(svg::Document& svg_doc) const {
    auto projector = *projector_;
    size_t color_count = 0;
    for (auto route : *routes_) {
        if (route.second->route_stops.empty()) { continue; }
        svg::Text name_start_text;
        name_start_text.SetData(std::string{route.first}).SetPosition( projector(route.second->route_stops.front()->coordinates) )
                .SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetFillColor(GetNextColor(color_count));

        svg::Text name_start_plate = name_start_text;
        name_start_plate.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        svg_doc.Add(name_start_plate);
        svg_doc.Add(name_start_text);
        if (route.second->type == transport::RouteType::CIRCLE_ROUTE) { continue; }
        if (route.second->route_stops.front()->stop_name == route.second->route_stops.back()->stop_name) { continue; }
        name_start_text.SetPosition(projector(route.second->route_stops.back()->coordinates));
        name_start_plate.SetPosition(projector(route.second->route_stops.back()->coordinates));
        svg_doc.Add(name_start_plate);
        svg_doc.Add(name_start_text);
    }
}

void MapRenderer::RenderStopCircles(const transport::TransportCatalogue& tc, svg::Document& svg_doc) const {
    auto projector = *projector_;
    for (const auto& stop : *stops_) {
        if ( tc.GetBusesForStop(stop.first).empty() ) { continue; }
        svg::Circle stop_circle;
        stop_circle.SetCenter( projector(stop.second->coordinates) ).SetRadius(settings_.stop_radius).SetFillColor("white"s);
        svg_doc.Add(stop_circle);
    }
}

void MapRenderer::RenderStopNames(const transport::TransportCatalogue& tc, svg::Document& svg_doc) const {
    auto projector = *projector_;
    for (const auto& stop : *stops_) {
        if (tc.GetBusesForStop(stop.first).empty()) { continue; }
        svg::Text stop_name;
        stop_name.SetPosition(projector(stop.second->coordinates)).SetOffset(settings_.stop_label_offset)
                .SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s).SetData(std::string{stop.first});
        svg::Text stop_plate = stop_name;
        stop_plate.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        stop_name.SetFillColor("black"s);
        svg_doc.Add(stop_plate);
        svg_doc.Add(stop_name);
    }
}