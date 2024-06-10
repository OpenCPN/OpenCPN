#include "MultiPoint.hpp"

namespace shp {

    MultiPoint::MultiPoint(std::vector<Point> pts) : points(pts) { }

    std::unique_ptr<Geometry> MultiPoint::clone() const  {
        return std::make_unique<MultiPoint>(points);
    }

    std::string MultiPoint::wkt() const {
        std::stringstream str;
        str << "MULTIPOINT ";
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

    std::vector<Point> MultiPoint::getPoints() const {
        return points;
    }

}