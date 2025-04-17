#include "Bounds.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, bounds_create_test) {
  shp::Bounds b {-180,-90,180,90};
  ASSERT_EQ(-180, b.getMinX());
  ASSERT_EQ(-90, b.getMinY());
}

TEST(ShapefileTests, bounds_tostring_test) {
  std::stringstream str;
  shp::Bounds b {-180,-90,180,90};
  str << b;
  ASSERT_EQ("BOUNDS (-180, -180, 180, 90)", str.str());
}

TEST(ShapefileTests, bounds_topolygon_test) {
  std::stringstream str;
  shp::Bounds b {-180,-90,180,90};
  std::unique_ptr<shp::Polygon> p = b.toPolygon();
  str << p->wkt();
  ASSERT_EQ("POLYGON ((-180 -90, 180 -90, 180 90, -180 -90))", str.str());
}
