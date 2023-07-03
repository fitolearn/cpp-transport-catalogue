#include <utility>
#include "domain.h"
namespace domain {

    Stop::Stop(std::string name, double lat, double lng)
            : name(std::make_shared<std::string>(std::move(name)))
            , latitude(lat)
            , longitude(lng)
    {}

    Bus::Bus(std::string name, std::vector<StopPtr> route, int unique, int actual, double geo, StopPtr last_stop)
            : name(std::make_shared<std::string>(std::move(name)))
            , route(std::vector<StopPtr>(std::move(route)))
            , unique_stops(unique)
            , route_actual_length(actual)
            , route_geographic_length(geo)
            , last_stop_name(std::move(last_stop))
    {}

    double Stop::GetDistanceTo(const StopPtr& stop_to) const {
        return geo::ComputeDistance(
                { latitude, longitude },
                { stop_to.get()->latitude, stop_to.get()->longitude }
        );
    }

    /*double ComputeDistance(const Stop* lhs, const Stop* rhs) {
        return geo::ComputeDistance(lhs->coordinates_, rhs->coordinates_);
    }*/
}
