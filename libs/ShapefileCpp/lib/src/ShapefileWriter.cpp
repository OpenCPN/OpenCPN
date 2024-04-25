#include "ShapefileWriter.hpp"

namespace shp {

    ShapefileWriter::ShapefileWriter(std::string fileName, const Schema& schema) : fileName(fileName), schema(schema) {
        shp = SHPCreate(fileName.c_str(), getShapeType());
        size_t i = fileName.find(".shp");
        dbf = DBFCreate(fileName.replace(i, 4, ".dbf").c_str());
        for(const auto& fld : schema.getFields()) {
            DBFAddField(
                dbf, 
                fld.getName().c_str(), 
                getFieldType(fld.getType()),
                fld.getWidth(),
                fld.getNumberOfDecimals()
            );
        }
    }

    ShapefileWriter::~ShapefileWriter() {
        SHPClose(shp);
        DBFClose(dbf);
    }

    void ShapefileWriter::write(Feature const & feature) {
        int n = writeGeometry(feature);
        writeProperties(n, feature); 
    }

    int ShapefileWriter::writeGeometry(Feature const & feature) {
        SHPObject* obj = createSHPObject(feature.getGeometry());
        int n = SHPWriteObject(shp, -1, obj);
        SHPDestroyObject(obj);  
        return n;    
    }

    void ShapefileWriter::writeProperties(int n, Feature const & feature) {
        int f = 0;
        for(const auto& fld : schema.getFields()) {
            auto value = feature.getAttributes()[fld.getName()];
            if (fld.getType() == FieldType::Integer) {
                DBFWriteIntegerAttribute(dbf, n, f, std::any_cast<int>(value));
            } else if (fld.getType() == FieldType::String) {
                std::string stringValue = std::any_cast<std::string>(value);
                DBFWriteStringAttribute(dbf, n, f, stringValue.c_str());
            } else if (fld.getType() == FieldType::Double) {
                DBFWriteDoubleAttribute(dbf, n, f, std::any_cast<double>(value));
            } else if (fld.getType() == FieldType::Logical) {
                DBFWriteLogicalAttribute(dbf, n, f, std::any_cast<bool>(value));
            }
            f++;
        }
    }

    SHPObject* ShapefileWriter::createSHPObject(Geometry* geometry) {
        SHPObject* obj;
        if (schema.getGeometryType() == GeometryType::Point) {
            Point* pt = static_cast<Point *>(geometry);
            obj = createPointObject(pt);
        } else if (schema.getGeometryType() == GeometryType::Line) {
            if (dynamic_cast<Line *>(geometry)) {
                Line* line = static_cast<Line *>(geometry);
                obj = createLineObject(line);
            } else {
                MultiLine* multiLine = static_cast<MultiLine *>(geometry);
                obj = createMultiLineObject(multiLine);
            }
        } else if (schema.getGeometryType() == GeometryType::Polygon) {
            if (dynamic_cast<Polygon *>(geometry)) {
                Polygon* poly = static_cast<Polygon *>(geometry);
                obj = createPolygonObject(poly);
            } else {
                MultiPolygon* multiPolygon = static_cast<MultiPolygon *>(geometry);    
                obj = createMultiPolygonObject(multiPolygon);                              
            }
        } else if (schema.getGeometryType() == GeometryType::MultiPoint) {
            MultiPoint* pts = static_cast<MultiPoint *>(geometry);
            obj = createMultiPointObject(pts);
        }
        return obj;
    }

    int ShapefileWriter::getShapeType() {
        int shapeType;
        if (schema.getGeometryType() == GeometryType::Point) {
            shapeType = SHPT_POINT;
        } else if (schema.getGeometryType() == GeometryType::PointZ) {
            shapeType = SHPT_POINTZ;
        } else if (schema.getGeometryType() == GeometryType::PointM) {
            shapeType = SHPT_POINTM;
        } else if (schema.getGeometryType() == GeometryType::Line) {
            shapeType = SHPT_ARC;
        } else if (schema.getGeometryType() == GeometryType::LineZ) {
            shapeType = SHPT_ARCZ;
        } else if (schema.getGeometryType() == GeometryType::LineM) {
            shapeType = SHPT_ARCM;
        } else if (schema.getGeometryType() == GeometryType::Polygon) {
            shapeType = SHPT_POLYGON;
        } else if (schema.getGeometryType() == GeometryType::PolygonZ) {
            shapeType = SHPT_POLYGONZ;
        } else if (schema.getGeometryType() == GeometryType::PolygonM) {
            shapeType = SHPT_POLYGONM;
        } else if (schema.getGeometryType() == GeometryType::MultiPoint) {
            shapeType = SHPT_MULTIPOINT;
        } else if (schema.getGeometryType() == GeometryType::MultiPointZ) {
            shapeType = SHPT_MULTIPOINTZ;
        } else if (schema.getGeometryType() == GeometryType::MultiPointM) {
            shapeType = SHPT_MULTIPOINTM;
        } else if (schema.getGeometryType() == GeometryType::MultiPatch) {
            shapeType = SHPT_MULTIPATCH;
        } else {
            shapeType = SHPT_NULL;
        }
        return shapeType;
    }

    DBFFieldType ShapefileWriter::getFieldType(FieldType fieldType) {
        if (fieldType == shp::FieldType::String) {
            return FTString;
        } else if (fieldType == shp::FieldType::Integer) {
            return FTInteger;
        } else if (fieldType == shp::FieldType::Double) {
            return FTDouble;
        } else if (fieldType == shp::FieldType::Logical) {
            return FTLogical;
        } else /*if (fieldType == shp::FieldType::Invalid)*/ {
            return FTInvalid;
        }
    }

    SHPObject* ShapefileWriter::createPointObject(Point *pt) {
        double x[1] = {pt->getX()};
        double y[1] = {pt->getY()};
        double z[1] = {pt->getZ()};
        double m[1] = {pt->getM()};
        int starts[0];
        int types[0];
        SHPObject* obj = SHPCreateObject(
            getShapeType(),
            -1,
            1,
            starts,
            types,
            1,
            x,
            y,
            z,
            m
        );        
        return obj;
    }

    SHPObject* ShapefileWriter::createLineObject(Line *line) {
        int numPoints = line->getPoints().size();
        double x[numPoints];
        double y[numPoints];
        double z[numPoints];
        double m[numPoints];
        int starts[0];
        int types[0];
        int i = 0;
        for(Point pt : line->getPoints()) {
            x[i] = pt.getX();
            y[i] = pt.getY();
            z[i] = pt.getZ();
            m[i] = pt.getM();
            i++;
        }
        SHPObject* obj = SHPCreateObject(
            getShapeType(),
            -1,
            1,
            starts,
            types,
            1,
            x,
            y,
            z,
            m
        );    
        return obj;
    }

    SHPObject* ShapefileWriter::createMultiLineObject(MultiLine *multiLine) {
        int numberOfLines = multiLine->getLines().size();
        int startOfLines[numberOfLines];
        int partTypes[numberOfLines];
        int i = 0;
        int numberOfVertices = 0;
        for(Line line : multiLine->getLines()) {
            partTypes[i] = SHPP_RING;
            startOfLines[i] = numberOfVertices;
            int numberOfPoints = line.getPoints().size();
            numberOfVertices += numberOfPoints;
            i++;
        }

        double x[numberOfVertices];
        double y[numberOfVertices];
        double z[numberOfVertices];
        double m[numberOfVertices];
        i = 0;
        for(Line line : multiLine->getLines()) {
            for(Point p : line.getPoints()) {
                x[i] = p.getX();
                y[i] = p.getY();
                z[i] = p.getZ();
                m[i] = p.getM();
                i++;
            }
        }
    
        SHPObject* obj = SHPCreateObject(
            getShapeType(),
            -1,
            numberOfLines,
            startOfLines,
            partTypes,
            numberOfVertices,
            x,
            y,
            z,
            m
        );    
        return obj;
    }

    SHPObject* ShapefileWriter::createPolygonObject(Polygon *poly) {
        int numRings = poly->getRings().size();
        int ringStart[numRings];
        int partTypes[numRings];
        int i = 0;
        int numberOfVertices = 0;
        for(Ring r : poly->getRings()) {
            if (i == 0) {
                partTypes[i] = SHPP_OUTERRING;
            } else {
                partTypes[i] = SHPP_INNERRING;
            }
            int numberOfPoints = r.getPoints().size();
            ringStart[i] = numberOfVertices;
            numberOfVertices += numberOfPoints;
            i++;
        }

        double x[numberOfVertices];
        double y[numberOfVertices];
        double z[numberOfVertices];
        double m[numberOfVertices];
        i = 0;
        for(Ring r : poly->getRings()) {
            for(Point p : r.getPoints()) {
                x[i] = p.getX();
                y[i] = p.getY();
                z[i] = p.getZ();
                m[i] = p.getM();
                i++;
            }
        }
        
        SHPObject* obj = SHPCreateObject(
            getShapeType(),
            -1,
            numRings,
            ringStart,
            partTypes,
            numberOfVertices,
            x,
            y,
            z,
            m
        );    
        return obj;
    }

    SHPObject* ShapefileWriter::createMultiPointObject(MultiPoint *pts) {
        int numberOfPoints = pts->getPoints().size();
        int offsets[numberOfPoints];
        double x[numberOfPoints];
        double y[numberOfPoints];
        double z[numberOfPoints];
        double m[numberOfPoints];
        int i = 0;
        for(Point p : pts->getPoints()) {
            offsets[i] = i * 2;
            x[i] = p.getX();
            y[i] = p.getY();
            z[i] = p.getZ();
            m[i] = p.getM();
            i++;
        }
        
        SHPObject* obj = SHPCreateObject(
            SHPT_MULTIPOINT,
            -1,
            numberOfPoints,
            offsets,
            NULL,
            numberOfPoints,
            x,
            y,
            z,
            m
        );   
        return obj; 
    }

    SHPObject* ShapefileWriter::createMultiPolygonObject(MultiPolygon *multiPolygon) {
        int numberOfPolygons = multiPolygon->getPolygons().size();
        int numberOfRings = 0;
        int numberOfVertices = 0;
        for(Polygon polygon : multiPolygon->getPolygons()) {
            numberOfRings += polygon.getRings().size();
            for(Ring r : polygon.getRings()) {
                for(Point p : r.getPoints()) {
                    numberOfVertices++;
                }
            }
        }
        int startOfRings[numberOfRings];
        int partTypes[numberOfRings];
        double x[numberOfVertices];
        double y[numberOfVertices];
        double z[numberOfVertices];
        double m[numberOfVertices];
        int i = 0;
        int totalRingCounter = 0;
        for(Polygon polygon : multiPolygon->getPolygons()) {
            int ringCounter = 0;
            for(Ring r : polygon.getRings()) {
                startOfRings[totalRingCounter] = i;
                if (ringCounter == 0) {
                    partTypes[totalRingCounter] = SHPP_RING;
                } else {
                    partTypes[totalRingCounter] = SHPP_INNERRING;    
                }
                for(Point p : r.getPoints()) {
                    x[i] = p.getX();
                    y[i] = p.getY();
                    z[i] = p.getZ();
                    m[i] = p.getM();
                    i++;
                }
                ringCounter++;
                totalRingCounter++;
            }
        }      
        SHPObject* obj = SHPCreateObject(
            getShapeType(),
            -1,
            numberOfRings,
            startOfRings,
            partTypes,
            numberOfVertices,
            x,
            y,
            z,
            m
        );
        return obj;
    }

}