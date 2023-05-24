#pragma once

#include <optional>
#include "transport_catalogue.h"
// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
namespace transport {

    using BusPtr = domain::Bus*;
    struct BusStat {
        size_t route_length;
        double curvature;
        size_t stop_count;
        size_t unique_stop_count;
    };

    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& db);
        std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;
        const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;
        std::optional <std::set<std::string_view>> GetSortedBusesByStop(const std::string_view& stop_name) const;

    private:
        const TransportCatalogue& db_;
    };
}//namespace transport