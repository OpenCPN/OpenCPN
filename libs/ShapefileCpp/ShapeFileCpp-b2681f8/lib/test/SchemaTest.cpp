#include "Schema.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, schema_create_test) {
  shp::Schema schema { shp::GeometryType::Point, std::vector<shp::Field> {
      shp::Field {"id", shp::FieldType::Integer, 3, 0},
      shp::Field {"name", shp::FieldType::String, 100, 0}
  }};
  ASSERT_EQ(shp::GeometryType::Point, schema.getGeometryType());
  ASSERT_EQ(2, std::size(schema.getFields()));
  ASSERT_EQ("id", schema.getFields()[0].getName());
  ASSERT_EQ("name", schema.getFields()[1].getName());
  ASSERT_EQ(shp::FieldType::Integer, schema.getFields()[0].getType());
  ASSERT_EQ(shp::FieldType::String, schema.getFields()[1].getType());
}
