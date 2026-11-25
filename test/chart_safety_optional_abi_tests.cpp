/* GPL-2.0-or-later. Freeze layouts consumed by released optional-ABI plugins.
 */
#include <cstddef>
#include <type_traits>
#include "ocpn_plugin.h"
#define WEATHER_ROUTING_DECLARE_OPTIONAL_CHART_SAFETY_ABI
#include "fixtures/chart_safety_optional_abi.h"

static_assert(sizeof(PlugInSegmentSafetyOptions) ==
              sizeof(HostApi123::SegmentSafetyOptions));
static_assert(alignof(PlugInSegmentSafetyOptions) ==
              alignof(HostApi123::SegmentSafetyOptions));
static_assert(offsetof(PlugInSegmentSafetyOptions, struct_size) ==
              offsetof(HostApi123::SegmentSafetyOptions, struct_size));
static_assert(offsetof(PlugInSegmentSafetyOptions, safety_margin_nm) ==
              offsetof(HostApi123::SegmentSafetyOptions, safety_margin_nm));
static_assert(offsetof(PlugInSegmentSafetyOptions, check_land) ==
              offsetof(HostApi123::SegmentSafetyOptions, check_land));
static_assert(offsetof(PlugInSegmentSafetyOptions, allow_gshhs_fallback) ==
              offsetof(HostApi123::SegmentSafetyOptions, allow_gshhs_fallback));
static_assert(offsetof(PlugInSegmentSafetyOptions, check_depth) ==
              offsetof(HostApi123::SegmentSafetyOptions, check_depth));
static_assert(offsetof(PlugInSegmentSafetyOptions, minimum_depth_m) ==
              offsetof(HostApi123::SegmentSafetyOptions, minimum_depth_m));
static_assert(offsetof(PlugInSegmentSafetyOptions,
                       force_authoritative_fine_validation) ==
              offsetof(HostApi123::SegmentSafetyOptions,
                       force_authoritative_fine_validation));
static_assert(sizeof(PlugInSegmentSafetyResult) ==
              sizeof(HostApi123::SegmentSafetyResult));
static_assert(alignof(PlugInSegmentSafetyResult) ==
              alignof(HostApi123::SegmentSafetyResult));
static_assert(offsetof(PlugInSegmentSafetyResult, struct_size) ==
              offsetof(HostApi123::SegmentSafetyResult, struct_size));
static_assert(offsetof(PlugInSegmentSafetyResult, status) ==
              offsetof(HostApi123::SegmentSafetyResult, status));
static_assert(offsetof(PlugInSegmentSafetyResult, source) ==
              offsetof(HostApi123::SegmentSafetyResult, source));
static_assert(offsetof(PlugInSegmentSafetyResult, used_fallback) ==
              offsetof(HostApi123::SegmentSafetyResult, used_fallback));
static_assert(offsetof(PlugInSegmentSafetyResult, message) ==
              offsetof(HostApi123::SegmentSafetyResult, message));
static_assert(offsetof(PlugInSegmentSafetyResult, diagnostic_reason) ==
              offsetof(HostApi123::SegmentSafetyResult, diagnostic_reason));
static_assert(offsetof(PlugInSegmentSafetyResult, chart_stack_entries) ==
              offsetof(HostApi123::SegmentSafetyResult, chart_stack_entries));
static_assert(offsetof(PlugInSegmentSafetyResult, candidate_chart_count) ==
              offsetof(HostApi123::SegmentSafetyResult, candidate_chart_count));
static_assert(offsetof(PlugInSegmentSafetyResult, raster_chart_count) ==
              offsetof(HostApi123::SegmentSafetyResult, raster_chart_count));
static_assert(offsetof(PlugInSegmentSafetyResult, unsupported_chart_count) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       unsupported_chart_count));
static_assert(offsetof(PlugInSegmentSafetyResult, s57_chart_count) ==
              offsetof(HostApi123::SegmentSafetyResult, s57_chart_count));
static_assert(offsetof(PlugInSegmentSafetyResult, land_ring_count) ==
              offsetof(HostApi123::SegmentSafetyResult, land_ring_count));
static_assert(offsetof(PlugInSegmentSafetyResult, bbox_ring_tests) ==
              offsetof(HostApi123::SegmentSafetyResult, bbox_ring_tests));
static_assert(offsetof(PlugInSegmentSafetyResult, edge_tests) ==
              offsetof(HostApi123::SegmentSafetyResult, edge_tests));
static_assert(offsetof(PlugInSegmentSafetyResult, cache_build_ms) ==
              offsetof(HostApi123::SegmentSafetyResult, cache_build_ms));
static_assert(offsetof(PlugInSegmentSafetyResult, chart_select_ms) ==
              offsetof(HostApi123::SegmentSafetyResult, chart_select_ms));
static_assert(offsetof(PlugInSegmentSafetyResult, geometry_check_ms) ==
              offsetof(HostApi123::SegmentSafetyResult, geometry_check_ms));
static_assert(offsetof(PlugInSegmentSafetyResult, chart_db_index) ==
              offsetof(HostApi123::SegmentSafetyResult, chart_db_index));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_cause) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_cause));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_ring_min_lat) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_ring_min_lat));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_ring_max_lat) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_ring_max_lat));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_ring_min_lon) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_ring_min_lon));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_ring_max_lon) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_ring_max_lon));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_ring_point_count) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_ring_point_count));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_edge_index) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_edge_index));
static_assert(offsetof(PlugInSegmentSafetyResult, chart_path) ==
              offsetof(HostApi123::SegmentSafetyResult, chart_path));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_sample_lat) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_sample_lat));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_sample_lon) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_sample_lon));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_sample_index) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_sample_index));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_sample_count) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_sample_count));
static_assert(offsetof(PlugInSegmentSafetyResult, chart_scale) ==
              offsetof(HostApi123::SegmentSafetyResult, chart_scale));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_object) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_object));
static_assert(offsetof(PlugInSegmentSafetyResult, point_cache_hits) ==
              offsetof(HostApi123::SegmentSafetyResult, point_cache_hits));
static_assert(offsetof(PlugInSegmentSafetyResult, point_cache_misses) ==
              offsetof(HostApi123::SegmentSafetyResult, point_cache_misses));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cache_hits) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cache_hits));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cache_misses) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cache_misses));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_build_ms) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_build_ms));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cells_total) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cells_total));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cells_land) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cells_land));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cells_water) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cells_water));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cells_drying) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cells_drying));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cells_unknown) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cells_unknown));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_lookups) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_lookups));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_lookup_ms) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_lookup_ms));
static_assert(offsetof(PlugInSegmentSafetyResult, segment_sample_count) ==
              offsetof(HostApi123::SegmentSafetyResult, segment_sample_count));
static_assert(offsetof(PlugInSegmentSafetyResult, water_tile_shortcuts) ==
              offsetof(HostApi123::SegmentSafetyResult, water_tile_shortcuts));
static_assert(offsetof(PlugInSegmentSafetyResult, unexpected_tile_builds) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       unexpected_tile_builds));
static_assert(offsetof(PlugInSegmentSafetyResult, unexpected_lat_tile) ==
              offsetof(HostApi123::SegmentSafetyResult, unexpected_lat_tile));
static_assert(offsetof(PlugInSegmentSafetyResult, unexpected_lon_tile) ==
              offsetof(HostApi123::SegmentSafetyResult, unexpected_lon_tile));
static_assert(offsetof(PlugInSegmentSafetyResult, unexpected_tile_min_lat) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       unexpected_tile_min_lat));
static_assert(offsetof(PlugInSegmentSafetyResult, unexpected_tile_min_lon) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       unexpected_tile_min_lon));
static_assert(offsetof(PlugInSegmentSafetyResult, segment_cache_hits) ==
              offsetof(HostApi123::SegmentSafetyResult, segment_cache_hits));
static_assert(offsetof(PlugInSegmentSafetyResult, segment_cache_misses) ==
              offsetof(HostApi123::SegmentSafetyResult, segment_cache_misses));
static_assert(offsetof(PlugInSegmentSafetyResult, segment_cache_stores) ==
              offsetof(HostApi123::SegmentSafetyResult, segment_cache_stores));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cache_size) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cache_size));
static_assert(offsetof(PlugInSegmentSafetyResult, grid_cache_evictions) ==
              offsetof(HostApi123::SegmentSafetyResult, grid_cache_evictions));
static_assert(offsetof(PlugInSegmentSafetyResult, has_depth) ==
              offsetof(HostApi123::SegmentSafetyResult, has_depth));
static_assert(offsetof(PlugInSegmentSafetyResult, min_depth_m) ==
              offsetof(HostApi123::SegmentSafetyResult, min_depth_m));
static_assert(offsetof(PlugInSegmentSafetyResult, required_depth_m) ==
              offsetof(HostApi123::SegmentSafetyResult, required_depth_m));
static_assert(offsetof(PlugInSegmentSafetyResult, hit_depth_m) ==
              offsetof(HostApi123::SegmentSafetyResult, hit_depth_m));
static_assert(offsetof(PlugInSegmentSafetyResult, has_drying) ==
              offsetof(HostApi123::SegmentSafetyResult, has_drying));
static_assert(offsetof(PlugInSegmentSafetyResult, depth_source_object) ==
              offsetof(HostApi123::SegmentSafetyResult, depth_source_object));
static_assert(offsetof(PlugInSegmentSafetyResult, depth_source_attribute) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       depth_source_attribute));
static_assert(offsetof(PlugInSegmentSafetyResult, prewarm_requested_tiles) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       prewarm_requested_tiles));
static_assert(offsetof(PlugInSegmentSafetyResult, prewarm_base_tiles_built) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       prewarm_base_tiles_built));
static_assert(offsetof(PlugInSegmentSafetyResult, prewarm_base_tiles_reused) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       prewarm_base_tiles_reused));
static_assert(offsetof(PlugInSegmentSafetyResult, prewarm_masks_built) ==
              offsetof(HostApi123::SegmentSafetyResult, prewarm_masks_built));
static_assert(offsetof(PlugInSegmentSafetyResult, prewarm_masks_reused) ==
              offsetof(HostApi123::SegmentSafetyResult, prewarm_masks_reused));
static_assert(offsetof(PlugInSegmentSafetyResult, prewarm_fine_tiles_avoided) ==
              offsetof(HostApi123::SegmentSafetyResult,
                       prewarm_fine_tiles_avoided));
static_assert(sizeof(PlugInSegmentSafetyRequestServiceResult) ==
              sizeof(HostApi123::SegmentSafetyRequestServiceResult));
static_assert(alignof(PlugInSegmentSafetyRequestServiceResult) ==
              alignof(HostApi123::SegmentSafetyRequestServiceResult));
static_assert(offsetof(PlugInSegmentSafetyRequestServiceResult, struct_size) ==
              offsetof(HostApi123::SegmentSafetyRequestServiceResult,
                       struct_size));
static_assert(offsetof(PlugInSegmentSafetyRequestServiceResult,
                       pending_before) ==
              offsetof(HostApi123::SegmentSafetyRequestServiceResult,
                       pending_before));
static_assert(offsetof(PlugInSegmentSafetyRequestServiceResult,
                       requests_serviced) ==
              offsetof(HostApi123::SegmentSafetyRequestServiceResult,
                       requests_serviced));
static_assert(offsetof(PlugInSegmentSafetyRequestServiceResult, masks_built) ==
              offsetof(HostApi123::SegmentSafetyRequestServiceResult,
                       masks_built));
static_assert(offsetof(PlugInSegmentSafetyRequestServiceResult,
                       requests_failed) ==
              offsetof(HostApi123::SegmentSafetyRequestServiceResult,
                       requests_failed));
static_assert(offsetof(PlugInSegmentSafetyRequestServiceResult,
                       pending_after) ==
              offsetof(HostApi123::SegmentSafetyRequestServiceResult,
                       pending_after));
static_assert(offsetof(PlugInSegmentSafetyRequestServiceResult, elapsed_ms) ==
              offsetof(HostApi123::SegmentSafetyRequestServiceResult,
                       elapsed_ms));
static_assert(sizeof(PlugInSegmentSafetyTile) ==
              sizeof(HostApi123::SegmentSafetyTile));
static_assert(alignof(PlugInSegmentSafetyTile) ==
              alignof(HostApi123::SegmentSafetyTile));
static_assert(offsetof(PlugInSegmentSafetyTile, struct_size) ==
              offsetof(HostApi123::SegmentSafetyTile, struct_size));
static_assert(offsetof(PlugInSegmentSafetyTile, group_index) ==
              offsetof(HostApi123::SegmentSafetyTile, group_index));
static_assert(offsetof(PlugInSegmentSafetyTile, lat_tile) ==
              offsetof(HostApi123::SegmentSafetyTile, lat_tile));
static_assert(offsetof(PlugInSegmentSafetyTile, lon_tile) ==
              offsetof(HostApi123::SegmentSafetyTile, lon_tile));
static_assert(offsetof(PlugInSegmentSafetyTile, resolution) ==
              offsetof(HostApi123::SegmentSafetyTile, resolution));
static_assert(offsetof(PlugInSegmentSafetyTile, rows) ==
              offsetof(HostApi123::SegmentSafetyTile, rows));
static_assert(offsetof(PlugInSegmentSafetyTile, cols) ==
              offsetof(HostApi123::SegmentSafetyTile, cols));
static_assert(offsetof(PlugInSegmentSafetyTile, chart_db_index) ==
              offsetof(HostApi123::SegmentSafetyTile, chart_db_index));
static_assert(offsetof(PlugInSegmentSafetyTile, chart_scale) ==
              offsetof(HostApi123::SegmentSafetyTile, chart_scale));
static_assert(offsetof(PlugInSegmentSafetyTile, source) ==
              offsetof(HostApi123::SegmentSafetyTile, source));
static_assert(offsetof(PlugInSegmentSafetyTile, hazard_summary_flags) ==
              offsetof(HostApi123::SegmentSafetyTile, hazard_summary_flags));
static_assert(offsetof(PlugInSegmentSafetyTile, depth_complete) ==
              offsetof(HostApi123::SegmentSafetyTile, depth_complete));
static_assert(offsetof(PlugInSegmentSafetyTile, chart_path) ==
              offsetof(HostApi123::SegmentSafetyTile, chart_path));
static_assert(offsetof(PlugInSegmentSafetyTile, dependency_identity) ==
              offsetof(HostApi123::SegmentSafetyTile, dependency_identity));
static_assert(offsetof(PlugInSegmentSafetyTile, hazard_flags) ==
              offsetof(HostApi123::SegmentSafetyTile, hazard_flags));
static_assert(offsetof(PlugInSegmentSafetyTile, has_depth) ==
              offsetof(HostApi123::SegmentSafetyTile, has_depth));
static_assert(offsetof(PlugInSegmentSafetyTile, min_depth_m) ==
              offsetof(HostApi123::SegmentSafetyTile, min_depth_m));
static_assert(offsetof(PlugInSegmentSafetyTile, cell_capacity) ==
              offsetof(HostApi123::SegmentSafetyTile, cell_capacity));
static_assert(sizeof(PlugInSegmentSafetyTileCacheCallbacks) ==
              sizeof(HostApi123::SegmentSafetyTileCacheCallbacks));
static_assert(alignof(PlugInSegmentSafetyTileCacheCallbacks) ==
              alignof(HostApi123::SegmentSafetyTileCacheCallbacks));
static_assert(offsetof(PlugInSegmentSafetyTileCacheCallbacks, struct_size) ==
              offsetof(HostApi123::SegmentSafetyTileCacheCallbacks,
                       struct_size));
static_assert(offsetof(PlugInSegmentSafetyTileCacheCallbacks, context) ==
              offsetof(HostApi123::SegmentSafetyTileCacheCallbacks, context));
static_assert(offsetof(PlugInSegmentSafetyTileCacheCallbacks, lookup) ==
              offsetof(HostApi123::SegmentSafetyTileCacheCallbacks, lookup));
static_assert(offsetof(PlugInSegmentSafetyTileCacheCallbacks, store) ==
              offsetof(HostApi123::SegmentSafetyTileCacheCallbacks, store));
static_assert(offsetof(PlugInSegmentSafetyTileCacheCallbacks,
                       identity_changed) ==
              offsetof(HostApi123::SegmentSafetyTileCacheCallbacks,
                       identity_changed));
static_assert(offsetof(PlugInSegmentSafetyTileCacheCallbacks,
                       dependencies_changed) ==
              offsetof(HostApi123::SegmentSafetyTileCacheCallbacks,
                       dependencies_changed));
static_assert(sizeof(PlugInSegmentSafetyChartInfoV1) ==
              sizeof(HostApi123::SegmentSafetyChartInfo));
static_assert(alignof(PlugInSegmentSafetyChartInfoV1) ==
              alignof(HostApi123::SegmentSafetyChartInfo));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, struct_size) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, struct_size));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, abi_version) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, abi_version));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, db_index) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, db_index));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, chart_type) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, chart_type));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, chart_family) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, chart_family));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, chart_scale) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, chart_scale));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, source) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, source));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, available) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, available));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, in_active_group) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, in_active_group));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, edition_time) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, edition_time));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, file_time) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, file_time));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, min_lat) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, min_lat));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, min_lon) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, min_lon));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, max_lat) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, max_lat));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, max_lon) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, max_lon));
static_assert(offsetof(PlugInSegmentSafetyChartInfoV1, chart_path) ==
              offsetof(HostApi123::SegmentSafetyChartInfo, chart_path));
