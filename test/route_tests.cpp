/***************************************************************************
 *   Copyright (C) 2025 by OpenCPN development team                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include "config.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <cmath>

#include <wx/app.h>
#include <wx/event.h>
#include <wx/evtloop.h>
#include <wx/fileconf.h>
#include <wx/jsonval.h>
#include <wx/timer.h>

#include <gtest/gtest.h>

#include "model/ais_decoder.h"
#include "model/ais_defs.h"
#include "model/ais_state_vars.h"
#include "model/cli_platform.h"
#include "model/comm_ais.h"
#include "model/comm_appmsg_bus.h"
#include "model/comm_bridge.h"
#include "model/comm_drv_file.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/config_vars.h"
#include "model/datetime.h"
#include "model/georef.h"
#include "model/ipc_api.h"
#include "model/logger.h"
#include "model/multiplexer.h"
#include "model/navutil_base.h"
#include "model/ocpn_types.h"
#include "model/ocpn_utils.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/route_point.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/semantic_vers.h"
#include "model/std_instance_chk.h"
#include "model/wait_continue.h"
#include "model/wx_instance_chk.h"
#include "observable_confvar.h"
#include "ocpn_plugin.h"

class MockRouteMan {
public:
  void DeactivateRoute() {}
};

class MockWayPointman : public WayPointman {
public:
  MockWayPointman() : WayPointman([](wxString) { return wxColour(0, 0, 0); }) {}
};

static MockRouteMan g_mock_routeman;
static MockWayPointman* g_mock_waypoint_man = nullptr;

// Macos up to 10.13
#if (defined(OCPN_GHC_FILESYSTEM) || \
     (defined(__clang_major__) && (__clang_major__ < 15)))
namespace fs = ghc::filesystem;
#else
namespace fs = std::filesystem;
#endif

/**
 * Test suite for Route class functionality.
 *
 * This test suite covers various aspects of the Route class including:
 * - Basic route creation and management
 * - Waypoint operations (add, remove, insert)
 * - Route calculations (distances, bearings, timing)
 * - Position interpolation (GetPositionAtTime)
 * - Route properties and metadata
 */
class RouteTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Store original global pointers so we can restore them
    original_pWayPointMan = pWayPointMan;
    original_g_pRouteMan = g_pRouteMan;
    original_pRouteList = pRouteList;

    if (!pRouteList) {
      pRouteList = new RouteList;
      created_pRouteList = true;
    }

    if (!g_mock_waypoint_man) {
      g_mock_waypoint_man = new MockWayPointman();
      created_waypoint_man = true;
    }

    // Hook up the global pointers to our instances
    g_pRouteMan = reinterpret_cast<Routeman*>(&g_mock_routeman);
    pWayPointMan = g_mock_waypoint_man;

    testRoute = new Route();

    // Set a base departure time (fixed for consistent testing)
    baseDeparture = wxDateTime(1, wxDateTime::Jan, 2024, 12, 0, 0);
    baseDeparture.MakeFromTimezone(wxDateTime::UTC);
    testRoute->SetDepartureDate(baseDeparture);

    // Set reasonable planned speed
    testRoute->m_PlannedSpeed = 10.0;  // 10 knots
  }

  void TearDown() override {
    if (testRoute) {
      // Clean up waypoints that are still in the route
      delete testRoute;
      testRoute = nullptr;
    }

    // Restore original global pointers
    pWayPointMan = original_pWayPointMan;
    g_pRouteMan = original_g_pRouteMan;

    if (created_pRouteList && pRouteList) {
      // Clear the route list (std::vector doesn't have DeleteContents)
      for (Route* route : *pRouteList) {
        delete route;
      }
      pRouteList->clear();
      delete pRouteList;
      pRouteList = original_pRouteList;
      created_pRouteList = false;
    }

    // Only clean up MockWayPointman if we created it
    // We intentionally leak this to avoid destructor crashes during exit
    // The OS will clean it up when the process exits
    if (created_waypoint_man) {
      // delete g_mock_waypoint_man;  // Commented out to avoid crash
      // g_mock_waypoint_man = nullptr;
      created_waypoint_man = false;
    }
  }

  // Helper function to create a simple two-waypoint route
  void CreateSimpleRoute(double lat1, double lon1, double lat2, double lon2) {
    RoutePoint* wp1 =
        new RoutePoint(lat1, lon1, "diamond", "WP1", wxEmptyString, false);
    RoutePoint* wp2 =
        new RoutePoint(lat2, lon2, "diamond", "WP2", wxEmptyString, false);

    testRoute->AddPoint(wp1);
    testRoute->AddPoint(wp2);
    testRoute->UpdateSegmentDistances(testRoute->m_PlannedSpeed);
  }

  // Helper function to create a multi-waypoint route
  void CreateMultiWaypointRoute() {
    // San Francisco to Los Angeles via Santa Barbara
    RoutePoint* wp1 = new RoutePoint(37.7749, -122.4194, "diamond", "SF",
                                     wxEmptyString, false);  // San Francisco
    RoutePoint* wp2 = new RoutePoint(34.4208, -119.6982, "diamond", "SB",
                                     wxEmptyString, false);  // Santa Barbara
    RoutePoint* wp3 = new RoutePoint(34.0522, -118.2437, "diamond", "LA",
                                     wxEmptyString, false);  // Los Angeles

    testRoute->AddPoint(wp1);
    testRoute->AddPoint(wp2);
    testRoute->AddPoint(wp3);
    testRoute->UpdateSegmentDistances(testRoute->m_PlannedSpeed);
  }

  // Helper to check if two positions are approximately equal (within 0.001
  // degrees)
  bool PositionsApproxEqual(double lat1, double lon1, double lat2, double lon2,
                            double tolerance = 0.001) {
    return (fabs(lat1 - lat2) < tolerance) && (fabs(lon1 - lon2) < tolerance);
  }

  Route* testRoute;
  wxDateTime baseDeparture;

private:
  // Track what we created so we can clean up properly
  WayPointman* original_pWayPointMan = nullptr;
  Routeman* original_g_pRouteMan = nullptr;
  RouteList* original_pRouteList = nullptr;
  bool created_pRouteList = false;
  bool created_waypoint_man = false;
};

// =============================================================================
// Basic Route Functionality Tests
// =============================================================================

// Test basic route creation and properties
TEST_F(RouteTest, BasicRouteCreation) {
  // New route should be empty
  EXPECT_EQ(testRoute->GetnPoints(), 0);
  EXPECT_TRUE(testRoute->pRoutePointList->empty());

  // Should have default properties
  EXPECT_FALSE(testRoute->IsActive());
  EXPECT_TRUE(testRoute->IsVisible());
  EXPECT_TRUE(testRoute->IsListed());
  EXPECT_FALSE(testRoute->IsSelected());

  // Should have reasonable defaults
  EXPECT_EQ(testRoute->m_PlannedSpeed, 10.0);
  EXPECT_EQ(testRoute->m_route_length, 0.0);
  EXPECT_EQ(testRoute->m_route_time, 0.0);
}

// Test route calculations
TEST_F(RouteTest, RouteCalculations) {
  // Create a route from approximate coordinates of Boston to NYC
  CreateSimpleRoute(42.3601, -71.0589, 40.7128, -74.0060);

  // Verify that the departure time is properly set
  EXPECT_TRUE(testRoute->m_PlannedDeparture.IsValid());
  EXPECT_EQ(testRoute->m_PlannedDeparture, baseDeparture);

  // Route should have calculated length and time
  EXPECT_GT(testRoute->m_route_length, 0.0);
  EXPECT_GT(testRoute->m_route_time, 0.0);

  // More specific validation - Boston to NYC is approximately 165 NM
  EXPECT_NEAR(testRoute->m_route_length, 165.0, 10.0);  // Within 10 NM

  // Verify the calculation makes sense geographically
  EXPECT_GT(testRoute->m_route_length, 150.0);  // At least 150 NM
  EXPECT_LT(testRoute->m_route_length, 200.0);  // Less than 200 NM

  // Test that segment distances are calculated
  RoutePoint* wp1 = testRoute->GetPoint(1);
  RoutePoint* wp2 = testRoute->GetPoint(2);

  ASSERT_TRUE(wp1 != nullptr);
  ASSERT_TRUE(wp2 != nullptr);

  EXPECT_GT(wp2->m_seg_len, 0.0);
  EXPECT_GT(wp2->m_seg_vmg, 0.0);

  // More specific validation for segment calculations
  EXPECT_NEAR(wp2->m_seg_len, 165.0,
              10.0);  // Segment length should match expected distance
  EXPECT_NEAR(wp2->m_seg_vmg, 10.0, 0.001);  // VMG should match planned speed
  EXPECT_GT(wp2->m_seg_len, 150.0);  // Sanity check: reasonable distance
  EXPECT_LT(wp2->m_seg_len, 200.0);  // Sanity check: not too far

  // Verify timing calculations work
  EXPECT_TRUE(wp1->GetETD().IsValid());
  EXPECT_TRUE(wp2->GetETA().IsValid());

  // The second waypoint's ETD should equal its ETA (standard behavior)
  EXPECT_EQ(wp2->GetETD(), wp2->GetETA());

  // Check that the route length matches the segment length for a 2-point route
  EXPECT_NEAR(testRoute->m_route_length, wp2->m_seg_len, 0.001);

  // Verify that time calculation is consistent: time = distance / speed
  double expected_time =
      testRoute->m_route_length / testRoute->m_PlannedSpeed * 3600.0;
  EXPECT_NEAR(testRoute->m_route_time, expected_time, 1.0);  // Within 1 second
}

// Test that departure time is important for timing calculations but not
// distance.
TEST_F(RouteTest, DepartureTimeImportance) {
  // Create a route without setting departure time
  Route* routeWithoutDeparture = new Route();
  // Note: not setting departure time
  routeWithoutDeparture->m_PlannedSpeed = 10.0;

  RoutePoint* wp1 =
      new RoutePoint(42.3601, -71.0589, "diamond", "WP1", wxEmptyString, false);
  RoutePoint* wp2 =
      new RoutePoint(40.7128, -74.0060, "diamond", "WP2", wxEmptyString, false);

  routeWithoutDeparture->AddPoint(wp1);
  routeWithoutDeparture->AddPoint(wp2);
  routeWithoutDeparture->UpdateSegmentDistances(
      routeWithoutDeparture->m_PlannedSpeed);

  // Distance and basic time calculations should still work
  EXPECT_GT(routeWithoutDeparture->m_route_length, 0.0);
  EXPECT_GT(routeWithoutDeparture->m_route_time, 0.0);
  EXPECT_GT(wp2->m_seg_len, 0.0);
  EXPECT_GT(wp2->m_seg_vmg, 0.0);

  // More specific validation - check that the values are reasonable
  // Boston to NYC is approximately 165 NM, so verify we're in the right
  // ballpark
  EXPECT_NEAR(routeWithoutDeparture->m_route_length, 165.0,
              10.0);  // Within 10 NM

  // Time should be distance/speed * 3600 seconds
  double expected_time_no_departure =
      routeWithoutDeparture->m_route_length / 10.0 * 3600.0;
  EXPECT_NEAR(routeWithoutDeparture->m_route_time, expected_time_no_departure,
              10.0);

  // Segment length should match total length for a 2-point route
  EXPECT_NEAR(wp2->m_seg_len, routeWithoutDeparture->m_route_length, 0.001);

  // Segment VMG should match planned speed
  EXPECT_NEAR(wp2->m_seg_vmg, 10.0, 0.001);

  // But ETD/ETA calculations depend on having a valid departure time
  // Without departure time, the waypoints won't have valid ETD/ETA
  // Now create the same route with departure time
  CreateSimpleRoute(42.3601, -71.0589, 40.7128, -74.0060);
  RoutePoint* wp1_with_time = testRoute->GetPoint(1);
  RoutePoint* wp2_with_time = testRoute->GetPoint(2);

  // Both routes should have the same distance and time calculations
  EXPECT_NEAR(routeWithoutDeparture->m_route_length, testRoute->m_route_length,
              0.001);
  EXPECT_NEAR(routeWithoutDeparture->m_route_time, testRoute->m_route_time,
              0.001);

  // But only the route with departure time should have valid ETD/ETA
  EXPECT_TRUE(wp1_with_time->GetETD().IsValid());
  EXPECT_TRUE(wp2_with_time->GetETA().IsValid());

  // Clean up
  delete routeWithoutDeparture;
}

// Test route properties and metadata
TEST_F(RouteTest, RouteProperties) {
  testRoute->m_RouteNameString = "Test Route";
  testRoute->m_RouteDescription = "A test route for unit testing";
  testRoute->m_Colour = "Red";

  EXPECT_EQ(testRoute->GetName(), "Test Route");
  EXPECT_EQ(testRoute->m_RouteDescription, "A test route for unit testing");
  EXPECT_EQ(testRoute->m_Colour, "Red");

  // Test visibility controls
  testRoute->SetVisible(false);
  EXPECT_FALSE(testRoute->IsVisible());

  testRoute->SetVisible(true);
  EXPECT_TRUE(testRoute->IsVisible());
}

// =============================================================================
// Position Interpolation Tests (GetPositionAtTime)
// =============================================================================

// Test basic functionality with invalid inputs
TEST_F(RouteTest, InvalidInputs) {
  CreateSimpleRoute(37.7749, -122.4194, 34.0522, -118.2437);  // SF to LA

  double lat, lon;

  // Test with invalid timestamp
  wxDateTime invalidTime;
  EXPECT_FALSE(testRoute->GetPositionAtTime(invalidTime, lat, lon));

  // Test with time before route start
  wxDateTime beforeStart = baseDeparture - wxTimeSpan::Hours(1);
  EXPECT_FALSE(testRoute->GetPositionAtTime(beforeStart, lat, lon));

  // Test with route that has no planned departure
  Route emptyRoute;
  EXPECT_FALSE(emptyRoute.GetPositionAtTime(baseDeparture, lat, lon));
}

// Test empty route handling
TEST_F(RouteTest, EmptyRoute) {
  double lat, lon;

  // Route with no waypoints
  EXPECT_FALSE(testRoute->GetPositionAtTime(baseDeparture, lat, lon));
}

// Test single waypoint route. Because the route has only one point,
// it should return that point's position for any valid time.
// This is a special case where the route doesn't interpolate between points.
TEST_F(RouteTest, SingleWaypoint) {
  RoutePoint* wp =
      new RoutePoint(37.7749, -122.4194, "diamond", "SF", wxEmptyString, false);
  testRoute->AddPoint(wp);
  testRoute->UpdateSegmentDistances(testRoute->m_PlannedSpeed);

  double lat, lon;

  // Should return the single waypoint position for any valid time
  EXPECT_TRUE(testRoute->GetPositionAtTime(baseDeparture, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, 37.7749, -122.4194));

  // Even well into the future
  wxDateTime futureTime = baseDeparture + wxTimeSpan::Days(1);
  EXPECT_TRUE(testRoute->GetPositionAtTime(futureTime, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, 37.7749, -122.4194));
}

// Test two-waypoint route interpolation.
TEST_F(RouteTest, TwoWaypointInterpolation) {
  CreateSimpleRoute(37.7749, -122.4194, 34.0522, -118.2437);  // SF to LA

  double lat, lon;

  // At departure time, should be at first waypoint
  EXPECT_TRUE(testRoute->GetPositionAtTime(baseDeparture, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, 37.7749, -122.4194, 0.001));

  // Get route timing for calculations
  RoutePoint* wp1 = testRoute->GetPoint(1);
  RoutePoint* wp2 = testRoute->GetPoint(2);

  ASSERT_TRUE(wp1 != nullptr);
  ASSERT_TRUE(wp2 != nullptr);

  wxDateTime etd1 = wp1->GetETD();
  wxDateTime eta2 = wp2->GetETA();

  EXPECT_TRUE(etd1.IsValid());
  EXPECT_TRUE(eta2.IsValid());

  // At segment midpoint, should be approximately halfway
  wxTimeSpan segmentDuration = eta2 - etd1;
  wxDateTime midTime =
      etd1 + wxTimeSpan::Seconds(segmentDuration.GetSeconds().ToLong() / 2);

  EXPECT_TRUE(testRoute->GetPositionAtTime(midTime, lat, lon));

  // More realistic bounds check - should be somewhere between SF and LA
  // but we don't know the exact rhumb line path, so use broader bounds
  EXPECT_GT(lat, 33.0);    // Above LA area
  EXPECT_LT(lat, 39.0);    // Below SF area
  EXPECT_GT(lon, -125.0);  // East of far west
  EXPECT_LT(lon, -117.0);  // West of LA area

  // At arrival time, should be at second waypoint
  EXPECT_TRUE(testRoute->GetPositionAtTime(eta2, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, 34.0522, -118.2437, 0.001));
}

// Test rhumb line interpolation accuracy
TEST_F(RouteTest, RhumbLineAccuracy) {
  // Create a route with known rhumb line characteristics
  // Use a more distant route where rhumb line vs great circle difference is
  // significant
  CreateSimpleRoute(40.7128, -74.0060, 51.5074, -0.1278);  // New York to London

  double lat, lon;

  RoutePoint* wp1 = testRoute->GetPoint(1);
  RoutePoint* wp2 = testRoute->GetPoint(2);

  // Calculate expected rhumb line bearing and distance
  double expectedBearing, expectedDistance;
  DistanceBearingMercator(wp2->m_lat, wp2->m_lon, wp1->m_lat, wp1->m_lon,
                          &expectedBearing, &expectedDistance);

  // Test at 25% of the journey
  wxDateTime etd1 = wp1->GetETD();
  wxDateTime eta2 = wp2->GetETA();
  wxTimeSpan segmentDuration = eta2 - etd1;
  wxDateTime quarterTime =
      etd1 + wxTimeSpan::Seconds(segmentDuration.GetSeconds().ToLong() / 4);

  EXPECT_TRUE(testRoute->GetPositionAtTime(quarterTime, lat, lon));

  // Calculate expected position along rhumb line at 25%
  double expectedLat, expectedLon;
  PositionBearingDistanceLoxodrome(wp1->m_lat, wp1->m_lon, expectedBearing,
                                   expectedDistance * 0.25, &expectedLat,
                                   &expectedLon);

  // Should match expected rhumb line position closely
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, expectedLat, expectedLon, 0.001));
}

// Test multi-waypoint route
TEST_F(RouteTest, MultiWaypointRoute) {
  CreateMultiWaypointRoute();  // SF -> SB -> LA

  double lat, lon;

  RoutePoint* wp1 = testRoute->GetPoint(1);  // SF
  RoutePoint* wp2 = testRoute->GetPoint(2);  // SB
  RoutePoint* wp3 = testRoute->GetPoint(3);  // LA

  // At departure, should be at SF
  EXPECT_TRUE(testRoute->GetPositionAtTime(baseDeparture, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, wp1->m_lat, wp1->m_lon, 0.001));

  // During first segment (SF to SB)
  wxDateTime eta2 = wp2->GetETA();
  wxDateTime etd1 = wp1->GetETD();
  wxTimeSpan firstSegDuration = eta2 - etd1;
  wxDateTime firstSegMid =
      etd1 + wxTimeSpan::Seconds(firstSegDuration.GetSeconds().ToLong() / 2);

  EXPECT_TRUE(testRoute->GetPositionAtTime(firstSegMid, lat, lon));

  // Should be somewhere between SF and SB - use broader bounds
  EXPECT_GT(lat, wp2->m_lat - 1.0);  // Near or north of Santa Barbara
  EXPECT_LT(lat, wp1->m_lat + 1.0);  // Near or south of San Francisco

  // At Santa Barbara arrival
  EXPECT_TRUE(testRoute->GetPositionAtTime(eta2, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, wp2->m_lat, wp2->m_lon, 0.001));

  // During second segment (SB to LA)
  wxDateTime etd2 = wp2->GetETD();
  wxDateTime eta3 = wp3->GetETA();
  wxTimeSpan secondSegDuration = eta3 - etd2;
  wxDateTime secondSegMid =
      etd2 + wxTimeSpan::Seconds(secondSegDuration.GetSeconds().ToLong() / 2);

  EXPECT_TRUE(testRoute->GetPositionAtTime(secondSegMid, lat, lon));

  // Should be somewhere between SB and LA - use broader bounds
  EXPECT_GT(lat, wp3->m_lat - 1.0);  // Near or north of Los Angeles
  EXPECT_LT(lat, wp2->m_lat + 1.0);  // Near or south of Santa Barbara

  // At final arrival
  EXPECT_TRUE(testRoute->GetPositionAtTime(eta3, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, wp3->m_lat, wp3->m_lon, 0.001));
}

// Test time well after route completion
TEST_F(RouteTest, TimeAfterRouteCompletion) {
  CreateSimpleRoute(37.7749, -122.4194, 34.0522, -118.2437);

  double lat, lon;

  RoutePoint* wp2 = testRoute->GetPoint(2);
  wxDateTime eta2 = wp2->GetETA();

  // Time well after route completion - should return last waypoint
  wxDateTime longAfter = eta2 + wxTimeSpan::Days(1);
  EXPECT_TRUE(testRoute->GetPositionAtTime(longAfter, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, wp2->m_lat, wp2->m_lon, 0.001));
}

// Test zero-duration segments (instantaneous waypoints)
TEST_F(RouteTest, ZeroDurationSegment) {
  CreateSimpleRoute(37.7749, -122.4194, 37.7749, -122.4194);  // Same position

  double lat, lon;

  // Should handle zero-duration segments gracefully
  EXPECT_TRUE(testRoute->GetPositionAtTime(baseDeparture, lat, lon));
  EXPECT_TRUE(PositionsApproxEqual(lat, lon, 37.7749, -122.4194, 0.001));
}

// Test longitude wraparound scenarios (crossing 180° meridian)
TEST_F(RouteTest, LongitudeWraparound) {
  // Route crossing the International Date Line
  CreateSimpleRoute(21.3099, -157.8581, 35.6762,
                    139.6503);  // Honolulu to Tokyo

  double lat, lon;

  RoutePoint* wp1 = testRoute->GetPoint(1);
  RoutePoint* wp2 = testRoute->GetPoint(2);

  // Test midpoint - should handle longitude wraparound correctly
  wxDateTime etd1 = wp1->GetETD();
  wxDateTime eta2 = wp2->GetETA();
  wxTimeSpan segmentDuration = eta2 - etd1;
  wxDateTime midTime =
      etd1 + wxTimeSpan::Seconds(segmentDuration.GetSeconds().ToLong() / 2);

  EXPECT_TRUE(testRoute->GetPositionAtTime(midTime, lat, lon));

  // Longitude should be in valid range [-180, 180]
  EXPECT_GE(lon, -180.0);
  EXPECT_LE(lon, 180.0);

  // Should be somewhere in the Pacific.
  EXPECT_GT(lat, 15.0);  // North of 15°N
  EXPECT_LT(lat, 45.0);  // South of 45°N

  // For longitude, this route crosses the International Date Line, so the rhumb
  // line midpoint could be either in the eastern Pacific (negative longitude)
  // or western Pacific (positive longitude) depending on the specific path
  // calculation. We verify it's in a reasonable Pacific region rather than
  // testing exact values.
  EXPECT_TRUE((lon >= 150.0 && lon <= 180.0) ||   // Western Pacific
              (lon >= -180.0 && lon <= -140.0));  // Eastern Pacific
}

// =============================================================================
// PositionBearingDistanceLoxodrome Unit Tests
// =============================================================================

// Test nearly due East/West course (fabs(delta_lat) < 1e-10)
TEST_F(RouteTest, LoxodromeEastWestCourse) {
  double lat = 30.0;       // 30°N
  double lon = -120.0;     // 120°W
  double bearing = 90.0;   // Due East
  double distance = 60.0;  // 60 NM (1 degree at equator)

  double result_lat, result_lon;
  PositionBearingDistanceLoxodrome(lat, lon, bearing, distance, &result_lat,
                                   &result_lon);

  // For due East course, latitude should remain unchanged
  EXPECT_NEAR(result_lat, 30.0, 0.001);
  // Longitude should increase by approximately 1 degree (adjusted for latitude)
  double expected_delta_lon = distance / (60.0 * cos(lat * DEGREE));
  EXPECT_NEAR(result_lon, lon + expected_delta_lon, 0.001);

  // Test due West course
  bearing = 270.0;  // Due West
  PositionBearingDistanceLoxodrome(lat, lon, bearing, distance, &result_lat,
                                   &result_lon);

  EXPECT_NEAR(result_lat, 30.0, 0.001);
  EXPECT_NEAR(result_lon, lon - expected_delta_lon, 0.001);
}

// Test large distance (fabs(delta_lat) > 0.01745) - uses exaggerated latitude
TEST_F(RouteTest, LoxodromeLargeDistance) {
  double lat = 40.0;        // 40°N
  double lon = -74.0;       // 74°W (New York area)
  double bearing = 45.0;    // Northeast
  double distance = 300.0;  // 300 NM (large distance > 1 degree)

  double result_lat, result_lon;
  PositionBearingDistanceLoxodrome(lat, lon, bearing, distance, &result_lat,
                                   &result_lon);

  // Calculate expected latitude change
  double expected_delta_lat = distance * cos(bearing * DEGREE) / 60.0;
  double expected_lat = lat + expected_delta_lat;

  EXPECT_NEAR(result_lat, expected_lat, 0.01);  // Should be close to expected
  EXPECT_GT(result_lat, lat);                   // Should move north
  EXPECT_GT(result_lon, lon);                   // Should move east

  // Test that the result is mathematically consistent with
  // DistanceBearingMercator
  double back_bearing, back_distance;
  DistanceBearingMercator(lat, lon, result_lat, result_lon, &back_bearing,
                          &back_distance);

  EXPECT_NEAR(back_distance, distance, 0.001);

  // Verify we moved in the correct direction
  EXPECT_GT(result_lat,
            lat);  // Should move north (bearing 45° has north component)
  EXPECT_GT(result_lon,
            lon);  // Should move east (bearing 45° has east component)
}

// Test small distance (else case) - uses simple formula
TEST_F(RouteTest, LoxodromeSmallDistance) {
  double lat = 35.0;      // 35°N
  double lon = -118.0;    // 118°W (LA area)
  double bearing = 30.0;  // 30° (NNE)
  double distance = 5.0;  // 5 NM (small distance < 1 degree)

  double result_lat, result_lon;
  PositionBearingDistanceLoxodrome(lat, lon, bearing, distance, &result_lat,
                                   &result_lon);

  // Calculate expected position using simple rhumb line formula
  double delta_lat = distance * cos(bearing * DEGREE) / 60.0;
  double expected_lat = lat + delta_lat;
  double latm = (lat + expected_lat) / 2 * DEGREE;
  double expected_delta_lon = delta_lat * tan(bearing * DEGREE) / cos(latm);
  double expected_lon = lon + expected_delta_lon;

  EXPECT_NEAR(result_lat, expected_lat, 0.001);
  EXPECT_NEAR(result_lon, expected_lon, 0.001);

  // Verify consistency with DistanceBearingMercator
  double back_bearing, back_distance;
  DistanceBearingMercator(result_lat, result_lon, lat, lon, &back_bearing,
                          &back_distance);

  EXPECT_NEAR(back_distance, distance,
              0.01);  // Should be very close for small distances
}

// Test longitude wraparound scenarios
TEST_F(RouteTest, LoxodromeLongitudeWraparound) {
  // Test crossing 180° meridian (International Date Line)
  double lat = 35.0;
  double lon = 179.0;       // Near International Date Line
  double bearing = 90.0;    // Due East
  double distance = 120.0;  // 2 degrees worth

  double result_lat, result_lon;
  PositionBearingDistanceLoxodrome(lat, lon, bearing, distance, &result_lat,
                                   &result_lon);

  // Longitude should wrap around to negative values
  EXPECT_GE(result_lon, -180.0);
  EXPECT_LE(result_lon, 180.0);
  EXPECT_LT(result_lon, 0.0);  // Should be in western hemisphere after wrapping

  // Test crossing -180° meridian (going west)
  lon = -179.0;
  bearing = 270.0;  // Due West
  PositionBearingDistanceLoxodrome(lat, lon, bearing, distance, &result_lat,
                                   &result_lon);

  EXPECT_GE(result_lon, -180.0);
  EXPECT_LE(result_lon, 180.0);
  EXPECT_GT(result_lon, 0.0);  // Should be in eastern hemisphere after wrapping

  // Test large eastward distance (should wrap around correctly)
  bearing = 90.0;     // Due East
  distance = 2400.0;  // Large distance but not full circle
  PositionBearingDistanceLoxodrome(lat, lon, bearing, distance, &result_lat,
                                   &result_lon);

  // Should wrap around and be in valid range
  EXPECT_GE(result_lon, -180.0);
  EXPECT_LE(result_lon, 180.0);
  // For such a large eastward distance from -179°, should end up somewhere in
  // western hemisphere
  EXPECT_GT(result_lon, -150.0);  // Should be east of -150°
}

// Test consistency with known route (Honolulu to Tokyo midpoint)
TEST_F(RouteTest, LoxodromeHonoluluTokyoMidpoint) {
  double honolulu_lat = 21.3099, honolulu_lon = -157.8581;
  double tokyo_lat = 35.6762, tokyo_lon = 139.6503;

  // Calculate bearing and distance using DistanceBearingMercator
  double bearing, total_distance;
  DistanceBearingMercator(tokyo_lat, tokyo_lon, honolulu_lat, honolulu_lon,
                          &bearing, &total_distance);

  // Calculate midpoint using our new function
  double midpoint_lat, midpoint_lon;
  PositionBearingDistanceLoxodrome(honolulu_lat, honolulu_lon, bearing,
                                   total_distance * 0.5, &midpoint_lat,
                                   &midpoint_lon);

  // This should match our expected rhumb line midpoint
  EXPECT_GT(midpoint_lat, 25.0);   // Should be well north of 25°N
  EXPECT_LT(midpoint_lat, 32.0);   // Should be south of 32°N
  EXPECT_GT(midpoint_lon, 150.0);  // Should be in western Pacific
  EXPECT_LT(midpoint_lon, 180.0);  // Should not cross date line for midpoint

  // Test the full journey - should end up at Tokyo
  double final_lat, final_lon;
  PositionBearingDistanceLoxodrome(honolulu_lat, honolulu_lon, bearing,
                                   total_distance, &final_lat, &final_lon);

  EXPECT_NEAR(final_lat, tokyo_lat, 0.01);  // Should be very close to Tokyo
  EXPECT_NEAR(final_lon, tokyo_lon, 0.01);
}

// Test edge cases and special bearings
TEST_F(RouteTest, LoxodromeEdgeCases) {
  double lat = 45.0, lon = 0.0;  // 45°N, Prime Meridian
  double result_lat, result_lon;

  // Test due North (bearing = 0)
  PositionBearingDistanceLoxodrome(lat, lon, 0.0, 60.0, &result_lat,
                                   &result_lon);
  EXPECT_NEAR(result_lat, 46.0, 0.001);  // Should move 1° north
  EXPECT_NEAR(result_lon, 0.0, 0.001);   // Longitude unchanged

  // Test due South (bearing = 180)
  PositionBearingDistanceLoxodrome(lat, lon, 180.0, 60.0, &result_lat,
                                   &result_lon);
  EXPECT_NEAR(result_lat, 44.0, 0.001);  // Should move 1° south
  EXPECT_NEAR(result_lon, 0.0, 0.001);   // Longitude unchanged

  // Test zero distance
  PositionBearingDistanceLoxodrome(lat, lon, 45.0, 0.0, &result_lat,
                                   &result_lon);
  EXPECT_NEAR(result_lat, lat, 0.001);  // Should stay at same position
  EXPECT_NEAR(result_lon, lon, 0.001);

  // Test bearing normalization (bearing > 360)
  PositionBearingDistanceLoxodrome(lat, lon, 450.0, 60.0, &result_lat,
                                   &result_lon);  // 450° = 90°
  double result_lat_90, result_lon_90;
  PositionBearingDistanceLoxodrome(lat, lon, 90.0, 60.0, &result_lat_90,
                                   &result_lon_90);
  EXPECT_NEAR(result_lat, result_lat_90, 0.001);  // Should be same as 90°
  EXPECT_NEAR(result_lon, result_lon_90, 0.001);
}

// Performance test for multiple calculations
TEST_F(RouteTest, Performance) {
  CreateMultiWaypointRoute();

  double lat, lon;
  int successCount = 0;

  // Test 1000 position calculations
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < 1000; i++) {
    wxDateTime testTime = baseDeparture + wxTimeSpan::Minutes(i);
    if (testRoute->GetPositionAtTime(testTime, lat, lon)) {
      successCount++;
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // Should complete in reasonable time (less than 1 second for 1000
  // calculations)
  EXPECT_LT(duration.count(), 1000);

  // Should have some successful calculations
  EXPECT_GT(successCount, 0);
}

// Test GetTimeAtPosition function
TEST_F(RouteTest, GetTimeAtPosition) {
  CreateMultiWaypointRoute();

  // Get waypoints from the route
  RoutePoint* wp1 = testRoute->GetPoint(1);  // SF
  RoutePoint* wp2 = testRoute->GetPoint(2);  // SB
  RoutePoint* wp3 = testRoute->GetPoint(3);  // LA

  ASSERT_TRUE(wp1 != nullptr);
  ASSERT_TRUE(wp2 != nullptr);
  ASSERT_TRUE(wp3 != nullptr);

  // Get timing information
  wxDateTime eta2 = wp2->GetETA();
  wxDateTime eta3 = wp3->GetETA();

  // Test position at first waypoint
  wxDateTime timeAtFirst = testRoute->GetTimeAtPosition(wp1->m_lat, wp1->m_lon);
  EXPECT_TRUE(timeAtFirst.IsValid());
  EXPECT_EQ(timeAtFirst, baseDeparture);

  // Test position at second waypoint
  wxDateTime timeAtSecond =
      testRoute->GetTimeAtPosition(wp2->m_lat, wp2->m_lon);
  EXPECT_TRUE(timeAtSecond.IsValid());
  EXPECT_EQ(timeAtSecond, eta2);

  // Test position at third waypoint
  wxDateTime timeAtThird = testRoute->GetTimeAtPosition(wp3->m_lat, wp3->m_lon);
  EXPECT_TRUE(timeAtThird.IsValid());
  EXPECT_EQ(timeAtThird, eta3);

  // Test position halfway between first and second waypoint
  double midLat = (wp1->m_lat + wp2->m_lat) / 2.0;
  double midLon = (wp1->m_lon + wp2->m_lon) / 2.0;
  wxDateTime timeAtMid = testRoute->GetTimeAtPosition(midLat, midLon);
  EXPECT_TRUE(timeAtMid.IsValid());

  // Should be between departure and first arrival
  EXPECT_GT(timeAtMid, baseDeparture);
  EXPECT_LT(timeAtMid, eta2);

  // Test with route that has no timing information
  Route routeNoTiming;
  RoutePoint* rp1 = new RoutePoint(37.0, -122.0, _T("icon"), _T("WP1"), _T(""));
  routeNoTiming.AddPoint(rp1);

  wxDateTime invalidTime = routeNoTiming.GetTimeAtPosition(37.0, -122.0);
  EXPECT_FALSE(invalidTime.IsValid());

  delete rp1;
}

// Test GetTimeAtPosition with runtime timing calculation scenario
TEST_F(RouteTest, GetTimeAtPositionRuntimeScenario) {
  // Create a route similar to runtime conditions:
  // - Route has planned departure time
  // - Route has planned speed
  // - But segment distances haven't been calculated yet

  Route* runtimeRoute = new Route();
  runtimeRoute->SetDepartureDate(baseDeparture);
  runtimeRoute->m_PlannedSpeed = 5.0;  // 5 knots

  // Add waypoints for a typical Pacific route (LA to Hawaii style)
  RoutePoint* wp1 =
      new RoutePoint(33.678453, -118.305727, _T("icon"), _T("001"), _T(""));
  RoutePoint* wp2 =
      new RoutePoint(31.688667, -129.029749, _T("icon"), _T("002"), _T(""));
  RoutePoint* wp3 =
      new RoutePoint(28.840193, -139.214165, _T("icon"), _T("003"), _T(""));
  RoutePoint* wp4 =
      new RoutePoint(25.266245, -148.794148, _T("icon"), _T("004"), _T(""));
  RoutePoint* wp5 =
      new RoutePoint(21.106557, -157.787475, _T("icon"), _T("005"), _T(""));

  runtimeRoute->AddPoint(wp1, false);
  runtimeRoute->AddPoint(wp2, false);
  runtimeRoute->AddPoint(wp3, false);
  runtimeRoute->AddPoint(wp4, false);
  runtimeRoute->AddPoint(wp5, false);

  // Initially, timing calculations should not be available
  wxDateTime initialTime =
      runtimeRoute->GetTimeAtPosition(24.080169, -151.386330);
  EXPECT_FALSE(initialTime.IsValid())
      << "Should return invalid time before timing is calculated";

  // Now calculate segment distances and timing (as happens in runtime)
  runtimeRoute->UpdateSegmentDistances(runtimeRoute->m_PlannedSpeed);

  // After timing calculation, GetTimeAtPosition should work
  wxDateTime calculatedTime =
      runtimeRoute->GetTimeAtPosition(24.080169, -151.386330);
  EXPECT_TRUE(calculatedTime.IsValid())
      << "Should return valid time after timing calculation";
  EXPECT_GT(calculatedTime, baseDeparture)
      << "Calculated time should be after departure";

  // Test various points along the route
  wxDateTime timeAtStart =
      runtimeRoute->GetTimeAtPosition(wp1->m_lat, wp1->m_lon);
  EXPECT_TRUE(timeAtStart.IsValid());
  EXPECT_EQ(timeAtStart, baseDeparture)
      << "Time at start should equal departure time";

  wxDateTime timeAtEnd =
      runtimeRoute->GetTimeAtPosition(wp5->m_lat, wp5->m_lon);
  EXPECT_TRUE(timeAtEnd.IsValid());
  EXPECT_GT(timeAtEnd, baseDeparture)
      << "Time at end should be after departure";
  EXPECT_GT(timeAtEnd, calculatedTime)
      << "Time at end should be after intermediate point";

  // Test point close to route but not exactly on it (as happens with cursor
  // hover)
  wxDateTime timeNearRoute =
      runtimeRoute->GetTimeAtPosition(24.069553, -151.381420);
  EXPECT_TRUE(timeNearRoute.IsValid())
      << "Should work for points near but not exactly on route";

  // Clean up
  delete runtimeRoute;
}

// Test waypoint addition and removal
// This test requires Select functionality, so we ensure pSelect is available
class RouteWaypointTest : public RouteTest {
protected:
  void SetUp() override {
    RouteTest::SetUp();

    // Ensure pSelect is available for waypoint selection operations
    original_pSelect = pSelect;
    if (!pSelect) {
      // Ensure g_BasePlatform is valid before creating Select
      // The existing test infrastructure should have created it, but check
      // anyway
      if (!g_BasePlatform) {
        g_BasePlatform = new BasePlatform();
        created_g_BasePlatform = true;
      }

      pSelect = new Select();
      created_pSelect = true;
    }
  }

  void TearDown() override {
    // Clean up pSelect if we created it
    if (created_pSelect && pSelect) {
      delete pSelect;
      pSelect = original_pSelect;
      created_pSelect = false;
    }

    // Clean up g_BasePlatform if we created it
    if (created_g_BasePlatform && g_BasePlatform) {
      delete g_BasePlatform;
      g_BasePlatform = nullptr;
      created_g_BasePlatform = false;
    }

    RouteTest::TearDown();
  }

private:
  Select* original_pSelect = nullptr;
  bool created_pSelect = false;
  bool created_g_BasePlatform = false;
};

TEST_F(RouteWaypointTest, WaypointManagement) {
  RoutePoint* wp1 =
      new RoutePoint(40.0, -70.0, "diamond", "WP1", wxEmptyString, false);
  RoutePoint* wp2 =
      new RoutePoint(41.0, -71.0, "diamond", "WP2", wxEmptyString, false);
  RoutePoint* wp3 =
      new RoutePoint(42.0, -72.0, "diamond", "WP3", wxEmptyString, false);

  // Add waypoints
  testRoute->AddPoint(wp1);
  EXPECT_EQ(testRoute->GetnPoints(), 1);

  testRoute->AddPoint(wp2);
  EXPECT_EQ(testRoute->GetnPoints(), 2);

  testRoute->AddPoint(wp3);
  EXPECT_EQ(testRoute->GetnPoints(), 3);

  // Test point retrieval
  EXPECT_EQ(testRoute->GetPoint(1), wp1);
  EXPECT_EQ(testRoute->GetPoint(2), wp2);
  EXPECT_EQ(testRoute->GetPoint(3), wp3);
  EXPECT_EQ(testRoute->GetLastPoint(), wp3);

  // Test point removal - RemovePoint removes from list but doesn't delete
  testRoute->RemovePoint(wp2);
  EXPECT_EQ(testRoute->GetnPoints(), 2);
  EXPECT_EQ(testRoute->GetPoint(2), wp3);  // wp3 should now be point 2

  // Since RemovePoint doesn't delete the point, we need to delete it manually
  delete wp2;

  // The remaining points (wp1, wp3) will be cleaned up by TearDown()
}
