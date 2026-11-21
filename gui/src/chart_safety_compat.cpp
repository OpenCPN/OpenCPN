/* GPL-2.0-or-later. Optional ABI adapter for existing chart-aware plugins.
 * New plugins use HostApi123. Both interfaces share services and state.
 */
#include <algorithm>
#include <cstring>
#include <type_traits>
#include "ocpn_plugin.h"
#include "chart_safety_api.h"
#include "chart_safety_internal.h"
#include "chart_safety_compat_abi.h"
using namespace ocpn::chart_safety;

namespace {
// Copy only fields present in the caller's size-initialised structure. The
// legacy C ABI uses int callbacks where HostApi123 uses bool: never cast or
// memcpy function pointers into the typed registry.
template <class From, class To>
bool CopySizedInput(const From* from, To* to) {
  static_assert(std::is_trivially_copyable_v<From> &&
                std::is_trivially_copyable_v<To>);
  static_assert(sizeof(From) == sizeof(To));
  if (!from || from->struct_size < static_cast<int>(sizeof(int))) return false;
  std::memcpy(static_cast<void*>(to), static_cast<const void*>(from),
              std::min(sizeof(To), static_cast<size_t>(from->struct_size)));
  return true;
}
template <class From, class To>
void CopySizedOutput(const From* from, To* to) {
  std::memcpy(static_cast<void*>(to), static_cast<const void*>(from),
              std::min(sizeof(To), static_cast<size_t>(to->struct_size)));
}
PlugInSegmentSafetyTileCacheCallbacks legacy_cache{};
bool LegacyLookup(void* context, long lat, long lon, bool depth,
                  SegmentSafetyTile* tile) {
  const auto& callbacks =
      *static_cast<PlugInSegmentSafetyTileCacheCallbacks*>(context);
  PlugInSegmentSafetyTile legacy{};
  if (!CopySizedInput(tile, &legacy)) return false;
  const bool found = callbacks.lookup(callbacks.context, lat, lon,
                                      depth ? 1 : 0, &legacy) != 0;
  CopySizedOutput(&legacy, tile);
  return found;
}
void LegacyStore(void* context, const SegmentSafetyTile* tile) {
  const auto& callbacks =
      *static_cast<PlugInSegmentSafetyTileCacheCallbacks*>(context);
  PlugInSegmentSafetyTile legacy{};
  if (CopySizedInput(tile, &legacy))
    callbacks.store(callbacks.context, &legacy);
}
void LegacyIdentity(void* context, const char* identity) {
  const auto& callbacks =
      *static_cast<PlugInSegmentSafetyTileCacheCallbacks*>(context);
  if (callbacks.identity_changed)
    callbacks.identity_changed(callbacks.context, identity);
}
void LegacyDependenciesChanged(void* context) {
  const auto& callbacks =
      *static_cast<PlugInSegmentSafetyTileCacheCallbacks*>(context);
  if (callbacks.dependencies_changed)
    callbacks.dependencies_changed(callbacks.context);
}
}  // namespace

extern "C" DECL_EXP int PlugIn_GetPendingSegmentSafetyRequestCount() {
  return GetPendingRequestCount();
}

extern "C" DECL_EXP bool PlugIn_SegmentSafetyProtectsRequestedMasks() {
  return ProtectsRequestedMasks();
}

extern "C" DECL_EXP bool PlugIn_ServicePendingSegmentSafetyRequests(
    int max_requests, int max_milliseconds,
    PlugInSegmentSafetyRequestServiceResult* result) {
  SegmentSafetyRequestServiceResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok =
      ServicePendingRequests(max_requests, max_milliseconds, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_PrewarmSegmentSafetyOpenWater(
    long lat_tile, long lon_tile, const PlugInSegmentSafetyOptions* options) {
  SegmentSafetyOptions input{};
  if (options && !CopySizedInput(options, &input)) return false;
  return PrepareOpenWater(lat_tile, lon_tile, options ? &input : nullptr);
}

extern "C" DECL_EXP bool PlugIn_ImportSegmentSafetyOpenWaterLayer(
    const char* path_utf8, OpenWaterLayerProgressFn progress, void* context,
    char* message_utf8, int message_size) {
  return ImportOpenWaterLayer(path_utf8, progress, context, message_utf8,
                              message_size);
}

extern "C" DECL_EXP bool PlugIn_RemoveSegmentSafetyOpenWaterLayer() {
  return RemoveOpenWaterLayer();
}

extern "C" DECL_EXP bool PlugIn_PrewarmSegmentSafetyRawTiles(
    const long* lat_tiles, const long* lon_tiles, int tile_count,
    int require_depth, PlugInSegmentSafetyResult* result) {
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok =
      PrepareRawTiles(lat_tiles, lon_tiles, tile_count, require_depth, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_PrewarmSegmentSafetyHazardSnapshot(
    double min_lat, double min_lon, double max_lat, double max_lon,
    int enable_fast_path, int shadow_compare,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  SegmentSafetyOptions input{};
  if (options && !CopySizedInput(options, &input)) return false;
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok = PrepareHazardSnapshot(min_lat, min_lon, max_lat, max_lon,
                                        enable_fast_path, shadow_compare,
                                        options ? &input : nullptr, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_CheckSegmentSafety(
    double lat1, double lon1, double lat2, double lon2,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  SegmentSafetyOptions input{};
  if (options && !CopySizedInput(options, &input)) return false;
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok =
      CheckSegment(lat1, lon1, lat2, lon2, options ? &input : nullptr, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_PrewarmSegmentSafetyGrid(
    double min_lat, double min_lon, double max_lat, double max_lon,
    PlugInSegmentSafetyResult* result) {
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok = PrepareGrid(min_lat, min_lon, max_lat, max_lon, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_PrewarmSegmentSafetyGridForSegment(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    PlugInSegmentSafetyResult* result) {
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok =
      PrepareGridForSegment(lat1, lon1, lat2, lon2, safety_margin_nm, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_PrewarmSegmentSafetyRouteMask(
    double min_lat, double min_lon, double max_lat, double max_lon,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  SegmentSafetyOptions input{};
  if (options && !CopySizedInput(options, &input)) return false;
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok = PrepareRouteMask(min_lat, min_lon, max_lat, max_lon,
                                   options ? &input : nullptr, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_PrewarmSegmentSafetyRouteMaskForSegment(
    double lat1, double lon1, double lat2, double lon2,
    double corridor_margin_nm, const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  SegmentSafetyOptions input{};
  if (options && !CopySizedInput(options, &input)) return false;
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok =
      PrepareRouteMaskForSegment(lat1, lon1, lat2, lon2, corridor_margin_nm,
                                 options ? &input : nullptr, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_PrewarmSegmentSafetyRouteMaskForPolylines(
    const double* latitudes, const double* longitudes, const int* point_counts,
    int polyline_count, double corridor_margin_nm,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  SegmentSafetyOptions input{};
  if (options && !CopySizedInput(options, &input)) return false;
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok = PrepareRouteMaskForPolylines(
      latitudes, longitudes, point_counts, polyline_count, corridor_margin_nm,
      options ? &input : nullptr, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP bool
PlugIn_PrewarmSegmentSafetyRouteMaskForPolylinesWithTileHalo(
    const double* latitudes, const double* longitudes, const int* point_counts,
    int polyline_count, double corridor_margin_nm, int fine_tile_halo,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  SegmentSafetyOptions input{};
  if (options && !CopySizedInput(options, &input)) return false;
  SegmentSafetyResult output{};
  if (!CopySizedInput(result, &output)) return false;
  const bool ok = PrepareCorridor(
      latitudes, longitudes, point_counts, polyline_count, corridor_margin_nm,
      fine_tile_halo, options ? &input : nullptr, &output);
  CopySizedOutput(&output, result);
  return ok;
}

extern "C" DECL_EXP void PlugIn_ReleaseSegmentSafetyRouteMaskPins() {
  return ReleasePins();
}

extern "C" DECL_EXP bool PlugIn_RegisterSegmentSafetyTileCache(
    const PlugInSegmentSafetyTileCacheCallbacks* callbacks) {
  if (!wxThread::IsMain()) return false;
  if (!callbacks) {
    const bool removed = RegisterTileCache(nullptr);
    if (removed) legacy_cache = {};
    return removed;
  }
  PlugInSegmentSafetyTileCacheCallbacks next{};
  if (!CopySizedInput(callbacks, &next) ||
      callbacks->struct_size <
          static_cast<int>(offsetof(PlugInSegmentSafetyTileCacheCallbacks,
                                    identity_changed) +
                           sizeof(callbacks->identity_changed)) ||
      !next.lookup || !next.store)
    return false;
  const auto previous = legacy_cache;
  legacy_cache = next;
  SegmentSafetyTileCacheCallbacks adapted{};
  adapted.struct_size = sizeof(adapted);
  adapted.context = &legacy_cache;
  adapted.lookup = LegacyLookup;
  adapted.store = LegacyStore;
  adapted.identity_changed = LegacyIdentity;
  adapted.dependencies_changed = LegacyDependenciesChanged;
  if (RegisterTileCache(&adapted)) return true;
  legacy_cache = previous;
  return false;
}

extern "C" DECL_EXP bool PlugIn_GetSegmentSafetyChartIdentity(
    char* identity, int identity_size) {
  if (!identity || identity_size <= 0) return false;
  identity[0] = '\0';
  std::string value;
  if (!GetChartIdentity(&value) ||
      value.size() >= static_cast<size_t>(identity_size))
    return false;
  std::memcpy(identity, value.c_str(), value.size() + 1);
  return true;
}

extern "C" DECL_EXP int PlugIn_GetSegmentSafetyChartInfoCount() {
  return GetChartInfoCount();
}

extern "C" DECL_EXP bool PlugIn_GetSegmentSafetyChartInfo(
    int ordinal, PlugInSegmentSafetyChartInfoV1* chart_info) {
  SegmentSafetyChartInfo output{};
  if (!CopySizedInput(chart_info, &output)) return false;
  const bool ok = GetChartInfo(ordinal, &output);
  CopySizedOutput(&output, chart_info);
  return ok;
}

extern "C" DECL_EXP bool PlugIn_GetSegmentSafetyChartCoverageTiles(
    const int* chart_db_indices, int chart_count, double tile_degrees,
    long* lat_tiles, long* lon_tiles, int tile_capacity, int* tile_count,
    int* complete) {
  bool all = false;
  const bool ok = GetChartCoverageTiles(chart_db_indices, chart_count,
                                        tile_degrees, lat_tiles, lon_tiles,
                                        tile_capacity, tile_count, &all);
  if (complete) *complete = all ? 1 : 0;
  return ok;
}

extern "C" DECL_EXP bool PlugIn_SetSegmentSafetyPersistentCacheEnabled(
    int enabled) {
  return SetPersistentCacheEnabled(enabled);
}

extern "C" DECL_EXP int PlugIn_GetSegmentSafetyPersistentCacheEnabled() {
  return GetPersistentCacheEnabled();
}

extern "C" DECL_EXP bool PlugIn_SaveSegmentSafetyPersistentCache() {
  return SavePersistentCache();
}

extern "C" DECL_EXP bool PlugIn_ClearSegmentSafetyPersistentCache() {
  return ClearPersistentCache();
}

extern "C" DECL_EXP int PlugIn_QueueSegmentSafetyRawTileHelpers(const long* lat,
                                                                const long* lon,
                                                                int count,
                                                                int depth) {
  return detail::QueueTileHelpersCompat(lat, lon, count, depth);
}

extern "C" DECL_EXP bool PlugIn_SegmentSafetyRawTileHelperPending(long lat,
                                                                  long lon) {
  return detail::TileHelperPendingCompat(lat, lon);
}

extern "C" DECL_EXP void PlugIn_SetSegmentSafetyHelperProcessLimit(int limit) {
  return detail::SetHelperProcessLimitCompat(limit);
}

extern "C" DECL_EXP void PlugIn_EnableSegmentSafetyTileHelpers(int enabled) {
  return detail::EnableTileHelpersCompat(enabled);
}

extern "C" DECL_EXP bool PlugIn_RunSegmentSafetyTileWorker(
    const char* directory) {
  return detail::RunTileWorkerCompat(directory);
}
