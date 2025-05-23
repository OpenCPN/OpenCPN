#include "ShapefileReader.hpp"
#include "gtest/gtest.h"
#include <sstream>
#include <filesystem>

TEST(ShapefileReaderTests, shapefile_read_features_test) {
  std::filesystem::path path {"../../../data/points.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  int count = 0;
  shp.features([&](shp::Feature f) {
    count++;
  });
  ASSERT_EQ(10, count);
}

TEST(ShapefileReaderTests, shapefile_read_points_test) {
  std::filesystem::path path {"../../../data/points.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  shp::Bounds b = shp.getBounds();
  EXPECT_NEAR(-158.899, b.getMinX(), 0.001);
  EXPECT_NEAR(-82.790, b.getMinY(), 0.001);
  EXPECT_NEAR(171.620, b.getMaxX(), 0.001);
  EXPECT_NEAR(79.2818, b.getMaxY(), 0.001);
}

TEST(ShapefileReaderTests, shapefile_count_points_test) {
  std::filesystem::path path {"../../../data/points.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(10, shp.getCount());
}

TEST(ShapefileReaderTests, shapefile_count_lines_test) {
  std::filesystem::path path {"../../../data/line.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(10, shp.getCount());
}

TEST(ShapefileReaderTests, shapefile_count_polygons_test) {
  std::filesystem::path path {"../../../data/polygons.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(10, shp.getCount());
}

TEST(ShapefileReaderTests, shapefile_count_multipoints_test) {
  std::filesystem::path path {"../../../data/multipoints.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(10, shp.getCount());
}

TEST(ShapefileReaderTests, shapefile_count_multilines_test) {
  std::filesystem::path path {"../../../data/multiline.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(10, shp.getCount());
}

TEST(ShapefileReaderTests, shapefile_count_multipolygons_test) {
  std::filesystem::path path {"../../../data/multipolygons.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(10, shp.getCount());
}

TEST(ShapefileReaderTests, shapefile_geometry_type_points_test) {
  std::filesystem::path path {"../../../data/points.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(shp::GeometryType::Point, shp.getGeometryType());
}

TEST(ShapefileReaderTests, shapefile_geometry_type_lines_test) {
  std::filesystem::path path {"../../../data/line.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(shp::GeometryType::Line, shp.getGeometryType());
}

TEST(ShapefileReaderTests, shapefile_geometry_type_polygons_test) {
  std::filesystem::path path {"../../../data/polygons.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(shp::GeometryType::Polygon, shp.getGeometryType());
}

TEST(ShapefileReaderTests, shapefile_geometry_type_multipoints_test) {
  std::filesystem::path path {"../../../data/multipoints.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(shp::GeometryType::MultiPoint, shp.getGeometryType());
}

TEST(ShapefileReaderTests, shapefile_geometry_type_multilines_test) {
  std::filesystem::path path {"../../../data/multiline.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(shp::GeometryType::Line, shp.getGeometryType());
}

TEST(ShapefileReaderTests, shapefile_geometry_type_multipolygons_test) {
  std::filesystem::path path {"../../../data/multipolygons.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(shp::GeometryType::Polygon, shp.getGeometryType());
}

TEST(ShapefileReaderTests, shapefile_read_multilines_test) {
  std::filesystem::path path {"../../../data/multiline.shp"};
  shp::ShapefileReader shp {std::filesystem::absolute(path)};
  ASSERT_EQ(shp::GeometryType::Line, shp.getGeometryType());
}
