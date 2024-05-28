#pragma once

#include <iostream>
#include <memory>
#include <map>
#include <any>
#include "Geometry.hpp"


namespace shp {

    class Feature {

        private:

            std::unique_ptr<Geometry> geometry;

            std::map<std::string, std::any> attributes;

        public:

            Feature(std::unique_ptr<Geometry> geom, std::map<std::string, std::any> attr);

            Feature(Feature const &other);    

            Geometry* getGeometry() const;

            std::map<std::string, std::any> getAttributes() const;

    };

    std::ostream& operator << (std::ostream& os, const Feature& feature);

}