add_executable(cli_tests
    BoundsCommandTest.cpp
    CountCommandTest.cpp
    InfoCommandTest.cpp
    ListCommandTest.cpp
    RandomCommandTest.cpp
)
target_link_libraries(cli_tests
    ShapefileCppAppLib
    shapelib::shapelib
    CLI11::CLI11
    GTest::GTest
)
add_test(NAME "ShapefileCliTests" COMMAND "cli_tests")
