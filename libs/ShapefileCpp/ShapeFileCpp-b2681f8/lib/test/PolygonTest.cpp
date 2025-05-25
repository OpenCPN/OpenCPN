#include "Geometry.hpp"
#include "Polygon.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, polygon_create_test) {
  shp::Ring ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}};
  shp::Polygon polygon {{ring}};
  ASSERT_EQ(1, std::size(polygon.getRings()));
  ASSERT_EQ(5, std::size(polygon.getRings()[0].getPoints()));
  ASSERT_EQ(0, polygon.getRings()[0].getPoints()[0].getX());
  ASSERT_EQ(0, polygon.getRings()[0].getPoints()[0].getY());
  ASSERT_EQ(0, polygon.getRings()[0].getPoints()[1].getX());
  ASSERT_EQ(10, polygon.getRings()[0].getPoints()[1].getY());
  ASSERT_EQ(0, polygon.getRings()[0].getPoints()[4].getX());
  ASSERT_EQ(0, polygon.getRings()[0].getPoints()[4].getY());
}

TEST(ShapefileTests, polygon_wkt_test) {
  shp::Ring ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}};
  shp::Polygon polygon {{ring}};
  ASSERT_EQ("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))", polygon.wkt());
}

TEST(ShapefileTests, polygon_clone_test) {
  shp::Ring ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}};
  shp::Polygon polygon {{ring}};
  std::unique_ptr<shp::Geometry> clonedGeometry = polygon.clone();
  shp::Polygon clonedPolygon = static_cast<shp::Polygon&>(*clonedGeometry);
  ASSERT_EQ(1, std::size(clonedPolygon.getRings()));
  ASSERT_EQ(5, std::size(clonedPolygon.getRings()[0].getPoints()));
  ASSERT_EQ(0, clonedPolygon.getRings()[0].getPoints()[0].getX());
  ASSERT_EQ(0, clonedPolygon.getRings()[0].getPoints()[0].getY());
  ASSERT_EQ(0, clonedPolygon.getRings()[0].getPoints()[1].getX());
  ASSERT_EQ(10, clonedPolygon.getRings()[0].getPoints()[1].getY());
  ASSERT_EQ(0, clonedPolygon.getRings()[0].getPoints()[4].getX());
  ASSERT_EQ(0, clonedPolygon.getRings()[0].getPoints()[4].getY());
}

TEST(ShapefileTests, polygon_tostring_test) {
  std::stringstream str;
  shp::Ring ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}};
  shp::Polygon polygon {{ring}};
  str << polygon;
  ASSERT_EQ("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))", str.str());
}
