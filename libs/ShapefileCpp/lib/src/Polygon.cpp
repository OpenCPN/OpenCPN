#include "Polygon.hpp"

namespace shp {

    Polygon::Polygon(Ring ring) : rings(std::vector<Ring> {ring}) {}

    Polygon::Polygon(std::vector<Ring> rngs) : rings(rngs) {}

    std::unique_ptr<Geometry> Polygon::clone() const  {
        return std::make_unique<Polygon>(rings);
    }

    std::string Polygon::wkt() const {
        std::stringstream str;
        str << "POLYGON ";
        if (std::size(rings) == 0) {
            str << "EMPTY";    
        } else {
            str << "(";
            bool firstRing = true;
            for(auto r : rings) {
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
            str <<  ")";
        }
        return str.str();
    }

    std::vector<Ring> Polygon::getRings() const {
        return rings;
    }

}