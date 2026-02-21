#include "Field.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, field_create_test) {
  shp::Field field {"id", shp::FieldType::Integer, 3, 0};
  ASSERT_EQ("id", field.getName());
  ASSERT_EQ(shp::FieldType::Integer, field.getType());
}

TEST(ShapefileTests, field_tostring_test) {
  std::stringstream str;
  shp::Field field {"id", shp::FieldType::Integer, 5, 0};
  str << field;
  ASSERT_EQ("Field(name=id, type=Integer, width=5, numberOfDecimals=0)", str.str());
}

TEST(ShapefileTests, field_create_string_test) {
  std::stringstream str;
  shp::Field field = shp::Field::createStringField("type", 10);
  str << field;
  ASSERT_EQ("Field(name=type, type=String, width=10, numberOfDecimals=0)", str.str());
}

TEST(ShapefileTests, field_create_integer_test) {
  std::stringstream str;
  shp::Field field = shp::Field::createIntegerField("id", 3);
  str << field;
  ASSERT_EQ("Field(name=id, type=Integer, width=3, numberOfDecimals=0)", str.str());
}

TEST(ShapefileTests, field_create_double_test) {
  std::stringstream str;
  shp::Field field = shp::Field::createDoubleField("value", 100, 5);
  str << field;
  ASSERT_EQ("Field(name=value, type=Double, width=100, numberOfDecimals=5)", str.str());
}

TEST(ShapefileTests, field_create_logical_test) {
  std::stringstream str;
  shp::Field field = shp::Field::createLogicalField("active");
  str << field;
  ASSERT_EQ("Field(name=active, type=Logical, width=5, numberOfDecimals=0)", str.str());
}
