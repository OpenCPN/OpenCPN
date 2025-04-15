#include <sstream>
#include <iostream>
#include <filesystem>
#include "CLI/CLI.hpp"
#include "Command.hpp"
#include "Commands.hpp"
#include "RandomCommand.hpp"
#include "ShapefileReader.hpp"
#include "gtest/gtest.h"

TEST(ShapefileCliTests, random_test) {

  CLI::App app{"ShapefileCpp CLI"};

  Commands commands;
  RandomCommand randomCommand{&app};
  commands.add(&randomCommand);

  int argc = 6;
  char const *argv[6] = {"shp-cli", "random", "-f", "random.shp", "-n", "10"};

  app.parse(argc, argv);

  std::istringstream instream;
  std::ostringstream outstream;

  randomCommand.execute(instream, outstream);

  shp::ShapefileReader shp {"random.shp"};
  ASSERT_EQ(10, shp.getCount());

}
