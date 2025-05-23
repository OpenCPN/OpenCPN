#include "Geometry.hpp"
#include "Point.hpp"
#include "Line.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, line_create_test) {
  shp::Point pt1 {1,1};
  shp::Point pt2 {5,5};
  shp::Line line {{pt1, pt2}};
  ASSERT_EQ(2, std::size(line.getPoints()));
  ASSERT_EQ(1, line.getPoints()[0].getX());
  ASSERT_EQ(1, line.getPoints()[0].getY());
  ASSERT_EQ(5, line.getPoints()[1].getX());
  ASSERT_EQ(5, line.getPoints()[1].getY());
}

TEST(ShapefileTests, line_wkt_test) {
  shp::Line line {{shp::Point {1,1}, shp::Point {5,5}}};
  ASSERT_EQ("LINESTRING (1 1, 5 5)", line.wkt());
}

TEST(ShapefileTests, line_clone_test) {
  shp::Point pt1 {1,1};
  shp::Point pt2 {5,5};
  shp::Line line {{pt1, pt2}};
  std::unique_ptr<shp::Geometry> clonedGeometry = line.clone();
  shp::Line clonedLine = static_cast<shp::Line&>(*clonedGeometry);
  ASSERT_EQ(2, std::size(clonedLine.getPoints()));
  ASSERT_EQ(1, clonedLine.getPoints()[0].getX());
  ASSERT_EQ(1, clonedLine.getPoints()[0].getY());
  ASSERT_EQ(5, clonedLine.getPoints()[1].getX());
  ASSERT_EQ(5, clonedLine.getPoints()[1].getY());
}

TEST(ShapefileTests, line_tostring_test) {
  std::stringstream str;
  shp::Line line {{shp::Point {1,1}, shp::Point {5,5}}};
  str << line;
  ASSERT_EQ("LINESTRING (1 1, 5 5)", str.str());
}
