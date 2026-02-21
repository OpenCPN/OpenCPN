#include "MultiPolygon.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, multipolygon_create_test) {
  shp::MultiPolygon multiPolygon {{
    shp::Polygon {{shp::Ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}} }},
    shp::Polygon {{shp::Ring {{{20,20}, {20, 30}, {30,30}, {30, 20}, {20,20}}} }}
  }};
  ASSERT_EQ(2, std::size(multiPolygon.getPolygons()));
  ASSERT_EQ("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))", multiPolygon.getPolygons()[0].wkt());
  ASSERT_EQ("POLYGON ((20 20, 20 30, 30 30, 30 20, 20 20))", multiPolygon.getPolygons()[1].wkt());
}

TEST(ShapefileTests, multipolygon_wkt_test) {
  shp::MultiPolygon multiPolygon {{
    shp::Polygon {{shp::Ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}} }},
    shp::Polygon {{shp::Ring {{{20,20}, {20, 30}, {30,30}, {30, 20}, {20,20}}} }}
  }};
  ASSERT_EQ("MULTIPOLYGON (((0 0, 0 10, 10 10, 10 0, 0 0)), ((20 20, 20 30, 30 30, 30 20, 20 20)))", multiPolygon.wkt());
}

TEST(ShapefileTests, multipolygon_clone_test) {
  shp::MultiPolygon multiPolygon {{
    shp::Polygon {{shp::Ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}} }},
    shp::Polygon {{shp::Ring {{{20,20}, {20, 30}, {30,30}, {30, 20}, {20,20}}} }}
  }};
  std::unique_ptr<shp::Geometry> clonedGeometry = multiPolygon.clone();
  shp::MultiPolygon clonedMultiPolygon = static_cast<shp::MultiPolygon&>(*clonedGeometry);
  ASSERT_EQ(2, std::size(clonedMultiPolygon.getPolygons()));
  ASSERT_EQ("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))", clonedMultiPolygon.getPolygons()[0].wkt());
  ASSERT_EQ("POLYGON ((20 20, 20 30, 30 30, 30 20, 20 20))", clonedMultiPolygon.getPolygons()[1].wkt());
}

TEST(ShapefileTests, multipolygon_tostring_test) {
  std::stringstream str;
  shp::MultiPolygon multiPolygon {{
    shp::Polygon {{shp::Ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}} }},
    shp::Polygon {{shp::Ring {{{20,20}, {20, 30}, {30,30}, {30, 20}, {20,20}}} }}
  }};
  str << multiPolygon;
  ASSERT_EQ("MULTIPOLYGON (((0 0, 0 10, 10 10, 10 0, 0 0)), ((20 20, 20 30, 30 30, 30 20, 20 20)))", str.str());
}
