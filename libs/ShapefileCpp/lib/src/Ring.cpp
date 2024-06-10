#include "Ring.hpp"

namespace shp {

    Ring:: Ring(std::vector<Point> pts) : points(pts) {}

    std::unique_ptr<Geometry> Ring::clone() const  {
        return std::make_unique<Ring>(points);
    }

    std::string Ring::wkt() const {
        std::stringstream str;
        str << "LINEARRING ";
        if (std::size(points) == 0) {
            str <<  "EMPTY";
        } else {
            str <<  "(";
            bool firstTime = true;
            for(auto p : points) {
                if (firstTime != true) {
                    str << ", ";
                }
                firstTime = false;
                str << p.getX() << " " << p.getY();
            }
            str <<  ")";
        }
        return str.str();
    }

    std::vector<Point> Ring::getPoints() const {
        return points;
    }

}