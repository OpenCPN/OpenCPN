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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * ocpn_plugin.h HostApi123 chart-safety implementation
 */

#include <wx/app.h>

#include "ocpn_plugin.h"

#include "chart_safety_api.h"
#include "navigation_safety.h"
#include "pluginmanager.h"

extern PlugInManager* g_pi_manager;

namespace {

class HostApi123Impl final : public HostApi123 {
public:
  explicit HostApi123Impl(Api122Impl* api_impl) : HostApi123(api_impl) {}

  bool RegisterChartSafetyProvider(
      const std::string& plugin_name,
      const ChartSafetyProviderCallbacks* callbacks) override;
  bool RegisterSegmentSafetyTileCache(
      const std::string& plugin_name,
      const SegmentSafetyTileCacheCallbacks* callbacks) override;
  bool GetSegmentSafetyChartIdentity(std::string* identity) override;
  int GetSegmentSafetyChartInfoCount() override;
  bool GetSegmentSafetyChartInfo(int ordinal,
                                 SegmentSafetyChartInfo* chart_info) override;
  bool GetSegmentSafetyChartCoverageTiles(const int* chart_db_indices,
                                          int chart_count, double tile_degrees,
                                          long* lat_tiles, long* lon_tiles,
                                          int tile_capacity, int* tile_count,
                                          bool* complete) override;
  bool CheckSegmentSafety(double lat1, double lon1, double lat2, double lon2,
                          const SegmentSafetyOptions* options,
                          SegmentSafetyResult* result) override;
  bool CheckNavigationRoute(
      const NavigationSafetyProfile& profile,
      const std::vector<NavigationSafetyPosition>& positions,
      NavigationRouteSafetyResult* result) override;
  bool PrepareSegmentSafetyTiles(const long* lat_tiles, const long* lon_tiles,
                                 int tile_count, bool require_depth,
                                 SegmentSafetyResult* result) override;
  bool PrepareSegmentSafetySnapshot(double min_lat, double min_lon,
                                    double max_lat, double max_lon,
                                    bool enable_fast_path, bool shadow_compare,
                                    const SegmentSafetyOptions* options,
                                    SegmentSafetyResult* result) override;
  bool PrepareSegmentSafetyCorridor(const double* latitudes,
                                    const double* longitudes,
                                    const int* point_counts, int polyline_count,
                                    double corridor_margin_nm,
                                    int fine_tile_halo,
                                    const SegmentSafetyOptions* options,
                                    SegmentSafetyResult* result) override;
  int GetPendingSegmentSafetyRequestCount() override;
  bool ServicePendingSegmentSafetyRequests(
      int max_requests, int max_milliseconds,
      SegmentSafetyRequestServiceResult* result) override;
  void ReleaseSegmentSafetyPins() override;
  bool SetSegmentSafetyPersistentCacheEnabled(bool enabled) override;
  bool GetSegmentSafetyPersistentCacheEnabled() override;
  bool SaveSegmentSafetyPersistentCache() override;
  bool ClearSegmentSafetyPersistentCache() override;
};

}  // namespace

std::unique_ptr<HostApi> ocpn::chart_safety::MakeHostApi(Api122Impl* support) {
  return std::make_unique<HostApi123Impl>(support);
}

bool HostApi123Impl::RegisterChartSafetyProvider(
    const std::string& plugin_name,
    const ChartSafetyProviderCallbacks* callbacks) {
  return g_pi_manager &&
         g_pi_manager->RegisterChartSafetyProvider(plugin_name, callbacks);
}

bool HostApi123Impl::RegisterSegmentSafetyTileCache(
    const std::string& plugin_name,
    const SegmentSafetyTileCacheCallbacks* callbacks) {
  return g_pi_manager &&
         g_pi_manager->RegisterSegmentSafetyTileCache(plugin_name, callbacks);
}

bool HostApi123Impl::GetSegmentSafetyChartIdentity(std::string* identity) {
  return ocpn::chart_safety::GetChartIdentity(identity);
}

int HostApi123Impl::GetSegmentSafetyChartInfoCount() {
  return ocpn::chart_safety::GetChartInfoCount();
}

bool HostApi123Impl::GetSegmentSafetyChartInfo(
    int ordinal, SegmentSafetyChartInfo* chart_info) {
  return ocpn::chart_safety::GetChartInfo(ordinal, chart_info);
}

bool HostApi123Impl::GetSegmentSafetyChartCoverageTiles(
    const int* chart_db_indices, int chart_count, double tile_degrees,
    long* lat_tiles, long* lon_tiles, int tile_capacity, int* tile_count,
    bool* complete) {
  return ocpn::chart_safety::GetChartCoverageTiles(
      chart_db_indices, chart_count, tile_degrees, lat_tiles, lon_tiles,
      tile_capacity, tile_count, complete);
}

bool HostApi123Impl::CheckSegmentSafety(double lat1, double lon1, double lat2,
                                        double lon2,
                                        const SegmentSafetyOptions* options,
                                        SegmentSafetyResult* result) {
  return ocpn::chart_safety::CheckSegment(lat1, lon1, lat2, lon2, options,
                                          result);
}

bool HostApi123Impl::PrepareSegmentSafetyTiles(const long* lat_tiles,
                                               const long* lon_tiles,
                                               int tile_count,
                                               bool require_depth,
                                               SegmentSafetyResult* result) {
  return ocpn::chart_safety::PrepareRawTiles(lat_tiles, lon_tiles, tile_count,
                                             require_depth, result);
}

bool HostApi123Impl::PrepareSegmentSafetySnapshot(
    double min_lat, double min_lon, double max_lat, double max_lon,
    bool enable_fast_path, bool shadow_compare,
    const SegmentSafetyOptions* options, SegmentSafetyResult* result) {
  return ocpn::chart_safety::PrepareHazardSnapshot(
      min_lat, min_lon, max_lat, max_lon, enable_fast_path, shadow_compare,
      options, result);
}

bool HostApi123Impl::PrepareSegmentSafetyCorridor(
    const double* latitudes, const double* longitudes, const int* point_counts,
    int polyline_count, double corridor_margin_nm, int fine_tile_halo,
    const SegmentSafetyOptions* options, SegmentSafetyResult* result) {
  return ocpn::chart_safety::PrepareCorridor(
      latitudes, longitudes, point_counts, polyline_count, corridor_margin_nm,
      fine_tile_halo, options, result);
}

int HostApi123Impl::GetPendingSegmentSafetyRequestCount() {
  return ocpn::chart_safety::GetPendingRequestCount();
}

bool HostApi123Impl::ServicePendingSegmentSafetyRequests(
    int max_requests, int max_milliseconds,
    SegmentSafetyRequestServiceResult* result) {
  return ocpn::chart_safety::ServicePendingRequests(max_requests,
                                                    max_milliseconds, result);
}

void HostApi123Impl::ReleaseSegmentSafetyPins() {
  ocpn::chart_safety::ReleasePins();
}

bool HostApi123Impl::SetSegmentSafetyPersistentCacheEnabled(bool enabled) {
  return ocpn::chart_safety::SetPersistentCacheEnabled(enabled);
}

bool HostApi123Impl::GetSegmentSafetyPersistentCacheEnabled() {
  return ocpn::chart_safety::GetPersistentCacheEnabled();
}

bool HostApi123Impl::SaveSegmentSafetyPersistentCache() {
  return ocpn::chart_safety::SavePersistentCache();
}

bool HostApi123Impl::ClearSegmentSafetyPersistentCache() {
  return ocpn::chart_safety::ClearPersistentCache();
}

bool HostApi123Impl::CheckNavigationRoute(
    const NavigationSafetyProfile& profile,
    const std::vector<NavigationSafetyPosition>& positions,
    NavigationRouteSafetyResult* result) {
  return ocpn::chart_safety::CheckNavigationRoute(
      profile, positions, ocpn::chart_safety::CheckSegment, result);
}
