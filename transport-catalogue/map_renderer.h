#pragma once
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "json.h"
#include "serialization.h"

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding)
            : padding_(padding){
        if (points_begin == points_end) { return; }
        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(points_begin,points_end,[](auto lhs, auto rhs) {
                                                                 return lhs.longitude < rhs.longitude;
                                                             }
        );
        min_lon_ = left_it->longitude;
        const double max_lon = right_it->longitude;
        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] =std::minmax_element(points_begin,points_end,[](auto lhs, auto rhs) {
                                                                return lhs.latitude < rhs.latitude;
                                                            }
        );
        const double min_lat = bottom_it->latitude;
        max_lat_ = top_it->latitude;
        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }
        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }
        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые, берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }
    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;
private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RendererSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    double bus_label_font_size;
    svg::Point bus_label_offset;
    double stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

class MapRenderer{
public:
    explicit MapRenderer(const RendererSettings& settings) : settings_(settings) {}
    void RenderSvgMap(const transport::TransportCatalogue& tc, std::ostream& out);
    void RenderSvgMap(const transport::TransportCatalogue &tc, svg::Document& svg_doc);
private:
    SphereProjector* projector_ = nullptr;
    svg::Color GetNextColor(size_t &color_count) const;
    const RendererSettings& settings_;
    const std::map<std::string_view, const transport::BusRoute*>* routes_ = nullptr;
    const std::map<std::string_view, const transport::Stop*>* stops_ = nullptr;
    void RenderLines(svg::Document& svg_doc) const;
    void RenderRouteNames(svg::Document& svg_doc) const;
    void RenderStopCircles(const transport::TransportCatalogue& tc, svg::Document& svg_doc) const;
    void RenderStopNames(const transport::TransportCatalogue& tc, svg::Document& svg_doc) const;
};