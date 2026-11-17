// SPDX-License-Identifier: GPL-2.0-or-later

#include <memory>
#include <limits>

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
class RouteDepartureTest : public ::testing::Test {
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

  void CheckDeparture(sqlite3_int64 ticks, bool valid, bool seed_directly,
                      bool clear_existing = false, bool representable = true) {
    RoutePoint first(50, -1, "circle", "First", "departure-first", false);
    RoutePoint second(51, -2, "circle", "Second", "departure-second", false);
    Route route;
    route.AddPoint(&first, false);
    route.AddPoint(&second, false);
    route.m_PlannedDeparture = valid && !seed_directly
                                   ? wxDateTime(wxLongLong(ticks) * 1000)
                                   : wxInvalidDateTime;
    auto& db = NavObj_dB::GetInstance();
    ASSERT_TRUE(db.InsertRoute(&route));
    if (clear_existing) {
      route.m_PlannedDeparture = wxInvalidDateTime;
      ASSERT_TRUE(db.UpdateRoute(&route));
      valid = false;
    }
    if (seed_directly) {
      sqlite3_stmt* update = nullptr;
      ASSERT_EQ(
          sqlite3_prepare_v2(sql, "UPDATE routes SET planned_departure = ?", -1,
                             &update, nullptr),
          SQLITE_OK);
      if (valid)
        sqlite3_bind_int64(update, 1, static_cast<sqlite3_int64>(ticks));
      else
        sqlite3_bind_null(update, 1);
      EXPECT_EQ(sqlite3_step(update), SQLITE_DONE);
      sqlite3_finalize(update);
    } else {
      sqlite3_stmt* query = nullptr;
      ASSERT_EQ(sqlite3_prepare_v2(sql, "SELECT planned_departure FROM routes",
                                   -1, &query, nullptr),
                SQLITE_OK);
      ASSERT_EQ(sqlite3_step(query), SQLITE_ROW);
      EXPECT_EQ(sqlite3_column_type(query, 0) != SQLITE_NULL, valid);
      if (valid)
        EXPECT_EQ(sqlite3_column_int64(query, 0),
                  static_cast<sqlite3_int64>(ticks));
      sqlite3_finalize(query);
    }
    ASSERT_TRUE(db.LoadAllRoutes());
    ASSERT_EQ(routes->size(), 1u);
    const auto& loaded = routes->front()->m_PlannedDeparture;
    EXPECT_EQ(loaded.IsValid(), valid && representable);
    if (valid && representable && loaded.IsValid())
      EXPECT_EQ(loaded.GetValue().GetValue(), ticks * 1000);
  }

  sqlite3* sql = nullptr;
  std::unique_ptr<WayPointman> waypoints;
  std::unique_ptr<RouteList> routes;
  std::unique_ptr<Routeman> routeman;
  std::unique_ptr<Select> selection;
};

TEST_F(RouteDepartureTest, WriterAndLoaderFuture) {
  CheckDeparture(static_cast<sqlite3_int64>(4102444800LL), true, false);
}

TEST_F(RouteDepartureTest, ExistingDatabaseFuture) {
  CheckDeparture(static_cast<sqlite3_int64>(4102444800LL), true, true);
}

TEST_F(RouteDepartureTest, WriterAndLoaderPast) {
  CheckDeparture(static_cast<sqlite3_int64>(-2208988800LL), true, false);
}

TEST_F(RouteDepartureTest, ExistingDatabasePast) {
  CheckDeparture(static_cast<sqlite3_int64>(-2208988800LL), true, true);
}

TEST_F(RouteDepartureTest, WriterAndLoaderEpoch) {
  CheckDeparture(static_cast<sqlite3_int64>(0LL), true, false);
}

TEST_F(RouteDepartureTest, ExistingDatabaseEpoch) {
  CheckDeparture(static_cast<sqlite3_int64>(0LL), true, true);
}

TEST_F(RouteDepartureTest, WriterAndLoaderUnset) {
  CheckDeparture(static_cast<sqlite3_int64>(0LL), false, false);
}

TEST_F(RouteDepartureTest, ExistingDatabaseUnset) {
  CheckDeparture(static_cast<sqlite3_int64>(0LL), false, true);
}

TEST_F(RouteDepartureTest, ClearStoredDeparture) {
  CheckDeparture(static_cast<sqlite3_int64>(1784203200), true, false, true);
}
TEST_F(RouteDepartureTest, WriterAndLoaderBeforeEpoch) {
  CheckDeparture(-1, true, false);
}
TEST_F(RouteDepartureTest, ExistingDatabaseBeforeEpoch) {
  CheckDeparture(-1, true, true);
}
TEST_F(RouteDepartureTest, OverflowingStoredSecondsStayUnknown) {
  CheckDeparture(std::numeric_limits<sqlite3_int64>::max(), true, true, false,
                 false);
}
TEST_F(RouteDepartureTest, UnderflowingStoredSecondsStayUnknown) {
  CheckDeparture(std::numeric_limits<sqlite3_int64>::min(), true, true, false,
                 false);
}
}  // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // wx must outlive the database/notification singletons.
  static wxInitializer wx;
  if (!wx.IsOk()) return 1;
  const wxString directory = wxFileName::CreateTempFileName("ocpn-route-db-");
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
