#pragma once
#include <string>
#include <vector>
#include <set>
#include <iomanip>
#include "geo.h"

namespace transport {

    struct Stop {
        uint32_t id = 0;
        std::string stop_name;
        geo::Coordinates coordinates;
    };

    struct StopDistanceData {
        std::string other_stop_name;
        size_t distance = 0;
    };

    struct StopWithDistances : Stop {
        std::vector<StopDistanceData> distances;
    };

    enum RouteType {
        NOT_SET,
        CIRCLE_ROUTE,
        RETURN_ROUTE
    };

    struct BusRoute {
        std::string bus_name;
        RouteType type;
        std::vector<const Stop *> route_stops;
    };

    const Stop EMPTY_STOP{};
    const BusRoute EMPTY_BUS_ROUTE{};
    const std::set<std::string_view> EMPTY_BUS_ROUTE_SET{};

    struct BusInfo {
        std::string_view bus_name;
        RouteType route_type;
        size_t stops_number;
        size_t unique_stops_counter;
        size_t route_length;
        double curvature;
    };
    std::ostream &operator<<(std::ostream &os, const BusInfo &bus_info);
} // namespace transport_catalogue