#pragma once

#include <iostream>
#include <tuple>
#include <string_view>
#include <vector>
#include <string>
#include <optional>
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace json_reader {

    using ::json::Node;
    using ::json::Document;
    using ::json::Array;
    using ::json::Dict;
    using BusWaitTime = int;
    using BusVelocity = double;

    class JsonReader {
    public:
        explicit JsonReader(request_handler::RequestHandler& r_h);
        void InputStatReader(std::istream& is, std::ostream& os);
    private:
        request_handler::RequestHandler& rh_;
        void FillTransportCatalogue(const Dict& dict);
        void FillGraphRouter();
        void FillBus(const Dict& bus);
        void AnswerStatRequests(const json::Dict& dict, std::ostream& out) const;
        const Dict& FillStop(const Dict& stop_req);
        static std::tuple<BusWaitTime, BusVelocity> ReadRoutingSettings(const json::Dict& dict);
        static map_renderer::RenderingSettings ReadRenderingSettings(const json::Dict& dict);
        [[nodiscard]] static double GetDoubleFromNode(const json::Node& node) ;
        [[nodiscard]] static std::vector<svg::Color> GetColorsFromArray(const json::Array& arr) ;
        [[nodiscard]] static svg::Color GetColor(const json::Node& node) ;
        [[nodiscard]] static Node OutStopStat(std::optional<domain::StopStat> stop_stat, int id) ;
        [[nodiscard]] static Node OutBusStat(std::optional<domain::BusStat> bus_stat, int id) ;
        [[nodiscard]] Node OutRouteReq(std::string_view from, std::string_view to, int id) const;
        [[nodiscard]] Node OutMapReq(int id) const;
        [[nodiscard]] std::tuple<std::vector<std::string_view>, int, domain::StopPtr> WordsToRoute(const json::Array& words, bool circular) const;
    };
}
