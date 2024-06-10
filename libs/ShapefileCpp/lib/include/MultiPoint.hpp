#pragma once

#include "Point.hpp"
#include <sstream>
#include <vector>

namespace shp {

    class MultiPoint : public Geometry {

        private:

            std::vector<Point> points;

        public:

            MultiPoint(std::vector<Point> points);

            virtual std::string wkt() const override;

            virtual std::unique_ptr<Geometry> clone() const override;

            std::vector<Point> getPoints() const;

    };

}