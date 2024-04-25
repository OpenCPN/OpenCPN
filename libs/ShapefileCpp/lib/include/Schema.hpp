#pragma once

#include <vector>
#include "GeometryType.hpp"
#include "Field.hpp"

namespace shp {
    class Schema {
        private:
            GeometryType geometryType;
            std::vector<Field> fields;
        public:
            Schema(GeometryType geometryType, std::vector<Field> fields);
            GeometryType getGeometryType() const;
            std::vector<Field> getFields() const;
    };
    std::ostream& operator << (std::ostream& os, const Schema& schema);
}