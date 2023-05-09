#pragma once

#include <cmath>

namespace geo {

    struct Coordinates {
        double latitude;
        double longitude;
        bool operator==(const Coordinates& other) const {
            return latitude == other.latitude && longitude == other.longitude;
        }
        bool operator!=(const Coordinates& other) const {
            return !(latitude == other.latitude && longitude == other.longitude);
        }
    };

    inline double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double DIAMETER = 3.1415926535 / 180.;
        static const int EARTH_RADIUS = 6371000;
        return acos(sin(from.latitude * DIAMETER) * sin(to.latitude * DIAMETER)
                    + cos(from.latitude * DIAMETER) * cos(to.latitude * DIAMETER) * cos(abs(from.longitude - to.longitude) * DIAMETER))
               * EARTH_RADIUS;
    }

} // namespace geo
