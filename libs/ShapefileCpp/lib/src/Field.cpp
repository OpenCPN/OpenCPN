#include "Field.hpp"

namespace shp {

    Field::Field(std::string n, FieldType t, int w, int num) : name(n), type(t), width(w), numberOfDecimals(num) {}

    std::string Field::getName() const {
        return name;
    }
    FieldType Field::getType() const {
        return type;
    }

    int Field::getWidth() const {
        return width;
    }

    int Field::getNumberOfDecimals() const {
        return numberOfDecimals;
    }

    std::ostream& operator << (std::ostream& os, const Field& field) {
        os << "Field(" 
            << "name=" << field.getName() 
            << ", type=" << field.getType() 
            << ", width=" << field.getWidth() 
            << ", numberOfDecimals=" << field.getNumberOfDecimals() 
            << ")";
        return os;
    }

    Field Field::createStringField(std::string name, int width) {
        return Field {name, FieldType::String, width, 0};
    }

    Field Field::createIntegerField(std::string name, int width) {
       return Field {name, FieldType::Integer, width, 0};     
    }
    Field Field::createDoubleField(std::string name, int width, int decimalPlaces) {
        return Field {name, FieldType::Double, width, decimalPlaces};
    }

    Field Field::createLogicalField(std::string name) {
        return Field {name, FieldType::Logical, 5, 0};
    }

}