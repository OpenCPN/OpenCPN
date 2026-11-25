#include <initializer_list>
#include <memory>
#include <vector>

#include <gtest/gtest.h>
#include <sqlite3.h>
#include <wx/filename.h>
#include <wx/init.h>

#include "model/base_platform.h"
#include "model/cmdline.h"
#include "model/navobj_db.h"
#include "model/routeman.h"
#include "model/select.h"

// Production schema initialization, also used when opening an existing file.
bool CreateTables(sqlite3* db);

namespace {
struct CloseSqlite {
  void operator()(sqlite3* db) const { sqlite3_close_v2(db); }
};
using Sqlite = std::unique_ptr<sqlite3, CloseSqlite>;

std::vector<int> ReadSegments(sqlite3* db) {
  sqlite3_stmt* statement = nullptr;
  const int prepared = sqlite3_prepare_v2(
      db, "SELECT segment FROM trk_points ORDER BY point_order", -1, &statement,
      nullptr);
  EXPECT_EQ(prepared, SQLITE_OK) << sqlite3_errmsg(db);
  if (prepared != SQLITE_OK) return {};
  std::vector<int> result;
  int status;
  while ((status = sqlite3_step(statement)) == SQLITE_ROW)
    result.push_back(sqlite3_column_int(statement, 0));
  EXPECT_EQ(status, SQLITE_DONE);
  sqlite3_finalize(statement);
  return result;
}

class TrackDatabaseTest : public ::testing::Test {
protected:
  void SetUp() override {
    ASSERT_TRUE(g_TrackList.empty());
    waypoints = std::make_unique<WayPointman>(
        [](wxString) { return wxColour(0, 0, 0); });
    pWayPointMan = waypoints.get();
    selection = std::make_unique<Select>();
    pSelect = selection.get();
    sqlite3* handle = nullptr;
    ASSERT_EQ(sqlite3_open((g_configdir + "/navobj.db").c_str(), &handle),
              SQLITE_OK);
    inspection.reset(handle);
    ASSERT_EQ(sqlite3_exec(inspection.get(),
                           "DELETE FROM trk_points; DELETE FROM tracks;",
                           nullptr, nullptr, nullptr),
              SQLITE_OK);
  }

  void ClearLoadedTracks() {
    for (auto* track : g_TrackList) {
      pSelect->DeleteAllSelectableTrackSegments(track);
      delete track;
    }
    g_TrackList.clear();
  }

  void TearDown() override {
    ClearLoadedTracks();
    pSelect = nullptr;
    pWayPointMan = nullptr;
  }

  static TrackPoint* AddPoint(Track& track, int segment) {
    const int index = track.GetnPoints();
    auto* point = new TrackPoint(50.0 + index * 0.01, -1.0 - index * 0.01,
                                 wxString("2026-11-16T12:00:00Z"));
    point->m_GPXTrkSegNo = segment;
    track.AddPoint(point);
    return point;
  }

  static void AddPoints(Track& track, std::initializer_list<int> segments) {
    for (int segment : segments) AddPoint(track, segment);
  }

  void ExpectReloaded(const Track& source, const std::vector<int>& segments) {
    ClearLoadedTracks();
    ASSERT_TRUE(NavObj_dB::GetInstance().LoadAllTracks());
    ASSERT_EQ(g_TrackList.size(), 1u);
    auto* loaded = g_TrackList.front();
    EXPECT_EQ(loaded->m_GUID, source.m_GUID);
    EXPECT_EQ(loaded->GetName(), source.GetName());
    ASSERT_EQ(loaded->GetnPoints(), static_cast<int>(segments.size()));
    for (int i = 0; i < loaded->GetnPoints(); ++i) {
      auto* point = loaded->GetPoint(i);
      EXPECT_EQ(point->m_GPXTrkSegNo, segments[i]);
      EXPECT_DOUBLE_EQ(point->m_lat, 50.0 + i * 0.01);
      EXPECT_DOUBLE_EQ(point->m_lon, -1.0 - i * 0.01);
      EXPECT_STREQ(point->GetTimeString(), "2026-11-16T12:00:00Z");
    }
    ASSERT_FALSE(segments.empty());
    EXPECT_EQ(loaded->GetCurrentTrackSeg(), segments.back());
  }

  Sqlite inspection;
  std::unique_ptr<WayPointman> waypoints;
  std::unique_ptr<Select> selection;
};

TEST_F(TrackDatabaseTest, InsertAndLoadPreserveSegments) {
  Track source;
  source.SetName("Two gaps");
  AddPoints(source, {1, 1, 3, 3, 5});
  ASSERT_TRUE(NavObj_dB::GetInstance().InsertTrack(&source));
  ExpectReloaded(source, {1, 1, 3, 3, 5});
  // Verify committed rows through an independent SQLite connection too.
  EXPECT_EQ(ReadSegments(inspection.get()), (std::vector<int>{1, 1, 3, 3, 5}));
}

TEST_F(TrackDatabaseTest, UpdateReplacesStoredSegments) {
  Track source;
  AddPoints(source, {1, 1, 1});
  ASSERT_TRUE(NavObj_dB::GetInstance().InsertTrack(&source));
  source.GetPoint(1)->m_GPXTrkSegNo = 4;
  source.GetPoint(2)->m_GPXTrkSegNo = 4;
  AddPoint(source, 9);
  ASSERT_TRUE(NavObj_dB::GetInstance().UpdateTrack(&source));
  ExpectReloaded(source, {1, 4, 4, 9});
}

TEST_F(TrackDatabaseTest, AppendPreservesNewSegment) {
  Track source;
  AddPoints(source, {1, 1});
  ASSERT_TRUE(NavObj_dB::GetInstance().InsertTrack(&source));
  auto* point = AddPoint(source, 3);
  ASSERT_TRUE(NavObj_dB::GetInstance().AddTrackPoint(&source, point));
  ExpectReloaded(source, {1, 1, 3});
}

TEST_F(TrackDatabaseTest, SingleSegmentRemainsContinuous) {
  Track source;
  AddPoints(source, {1, 1, 1});
  ASSERT_TRUE(NavObj_dB::GetInstance().InsertTrack(&source));
  ExpectReloaded(source, {1, 1, 1});
}

TEST(TrackSchemaTest, MigratesLegacyRowsAndCanRunAgain) {
  sqlite3* handle = nullptr;
  ASSERT_EQ(sqlite3_open(":memory:", &handle), SQLITE_OK);
  Sqlite db(handle);
  ASSERT_TRUE(CreateTables(db.get()));
  // This is the old track-point schema, including a row with no segment data.
  ASSERT_EQ(sqlite3_exec(db.get(), R"(
      PRAGMA foreign_keys = ON;
      DROP TABLE trk_points;
      CREATE TABLE trk_points (
          track_guid TEXT NOT NULL, latitude REAL NOT NULL,
          longitude REAL NOT NULL, timestamp TEXT NOT NULL,
          point_order INTEGER,
          FOREIGN KEY (track_guid) REFERENCES tracks(guid) ON DELETE CASCADE);
      INSERT INTO tracks (guid) VALUES ('legacy');
      INSERT INTO trk_points VALUES ('legacy', 50.0, -1.0, 'old-time', 0);
  )",
                         nullptr, nullptr, nullptr),
            SQLITE_OK);
  ASSERT_TRUE(CreateTables(db.get()));
  EXPECT_EQ(ReadSegments(db.get()), (std::vector<int>{1}));
  // A subsequent startup must retain existing values, not reset them to 1.
  ASSERT_EQ(sqlite3_exec(db.get(), "UPDATE trk_points SET segment = 7", nullptr,
                         nullptr, nullptr),
            SQLITE_OK);
  ASSERT_TRUE(CreateTables(db.get()));
  EXPECT_EQ(ReadSegments(db.get()), (std::vector<int>{7}));
  sqlite3_stmt* statement = nullptr;
  ASSERT_EQ(
      sqlite3_prepare_v2(
          db.get(),
          "SELECT latitude, longitude, timestamp, point_order FROM trk_points",
          -1, &statement, nullptr),
      SQLITE_OK);
  ASSERT_EQ(sqlite3_step(statement), SQLITE_ROW);
  EXPECT_DOUBLE_EQ(sqlite3_column_double(statement, 0), 50.0);
  EXPECT_DOUBLE_EQ(sqlite3_column_double(statement, 1), -1.0);
  EXPECT_STREQ(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)),
               "old-time");
  EXPECT_EQ(sqlite3_column_int(statement, 3), 0);
  sqlite3_finalize(statement);
}

TEST(TrackSchemaTest, FreshSchemaDefaultsToOneAndCanRunAgain) {
  sqlite3* handle = nullptr;
  ASSERT_EQ(sqlite3_open(":memory:", &handle), SQLITE_OK);
  Sqlite db(handle);
  ASSERT_TRUE(CreateTables(db.get()));
  ASSERT_TRUE(CreateTables(db.get()));
  ASSERT_EQ(sqlite3_exec(db.get(), R"(
      PRAGMA foreign_keys = ON;
      INSERT INTO tracks (guid) VALUES ('old-writer');
      INSERT INTO trk_points
          (track_guid, latitude, longitude, timestamp, point_order)
          VALUES ('old-writer', 50.0, -1.0, 'old-time', 0);
  )",
                         nullptr, nullptr, nullptr),
            SQLITE_OK);
  EXPECT_EQ(ReadSegments(db.get()), (std::vector<int>{1}));
}
}  // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // Keep wx alive until database/notification singletons have been destroyed.
  static wxInitializer wx;
  if (!wx.IsOk()) return 1;
  const wxString directory = wxFileName::CreateTempFileName("ocpn-track-db-");
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
