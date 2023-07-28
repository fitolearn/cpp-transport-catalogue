#include "transport_catalogue.h"
#include <utility>
#include <iostream>
#include <set>

namespace transport{

    void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates coords) {
        const Stop stop {0, name, coords};
        AddStop(stop);
    }

    void TransportCatalogue::AddStop(const Stop& stop) {
        if (stops_index_.count(stop.stop_name) > 0) return;
        Stop* ptr = &stops_.emplace_back(stop);
        if (ptr->id == 0){
            ptr->id = ++stop_id_counter_;
        } else {
            stop_id_counter_ = ptr->id;
        }
        std::string_view stop_name(ptr->stop_name);
        stops_index_.emplace(stop_name, ptr);
        stop_id_name_index_.emplace(ptr->id, ptr);
    }

    std::pair<bool, const Stop&> TransportCatalogue::FindStop(const std::string_view name) const {
        const auto it = stops_index_.find(name);
        if (it == stops_index_.end()) {
            return {false, EMPTY_STOP};
        }
        return {true, *it->second};
    }

    bool TransportCatalogue::AddBus(const BusRoute &bus_route) {
        const auto it = routes_index_.find(bus_route.bus_name);
        if (it != routes_index_.end()) { return false; }
        const BusRoute* ptr = &bus_routes_.emplace_back(bus_route);
        std::string_view bus_name (ptr->bus_name);
        routes_index_.emplace(bus_name, ptr);
        for (const Stop* stop : ptr->route_stops) {
            stop_and_buses_[stop->stop_name].insert(bus_name);
        }
        return true;
    }

    const BusRoute& TransportCatalogue::FindBus(std::string_view name) const {
        const auto iter = routes_index_.find(name);
        if (iter == routes_index_.end()) {
            return EMPTY_BUS_ROUTE;
        }
        return *iter->second;
    }

    BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
        BusInfo result;
        result.route_type = RouteType::NOT_SET;
        auto iter = routes_index_.find(bus_name);
        if (iter == routes_index_.end()) { return result; }
        const BusRoute& route = *iter->second;
        std::set<const Stop*> unique_stops(route.route_stops.begin(), route.route_stops.end());
        result.unique_stops_counter = unique_stops.size();
        double length_geo = 0.0;
        size_t length_meters = 0;
        for(auto first = route.route_stops.begin(); first != route.route_stops.end(); ++first) {
            auto second = std::next(first);
            if (second == route.route_stops.end()) {
                break;
            }
            length_geo += ComputeDistance((**first).coordinates, (**second).coordinates);
            length_meters += GetDistanceBetweenStops((**first).stop_name, (**second).stop_name);
            if (route.type == RouteType::RETURN_ROUTE) {
                length_meters += GetDistanceBetweenStops((**second).stop_name, (**first).stop_name);
            }
        }
        result.stops_number = route.route_stops.size();
        if (route.type == RouteType::RETURN_ROUTE){
            result.stops_number *= 2;
            result.stops_number -= 1;
            length_geo *= 2;
        }
        result.route_length = length_meters;
        result.curvature = static_cast<double>(length_meters) / length_geo;
        result.bus_name = route.bus_name;
        result.route_type = route.type;
        return result;
    }

    const std::set<std::string_view>& TransportCatalogue::GetBusesForStop(std::string_view stop) const {
        const auto it = stop_and_buses_.find(stop);
        if (it == stop_and_buses_.end()) {
            return EMPTY_BUS_ROUTE_SET;
        }
        return it->second;
    }

    bool TransportCatalogue::SetDistanceBetweenStops(std::string_view stop, std::string_view other_stop, int distance) {
        auto it_stop = stops_index_.find(stop);
        auto it_other = stops_index_.find(other_stop);
        if (it_stop == stops_index_.end() || it_other == stops_index_.end()) {
            return false;
        }
        StopsPointers direct {};
        direct.stop = it_stop->second;
        direct.other = it_other->second;
        stops_distance_index_[direct] = distance;
        StopsPointers reverse {};
        reverse.stop = direct.other;
        reverse.other = direct.stop;
        auto it_rev = stops_distance_index_.find(reverse);
        if (it_rev == stops_distance_index_.end()) {
            stops_distance_index_[reverse] = distance;
        }
        return true;
    }

    int TransportCatalogue::GetDistanceBetweenStops(std::string_view stop, std::string_view other_stop) const {
        const auto it_stop = stops_index_.find(stop);
        const auto it_other = stops_index_.find(other_stop);
        if (it_stop == stops_index_.end() || it_other == stops_index_.end()) { return -1; }
        StopsPointers direct {};
        direct.stop = it_stop->second;
        direct.other = it_other->second;
        auto it_dist = stops_distance_index_.find(direct);
        if (it_dist == stops_distance_index_.end()) { return -1; }
        return it_dist->second;
    }

    std::map<std::string_view, const BusRoute*> TransportCatalogue::GetAllRoutesIndex() const {
        std::map<std::string_view, const BusRoute*>  result(routes_index_.begin(), routes_index_.end());
        return result;
    }

    std::map<std::string_view, const Stop *> TransportCatalogue::GetAllStopsIndex() const {
        std::map<std::string_view, const Stop*> result(stops_index_.begin(), stops_index_.end());
        return result;
    }

    const std::unordered_map<std::string_view, const Stop *>& TransportCatalogue::RawStopsIndex() const {
        return stops_index_;
    }

    void TransportCatalogue::SaveToSerializeTransportCat(tc_serialize::TransportCatalogue& t_cat) const {
        tc_serialize::StopsList st_list;
        for (const auto [_, stop_ptr] : stop_id_name_index_ ) {
            *st_list.add_all_stops() = std::move(SerializeStop(*stop_ptr));
        }
        *(t_cat.mutable_base_settings()->mutable_stops_list()) = std::move(st_list);
        tc_serialize::StopDistanceIndex stop_distances;
        for (const auto [stop_ptrs, distance] : stops_distance_index_) {
            *stop_distances.add_all_stops_distance_index() = std::move(
                    SerializeDistance(stop_ptrs.stop->id, stop_ptrs.other->id, distance));
        }
        *(t_cat.mutable_base_settings()->mutable_stop_dist_index()) = std::move(stop_distances);
        tc_serialize::AllRoutesList routes_list;
        for (const BusRoute& route : bus_routes_) {
            tc_serialize::BusRoute br_out;
            br_out.set_bus_name(route.bus_name);
            int8_t route_type;
            if (route.type == RouteType::CIRCLE_ROUTE) { route_type = 1; }
            else if (route.type == RouteType::RETURN_ROUTE) { route_type = 2; }
            else { route_type = 0; }
            br_out.set_route_type(route_type);
            for (auto stop_ptr : route.route_stops) {
                br_out.add_stop_ids(stop_ptr->id);
            }
            *routes_list.add_routes_list() = std::move(br_out);
        }
        *(t_cat.mutable_base_settings()->mutable_all_routes_list()) = std::move(routes_list);
    }


    bool TransportCatalogue::RestoreFromSerializedTransportCat(tc_serialize::TransportCatalogue& t_cat) {
        tc_serialize::StopsList st_list = t_cat.base_settings().stops_list();
        for (int i = 0; i < st_list.all_stops_size(); ++i) {
            AddStop(DeserializeStop(st_list.all_stops(i)));
        }
        tc_serialize::StopDistanceIndex stops_distances = t_cat.base_settings().stop_dist_index();
        for (int i = 0; i < stops_distances.all_stops_distance_index_size(); ++i) {
            const auto& dist = stops_distances.all_stops_distance_index(i);
            std::string_view from = GetStopNameById(dist.from_id());
            std::string_view to = GetStopNameById(dist.to_id());
            SetDistanceBetweenStops(from, to, dist.distance());
        }
        tc_serialize::AllRoutesList all_routes = t_cat.base_settings().all_routes_list();
        for (int i = 0; i < all_routes.routes_list_size(); ++i) {
            const tc_serialize::BusRoute& route_in = all_routes.routes_list(i);
            BusRoute bus_out;
            bus_out.bus_name = route_in.bus_name();
            if (route_in.route_type() == 1) {
                bus_out.type = RouteType::CIRCLE_ROUTE;
            } else if (route_in.route_type() == 2) {
                bus_out.type = RouteType::RETURN_ROUTE;
            } else {
                bus_out.type = RouteType::NOT_SET;
            }
            bus_out.route_stops.reserve(route_in.stop_ids_size());
            for (int j = 0; j < route_in.stop_ids_size(); ++j) {
                const std::string_view stop_name = GetStopNameById(route_in.stop_ids(j));
                const auto [result, stop_const_ref] = FindStop(stop_name);
                if (!result) { return false; }
                bus_out.route_stops.emplace_back(&stop_const_ref);
            }
            AddBus(bus_out);
        } return true;
    }

    uint32_t TransportCatalogue::GetStopId(const std::string_view stop_name) const {
        auto it = stops_index_.find(stop_name);
        if (it == stops_index_.end()) {
            return 0;
        } return it->second->id;
    }

    std::string_view TransportCatalogue::GetStopNameById(uint32_t stop_id) const {
        auto it = stop_id_name_index_.find(stop_id);
        if (it == stop_id_name_index_.end()) {
            return {};
        } return {it->second->stop_name};
    }
} // namespace transport_catalogue
