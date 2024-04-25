#pragma once

#include <string>
#include <iostream>
#include "FieldType.hpp"

namespace shp {
    class Field {
        private:
            std::string name;
            FieldType type;
            int width;
            int numberOfDecimals;
        public:
            Field(std::string n, FieldType t, int width, int numberOfDecimals);
            std::string getName() const;
            FieldType getType() const;
            int getWidth() const;
            int getNumberOfDecimals() const;
            static Field createStringField(std::string name, int width);
            static Field createIntegerField(std::string name, int width);
            static Field createDoubleField(std::string name, int width, int decimalPlaces);
            static Field createLogicalField(std::string name);
    };
    std::ostream& operator << (std::ostream& os, const Field& field);
}
