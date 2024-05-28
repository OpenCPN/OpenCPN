#include "Line.hpp"

namespace shp {

    Line:: Line(std::vector<Point> pts) : points(pts) { }

    std::unique_ptr<Geometry> Line::clone() const  {
        return std::make_unique<Line>(points);
    }

    std::string Line::wkt() const {
        std::stringstream str;
        str << "LINESTRING ";
        if (std::size(points) == 0) {
            str << "EMPTY";
        } else {
            str << "(";
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

    std::vector<Point> Line::getPoints() const {
        return points;
    }

}