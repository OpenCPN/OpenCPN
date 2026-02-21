#include "FieldType.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, field_type_string_to_string_test) {
  std::stringstream str;
  shp::FieldType type = shp::FieldType::String;
  str << type;
  ASSERT_EQ("String", str.str());
}

TEST(ShapefileTests, field_type_integer_to_string_test) {
  std::stringstream str;
  shp::FieldType type = shp::FieldType::Integer;
  str << type;
  ASSERT_EQ("Integer", str.str());
}

TEST(ShapefileTests, field_type_double_to_string_test) {
  std::stringstream str;
  shp::FieldType type = shp::FieldType::Double;
  str << type;
  ASSERT_EQ("Double", str.str());
}

TEST(ShapefileTests, field_type_logical_to_string_test) {
  std::stringstream str;
  shp::FieldType type = shp::FieldType::Logical;
  str << type;
  ASSERT_EQ("Logical", str.str());
}

TEST(ShapefileTests, field_type_invalid_to_string_test) {
  std::stringstream str;
  shp::FieldType type = shp::FieldType::Invalid;
  str << type;
  ASSERT_EQ("Invalid", str.str());
}
