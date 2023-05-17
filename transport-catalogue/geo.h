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
        static const double RADIANS_PER_DEGREE = 3.1415926535 / 180.;
        static const int EARTH_RADIUS = 6371000;
        return acos(sin(from.latitude * RADIANS_PER_DEGREE) * sin(to.latitude * RADIANS_PER_DEGREE)
                    + cos(from.latitude * RADIANS_PER_DEGREE) * cos(to.latitude * RADIANS_PER_DEGREE) * cos(abs(from.longitude - to.longitude) * RADIANS_PER_DEGREE))
               * EARTH_RADIUS;
    }

} // namespace geo
