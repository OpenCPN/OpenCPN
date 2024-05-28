#pragma once

#include "Ring.hpp"
#include <sstream>
#include <vector>

namespace shp {

    class Polygon : public Geometry {

        private:

            std::vector<Ring> rings;

        public:

            Polygon(Ring ring);

            Polygon(std::vector<Ring> rings);

            virtual std::string wkt() const override;

            virtual std::unique_ptr<Geometry> clone() const override;

            std::vector<Ring> getRings() const;

    };


}