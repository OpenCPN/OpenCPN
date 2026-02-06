#include <memory>

#include <gtest/gtest.h>
#include <wx/colour.h>
#include <wx/string.h>

#include "model/config_vars.h"
#include "model/route.h"
#include "model/route_point.h"
#include "model/routeman.h"

class RoutePointLayerTest : public ::testing::Test {
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

TEST_F(RoutePointLayerTest, DuplicateLayerPointUsesDefaultRoutepointIcon) {
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

TEST_F(RoutePointLayerTest, DuplicateLayerPointIsListed) {
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

TEST_F(RoutePointLayerTest, AddPointAndSegmentDuplicatesLayerPointIcon) {
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

TEST_F(RoutePointLayerTest, InsertPointAndSegmentDuplicatesLayerPointIcon) {
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

TEST_F(RoutePointLayerTest, DuplicateRoutePointForRouteRegistersWhenRequested) {
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

TEST_F(RoutePointLayerTest, DuplicateRoutePointForRouteSkipsRegisterByDefault) {
  size_t start_count = pWayPointMan->GetWaypointList()->size();

  RoutePoint point(1.0, 2.0, "route-icon", "Route", wxEmptyString, false);

  RoutePoint *replacement =
      DuplicateRoutePointForRoute(&point, point.GetName());
  ASSERT_NE(replacement, nullptr);
  EXPECT_NE(replacement, &point);
  EXPECT_EQ(pWayPointMan->GetWaypointList()->size(), start_count);
  delete replacement;
}
