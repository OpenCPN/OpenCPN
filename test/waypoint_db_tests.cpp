#include <memory>

#include <gtest/gtest.h>
#include <sqlite3.h>
#include <wx/filename.h>
#include <wx/init.h>

#include "model/base_platform.h"
#include "model/cmdline.h"
#include "model/navobj_db.h"
#include "model/routeman.h"
#include "model/select.h"

namespace {
class WaypointTimestampTest : public ::testing::TestWithParam<bool> {
protected:
  void SetUp() override {
    waypoints = std::make_unique<WayPointman>(
        [](wxString) { return wxColour(0, 0, 0); });
    pWayPointMan = waypoints.get();
    routes = std::make_unique<RouteList>();
    pRouteList = routes.get();
    routeman = std::make_unique<Routeman>(RoutePropDlgCtx(), RoutemanDlgCtx());
    g_pRouteMan = routeman.get();
    selection = std::make_unique<Select>();
    pSelect = selection.get();
    ASSERT_EQ(sqlite3_open((g_configdir + "/navobj.db").c_str(), &sql),
              SQLITE_OK);
    ASSERT_EQ(sqlite3_exec(sql,
                           "DELETE FROM routepoints_link;"
                           "DELETE FROM routes; DELETE FROM routepoints;",
                           nullptr, nullptr, nullptr),
              SQLITE_OK);
  }

  void TearDown() override {
    for (auto* route : *routes) delete route;
    routes->clear();
    selection.reset();
    pSelect = nullptr;
    routeman.reset();
    g_pRouteMan = nullptr;
    waypoints.reset();
    pWayPointMan = nullptr;
    pRouteList = nullptr;
    sqlite3_close_v2(sql);
  }

  void LoadAndCheck(const char* first_time, const char* second_time,
                    bool valid) {
    RoutePoint first(50, -1, "circle", "First", "time-first", false);
    RoutePoint second(51, -2, "circle", "Second", "time-second", false);
    auto& db = NavObj_dB::GetInstance();
    if (GetParam()) {
      Route route;
      route.AddPoint(&first, false);
      route.AddPoint(&second, false);
      ASSERT_TRUE(db.InsertRoute(&route));
    } else {
      first.m_bIsolatedMark = second.m_bIsolatedMark = true;
      ASSERT_TRUE(db.InsertRoutePoint(&first));
      ASSERT_TRUE(db.InsertRoutePoint(&second));
    }

    // Seed exact stored values independently of the writer's wall clock.
    sqlite3_stmt* statement = nullptr;
    ASSERT_EQ(sqlite3_prepare_v2(
                  sql, "UPDATE routepoints SET created_at = ? WHERE guid = ?",
                  -1, &statement, nullptr),
              SQLITE_OK);
    const char* values[] = {first_time, second_time};
    const char* guids[] = {"time-first", "time-second"};
    for (int i = 0; i < 2; ++i) {
      sqlite3_reset(statement);
      if (values[i])
        sqlite3_bind_text(statement, 1, values[i], -1, SQLITE_TRANSIENT);
      else
        sqlite3_bind_null(statement, 1);
      sqlite3_bind_text(statement, 2, guids[i], -1, SQLITE_STATIC);
      EXPECT_EQ(sqlite3_step(statement), SQLITE_DONE);
    }
    sqlite3_finalize(statement);

    if (GetParam()) {
      ASSERT_TRUE(db.LoadAllRoutes());
      ASSERT_EQ(routes->size(), 1u);
      ASSERT_EQ(routes->front()->GetnPoints(), 2);
    } else {
      ASSERT_TRUE(db.LoadAllPoints());
    }
    const time_t expected[] = {1768564800, 1784203200};
    for (int i = 0; i < 2; ++i) {
      auto* point = pWayPointMan->FindRoutePointByGUID(guids[i]);
      ASSERT_NE(point, nullptr);
      EXPECT_DOUBLE_EQ(point->m_lat, 50.0 + i);
      EXPECT_DOUBLE_EQ(point->m_lon, -1.0 - i);
      EXPECT_EQ(point->m_CreateTimeX.IsValid(), valid);
      if (valid && point->m_CreateTimeX.IsValid())
        EXPECT_EQ(point->m_CreateTimeX.GetTicks(), expected[i]);
    }
  }

  sqlite3* sql = nullptr;
  std::unique_ptr<WayPointman> waypoints;
  std::unique_ptr<RouteList> routes;
  std::unique_ptr<Routeman> routeman;
  std::unique_ptr<Select> selection;
};

TEST_P(WaypointTimestampTest, WinterAndSummerTimesAreUtc) {
  LoadAndCheck("2026-01-16 12:00:00", "2026-07-16 12:00:00", true);
}

TEST_P(WaypointTimestampTest, NullTimesStayUnknown) {
  LoadAndCheck(nullptr, nullptr, false);
}

TEST_P(WaypointTimestampTest, InvalidTimesStayUnknown) {
  LoadAndCheck("not-a-date", "", false);
}

INSTANTIATE_TEST_SUITE_P(Loaders, WaypointTimestampTest, ::testing::Bool(),
                         [](const ::testing::TestParamInfo<bool>& info) {
                           return info.param ? "Route" : "Standalone";
                         });
}  // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // wx must outlive the database/notification singletons.
  static wxInitializer wx;
  if (!wx.IsOk()) return 1;
  const wxString directory = wxFileName::CreateTempFileName("ocpn-wpt-db-");
  if (directory.empty() || !wxRemoveFile(directory) ||
      !wxFileName::Mkdir(directory))
    return 1;
  g_configdir = directory.ToStdString();
  int result;
  {
    BasePlatform platform;
    g_BasePlatform = &platform;
    NavObj_dB::GetInstance();
    result = RUN_ALL_TESTS();
    NavObj_dB::GetInstance().Close();
    g_BasePlatform = nullptr;
  }
  if (!wxFileName::Rmdir(directory, wxPATH_RMDIR_RECURSIVE)) return 1;
  return result;
}
