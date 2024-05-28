#include "ShapefileReader.hpp"

namespace shp {

    ShapefileReader::ShapefileReader(std::string fileName) {
        shp = SHPOpen(fileName.c_str(), "rb");
        size_t i = fileName.find(".shp");
        dbf = DBFOpen(fileName.replace(i, 4, ".dbf").c_str(), "rb");
    }

    ShapefileReader::~ShapefileReader() {
        SHPClose(shp);
        DBFClose(dbf);
    }

    void ShapefileReader::getShapeInfo() {
        if (count == -1) {
            SHPGetInfo(shp, &count, &shapeType, min, max);
            numberOfFeatures = DBFGetRecordCount(dbf);
        }
    }

    int ShapefileReader::getCount() {
        getShapeInfo();
        return numberOfFeatures;
    }

    GeometryType ShapefileReader::getGeometryType() {
        getShapeInfo();
        // Point
        if (shapeType == SHPT_POINT) {
            return GeometryType::Point;
        } else if (shapeType == SHPT_POINTM) {
            return GeometryType::PointM;
        } else if (shapeType == SHPT_POINTZ) {
            return GeometryType::PointZ;
        } 
        // Line
        else if (shapeType == SHPT_ARC) {
            return GeometryType::Line;
        } else if (shapeType == SHPT_ARCM) {
            return GeometryType::LineM;
        } else if (shapeType == SHPT_ARCZ) {
            return GeometryType::LineZ;
        }
        // Polygon
        else if (shapeType == SHPT_POLYGON) {
            return GeometryType::Polygon;
        } else if (shapeType == SHPT_POLYGONM) {
            return GeometryType::PolygonM;
        } else if (shapeType == SHPT_POLYGONZ) {
            return GeometryType::PolygonZ;
        }
        // MultiPoint
        else if (shapeType == SHPT_MULTIPOINT) {
            return GeometryType::MultiPoint;
        } else if (shapeType == SHPT_MULTIPOINTM) {
            return GeometryType::MultiPointM;
        } else if (shapeType == SHPT_MULTIPOINTZ) {
            return GeometryType::MultiPointZ;
        }
        else {
            return GeometryType::NoGeometry;
        }
    }

    Bounds ShapefileReader::getBounds() {
        getShapeInfo();
        return Bounds(min[0], min[1], max[0], max[1]);
    }

    FieldType getFieldType(DBFFieldType dbfFieldType) {
        if (dbfFieldType == FTString) {
            return FieldType::String;
        } else if (dbfFieldType == FTInteger) {
            return FieldType::Integer;
        } else if (dbfFieldType == FTDouble) {
            return FieldType::Double;
        } else if (dbfFieldType == FTLogical) {
            return FieldType::Logical;
        } else /*if (dbfFieldType == FTInvalid)*/ {
            return FieldType::Invalid;
        }
    }

    std::vector<Field> ShapefileReader::getFields() {
        std::vector<Field> fields;
        int numberOfFields = DBFGetFieldCount(dbf);
        for(int i = 0; i<numberOfFields; i++) {
            char fieldName[50];
            int fieldWidth;
            int fieldDecimals;
            DBFFieldType fieldType = DBFGetFieldInfo(dbf, i, fieldName, &fieldWidth, &fieldDecimals);
            Field field = {fieldName, getFieldType(fieldType), fieldWidth, fieldDecimals};
            fields.push_back(field);
        }
        return fields;
    }

    bool ShapefileReader::isPoint(int shapeType) {
        return shapeType == SHPT_POINT || shapeType == SHPT_POINTM || shapeType == SHPT_POINTZ;
    }

    bool ShapefileReader::isMultiPoint(int shapeType) {
        return shapeType == SHPT_MULTIPOINT || shapeType == SHPT_MULTIPOINTM || shapeType == SHPT_MULTIPOINTZ;
    }

    bool ShapefileReader::isLine(int shapeType) {
        return shapeType == SHPT_ARC || shapeType == SHPT_ARCM || shapeType == SHPT_ARCZ;
    }

    bool ShapefileReader::isPolygon(int shapeType) {
        return shapeType == SHPT_POLYGON || shapeType == SHPT_POLYGONM || shapeType == SHPT_POLYGONZ;
    }

    bool ShapefileReader::isXY(int shapeType) {
        return shapeType == SHPT_POINT || shapeType == SHPT_ARC || shapeType == SHPT_POLYGON || shapeType == SHPT_MULTIPOINT;
    }

    bool ShapefileReader::isXYM(int shapeType) {
        return shapeType == SHPT_POINTM|| shapeType == SHPT_ARCM || shapeType == SHPT_POLYGONM || shapeType == SHPT_MULTIPOINTM;
    }

    bool ShapefileReader::isXYZ(int shapeType) {
        return shapeType == SHPT_POINTZ || shapeType == SHPT_ARCZ || shapeType == SHPT_POLYGONZ || shapeType == SHPT_MULTIPOINTZ;
    }

    Point ShapefileReader::getPoint(const SHPObject& obj, int index) {
        int shapeType = obj.nSHPType;
        if (isXY(shapeType)) {
            return Point::xy(obj.padfX[index], obj.padfY[index]);
        } else if (isXYM(shapeType)) {
            return Point::xym(obj.padfX[index], obj.padfY[index], obj.padfM[index]);
        } else /* if (isXYZ(shapeType))*/ {
            return Point::xyzm(obj.padfX[index], obj.padfY[index], obj.padfM[index], obj.padfZ[index]);
        }
    }

    std::unique_ptr<Geometry> ShapefileReader::createPoint(const SHPObject& obj, int index) {
        return std::make_unique<Point>(getPoint(obj, index));
    }

    std::unique_ptr<Geometry> ShapefileReader::createLine(const SHPObject& obj) {
        std::vector<Line> lines;
        int parts = obj.nParts;
        for(int p = 0; p < parts; p++) {
            std::vector<Point> points;
            int start = obj.panPartStart[p];
            int end = p + 1 < parts ? obj.panPartStart[p + 1] : obj.nVertices;
            for(int i = start; i < end; i++) {
                points.push_back(getPoint(obj, i));
            }
            lines.push_back(Line(points));
        }
        if (parts > 1) {
            std::unique_ptr<Geometry> g = std::make_unique<MultiLine>(lines);    
            return g;
        } else {
            std::unique_ptr<Geometry> g = std::make_unique<Line>(lines[0].getPoints());
            return g;
        }
    }

    std::unique_ptr<Geometry> ShapefileReader::createPolygon(const SHPObject& obj) {
            std::vector<Ring> rings;
            int parts = obj.nParts;
            for(int p = 0; p < parts; p++) {
                std::vector<Point> points;
                int start = obj.panPartStart[p];
                int end = p + 1 < parts ? obj.panPartStart[p + 1] : obj.nVertices;
                for(int i = start; i < end; i++) {
                    points.push_back(getPoint(obj, i));
                }
                rings.push_back(points);
            }
            std::unique_ptr<Geometry> g = std::make_unique<Polygon>(rings);
            return g;
    }

    std::unique_ptr<Geometry> ShapefileReader::createMultiPoint(const SHPObject& obj) {
        std::vector<Point> points;
        int numberOfVertices = obj.nVertices;
        for(int i = 0; i < numberOfVertices; i++) {
            points.push_back(getPoint(obj, i));
        }
        std::unique_ptr<Geometry> g = std::make_unique<MultiPoint>(points);
        return g;
    }

    std::unique_ptr<Geometry> ShapefileReader::createGeometry(const SHPObject& obj) {
        if (obj.nSHPType == SHPT_POINT || obj.nSHPType == SHPT_POINTZ || obj.nSHPType == SHPT_POINTM) {
            return createPoint(obj, 0);
        } 
        else if (obj.nSHPType == SHPT_ARC || obj.nSHPType == SHPT_ARCZ || obj.nSHPType == SHPT_ARCM) {
            return createLine(obj);
        } 
        else if (obj.nSHPType == SHPT_POLYGON || obj.nSHPType == SHPT_POLYGONZ || obj.nSHPType == SHPT_POLYGONM) {
            return createPolygon(obj);
        }
        else /*if (obj.nSHPType == SHPT_MULTIPOINT || obj.nSHPType == SHPT_MULTIPOINTZ || obj.nSHPType == SHPT_MULTIPOINTM)*/ {
            return createMultiPoint(obj);
        } 
    }

    std::map<std::string, std::any> ShapefileReader::getProperties(int index) {
        std::map<std::string, std::any> attributes;
        std::vector<Field> fields = getFields();
        
        for(int f = 0; f < fields.size(); f++) {
            Field field = fields[f];
            if (field.getType() == FieldType::String) {
                attributes[field.getName()] = DBFReadStringAttribute(dbf, index, f);
            } else if (field.getType() == FieldType::Integer) {
                attributes[field.getName()] = DBFReadIntegerAttribute(dbf, index, f);
            } else if (field.getType() == FieldType::Double) {
                attributes[field.getName()] = DBFReadDoubleAttribute(dbf, index, f);
            } else if (field.getType() == FieldType::Logical) {
                attributes[field.getName()] = DBFReadStringAttribute(dbf, index, f);
            } else {
                attributes[field.getName()] = NULL;
            }
        }    
        
        return attributes;
    }

    Feature ShapefileReader::getFeature(int i) {
        getShapeInfo();
        SHPObject *obj = SHPReadObject(shp, i);
        std::unique_ptr<Geometry> g = createGeometry(*obj);
        std::map<std::string, std::any> attributes = getProperties(i);
        Feature f{std::move(g), attributes};
        SHPDestroyObject(obj);
        return f;
    }

    void ShapefileReader::features(std::function<void(Feature feature)> func) {
        int count = this->getCount();
        for(int i = 0; i < count; ++i) {
            func(this->getFeature(i));
        }
    }

    FeatureIterator ShapefileReader::begin() {
        return FeatureIterator(*this, 0);
    }

    FeatureIterator ShapefileReader::end() {
        return FeatureIterator(*this, this->getCount());
    }

}
