    #include <sstream>
#include <iostream>
#include <filesystem>
#include "CLI/CLI.hpp"
#include "Command.hpp"
#include "Commands.hpp"
#include "BoundsCommand.hpp"
#include "gtest/gtest.h"

TEST(ShapefileCliTests, bounds_test) {

  CLI::App app{"ShapefileCpp CLI"};

  Commands commands;
  BoundsCommand boundsCommand{&app};
  commands.add(&boundsCommand);

  int argc = 4;
  char const *argv[4] = {"shp-cli", "bounds", "-f", "../../../data/points.shp"};

  app.parse(argc, argv);

  std::istringstream instream;
  std::ostringstream outstream;

  boundsCommand.execute(instream, outstream);

  std::string result = outstream.str();
  ASSERT_EQ("POLYGON ((-158.899 -82.7908, 171.62 -82.7908, 171.62 79.2818, -158.899 -82.7908))\n", result);
}
