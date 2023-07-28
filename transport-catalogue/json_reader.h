#pragma once
#include <sstream>
#include <vector>
#include "json.h"
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.h"
#include "domain.h"
#include "router.h"
#include "transport_router.h"
#include "serialization.h"

using namespace std::literals;
namespace json_reader {

    using ::json::Node;
    using ::json::Document;
    using ::json::Array;
    using ::json::Dict;

    struct BusRouteJson {
        std::string bus_name;
        transport::RouteType type;
        std::vector<std::string> route_stops;
    };

    using BaseRequest = std::variant<std::monostate, transport::StopWithDistances, BusRouteJson>;

    class JsonReader {
    public:
        explicit JsonReader(transport::TransportCatalogue &tc) : transport_catalogue_(tc) {}
        size_t ReadJson(std::istream &input);
        size_t InputStatReader(std::istream &input);
        size_t OutputStatReader(std::ostream &output);
        [[nodiscard]] RendererSettings GetRendererSetting() const;
        RoutingSettings GetRoutingSettings() const;
        SerializationSettings GetSerializationSettings() const;
        void SaveToReader(tc_serialize::TransportCatalogue &t_cat) const;
        bool RestoreFrom(tc_serialize::TransportCatalogue &t_cat);
        static inline json::Node GetErrorNode(int id) ;
    private:
        mutable std::optional<RoutingSettings> routing_settings_;
        mutable std::optional<RendererSettings> renderer_settings_;
        transport::TransportCatalogue &transport_catalogue_;
        std::vector<json::Document> root_;
        std::vector<transport::StopWithDistances> raw_stops_;
        std::vector<BusRouteJson> raw_buses_;
        std::unique_ptr<TransportCatalogueRouterGraph> graph_ptr_;
        json::Node ProcessOneUserRequestNode(const json::Node &user_request);
        json::Node GenerateMapNode(int id) const;
        json::Node GenerateBusNode(int id, std::string &name) const;
        json::Node GenerateStopNode(int id, std::string &name) const;
        json::Node GenerateRouteNode(int id, std::string_view from, std::string_view to) const;
        json::Dict GetDictForRenderSettings() const;
        size_t ParseJsonToRawData();
        static BaseRequest ParseDataNode(const json::Node &node);
        bool FillTransportCatalogue();
        static std::optional<geo::Coordinates> ParseCoordinates(const json::Dict &dict);
        static BaseRequest ParseDataStop(const json::Dict &dict);
        static BaseRequest ParseDataBus(const json::Dict &dict);
        static svg::Color GetColor(const Node& node);
        static void ThrowParsError();
        static double CheckSettingParam(json::Dict& settings, std::string&& str);
    };
} // namespace json_reader