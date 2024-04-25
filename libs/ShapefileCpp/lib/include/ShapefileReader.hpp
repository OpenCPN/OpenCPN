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

            int count = -1;
            int shapeType = -1;
            double min[4];
            double max[4];
            int numberOfFeatures = 0;

            void getShapeInfo();

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

            int getCount();

            GeometryType getGeometryType(); 

            Bounds getBounds();

            Feature getFeature(int i);

            std::vector<Field> getFields();
            
            void features(std::function<void(Feature feature)> f);

            FeatureIterator begin();

            FeatureIterator end();

    };

}
