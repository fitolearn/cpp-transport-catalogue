#include "geo.h"
 namespace geo {

    double ComputeDistance(Coordinates from, Coordinates to) {
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
}//namespace geo