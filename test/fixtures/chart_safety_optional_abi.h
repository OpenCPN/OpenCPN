/***************************************************************************
 * Optional Weather Routing chart-safety host ABI.
 *
 * These declarations describe an experimental, dynamically resolved OpenCPN
 * capability.  They deliberately contain no imported function declarations:
 * stock OpenCPN does not provide the capability and must remain a valid host.
 ***************************************************************************/

#ifndef WEATHER_ROUTING_OPTIONAL_CHART_SAFETY_API_H
#define WEATHER_ROUTING_OPTIONAL_CHART_SAFETY_API_H

#ifdef WEATHER_ROUTING_DECLARE_OPTIONAL_CHART_SAFETY_ABI

enum PlugInSegmentSafetyStatus {
  PI_SEGMENT_SAFETY_SAFE = 0,
  PI_SEGMENT_SAFETY_CROSSES_LAND,
  PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN,
  PI_SEGMENT_SAFETY_UNSAFE_AREA,
  PI_SEGMENT_SAFETY_NO_DATA,
  PI_SEGMENT_SAFETY_ERROR,
  PI_SEGMENT_SAFETY_DRYING_AREA,
  PI_SEGMENT_SAFETY_TOO_SHALLOW,
  PI_SEGMENT_SAFETY_UNKNOWN_DEPTH,
  PI_SEGMENT_SAFETY_PENDING_DATA
};

enum PlugInSegmentSafetySource {
  PI_SEGMENT_SAFETY_SOURCE_NONE = 0,
  PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART,
  PI_SEGMENT_SAFETY_SOURCE_CM93,
  PI_SEGMENT_SAFETY_SOURCE_GSHHS_FALLBACK,
  PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR
};

enum PlugInSegmentSafetyDiagnosticReason {
  PI_SEGMENT_SAFETY_DIAG_NONE = 0,
  PI_SEGMENT_SAFETY_DIAG_NO_CHART_DATABASE,
  PI_SEGMENT_SAFETY_DIAG_NO_CANDIDATE_CHART,
  PI_SEGMENT_SAFETY_DIAG_RASTER_ONLY,
  PI_SEGMENT_SAFETY_DIAG_UNSUPPORTED_CHART_TYPE,
  PI_SEGMENT_SAFETY_DIAG_CHART_LOAD_FAILED,
  PI_SEGMENT_SAFETY_DIAG_NO_LANDARE_GEOMETRY,
  PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR,
  PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_HIT,
  PI_SEGMENT_SAFETY_DIAG_GSHHS_FALLBACK,
  PI_SEGMENT_SAFETY_DIAG_PENDING_DATA
};

enum PlugInSegmentSafetyHitCause {
  PI_SEGMENT_SAFETY_HIT_NONE = 0,
  PI_SEGMENT_SAFETY_HIT_ENDPOINT_IN_LANDARE,
  PI_SEGMENT_SAFETY_HIT_SEGMENT_INTERSECTS_LANDARE_EDGE,
  PI_SEGMENT_SAFETY_HIT_MARGIN_TO_LANDARE_EDGE
};

struct PlugInSegmentSafetyOptions {
  int struct_size;
  double safety_margin_nm;
  int check_land;
  int allow_gshhs_fallback;
  int check_depth;
  double minimum_depth_m;
  int force_authoritative_fine_validation;
};

struct PlugInSegmentSafetyResult {
  int struct_size;
  int status;
  int source;
  int used_fallback;
  char message[256];
  int diagnostic_reason;
  int chart_stack_entries;
  int candidate_chart_count;
  int raster_chart_count;
  int unsupported_chart_count;
  int s57_chart_count;
  int land_ring_count;
  int bbox_ring_tests;
  int edge_tests;
  int cache_build_ms;
  int chart_select_ms;
  int geometry_check_ms;
  int chart_db_index;
  int hit_cause;
  double hit_ring_min_lat;
  double hit_ring_max_lat;
  double hit_ring_min_lon;
  double hit_ring_max_lon;
  int hit_ring_point_count;
  int hit_edge_index;
  char chart_path[256];
  double hit_sample_lat;
  double hit_sample_lon;
  int hit_sample_index;
  int hit_sample_count;
  int chart_scale;
  char hit_object[128];
  int point_cache_hits;
  int point_cache_misses;
  int grid_cache_hits;
  int grid_cache_misses;
  int grid_build_ms;
  int grid_cells_total;
  int grid_cells_land;
  int grid_cells_water;
  int grid_cells_drying;
  int grid_cells_unknown;
  int grid_lookups;
  int grid_lookup_ms;
  int segment_sample_count;
  int water_tile_shortcuts;
  int unexpected_tile_builds;
  int unexpected_lat_tile;
  int unexpected_lon_tile;
  double unexpected_tile_min_lat;
  double unexpected_tile_min_lon;
  int segment_cache_hits;
  int segment_cache_misses;
  int segment_cache_stores;
  int grid_cache_size;
  int grid_cache_evictions;
  int has_depth;
  double min_depth_m;
  double required_depth_m;
  double hit_depth_m;
  int has_drying;
  char depth_source_object[128];
  char depth_source_attribute[32];
  int prewarm_requested_tiles;
  int prewarm_base_tiles_built;
  int prewarm_base_tiles_reused;
  int prewarm_masks_built;
  int prewarm_masks_reused;
  int prewarm_fine_tiles_avoided;
};

struct PlugInSegmentSafetyRequestServiceResult {
  int struct_size;
  int pending_before;
  int requests_serviced;
  int masks_built;
  int requests_failed;
  int pending_after;
  int elapsed_ms;
};

struct PlugInSegmentSafetyTile {
  int struct_size;
  int group_index;
  long lat_tile;
  long lon_tile;
  double resolution;
  int rows;
  int cols;
  int chart_db_index;
  int chart_scale;
  int source;
  unsigned int hazard_summary_flags;
  int depth_complete;
  char chart_path[256];
  char dependency_identity[80];
  unsigned short* hazard_flags;
  unsigned char* has_depth;
  float* min_depth_m;
  int cell_capacity;
};

typedef int (*PlugInSegmentSafetyTileCacheLookupFn)(
    void* context, long lat_tile, long lon_tile, int require_depth,
    PlugInSegmentSafetyTile* tile);
typedef void (*PlugInSegmentSafetyTileCacheStoreFn)(
    void* context, const PlugInSegmentSafetyTile* tile);
typedef void (*PlugInSegmentSafetyTileCacheIdentityFn)(void* context,
                                                       const char* identity);
typedef void (*PlugInSegmentSafetyTileCacheDependenciesChangedFn)(
    void* context);

struct PlugInSegmentSafetyTileCacheCallbacks {
  int struct_size;
  void* context;
  PlugInSegmentSafetyTileCacheLookupFn lookup;
  PlugInSegmentSafetyTileCacheStoreFn store;
  PlugInSegmentSafetyTileCacheIdentityFn identity_changed;
  PlugInSegmentSafetyTileCacheDependenciesChangedFn dependencies_changed;
};

#define PI_SEGMENT_SAFETY_CHART_INFO_ABI_V1 1

struct PlugInSegmentSafetyChartInfoV1 {
  int struct_size;
  int abi_version;
  int db_index;
  int chart_type;
  int chart_family;
  int chart_scale;
  int source;
  int available;
  int in_active_group;
  long long edition_time;
  long long file_time;
  double min_lat;
  double min_lon;
  double max_lat;
  double max_lon;
  char chart_path[512];
};

#define PI_SEGMENT_SAFETY_COVERAGE_TILES_ABI_V1 1

#endif  // WEATHER_ROUTING_DECLARE_OPTIONAL_CHART_SAFETY_ABI

#endif  // WEATHER_ROUTING_OPTIONAL_CHART_SAFETY_API_H
