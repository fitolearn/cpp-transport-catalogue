#pragma once
#include <string>
#include <vector>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <optional>
#include <memory>
#include "domain.h"

using namespace domain;

namespace transport {

    class TransportCatalogue {
    private:
        using StopsPair = std::pair<StopPtr, StopPtr>;

        class StopsPairHasher {
            std::hash<const void*> hash_;
        public:
            std::size_t operator()(const StopsPair& stops_pair) const;
        };

        std::deque<std::shared_ptr<Stop>> stops_;
        std::deque<std::shared_ptr<Bus>> buses_;
        std::unordered_map<std::string_view, BusPtr, std::hash<std::string_view>> name_to_bus_;
        std::unordered_map<std::string_view, StopPtr, std::hash<std::string_view>> name_to_stop_;
        std::unordered_map<StopsPair, int, StopsPairHasher> stops_pair_to_distance_;
        std::unordered_map<StopPtr, std::unordered_set<domain::BusPtr>, std::hash<StopPtr>> stop_to_passing_buses_;
        void AddToStopPassingBuses(const std::vector<StopPtr>& stops, std::string_view bus_name);

    public:
        void AddBus(Bus&& bus);
        void AddStop(Stop&& stop);
        void SetDistanceBetweenStops(std::string_view first, std::string_view second, int distance);
        BusPtr SearchBus(std::string_view name) const;
        StopPtr SearchStop(std::string_view name) const;
        std::optional<int> GetActualDistanceBetweenStops(std::string_view stop1_name, std::string_view stop2_name) const;
        std::optional<double> GetGeographicDistanceBetweenStops(std::string_view stop1_name, std::string_view stop2_name) const;
        const std::unordered_set<BusPtr>* GetPassingBusesByStop(const StopPtr& stop) const;
        std::vector<BusPtr> GetBusesInVector() const;
        std::vector<StopPtr> GetStopsInVector() const;
    };
}
