#pragma once
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "router.h"
#include <memory>

/*"routing_settings": {
            "bus_wait_time": 6,
            bus_velocity": 40
            } */

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

struct TwoStopsLink {
    std::string_view bus_name = {};
    graph::VertexId stop_from = {};
    graph::VertexId stop_to = {};
    size_t number_of_stops = {};

    explicit TwoStopsLink(std::string_view bus, graph::VertexId from, graph::VertexId to, size_t num) :
            bus_name(bus), stop_from(from), stop_to(to), number_of_stops(num) {
    }
    TwoStopsLink() = default;

    size_t operator()(const TwoStopsLink& sor) const {
        return hasher_num_(number_of_stops) + 43 * hasher_num_(sor.stop_from) +
               43 * 43 * hasher_num_(sor.stop_to) + 43 * 43 * 43 * hasher_(bus_name);
    }
    bool operator()(const TwoStopsLink& lhs, const TwoStopsLink& rhs) const {
        return lhs.bus_name == rhs.bus_name && lhs.stop_from == rhs.stop_from
               && lhs.stop_to == rhs.stop_to && lhs.number_of_stops == rhs.number_of_stops;
    }
private:
    std::hash<size_t> hasher_num_;
    std::hash<std::string_view> hasher_;
};

class TransportCatalogueRouterGraph : public graph::DirectedWeightedGraph<double> {
public:
    struct StopOnRoute {
        size_t stop_number;
        std::string_view stop_name;
        std::string_view bus_name;

        explicit StopOnRoute(size_t num, std::string_view stop, std::string_view bus) : stop_number(num), stop_name(stop), bus_name(bus) {
        }

        StopOnRoute() = default;
        size_t operator()(const StopOnRoute& sor) const {
            return hasher_num_(stop_number) + 43 * hasher_(sor.stop_name) + 43 * 43 * hasher_(sor.bus_name);
        }
        bool operator()(const StopOnRoute& lhs, const StopOnRoute& rhs) const {
            return lhs.stop_name == rhs.stop_name && lhs.bus_name == rhs.bus_name && lhs.stop_number == rhs.stop_number;
        }
    private:
        std::hash<size_t> hasher_num_;
        std::hash<std::string_view> hasher_;
    };
    TransportCatalogueRouterGraph(const transport::TransportCatalogue& tc, RoutingSettings rs);
    TransportCatalogueRouterGraph(const transport::TransportCatalogue& tc, RoutingSettings rs, const tc_serialize::TransportCatalogue& tc_);
    ~TransportCatalogueRouterGraph() = default;
    bool SaveTo(tc_serialize::TransportCatalogue& tc_to) const;
    bool RestoreFrom(const tc_serialize::TransportCatalogue& tc_from);
    std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to) const;
    const StopOnRoute& GetStopById(graph::VertexId id) const;
    const TwoStopsLink& GetLinkById(graph::EdgeId id) const;
    double GetBusWaitingTime() const;
private:
    const transport::TransportCatalogue& tc_;
    RoutingSettings rs_;
    graph::EdgeId edge_count_ = 0;
    std::unique_ptr<graph::Router<double>> router_ptr_;
    std::unordered_map<StopOnRoute, graph::VertexId , StopOnRoute, StopOnRoute> stop_to_vertex_;
    std::unordered_map<size_t , StopOnRoute> vertex_to_stop_;
    graph::VertexId vertex_id_count_ = 0;
    std::unordered_map<TwoStopsLink, graph::EdgeId, TwoStopsLink, TwoStopsLink> stoplink_to_edge_;
    std::unordered_map<graph::EdgeId, TwoStopsLink> edge_to_stoplink_;
    graph::VertexId RegisterStop(const StopOnRoute& stop);
    graph::EdgeId StoreLink(const TwoStopsLink& link, graph::EdgeId edge);
    graph::VertexId GetStopVertexId(std::string_view stop_name) const;
    TwoStopsLink DeserializeTwoStopsLink(const tc_serialize::TwoStopsLinkPB& link) const;
    tc_serialize::StopOnRoutePB SerializeStopOnRoute(const StopOnRoute& stop, graph::VertexId vertexId) const;
    StopOnRoute DeserializeStopOnRoute(const tc_serialize::StopOnRoutePB& stop);
    void FillWithReturnRouteStops(const transport::BusRoute* bus_route);
    void FillWithCircleRouteStops(const transport::BusRoute* bus_route);
    double CalculateTimeForDistance(int distance) const;
    static tc_serialize::TwoStopsLinkPB SerializeTwoStopsLink(const TwoStopsLink& link, graph::EdgeId edge) ;
    static tc_serialize::EdgePB SerializeEdge(const graph::Edge<double>& edge) ;
    static graph::Edge<double> DeserializeEdge(const tc_serialize::EdgePB& edge) ;
    static tc_serialize::IncListPB SerializeIncList(const std::vector<size_t>& list) ;
    static std::vector<size_t> DeserializeIncList(const tc_serialize::IncListPB& list) ;
};