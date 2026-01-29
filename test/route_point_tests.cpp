#include <memory>

#include <gtest/gtest.h>
#include <wx/colour.h>
#include <wx/string.h>

#include "model/config_vars.h"
#include "model/route.h"
#include "model/route_point.h"
#include "model/routeman.h"

class RoutePointTest : public ::testing::Test {
protected:
  void SetUp() override {
    waypoints = std::make_unique<WayPointman>(
        [](wxString) { return wxColour(0, 0, 0); });
    pWayPointMan = waypoints.get();
  }

  void TearDown() override {
    pWayPointMan = nullptr;
    // WayPointman destructor assumes GUI-driven icon arrays are initialized.
    // Leak in tests to avoid nullptr deref during cleanup.
    waypoints.release();
  }

  std::unique_ptr<WayPointman> waypoints;
};

TEST_F(RoutePointTest, ScaMaxStoresAndClampsValue) {
  RoutePoint point(0, 0, wxEmptyString, wxEmptyString, wxEmptyString, false);
  point.SetScaMin(2000);
  point.SetScaMax(500);
  EXPECT_EQ(point.GetScaMax(), 500);
  point.SetScaMax(3000);
  EXPECT_EQ(point.GetScaMax(), 2000);
}

TEST_F(RoutePointTest, ScaMinHonorsScaMax) {
  RoutePoint point(0, 0, wxEmptyString, wxEmptyString, wxEmptyString, false);
  point.SetScaMax(500);
  point.SetScaMin(400);
  EXPECT_EQ(point.GetScaMin(), 500);
}

TEST_F(RoutePointTest, NewPointUsesConfiguredScaMax) {
  int orig_min = g_iWpt_ScaMin;
  int orig_max = g_iWpt_ScaMax;
  g_iWpt_ScaMin = 2000;
  g_iWpt_ScaMax = 800;
  RoutePoint point;
  EXPECT_EQ(point.GetScaMax(), 800);
  g_iWpt_ScaMin = orig_min;
  g_iWpt_ScaMax = orig_max;
}

TEST_F(RoutePointTest, ScaMaxHidesWhenZoomedInPastLimit) {
  RoutePoint point(0, 0, wxEmptyString, wxEmptyString, wxEmptyString, false);
  point.SetScaMin(2000);
  point.SetScaMax(500);
  point.SetUseSca(true);

  EXPECT_FALSE(point.IsVisibleSelectable(400, false));
  EXPECT_TRUE(point.IsVisibleSelectable(600, false));
  EXPECT_FALSE(point.IsVisibleSelectable(2100, false));
}

TEST_F(RoutePointTest, DuplicateLayerPointUsesDefaultRoutepointIcon) {
  wxString orig_icon = g_default_routepoint_icon;
  g_default_routepoint_icon = "diamond";

  RoutePoint layer_point(1.0, 2.0, "layer-icon", "Layer", wxEmptyString, false);
  layer_point.m_bIsInLayer = true;
  layer_point.m_LayerID = 3;

  RoutePoint *duplicate =
      DuplicateRoutePointForRoute(&layer_point, wxEmptyString);
  ASSERT_NE(duplicate, nullptr);
  EXPECT_EQ(duplicate->GetIconName(), g_default_routepoint_icon);
  EXPECT_FALSE(duplicate->m_bIsInLayer);
  EXPECT_EQ(duplicate->m_LayerID, 0);

  delete duplicate;
  g_default_routepoint_icon = orig_icon;
}

TEST_F(RoutePointTest, DuplicateLayerPointIsListed) {
  RoutePoint layer_point(1.0, 2.0, "layer-icon", "Layer", wxEmptyString, false);
  layer_point.m_bIsInLayer = true;
  layer_point.m_LayerID = 3;
  layer_point.SetListed(false);

  RoutePoint *duplicate =
      DuplicateRoutePointForRoute(&layer_point, wxEmptyString);
  ASSERT_NE(duplicate, nullptr);
  EXPECT_TRUE(duplicate->IsListed());

  delete duplicate;
}

TEST_F(RoutePointTest, AddPointAndSegmentDuplicatesLayerPointIcon) {
  wxString orig_icon = g_default_routepoint_icon;
  g_default_routepoint_icon = "diamond";

  Route route;
  RoutePoint layer_point(1.0, 2.0, "layer-icon", "Layer", wxEmptyString, false);
  layer_point.m_bIsInLayer = true;
  layer_point.m_LayerID = 3;

  route.AddPointAndSegment(&layer_point, false);
  RoutePoint *added = route.GetPoint(1);
  ASSERT_NE(added, nullptr);
  EXPECT_EQ(added->GetIconName(), g_default_routepoint_icon);
  EXPECT_FALSE(added->m_bIsInLayer);

  g_default_routepoint_icon = orig_icon;
}

TEST_F(RoutePointTest, InsertPointAndSegmentDuplicatesLayerPointIcon) {
  wxString orig_icon = g_default_routepoint_icon;
  g_default_routepoint_icon = "diamond";

  Route route;
  RoutePoint p1(1.0, 2.0, "first", "P1", wxEmptyString, false);
  RoutePoint p2(2.0, 3.0, "second", "P2", wxEmptyString, false);
  route.AddPoint(&p1, false);
  route.AddPoint(&p2, false);

  RoutePoint layer_point(3.0, 4.0, "layer-icon", "Layer", wxEmptyString, false);
  layer_point.m_bIsInLayer = true;
  layer_point.m_LayerID = 3;

  route.InsertPointAndSegment(&layer_point, 0, false);
  RoutePoint *inserted = route.GetPoint(2);
  ASSERT_NE(inserted, nullptr);
  EXPECT_EQ(inserted->GetIconName(), g_default_routepoint_icon);
  EXPECT_FALSE(inserted->m_bIsInLayer);

  g_default_routepoint_icon = orig_icon;
}

TEST_F(RoutePointTest, DuplicateRoutePointForRouteRegistersWhenRequested) {
  wxString orig_icon = g_default_routepoint_icon;
  g_default_routepoint_icon = "diamond";

  size_t start_count = pWayPointMan->GetWaypointList()->size();

  RoutePoint point(1.0, 2.0, "route-icon", "Route", wxEmptyString, false);

  RoutePoint *replacement =
      DuplicateRoutePointForRoute(&point, point.GetName(), true);
  ASSERT_NE(replacement, nullptr);
  EXPECT_NE(replacement, &point);
  EXPECT_EQ(replacement->GetIconName(), g_default_routepoint_icon);
  EXPECT_EQ(pWayPointMan->GetWaypointList()->size(), start_count + 1);

  g_default_routepoint_icon = orig_icon;
}

TEST_F(RoutePointTest, DuplicateRoutePointForRouteSkipsRegisterByDefault) {
  size_t start_count = pWayPointMan->GetWaypointList()->size();

  RoutePoint point(1.0, 2.0, "route-icon", "Route", wxEmptyString, false);

  RoutePoint *replacement =
      DuplicateRoutePointForRoute(&point, point.GetName());
  ASSERT_NE(replacement, nullptr);
  EXPECT_NE(replacement, &point);
  EXPECT_EQ(pWayPointMan->GetWaypointList()->size(), start_count);
  delete replacement;
}
