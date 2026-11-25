#include <gtest/gtest.h>

#include <array>
#include <random>
#include "gui/chart_safety_cache_eviction.h"

#include "gui/chart_safety_service.h"

namespace {

using ocpn::chart_safety::AddCoverageTiles;
using ocpn::chart_safety::CoverageRing;
using ocpn::chart_safety::ExpandCandidateDiscoveryBounds;
using ocpn::chart_safety::GeographicBounds;
using ocpn::chart_safety::GlobalGridCoordinate;
using ocpn::chart_safety::MayPrefetchNeighbour;
using ocpn::chart_safety::PlanAdaptiveLookaheadTiles;
using ocpn::chart_safety::PlanTileBatchBlocks;
using ocpn::chart_safety::RouteMaskEvictionVictim;
using ocpn::chart_safety::RouteMaskRegionIsClear;
using ocpn::chart_safety::RouteMaskRegionStart;

using ocpn::chart_safety::kRouteMaskDeliveryWindow;
using ocpn::chart_safety::RouteMaskClock;

struct TestRouteMask {
  unsigned last_use;
  RouteMaskClock::time_point protected_until;
};

TEST(ChartSafetyService, EvictionProtectsRecentDeliveryAndPinnedMasks) {
  const auto now = RouteMaskClock::time_point{} + std::chrono::seconds(30);
  std::map<std::string, TestRouteMask> masks = {
      {"0:1067:-317", {4, now + kRouteMaskDeliveryWindow}},
      {"0:1600:-300", {1, {}}},
      {"0:1601:-300", {3, {}}},
      {"0:1602:-300", {2, {}}}};
  const std::set<std::string> pinned = {"0:1600:-300"};
  EXPECT_EQ(RouteMaskEvictionVictim(masks, pinned, now)->first, "0:1602:-300");
  masks.erase("0:1602:-300");
  masks.erase("0:1601:-300");
  EXPECT_EQ(RouteMaskEvictionVictim(masks, pinned, now)->first, "0:1067:-317");
}

TEST(ChartSafetyService, PartialRetriesKeepTheirMasksUnderSaturatedCache) {
  const auto start = RouteMaskClock::time_point{} + std::chrono::seconds(30);
  const auto retry = start + 2 * kRouteMaskDeliveryWindow;
  std::map<long, TestRouteMask> masks;
  constexpr long capacity = 4096;
  for (long old = 0; old < capacity; ++old)
    masks.emplace(old, TestRouteMask{static_cast<unsigned>(old),
                                     start + kRouteMaskDeliveryWindow});

  const std::set<long> pinned{0};
  // This models a retry that accumulates several recent masks while older
  // workers took a coarse shortcut and never consumed their own deliveries.
  const std::array<long, 6> demand{{capacity, capacity + 1, capacity + 2,
                                    capacity + 3, capacity + 4, capacity + 5}};
  unsigned clock = capacity;
  for (const auto key : demand) {
    const auto victim = RouteMaskEvictionVictim(masks, pinned, retry);
    ASSERT_NE(victim, masks.end());
    EXPECT_LT(victim->first, capacity);
    masks.erase(victim);
    masks.emplace(key,
                  TestRouteMask{++clock, retry + kRouteMaskDeliveryWindow});
    for (auto& existing : masks)
      if (existing.first >= capacity) existing.second.last_use = ++clock;
  }
  for (const auto key : demand) EXPECT_EQ(masks.count(key), 1u);
  EXPECT_EQ(masks.count(0), 1u);
  EXPECT_EQ(masks.size(), static_cast<std::size_t>(capacity));
}

TEST(ChartSafetyService, RegionalProofRequiresEveryChild) {
  const auto is_incomplete = [](long lat, long lon) {
    return lat != 266 || lon != -80;
  };
  EXPECT_EQ(RouteMaskRegionStart(-76, 5), -80);
  EXPECT_EQ(RouteMaskRegionStart(269, 5), 265);
  EXPECT_FALSE(RouteMaskRegionIsClear(265, -80, 5, is_incomplete));
  EXPECT_TRUE(
      RouteMaskRegionIsClear(265, -80, 5, [](long, long) { return true; }));
}

TEST(ChartSafetyService, PermitsPrefetchBeforePositiveBudgetExpires) {
  EXPECT_TRUE(MayPrefetchNeighbour(49, 50));
}

TEST(ChartSafetyService, StopsPrefetchAtAndAfterBudget) {
  EXPECT_FALSE(MayPrefetchNeighbour(50, 50));
  EXPECT_FALSE(MayPrefetchNeighbour(5000, 50));
}

TEST(ChartSafetyService, NonPositiveBudgetIsUnlimited) {
  EXPECT_TRUE(MayPrefetchNeighbour(5000, 0));
  EXPECT_TRUE(MayPrefetchNeighbour(5000, -1));
}

TEST(ChartSafetyService, AdaptiveLookaheadFillsAlignedCoarseBlock) {
  const auto tiles = PlanAdaptiveLookaheadTiles(-371, -3420, false, false);
  ASSERT_EQ(tiles.size(), 15u);
  std::set<std::pair<long, long>> unique(tiles.begin(), tiles.end());
  EXPECT_EQ(unique.size(), 15u);
  EXPECT_EQ(unique.count({-371, -3420}), 0u);
  for (const auto& tile : tiles) {
    EXPECT_GE(tile.first, -372);
    EXPECT_LE(tile.first, -369);
    EXPECT_GE(tile.second, -3420);
    EXPECT_LE(tile.second, -3417);
  }
}

TEST(ChartSafetyService, AdaptiveLookaheadKeepsExpensiveChecksDemandOnly) {
  EXPECT_TRUE(PlanAdaptiveLookaheadTiles(100, -20, true, false).empty());
  EXPECT_TRUE(PlanAdaptiveLookaheadTiles(100, -20, false, true).empty());
}

TEST(ChartSafetyService, PlansProviderSizedGeographicBlocks) {
  std::set<std::pair<long, long>> tiles;
  for (long lat = 0; lat < 7; ++lat)
    for (long lon = 0; lon < 7; ++lon) tiles.insert({lat, lon});

  const auto blocks = PlanTileBatchBlocks(tiles, 6);
  ASSERT_EQ(blocks.size(), 4u);
  for (const auto& block : blocks) {
    EXPECT_LE(block.max_lat_tile - block.min_lat_tile + 1, 6);
    EXPECT_LE(block.max_lon_tile - block.min_lon_tile + 1, 6);
    const long rows = (block.max_lat_tile - block.min_lat_tile + 1) * 40 + 1;
    const long cols = (block.max_lon_tile - block.min_lon_tile + 1) * 40 + 1;
    EXPECT_LE(rows * cols, 65536);
  }
}

TEST(ChartSafetyService, NegativeTileIndexesStayInBoundedBlocks) {
  const std::set<std::pair<long, long>> tiles = {{-7, -7}, {-6, -6}, {-1, -1},
                                                 {0, 0},   {5, 5},   {6, 6}};
  const auto blocks = PlanTileBatchBlocks(tiles, 6);
  ASSERT_FALSE(blocks.empty());
  for (const auto& block : blocks) {
    EXPECT_LE(block.max_lat_tile - block.min_lat_tile + 1, 6);
    EXPECT_LE(block.max_lon_tile - block.min_lon_tile + 1, 6);
  }
}

TEST(ChartSafetyService, SharedCellsUseOneCanonicalGlobalCoordinate) {
  constexpr int cells_per_tile = 40;
  constexpr double resolution = 0.00125;

  // The north/east edge of a tile is the south/west edge of its neighbour.
  EXPECT_DOUBLE_EQ(
      GlobalGridCoordinate(1089, cells_per_tile, cells_per_tile, resolution),
      GlobalGridCoordinate(1090, 0, cells_per_tile, resolution));
  EXPECT_DOUBLE_EQ(
      GlobalGridCoordinate(-86, cells_per_tile, cells_per_tile, resolution),
      GlobalGridCoordinate(-85, 0, cells_per_tile, resolution));

  // The result is also independent of a provider block's chosen origin.
  EXPECT_DOUBLE_EQ(GlobalGridCoordinate(1084, 6 * cells_per_tile,
                                        cells_per_tile, resolution),
                   GlobalGridCoordinate(1090, 0, cells_per_tile, resolution));
}

TEST(ChartSafetyService, CandidateDiscoveryHasOneCellHalo) {
  constexpr double resolution = 0.00125;
  const GeographicBounds raster{54.45, -4.30, 54.50, -4.25};
  const GeographicBounds discovery =
      ExpandCandidateDiscoveryBounds(raster, resolution);

  EXPECT_DOUBLE_EQ(discovery.min_lat, raster.min_lat - resolution);
  EXPECT_DOUBLE_EQ(discovery.min_lon, raster.min_lon - resolution);
  EXPECT_DOUBLE_EQ(discovery.max_lat, raster.max_lat + resolution);
  EXPECT_DOUBLE_EQ(discovery.max_lon, raster.max_lon + resolution);
  // The provider raster itself remains exactly unchanged.
  EXPECT_DOUBLE_EQ(raster.max_lat, 54.50);
  EXPECT_DOUBLE_EQ(raster.max_lon, -4.25);
}

TEST(ChartSafetyService, AtlasUsesPolygonRatherThanItsBoundingRectangle) {
  const CoverageRing triangle = {
      {53.00, -5.00}, {53.00, -4.80}, {53.20, -5.00}};
  std::set<std::pair<long, long>> tiles;
  ASSERT_TRUE(AddCoverageTiles({triangle}, {}, 0.05, 1000, &tiles));

  EXPECT_TRUE(tiles.count({1060, -100}));
  EXPECT_FALSE(tiles.count({1063, -97}));
  EXPECT_LT(tiles.size(), 16u);
}

TEST(ChartSafetyService, AtlasOmitsTilesWhollyInsideNoCoverageHole) {
  const CoverageRing coverage = {
      {53.00, -5.00}, {53.00, -4.80}, {53.20, -4.80}, {53.20, -5.00}};
  const CoverageRing hole = {
      {53.04, -4.96}, {53.04, -4.84}, {53.16, -4.84}, {53.16, -4.96}};
  std::set<std::pair<long, long>> tiles;
  ASSERT_TRUE(AddCoverageTiles({coverage}, {hole}, 0.05, 1000, &tiles));

  EXPECT_FALSE(tiles.count({1061, -99}));
  EXPECT_TRUE(tiles.count({1060, -100}));
}

TEST(ChartSafetyService, AtlasHonoursHardTileCapacity) {
  const CoverageRing coverage = {
      {50.0, -10.0}, {50.0, 2.0}, {60.0, 2.0}, {60.0, -10.0}};
  std::set<std::pair<long, long>> tiles;
  EXPECT_FALSE(AddCoverageTiles({coverage}, {}, 0.05, 100, &tiles));
  EXPECT_EQ(tiles.size(), 100u);
}

}  // namespace
// Longitude arithmetic is shared by public queries, tile halos and workers.
#include "gui/chart_safety_longitude.h"
TEST(ChartSafetyLongitude, OrdinaryCoordinatesKeepTheirExactRepresentation) {
  for (double lon : {-179.999, -174.309446, -0.0, 0.0, 177.353350, 179.999}) {
    const double actual = ocpn::chart_safety::CanonicalLongitude(lon);
    EXPECT_EQ(std::signbit(actual), std::signbit(lon));
    EXPECT_DOUBLE_EQ(actual, lon);
  }
}
TEST(ChartSafetyLongitude, AliasesHaveOneCanonicalIdentity) {
  for (long turns = -100; turns <= 100; ++turns) {
    EXPECT_DOUBLE_EQ(
        ocpn::chart_safety::CanonicalLongitude(180.0 + 360 * turns), -180.0);
    EXPECT_EQ(
        ocpn::chart_safety::CanonicalLongitudeIndex(3600 + 7200 * turns, 3600),
        -3600);
    EXPECT_EQ(
        ocpn::chart_safety::CanonicalLongitudeIndex(-3601 + 7200 * turns, 3600),
        3599);
  }
}
TEST(ChartSafetyLongitude, BothDirectionsFollowTheShortContinuousCrossing) {
  for (bool reverse : {false, true}) {
    double start = reverse ? -179.99 : 179.99, end = reverse ? 179.99 : -179.99;
    ocpn::chart_safety::UnwrapSafetySegment(start, end);
    EXPECT_NEAR(std::abs(end - start), .02, 1e-10);
    EXPECT_EQ(end > start, !reverse);
  }
}
TEST(ChartSafetyLongitude, ExactSeamAndDegenerateSegmentsRemainLocal) {
  for (double start : {-180., 180., 540.}) {
    double end = -180.;
    ocpn::chart_safety::UnwrapSafetySegment(start, end);
    EXPECT_DOUBLE_EQ(start, end);
  }
}

TEST(ChartSafetyService, PinnedEvictionIndexMatchesOriginalThroughKeyChanges) {
  std::map<std::string, std::vector<int>> actual, expected;
  std::set<std::string> pins;
  ocpn::chart_safety::PinnedCacheEvictionIndex<std::string> index;
  std::mt19937 random(9001);
  size_t compared_evictions = 0;
  for (int step = 0; step < 12000; ++step) {
    const auto key = std::to_string(random() % 91);
    switch (random() % 64) {
      case 0:
        pins.insert(key);
        index.Pinned(key);
        break;
      case 1:
        pins.clear();
        index.Reset();
        break;
      case 2:
        actual.erase(key);
        expected.erase(key);
        index.Erased(key);
        break;
      case 3:
        actual.clear();
        expected.clear();
        index.Reset();
        break;
      default:
        if (!expected.count(key))
          while (expected.size() >= 17) {
            auto victim = expected.begin();
            while (victim != expected.end() && pins.count(victim->first))
              ++victim;
            if (victim == expected.end()) break;
            auto chosen = index.First(actual, pins);
            ASSERT_NE(chosen, actual.end());
            ASSERT_EQ(chosen->first, victim->first);
            ++compared_evictions;
            index.Erased(chosen->first);
            actual.erase(chosen);
            expected.erase(victim);
          }
        if (!expected.count(key) && expected.size() >= 17)
          ASSERT_EQ(index.First(actual, pins), actual.end());
        actual[key] = expected[key] = {step, step ^ 127};
        index.Stored(key, pins);
    }
    ASSERT_EQ(actual, expected) << step;
  }
  EXPECT_GT(compared_evictions, 1000u);
}
TEST(ChartSafetyService, PinnedEvictionIndexPreservesAllPinnedOverflow) {
  std::map<std::string, int> cache;
  std::set<std::string> pins;
  ocpn::chart_safety::PinnedCacheEvictionIndex<std::string> index;
  for (int i = 0; i < 9000; ++i) {
    auto key = std::to_string(i);
    cache[key] = i;
    pins.insert(key);
  }
  EXPECT_EQ(index.First(cache, pins), cache.end());
  for (int i = 0; i < 100; ++i) {
    auto victim = index.First(cache, pins);
    if (i) {
      ASSERT_NE(victim, cache.end());
      EXPECT_EQ(victim->first, "outside-" + std::to_string(i - 1));
      index.Erased(victim->first);
      cache.erase(victim);
    }
    EXPECT_EQ(index.First(cache, pins), cache.end());
    auto key = "outside-" + std::to_string(i);
    cache[key] = i;
    index.Stored(key, pins);
  }
  EXPECT_EQ(cache.size(), 9001u);
  for (const auto& key : pins) EXPECT_TRUE(cache.count(key));
}
