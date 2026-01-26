#include <memory>

#include <gtest/gtest.h>
#include <wx/colour.h>
#include <wx/string.h>

#include "model/route.h"
#include "model/route_point.h"
#include "model/routeman.h"

class RoutePointScaleTest : public ::testing::Test {
protected:
  void SetUp() override {
    waypoints = std::make_unique<WayPointman>(
        [](wxString) { return wxColour(0, 0, 0); });
    pWayPointMan = waypoints.get();
  }

  void TearDown() override {
    pWayPointMan = nullptr;
    waypoints.reset();
  }

  std::unique_ptr<WayPointman> waypoints;
};

TEST_F(RoutePointScaleTest, ScaMaxStoresAndClampsValue) {
  RoutePoint point(0, 0, wxEmptyString, wxEmptyString, wxEmptyString, false);
  point.SetScaMin(2000);
  point.SetScaMax(500);
  EXPECT_EQ(point.GetScaMax(), 500);
  point.SetScaMax(3000);
  EXPECT_EQ(point.GetScaMax(), 2000);
}

TEST_F(RoutePointScaleTest, ScaMinHonorsScaMax) {
  RoutePoint point(0, 0, "", "", "", false);
  point.SetScaMax(500);
  point.SetScaMin(400);
  EXPECT_EQ(point.GetScaMin(), 800);  // Limited to  SCAMIN_MIN
}

TEST_F(RoutePointScaleTest, NewPointUsesConfiguredScaMax) {
  int orig_min = g_iWpt_ScaMin;
  int orig_max = g_iWpt_ScaMax;
  g_iWpt_ScaMin = 2000;
  g_iWpt_ScaMax = 800;
  RoutePoint point;
  EXPECT_EQ(point.GetScaMax(), 800);
  g_iWpt_ScaMin = orig_min;
  g_iWpt_ScaMax = orig_max;
}

TEST_F(RoutePointScaleTest, ScaMaxHidesWhenZoomedInPastLimit) {
  RoutePoint point(0, 0, wxEmptyString, wxEmptyString, wxEmptyString, false);
  point.SetScaMin(2000);
  point.SetScaMax(500);
  point.SetUseSca(true);

  EXPECT_FALSE(point.IsVisibleSelectable(400, false));
  EXPECT_TRUE(point.IsVisibleSelectable(600, false));
  EXPECT_FALSE(point.IsVisibleSelectable(2100, false));
}
