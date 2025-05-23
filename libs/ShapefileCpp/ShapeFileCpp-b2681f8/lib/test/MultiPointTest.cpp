#include "Geometry.hpp"
#include "MultiPoint.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, multipoint_create_test) {
  shp::MultiPoint pts {{{4,5},{1,2},{6,7}}};
  ASSERT_EQ(3, std::size(pts.getPoints()));
  ASSERT_EQ("POINT (4 5)", pts.getPoints()[0].wkt());
  ASSERT_EQ("POINT (1 2)", pts.getPoints()[1].wkt());
  ASSERT_EQ("POINT (6 7)", pts.getPoints()[2].wkt());
}

TEST(ShapefileTests, multipoint_wkt_test) {
  shp::MultiPoint pts {{{4,5},{1,2},{6,7}}};
  ASSERT_EQ("MULTIPOINT (4 5, 1 2, 6 7)", pts.wkt());
}

TEST(ShapefileTests, multipoint_clone_test) {
  shp::MultiPoint pts {{{4,5},{1,2},{6,7}}};
  std::unique_ptr<shp::Geometry> clonedGeometry = pts.clone();
  shp::MultiPoint clonedMultiPoint = static_cast<shp::MultiPoint&>(*clonedGeometry);
  ASSERT_EQ(3, std::size(clonedMultiPoint.getPoints()));
  ASSERT_EQ("POINT (4 5)", clonedMultiPoint.getPoints()[0].wkt());
  ASSERT_EQ("POINT (1 2)", clonedMultiPoint.getPoints()[1].wkt());
  ASSERT_EQ("POINT (6 7)", clonedMultiPoint.getPoints()[2].wkt());
}

TEST(ShapefileTests, multipoint_tostring_test) {
  std::stringstream str;
  shp::MultiPoint pts {{{4,5},{1,2},{6,7}}};
  str << pts;
  ASSERT_EQ("MULTIPOINT (4 5, 1 2, 6 7)", str.str());
}
