#include "MultiLine.hpp"

namespace shp {

    MultiLine::MultiLine(std::vector<Line> lns) : lines(lns) { }

    std::unique_ptr<Geometry> MultiLine::clone() const  {
        return std::make_unique<MultiLine>(lines);
    }

    std::string MultiLine::wkt() const {
        std::stringstream str;
        str << "MULTILINESTRING ";
        if (std::size(lines) == 0) {
            str <<  "EMPTY";
        } else {
            str <<  "(";
            bool firstLine = true;
            for(auto l : lines) {
                if (firstLine != true) {
                    str << ", ";
                }
                firstLine = false;
                bool firstTime = true;
                str << "(";
                for(auto p : l.getPoints()) {
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

    std::vector<Line> MultiLine::getLines() const {
        return lines;
    }

}