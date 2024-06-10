project(ShapefileCppLib)

set(ShapefileCppLib_SOURCES 
	src/ShapefileReader.cpp
	src/Schema.cpp
	src/FeatureIterator.cpp
	src/GeometryType.cpp
	src/Bounds.cpp
	src/Geometry.cpp
	src/Point.cpp
	src/Line.cpp
	src/Ring.cpp
	src/Polygon.cpp
	src/MultiPoint.cpp
	src/MultiLine.cpp
	src/MultiPolygon.cpp
	src/Feature.cpp
	src/FieldType.cpp
	src/Field.cpp
)

add_library(ShapefileCppLib STATIC ${ShapefileCppLib_SOURCES})

target_include_directories(ShapefileCppLib
	PUBLIC include shapelib::shapelib
	PRIVATE src
)
target_link_libraries(ShapefileCppLib shapelib::shapelib)
