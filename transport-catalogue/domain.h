#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <memory>
#include <optional>
#include <list>
#include "geo.h"

namespace domain {

    struct Bus;
    struct Stop;
    using BusPtr = std::shared_ptr<Bus>;
    using StopPtr = std::shared_ptr<Stop>;

    struct Stop {
        Stop(std::string name, double lat, double lng);
        [[nodiscard]] double GetDistanceTo(const StopPtr& stop_to) const;
        std::shared_ptr<std::string> name;
        double latitude = 0.;
        double longitude = 0.;
    };

    struct Bus {
        Bus(std::string name, std::vector<StopPtr> route, int unique, int actual, double geo, StopPtr last_stop = nullptr);
        Bus& operator=(const Bus& bus) = default;
        std::shared_ptr<std::string> name;
        std::vector<StopPtr> route;
        int unique_stops = 0;
        int route_actual_length = 0;
        double route_geographic_length = 0;
        StopPtr last_stop_name;
    };

    struct BusStat {
        std::string_view name;
        int stops_on_route = 0;
        int unique_stops  = 0;
        int routh_actual_length = 0;
        double curvature = 0.;
    };

    struct StopStat {
        std::string_view name;
        const std::unordered_set<BusPtr>* passing_buses;
    };

    //double ComputeDistance(const Stop*, const Stop*);
}
