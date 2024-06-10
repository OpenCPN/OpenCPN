#pragma once

#include <iostream>
#include <memory>
#include "Polygon.hpp"

namespace shp {

    class Bounds {

        private:

            double minX;
            double minY;
            double maxX;
            double maxY;

        public:

            Bounds(double min_x, double min_y, double max_x, double max_y);

            double getMinX() const;

            double getMinY() const;

            double getMaxX() const;

            double getMaxY() const;

            std::unique_ptr<Polygon> toPolygon() const;

    };

    std::ostream& operator << (std::ostream& os, const Bounds& bounds);

}