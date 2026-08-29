#include <gtest/gtest.h>

#include "gui/chart_safety_service.h"

namespace {

using ocpn::chart_safety::MayPrefetchNeighbour;
using ocpn::chart_safety::PlanTileBatchBlocks;
using ocpn::chart_safety::ExpandCandidateDiscoveryBounds;
using ocpn::chart_safety::GeographicBounds;
using ocpn::chart_safety::GlobalGridCoordinate;

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

TEST(ChartSafetyService, PlansProviderSizedGeographicBlocks) {
  std::set<std::pair<long, long>> tiles;
  for (long lat = 0; lat < 7; ++lat)
    for (long lon = 0; lon < 7; ++lon) tiles.insert({lat, lon});

  const auto blocks = PlanTileBatchBlocks(tiles, 6);
  ASSERT_EQ(blocks.size(), 4u);
  for (const auto& block : blocks) {
    EXPECT_LE(block.max_lat_tile - block.min_lat_tile + 1, 6);
    EXPECT_LE(block.max_lon_tile - block.min_lon_tile + 1, 6);
    const long rows =
        (block.max_lat_tile - block.min_lat_tile + 1) * 40 + 1;
    const long cols =
        (block.max_lon_tile - block.min_lon_tile + 1) * 40 + 1;
    EXPECT_LE(rows * cols, 65536);
  }
}

TEST(ChartSafetyService, NegativeTileIndexesStayInBoundedBlocks) {
  const std::set<std::pair<long, long>> tiles = {
      {-7, -7}, {-6, -6}, {-1, -1}, {0, 0}, {5, 5}, {6, 6}};
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
  EXPECT_DOUBLE_EQ(GlobalGridCoordinate(1089, cells_per_tile,
                                        cells_per_tile, resolution),
                   GlobalGridCoordinate(1090, 0, cells_per_tile,
                                        resolution));
  EXPECT_DOUBLE_EQ(GlobalGridCoordinate(-86, cells_per_tile,
                                        cells_per_tile, resolution),
                   GlobalGridCoordinate(-85, 0, cells_per_tile,
                                        resolution));

  // The result is also independent of a provider block's chosen origin.
  EXPECT_DOUBLE_EQ(GlobalGridCoordinate(1084, 6 * cells_per_tile,
                                        cells_per_tile, resolution),
                   GlobalGridCoordinate(1090, 0, cells_per_tile,
                                        resolution));
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

}  // namespace
