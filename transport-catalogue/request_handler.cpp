#include <unordered_set>
#include <vector>
#include <utility>
#include "request_handler.h"
using namespace domain;

namespace request_handler {

    RequestHandler::RequestHandler(transport::TransportCatalogue& db, map_renderer::MapRenderer& mr)
            : tc_(db)
            , mr_(mr)
    {}

    void RequestHandler::AddBus(std::string_view raw_query) {
        auto [words, separator] = SplitIntoWordsBySeparator(raw_query);
        auto [route, unique_stops] = WordsToRoute(words, separator);
        const auto [geographic, actual] = ComputeRouteLengths(route);
        Bus new_bus(
                std::move(words[0]),
                std::move(StopsToStopPtr(route)),
                unique_stops,
                actual,
                geographic
        );
        tc_.AddBus(std::move(new_bus));
    }

    void RequestHandler::AddStop(std::string_view raw_query) {
        auto [words, sep] = SplitIntoWordsBySeparator(raw_query);
        Stop new_stop(std::move(words[0]),std::stod(words[1]),std::stod(words[2]));
        tc_.AddStop(std::move(new_stop));
    }

    void RequestHandler::SetDistanceBetweenStops(std::string_view raw_query) {
        auto [parts, _] = SplitIntoWordsBySeparator(raw_query);
        const auto& stop_X = parts[0];
        for (size_t i = 3; i < parts.size(); ++i) {
            auto [raw_distance, stop_To] = SplitIntoLengthStop(parts[i]);
            int distance = std::stoi(raw_distance.substr(0, raw_distance.size() - 1));
            tc_.SetDistanceBetweenStops(stop_X, stop_To, distance);
        }
    }

    void RequestHandler::AddBus(Bus&& bus) {
        tc_.AddBus(std::move(bus));
    }


    void RequestHandler::AddStop(Stop&& stop) {
        tc_.AddStop(std::move(stop));
    }

    void RequestHandler::SetDistanceBetweenTwoStops(std::string_view first, std::string_view second, int distance) {
        tc_.SetDistanceBetweenStops(first, second, distance);
    }

    BusPtr RequestHandler::SearchBusByName(std::string_view name) const {
        return tc_.SearchBus(name);
    }

    StopPtr RequestHandler::SearchStopByName(std::string_view name) const {
        return tc_.SearchStop(name);
    }

    std::vector<BusPtr> RequestHandler::GetBuses() const {
        return tc_.GetBusesInVector();
    }

    std::vector<StopPtr> RequestHandler::GetStops() const {
        return tc_.GetStopsInVector();
    }

    std::optional<BusStat> RequestHandler::GetBusStatByName(std::string_view bus_name) const {
        BusPtr bus = tc_.SearchBus(bus_name);
        if (bus == nullptr) { return std::nullopt; }
        return std::optional<BusStat>({bus_name,
                                       static_cast<int>(bus.get()->route.size()),
                                       bus->unique_stops,
                                       bus->route_actual_length,
                                       bus->route_actual_length / bus->route_geographic_length
                                      });
    }

    std::optional<StopStat> RequestHandler::GetStopStatByName(std::string_view stop_name) const {
        StopPtr stop = tc_.SearchStop(stop_name);
        if (stop == nullptr) { return std::nullopt; }
        return std::optional<StopStat>({stop_name, GetBusesByStop(stop_name)});
    }

    const std::unordered_set<BusPtr>* RequestHandler::GetBusesByStop(std::string_view stop_name) const {
        StopPtr stop = tc_.SearchStop(stop_name);
        return tc_.GetPassingBusesByStop(stop);
    }

    std::tuple<double, int> RequestHandler::ComputeRouteLengths(const std::vector<std::string_view>& route) const {
        double geographic = 0;
        int actual = 0;
        auto prev_stop = &route[0];
        size_t route_sz = route.size();

        for (size_t i = 1; i < route_sz; ++i) {
            const auto cur_stop = &route[i];
            const auto res_geogr = tc_.GetGeographicDistanceBetweenStops(*prev_stop, *cur_stop);
            geographic += (res_geogr.has_value()) ? *res_geogr : 0;
            const auto res_actual = tc_.GetActualDistanceBetweenStops(*prev_stop, *cur_stop);
            actual += (res_actual.has_value()) ? *res_actual : 0;
            prev_stop = cur_stop;
        }
        std::tuple<double, int> res = std::tuple<double, int>(geographic, actual);
        return res;
    }

    std::vector<StopPtr> RequestHandler::StopsToStopPtr(const std::vector<std::string_view>& stops) const {
        std::vector<StopPtr> res;
        res.reserve(stops.size());
        for (const auto& stop : stops) {
            res.push_back(tc_.SearchStop(stop));
        }
        return res;
    }

    std::optional<int> RequestHandler::GetDistanceBetweenTwoStops(std::string_view stop1_name, std::string_view stop2_name) const {
        return tc_.GetActualDistanceBetweenStops(stop1_name, stop2_name);
    }

    svg::Document RequestHandler::RenderMap() const {
        std::vector<BusPtr> buses = tc_.GetBusesInVector();
        std::vector<std::pair<StopPtr, StopStat>> stops;
        for (const StopPtr& stop : tc_.GetStopsInVector()) {
            stops.emplace_back(std::pair<StopPtr, StopStat>{ stop, *GetStopStatByName(*stop->name) });
        }
        return mr_.MakeDocument(std::move(buses), std::move(stops));
    }

    void RequestHandler::SetRenderSettings(map_renderer::RenderingSettings settings) {
        mr_.SetSettings(std::move(settings));
    }

    void RequestHandler::SetRoutingSettings(double bus_wait_time, double bus_velocity) {
        rt_.SetSettings(bus_wait_time, bus_velocity);
    }

    void RequestHandler::AddStopToRouter(std::string_view name) {
        rt_.AddStop(name);
    }

    void RequestHandler::AddWaitEdgeToRouter(std::string_view stop_name) {
        rt_.AddWaitEdge(stop_name);
    }

    void RequestHandler::AddBusEdgeToRouter(std::string_view stop_from, std::string_view stop_to, std::string_view bus_name, int span_count, int dist) {
        rt_.AddBusEdge(stop_from, stop_to, bus_name, span_count, dist);
    }

    void RequestHandler::BuildRouter() {
        rt_.BuildGraph();
        rt_.BuildRouter();
    }

    std::optional<transport::RouteInfo> RequestHandler::GetRouteInfo(std::string_view from, std::string_view to) const {
        return rt_.GetRouteInfo(from, to);
    }

    std::tuple<std::string, size_t> RequestHandler::QueryGetName(std::string_view str) {
        auto pos = str.find_first_of(' ', 0) + 1;
        auto new_pos = str.find_first_of(':', pos);
        if (new_pos == std::string_view::npos) {
            return std::tuple<std::string, size_t>(str.substr(pos), new_pos);
        }
        auto name = str.substr(pos, new_pos - pos);
        std::tuple<std::string, size_t> res = std::tuple<std::string, size_t>(name, ++new_pos);
        return res;
    }

    std::tuple<std::string, std::string> RequestHandler::SplitIntoLengthStop(const std::string& str) {
        const auto pos = str.find_first_of(' ');
        std::string length = str.substr(0, pos);
        std::string name_stop = str.substr(pos + 4);
        std::tuple<std::string, std::string> res{std::move(length), std::move(name_stop)};
        return res;
    }

    std::tuple<std::vector<std::string>, RequestHandler::SeparatorType> RequestHandler::SplitIntoWordsBySeparator(std::string_view str) {
        std::vector<std::string> words;
        SeparatorType sep_type;
        auto [name, pos_start] = QueryGetName(str);
        words.push_back(std::move(name));
        const size_t str_sz = str.size();
        std::string word;
        for (size_t i = pos_start; i < str_sz; ++i) {
            if (str[0] == 'B' && (str[i] == '>' || str[i] == '-')) {
                sep_type = (str[i] == '>') ? SeparatorType::GREATER_THAN : SeparatorType::DASH;
                words.push_back(std::move(word.substr(1, word.size() - 2)));
                word.clear();
            } else if (str[0] == 'S' && str[i] == ',') {
                words.push_back(std::move(word.substr(1, word.size() - 1)));
                word.clear();
            } else {
                word += str[i];
            }
        }
        words.emplace_back(std::move(word.substr(1, word.size() - 1)));
        std::tuple<std::vector<std::string>, SeparatorType> res = std::tuple(std::move(words), sep_type);
        return res;
    }

    std::tuple<std::vector<std::string_view>, int> RequestHandler::WordsToRoute(const std::vector<std::string>& words, SeparatorType separator) const {
        std::vector<std::string_view> result;
        std::unordered_set<std::string_view, std::hash<std::string_view>> stops_unique_names;
        result.reserve(words.size() - 1);

        for (size_t i = 1; i < words.size(); ++i) {
            StopPtr stop = tc_.SearchStop(words[i]);
            result.emplace_back(*stop->name);
            stops_unique_names.insert(words[i]);
        }
        if (separator == SeparatorType::DASH) {
            result.reserve(words.size() * 2 - 1);
            for (int i = words.size() - 2; i >= 1; --i) {
                StopPtr stop = tc_.SearchStop(words[i]);
                result.emplace_back(*stop->name);
            }
        }
        std::tuple<std::vector<std::string_view>, int> res = std::tuple{std::move(result),(int)stops_unique_names.size()};
        return res;
    }
}