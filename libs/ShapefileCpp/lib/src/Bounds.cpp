#include "Bounds.hpp"

namespace shp {

    Bounds::Bounds(double min_x, double min_y, double max_x, double max_y) :
                minX(min_x), minY(min_y),  maxX(max_x), maxY(max_y) {}

    double Bounds::getMinX() const {
        return minX;
    }

    double Bounds::getMinY() const {
        return minY;
    }

    double Bounds::getMaxX() const {
        return maxX;
    }

    double Bounds::getMaxY() const {
        return maxY;
    }

    std::unique_ptr<Polygon> Bounds::toPolygon() const {
        shp::Ring ring {{
            {minX,minY}, {maxX, minY}, {maxX,maxY}, {minX, minY}
        }};
        shp::Polygon polygon {{ring}};
        return std::make_unique<Polygon>(polygon);
    }

    std::ostream& operator << (std::ostream& os, const Bounds& b) {
        os << "BOUNDS (" << b.getMinX() << ", " << b.getMinX()  << ", " << b.getMaxX() << ", " << b.getMaxY() << ")";
        return os;
    }

}