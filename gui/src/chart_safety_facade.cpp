/**************************************************************************
 *   Copyright (C) 2024 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

/**
 * \file
 *
 * Public HostApi122 facade and request lifecycle for chart-safety services.
 */
#include <cstddef>
#include <cstdint>
#include <climits>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "dychart.h"  // Must be ahead due to buggy GL includes handling

#include <wx/wx.h>
#include <wx/arrstr.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/event.h>
#include <wx/glcanvas.h>
#include <wx/thread.h>
#include <wx/notebook.h>
#include <wx/string.h>
#include <wx/window.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/jsonreader.h>
#include <wx/jsonval.h>
#include <wx/jsonwriter.h>
#include <wx/wfstream.h>

#include "o_sound/o_sound.h"

#include "model/ais_decoder.h"
#include "model/comm_bridge.h"
#include "model/comm_navmsg_bus.h"
#include "model/gui_vars.h"
#include "model/idents.h"
#include "model/multiplexer.h"
#include "model/navobj_db.h"
#include "model/notification_manager.h"
#include "model/own_ship.h"
#include "model/plugin_loader.h"
#include "model/plugin_comm.h"
#include "model/svg_utils.h"
#include "model/route.h"
#include "model/track.h"

#include "ais.h"
#include "chartdb.h"
#include "chcanv.h"
#include "cm93.h"
#include "config_mgr.h"
#include "font_mgr.h"
#include "gl_chart_canvas.h"
#include "chart_safety_depth.h"
#include "chart_safety_api.h"
#include "chart_safety_service.h"
#include "chart_safety_internal.h"
#include "navigation_safety.h"
#include "gui_lib.h"
#include "navutil.h"
#include "ocpn_aui_manager.h"
#include "ocpn_frame.h"
#include "ocpn_platform.h"
#include "ocpn_plugin.h"
#include "options.h"
#include "piano.h"
#include "pluginmanager.h"
#include "routemanagerdialog.h"
#include "routeman_gui.h"
#include "s52plib.h"
#include "s57chart.h"
#include "shapefile_basemap.h"
#include "toolbar.h"
#include "waypointman_gui.h"

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
extern wxLocale* plocale_def_lang;
#endif

extern PlugInManager* s_ppim;  // FIXME (leamas) another name for global mgr

extern options* g_pOptions;  // FIXME (leamas) merge to g_options

extern arrayofCanvasPtr g_canvasArray;  // FIXME (leamas) find new home

namespace ocpn::chart_safety {

using namespace detail;

wxString PointDiagnostic(double lat, double lon) {
  return SegmentSafetyPointDiagnostic(lat, lon);
}

int GetPendingRequestCount() {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  return static_cast<int>(s_segment_safety_pending_route_mask_requests.size() +
                          s_segment_safety_inflight_route_mask_requests.size());
}

bool ServicePendingRequests(int max_requests, int max_milliseconds,
                            SegmentSafetyRequestServiceResult* result) {
  int result_size = result ? result->struct_size : 0;
  if (result && result_size >= static_cast<int>(sizeof(int))) {
    memset(result, 0,
           wxMin(result_size,
                 static_cast<int>(sizeof(SegmentSafetyRequestServiceResult))));
    result->struct_size = result_size;
  }
  if (!wxThread::IsMain()) return false;

  wxStopWatch timer;
  int pending_before = GetPendingRequestCount();
  int serviced = 0;
  int built_count = 0;
  int prefetched_count = 0;
  int failed = 0;

  while ((max_requests <= 0 || serviced < max_requests) &&
         (max_milliseconds <= 0 || serviced == 0 ||
          timer.Time() < max_milliseconds)) {
    SegmentSafetyRouteMaskRequest request;
    bool have_request = false;
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      if (!s_segment_safety_pending_route_mask_requests.empty()) {
        std::map<std::string, SegmentSafetyRouteMaskRequest>::iterator it =
            s_segment_safety_pending_route_mask_requests.begin();
        request = it->second;
        s_segment_safety_pending_route_mask_requests.erase(it);
        s_segment_safety_inflight_route_mask_requests.insert(request.key);
        have_request = true;
      }
    }
    if (!have_request) break;

    bool built = false;
    bool ok = request.group_index == SegmentSafetyCurrentGroupIndex() &&
              EnsureSegmentSafetyRouteMaskTile(
                  request.lat_tile, request.lon_tile, request.safety_margin_nm,
                  request.check_depth, request.minimum_depth_m, NULL, &built,
                  request.force_authoritative_fine);
    // A worker stops at the first absent mask and can otherwise incur one GUI
    // timer wake-up per 0.05-degree tile.  Populate the immediately adjacent
    // exact masks while the chart is already active on the main thread.  This
    // changes only scheduling/cache locality: every neighbour is built by the
    // same authoritative code as an on-demand request.  Do not fan out from
    // an all-blocked centre tile: propagation cannot continue through it, and
    // prefetching eight more inland tiles was pure work.  Keep independent
    // force-fine validation demand-only to avoid speculative validation work.
    CachedSegmentSafetyRouteMaskTile centre_mask;
    const bool useful_to_prefetch_neighbours =
        ok && LookupSegmentSafetyRouteMaskTile(request.key, &centre_mask) &&
        centre_mask.clear_count > 0;
    if (useful_to_prefetch_neighbours && !request.force_authoritative_fine) {
      bool budget_exhausted = false;
      for (int dlat = -1; dlat <= 1; ++dlat) {
        for (int dlon = -1; dlon <= 1; ++dlon) {
          if (dlat == 0 && dlon == 0) continue;
          if (!ocpn::chart_safety::MayPrefetchNeighbour(timer.Time(),
                                                        max_milliseconds)) {
            budget_exhausted = true;
            break;
          }
          const long neighbour_lat_tile = request.lat_tile + dlat;
          const long neighbour_lon_tile = request.lon_tile + dlon;
          const double neighbour_min_lat =
              neighbour_lat_tile * kGridTileDegrees;
          const double neighbour_min_lon =
              neighbour_lon_tile * kGridTileDegrees;
          if (neighbour_min_lat < -90.0 || neighbour_min_lat >= 90.0 ||
              neighbour_min_lon < -180.0 || neighbour_min_lon >= 180.0)
            continue;
          bool neighbour_built = false;
          if (EnsureSegmentSafetyRouteMaskTile(
                  neighbour_lat_tile, neighbour_lon_tile,
                  request.safety_margin_nm, request.check_depth,
                  request.minimum_depth_m, NULL, &neighbour_built, false) &&
              neighbour_built) {
            ++built_count;
            ++prefetched_count;
          }
        }
        if (budget_exhausted) break;
      }
    }
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      s_segment_safety_inflight_route_mask_requests.erase(request.key);
      if (ok) s_segment_safety_pending_route_mask_requests.erase(request.key);
    }
    ++serviced;
    if (ok && built)
      ++built_count;
    else if (!ok)
      ++failed;
  }

  int pending_after = GetPendingRequestCount();
  if (serviced > 0 && s_segment_safety_persistent_tiles_since_checkpoint >=
                          kPersistentCheckpointTiles)
    SegmentSafetyPersistentCacheSave();
  if (result && result_size >= static_cast<int>(
                                   sizeof(SegmentSafetyRequestServiceResult))) {
    result->pending_before = pending_before;
    result->requests_serviced = serviced;
    result->masks_built = built_count;
    result->requests_failed = failed;
    result->pending_after = pending_after;
    result->elapsed_ms = timer.Time();
  }
  if (serviced > 0 || pending_before > 0) {
    wxLogMessage(
        "WR_GRID_REQUEST_SERVICE pending_before=%d serviced=%d built=%d "
        "prefetched=%d failed=%d pending_after=%d elapsed_ms=%ld "
        "main_thread=1",
        pending_before, serviced, built_count, prefetched_count, failed,
        pending_after, timer.Time());
  }
  return failed == 0;
}

bool PrepareRawTiles(const long* lat_tiles, const long* lon_tiles,
                     int tile_count, bool require_depth,
                     SegmentSafetyResult* result) {
  wxStopWatch timer;
  InitSegmentSafetyResult(result);
  if (!wxThread::IsMain() || !ChartData) {
    SetSegmentSafetyStatus(result, kNoData);
    SetSegmentSafetyMessage(
        result, wxThread::IsMain()
                    ? "chart database unavailable for raw tile extraction"
                    : "raw chart tile extraction is main-thread only");
    return false;
  }
  if (result && result->struct_size < (int)sizeof(int)) return false;
  if (!lat_tiles || !lon_tiles || tile_count <= 0 || tile_count > 1000000) {
    SetSegmentSafetyStatus(result, kError);
    SetSegmentSafetyMessage(result, "invalid raw chart tile request");
    return false;
  }

  SegmentSafetyRefreshPersistentChartIdentity();
  SegmentSafetyPersistentCacheEnsureLoaded();
  std::set<std::pair<long, long>> tiles;
  for (int index = 0; index < tile_count; ++index)
    tiles.insert(std::make_pair(lat_tiles[index], lon_tiles[index]));

  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    for (std::set<std::pair<long, long>>::const_iterator it = tiles.begin();
         it != tiles.end(); ++it)
      s_segment_safety_pinned_grid_keys.insert(
          SegmentSafetyGridTileKeyForIndices(it->first, it->second));
  }

  SegmentSafetyCoreStats stats;
  long built_tiles = 0;
  long reused_tiles = 0;
  long failed_tiles = 0;
  SegmentSafetySource source = kSourceNone;
  // This is the primary xWeatherRouting prewarm entry point.  Classify
  // adjacent licensed plugin-vector tiles in provider-sized rectangles before
  // the compatibility loop below; any mixed, unsupported or incomplete tile
  // still falls through to the established per-tile path.
  const std::set<std::pair<long, long>> plugin_batch_built_tiles =
      PrebuildSegmentSafetyPluginVectorGridTiles(tiles, &stats,
                                                 require_depth != 0);
  for (std::set<std::pair<long, long>>::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    bool built = false;
    if (!EnsureSegmentSafetyGridTile(it->first, it->second, &stats, &built,
                                     require_depth != 0)) {
      ++failed_tiles;
      continue;
    }
    CachedPointSafetyGridTile tile;
    if (!LookupSegmentSafetyGridTile(
            SegmentSafetyGridTileKeyForIndices(it->first, it->second), &tile) ||
        !tile.built || (require_depth && !tile.depth_complete)) {
      ++failed_tiles;
      continue;
    }
    // An already-hot host tile might predate plugin cache registration.
    // Republish it so the caller always owns a complete immutable snapshot.
    SegmentSafetyExternalTileCacheStore(tile);
    if (source == kSourceNone) source = tile.source;
    if (built || plugin_batch_built_tiles.count(*it))
      ++built_tiles;
    else
      ++reused_tiles;
  }

  if (result) {
    SetSegmentSafetyStatus(result, failed_tiles == 0 ? kSafe : kNoData);
    SetSegmentSafetySource(result, source);
    SetSegmentSafetyDiagnosticReason(result, failed_tiles == 0
                                                 ? kDiagnosticChartGeometryClear
                                                 : kDiagnosticNoCandidateChart);
    SetSegmentSafetyMessage(
        result, failed_tiles == 0
                    ? "raw chart classification tiles published to plugin"
                    : "one or more raw chart tiles could not be extracted");
    ApplySegmentSafetyStats(result, stats);
    if (result->struct_size >=
        (int)(offsetof(SegmentSafetyResult, prewarm_fine_tiles_avoided) +
              sizeof(result->prewarm_fine_tiles_avoided))) {
      result->prewarm_requested_tiles = static_cast<int>(tiles.size());
      result->prewarm_base_tiles_built = static_cast<int>(built_tiles);
      result->prewarm_base_tiles_reused = static_cast<int>(reused_tiles);
      result->prewarm_masks_built = 0;
      result->prewarm_masks_reused = 0;
      result->prewarm_fine_tiles_avoided = 0;
    }
  }
  wxLogMessage(
      "WR_RAW_TILE_PREWARM requested=%lu built=%ld reused=%ld failed=%ld "
      "require_depth=%d elapsed_ms=%ld cells=%d land=%d water=%d drying=%d "
      "unknown=%d ownership=weather-routing-plugin",
      static_cast<unsigned long>(tiles.size()), built_tiles, reused_tiles,
      failed_tiles, require_depth ? 1 : 0, timer.Time(), stats.grid_cells_total,
      stats.grid_cells_land, stats.grid_cells_water, stats.grid_cells_drying,
      stats.grid_cells_unknown);
  return failed_tiles == 0;
}

bool PrepareHazardSnapshot(double min_lat, double min_lon, double max_lat,
                           double max_lon, bool enable_fast_path,
                           bool shadow_compare,
                           const SegmentSafetyOptions* options,
                           SegmentSafetyResult* result) {
  wxStopWatch timer;
  InitSegmentSafetyResult(result);
  if (!wxThread::IsMain() || !ChartData) {
    SetSegmentSafetyStatus(result, kNoData);
    SetSegmentSafetyMessage(
        result, wxThread::IsMain()
                    ? "chart database unavailable for hazard snapshot"
                    : "hazard snapshot construction is main-thread only");
    return false;
  }
  if (options && options->struct_size < (int)sizeof(int)) {
    SetSegmentSafetyStatus(result, kError);
    SetSegmentSafetyMessage(result, "invalid segment safety options");
    return false;
  }
  if (result && result->struct_size < (int)sizeof(int)) return false;

  if (min_lat > max_lat) std::swap(min_lat, max_lat);
  if (min_lon > max_lon) std::swap(min_lon, max_lon);
  min_lat = wxMax(-89.0, wxMin(89.0, min_lat));
  max_lat = wxMax(-89.0, wxMin(89.0, max_lat));
  min_lon = wxMax(-180.0, wxMin(180.0, min_lon));
  max_lon = wxMax(-180.0, wxMin(180.0, max_lon));
  if (min_lat >= max_lat || min_lon >= max_lon) {
    SetSegmentSafetyStatus(result, kError);
    SetSegmentSafetyMessage(result, "invalid hazard snapshot bounds");
    return false;
  }

  SegmentSafetyRefreshPersistentChartIdentity();
  std::shared_ptr<SegmentSafetyHazardSnapshot> snapshot(
      new SegmentSafetyHazardSnapshot);
  snapshot->chart_identity = SegmentSafetyChartIdentity();
  snapshot->group_index = SegmentSafetyCurrentGroupIndex();
  snapshot->area = {min_lat, max_lat, min_lon, max_lon};
  snapshot->fast_path_enabled = enable_fast_path != 0;
  snapshot->shadow_compare = shadow_compare != 0;

  const SegmentSafetyBBox requested = snapshot->area;
  int candidate_entries = 0;
  int supported_entries = 0;
  int unsupported_entries = 0;
  const int entries = ChartData->GetChartTableEntries();
  for (int index = 0; index < entries; ++index) {
    const ChartTableEntry& entry = ChartData->GetChartTableEntry(index);
    ChartBase* chart = ChartData->OpenChartFromDB(index, FULL_INIT);
    const bool composite =
        chart && (chart->GetChartType() == CHART_TYPE_CM93 ||
                  chart->GetChartType() == CHART_TYPE_CM93COMP);
    const bool table_composite = entry.GetChartType() == CHART_TYPE_CM93 ||
                                 entry.GetChartType() == CHART_TYPE_CM93COMP;
    const bool global_or_invalid_bbox =
        entry.GetLatMax() > 90.0 || entry.GetLatMin() < -90.0;
    if (global_or_invalid_bbox && !table_composite && !composite) continue;
    const std::vector<int>& groups = entry.GetGroupArray();
    if (snapshot->group_index > 0 &&
        std::find(groups.begin(), groups.end(), snapshot->group_index) ==
            groups.end())
      continue;

    SegmentSafetySnapshotChart captured;
    captured.db_index = index;
    captured.native_scale = entry.GetScale();
    captured.edition_date = entry.GetChartEditionDate();
    captured.file_time = entry.GetFileTime();
    captured.chart_path = wxString::FromUTF8(entry.GetFullPath().c_str());
    captured.bbox =
        (global_or_invalid_bbox || composite)
            ? requested
            : SegmentSafetyBBox{entry.GetLatMin(), entry.GetLatMax(),
                                entry.GetLonMin(), entry.GetLonMax()};
    if (!SegmentSafetyBBoxIntersects(captured.bbox, requested)) continue;
    ++candidate_entries;

    for (int aux = 0; aux < entry.GetnAuxPlyEntries(); ++aux) {
      CachedLandRing ring = SegmentSafetyRingFromFloatTable(
          entry.GetpAuxPlyTableEntry(aux), entry.GetAuxCntTableEntry(aux));
      if (ring.points.size() >= 3) captured.coverage.push_back(ring);
    }
    if (captured.coverage.empty()) {
      CachedLandRing ring = SegmentSafetyRingFromFloatTable(
          entry.GetpPlyTable(), entry.GetnPlyEntries());
      if (ring.points.size() >= 3) captured.coverage.push_back(ring);
    }
    for (int hole = 0; hole < entry.GetnNoCovrPlyEntries(); ++hole) {
      CachedLandRing ring =
          SegmentSafetyRingFromFloatTable(entry.GetpNoCovrPlyTableEntry(hole),
                                          entry.GetNoCovrCntTableEntry(hole));
      if (ring.points.size() >= 3) captured.no_coverage.push_back(ring);
    }

    s57chart* s57 = dynamic_cast<s57chart*>(chart);
    captured.vector_supported = s57 != NULL && !composite &&
                                entry.GetChartFamily() == CHART_FAMILY_VECTOR;
    if (captured.vector_supported) {
      captured.source = kSourceVectorChart;
      std::set<std::string> seen_hazards;
      SegmentSafetyAppendFeatureRings(s57, "LNDARE", &captured.hazards,
                                      &seen_hazards);
      ++supported_entries;
    } else {
      // CM93 composites dynamically rebuild their active rule set while
      // selecting cells.  Identical scans can expose different ring sets, so
      // they are not accepted as immutable SAFE proof.  The persistent
      // highest-detail authoritative raster/coarse hierarchy remains active.
      captured.source = composite ? kSourceCm93 : kSourceNone;
      ++unsupported_entries;
    }
    snapshot->coverage_rings += captured.coverage.size();
    snapshot->hazard_rings += captured.hazards.size();
    snapshot->charts.push_back(std::move(captured));
  }

  BuildSegmentSafetySnapshotHierarchy(snapshot.get());
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    s_segment_safety_hazard_snapshot = snapshot;
    s_segment_safety_snapshot_queries = 0;
    s_segment_safety_snapshot_safe = 0;
    s_segment_safety_snapshot_unknown = 0;
    s_segment_safety_snapshot_shadow_disagreements = 0;
  }

  if (result) {
    SetSegmentSafetyStatus(result, snapshot->charts.empty() ? kNoData : kSafe);
    SetSegmentSafetySource(
        result, supported_entries > 0 ? kSourceVectorChart : kSourceNone);
    SetSegmentSafetyMessage(result,
                            snapshot->charts.empty()
                                ? "no chart coverage found for hazard snapshot"
                                : "immutable chart hazard snapshot ready");
    result->candidate_chart_count = candidate_entries;
    result->s57_chart_count = supported_entries;
    result->unsupported_chart_count = unsupported_entries;
    result->land_ring_count = snapshot->hazard_rings;
    result->cache_build_ms = timer.Time();
  }
  wxLogMessage(
      "WR_HAZARD_SNAPSHOT_BUILD area=[%.6f..%.6f,%.6f..%.6f] group=%d "
      "chart_identity=\"%s\" candidates=%d supported=%d unsupported=%d "
      "coverage_rings=%ld hazard_rings=%ld safe_large_cells=%lu "
      "safe_fine_cells=%lu fast_path=%d shadow=%d "
      "elapsed_ms=%ld selection=largest-scale-then-newest "
      "uncertainty=fallback-authoritative",
      min_lat, max_lat, min_lon, max_lon, snapshot->group_index,
      snapshot->chart_identity, candidate_entries, supported_entries,
      unsupported_entries, snapshot->coverage_rings, snapshot->hazard_rings,
      static_cast<unsigned long>(snapshot->certified_safe_large_cells.size()),
      static_cast<unsigned long>(snapshot->certified_safe_fine_cells.size()),
      snapshot->fast_path_enabled ? 1 : 0, snapshot->shadow_compare ? 1 : 0,
      timer.Time());
  return !snapshot->charts.empty();
}

bool CheckSegment(double lat1, double lon1, double lat2, double lon2,
                  const SegmentSafetyOptions* options,
                  SegmentSafetyResult* result) {
  std::chrono::steady_clock::time_point query_start =
      std::chrono::steady_clock::now();
  InitSegmentSafetyResult(result);
  if (!ValidSegmentInput(lat1, lon1, lat2, lon2, options)) {
    SetSegmentSafetyStatus(result, kError);
    SetSegmentSafetyMessage(result, "invalid segment coordinates or options");
    return false;
  }
  // Accept finite longitudes in either common convention, but never feed
  // unbounded coordinates into tile-index integer conversions.
  lon1 = std::remainder(lon1, 360.0);
  lon2 = std::remainder(lon2, 360.0);
  if (wxThread::IsMain() && (!ChartData || !ChartData->IsValid())) {
    SetSegmentSafetyStatus(result, kNoData);
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticNoChartDatabase);
    SetSegmentSafetyMessage(result,
                            "chart database is unavailable or rebuilding");
    // This is transient, not a geographic proof of absent coverage. Do not
    // populate the point/tile caches until the database is ready.
    return true;
  }
  if (wxThread::IsMain()) {
    SegmentSafetyRefreshPersistentChartIdentity();
    SegmentSafetyPersistentCacheEnsureLoaded();
  }

  if (options && options->struct_size < (int)sizeof(int)) {
    SetSegmentSafetyStatus(result, kError);
    SetSegmentSafetyMessage(result, "invalid segment safety options");
    return false;
  }

  if (result && result->struct_size < (int)sizeof(int)) return false;

  const double safety_margin_nm = SegmentSafetyOptionMargin(options);
  const bool check_land = SegmentSafetyOptionCheckLand(options);
  const bool allow_gshhs_fallback =
      SegmentSafetyOptionAllowGshhsFallback(options);
  const bool check_depth = SegmentSafetyOptionCheckDepth(options);
  const double minimum_depth_m = SegmentSafetyOptionMinimumDepthM(options);
  const bool force_authoritative_fine =
      SegmentSafetyOptionForceAuthoritativeFineValidation(options);

  if (!check_land && !check_depth) {
    if (result) {
      SetSegmentSafetyStatus(result, kSafe);
      SetSegmentSafetyMessage(result, "land checks disabled");
    }
    return true;
  }

  bool chart_data_available = false;
  SegmentSafetyCoreStats stats;
  SegmentSafetySnapshotDecision snapshot_decision = kSnapshotUnknown;
  bool snapshot_fast_path = false;
  bool snapshot_shadow_compare = false;
  if (check_land && !force_authoritative_fine) {
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      if (s_segment_safety_hazard_snapshot) {
        snapshot_fast_path =
            s_segment_safety_hazard_snapshot->fast_path_enabled;
        snapshot_shadow_compare =
            s_segment_safety_hazard_snapshot->shadow_compare;
      }
    }
    snapshot_decision = QuerySegmentSafetyHazardSnapshot(
        lat1, lon1, lat2, lon2, safety_margin_nm, check_depth);
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    ++s_segment_safety_snapshot_queries;
    if (snapshot_decision == kSnapshotSafe)
      ++s_segment_safety_snapshot_safe;
    else
      ++s_segment_safety_snapshot_unknown;
  }
  auto log_query = [&](const char* phase) {
    if (!result) return;
    if (snapshot_shadow_compare && snapshot_decision == kSnapshotSafe &&
        (result->status == kCrossesLand ||
         result->status == kWithinLandMargin || result->status == kDryingArea ||
         result->status == kTooShallow)) {
      long disagreements = 0;
      {
        wxMutexLocker lock(s_segment_safety_cache_mutex);
        disagreements = ++s_segment_safety_snapshot_shadow_disagreements;
      }
      wxLogMessage(
          "WR_HAZARD_SNAPSHOT_SHADOW_MISMATCH #%ld "
          "segment=(%.8f,%.8f)->(%.8f,%.8f) margin_nm=%.3f "
          "snapshot=safe authoritative_status=%d phase=%s",
          disagreements, lat1, lon1, lat2, lon2, safety_margin_nm,
          result->status, phase);
    }
    long query_log_index = 0;
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      if (s_segment_safety_query_logs >= kMaxQueryLogs) return;
      int64_t elapsed_us =
          std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::steady_clock::now() - query_start)
              .count();
      bool notable = result->status != kSafe ||
                     stats.unexpected_tile_builds > 0 || elapsed_us > 5000 ||
                     stats.water_tile_shortcuts > 0;
      if (!notable && s_segment_safety_query_logs >= 40) return;
      query_log_index = ++s_segment_safety_query_logs;
    }

    int unsafe_flags = 0;
    if (result->status == kCrossesLand || result->status == kWithinLandMargin)
      unsafe_flags |= 1;
    if (result->status == kDryingArea) unsafe_flags |= 2;
    if (result->status == kTooShallow) unsafe_flags |= 4;
    if (result->status == kUnknownDepth) unsafe_flags |= 8;
    if (result->status == kNoData || result->status == kError ||
        result->status == kPendingData || stats.unexpected_tile_builds > 0)
      unsafe_flags |= 16;

    int64_t elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
                             std::chrono::steady_clock::now() - query_start)
                             .count();
    int chart_api_calls = stats.point_cache_misses + stats.s57_chart_count +
                          stats.candidate_chart_count + stats.land_ring_count;
    long mixed_cell_checks =
        stats.grid_lookups > stats.water_tile_shortcuts
            ? stats.grid_lookups - stats.water_tile_shortcuts
            : stats.grid_lookups;
    wxLogMessage(
        "WR_GRID_SEGMENT_QUERY #%ld phase=%s worker=%d segment=(%.8f,%.8f)->"
        "(%.8f,%.8f) margin_nm=%.3f depth_check=%d status=%d source=%d "
        "authoritative_fine=%d persistent_cache_used_in_query=%d "
        "unsafe_flags=%d all_safe_tile_shortcuts=%d mixed_tile_cell_checks=%ld "
        "missing_tile_requests=%d tile_cache_hits=%d tile_cache_misses=%d "
        "grid_lookups=%d samples=%d query_time_us=%lld "
        "chart_api_calls_during_query=%d "
        "worker_thread_query_without_chart_api=%d "
        "message=\"%s\".",
        query_log_index, phase, wxThread::IsMain() ? 0 : 1, lat1, lon1, lat2,
        lon2, safety_margin_nm, check_depth ? 1 : 0, result->status,
        result->source, force_authoritative_fine ? 1 : 0,
        force_authoritative_fine ? 0 : stats.coarse_certified_safe_hits,
        unsafe_flags, stats.water_tile_shortcuts, mixed_cell_checks,
        stats.unexpected_tile_builds, stats.grid_cache_hits,
        stats.grid_cache_misses, stats.grid_lookups, stats.segment_sample_count,
        (long long)elapsed_us, chart_api_calls,
        (!wxThread::IsMain() && chart_api_calls == 0) ? 1 : 0, result->message);
    if (stats.coarse_cells_checked > 0 ||
        stats.coarse_certified_safe_hits > 0 || stats.coarse_missing > 0) {
      wxLogMessage(
          "WR_COARSE_SAFETY_QUERY_SUMMARY #%ld coarse_cells_checked=%d "
          "coarse_certified_safe_hits=%d coarse_mixed_fallbacks=%d "
          "coarse_unknown_fallbacks=%d coarse_no_chart=%d "
          "coarse_depth_unproven=%d coarse_missing=%d fine_tiles_avoided=%d "
          "query_time_us=%lld chart_api_calls_during_query=%d "
          "worker_thread_query_without_chart_api=%d",
          query_log_index, stats.coarse_cells_checked,
          stats.coarse_certified_safe_hits, stats.coarse_mixed_fallbacks,
          stats.coarse_unknown_fallbacks, stats.coarse_no_chart,
          stats.coarse_depth_unproven, stats.coarse_missing,
          stats.fine_tiles_avoided, (long long)elapsed_us, chart_api_calls,
          (!wxThread::IsMain() && chart_api_calls == 0) ? 1 : 0);
    }
  };
  if (snapshot_fast_path && snapshot_decision == kSnapshotSafe) {
    SetSegmentSafetyStatus(result, kSafe);
    SetSegmentSafetySource(result, kSourceVectorChart);
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryClear);
    SetSegmentSafetyMessage(
        result, "segment is clear using immutable best-chart hazard snapshot");
    ApplySegmentSafetyStats(result, stats);
    log_query("hazard-snapshot-safe");
    return true;
  }
  const std::string segment_cache_key = SegmentSafetySegmentCacheKey(
      lat1, lon1, lat2, lon2, safety_margin_nm, check_depth, minimum_depth_m);
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute)))
    result->required_depth_m = minimum_depth_m;
  CachedSegmentSafetyResult cached_segment_result;
  if (LookupSegmentSafetySegmentCache(segment_cache_key,
                                      &cached_segment_result)) {
    // Replaying unsafe rejects is conservative.  Replaying SAFE accepts is not
    // yet used while the experimental chart/grid service is being hardened,
    // since chart-set and scale changes are not represented by a stable
    // generation id in this cache key.
    if (cached_segment_result.status != kSafe) {
      ++stats.segment_cache_hits;
      CopyCachedSegmentSafetyToResult(cached_segment_result, result);
      ApplySegmentSafetyStats(result, stats);
      log_query("segment-cache-hit");
      return true;
    }
  }
  ++stats.segment_cache_misses;

  bool open_water_shortcut = false;
  if (ChartSegmentPointClassificationCheck(
          lat1, lon1, lat2, lon2, safety_margin_nm, check_depth,
          minimum_depth_m, force_authoritative_fine, result,
          &chart_data_available, &stats, &open_water_shortcut)) {
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryHit);
    StoreSegmentSafetySegmentCache(
        segment_cache_key,
        MakeCachedSegmentSafetyResult(result, kDiagnosticChartGeometryHit),
        &stats);
    ApplySegmentSafetyStats(result, stats);
    log_query("chart-grid-hit");
    return true;
  }

  if (!chart_data_available && !wxThread::IsMain()) {
    SetSegmentSafetyStatus(result, kPendingData);
    SetSegmentSafetySource(result, kSourceNone);
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticPendingData);
    SetSegmentSafetyMessage(
        result,
        "chart safety grid tile requested; worker is waiting for main-thread "
        "chart object classification");
    ApplySegmentSafetyStats(result, stats);
    log_query("missing-tile-worker");
    return true;
  }

  if (!chart_data_available &&
      CachedChartSegmentSafetyCheck(lat1, lon1, lat2, lon2, safety_margin_nm,
                                    result, &chart_data_available, &stats)) {
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryHit);
    ApplySegmentSafetyStats(result, stats);
    log_query("legacy-chart-hit");
    return true;
  }

  if (chart_data_available) {
    if (result) {
      SetSegmentSafetyStatus(result, kSafe);
      if (GetSegmentSafetySource(result) == kSourceNone)
        SetSegmentSafetySource(result, kSourceVectorChart);
      SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryClear);
      SetSegmentSafetyMessage(
          result, "segment is clear using chart point land classification");
      if (open_water_shortcut) {
        SetSegmentSafetyMessage(
            result,
            "segment is clear using cached open-water chart grid cells");
      }
      ApplySegmentSafetyStats(result, stats);
      log_query(open_water_shortcut ? "chart-grid-all-safe"
                                    : "chart-grid-clear");
    }
    return true;
  }

  SegmentSafetyDiagnosticReason unavailable_reason =
      SegmentSafetyUnavailableReason(stats);
  if (allow_gshhs_fallback) {
    SegmentSafetyStatus fallback_status = kSafe;
    bool crosses_land = GshhsSegmentSafetyHitsLand(
        lat1, lon1, lat2, lon2, safety_margin_nm, &fallback_status);
    if (result) {
      SetSegmentSafetySource(result, kSourceGshhsFallback);
      SetSegmentSafetyFallback(result, true);
      SetSegmentSafetyStatus(result, crosses_land ? fallback_status : kSafe);
      SetSegmentSafetyDiagnosticReason(result, unavailable_reason);
      SetSegmentSafetyMessage(
          result, crosses_land
                      ? "segment crosses GSHHS shoreline fallback"
                      : SegmentSafetyUnavailableMessage(unavailable_reason));
      ApplySegmentSafetyStats(result, stats);
      log_query("gshhs-fallback");
    }
    return true;
  }

  if (result) {
    SetSegmentSafetyStatus(result, kNoData);
    SetSegmentSafetyDiagnosticReason(result, unavailable_reason);
    SetSegmentSafetyMessage(
        result, SegmentSafetyUnavailableMessage(unavailable_reason));
    ApplySegmentSafetyStats(result, stats);
    log_query("no-data");
  }
  return true;
}

bool PrepareGrid(double min_lat, double min_lon, double max_lat, double max_lon,
                 SegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);

  if (result && result->struct_size < (int)sizeof(int)) return false;

  if (min_lat > max_lat) std::swap(min_lat, max_lat);
  if (min_lon > max_lon) std::swap(min_lon, max_lon);
  min_lat = wxMax(-90.0, wxMin(90.0, min_lat));
  max_lat = wxMax(-90.0, wxMin(90.0, max_lat));

  long min_lat_tile = floor(min_lat / kGridTileDegrees);
  long max_lat_tile = floor(max_lat / kGridTileDegrees);
  long min_lon_tile = floor(min_lon / kGridTileDegrees);
  long max_lon_tile = floor(max_lon / kGridTileDegrees);

  long lat_count = max_lat_tile - min_lat_tile + 1;
  long lon_count = max_lon_tile - min_lon_tile + 1;
  long requested_tiles =
      lat_count > 0 && lon_count > 0 ? lat_count * lon_count : 0;
  SegmentSafetyCoreStats stats;
  long built_tiles = 0;
  long reused_tiles = 0;

  for (long lat_tile = min_lat_tile; lat_tile <= max_lat_tile; ++lat_tile) {
    for (long lon_tile = min_lon_tile; lon_tile <= max_lon_tile; ++lon_tile) {
      std::string key = SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
      if (LookupSegmentSafetyGridTile(key, NULL)) {
        ++stats.grid_cache_hits;
        ++reused_tiles;
        continue;
      }

      bool built = false;
      EnsureSegmentSafetyGridTile(lat_tile, lon_tile, &stats, &built);
      if (built) ++built_tiles;
    }
  }

  if (result) {
    SetSegmentSafetyStatus(result, kSafe);
    SetSegmentSafetySource(result, kSourceVectorChart);
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryClear);
    SetSegmentSafetyMessage(result, "segment safety grid prewarmed");
    ApplySegmentSafetyStats(result, stats);
  }

  wxString message = wxString::Format(
      "SEGMENT_SAFETY_GRID prewarm bbox=[lat %.6f..%.6f lon %.6f..%.6f] "
      "requested_tiles=%ld built_tiles=%ld reused_tiles=%ld ",
      min_lat, max_lat, min_lon, max_lon, requested_tiles, built_tiles,
      reused_tiles);
  message += wxString::Format(
      "build_ms=%d cells=%d land=%d water=%d drying=%d unknown=%d "
      "point_cache_hits=%d point_cache_misses=%d grid_cache_size=%lu "
      "grid_cache_evictions=%ld",
      stats.grid_build_ms, stats.grid_cells_total, stats.grid_cells_land,
      stats.grid_cells_water, stats.grid_cells_drying, stats.grid_cells_unknown,
      stats.point_cache_hits, stats.point_cache_misses,
      (unsigned long)SegmentSafetyGridCacheSize(),
      s_segment_safety_grid_cache_evictions);
  wxLogMessage("%s", message.c_str());

  return true;
}

bool PrepareGridForSegment(double lat1, double lon1, double lat2, double lon2,
                           double safety_margin_nm,
                           SegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);

  if (result && result->struct_size < (int)sizeof(int)) return false;

  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);

  const double tile_sample_spacing_nm = 1.5;
  const int max_samples = 1024;
  int samples = wxMax(
      2, wxMin(max_samples, (int)ceil(dist_nm / tile_sample_spacing_nm) + 1));
  std::set<std::pair<long, long>> tiles;

  double offset_step_nm = safety_margin_nm > 0.0 ? 2.0 : 1.0;
  int offset_count = safety_margin_nm > 0.0
                         ? (int)ceil((2.0 * safety_margin_nm) / offset_step_nm)
                         : 0;

  for (int offset_index = 0; offset_index <= offset_count; ++offset_index) {
    double offset_nm = 0.0;
    if (offset_count > 0)
      offset_nm = -safety_margin_nm +
                  (2.0 * safety_margin_nm * offset_index) / offset_count;

    for (int i = 0; i < samples; ++i) {
      double sample_dist = samples == 1 ? 0.0 : dist_nm * i / (samples - 1);
      double lat = lat1;
      double lon = lon1;
      if (sample_dist > 0.0)
        ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);
      if (fabs(offset_nm) > 0.0) {
        double offset_lat, offset_lon;
        ll_gc_ll(lat, lon,
                 SegmentSafetyNormalizeBearing(
                     bearing + (offset_nm < 0.0 ? -90.0 : 90.0)),
                 fabs(offset_nm), &offset_lat, &offset_lon);
        lat = offset_lat;
        lon = offset_lon;
      }

      long lat_tile = 0;
      long lon_tile = 0;
      SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
      tiles.insert(std::make_pair(lat_tile, lon_tile));
    }
  }

  SegmentSafetyCoreStats stats;
  long built_tiles = 0;
  long reused_tiles = 0;

  for (std::set<std::pair<long, long>>::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    std::string key = SegmentSafetyGridTileKeyForIndices(it->first, it->second);
    if (LookupSegmentSafetyGridTile(key, NULL)) {
      ++stats.grid_cache_hits;
      ++reused_tiles;
      continue;
    }
    bool built = false;
    EnsureSegmentSafetyGridTile(it->first, it->second, &stats, &built);
    if (built) ++built_tiles;
  }

  if (result) {
    SetSegmentSafetyStatus(result, kSafe);
    SetSegmentSafetySource(result, kSourceVectorChart);
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryClear);
    SetSegmentSafetyMessage(result, "segment safety corridor prewarmed");
    ApplySegmentSafetyStats(result, stats);
  }

  wxString message = wxString::Format(
      "SEGMENT_SAFETY_GRID prewarm_segment start=(%.6f,%.6f) "
      "end=(%.6f,%.6f) margin_nm=%.3f samples=%d requested_tiles=%lu "
      "built_tiles=%ld reused_tiles=%ld ",
      lat1, lon1, lat2, lon2, safety_margin_nm, samples,
      static_cast<unsigned long>(tiles.size()), built_tiles, reused_tiles);
  message += wxString::Format(
      "build_ms=%d cells=%d land=%d water=%d drying=%d unknown=%d "
      "point_cache_hits=%d point_cache_misses=%d grid_cache_size=%lu "
      "grid_cache_evictions=%ld",
      stats.grid_build_ms, stats.grid_cells_total, stats.grid_cells_land,
      stats.grid_cells_water, stats.grid_cells_drying, stats.grid_cells_unknown,
      stats.point_cache_hits, stats.point_cache_misses,
      (unsigned long)SegmentSafetyGridCacheSize(),
      s_segment_safety_grid_cache_evictions);
  wxLogMessage("%s", message.c_str());

  return true;
}

bool PrepareRouteMask(double min_lat, double min_lon, double max_lat,
                      double max_lon, const SegmentSafetyOptions* options,
                      SegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);
  SegmentSafetyRefreshPersistentChartIdentity();
  SegmentSafetyPersistentCacheEnsureLoaded();

  if (result && result->struct_size < (int)sizeof(int)) return false;

  SegmentSafetyOptions effective_options = {};
  effective_options.struct_size = sizeof(effective_options);
  if (options) effective_options = *options;
  double safety_margin_nm = wxMax(0.0, effective_options.safety_margin_nm);
  bool check_depth = effective_options.check_depth != 0;
  double minimum_depth_m = effective_options.minimum_depth_m;

  if (min_lat > max_lat) std::swap(min_lat, max_lat);
  if (min_lon > max_lon) std::swap(min_lon, max_lon);
  min_lat = wxMax(-90.0, wxMin(90.0, min_lat));
  max_lat = wxMax(-90.0, wxMin(90.0, max_lat));

  long min_lat_tile = floor(min_lat / kGridTileDegrees);
  long max_lat_tile = floor(max_lat / kGridTileDegrees);
  long min_lon_tile = floor(min_lon / kGridTileDegrees);
  long max_lon_tile = floor(max_lon / kGridTileDegrees);

  long lat_count = max_lat_tile - min_lat_tile + 1;
  long lon_count = max_lon_tile - min_lon_tile + 1;
  long requested_tiles =
      lat_count > 0 && lon_count > 0 ? lat_count * lon_count : 0;

  // Keep the complete active envelope resident.  In particular, do this
  // before the first tile build so an envelope larger than the inactive cache
  // target cannot evict its own start tiles while it is still being built.
  PinSegmentSafetyRouteMaskEnvelope(min_lat_tile, max_lat_tile, min_lon_tile,
                                    max_lon_tile, safety_margin_nm, check_depth,
                                    minimum_depth_m);

  SegmentSafetyCoreStats stats;
  long base_built_tiles = 0;
  long base_reused_tiles = 0;
  long mask_built_tiles = 0;
  long mask_reused_tiles = 0;
  long fine_tiles_avoided_by_certified_safe = 0;
  long coarse_requested_cells = 0;
  long coarse_built_cells = 0;
  long coarse_reused_cells = 0;
  long coarse_certified_safe_cells = 0;
  long coarse_missing_cells = 0;

  for (long lat_tile = min_lat_tile; lat_tile <= max_lat_tile; ++lat_tile) {
    for (long lon_tile = min_lon_tile; lon_tile <= max_lon_tile; ++lon_tile) {
      CachedSegmentSafetyCoarseRouteMaskCell certified_coarse;
      if (LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
              lat_tile, lon_tile, safety_margin_nm, check_depth,
              minimum_depth_m, &certified_coarse)) {
        std::string mask_key = SegmentSafetyRouteMaskKey(
            lat_tile, lon_tile, safety_margin_nm, check_depth, minimum_depth_m);
        if (!LookupSegmentSafetyRouteMaskTile(mask_key, NULL)) {
          StoreSegmentSafetyRouteMaskTile(
              mask_key, BuildPersistentCertifiedSafeRouteMaskTile(
                            lat_tile, lon_tile, safety_margin_nm, check_depth,
                            minimum_depth_m, certified_coarse));
        }
        ++fine_tiles_avoided_by_certified_safe;
        ++base_reused_tiles;
        ++mask_reused_tiles;
        ++stats.fine_tiles_avoided;
        continue;
      }

      std::string base_key =
          SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
      if (LookupSegmentSafetyGridTile(base_key, NULL)) {
        ++stats.grid_cache_hits;
        ++base_reused_tiles;
      } else {
        bool built = false;
        EnsureSegmentSafetyGridTile(lat_tile, lon_tile, &stats, &built);
        if (built)
          ++base_built_tiles;
        else
          ++base_reused_tiles;
      }

      bool mask_built = false;
      std::string mask_key = SegmentSafetyRouteMaskKey(
          lat_tile, lon_tile, safety_margin_nm, check_depth, minimum_depth_m);
      if (LookupSegmentSafetyRouteMaskTile(mask_key, NULL)) {
        ++mask_reused_tiles;
      } else if (EnsureSegmentSafetyRouteMaskTile(
                     lat_tile, lon_tile, safety_margin_nm, check_depth,
                     minimum_depth_m, &stats, &mask_built)) {
        if (mask_built)
          ++mask_built_tiles;
        else
          ++mask_reused_tiles;
      }
    }
  }

  double coarse_degrees = kGridTileDegrees * kCoarseRouteMaskFactor;
  long min_coarse_lat = (long)floor(min_lat / coarse_degrees);
  long max_coarse_lat = (long)floor(max_lat / coarse_degrees);
  long min_coarse_lon = (long)floor(min_lon / coarse_degrees);
  long max_coarse_lon = (long)floor(max_lon / coarse_degrees);
  for (long lat_cell = min_coarse_lat; lat_cell <= max_coarse_lat; ++lat_cell) {
    for (long lon_cell = min_coarse_lon; lon_cell <= max_coarse_lon;
         ++lon_cell) {
      ++coarse_requested_cells;
      std::string coarse_key = SegmentSafetyCoarseRouteMaskKey(
          lat_cell, lon_cell, safety_margin_nm, check_depth, minimum_depth_m);
      CachedSegmentSafetyCoarseRouteMaskCell coarse;
      if (LookupSegmentSafetyCoarseRouteMaskCell(coarse_key, &coarse)) {
        ++coarse_reused_cells;
      } else if (EnsureSegmentSafetyCoarseRouteMaskCell(
                     lat_cell, lon_cell, safety_margin_nm, check_depth,
                     minimum_depth_m, &coarse, &stats)) {
        ++coarse_built_cells;
      } else {
        ++coarse_missing_cells;
        continue;
      }
      if (coarse.state == kCoarseCertifiedSafe) ++coarse_certified_safe_cells;
    }
  }

  if (result) {
    SetSegmentSafetyStatus(result, kSafe);
    SetSegmentSafetySource(result, kSourceVectorChart);
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryClear);
    SetSegmentSafetyMessage(result, "segment safety route mask prewarmed");
    ApplySegmentSafetyStats(result, stats);
  }

  wxString message = wxString::Format(
      "WR_ROUTE_MASK_PREWARM bbox=[lat %.6f..%.6f lon %.6f..%.6f] "
      "requested_tiles=%ld base_built=%ld base_reused=%ld "
      "masks_built=%ld masks_reused=%ld fine_tiles_avoided=%ld "
      "margin_nm=%.3f depth_check=%d min_depth_m=%.2f coarse_requested=%ld "
      "coarse_built=%ld coarse_reused=%ld coarse_certified_safe=%ld "
      "coarse_missing=%ld ",
      min_lat, max_lat, min_lon, max_lon, requested_tiles, base_built_tiles,
      base_reused_tiles, mask_built_tiles, mask_reused_tiles,
      fine_tiles_avoided_by_certified_safe, safety_margin_nm,
      check_depth ? 1 : 0, minimum_depth_m, coarse_requested_cells,
      coarse_built_cells, coarse_reused_cells, coarse_certified_safe_cells,
      coarse_missing_cells);
  message += wxString::Format(
      "build_ms=%d cells=%d land=%d water=%d drying=%d unknown=%d "
      "point_cache_hits=%d point_cache_misses=%d grid_cache_size=%lu "
      "grid_cache_evictions=%ld",
      stats.grid_build_ms, stats.grid_cells_total, stats.grid_cells_land,
      stats.grid_cells_water, stats.grid_cells_drying, stats.grid_cells_unknown,
      stats.point_cache_hits, stats.point_cache_misses,
      (unsigned long)SegmentSafetyGridCacheSize(),
      s_segment_safety_grid_cache_evictions);
  wxLogMessage("%s", message.c_str());
  wxLogMessage(
      "WR_COARSE_SAFETY_BUILD scope=bbox requested=%ld built=%ld reused=%ld "
      "certified_safe=%ld missing=%ld certification_method=fine-mask-all-clear "
      "coarse_build_time_ms=%d fine_tiles_built=%ld fine_tiles_reused=%ld",
      coarse_requested_cells, coarse_built_cells, coarse_reused_cells,
      coarse_certified_safe_cells, coarse_missing_cells, stats.coarse_build_ms,
      mask_built_tiles, mask_reused_tiles);
  SegmentSafetyPersistentCacheSave();

  return true;
}

void AddSegmentSafetyRouteMaskCorridorTiles(
    double lat1, double lon1, double lat2, double lon2,
    double corridor_margin_nm, std::set<std::pair<long, long>>* tiles) {
  if (!tiles || !std::isfinite(lat1) || !std::isfinite(lon1) ||
      !std::isfinite(lat2) || !std::isfinite(lon2))
    return;

  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);
  if (!std::isfinite(dist_nm) || dist_nm < 0.0) return;

  // With no speculative corridor width, use the identical fine-grid
  // supercover traversal used by worker safety queries.  This records the
  // exact route-mask tile footprint without distance-dependent sampling gaps.
  if (corridor_margin_nm <= 0.0) {
    long y0 = lround(lat1 / kGridResolutionDegrees);
    long x0 = lround(lon1 / kGridResolutionDegrees);
    long y1 = lround(lat2 / kGridResolutionDegrees);
    long x1 = lround(lon2 / kGridResolutionDegrees);
    long dx = labs(x1 - x0);
    long dy = labs(y1 - y0);
    long sx = x0 < x1 ? 1 : -1;
    long sy = y0 < y1 ? 1 : -1;
    long err = dx - dy;
    long x = x0;
    long y = y0;
    for (;;) {
      double lat = y * kGridResolutionDegrees;
      double lon = x * kGridResolutionDegrees;
      long lat_tile = 0;
      long lon_tile = 0;
      SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
      tiles->insert(std::make_pair(lat_tile, lon_tile));
      if (x == x1 && y == y1) break;
      long e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        x += sx;
      }
      if (e2 < dx) {
        err += dx;
        y += sy;
      }
    }
    return;
  }

  const double tile_sample_spacing_nm = 1.5;
  const int max_samples = 1024;
  int samples = wxMax(
      2, wxMin(max_samples, (int)ceil(dist_nm / tile_sample_spacing_nm) + 1));
  double offset_step_nm = corridor_margin_nm > 0.0 ? 2.0 : 1.0;
  int offset_count =
      corridor_margin_nm > 0.0
          ? (int)ceil((2.0 * corridor_margin_nm) / offset_step_nm)
          : 0;

  for (int offset_index = 0; offset_index <= offset_count; ++offset_index) {
    double offset_nm = 0.0;
    if (offset_count > 0)
      offset_nm = -corridor_margin_nm +
                  (2.0 * corridor_margin_nm * offset_index) / offset_count;

    for (int i = 0; i < samples; ++i) {
      double sample_dist = dist_nm * i / (samples - 1);
      double lat = lat1;
      double lon = lon1;
      if (sample_dist > 0.0)
        ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);
      if (fabs(offset_nm) > 0.0) {
        double offset_lat = lat;
        double offset_lon = lon;
        ll_gc_ll(lat, lon,
                 SegmentSafetyNormalizeBearing(
                     bearing + (offset_nm < 0.0 ? -90.0 : 90.0)),
                 fabs(offset_nm), &offset_lat, &offset_lon);
        lat = offset_lat;
        lon = offset_lon;
      }
      long lat_tile = 0;
      long lon_tile = 0;
      SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
      tiles->insert(std::make_pair(lat_tile, lon_tile));
    }
  }

  // Give the route ends rounded caps instead of relying on perpendicular
  // offsets alone.  This also provides the required start/end neighbourhoods.
  if (corridor_margin_nm > 0.0) {
    const int radial_bearings = 24;
    const int radial_steps = wxMax(1, (int)ceil(corridor_margin_nm / 2.0));
    const double endpoint_lats[2] = {lat1, lat2};
    const double endpoint_lons[2] = {lon1, lon2};
    for (int endpoint = 0; endpoint < 2; ++endpoint) {
      for (int radius_step = 0; radius_step <= radial_steps; ++radius_step) {
        double radius_nm = corridor_margin_nm * radius_step / radial_steps;
        for (int direction = 0; direction < radial_bearings; ++direction) {
          double lat = endpoint_lats[endpoint];
          double lon = endpoint_lons[endpoint];
          if (radius_nm > 0.0)
            ll_gc_ll(endpoint_lats[endpoint], endpoint_lons[endpoint],
                     360.0 * direction / radial_bearings, radius_nm, &lat,
                     &lon);
          long lat_tile = 0;
          long lon_tile = 0;
          SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
          tiles->insert(std::make_pair(lat_tile, lon_tile));
        }
      }
    }
  }
}

bool PrewarmSegmentSafetyRouteMaskTileSet(
    const std::set<std::pair<long, long>>& tiles, double corridor_margin_nm,
    const SegmentSafetyOptions& options, SegmentSafetyResult* result,
    const char* scope, int polyline_count, int segment_count,
    int fine_tile_halo) {
  double safety_margin_nm = wxMax(0.0, options.safety_margin_nm);
  bool check_depth = options.check_depth != 0;
  double minimum_depth_m = options.minimum_depth_m;

  if (tiles.empty()) {
    if (result) {
      SetSegmentSafetyStatus(result, kNoData);
      SetSegmentSafetySource(result, kSourceNone);
      SetSegmentSafetyMessage(result, "route mask prewarm geometry is empty");
    }
    return false;
  }

  PinSegmentSafetyRouteMaskTiles(tiles, safety_margin_nm, check_depth,
                                 minimum_depth_m);

  SegmentSafetyCoreStats stats;
  long base_built_tiles = 0;
  long base_reused_tiles = 0;
  long mask_built_tiles = 0;
  long mask_reused_tiles = 0;
  long fine_tiles_avoided_by_certified_safe = 0;
  long coarse_requested_cells = 0;
  long coarse_built_cells = 0;
  long coarse_reused_cells = 0;
  long coarse_certified_safe_cells = 0;
  long coarse_missing_cells = 0;

  std::map<std::pair<long, long>, std::set<std::pair<long, long>>>
      coarse_occupancy;
  for (std::set<std::pair<long, long>>::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    const long coarse_lat =
        (long)floor((double)it->first / kCoarseRouteMaskFactor);
    const long coarse_lon =
        (long)floor((double)it->second / kCoarseRouteMaskFactor);
    coarse_occupancy[std::make_pair(coarse_lat, coarse_lon)].insert(*it);
  }

  long coarse_base_proof_cells = 0;
  long coarse_base_proof_chart_build_cells = 0;
  for (std::map<std::pair<long, long>,
                std::set<std::pair<long, long>>>::const_iterator it =
           coarse_occupancy.begin();
       it != coarse_occupancy.end(); ++it) {
    ++coarse_requested_cells;
    const long coarse_lat = it->first.first;
    const long coarse_lon = it->first.second;
    const std::string coarse_key = SegmentSafetyCoarseRouteMaskKey(
        coarse_lat, coarse_lon, safety_margin_nm, check_depth, minimum_depth_m);
    CachedSegmentSafetyCoarseRouteMaskCell coarse;
    if (LookupSegmentSafetyCoarseRouteMaskCell(coarse_key, &coarse)) {
      ++coarse_reused_cells;
      continue;
    }
    if (SegmentSafetyPersistentLookupCertifiedSafe(
            coarse_lat, coarse_lon, safety_margin_nm, check_depth,
            minimum_depth_m, &coarse)) {
      StoreSegmentSafetyCoarseRouteMaskCell(coarse_key, coarse);
      ++coarse_reused_cells;
      continue;
    }

    bool built_from_base = BuildSegmentSafetyCoarseRouteMaskCellFromBase(
        coarse_lat, coarse_lon, safety_margin_nm, check_depth, minimum_depth_m,
        false, &coarse, &stats, &base_built_tiles);
    if (!built_from_base &&
        SegmentSafetyCoarseProofDoesNotExpandBaseBuildSet(
            coarse_lat, coarse_lon, it->second, safety_margin_nm)) {
      built_from_base = BuildSegmentSafetyCoarseRouteMaskCellFromBase(
          coarse_lat, coarse_lon, safety_margin_nm, check_depth,
          minimum_depth_m, true, &coarse, &stats, &base_built_tiles);
      if (built_from_base) ++coarse_base_proof_chart_build_cells;
    }
    if (built_from_base) {
      StoreSegmentSafetyCoarseRouteMaskCell(coarse_key, coarse);
      SegmentSafetyPersistentStoreCertifiedSafe(coarse);
      ++coarse_built_cells;
      ++coarse_base_proof_cells;
    }
  }

  // Build the complete fine-grid evidence halo before deriving any route
  // masks.  A mask with a non-zero safety margin depends on neighbouring base
  // tiles even when only its own route tile is requested.  Building masks
  // immediately after each centre tile left those neighbours to be discovered
  // on demand, causing long GUI-thread stalls and defeating the prewarm
  // contract.
  std::set<std::pair<long, long>> certified_fine_tiles;
  std::set<std::pair<long, long>> fine_base_tiles;
  for (std::set<std::pair<long, long>>::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    CachedSegmentSafetyCoarseRouteMaskCell certified_coarse;
    if (LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
            it->first, it->second, safety_margin_nm, check_depth,
            minimum_depth_m, &certified_coarse)) {
      certified_fine_tiles.insert(*it);
      continue;
    }
    const double tile_min_lat = it->first * kGridTileDegrees;
    const int margin_radius = SegmentSafetyMarginTileRadius(
        safety_margin_nm,
        wxMax(fabs(tile_min_lat), fabs(tile_min_lat + kGridTileDegrees)));
    AddSegmentSafetyTileHalo(it->first, it->second, margin_radius,
                             &fine_base_tiles);
  }

  // Licensed plugin-vector providers can classify a rectangular grid in one
  // semantic object traversal.  Prebuild adjacent requested tiles in bounded
  // blocks, then retain the existing per-tile path for mixed-provider,
  // unsupported or failed cells.  This changes only cache population; route
  // masks, missing-tile retries and routing decisions remain identical.
  const std::set<std::pair<long, long>> plugin_batch_built_tiles =
      PrebuildSegmentSafetyPluginVectorGridTiles(fine_base_tiles, &stats,
                                                 check_depth);

  for (std::set<std::pair<long, long>>::const_iterator it =
           fine_base_tiles.begin();
       it != fine_base_tiles.end(); ++it) {
    const std::string base_key =
        SegmentSafetyGridTileKeyForIndices(it->first, it->second);
    if (LookupSegmentSafetyGridTile(base_key, NULL)) {
      ++stats.grid_cache_hits;
      if (plugin_batch_built_tiles.count(*it))
        ++base_built_tiles;
      else
        ++base_reused_tiles;
    } else {
      bool built = false;
      EnsureSegmentSafetyGridTile(it->first, it->second, &stats, &built);
      if (built)
        ++base_built_tiles;
      else
        ++base_reused_tiles;
    }
  }

  for (std::set<std::pair<long, long>>::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    if (certified_fine_tiles.count(*it) != 0) {
      CachedSegmentSafetyCoarseRouteMaskCell certified_coarse;
      LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
          it->first, it->second, safety_margin_nm, check_depth, minimum_depth_m,
          &certified_coarse);
      std::string mask_key =
          SegmentSafetyRouteMaskKey(it->first, it->second, safety_margin_nm,
                                    check_depth, minimum_depth_m);
      if (!LookupSegmentSafetyRouteMaskTile(mask_key, NULL)) {
        StoreSegmentSafetyRouteMaskTile(
            mask_key, BuildPersistentCertifiedSafeRouteMaskTile(
                          it->first, it->second, safety_margin_nm, check_depth,
                          minimum_depth_m, certified_coarse));
      }
      ++fine_tiles_avoided_by_certified_safe;
      ++base_reused_tiles;
      ++mask_reused_tiles;
      ++stats.fine_tiles_avoided;
      continue;
    }

    bool mask_built = false;
    std::string mask_key = SegmentSafetyRouteMaskKey(
        it->first, it->second, safety_margin_nm, check_depth, minimum_depth_m);
    if (LookupSegmentSafetyRouteMaskTile(mask_key, NULL)) {
      ++mask_reused_tiles;
    } else if (EnsureSegmentSafetyRouteMaskTile(
                   it->first, it->second, safety_margin_nm, check_depth,
                   minimum_depth_m, &stats, &mask_built)) {
      if (mask_built)
        ++mask_built_tiles;
      else
        ++mask_reused_tiles;
    }
  }

  for (std::map<std::pair<long, long>,
                std::set<std::pair<long, long>>>::const_iterator it =
           coarse_occupancy.begin();
       it != coarse_occupancy.end(); ++it) {
    std::string coarse_key = SegmentSafetyCoarseRouteMaskKey(
        it->first.first, it->first.second, safety_margin_nm, check_depth,
        minimum_depth_m);
    CachedSegmentSafetyCoarseRouteMaskCell coarse;
    if (!LookupSegmentSafetyCoarseRouteMaskCell(coarse_key, &coarse) &&
        EnsureSegmentSafetyCoarseRouteMaskCell(
            it->first.first, it->first.second, safety_margin_nm, check_depth,
            minimum_depth_m, &coarse, &stats)) {
      ++coarse_built_cells;
    } else if (!LookupSegmentSafetyCoarseRouteMaskCell(coarse_key, &coarse)) {
      ++coarse_missing_cells;
      continue;
    }
    if (coarse.state == kCoarseCertifiedSafe) ++coarse_certified_safe_cells;
  }

  if (result) {
    SetSegmentSafetyStatus(result, kSafe);
    SetSegmentSafetySource(result, kSourceVectorChart);
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryClear);
    SetSegmentSafetyMessage(result,
                            "segment safety route mask shape prewarmed");
    ApplySegmentSafetyStats(result, stats);
    if (result->struct_size >=
        (int)(offsetof(SegmentSafetyResult, prewarm_fine_tiles_avoided) +
              sizeof(result->prewarm_fine_tiles_avoided))) {
      result->prewarm_requested_tiles = static_cast<int>(tiles.size());
      result->prewarm_base_tiles_built = static_cast<int>(base_built_tiles);
      result->prewarm_base_tiles_reused = static_cast<int>(base_reused_tiles);
      result->prewarm_masks_built = static_cast<int>(mask_built_tiles);
      result->prewarm_masks_reused = static_cast<int>(mask_reused_tiles);
      result->prewarm_fine_tiles_avoided =
          static_cast<int>(fine_tiles_avoided_by_certified_safe);
    }
  }

  wxLogMessage(
      "WR_ROUTE_MASK_PREWARM_SHAPE scope=%s segments=%d "
      "corridor_margin_nm=%.3f fine_tile_halo=%d margin_nm=%.3f "
      "requested_tiles=%lu "
      "base_built=%ld base_reused=%ld masks_built=%ld masks_reused=%ld "
      "fine_tiles_avoided=%ld depth_check=%d min_depth_m=%.2f "
      "coarse_requested=%ld coarse_built=%ld coarse_reused=%ld "
      "coarse_certified_safe=%ld coarse_missing=%ld "
      "coarse_base_proofs=%ld coarse_base_proof_chart_builds=%ld "
      "build_ms=%d cells=%d "
      "land=%d water=%d drying=%d unknown=%d point_cache_hits=%d "
      "point_cache_misses=%d grid_cache_size=%lu grid_cache_evictions=%ld",
      scope ? scope : "unknown", segment_count, corridor_margin_nm,
      fine_tile_halo, safety_margin_nm,
      static_cast<unsigned long>(tiles.size()), base_built_tiles,
      base_reused_tiles, mask_built_tiles, mask_reused_tiles,
      fine_tiles_avoided_by_certified_safe, check_depth ? 1 : 0,
      minimum_depth_m, coarse_requested_cells, coarse_built_cells,
      coarse_reused_cells, coarse_certified_safe_cells, coarse_missing_cells,
      coarse_base_proof_cells, coarse_base_proof_chart_build_cells,
      stats.grid_build_ms, stats.grid_cells_total, stats.grid_cells_land,
      stats.grid_cells_water, stats.grid_cells_drying, stats.grid_cells_unknown,
      stats.point_cache_hits, stats.point_cache_misses,
      (unsigned long)SegmentSafetyGridCacheSize(),
      s_segment_safety_grid_cache_evictions);
  wxLogMessage(
      "WR_COARSE_SAFETY_BUILD scope=%s requested=%ld built=%ld reused=%ld "
      "certified_safe=%ld missing=%ld base_proofs=%ld "
      "base_proof_chart_builds=%ld "
      "certification_method=base-proof-first-then-fine-mask "
      "coarse_build_time_ms=%d fine_tiles_built=%ld fine_tiles_reused=%ld",
      scope ? scope : "unknown", coarse_requested_cells, coarse_built_cells,
      coarse_reused_cells, coarse_certified_safe_cells, coarse_missing_cells,
      coarse_base_proof_cells, coarse_base_proof_chart_build_cells,
      stats.coarse_build_ms, mask_built_tiles, mask_reused_tiles);
  SegmentSafetyPersistentCacheSave();
  return true;
}

bool PrepareRouteMaskForSegment(double lat1, double lon1, double lat2,
                                double lon2, double corridor_margin_nm,
                                const SegmentSafetyOptions* options,
                                SegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);
  SegmentSafetyRefreshPersistentChartIdentity();
  SegmentSafetyPersistentCacheEnsureLoaded();

  if (result && result->struct_size < (int)sizeof(int)) return false;

  SegmentSafetyOptions effective_options = {};
  effective_options.struct_size = sizeof(effective_options);
  if (options) effective_options = *options;
  corridor_margin_nm = wxMax(0.0, corridor_margin_nm);
  std::set<std::pair<long, long>> tiles;
  AddSegmentSafetyRouteMaskCorridorTiles(lat1, lon1, lat2, lon2,
                                         corridor_margin_nm, &tiles);
  return PrewarmSegmentSafetyRouteMaskTileSet(
      tiles, corridor_margin_nm, effective_options, result, "segment", 1, 1, 0);
}

static bool PrewarmSegmentSafetyRouteMaskForPolylinesImpl(
    const double* latitudes, const double* longitudes, const int* point_counts,
    int polyline_count, double corridor_margin_nm, int fine_tile_halo,
    const SegmentSafetyOptions* options, SegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);
  SegmentSafetyRefreshPersistentChartIdentity();
  SegmentSafetyPersistentCacheEnsureLoaded();
  if (result && result->struct_size < (int)sizeof(int)) return false;
  if (!latitudes || !longitudes || !point_counts || polyline_count <= 0)
    return false;

  SegmentSafetyOptions effective_options = {};
  effective_options.struct_size = sizeof(effective_options);
  if (options) effective_options = *options;
  corridor_margin_nm = wxMax(0.0, corridor_margin_nm);

  std::set<std::pair<long, long>> tiles;
  int point_offset = 0;
  int valid_polylines = 0;
  int segment_count = 0;
  for (int polyline = 0; polyline < polyline_count; ++polyline) {
    int count = point_counts[polyline];
    if (count < 0 || count > 1000000) return false;
    if (count == 1) {
      AddSegmentSafetyRouteMaskCorridorTiles(
          latitudes[point_offset], longitudes[point_offset],
          latitudes[point_offset], longitudes[point_offset], corridor_margin_nm,
          &tiles);
      ++valid_polylines;
    } else if (count >= 2) {
      ++valid_polylines;
      for (int point = 1; point < count; ++point) {
        AddSegmentSafetyRouteMaskCorridorTiles(
            latitudes[point_offset + point - 1],
            longitudes[point_offset + point - 1],
            latitudes[point_offset + point], longitudes[point_offset + point],
            corridor_margin_nm, &tiles);
        ++segment_count;
      }
    }
    point_offset += count;
  }

  fine_tile_halo = wxMax(0, wxMin(8, fine_tile_halo));
  if (fine_tile_halo > 0) {
    const std::set<std::pair<long, long>> exact_tiles = tiles;
    for (std::set<std::pair<long, long>>::const_iterator it =
             exact_tiles.begin();
         it != exact_tiles.end(); ++it)
      AddSegmentSafetyTileHalo(it->first, it->second, fine_tile_halo, &tiles);
  }
  return PrewarmSegmentSafetyRouteMaskTileSet(
      tiles, corridor_margin_nm, effective_options, result, "polylines",
      valid_polylines, segment_count, fine_tile_halo);
}

bool PrepareRouteMaskForPolylines(const double* latitudes,
                                  const double* longitudes,
                                  const int* point_counts, int polyline_count,
                                  double corridor_margin_nm,
                                  const SegmentSafetyOptions* options,
                                  SegmentSafetyResult* result) {
  return PrewarmSegmentSafetyRouteMaskForPolylinesImpl(
      latitudes, longitudes, point_counts, polyline_count, corridor_margin_nm,
      0, options, result);
}

bool PrepareCorridor(const double* latitudes, const double* longitudes,
                     const int* point_counts, int polyline_count,
                     double corridor_margin_nm, int fine_tile_halo,
                     const SegmentSafetyOptions* options,
                     SegmentSafetyResult* result) {
  return PrewarmSegmentSafetyRouteMaskForPolylinesImpl(
      latitudes, longitudes, point_counts, polyline_count, corridor_margin_nm,
      fine_tile_halo, options, result);
}

void ReleasePins() {
  size_t base_before = 0;
  size_t masks_before = 0;
  size_t base_after = 0;
  size_t masks_after = 0;
  size_t pinned_base = 0;
  size_t pinned_masks = 0;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    base_before = s_segment_safety_grid_cache.size();
    masks_before = s_segment_safety_route_mask_cache.size();
    pinned_base = s_segment_safety_pinned_grid_keys.size();
    pinned_masks = s_segment_safety_pinned_route_mask_keys.size();
    s_segment_safety_pinned_grid_keys.clear();
    s_segment_safety_pinned_route_mask_keys.clear();

    while (s_segment_safety_grid_cache.size() > kMaxGridTiles) {
      s_segment_safety_grid_cache.erase(s_segment_safety_grid_cache.begin());
      ++s_segment_safety_grid_cache_evictions;
    }
    while (s_segment_safety_route_mask_cache.size() > kMaxGridTiles) {
      s_segment_safety_route_mask_cache.erase(
          s_segment_safety_route_mask_cache.begin());
    }
    base_after = s_segment_safety_grid_cache.size();
    masks_after = s_segment_safety_route_mask_cache.size();
  }
  wxLogMessage(
      "WR_ROUTE_MASK_PINS_RELEASED pinned_base=%lu pinned_masks=%lu "
      "base_before=%lu base_after=%lu masks_before=%lu masks_after=%lu "
      "evictions=%ld",
      (unsigned long)pinned_base, (unsigned long)pinned_masks,
      (unsigned long)base_before, (unsigned long)base_after,
      (unsigned long)masks_before, (unsigned long)masks_after,
      s_segment_safety_grid_cache_evictions);
}

bool RegisterTileCache(const SegmentSafetyTileCacheCallbacks* callbacks) {
  if (!wxThread::IsMain()) return false;
  wxString identity = SegmentSafetyChartIdentity();
  SegmentSafetyTileCacheIdentity identity_callback = nullptr;
  void* identity_context = nullptr;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    if (!callbacks) {
      memset(&s_segment_safety_external_tile_cache, 0,
             sizeof(s_segment_safety_external_tile_cache));
      wxLogMessage("WR_PLUGIN_TILE_CACHE registered=0");
      return true;
    }
    if (callbacks->struct_size <
            static_cast<int>(
                offsetof(SegmentSafetyTileCacheCallbacks, identity_changed) +
                sizeof(callbacks->identity_changed)) ||
        !callbacks->lookup || !callbacks->store)
      return false;
    memset(&s_segment_safety_external_tile_cache, 0,
           sizeof(s_segment_safety_external_tile_cache));
    memcpy(&s_segment_safety_external_tile_cache, callbacks,
           std::min(static_cast<size_t>(callbacks->struct_size),
                    sizeof(s_segment_safety_external_tile_cache)));
    identity_callback = callbacks->identity_changed;
    identity_context = callbacks->context;
  }
  if (identity_callback) {
    wxCharBuffer utf8 = identity.ToUTF8();
    identity_callback(identity_context, utf8.data() ? utf8.data() : "");
  }
  wxLogMessage(
      "WR_PLUGIN_TILE_CACHE registered=1 identity=\"%s\" lookup=1 store=1",
      identity);
  return true;
}

bool GetChartIdentity(std::string* identity) {
  if (!identity || !wxThread::IsMain()) return false;
  wxString value = SegmentSafetyChartIdentity();
  wxCharBuffer utf8 = value.ToUTF8();
  *identity = utf8.data() ? utf8.data() : "";
  return !value.IsEmpty();
}

int GetChartInfoCount() {
  if (!wxThread::IsMain() || !ChartData) return 0;
  return ChartData->GetChartTableEntries();
}

bool GetChartInfo(int ordinal, SegmentSafetyChartInfo* chart_info) {
  if (!wxThread::IsMain() || !ChartData || !chart_info || ordinal < 0 ||
      ordinal >= ChartData->GetChartTableEntries() ||
      chart_info->struct_size <
          static_cast<int>(sizeof(SegmentSafetyChartInfo)))
    return false;

  const ChartTableEntry& entry = ChartData->GetChartTableEntry(ordinal);
  const ChartTypeEnum type = static_cast<ChartTypeEnum>(entry.GetChartType());
  const ChartFamilyEnum family =
      static_cast<ChartFamilyEnum>(entry.GetChartFamily());
  const bool plugin_vector =
      type == CHART_TYPE_PLUGIN && family == CHART_FAMILY_VECTOR;
  const bool cm93 = type == CHART_TYPE_CM93 || type == CHART_TYPE_CM93COMP;
  // A proactive atlas targets bounded official/native and plugin vector
  // charts. CM93 is a global composite fallback and must never turn "all
  // charts" into an accidental whole-world atlas build.
  if (cm93 || (!plugin_vector && family != CHART_FAMILY_VECTOR)) return false;
  if (ChartData->IsChartDirectoryExcluded(entry.GetFullPath())) return false;

  const int group_index = SegmentSafetyCurrentGroupIndex();
  const bool in_active_group = ChartData->IsChartInGroup(ordinal, group_index);
  if (!in_active_group) return false;
  const bool available =
      type != CHART_TYPE_PLUGIN || ChartData->IsChartAvailable(ordinal);
  if (!available) return false;

  SegmentSafetyChartInfo result = {};
  result.struct_size = sizeof(result);
  result.abi_version = HostApi123::kChartSafetyProviderAbiVersion;
  result.db_index = ordinal;
  result.chart_type = entry.GetChartType();
  result.chart_family = entry.GetChartFamily();
  result.chart_scale = entry.GetScale();
  result.source = plugin_vector ? kSourcePluginVector : kSourceVectorChart;
  result.available = available ? 1 : 0;
  result.in_active_group = in_active_group ? 1 : 0;
  result.edition_time = static_cast<long long>(entry.GetChartEditionDate());
  result.file_time = static_cast<long long>(entry.GetFileTime());
  result.min_lat = entry.GetLatMin();
  result.min_lon = entry.GetLonMin();
  result.max_lat = entry.GetLatMax();
  result.max_lon = entry.GetLonMax();
  CopySegmentSafetyString(result.chart_path, sizeof(result.chart_path),
                          entry.GetFullPath().c_str());
  *chart_info = result;
  return true;
}

bool GetChartCoverageTiles(const int* chart_db_indices, int chart_count,
                           double tile_degrees, long* lat_tiles,
                           long* lon_tiles, int tile_capacity, int* tile_count,
                           bool* complete) {
  const auto started_at = std::chrono::steady_clock::now();
  if (tile_count) *tile_count = 0;
  if (complete) *complete = false;
  if (!wxThread::IsMain() || !ChartData || !chart_db_indices ||
      chart_count <= 0 || chart_count > ChartData->GetChartTableEntries() ||
      !lat_tiles || !lon_tiles || tile_capacity <= 0 ||
      tile_capacity > 2000000 || !tile_count || !complete ||
      std::abs(tile_degrees - kGridTileDegrees) > 1e-12)
    return false;

  std::set<std::pair<long, long>> tiles;
  bool all_added = true;
  const int group_index = SegmentSafetyCurrentGroupIndex();
  for (int selected = 0; selected < chart_count && all_added; ++selected) {
    const int db_index = chart_db_indices[selected];
    if (db_index < 0 || db_index >= ChartData->GetChartTableEntries())
      return false;
    const ChartTableEntry& entry = ChartData->GetChartTableEntry(db_index);
    const ChartTypeEnum type = static_cast<ChartTypeEnum>(entry.GetChartType());
    const ChartFamilyEnum family =
        static_cast<ChartFamilyEnum>(entry.GetChartFamily());
    const bool plugin_vector =
        type == CHART_TYPE_PLUGIN && family == CHART_FAMILY_VECTOR;
    const bool cm93 = type == CHART_TYPE_CM93 || type == CHART_TYPE_CM93COMP;
    if (cm93 || (!plugin_vector && family != CHART_FAMILY_VECTOR) ||
        ChartData->IsChartDirectoryExcluded(entry.GetFullPath()) ||
        !ChartData->IsChartInGroup(db_index, group_index) ||
        (type == CHART_TYPE_PLUGIN && !ChartData->IsChartAvailable(db_index)))
      continue;

    const auto coverage_ring = [](const float* points, int count) {
      ocpn::chart_safety::CoverageRing ring;
      if (!points || count < 3) return ring;
      ring.reserve(static_cast<std::size_t>(count));
      for (int point = 0; point < count; ++point)
        ring.push_back({points[point * 2], points[point * 2 + 1]});
      return ring;
    };
    std::vector<ocpn::chart_safety::CoverageRing> coverage;
    for (int ring = 0; ring < entry.GetnAuxPlyEntries(); ++ring) {
      auto points = coverage_ring(entry.GetpAuxPlyTableEntry(ring),
                                  entry.GetAuxCntTableEntry(ring));
      if (points.size() >= 3) coverage.push_back(std::move(points));
    }
    if (coverage.empty()) {
      auto points = coverage_ring(entry.GetpPlyTable(), entry.GetnPlyEntries());
      if (points.size() >= 3) coverage.push_back(std::move(points));
    }
    std::vector<ocpn::chart_safety::CoverageRing> no_coverage;
    for (int ring = 0; ring < entry.GetnNoCovrPlyEntries(); ++ring) {
      auto points = coverage_ring(entry.GetpNoCovrPlyTableEntry(ring),
                                  entry.GetNoCovrCntTableEntry(ring));
      if (points.size() >= 3) no_coverage.push_back(std::move(points));
    }
    if (coverage.empty()) continue;
    all_added = ocpn::chart_safety::AddCoverageTiles(
        coverage, no_coverage, tile_degrees,
        static_cast<std::size_t>(tile_capacity), &tiles);
  }

  int output = 0;
  for (const auto& tile : tiles) {
    lat_tiles[output] = tile.first;
    lon_tiles[output] = tile.second;
    ++output;
  }
  *tile_count = output;
  *complete = all_added;
  long min_lat_tile = 0;
  long max_lat_tile = 0;
  long min_lon_tile = 0;
  long max_lon_tile = 0;
  if (!tiles.empty()) {
    min_lat_tile = max_lat_tile = tiles.begin()->first;
    min_lon_tile = max_lon_tile = tiles.begin()->second;
    for (const auto& tile : tiles) {
      min_lat_tile = std::min(min_lat_tile, tile.first);
      max_lat_tile = std::max(max_lat_tile, tile.first);
      min_lon_tile = std::min(min_lon_tile, tile.second);
      max_lon_tile = std::max(max_lon_tile, tile.second);
    }
  }
  const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::steady_clock::now() - started_at)
                              .count();
  wxLogMessage(
      "WR_CHART_ATLAS_COVERAGE charts=%d tiles=%d capacity=%d complete=%d "
      "tile_bbox=%ld,%ld:%ld,%ld elapsed_ms=%lld",
      chart_count, output, tile_capacity, all_added ? 1 : 0, min_lat_tile,
      min_lon_tile, max_lat_tile, max_lon_tile,
      static_cast<long long>(elapsed_ms));
  return true;
}

bool SetPersistentCacheEnabled(bool enabled) {
  const bool was_enabled = s_segment_safety_persistent_cache_enabled;
  const bool will_enable = enabled != 0;
  if (was_enabled && !will_enable) SegmentSafetyPersistentCacheSave();
  s_segment_safety_persistent_cache_enabled = will_enable;
  if (will_enable) {
    SegmentSafetyRefreshPersistentChartIdentity();
    SegmentSafetyPersistentCacheEnsureLoaded();
  }
  wxLogMessage(
      "WR_CERT_SAFE_CACHE enabled=%d entries_loaded=%ld entries_used=%ld "
      "entries_ignored=%ld stale_entries_ignored=%ld base_tiles_loaded=%ld "
      "base_tiles_used=%ld cache_file_path=\"%s\" "
      "base_cache_file_path=\"%s\"",
      s_segment_safety_persistent_cache_enabled ? 1 : 0,
      s_segment_safety_persistent_entries_loaded,
      s_segment_safety_persistent_entries_used,
      s_segment_safety_persistent_entries_ignored,
      s_segment_safety_persistent_stale_ignored,
      s_segment_safety_persistent_base_tiles_loaded_count,
      s_segment_safety_persistent_base_tiles_used,
      SegmentSafetyPersistentCachePath(),
      SegmentSafetyPersistentBaseTileCachePath());
  return true;
}

bool GetPersistentCacheEnabled() {
  return s_segment_safety_persistent_cache_enabled;
}

bool SavePersistentCache() { return SegmentSafetyPersistentCacheSave(); }

bool ClearPersistentCache() {
  wxString path = SegmentSafetyPersistentCachePath();
  wxString base_path = SegmentSafetyPersistentBaseTileCachePath();
  bool removed = true;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    s_segment_safety_persistent_certified_safe_cache.clear();
    s_segment_safety_persistent_cache_dirty = false;
    s_segment_safety_persistent_entries_loaded = 0;
    s_segment_safety_persistent_entries_saved = 0;
    s_segment_safety_persistent_entries_used = 0;
    s_segment_safety_persistent_entries_ignored = 0;
    s_segment_safety_persistent_stale_ignored = 0;
    s_segment_safety_persistent_malformed_ignored = 0;
    s_segment_safety_persistent_entries_stored = 0;
    s_segment_safety_persistent_cache_loaded = true;
    s_segment_safety_persistent_base_tile_cache.clear();
    s_segment_safety_persistent_base_tiles_dirty = false;
    s_segment_safety_persistent_base_tiles_loaded = true;
    s_segment_safety_persistent_base_tiles_loaded_count = 0;
    s_segment_safety_persistent_base_tiles_saved = 0;
    s_segment_safety_persistent_base_tiles_used = 0;
    s_segment_safety_persistent_base_tiles_ignored = 0;
    s_segment_safety_persistent_tiles_since_checkpoint = 0;
  }
  if (wxFileExists(path)) removed = wxRemoveFile(path);
  if (wxFileExists(base_path)) removed = wxRemoveFile(base_path) && removed;
  wxLogMessage(
      "WR_CERT_SAFE_CACHE clear success=%d cache_file_path=\"%s\" "
      "base_cache_file_path=\"%s\"",
      removed ? 1 : 0, path, base_path);
  return removed;
}

}  // namespace ocpn::chart_safety
