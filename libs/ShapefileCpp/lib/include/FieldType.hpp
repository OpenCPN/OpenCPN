#pragma once

#include <iostream>

namespace shp {

    enum class FieldType {
        String, Integer, Double, Logical, Invalid
    };

    std::ostream& operator << (std::ostream& os, const FieldType& fieldType);

}