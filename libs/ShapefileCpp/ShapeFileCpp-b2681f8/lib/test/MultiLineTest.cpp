#include "Geometry.hpp"
#include "MultiLine.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, multiline_create_test) {
  shp::MultiLine multiLine {{
    shp::Line {{shp::Point {1,1}, shp::Point {5,5}}},
    shp::Line {{shp::Point {2,2}, shp::Point {8,8}}}
  }};
  ASSERT_EQ(2, std::size(multiLine.getLines()));
  ASSERT_EQ("LINESTRING (1 1, 5 5)", multiLine.getLines()[0].wkt());
  ASSERT_EQ("LINESTRING (2 2, 8 8)", multiLine.getLines()[1].wkt());
}

TEST(ShapefileTests, multiline_wkt_test) {
  shp::MultiLine multiLine {{
    shp::Line {{shp::Point {1,1}, shp::Point {5,5}}},
    shp::Line {{shp::Point {2,2}, shp::Point {8,8}}}
  }};
  ASSERT_EQ("MULTILINESTRING ((1 1, 5 5), (2 2, 8 8))", multiLine.wkt());
}

TEST(ShapefileTests, multiline_clone_test) {
  shp::MultiLine multiLine {{
    shp::Line {{shp::Point {1,1}, shp::Point {5,5}}},
    shp::Line {{shp::Point {2,2}, shp::Point {8,8}}}
  }};
  std::unique_ptr<shp::Geometry> clonedGeometry = multiLine.clone();
  shp::MultiLine clonedMultiLine = static_cast<shp::MultiLine&>(*clonedGeometry);
  ASSERT_EQ(2, std::size(clonedMultiLine.getLines()));
  ASSERT_EQ("LINESTRING (1 1, 5 5)", clonedMultiLine.getLines()[0].wkt());
  ASSERT_EQ("LINESTRING (2 2, 8 8)", clonedMultiLine.getLines()[1].wkt());
}

TEST(ShapefileTests, multiline_tostring_test) {
  std::stringstream str;
  shp::MultiLine multiLine {{
    shp::Line {{shp::Point {1,1}, shp::Point {5,5}}},
    shp::Line {{shp::Point {2,2}, shp::Point {8,8}}}
  }};
  str << multiLine;
  ASSERT_EQ("MULTILINESTRING ((1 1, 5 5), (2 2, 8 8))", str.str());
}
