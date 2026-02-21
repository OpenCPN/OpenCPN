#include "Geometry.hpp"
#include "Point.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, point_create_test) {
  shp::Point pt {4,5};
  ASSERT_EQ(4, pt.getX());
  ASSERT_EQ(5, pt.getY());
}

TEST(ShapefileTests, point_empty_test) {
  shp::Point pt = shp::Point::empty();
  ASSERT_TRUE(isnan(pt.getX()));
  ASSERT_TRUE(isnan(pt.getX()));
  ASSERT_EQ("POINT EMPTY", pt.wkt());
}

TEST(ShapefileTests, point_wkt_test) {
  shp::Point pt {4,5};
  ASSERT_EQ("POINT (4 5)", pt.wkt());
}

TEST(ShapefileTests, point_clone_test) {
  shp::Point pt {4,5};
  std::unique_ptr<shp::Geometry> clonedGeometry = pt.clone();
  shp::Point clonedPoint = static_cast<shp::Point&>(*clonedGeometry);
  ASSERT_EQ(4, clonedPoint.getX());
  ASSERT_EQ(5, clonedPoint.getY());
}

TEST(ShapefileTests, point_tostring_test) {
  std::stringstream str;
  shp::Point pt {4,5};
  str << pt;
  ASSERT_EQ("POINT (4 5)", str.str());
}

TEST(ShapefileTests, point_create_xy_test) {
  std::stringstream str;
  shp::Point pt = shp::Point::xy(1.23, 3.45);
  str << pt;
  ASSERT_EQ("POINT (1.23 3.45)", str.str());
}

TEST(ShapefileTests, point_create_xyz_test) {
  std::stringstream str;
  shp::Point pt = shp::Point::xyz(1.23, 3.45, 10.89);
  str << pt;
  ASSERT_EQ("POINT Z (1.23 3.45 10.89)", str.str());
}

TEST(ShapefileTests, point_create_xym_test) {
  std::stringstream str;
  shp::Point pt = shp::Point::xym(1.23, 3.45, 10.89);
  str << pt;
  ASSERT_EQ("POINT M (1.23 3.45 10.89)", str.str());
}

TEST(ShapefileTests, point_create_xyzm_test) {
  std::stringstream str;
  shp::Point pt = shp::Point::xyzm(1.23, 3.45, 10.89, 22.32);
  str << pt;
  ASSERT_EQ("POINT ZM (1.23 3.45 10.89 22.32)", str.str());
}
