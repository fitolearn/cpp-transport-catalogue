#pragma once

#include <map>
#include <set>
#include <functional>
#include <deque>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include "geo.h"
#include "domain.h"
#include "graph.h"
#include "serialization.h"
#include "transport_catalogue.pb.h"

namespace transport {

    struct StopsPointers {
    public:
        const Stop* stop;
        const Stop* other;
        size_t operator()(const StopsPointers& st_pair) const{
            return hasher_(st_pair.stop) + 43*hasher_(st_pair.other);
        }
        bool operator()(const StopsPointers& lhs, const StopsPointers& rhs) const{
            return lhs.stop == rhs.stop && lhs.other == rhs.other;
        }
    private:
        std::hash<const Stop*> hasher_;
    };

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;
        bool AddBus(const BusRoute& bus_route);
        bool RestoreFromSerializedTransportCat(tc_serialize::TransportCatalogue& t_cat);
        bool SetDistanceBetweenStops(std::string_view stop, std::string_view other_stop, int distance);
        int GetDistanceBetweenStops(std::string_view stop, std::string_view other_stop) const;
        void AddStop(const std::string& name, geo::Coordinates coords);
        void AddStop(const Stop& stop);
        void SaveToSerializeTransportCat(tc_serialize::TransportCatalogue& t_cat) const;
        const std::unordered_map<std::string_view, const Stop*>& RawStopsIndex() const;
        const BusRoute& FindBus(std::string_view name) const;
        const std::set<std::string_view>& GetBusesForStop(std::string_view stop) const;
        uint32_t GetStopId(std::string_view stop_name) const;
        BusInfo GetBusInfo(std::string_view bus_name) const;
        std::pair<bool, const Stop&> FindStop(std::string_view name) const;
        std::map<std::string_view, const BusRoute*> GetAllRoutesIndex() const;
        std::map<std::string_view, const Stop*> GetAllStopsIndex() const;
        std::string_view GetStopNameById(uint32_t stop_id) const;
    private:
        uint32_t stop_id_counter_ = 0;
        std::deque<Stop> stops_;
        std::deque<BusRoute> bus_routes_;
        std::unordered_map<std::string_view, const Stop*> stops_index_;
        std::unordered_map<uint32_t, const Stop*> stop_id_name_index_;
        std::unordered_map<std::string_view, const BusRoute*> routes_index_;
        std::unordered_map<std::string_view, std::set<std::string_view>> stop_and_buses_;
        std::unordered_map<StopsPointers, int, StopsPointers, StopsPointers> stops_distance_index_;
    };
} // namespace transport_catalogue
