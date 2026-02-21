#include <sstream>
#include <iostream>
#include <filesystem>
#include "CLI/CLI.hpp"
#include "Command.hpp"
#include "Commands.hpp"
#include "InfoCommand.hpp"
#include "gtest/gtest.h"

TEST(ShapefileCliTests, info_test) {

  CLI::App app{"ShapefileCpp CLI"};

  Commands commands;
  InfoCommand infoCommand{&app};
  commands.add(&infoCommand);

  int argc = 4;
  char const *argv[4] = {"shp-cli", "info", "-f", "../../../data/points.shp"};

  app.parse(argc, argv);

  std::istringstream instream;
  std::ostringstream outstream;

  infoCommand.execute(instream, outstream);

  std::string result = outstream.str();
  ASSERT_NE(std::string::npos, result.find("# Features = "));
  ASSERT_NE(std::string::npos, result.find("Bounds = "));
  ASSERT_NE(std::string::npos, result.find("Geometry Type = "));
  ASSERT_NE(std::string::npos, result.find("Schema:"));
}
