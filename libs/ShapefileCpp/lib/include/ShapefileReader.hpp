#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <functional>
#include <shapefil.h>
#include "GeometryType.hpp"
#include "Bounds.hpp"
#include "Feature.hpp"
#include "Field.hpp"
#include "Geometry.hpp"
#include "Point.hpp"
#include "Line.hpp"
#include "Ring.hpp"
#include "Polygon.hpp"
#include "MultiPoint.hpp"
#include "MultiLine.hpp"
#include "MultiPolygon.hpp"
#include "FeatureIterator.hpp"

namespace shp {

    class FeatureIterator;

    class ShapefileReader {

        private:

            SHPHandle shp;
            DBFHandle dbf;

            mutable int count = -1;
            mutable int shapeType = -1;
            /**
             * Minimum bounds of all shapes in the file [x, y, z, m].
             * x and y represent spatial coordinates, while z is elevation
             * and m is measure value (when present in shapefile).
             */
            mutable double min[4];
            /**
             * Maximum bounds of all shapes in the file [x, y, z, m].
             * x and y represent spatial coordinates, while z is elevation
             * and m is measure value (when present in shapefile).
             */
            mutable double max[4];
            /**
             * Total number of features (records) from the DBF file.
             */
            mutable int numberOfFeatures = 0;

            void getShapeInfo();
            void getShapeInfo() const;

            bool isPoint(int shapeType);
            bool isMultiPoint(int shapeType);
            bool isLine(int shapeType);
            bool isPolygon(int shapeType);

            bool isXY(int shapeType);
            bool isXYM(int shapeType);
            bool isXYZ(int shapeType);    

            Point getPoint(const SHPObject& obj, int index);    
            std::unique_ptr<Geometry> createPoint(const SHPObject& obj, int index);
            std::unique_ptr<Geometry> createLine(const SHPObject& obj);
            std::unique_ptr<Geometry> createGeometry(const SHPObject& obj);
            std::unique_ptr<Geometry> createPolygon(const SHPObject& obj);
            std::unique_ptr<Geometry> createMultiPoint(const SHPObject& obj);
            std::map<std::string, std::any> getProperties(int index);

        public:

            ShapefileReader(std::string fileName);

            ~ShapefileReader();

            /** Return true if the reader has opened and loaded the shapefile successfully. */
            bool isOpen();

            int getCount();
            int getCount() const;

            GeometryType getGeometryType(); 

            Bounds getBounds();

            Feature getFeature(int i);

            std::vector<Field> getFields();
            
            void features(std::function<void(Feature feature)> f);

            FeatureIterator begin();

            FeatureIterator end();

            FeatureIterator begin() const;
            FeatureIterator end() const;

    };

}
