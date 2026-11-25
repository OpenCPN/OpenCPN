#include <initializer_list>
#include <memory>
#include <vector>

#include <gtest/gtest.h>
#include <wx/colour.h>

#include "model/routeman.h"
#include "model/track.h"

class TrackCloneTest : public ::testing::Test {
protected:
  void SetUp() override {
    previous_waypoints = pWayPointMan;
    waypoints = std::make_unique<WayPointman>(
        [](wxString) { return wxColour(0, 0, 0); });
    pWayPointMan = waypoints.get();
  }

  void TearDown() override { pWayPointMan = previous_waypoints; }

  static void AddPoints(Track &track, std::initializer_list<int> segments) {
    for (int segment : segments) {
      const int index = track.GetnPoints();
      auto *point = new TrackPoint(50.0 + index * 0.01, -1.0 - index * 0.01,
                                   wxString("2026-11-16T12:00:00Z"));
      point->m_GPXTrkSegNo = segment;
      track.AddPoint(point);
    }
  }

  static std::vector<int> Segments(Track &track) {
    std::vector<int> result;
    for (int i = 0; i < track.GetnPoints(); ++i)
      result.push_back(track.GetPoint(i)->m_GPXTrkSegNo);
    return result;
  }

  WayPointman *previous_waypoints = nullptr;
  std::unique_ptr<WayPointman> waypoints;
};

TEST_F(TrackCloneTest, SplitPreservesBreaksAndCopiesPointData) {
  Track source, head, tail;
  AddPoints(source, {1, 1, 3, 3, 5});
  source.SetName("Voyage");
  source.m_TrackStartString = "Departure";
  source.m_TrackEndString = "Arrival";

  // The split dialog includes the selected point in both resulting tracks
  // and passes GetnPoints() as the tail's inclusive upper bound.
  head.Clone(&source, 0, 2, "_A");
  tail.Clone(&source, 2, source.GetnPoints(), "_B");

  EXPECT_EQ(Segments(head), (std::vector<int>{1, 1, 2}));
  EXPECT_EQ(Segments(tail), (std::vector<int>{1, 1, 2}));
  EXPECT_EQ(Segments(source), (std::vector<int>{1, 1, 3, 3, 5}));
  EXPECT_EQ(tail.GetName(), "Voyage_B");
  EXPECT_EQ(tail.m_TrackStartString, "Departure");
  EXPECT_EQ(tail.m_TrackEndString, "Arrival");
  ASSERT_EQ(tail.GetnPoints(), 3);
  for (int i = 0; i < tail.GetnPoints(); ++i) {
    auto *original = source.GetPoint(i + 2);
    auto *copy = tail.GetPoint(i);
    EXPECT_NE(copy, original);
    EXPECT_DOUBLE_EQ(copy->m_lat, original->m_lat);
    EXPECT_DOUBLE_EQ(copy->m_lon, original->m_lon);
    EXPECT_EQ(copy->GetCreateTime(), original->GetCreateTime());
  }
}

TEST_F(TrackCloneTest, SplitWithinSegmentRebasesLaterBreaks) {
  Track source, tail;
  AddPoints(source, {1, 3, 3, 5, 5});
  tail.Clone(&source, 2, source.GetnPoints(), "_B");
  EXPECT_EQ(Segments(tail), (std::vector<int>{1, 2, 2}));
}

TEST_F(TrackCloneTest, ExtendJoinsLastSegmentAndPreservesLaterBreaks) {
  Track source, destination;
  AddPoints(source, {1, 1, 3, 3, 5});
  AddPoints(destination, {1, 2, 2});
  // Extension skips the first point when its timestamp matches the existing
  // endpoint. Its first copied run must continue the destination's last run.
  destination.Clone(&source, 1, source.GetnPoints(), "_plus");
  EXPECT_EQ(Segments(destination), (std::vector<int>{1, 2, 2, 2, 3, 3, 4}));
  EXPECT_EQ(Segments(source), (std::vector<int>{1, 1, 3, 3, 5}));
}

TEST_F(TrackCloneTest, SingleSegmentStaysContinuous) {
  Track source, destination;
  AddPoints(source, {7, 7, 7});
  destination.Clone(&source, 0, source.GetnPoints(), "_copy");
  EXPECT_EQ(Segments(destination), (std::vector<int>{1, 1, 1}));
}

TEST_F(TrackCloneTest, LayerTrackIsNotCopied) {
  Track source, destination;
  AddPoints(source, {1, 2});
  AddPoints(destination, {4});
  source.m_bIsInLayer = true;
  destination.SetName("Unchanged");
  destination.Clone(&source, 0, source.GetnPoints(), "_copy");
  EXPECT_EQ(Segments(destination), (std::vector<int>{4}));
  EXPECT_EQ(destination.GetName(), "Unchanged");
}
