#include "Geometry.hpp"
#include "Point.hpp"
#include "Ring.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, ring_create_test) {
  shp::Ring ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}};
  ASSERT_EQ(5, std::size(ring.getPoints()));
  ASSERT_EQ(0, ring.getPoints()[0].getX());
  ASSERT_EQ(0, ring.getPoints()[0].getY());
  ASSERT_EQ(0, ring.getPoints()[1].getX());
  ASSERT_EQ(10, ring.getPoints()[1].getY());
  ASSERT_EQ(0, ring.getPoints()[4].getX());
  ASSERT_EQ(0, ring.getPoints()[4].getY());
}

TEST(ShapefileTests, ring_wkt_test) {
  shp::Ring ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}};
  ASSERT_EQ("LINEARRING (0 0, 0 10, 10 10, 10 0, 0 0)", ring.wkt());
}

TEST(ShapefileTests, ring_clone_test) {
  shp::Ring ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}};
  std::unique_ptr<shp::Geometry> clonedGeometry = ring.clone();
  shp::Ring clonedRing = static_cast<shp::Ring&>(*clonedGeometry);
  ASSERT_EQ(5, std::size(clonedRing.getPoints()));
  ASSERT_EQ(0, clonedRing.getPoints()[0].getX());
  ASSERT_EQ(0, clonedRing.getPoints()[0].getY());
  ASSERT_EQ(0, clonedRing.getPoints()[1].getX());
  ASSERT_EQ(10, clonedRing.getPoints()[1].getY());
}

TEST(ShapefileTests, ring_tostring_test) {
  std::stringstream str;
  shp::Ring ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}};
  str << ring;
  ASSERT_EQ("LINEARRING (0 0, 0 10, 10 10, 10 0, 0 0)", str.str());
}
