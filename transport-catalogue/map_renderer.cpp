#include <algorithm>
#include "map_renderer.h"

using namespace std::literals;
using namespace domain;

namespace map_renderer {

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {(coords.longitude - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.latitude) * zoom_coeff_ + padding_};
    }

    void MapRenderer::SetSettings(RenderingSettings settings) {
        settings_ = std::move(settings);
    }

    svg::Document MapRenderer::MakeDocument(std::vector<BusPtr>&& buses, std::vector<std::pair<StopPtr, StopStat>>&& stops) const {
        svg::Document result;
        std::sort(buses.begin(),buses.end(),[](const BusPtr& lhs, const BusPtr& rhs) {
                    return std::lexicographical_compare(
                            lhs->name->begin(), lhs->name->end(),
                            rhs->name->begin(), rhs->name->end());});
        std::sort(stops.begin(),stops.end(),
                [](const std::pair<StopPtr, StopStat>& lhs, const std::pair<StopPtr, StopStat>& rhs) {
                    return std::lexicographical_compare(
                            lhs.first->name->begin(), lhs.first->name->end(),
                            rhs.first->name->begin(), rhs.first->name->end());});
        const auto coordinates = StopsToCoordinates(stops.begin(), stops.end());
        SphereProjector projector(coordinates.begin(), coordinates.end(), settings_.width, settings_.height, settings_.padding);
        AddBusesLines(result, projector, buses);
        AddBusesNames(result, projector, buses);
        AddStopsCircles(result, projector, stops);
        AddStopsNames(result, projector, stops);
        return result;
    }

    void MapRenderer::AddBusesLines(svg::Document& doc, SphereProjector& proj, const std::vector<BusPtr>& buses) const {
        size_t cnt = 0;
        size_t sz_palette = settings_.color_palette.size();
        for (const BusPtr& bus : buses) {
            if (bus.get()->route.empty()) {
                continue;
            }
            svg::Polyline polyline;
            polyline // Designated initializers https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers
                    .SetStrokeColor(settings_.color_palette[cnt++ % sz_palette])
                    .SetFillColor(svg::Color())
                    .SetStrokeWidth(settings_.line_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            cnt = cnt == sz_palette ? 0u : cnt;
            for (const auto& stop_ptr : bus.get()->route) {
                const auto& stop = *stop_ptr;
                polyline.AddPoint(proj({ stop.latitude, stop.longitude }));
            }
            doc.Add(std::move(polyline));
        }
    }

    void MapRenderer::AddBusesNames(svg::Document& doc, SphereProjector& proj, const std::vector<BusPtr>& buses) const {
        size_t cnt = 0;
        size_t sz_palette = settings_.color_palette.size();
        for (const BusPtr& bus_ptr : buses) {
            const auto& bus = *bus_ptr.get();
            if (bus.route.empty()) {
                continue;
            }
            svg::Text text;
            text // Designated initializers https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers
                    .SetPosition(proj({bus.route[0].get()->latitude, bus.route[0].get()->longitude }))
                    .SetOffset(settings_.bus_label_offset)
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s)
                    .SetData(*bus.name);
            svg::Text text_substrate = text;
            text_substrate // Designated initializers https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            text.SetFillColor(settings_.color_palette[cnt++ % sz_palette]);
            cnt = cnt == sz_palette ? 0u : cnt;
            svg::Text text_substrate_last_stop = text_substrate;
            svg::Text text_last_stop = text;
            doc.Add(std::move(text_substrate));
            doc.Add(std::move(text));

            if (bus.last_stop_name != nullptr && bus.last_stop_name.get() != bus.route.front().get()) {
                svg::Point p = proj({ bus.last_stop_name.get()->latitude, bus.last_stop_name.get()->longitude });
                text_substrate_last_stop.SetPosition(p);
                doc.Add(std::move(text_substrate_last_stop));
                text_last_stop.SetPosition(p);
                doc.Add(std::move(text_last_stop));
            }
        }
    }

    void MapRenderer::AddStopsCircles(svg::Document& doc, SphereProjector& proj, const std::vector<std::pair<StopPtr, StopStat>>& stops) const {
        for (const auto& [stop_ptr, stop_stat] : stops) {
            const auto& stop = *stop_ptr.get();
            if (stop_stat.passing_buses == nullptr || stop_stat.passing_buses->empty()) {
                continue;
            }
            svg::Circle circle;
            circle // Designated initializers https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers
                    .SetCenter(proj({ stop.latitude, stop.longitude }))
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white"s);

            doc.Add(std::move(circle));
        }
    }

    void MapRenderer::AddStopsNames(svg::Document& doc, SphereProjector& proj, const std::vector<std::pair<StopPtr, StopStat>>& stops) const {
        for (const auto& [stop_ptr, stop_stat] : stops) {
            const auto& stop = *stop_ptr.get();
            if (stop_stat.passing_buses == nullptr || stop_stat.passing_buses->empty()) {
                continue;
            }
            svg::Text text;
            text // Designated initializers https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers
                    .SetPosition(proj({ stop.latitude, stop.longitude }))
                    .SetOffset(settings_.stop_label_offset)
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetFontFamily("Verdana"s)
                    .SetData(*stop.name);

            svg::Text text_substrate = text;
            text_substrate // Designated initializers https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(std::move(text_substrate));
            text.SetFillColor("black"s);
            doc.Add(std::move(text));
        }
    }
}
