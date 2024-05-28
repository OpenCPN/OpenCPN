#pragma once

#include "Geometry.hpp"
#include <sstream>
#include <math.h>

namespace shp {

    class Point : public Geometry {
        
        private:

            double x;
            double y;
            double m;
            double z;

        public:

            Point(double xCoord, double yCoord);

            Point(double xCoord, double yCoord, double z, double m);

            double getX() const;

            double getY() const;

            double getZ() const;

            double getM() const;

            virtual std::string wkt() const override;

            virtual std::unique_ptr<Geometry> clone() const override;
            
            static Point xy(double x, double y);
            static Point xym(double x, double y, double m);
            static Point xyz(double x, double y, double z);
            static Point xyzm(double x, double y, double z, double m);
            static Point empty();

    };

}