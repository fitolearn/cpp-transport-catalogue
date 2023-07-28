#include "json_reader.h"
#include "json_builder.h"

using namespace std::literals;
using namespace transport;

namespace json_reader {

    size_t JsonReader::ReadJson(std::istream &input) {
        size_t result = 0;
        try {
            json::Document doc = json::Load(input);
            if (doc.GetRoot().IsMap()) {
                result = doc.GetRoot().AsMap().size();
                root_.emplace_back(std::move(doc));
            }
        } catch (const json::ParsingError &e) {
            std::cerr << e.what() << std::endl;
        }
        return result;
    }

    size_t JsonReader::InputStatReader(std::istream &input) {
        size_t num = ReadJson(input);
        if (num == 0) { return num;}
        size_t result = ParseJsonToRawData();
        FillTransportCatalogue();
        routing_settings_ = GetRoutingSettings();
        graph_ptr_ = std::make_unique<TransportCatalogueRouterGraph>(transport_catalogue_, routing_settings_.value());
        return result;
    }

    size_t JsonReader::ParseJsonToRawData() {
        size_t result = 0;
        const json::Node &root_node = root_.back().GetRoot();
        if (!root_node.IsMap()) { throw json::ParsingError("Error reading JSON data.");}
        const json::Dict &dict = root_node.AsMap();
        auto iter = dict.find("base_requests");
        if (iter == dict.end() || !(iter->second.IsArray())) { throw json::ParsingError("Error reading JSON data.");}
        const json::Array &nodes = iter->second.AsArray();
        for (const auto &node: nodes) {
            BaseRequest data = std::move(ParseDataNode(node));
            if (auto *stop = std::get_if<StopWithDistances>(&data)) {
                raw_stops_.emplace_back(std::move(*stop));
            } else if (auto *bus = std::get_if<BusRouteJson>(&data)) {
                raw_buses_.emplace_back(std::move(*bus));
            } else { throw json::ParsingError("Error reading JSON data.");}
            ++result;
        }
        return result;
    }

    BaseRequest JsonReader::ParseDataStop(const json::Dict &dict) {
        using namespace transport;
        StopWithDistances stop;
        stop.id = 0;
        if (const auto name_i = dict.find("name"s); name_i != dict.end() && name_i->second.IsString()) {
            stop.stop_name = name_i->second.AsString();
        } else { return {};}
        if (const auto coords = ParseCoordinates(dict); coords) {
            stop.coordinates = coords.value();
        } else { return {};}
        const auto dist_i = dict.find("road_distances"s);
        if (dist_i != dict.end() && !(dist_i->second.IsMap())) { return {}; }
        for (const auto &[other_name, other_dist]: dist_i->second.AsMap()) {
            if (!other_dist.IsInt()) return {};
            stop.distances.emplace_back(StopDistanceData{other_name, static_cast<size_t>(other_dist.AsInt())});
        }
        return {stop};
    }

    BaseRequest JsonReader::ParseDataBus(const json::Dict &dict) {
        using namespace transport;
        BusRouteJson route;
        if (const auto name_i = dict.find("name"s); name_i != dict.end() && name_i->second.IsString()) {
            route.bus_name = name_i->second.AsString();
        } else { return {}; }
        if (const auto route_i = dict.find("is_roundtrip"s); route_i != dict.end() && route_i->second.IsBool()) {
            route.type = route_i->second.AsBool() ? RouteType::CIRCLE_ROUTE : RouteType::RETURN_ROUTE;
        } else { return {}; }
        const auto stops_i = dict.find("stops"s);
        if (stops_i != dict.end() && !(stops_i->second.IsArray())) return {};
        for (const auto &stop_name: stops_i->second.AsArray()) {
            if (!stop_name.IsString()) {return {};}
            route.route_stops.emplace_back(stop_name.AsString());
        }
        return {route};
    }

    BaseRequest JsonReader::ParseDataNode(const json::Node &node) {
        using namespace transport;
        if (!node.IsMap()) { return {};}
        const json::Dict &dict = node.AsMap();
        const auto it_type = dict.find("type"s);
        if (it_type == dict.end()) {return {};}
        if (it_type->second == json::Node{"Stop"s}) {
            return ParseDataStop(dict);
        } else if (it_type->second == json::Node{"Bus"s}) {
            return ParseDataBus(dict);
        } else { return {};}
    }

    std::optional<geo::Coordinates> JsonReader::ParseCoordinates(const json::Dict &dict) {
        geo::Coordinates result{};
        if (const auto lat_i = dict.find("latitude"s); lat_i != dict.end() && lat_i->second.IsDouble()) {
            result.latitude = lat_i->second.AsDouble();
        } else { return {}; }
        if (const auto lng_i = dict.find("longitude"s); lng_i != dict.end() && lng_i->second.IsDouble()) {
            result.longitude = lng_i->second.AsDouble();
        } else { return {}; }
        return {result};
    }

    bool JsonReader::FillTransportCatalogue() {
        for (const auto &stop: raw_stops_) {
            transport_catalogue_.AddStop(stop);
        }
        for (const auto &stop: raw_stops_) {
            for (const auto &[other, distance]: stop.distances) {
                bool valid = transport_catalogue_.SetDistanceBetweenStops(stop.stop_name, other, distance);
                if (!valid) { return false; }
            }
        }
        for (auto &route: raw_buses_) {
            if (route.route_stops.size() < 2) { return false; }
            transport::BusRoute br;
            br.bus_name = route.bus_name;
            br.type = route.type;
            for (auto &route_stop: route.route_stops) {
                br.route_stops.emplace_back(&(transport_catalogue_.FindStop(route_stop).second));
            }
            transport_catalogue_.AddBus(br);
        }
        return true;
    }

    size_t JsonReader::OutputStatReader(std::ostream &out) {
        const auto &root_node = root_.back().GetRoot();
        if (!root_node.IsMap()) {
            throw json::ParsingError("Node is not a dictionary.");
        }
        const json::Dict &dict = root_node.AsMap();
        auto iter = dict.find("stat_requests");
        if (iter == dict.end() || !(iter->second.IsArray())) {
            throw json::ParsingError("Parsing error.");
        }
        json::Builder builder;
        builder.StartArray();
        for (const json::Node &node: iter->second.AsArray()) {
            if (!node.IsMap()) {
                throw json::ParsingError("Node is not a dictionary.");
            }
            builder.Value(ProcessOneUserRequestNode(node));
        }
        json::Node res_node = builder.EndArray().Build();
        svg::RenderContext context(out, 4, 0);
        context.RenderIndent();
        json::PrintNode(res_node, context);
        return res_node.AsArray().size();
    }

    json::Node JsonReader::ProcessOneUserRequestNode(const json::Node &user_request) {
        if (!user_request.IsMap()) { ThrowParsError();}
        const json::Dict &request_fields = user_request.AsMap();
        int id = -1;
        if (const auto id_i = request_fields.find("id"s); id_i != request_fields.end() && id_i->second.IsInt()) {
            id = id_i->second.AsInt();
        } else { ThrowParsError() ;}
        const auto type_i = request_fields.find("type"s);
        if (type_i == request_fields.end() || !(type_i->second.IsString())) { ThrowParsError();}
        std::string type = type_i->second.AsString();
        if (type == "Map"s) {
            return GenerateMapNode(id);
        }
        if (type == "Route"s) {
            std::string from_stop, to_stop;
            if (const auto from_it = request_fields.find("from"s); from_it != request_fields.end() && from_it->second.IsString()) {
                from_stop = from_it->second.AsString();
            } else { ThrowParsError();}
            if (const auto to_it = request_fields.find("to"s); to_it != request_fields.end() && to_it->second.IsString()) {
                to_stop = to_it->second.AsString();
            } else { ThrowParsError();}
            return GenerateRouteNode(id, from_stop, to_stop);
        }
        std::string name;
        if (const auto name_i = request_fields.find("name"s); name_i != request_fields.end() && name_i->second.IsString()) {
            name = name_i->second.AsString();
        } else { ThrowParsError();}
        if (type == "Bus"s) {
            return GenerateBusNode(id, name);
        }
        if (type == "Stop"s) {
            return GenerateStopNode(id, name);
        }
        ThrowParsError();
        return {};
    }

    json::Node JsonReader::GenerateMapNode(int id) const {
        RendererSettings rs = GetRendererSetting();
        MapRenderer mr(rs);
        std::ostringstream stream;
        mr.RenderSvgMap(transport_catalogue_, stream);
        return json::Builder().StartDict().Key("request_id"s).Value(id).Key("map"s).Value(
                std::move(stream.str())).EndDict().Build();
    }

    json::Node JsonReader::GenerateBusNode(int id, std::string &name) const {
        BusInfo bi = transport_catalogue_.GetBusInfo(name);
        if (bi.route_type == RouteType::NOT_SET) {return GetErrorNode(id);}
        return json::Builder().StartDict().Key("request_id"s).Value(id).Key("curvature"s).Value(bi.curvature)
                .Key("route_length"s).Value(static_cast<double>(bi.route_length)).Key("stop_count"s).Value(static_cast<int>(bi.stops_number))
                .Key("unique_stop_count"s).Value(static_cast<int>(bi.unique_stops_counter)).EndDict().Build();
    }

    json::Node JsonReader::GenerateStopNode(int id, std::string &name) const {
        if (!transport_catalogue_.FindStop(name).first) { return GetErrorNode(id);}
        json::Dict result;
        json::Array buses;
        json::Builder builder;
        builder.StartDict().Key("buses"s).StartArray();
        const std::set<std::string_view> &bus_routes = transport_catalogue_.GetBusesForStop(name);
        for (auto bus_route: bus_routes) {
            builder.Value(std::move(std::string{bus_route}));
        }
        builder.EndArray();
        builder.Key("request_id"s).Value(id).EndDict();
        return builder.Build();
    }

    void JsonReader::ThrowParsError() {
        throw json::ParsingError("Error while reading JSON data.");
    }

    json::Dict JsonReader::GetDictForRenderSettings() const {
        const auto &root_node = root_.back().GetRoot();
        if (!root_node.IsMap()) { ThrowParsError();}
        const json::Dict &dict = root_node.AsMap();
        auto iter = dict.find("render_settings");
        if (iter == dict.end() || !(iter->second.IsMap())) { ThrowParsError();}
        json::Dict render_settings = iter->second.AsMap();
        return render_settings;
    }

    double JsonReader::CheckSettingParam(json::Dict& render_settings, std::string&& str) {
        if (const auto it_param = render_settings.find(str); it_param != render_settings.end() && it_param->second.IsDouble()) {
            return it_param->second.AsDouble();
        } else { ThrowParsError();}
        return {};
    }

    RendererSettings JsonReader::GetRendererSetting() const {
        if (renderer_settings_.has_value()) {
            return renderer_settings_.value();
        }
        RendererSettings settings;
        json::Dict render_settings = GetDictForRenderSettings();
        settings.width = CheckSettingParam(render_settings, "width"s);
        settings.height = CheckSettingParam(render_settings, "height"s);
        settings.padding = CheckSettingParam(render_settings, "padding"s);
        settings.line_width = CheckSettingParam(render_settings, "line_width"s);
        settings.stop_radius = CheckSettingParam(render_settings, "stop_radius"s);
        settings.bus_label_font_size = CheckSettingParam(render_settings, "bus_label_font_size"s);
        if (const auto field_iter = render_settings.find("bus_label_offset"s); field_iter != render_settings.end() &&
                                                                               field_iter->second.IsArray()) {
            json::Array arr = field_iter->second.AsArray();
            if (arr.size() != 2) { ThrowParsError();}
            settings.bus_label_offset.x = arr[0].AsDouble();
            settings.bus_label_offset.y = arr[1].AsDouble();
        } else { ThrowParsError();}
        settings.stop_label_font_size = CheckSettingParam(render_settings, "stop_label_font_size"s);
        if (const auto field_iter = render_settings.find("stop_label_offset"s); field_iter != render_settings.end() &&
                                                                                field_iter->second.IsArray()) {
            json::Array arr = field_iter->second.AsArray();
            if (arr.size() != 2) { ThrowParsError();}
            settings.stop_label_offset.x = arr[0].AsDouble();
            settings.stop_label_offset.y = arr[1].AsDouble();
        } else { ThrowParsError();}
        if (const auto field_iter = render_settings.find("underlayer_color"s); field_iter != render_settings.end()) {
            svg::Color color = GetColor(field_iter->second);
            if (std::holds_alternative<std::monostate>(color)) { ThrowParsError();}
            settings.underlayer_color = color;
        } else { ThrowParsError();}
        settings.underlayer_width = CheckSettingParam(render_settings, "underlayer_width"s);
        if (const auto field_iter = render_settings.find("color_palette"s); field_iter != render_settings.end() &&
                                                                            field_iter->second.IsArray()) {
            json::Array arr = field_iter->second.AsArray();
            for (const auto &color_node: arr) {
                svg::Color color = GetColor(color_node);
                if (std::holds_alternative<std::monostate>(color)) { ThrowParsError();}
                settings.color_palette.emplace_back(color);
            }
        } else { ThrowParsError();}
        renderer_settings_.emplace(settings);
        return settings;
    }

    RoutingSettings JsonReader::GetRoutingSettings() const {
        if (routing_settings_.has_value()) {
            return routing_settings_.value();
        }
        const auto &root_node = root_.back().GetRoot();
        if (!root_node.IsMap()) { ThrowParsError();}
        const json::Dict &root_dict = root_node.AsMap();
        auto it = root_dict.find("routing_settings");
        if (it == root_dict.end() || !(it->second.IsMap())) { return {};}
        RoutingSettings settings{};
        const json::Dict &routing_settings = it->second.AsMap();
        if (const auto &bus_wait = routing_settings.find("bus_wait_time"); bus_wait != routing_settings.end() &&
            bus_wait->second.IsInt()) {settings.bus_wait_time = bus_wait->second.AsInt();
        } else { ThrowParsError();}
        if (const auto &bus_velocity = routing_settings.find("bus_velocity"); bus_velocity != routing_settings.end() && bus_velocity->second.IsDouble()) {
            settings.bus_velocity = bus_velocity->second.AsDouble();
        } else { ThrowParsError();}
        routing_settings_.emplace(settings);
        return settings;
    }

    json::Node JsonReader::GenerateRouteNode(int id, std::string_view from, std::string_view to) const {
        const auto &[found_from, from_stop] = transport_catalogue_.FindStop(from);
        const auto &[found_to, to_stop] = transport_catalogue_.FindStop(to);
        if (!found_from || !found_to) { ThrowParsError();}
        auto route = graph_ptr_->BuildRoute(from, to);
        if (!route) { return GetErrorNode(id); }
        json::Builder builder;
        builder.StartDict().Key("request_id"s).Value(id).Key("total_time"s).Value(route->weight).Key("items"s).StartArray();
        double waiting_time = graph_ptr_->GetBusWaitingTime();
        for (const auto &edge_id: route->edges) {
            const graph::Edge<double> &edge = graph_ptr_->GetEdge(edge_id);
            auto link = graph_ptr_->GetLinkById(edge_id);
            const auto &stop_from = graph_ptr_->GetStopById(edge.from);
            json::Builder wait_builder;
            wait_builder.StartDict().Key("type"s).Value("Wait"s)
                    .Key("stop_name"s).Value(std::string{stop_from.stop_name})
                    .Key("time"s).Value(waiting_time).EndDict();
            builder.Value(wait_builder.Build());
            json::Builder bus_builder;
            double time = edge.weight - waiting_time;
            bus_builder.StartDict().Key("type"s).Value("Bus"s).Key("bus"s).Value(std::string{link.bus_name})
                    .Key("span_count"s).Value(static_cast<int>(link.number_of_stops)).Key("time"s).Value(time).EndDict();
            builder.Value(bus_builder.Build());
        }
        builder.EndArray().EndDict();
        return builder.Build();
    }

    SerializationSettings JsonReader::GetSerializationSettings() const {
        const auto &root_node = root_.back().GetRoot();
        if (!root_node.IsMap()) { throw json::ParsingError("Error while reading serialization settings.");}
        const json::Dict &root_dict = root_node.AsMap();
        auto it = root_dict.find("serialization_settings");
        if (it == root_dict.end() || !(it->second.IsMap())) {throw json::ParsingError("Error while reading serialization settings.");}
        SerializationSettings result;
        const json::Dict &serialization_settings = it->second.AsMap();
        if (const auto &file_name = serialization_settings.find("file"); file_name != serialization_settings.end() && file_name->second.IsString()) {
            result.file_name = file_name->second.AsString();
        } else { throw json::ParsingError("Error while parsing serialization settings."); }
        return std::move(result);
    }

    void JsonReader::SaveToReader(tc_serialize::TransportCatalogue &t_cat) const {
        *t_cat.mutable_render_settings() = std::move(SerializeRendererSettings(GetRendererSetting()));
        *(t_cat.mutable_router_settings()->mutable_routing_settings()) = std::move(SerializeRouting(GetRoutingSettings()));
        graph_ptr_->SaveTo(t_cat);
    }

    bool JsonReader::RestoreFrom(tc_serialize::TransportCatalogue &t_cat) {
        renderer_settings_.emplace(DeserializeRenderSetting(t_cat.render_settings()));
        routing_settings_.emplace(DeserializeRouting(t_cat.router_settings().routing_settings()));
        graph_ptr_ = std::make_unique<TransportCatalogueRouterGraph>(transport_catalogue_, routing_settings_.value(), t_cat);
        return true;
    }

    svg::Color JsonReader::GetColor(const Node& node) {
        if (node.IsString()) {
            return node.AsString();
        } else if (node.IsArray()) {
            const Array& arr = node.AsArray();
            if (arr.size() == 3) {
                return svg::Rgb {static_cast<uint8_t>(arr[0].AsInt()),
                                 static_cast<uint8_t>(arr[1].AsInt()),
                                 static_cast<uint8_t>(arr[2].AsInt())};
            } else {
                return svg::Rgba {static_cast<uint8_t>(arr[0].AsInt()),
                                  static_cast<uint8_t>(arr[1].AsInt()),
                                  static_cast<uint8_t>(arr[2].AsInt()),
                                  arr[3].AsDouble()};
            }
        } return {};
    }

    inline json::Node JsonReader::GetErrorNode(int id) {
        return json::Builder().StartDict().Key("request_id"s).Value(id)
                .Key("error_message"s).Value("not found"s).EndDict()
                .Build();
    }
} // namespace json_reader