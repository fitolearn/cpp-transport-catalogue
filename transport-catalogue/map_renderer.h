#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <optional>
#include <cmath>
#include <utility>
#include <cstdlib>
#include "svg.h"
#include "domain.h"
#include "geo.h"

namespace map_renderer {

    const double EPSILON = 1e-6;

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding)
                : padding_(padding){
            if (points_begin == points_end) {
                return;
            }
            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] =
                    std::minmax_element(
                            points_begin,
                            points_end,
                            [](auto lhs, auto rhs) {
                                return lhs.longitude < rhs.longitude;
                            }
                    );
            min_lon_ = left_it->longitude;
            const double max_lon = right_it->longitude;
            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] =
                    std::minmax_element(
                            points_begin,
                            points_end,
                            [](auto lhs, auto rhs) {
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
        static bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    };

    struct RenderingSettings {
        double width = 0;
        double height = 0;
        double padding = 0;
        double line_width = 0;
        double stop_radius = 0;
        double underlayer_width = 0;
        int bus_label_font_size = 0;
        int stop_label_font_size = 0;
        svg::Point bus_label_offset;
        svg::Point stop_label_offset;
        svg::Color underlayer_color;
        std::vector<svg::Color> color_palette;
    };

    class MapRenderer {
    public:
        MapRenderer() = default;
        void SetSettings(RenderingSettings settings);
        svg::Document MakeDocument(std::vector<domain::BusPtr>&& buses, std::vector<std::pair<domain::StopPtr, domain::StopStat>>&& stops) const;

    private:
        RenderingSettings settings_;
        template <typename It>
        std::vector<geo::Coordinates> StopsToCoordinates(It begin, It end) const {
            std::vector<geo::Coordinates> result;
            result.reserve(end - begin);
            for (It it = begin; it != end; ++it) {
                if (it->second.passing_buses != nullptr && !it->second.passing_buses->empty()) {
                    result.emplace_back(geo::Coordinates{ it->first.get()->latitude, it->first.get()->longitude });
                }
            } return result;
        }

        void AddBusesLines(svg::Document& doc, SphereProjector& proj, const std::vector<domain::BusPtr>& buses) const;
        void AddBusesNames(svg::Document& doc, SphereProjector& proj, const std::vector<domain::BusPtr>& buses) const;
        void AddStopsCircles(svg::Document& doc, SphereProjector& proj, const std::vector<std::pair<domain::StopPtr, domain::StopStat>>& stops) const;
        void AddStopsNames(svg::Document& doc, SphereProjector& proj, const std::vector<std::pair<domain::StopPtr, domain::StopStat>>& stops) const;
    };
}
