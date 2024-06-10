#include "Feature.hpp"

namespace shp {

    Feature::Feature(std::unique_ptr<Geometry> geom, std::map<std::string, std::any> attr): geometry(geom->clone()), attributes(attr) {}

    Feature::Feature(Feature const &other) : geometry(other.geometry->clone()), attributes(other.attributes) {}

    Geometry* Feature::getGeometry() const {
        return geometry.get();
    }

    std::map<std::string, std::any> Feature::getAttributes() const {
        return attributes;
    }
    
    std::ostream& operator << (std::ostream& os, const Feature& feature) {
        os << "Feature(" << feature.getGeometry()->wkt() << ")";
        return os;
    }

}