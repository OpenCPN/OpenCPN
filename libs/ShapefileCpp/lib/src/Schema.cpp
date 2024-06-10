#include "Schema.hpp"

namespace shp {

    Schema::Schema(GeometryType gt, std::vector<Field> flds) : geometryType(gt), fields(flds) {}

    GeometryType Schema::getGeometryType() const {
        return geometryType;
    }

    std::vector<Field> Schema::getFields() const {
        return fields;
    }

    std::ostream& operator << (std::ostream& os, const Schema& schema) {
        os << "Schema(" << schema.getGeometryType();
        for (Field fld : schema.getFields()) {
            os << ", "  << fld;
        }
        os << ")";
        return os;
    }


}