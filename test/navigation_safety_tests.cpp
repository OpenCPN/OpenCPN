#include <gtest/gtest.h>
#include <limits>
#include "navigation_safety.h"

namespace {
using Api = HostApi123;
using ocpn::chart_safety::CheckNavigationRoute;
using ocpn::chart_safety::MakeNavigationOptions;
const std::vector<Api::NavigationSafetyPosition> kRoute{
    {28.0, -16.0}, {27.5, -17.0}, {27.0, -18.0}};

TEST(NavigationSafety, SegmentInputRejectsNonFiniteAndNegativeOptions) {
  using ocpn::chart_safety::ValidSegmentInput;
  Api::SegmentSafetyOptions options{};
  options.struct_size = sizeof(options);
  EXPECT_TRUE(ValidSegmentInput(10, 179, 10, 181, &options));
  EXPECT_FALSE(ValidSegmentInput(91, 0, 10, 0, &options));
  EXPECT_FALSE(ValidSegmentInput(10, std::numeric_limits<double>::infinity(),
                                 10, 0, &options));
  options.safety_margin_nm = -1;
  EXPECT_FALSE(ValidSegmentInput(10, 0, 10, 0, &options));
  options.safety_margin_nm = 0;
  options.minimum_depth_m = std::numeric_limits<double>::quiet_NaN();
  EXPECT_FALSE(ValidSegmentInput(10, 0, 10, 0, &options));
  options.struct_size = sizeof(int);
  EXPECT_TRUE(ValidSegmentInput(10, 0, 10, 0, &options));
}

TEST(NavigationSafety, UsesDraftAndClearanceWithExplicitMinimum) {
  Api::NavigationSafetyProfile profile;
  profile.check_depth = true;
  profile.vessel_draft_m = 2.0;
  profile.under_keel_clearance_m = 0.8;
  profile.land_margin_nm = 0.25;
  Api::SegmentSafetyOptions options{};
  ASSERT_TRUE(MakeNavigationOptions(profile, &options));
  EXPECT_DOUBLE_EQ(options.minimum_depth_m, 2.8);
  EXPECT_DOUBLE_EQ(options.safety_margin_nm, 0.25);
  EXPECT_EQ(options.force_authoritative_fine_validation, 1);
  profile.minimum_charted_depth_m = 5.0;
  ASSERT_TRUE(MakeNavigationOptions(profile, &options));
  EXPECT_DOUBLE_EQ(options.minimum_depth_m, 5.0);
}

TEST(NavigationSafety, RejectsInvalidAllowancesAndOverflow) {
  for (double value : {-1.0, std::numeric_limits<double>::infinity(),
                       std::numeric_limits<double>::quiet_NaN()}) {
    Api::NavigationSafetyProfile profile;
    profile.land_margin_nm = value;
    Api::SegmentSafetyOptions options{};
    EXPECT_FALSE(MakeNavigationOptions(profile, &options));
    profile.land_margin_nm = 0;
    profile.under_keel_clearance_m = value;
    EXPECT_FALSE(MakeNavigationOptions(profile, &options));
  }
  Api::NavigationSafetyProfile profile;
  profile.vessel_draft_m = std::numeric_limits<double>::max();
  profile.under_keel_clearance_m = std::numeric_limits<double>::max();
  Api::SegmentSafetyOptions options{};
  EXPECT_FALSE(MakeNavigationOptions(profile, &options));
  profile = {};
  profile.check_land = false;
  EXPECT_FALSE(MakeNavigationOptions(profile, &options));
}

TEST(NavigationSafety, ChecksFinalLegAndReportsUnsafeGeometry) {
  int calls = 0;
  Api::NavigationRouteSafetyResult result;
  auto query = [&](double lat1, double lon1, double lat2, double lon2,
                   const Api::SegmentSafetyOptions* options,
                   Api::SegmentSafetyResult* segment) {
    EXPECT_DOUBLE_EQ(lat1, kRoute[calls].latitude);
    EXPECT_DOUBLE_EQ(lon1, kRoute[calls].longitude);
    EXPECT_DOUBLE_EQ(lat2, kRoute[calls + 1].latitude);
    EXPECT_DOUBLE_EQ(lon2, kRoute[calls + 1].longitude);
    EXPECT_EQ(options->force_authoritative_fine_validation, 1);
    segment->status =
        calls++ == 0 ? Api::kSegmentSafetySafe : Api::kSegmentSafetyCrossesLand;
    return true;
  };
  ASSERT_TRUE(CheckNavigationRoute({}, kRoute, query, &result));
  EXPECT_EQ(calls, 2);
  ASSERT_EQ(result.segments.size(), 2u);
  EXPECT_TRUE(result.valid_input);
  EXPECT_TRUE(result.complete);
  EXPECT_FALSE(result.safe);
  EXPECT_EQ(result.segments.back().status, Api::kSegmentSafetyCrossesLand);
}

TEST(NavigationSafety, IncompleteFirstLegDoesNotHideLaterHazard) {
  for (int status :
       {Api::kSegmentSafetyNoData, Api::kSegmentSafetyUnknownDepth,
        Api::kSegmentSafetyPendingData, Api::kSegmentSafetyError}) {
    int calls = 0;
    Api::NavigationRouteSafetyResult result;
    auto query = [&](double, double, double, double,
                     const Api::SegmentSafetyOptions*,
                     Api::SegmentSafetyResult* segment) {
      segment->status = calls++ == 0 ? status : Api::kSegmentSafetyTooShallow;
      return true;
    };
    ASSERT_TRUE(CheckNavigationRoute({}, kRoute, query, &result));
    EXPECT_FALSE(result.complete);
    EXPECT_FALSE(result.safe);
    ASSERT_EQ(result.segments.size(), 2u);
    EXPECT_EQ(result.segments.back().status, Api::kSegmentSafetyTooShallow);
  }
}

TEST(NavigationSafety, FailedOrUnrecognizedQueriesCannotReportSafe) {
  for (bool succeeds : {true, false}) {
    Api::NavigationRouteSafetyResult result;
    auto query = [&](double, double, double, double,
                     const Api::SegmentSafetyOptions*,
                     Api::SegmentSafetyResult* segment) {
      segment->status = succeeds ? 999 : Api::kSegmentSafetySafe;
      return succeeds;
    };
    ASSERT_TRUE(CheckNavigationRoute({}, kRoute, query, &result));
    EXPECT_FALSE(result.complete);
    EXPECT_FALSE(result.safe);
    EXPECT_EQ(result.segments[0].status, Api::kSegmentSafetyError);
  }
}

TEST(NavigationSafety, RejectsWholeInvalidRouteBeforeAnyChartQuery) {
  int calls = 0;
  auto query = [&](double, double, double, double,
                   const Api::SegmentSafetyOptions*,
                   Api::SegmentSafetyResult*) {
    ++calls;
    return true;
  };
  Api::NavigationRouteSafetyResult result;
  result.safe = true;
  EXPECT_FALSE(CheckNavigationRoute({}, {}, query, &result));
  EXPECT_FALSE(result.safe);
  EXPECT_FALSE(CheckNavigationRoute({}, {kRoute[0]}, query, &result));
  auto invalid = kRoute;
  invalid.back().latitude = 91;
  EXPECT_FALSE(CheckNavigationRoute({}, invalid, query, &result));
  invalid = kRoute;
  invalid.back().longitude = std::numeric_limits<double>::quiet_NaN();
  EXPECT_FALSE(CheckNavigationRoute({}, invalid, query, &result));
  EXPECT_EQ(calls, 0);
}

TEST(NavigationSafety, RecheckingDeferredRouteCanComplete) {
  bool ready = false;
  auto query = [&](double, double, double, double,
                   const Api::SegmentSafetyOptions*,
                   Api::SegmentSafetyResult* segment) {
    segment->status =
        ready ? Api::kSegmentSafetySafe : Api::kSegmentSafetyPendingData;
    return true;
  };
  Api::NavigationRouteSafetyResult result;
  ASSERT_TRUE(CheckNavigationRoute({}, kRoute, query, &result));
  EXPECT_FALSE(result.safe);
  ready = true;
  ASSERT_TRUE(CheckNavigationRoute({}, kRoute, query, &result));
  EXPECT_TRUE(result.complete);
  EXPECT_TRUE(result.safe);
  EXPECT_EQ(result.segments.size(), 2u);
}

TEST(NavigationSafety, AcceptsDatelineLegWithoutChangingCoordinates) {
  auto query = [](double, double lon1, double, double lon2,
                  const Api::SegmentSafetyOptions*,
                  Api::SegmentSafetyResult* segment) {
    EXPECT_DOUBLE_EQ(lon1, 179.5);
    EXPECT_DOUBLE_EQ(lon2, -179.5);
    segment->status = Api::kSegmentSafetySafe;
    return true;
  };
  Api::NavigationRouteSafetyResult result;
  EXPECT_TRUE(
      CheckNavigationRoute({}, {{10, 179.5}, {10, -179.5}}, query, &result));
  EXPECT_TRUE(result.safe);
}
}  // namespace
