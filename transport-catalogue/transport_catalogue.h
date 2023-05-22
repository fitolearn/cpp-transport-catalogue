#pragma once
#include <string_view>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <list>
#include <set>
#include <map>
#include <deque>
#include <functional>
#include <iostream>
#include "domain.h"

namespace transport {
    template <typename Pointer>
    struct PairPointerHasher {
        size_t operator() (const std::pair<Pointer, Pointer>& f) const {
            static constexpr std::hash<Pointer> hasher;
            return hasher(f.first) + hasher(f.second) * 37;
        }
    };

    class TransportCatalogue {
    public:
        template<typename Container>
        void AddBus(std::string name, bool circular, const Container& stop_names);
        const domain::Bus* GetBus(const std::string_view bus_name) const;
        size_t GetStopsCount(const domain::Bus* bus) const;
        size_t GetUniqueStopsCount(const domain::Bus* bus) const;
        double GetGeoLength(const domain::Bus* bus) const;
        size_t GetLength(const domain::Bus* bus) const;
        const std::deque<domain::Bus>& GetBuses() const;

        void AddStop(std::string name, geo::Coordinates coordinates);
        const domain::Stop* GetStop(const std::string_view stop_name) const;
        std::set<std::string> GetBusesNamesFromStop(const domain::Stop*) const;
        size_t GetLengthFromTo(const domain::Stop* from, const domain::Stop* to) const;
        const std::unordered_set<domain::Bus*>* GetBusesByStop(const domain::Stop*) const;

        void SetLengthBetweenStops(const std::unordered_map<std::string, std::unordered_map<std::string, size_t>>& length_from_to);

        const std::deque<domain::Stop>& GetStops() const;
    private:
        std::deque<domain::Bus> buses_storage_;
        std::deque<domain::Stop> stops_storage_;
        std::unordered_map<std::string_view, domain::Stop*> stops_;
        std::unordered_map<std::string_view, domain::Bus*> buses_;
        std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, size_t, PairPointerHasher<const domain::Stop*>> length_from_to_;
        std::unordered_map<const domain::Stop*, std::unordered_set<domain::Bus*>> buses_of_stop_;
    };

    template<typename Container>
    void TransportCatalogue::AddBus(std::string name, bool circular, const Container& stop_names) {
        domain::ConteinerOfStopPointers stops;

        for (auto& name : stop_names) {
            stops.push_back(stops_[name]);
        }
        std::unordered_set<domain::Stop*> stops_set{ stops.begin(), stops.end() };
        domain::Bus bus{std::move(name), circular, std::move(stops), std::move(stops_set)
        };
        buses_storage_.push_back(std::move(bus));
        domain::Bus* pbus = &buses_storage_.back();
        buses_[pbus->name_] = pbus;
        for (domain::Stop* pstop : pbus->stops_set_) {
            buses_of_stop_[pstop].insert(pbus);
        }
    }
}
