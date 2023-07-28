#include <utility>
#include "domain.h"
using namespace std::literals;

namespace transport {

    std::ostream& operator<<(std::ostream& os, const BusInfo& bus_info) {
        size_t length = bus_info.route_length;
        os << "Bus "s << bus_info.bus_name << ": "s << bus_info.stops_number << " stops on route, "s
           << bus_info.unique_stops_counter << " unique stops, "s << std::setprecision(6) << length << " route length, "s
           << std::setprecision(6) << bus_info.curvature << " curvature"s << std::endl;
        return os;
    }
} // namespace transport_catalogue
