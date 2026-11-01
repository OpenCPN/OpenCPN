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
 * Core chart selection, semantic extraction, tile caching and depth
 * classification for the chart-safety service.
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

void CopySegmentSafetyString(char* dest, size_t dest_size, const char* source) {
  if (!dest || dest_size == 0) return;
  snprintf(dest, dest_size, "%s", source ? source : "");
}

void SetSegmentSafetyMessage(SegmentSafetyResult* result, const char* message) {
  if (!result) return;
  if (result->struct_size <
      (int)(offsetof(SegmentSafetyResult, message) + sizeof(result->message)))
    return;
  CopySegmentSafetyString(result->message, sizeof(result->message), message);
}

void InitSegmentSafetyResult(SegmentSafetyResult* result) {
  if (!result) return;
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, status) + sizeof(result->status)))
    result->status = kError;
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, source) + sizeof(result->source)))
    result->source = kSourceNone;
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, used_fallback) +
            sizeof(result->used_fallback)))
    result->used_fallback = 0;
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, message) + sizeof(result->message)))
    result->message[0] = '\0';
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, geometry_check_ms) +
            sizeof(result->geometry_check_ms))) {
    result->diagnostic_reason = kDiagnosticNone;
    result->chart_stack_entries = 0;
    result->candidate_chart_count = 0;
    result->raster_chart_count = 0;
    result->unsupported_chart_count = 0;
    result->s57_chart_count = 0;
    result->land_ring_count = 0;
    result->bbox_ring_tests = 0;
    result->edge_tests = 0;
    result->cache_build_ms = 0;
    result->chart_select_ms = 0;
    result->geometry_check_ms = 0;
  }
  if (result->struct_size >= (int)(offsetof(SegmentSafetyResult, chart_path) +
                                   sizeof(result->chart_path))) {
    result->chart_db_index = -1;
    result->hit_cause = kHitNone;
    result->hit_ring_min_lat = 0.0;
    result->hit_ring_max_lat = 0.0;
    result->hit_ring_min_lon = 0.0;
    result->hit_ring_max_lon = 0.0;
    result->hit_ring_point_count = 0;
    result->hit_edge_index = -1;
    result->chart_path[0] = '\0';
  }
  if (result->struct_size >= (int)(offsetof(SegmentSafetyResult, hit_object) +
                                   sizeof(result->hit_object))) {
    result->hit_sample_lat = 0.0;
    result->hit_sample_lon = 0.0;
    result->hit_sample_index = -1;
    result->hit_sample_count = 0;
    result->chart_scale = -1;
    result->hit_object[0] = '\0';
  }
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, point_cache_misses) +
            sizeof(result->point_cache_misses))) {
    result->point_cache_hits = 0;
    result->point_cache_misses = 0;
  }
  if (result->struct_size >= (int)(offsetof(SegmentSafetyResult, grid_lookups) +
                                   sizeof(result->grid_lookups))) {
    result->grid_cache_hits = 0;
    result->grid_cache_misses = 0;
    result->grid_build_ms = 0;
    result->grid_cells_total = 0;
    result->grid_cells_land = 0;
    result->grid_cells_water = 0;
    result->grid_cells_drying = 0;
    result->grid_cells_unknown = 0;
    result->grid_lookups = 0;
  }
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, unexpected_tile_builds) +
            sizeof(result->unexpected_tile_builds))) {
    result->grid_lookup_ms = 0;
    result->segment_sample_count = 0;
    result->water_tile_shortcuts = 0;
    result->unexpected_tile_builds = 0;
  }
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, unexpected_tile_min_lon) +
            sizeof(result->unexpected_tile_min_lon))) {
    result->unexpected_lat_tile = 0;
    result->unexpected_lon_tile = 0;
    result->unexpected_tile_min_lat = 0.0;
    result->unexpected_tile_min_lon = 0.0;
  }
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, grid_cache_evictions) +
            sizeof(result->grid_cache_evictions))) {
    result->segment_cache_hits = 0;
    result->segment_cache_misses = 0;
    result->segment_cache_stores = 0;
    result->grid_cache_size = 0;
    result->grid_cache_evictions = 0;
  }
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, depth_source_attribute) +
            sizeof(result->depth_source_attribute))) {
    result->has_depth = 0;
    result->min_depth_m = 0.0;
    result->required_depth_m = 0.0;
    result->hit_depth_m = 0.0;
    result->has_drying = 0;
    result->depth_source_object[0] = '\0';
    result->depth_source_attribute[0] = '\0';
  }
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, prewarm_fine_tiles_avoided) +
            sizeof(result->prewarm_fine_tiles_avoided))) {
    result->prewarm_requested_tiles = 0;
    result->prewarm_base_tiles_built = 0;
    result->prewarm_base_tiles_reused = 0;
    result->prewarm_masks_built = 0;
    result->prewarm_masks_reused = 0;
    result->prewarm_fine_tiles_avoided = 0;
  }
}

void SetSegmentSafetyStatus(SegmentSafetyResult* result,
                            SegmentSafetyStatus status) {
  if (!result) return;
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, status) + sizeof(result->status)))
    result->status = status;
}

void SetSegmentSafetySource(SegmentSafetyResult* result,
                            SegmentSafetySource source) {
  if (!result) return;
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, source) + sizeof(result->source)))
    result->source = source;
}

void SetSegmentSafetyFallback(SegmentSafetyResult* result, bool used_fallback) {
  if (!result) return;
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, used_fallback) +
            sizeof(result->used_fallback)))
    result->used_fallback = used_fallback ? 1 : 0;
}

SegmentSafetySource GetSegmentSafetySource(const SegmentSafetyResult* result) {
  if (!result) return kSourceNone;
  if (result->struct_size >=
      (int)(offsetof(SegmentSafetyResult, source) + sizeof(result->source)))
    return (SegmentSafetySource)result->source;
  return kSourceNone;
}

bool SegmentSafetyResultHas(const SegmentSafetyResult* result, size_t offset,
                            size_t size) {
  return result && result->struct_size >= (int)(offset + size);
}

void SetSegmentSafetyDiagnosticReason(SegmentSafetyResult* result,
                                      SegmentSafetyDiagnosticReason reason) {
  if (!SegmentSafetyResultHas(result,
                              offsetof(SegmentSafetyResult, diagnostic_reason),
                              sizeof(result->diagnostic_reason)))
    return;
  result->diagnostic_reason = reason;
}

void SetSegmentSafetyDiagnosticInt(SegmentSafetyResult* result, size_t offset,
                                   int value) {
  if (!SegmentSafetyResultHas(result, offset, sizeof(int))) return;
  *reinterpret_cast<int*>(reinterpret_cast<char*>(result) + offset) = value;
}

void SetSegmentSafetyDiagnosticDouble(SegmentSafetyResult* result,
                                      size_t offset, double value) {
  if (!SegmentSafetyResultHas(result, offset, sizeof(double))) return;
  *reinterpret_cast<double*>(reinterpret_cast<char*>(result) + offset) = value;
}

bool SegmentSafetyOptionsHas(const SegmentSafetyOptions* options, size_t offset,
                             size_t size) {
  return options && options->struct_size >= (int)(offset + size);
}

double SegmentSafetyOptionMargin(const SegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(options,
                              offsetof(SegmentSafetyOptions, safety_margin_nm),
                              sizeof(options->safety_margin_nm)))
    return wxMax(0.0, options->safety_margin_nm);
  return 0.0;
}

bool SegmentSafetyOptionCheckLand(const SegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(options,
                              offsetof(SegmentSafetyOptions, check_land),
                              sizeof(options->check_land)))
    return options->check_land != 0;
  return true;
}

bool SegmentSafetyOptionAllowGshhsFallback(
    const SegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(
          options, offsetof(SegmentSafetyOptions, allow_gshhs_fallback),
          sizeof(options->allow_gshhs_fallback)))
    return options->allow_gshhs_fallback != 0;
  return true;
}

bool SegmentSafetyOptionCheckDepth(const SegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(options,
                              offsetof(SegmentSafetyOptions, check_depth),
                              sizeof(options->check_depth)))
    return options->check_depth != 0;
  return false;
}

double SegmentSafetyOptionMinimumDepthM(const SegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(options,
                              offsetof(SegmentSafetyOptions, minimum_depth_m),
                              sizeof(options->minimum_depth_m)))
    return wxMax(0.0, options->minimum_depth_m);
  return 0.0;
}

bool SegmentSafetyOptionForceAuthoritativeFineValidation(
    const SegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(
          options,
          offsetof(SegmentSafetyOptions, force_authoritative_fine_validation),
          sizeof(options->force_authoritative_fine_validation)))
    return options->force_authoritative_fine_validation != 0;
  return false;
}

bool IsSegmentSafetyLandObject(const char* feature_name) {
  return feature_name && !strncmp(feature_name, "LNDARE", 6);
}

bool IsCm93Chart(ChartBase* chart) {
  return chart && chart->GetChartType() == CHART_TYPE_CM93COMP;
}

std::string SegmentSafetyCacheKey(int db_index, bool cm93, double lat,
                                  double lon) {
  if (!cm93) return std::to_string(db_index);

  double lat_bucket = floor(lat * 4.0) / 4.0;
  double lon_bucket = floor(lon * 4.0) / 4.0;
  return wxString::Format("%d:%.2f:%.2f", db_index, lat_bucket, lon_bucket)
      .ToStdString();
}

ViewPort SegmentSafetyViewPortAt(double lat, double lon) {
  ViewPort vp;
  ChartCanvas* canvas = gFrame ? gFrame->GetFocusCanvas() : NULL;
  if (canvas) vp = canvas->GetVP();

  vp.clat = lat;
  vp.clon = lon;
  if (vp.pix_width <= 0) vp.pix_width = 1024;
  if (vp.pix_height <= 0) vp.pix_height = 768;
  if (vp.view_scale_ppm <= 0.0) vp.view_scale_ppm = 1.0 / 1852.0;
  if (vp.chart_scale <= 0.0) vp.chart_scale = 100000;
  if (vp.ref_scale <= 0) vp.ref_scale = vp.chart_scale;
  vp.SetBoxes();
  return vp;
}

ViewPort SegmentSafetyHighestDetailViewPortAt(double lat, double lon) {
  ViewPort vp = SegmentSafetyViewPortAt(lat, lon);
  // CM93 composite chart selection is viewport-scale dependent.  A safety
  // classification must not depend on the user's current zoom level: request
  // the highest local CM93 scale and let cm93compchart fall back only when
  // that cell scale is unavailable at this position.
  vp.view_scale_ppm = 1.0;
  vp.chart_scale = 5000.0;
  vp.ref_scale = 5000;
  vp.SetBoxes();
  return vp;
}

double SegmentSafetyNormalizeBearing(double bearing) {
  while (bearing < 0.0) bearing += 360.0;
  while (bearing >= 360.0) bearing -= 360.0;
  return bearing;
}

bool ChartPointIsLand(s57chart* chart, double lat, double lon, ViewPort& vp) {
  if (!chart) return false;

  ListOfObjRazRules* rule_list =
      chart->GetObjRuleListAtLatLon(lat, lon, 0.0, &vp, MASK_AREA);
  if (!rule_list) return false;

  bool is_land = false;
  for (ListOfObjRazRules::Node* node = rule_list->GetFirst(); node;
       node = node->GetNext()) {
    ObjRazRules* rule = node->GetData();
    if (rule && rule->obj &&
        IsSegmentSafetyLandObject(rule->obj->FeatureName)) {
      is_land = true;
      break;
    }
  }

  rule_list->Clear();
  delete rule_list;
  return is_land;
}

const char* SegmentSafetyPrimitiveName(GeoPrim_t primitive) {
  switch (primitive) {
    case GEO_POINT:
      return "point";
    case GEO_LINE:
      return "line";
    case GEO_AREA:
      return "area";
    case GEO_META:
      return "meta";
    case GEO_PRIM:
      return "prim";
    default:
      return "unknown";
  }
}

wxString SegmentSafetyObjectAttr(S57Obj* obj, const char* attr) {
  if (!obj || !attr) return wxString();
  wxString value = obj->GetAttrValueAsString(attr);
  value.Replace("\"", "'");
  value.Replace(";", ",");
  return value;
}

wxString SegmentSafetyRuleSummary(ObjRazRules* rule) {
  if (!rule || !rule->obj) return wxString();

  S57Obj* obj = rule->obj;
  wxString summary =
      wxString::Format("%s/%s", obj->FeatureName,
                       SegmentSafetyPrimitiveName(obj->Primitive_type));
  if (rule->LUP) {
    summary += wxString::Format("/TNAM=%d/DPRI=%c/DISC=%c", rule->LUP->TNAM,
                                rule->LUP->DPRI, rule->LUP->DISC);
    if (!rule->LUP->INST.empty()) {
      wxString inst = rule->LUP->INST.Left(80);
      inst.Replace("\"", "'");
      inst.Replace(";", ",");
      summary += wxString::Format("/INST=%s", inst);
    }
  }

  const char* attrs[] = {"DRVAL1", "DRVAL2", "VALSOU", "VALDCO", "WATLEV",
                         "CATWAT", "CATWRK", "CATOBS", "EXPSOU"};
  for (size_t i = 0; i < WXSIZEOF(attrs); ++i) {
    wxString value = SegmentSafetyObjectAttr(obj, attrs[i]);
    if (!value.empty()) summary += wxString::Format("/%s=%s", attrs[i], value);
  }

  return summary;
}

bool SegmentSafetyParseDouble(wxString value, double* out) {
  value.Trim(true);
  value.Trim(false);
  if (value.empty()) return false;
  double parsed = 0.0;
  if (!value.ToDouble(&parsed)) return false;
  if (out) *out = parsed;
  return true;
}

bool SegmentSafetyRuleDepthMinM(ObjRazRules* rule, double* depth_m) {
  if (!rule || !rule->obj ||
      (strncmp(rule->obj->FeatureName, "DEPARE", 6) &&
       strncmp(rule->obj->FeatureName, "DRGARE", 6)))
    return false;
  return SegmentSafetyParseDouble(rule->obj->GetAttrValueAsString("DRVAL1"),
                                  depth_m);
}

bool SegmentSafetyWaterLevelIs(const wxString& value, int code) {
  wxString normalized = value;
  normalized.Trim(true);
  normalized.Trim(false);
  long parsed = -1;
  if (normalized.ToLong(&parsed)) return parsed == code;
  return normalized.Find(wxString::Format("(%d)", code)) != wxNOT_FOUND;
}

bool SegmentSafetyIsIsolatedDanger(const char* feature_name) {
  return feature_name && (!strncmp(feature_name, "WRECKS", 6) ||
                          !strncmp(feature_name, "UWTROC", 6) ||
                          !strncmp(feature_name, "OBSTRN", 6));
}

bool SegmentSafetyRuleIsDrying(ObjRazRules* rule) {
  if (!rule || !rule->obj) return false;
  wxString watlev = rule->obj->GetAttrValueAsString("WATLEV");
  return SegmentSafetyWaterLevelIs(watlev, 4) ||
         SegmentSafetyWaterLevelIs(watlev, 5);
}

bool SegmentSafetyRuleIsAlwaysDry(ObjRazRules* rule) {
  if (!rule || !rule->obj) return false;
  return SegmentSafetyWaterLevelIs(rule->obj->GetAttrValueAsString("WATLEV"),
                                   2);
}

bool SegmentSafetyRuleDangerDepthM(ObjRazRules* rule, double* depth_m,
                                   bool* unknown_depth) {
  if (unknown_depth) *unknown_depth = false;
  if (!rule || !rule->obj ||
      !SegmentSafetyIsIsolatedDanger(rule->obj->FeatureName))
    return false;
  if (SegmentSafetyParseDouble(rule->obj->GetAttrValueAsString("VALSOU"),
                               depth_m))
    return true;
  if (unknown_depth) *unknown_depth = true;
  return false;
}

int SegmentSafetyPluginAttributeIndex(PI_S57Obj* obj, const char* attr) {
  if (!obj || !attr || !obj->att_array || obj->n_attr <= 0) return -1;
  const char* current = obj->att_array;
  for (int index = 0; index < obj->n_attr; ++index, current += 6) {
    if (!strncmp(current, attr, 6)) return index;
  }
  return -1;
}

wxString SegmentSafetyPluginObjectAttr(PI_S57Obj* obj, const char* attr) {
  const int index = SegmentSafetyPluginAttributeIndex(obj, attr);
  if (index < 0 || !obj->attVal || index >= (int)obj->attVal->GetCount())
    return wxString();
  S57attVal* value = obj->attVal->Item(index);
  if (!value || !value->value) return wxString();
  switch (value->valType) {
    case OGR_STR:
      return wxString(static_cast<const char*>(value->value), wxConvUTF8);
    case OGR_REAL:
      return wxString::Format("%.12g", *static_cast<double*>(value->value));
    case OGR_INT:
      return wxString::Format("%d", *static_cast<int*>(value->value));
    default:
      return wxString();
  }
}

wxString SegmentSafetyPluginObjectSummary(PI_S57Obj* obj) {
  if (!obj) return wxString();
  wxString summary = wxString::Format(
      "%s/%s", obj->FeatureName,
      SegmentSafetyPrimitiveName(static_cast<GeoPrim_t>(obj->Primitive_type)));
  const char* attrs[] = {"DRVAL1", "DRVAL2", "VALSOU", "WATLEV",
                         "CATWRK", "CATOBS", "EXPSOU"};
  for (size_t i = 0; i < WXSIZEOF(attrs); ++i) {
    wxString value = SegmentSafetyPluginObjectAttr(obj, attrs[i]);
    value.Replace("\"", "'");
    value.Replace(";", ",");
    if (!value.empty()) summary += wxString::Format("/%s=%s", attrs[i], value);
  }
  return summary;
}

bool SegmentSafetyPluginObjectIsDrying(PI_S57Obj* obj) {
  if (!obj) return false;
  const wxString watlev = SegmentSafetyPluginObjectAttr(obj, "WATLEV");
  return SegmentSafetyWaterLevelIs(watlev, 4) ||
         SegmentSafetyWaterLevelIs(watlev, 5);
}

bool SegmentSafetyPluginObjectIsAlwaysDry(PI_S57Obj* obj) {
  return obj && SegmentSafetyWaterLevelIs(
                    SegmentSafetyPluginObjectAttr(obj, "WATLEV"), 2);
}

bool SegmentSafetyPluginObjectDepthM(PI_S57Obj* obj, double* depth_m,
                                     wxString* source_attribute,
                                     bool* unknown_danger_depth) {
  if (unknown_danger_depth) *unknown_danger_depth = false;
  if (!obj) return false;
  if (!strncmp(obj->FeatureName, "DEPARE", 6) ||
      !strncmp(obj->FeatureName, "DRGARE", 6)) {
    if (SegmentSafetyParseDouble(SegmentSafetyPluginObjectAttr(obj, "DRVAL1"),
                                 depth_m)) {
      if (source_attribute)
        *source_attribute = wxString::Format("%s/DRVAL1", obj->FeatureName);
      return true;
    }
    return false;
  }
  if (SegmentSafetyIsIsolatedDanger(obj->FeatureName)) {
    if (SegmentSafetyParseDouble(SegmentSafetyPluginObjectAttr(obj, "VALSOU"),
                                 depth_m)) {
      if (source_attribute)
        *source_attribute = wxString::Format("%s/VALSOU", obj->FeatureName);
      return true;
    }
    if (unknown_danger_depth) *unknown_danger_depth = true;
  }
  return false;
}

bool IsSupportedSegmentSafetyPluginChart(ChartBase* chart) {
  ChartPlugInWrapper* wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
  if (!wrapper || chart->GetChartFamily() != CHART_FAMILY_VECTOR) return false;
  PlugInChartBase* plugin_chart = wrapper->GetPlugInChart();
  return dynamic_cast<PlugInChartBaseGL*>(plugin_chart) != NULL ||
         dynamic_cast<PlugInChartBaseExtended*>(plugin_chart) != NULL;
}

s57chart* GetSegmentSafetyChartAtPoint(ChartCanvas* canvas, double lat,
                                       double lon, ViewPort& vp,
                                       SegmentSafetySource* source) {
  if (!canvas) return NULL;

  wxPoint point;
  if (!canvas->GetCanvasPointPixVP(vp, lat, lon, &point)) return NULL;

  ChartBase* chart = NULL;
  if (canvas->GetQuiltMode() && canvas->m_pQuilt) {
    chart = canvas->m_pQuilt->GetChartAtPix(vp, point);
    if (!chart) chart = canvas->m_pQuilt->GetOverlayChartAtPix(vp, point);
  } else {
    chart = canvas->m_singleChart;
  }

  s57chart* s57 = dynamic_cast<s57chart*>(chart);
  if (s57 && source) {
    *source = IsCm93Chart(chart) ? kSourceCm93 : kSourceVectorChart;
  }
  return s57;
}

bool ChartSegmentPointSamplesHitLand(double lat1, double lon1, double lat2,
                                     double lon2, double safety_margin_nm,
                                     SegmentSafetyResult* result,
                                     int* chart_sample_count,
                                     int* total_sample_count) {
  ChartCanvas* canvas = gFrame && gFrame->GetFocusCanvas()
                            ? gFrame->GetFocusCanvas()
                            : (gFrame ? gFrame->GetPrimaryCanvas() : NULL);
  if (!canvas) return false;

  ViewPort vp = canvas->GetVP();
  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);

  const int max_samples = 256;
  int samples = wxMax(2, wxMin(max_samples, (int)ceil(dist_nm / 0.1) + 1));
  if (total_sample_count) *total_sample_count = samples;

  for (int i = 0; i < samples; ++i) {
    double sample_dist = samples == 1 ? 0.0 : dist_nm * i / (samples - 1);
    double lat = lat1;
    double lon = lon1;
    if (sample_dist > 0.0)
      ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);

    SegmentSafetySource source = kSourceNone;
    s57chart* chart =
        GetSegmentSafetyChartAtPoint(canvas, lat, lon, vp, &source);
    if (!chart) continue;

    if (chart_sample_count) ++*chart_sample_count;
    if (GetSegmentSafetySource(result) == kSourceNone)
      SetSegmentSafetySource(result, source);

    if (ChartPointIsLand(chart, lat, lon, vp)) {
      if (result) {
        SetSegmentSafetyStatus(result, kCrossesLand);
        SetSegmentSafetySource(result, source);
        SetSegmentSafetyMessage(result, "segment intersects chart land area");
      }
      return true;
    }

    if (safety_margin_nm > 0.0) {
      double left_lat, left_lon, right_lat, right_lon;
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing - 90.0),
               safety_margin_nm, &left_lat, &left_lon);
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing + 90.0),
               safety_margin_nm, &right_lat, &right_lon);
      if (ChartPointIsLand(chart, left_lat, left_lon, vp) ||
          ChartPointIsLand(chart, right_lat, right_lon, vp)) {
        if (result) {
          SetSegmentSafetyStatus(result, kWithinLandMargin);
          SetSegmentSafetySource(result, source);
          SetSegmentSafetyMessage(result,
                                  "segment is within chart land safety margin");
        }
        return true;
      }
    }
  }

  return false;
}

std::map<std::string, CachedChartLandGeometry> s_segment_safety_land_cache;

std::shared_ptr<const SegmentSafetyHazardSnapshot>
    s_segment_safety_hazard_snapshot;
long s_segment_safety_snapshot_queries = 0;
long s_segment_safety_snapshot_safe = 0;
long s_segment_safety_snapshot_unknown = 0;
long s_segment_safety_snapshot_shadow_disagreements = 0;

std::map<std::string, CachedPointSafetyClassification>
    s_segment_safety_point_cache;

// Keep the hot in-memory working set bounded, but retain a larger certified
// disk-backed history so repeated routes do not rebuild recently used waters.
// Each entry certifies a 0.2-degree coarse cell for one exact margin/depth
// policy.  Keep the expanding regional proof cache bounded on disk.
long s_segment_safety_grid_cache_evictions = 0;

std::map<std::string, CachedPointSafetyGridTile> s_segment_safety_grid_cache;
std::set<std::string> s_segment_safety_pinned_grid_keys;
std::map<std::string, CachedPointSafetyGridTile>
    s_segment_safety_persistent_base_tile_cache;
SegmentSafetyTileCacheCallbacks s_segment_safety_external_tile_cache = {};

uint16_t SegmentSafetyPointHazardFlags(SegmentSafetyPointClass point_class) {
  switch (point_class) {
    case kPointLand:
      return kHazardLand;
    case kPointDrying:
      return kHazardDrying;
    case kPointWater:
      return kHazardNone;
    case kPointNoData:
    default:
      return kHazardNoChart;
  }
}

std::map<std::string, CachedSegmentSafetyRouteMaskTile>
    s_segment_safety_route_mask_cache;
std::set<std::string> s_segment_safety_pinned_route_mask_keys;

std::map<std::string, SegmentSafetyRouteMaskRequest>
    s_segment_safety_pending_route_mask_requests;
std::set<std::string> s_segment_safety_inflight_route_mask_requests;

std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>
    s_segment_safety_coarse_route_mask_cache;

std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>
    s_segment_safety_persistent_certified_safe_cache;
bool s_segment_safety_persistent_cache_enabled = false;
bool s_segment_safety_persistent_cache_loaded = false;
bool s_segment_safety_persistent_cache_dirty = false;
bool s_segment_safety_persistent_base_tiles_loaded = false;
bool s_segment_safety_persistent_base_tiles_dirty = false;
wxString s_segment_safety_chart_identity;
wxString s_segment_safety_chart_catalog_identity;
long s_segment_safety_persistent_entries_loaded = 0;
long s_segment_safety_persistent_entries_saved = 0;
long s_segment_safety_persistent_entries_used = 0;
long s_segment_safety_persistent_entries_ignored = 0;
long s_segment_safety_persistent_stale_ignored = 0;
long s_segment_safety_persistent_malformed_ignored = 0;
long s_segment_safety_persistent_entries_stored = 0;
long s_segment_safety_persistent_base_tiles_loaded_count = 0;
long s_segment_safety_persistent_base_tiles_saved = 0;
long s_segment_safety_persistent_base_tiles_used = 0;
long s_segment_safety_persistent_base_tiles_ignored = 0;
long s_segment_safety_persistent_tiles_since_checkpoint = 0;

int SegmentSafetyCurrentGroupIndex();
std::string SegmentSafetyGridTileKeyForIndices(long lat_tile, long lon_tile);

std::string SegmentSafetyRouteMaskKey(long lat_tile, long lon_tile,
                                      double safety_margin_nm, bool check_depth,
                                      double minimum_depth_m) {
  long margin_mm = lround(wxMax(0.0, safety_margin_nm) * 1000.0);
  long depth_cm = check_depth ? lround(wxMax(0.0, minimum_depth_m) * 100.0) : 0;
  char buf[160];
  snprintf(buf, sizeof(buf), "%d:%ld:%ld:r%.8f:m%ld:d%d:%ld",
           SegmentSafetyCurrentGroupIndex(), lat_tile, lon_tile,
           kGridResolutionDegrees, margin_mm, check_depth ? 1 : 0, depth_cm);
  return std::string(buf);
}

std::string SegmentSafetyCoarseRouteMaskKey(long lat_cell, long lon_cell,
                                            double safety_margin_nm,
                                            bool check_depth,
                                            double minimum_depth_m) {
  long margin_mm = lround(wxMax(0.0, safety_margin_nm) * 1000.0);
  long depth_cm = check_depth ? lround(wxMax(0.0, minimum_depth_m) * 100.0) : 0;
  char buf[160];
  snprintf(buf, sizeof(buf), "%d:%ld:%ld:cr%.8f:f%d:m%ld:d%d:%ld",
           SegmentSafetyCurrentGroupIndex(), lat_cell, lon_cell,
           kGridTileDegrees * kCoarseRouteMaskFactor, kCoarseRouteMaskFactor,
           margin_mm, check_depth ? 1 : 0, depth_cm);
  return std::string(buf);
}

std::map<std::string, CachedSegmentSafetyResult> s_segment_safety_segment_cache;

long s_segment_safety_chart_hit_logs = 0;
wxMutex s_segment_safety_cache_mutex;
long s_segment_safety_worker_tile_miss_logs = 0;
long s_segment_safety_query_logs = 0;

int SegmentSafetyMarginTileRadius(double safety_margin_nm, double max_abs_lat) {
  if (safety_margin_nm <= 0.0) return 0;
  double limited_lat = wxMin(89.9, max_abs_lat);
  double cos_lat = fabs(cos(limited_lat * M_PI / 180.0));
  double tile_width_nm = kGridTileDegrees * 60.0 * wxMax(0.001, cos_lat);
  double tile_height_nm = kGridTileDegrees * 60.0;
  return (int)ceil(safety_margin_nm /
                   wxMax(0.001, wxMin(tile_width_nm, tile_height_nm)));
}

void PinSegmentSafetyRouteMaskTiles(
    const std::set<std::pair<long, long> >& tiles, double safety_margin_nm,
    bool check_depth, double minimum_depth_m) {
  if (tiles.empty()) return;

  double max_abs_lat = 0.0;
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    max_abs_lat = wxMax(max_abs_lat, fabs(it->first * kGridTileDegrees));
  }
  int margin_radius =
      SegmentSafetyMarginTileRadius(safety_margin_nm, max_abs_lat);

  wxMutexLocker lock(s_segment_safety_cache_mutex);
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    s_segment_safety_pinned_route_mask_keys.insert(SegmentSafetyRouteMaskKey(
        it->first, it->second, safety_margin_nm, check_depth, minimum_depth_m));
    for (int lat_offset = -margin_radius; lat_offset <= margin_radius;
         ++lat_offset) {
      for (int lon_offset = -margin_radius; lon_offset <= margin_radius;
           ++lon_offset) {
        s_segment_safety_pinned_grid_keys.insert(
            SegmentSafetyGridTileKeyForIndices(it->first + lat_offset,
                                               it->second + lon_offset));
      }
    }
  }
}

void PinSegmentSafetyRouteMaskEnvelope(long min_lat_tile, long max_lat_tile,
                                       long min_lon_tile, long max_lon_tile,
                                       double safety_margin_nm,
                                       bool check_depth,
                                       double minimum_depth_m) {
  std::set<std::pair<long, long> > tiles;
  for (long lat_tile = min_lat_tile; lat_tile <= max_lat_tile; ++lat_tile)
    for (long lon_tile = min_lon_tile; lon_tile <= max_lon_tile; ++lon_tile)
      tiles.insert(std::make_pair(lat_tile, lon_tile));
  PinSegmentSafetyRouteMaskTiles(tiles, safety_margin_nm, check_depth,
                                 minimum_depth_m);
}

void ApplySegmentSafetyStats(SegmentSafetyResult* result,
                             const SegmentSafetyCoreStats& stats) {
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, chart_stack_entries),
      stats.chart_stack_entries);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, candidate_chart_count),
      stats.candidate_chart_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, raster_chart_count),
      stats.raster_chart_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, unsupported_chart_count),
      stats.unsupported_chart_count);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, s57_chart_count),
                                stats.s57_chart_count);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, land_ring_count),
                                stats.land_ring_count);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, bbox_ring_tests),
                                stats.bbox_ring_tests);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, edge_tests), stats.edge_tests);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, cache_build_ms),
                                stats.cache_build_ms);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, chart_select_ms),
                                stats.chart_select_ms);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, geometry_check_ms),
      stats.geometry_check_ms);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, point_cache_hits),
                                stats.point_cache_hits);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, point_cache_misses),
      stats.point_cache_misses);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, grid_cache_hits),
                                stats.grid_cache_hits);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, grid_cache_misses),
      stats.grid_cache_misses);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, grid_build_ms),
                                stats.grid_build_ms);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, grid_cells_total),
                                stats.grid_cells_total);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, grid_cells_land),
                                stats.grid_cells_land);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, grid_cells_water),
                                stats.grid_cells_water);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, grid_cells_drying),
      stats.grid_cells_drying);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, grid_cells_unknown),
      stats.grid_cells_unknown);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, grid_lookups), stats.grid_lookups);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, grid_lookup_ms),
                                stats.grid_lookup_ms);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, segment_sample_count),
      stats.segment_sample_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, water_tile_shortcuts),
      stats.water_tile_shortcuts);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, segment_cache_hits),
      stats.segment_cache_hits);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, segment_cache_misses),
      stats.segment_cache_misses);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, segment_cache_stores),
      stats.segment_cache_stores);
  size_t grid_cache_size = 0;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    grid_cache_size = s_segment_safety_grid_cache.size();
  }
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(SegmentSafetyResult, grid_cache_size),
                                (int)grid_cache_size);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, grid_cache_evictions),
      (int)s_segment_safety_grid_cache_evictions);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, unexpected_tile_builds),
      stats.unexpected_tile_builds);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, unexpected_lat_tile),
      stats.unexpected_lat_tile);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(SegmentSafetyResult, unexpected_lon_tile),
      stats.unexpected_lon_tile);
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, unexpected_tile_min_lon),
          sizeof(result->unexpected_tile_min_lon))) {
    result->unexpected_tile_min_lat = stats.unexpected_tile_min_lat;
    result->unexpected_tile_min_lon = stats.unexpected_tile_min_lon;
  }
}

SegmentSafetyDiagnosticReason SegmentSafetyUnavailableReason(
    const SegmentSafetyCoreStats& stats) {
  if (stats.no_chart_database) return kDiagnosticNoChartDatabase;
  if (stats.chart_load_failed) return kDiagnosticChartLoadFailed;
  if (stats.candidate_chart_count == 0) {
    if (stats.raster_chart_count > 0 && stats.unsupported_chart_count == 0)
      return kDiagnosticRasterOnly;
    if (stats.unsupported_chart_count > 0 && stats.raster_chart_count == 0)
      return kDiagnosticUnsupportedChartType;
    return kDiagnosticNoCandidateChart;
  }
  if (stats.zero_land_geometry) return kDiagnosticNoLandAreaGeometry;
  return kDiagnosticNoCandidateChart;
}

const char* SegmentSafetyUnavailableMessage(
    SegmentSafetyDiagnosticReason reason) {
  switch (reason) {
    case kDiagnosticNoChartDatabase:
      return "no chart database available for chart land checks";
    case kDiagnosticNoCandidateChart:
      return "no candidate vector chart found for segment";
    case kDiagnosticRasterOnly:
      return "only raster chart coverage found for segment";
    case kDiagnosticUnsupportedChartType:
      return "only unsupported chart types found for segment";
    case kDiagnosticChartLoadFailed:
      return "candidate chart could not be loaded for segment safety";
    case kDiagnosticNoLandAreaGeometry:
      return "candidate vector chart has no LNDARE land geometry";
    default:
      return "chart land geometry unavailable for segment";
  }
}

double SegmentSafetyDegToRad(double degrees) {
  return degrees * 3.14159265358979323846 / 180.0;
}

double SegmentSafetyCross(const wxPoint2DDouble& a, const wxPoint2DDouble& b,
                          const wxPoint2DDouble& c) {
  return (b.m_x - a.m_x) * (c.m_y - a.m_y) - (b.m_y - a.m_y) * (c.m_x - a.m_x);
}

bool SegmentSafetyBBoxIntersects(const SegmentSafetyBBox& a,
                                 const SegmentSafetyBBox& b) {
  return !(a.max_lat < b.min_lat || a.min_lat > b.max_lat ||
           a.max_lon < b.min_lon || a.min_lon > b.max_lon);
}

SegmentSafetyBBox SegmentSafetyRingBBox(
    const std::vector<wxPoint2DDouble>& points) {
  SegmentSafetyBBox box;
  box.min_lat = box.max_lat = points.empty() ? 0.0 : points[0].m_y;
  box.min_lon = box.max_lon = points.empty() ? 0.0 : points[0].m_x;
  for (size_t i = 1; i < points.size(); ++i) {
    box.min_lat = wxMin(box.min_lat, points[i].m_y);
    box.max_lat = wxMax(box.max_lat, points[i].m_y);
    box.min_lon = wxMin(box.min_lon, points[i].m_x);
    box.max_lon = wxMax(box.max_lon, points[i].m_x);
  }
  return box;
}

SegmentSafetyBBox SegmentSafetySegmentBBox(double lat1, double lon1,
                                           double lat2, double lon2,
                                           double margin_nm) {
  double margin_lat = margin_nm / 60.0;
  double mid_lat = (lat1 + lat2) / 2.0;
  double cos_lat = wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mid_lat))));
  double margin_lon = margin_nm / (60.0 * cos_lat);

  SegmentSafetyBBox box;
  box.min_lat = wxMin(lat1, lat2) - margin_lat;
  box.max_lat = wxMax(lat1, lat2) + margin_lat;
  box.min_lon = wxMin(lon1, lon2) - margin_lon;
  box.max_lon = wxMax(lon1, lon2) + margin_lon;
  return box;
}

bool SegmentSafetyPointInRing(double lat, double lon,
                              const std::vector<wxPoint2DDouble>& ring) {
  bool inside = false;
  size_t count = ring.size();
  if (count < 3) return false;

  for (size_t i = 0, j = count - 1; i < count; j = i++) {
    double xi = ring[i].m_x, yi = ring[i].m_y;
    double xj = ring[j].m_x, yj = ring[j].m_y;
    bool intersect = ((yi > lat) != (yj > lat)) &&
                     (lon < (xj - xi) * (lat - yi) / (yj - yi) + xi);
    if (intersect) inside = !inside;
  }
  return inside;
}

bool SegmentSafetyOnSegment(const wxPoint2DDouble& a, const wxPoint2DDouble& b,
                            const wxPoint2DDouble& p) {
  const double eps = 1e-10;
  return fabs(SegmentSafetyCross(a, b, p)) < eps &&
         p.m_x >= wxMin(a.m_x, b.m_x) - eps &&
         p.m_x <= wxMax(a.m_x, b.m_x) + eps &&
         p.m_y >= wxMin(a.m_y, b.m_y) - eps &&
         p.m_y <= wxMax(a.m_y, b.m_y) + eps;
}

bool SegmentSafetySegmentsIntersect(const wxPoint2DDouble& a,
                                    const wxPoint2DDouble& b,
                                    const wxPoint2DDouble& c,
                                    const wxPoint2DDouble& d) {
  double c1 = SegmentSafetyCross(a, b, c);
  double c2 = SegmentSafetyCross(a, b, d);
  double c3 = SegmentSafetyCross(c, d, a);
  double c4 = SegmentSafetyCross(c, d, b);

  if (((c1 > 0 && c2 < 0) || (c1 < 0 && c2 > 0)) &&
      ((c3 > 0 && c4 < 0) || (c3 < 0 && c4 > 0)))
    return true;

  return SegmentSafetyOnSegment(a, b, c) || SegmentSafetyOnSegment(a, b, d) ||
         SegmentSafetyOnSegment(c, d, a) || SegmentSafetyOnSegment(c, d, b);
}

double SegmentSafetyPointSegmentDistanceNm(const wxPoint2DDouble& p,
                                           const wxPoint2DDouble& a,
                                           const wxPoint2DDouble& b,
                                           double mean_lat) {
  double cos_lat = wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mean_lat))));
  double px = p.m_x * 60.0 * cos_lat;
  double py = p.m_y * 60.0;
  double ax = a.m_x * 60.0 * cos_lat;
  double ay = a.m_y * 60.0;
  double bx = b.m_x * 60.0 * cos_lat;
  double by = b.m_y * 60.0;

  double dx = bx - ax;
  double dy = by - ay;
  double denom = dx * dx + dy * dy;
  double t = denom > 0.0 ? ((px - ax) * dx + (py - ay) * dy) / denom : 0.0;
  t = wxMax(0.0, wxMin(1.0, t));
  double cx = ax + t * dx;
  double cy = ay + t * dy;
  return sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
}

double SegmentSafetySegmentDistanceNm(const wxPoint2DDouble& a,
                                      const wxPoint2DDouble& b,
                                      const wxPoint2DDouble& c,
                                      const wxPoint2DDouble& d) {
  if (SegmentSafetySegmentsIntersect(a, b, c, d)) return 0.0;
  double mean_lat = (a.m_y + b.m_y + c.m_y + d.m_y) / 4.0;
  return wxMin(wxMin(SegmentSafetyPointSegmentDistanceNm(a, c, d, mean_lat),
                     SegmentSafetyPointSegmentDistanceNm(b, c, d, mean_lat)),
               wxMin(SegmentSafetyPointSegmentDistanceNm(c, a, b, mean_lat),
                     SegmentSafetyPointSegmentDistanceNm(d, a, b, mean_lat)));
}

bool SegmentSafetyPointInsideBBox(const wxPoint2DDouble& point,
                                  const SegmentSafetyBBox& box) {
  return point.m_y >= box.min_lat && point.m_y <= box.max_lat &&
         point.m_x >= box.min_lon && point.m_x <= box.max_lon;
}

bool SegmentSafetyRingContainsBBox(const CachedLandRing& ring,
                                   const SegmentSafetyBBox& box) {
  const wxPoint2DDouble corners[] = {wxPoint2DDouble(box.min_lon, box.min_lat),
                                     wxPoint2DDouble(box.max_lon, box.min_lat),
                                     wxPoint2DDouble(box.max_lon, box.max_lat),
                                     wxPoint2DDouble(box.min_lon, box.max_lat)};
  for (size_t i = 0; i < WXSIZEOF(corners); ++i)
    if (!SegmentSafetyPointInRing(corners[i].m_y, corners[i].m_x, ring.points))
      return false;

  // Corners alone are insufficient for a concave coverage polygon.  If its
  // boundary enters the rectangle, the entire buffered segment cannot be
  // certified as covered.
  for (size_t i = 0; i < ring.points.size(); ++i) {
    const wxPoint2DDouble& a = ring.points[i];
    const wxPoint2DDouble& b = ring.points[(i + 1) % ring.points.size()];
    if (SegmentSafetyPointInsideBBox(a, box) ||
        SegmentSafetyPointInsideBBox(b, box))
      return false;
    for (size_t edge = 0; edge < WXSIZEOF(corners); ++edge)
      if (SegmentSafetySegmentsIntersect(
              a, b, corners[edge], corners[(edge + 1) % WXSIZEOF(corners)]))
        return false;
  }
  return true;
}

bool SegmentSafetyChartFullyCoversBBox(const SegmentSafetySnapshotChart& chart,
                                       const SegmentSafetyBBox& box) {
  if (!SegmentSafetyBBoxIntersects(chart.bbox, box)) return false;
  bool covered = false;
  for (std::vector<CachedLandRing>::const_iterator it = chart.coverage.begin();
       it != chart.coverage.end(); ++it) {
    if (SegmentSafetyRingContainsBBox(*it, box)) {
      covered = true;
      break;
    }
  }
  if (!covered) return false;

  // Any possible contact with a no-coverage polygon invalidates a positive
  // certificate.  Falling back is intentionally more conservative than
  // attempting to infer hole topology here.
  for (std::vector<CachedLandRing>::const_iterator it =
           chart.no_coverage.begin();
       it != chart.no_coverage.end(); ++it)
    if (SegmentSafetyBBoxIntersects(it->bbox, box)) return false;
  return true;
}

bool SegmentSafetyChartHazardMayAffectBBox(
    const SegmentSafetySnapshotChart& chart, const SegmentSafetyBBox& box) {
  const wxPoint2DDouble corners[] = {wxPoint2DDouble(box.min_lon, box.min_lat),
                                     wxPoint2DDouble(box.max_lon, box.min_lat),
                                     wxPoint2DDouble(box.max_lon, box.max_lat),
                                     wxPoint2DDouble(box.min_lon, box.max_lat)};
  for (std::vector<CachedLandRing>::const_iterator it = chart.hazards.begin();
       it != chart.hazards.end(); ++it) {
    if (!SegmentSafetyBBoxIntersects(it->bbox, box)) continue;

    // Resolve broad coastline bounding-box overlap against the actual ring.
    // Contact and boundary ambiguity both remain hazardous; only a proven
    // disjoint ring permits an open-sea certificate.
    for (size_t corner = 0; corner < WXSIZEOF(corners); ++corner)
      if (SegmentSafetyPointInRing(corners[corner].m_y, corners[corner].m_x,
                                   it->points))
        return true;
    for (std::vector<wxPoint2DDouble>::const_iterator point =
             it->points.begin();
         point != it->points.end(); ++point)
      if (SegmentSafetyPointInsideBBox(*point, box)) return true;
    for (size_t edge = 0; edge < it->points.size(); ++edge) {
      const wxPoint2DDouble& a = it->points[edge];
      const wxPoint2DDouble& b = it->points[(edge + 1) % it->points.size()];
      for (size_t box_edge = 0; box_edge < WXSIZEOF(corners); ++box_edge)
        if (SegmentSafetySegmentsIntersect(
                a, b, corners[box_edge],
                corners[(box_edge + 1) % WXSIZEOF(corners)]))
          return true;
    }
  }
  return false;
}

bool SegmentSafetySnapshotChartPrecedes(const SegmentSafetySnapshotChart& a,
                                        const SegmentSafetySnapshotChart& b) {
  if (a.native_scale != b.native_scale) return a.native_scale < b.native_scale;
  if (a.edition_date != b.edition_date) return a.edition_date > b.edition_date;
  if (a.file_time != b.file_time) return a.file_time > b.file_time;
  if (a.chart_path != b.chart_path) return a.chart_path < b.chart_path;
  return a.db_index < b.db_index;
}

SegmentSafetySnapshotDecision ClassifySegmentSafetySnapshotBBox(
    const SegmentSafetyHazardSnapshot& snapshot,
    const SegmentSafetyBBox& query) {
  const SegmentSafetySnapshotChart* best = NULL;
  std::vector<const SegmentSafetySnapshotChart*> covering;
  for (std::vector<SegmentSafetySnapshotChart>::const_iterator it =
           snapshot.charts.begin();
       it != snapshot.charts.end(); ++it) {
    if (!SegmentSafetyChartFullyCoversBBox(*it, query)) continue;
    covering.push_back(&*it);
    if (!best || SegmentSafetySnapshotChartPrecedes(*it, *best)) best = &*it;
  }
  if (!best || !best->vector_supported) return kSnapshotUnknown;

  // A hazard on any overlapping supported chart represents disagreement.
  // Do not fast-reject and potentially lose a valid passage; fall back to the
  // existing best-chart classifier to resolve it.
  for (std::vector<const SegmentSafetySnapshotChart*>::const_iterator it =
           covering.begin();
       it != covering.end(); ++it) {
    if (!(*it)->vector_supported ||
        SegmentSafetyChartHazardMayAffectBBox(**it, query))
      return kSnapshotUnknown;
  }
  return kSnapshotSafe;
}

bool SegmentSafetyBBoxInsideCertifiedCell(
    const SegmentSafetyBBox& query, double cell_degrees,
    const std::set<std::pair<long, long> >& certified) {
  const long min_lat_cell = floor(query.min_lat / cell_degrees);
  const long max_lat_cell = floor(query.max_lat / cell_degrees);
  const long min_lon_cell = floor(query.min_lon / cell_degrees);
  const long max_lon_cell = floor(query.max_lon / cell_degrees);
  return min_lat_cell == max_lat_cell && min_lon_cell == max_lon_cell &&
         certified.find(std::make_pair(min_lat_cell, min_lon_cell)) !=
             certified.end();
}

void BuildSegmentSafetySnapshotHierarchy(
    SegmentSafetyHazardSnapshot* snapshot) {
  if (!snapshot) return;
  constexpr double kLargeCellDegrees = 1.0;
  constexpr double kFineCellDegrees = 0.25;
  const long first_lat = floor(snapshot->area.min_lat / kLargeCellDegrees);
  const long last_lat =
      floor(nextafter(snapshot->area.max_lat, snapshot->area.min_lat) /
            kLargeCellDegrees);
  const long first_lon = floor(snapshot->area.min_lon / kLargeCellDegrees);
  const long last_lon =
      floor(nextafter(snapshot->area.max_lon, snapshot->area.min_lon) /
            kLargeCellDegrees);
  for (long lat = first_lat; lat <= last_lat; ++lat) {
    for (long lon = first_lon; lon <= last_lon; ++lon) {
      const SegmentSafetyBBox large = {
          lat * kLargeCellDegrees, (lat + 1) * kLargeCellDegrees,
          lon * kLargeCellDegrees, (lon + 1) * kLargeCellDegrees};
      const bool large_inside = large.min_lat >= snapshot->area.min_lat &&
                                large.max_lat <= snapshot->area.max_lat &&
                                large.min_lon >= snapshot->area.min_lon &&
                                large.max_lon <= snapshot->area.max_lon;
      if (large_inside && ClassifySegmentSafetySnapshotBBox(*snapshot, large) ==
                              kSnapshotSafe) {
        snapshot->certified_safe_large_cells.insert(std::make_pair(lat, lon));
        continue;
      }
      // Subdivide only uncertain large cells.  These quarter-degree cells
      // bridge the open-sea certificate and the existing coarse/fine tiles.
      for (int lat_part = 0; lat_part < 4; ++lat_part) {
        for (int lon_part = 0; lon_part < 4; ++lon_part) {
          const long fine_lat = lat * 4 + lat_part;
          const long fine_lon = lon * 4 + lon_part;
          const SegmentSafetyBBox fine = {
              fine_lat * kFineCellDegrees, (fine_lat + 1) * kFineCellDegrees,
              fine_lon * kFineCellDegrees, (fine_lon + 1) * kFineCellDegrees};
          const bool fine_inside = fine.min_lat >= snapshot->area.min_lat &&
                                   fine.max_lat <= snapshot->area.max_lat &&
                                   fine.min_lon >= snapshot->area.min_lon &&
                                   fine.max_lon <= snapshot->area.max_lon;
          if (fine_inside && ClassifySegmentSafetySnapshotBBox(
                                 *snapshot, fine) == kSnapshotSafe)
            snapshot->certified_safe_fine_cells.insert(
                std::make_pair(fine_lat, fine_lon));
        }
      }
    }
  }
}

SegmentSafetySnapshotDecision QuerySegmentSafetyHazardSnapshot(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth) {
  std::shared_ptr<const SegmentSafetyHazardSnapshot> snapshot;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    snapshot = s_segment_safety_hazard_snapshot;
  }
  if (!snapshot || check_depth || lon1 < -180.0 || lon1 > 180.0 ||
      lon2 < -180.0 || lon2 > 180.0)
    return kSnapshotUnknown;

  const SegmentSafetyBBox query =
      SegmentSafetySegmentBBox(lat1, lon1, lat2, lon2, safety_margin_nm);
  if (query.min_lat < snapshot->area.min_lat ||
      query.max_lat > snapshot->area.max_lat ||
      query.min_lon < snapshot->area.min_lon ||
      query.max_lon > snapshot->area.max_lon)
    return kSnapshotUnknown;

  if (SegmentSafetyBBoxInsideCertifiedCell(
          query, 1.0, snapshot->certified_safe_large_cells) ||
      SegmentSafetyBBoxInsideCertifiedCell(query, 0.25,
                                           snapshot->certified_safe_fine_cells))
    return kSnapshotSafe;
  return ClassifySegmentSafetySnapshotBBox(*snapshot, query);
}

CachedLandRing SegmentSafetyRingFromFloatTable(const float* points, int count) {
  CachedLandRing ring;
  if (!points || count < 3) return ring;
  ring.points.reserve(count);
  for (int i = 0; i < count; ++i)
    ring.points.push_back(wxPoint2DDouble(points[i * 2 + 1], points[i * 2]));
  ring.bbox = SegmentSafetyRingBBox(ring.points);
  return ring;
}

void SegmentSafetyAppendFeatureRings(s57chart* chart, const char* feature_name,
                                     std::vector<CachedLandRing>* destination,
                                     std::set<std::string>* seen) {
  if (!chart || !feature_name || !destination) return;
  std::vector<std::vector<wxPoint2DDouble> > rings;
  chart->CollectFeatureAreaRings(feature_name, rings);
  for (std::vector<std::vector<wxPoint2DDouble> >::iterator it = rings.begin();
       it != rings.end(); ++it) {
    if (it->size() < 3) continue;
    CachedLandRing ring;
    ring.points.swap(*it);
    ring.bbox = SegmentSafetyRingBBox(ring.points);
    if (seen) {
      std::ostringstream signature;
      signature << ring.points.size();
      for (std::vector<wxPoint2DDouble>::const_iterator point =
               ring.points.begin();
           point != ring.points.end(); ++point)
        signature << ':' << llround(point->m_y * 1e7) << ','
                  << llround(point->m_x * 1e7);
      if (!seen->insert(signature.str()).second) continue;
    }
    destination->push_back(ring);
  }
}

int SegmentSafetyCurrentGroupIndex() {
  ChartCanvas* canvas = gFrame && gFrame->GetFocusCanvas()
                            ? gFrame->GetFocusCanvas()
                            : (gFrame ? gFrame->GetPrimaryCanvas() : NULL);
  return canvas ? canvas->m_groupIndex : 0;
}

void SegmentSafetyHashAdd(uint64_t* hash, const wxString& text) {
  if (!hash) return;
  wxCharBuffer utf8 = text.ToUTF8();
  const char* data = utf8.data() ? utf8.data() : "";
  while (*data) {
    *hash ^= (unsigned char)*data++;
    *hash *= 1099511628211ULL;
  }
  *hash ^= (unsigned char)'|';
  *hash *= 1099511628211ULL;
}

wxString SegmentSafetyPluginBatchProviderIdentity() {
  wxArrayString providers;
  auto plugin_array = PluginLoader::GetInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); ++i) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (!pic || !pic->m_enabled || !pic->m_init_state || !pic->m_pplugin ||
        !g_pi_manager->HasChartSafetyProvider(
            pic->m_pplugin->GetCommonName().ToStdString()))
      continue;
    wxFileName file(pic->m_plugin_file);
    const wxULongLong size = file.GetSize();
    const wxDateTime modified = file.GetModificationTime();
    providers.Add(wxString::Format(
        "%s:size=%s:mtime=%lld:version=%s", pic->m_plugin_file,
        size != wxInvalidSize ? size.ToString() : wxString("unknown"),
        modified.IsValid() ? static_cast<long long>(modified.GetTicks()) : -1LL,
        pic->m_version_str));
  }
  providers.Sort();
  return wxJoin(providers, ';');
}

wxString SegmentSafetyChartIdentity() {
  if (!ChartData) return wxEmptyString;

  uint64_t hash = 1469598103934665603ULL;
  // Keep only global semantic inputs here.  Individual chart metadata is
  // fingerprinted per tile, allowing a chart update to invalidate affected
  // waters without discarding an otherwise valid regional atlas.
  SegmentSafetyHashAdd(&hash, "semantic-grid-v6");
  SegmentSafetyHashAdd(
      &hash, wxString::Format("group=%d", SegmentSafetyCurrentGroupIndex()));
  if (g_pi_manager)
    SegmentSafetyHashAdd(
        &hash, "batch-providers=" + SegmentSafetyPluginBatchProviderIdentity());
  // v6 also identifies canonical global-grid coordinates and padded chart
  // candidate discovery at shared provider-batch edges.  Earlier v4 stores
  // can contain edge cells classified from an incomplete candidate set, so
  // they must be invalidated once even when the chart database is unchanged.
  return wxString::Format("ocpn-chart-safety-v6-%016llx",
                          (unsigned long long)hash);
}

wxString SegmentSafetyChartCatalogIdentity() {
  if (!ChartData) return wxEmptyString;
  uint64_t hash = 1469598103934665603ULL;
  SegmentSafetyHashAdd(&hash, ChartData->GetDBFileName());
  SegmentSafetyHashAdd(&hash,
                       wxString::Format("dbv=%d", ChartData->GetVersion()));
  const int entries = ChartData->GetChartTableEntries();
  SegmentSafetyHashAdd(&hash, wxString::Format("entries=%d", entries));
  for (int i = 0; i < entries; ++i) {
    const ChartTableEntry& entry = ChartData->GetChartTableEntry(i);
    SegmentSafetyHashAdd(&hash,
                         wxString::FromUTF8(entry.GetFullPath().c_str()));
    SegmentSafetyHashAdd(
        &hash,
        wxString::Format("t=%lld:e=%lld:scale=%d:type=%d:family=%d",
                         static_cast<long long>(entry.GetFileTime()),
                         static_cast<long long>(entry.GetChartEditionDate()),
                         entry.GetScale(), entry.GetChartType(),
                         entry.GetChartFamily()));
    const std::vector<int>& groups = entry.GetGroupArray();
    for (size_t group = 0; group < groups.size(); ++group)
      SegmentSafetyHashAdd(&hash, wxString::Format("g%d", groups[group]));
  }
  return wxString::Format("chart-catalog-v1-%016llx",
                          static_cast<unsigned long long>(hash));
}

void SegmentSafetyRefreshPersistentChartIdentity() {
  if (!wxThread::IsMain()) return;
  wxString identity = SegmentSafetyChartIdentity();
  wxString catalog_identity = SegmentSafetyChartCatalogIdentity();
  SegmentSafetyTileCacheIdentity identity_callback = nullptr;
  SegmentSafetyTileCacheDependenciesChanged dependencies_callback = nullptr;
  void* identity_context = nullptr;
  void* dependencies_context = nullptr;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    if (identity != s_segment_safety_chart_identity) {
      wxLogMessage("WR_CERT_SAFE_CACHE chart_identity old=\"%s\" new=\"%s\"",
                   s_segment_safety_chart_identity, identity);
      s_segment_safety_chart_identity = identity;
      // Every classification and derived proof is chart-set specific. Clear
      // session state as well as pending persistent data when the chart
      // identity changes so stale tiles cannot survive a chart database
      // update.
      s_segment_safety_land_cache.clear();
      s_segment_safety_hazard_snapshot.reset();
      s_segment_safety_point_cache.clear();
      s_segment_safety_grid_cache.clear();
      s_segment_safety_route_mask_cache.clear();
      s_segment_safety_coarse_route_mask_cache.clear();
      s_segment_safety_segment_cache.clear();
      s_segment_safety_pinned_grid_keys.clear();
      s_segment_safety_pinned_route_mask_keys.clear();
      s_segment_safety_persistent_base_tile_cache.clear();
      s_segment_safety_persistent_certified_safe_cache.clear();
      s_segment_safety_persistent_cache_loaded = false;
      s_segment_safety_persistent_base_tiles_loaded = false;
      s_segment_safety_persistent_cache_dirty = false;
      s_segment_safety_persistent_base_tiles_dirty = false;
      s_segment_safety_persistent_tiles_since_checkpoint = 0;
      identity_callback = s_segment_safety_external_tile_cache.identity_changed;
      identity_context = s_segment_safety_external_tile_cache.context;
    }
    if (catalog_identity != s_segment_safety_chart_catalog_identity) {
      const bool initial_catalog =
          s_segment_safety_chart_catalog_identity.IsEmpty();
      wxLogMessage(
          "WR_CERT_SAFE_CACHE chart_catalog old=\"%s\" new=\"%s\" "
          "selective_base_tile_validation=1",
          s_segment_safety_chart_catalog_identity, catalog_identity);
      s_segment_safety_chart_catalog_identity = catalog_identity;
      // Base semantic tiles carry a local dependency fingerprint and remain
      // available for selective validation.  Derived masks and broad proofs
      // do not, so discard them whenever the catalogue changes.
      if (!initial_catalog) {
        s_segment_safety_land_cache.clear();
        s_segment_safety_hazard_snapshot.reset();
        s_segment_safety_point_cache.clear();
        s_segment_safety_route_mask_cache.clear();
        s_segment_safety_coarse_route_mask_cache.clear();
        s_segment_safety_segment_cache.clear();
        s_segment_safety_pinned_route_mask_keys.clear();
        s_segment_safety_persistent_certified_safe_cache.clear();
        s_segment_safety_persistent_cache_dirty = true;
        dependencies_callback =
            s_segment_safety_external_tile_cache.dependencies_changed;
        dependencies_context = s_segment_safety_external_tile_cache.context;
      }
    }
  }
  // Plugin callbacks may perform disk I/O and use their own mutex. Never call
  // them while holding the host cache mutex.
  if (identity_callback) {
    wxCharBuffer utf8 = identity.ToUTF8();
    identity_callback(identity_context, utf8.data() ? utf8.data() : "");
  }
  if (dependencies_callback) dependencies_callback(dependencies_context);
}

wxString SegmentSafetyPersistentCachePath() {
  wxString base = g_Platform ? g_Platform->GetPrivateDataDir()
                             : *GetpPrivateApplicationDataLocation();
  wxFileName dir(base, "");
  dir.AppendDir("weather_routing");
  wxFileName::Mkdir(dir.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
  return dir.GetPathWithSep() + "chart_safety_certified_cache.json";
}

wxString SegmentSafetyPersistentBaseTileCachePath() {
  wxFileName certified(SegmentSafetyPersistentCachePath());
  return certified.GetPathWithSep() + "chart_safety_base_tiles.bin";
}

std::string SegmentSafetyPersistentProofKey(long lat_cell, long lon_cell,
                                            double safety_margin_nm,
                                            bool check_depth,
                                            double minimum_depth_m) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_chart_identity.IsEmpty()) return std::string();
  return (s_segment_safety_chart_identity + ":" +
          wxString::FromUTF8(SegmentSafetyCoarseRouteMaskKey(
                                 lat_cell, lon_cell, safety_margin_nm,
                                 check_depth, minimum_depth_m)
                                 .c_str()))
      .ToStdString();
}

bool SegmentSafetyPersistentCacheLoadLocked(const wxString& path) {
  if (s_segment_safety_persistent_cache_loaded) return true;
  s_segment_safety_persistent_cache_loaded = true;
  s_segment_safety_persistent_certified_safe_cache.clear();
  s_segment_safety_persistent_entries_loaded = 0;
  s_segment_safety_persistent_entries_ignored = 0;
  s_segment_safety_persistent_stale_ignored = 0;
  s_segment_safety_persistent_malformed_ignored = 0;

  if (!wxFileExists(path)) {
    wxLogMessage(
        "WR_CERT_SAFE_CACHE load enabled=1 entries_loaded=0 "
        "cache_file_path=\"%s\" reason=missing",
        path);
    return true;
  }

  wxFileInputStream input(path);
  if (!input.IsOk()) {
    ++s_segment_safety_persistent_malformed_ignored;
    wxLogMessage(
        "WR_CERT_SAFE_CACHE load enabled=1 cache_malformed_ignored=1 "
        "cache_file_path=\"%s\" reason=open_failed",
        path);
    return false;
  }

  wxJSONReader reader;
  wxJSONValue root;
  int errors = reader.Parse(input, &root);
  if (errors || !root.IsObject() ||
      root.Get("format_version", 0).AsInt() != kPersistentCacheVersion ||
      root.Get("route_mask_algorithm_version", 0).AsInt() !=
          kRouteMaskAlgorithmVersion ||
      !root.HasMember("entries") || !root["entries"].IsArray()) {
    ++s_segment_safety_persistent_malformed_ignored;
    wxLogMessage(
        "WR_CERT_SAFE_CACHE load enabled=1 cache_malformed_ignored=1 "
        "cache_file_path=\"%s\" reason=parse_or_version errors=%d",
        path, errors);
    return false;
  }

  wxJSONValue entries = root["entries"];
  for (int i = 0; i < entries.Size(); ++i) {
    wxJSONValue entry = entries[i];
    if (!entry.IsObject() || !entry.HasMember("key") ||
        !entry.Get("certified_safe", false).AsBool()) {
      ++s_segment_safety_persistent_entries_ignored;
      continue;
    }
    CachedSegmentSafetyCoarseRouteMaskCell cell;
    cell.group_index = entry.Get("group_index", 0).AsInt();
    cell.lat_cell = entry.Get("lat_cell", 0).AsInt();
    cell.lon_cell = entry.Get("lon_cell", 0).AsInt();
    cell.degrees = entry.Get("coarse_degrees", cell.degrees).AsDouble();
    cell.min_lat =
        entry.Get("min_lat", cell.lat_cell * cell.degrees).AsDouble();
    cell.min_lon =
        entry.Get("min_lon", cell.lon_cell * cell.degrees).AsDouble();
    cell.check_depth = entry.Get("check_depth", false).AsBool();
    cell.minimum_depth_m = entry.Get("minimum_depth_m", 0.0).AsDouble();
    cell.safety_margin_nm = entry.Get("safety_margin_nm", 0.0).AsDouble();
    cell.state = kCoarseCertifiedSafe;
    cell.block_summary_flags = kRouteClear;
    cell.fine_tiles_checked = kCoarseRouteMaskFactor * kCoarseRouteMaskFactor;
    cell.fine_tiles_clear = cell.fine_tiles_checked;
    cell.fine_tiles_mixed = 0;
    cell.source =
        (SegmentSafetySource)entry.Get("source", (int)kSourceVectorChart)
            .AsInt();
    cell.persistent_cache_allowed =
        entry
            .Get("persistent_cache_allowed", cell.source != kSourcePluginVector)
            .AsBool();
    if (cell.source == kSourcePluginVector && !cell.persistent_cache_allowed) {
      ++s_segment_safety_persistent_entries_ignored;
      continue;
    }
    wxString key = entry.Get("key", "").AsString();
    if (key.IsEmpty()) {
      ++s_segment_safety_persistent_entries_ignored;
      continue;
    }
    s_segment_safety_persistent_certified_safe_cache[key.ToStdString()] = cell;
    while (s_segment_safety_persistent_certified_safe_cache.size() >
           kMaxPersistentCertifiedCells)
      s_segment_safety_persistent_certified_safe_cache.erase(
          s_segment_safety_persistent_certified_safe_cache.begin());
    ++s_segment_safety_persistent_entries_loaded;
  }

  wxLogMessage(
      "WR_CERT_SAFE_CACHE load enabled=1 entries_loaded=%ld "
      "entries_ignored=%ld "
      "cache_malformed_ignored=%ld cache_file_path=\"%s\"",
      s_segment_safety_persistent_entries_loaded,
      s_segment_safety_persistent_entries_ignored,
      s_segment_safety_persistent_malformed_ignored, path);
  return true;
}

template <typename T>
bool SegmentSafetyWriteBinary(std::ostream& output, const T& value) {
  output.write(reinterpret_cast<const char*>(&value), sizeof(value));
  return output.good();
}

template <typename T>
bool SegmentSafetyReadBinary(std::istream& input, T* value) {
  if (!value) return false;
  input.read(reinterpret_cast<char*>(value), sizeof(*value));
  return input.good();
}

bool SegmentSafetyWriteBinaryString(std::ostream& output,
                                    const std::string& value) {
  if (value.size() > 4096) return false;
  uint32_t size = static_cast<uint32_t>(value.size());
  if (!SegmentSafetyWriteBinary(output, size)) return false;
  output.write(value.data(), size);
  return output.good();
}

bool SegmentSafetyReadBinaryString(std::istream& input, std::string* value) {
  if (!value) return false;
  uint32_t size = 0;
  if (!SegmentSafetyReadBinary(input, &size) || size > 4096) return false;
  value->assign(size, '\0');
  if (size) input.read(&(*value)[0], size);
  return input.good();
}

bool SegmentSafetyPersistentBaseTileCacheLoadLocked(const wxString& path) {
  if (s_segment_safety_persistent_base_tiles_loaded) return true;
  s_segment_safety_persistent_base_tiles_loaded = true;
  s_segment_safety_persistent_base_tile_cache.clear();
  s_segment_safety_persistent_base_tiles_loaded_count = 0;
  s_segment_safety_persistent_base_tiles_ignored = 0;

  if (!wxFileExists(path)) {
    wxLogMessage(
        "WR_BASE_TILE_CACHE load enabled=1 tiles_loaded=0 "
        "cache_file_path=\"%s\" reason=missing",
        path);
    return true;
  }

  std::ifstream input(path.ToStdString().c_str(),
                      std::ios::in | std::ios::binary);
  char magic[8] = {};
  uint32_t format_version = 0;
  uint32_t algorithm_version = 0;
  uint32_t endian_marker = 0;
  double tile_degrees = 0.0;
  double resolution = 0.0;
  uint32_t tile_count = 0;
  std::string identity;
  input.read(magic, sizeof(magic));
  bool header_ok = input.good() && !memcmp(magic, "WRBASE1", 7) &&
                   SegmentSafetyReadBinary(input, &format_version) &&
                   SegmentSafetyReadBinary(input, &algorithm_version) &&
                   SegmentSafetyReadBinary(input, &endian_marker) &&
                   SegmentSafetyReadBinary(input, &tile_degrees) &&
                   SegmentSafetyReadBinary(input, &resolution) &&
                   SegmentSafetyReadBinary(input, &tile_count) &&
                   SegmentSafetyReadBinaryString(input, &identity);
  const std::string expected_identity =
      s_segment_safety_chart_identity.ToStdString();
  if (!header_ok || format_version != kPersistentBaseTileVersion ||
      algorithm_version != kRouteMaskAlgorithmVersion ||
      endian_marker != 0x01020304u ||
      fabs(tile_degrees - kGridTileDegrees) > 1e-12 ||
      fabs(resolution - kGridResolutionDegrees) > 1e-12 ||
      tile_count > kMaxPersistentGridTiles || identity != expected_identity) {
    ++s_segment_safety_persistent_base_tiles_ignored;
    wxLogMessage(
        "WR_BASE_TILE_CACHE load enabled=1 tiles_loaded=0 tiles_ignored=1 "
        "cache_file_path=\"%s\" reason=%s cache_key_match=%d",
        path, header_ok ? "identity_or_version_mismatch" : "malformed",
        header_ok && identity == expected_identity ? 1 : 0);
    return false;
  }

  const uint16_t valid_hazards =
      kHazardLand | kHazardDrying | kHazardNoChart | kHazardUnknownClass;
  for (uint32_t record = 0; record < tile_count; ++record) {
    int32_t group_index = 0;
    int64_t lat_tile = 0;
    int64_t lon_tile = 0;
    int32_t rows = 0;
    int32_t cols = 0;
    int32_t chart_db_index = -1;
    int32_t chart_scale = -1;
    int32_t source = kSourceNone;
    uint32_t hazard_summary = 0;
    uint8_t depth_complete = 0;
    uint32_t cell_count = 0;
    std::string chart_path;
    std::string dependency_identity;
    bool record_ok =
        SegmentSafetyReadBinary(input, &group_index) &&
        SegmentSafetyReadBinary(input, &lat_tile) &&
        SegmentSafetyReadBinary(input, &lon_tile) &&
        SegmentSafetyReadBinary(input, &rows) &&
        SegmentSafetyReadBinary(input, &cols) &&
        SegmentSafetyReadBinary(input, &chart_db_index) &&
        SegmentSafetyReadBinary(input, &chart_scale) &&
        SegmentSafetyReadBinary(input, &source) &&
        SegmentSafetyReadBinary(input, &hazard_summary) &&
        SegmentSafetyReadBinary(input, &depth_complete) &&
        SegmentSafetyReadBinaryString(input, &chart_path) &&
        SegmentSafetyReadBinaryString(input, &dependency_identity) &&
        SegmentSafetyReadBinary(input, &cell_count);
    const bool dimensions_ok =
        rows > 0 && cols > 0 && rows <= 256 && cols <= 256;
    const uint32_t expected_cells =
        dimensions_ok
            ? static_cast<uint32_t>(rows) * static_cast<uint32_t>(cols)
            : 0;
    if (!record_ok || group_index != SegmentSafetyCurrentGroupIndex() ||
        !dimensions_ok || lat_tile < -1800 || lat_tile > 1800 ||
        lon_tile < -7200 || lon_tile > 7200 || source < kSourceNone ||
        source > kSourcePluginVector || source == kSourcePluginVector ||
        expected_cells == 0 || cell_count != expected_cells ||
        depth_complete > 1 || dependency_identity.empty() ||
        dependency_identity.size() >=
            sizeof(CachedPointSafetyGridTile().dependency_identity) ||
        (hazard_summary & ~valid_hazards)) {
      ++s_segment_safety_persistent_base_tiles_ignored;
      wxLogMessage(
          "WR_BASE_TILE_CACHE load cache_malformed_ignored=1 record=%u "
          "cache_file_path=\"%s\"",
          record, path);
      s_segment_safety_persistent_base_tile_cache.clear();
      return false;
    }

    CachedPointSafetyGridTile tile;
    tile.group_index = group_index;
    tile.lat_tile = static_cast<long>(lat_tile);
    tile.lon_tile = static_cast<long>(lon_tile);
    tile.min_lat = tile.lat_tile * kGridTileDegrees;
    tile.min_lon = tile.lon_tile * kGridTileDegrees;
    tile.resolution = kGridResolutionDegrees;
    tile.rows = rows;
    tile.cols = cols;
    tile.chart_db_index = chart_db_index;
    tile.chart_scale = chart_scale;
    tile.source = static_cast<SegmentSafetySource>(source);
    tile.hazard_summary_flags = hazard_summary;
    tile.depth_complete = depth_complete != 0;
    strncpy(tile.chart_path, chart_path.c_str(), sizeof(tile.chart_path) - 1);
    tile.chart_path[sizeof(tile.chart_path) - 1] = '\0';
    strncpy(tile.dependency_identity, dependency_identity.c_str(),
            sizeof(tile.dependency_identity) - 1);
    tile.dependency_identity[sizeof(tile.dependency_identity) - 1] = '\0';
    tile.hazard_flags.resize(cell_count);
    tile.has_depth.resize(cell_count);
    tile.min_depth_m.resize(cell_count);
    input.read(reinterpret_cast<char*>(&tile.hazard_flags[0]),
               cell_count * sizeof(tile.hazard_flags[0]));
    input.read(reinterpret_cast<char*>(&tile.has_depth[0]),
               cell_count * sizeof(tile.has_depth[0]));
    input.read(reinterpret_cast<char*>(&tile.min_depth_m[0]),
               cell_count * sizeof(tile.min_depth_m[0]));
    if (!input.good()) {
      ++s_segment_safety_persistent_base_tiles_ignored;
      s_segment_safety_persistent_base_tile_cache.clear();
      return false;
    }

    tile.classes.resize(cell_count);
    tile.has_drying.resize(cell_count);
    tile.land_count = tile.water_count = tile.drying_count =
        tile.unknown_count = 0;
    for (uint32_t i = 0; i < cell_count; ++i) {
      uint16_t hazards = tile.hazard_flags[i];
      if ((hazards & ~valid_hazards) || tile.has_depth[i] > 1 ||
          (tile.has_depth[i] && !std::isfinite(tile.min_depth_m[i]))) {
        ++s_segment_safety_persistent_base_tiles_ignored;
        s_segment_safety_persistent_base_tile_cache.clear();
        return false;
      }
      if (hazards & kHazardLand) {
        tile.classes[i] = kPointLand;
        ++tile.land_count;
      } else if (hazards & kHazardDrying) {
        tile.classes[i] = kPointDrying;
        ++tile.drying_count;
      } else if (hazards & (kHazardNoChart | kHazardUnknownClass)) {
        tile.classes[i] = kPointNoData;
        ++tile.unknown_count;
      } else {
        tile.classes[i] = kPointWater;
        ++tile.water_count;
      }
      tile.has_drying[i] = (hazards & kHazardDrying) ? 1 : 0;
    }
    tile.built = true;
    tile.persistent_loaded = true;
    const std::string key =
        SegmentSafetyGridTileKeyForIndices(tile.lat_tile, tile.lon_tile);
    s_segment_safety_persistent_base_tile_cache[key] = tile;
    ++s_segment_safety_persistent_base_tiles_loaded_count;
  }

  wxLogMessage(
      "WR_BASE_TILE_CACHE load enabled=1 cache_key_match=1 tiles_loaded=%ld "
      "tiles_ignored=%ld cache_file_path=\"%s\" cache_file_size=%llu",
      s_segment_safety_persistent_base_tiles_loaded_count,
      s_segment_safety_persistent_base_tiles_ignored, path,
      (unsigned long long)wxFileName(path).GetSize().GetValue());
  return true;
}

void SegmentSafetyPersistentCacheEnsureLoaded() {
  if (!s_segment_safety_persistent_cache_enabled) return;
  wxString path = SegmentSafetyPersistentCachePath();
  wxString base_path = SegmentSafetyPersistentBaseTileCachePath();
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  SegmentSafetyPersistentCacheLoadLocked(path);
  SegmentSafetyPersistentBaseTileCacheLoadLocked(base_path);
}

bool SegmentSafetyPersistentCertifiedCacheSave() {
  wxString path = SegmentSafetyPersistentCachePath();
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (!s_segment_safety_persistent_cache_enabled) {
    wxLogMessage("WR_CERT_SAFE_CACHE save enabled=0 cache_file_path=\"%s\"",
                 path);
    return true;
  }
  if (!s_segment_safety_persistent_cache_dirty) return true;

  wxJSONValue root;
  root["format_version"] = kPersistentCacheVersion;
  root["route_mask_algorithm_version"] = kRouteMaskAlgorithmVersion;
  root["updated_utc"] = wxDateTime::UNow().ToUTC().FormatISOCombined('T');
  wxJSONValue entries;
  long saved = 0;
  for (std::map<std::string,
                CachedSegmentSafetyCoarseRouteMaskCell>::const_iterator it =
           s_segment_safety_persistent_certified_safe_cache.begin();
       it != s_segment_safety_persistent_certified_safe_cache.end(); ++it) {
    const CachedSegmentSafetyCoarseRouteMaskCell& cell = it->second;
    if (cell.state != kCoarseCertifiedSafe ||
        (cell.source == kSourcePluginVector && !cell.persistent_cache_allowed))
      continue;
    wxJSONValue entry;
    entry["key"] = wxString::FromUTF8(it->first.c_str());
    entry["certified_safe"] = true;
    entry["group_index"] = cell.group_index;
    entry["lat_cell"] = (wxInt64)cell.lat_cell;
    entry["lon_cell"] = (wxInt64)cell.lon_cell;
    entry["min_lat"] = cell.min_lat;
    entry["min_lon"] = cell.min_lon;
    entry["coarse_degrees"] = cell.degrees;
    entry["coarse_factor"] = kCoarseRouteMaskFactor;
    entry["grid_resolution_degrees"] = kGridResolutionDegrees;
    entry["safety_margin_nm"] = cell.safety_margin_nm;
    entry["check_depth"] = cell.check_depth;
    entry["minimum_depth_m"] = cell.minimum_depth_m;
    entry["source"] = (int)cell.source;
    entry["persistent_cache_allowed"] = cell.persistent_cache_allowed;
    entries.Append(entry);
    ++saved;
  }
  root["entries"] = entries;

  wxFileName::Mkdir(wxFileName(path).GetPath(), wxS_DIR_DEFAULT,
                    wxPATH_MKDIR_FULL);
  wxString tmp_path = path + ".tmp";
  if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
  bool write_ok = false;
  {
    wxFileOutputStream output(tmp_path);
    if (output.IsOk()) {
      wxJSONWriter writer(wxJSONWRITER_STYLED);
      writer.Write(root, output);
      write_ok = output.IsOk();
    }
  }
  if (!write_ok) {
    wxLogMessage(
        "WR_CERT_SAFE_CACHE save failed cache_file_path=\"%s\" entries=%ld",
        path, saved);
    if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
    return false;
  }
  if (!wxRenameFile(tmp_path, path, true)) {
    wxLogMessage(
        "WR_CERT_SAFE_CACHE save failed cache_file_path=\"%s\" entries=%ld "
        "reason=rename_failed",
        path, saved);
    if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
    return false;
  }
  s_segment_safety_persistent_entries_saved = saved;
  s_segment_safety_persistent_cache_dirty = false;
  wxLogMessage(
      "WR_CERT_SAFE_CACHE save enabled=1 entries_saved=%ld "
      "cache_file_path=\"%s\" "
      "cache_file_size=%llu",
      saved, path, (unsigned long long)wxFileName(path).GetSize().GetValue());
  return true;
}

bool SegmentSafetyPersistentBaseTileCacheSave() {
  const wxString path = SegmentSafetyPersistentBaseTileCachePath();
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (!s_segment_safety_persistent_cache_enabled) {
    wxLogMessage("WR_BASE_TILE_CACHE save enabled=0 cache_file_path=\"%s\"",
                 path);
    return true;
  }
  if (!s_segment_safety_persistent_base_tiles_dirty) return true;

  std::map<std::string, CachedPointSafetyGridTile> tiles =
      s_segment_safety_persistent_base_tile_cache;
  for (std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
           s_segment_safety_grid_cache.begin();
       it != s_segment_safety_grid_cache.end(); ++it) {
    // Licensed plugin-vector semantics are checkpointed by the registered
    // external cache only when the provider advertised permission.  Keep the
    // core raw-tile file native-only so its older format never serializes a
    // plugin tile without carrying that permission bit.
    if (it->second.built && it->second.source != kSourcePluginVector)
      tiles[it->first] = it->second;
  }
  for (std::map<std::string, CachedPointSafetyGridTile>::iterator it =
           tiles.begin();
       it != tiles.end();) {
    if (it->second.source == kSourcePluginVector)
      tiles.erase(it++);
    else
      ++it;
  }
  while (tiles.size() > kMaxPersistentGridTiles) tiles.erase(tiles.begin());

  wxFileName::Mkdir(wxFileName(path).GetPath(), wxS_DIR_DEFAULT,
                    wxPATH_MKDIR_FULL);
  const wxString tmp_path = path + ".tmp";
  if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
  std::ofstream output(tmp_path.ToStdString().c_str(),
                       std::ios::out | std::ios::binary | std::ios::trunc);
  const char magic[8] = {'W', 'R', 'B', 'A', 'S', 'E', '1', '\0'};
  output.write(magic, sizeof(magic));
  const uint32_t format_version = kPersistentBaseTileVersion;
  const uint32_t algorithm_version = kRouteMaskAlgorithmVersion;
  const uint32_t endian_marker = 0x01020304u;
  const uint32_t tile_count = static_cast<uint32_t>(tiles.size());
  bool ok = output.good() && SegmentSafetyWriteBinary(output, format_version) &&
            SegmentSafetyWriteBinary(output, algorithm_version) &&
            SegmentSafetyWriteBinary(output, endian_marker) &&
            SegmentSafetyWriteBinary(output, kGridTileDegrees) &&
            SegmentSafetyWriteBinary(output, kGridResolutionDegrees) &&
            SegmentSafetyWriteBinary(output, tile_count) &&
            SegmentSafetyWriteBinaryString(
                output, s_segment_safety_chart_identity.ToStdString());
  long saved = 0;
  for (std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
           tiles.begin();
       ok && it != tiles.end(); ++it) {
    const CachedPointSafetyGridTile& tile = it->second;
    const size_t cells = static_cast<size_t>(tile.rows) * tile.cols;
    if (!tile.built || tile.rows <= 0 || tile.cols <= 0 || cells > 65536 ||
        tile.hazard_flags.size() != cells || tile.has_depth.size() != cells ||
        tile.min_depth_m.size() != cells) {
      ok = false;
      break;
    }
    const int32_t group_index = tile.group_index;
    const int64_t lat_tile = tile.lat_tile;
    const int64_t lon_tile = tile.lon_tile;
    const int32_t rows = tile.rows;
    const int32_t cols = tile.cols;
    const int32_t chart_db_index = tile.chart_db_index;
    const int32_t chart_scale = tile.chart_scale;
    const int32_t source = tile.source;
    const uint32_t hazard_summary = tile.hazard_summary_flags;
    const uint8_t depth_complete = tile.depth_complete ? 1 : 0;
    const uint32_t cell_count = static_cast<uint32_t>(cells);
    ok = SegmentSafetyWriteBinary(output, group_index) &&
         SegmentSafetyWriteBinary(output, lat_tile) &&
         SegmentSafetyWriteBinary(output, lon_tile) &&
         SegmentSafetyWriteBinary(output, rows) &&
         SegmentSafetyWriteBinary(output, cols) &&
         SegmentSafetyWriteBinary(output, chart_db_index) &&
         SegmentSafetyWriteBinary(output, chart_scale) &&
         SegmentSafetyWriteBinary(output, source) &&
         SegmentSafetyWriteBinary(output, hazard_summary) &&
         SegmentSafetyWriteBinary(output, depth_complete) &&
         SegmentSafetyWriteBinaryString(output, tile.chart_path) &&
         SegmentSafetyWriteBinaryString(output, tile.dependency_identity) &&
         SegmentSafetyWriteBinary(output, cell_count);
    if (!ok) break;
    output.write(reinterpret_cast<const char*>(&tile.hazard_flags[0]),
                 cells * sizeof(tile.hazard_flags[0]));
    output.write(reinterpret_cast<const char*>(&tile.has_depth[0]),
                 cells * sizeof(tile.has_depth[0]));
    output.write(reinterpret_cast<const char*>(&tile.min_depth_m[0]),
                 cells * sizeof(tile.min_depth_m[0]));
    ok = output.good();
    if (ok) ++saved;
  }
  output.close();
  if (!ok || saved != static_cast<long>(tile_count)) {
    if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
    wxLogMessage(
        "WR_BASE_TILE_CACHE save failed tiles=%ld cache_file_path=\"%s\"",
        saved, path);
    return false;
  }
  if (!wxRenameFile(tmp_path, path, true)) {
    if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
    wxLogMessage(
        "WR_BASE_TILE_CACHE save failed tiles=%ld cache_file_path=\"%s\" "
        "reason=rename_failed",
        saved, path);
    return false;
  }
  s_segment_safety_persistent_base_tile_cache.swap(tiles);
  s_segment_safety_persistent_base_tiles_saved = saved;
  s_segment_safety_persistent_base_tiles_dirty = false;
  s_segment_safety_persistent_tiles_since_checkpoint = 0;
  wxLogMessage(
      "WR_BASE_TILE_CACHE save enabled=1 tiles_saved=%ld "
      "cache_file_path=\"%s\" cache_file_size=%llu",
      saved, path, (unsigned long long)wxFileName(path).GetSize().GetValue());
  return true;
}

bool SegmentSafetyPersistentCacheSave() {
  bool certified_ok = SegmentSafetyPersistentCertifiedCacheSave();
  bool base_ok = SegmentSafetyPersistentBaseTileCacheSave();
  return certified_ok && base_ok;
}

bool SegmentSafetyPersistentLookupCertifiedSafe(
    long lat_cell, long lon_cell, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, CachedSegmentSafetyCoarseRouteMaskCell* cell) {
  if (!s_segment_safety_persistent_cache_enabled) return false;
  SegmentSafetyPersistentCacheEnsureLoaded();
  std::string key = SegmentSafetyPersistentProofKey(
      lat_cell, lon_cell, safety_margin_nm, check_depth, minimum_depth_m);
  if (key.empty()) return false;
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>::const_iterator
      it = s_segment_safety_persistent_certified_safe_cache.find(key);
  if (it == s_segment_safety_persistent_certified_safe_cache.end()) {
    ++s_segment_safety_persistent_stale_ignored;
    return false;
  }
  if (cell) *cell = it->second;
  ++s_segment_safety_persistent_entries_used;
  wxLogMessage(
      "WR_CERT_SAFE_CACHE use cache_used_as_proof=1 cache_key_match=1 "
      "lat_cell=%ld lon_cell=%ld entries_used=%ld fine_tiles_avoided=%d",
      lat_cell, lon_cell, s_segment_safety_persistent_entries_used,
      kCoarseRouteMaskFactor * kCoarseRouteMaskFactor);
  return true;
}

void SegmentSafetyPersistentStoreCertifiedSafe(
    const CachedSegmentSafetyCoarseRouteMaskCell& cell) {
  if (!s_segment_safety_persistent_cache_enabled ||
      cell.state != kCoarseCertifiedSafe ||
      (cell.source == kSourcePluginVector && !cell.persistent_cache_allowed))
    return;
  std::string key = SegmentSafetyPersistentProofKey(
      cell.lat_cell, cell.lon_cell, cell.safety_margin_nm, cell.check_depth,
      cell.minimum_depth_m);
  if (key.empty()) return;
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  s_segment_safety_persistent_certified_safe_cache[key] = cell;
  while (s_segment_safety_persistent_certified_safe_cache.size() >
         kMaxPersistentCertifiedCells)
    s_segment_safety_persistent_certified_safe_cache.erase(
        s_segment_safety_persistent_certified_safe_cache.begin());
  s_segment_safety_persistent_cache_dirty = true;
  ++s_segment_safety_persistent_entries_stored;
}

std::string SegmentSafetyPointCacheKey(double lat, double lon) {
  const double bucket_degrees = 0.00025;
  long lat_bucket = lround(lat / bucket_degrees);
  long lon_bucket = lround(lon / bucket_degrees);
  return wxString::Format("%d:%ld:%ld", SegmentSafetyCurrentGroupIndex(),
                          lat_bucket, lon_bucket)
      .ToStdString();
}

std::string SegmentSafetySegmentCacheKey(double lat1, double lon1, double lat2,
                                         double lon2, double safety_margin_nm,
                                         bool check_depth,
                                         double minimum_depth_m) {
  long a_lat = lround(lat1 / kGridResolutionDegrees);
  long a_lon = lround(lon1 / kGridResolutionDegrees);
  long b_lat = lround(lat2 / kGridResolutionDegrees);
  long b_lon = lround(lon2 / kGridResolutionDegrees);
  if (std::make_pair(b_lat, b_lon) < std::make_pair(a_lat, a_lon)) {
    std::swap(a_lat, b_lat);
    std::swap(a_lon, b_lon);
  }
  double mid_lat = (lat1 + lat2) / 2.0;
  double cell_nm =
      wxMin(kGridResolutionDegrees * 60.0,
            kGridResolutionDegrees * 60.0 *
                wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mid_lat)))));
  long margin_bucket = lround(safety_margin_nm / wxMax(0.01, cell_nm));
  long depth_bucket = lround(minimum_depth_m * 100.0);
  return wxString::Format("%d:%.6f:%ld:%d:%ld:%ld:%ld:%ld:%ld",
                          SegmentSafetyCurrentGroupIndex(),
                          kGridResolutionDegrees, margin_bucket,
                          check_depth ? 1 : 0, depth_bucket, a_lat, a_lon,
                          b_lat, b_lon)
      .ToStdString();
}

void StoreSegmentSafetySegmentCache(const std::string& key,
                                    const CachedSegmentSafetyResult& value,
                                    SegmentSafetyCoreStats* stats) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_segment_cache.size() >= kMaxSegmentCacheEntries) {
    s_segment_safety_segment_cache.clear();
  }
  s_segment_safety_segment_cache[key] = value;
  if (stats) ++stats->segment_cache_stores;
}

bool LookupSegmentSafetySegmentCache(const std::string& key,
                                     CachedSegmentSafetyResult* value) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedSegmentSafetyResult>::const_iterator it =
      s_segment_safety_segment_cache.find(key);
  if (it == s_segment_safety_segment_cache.end()) return false;
  if (value) *value = it->second;
  return true;
}

void CopyCachedSegmentSafetyToResult(const CachedSegmentSafetyResult& cached,
                                     SegmentSafetyResult* result) {
  if (!result) return;
  SetSegmentSafetyStatus(result, (SegmentSafetyStatus)cached.status);
  SetSegmentSafetySource(result, (SegmentSafetySource)cached.source);
  SetSegmentSafetyDiagnosticReason(
      result, (SegmentSafetyDiagnosticReason)cached.diagnostic_reason);
  SetSegmentSafetyMessage(result, cached.message);
  if (!SegmentSafetyResultHas(result, offsetof(SegmentSafetyResult, hit_object),
                              sizeof(result->hit_object)))
    return;

  result->chart_db_index = cached.chart_db_index;
  result->chart_scale = cached.chart_scale;
  result->hit_sample_lat = cached.hit_sample_lat;
  result->hit_sample_lon = cached.hit_sample_lon;
  result->hit_sample_index = cached.hit_sample_index;
  result->hit_sample_count = cached.hit_sample_count;
  CopySegmentSafetyString(result->chart_path, sizeof(result->chart_path),
                          cached.chart_path);
  CopySegmentSafetyString(result->hit_object, sizeof(result->hit_object),
                          cached.hit_object);
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    result->has_depth = cached.has_depth;
    result->min_depth_m = cached.min_depth_m;
    result->required_depth_m = cached.required_depth_m;
    result->hit_depth_m = cached.hit_depth_m;
    result->has_drying = cached.has_drying;
    CopySegmentSafetyString(result->depth_source_object,
                            sizeof(result->depth_source_object),
                            cached.depth_source_object);
    CopySegmentSafetyString(result->depth_source_attribute,
                            sizeof(result->depth_source_attribute),
                            cached.depth_source_attribute);
  }
}

CachedSegmentSafetyResult MakeCachedSegmentSafetyResult(
    const SegmentSafetyResult* result, int diagnostic_reason_override) {
  CachedSegmentSafetyResult cached;
  if (!result) return cached;
  cached.status = result->status;
  cached.source = result->source;
  cached.diagnostic_reason = diagnostic_reason_override;
  cached.chart_db_index = result->chart_db_index;
  cached.chart_scale = result->chart_scale;
  cached.hit_sample_lat = result->hit_sample_lat;
  cached.hit_sample_lon = result->hit_sample_lon;
  cached.hit_sample_index = result->hit_sample_index;
  cached.hit_sample_count = result->hit_sample_count;
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    cached.has_depth = result->has_depth;
    cached.min_depth_m = result->min_depth_m;
    cached.required_depth_m = result->required_depth_m;
    cached.hit_depth_m = result->hit_depth_m;
    cached.has_drying = result->has_drying;
    CopySegmentSafetyString(cached.depth_source_object,
                            sizeof(cached.depth_source_object),
                            result->depth_source_object);
    CopySegmentSafetyString(cached.depth_source_attribute,
                            sizeof(cached.depth_source_attribute),
                            result->depth_source_attribute);
  }
  CopySegmentSafetyString(cached.message, sizeof(cached.message),
                          result->message);
  CopySegmentSafetyString(cached.chart_path, sizeof(cached.chart_path),
                          result->chart_path);
  CopySegmentSafetyString(cached.hit_object, sizeof(cached.hit_object),
                          result->hit_object);
  return cached;
}

void StoreSegmentSafetyPointCache(
    const std::string& key, const CachedPointSafetyClassification& value) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_point_cache.size() >= kMaxPointCacheEntries) {
    s_segment_safety_point_cache.clear();
  }
  s_segment_safety_point_cache[key] = value;
}

bool LookupSegmentSafetyPointCache(const std::string& key,
                                   CachedPointSafetyClassification* value) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedPointSafetyClassification>::const_iterator it =
      s_segment_safety_point_cache.find(key);
  if (it == s_segment_safety_point_cache.end()) return false;
  if (value) *value = it->second;
  return true;
}

void CopySegmentSafetyPointCacheToResult(
    const CachedPointSafetyClassification& cached,
    SegmentSafetyResult* result) {
  if (!SegmentSafetyResultHas(result, offsetof(SegmentSafetyResult, hit_object),
                              sizeof(result->hit_object)))
    return;

  result->chart_db_index = cached.chart_db_index;
  result->chart_scale = cached.chart_scale;
  strncpy(result->chart_path, cached.chart_path,
          sizeof(result->chart_path) - 1);
  result->chart_path[sizeof(result->chart_path) - 1] = '\0';
  strncpy(result->hit_object, cached.hit_object,
          sizeof(result->hit_object) - 1);
  result->hit_object[sizeof(result->hit_object) - 1] = '\0';
  if (SegmentSafetyResultHas(
          result, offsetof(SegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    result->has_depth = cached.has_depth ? 1 : 0;
    result->min_depth_m = cached.min_depth_m;
    result->has_drying = cached.has_drying ? 1 : 0;
    strncpy(result->depth_source_object, cached.depth_source_object,
            sizeof(result->depth_source_object) - 1);
    result->depth_source_object[sizeof(result->depth_source_object) - 1] = '\0';
    strncpy(result->depth_source_attribute, cached.depth_source_attribute,
            sizeof(result->depth_source_attribute) - 1);
    result->depth_source_attribute[sizeof(result->depth_source_attribute) - 1] =
        '\0';
  }
}

CachedPointSafetyClassification MakeSegmentSafetyPointCacheEntry(
    SegmentSafetyPointClass point_class, SegmentSafetySource source,
    int chart_db_index, int chart_scale, const char* chart_path,
    const char* hit_object, bool has_depth, double min_depth_m, bool has_drying,
    const char* depth_source_object, const char* depth_source_attribute) {
  CachedPointSafetyClassification cached;
  cached.point_class = point_class;
  cached.source = source;
  cached.chart_db_index = chart_db_index;
  cached.chart_scale = chart_scale;
  cached.has_depth = has_depth;
  cached.min_depth_m = min_depth_m;
  cached.has_drying = has_drying;
  if (chart_path) {
    strncpy(cached.chart_path, chart_path, sizeof(cached.chart_path) - 1);
    cached.chart_path[sizeof(cached.chart_path) - 1] = '\0';
  }
  if (hit_object) {
    strncpy(cached.hit_object, hit_object, sizeof(cached.hit_object) - 1);
    cached.hit_object[sizeof(cached.hit_object) - 1] = '\0';
  }
  if (depth_source_object) {
    strncpy(cached.depth_source_object, depth_source_object,
            sizeof(cached.depth_source_object) - 1);
    cached.depth_source_object[sizeof(cached.depth_source_object) - 1] = '\0';
  }
  if (depth_source_attribute) {
    strncpy(cached.depth_source_attribute, depth_source_attribute,
            sizeof(cached.depth_source_attribute) - 1);
    cached.depth_source_attribute[sizeof(cached.depth_source_attribute) - 1] =
        '\0';
  }
  return cached;
}

std::string SegmentSafetyGridTileKeyForIndices(long lat_tile, long lon_tile) {
  char key[96];
  snprintf(key, sizeof(key), "%d:%ld:%ld:%.6f",
           SegmentSafetyCurrentGroupIndex(), lat_tile, lon_tile,
           kGridResolutionDegrees);
  key[sizeof(key) - 1] = '\0';
  return std::string(key);
}

std::string SegmentSafetyGridTileKey(double lat, double lon, long* lat_tile,
                                     long* lon_tile) {
  long lt = floor(lat / kGridTileDegrees);
  long ln = floor(lon / kGridTileDegrees);
  if (lat_tile) *lat_tile = lt;
  if (lon_tile) *lon_tile = ln;
  return SegmentSafetyGridTileKeyForIndices(lt, ln);
}

SegmentSafetyPointClass ChartPointSafetyClassAtRaw(
    double lat, double lon, SegmentSafetySource* source,
    SegmentSafetyCoreStats* stats, SegmentSafetyResult* result = NULL);
CachedPointSafetyGridTile BuildSegmentSafetyGridTile(
    double lat, double lon, long lat_tile, long lon_tile,
    SegmentSafetyCoreStats* stats, bool require_depth);

bool SegmentSafetyExternalTileCacheLookup(long lat_tile, long lon_tile,
                                          bool require_depth,
                                          CachedPointSafetyGridTile* result) {
  if (!result || !wxThread::IsMain()) return false;
  SegmentSafetyTileCacheCallbacks callbacks = {};
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    callbacks = s_segment_safety_external_tile_cache;
  }
  if (!callbacks.lookup) return false;

  const int side =
      static_cast<int>(lround(kGridTileDegrees / kGridResolutionDegrees)) + 1;
  const int cells = side * side;
  CachedPointSafetyGridTile tile;
  tile.hazard_flags.resize(cells);
  tile.has_depth.resize(cells);
  tile.min_depth_m.resize(cells);
  SegmentSafetyTile external = {};
  external.struct_size = sizeof(external);
  external.group_index = SegmentSafetyCurrentGroupIndex();
  external.lat_tile = lat_tile;
  external.lon_tile = lon_tile;
  external.resolution = kGridResolutionDegrees;
  external.rows = side;
  external.cols = side;
  external.hazard_flags = tile.hazard_flags.data();
  external.has_depth = tile.has_depth.data();
  external.min_depth_m = tile.min_depth_m.data();
  external.cell_capacity = cells;
  if (!callbacks.lookup(callbacks.context, lat_tile, lon_tile,
                        require_depth ? 1 : 0, &external))
    return false;

  const uint16_t valid_hazards =
      kHazardLand | kHazardDrying | kHazardNoChart | kHazardUnknownClass;
  if (external.group_index != SegmentSafetyCurrentGroupIndex() ||
      external.lat_tile != lat_tile || external.lon_tile != lon_tile ||
      external.rows != side || external.cols != side ||
      external.cell_capacity < cells ||
      fabs(external.resolution - kGridResolutionDegrees) > 1e-12 ||
      external.source < kSourceNone || external.source > kSourcePluginVector ||
      (external.hazard_summary_flags & ~valid_hazards) ||
      (require_depth && !external.depth_complete))
    return false;

  tile.group_index = external.group_index;
  tile.lat_tile = lat_tile;
  tile.lon_tile = lon_tile;
  tile.min_lat = lat_tile * kGridTileDegrees;
  tile.min_lon = lon_tile * kGridTileDegrees;
  tile.resolution = external.resolution;
  tile.rows = external.rows;
  tile.cols = external.cols;
  tile.chart_db_index = external.chart_db_index;
  tile.chart_scale = external.chart_scale;
  tile.source = static_cast<SegmentSafetySource>(external.source);
  tile.hazard_summary_flags = external.hazard_summary_flags;
  tile.depth_complete = external.depth_complete != 0;
  // Plugin-vector tiles can reach the external cache only after the provider
  // advertised the derived-cache contract in its grid result.
  tile.persistent_cache_allowed = true;
  strncpy(tile.chart_path, external.chart_path, sizeof(tile.chart_path) - 1);
  tile.chart_path[sizeof(tile.chart_path) - 1] = '\0';
  strncpy(tile.dependency_identity, external.dependency_identity,
          sizeof(tile.dependency_identity) - 1);
  tile.dependency_identity[sizeof(tile.dependency_identity) - 1] = '\0';
  tile.classes.resize(cells);
  tile.has_drying.resize(cells);
  tile.land_count = tile.water_count = tile.drying_count = tile.unknown_count =
      0;
  for (int i = 0; i < cells; ++i) {
    const uint16_t hazards = tile.hazard_flags[i];
    if ((hazards & ~valid_hazards) || tile.has_depth[i] > 1 ||
        (tile.has_depth[i] && !std::isfinite(tile.min_depth_m[i])))
      return false;
    if (hazards & kHazardLand) {
      tile.classes[i] = kPointLand;
      ++tile.land_count;
    } else if (hazards & kHazardDrying) {
      tile.classes[i] = kPointDrying;
      ++tile.drying_count;
    } else if (hazards & (kHazardNoChart | kHazardUnknownClass)) {
      tile.classes[i] = kPointNoData;
      ++tile.unknown_count;
    } else {
      tile.classes[i] = kPointWater;
      ++tile.water_count;
    }
    tile.has_drying[i] = (hazards & kHazardDrying) ? 1 : 0;
  }
  tile.built = true;
  tile.persistent_loaded = true;
  *result = tile;
  return true;
}

wxString SegmentSafetyTileDependencyIdentity(long lat_tile, long lon_tile);
bool SegmentSafetyTileDependencyIsCurrent(
    const CachedPointSafetyGridTile& tile);

void SegmentSafetyExternalTileCacheStore(
    const CachedPointSafetyGridTile& tile) {
  if (!tile.built || tile.persistent_loaded || !wxThread::IsMain()) return;
  if (tile.source == kSourcePluginVector && !tile.persistent_cache_allowed)
    return;
  SegmentSafetyTileCacheCallbacks callbacks = {};
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    callbacks = s_segment_safety_external_tile_cache;
  }
  if (!callbacks.store) return;
  const size_t cells = static_cast<size_t>(tile.rows) * tile.cols;
  if (!cells || tile.hazard_flags.size() != cells ||
      tile.has_depth.size() != cells || tile.min_depth_m.size() != cells)
    return;
  SegmentSafetyTile external = {};
  external.struct_size = sizeof(external);
  external.group_index = tile.group_index;
  external.lat_tile = tile.lat_tile;
  external.lon_tile = tile.lon_tile;
  external.resolution = tile.resolution;
  external.rows = tile.rows;
  external.cols = tile.cols;
  external.chart_db_index = tile.chart_db_index;
  external.chart_scale = tile.chart_scale;
  external.source = tile.source;
  external.hazard_summary_flags = tile.hazard_summary_flags;
  external.depth_complete = tile.depth_complete ? 1 : 0;
  const size_t chart_path_size =
      strnlen(tile.chart_path, sizeof(external.chart_path) - 1);
  memcpy(external.chart_path, tile.chart_path, chart_path_size);
  external.chart_path[chart_path_size] = '\0';
  CopySegmentSafetyString(external.dependency_identity,
                          sizeof(external.dependency_identity),
                          tile.dependency_identity);
  external.hazard_flags = const_cast<unsigned short*>(tile.hazard_flags.data());
  external.has_depth = const_cast<unsigned char*>(tile.has_depth.data());
  external.min_depth_m = const_cast<float*>(tile.min_depth_m.data());
  external.cell_capacity = static_cast<int>(cells);
  callbacks.store(callbacks.context, &external);
}

void StoreSegmentSafetyGridTile(const std::string& key,
                                const CachedPointSafetyGridTile& tile) {
  CachedPointSafetyGridTile stored = tile;
  if (wxThread::IsMain()) {
    const wxString dependency =
        SegmentSafetyTileDependencyIdentity(tile.lat_tile, tile.lon_tile);
    CopySegmentSafetyString(stored.dependency_identity,
                            sizeof(stored.dependency_identity),
                            dependency.mb_str());
  }
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    if (s_segment_safety_grid_cache.find(key) ==
        s_segment_safety_grid_cache.end()) {
      while (s_segment_safety_grid_cache.size() >= kMaxGridTiles) {
        std::map<std::string, CachedPointSafetyGridTile>::iterator victim =
            s_segment_safety_grid_cache.end();
        for (std::map<std::string, CachedPointSafetyGridTile>::iterator it =
                 s_segment_safety_grid_cache.begin();
             it != s_segment_safety_grid_cache.end(); ++it) {
          if (s_segment_safety_pinned_grid_keys.find(it->first) ==
              s_segment_safety_pinned_grid_keys.end()) {
            victim = it;
            break;
          }
        }
        if (victim == s_segment_safety_grid_cache.end()) break;
        s_segment_safety_grid_cache.erase(victim);
        ++s_segment_safety_grid_cache_evictions;
      }
    }
    s_segment_safety_grid_cache[key] = stored;
    if (s_segment_safety_persistent_cache_enabled && stored.built &&
        !stored.persistent_loaded && stored.source != kSourcePluginVector) {
      // Compatibility store for older plugins which use the host-owned
      // persistent cache. New weather-routing builds register an external
      // plugin-owned tile cache instead.
      s_segment_safety_persistent_base_tile_cache[key] = stored;
      while (s_segment_safety_persistent_base_tile_cache.size() >
             kMaxPersistentGridTiles)
        s_segment_safety_persistent_base_tile_cache.erase(
            s_segment_safety_persistent_base_tile_cache.begin());
      s_segment_safety_persistent_base_tiles_dirty = true;
      ++s_segment_safety_persistent_tiles_since_checkpoint;
    }
  }
  SegmentSafetyExternalTileCacheStore(stored);
}

bool LookupSegmentSafetyGridTile(const std::string& key,
                                 CachedPointSafetyGridTile* tile) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
      s_segment_safety_grid_cache.find(key);
  if (it == s_segment_safety_grid_cache.end()) return false;
  if (tile) *tile = it->second;
  return true;
}

size_t SegmentSafetyGridCacheSize() {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  return s_segment_safety_grid_cache.size();
}

void RecordUnexpectedSegmentSafetyTileBuild(SegmentSafetyCoreStats* stats,
                                            long lat_tile, long lon_tile);
bool SegmentSafetyCachedTileProviderIsCurrent(
    const CachedPointSafetyGridTile& tile);

bool EnsureSegmentSafetyGridTile(long lat_tile, long lon_tile,
                                 SegmentSafetyCoreStats* stats, bool* built,
                                 bool require_depth) {
  if (built) *built = false;
  std::string key = SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
  CachedPointSafetyGridTile active_tile;
  const bool active_found = LookupSegmentSafetyGridTile(key, &active_tile);
  const bool active_current =
      active_found && (!wxThread::IsMain() ||
                       SegmentSafetyTileDependencyIsCurrent(active_tile));
  if (active_current && (!require_depth || active_tile.depth_complete)) {
    if (stats) ++stats->grid_cache_hits;
    return true;
  }
  if (active_found && (!active_current || (require_depth && active_tile.built &&
                                           !active_tile.depth_complete))) {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    s_segment_safety_grid_cache.erase(key);
    s_segment_safety_persistent_base_tile_cache.erase(key);
    s_segment_safety_persistent_base_tiles_dirty = true;
  }

  if (stats) ++stats->grid_cache_misses;
  if (!wxThread::IsMain()) {
    RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    bool log_worker_miss = false;
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      if (s_segment_safety_worker_tile_miss_logs < kMaxWorkerTileMissLogs) {
        ++s_segment_safety_worker_tile_miss_logs;
        log_worker_miss = true;
      }
    }
    if (log_worker_miss) {
      wxLogMessage(
          "WR_GRID_WORKER_TILE_MISS thread=%p main_thread=%p tile=(%ld,%ld) "
          "tile_min=(%.6f,%.6f). Chart object classification is main-thread "
          "only; worker will treat this segment as not chart-safe.",
          wxThread::GetCurrentId(), wxThread::GetMainId(), lat_tile, lon_tile,
          lat_tile * kGridTileDegrees, lon_tile * kGridTileDegrees);
    }
    return false;
  }

  CachedPointSafetyGridTile external_tile;
  if (SegmentSafetyExternalTileCacheLookup(lat_tile, lon_tile, require_depth,
                                           &external_tile) &&
      SegmentSafetyTileDependencyIsCurrent(external_tile) &&
      SegmentSafetyCachedTileProviderIsCurrent(external_tile)) {
    StoreSegmentSafetyGridTile(key, external_tile);
    if (stats) ++stats->grid_cache_hits;
    return true;
  }

  if (s_segment_safety_persistent_cache_enabled) {
    SegmentSafetyPersistentCacheEnsureLoaded();
    CachedPointSafetyGridTile persistent_tile;
    bool found_persistent = false;
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
          s_segment_safety_persistent_base_tile_cache.find(key);
      if (it != s_segment_safety_persistent_base_tile_cache.end()) {
        persistent_tile = it->second;
        found_persistent = persistent_tile.built &&
                           persistent_tile.source != kSourcePluginVector &&
                           (!require_depth || persistent_tile.depth_complete);
        if (found_persistent) ++s_segment_safety_persistent_base_tiles_used;
      }
    }
    if (found_persistent &&
        SegmentSafetyTileDependencyIsCurrent(persistent_tile) &&
        SegmentSafetyCachedTileProviderIsCurrent(persistent_tile)) {
      StoreSegmentSafetyGridTile(key, persistent_tile);
      if (stats) ++stats->grid_cache_hits;
      return true;
    }
  }

  CachedPointSafetyGridTile tile = BuildSegmentSafetyGridTile(
      lat_tile * kGridTileDegrees, lon_tile * kGridTileDegrees, lat_tile,
      lon_tile, stats, require_depth);
  StoreSegmentSafetyGridTile(key, tile);
  if (built) *built = true;
  return LookupSegmentSafetyGridTile(key, NULL);
}

void StoreSegmentSafetyRouteMaskTile(
    const std::string& key, const CachedSegmentSafetyRouteMaskTile& tile) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_route_mask_cache.find(key) ==
      s_segment_safety_route_mask_cache.end()) {
    while (s_segment_safety_route_mask_cache.size() >= kMaxGridTiles) {
      std::map<std::string, CachedSegmentSafetyRouteMaskTile>::iterator victim =
          s_segment_safety_route_mask_cache.end();
      for (std::map<std::string, CachedSegmentSafetyRouteMaskTile>::iterator
               it = s_segment_safety_route_mask_cache.begin();
           it != s_segment_safety_route_mask_cache.end(); ++it) {
        if (s_segment_safety_pinned_route_mask_keys.find(it->first) ==
            s_segment_safety_pinned_route_mask_keys.end()) {
          victim = it;
          break;
        }
      }
      if (victim == s_segment_safety_route_mask_cache.end()) break;
      s_segment_safety_route_mask_cache.erase(victim);
    }
  }
  s_segment_safety_route_mask_cache[key] = tile;
}

bool LookupSegmentSafetyRouteMaskTile(const std::string& key,
                                      CachedSegmentSafetyRouteMaskTile* tile) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedSegmentSafetyRouteMaskTile>::const_iterator it =
      s_segment_safety_route_mask_cache.find(key);
  if (it == s_segment_safety_route_mask_cache.end()) return false;
  if (tile) *tile = it->second;
  return true;
}

void StoreSegmentSafetyCoarseRouteMaskCell(
    const std::string& key,
    const CachedSegmentSafetyCoarseRouteMaskCell& cell) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_coarse_route_mask_cache.size() >= kMaxGridTiles)
    s_segment_safety_coarse_route_mask_cache.clear();
  s_segment_safety_coarse_route_mask_cache[key] = cell;
}

bool LookupSegmentSafetyCoarseRouteMaskCell(
    const std::string& key, CachedSegmentSafetyCoarseRouteMaskCell* cell) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>::const_iterator
      it = s_segment_safety_coarse_route_mask_cache.find(key);
  if (it == s_segment_safety_coarse_route_mask_cache.end()) return false;
  if (cell) *cell = it->second;
  return true;
}

bool BuildSegmentSafetyCoarseRouteMaskCellFromFine(
    long coarse_lat_cell, long coarse_lon_cell, double safety_margin_nm,
    bool check_depth, double minimum_depth_m,
    CachedSegmentSafetyCoarseRouteMaskCell* cell,
    SegmentSafetyCoreStats* stats) {
  if (!cell) return false;

  wxStopWatch timer;
  CachedSegmentSafetyCoarseRouteMaskCell coarse;
  coarse.group_index = SegmentSafetyCurrentGroupIndex();
  coarse.lat_cell = coarse_lat_cell;
  coarse.lon_cell = coarse_lon_cell;
  coarse.min_lat = coarse_lat_cell * coarse.degrees;
  coarse.min_lon = coarse_lon_cell * coarse.degrees;
  coarse.check_depth = check_depth;
  coarse.minimum_depth_m = minimum_depth_m;
  coarse.safety_margin_nm = safety_margin_nm;
  coarse.block_summary_flags = kRouteClear;

  bool missing = false;
  bool all_clear = true;
  long start_lat_tile = coarse_lat_cell * kCoarseRouteMaskFactor;
  long start_lon_tile = coarse_lon_cell * kCoarseRouteMaskFactor;

  for (int dlat = 0; dlat < kCoarseRouteMaskFactor; ++dlat) {
    for (int dlon = 0; dlon < kCoarseRouteMaskFactor; ++dlon) {
      long fine_lat_tile = start_lat_tile + dlat;
      long fine_lon_tile = start_lon_tile + dlon;
      std::string mask_key = SegmentSafetyRouteMaskKey(
          fine_lat_tile, fine_lon_tile, safety_margin_nm, check_depth,
          minimum_depth_m);
      CachedSegmentSafetyRouteMaskTile mask;
      if (!LookupSegmentSafetyRouteMaskTile(mask_key, &mask) || !mask.built ||
          mask.block_flags.empty()) {
        missing = true;
        continue;
      }

      ++coarse.fine_tiles_checked;
      if (mask.uses_plugin_vector) {
        coarse.source = kSourcePluginVector;
        coarse.persistent_cache_allowed =
            coarse.persistent_cache_allowed && mask.persistent_cache_allowed;
      } else if (coarse.source == kSourceNone)
        coarse.source = mask.source;
      coarse.block_summary_flags |= mask.block_summary_flags;
      bool tile_clear = mask.block_summary_flags == kRouteClear &&
                        mask.clear_count == mask.rows * mask.cols;
      if (tile_clear) {
        ++coarse.fine_tiles_clear;
      } else {
        ++coarse.fine_tiles_mixed;
        all_clear = false;
      }
    }
  }

  if (missing) {
    coarse.state = kCoarseMissing;
    coarse.block_summary_flags |= kRouteNeedsTile;
    return false;
  }

  if (all_clear) {
    coarse.state = kCoarseCertifiedSafe;
  } else if (coarse.block_summary_flags & kRouteBlockNoChart) {
    coarse.state = kCoarseNoChart;
  } else if (coarse.block_summary_flags & kRouteBlockUnknownDepth) {
    coarse.state = kCoarseDepthUnproven;
  } else {
    coarse.state = kCoarseMixed;
  }

  if (stats) stats->coarse_build_ms += timer.Time();
  *cell = coarse;
  return true;
}

bool EnsureSegmentSafetyCoarseRouteMaskCell(
    long coarse_lat_cell, long coarse_lon_cell, double safety_margin_nm,
    bool check_depth, double minimum_depth_m,
    CachedSegmentSafetyCoarseRouteMaskCell* cell,
    SegmentSafetyCoreStats* stats) {
  std::string key = SegmentSafetyCoarseRouteMaskKey(
      coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
      minimum_depth_m);
  if (LookupSegmentSafetyCoarseRouteMaskCell(key, cell)) return true;

  CachedSegmentSafetyCoarseRouteMaskCell persistent;
  if (SegmentSafetyPersistentLookupCertifiedSafe(
          coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
          minimum_depth_m, &persistent)) {
    StoreSegmentSafetyCoarseRouteMaskCell(key, persistent);
    if (cell) *cell = persistent;
    return true;
  }

  CachedSegmentSafetyCoarseRouteMaskCell built;
  if (!BuildSegmentSafetyCoarseRouteMaskCellFromFine(
          coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
          minimum_depth_m, &built, stats))
    return false;
  StoreSegmentSafetyCoarseRouteMaskCell(key, built);
  SegmentSafetyPersistentStoreCertifiedSafe(built);
  if (cell) *cell = built;
  return true;
}

bool LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
    long lat_tile, long lon_tile, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, CachedSegmentSafetyCoarseRouteMaskCell* cell) {
  long coarse_lat_cell = (long)floor((double)lat_tile / kCoarseRouteMaskFactor);
  long coarse_lon_cell = (long)floor((double)lon_tile / kCoarseRouteMaskFactor);
  std::string key = SegmentSafetyCoarseRouteMaskKey(
      coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
      minimum_depth_m);

  CachedSegmentSafetyCoarseRouteMaskCell coarse;
  if (LookupSegmentSafetyCoarseRouteMaskCell(key, &coarse)) {
    if (coarse.state == kCoarseCertifiedSafe) {
      if (cell) *cell = coarse;
      return true;
    }
  }
  if (SegmentSafetyPersistentLookupCertifiedSafe(
          coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
          minimum_depth_m, &coarse)) {
    StoreSegmentSafetyCoarseRouteMaskCell(key, coarse);
    if (coarse.state == kCoarseCertifiedSafe) {
      if (cell) *cell = coarse;
      return true;
    }
  }
  return false;
}

CachedSegmentSafetyRouteMaskTile BuildPersistentCertifiedSafeRouteMaskTile(
    long lat_tile, long lon_tile, double safety_margin_nm, bool check_depth,
    double minimum_depth_m,
    const CachedSegmentSafetyCoarseRouteMaskCell& coarse) {
  CachedSegmentSafetyRouteMaskTile mask;
  mask.group_index = SegmentSafetyCurrentGroupIndex();
  mask.lat_tile = lat_tile;
  mask.lon_tile = lon_tile;
  mask.min_lat = lat_tile * kGridTileDegrees;
  mask.min_lon = lon_tile * kGridTileDegrees;
  mask.resolution = kGridResolutionDegrees;
  mask.rows = (int)lround(kGridTileDegrees / kGridResolutionDegrees) + 1;
  mask.cols = mask.rows;
  mask.check_depth = check_depth;
  mask.minimum_depth_m = minimum_depth_m;
  mask.safety_margin_nm = safety_margin_nm;
  mask.margin_cells = 0;
  mask.built = true;
  mask.source =
      coarse.source != kSourceNone ? coarse.source : kSourceVectorChart;
  mask.chart_db_index = -1;
  mask.chart_scale = -1;
  snprintf(mask.chart_path, sizeof(mask.chart_path),
           "persistent certified safe coarse cell");
  mask.block_flags.assign(mask.rows * mask.cols, kRouteClear);
  mask.block_summary_flags = kRouteClear;
  mask.clear_count = mask.rows * mask.cols;
  mask.authoritative_fine = false;
  mask.persistent_certified_safe = true;
  mask.uses_plugin_vector = coarse.source == kSourcePluginVector;
  mask.persistent_cache_allowed = coarse.persistent_cache_allowed;
  return mask;
}

uint16_t SegmentSafetyRouteMaskFlagsForBaseCell(
    const CachedPointSafetyGridTile& tile, int cell_index, bool check_depth,
    double minimum_depth_m) {
  uint16_t flags = kRouteClear;
  uint16_t hazards =
      cell_index >= 0 && cell_index < (int)tile.hazard_flags.size()
          ? tile.hazard_flags[cell_index]
          : (uint16_t)kHazardNoChart;

  if (hazards & kHazardLand) flags |= kRouteBlockLand;
  if (hazards & kHazardDrying) flags |= kRouteBlockDrying;
  if (hazards & kHazardNoChart) flags |= kRouteBlockNoChart;
  if (hazards & kHazardUnknownClass) flags |= kRouteBlockUnknownClass;

  if (check_depth) {
    bool has_depth = cell_index >= 0 &&
                     cell_index < (int)tile.has_depth.size() &&
                     tile.has_depth[cell_index] != 0;
    if (!has_depth) {
      if (!(flags & (kRouteBlockLand | kRouteBlockDrying | kRouteBlockNoChart)))
        flags |= kRouteBlockUnknownDepth;
    } else if (cell_index >= 0 && cell_index < (int)tile.min_depth_m.size() &&
               tile.min_depth_m[cell_index] < minimum_depth_m) {
      flags |= kRouteBlockTooShallow;
    }
  }

  return flags;
}

// Load an exact point-safety tile from either the active cache or the
// persistent cache without consulting chart objects.  Coarse certification
// uses this first so that a proof cannot accidentally make sparse route
// footprints more expensive than their fine-mask fallback.
bool LoadSegmentSafetyGridTileWithoutBuilding(long lat_tile, long lon_tile,
                                              CachedPointSafetyGridTile* tile) {
  const std::string key =
      SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
  if (LookupSegmentSafetyGridTile(key, tile)) return true;
  CachedPointSafetyGridTile external;
  if (SegmentSafetyExternalTileCacheLookup(lat_tile, lon_tile, false,
                                           &external)) {
    StoreSegmentSafetyGridTile(key, external);
    if (tile) *tile = external;
    return true;
  }
  if (!s_segment_safety_persistent_cache_enabled) return false;

  SegmentSafetyPersistentCacheEnsureLoaded();
  CachedPointSafetyGridTile persistent;
  bool found = false;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
        s_segment_safety_persistent_base_tile_cache.find(key);
    if (it != s_segment_safety_persistent_base_tile_cache.end() &&
        it->second.built && it->second.source != kSourcePluginVector) {
      persistent = it->second;
      found = true;
      ++s_segment_safety_persistent_base_tiles_used;
    }
  }
  if (!found) return false;
  StoreSegmentSafetyGridTile(key, persistent);
  if (tile) *tile = persistent;
  return true;
}

bool SegmentSafetyBaseTileHasClearTargetCells(
    const CachedPointSafetyGridTile& tile, bool check_depth,
    double minimum_depth_m, uint32_t* summary_flags) {
  bool clear = tile.built && !tile.classes.empty();
  if (!clear) {
    if (summary_flags) *summary_flags |= kRouteNeedsTile;
    return false;
  }

  for (int i = 0; i < tile.rows * tile.cols; ++i) {
    const uint16_t flags = SegmentSafetyRouteMaskFlagsForBaseCell(
        tile, i, check_depth, minimum_depth_m);
    if (summary_flags) *summary_flags |= flags;
    if (flags != kRouteClear) clear = false;
  }
  return clear;
}

bool SegmentSafetyBaseTileCanContributeMargin(
    const CachedPointSafetyGridTile& tile, bool check_depth,
    double minimum_depth_m) {
  if (!tile.built || tile.classes.empty()) return false;
  if (tile.hazard_summary_flags & (kHazardLand | kHazardDrying)) return true;
  if (!check_depth) return false;

  for (int i = 0; i < tile.rows * tile.cols; ++i) {
    const uint16_t flags =
        SegmentSafetyRouteMaskFlagsForBaseCell(tile, i, true, minimum_depth_m);
    if (flags & kRouteBlockTooShallow) return true;
  }
  return false;
}

// Build a conservative coarse certificate directly from the exact base-grid
// evidence.  A cell is certified only when every target cell is clear and the
// complete tile halo which can influence the configured margin contains no
// land, drying or too-shallow cell.  Missing evidence never becomes a proof.
bool BuildSegmentSafetyCoarseRouteMaskCellFromBase(
    long coarse_lat_cell, long coarse_lon_cell, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, bool allow_chart_builds,
    CachedSegmentSafetyCoarseRouteMaskCell* cell, SegmentSafetyCoreStats* stats,
    long* base_tiles_built) {
  if (!cell || !wxThread::IsMain()) return false;

  wxStopWatch timer;
  CachedSegmentSafetyCoarseRouteMaskCell coarse;
  coarse.group_index = SegmentSafetyCurrentGroupIndex();
  coarse.lat_cell = coarse_lat_cell;
  coarse.lon_cell = coarse_lon_cell;
  coarse.min_lat = coarse_lat_cell * coarse.degrees;
  coarse.min_lon = coarse_lon_cell * coarse.degrees;
  coarse.check_depth = check_depth;
  coarse.minimum_depth_m = minimum_depth_m;
  coarse.safety_margin_nm = safety_margin_nm;
  coarse.block_summary_flags = kRouteClear;

  const long start_lat_tile = coarse_lat_cell * kCoarseRouteMaskFactor;
  const long start_lon_tile = coarse_lon_cell * kCoarseRouteMaskFactor;
  const double min_cell_lat = start_lat_tile * kGridTileDegrees;
  const double max_cell_lat =
      (start_lat_tile + kCoarseRouteMaskFactor) * kGridTileDegrees;
  const int margin_radius = SegmentSafetyMarginTileRadius(
      safety_margin_nm, wxMax(fabs(min_cell_lat), fabs(max_cell_lat)));

  bool all_targets_clear = true;
  bool margin_hit = false;
  for (int dlat = -margin_radius; dlat < kCoarseRouteMaskFactor + margin_radius;
       ++dlat) {
    for (int dlon = -margin_radius;
         dlon < kCoarseRouteMaskFactor + margin_radius; ++dlon) {
      const long lat_tile = start_lat_tile + dlat;
      const long lon_tile = start_lon_tile + dlon;
      CachedPointSafetyGridTile base;
      bool available =
          LoadSegmentSafetyGridTileWithoutBuilding(lat_tile, lon_tile, &base);
      if (!available && allow_chart_builds) {
        bool built = false;
        available =
            EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &built,
                                        check_depth) &&
            LookupSegmentSafetyGridTile(
                SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile), &base);
        if (built && base_tiles_built) ++*base_tiles_built;
      }
      if (!available) return false;

      if (base.source == kSourcePluginVector) {
        coarse.source = kSourcePluginVector;
        coarse.persistent_cache_allowed =
            coarse.persistent_cache_allowed && base.persistent_cache_allowed;
      }

      const bool target = dlat >= 0 && dlat < kCoarseRouteMaskFactor &&
                          dlon >= 0 && dlon < kCoarseRouteMaskFactor;
      if (target) {
        ++coarse.fine_tiles_checked;
        if (coarse.source == kSourceNone) coarse.source = base.source;
        if (SegmentSafetyBaseTileHasClearTargetCells(
                base, check_depth, minimum_depth_m,
                &coarse.block_summary_flags)) {
          ++coarse.fine_tiles_clear;
        } else {
          ++coarse.fine_tiles_mixed;
          all_targets_clear = false;
        }
      }

      if (margin_radius > 0 && SegmentSafetyBaseTileCanContributeMargin(
                                   base, check_depth, minimum_depth_m))
        margin_hit = true;
    }
  }

  if (margin_hit) {
    coarse.block_summary_flags |= kRouteBlockMargin;
    all_targets_clear = false;
  }

  if (all_targets_clear) {
    coarse.state = kCoarseCertifiedSafe;
  } else if (coarse.block_summary_flags & kRouteBlockNoChart) {
    coarse.state = kCoarseNoChart;
  } else if (coarse.block_summary_flags & kRouteBlockUnknownDepth) {
    coarse.state = kCoarseDepthUnproven;
  } else {
    coarse.state = kCoarseMixed;
  }

  if (stats) stats->coarse_build_ms += timer.Time();
  *cell = coarse;
  return true;
}

void AddSegmentSafetyTileHalo(long lat_tile, long lon_tile, int radius,
                              std::set<std::pair<long, long> >* tiles) {
  if (!tiles) return;
  for (int dlat = -radius; dlat <= radius; ++dlat) {
    for (int dlon = -radius; dlon <= radius; ++dlon)
      tiles->insert(std::make_pair(lat_tile + dlat, lon_tile + dlon));
  }
}

long CountSegmentSafetyTilesUnavailableWithoutBuilding(
    const std::set<std::pair<long, long> >& tiles) {
  long missing = 0;
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    if (!LoadSegmentSafetyGridTileWithoutBuilding(it->first, it->second, NULL))
      ++missing;
  }
  return missing;
}

// Compare the chart-object work required by a coarse proof with the work the
// fine fallback is already committed to for the occupied tiles.  This makes
// coarse-first deterministic and prevents it from widening sparse corridors
// merely to obtain a performance certificate.
bool SegmentSafetyCoarseProofDoesNotExpandBaseBuildSet(
    long coarse_lat_cell, long coarse_lon_cell,
    const std::set<std::pair<long, long> >& occupied_tiles,
    double safety_margin_nm) {
  const long start_lat_tile = coarse_lat_cell * kCoarseRouteMaskFactor;
  const long start_lon_tile = coarse_lon_cell * kCoarseRouteMaskFactor;
  const double min_cell_lat = start_lat_tile * kGridTileDegrees;
  const double max_cell_lat =
      (start_lat_tile + kCoarseRouteMaskFactor) * kGridTileDegrees;
  const int coarse_radius = SegmentSafetyMarginTileRadius(
      safety_margin_nm, wxMax(fabs(min_cell_lat), fabs(max_cell_lat)));

  std::set<std::pair<long, long> > coarse_required;
  for (int dlat = 0; dlat < kCoarseRouteMaskFactor; ++dlat) {
    for (int dlon = 0; dlon < kCoarseRouteMaskFactor; ++dlon) {
      AddSegmentSafetyTileHalo(start_lat_tile + dlat, start_lon_tile + dlon,
                               coarse_radius, &coarse_required);
    }
  }

  std::set<std::pair<long, long> > fine_required;
  for (std::set<std::pair<long, long> >::const_iterator it =
           occupied_tiles.begin();
       it != occupied_tiles.end(); ++it) {
    const double tile_min_lat = it->first * kGridTileDegrees;
    const int fine_radius = SegmentSafetyMarginTileRadius(
        safety_margin_nm,
        wxMax(fabs(tile_min_lat), fabs(tile_min_lat + kGridTileDegrees)));
    AddSegmentSafetyTileHalo(it->first, it->second, fine_radius,
                             &fine_required);
  }

  return CountSegmentSafetyTilesUnavailableWithoutBuilding(coarse_required) <=
         CountSegmentSafetyTilesUnavailableWithoutBuilding(fine_required);
}

CachedSegmentSafetyRouteMaskTile BuildSegmentSafetyRouteMaskTile(
    long lat_tile, long lon_tile, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, SegmentSafetyCoreStats* stats) {
  CachedSegmentSafetyRouteMaskTile mask;
  if (!wxThread::IsMain()) {
    RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    return mask;
  }

  wxStopWatch timer;
  CachedPointSafetyGridTile base_tile;
  std::string base_key = SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
  bool base_built = false;
  if (!LookupSegmentSafetyGridTile(base_key, &base_tile) ||
      (check_depth && !base_tile.depth_complete)) {
    if (!EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &base_built,
                                     check_depth))
      return mask;
    LookupSegmentSafetyGridTile(base_key, &base_tile);
  }
  if (base_tile.classes.empty()) return mask;

  mask.group_index = SegmentSafetyCurrentGroupIndex();
  mask.lat_tile = lat_tile;
  mask.lon_tile = lon_tile;
  mask.min_lat = base_tile.min_lat;
  mask.min_lon = base_tile.min_lon;
  mask.resolution = base_tile.resolution;
  mask.rows = base_tile.rows;
  mask.cols = base_tile.cols;
  mask.check_depth = check_depth;
  mask.minimum_depth_m = minimum_depth_m;
  mask.safety_margin_nm = safety_margin_nm;
  mask.built = true;
  mask.authoritative_fine = true;
  mask.persistent_certified_safe = false;
  mask.uses_plugin_vector = base_tile.source == kSourcePluginVector;
  mask.persistent_cache_allowed =
      !mask.uses_plugin_vector || base_tile.persistent_cache_allowed;
  mask.source = base_tile.source;
  mask.chart_db_index = base_tile.chart_db_index;
  mask.chart_scale = base_tile.chart_scale;
  snprintf(mask.chart_path, sizeof(mask.chart_path), "%s",
           base_tile.chart_path);
  mask.block_flags.assign(mask.rows * mask.cols, kRouteNeedsTile);

  double mid_lat = mask.min_lat + kGridTileDegrees / 2.0;
  double cell_nm =
      wxMin(mask.resolution * 60.0,
            mask.resolution * 60.0 *
                wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mid_lat)))));
  mask.margin_cells = safety_margin_nm > 0.0
                          ? (int)ceil(safety_margin_nm / wxMax(0.01, cell_nm))
                          : 0;
  mask.margin_cells = wxMin(mask.margin_cells, 128);

  auto CountRouteMaskFlags = [&mask](uint16_t flags) {
    mask.block_summary_flags |= flags;
    if (flags == kRouteClear) ++mask.clear_count;
    if (flags & kRouteBlockLand) ++mask.land_count;
    if (flags & kRouteBlockDrying) ++mask.drying_count;
    if (flags & kRouteBlockTooShallow) ++mask.shallow_count;
    if (flags & kRouteBlockUnknownDepth) ++mask.unknown_depth_count;
    if (flags & kRouteBlockNoChart) ++mask.no_chart_count;
    if (flags & kRouteBlockMargin) ++mask.margin_count;
  };

  // Most route checks are open-water tiles.  If this tile has no chart-derived
  // hazards and no adjacent land/drying tile can contribute margin, publish an
  // all-clear derived mask without scanning every neighbouring cell.
  bool used_clear_tile_shortcut = false;
  int prefix_verify_cells = 0;
  int prefix_verify_mismatches = 0;
  bool can_mark_tile_clear =
      !check_depth && base_tile.hazard_summary_flags == kHazardNone;
  if (can_mark_tile_clear && mask.margin_cells > 0) {
    int lat_tile_radius =
        wxMax(1, (int)ceil((double)mask.margin_cells / wxMax(1, mask.rows)));
    int lon_tile_radius =
        wxMax(1, (int)ceil((double)mask.margin_cells / wxMax(1, mask.cols)));
    for (int dlat = -lat_tile_radius;
         dlat <= lat_tile_radius && can_mark_tile_clear; ++dlat) {
      for (int dlon = -lon_tile_radius; dlon <= lon_tile_radius; ++dlon) {
        long neighbor_lat_tile = lat_tile + dlat;
        long neighbor_lon_tile = lon_tile + dlon;
        std::string neighbor_key = SegmentSafetyGridTileKeyForIndices(
            neighbor_lat_tile, neighbor_lon_tile);
        CachedPointSafetyGridTile neighbor_tile;
        bool neighbor_built = false;
        if (!LookupSegmentSafetyGridTile(neighbor_key, &neighbor_tile) ||
            (check_depth && !neighbor_tile.depth_complete)) {
          if (!EnsureSegmentSafetyGridTile(neighbor_lat_tile, neighbor_lon_tile,
                                           stats, &neighbor_built,
                                           check_depth) ||
              !LookupSegmentSafetyGridTile(neighbor_key, &neighbor_tile)) {
            can_mark_tile_clear = false;
            break;
          }
          if (neighbor_built)
            RecordUnexpectedSegmentSafetyTileBuild(stats, neighbor_lat_tile,
                                                   neighbor_lon_tile);
        }

        if (neighbor_tile.hazard_summary_flags &
            (kHazardLand | kHazardDrying)) {
          can_mark_tile_clear = false;
          break;
        }
        if (neighbor_tile.source == kSourcePluginVector)
          mask.uses_plugin_vector = true;
      }
    }
  }

  if (can_mark_tile_clear) {
    used_clear_tile_shortcut = true;
    std::fill(mask.block_flags.begin(), mask.block_flags.end(), kRouteClear);
    mask.clear_count = mask.rows * mask.cols;
  }

  if (!used_clear_tile_shortcut) {
    if (mask.margin_cells == 0) {
      for (int r = 0; r < mask.rows; ++r) {
        for (int c = 0; c < mask.cols; ++c) {
          const int cell_index = r * mask.cols + c;
          const uint16_t flags = SegmentSafetyRouteMaskFlagsForBaseCell(
              base_tile, cell_index, check_depth, minimum_depth_m);
          mask.block_flags[cell_index] = flags;
          CountRouteMaskFlags(flags);
        }
      }
    } else {
      // Materialize the complete halo once, then use summed-area tables for
      // conservative square-margin dilation.  The previous implementation did
      // a cache lookup (and mutex acquisition) for every neighbour of every
      // target cell: about 600,000 lookups per tile at a 0.4 NM margin.  This
      // is exactly the same square-neighbourhood safety rule in O(cells + halo)
      // time, with missing evidence still failing closed.
      const int halo = mask.margin_cells;
      const int extended_rows = mask.rows + 2 * halo;
      const int extended_cols = mask.cols + 2 * halo;
      const int cells_per_tile =
          (int)lround(kGridTileDegrees / kGridResolutionDegrees);
      const long first_lat_cell =
          lround(mask.min_lat / kGridResolutionDegrees) - halo;
      const long first_lon_cell =
          lround(mask.min_lon / kGridResolutionDegrees) - halo;
      std::vector<uint16_t> extended_flags(
          static_cast<size_t>(extended_rows) * extended_cols, kRouteNeedsTile);
      std::map<std::pair<long, long>, CachedPointSafetyGridTile> local_tiles;

      const auto FloorTileIndex = [cells_per_tile](long cell) {
        return (long)floor((double)cell / cells_per_tile);
      };
      for (int r = 0; r < extended_rows; ++r) {
        const long global_lat_cell = first_lat_cell + r;
        const long source_lat_tile = FloorTileIndex(global_lat_cell);
        const int source_row =
            (int)(global_lat_cell - source_lat_tile * cells_per_tile);
        for (int c = 0; c < extended_cols; ++c) {
          const long global_lon_cell = first_lon_cell + c;
          const long source_lon_tile = FloorTileIndex(global_lon_cell);
          const int source_col =
              (int)(global_lon_cell - source_lon_tile * cells_per_tile);
          const std::pair<long, long> tile_id(source_lat_tile, source_lon_tile);
          std::map<std::pair<long, long>, CachedPointSafetyGridTile>::iterator
              local = local_tiles.find(tile_id);
          if (local == local_tiles.end()) {
            const std::string source_key = SegmentSafetyGridTileKeyForIndices(
                source_lat_tile, source_lon_tile);
            CachedPointSafetyGridTile source_tile;
            bool source_built = false;
            if ((!LookupSegmentSafetyGridTile(source_key, &source_tile) ||
                 (check_depth && !source_tile.depth_complete)) &&
                EnsureSegmentSafetyGridTile(source_lat_tile, source_lon_tile,
                                            stats, &source_built, check_depth))
              LookupSegmentSafetyGridTile(source_key, &source_tile);
            if (source_built)
              RecordUnexpectedSegmentSafetyTileBuild(stats, source_lat_tile,
                                                     source_lon_tile);
            local =
                local_tiles.insert(std::make_pair(tile_id, source_tile)).first;
          }

          const CachedPointSafetyGridTile& source_tile = local->second;
          if (source_tile.source == kSourcePluginVector)
            mask.uses_plugin_vector = true;
          if (!source_tile.built || source_tile.classes.empty() ||
              source_row < 0 || source_row >= source_tile.rows ||
              source_col < 0 || source_col >= source_tile.cols)
            continue;
          const int source_index = source_row * source_tile.cols + source_col;
          extended_flags[static_cast<size_t>(r) * extended_cols + c] =
              SegmentSafetyRouteMaskFlagsForBaseCell(
                  source_tile, source_index, check_depth, minimum_depth_m);
        }
      }

      const int prefix_stride = extended_cols + 1;
      std::vector<int> hazard_prefix(
          static_cast<size_t>(extended_rows + 1) * prefix_stride, 0);
      std::vector<int> missing_prefix(
          static_cast<size_t>(extended_rows + 1) * prefix_stride, 0);
      for (int r = 0; r < extended_rows; ++r) {
        int hazard_row_sum = 0;
        int missing_row_sum = 0;
        for (int c = 0; c < extended_cols; ++c) {
          const uint16_t flags =
              extended_flags[static_cast<size_t>(r) * extended_cols + c];
          hazard_row_sum += (flags & (kRouteBlockLand | kRouteBlockDrying |
                                      kRouteBlockTooShallow))
                                ? 1
                                : 0;
          missing_row_sum += (flags & kRouteNeedsTile) ? 1 : 0;
          const size_t prefix_index =
              static_cast<size_t>(r + 1) * prefix_stride + c + 1;
          hazard_prefix[prefix_index] =
              hazard_prefix[static_cast<size_t>(r) * prefix_stride + c + 1] +
              hazard_row_sum;
          missing_prefix[prefix_index] =
              missing_prefix[static_cast<size_t>(r) * prefix_stride + c + 1] +
              missing_row_sum;
        }
      }
      const auto RectangleSum = [prefix_stride](const std::vector<int>& prefix,
                                                int min_row, int min_col,
                                                int max_row, int max_col) {
        const size_t a = static_cast<size_t>(min_row) * prefix_stride + min_col;
        const size_t b =
            static_cast<size_t>(min_row) * prefix_stride + max_col + 1;
        const size_t c =
            static_cast<size_t>(max_row + 1) * prefix_stride + min_col;
        const size_t d =
            static_cast<size_t>(max_row + 1) * prefix_stride + max_col + 1;
        return prefix[d] - prefix[b] - prefix[c] + prefix[a];
      };

      for (int r = 0; r < mask.rows; ++r) {
        for (int c = 0; c < mask.cols; ++c) {
          const int extended_row = r + halo;
          const int extended_col = c + halo;
          const int cell_index = r * mask.cols + c;
          uint16_t flags = SegmentSafetyRouteMaskFlagsForBaseCell(
              base_tile, cell_index, check_depth, minimum_depth_m);
          const int min_row = extended_row - halo;
          const int min_col = extended_col - halo;
          const int max_row = extended_row + halo;
          const int max_col = extended_col + halo;
          const uint16_t global_centre_flags =
              extended_flags[static_cast<size_t>(extended_row) * extended_cols +
                             extended_col];
          const int neighbour_missing =
              RectangleSum(missing_prefix, min_row, min_col, max_row, max_col) -
              ((global_centre_flags & kRouteNeedsTile) ? 1 : 0);
          if (neighbour_missing > 0) {
            flags |= kRouteNeedsTile;
          } else {
            const int neighbour_hazards =
                RectangleSum(hazard_prefix, min_row, min_col, max_row,
                             max_col) -
                ((global_centre_flags &
                  (kRouteBlockLand | kRouteBlockDrying | kRouteBlockTooShallow))
                     ? 1
                     : 0);
            if (neighbour_hazards > 0) flags |= kRouteBlockMargin;
          }

          mask.block_flags[cell_index] = flags;
          CountRouteMaskFlags(flags);
        }
      }

      const char* verify_prefix =
          getenv("WR_SEGMENT_SAFETY_VERIFY_MASK_PREFIX");
      if (verify_prefix && !strcmp(verify_prefix, "1")) {
        for (int r = 0; r < mask.rows; ++r) {
          for (int c = 0; c < mask.cols; ++c) {
            const int cell_index = r * mask.cols + c;
            uint16_t reference_flags = SegmentSafetyRouteMaskFlagsForBaseCell(
                base_tile, cell_index, check_depth, minimum_depth_m);
            bool margin_hit = false;
            for (int dlat = -halo; dlat <= halo && !margin_hit; ++dlat) {
              for (int dlon = -halo; dlon <= halo; ++dlon) {
                if (dlat == 0 && dlon == 0) continue;
                const uint16_t neighbour_flags =
                    extended_flags[static_cast<size_t>(r + halo + dlat) *
                                       extended_cols +
                                   c + halo + dlon];
                if (neighbour_flags & kRouteNeedsTile) {
                  reference_flags |= kRouteNeedsTile;
                  margin_hit = true;
                  break;
                }
                if (neighbour_flags & (kRouteBlockLand | kRouteBlockDrying |
                                       kRouteBlockTooShallow)) {
                  margin_hit = true;
                  break;
                }
              }
            }
            if (margin_hit && !(reference_flags & kRouteNeedsTile))
              reference_flags |= kRouteBlockMargin;
            ++prefix_verify_cells;
            if (reference_flags != mask.block_flags[cell_index])
              ++prefix_verify_mismatches;
          }
        }
        if (prefix_verify_mismatches > 0) mask.built = false;
      }
    }
  } else {
    CountRouteMaskFlags(kRouteClear);
    mask.clear_count = mask.rows * mask.cols;
  }

  wxLogMessage(
      "WR_ROUTE_MASK_BUILD key=%ld:%ld group=%d margin_nm=%.3f "
      "depth_check=%d min_depth_m=%.2f cells=%d clear=%d land=%d drying=%d "
      "shallow=%d unknown_depth=%d no_chart=%d margin=%d summary_flags=%u "
      "base_built=%d clear_shortcut=%d prefix_verify_cells=%d "
      "prefix_verify_mismatches=%d build_ms=%d",
      lat_tile, lon_tile, mask.group_index, safety_margin_nm,
      check_depth ? 1 : 0, minimum_depth_m, mask.rows * mask.cols,
      mask.clear_count, mask.land_count, mask.drying_count, mask.shallow_count,
      mask.unknown_depth_count, mask.no_chart_count, mask.margin_count,
      mask.block_summary_flags, base_built ? 1 : 0,
      used_clear_tile_shortcut ? 1 : 0, prefix_verify_cells,
      prefix_verify_mismatches, timer.Time());
  return mask;
}

void QueueSegmentSafetyRouteMaskRequest(long lat_tile, long lon_tile,
                                        double safety_margin_nm,
                                        bool check_depth,
                                        double minimum_depth_m,
                                        bool force_authoritative_fine) {
  SegmentSafetyRouteMaskRequest request;
  request.key = SegmentSafetyRouteMaskKey(lat_tile, lon_tile, safety_margin_nm,
                                          check_depth, minimum_depth_m);
  request.group_index = SegmentSafetyCurrentGroupIndex();
  request.lat_tile = lat_tile;
  request.lon_tile = lon_tile;
  request.safety_margin_nm = safety_margin_nm;
  request.check_depth = check_depth;
  request.minimum_depth_m = minimum_depth_m;
  request.force_authoritative_fine = force_authoritative_fine;

  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_inflight_route_mask_requests.count(request.key) == 0)
    s_segment_safety_pending_route_mask_requests[request.key] = request;
}

bool EnsureSegmentSafetyRouteMaskTile(long lat_tile, long lon_tile,
                                      double safety_margin_nm, bool check_depth,
                                      double minimum_depth_m,
                                      SegmentSafetyCoreStats* stats,
                                      bool* built,
                                      bool force_authoritative_fine) {
  if (built) *built = false;
  std::string key = SegmentSafetyRouteMaskKey(
      lat_tile, lon_tile, safety_margin_nm, check_depth, minimum_depth_m);
  CachedSegmentSafetyRouteMaskTile existing;
  if (LookupSegmentSafetyRouteMaskTile(key, &existing) &&
      (!force_authoritative_fine || existing.authoritative_fine)) {
    if (stats) ++stats->grid_cache_hits;
    return true;
  }

  if (stats) ++stats->grid_cache_misses;
  if (!wxThread::IsMain()) {
    RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    QueueSegmentSafetyRouteMaskRequest(lat_tile, lon_tile, safety_margin_nm,
                                       check_depth, minimum_depth_m,
                                       force_authoritative_fine);
    return false;
  }

  CachedSegmentSafetyRouteMaskTile mask =
      BuildSegmentSafetyRouteMaskTile(lat_tile, lon_tile, safety_margin_nm,
                                      check_depth, minimum_depth_m, stats);
  if (!mask.built) return false;
  StoreSegmentSafetyRouteMaskTile(key, mask);
  if (built) *built = true;
  return true;
}

void RecordUnexpectedSegmentSafetyTileBuild(SegmentSafetyCoreStats* stats,
                                            long lat_tile, long lon_tile) {
  if (!stats) return;
  ++stats->unexpected_tile_builds;
  if (stats->unexpected_tile_builds == 1) {
    stats->unexpected_lat_tile = lat_tile;
    stats->unexpected_lon_tile = lon_tile;
    stats->unexpected_tile_min_lat = lat_tile * kGridTileDegrees;
    stats->unexpected_tile_min_lon = lon_tile * kGridTileDegrees;
  }
}

}  // namespace detail

}  // namespace ocpn::chart_safety
