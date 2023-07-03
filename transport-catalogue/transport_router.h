#pragma once
#include <string>
#include <optional>
#include <utility>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <functional>
#include "graph.h"
#include "router.h"

namespace transport {

    static constexpr double TO_MINUTES = (3.6 / 60.0);
// ---------------------------------------------------------------- Structs
    struct RouteItemBus {
        std::string_view bus_name;
        int span_count;
        double time;
    };

    /*
     items — список элементов маршрута, каждый из которых описывает непрерывную активность пассажира, требующую временных затрат. А именно элементы маршрута бывают двух типов.
Wait — подождать нужное количество минут (в нашем случае всегда bus_wait_time) на указанной остановке:
     Bus — проехать span_count остановок (перегонов между остановками) на автобусе bus, потратив указанное количество минут:
     */

    struct RouteItemWait {
        std::string_view stop_name;
        double time;
    };

    struct RouteItem {
        std::optional<RouteItemWait> wait_item;
        std::optional<RouteItemBus> bus_item;
    };

    struct RouteInfo {
        double total_time = 0.;
        std::vector<RouteItem> items;
    };

    struct EdgeInfo {
        graph::Edge<double> edge;
        std::string_view name;
        int span_count = -1;
        double time = 0.;
    };
// ----------------------------------------------------------------

    class Router {
    private:
        using Graph = graph::DirectedWeightedGraph<double>;
        using GraphRouter = graph::Router<double>;

        struct Settings {
            double wait_time = 6;
            double velocity  = 40.;
        };

        /*"routing_settings": {
            "bus_wait_time": 6,
            bus_velocity": 40
            } */

        struct Vertexes {
            size_t start_wait;
            size_t end_wait;
        };

        Settings settings_;
        std::optional<Graph> graph_ = std::nullopt;
        std::optional<GraphRouter> router_ = std::nullopt;
        std::unordered_map<std::string_view, Vertexes, std::hash<std::string_view>> stop_to_vertex_id_;
        std::vector<EdgeInfo> edges_;
        std::vector<RouteItem> MakeItemsByEdgeIds(const std::vector<graph::EdgeId>& edge_ids) const;
    public:
        Router() = default;
        explicit Router(size_t graph_size);
        void BuildGraph();
        void BuildRouter();
        void AddEdgesToGraph();
        void SetSettings(double bus_wait_time, double bus_velocity);
        void AddWaitEdge(std::string_view stop_name);
        void AddBusEdge(std::string_view stop_from, std::string_view stop_to, std::string_view bus_name, int span_count, int dist);
        void AddStop(std::string_view stop_name);
        std::optional<RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const;
    };
}
