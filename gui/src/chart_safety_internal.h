/***************************************************************************
 * Internal chart-safety engine types shared by focused implementation files.
 ***************************************************************************/

#ifndef GUI_SRC_CHART_SAFETY_INTERNAL_H_
#define GUI_SRC_CHART_SAFETY_INTERNAL_H_

#include <climits>
#include <cstdint>
#include <ctime>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/thread.h>

#include "chart_safety_api.h"

class ChartBase;
class ChartPlugInWrapper;
struct _ObjRazRules;
using ObjRazRules = _ObjRazRules;
class PI_S57Obj;
class ViewPort;
class s57chart;

namespace ocpn::chart_safety::detail {

enum SegmentSafetyPointClass {
  kPointNoData = 0,
  kPointWater,
  kPointLand,
  kPointDrying
};

enum SegmentSafetyHazardFlags : uint16_t {
  kHazardNone = 0,
  kHazardLand = 1 << 0,
  kHazardDrying = 1 << 1,
  kHazardNoChart = 1 << 4,
  kHazardUnknownClass = 1 << 5
};

enum SegmentSafetyRouteBlockFlags : uint16_t {
  kRouteClear = 0,
  kRouteBlockLand = 1 << 0,
  kRouteBlockDrying = 1 << 1,
  kRouteBlockTooShallow = 1 << 2,
  kRouteBlockUnknownDepth = 1 << 3,
  kRouteBlockNoChart = 1 << 4,
  kRouteBlockUnknownClass = 1 << 5,
  kRouteBlockMargin = 1 << 6,
  kRouteNeedsTile = 1 << 15
};

struct SegmentSafetyBBox {
  double min_lat{0.0};
  double max_lat{0.0};
  double min_lon{0.0};
  double max_lon{0.0};
};

struct CachedLandRing {
  std::vector<wxPoint2DDouble> points;
  SegmentSafetyBBox bbox;
};

struct CachedChartLandGeometry {
  bool loaded{false};
  SegmentSafetySource source{kSourceNone};
  std::string cache_key;
  wxString chart_path;
  std::vector<CachedLandRing> rings;
};

enum SegmentSafetySnapshotDecision { kSnapshotUnknown = 0, kSnapshotSafe };

struct SegmentSafetySnapshotChart {
  int db_index{-1};
  int native_scale{INT_MAX};
  time_t edition_date{0};
  time_t file_time{0};
  bool vector_supported{false};
  SegmentSafetySource source{kSourceNone};
  wxString chart_path;
  SegmentSafetyBBox bbox;
  std::vector<CachedLandRing> coverage;
  std::vector<CachedLandRing> no_coverage;
  std::vector<CachedLandRing> hazards;
};

struct SegmentSafetyHazardSnapshot {
  wxString chart_identity;
  int group_index{0};
  SegmentSafetyBBox area;
  bool fast_path_enabled{false};
  bool shadow_compare{true};
  std::vector<SegmentSafetySnapshotChart> charts;
  std::set<std::pair<long, long>> certified_safe_large_cells;
  std::set<std::pair<long, long>> certified_safe_fine_cells;
  long coverage_rings{0};
  long hazard_rings{0};
};

struct CachedPointSafetyClassification {
  SegmentSafetyPointClass point_class{kPointNoData};
  SegmentSafetySource source{kSourceNone};
  int chart_db_index{-1};
  int chart_scale{-1};
  bool has_depth{false};
  double min_depth_m{0.0};
  bool has_drying{false};
  char chart_path[256]{};
  char hit_object[128]{};
  char depth_source_object[128]{};
  char depth_source_attribute[32]{};
};

inline constexpr double kGridTileDegrees = 0.05;
inline constexpr double kGridResolutionDegrees = 0.00125;
inline constexpr int kCoarseRouteMaskFactor = 4;
inline constexpr int kPersistentCacheVersion = 3;
inline constexpr int kPersistentBaseTileVersion = 4;
inline constexpr int kRouteMaskAlgorithmVersion = 5;
inline constexpr size_t kMaxGridTiles = 4096;
inline constexpr size_t kMaxPersistentGridTiles = 8192;
inline constexpr size_t kMaxPersistentCertifiedCells = 32768;
inline constexpr long kPersistentCheckpointTiles = 64;
inline constexpr size_t kMaxSegmentCacheEntries = 100000;
inline constexpr size_t kMaxPointCacheEntries = 250000;
inline constexpr long kMaxChartHitLogs = 20;
inline constexpr long kMaxWorkerTileMissLogs = 20;
inline constexpr long kMaxQueryLogs = 160;

struct CachedPointSafetyGridTile {
  int group_index{0};
  long lat_tile{0};
  long lon_tile{0};
  double min_lat{0.0};
  double min_lon{0.0};
  double resolution{kGridResolutionDegrees};
  int rows{0};
  int cols{0};
  int land_count{0};
  int water_count{0};
  int drying_count{0};
  int unknown_count{0};
  bool built{false};
  int chart_db_index{-1};
  int chart_scale{-1};
  SegmentSafetySource source{kSourceNone};
  char chart_path[256]{};
  char dependency_identity[80]{};
  std::vector<unsigned char> classes;
  std::vector<uint16_t> hazard_flags;
  std::vector<unsigned char> has_depth;
  std::vector<float> min_depth_m;
  std::vector<unsigned char> has_drying;
  uint32_t hazard_summary_flags{kHazardNone};
  bool persistent_loaded{false};
  bool depth_complete{true};
  bool persistent_cache_allowed{false};
};

struct CachedSegmentSafetyRouteMaskTile {
  int group_index{0};
  long lat_tile{0};
  long lon_tile{0};
  double min_lat{0.0};
  double min_lon{0.0};
  double resolution{kGridResolutionDegrees};
  int rows{0};
  int cols{0};
  bool check_depth{false};
  double minimum_depth_m{0.0};
  double safety_margin_nm{0.0};
  int margin_cells{0};
  bool built{false};
  SegmentSafetySource source{kSourceNone};
  int chart_db_index{-1};
  int chart_scale{-1};
  char chart_path[256]{};
  std::vector<uint16_t> block_flags;
  uint32_t block_summary_flags{kRouteClear};
  int clear_count{0};
  int land_count{0};
  int drying_count{0};
  int shallow_count{0};
  int unknown_depth_count{0};
  int no_chart_count{0};
  int margin_count{0};
  bool authoritative_fine{false};
  bool persistent_certified_safe{false};
  bool uses_plugin_vector{false};
  bool persistent_cache_allowed{false};
};

struct SegmentSafetyRouteMaskRequest {
  std::string key;
  int group_index{0};
  long lat_tile{0};
  long lon_tile{0};
  double safety_margin_nm{0.0};
  bool check_depth{false};
  double minimum_depth_m{0.0};
  bool force_authoritative_fine{false};
};

enum SegmentSafetyCoarseRouteMaskState {
  kCoarseCertifiedSafe = 0,
  kCoarseMixed,
  kCoarseNoChart,
  kCoarseDepthUnproven,
  kCoarseMissing
};

struct CachedSegmentSafetyCoarseRouteMaskCell {
  int group_index{0};
  long lat_cell{0};
  long lon_cell{0};
  double min_lat{0.0};
  double min_lon{0.0};
  double degrees{kGridTileDegrees * kCoarseRouteMaskFactor};
  bool check_depth{false};
  double minimum_depth_m{0.0};
  double safety_margin_nm{0.0};
  SegmentSafetyCoarseRouteMaskState state{kCoarseMissing};
  uint32_t block_summary_flags{kRouteNeedsTile};
  int fine_tiles_checked{0};
  int fine_tiles_clear{0};
  int fine_tiles_mixed{0};
  SegmentSafetySource source{kSourceNone};
  bool persistent_cache_allowed{true};
};

struct CachedSegmentSafetyResult {
  int status{kError};
  int source{kSourceNone};
  int diagnostic_reason{kDiagnosticNone};
  int chart_db_index{-1};
  int chart_scale{-1};
  int hit_sample_index{-1};
  int hit_sample_count{0};
  double hit_sample_lat{0.0};
  double hit_sample_lon{0.0};
  int has_depth{0};
  double min_depth_m{0.0};
  double required_depth_m{0.0};
  double hit_depth_m{0.0};
  int has_drying{0};
  char message[256]{};
  char chart_path[256]{};
  char hit_object[128]{};
  char depth_source_object[128]{};
  char depth_source_attribute[32]{};
};

struct SegmentSafetyCoreStats {
  int chart_stack_entries{0};
  int candidate_chart_count{0};
  int raster_chart_count{0};
  int unsupported_chart_count{0};
  int s57_chart_count{0};
  int land_ring_count{0};
  int bbox_ring_tests{0};
  int edge_tests{0};
  int cache_build_ms{0};
  int chart_select_ms{0};
  int geometry_check_ms{0};
  int point_cache_hits{0};
  int point_cache_misses{0};
  int grid_cache_hits{0};
  int grid_cache_misses{0};
  int grid_build_ms{0};
  int grid_cells_total{0};
  int grid_cells_land{0};
  int grid_cells_water{0};
  int grid_cells_drying{0};
  int grid_cells_unknown{0};
  int grid_lookups{0};
  int grid_lookup_ms{0};
  int segment_sample_count{0};
  int water_tile_shortcuts{0};
  int segment_cache_hits{0};
  int segment_cache_misses{0};
  int segment_cache_stores{0};
  int unexpected_tile_builds{0};
  int coarse_cells_checked{0};
  int coarse_certified_safe_hits{0};
  int coarse_mixed_fallbacks{0};
  int coarse_unknown_fallbacks{0};
  int coarse_no_chart{0};
  int coarse_depth_unproven{0};
  int coarse_missing{0};
  int fine_tiles_avoided{0};
  int coarse_build_ms{0};
  int unexpected_lat_tile{0};
  int unexpected_lon_tile{0};
  double unexpected_tile_min_lat{0.0};
  double unexpected_tile_min_lon{0.0};
  bool no_chart_database{false};
  bool chart_load_failed{false};
  bool zero_land_geometry{false};
};

struct SegmentSafetyChartCandidate {
  int db_index{-1};
  int provider_priority{INT_MAX};
  int native_scale{INT_MAX};
  time_t edition_date{0};
  time_t file_time{0};
  bool plugin_vector{false};
  bool cm93{false};
  std::string path;
};

struct SegmentSafetyPluginBatchGroup {
  int db_index{-1};
  ChartPlugInWrapper *wrapper{nullptr};
  std::vector<uint8_t> active;
  std::vector<uint8_t> land;
  std::vector<uint8_t> drying;
  std::vector<uint8_t> has_depth;
  std::vector<uint8_t> unknown_danger_depth;
  std::vector<float> min_depth_m;
  int active_count{0};

  explicit SegmentSafetyPluginBatchGroup(size_t cells = 0)
      : active(cells, 0),
        land(cells, 0),
        drying(cells, 0),
        has_depth(cells, 0),
        unknown_danger_depth(cells, 0),
        min_depth_m(cells, 0.0f) {}
};

extern std::map<std::string, CachedChartLandGeometry>
    s_segment_safety_land_cache;
extern std::shared_ptr<const SegmentSafetyHazardSnapshot>
    s_segment_safety_hazard_snapshot;
extern long s_segment_safety_snapshot_queries;
extern long s_segment_safety_snapshot_safe;
extern long s_segment_safety_snapshot_unknown;
extern long s_segment_safety_snapshot_shadow_disagreements;
extern std::map<std::string, CachedPointSafetyClassification>
    s_segment_safety_point_cache;
extern std::map<std::string, CachedPointSafetyGridTile>
    s_segment_safety_grid_cache;
extern std::set<std::string> s_segment_safety_pinned_grid_keys;
extern std::map<std::string, CachedPointSafetyGridTile>
    s_segment_safety_persistent_base_tile_cache;
extern SegmentSafetyTileCacheCallbacks s_segment_safety_external_tile_cache;
extern std::map<std::string, CachedSegmentSafetyRouteMaskTile>
    s_segment_safety_route_mask_cache;
extern std::set<std::string> s_segment_safety_pinned_route_mask_keys;
extern std::map<std::string, SegmentSafetyRouteMaskRequest>
    s_segment_safety_pending_route_mask_requests;
extern std::set<std::string> s_segment_safety_inflight_route_mask_requests;
extern std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>
    s_segment_safety_coarse_route_mask_cache;
extern std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>
    s_segment_safety_persistent_certified_safe_cache;
extern std::map<std::string, CachedSegmentSafetyResult>
    s_segment_safety_segment_cache;
extern bool s_segment_safety_persistent_cache_enabled;
extern bool s_segment_safety_persistent_cache_loaded;
extern bool s_segment_safety_persistent_cache_dirty;
extern bool s_segment_safety_persistent_base_tiles_loaded;
extern bool s_segment_safety_persistent_base_tiles_dirty;
extern wxString s_segment_safety_chart_identity;
extern wxString s_segment_safety_chart_catalog_identity;
extern long s_segment_safety_persistent_entries_loaded;
extern long s_segment_safety_persistent_entries_saved;
extern long s_segment_safety_persistent_entries_used;
extern long s_segment_safety_persistent_entries_ignored;
extern long s_segment_safety_persistent_stale_ignored;
extern long s_segment_safety_persistent_malformed_ignored;
extern long s_segment_safety_persistent_entries_stored;
extern long s_segment_safety_persistent_base_tiles_loaded_count;
extern long s_segment_safety_persistent_base_tiles_saved;
extern long s_segment_safety_persistent_base_tiles_used;
extern long s_segment_safety_persistent_base_tiles_ignored;
extern long s_segment_safety_persistent_tiles_since_checkpoint;
extern long s_segment_safety_grid_cache_evictions;
extern long s_segment_safety_chart_hit_logs;
extern long s_segment_safety_worker_tile_miss_logs;
extern long s_segment_safety_query_logs;
extern wxMutex s_segment_safety_cache_mutex;

// Cross-module engine operations.
void CopySegmentSafetyString(char *dest, size_t dest_size, const char *source);
void InitSegmentSafetyResult(SegmentSafetyResult *result);
void SetSegmentSafetyMessage(SegmentSafetyResult *result, const char *message);
void SetSegmentSafetyStatus(SegmentSafetyResult *result,
                            SegmentSafetyStatus status);
void SetSegmentSafetySource(SegmentSafetyResult *result,
                            SegmentSafetySource source);
void SetSegmentSafetyFallback(SegmentSafetyResult *result, bool used_fallback);
SegmentSafetySource GetSegmentSafetySource(const SegmentSafetyResult *result);
bool SegmentSafetyResultHas(const SegmentSafetyResult *result, size_t offset,
                            size_t field_size);
void SetSegmentSafetyDiagnosticReason(SegmentSafetyResult *result,
                                      SegmentSafetyDiagnosticReason reason);
void SetSegmentSafetyDiagnosticInt(SegmentSafetyResult *result, size_t offset,
                                   int value);
void SetSegmentSafetyDiagnosticDouble(SegmentSafetyResult *result,
                                      size_t offset, double value);
double SegmentSafetyOptionMargin(const SegmentSafetyOptions *options);
bool SegmentSafetyOptionCheckLand(const SegmentSafetyOptions *options);
bool SegmentSafetyOptionAllowGshhsFallback(const SegmentSafetyOptions *options);
bool SegmentSafetyOptionCheckDepth(const SegmentSafetyOptions *options);
double SegmentSafetyOptionMinimumDepthM(const SegmentSafetyOptions *options);
bool SegmentSafetyOptionForceAuthoritativeFineValidation(
    const SegmentSafetyOptions *options);

ViewPort SegmentSafetyViewPortAt(double lat, double lon);
ViewPort SegmentSafetyHighestDetailViewPortAt(double lat, double lon);
double SegmentSafetyNormalizeBearing(double bearing);
bool IsCm93Chart(ChartBase *chart);
bool IsSupportedSegmentSafetyPluginChart(ChartBase *chart);
wxString SegmentSafetyRuleSummary(ObjRazRules *rule);
bool SegmentSafetyRuleDepthMinM(ObjRazRules *rule, double *depth_m);
bool SegmentSafetyRuleIsDrying(ObjRazRules *rule);
bool SegmentSafetyRuleIsAlwaysDry(ObjRazRules *rule);
bool SegmentSafetyRuleDangerDepthM(ObjRazRules *rule, double *depth_m,
                                   bool *unknown_depth);
wxString SegmentSafetyPluginObjectSummary(PI_S57Obj *obj);
bool SegmentSafetyPluginObjectIsDrying(PI_S57Obj *obj);
bool SegmentSafetyPluginObjectIsAlwaysDry(PI_S57Obj *obj);
bool SegmentSafetyPluginObjectDepthM(PI_S57Obj *obj, double *depth_m,
                                     wxString *source_attribute,
                                     bool *unknown_danger_depth);

int SegmentSafetyCurrentGroupIndex();
void SegmentSafetyHashAdd(uint64_t *hash, const wxString &text);
wxString SegmentSafetyChartIdentity();
void SegmentSafetyRefreshPersistentChartIdentity();
void SegmentSafetyPersistentCacheEnsureLoaded();
bool SegmentSafetyPersistentCacheSave();
std::string SegmentSafetyPointCacheKey(double lat, double lon);
std::string SegmentSafetyGridTileKeyForIndices(long lat_tile, long lon_tile);
std::string SegmentSafetyGridTileKey(double lat, double lon, long *lat_tile,
                                     long *lon_tile);
std::string SegmentSafetyRouteMaskKey(long lat_tile, long lon_tile,
                                      double safety_margin_nm, bool check_depth,
                                      double minimum_depth_m);
bool SegmentSafetyExternalTileCacheLookup(long lat_tile, long lon_tile,
                                          bool require_depth,
                                          CachedPointSafetyGridTile *tile);
void SegmentSafetyExternalTileCacheStore(const CachedPointSafetyGridTile &tile);
void StoreSegmentSafetyGridTile(const std::string &key,
                                const CachedPointSafetyGridTile &tile);
bool LookupSegmentSafetyGridTile(const std::string &key,
                                 CachedPointSafetyGridTile *tile);
size_t SegmentSafetyGridCacheSize();
bool EnsureSegmentSafetyGridTile(long lat_tile, long lon_tile,
                                 SegmentSafetyCoreStats *stats, bool *built,
                                 bool require_depth = false);
bool LookupSegmentSafetyRouteMaskTile(const std::string &key,
                                      CachedSegmentSafetyRouteMaskTile *tile);
bool EnsureSegmentSafetyRouteMaskTile(long lat_tile, long lon_tile,
                                      double safety_margin_nm, bool check_depth,
                                      double minimum_depth_m,
                                      SegmentSafetyCoreStats *stats,
                                      bool *built,
                                      bool force_authoritative_fine = false);
bool SegmentSafetyCachedTileProviderIsCurrent(
    const CachedPointSafetyGridTile &tile);
wxString SegmentSafetyTileDependencyIdentity(long lat_tile, long lon_tile);
bool SegmentSafetyTileDependencyIsCurrent(
    const CachedPointSafetyGridTile &tile);
uint16_t SegmentSafetyPointHazardFlags(SegmentSafetyPointClass point_class);
double SegmentSafetyDegToRad(double degrees);
SegmentSafetyBBox SegmentSafetyRingBBox(
    const std::vector<wxPoint2DDouble> &points);
SegmentSafetyBBox SegmentSafetySegmentBBox(double lat1, double lon1,
                                           double lat2, double lon2,
                                           double margin_nm);
bool SegmentSafetyBBoxIntersects(const SegmentSafetyBBox &a,
                                 const SegmentSafetyBBox &b);
bool SegmentSafetyPointInRing(double lat, double lon,
                              const std::vector<wxPoint2DDouble> &ring);
bool SegmentSafetySegmentsIntersect(const wxPoint2DDouble &a,
                                    const wxPoint2DDouble &b,
                                    const wxPoint2DDouble &c,
                                    const wxPoint2DDouble &d);
double SegmentSafetySegmentDistanceNm(const wxPoint2DDouble &a,
                                      const wxPoint2DDouble &b,
                                      const wxPoint2DDouble &c,
                                      const wxPoint2DDouble &d);
CachedChartLandGeometry *LoadSegmentSafetyChartLandGeometry(
    s57chart *chart, int db_index, bool cm93, SegmentSafetyCoreStats *stats);
void ApplySegmentSafetyStats(SegmentSafetyResult *result,
                             const SegmentSafetyCoreStats &stats);
SegmentSafetyDiagnosticReason SegmentSafetyUnavailableReason(
    const SegmentSafetyCoreStats &stats);
const char *SegmentSafetyUnavailableMessage(
    SegmentSafetyDiagnosticReason reason);
void CopySegmentSafetyPointCacheToResult(
    const CachedPointSafetyClassification &cached, SegmentSafetyResult *result);
bool LookupSegmentSafetyPointCache(const std::string &key,
                                   CachedPointSafetyClassification *cached);
CachedPointSafetyGridTile BuildSegmentSafetyGridTile(
    long lat_tile, long lon_tile, SegmentSafetyCoreStats *stats,
    bool require_depth);
bool GshhsSegmentSafetyHitsLand(double lat1, double lon1, double lat2,
                                double lon2, double safety_margin_nm,
                                SegmentSafetyStatus *status);
wxString SegmentSafetyPointDiagnostic(double lat, double lon);

std::string SegmentSafetyCacheKey(int db_index, bool cm93, double lat,
                                  double lon);
std::string SegmentSafetySegmentCacheKey(double lat1, double lon1, double lat2,
                                         double lon2, double safety_margin_nm,
                                         bool check_depth,
                                         double minimum_depth_m);
void StoreSegmentSafetySegmentCache(const std::string &key,
                                    const CachedSegmentSafetyResult &cached,
                                    SegmentSafetyCoreStats *stats);
bool LookupSegmentSafetySegmentCache(const std::string &key,
                                     CachedSegmentSafetyResult *cached);
void CopyCachedSegmentSafetyToResult(const CachedSegmentSafetyResult &cached,
                                     SegmentSafetyResult *result);
CachedSegmentSafetyResult MakeCachedSegmentSafetyResult(
    const SegmentSafetyResult *result, int diagnostic_reason_override);
void StoreSegmentSafetyPointCache(
    const std::string &key, const CachedPointSafetyClassification &cached);
CachedPointSafetyClassification MakeSegmentSafetyPointCacheEntry(
    SegmentSafetyPointClass point_class, SegmentSafetySource source,
    int chart_db_index, int chart_scale, const char *chart_path,
    const char *hit_object, bool has_depth = false, double min_depth_m = 0.0,
    bool has_drying = false, const char *depth_source_object = nullptr,
    const char *depth_source_attribute = nullptr);

CachedLandRing SegmentSafetyRingFromFloatTable(const float *points, int count);
void SegmentSafetyAppendFeatureRings(s57chart *chart, const char *feature_name,
                                     std::vector<CachedLandRing> *rings,
                                     std::set<std::string> *seen = nullptr);
void BuildSegmentSafetySnapshotHierarchy(SegmentSafetyHazardSnapshot *snapshot);
SegmentSafetySnapshotDecision QuerySegmentSafetyHazardSnapshot(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth);

void PinSegmentSafetyRouteMaskTiles(
    const std::set<std::pair<long, long>> &tiles, double safety_margin_nm,
    bool check_depth, double minimum_depth_m);
void PinSegmentSafetyRouteMaskEnvelope(long min_lat_tile, long max_lat_tile,
                                       long min_lon_tile, long max_lon_tile,
                                       double safety_margin_nm,
                                       bool check_depth,
                                       double minimum_depth_m);
std::string SegmentSafetyCoarseRouteMaskKey(long lat_cell, long lon_cell,
                                            double safety_margin_nm,
                                            bool check_depth,
                                            double minimum_depth_m);
void StoreSegmentSafetyRouteMaskTile(
    const std::string &key, const CachedSegmentSafetyRouteMaskTile &tile);
void StoreSegmentSafetyCoarseRouteMaskCell(
    const std::string &key, const CachedSegmentSafetyCoarseRouteMaskCell &cell);
bool LookupSegmentSafetyCoarseRouteMaskCell(
    const std::string &key, CachedSegmentSafetyCoarseRouteMaskCell *cell);
bool EnsureSegmentSafetyCoarseRouteMaskCell(
    long lat_cell, long lon_cell, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, CachedSegmentSafetyCoarseRouteMaskCell *cell,
    SegmentSafetyCoreStats *stats);
bool LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
    long lat_tile, long lon_tile, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, CachedSegmentSafetyCoarseRouteMaskCell *cell);
CachedSegmentSafetyRouteMaskTile BuildPersistentCertifiedSafeRouteMaskTile(
    long lat_tile, long lon_tile, double safety_margin_nm, bool check_depth,
    double minimum_depth_m,
    const CachedSegmentSafetyCoarseRouteMaskCell &coarse);
bool BuildSegmentSafetyCoarseRouteMaskCellFromBase(
    long lat_cell, long lon_cell, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, bool allow_chart_builds,
    CachedSegmentSafetyCoarseRouteMaskCell *cell, SegmentSafetyCoreStats *stats,
    long *base_tiles_built);
bool SegmentSafetyCoarseProofDoesNotExpandBaseBuildSet(
    long lat_cell, long lon_cell,
    const std::set<std::pair<long, long>> &occupied_tiles,
    double safety_margin_nm);
bool SegmentSafetyPersistentLookupCertifiedSafe(
    long lat_cell, long lon_cell, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, CachedSegmentSafetyCoarseRouteMaskCell *cell);
void SegmentSafetyPersistentStoreCertifiedSafe(
    const CachedSegmentSafetyCoarseRouteMaskCell &cell);
wxString SegmentSafetyPersistentCachePath();
wxString SegmentSafetyPersistentBaseTileCachePath();
void RecordUnexpectedSegmentSafetyTileBuild(SegmentSafetyCoreStats *stats,
                                            long lat_tile, long lon_tile);
int SegmentSafetyMarginTileRadius(double safety_margin_nm, double max_abs_lat);
void AddSegmentSafetyTileHalo(long lat_tile, long lon_tile, int radius,
                              std::set<std::pair<long, long>> *tiles);

std::set<std::pair<long, long>> PrebuildSegmentSafetyPluginVectorGridTiles(
    const std::set<std::pair<long, long>> &requested_tiles,
    SegmentSafetyCoreStats *stats, bool require_depth);
bool CachedChartSegmentSafetyCheck(double lat1, double lon1, double lat2,
                                   double lon2, double safety_margin_nm,
                                   SegmentSafetyResult *result,
                                   bool *chart_data_available,
                                   SegmentSafetyCoreStats *stats);
bool ChartSegmentPointClassificationCheck(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, bool force_authoritative_fine,
    SegmentSafetyResult *result, bool *chart_data_available,
    SegmentSafetyCoreStats *stats, bool *open_water_shortcut = nullptr);

}  // namespace ocpn::chart_safety::detail

#endif  // GUI_SRC_CHART_SAFETY_INTERNAL_H_
