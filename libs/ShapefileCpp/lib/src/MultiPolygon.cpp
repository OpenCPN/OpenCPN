#include "MultiPolygon.hpp"

namespace shp {

    MultiPolygon::MultiPolygon(std::vector<Polygon> polys) : polygons(polys) { }

    std::unique_ptr<Geometry> MultiPolygon::clone() const  {
        return std::make_unique<MultiPolygon>(polygons);
    }

    std::string MultiPolygon::wkt() const {
        std::stringstream str;
        str << "MULTIPOLYGON ";
        if (std::size(polygons) == 0) {
           str << " EMPTY";     
        } else {
            str << "(";
            bool firstPolygon = true;
            for(auto p : polygons) {
                if (firstPolygon != true) {
                    str << ", ";
                }
                firstPolygon = false;
                str << "(";
                bool firstRing = true;
                for(auto r : p.getRings()) {
                    if (firstRing != true) {
                        str << ", ";
                    }
                    firstRing = false;
                    str << "(";
                    bool firstTime = true;
                    for(auto p : r.getPoints()) {
                        if (firstTime != true) {
                            str << ", ";
                        }
                        firstTime = false;
                        str << p.getX() << " " << p.getY();
                    }
                    str << ")";
                }
                str << ")";
            }
        }
        str <<  ")";
        return str.str();
    }

    std::vector<Polygon> MultiPolygon::getPolygons() const {
        return polygons;
    }

}