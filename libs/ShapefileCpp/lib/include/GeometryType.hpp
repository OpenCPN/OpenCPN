#pragma once

#include <iostream>

namespace shp {

    enum class GeometryType {
        Point, Line, Polygon, MultiPoint,
        PointZ, LineZ, PolygonZ, MultiPointZ,
        PointM, LineM, PolygonM, MultiPointM,
        MultiPatch, NoGeometry
    };

    std::ostream& operator << (std::ostream& os, const GeometryType& geometryType);
    
    bool isPoint(GeometryType type);
    bool isLine(GeometryType type);
    bool isPolygon(GeometryType type);

}