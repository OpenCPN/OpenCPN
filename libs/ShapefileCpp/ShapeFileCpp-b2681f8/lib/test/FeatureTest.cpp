#include "Feature.hpp"
#include "Point.hpp"
#include "gtest/gtest.h"
#include <sstream>

TEST(ShapefileTests, feature_create_test) {
  std::unique_ptr<shp::Geometry> geom = std::make_unique<shp::Point>(1.2, 3.4);
  std::map<std::string, std::any> attr = {{"id", 1}, {"name", std::string{"One"}}};
  shp::Feature f {std::move(geom), attr};
  ASSERT_EQ("POINT (1.2 3.4)", f.getGeometry()->wkt());
  ASSERT_EQ(1, std::any_cast<int>(f.getAttributes()["id"]));
  ASSERT_EQ("One", std::any_cast<std::string>(f.getAttributes()["name"]));
}

TEST(ShapefileTests, feature_tostring_test) {
  std::stringstream str;
  std::unique_ptr<shp::Geometry> geom = std::make_unique<shp::Point>(1.2, 3.4);
  std::map<std::string, std::any> attr = {{"id", 1}, {"name", std::string{"One"}}};
  shp::Feature f {std::move(geom), attr};
  str << f;
  ASSERT_EQ("Feature(POINT (1.2 3.4))", str.str());
}
