#include "ShapefileWriter.hpp"
#include "ShapefileReader.hpp"
#include "gtest/gtest.h"
#include <sstream>
#include <filesystem>

TEST(ShapefileTests, shapefilewriter_create_points_test) {

  std::filesystem::path path {"points.shp"};

  {
    shp::Schema schema { shp::GeometryType::Point, std::vector<shp::Field> {
        shp::Field {"id", shp::FieldType::Integer, 3, 0},
        shp::Field {"name", shp::FieldType::String, 100, 0}
    }};
    shp::ShapefileWriter shp {std::filesystem::absolute(path), schema};

    std::unique_ptr<shp::Geometry> pt = std::make_unique<shp::Point>(123.45, -47.65);
    std::map<std::string, std::any> props {{"id",1},{"name", std::string{"Seattle"}}};
    const shp::Feature feature = {std::move(pt), props};
    shp.write(feature);
  }

  {
    shp::ShapefileReader reader { std::filesystem::absolute(path) };
    ASSERT_EQ(1, reader.getCount());
  }

}

TEST(ShapefileTests, shapefilewriter_create_lines_test) {

  std::filesystem::path path {"lines.shp"};

  {
    shp::Schema schema { shp::GeometryType::Line, std::vector<shp::Field> {
        shp::Field {"id", shp::FieldType::Integer, 3, 0},
        shp::Field {"name", shp::FieldType::String, 100, 0}
    }};
    shp::ShapefileWriter shp {std::filesystem::absolute(path), schema};

    std::unique_ptr<shp::Geometry> pt = std::make_unique<shp::Line>(std::vector{shp::Point(123.45, -47.65), shp::Point(124.12, -48.73)});
    std::map<std::string, std::any> props {{"id",1},{"name", std::string{"Road"}}};
    const shp::Feature feature = {std::move(pt), props};
    shp.write(feature);

  }

  {
    shp::ShapefileReader reader { std::filesystem::absolute(path) };
    ASSERT_EQ(1, reader.getCount());
  }

}

TEST(ShapefileTests, shapefilewriter_create_polygons_test) {

  std::filesystem::path path {"polygons.shp"};

  {
    shp::Schema schema { shp::GeometryType::Polygon, std::vector<shp::Field> {
        shp::Field {"id", shp::FieldType::Integer, 3, 0},
        shp::Field {"name", shp::FieldType::String, 100, 0}
    }};
    shp::ShapefileWriter shp {std::filesystem::absolute(path), schema};

    std::unique_ptr<shp::Geometry> polygon = std::make_unique<shp::Polygon>(
      shp::Ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}}
    );
    std::map<std::string, std::any> props {{"id",1},{"name", std::string{"Area 1"}}};
    const shp::Feature feature = {std::move(polygon), props};
    shp.write(feature);

    std::unique_ptr<shp::Geometry> polygonWithHoles = std::make_unique<shp::Polygon>(
      std::vector{
        shp::Ring {{{35,10}, {45, 45}, {15,40}, {10, 20}, {35,10}}},
        shp::Ring {{{20,30}, {35, 35}, {30,20}, {20, 30}}}
      }
    );
    std::map<std::string, std::any> propsWihHoles {{"id",2},{"name", std::string{"Area 2"}}};
    const shp::Feature featureWithHoles = {std::move(polygonWithHoles), propsWihHoles};
    shp.write(featureWithHoles);
  }

  {
    shp::ShapefileReader reader { std::filesystem::absolute(path) };
    ASSERT_EQ(2, reader.getCount());
  }

}

TEST(ShapefileTests, shapefilewriter_create_multipoints_test) {

  std::filesystem::path path {"multipoints.shp"};

  {
    shp::Schema schema { shp::GeometryType::MultiPoint, std::vector<shp::Field> {
        shp::Field {"id", shp::FieldType::Integer, 3, 0},
        shp::Field {"name", shp::FieldType::String, 100, 0}
    }};
    shp::ShapefileWriter shp {std::filesystem::absolute(path), schema};

    std::unique_ptr<shp::Geometry> geometry = std::make_unique<shp::MultiPoint>(
      std::vector<shp::Point>{{4,5},{1,2},{6,7}}
    );

    std::map<std::string, std::any> props {{"id",1},{"name", std::string{"Places"}}};
    const shp::Feature feature = {std::move(geometry), props};
    shp.write(feature);
  }

  {
    shp::ShapefileReader reader { std::filesystem::absolute(path) };
    ASSERT_EQ(1, reader.getCount());
  }
}

TEST(ShapefileTests, shapefilewriter_create_multipolygons_test) {

  std::filesystem::path path {"multipolygons.shp"};

  {
    shp::Schema schema { shp::GeometryType::Polygon, std::vector<shp::Field> {
        shp::Field {"id", shp::FieldType::Integer, 3, 0},
        shp::Field {"name", shp::FieldType::String, 100, 0}
    }};
    shp::ShapefileWriter shp {std::filesystem::absolute(path), schema};

    std::unique_ptr<shp::Geometry> geometry1 = std::make_unique<shp::MultiPolygon>(
      std::vector<shp::Polygon> {
        shp::Polygon {{shp::Ring {{{0,0}, {0, 10}, {10,10}, {10, 0}, {0,0}}} }},
        shp::Polygon {{shp::Ring {{{20,20}, {20, 30}, {30,30}, {30, 20}, {20,20}}} }}
      }
    );

    std::map<std::string, std::any> props1 {{"id",1},{"name", std::string{"Area 1"}}};
    const shp::Feature feature1 = {std::move(geometry1), props1};
    shp.write(feature1);

    std::unique_ptr<shp::Geometry> geometry2 = std::make_unique<shp::MultiPolygon>(
      std::vector<shp::Polygon> {
        shp::Polygon {{ shp::Ring {{{40,40}, {20, 45}, {45,30}, {40, 40}}} }},
        shp::Polygon {{
          shp::Ring {{ {20,35}, {10, 30}, {10,10}, {30, 5}, {45,20}, {20,35} }},
          shp::Ring {{ {30,20}, {20, 15}, {20,25}, {30, 20} }}
        }}
      }
    );

    std::map<std::string, std::any> props2 {{"id",2},{"name", std::string{"Area 2"}}};
    const shp::Feature feature2 = {std::move(geometry2), props2};
    shp.write(feature2);
  }

  {
    shp::ShapefileReader reader { std::filesystem::absolute(path) };
    ASSERT_EQ(2, reader.getCount());
  }

}
