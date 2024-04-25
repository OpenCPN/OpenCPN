#include "GeometryType.hpp"

namespace shp {

    std::ostream& operator << (std::ostream& os, const shp::GeometryType& geometryType) {
        if (geometryType == shp::GeometryType::Point) {
            os << "Point";
        } else if (geometryType == shp::GeometryType::Line) {
            os << "Line";
        } else if (geometryType == shp::GeometryType::Polygon) {
            os << "Polygon";
        } else if (geometryType == shp::GeometryType::MultiPoint) {
            os << "MultiPoint";
        } else if (geometryType == shp::GeometryType::PointZ) {
            os << "PointZ";
        } else if (geometryType == shp::GeometryType::LineZ) {
            os << "LineZ";
        } else if (geometryType == shp::GeometryType::PolygonZ) {
            os << "PolygonZ";
        } else if (geometryType == shp::GeometryType::MultiPointZ) {
            os << "MultiPointZ";
        } else if (geometryType == shp::GeometryType::PointM) {
            os << "PointM";
        } else if (geometryType == shp::GeometryType::LineM) {
            os << "LineM";
        } else if (geometryType == shp::GeometryType::PolygonM) {
            os << "PolygonM";
        } else if (geometryType == shp::GeometryType::MultiPointM) {
            os << "MultiPointM";
        } else if (geometryType == shp::GeometryType::MultiPoint) {
            os << "MultiPoint";
        } else if (geometryType == shp::GeometryType::MultiPatch) {
            os << "MultiPatch";
        } else {
            os << "None";
        }
        return os;
    }

    bool isPoint(GeometryType type) {
        return type == GeometryType::Point || type == GeometryType::PointM || type == GeometryType::PointZ;
    }

    bool isLine(GeometryType type) {
        return type == GeometryType::Line || type == GeometryType::LineM || type == GeometryType::LineZ;
    }

    bool isPolygon(GeometryType type) {
        return type == GeometryType::Polygon || type == GeometryType::PolygonM || type == GeometryType::PolygonZ;
    }

}