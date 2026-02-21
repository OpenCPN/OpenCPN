#include <sstream>
#include <iostream>
#include <filesystem>
#include "CLI/CLI.hpp"
#include "Command.hpp"
#include "Commands.hpp"
#include "CountCommand.hpp"
#include "gtest/gtest.h"

TEST(ShapefileCliTests, count_test) {

  CLI::App app{"ShapefileCpp CLI"};

  Commands commands;
  CountCommand countCommand{&app};
  commands.add(&countCommand);

  int argc = 4;
  char const *argv[4] = {"shp-cli", "count", "-f", "../../../data/points.shp"};

  app.parse(argc, argv);

  std::istringstream instream;
  std::ostringstream outstream;

  countCommand.execute(instream, outstream);

  std::string result = outstream.str();
  ASSERT_EQ("10\n", result);
}
