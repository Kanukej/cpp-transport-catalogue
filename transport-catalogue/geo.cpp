#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

const int EARTH_RAD = 6371000;

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    const double dr = M_PI / 180.0;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * EARTH_RAD;
}

}  // namespace geo
