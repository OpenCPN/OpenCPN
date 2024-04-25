#pragma once

#include "Line.hpp"
#include <sstream>
#include <vector>

namespace shp {

    class MultiLine : public Geometry {

        private:

            std::vector<Line> lines;

        public:

            MultiLine(std::vector<Line> lines);

            virtual std::string wkt() const override;

            virtual std::unique_ptr<Geometry> clone() const override;

            std::vector<Line> getLines() const;

    };

}