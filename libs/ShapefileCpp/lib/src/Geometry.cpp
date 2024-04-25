#include "Geometry.hpp"

namespace shp {

    std::ostream& operator << (std::ostream& os, const Geometry& geometry) {
        os << geometry.wkt();
        return os;
    }

}