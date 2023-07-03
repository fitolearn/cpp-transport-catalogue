#include "transport_router.h"
using namespace std;

namespace transport {

    Router::Router(const size_t graph_size)
            : graph_(graph_size)
    {}

    void Router::BuildRouter() {
        if (!router_ && graph_) { router_.emplace(GraphRouter(*graph_)); }
    }

    void Router::AddEdgesToGraph() {
        for (auto& edge_info : edges_) {
            graph_->AddEdge(edge_info.edge);
        }
    }

    void Router::SetSettings(double bus_wait_time, double bus_velocity) {
        settings_ = {bus_wait_time, bus_velocity};
    }

    void Router::AddWaitEdge(string_view stop_name) {
        EdgeInfo new_edge{
            {stop_to_vertex_id_[stop_name].start_wait, stop_to_vertex_id_[stop_name].end_wait,
             settings_.wait_time},
            stop_name,
            -1,
            settings_.wait_time};
        edges_.emplace_back(new_edge);
    }

    void Router::AddBusEdge(string_view stop_from, string_view stop_to, string_view bus_name, int span_count, int dist) {
        EdgeInfo new_edge{
                {stop_to_vertex_id_[stop_from].end_wait, stop_to_vertex_id_[stop_to].start_wait,
                 dist / settings_.velocity * TO_MINUTES},
                bus_name,
                span_count,
                dist / settings_.velocity * TO_MINUTES};
        edges_.emplace_back(new_edge);
    }

    void Router::AddStop(string_view stop_name) {
        if (!stop_to_vertex_id_.count(stop_name)) {
            size_t sz = stop_to_vertex_id_.size();
            stop_to_vertex_id_[stop_name] = { sz * 2, sz * 2 + 1 };
        }
    }

    void Router::BuildGraph() {
        if (!graph_) { graph_ = std::move(Graph(stop_to_vertex_id_.size() * 2)); }
        AddEdgesToGraph();
    }

    optional<RouteInfo> Router::GetRouteInfo(string_view from, string_view to) const {
        const auto rout = router_->BuildRoute( stop_to_vertex_id_.at(from).start_wait, stop_to_vertex_id_.at(to).start_wait);
        if (!rout) { return nullopt; }
        return RouteInfo{rout->weight,MakeItemsByEdgeIds(rout->edges)};
    }

    vector<RouteItem> Router::MakeItemsByEdgeIds(const vector<graph::EdgeId>& edge_ids) const {
        vector<RouteItem> result;
        result.reserve(edge_ids.size());

        for (const auto id : edge_ids) {
            const EdgeInfo& edge_info = edges_[id];
            RouteItem tmp;
            if (edge_info.span_count == -1) {
                tmp.wait_item = {edge_info.name,edge_info.time};
            } else {
                tmp.bus_item = {edge_info.name,edge_info.span_count,edge_info.time};
            }
            result.emplace_back(tmp);
        } return result;
    }
}
