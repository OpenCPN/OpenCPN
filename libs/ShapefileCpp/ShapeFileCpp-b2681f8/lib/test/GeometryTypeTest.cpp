#include "GeometryType.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, geometry_type_multipatch_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::MultiPatch;
  str << type;
  ASSERT_EQ("MultiPatch", str.str());
}

TEST(ShapefileTests, geometry_type_none_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::None;
  str << type;
  ASSERT_EQ("None", str.str());
}

// Regular

TEST(ShapefileTests, geometry_type_point_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::Point;
  str << type;
  ASSERT_EQ("Point", str.str());
}

TEST(ShapefileTests, geometry_type_line_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::Line;
  str << type;
  ASSERT_EQ("Line", str.str());
}

TEST(ShapefileTests, geometry_type_polygon_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::Polygon;
  str << type;
  ASSERT_EQ("Polygon", str.str());
}

TEST(ShapefileTests, geometry_type_multipoint_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::MultiPoint;
  str << type;
  ASSERT_EQ("MultiPoint", str.str());
}

// Z

TEST(ShapefileTests, geometry_type_pointz_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::PointZ;
  str << type;
  ASSERT_EQ("PointZ", str.str());
}

TEST(ShapefileTests, geometry_type_linez_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::LineZ;
  str << type;
  ASSERT_EQ("LineZ", str.str());
}

TEST(ShapefileTests, geometry_type_polygonz_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::PolygonZ;
  str << type;
  ASSERT_EQ("PolygonZ", str.str());
}

TEST(ShapefileTests, geometry_type_multipointz_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::MultiPointZ;
  str << type;
  ASSERT_EQ("MultiPointZ", str.str());
}

// M

TEST(ShapefileTests, geometry_type_pointm_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::PointM;
  str << type;
  ASSERT_EQ("PointM", str.str());
}

TEST(ShapefileTests, geometry_type_linem_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::LineM;
  str << type;
  ASSERT_EQ("LineM", str.str());
}

TEST(ShapefileTests, geometry_type_polygonm_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::PolygonM;
  str << type;
  ASSERT_EQ("PolygonM", str.str());
}

TEST(ShapefileTests, geometry_type_multipointm_to_string_test) {
  std::stringstream str;
  shp::GeometryType type = shp::GeometryType::MultiPointM;
  str << type;
  ASSERT_EQ("MultiPointM", str.str());
}

TEST(ShapefileTests, geometry_type_is_point_test) {
  ASSERT_TRUE(shp::isPoint(shp::GeometryType::Point));
  ASSERT_TRUE(shp::isPoint(shp::GeometryType::PointM));
  ASSERT_TRUE(shp::isPoint(shp::GeometryType::PointZ));
  ASSERT_FALSE(shp::isPoint(shp::GeometryType::Line));
}

TEST(ShapefileTests, geometry_type_is_line_test) {
  ASSERT_TRUE(shp::isLine(shp::GeometryType::Line));
  ASSERT_TRUE(shp::isLine(shp::GeometryType::LineM));
  ASSERT_TRUE(shp::isLine(shp::GeometryType::LineZ));
  ASSERT_FALSE(shp::isLine(shp::GeometryType::Point));
}

TEST(ShapefileTests, geometry_type_is_polygon_test) {
  ASSERT_TRUE(shp::isPolygon(shp::GeometryType::Polygon));
  ASSERT_TRUE(shp::isPolygon(shp::GeometryType::PolygonM));
  ASSERT_TRUE(shp::isPolygon(shp::GeometryType::PolygonZ));
  ASSERT_FALSE(shp::isPolygon(shp::GeometryType::Line));
}
