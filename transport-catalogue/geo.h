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

    double ComputeDistance(Coordinates from, Coordinates to);

}// namespace geo
