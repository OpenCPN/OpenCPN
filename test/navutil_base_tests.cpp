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
      {"122w 18 37", -122.31027},
      {"-122.31035", -122.31035},
      {"-122 18.37", -122.30616},
      {"-122 18 37", -122.31027},
  };

  for (const auto& test_case : test_cases) {
    double result = fromDMM(test_case.input);
    ASSERT_NEAR(result, test_case.expected, 1e-4)
        << "Input: " << test_case.input;
  }
}

TEST(NavUtils, toSDMM) {
  struct Params {
    int NEflag;
    double degrees;
    bool hi_precision;
  };

  struct TestCase {
    Params input;
    std::string expected;
  };

  std::vector<TestCase> test_cases = {
      { {1, 0.0, true}, "00\xB0 00.0000' N"},
      { {1, 37.9034, true}, "37\xB0 54.2040' N"},
      { {1, -122.31035, true}, "122\xB0 18.6210' S"},
      { {2, 0.0, true}, "000\xB0 00.0000' E"},
      { {2, 37.9034, true}, "037\xB0 54.2040' E"},
      { {2, -122.31035, true}, "122\xB0 18.6210' W"},
      { {0, 0.0, true}, "0\xB0 00.0000'"},
      { {0, 37.9034, true}, "37\xB0 54.2040'"},
      { {0, -42.42, true}, "-42\xB0 25.2000'"}, // @todo Fix code to produce correct output, see https://github.com/OpenCPN/OpenCPN/pull/4851 - today it produces a garbage value of "-42° 5065.2000'"
      { {0, -122.31035, true}, "-122\xB0 18.6210'"}, // @todo Fix code to produce correct output, see https://github.com/OpenCPN/OpenCPN/pull/4851 - today it produces a garbage value of "-122\xB0 14658.6210'"
      { {1, 0.0, false}, "00\xB0 00.0' N"},
      { {1, 37.9034, false}, "37\xB0 54.2' N"},
      { {1, -122.31035, false}, "122\xB0 18.6' S"},
      { {2, 0.0, false}, "000\xB0 00.0' E"},
      { {2, 37.9034, false}, "037\xB0 54.2' E"},
      { {2, -122.31035, false}, "122\xB0 18.6' W"},
  };

  for (const auto& test_case : test_cases) {
    std::string result = toSDMM(test_case.input.NEflag, test_case.input.degrees, test_case.input.hi_precision).ToStdString();
    EXPECT_EQ(result, test_case.expected)
        << "Input: " << "NEflag = " << test_case.input.NEflag
        << ", degrees = " << test_case.input.degrees
        << ", hi_precision = " << test_case.input.hi_precision;
  }
}

TEST(NavUtils, ParseGPXDateTime) {
  struct TestCase {
    std::wstring dateString;
    struct tm expected;
    wxDateTime::TimeZone tz;
    int charsConsumed=-1; // -1 means we all chars should be consumed
  };
  std::vector<TestCase> test_cases = {
      {L"2010-10-30T14:34:56Z", {56,34,14,30,10,110,}, wxDateTime::UTC},
      {L"2010-10-30T14:34:56-04:00", {56,34,14,30,10,110,}, wxDateTime::GMT_4},
      {L"2010-12-01T23:59:59+02:00", {59,59,23,01,12,110,}, wxDateTime::GMT2},
  };
  for (const auto& test_case : test_cases) {
    wxDateTime dateTime;
    auto result = ParseGPXDateTime(dateTime, wxString(test_case.dateString));
    std::stringstream msg;
    msg << "Input: " << "dateString = " << test_case.dateString
        << ", expected.tz = " << test_case.tz.GetOffset()/3600;
    EXPECT_TRUE(result != NULL) << msg.str(); 
    if (result) {
      dateTime.MakeFromTimezone(wxDateTime::UTC);
      EXPECT_EQ(dateTime.GetYear(test_case.tz), test_case.expected.tm_year+1900)
        << msg.str();
      EXPECT_EQ(dateTime.GetMonth(test_case.tz), test_case.expected.tm_mon-1)
        << msg.str();
      EXPECT_EQ(dateTime.GetDay(test_case.tz), test_case.expected.tm_mday)
        << msg.str();
      EXPECT_EQ(dateTime.GetHour(test_case.tz), test_case.expected.tm_hour)
        << msg.str();
      EXPECT_EQ(dateTime.GetMinute(test_case.tz), test_case.expected.tm_min)
        << msg.str();
      EXPECT_EQ(dateTime.GetSecond(test_case.tz), test_case.expected.tm_sec)
        << msg.str();
    }
  }
}
