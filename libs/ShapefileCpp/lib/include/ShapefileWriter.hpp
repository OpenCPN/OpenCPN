#pragma once

#include <shapefil.h>
#include "Feature.hpp"
#include "Schema.hpp"
#include "Geometry.hpp"
#include "Line.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "Ring.hpp"
#include "MultiPoint.hpp"
#include "MultiLine.hpp"
#include "MultiPolygon.hpp"

namespace shp {

    class ShapefileWriter {

        private:

            SHPHandle shp;
            DBFHandle dbf;
            std::string fileName;
            const Schema& schema;

            int writeGeometry(Feature const & feature);
            void writeProperties(int n, Feature const & feature);

            DBFFieldType getFieldType(FieldType fieldType);
            int getShapeType();

            SHPObject* createSHPObject(Geometry* geometry);
            SHPObject* createPointObject(Point *pt);
            SHPObject* createMultiPointObject(MultiPoint *multiPoint);
            SHPObject* createLineObject(Line *line);
            SHPObject* createMultiLineObject(MultiLine *lines);
            SHPObject* createPolygonObject(Polygon *polygon);
            SHPObject* createMultiPolygonObject(MultiPolygon *multiPolygon);

        public:

            ShapefileWriter(std::string fileName, const Schema& schema);

            ~ShapefileWriter();

            void write(Feature const & feature);
            

    };


}