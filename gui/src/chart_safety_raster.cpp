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
 * Rasterisation and geometric sampling used by the chart-safety service.
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

namespace detail {

void SegmentSafetyCandidateChartsAt(double lat, double lon,
                                    std::set<int>& chart_indexes,
                                    SegmentSafetyCoreStats* stats) {
  if (!ChartData) {
    if (stats) stats->no_chart_database = true;
    return;
  }

  ChartStack stack;
  ChartData->BuildChartStack(&stack, lat, lon,
                             SegmentSafetyCurrentGroupIndex());
  if (stats) stats->chart_stack_entries += stack.nEntry;
  for (int i = 0; i < stack.nEntry; ++i) {
    int db_index = stack.GetDBIndex(i);
    if (db_index < 0) continue;
    ChartFamilyEnum family =
        (ChartFamilyEnum)ChartData->GetCSChartFamily(&stack, i);
    ChartTypeEnum type = (ChartTypeEnum)ChartData->GetCSChartType(&stack, i);
    // GetCSChartFamily() derives family from the built-in chart type and
    // returns UNKNOWN for CHART_TYPE_PLUGIN, even when the chart-table entry
    // records a vector family.  Keep plugin entries here and validate the
    // opened wrapper in SegmentSafetySortedChartCandidates().
    if (family == CHART_FAMILY_VECTOR || type == CHART_TYPE_PLUGIN ||
        type == CHART_TYPE_CM93 || type == CHART_TYPE_CM93COMP) {
      chart_indexes.insert(db_index);
    } else if (family == CHART_FAMILY_RASTER) {
      if (stats) ++stats->raster_chart_count;
    } else {
      if (stats) ++stats->unsupported_chart_count;
    }
  }
}

bool SegmentSafetyChartCandidateLess(const SegmentSafetyChartCandidate& a,
                                     const SegmentSafetyChartCandidate& b) {
  // A licensed/native vector chart is authoritative for its coverage.  CM93
  // is retained only as a degraded vector fallback even when its dynamically
  // selected local scale happens to have a smaller denominator.
  if (a.provider_priority != b.provider_priority)
    return a.provider_priority < b.provider_priority;
  if (a.native_scale != b.native_scale) return a.native_scale < b.native_scale;
  if (a.edition_date != b.edition_date) return a.edition_date > b.edition_date;
  if (a.file_time != b.file_time) return a.file_time > b.file_time;
  if (a.path != b.path) return a.path < b.path;
  return a.db_index < b.db_index;
}

bool SegmentSafetyLongitudeRangesOverlap(double first_min, double first_max,
                                         double second_min, double second_max) {
  for (int shift = -1; shift <= 1; ++shift) {
    const double shifted_min = second_min + shift * 360.0;
    const double shifted_max = second_max + shift * 360.0;
    if (shifted_max >= first_min && shifted_min <= first_max) return true;
  }
  return false;
}

std::vector<SegmentSafetyChartCandidate> SegmentSafetyTileChartCandidates(
    double min_lat, double min_lon, double max_lat, double max_lon) {
  std::vector<SegmentSafetyChartCandidate> candidates;
  if (!ChartData) return candidates;

  const ocpn::chart_safety::GeographicBounds discovery =
      ocpn::chart_safety::ExpandCandidateDiscoveryBounds(
          {min_lat, min_lon, max_lat, max_lon}, kGridResolutionDegrees);

  const int group_index = SegmentSafetyCurrentGroupIndex();
  const int entries = ChartData->GetChartTableEntries();
  candidates.reserve(entries);
  for (int db_index = 0; db_index < entries; ++db_index) {
    ChartTableEntry& entry = ChartData->GetChartTableEntry(db_index);
    const ChartTypeEnum type = static_cast<ChartTypeEnum>(entry.GetChartType());
    const ChartFamilyEnum family =
        static_cast<ChartFamilyEnum>(entry.GetChartFamily());
    const bool plugin_vector =
        type == CHART_TYPE_PLUGIN && family == CHART_FAMILY_VECTOR;
    const bool cm93 = type == CHART_TYPE_CM93 || type == CHART_TYPE_CM93COMP;
    if (!plugin_vector && !cm93 && family != CHART_FAMILY_VECTOR) continue;
    if (ChartData->IsChartDirectoryExcluded(entry.GetFullPath())) continue;
    if (!ChartData->IsChartInGroup(db_index, group_index)) continue;
    if (type == CHART_TYPE_PLUGIN && !ChartData->IsChartAvailable(db_index))
      continue;
    if (entry.GetLatMax() < discovery.min_lat ||
        entry.GetLatMin() > discovery.max_lat)
      continue;
    if (!SegmentSafetyLongitudeRangesOverlap(
            entry.GetLonMin(), entry.GetLonMax(), discovery.min_lon,
            discovery.max_lon))
      continue;

    SegmentSafetyChartCandidate candidate;
    candidate.db_index = db_index;
    candidate.provider_priority = cm93 ? 1 : 0;
    candidate.native_scale = entry.GetScale();
    candidate.edition_date = entry.GetChartEditionDate();
    candidate.file_time = entry.GetFileTime();
    candidate.plugin_vector = plugin_vector;
    candidate.cm93 = cm93;
    candidate.path = entry.GetFullPath();
    candidates.push_back(candidate);
  }
  std::sort(candidates.begin(), candidates.end(),
            SegmentSafetyChartCandidateLess);
  return candidates;
}

wxString SegmentSafetyTileDependencyIdentity(long lat_tile, long lon_tile) {
  uint64_t hash = 1469598103934665603ULL;
  SegmentSafetyHashAdd(&hash, "tile-dependencies-v1");
  SegmentSafetyHashAdd(
      &hash, wxString::Format("group=%d", SegmentSafetyCurrentGroupIndex()));
  const double min_lat = lat_tile * kGridTileDegrees;
  const double min_lon = lon_tile * kGridTileDegrees;
  const std::vector<SegmentSafetyChartCandidate> candidates =
      SegmentSafetyTileChartCandidates(min_lat, min_lon,
                                       min_lat + kGridTileDegrees,
                                       min_lon + kGridTileDegrees);
  for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
           candidates.begin();
       it != candidates.end(); ++it) {
    SegmentSafetyHashAdd(&hash, wxString::FromUTF8(it->path.c_str()));
    SegmentSafetyHashAdd(
        &hash, wxString::Format("p=%d:s=%d:e=%lld:f=%lld:plugin=%d:cm93=%d",
                                it->provider_priority, it->native_scale,
                                static_cast<long long>(it->edition_date),
                                static_cast<long long>(it->file_time),
                                it->plugin_vector ? 1 : 0, it->cm93 ? 1 : 0));
  }
  return wxString::Format("tile-v1-%016llx",
                          static_cast<unsigned long long>(hash));
}

bool SegmentSafetyTileDependencyIsCurrent(
    const CachedPointSafetyGridTile& tile) {
  if (!wxThread::IsMain() || !tile.dependency_identity[0]) return false;
  return SegmentSafetyTileDependencyIdentity(tile.lat_tile, tile.lon_tile) ==
         wxString::FromUTF8(tile.dependency_identity);
}

std::vector<SegmentSafetyChartCandidate> SegmentSafetySortedChartCandidates(
    double lat, double lon, const std::set<int>& chart_indexes) {
  std::vector<SegmentSafetyChartCandidate> candidates;
  ViewPort detail_vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
  for (std::set<int>::const_iterator it = chart_indexes.begin();
       it != chart_indexes.end(); ++it) {
    ChartBase* chart =
        ChartData ? ChartData->OpenChartFromDB(*it, FULL_INIT) : NULL;
    s57chart* s57 = dynamic_cast<s57chart*>(chart);
    const bool plugin_vector = IsSupportedSegmentSafetyPluginChart(chart);
    if (!s57 && !plugin_vector) continue;
    const bool cm93 = IsCm93Chart(chart);
    if (cm93) {
      cm93compchart* cm93_chart = dynamic_cast<cm93compchart*>(chart);
      if (cm93_chart) cm93_chart->SetVPParms(detail_vp);
    }
    SegmentSafetyChartCandidate candidate;
    candidate.db_index = *it;
    candidate.provider_priority = cm93 ? 1 : 0;
    candidate.native_scale = chart->GetNativeScale();
    candidate.plugin_vector = plugin_vector;
    candidate.cm93 = cm93;
    candidate.path = chart->GetFullPath().ToStdString();
    if (ChartData && *it >= 0 && *it < ChartData->GetChartTableEntries()) {
      const ChartTableEntry& entry = ChartData->GetChartTableEntry(*it);
      candidate.edition_date = entry.GetChartEditionDate();
      candidate.file_time = entry.GetFileTime();
    }
    candidates.push_back(candidate);
  }
  std::sort(candidates.begin(), candidates.end(),
            SegmentSafetyChartCandidateLess);
  return candidates;
}

bool SegmentSafetyCachedTileProviderIsCurrent(
    const CachedPointSafetyGridTile& tile) {
  if (!tile.built || tile.source == kSourcePluginVector) return true;

  // A chart-set identity change invalidates the complete persistent store,
  // but provider availability can change while the same chart database is in
  // use (for example, enabling o-charts after a CM93-only run).  Probe the
  // centre and corners of the cached tile.  If current chart selection prefers
  // a different provider class anywhere in the tile, rebuild it instead of
  // allowing stale lower-priority evidence to satisfy an authoritative query.
  const double inset = wxMax(tile.resolution * 0.5, 1e-6);
  const double min_lat = tile.min_lat + inset;
  const double min_lon = tile.min_lon + inset;
  const double max_lat = tile.min_lat + kGridTileDegrees - inset;
  const double max_lon = tile.min_lon + kGridTileDegrees - inset;
  const double samples[][2] = {
      {(min_lat + max_lat) / 2.0, (min_lon + max_lon) / 2.0},
      {min_lat, min_lon},
      {min_lat, max_lon},
      {max_lat, min_lon},
      {max_lat, max_lon},
  };

  for (size_t i = 0; i < WXSIZEOF(samples); ++i) {
    std::set<int> chart_indexes;
    SegmentSafetyCandidateChartsAt(samples[i][0], samples[i][1], chart_indexes,
                                   NULL);

    // The common no-o-chart case should remain a cheap cache hit.  Opening
    // charts is only necessary when the chart table contains a provider class
    // which could supersede the cached source at this sample.
    bool provider_may_differ = false;
    for (std::set<int>::const_iterator it = chart_indexes.begin();
         it != chart_indexes.end(); ++it) {
      if (!ChartData || *it < 0 || *it >= ChartData->GetChartTableEntries())
        continue;
      const ChartTypeEnum type =
          (ChartTypeEnum)ChartData->GetChartTableEntry(*it).GetChartType();
      if (tile.source == kSourceCm93) {
        provider_may_differ =
            type != CHART_TYPE_CM93 && type != CHART_TYPE_CM93COMP;
      } else if (tile.source == kSourceVectorChart) {
        provider_may_differ = type == CHART_TYPE_PLUGIN;
      } else {
        provider_may_differ = true;
      }
      if (provider_may_differ) break;
    }
    if (!provider_may_differ) continue;

    const std::vector<SegmentSafetyChartCandidate> candidates =
        SegmentSafetySortedChartCandidates(samples[i][0], samples[i][1],
                                           chart_indexes);
    if (candidates.empty()) continue;
    const SegmentSafetySource preferred_source =
        candidates.front().plugin_vector
            ? kSourcePluginVector
            : (candidates.front().cm93 ? kSourceCm93 : kSourceVectorChart);
    if (preferred_source == tile.source) continue;

    static long provider_rejection_logs = 0;
    if (provider_rejection_logs < 20) {
      ++provider_rejection_logs;
      wxLogMessage(
          "WR_CACHED_TILE_PROVIDER_REJECT #%ld tile=(%ld,%ld) "
          "cached_source=%d preferred_source=%d sample=(%.8f,%.8f) "
          "cached_chart_path=\"%s\" preferred_chart_path=\"%s\"",
          provider_rejection_logs, tile.lat_tile, tile.lon_tile,
          (int)tile.source, (int)preferred_source, samples[i][0], samples[i][1],
          tile.chart_path, candidates.front().path.c_str());
    }
    return false;
  }
  return true;
}

SegmentSafetyPointClass ChartPluginPointSafetyClassAtRaw(
    ChartPlugInWrapper* wrapper, const SegmentSafetyChartCandidate& candidate,
    double lat, double lon, const std::string& point_cache_key,
    SegmentSafetySource* source, SegmentSafetyCoreStats* stats,
    SegmentSafetyResult* result) {
  if (!wrapper || !g_pi_manager) return kPointNoData;

  const SegmentSafetySource chart_source = kSourcePluginVector;
  if (source) *source = chart_source;
  if (stats) ++stats->s57_chart_count;

  ViewPort vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
  // Query a complete fine-cell neighbourhood.  This ensures point and line
  // dangers such as WRECKS, UWTROC and OBSTRN cannot fall between grid
  // samples.  Route-mask dilation subsequently applies the user's margin.
  const float select_radius = static_cast<float>(kGridResolutionDegrees * 0.75);
  ListOfPI_S57Obj* objects = g_pi_manager->GetPlugInObjRuleListAtLatLon(
      wrapper, static_cast<float>(lat), static_cast<float>(lon), select_radius,
      vp);
  // The chart stack says this licensed chart covers the point. A null query
  // therefore means the provider/helper could not supply authoritative data;
  // it must not be converted into an empty-water answer or hidden by CM93.
  if (!objects) return kPointNoData;

  bool land = false;
  bool drying = false;
  bool has_depth = false;
  bool unknown_danger_depth = false;
  double min_depth_m = 0.0;
  wxString hit_object;
  wxString depth_object;
  wxString depth_attribute;

  if (objects) {
    for (ListOfPI_S57Obj::Node* node = objects->GetFirst(); node;
         node = node->GetNext()) {
      PI_S57Obj* object = node->GetData();
      if (!object) continue;
      const wxString summary = SegmentSafetyPluginObjectSummary(object);
      if (!strncmp(object->FeatureName, "LNDARE", 6) ||
          SegmentSafetyPluginObjectIsAlwaysDry(object)) {
        land = true;
        hit_object = summary;
        break;
      }
      if (SegmentSafetyPluginObjectIsDrying(object)) {
        drying = true;
        if (hit_object.empty()) hit_object = summary;
      }

      double object_depth_m = 0.0;
      bool object_unknown_danger_depth = false;
      wxString object_depth_attribute;
      if (SegmentSafetyPluginObjectDepthM(object, &object_depth_m,
                                          &object_depth_attribute,
                                          &object_unknown_danger_depth)) {
        if (!has_depth || object_depth_m < min_depth_m) {
          has_depth = true;
          min_depth_m = object_depth_m;
          depth_object = summary;
          depth_attribute = object_depth_attribute;
        }
      }
      if (object_unknown_danger_depth) {
        unknown_danger_depth = true;
        depth_object = summary;
        depth_attribute =
            wxString::Format("%s/VALSOU missing", object->FeatureName);
      }
    }
    objects->Clear();
    delete objects;
  }

  const wxString chart_path = wrapper->GetFullPath();
  if (land) {
    if (SegmentSafetyResultHas(result,
                               offsetof(SegmentSafetyResult, hit_object),
                               sizeof(result->hit_object))) {
      result->chart_db_index = candidate.db_index;
      result->chart_scale = wrapper->GetNativeScale();
      CopySegmentSafetyString(result->chart_path, sizeof(result->chart_path),
                              chart_path.mb_str());
      CopySegmentSafetyString(result->hit_object, sizeof(result->hit_object),
                              hit_object.mb_str());
    }
    StoreSegmentSafetyPointCache(
        point_cache_key, MakeSegmentSafetyPointCacheEntry(
                             kPointLand, chart_source, candidate.db_index,
                             wrapper->GetNativeScale(), chart_path.mb_str(),
                             hit_object.mb_str()));
    return kPointLand;
  }

  if (unknown_danger_depth) has_depth = false;
  const SegmentSafetyPointClass point_class =
      drying ? kPointDrying : kPointWater;
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    result->chart_db_index = candidate.db_index;
    result->chart_scale = wrapper->GetNativeScale();
    CopySegmentSafetyString(result->chart_path, sizeof(result->chart_path),
                            chart_path.mb_str());
    result->has_depth = has_depth ? 1 : 0;
    result->min_depth_m = has_depth ? min_depth_m : 0.0;
    result->has_drying = drying ? 1 : 0;
    CopySegmentSafetyString(result->hit_object, sizeof(result->hit_object),
                            hit_object.mb_str());
    CopySegmentSafetyString(result->depth_source_object,
                            sizeof(result->depth_source_object),
                            depth_object.mb_str());
    CopySegmentSafetyString(result->depth_source_attribute,
                            sizeof(result->depth_source_attribute),
                            depth_attribute.mb_str());
  }
  StoreSegmentSafetyPointCache(
      point_cache_key,
      MakeSegmentSafetyPointCacheEntry(
          point_class, chart_source, candidate.db_index,
          wrapper->GetNativeScale(), chart_path.mb_str(), hit_object.mb_str(),
          has_depth, min_depth_m, drying, depth_object.mb_str(),
          depth_attribute.empty() ? nullptr : depth_attribute.mb_str().data()));
  return point_class;
}

SegmentSafetyPointClass ChartPointSafetyClassAtRaw(
    double lat, double lon, SegmentSafetySource* source,
    SegmentSafetyCoreStats* stats, SegmentSafetyResult* result) {
  std::string point_cache_key = SegmentSafetyPointCacheKey(lat, lon);
  CachedPointSafetyClassification cached_point;
  if (LookupSegmentSafetyPointCache(point_cache_key, &cached_point)) {
    if (stats) ++stats->point_cache_hits;
    if (source) *source = cached_point.source;
    CopySegmentSafetyPointCacheToResult(cached_point, result);
    return cached_point.point_class;
  }
  if (stats) ++stats->point_cache_misses;

  if (!wxThread::IsMain()) {
    if (source) *source = kSourceNone;
    if (result) {
      SetSegmentSafetyStatus(result, kNoData);
      SetSegmentSafetySource(result, kSourceNone);
      SetSegmentSafetyMessage(
          result,
          "chart point classification is unavailable from worker thread");
    }
    return kPointNoData;
  }

  std::set<int> chart_indexes;
  SegmentSafetyCandidateChartsAt(lat, lon, chart_indexes, stats);

  std::vector<SegmentSafetyChartCandidate> candidates =
      SegmentSafetySortedChartCandidates(lat, lon, chart_indexes);

  bool chart_checked = false;
  bool licensed_plugin_depth_missing = false;
  for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
           candidates.begin();
       it != candidates.end(); ++it) {
    // Once the selected licensed chart has established ordinary water, later
    // candidates may supply only its missing numeric depth.  Native/CM93
    // charts use a different object path and must not replace the selected
    // licensed chart's semantic classification.
    if (licensed_plugin_depth_missing && !it->plugin_vector) break;
    ChartBase* chart =
        ChartData ? ChartData->OpenChartFromDB(it->db_index, FULL_INIT) : NULL;
    s57chart* s57 = dynamic_cast<s57chart*>(chart);
    ChartPlugInWrapper* plugin_wrapper =
        dynamic_cast<ChartPlugInWrapper*>(chart);
    if (!s57 && !(it->plugin_vector && plugin_wrapper)) continue;
    chart_checked = true;
    if (it->plugin_vector && plugin_wrapper) {
      SegmentSafetyResult supplemental_result = {};
      supplemental_result.struct_size = sizeof(supplemental_result);
      InitSegmentSafetyResult(&supplemental_result);
      SegmentSafetyResult* query_result =
          licensed_plugin_depth_missing ? &supplemental_result : result;
      const std::string query_cache_key =
          licensed_plugin_depth_missing
              ? point_cache_key + ":depth:" + std::to_string(it->db_index)
              : point_cache_key;
      const SegmentSafetyPointClass plugin_class =
          ChartPluginPointSafetyClassAtRaw(plugin_wrapper, *it, lat, lon,
                                           query_cache_key, source, stats,
                                           query_result);
      if (licensed_plugin_depth_missing) {
        // This chart is consulted for depth only.  Land, drying and unknown
        // danger semantics from a lower-priority chart cannot override the
        // selected detailed chart; try the next licensed candidate instead.
        if (plugin_class != kPointWater || !supplemental_result.has_depth)
          continue;
        if (SegmentSafetyResultHas(
                result, offsetof(SegmentSafetyResult, depth_source_attribute),
                sizeof(result->depth_source_attribute))) {
          result->chart_db_index = supplemental_result.chart_db_index;
          result->chart_scale = supplemental_result.chart_scale;
          result->has_depth = 1;
          result->min_depth_m = supplemental_result.min_depth_m;
          CopySegmentSafetyString(result->chart_path,
                                  sizeof(result->chart_path),
                                  supplemental_result.chart_path);
          CopySegmentSafetyString(result->depth_source_object,
                                  sizeof(result->depth_source_object),
                                  supplemental_result.depth_source_object);
          CopySegmentSafetyString(result->depth_source_attribute,
                                  sizeof(result->depth_source_attribute),
                                  supplemental_result.depth_source_attribute);
        }
        StoreSegmentSafetyPointCache(
            point_cache_key, MakeSegmentSafetyPointCacheEntry(
                                 kPointWater, kSourcePluginVector, it->db_index,
                                 plugin_wrapper->GetNativeScale(),
                                 plugin_wrapper->GetFullPath().mb_str(), "",
                                 true, supplemental_result.min_depth_m, false,
                                 supplemental_result.depth_source_object,
                                 supplemental_result.depth_source_attribute));
        return kPointWater;
      }
      const bool plain_missing_depth =
          plugin_class == kPointWater && result && !result->has_depth &&
          result->depth_source_attribute[0] == '\0';
      if (!plain_missing_depth) return plugin_class;
      // Retain the detailed chart's water classification while asking only
      // later licensed/native vector charts for a numeric depth.  This is the
      // legacy point-query equivalent of the batch depth supplementation.
      licensed_plugin_depth_missing = true;
      continue;
    }
    bool cm93 = IsCm93Chart(chart);
    SegmentSafetySource chart_source = cm93 ? kSourceCm93 : kSourceVectorChart;
    if (source) *source = chart_source;
    if (stats) ++stats->s57_chart_count;

    ViewPort vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
    if (cm93) {
      cm93compchart* cm93_chart = dynamic_cast<cm93compchart*>(chart);
      if (cm93_chart) cm93_chart->SetVPParms(vp);
    }

    const float select_radius =
        static_cast<float>(kGridResolutionDegrees * 0.75);
    ListOfObjRazRules* rule_list =
        s57->GetObjRuleListAtLatLon(lat, lon, select_radius, &vp, MASK_ALL);
    if (!rule_list) continue;

    bool drying = false;
    bool has_depth = false;
    bool unknown_danger_depth = false;
    double min_depth_m = 0.0;
    wxString depth_object;
    wxString depth_attribute;
    for (ListOfObjRazRules::Node* node = rule_list->GetFirst(); node;
         node = node->GetNext()) {
      ObjRazRules* rule = node->GetData();
      if (!rule || !rule->obj) continue;
      if (!strncmp(rule->obj->FeatureName, "LNDARE", 6) ||
          SegmentSafetyRuleIsAlwaysDry(rule)) {
        wxString chart_path = chart->GetFullPath();
        wxString object = SegmentSafetyRuleSummary(rule);
        if (SegmentSafetyResultHas(result,
                                   offsetof(SegmentSafetyResult, hit_object),
                                   sizeof(result->hit_object))) {
          result->chart_db_index = it->db_index;
          result->chart_scale = chart->GetNativeScale();
          strncpy(result->chart_path, chart_path.mb_str(),
                  sizeof(result->chart_path) - 1);
          result->chart_path[sizeof(result->chart_path) - 1] = '\0';
          strncpy(result->hit_object, object.mb_str(),
                  sizeof(result->hit_object) - 1);
          result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        }
        StoreSegmentSafetyPointCache(
            point_cache_key,
            MakeSegmentSafetyPointCacheEntry(
                kPointLand, chart_source, it->db_index, chart->GetNativeScale(),
                chart_path.mb_str(), object.mb_str()));
        rule_list->Clear();
        delete rule_list;
        return kPointLand;
      }
      if (SegmentSafetyRuleIsDrying(rule)) drying = true;
      double rule_depth = 0.0;
      if (SegmentSafetyRuleDepthMinM(rule, &rule_depth)) {
        if (!has_depth || rule_depth < min_depth_m) {
          has_depth = true;
          min_depth_m = rule_depth;
          depth_object = SegmentSafetyRuleSummary(rule);
          depth_attribute =
              wxString::Format("%s/DRVAL1", rule->obj->FeatureName);
        }
      }
      bool danger_unknown = false;
      if (SegmentSafetyRuleDangerDepthM(rule, &rule_depth, &danger_unknown)) {
        if (!has_depth || rule_depth < min_depth_m) {
          has_depth = true;
          min_depth_m = rule_depth;
          depth_object = SegmentSafetyRuleSummary(rule);
          depth_attribute =
              wxString::Format("%s/VALSOU", rule->obj->FeatureName);
        }
      }
      if (danger_unknown) {
        unknown_danger_depth = true;
        depth_object = SegmentSafetyRuleSummary(rule);
        depth_attribute =
            wxString::Format("%s/VALSOU missing", rule->obj->FeatureName);
      }
    }

    rule_list->Clear();
    delete rule_list;
    if (unknown_danger_depth) has_depth = false;
    SegmentSafetyPointClass point_class = drying ? kPointDrying : kPointWater;
    wxString chart_path = chart->GetFullPath();
    if (SegmentSafetyResultHas(
            result, offsetof(SegmentSafetyResult, depth_source_attribute),
            sizeof(result->depth_source_attribute))) {
      result->has_depth = has_depth ? 1 : 0;
      result->min_depth_m = has_depth ? min_depth_m : 0.0;
      result->has_drying = drying ? 1 : 0;
      if (has_depth) {
        strncpy(result->depth_source_object, depth_object.mb_str(),
                sizeof(result->depth_source_object) - 1);
        result->depth_source_object[sizeof(result->depth_source_object) - 1] =
            '\0';
        CopySegmentSafetyString(result->depth_source_attribute,
                                sizeof(result->depth_source_attribute),
                                depth_attribute.mb_str());
      } else if (unknown_danger_depth) {
        CopySegmentSafetyString(result->depth_source_object,
                                sizeof(result->depth_source_object),
                                depth_object.mb_str());
        CopySegmentSafetyString(result->depth_source_attribute,
                                sizeof(result->depth_source_attribute),
                                depth_attribute.mb_str());
      }
    }
    StoreSegmentSafetyPointCache(
        point_cache_key,
        MakeSegmentSafetyPointCacheEntry(
            point_class, chart_source, it->db_index, chart->GetNativeScale(),
            chart_path.mb_str(), "", has_depth, min_depth_m, drying,
            depth_object.mb_str(),
            depth_attribute.empty() ? nullptr
                                    : depth_attribute.mb_str().data()));
    if (point_class == kPointWater && !has_depth && !unknown_danger_depth)
      continue;
    return point_class;
  }

  SegmentSafetyPointClass point_class =
      chart_checked ? kPointWater : kPointNoData;
  StoreSegmentSafetyPointCache(
      point_cache_key,
      MakeSegmentSafetyPointCacheEntry(
          point_class, chart_checked ? kSourceVectorChart : kSourceNone, -1, -1,
          "", ""));
  return point_class;
}

SegmentSafetyPointClass ChartPointSafetyClassAtPreparedCm93(
    cm93chart* chart, int chart_db_index, double lat, double lon,
    ViewPort* viewport, SegmentSafetySource* source,
    SegmentSafetyCoreStats* stats, SegmentSafetyResult* result) {
  if (!chart || !viewport) return kPointNoData;

  const std::string point_cache_key = SegmentSafetyPointCacheKey(lat, lon);
  CachedPointSafetyClassification cached_point;
  if (LookupSegmentSafetyPointCache(point_cache_key, &cached_point)) {
    if (stats) ++stats->point_cache_hits;
    if (source) *source = cached_point.source;
    CopySegmentSafetyPointCacheToResult(cached_point, result);
    return cached_point.point_class;
  }
  if (stats) {
    ++stats->point_cache_misses;
    ++stats->s57_chart_count;
  }

  const SegmentSafetySource chart_source = kSourceCm93;
  if (source) *source = chart_source;

  ListOfObjRazRules* rule_list =
      chart->GetObjRuleListAtLatLon(lat, lon, 0.0, viewport, MASK_AREA);
  bool drying = false;
  bool has_depth = false;
  double min_depth_m = 0.0;
  wxString depth_object;
  if (rule_list) {
    for (ListOfObjRazRules::Node* node = rule_list->GetFirst(); node;
         node = node->GetNext()) {
      ObjRazRules* rule = node->GetData();
      if (!rule || !rule->obj) continue;
      if (!strncmp(rule->obj->FeatureName, "LNDARE", 6)) {
        const wxString chart_path = chart->GetFullPath();
        const wxString object = SegmentSafetyRuleSummary(rule);
        if (SegmentSafetyResultHas(result,
                                   offsetof(SegmentSafetyResult, hit_object),
                                   sizeof(result->hit_object))) {
          result->chart_db_index = chart_db_index;
          result->chart_scale = chart->GetNativeScale();
          strncpy(result->chart_path, chart_path.mb_str(),
                  sizeof(result->chart_path) - 1);
          result->chart_path[sizeof(result->chart_path) - 1] = '\0';
          strncpy(result->hit_object, object.mb_str(),
                  sizeof(result->hit_object) - 1);
          result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        }
        StoreSegmentSafetyPointCache(
            point_cache_key,
            MakeSegmentSafetyPointCacheEntry(
                kPointLand, chart_source, chart_db_index,
                chart->GetNativeScale(), chart_path.mb_str(), object.mb_str()));
        rule_list->Clear();
        delete rule_list;
        return kPointLand;
      }
      if (SegmentSafetyRuleIsDrying(rule)) drying = true;
      double rule_depth = 0.0;
      if (SegmentSafetyRuleDepthMinM(rule, &rule_depth) &&
          (!has_depth || rule_depth < min_depth_m)) {
        has_depth = true;
        min_depth_m = rule_depth;
        depth_object = SegmentSafetyRuleSummary(rule);
      }
    }
    rule_list->Clear();
    delete rule_list;
  }

  const SegmentSafetyPointClass point_class =
      drying ? kPointDrying : kPointWater;
  const wxString chart_path = chart->GetFullPath();
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    result->has_depth = has_depth ? 1 : 0;
    result->min_depth_m = has_depth ? min_depth_m : 0.0;
    result->has_drying = drying ? 1 : 0;
    if (has_depth) {
      strncpy(result->depth_source_object, depth_object.mb_str(),
              sizeof(result->depth_source_object) - 1);
      result->depth_source_object[sizeof(result->depth_source_object) - 1] =
          '\0';
      strncpy(result->depth_source_attribute, "DEPARE/DRVAL1",
              sizeof(result->depth_source_attribute) - 1);
      result
          ->depth_source_attribute[sizeof(result->depth_source_attribute) - 1] =
          '\0';
    }
  }
  StoreSegmentSafetyPointCache(
      point_cache_key,
      MakeSegmentSafetyPointCacheEntry(
          point_class, chart_source, chart_db_index, chart->GetNativeScale(),
          chart_path.mb_str(), "", has_depth, min_depth_m, drying,
          depth_object.mb_str(), has_depth ? "DEPARE/DRVAL1" : NULL));
  return point_class;
}

ocpn::chart_safety::DepthProbeClass SegmentSafetyDepthProbeClass(
    SegmentSafetyPointClass point_class) {
  using ocpn::chart_safety::DepthProbeClass;
  switch (point_class) {
    case kPointWater:
      return DepthProbeClass::kWater;
    case kPointLand:
      return DepthProbeClass::kLand;
    case kPointDrying:
      return DepthProbeClass::kDrying;
    case kPointNoData:
    default:
      return DepthProbeClass::kNoData;
  }
}

bool RecoverPreparedCm93BoundaryDepth(double lat, double lon, double resolution,
                                      SegmentSafetyCoreStats* stats,
                                      SegmentSafetyResult* result) {
  if (!result) return false;

  using ocpn::chart_safety::DepthProbe;
  std::array<DepthProbe, 4> probes;
  std::array<SegmentSafetyResult, 4> probe_results = {};
  const double offset = resolution * 0.25;
  const double lat_signs[4] = {-1.0, -1.0, 1.0, 1.0};
  const double lon_signs[4] = {-1.0, 1.0, -1.0, 1.0};
  int minimum_probe = -1;
  double minimum_depth = 0.0;

  for (size_t i = 0; i < probes.size(); ++i) {
    const double probe_lat = lat + lat_signs[i] * offset;
    const double probe_lon = lon + lon_signs[i] * offset;
    probe_results[i].struct_size = sizeof(probe_results[i]);
    InitSegmentSafetyResult(&probe_results[i]);
    SegmentSafetySource source = kSourceNone;
    const SegmentSafetyPointClass point_class = ChartPointSafetyClassAtRaw(
        probe_lat, probe_lon, &source, stats, &probe_results[i]);
    probes[i] = {SegmentSafetyDepthProbeClass(point_class),
                 probe_results[i].has_depth != 0, probe_results[i].min_depth_m};
    if (probe_results[i].has_depth &&
        (minimum_probe < 0 || probe_results[i].min_depth_m < minimum_depth)) {
      minimum_probe = static_cast<int>(i);
      minimum_depth = probe_results[i].min_depth_m;
    }
  }

  const std::optional<double> recovered =
      ocpn::chart_safety::ConservativeBoundaryDepth(probes);
  if (!recovered || minimum_probe < 0) return false;

  result->has_depth = 1;
  result->min_depth_m = *recovered;
  result->has_drying = 0;
  CopySegmentSafetyString(result->depth_source_object,
                          sizeof(result->depth_source_object),
                          probe_results[minimum_probe].depth_source_object);
  CopySegmentSafetyString(result->depth_source_attribute,
                          sizeof(result->depth_source_attribute),
                          "DEPARE/DRVAL1 boundary-min");
  return true;
}

void SegmentSafetyPluginBatchVisit(void* context,
                                   const ChartSafetyFeature* feature,
                                   const uint64_t* hit_cells,
                                   uint32_t hit_word_count) {
  SegmentSafetyPluginBatchGroup* group =
      static_cast<SegmentSafetyPluginBatchGroup*>(context);
  if (!group || !feature || !hit_cells ||
      feature->struct_size < sizeof(ChartSafetyFeature))
    return;

  const bool land = feature->flags & HostApi123::kChartSafetyFeatureLand;
  const bool drying = feature->flags & HostApi123::kChartSafetyFeatureDrying;
  const bool has_depth =
      feature->flags & HostApi123::kChartSafetyFeatureHasDepth;
  const bool unknown_danger_depth =
      feature->flags & HostApi123::kChartSafetyFeatureUnknownDangerDepth;
  const double depth_m = feature->minimum_depth_m;
  if (!land && !drying && !has_depth && !unknown_danger_depth) return;

  const size_t cells = group->active.size();
  for (uint32_t word = 0; word < hit_word_count; ++word) {
    uint64_t bits = hit_cells[word];
    while (bits) {
#if defined(__GNUC__) || defined(__clang__)
      const unsigned bit = static_cast<unsigned>(__builtin_ctzll(bits));
#else
      unsigned bit = 0;
      while (((bits >> bit) & 1u) == 0) ++bit;
#endif
      const size_t index = static_cast<size_t>(word) * 64 + bit;
      if (index < cells && group->active[index]) {
        if (land) group->land[index] = 1;
        if (drying) group->drying[index] = 1;
        if (has_depth &&
            (!group->has_depth[index] || depth_m < group->min_depth_m[index])) {
          group->has_depth[index] = 1;
          group->min_depth_m[index] = static_cast<float>(depth_m);
        }
        if (unknown_danger_depth) group->unknown_danger_depth[index] = 1;
      }
      bits &= bits - 1;
    }
  }
}

std::set<std::pair<long, long> > PrebuildSegmentSafetyPluginVectorGridTiles(
    const std::set<std::pair<long, long> >& requested_tiles,
    SegmentSafetyCoreStats* stats, bool require_depth) {
  std::set<std::pair<long, long> > built_tiles;
  if (!wxThread::IsMain() || requested_tiles.empty() || !ChartData ||
      !g_pi_manager || !g_pi_manager->HasPlugInChartSafetyGrid())
    return built_tiles;

  std::set<std::pair<long, long> > missing_tiles;
  for (const auto& tile : requested_tiles) {
    const std::string key =
        SegmentSafetyGridTileKeyForIndices(tile.first, tile.second);
    CachedPointSafetyGridTile cached;
    if (LookupSegmentSafetyGridTile(key, &cached) &&
        (!require_depth || cached.depth_complete))
      continue;

    // Consult the weather-routing-owned persistent store before planning
    // provider rectangles.  Without this, a restart found all host RAM tiles
    // empty and needlessly re-extracted every licensed chart even though the
    // exact final-identity semantic tiles were already on disk.
    CachedPointSafetyGridTile external;
    if (SegmentSafetyExternalTileCacheLookup(tile.first, tile.second,
                                             require_depth, &external) &&
        SegmentSafetyCachedTileProviderIsCurrent(external)) {
      StoreSegmentSafetyGridTile(key, external);
      continue;
    }
    missing_tiles.insert(tile);
  }

  constexpr int kTileCells = 40;
  constexpr int kMaximumTileSpan = 6;
  const auto blocks =
      ocpn::chart_safety::PlanTileBatchBlocks(missing_tiles, kMaximumTileSpan);
  for (const auto& block : blocks) {
    const int tile_rows =
        static_cast<int>(block.max_lat_tile - block.min_lat_tile + 1);
    const int tile_cols =
        static_cast<int>(block.max_lon_tile - block.min_lon_tile + 1);
    const int rows = tile_rows * kTileCells + 1;
    const int cols = tile_cols * kTileCells + 1;
    const size_t cell_count = static_cast<size_t>(rows) * cols;
    if (cell_count == 0 || cell_count > 65536) continue;

    wxStopWatch timer;
    const double min_lat = ocpn::chart_safety::GlobalGridCoordinate(
        block.min_lat_tile, 0, kTileCells, kGridResolutionDegrees);
    const double min_lon = ocpn::chart_safety::GlobalGridCoordinate(
        block.min_lon_tile, 0, kTileCells, kGridResolutionDegrees);
    const double max_lat = ocpn::chart_safety::GlobalGridCoordinate(
        block.min_lat_tile, rows - 1, kTileCells, kGridResolutionDegrees);
    const double max_lon = ocpn::chart_safety::GlobalGridCoordinate(
        block.min_lon_tile, cols - 1, kTileCells, kGridResolutionDegrees);
    const std::vector<SegmentSafetyChartCandidate> candidates =
        SegmentSafetyTileChartCandidates(min_lat, min_lon, max_lat, max_lon);

    std::vector<uint8_t> classified(cell_count, 0);
    std::vector<uint8_t> classes(cell_count,
                                 static_cast<uint8_t>(kPointNoData));
    std::vector<uint16_t> hazards(cell_count, kHazardNoChart);
    std::vector<uint8_t> has_depth(cell_count, 0);
    std::vector<float> min_depth_m(cell_count, 0.0f);
    std::vector<uint8_t> has_drying(cell_count, 0);
    std::vector<uint8_t> unknown_danger_depth(cell_count, 0);
    std::vector<uint8_t> persistent_cache_allowed(cell_count, 0);
    std::vector<int> selected_db_index(cell_count, -1);
    std::vector<std::vector<int> > depth_candidates(cell_count);
    std::map<int, SegmentSafetyPluginBatchGroup> groups;

    for (int row = 0; row < rows; ++row) {
      const double lat = ocpn::chart_safety::GlobalGridCoordinate(
          block.min_lat_tile, row, kTileCells, kGridResolutionDegrees);
      for (int col = 0; col < cols; ++col) {
        const double lon = ocpn::chart_safety::GlobalGridCoordinate(
            block.min_lon_tile, col, kTileCells, kGridResolutionDegrees);
        const size_t index = static_cast<size_t>(row) * cols + col;
        bool selected_plugin = false;
        for (const auto& candidate : candidates) {
          if (!ChartData->ChartCoversPosition(candidate.db_index,
                                              static_cast<float>(lat),
                                              static_cast<float>(lon)))
            continue;
          if (stats) ++stats->chart_stack_entries;
          if (candidate.plugin_vector) {
            depth_candidates[index].push_back(candidate.db_index);
            if (!selected_plugin) {
              auto found = groups.find(candidate.db_index);
              if (found == groups.end()) {
                found = groups
                            .insert(std::make_pair(
                                candidate.db_index,
                                SegmentSafetyPluginBatchGroup(cell_count)))
                            .first;
                found->second.db_index = candidate.db_index;
              }
              found->second.active[index] = 1;
              ++found->second.active_count;
              selected_db_index[index] = candidate.db_index;
              selected_plugin = true;
            }
            continue;
          }
          if (!candidate.cm93) break;
        }
      }
    }

    std::map<int, ChartPlugInWrapper*> wrappers;
    for (auto& item : groups) {
      ChartBase* chart = ChartData->OpenChartFromDB(item.first, FULL_INIT);
      if (!IsSupportedSegmentSafetyPluginChart(chart)) continue;
      item.second.wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
      wrappers[item.first] = item.second.wrapper;
    }

    const double centre_lat = (min_lat + max_lat) / 2.0;
    const double centre_lon = (min_lon + max_lon) / 2.0;
    ViewPort vp = SegmentSafetyHighestDetailViewPortAt(centre_lat, centre_lon);
    const double cos_lat =
        wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(centre_lat))));
    const double width_m = (max_lon - min_lon) * 60.0 * 1852.0 * cos_lat;
    const double height_m = (max_lat - min_lat) * 60.0 * 1852.0;
    vp.pix_width =
        wxMax(vp.pix_width,
              static_cast<int>(ceil(width_m * vp.view_scale_ppm)) + 512);
    vp.pix_height =
        wxMax(vp.pix_height,
              static_cast<int>(ceil(height_m * vp.view_scale_ppm)) + 512);
    vp.SetBoxes();

    int provider_calls = 0;
    int provider_failures = 0;
    int candidate_objects = 0;
    int hit_objects = 0;
    for (auto& item : groups) {
      SegmentSafetyPluginBatchGroup& group = item.second;
      if (!group.wrapper || group.active_count <= 0) continue;
      HostApi123::ChartSafetyProviderRequest request = {};
      request.struct_size = sizeof(request);
      request.abi_version = HostApi123::kChartSafetyProviderAbiVersion;
      request.min_lat = min_lat;
      request.min_lon = min_lon;
      request.lat_step = kGridResolutionDegrees;
      request.lon_step = kGridResolutionDegrees;
      request.rows = rows;
      request.cols = cols;
      request.select_radius_degrees =
          static_cast<float>(kGridResolutionDegrees * 0.75);
      request.active_cells = group.active.data();
      request.visitor_context = &group;
      request.visit_object = SegmentSafetyPluginBatchVisit;
      HostApi123::ChartSafetyProviderResult provider_result = {};
      provider_result.struct_size = sizeof(provider_result);
      ++provider_calls;
      const HostApi123::ChartSafetyProviderStatus status =
          g_pi_manager->QueryPlugInChartSafetyGrid(group.wrapper, request,
                                                   &provider_result, vp);
      if (status != HostApi123::kChartSafetyProviderComplete ||
          provider_result.abi_version !=
              HostApi123::kChartSafetyProviderAbiVersion ||
          provider_result.processed_cells !=
              static_cast<uint32_t>(group.active_count)) {
        ++provider_failures;
        continue;
      }
      candidate_objects += provider_result.candidate_objects;
      hit_objects += provider_result.hit_objects;
      if (stats) ++stats->s57_chart_count;
      const bool provider_cache_allowed =
          (provider_result.result_flags &
           HostApi123::kChartSafetyDerivedCacheAllowed) != 0;
      for (size_t index = 0; index < cell_count; ++index) {
        if (!group.active[index]) continue;
        classified[index] = 1;
        persistent_cache_allowed[index] = provider_cache_allowed ? 1 : 0;
        SegmentSafetyPointClass point_class = kPointWater;
        if (group.land[index])
          point_class = kPointLand;
        else if (group.drying[index])
          point_class = kPointDrying;
        classes[index] = static_cast<uint8_t>(point_class);
        hazards[index] = SegmentSafetyPointHazardFlags(point_class);
        const bool depth_known =
            group.has_depth[index] && !group.unknown_danger_depth[index];
        has_depth[index] = depth_known ? 1 : 0;
        min_depth_m[index] = depth_known ? group.min_depth_m[index] : 0.0f;
        has_drying[index] = group.drying[index] ? 1 : 0;
        unknown_danger_depth[index] = group.unknown_danger_depth[index] ? 1 : 0;
      }
    }

    size_t maximum_depth_candidate = 0;
    for (const auto& chain : depth_candidates)
      maximum_depth_candidate = std::max(maximum_depth_candidate, chain.size());
    for (size_t rank = 1; rank < maximum_depth_candidate; ++rank) {
      std::map<int, SegmentSafetyPluginBatchGroup> fallback_groups;
      for (size_t index = 0; index < cell_count; ++index) {
        if (!classified[index] || has_depth[index] ||
            unknown_danger_depth[index] || classes[index] != kPointWater ||
            depth_candidates[index].size() <= rank)
          continue;
        const int db_index = depth_candidates[index][rank];
        auto found = fallback_groups.find(db_index);
        if (found == fallback_groups.end()) {
          found = fallback_groups
                      .insert(std::make_pair(
                          db_index, SegmentSafetyPluginBatchGroup(cell_count)))
                      .first;
          found->second.db_index = db_index;
        }
        found->second.active[index] = 1;
        ++found->second.active_count;
      }
      for (auto& item : fallback_groups) {
        ChartBase* chart = ChartData->OpenChartFromDB(item.first, FULL_INIT);
        if (!IsSupportedSegmentSafetyPluginChart(chart)) continue;
        item.second.wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
        wrappers[item.first] = item.second.wrapper;
      }
      for (auto& item : fallback_groups) {
        SegmentSafetyPluginBatchGroup& group = item.second;
        if (!group.wrapper || group.active_count <= 0) continue;
        HostApi123::ChartSafetyProviderRequest request = {};
        request.struct_size = sizeof(request);
        request.abi_version = HostApi123::kChartSafetyProviderAbiVersion;
        request.min_lat = min_lat;
        request.min_lon = min_lon;
        request.lat_step = kGridResolutionDegrees;
        request.lon_step = kGridResolutionDegrees;
        request.rows = rows;
        request.cols = cols;
        request.select_radius_degrees =
            static_cast<float>(kGridResolutionDegrees * 0.75);
        request.active_cells = group.active.data();
        request.visitor_context = &group;
        request.visit_object = SegmentSafetyPluginBatchVisit;
        HostApi123::ChartSafetyProviderResult provider_result = {};
        provider_result.struct_size = sizeof(provider_result);
        ++provider_calls;
        const HostApi123::ChartSafetyProviderStatus status =
            g_pi_manager->QueryPlugInChartSafetyGrid(group.wrapper, request,
                                                     &provider_result, vp);
        if (status != HostApi123::kChartSafetyProviderComplete ||
            provider_result.abi_version !=
                HostApi123::kChartSafetyProviderAbiVersion ||
            provider_result.processed_cells !=
                static_cast<uint32_t>(group.active_count)) {
          ++provider_failures;
          continue;
        }
        candidate_objects += provider_result.candidate_objects;
        hit_objects += provider_result.hit_objects;
        const bool fallback_cache_allowed =
            (provider_result.result_flags &
             HostApi123::kChartSafetyDerivedCacheAllowed) != 0;
        for (size_t index = 0; index < cell_count; ++index) {
          if (group.active[index] && !fallback_cache_allowed)
            persistent_cache_allowed[index] = 0;
          if (!group.active[index] || group.land[index] ||
              group.drying[index] || group.unknown_danger_depth[index])
            continue;
          if (group.has_depth[index]) {
            has_depth[index] = 1;
            min_depth_m[index] = group.min_depth_m[index];
          }
        }
      }
    }

    int stored_in_block = 0;
    for (const auto& requested : block.tiles) {
      const int row_offset =
          static_cast<int>((requested.first - block.min_lat_tile) * kTileCells);
      const int col_offset = static_cast<int>(
          (requested.second - block.min_lon_tile) * kTileCells);
      bool complete = true;
      for (int row = 0; row <= kTileCells && complete; ++row)
        for (int col = 0; col <= kTileCells; ++col) {
          const size_t source =
              static_cast<size_t>(row_offset + row) * cols + col_offset + col;
          if (!classified[source]) {
            complete = false;
            break;
          }
        }
      if (!complete) continue;

      CachedPointSafetyGridTile tile;
      tile.group_index = SegmentSafetyCurrentGroupIndex();
      tile.lat_tile = requested.first;
      tile.lon_tile = requested.second;
      tile.min_lat = requested.first * kGridTileDegrees;
      tile.min_lon = requested.second * kGridTileDegrees;
      tile.resolution = kGridResolutionDegrees;
      tile.rows = tile.cols = kTileCells + 1;
      const size_t tile_cell_count = static_cast<size_t>(tile.rows) * tile.cols;
      tile.classes.resize(tile_cell_count);
      tile.hazard_flags.resize(tile_cell_count);
      tile.has_depth.resize(tile_cell_count);
      tile.min_depth_m.resize(tile_cell_count);
      tile.has_drying.resize(tile_cell_count);
      tile.hazard_summary_flags = kHazardNone;
      tile.land_count = tile.water_count = tile.drying_count =
          tile.unknown_count = 0;
      int representative_db_index = -1;
      for (int row = 0; row < tile.rows; ++row) {
        for (int col = 0; col < tile.cols; ++col) {
          const size_t source =
              static_cast<size_t>(row_offset + row) * cols + col_offset + col;
          const size_t target = static_cast<size_t>(row) * tile.cols + col;
          tile.classes[target] = classes[source];
          tile.hazard_flags[target] = hazards[source];
          tile.has_depth[target] = has_depth[source];
          tile.min_depth_m[target] = min_depth_m[source];
          tile.has_drying[target] = has_drying[source];
          tile.hazard_summary_flags |= hazards[source];
          if (representative_db_index < 0)
            representative_db_index = selected_db_index[source];
          switch (static_cast<SegmentSafetyPointClass>(classes[source])) {
            case kPointLand:
              ++tile.land_count;
              break;
            case kPointDrying:
              ++tile.drying_count;
              break;
            case kPointWater:
              ++tile.water_count;
              break;
            default:
              ++tile.unknown_count;
              break;
          }
        }
      }
      tile.source = kSourcePluginVector;
      tile.chart_db_index = representative_db_index;
      const auto wrapper = wrappers.find(representative_db_index);
      if (wrapper != wrappers.end() && wrapper->second) {
        tile.chart_scale = wrapper->second->GetNativeScale();
        CopySegmentSafetyString(tile.chart_path, sizeof(tile.chart_path),
                                wrapper->second->GetFullPath().mb_str());
      }
      tile.depth_complete = true;
      tile.persistent_cache_allowed = true;
      for (int row = 0; row < tile.rows && tile.persistent_cache_allowed; ++row)
        for (int col = 0; col < tile.cols; ++col) {
          const size_t source =
              static_cast<size_t>(row_offset + row) * cols + col_offset + col;
          if (!persistent_cache_allowed[source]) {
            tile.persistent_cache_allowed = false;
            break;
          }
        }
      tile.built = true;
      StoreSegmentSafetyGridTile(
          SegmentSafetyGridTileKeyForIndices(requested.first, requested.second),
          tile);
      built_tiles.insert(requested);
      ++stored_in_block;
      if (stats) {
        stats->grid_cells_total += tile.rows * tile.cols;
        stats->grid_cells_land += tile.land_count;
        stats->grid_cells_water += tile.water_count;
        stats->grid_cells_drying += tile.drying_count;
        stats->grid_cells_unknown += tile.unknown_count;
      }
    }
    const long elapsed_ms = timer.Time();
    if (stats) stats->grid_build_ms += elapsed_ms;
    wxLogMessage(
        "SEGMENT_SAFETY_PLUGIN_CORRIDOR_BATCH "
        "tile_bbox=[%ld..%ld,%ld..%ld] requested_tiles=%lu "
        "stored_tiles=%d cells=%lu provider_calls=%d failures=%d "
        "candidate_objects=%d hit_objects=%d require_depth=%d "
        "elapsed_ms=%ld",
        block.min_lat_tile, block.max_lat_tile, block.min_lon_tile,
        block.max_lon_tile, static_cast<unsigned long>(block.tiles.size()),
        stored_in_block, static_cast<unsigned long>(cell_count), provider_calls,
        provider_failures, candidate_objects, hit_objects,
        require_depth ? 1 : 0, elapsed_ms);
  }
  return built_tiles;
}

CachedPointSafetyGridTile BuildSegmentSafetyGridTile(
    double lat, double lon, long lat_tile, long lon_tile,
    SegmentSafetyCoreStats* stats, bool require_depth) {
  if (!wxThread::IsMain()) {
    wxLogMessage(
        "WR_GRID_THREAD_VIOLATION BuildSegmentSafetyGridTile called from "
        "worker thread=%p main_thread=%p tile=(%ld,%ld). Refusing chart "
        "object access.",
        wxThread::GetCurrentId(), wxThread::GetMainId(), lat_tile, lon_tile);
    if (stats) stats->chart_load_failed = true;
    return CachedPointSafetyGridTile();
  }

  if (!ChartData || !ChartData->IsValid()) {
    if (stats) stats->no_chart_database = true;
    return CachedPointSafetyGridTile();
  }

  wxStopWatch timer;
  CachedPointSafetyGridTile tile;
  constexpr int kTileCells = 40;
  tile.group_index = SegmentSafetyCurrentGroupIndex();
  tile.lat_tile = lat_tile;
  tile.lon_tile = lon_tile;
  tile.min_lat = ocpn::chart_safety::GlobalGridCoordinate(
      lat_tile, 0, kTileCells, kGridResolutionDegrees);
  tile.min_lon = ocpn::chart_safety::GlobalGridCoordinate(
      lon_tile, 0, kTileCells, kGridResolutionDegrees);
  tile.resolution = kGridResolutionDegrees;
  tile.rows = kTileCells + 1;
  tile.cols = tile.rows;
  tile.built = true;
  tile.classes.assign(tile.rows * tile.cols, (unsigned char)kPointNoData);
  tile.hazard_flags.assign(tile.rows * tile.cols, kHazardNoChart);
  tile.has_depth.assign(tile.rows * tile.cols, 0);
  tile.min_depth_m.assign(tile.rows * tile.cols, 0.0f);
  tile.has_drying.assign(tile.rows * tile.cols, 0);
  tile.hazard_summary_flags = kHazardNone;

  // Optional plugin-vector batch path.  Chart selection remains in OpenCPN:
  // every cell is assigned to the same highest-detail/newest usable chart the
  // point path would choose.  Each upgraded provider then walks its objects
  // once and reports cell membership through the versioned C ABI.  If the
  // symbol is absent, incomplete, or errors, those cells retain the exact
  // point-query fallback below.
  const size_t grid_cell_count = static_cast<size_t>(tile.rows) * tile.cols;
  std::vector<uint8_t> plugin_batch_classified(grid_cell_count, 0);
  std::vector<uint8_t> plugin_batch_unknown_danger(grid_cell_count, 0);
  std::vector<uint8_t> plugin_batch_persistent_cache_allowed(grid_cell_count,
                                                             0);
  std::vector<std::vector<int> > plugin_depth_candidates(grid_cell_count);
  int plugin_batch_groups = 0;
  int plugin_batch_cells = 0;
  int plugin_batch_fallback_cells = 0;
  int plugin_batch_candidate_objects = 0;
  int plugin_batch_hit_objects = 0;
  bool plugin_point_fallback_without_cache_permission = false;
  long plugin_batch_select_ms = 0;
  long plugin_batch_query_ms = 0;
  if (ChartData && g_pi_manager && g_pi_manager->HasPlugInChartSafetyGrid()) {
    wxStopWatch batch_select_timer;
    std::map<int, SegmentSafetyPluginBatchGroup> groups;
    const double grid_max_lat = ocpn::chart_safety::GlobalGridCoordinate(
        lat_tile, tile.rows - 1, kTileCells, tile.resolution);
    const double grid_max_lon = ocpn::chart_safety::GlobalGridCoordinate(
        lon_tile, tile.cols - 1, kTileCells, tile.resolution);
    const std::vector<SegmentSafetyChartCandidate> tile_candidates =
        SegmentSafetyTileChartCandidates(tile.min_lat, tile.min_lon,
                                         grid_max_lat, grid_max_lon);
    for (int r = 0; r < tile.rows; ++r) {
      const double cell_lat = ocpn::chart_safety::GlobalGridCoordinate(
          lat_tile, r, kTileCells, tile.resolution);
      for (int c = 0; c < tile.cols; ++c) {
        const double cell_lon = ocpn::chart_safety::GlobalGridCoordinate(
            lon_tile, c, kTileCells, tile.resolution);
        const size_t index = static_cast<size_t>(r) * tile.cols + c;
        bool selected_plugin = false;
        for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
                 tile_candidates.begin();
             it != tile_candidates.end(); ++it) {
          if (!ChartData->ChartCoversPosition(it->db_index,
                                              static_cast<float>(cell_lat),
                                              static_cast<float>(cell_lon)))
            continue;
          if (stats) ++stats->chart_stack_entries;
          if (it->plugin_vector) {
            plugin_depth_candidates[index].push_back(it->db_index);
            if (!selected_plugin) {
              std::map<int, SegmentSafetyPluginBatchGroup>::iterator found =
                  groups.find(it->db_index);
              if (found == groups.end()) {
                found = groups
                            .insert(std::make_pair(
                                it->db_index,
                                SegmentSafetyPluginBatchGroup(grid_cell_count)))
                            .first;
                found->second.db_index = it->db_index;
              }
              found->second.active[index] = 1;
              ++found->second.active_count;
              selected_plugin = true;
            }
            continue;
          }
          // The sorted table metadata uses the same provider, native-scale,
          // edition-date, file-time and path ordering as the point path.  A
          // native vector chart therefore supersedes later plugin charts; CM93
          // cannot, because it has the lower provider priority.
          if (!it->cm93) break;
        }
      }
    }

    size_t fast_selected_cells = 0;
    for (const auto& item : groups)
      fast_selected_cells += static_cast<size_t>(item.second.active_count);
    // Match the exact point path for a narrow plugin-coverage edge, where a
    // small omitted strip would otherwise force many slow point queries.  Do
    // not perform this chart-opening recovery when the plugin covers less than
    // half the tile: in that complementary case the established prepared-CM93
    // or native-vector fallback is both exact and much faster.
    if (fast_selected_cells * 2 >= grid_cell_count &&
        fast_selected_cells < grid_cell_count) {
      std::map<std::vector<int>, std::vector<SegmentSafetyChartCandidate> >
          stack_candidate_cache;
      for (int r = 0; r < tile.rows; ++r) {
        const double cell_lat = ocpn::chart_safety::GlobalGridCoordinate(
            lat_tile, r, kTileCells, tile.resolution);
        for (int c = 0; c < tile.cols; ++c) {
          const double cell_lon = ocpn::chart_safety::GlobalGridCoordinate(
              lon_tile, c, kTileCells, tile.resolution);
          const size_t index = static_cast<size_t>(r) * tile.cols + c;
          if (!plugin_depth_candidates[index].empty()) continue;
          std::set<int> stack_indexes;
          SegmentSafetyCandidateChartsAt(cell_lat, cell_lon, stack_indexes,
                                         stats);
          const std::vector<int> stack_key(stack_indexes.begin(),
                                           stack_indexes.end());
          std::map<std::vector<int>,
                   std::vector<SegmentSafetyChartCandidate> >::iterator cached =
              stack_candidate_cache.find(stack_key);
          if (cached == stack_candidate_cache.end()) {
            cached = stack_candidate_cache
                         .insert(std::make_pair(
                             stack_key, SegmentSafetySortedChartCandidates(
                                            cell_lat, cell_lon, stack_indexes)))
                         .first;
          }
          const std::vector<SegmentSafetyChartCandidate>& stack_candidates =
              cached->second;
          bool selected_plugin = false;
          for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
                   stack_candidates.begin();
               it != stack_candidates.end(); ++it) {
            if (it->plugin_vector) {
              plugin_depth_candidates[index].push_back(it->db_index);
              if (!selected_plugin) {
                std::map<int, SegmentSafetyPluginBatchGroup>::iterator found =
                    groups.find(it->db_index);
                if (found == groups.end()) {
                  found =
                      groups
                          .insert(std::make_pair(
                              it->db_index,
                              SegmentSafetyPluginBatchGroup(grid_cell_count)))
                          .first;
                  found->second.db_index = it->db_index;
                }
                found->second.active[index] = 1;
                ++found->second.active_count;
                selected_plugin = true;
              }
              continue;
            }
            if (!it->cm93) break;
          }
        }
      }
    }

    // Open only the plugin charts actually selected by at least one cell, and
    // only once per tile.  Unsupported plugin-vector formats deliberately
    // retain the exact point-query fallback below.
    for (std::map<int, SegmentSafetyPluginBatchGroup>::iterator it =
             groups.begin();
         it != groups.end(); ++it) {
      ChartBase* chart =
          ChartData->OpenChartFromDB(it->second.db_index, FULL_INIT);
      if (!IsSupportedSegmentSafetyPluginChart(chart)) continue;
      it->second.wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
    }
    plugin_batch_select_ms = batch_select_timer.Time();

    wxStopWatch batch_query_timer;
    for (std::map<int, SegmentSafetyPluginBatchGroup>::iterator it =
             groups.begin();
         it != groups.end(); ++it) {
      SegmentSafetyPluginBatchGroup& group = it->second;
      if (!group.wrapper || group.active_count <= 0) continue;
      HostApi123::ChartSafetyProviderRequest request = {};
      request.struct_size = sizeof(request);
      request.abi_version = HostApi123::kChartSafetyProviderAbiVersion;
      request.min_lat = tile.min_lat;
      request.min_lon = tile.min_lon;
      request.lat_step = tile.resolution;
      request.lon_step = tile.resolution;
      request.rows = tile.rows;
      request.cols = tile.cols;
      request.select_radius_degrees =
          static_cast<float>(kGridResolutionDegrees * 0.75);
      request.active_cells = group.active.data();
      request.visitor_context = &group;
      request.visit_object = SegmentSafetyPluginBatchVisit;
      HostApi123::ChartSafetyProviderResult provider_result = {};
      provider_result.struct_size = sizeof(provider_result);
      const double centre_lat = tile.min_lat + kGridTileDegrees / 2.0;
      const double centre_lon = tile.min_lon + kGridTileDegrees / 2.0;
      const ViewPort vp =
          SegmentSafetyHighestDetailViewPortAt(centre_lat, centre_lon);
      const HostApi123::ChartSafetyProviderStatus status =
          g_pi_manager->QueryPlugInChartSafetyGrid(group.wrapper, request,
                                                   &provider_result, vp);
      if (status != HostApi123::kChartSafetyProviderComplete ||
          provider_result.abi_version !=
              HostApi123::kChartSafetyProviderAbiVersion ||
          provider_result.processed_cells !=
              static_cast<uint32_t>(group.active_count)) {
        plugin_batch_fallback_cells += group.active_count;
        continue;
      }

      ++plugin_batch_groups;
      plugin_batch_cells += group.active_count;
      plugin_batch_candidate_objects += provider_result.candidate_objects;
      plugin_batch_hit_objects += provider_result.hit_objects;
      if (stats) ++stats->s57_chart_count;
      const bool provider_cache_allowed =
          (provider_result.result_flags &
           HostApi123::kChartSafetyDerivedCacheAllowed) != 0;
      for (size_t index = 0; index < grid_cell_count; ++index) {
        if (!group.active[index]) continue;
        plugin_batch_classified[index] = 1;
        plugin_batch_persistent_cache_allowed[index] =
            provider_cache_allowed ? 1 : 0;
        SegmentSafetyPointClass point_class = kPointWater;
        if (group.land[index])
          point_class = kPointLand;
        else if (group.drying[index])
          point_class = kPointDrying;
        tile.classes[index] = static_cast<unsigned char>(point_class);
        const uint16_t hazards = SegmentSafetyPointHazardFlags(point_class);
        tile.hazard_flags[index] = hazards;
        tile.hazard_summary_flags |= hazards;
        const bool depth_known =
            group.has_depth[index] && !group.unknown_danger_depth[index];
        plugin_batch_unknown_danger[index] =
            group.unknown_danger_depth[index] ? 1 : 0;
        tile.has_depth[index] = depth_known ? 1 : 0;
        tile.min_depth_m[index] = depth_known ? group.min_depth_m[index] : 0.0f;
        tile.has_drying[index] = group.drying[index] ? 1 : 0;
      }
      if (tile.source == kSourceNone) {
        tile.source = kSourcePluginVector;
        tile.chart_db_index = group.db_index;
        tile.chart_scale = group.wrapper->GetNativeScale();
        CopySegmentSafetyString(tile.chart_path, sizeof(tile.chart_path),
                                group.wrapper->GetFullPath().mb_str());
      }
    }

    // A detailed inset can legitimately cover a point without supplying a
    // DEPARE there (most commonly on an M_COVR boundary).  Preserve its
    // authoritative land/drying classification, but fill an otherwise
    // unknown water depth from the next licensed chart in the same sorted
    // priority chain.  An explicit danger object with unknown depth remains
    // unknown and cannot be hidden by a smaller-scale chart.
    size_t maximum_depth_candidate = 0;
    for (size_t index = 0; index < grid_cell_count; ++index)
      maximum_depth_candidate = std::max(maximum_depth_candidate,
                                         plugin_depth_candidates[index].size());
    for (size_t rank = 1; rank < maximum_depth_candidate; ++rank) {
      std::map<int, SegmentSafetyPluginBatchGroup> fallback_groups;
      for (size_t index = 0; index < grid_cell_count; ++index) {
        if (!plugin_batch_classified[index] || tile.has_depth[index] ||
            plugin_batch_unknown_danger[index] ||
            tile.classes[index] != kPointWater ||
            plugin_depth_candidates[index].size() <= rank)
          continue;
        const int db_index = plugin_depth_candidates[index][rank];
        std::map<int, SegmentSafetyPluginBatchGroup>::iterator found =
            fallback_groups.find(db_index);
        if (found == fallback_groups.end()) {
          found =
              fallback_groups
                  .insert(std::make_pair(
                      db_index, SegmentSafetyPluginBatchGroup(grid_cell_count)))
                  .first;
          found->second.db_index = db_index;
        }
        found->second.active[index] = 1;
        ++found->second.active_count;
      }

      for (std::map<int, SegmentSafetyPluginBatchGroup>::iterator it =
               fallback_groups.begin();
           it != fallback_groups.end(); ++it) {
        ChartBase* chart =
            ChartData->OpenChartFromDB(it->second.db_index, FULL_INIT);
        if (!IsSupportedSegmentSafetyPluginChart(chart)) continue;
        it->second.wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
      }

      for (std::map<int, SegmentSafetyPluginBatchGroup>::iterator it =
               fallback_groups.begin();
           it != fallback_groups.end(); ++it) {
        SegmentSafetyPluginBatchGroup& group = it->second;
        if (!group.wrapper || group.active_count <= 0) continue;
        HostApi123::ChartSafetyProviderRequest request = {};
        request.struct_size = sizeof(request);
        request.abi_version = HostApi123::kChartSafetyProviderAbiVersion;
        request.min_lat = tile.min_lat;
        request.min_lon = tile.min_lon;
        request.lat_step = tile.resolution;
        request.lon_step = tile.resolution;
        request.rows = tile.rows;
        request.cols = tile.cols;
        request.select_radius_degrees =
            static_cast<float>(kGridResolutionDegrees * 0.75);
        request.active_cells = group.active.data();
        request.visitor_context = &group;
        request.visit_object = SegmentSafetyPluginBatchVisit;
        HostApi123::ChartSafetyProviderResult provider_result = {};
        provider_result.struct_size = sizeof(provider_result);
        const double centre_lat = tile.min_lat + kGridTileDegrees / 2.0;
        const double centre_lon = tile.min_lon + kGridTileDegrees / 2.0;
        const ViewPort vp =
            SegmentSafetyHighestDetailViewPortAt(centre_lat, centre_lon);
        const HostApi123::ChartSafetyProviderStatus status =
            g_pi_manager->QueryPlugInChartSafetyGrid(group.wrapper, request,
                                                     &provider_result, vp);
        if (status != HostApi123::kChartSafetyProviderComplete ||
            provider_result.abi_version !=
                HostApi123::kChartSafetyProviderAbiVersion ||
            provider_result.processed_cells !=
                static_cast<uint32_t>(group.active_count)) {
          plugin_batch_fallback_cells += group.active_count;
          continue;
        }

        ++plugin_batch_groups;
        plugin_batch_cells += group.active_count;
        plugin_batch_candidate_objects += provider_result.candidate_objects;
        plugin_batch_hit_objects += provider_result.hit_objects;
        const bool fallback_cache_allowed =
            (provider_result.result_flags &
             HostApi123::kChartSafetyDerivedCacheAllowed) != 0;
        for (size_t index = 0; index < grid_cell_count; ++index) {
          if (group.active[index] && !fallback_cache_allowed)
            plugin_batch_persistent_cache_allowed[index] = 0;
          if (!group.active[index]) continue;
          if (group.land[index] || group.drying[index] ||
              group.unknown_danger_depth[index])
            continue;
          if (group.has_depth[index]) {
            tile.has_depth[index] = 1;
            tile.min_depth_m[index] = group.min_depth_m[index];
          }
        }
      }
    }
    plugin_batch_query_ms = batch_query_timer.Time();
  }

  // A CM93 composite normally selects its scale and reconstructs viewport
  // state for every queried point.  A 41x41 safety tile therefore used to do
  // 1,681 full CM93 viewport selections, taking tens of seconds even in
  // obvious open water.  Prepare every scale over the complete tile once and
  // select the highest-detail real M_COVR at each point from that immutable
  // working set.  If another vector chart competes at the tile centre, retain
  // the general per-point selection path so chart priority is unchanged.
  cm93compchart* prepared_cm93 = NULL;
  int prepared_cm93_db_index = -1;
  ViewPort prepared_cm93_vp;
  long cm93_prepare_ms = 0;
  int cm93_batch_cells = 0;
  int cm93_fallback_cells = 0;
  bool cm93_clear_shortcut = false;
  int cm93_hazard_objects = 0;
  int cm93_depth_boundary_attempts = 0;
  int cm93_depth_boundary_recoveries = 0;
  if (ChartData) {
    const double centre_lat = tile.min_lat + kGridTileDegrees / 2.0;
    const double centre_lon = tile.min_lon + kGridTileDegrees / 2.0;
    std::set<int> centre_chart_indexes;
    SegmentSafetyCandidateChartsAt(centre_lat, centre_lon, centre_chart_indexes,
                                   stats);
    if (centre_chart_indexes.size() == 1) {
      prepared_cm93_db_index = *centre_chart_indexes.begin();
      ChartBase* chart =
          ChartData->OpenChartFromDB(prepared_cm93_db_index, FULL_INIT);
      prepared_cm93 = dynamic_cast<cm93compchart*>(chart);
      if (prepared_cm93) {
        wxStopWatch prepare_timer;
        prepared_cm93_vp =
            SegmentSafetyHighestDetailViewPortAt(centre_lat, centre_lon);
        prepared_cm93_vp.b_quilt = false;
        prepared_cm93_vp.b_FullScreenQuilt = false;
        // At 1 pixel/metre an 8192-pixel viewport covers a 0.05-degree
        // tile at every navigable latitude, including a generous boundary
        // halo.  CM93 loads every intersecting native cell for each scale.
        prepared_cm93_vp.pix_width = 8192;
        prepared_cm93_vp.pix_height = 8192;
        prepared_cm93_vp.SetBoxes();
        prepared_cm93->PrepareSafetyTile(prepared_cm93_vp);
        cm93_prepare_ms = prepare_timer.Time();
      }
    }
  }

  // Most cold-route CM93 work is in 1,681 repeated point-in-object scans per
  // 0.05-degree tile.  Before doing those scans, prove that no loaded CM93
  // land/drying object's bounding box can affect this tile and that every
  // grid point has real M_COVR coverage.  This is a negative-only,
  // conservative shortcut: coastal/ambiguous tiles retain exact
  // classification, and depth-enabled routing also retains exact DEPARE
  // extraction.
  if (prepared_cm93 && !require_depth &&
      !prepared_cm93->SafetyAreaHazardMayIntersect(
          tile.min_lat, tile.min_lat + kGridTileDegrees, tile.min_lon,
          tile.min_lon + kGridTileDegrees, &cm93_hazard_objects)) {
    cm93_clear_shortcut = true;
    for (int r = 0; r < tile.rows && cm93_clear_shortcut; ++r) {
      const double cell_lat = ocpn::chart_safety::GlobalGridCoordinate(
          lat_tile, r, kTileCells, tile.resolution);
      for (int c = 0; c < tile.cols; ++c) {
        const double cell_lon = ocpn::chart_safety::GlobalGridCoordinate(
            lon_tile, c, kTileCells, tile.resolution);
        if (!prepared_cm93->GetHighestDetailSafetyChartAt(cell_lat, cell_lon)) {
          cm93_clear_shortcut = false;
          break;
        }
      }
    }
  }

  int land = 0, water = 0, drying = 0, unknown = 0;
  for (int r = 0; r < tile.rows; ++r) {
    const double cell_lat = ocpn::chart_safety::GlobalGridCoordinate(
        lat_tile, r, kTileCells, tile.resolution);
    for (int c = 0; c < tile.cols; ++c) {
      const int cell_index = r * tile.cols + c;
      if (plugin_batch_classified[cell_index]) {
        const SegmentSafetyPointClass point_class =
            static_cast<SegmentSafetyPointClass>(tile.classes[cell_index]);
        switch (point_class) {
          case kPointLand:
            ++land;
            break;
          case kPointWater:
            ++water;
            break;
          case kPointDrying:
            ++drying;
            break;
          default:
            ++unknown;
            break;
        }
        continue;
      }
      if (cm93_clear_shortcut) {
        tile.classes[cell_index] = (unsigned char)kPointWater;
        tile.hazard_flags[cell_index] = kHazardNone;
        ++water;
        continue;
      }
      const double cell_lon = ocpn::chart_safety::GlobalGridCoordinate(
          lon_tile, c, kTileCells, tile.resolution);
      SegmentSafetySource source = kSourceNone;
      SegmentSafetyResult cell_result = {};
      cell_result.struct_size = sizeof(cell_result);
      InitSegmentSafetyResult(&cell_result);
      SegmentSafetyPointClass point_class = kPointNoData;
      cm93chart* prepared_point_chart =
          prepared_cm93
              ? prepared_cm93->GetHighestDetailSafetyChartAt(cell_lat, cell_lon)
              : NULL;
      if (prepared_point_chart) {
        point_class = ChartPointSafetyClassAtPreparedCm93(
            prepared_point_chart, prepared_cm93_db_index, cell_lat, cell_lon,
            &prepared_cm93_vp, &source, stats, &cell_result);
        ++cm93_batch_cells;
      } else {
        point_class = ChartPointSafetyClassAtRaw(cell_lat, cell_lon, &source,
                                                 stats, &cell_result);
        if (prepared_cm93) ++cm93_fallback_cells;
      }
      // Built-in vector, CM93 and explicit no-chart results are core-owned
      // derived semantics and may share a persistent tile with provider cells.
      // A plugin-vector cell reached through the legacy point path has not
      // supplied the provider's cache-permission flag, so retain the previous
      // conservative refusal for that case.
      if (source == kSourcePluginVector)
        plugin_point_fallback_without_cache_permission = true;
      if (require_depth && prepared_cm93 && point_class == kPointWater &&
          !cell_result.has_depth) {
        ++cm93_depth_boundary_attempts;
        if (RecoverPreparedCm93BoundaryDepth(
                cell_lat, cell_lon, tile.resolution, stats, &cell_result))
          ++cm93_depth_boundary_recoveries;
      }
      tile.classes[cell_index] = (unsigned char)point_class;
      uint16_t cell_hazards = SegmentSafetyPointHazardFlags(point_class);
      tile.hazard_flags[cell_index] = cell_hazards;
      tile.hazard_summary_flags |= cell_hazards;
      if (SegmentSafetyResultHas(
              &cell_result,
              offsetof(SegmentSafetyResult, depth_source_attribute),
              sizeof(cell_result.depth_source_attribute))) {
        tile.has_depth[cell_index] = cell_result.has_depth ? 1 : 0;
        tile.min_depth_m[cell_index] = (float)cell_result.min_depth_m;
        tile.has_drying[cell_index] = cell_result.has_drying ? 1 : 0;
      }
      if (tile.source == kSourceNone && source != kSourceNone)
        tile.source = source;
      if (tile.chart_db_index < 0 && cell_result.chart_db_index >= 0) {
        tile.chart_db_index = cell_result.chart_db_index;
        tile.chart_scale = cell_result.chart_scale;
        snprintf(tile.chart_path, sizeof(tile.chart_path), "%s",
                 cell_result.chart_path);
      }
      switch (point_class) {
        case kPointLand:
          ++land;
          break;
        case kPointWater:
          ++water;
          break;
        case kPointDrying:
          ++drying;
          break;
        default:
          ++unknown;
          break;
      }
    }
  }

  int build_ms = timer.Time();
  tile.land_count = land;
  tile.water_count = water;
  tile.drying_count = drying;
  tile.unknown_count = unknown;
  tile.depth_complete = !cm93_clear_shortcut;
  tile.persistent_cache_allowed = tile.source != kSourcePluginVector;
  if (tile.source == kSourcePluginVector) {
    tile.persistent_cache_allowed =
        !plugin_point_fallback_without_cache_permission;
    for (size_t index = 0; index < grid_cell_count; ++index) {
      // Every plugin cell classified by the batch provider must explicitly
      // advertise the derived-cache contract. Unclassified cells are safe to
      // persist only when the point path above established that they came from
      // a built-in/core-owned source.
      if (plugin_batch_classified[index] &&
          !plugin_batch_persistent_cache_allowed[index]) {
        tile.persistent_cache_allowed = false;
        break;
      }
    }
  }
  if (cm93_clear_shortcut) {
    const double centre_lat = tile.min_lat + kGridTileDegrees / 2.0;
    const double centre_lon = tile.min_lon + kGridTileDegrees / 2.0;
    cm93chart* representative =
        prepared_cm93->GetHighestDetailSafetyChartAt(centre_lat, centre_lon);
    tile.source = kSourceCm93;
    tile.chart_db_index = prepared_cm93_db_index;
    tile.chart_scale = representative ? representative->GetNativeScale()
                                      : prepared_cm93->GetNativeScale();
    snprintf(tile.chart_path, sizeof(tile.chart_path), "%s",
             (representative ? representative->GetFullPath()
                             : prepared_cm93->GetFullPath())
                 .mb_str()
                 .data());
  }
  if (stats) {
    stats->grid_build_ms += build_ms;
    stats->grid_cells_total += tile.rows * tile.cols;
    stats->grid_cells_land += land;
    stats->grid_cells_water += water;
    stats->grid_cells_drying += drying;
    stats->grid_cells_unknown += unknown;
  }

  wxLogMessage(
      "SEGMENT_SAFETY_GRID built key=%ld:%ld group=%d bbox=[lat %.6f..%.6f "
      "lon %.6f..%.6f] resolution_deg=%.6f cells=%d land=%d water=%d "
      "drying=%d unknown=%d build_ms=%d source=%d chart_db_index=%d "
      "chart_scale=%d chart_path=\"%s\" cm93_prepare_ms=%ld "
      "cm93_batch_cells=%d cm93_fallback_cells=%d "
      "cm93_clear_shortcut=%d cm93_hazard_objects=%d "
      "cm93_depth_boundary_attempts=%d cm93_depth_boundary_recoveries=%d "
      "depth_complete=%d",
      lat_tile, lon_tile, tile.group_index, tile.min_lat,
      tile.min_lat + kGridTileDegrees, tile.min_lon,
      tile.min_lon + kGridTileDegrees, tile.resolution, tile.rows * tile.cols,
      land, water, drying, unknown, build_ms, tile.source, tile.chart_db_index,
      tile.chart_scale, tile.chart_path, cm93_prepare_ms, cm93_batch_cells,
      cm93_fallback_cells, cm93_clear_shortcut ? 1 : 0, cm93_hazard_objects,
      cm93_depth_boundary_attempts, cm93_depth_boundary_recoveries,
      tile.depth_complete ? 1 : 0);
  if (plugin_batch_groups || plugin_batch_fallback_cells) {
    wxLogMessage(
        "SEGMENT_SAFETY_PLUGIN_BATCH key=%ld:%ld groups=%d cells=%d "
        "fallback_cells=%d select_ms=%ld query_ms=%ld "
        "candidate_objects=%d hit_objects=%d",
        lat_tile, lon_tile, plugin_batch_groups, plugin_batch_cells,
        plugin_batch_fallback_cells, plugin_batch_select_ms,
        plugin_batch_query_ms, plugin_batch_candidate_objects,
        plugin_batch_hit_objects);
  }

  return tile;
}

bool SegmentSafetyAllTouchedTilesAreWater(double lat1, double lon1, double lat2,
                                          double lon2, double safety_margin_nm,
                                          double bearing, double dist_nm,
                                          int samples,
                                          SegmentSafetyCoreStats* stats) {
  if (stats) stats->segment_sample_count += samples;
  std::set<std::pair<long, long> > tiles;
  for (int i = 0; i < samples; ++i) {
    double sample_dist = samples == 1 ? 0.0 : dist_nm * i / (samples - 1);
    double lat = lat1;
    double lon = lon1;
    if (sample_dist > 0.0)
      ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);

    long lat_tile = 0;
    long lon_tile = 0;
    SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
    tiles.insert(std::make_pair(lat_tile, lon_tile));

    if (safety_margin_nm > 0.0) {
      double left_lat, left_lon, right_lat, right_lon;
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing - 90.0),
               safety_margin_nm, &left_lat, &left_lon);
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing + 90.0),
               safety_margin_nm, &right_lat, &right_lon);
      SegmentSafetyGridTileKey(left_lat, left_lon, &lat_tile, &lon_tile);
      tiles.insert(std::make_pair(lat_tile, lon_tile));
      SegmentSafetyGridTileKey(right_lat, right_lon, &lat_tile, &lon_tile);
      tiles.insert(std::make_pair(lat_tile, lon_tile));
    }
  }

  if (tiles.empty()) return false;

  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    std::string key = SegmentSafetyGridTileKeyForIndices(it->first, it->second);
    CachedPointSafetyGridTile tile;
    if (!LookupSegmentSafetyGridTile(key, &tile)) {
      bool built = false;
      if (!EnsureSegmentSafetyGridTile(it->first, it->second, stats, &built))
        return false;
      if (built)
        RecordUnexpectedSegmentSafetyTileBuild(stats, it->first, it->second);
      if (!LookupSegmentSafetyGridTile(key, &tile)) return false;
    } else if (stats) {
      ++stats->grid_cache_hits;
    }

    if (tile.classes.empty() || tile.hazard_summary_flags != kHazardNone)
      return false;
  }

  if (stats) ++stats->water_tile_shortcuts;
  return true;
}

SegmentSafetyPointClass ChartPointSafetyClassAt(
    double lat, double lon, SegmentSafetySource* source,
    SegmentSafetyCoreStats* stats, SegmentSafetyResult* result = NULL) {
  long lat_tile = 0;
  long lon_tile = 0;
  std::string key = SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
  CachedPointSafetyGridTile tile;
  if (!LookupSegmentSafetyGridTile(key, &tile)) {
    bool built = false;
    EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &built);
    if (built)
      RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    LookupSegmentSafetyGridTile(key, &tile);
  } else if (stats) {
    ++stats->grid_cache_hits;
  }

  if (tile.classes.empty()) {
    if (!wxThread::IsMain()) return kPointNoData;
    return ChartPointSafetyClassAtRaw(lat, lon, source, stats, result);
  }

  if (stats) ++stats->grid_lookups;
  int row = (int)lround((lat - tile.min_lat) / tile.resolution);
  int col = (int)lround((lon - tile.min_lon) / tile.resolution);
  if (row < 0 || row >= tile.rows || col < 0 || col >= tile.cols) {
    if (!wxThread::IsMain()) return kPointNoData;
    return ChartPointSafetyClassAtRaw(lat, lon, source, stats, result);
  }

  SegmentSafetyPointClass point_class =
      (SegmentSafetyPointClass)tile.classes[row * tile.cols + col];
  if (source) *source = tile.source;
  int cell_index = row * tile.cols + col;
  if (SegmentSafetyResultHas(result, offsetof(SegmentSafetyResult, hit_object),
                             sizeof(result->hit_object))) {
    result->chart_db_index = tile.chart_db_index;
    result->chart_scale = tile.chart_scale;
    strncpy(result->chart_path, tile.chart_path,
            sizeof(result->chart_path) - 1);
    result->chart_path[sizeof(result->chart_path) - 1] = '\0';
    if (point_class == kPointLand)
      strncpy(result->hit_object, "grid LAND cell",
              sizeof(result->hit_object) - 1);
  }
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    bool has_depth = cell_index >= 0 &&
                     cell_index < (int)tile.has_depth.size() &&
                     tile.has_depth[cell_index] != 0;
    bool has_drying = cell_index >= 0 &&
                      cell_index < (int)tile.has_drying.size() &&
                      tile.has_drying[cell_index] != 0;
    result->has_depth = has_depth ? 1 : 0;
    result->min_depth_m = has_depth ? tile.min_depth_m[cell_index] : 0.0;
    result->has_drying = has_drying ? 1 : 0;
    if (has_depth) {
      strncpy(result->depth_source_object, "grid DEPARE cell",
              sizeof(result->depth_source_object) - 1);
      result->depth_source_object[sizeof(result->depth_source_object) - 1] =
          '\0';
      strncpy(result->depth_source_attribute, "DEPARE/DRVAL1",
              sizeof(result->depth_source_attribute) - 1);
      result
          ->depth_source_attribute[sizeof(result->depth_source_attribute) - 1] =
          '\0';
    }
  }
  return point_class;
}

bool SegmentSafetyWaterNeighborhoodAt(double lat, double lon, int radius_cells,
                                      SegmentSafetyCoreStats* stats) {
  for (int dlat = -radius_cells; dlat <= radius_cells; ++dlat) {
    for (int dlon = -radius_cells; dlon <= radius_cells; ++dlon) {
      double check_lat = lat + dlat * kGridResolutionDegrees;
      double check_lon = lon + dlon * kGridResolutionDegrees;
      SegmentSafetySource source = kSourceNone;
      SegmentSafetyPointClass point_class =
          ChartPointSafetyClassAt(check_lat, check_lon, &source, stats);
      if (point_class != kPointWater) return false;
    }
  }
  return true;
}

bool SegmentSafetyCoarseSampledCellsAreWater(double lat1, double lon1,
                                             double lat2, double lon2,
                                             double safety_margin_nm,
                                             double bearing, double dist_nm,
                                             SegmentSafetyCoreStats* stats) {
  const double coarse_spacing_nm = 0.25;
  const int max_coarse_samples = 256;
  int coarse_samples = wxMax(
      2, wxMin(max_coarse_samples, (int)ceil(dist_nm / coarse_spacing_nm) + 1));
  if (stats) stats->segment_sample_count += coarse_samples;

  for (int i = 0; i < coarse_samples; ++i) {
    double sample_dist =
        coarse_samples == 1 ? 0.0 : dist_nm * i / (coarse_samples - 1);
    double lat = lat1;
    double lon = lon1;
    if (sample_dist > 0.0)
      ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);

    if (!SegmentSafetyWaterNeighborhoodAt(lat, lon, 1, stats)) return false;

    if (safety_margin_nm > 0.0) {
      double left_lat, left_lon, right_lat, right_lon;
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing - 90.0),
               safety_margin_nm, &left_lat, &left_lon);
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing + 90.0),
               safety_margin_nm, &right_lat, &right_lon);
      if (!SegmentSafetyWaterNeighborhoodAt(left_lat, left_lon, 1, stats) ||
          !SegmentSafetyWaterNeighborhoodAt(right_lat, right_lon, 1, stats))
        return false;
    }
  }

  if (stats) ++stats->water_tile_shortcuts;
  return true;
}

bool SegmentSafetyGridCellAt(long lat_cell, long lon_cell,
                             SegmentSafetySource* source,
                             SegmentSafetyCoreStats* stats,
                             SegmentSafetyResult* result,
                             SegmentSafetyPointClass* point_class,
                             uint16_t* hazard_flags) {
  double lat = lat_cell * kGridResolutionDegrees;
  double lon = lon_cell * kGridResolutionDegrees;

  long lat_tile = 0;
  long lon_tile = 0;
  std::string key = SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
  if (!LookupSegmentSafetyGridTile(key, NULL)) {
    bool built = false;
    if (!EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &built))
      return false;
    if (built)
      RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
  } else if (stats) {
    ++stats->grid_cache_hits;
  }

  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
      s_segment_safety_grid_cache.find(key);
  if (it == s_segment_safety_grid_cache.end()) return false;

  const CachedPointSafetyGridTile& tile = it->second;
  if (tile.classes.empty()) return false;

  int row = (int)lround((lat - tile.min_lat) / tile.resolution);
  int col = (int)lround((lon - tile.min_lon) / tile.resolution);
  if (row < 0 || row >= tile.rows || col < 0 || col >= tile.cols) return false;

  int cell_index = row * tile.cols + col;
  SegmentSafetyPointClass cls =
      (SegmentSafetyPointClass)tile.classes[cell_index];
  uint16_t hazards =
      cell_index >= 0 && cell_index < (int)tile.hazard_flags.size()
          ? tile.hazard_flags[cell_index]
          : (uint16_t)kHazardNoChart;
  if (point_class) *point_class = cls;
  if (hazard_flags) *hazard_flags = hazards;
  if (source) *source = tile.source;
  if (stats) ++stats->grid_lookups;

  if (SegmentSafetyResultHas(result, offsetof(SegmentSafetyResult, hit_object),
                             sizeof(result->hit_object))) {
    result->chart_db_index = tile.chart_db_index;
    result->chart_scale = tile.chart_scale;
    strncpy(result->chart_path, tile.chart_path,
            sizeof(result->chart_path) - 1);
    result->chart_path[sizeof(result->chart_path) - 1] = '\0';
    if (cls == kPointLand)
      strncpy(result->hit_object, "grid LAND cell",
              sizeof(result->hit_object) - 1);
  }
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    bool has_depth = cell_index >= 0 &&
                     cell_index < (int)tile.has_depth.size() &&
                     tile.has_depth[cell_index] != 0;
    bool has_drying = cell_index >= 0 &&
                      cell_index < (int)tile.has_drying.size() &&
                      tile.has_drying[cell_index] != 0;
    result->has_depth = has_depth ? 1 : 0;
    result->min_depth_m = has_depth ? tile.min_depth_m[cell_index] : 0.0;
    result->has_drying = has_drying ? 1 : 0;
    if (has_depth) {
      strncpy(result->depth_source_object, "grid DEPARE cell",
              sizeof(result->depth_source_object) - 1);
      result->depth_source_object[sizeof(result->depth_source_object) - 1] =
          '\0';
      strncpy(result->depth_source_attribute, "DEPARE/DRVAL1",
              sizeof(result->depth_source_attribute) - 1);
      result
          ->depth_source_attribute[sizeof(result->depth_source_attribute) - 1] =
          '\0';
    }
  }

  return cls != kPointNoData;
}

bool SegmentSafetyRouteMaskCellAt(
    long lat_cell, long lon_cell, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, bool force_authoritative_fine,
    SegmentSafetyCoreStats* stats, SegmentSafetyResult* result,
    uint16_t* block_flags, SegmentSafetySource* source) {
  double lat = lat_cell * kGridResolutionDegrees;
  double lon = lon_cell * kGridResolutionDegrees;
  long lat_tile = 0;
  long lon_tile = 0;
  SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
  std::string key = SegmentSafetyRouteMaskKey(
      lat_tile, lon_tile, safety_margin_nm, check_depth, minimum_depth_m);
  CachedSegmentSafetyRouteMaskTile mask;
  if (!LookupSegmentSafetyRouteMaskTile(key, &mask) ||
      (force_authoritative_fine && !mask.authoritative_fine)) {
    bool built = false;
    if (!EnsureSegmentSafetyRouteMaskTile(lat_tile, lon_tile, safety_margin_nm,
                                          check_depth, minimum_depth_m, stats,
                                          &built, force_authoritative_fine))
      return false;
    if (built)
      RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    if (!LookupSegmentSafetyRouteMaskTile(key, &mask)) return false;
    if (force_authoritative_fine && !mask.authoritative_fine) return false;
  } else if (stats) {
    ++stats->grid_cache_hits;
  }

  if (mask.block_flags.empty()) return false;
  int row = (int)lround((lat - mask.min_lat) / mask.resolution);
  int col = (int)lround((lon - mask.min_lon) / mask.resolution);
  if (row < 0 || row >= mask.rows || col < 0 || col >= mask.cols) return false;

  int cell_index = row * mask.cols + col;
  uint16_t flags = cell_index >= 0 && cell_index < (int)mask.block_flags.size()
                       ? mask.block_flags[cell_index]
                       : (uint16_t)kRouteNeedsTile;
  if (block_flags) *block_flags = flags;
  if (source) *source = mask.source;
  if (stats) ++stats->grid_lookups;

  if (SegmentSafetyResultHas(result, offsetof(SegmentSafetyResult, hit_object),
                             sizeof(result->hit_object))) {
    result->chart_db_index = mask.chart_db_index;
    result->chart_scale = mask.chart_scale;
    strncpy(result->chart_path, mask.chart_path,
            sizeof(result->chart_path) - 1);
    result->chart_path[sizeof(result->chart_path) - 1] = '\0';
  }
  return true;
}

void SegmentSafetySetRouteMaskHitResult(SegmentSafetyResult* result,
                                        uint16_t flags,
                                        SegmentSafetySource source,
                                        long lat_cell, long lon_cell,
                                        int sample_index, int sample_count) {
  if (!result) return;

  result->hit_sample_lat = lat_cell * kGridResolutionDegrees;
  result->hit_sample_lon = lon_cell * kGridResolutionDegrees;
  result->hit_sample_index = sample_index;
  result->hit_sample_count = sample_count;
  SetSegmentSafetySource(result, source);

  if (flags & kRouteBlockLand) {
    SetSegmentSafetyStatus(result, kCrossesLand);
    SetSegmentSafetyMessage(result, "route mask segment intersects chart land");
    strncpy(result->hit_object, "route mask LAND cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & kRouteBlockDrying) {
    SetSegmentSafetyStatus(result, kDryingArea);
    SetSegmentSafetyMessage(result,
                            "route mask segment intersects drying area");
    strncpy(result->hit_object, "route mask DRYING cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & kRouteBlockTooShallow) {
    SetSegmentSafetyStatus(result, kTooShallow);
    SetSegmentSafetyMessage(result, "route mask segment is too shallow");
    strncpy(result->hit_object, "route mask TOO_SHALLOW cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & kRouteBlockUnknownDepth) {
    SetSegmentSafetyStatus(result, kUnknownDepth);
    SetSegmentSafetyMessage(result, "route mask segment has unknown depth");
    strncpy(result->hit_object, "route mask UNKNOWN_DEPTH cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & kRouteBlockNoChart) {
    SetSegmentSafetyStatus(result, kNoData);
    SetSegmentSafetyMessage(result, "route mask segment has no chart coverage");
    strncpy(result->hit_object, "route mask NO_CHART cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & kRouteBlockUnknownClass) {
    SetSegmentSafetyStatus(result, kNoData);
    SetSegmentSafetyMessage(result,
                            "route mask segment has unknown chart class");
    strncpy(result->hit_object, "route mask UNKNOWN_CLASS cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & kRouteNeedsTile) {
    SetSegmentSafetyStatus(result, kUnsafeArea);
    SetSegmentSafetyMessage(result, "route mask tile is not built");
    strncpy(result->hit_object, "route mask NEEDS_TILE cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & kRouteBlockMargin) {
    SetSegmentSafetyStatus(result, kWithinLandMargin);
    SetSegmentSafetyMessage(result, "route mask segment enters safety margin");
    strncpy(result->hit_object, "route mask MARGIN cell",
            sizeof(result->hit_object) - 1);
  }
  result->hit_object[sizeof(result->hit_object) - 1] = '\0';
}

bool SegmentSafetyCoarseRouteMaskCertifiedSafeCheck(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, SegmentSafetyResult* result,
    SegmentSafetyCoreStats* stats, bool* answered) {
  if (answered) *answered = false;

  double coarse_degrees = kGridTileDegrees * kCoarseRouteMaskFactor;
  long min_lat_cell = (long)floor(wxMin(lat1, lat2) / coarse_degrees);
  long max_lat_cell = (long)floor(wxMax(lat1, lat2) / coarse_degrees);
  long min_lon_cell = (long)floor(wxMin(lon1, lon2) / coarse_degrees);
  long max_lon_cell = (long)floor(wxMax(lon1, lon2) / coarse_degrees);

  long lat_count = max_lat_cell - min_lat_cell + 1;
  long lon_count = max_lon_cell - min_lon_cell + 1;
  long coarse_count =
      lat_count > 0 && lon_count > 0 ? lat_count * lon_count : 0;
  const long max_coarse_bbox_cells = 256;
  if (coarse_count <= 0 || coarse_count > max_coarse_bbox_cells) {
    if (stats) ++stats->coarse_unknown_fallbacks;
    return false;
  }

  SegmentSafetySource first_source = kSourceNone;
  for (long lat_cell = min_lat_cell; lat_cell <= max_lat_cell; ++lat_cell) {
    for (long lon_cell = min_lon_cell; lon_cell <= max_lon_cell; ++lon_cell) {
      CachedSegmentSafetyCoarseRouteMaskCell coarse;
      if (stats) ++stats->coarse_cells_checked;
      if (!EnsureSegmentSafetyCoarseRouteMaskCell(
              lat_cell, lon_cell, safety_margin_nm, check_depth,
              minimum_depth_m, &coarse, stats)) {
        if (stats) ++stats->coarse_missing;
        return false;
      }

      if (coarse.source != kSourceNone && first_source == kSourceNone)
        first_source = coarse.source;

      if (coarse.state != kCoarseCertifiedSafe) {
        if (stats) {
          if (coarse.state == kCoarseNoChart)
            ++stats->coarse_no_chart;
          else if (coarse.state == kCoarseDepthUnproven)
            ++stats->coarse_depth_unproven;
          else
            ++stats->coarse_mixed_fallbacks;
        }
        return false;
      }
    }
  }

  if (answered) *answered = true;
  if (stats) {
    ++stats->coarse_certified_safe_hits;
    stats->fine_tiles_avoided +=
        (int)(coarse_count * kCoarseRouteMaskFactor * kCoarseRouteMaskFactor);
  }
  SetSegmentSafetyStatus(result, kSafe);
  SetSegmentSafetySource(
      result, first_source != kSourceNone ? first_source : kSourceVectorChart);
  SetSegmentSafetyDiagnosticReason(result, kDiagnosticChartGeometryClear);
  SetSegmentSafetyMessage(result, "coarse route mask certified clear");
  return true;
}

bool SegmentSafetyRouteMaskTraversalCheck(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, bool force_authoritative_fine,
    SegmentSafetyResult* result, bool* chart_data_available,
    SegmentSafetyCoreStats* stats, bool* open_water_shortcut, bool* answered) {
  if (answered) *answered = false;

  bool coarse_answered = false;
  if (!force_authoritative_fine &&
      SegmentSafetyCoarseRouteMaskCertifiedSafeCheck(
          lat1, lon1, lat2, lon2, safety_margin_nm, check_depth,
          minimum_depth_m, result, stats, &coarse_answered)) {
    if (chart_data_available) *chart_data_available = true;
    if (open_water_shortcut) *open_water_shortcut = true;
    if (answered) *answered = true;
    return false;
  }
  if (!force_authoritative_fine && coarse_answered) {
    if (chart_data_available) *chart_data_available = true;
    if (open_water_shortcut) *open_water_shortcut = true;
    if (answered) *answered = true;
    return false;
  }

  long y0 = lround(lat1 / kGridResolutionDegrees);
  long x0 = lround(lon1 / kGridResolutionDegrees);
  long y1 = lround(lat2 / kGridResolutionDegrees);
  long x1 = lround(lon2 / kGridResolutionDegrees);
  long dx = labs(x1 - x0);
  long dy = labs(y1 - y0);
  long sx = x0 < x1 ? 1 : -1;
  long sy = y0 < y1 ? 1 : -1;
  long err = dx - dy;
  long steps = wxMax(dx, dy) + 1;
  if (steps <= 0) steps = 1;
  if (stats) stats->segment_sample_count += steps;

  bool any_chart_data = false;
  bool all_clear = true;
  bool missing_mask = false;
  SegmentSafetySource first_source = kSourceNone;

  long x = x0;
  long y = y0;
  for (long i = 0; i < steps; ++i) {
    uint16_t flags = kRouteNeedsTile;
    SegmentSafetySource source = kSourceNone;
    if (!SegmentSafetyRouteMaskCellAt(y, x, safety_margin_nm, check_depth,
                                      minimum_depth_m, force_authoritative_fine,
                                      stats, result, &flags, &source)) {
      // Worker-side misses enqueue their exact mask tile.  Continue walking
      // the segment so a single query publishes every missing tile instead
      // of forcing one worker/GUI round trip per 0.05-degree boundary.  The
      // query still fails closed below until all samples are available.
      missing_mask = true;
      all_clear = false;
    } else {
      any_chart_data = true;
      if (first_source == kSourceNone) first_source = source;
      if (flags != kRouteClear) {
        all_clear = false;
        if (chart_data_available) *chart_data_available = true;
        if (answered) *answered = true;
        SegmentSafetySetRouteMaskHitResult(result, flags, source, y, x, (int)i,
                                           (int)steps);
        return true;
      }
    }

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

  if (missing_mask) {
    if (answered) *answered = false;
    return false;
  }

  if (any_chart_data) {
    if (chart_data_available) *chart_data_available = true;
    if (answered) *answered = true;
    if (GetSegmentSafetySource(result) == kSourceNone)
      SetSegmentSafetySource(result, first_source);
    if (open_water_shortcut && all_clear) *open_water_shortcut = true;
    if (all_clear && stats) ++stats->water_tile_shortcuts;
  }
  return false;
}

void SegmentSafetyCheckGridCellNeighborhood(
    long lat_cell, long lon_cell, int radius_cells,
    SegmentSafetySource* first_source, bool* any_chart_data, bool* all_water,
    bool* land_hit, bool* drying_hit, bool* shallow_hit,
    bool* unknown_depth_hit, bool check_depth, double minimum_depth_m,
    SegmentSafetyCoreStats* stats, SegmentSafetyResult* result,
    int sample_index, int sample_count) {
  for (int dlat = -radius_cells; dlat <= radius_cells; ++dlat) {
    for (int dlon = -radius_cells; dlon <= radius_cells; ++dlon) {
      SegmentSafetySource source = kSourceNone;
      SegmentSafetyPointClass point_class = kPointNoData;
      uint16_t hazard_flags = kHazardNoChart;
      if (!SegmentSafetyGridCellAt(lat_cell + dlat, lon_cell + dlon, &source,
                                   stats, result, &point_class,
                                   &hazard_flags)) {
        *all_water = false;
        if (check_depth) *unknown_depth_hit = true;
        continue;
      }

      *any_chart_data = true;
      if (*first_source == kSourceNone) *first_source = source;
      if (hazard_flags != kHazardNone) *all_water = false;
      if (SegmentSafetyResultHas(
              result, offsetof(SegmentSafetyResult, depth_source_attribute),
              sizeof(result->depth_source_attribute))) {
        result->required_depth_m = minimum_depth_m;
      }
      if (hazard_flags & kHazardLand) {
        *land_hit = true;
        if (SegmentSafetyResultHas(result,
                                   offsetof(SegmentSafetyResult, hit_object),
                                   sizeof(result->hit_object))) {
          result->hit_sample_lat = (lat_cell + dlat) * kGridResolutionDegrees;
          result->hit_sample_lon = (lon_cell + dlon) * kGridResolutionDegrees;
          result->hit_sample_index = sample_index;
          result->hit_sample_count = sample_count;
        }
        return;
      }
      if (hazard_flags & kHazardNoChart) {
        if (check_depth) *unknown_depth_hit = true;
        continue;
      }
      if (!check_depth) continue;
      if (point_class == kPointWater &&
          SegmentSafetyResultHas(
              result, offsetof(SegmentSafetyResult, depth_source_attribute),
              sizeof(result->depth_source_attribute)) &&
          !result->has_depth) {
        *unknown_depth_hit = true;
        result->hit_sample_lat = (lat_cell + dlat) * kGridResolutionDegrees;
        result->hit_sample_lon = (lon_cell + dlon) * kGridResolutionDegrees;
        result->hit_sample_index = sample_index;
        result->hit_sample_count = sample_count;
        result->required_depth_m = minimum_depth_m;
        strncpy(result->hit_object, "grid UNKNOWN_DEPTH cell",
                sizeof(result->hit_object) - 1);
        result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        return;
      }
      if (hazard_flags & kHazardDrying) {
        *drying_hit = true;
        if (SegmentSafetyResultHas(result,
                                   offsetof(SegmentSafetyResult, hit_object),
                                   sizeof(result->hit_object))) {
          result->hit_sample_lat = (lat_cell + dlat) * kGridResolutionDegrees;
          result->hit_sample_lon = (lon_cell + dlon) * kGridResolutionDegrees;
          result->hit_sample_index = sample_index;
          result->hit_sample_count = sample_count;
          strncpy(result->hit_object, "grid DRYING cell",
                  sizeof(result->hit_object) - 1);
          result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        }
        return;
      }
      if (SegmentSafetyResultHas(
              result, offsetof(SegmentSafetyResult, depth_source_attribute),
              sizeof(result->depth_source_attribute)) &&
          result->has_depth && result->min_depth_m < minimum_depth_m) {
        *shallow_hit = true;
        result->hit_sample_lat = (lat_cell + dlat) * kGridResolutionDegrees;
        result->hit_sample_lon = (lon_cell + dlon) * kGridResolutionDegrees;
        result->hit_sample_index = sample_index;
        result->hit_sample_count = sample_count;
        result->hit_depth_m = result->min_depth_m;
        strncpy(result->hit_object, "grid TOO_SHALLOW cell",
                sizeof(result->hit_object) - 1);
        result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        return;
      }
    }
  }
}

bool SegmentSafetyGridTraversalCheck(double lat1, double lon1, double lat2,
                                     double lon2, double safety_margin_nm,
                                     bool check_depth, double minimum_depth_m,
                                     SegmentSafetyResult* result,
                                     bool* chart_data_available,
                                     SegmentSafetyCoreStats* stats,
                                     bool* open_water_shortcut) {
  long y0 = lround(lat1 / kGridResolutionDegrees);
  long x0 = lround(lon1 / kGridResolutionDegrees);
  long y1 = lround(lat2 / kGridResolutionDegrees);
  long x1 = lround(lon2 / kGridResolutionDegrees);

  long dx = labs(x1 - x0);
  long dy = labs(y1 - y0);
  long sx = x0 < x1 ? 1 : -1;
  long sy = y0 < y1 ? 1 : -1;
  long err = dx - dy;
  long steps = wxMax(dx, dy) + 1;
  if (steps <= 0) steps = 1;

  double mid_lat = (lat1 + lat2) / 2.0;
  double cell_nm =
      wxMin(kGridResolutionDegrees * 60.0,
            kGridResolutionDegrees * 60.0 *
                wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mid_lat)))));
  int radius_cells = safety_margin_nm > 0.0
                         ? (int)ceil(safety_margin_nm / wxMax(0.01, cell_nm))
                         : 0;
  radius_cells = wxMin(radius_cells, 128);

  if (stats) stats->segment_sample_count += steps;

  bool any_chart_data = false;
  bool all_water = true;
  bool land_hit = false;
  bool drying_hit = false;
  bool shallow_hit = false;
  bool unknown_depth_hit = false;
  SegmentSafetySource first_source = kSourceNone;

  long x = x0;
  long y = y0;
  for (long i = 0; i < steps; ++i) {
    SegmentSafetyCheckGridCellNeighborhood(
        y, x, radius_cells, &first_source, &any_chart_data, &all_water,
        &land_hit, &drying_hit, &shallow_hit, &unknown_depth_hit, check_depth,
        minimum_depth_m, stats, result, (int)i, (int)steps);
    if (land_hit || drying_hit || shallow_hit || unknown_depth_hit) {
      if (chart_data_available) *chart_data_available = true;
      SegmentSafetyStatus status =
          radius_cells > 0 ? kWithinLandMargin : kCrossesLand;
      const char* message =
          radius_cells > 0 ? "segment grid traversal enters chart land margin"
                           : "segment grid traversal intersects chart land";
      if (drying_hit) {
        status = kDryingArea;
        message = "segment grid traversal intersects chart drying area";
      } else if (shallow_hit) {
        status = kTooShallow;
        message =
            "segment grid traversal intersects chart area shallower than "
            "required depth";
      } else if (unknown_depth_hit) {
        status = kUnknownDepth;
        message = "segment grid traversal lacks chart depth data";
      }
      SetSegmentSafetyStatus(result, status);
      SetSegmentSafetySource(result, first_source);
      SetSegmentSafetyMessage(result, message);
      return true;
    }

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

  if (any_chart_data) {
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == kSourceNone)
      SetSegmentSafetySource(result, first_source);
    if (open_water_shortcut) *open_water_shortcut = all_water;
  }

  if (all_water && any_chart_data && stats) ++stats->water_tile_shortcuts;
  return false;
}

wxString SegmentSafetyPointDiagnostic(double lat, double lon) {
  SegmentSafetyCoreStats stats;
  std::set<int> chart_indexes;
  SegmentSafetyCandidateChartsAt(lat, lon, chart_indexes, &stats);

  wxString objects;
  wxString source_name = "none";
  wxString chart_path;
  wxString point_class = "UNKNOWN";
  int chart_db_index = -1;
  int chart_scale = -1;
  int area_count = 0;
  int land_count = 0;
  int drying_count = 0;
  int depare_count = 0;
  bool has_depth = false;
  bool unknown_danger_depth_seen = false;
  double min_depth_m = 0.0;
  wxString depth_attr = "none";
  bool chart_checked = false;
  time_t chart_edition_date = 0;
  time_t chart_file_time = 0;

  std::vector<SegmentSafetyChartCandidate> candidates =
      SegmentSafetySortedChartCandidates(lat, lon, chart_indexes);
  for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
           candidates.begin();
       it != candidates.end(); ++it) {
    ChartBase* chart =
        ChartData ? ChartData->OpenChartFromDB(it->db_index, FULL_INIT) : NULL;
    s57chart* s57 = dynamic_cast<s57chart*>(chart);
    ChartPlugInWrapper* plugin_wrapper =
        dynamic_cast<ChartPlugInWrapper*>(chart);
    if (it->plugin_vector && plugin_wrapper) {
      chart_checked = true;
      chart_db_index = it->db_index;
      chart_path = chart->GetFullPath();
      chart_edition_date = it->edition_date;
      chart_file_time = it->file_time;
      chart_scale = chart->GetNativeScale();
      source_name = "PLUGIN_VECTOR";
      const ViewPort vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
      ListOfPI_S57Obj* object_list =
          g_pi_manager
              ? g_pi_manager->GetPlugInObjRuleListAtLatLon(
                    plugin_wrapper, static_cast<float>(lat),
                    static_cast<float>(lon),
                    static_cast<float>(kGridResolutionDegrees * 0.75), vp)
              : NULL;
      if (!object_list) {
        chart_checked = false;
        break;
      }
      for (ListOfPI_S57Obj::Node* node = object_list->GetFirst(); node;
           node = node->GetNext()) {
        PI_S57Obj* object = node->GetData();
        if (!object) continue;
        ++area_count;
        if (!objects.empty()) objects += ";";
        objects += SegmentSafetyPluginObjectSummary(object);
        if (!strncmp(object->FeatureName, "LNDARE", 6) ||
            SegmentSafetyPluginObjectIsAlwaysDry(object))
          ++land_count;
        if (SegmentSafetyPluginObjectIsDrying(object)) ++drying_count;
        double object_depth_m = 0.0;
        wxString object_depth_attr;
        bool unknown_danger_depth = false;
        if (SegmentSafetyPluginObjectDepthM(object, &object_depth_m,
                                            &object_depth_attr,
                                            &unknown_danger_depth)) {
          ++depare_count;
          if (!has_depth || object_depth_m < min_depth_m) {
            has_depth = true;
            min_depth_m = object_depth_m;
            depth_attr = object_depth_attr;
          }
        }
        if (unknown_danger_depth) {
          unknown_danger_depth_seen = true;
          depth_attr =
              wxString::Format("%s/VALSOU missing", object->FeatureName);
        }
      }
      if (unknown_danger_depth_seen) has_depth = false;
      object_list->Clear();
      delete object_list;
      break;
    }
    if (!s57) continue;

    chart_checked = true;
    chart_db_index = it->db_index;
    chart_path = chart->GetFullPath();
    chart_edition_date = it->edition_date;
    chart_file_time = it->file_time;
    bool cm93 = IsCm93Chart(chart);
    source_name = cm93 ? "CM93" : "VECTOR_CHART";
    ViewPort vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
    if (cm93) {
      cm93compchart* cm93_chart = dynamic_cast<cm93compchart*>(chart);
      if (cm93_chart) cm93_chart->SetVPParms(vp);
    }
    chart_scale = chart->GetNativeScale();

    ListOfObjRazRules* rule_list =
        s57->GetObjRuleListAtLatLon(lat, lon, 0.0, &vp, MASK_AREA);
    if (!rule_list) break;

    for (ListOfObjRazRules::Node* node = rule_list->GetFirst(); node;
         node = node->GetNext()) {
      ObjRazRules* rule = node->GetData();
      if (!rule || !rule->obj) continue;
      ++area_count;
      if (!objects.empty()) objects += ";";
      objects += SegmentSafetyRuleSummary(rule);

      if (!strncmp(rule->obj->FeatureName, "LNDARE", 6)) ++land_count;
      if (SegmentSafetyRuleIsDrying(rule)) ++drying_count;
      double rule_depth = 0.0;
      if (SegmentSafetyRuleDepthMinM(rule, &rule_depth)) {
        ++depare_count;
        if (!has_depth || rule_depth < min_depth_m) {
          has_depth = true;
          min_depth_m = rule_depth;
          depth_attr = rule->obj->GetAttrValueAsString("DRVAL1");
        }
      } else if (!strncmp(rule->obj->FeatureName, "DEPARE", 6)) {
        ++depare_count;
      }
    }

    rule_list->Clear();
    delete rule_list;
    break;
  }

  if (!chart_checked)
    point_class = "NO_DATA";
  else if (land_count > 0)
    point_class = "LAND";
  else if (drying_count > 0)
    point_class = "DRYING";
  else
    point_class = "WATER_OR_NO_UNSAFE_AREA";

  if (objects.empty()) objects = "none";
  chart_path.Replace("\"", "'");
  return wxString::Format(
             "class=%s source=%s chart_db_index=%d chart_scale=%d "
             "chart_path=\"%s\" "
             "chart_edition=%ld chart_file_time=%ld "
             "selection=highest-detail-newest "
             "chart_stack_entries=%d candidate_charts=%zu area_objects=%d "
             "land_objects=%d drying_objects=%d depare_objects=%d "
             "objects=\"%s\"",
             point_class, source_name, chart_db_index, chart_scale, chart_path,
             (long)chart_edition_date, (long)chart_file_time,
             stats.chart_stack_entries, candidates.size(), area_count,
             land_count, drying_count, depare_count, objects) +
         wxString::Format(" has_depth=%d min_depth_m=%.2f depth_attr=\"%s\"",
                          has_depth ? 1 : 0, has_depth ? min_depth_m : 0.0,
                          depth_attr);
}

CachedChartLandGeometry& SegmentSafetyLoadChartLandGeometry(
    int db_index, double lat, double lon, SegmentSafetyCoreStats* stats) {
  if (!wxThread::IsMain()) {
    if (stats) stats->chart_load_failed = true;
    static CachedChartLandGeometry worker_no_chart_data;
    wxLogMessage(
        "WR_GRID_THREAD_VIOLATION SegmentSafetyLoadChartLandGeometry called "
        "from worker thread=%p main_thread=%p db_index=%d. Chart geometry "
        "loading is main-thread only.",
        wxThread::GetCurrentId(), wxThread::GetMainId(), db_index);
    return worker_no_chart_data;
  }

  if (!ChartData) {
    if (stats) stats->no_chart_database = true;
    static CachedChartLandGeometry no_chart_data;
    return no_chart_data;
  }
  wxStopWatch cache_timer;
  ChartBase* chart = ChartData->OpenChartFromDB(db_index, FULL_INIT);
  bool cm93 = IsCm93Chart(chart);
  std::string cache_key = SegmentSafetyCacheKey(db_index, cm93, lat, lon);
  CachedChartLandGeometry& cached = s_segment_safety_land_cache[cache_key];
  if (cached.loaded) return cached;
  cached.loaded = true;
  cached.cache_key = cache_key;
  if (chart) cached.chart_path = chart->GetFullPath();

  s57chart* s57 = dynamic_cast<s57chart*>(chart);
  if (!s57) {
    if (stats) {
      stats->chart_load_failed = chart == NULL;
      stats->cache_build_ms += cache_timer.Time();
    }
    return cached;
  }

  cached.source = cm93 ? kSourceCm93 : kSourceVectorChart;
  if (stats) ++stats->s57_chart_count;

  if (cm93) {
    cm93compchart* cm93_chart = dynamic_cast<cm93compchart*>(chart);
    if (cm93_chart) cm93_chart->SetVPParms(SegmentSafetyViewPortAt(lat, lon));
  }

  std::vector<std::vector<wxPoint2DDouble> > rings;
  s57->CollectFeatureAreaRings("LNDARE", rings);
  for (size_t i = 0; i < rings.size(); ++i) {
    if (rings[i].size() < 3) continue;
    CachedLandRing ring;
    ring.points.swap(rings[i]);
    ring.bbox = SegmentSafetyRingBBox(ring.points);
    if (ring.bbox.max_lat < lat - 10.0 || ring.bbox.min_lat > lat + 10.0 ||
        ring.bbox.max_lon < lon - 10.0 || ring.bbox.min_lon > lon + 10.0)
      continue;
    cached.rings.push_back(ring);
  }

  wxLogMessage(
      "OpenCPN segment safety: cached chart land geometry "
      "db_index=%d key=%s source=%d type=%d scale=%d land_rings=%zu",
      db_index, cache_key.c_str(), (int)cached.source,
      chart ? (int)chart->GetChartType() : -1,
      chart ? chart->GetNativeScale() : -1, cached.rings.size());
  for (size_t i = 0; i < cached.rings.size() && i < 8; ++i) {
    const CachedLandRing& ring = cached.rings[i];
    wxLogMessage(
        "OpenCPN segment safety: land ring sample db_index=%d key=%s "
        "ring=%zu bbox=[lat %.8f..%.8f lon %.8f..%.8f] points=%zu",
        db_index, cache_key.c_str(), i, ring.bbox.min_lat, ring.bbox.max_lat,
        ring.bbox.min_lon, ring.bbox.max_lon, ring.points.size());
  }
  if (cached.rings.empty()) {
    wxString chart_path = chart ? chart->GetFullPath() : wxString();
    wxLogMessage(
        "OpenCPN segment safety: no LNDARE rings db_index=%d key=%s "
        "path=%s summary=%s",
        db_index, cache_key.c_str(), chart_path.c_str(),
        s57->GetFeatureDebugSummary());
  }
  if (stats) {
    stats->cache_build_ms += cache_timer.Time();
    if (cached.rings.empty()) stats->zero_land_geometry = true;
  }

  return cached;
}

void LogSegmentSafetyChartHit(const char* cause, int db_index,
                              const std::string& cache_key,
                              SegmentSafetySource source,
                              const CachedLandRing& ring, double lat1,
                              double lon1, double lat2, double lon2,
                              double safety_margin_nm, size_t edge_index) {
  if (s_segment_safety_chart_hit_logs >= kMaxChartHitLogs) return;
  ++s_segment_safety_chart_hit_logs;
  wxLogMessage(
      "OpenCPN segment safety: LNDARE hit #%ld cause=%s db_index=%d key=%s "
      "source=%d segment=(%.8f,%.8f)->(%.8f,%.8f) margin_nm=%.3f "
      "ring_bbox=[lat %.8f..%.8f lon %.8f..%.8f] ring_points=%zu "
      "edge_index=%zu",
      s_segment_safety_chart_hit_logs, cause, db_index, cache_key.c_str(),
      (int)source, lat1, lon1, lat2, lon2, safety_margin_nm, ring.bbox.min_lat,
      ring.bbox.max_lat, ring.bbox.min_lon, ring.bbox.max_lon,
      ring.points.size(), edge_index);
}

void SetSegmentSafetyChartHitDetails(SegmentSafetyResult* result,
                                     SegmentSafetyHitCause cause, int db_index,
                                     const CachedChartLandGeometry& chart_cache,
                                     const CachedLandRing& ring,
                                     size_t edge_index) {
  if (!SegmentSafetyResultHas(result, offsetof(SegmentSafetyResult, chart_path),
                              sizeof(result->chart_path)))
    return;

  result->chart_db_index = db_index;
  result->hit_cause = cause;
  result->hit_ring_min_lat = ring.bbox.min_lat;
  result->hit_ring_max_lat = ring.bbox.max_lat;
  result->hit_ring_min_lon = ring.bbox.min_lon;
  result->hit_ring_max_lon = ring.bbox.max_lon;
  result->hit_ring_point_count = (int)ring.points.size();
  result->hit_edge_index = (int)edge_index;
  strncpy(result->chart_path, chart_cache.chart_path.mb_str(),
          sizeof(result->chart_path) - 1);
  result->chart_path[sizeof(result->chart_path) - 1] = '\0';
}

bool CachedChartSegmentSafetyCheck(double lat1, double lon1, double lat2,
                                   double lon2, double safety_margin_nm,
                                   SegmentSafetyResult* result,
                                   bool* chart_data_available,
                                   SegmentSafetyCoreStats* stats) {
  if (!wxThread::IsMain()) {
    if (stats) stats->chart_load_failed = true;
    if (chart_data_available) *chart_data_available = false;
    SetSegmentSafetyDiagnosticReason(result, kDiagnosticNoCandidateChart);
    SetSegmentSafetyMessage(
        result,
        "legacy chart geometry check is unavailable from worker thread");
    return false;
  }

  wxStopWatch select_timer;
  std::set<int> chart_indexes;
  SegmentSafetyCandidateChartsAt(lat1, lon1, chart_indexes, stats);
  SegmentSafetyCandidateChartsAt(lat2, lon2, chart_indexes, stats);
  SegmentSafetyCandidateChartsAt((lat1 + lat2) / 2.0, (lon1 + lon2) / 2.0,
                                 chart_indexes, stats);
  if (stats) {
    stats->candidate_chart_count = chart_indexes.size();
    stats->chart_select_ms += select_timer.Time();
  }
  if (chart_indexes.empty()) return false;

  wxStopWatch geometry_timer;
  wxPoint2DDouble start(lon1, lat1);
  wxPoint2DDouble end(lon2, lat2);
  SegmentSafetyBBox segment_box =
      SegmentSafetySegmentBBox(lat1, lon1, lat2, lon2, safety_margin_nm);

  for (std::set<int>::const_iterator it = chart_indexes.begin();
       it != chart_indexes.end(); ++it) {
    CachedChartLandGeometry& chart_cache = SegmentSafetyLoadChartLandGeometry(
        *it, (lat1 + lat2) / 2.0, (lon1 + lon2) / 2.0, stats);
    if (chart_cache.source == kSourceNone) continue;
    if (chart_cache.rings.empty()) {
      if (stats) stats->zero_land_geometry = true;
      continue;
    }
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == kSourceNone)
      SetSegmentSafetySource(result, chart_cache.source);
    if (stats) stats->land_ring_count += chart_cache.rings.size();

    for (size_t i = 0; i < chart_cache.rings.size(); ++i) {
      const CachedLandRing& ring = chart_cache.rings[i];
      if (!SegmentSafetyBBoxIntersects(segment_box, ring.bbox)) continue;
      if (stats) ++stats->bbox_ring_tests;

      if (SegmentSafetyPointInRing(lat1, lon1, ring.points) ||
          SegmentSafetyPointInRing(lat2, lon2, ring.points)) {
        if (stats) stats->geometry_check_ms += geometry_timer.Time();
        LogSegmentSafetyChartHit("endpoint-inside-LNDARE", *it,
                                 chart_cache.cache_key, chart_cache.source,
                                 ring, lat1, lon1, lat2, lon2, safety_margin_nm,
                                 0);
        SetSegmentSafetyChartHitDetails(result, kHitEndpointInLandArea, *it,
                                        chart_cache, ring, 0);
        SetSegmentSafetyStatus(result, kCrossesLand);
        SetSegmentSafetySource(result, chart_cache.source);
        SetSegmentSafetyMessage(result,
                                "segment endpoint is inside chart land area");
        return true;
      }

      for (size_t j = 0; j < ring.points.size(); ++j) {
        const wxPoint2DDouble& a = ring.points[j];
        const wxPoint2DDouble& b = ring.points[(j + 1) % ring.points.size()];
        if (stats) ++stats->edge_tests;
        if (SegmentSafetySegmentsIntersect(start, end, a, b)) {
          if (stats) stats->geometry_check_ms += geometry_timer.Time();
          LogSegmentSafetyChartHit("segment-intersects-LNDARE-edge", *it,
                                   chart_cache.cache_key, chart_cache.source,
                                   ring, lat1, lon1, lat2, lon2,
                                   safety_margin_nm, j);
          SetSegmentSafetyChartHitDetails(result,
                                          kHitSegmentIntersectsLandAreaEdge,
                                          *it, chart_cache, ring, j);
          SetSegmentSafetyStatus(result, kCrossesLand);
          SetSegmentSafetySource(result, chart_cache.source);
          SetSegmentSafetyMessage(result,
                                  "segment intersects chart land boundary");
          return true;
        }

        if (safety_margin_nm > 0.0 &&
            SegmentSafetySegmentDistanceNm(start, end, a, b) <=
                safety_margin_nm) {
          if (stats) stats->geometry_check_ms += geometry_timer.Time();
          LogSegmentSafetyChartHit("approx-margin-to-LNDARE-edge", *it,
                                   chart_cache.cache_key, chart_cache.source,
                                   ring, lat1, lon1, lat2, lon2,
                                   safety_margin_nm, j);
          SetSegmentSafetyChartHitDetails(result, kHitMarginToLandAreaEdge, *it,
                                          chart_cache, ring, j);
          SetSegmentSafetyStatus(result, kWithinLandMargin);
          SetSegmentSafetySource(result, chart_cache.source);
          SetSegmentSafetyMessage(
              result, "segment is within approximate chart land safety margin");
          return true;
        }
      }
    }
  }

  if (stats) stats->geometry_check_ms += geometry_timer.Time();
  return false;
}

bool ChartSegmentPointClassificationCheck(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, bool force_authoritative_fine,
    SegmentSafetyResult* result, bool* chart_data_available,
    SegmentSafetyCoreStats* stats, bool* open_water_shortcut) {
  wxStopWatch geometry_timer;
  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);
  if (open_water_shortcut) *open_water_shortcut = false;

  const int max_samples = 512;
  int samples = wxMax(2, wxMin(max_samples, (int)ceil(dist_nm / 0.05) + 1));
  bool any_chart_data = false;
  SegmentSafetySource first_source = kSourceNone;

  bool route_mask_answered = false;
  if (SegmentSafetyRouteMaskTraversalCheck(
          lat1, lon1, lat2, lon2, safety_margin_nm, check_depth,
          minimum_depth_m, force_authoritative_fine, result,
          chart_data_available, stats, open_water_shortcut,
          &route_mask_answered)) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    return true;
  }
  if (route_mask_answered) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    return false;
  }

  if (!wxThread::IsMain()) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    if (chart_data_available) *chart_data_available = false;
    return false;
  }

  wxStopWatch grid_lookup_timer;
  if (!check_depth && SegmentSafetyAllTouchedTilesAreWater(
                          lat1, lon1, lat2, lon2, safety_margin_nm, bearing,
                          dist_nm, samples, stats)) {
    if (stats) {
      stats->grid_lookup_ms += grid_lookup_timer.Time();
      stats->geometry_check_ms += geometry_timer.Time();
    }
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == kSourceNone)
      SetSegmentSafetySource(result, kSourceVectorChart);
    if (open_water_shortcut) *open_water_shortcut = true;
    return false;
  }

  if (SegmentSafetyGridTraversalCheck(lat1, lon1, lat2, lon2, safety_margin_nm,
                                      check_depth, minimum_depth_m, result,
                                      chart_data_available, stats,
                                      open_water_shortcut)) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    return true;
  }
  if (chart_data_available && *chart_data_available) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    return false;
  }

  /*
   * From this point down the legacy fallback path uses point classification
   * helpers which may need to build grid tiles.  Tile building requires
   * OpenCPN chart object/rule access, so worker threads must stop here and let
   * the caller's missing-tile retry/prewarm path service the request on the
   * main thread.
   */
  if (!wxThread::IsMain()) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    if (chart_data_available) *chart_data_available = false;
    return false;
  }

  if (!check_depth &&
      SegmentSafetyCoarseSampledCellsAreWater(
          lat1, lon1, lat2, lon2, safety_margin_nm, bearing, dist_nm, stats)) {
    if (stats) {
      stats->grid_lookup_ms += grid_lookup_timer.Time();
      stats->geometry_check_ms += geometry_timer.Time();
    }
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == kSourceNone)
      SetSegmentSafetySource(result, kSourceVectorChart);
    if (open_water_shortcut) *open_water_shortcut = true;
    return false;
  }

  if (stats) stats->segment_sample_count += samples;
  for (int i = 0; i < samples; ++i) {
    double sample_dist = samples == 1 ? 0.0 : dist_nm * i / (samples - 1);
    double lat = lat1;
    double lon = lon1;
    if (sample_dist > 0.0)
      ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);

    SegmentSafetySource source = kSourceNone;
    wxStopWatch lookup_timer;
    SegmentSafetyPointClass point_class =
        ChartPointSafetyClassAt(lat, lon, &source, stats, result);
    if (stats) stats->grid_lookup_ms += lookup_timer.Time();
    if (point_class == kPointNoData) continue;

    any_chart_data = true;
    if (first_source == kSourceNone) first_source = source;
    if (GetSegmentSafetySource(result) == kSourceNone)
      SetSegmentSafetySource(result, source);

    if (point_class == kPointLand) {
      if (stats) stats->geometry_check_ms += geometry_timer.Time();
      if (chart_data_available) *chart_data_available = true;
      if (SegmentSafetyResultHas(result,
                                 offsetof(SegmentSafetyResult, hit_object),
                                 sizeof(result->hit_object))) {
        result->hit_sample_lat = lat;
        result->hit_sample_lon = lon;
        result->hit_sample_index = i;
        result->hit_sample_count = samples;
      }
      SetSegmentSafetyStatus(result, kCrossesLand);
      SetSegmentSafetySource(result, source);
      SetSegmentSafetyMessage(result,
                              "segment samples intersect chart land area");
      if (s_segment_safety_chart_hit_logs < kMaxChartHitLogs) {
        ++s_segment_safety_chart_hit_logs;
        wxLogMessage(
            "FIRST_LAND_HIT source=chart-point segment=(%.8f,%.8f)->"
            "(%.8f,%.8f) sample=(%.8f,%.8f) sample_index=%d/%d "
            "object=\"%s\" chart_db_index=%d chart_scale=%d "
            "chart_path=\"%s\"",
            lat1, lon1, lat2, lon2, lat, lon, i + 1, samples,
            result ? result->hit_object : "",
            result ? result->chart_db_index : -1,
            result ? result->chart_scale : -1,
            result ? result->chart_path : "");
      }
      return true;
    }

    if (check_depth && point_class == kPointWater &&
        SegmentSafetyResultHas(
            result, offsetof(SegmentSafetyResult, depth_source_attribute),
            sizeof(result->depth_source_attribute)) &&
        !result->has_depth) {
      if (stats) stats->geometry_check_ms += geometry_timer.Time();
      if (chart_data_available) *chart_data_available = true;
      result->hit_sample_lat = lat;
      result->hit_sample_lon = lon;
      result->hit_sample_index = i;
      result->hit_sample_count = samples;
      result->required_depth_m = minimum_depth_m;
      strncpy(result->hit_object, "chart UNKNOWN_DEPTH area",
              sizeof(result->hit_object) - 1);
      result->hit_object[sizeof(result->hit_object) - 1] = '\0';
      SetSegmentSafetyStatus(result, kUnknownDepth);
      SetSegmentSafetySource(result, source);
      SetSegmentSafetyMessage(result, "segment samples lack chart depth data");
      return true;
    }

    if (check_depth && point_class == kPointDrying) {
      if (stats) stats->geometry_check_ms += geometry_timer.Time();
      if (chart_data_available) *chart_data_available = true;
      if (SegmentSafetyResultHas(result,
                                 offsetof(SegmentSafetyResult, hit_object),
                                 sizeof(result->hit_object))) {
        result->hit_sample_lat = lat;
        result->hit_sample_lon = lon;
        result->hit_sample_index = i;
        result->hit_sample_count = samples;
        result->required_depth_m = minimum_depth_m;
        result->has_drying = 1;
        strncpy(result->hit_object, "chart DRYING area",
                sizeof(result->hit_object) - 1);
        result->hit_object[sizeof(result->hit_object) - 1] = '\0';
      }
      SetSegmentSafetyStatus(result, kDryingArea);
      SetSegmentSafetySource(result, source);
      SetSegmentSafetyMessage(result,
                              "segment samples intersect chart drying area");
      return true;
    }

    if (check_depth &&
        SegmentSafetyResultHas(
            result, offsetof(SegmentSafetyResult, depth_source_attribute),
            sizeof(result->depth_source_attribute)) &&
        result->has_depth && result->min_depth_m < minimum_depth_m) {
      if (stats) stats->geometry_check_ms += geometry_timer.Time();
      if (chart_data_available) *chart_data_available = true;
      result->hit_sample_lat = lat;
      result->hit_sample_lon = lon;
      result->hit_sample_index = i;
      result->hit_sample_count = samples;
      result->required_depth_m = minimum_depth_m;
      result->hit_depth_m = result->min_depth_m;
      strncpy(result->hit_object, "chart TOO_SHALLOW area",
              sizeof(result->hit_object) - 1);
      result->hit_object[sizeof(result->hit_object) - 1] = '\0';
      SetSegmentSafetyStatus(result, kTooShallow);
      SetSegmentSafetySource(result, source);
      SetSegmentSafetyMessage(
          result,
          "segment samples intersect chart area shallower than required depth");
      return true;
    }

    if (safety_margin_nm > 0.0) {
      double left_lat, left_lon, right_lat, right_lon;
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing - 90.0),
               safety_margin_nm, &left_lat, &left_lon);
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing + 90.0),
               safety_margin_nm, &right_lat, &right_lon);
      SegmentSafetySource left_source = kSourceNone;
      SegmentSafetySource right_source = kSourceNone;
      wxStopWatch margin_lookup_timer;
      SegmentSafetyPointClass left_class =
          ChartPointSafetyClassAt(left_lat, left_lon, &left_source, stats);
      SegmentSafetyPointClass right_class =
          ChartPointSafetyClassAt(right_lat, right_lon, &right_source, stats);
      if (stats) stats->grid_lookup_ms += margin_lookup_timer.Time();
      if (left_class != kPointNoData || right_class != kPointNoData)
        any_chart_data = true;
      if (left_class == kPointLand || right_class == kPointLand) {
        if (stats) stats->geometry_check_ms += geometry_timer.Time();
        if (chart_data_available) *chart_data_available = true;
        SetSegmentSafetyStatus(result, kWithinLandMargin);
        SetSegmentSafetySource(
            result, left_class == kPointLand ? left_source : right_source);
        SetSegmentSafetyMessage(
            result, "segment samples are within approximate chart land margin");
        return true;
      }
    }
  }

  if (any_chart_data) {
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == kSourceNone)
      SetSegmentSafetySource(result, first_source);
  }
  if (stats) stats->geometry_check_ms += geometry_timer.Time();
  return false;
}

bool GshhsSegmentSafetyHitsLand(double lat1, double lon1, double lat2,
                                double lon2, double safety_margin_nm,
                                SegmentSafetyStatus* status) {
  if (PlugIn_GSHHS_CrossesLand(lat1, lon1, lat2, lon2)) {
    if (status) *status = kCrossesLand;
    return true;
  }

  if (safety_margin_nm <= 0.0) return false;

  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);

  double lat_up1, lon_up1, lat_up2, lon_up2;
  double lat_down1, lon_down1, lat_down2, lon_down2;
  ll_gc_ll(lat1, lon1, SegmentSafetyNormalizeBearing(bearing - 90.0),
           safety_margin_nm, &lat_up1, &lon_up1);
  ll_gc_ll(lat2, lon2, SegmentSafetyNormalizeBearing(bearing - 90.0),
           safety_margin_nm, &lat_up2, &lon_up2);
  ll_gc_ll(lat1, lon1, SegmentSafetyNormalizeBearing(bearing + 90.0),
           safety_margin_nm, &lat_down1, &lon_down1);
  ll_gc_ll(lat2, lon2, SegmentSafetyNormalizeBearing(bearing + 90.0),
           safety_margin_nm, &lat_down2, &lon_down2);

  if (PlugIn_GSHHS_CrossesLand(lat_up1, lon_up1, lat_up2, lon_up2) ||
      PlugIn_GSHHS_CrossesLand(lat_down1, lon_down1, lat_down2, lon_down2) ||
      PlugIn_GSHHS_CrossesLand(lat_up1, lon_up1, lat_down2, lon_down2) ||
      PlugIn_GSHHS_CrossesLand(lat_down1, lon_down1, lat_up2, lon_up2)) {
    if (status) *status = kWithinLandMargin;
    return true;
  }

  return false;
}

}  // namespace detail

}  // namespace ocpn::chart_safety
