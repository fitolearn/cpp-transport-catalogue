#include <utility>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <sstream>
#include <cassert>
#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"

using namespace domain;
using namespace std::literals;
using namespace std;

namespace json_reader {

    JsonReader::JsonReader(request_handler::RequestHandler& req_handler)
            : rh_(req_handler)
    {}

    void JsonReader::InputStatReader(std::istream& is, std::ostream& os) {
        const json::Document doc = json::Load(is);
        const Node& node = doc.GetRoot();
        const Dict& dict = node.AsMap();
        if (dict.count("routing_settings"s)) {
            const auto [wait, vel] = ReadRoutingSettings(dict.at("routing_settings"s).AsMap());
            rh_.SetRoutingSettings(wait, vel);
        }
        if (dict.count("base_requests"s)) {
            FillTransportCatalogue(dict);
            FillGraphRouter();
        }
        if (dict.count("render_settings"s)) {
            rh_.SetRenderSettings(std::move(ReadRenderingSettings(dict)));
        }
        if (dict.count("stat_requests"s)) {
            AnswerStatRequests(dict, os);
        }
    }

    void JsonReader::FillTransportCatalogue(const json::Dict& dict) {
        const Array& base_requests = dict.at("base_requests"s).AsArray();
        vector<pair<const string&, const Dict&>> stops_road_distances;

        for (const auto& req_node : base_requests) {
            const Dict& req = req_node.AsMap();
            if (req.at("type"s).AsString() == "Stop"s) {
                stops_road_distances.emplace_back(req.at("name"s).AsString(), FillStop(req));
            }
        }
        for (const auto& [stop_name, dict_] : stops_road_distances) {
            for (const auto& [stop_name_to, distance] : dict_) {
                rh_.SetDistanceBetweenTwoStops(stop_name, stop_name_to, distance.AsInt());
            }
        }
        for (const auto& req_node : base_requests) {
            const Dict& req = req_node.AsMap();
            if (req.at("type"s).AsString() == "Bus"s) {
                FillBus(req);
            }
        }
    }

    void JsonReader::FillGraphRouter() {
        for (const StopPtr& stop : rh_.GetStops()) {
            std::string_view stop_name(*stop->name);
            rh_.AddStopToRouter(stop_name);
            rh_.AddWaitEdgeToRouter(stop_name);
        }
        for (const BusPtr& bus : rh_.GetBuses()) {
            const std::string_view bus_name = *bus->name;
            for (size_t i = 0; i < bus->route.size() - 1; ++i) {
                const std::string_view stop_name_from = *bus->route[i]->name;
                int prev_actual = 0;
                std::string_view prev_stop_name = stop_name_from;
                for (size_t j = i + 1; j < bus->route.size(); ++j) {
                    const string_view stop_name_to = *bus->route[j]->name;
                    int actual = *rh_.GetDistanceBetweenTwoStops(prev_stop_name, stop_name_to);
                    rh_.AddBusEdgeToRouter(
                            stop_name_from,
                            stop_name_to,
                            bus_name,
                            j - i,
                            prev_actual + actual
                    );
                    prev_stop_name = stop_name_to;
                    prev_actual += actual;
                }
            }
        } rh_.BuildRouter();
    }

    const Dict& JsonReader::FillStop(const json::Dict& stop_req) {
        const auto& node_latitude = stop_req.at("latitude"s);
        double latitude = node_latitude.IsPureDouble() ? node_latitude.AsDouble() : node_latitude.AsInt();
        const auto& node_longitude = stop_req.at("longitude"s);
        double longitude = node_longitude.IsPureDouble() ? node_longitude.AsDouble() : node_longitude.AsInt();
        Stop stop(std::move(std::string(stop_req.at("name"s).AsString())), latitude, longitude);
        rh_.AddStop(std::move(stop));
        return stop_req.at("road_distances"s).AsMap();
    }

    void JsonReader::FillBus(const json::Dict& bus_req) {
        auto [route, unique_stops_num, last_stop] = WordsToRoute(bus_req.at("stops"s).AsArray(), bus_req.at("is_roundtrip"s).AsBool());
        const auto [geographic, actual] = rh_.ComputeRouteLengths(route);
        if (last_stop.get() == rh_.SearchStopByName(route.front()).get()) {
            Bus bus(std::move(std::string(bus_req.at("name"s).AsString())), rh_.StopsToStopPtr(route), unique_stops_num, actual, geographic);
            rh_.AddBus(std::move(bus));
        } else {
            Bus bus(std::move(std::string(bus_req.at("name"s).AsString())), rh_.StopsToStopPtr(route), unique_stops_num, actual, geographic, last_stop);
            rh_.AddBus(std::move(bus));
        }
    }

    std::tuple<json_reader::BusWaitTime, json_reader::BusVelocity> JsonReader::ReadRoutingSettings(const json::Dict& dict) {
        const int wait_time = dict.at("bus_wait_time"s).AsInt();
        const double velocity = GetDoubleFromNode(dict.at("bus_velocity"s));
        return { wait_time, velocity };
    }

    map_renderer::RenderingSettings JsonReader::ReadRenderingSettings(const json::Dict& dict) {
        map_renderer::RenderingSettings settings;
        const Dict& dict_deeper = dict.at("render_settings"s).AsMap();
        const Node& node_width = dict_deeper.at("width"s);
        settings.width = GetDoubleFromNode(node_width);
        const Node& node_height = dict_deeper.at("height"s);
        settings.height = GetDoubleFromNode(node_height);
        const Node& node_padding = dict_deeper.at("padding"s);
        settings.padding = GetDoubleFromNode(node_padding);
        const Node& node_stop_radius = dict_deeper.at("stop_radius"s);
        settings.stop_radius = GetDoubleFromNode(node_stop_radius);
        const Node& node_line_width = dict_deeper.at("line_width"s);
        settings.line_width = GetDoubleFromNode(node_line_width);
        settings.bus_label_font_size = dict_deeper.at("bus_label_font_size"s).AsInt();
        const Array& arr_bus_label_offset = dict_deeper.at("bus_label_offset"s).AsArray();
        settings.bus_label_offset.x = GetDoubleFromNode(arr_bus_label_offset[0]);
        settings.bus_label_offset.y = GetDoubleFromNode(arr_bus_label_offset[1]);
        settings.stop_label_font_size = dict_deeper.at("stop_label_font_size"s).AsInt();
        const Array& arr_stop_label_offset = dict_deeper.at("stop_label_offset"s).AsArray();
        settings.stop_label_offset.x = GetDoubleFromNode(arr_stop_label_offset[0]);
        settings.stop_label_offset.y = GetDoubleFromNode(arr_stop_label_offset[1]);
        const Node& arr_underlayer_color = dict_deeper.at("underlayer_color"s);
        settings.underlayer_color = GetColor(arr_underlayer_color);
        const Node& node_underlayer_width = dict_deeper.at("underlayer_width"s);
        settings.underlayer_width = GetDoubleFromNode(node_underlayer_width);
        const Array& node_color_palette = dict_deeper.at("color_palette"s).AsArray();
        settings.color_palette = GetColorsFromArray(node_color_palette);
        return settings;
    }

    double JsonReader::GetDoubleFromNode(const Node& node) {
        return (node.IsPureDouble() ? node.AsDouble() : node.AsInt());
    }

    vector<svg::Color> JsonReader::GetColorsFromArray(const json::Array& arr) {
        vector<svg::Color> result;
        result.reserve(arr.size());
        for (const auto & node : arr) {
            result.emplace_back(std::move(GetColor(node)));
        } return result;
    }

    svg::Color JsonReader::GetColor(const Node& node) {
        if (node.IsString()) {
            return node.AsString();
        } else if (node.IsArray()) {
            const Array& arr = node.AsArray();
            if (arr.size() == 3) {
                return svg::Rgb {
                            static_cast<uint8_t>(arr[0].AsInt()),
                            static_cast<uint8_t>(arr[1].AsInt()),
                            static_cast<uint8_t>(arr[2].AsInt())
                };
            } else {
                return svg::Rgba {
                            static_cast<uint8_t>(arr[0].AsInt()),
                            static_cast<uint8_t>(arr[1].AsInt()),
                            static_cast<uint8_t>(arr[2].AsInt()),
                            GetDoubleFromNode(arr[3])
                     };
            }
        } return {};
    }

    void JsonReader::AnswerStatRequests(const Dict& dict, ostream& out) const {
        Array result;
        const Array& stat_requests = dict.at("stat_requests"s).AsArray();
        for (const auto& req_node : stat_requests) {
            const Dict& req = req_node.AsMap();
            const string& type = req.at("type"s).AsString();
            Node node;
            if (type == "Stop"s) {
                node = OutStopStat(
                        rh_.GetStopStatByName(req.at("name"s).AsString()),
                        req.at("id"s).AsInt()
                );
            } else if (type == "Bus"s) {
                node = OutBusStat(
                        rh_.GetBusStatByName(req.at("name"s).AsString()),
                        req.at("id"s).AsInt()
                );
            } else if (type == "Route"s) {
                node = OutRouteReq(
                        req.at("from"s).AsString(),
                        req.at("to"s).AsString(),
                        req.at("id"s).AsInt()
                );
            } else {
                node = OutMapReq(req.at("id"s).AsInt());
            } result.emplace_back(std::move(node));
        } json::Print(json::Document(Node(result)), out);
    }

    Node JsonReader::OutStopStat(const optional<StopStat> stop_stat, int id) {
        if (stop_stat.has_value()) {
            Array arr;
            if (stop_stat->passing_buses == nullptr) {
                Dict dict = {{"buses"s, Node(std::move(arr))},{"request_id"s, Node(id)}};
                return Node(std::move(dict));
            }
            const auto buses = *stop_stat->passing_buses;
            arr.reserve(buses.size());
            vector<BusPtr> tmp(buses.begin(), buses.end());
            sort(tmp.begin(), tmp.end(), [](const BusPtr& lhs, const BusPtr& rhs) { return lexicographical_compare(
                 lhs->name->begin(), lhs->name->end(),
                 rhs->name->begin(), rhs->name->end()
                 );
            });
            for (const BusPtr& bus : std::move(tmp)) {
                arr.push_back(json::Node(*bus->name));
            }
            Dict dict = {{"buses"s, Node(std::move(arr))}, {"request_id"s, Node(id)}};
            return Node(std::move(dict));
        } else {
            Dict dict = {{"request_id"s,Node(id)}, {"error_message"s,Node(std::move("not found"s))}};
            return Node(std::move(dict));
        }
    }

    Node JsonReader::OutBusStat(const optional<BusStat> bus_stat, int id) {
        Dict dict;
        if (bus_stat.has_value()) {
            dict = {{"request_id"s, Node(id)}, {"curvature"s, Node(bus_stat->curvature)},
                 {"unique_stop_count"s, Node(bus_stat->unique_stops)},{"stop_count"s, Node(bus_stat->stops_on_route)},
                 {"route_length"s, Node(bus_stat->routh_actual_length)}
            };
        } else {
            dict = {{"request_id"s, Node(id)},{"error_message"s, Node(std::move("not found"s))}};
        }
        return Node(std::move(dict));
    }

    Node JsonReader::OutRouteReq(const string_view from, const string_view to, int id) const {
        const auto route_info = rh_.GetRouteInfo(from, to);
        if (route_info) {
            Array arr;
            arr.reserve(route_info->items.size());
            for (const auto item : route_info->items) {
                if (item.wait_item) {
                    string stop_name(item.wait_item->stop_name);
                    Dict dict = {{"type"s, Node(std::move("Wait"s))},{"stop_name"s, Node(std::move(stop_name))},
                                 {"time"s, Node(item.wait_item->time)}
                    };
                    arr.emplace_back(std::move(dict));
                } else {
                    string bus_name(item.bus_item->bus_name);
                    Dict dict = {{"type"s, Node(std::move("Bus"s))}, {"bus"s,Node(std::move(bus_name))},
                            {"span_count"s, Node(item.bus_item->span_count)}, {"time"s,Node(item.bus_item->time)}
                    };
                    arr.emplace_back(std::move(dict));
                }
            }
            Dict dict = {{"request_id"s, Node(id)}, {"total_time"s, Node(route_info->total_time)},
                    {"items"s, Node(std::move(arr))}
            };
            return Node(std::move(dict));
        } else {
            Dict dict = {{"request_id"s, Node(id)}, {"error_message"s, Node(std::move("not found"s))}};
            return Node(std::move(dict));
        }
    }

    Node JsonReader::OutMapReq(int id) const {
        ostringstream out;
        svg::Document doc = rh_.RenderMap();
        doc.Render(out);
        Dict dict = {{"request_id"s, Node(id)}, {"map"s, Node(std::move(out.str()))}};
        return Node(std::move(dict));
    }

    tuple<vector<string_view>, int, StopPtr> JsonReader::WordsToRoute(const Array& words, bool circular) const {
        vector<string_view> result;
        unordered_set<std::string_view, hash<string_view>> stops_unique_names;
        result.reserve(words.size());

        for (const auto & word : words) {
            StopPtr stop = rh_.SearchStopByName(word.AsString());
            result.emplace_back(*stop->name);
            stops_unique_names.insert(word.AsString());
        }
        const StopPtr last_stop = rh_.SearchStopByName(result.back());
        if (!circular && words.size() > 1) {
            result.reserve(words.size() * 2);
            for (int i = (int)words.size() - 2; i >= 0; --i) {
                StopPtr stop = rh_.SearchStopByName(words[i].AsString());
                result.emplace_back(*stop->name);
            }
        } return { std::move(result), static_cast<int>(stops_unique_names.size()), last_stop };
    }
}
