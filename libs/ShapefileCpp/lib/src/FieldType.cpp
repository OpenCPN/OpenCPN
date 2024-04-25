#include "FieldType.hpp"

namespace shp {

    std::ostream& operator << (std::ostream& os, const shp::FieldType& fieldType) {
        if (fieldType == shp::FieldType::String) {
            os << "String";
        } else if (fieldType == shp::FieldType::Integer) {
            os << "Integer";
        } else if (fieldType == shp::FieldType::Double) {
            os << "Double";
        } else if (fieldType == shp::FieldType::Logical) {
            os << "Logical";
        } else if (fieldType == shp::FieldType::Invalid) {
            os << "Invalid";
        } else {
            os << "None";
        }
        return os;
    }

}