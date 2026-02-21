project(ShapefileCppApp)

add_library(ShapefileCppAppLib STATIC
	src/Command.cpp
	src/Commands.cpp
	src/BoundsCommand.cpp
	src/CountCommand.cpp
	src/InfoCommand.cpp
	src/ListCommand.cpp
	src/RandomCommand.cpp
)
target_include_directories(ShapefileCppAppLib PUBLIC "include" "../lib/include" CLI11::CLI11)
target_compile_features(ShapefileCppAppLib PUBLIC cxx_std_17)
target_link_libraries(ShapefileCppAppLib PRIVATE ShapefileCppLib shapelib::shapelib CLI11::CLI11)

# Application
add_executable(shp-cli src/App.cpp)
target_link_libraries(shp-cli PRIVATE ShapefileCppAppLib ShapefileCppLib shapelib::shapelib CLI11::CLI11)

add_subdirectory(test)
