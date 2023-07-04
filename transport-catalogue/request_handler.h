#pragma once
#include <optional>
#include <unordered_set>
#include <tuple>
#include <string>
#include <string_view>
#include <memory>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

namespace request_handler {

    class RequestHandler {
    private:
        enum class SeparatorType {
            DASH,
            GREATER_THAN
        };
        transport::TransportCatalogue& tc_;
        map_renderer::MapRenderer& mr_;
        transport::Router rt_;
        static std::tuple<std::string, std::size_t> QueryGetName(std::string_view str) ;
        static std::tuple<std::string, std::string> SplitIntoLengthStop(const std::string& str) ;
        static std::tuple<std::vector<std::string>, SeparatorType> SplitIntoWordsBySeparator(std::string_view str) ;
        std::tuple<std::vector<std::string_view>, int> WordsToRoute(const std::vector<std::string>& words, SeparatorType separator) const;

    public:
        RequestHandler(transport::TransportCatalogue& db, map_renderer::MapRenderer& mr);
        void BuildRouter();
        void AddBus(domain::Bus&& bus);
        void AddStop(domain::Stop&& stop);
        void SetDistanceBetweenTwoStops(std::string_view stop_name_one, std::string_view stop_name_two, int distance);
        void AddBus(std::string_view raw_query);
        void AddStop(std::string_view raw_query);
        void SetDistanceBetweenStops(std::string_view raw_query);
        void SetRenderSettings(map_renderer::RenderingSettings settings);
        void SetRoutingSettings(double bus_wait_time, double bus_velocity);
        void AddStopToRouter(std::string_view name);
        void AddWaitEdgeToRouter(std::string_view stop_name);
        void AddBusEdgeToRouter( std::string_view stop_from, std::string_view stop_to, std::string_view bus_name, int span_count, int dist);
        domain::BusPtr SearchBusByName(std::string_view name) const;
        domain::StopPtr SearchStopByName(std::string_view name) const;
        std::vector<domain::BusPtr> GetBuses() const;
        std::vector<domain::StopPtr> GetStops() const;
        std::optional<domain::BusStat> GetBusStatByName(std::string_view bus_name) const;
        std::optional<domain::StopStat> GetStopStatByName(std::string_view stop_name) const;
        const std::unordered_set<domain::BusPtr>* GetBusesByStop(std::string_view stop_name) const;
        std::tuple<double, int> ComputeRouteLengths(const std::vector<std::string_view>& routes) const;
        std::vector<domain::StopPtr> StopsToStopPtr(const std::vector<std::string_view>& stops) const;
        std::optional<int> GetDistanceBetweenTwoStops(std::string_view stop_name_one, std::string_view stop_name_two) const;
        std::optional<transport::RouteInfo> GetRouteInfo(std::string_view from, std::string_view to) const;
        svg::Document RenderMap() const;
    };
}
