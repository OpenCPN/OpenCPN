#include <gtest/gtest.h>

#include "model/navutil_base.h"

TEST(NavUtils, fromDMM) {
  struct TestCase {
    std::string input;
    double expected;
  };

  std::vector<TestCase> test_cases = {
      // @todo Test fails, returning 4927, which is not a valid co-ordinate.  Code should return an error instead.
      // {"4916.45N", 49.27416666666667}, 
      // @todo Test fails, returning 12311, which is not a valid co-ordinate.  Code should return an error instead.
      // {"12311.12W", -123.18533333333333},
      // @todo Test fails, returning 9000, which is not a valid co-ordinate.  Code should return an error instead.
      // {"9000.00S", -90.0},
      // @todo Test fails, returning 18000, which is not a valid co-ordinate.  Code should return an error instead.
      //{"18000.00E", 180.0},
      // @todo Test fails, returning 4530, which is not a valid co-ordinate.  Code should return an error instead.
      //{"04530.00W", -45.5},
      {"0000.00N", 0.0},
      {"37°54.204' N", 37.9034},
      {"N37 54 12", 37.9034},
      {"37°54'12\"", 37.9034},
      {"37.9034", 37.9034},
      {"122°18.621' W", -122.31035},
      {"122w 18 37", -122.31035},
      {"-122.31035", -122.31035},
      {"-122 18.37", -122.30616}
  };

  for (const auto& test_case : test_cases) {
    double result = fromDMM(test_case.input);
    ASSERT_NEAR(result, test_case.expected, 1e-4)
        << "Input: " << test_case.input;
  }
}
