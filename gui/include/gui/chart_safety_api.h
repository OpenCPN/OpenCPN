/***************************************************************************
 * Internal chart-safety service declarations.                             *
 ***************************************************************************/

#ifndef GUI_CHART_SAFETY_API_H_
#define GUI_CHART_SAFETY_API_H_

#include <string>

#include "ocpn_plugin.h"

namespace ocpn::chart_safety {

std::unique_ptr<HostApi> MakeHostApi(Api122Impl *support);

using SegmentSafetyStatus = HostApi123::SegmentSafetyStatus;
using SegmentSafetySource = HostApi123::SegmentSafetySource;
using SegmentSafetyDiagnosticReason = HostApi123::SegmentSafetyDiagnosticReason;
using SegmentSafetyHitCause = HostApi123::SegmentSafetyHitCause;
using SegmentSafetyOptions = HostApi123::SegmentSafetyOptions;
using SegmentSafetyResult = HostApi123::SegmentSafetyResult;
using SegmentSafetyRequestServiceResult =
    HostApi123::SegmentSafetyRequestServiceResult;
using SegmentSafetyTile = HostApi123::SegmentSafetyTile;
using SegmentSafetyTileCacheLookup = HostApi123::SegmentSafetyTileCacheLookup;
using SegmentSafetyTileCacheStore = HostApi123::SegmentSafetyTileCacheStore;
using SegmentSafetyTileCacheIdentity =
    HostApi123::SegmentSafetyTileCacheIdentity;
using SegmentSafetyTileCacheDependenciesChanged =
    HostApi123::SegmentSafetyTileCacheDependenciesChanged;
using SegmentSafetyTileCacheCallbacks =
    HostApi123::SegmentSafetyTileCacheCallbacks;
using SegmentSafetyChartInfo = HostApi123::SegmentSafetyChartInfo;
using ChartSafetyFeature = HostApi123::ChartSafetyFeature;

constexpr auto kSafe = HostApi123::kSegmentSafetySafe;
constexpr auto kCrossesLand = HostApi123::kSegmentSafetyCrossesLand;
constexpr auto kWithinLandMargin = HostApi123::kSegmentSafetyWithinLandMargin;
constexpr auto kUnsafeArea = HostApi123::kSegmentSafetyUnsafeArea;
constexpr auto kNoData = HostApi123::kSegmentSafetyNoData;
constexpr auto kError = HostApi123::kSegmentSafetyError;
constexpr auto kDryingArea = HostApi123::kSegmentSafetyDryingArea;
constexpr auto kTooShallow = HostApi123::kSegmentSafetyTooShallow;
constexpr auto kUnknownDepth = HostApi123::kSegmentSafetyUnknownDepth;
constexpr auto kPendingData = HostApi123::kSegmentSafetyPendingData;

constexpr auto kSourceNone = HostApi123::kSegmentSafetySourceNone;
constexpr auto kSourceVectorChart = HostApi123::kSegmentSafetySourceVectorChart;
constexpr auto kSourceCm93 = HostApi123::kSegmentSafetySourceCm93;
constexpr auto kSourceGshhsFallback =
    HostApi123::kSegmentSafetySourceGshhsFallback;
constexpr auto kSourcePluginVector =
    HostApi123::kSegmentSafetySourcePluginVector;

constexpr auto kDiagnosticNone = HostApi123::kSegmentSafetyDiagnosticNone;
constexpr auto kDiagnosticNoChartDatabase =
    HostApi123::kSegmentSafetyDiagnosticNoChartDatabase;
constexpr auto kDiagnosticNoCandidateChart =
    HostApi123::kSegmentSafetyDiagnosticNoCandidateChart;
constexpr auto kDiagnosticRasterOnly =
    HostApi123::kSegmentSafetyDiagnosticRasterOnly;
constexpr auto kDiagnosticUnsupportedChartType =
    HostApi123::kSegmentSafetyDiagnosticUnsupportedChartType;
constexpr auto kDiagnosticChartLoadFailed =
    HostApi123::kSegmentSafetyDiagnosticChartLoadFailed;
constexpr auto kDiagnosticNoLandAreaGeometry =
    HostApi123::kSegmentSafetyDiagnosticNoLandAreaGeometry;
constexpr auto kDiagnosticChartGeometryClear =
    HostApi123::kSegmentSafetyDiagnosticChartGeometryClear;
constexpr auto kDiagnosticChartGeometryHit =
    HostApi123::kSegmentSafetyDiagnosticChartGeometryHit;
constexpr auto kDiagnosticGshhsFallback =
    HostApi123::kSegmentSafetyDiagnosticGshhsFallback;
constexpr auto kDiagnosticPendingData =
    HostApi123::kSegmentSafetyDiagnosticPendingData;

constexpr auto kHitNone = HostApi123::kSegmentSafetyHitNone;
constexpr auto kHitEndpointInLandArea =
    HostApi123::kSegmentSafetyHitEndpointInLandArea;
constexpr auto kHitSegmentIntersectsLandAreaEdge =
    HostApi123::kSegmentSafetyHitSegmentIntersectsLandAreaEdge;
constexpr auto kHitMarginToLandAreaEdge =
    HostApi123::kSegmentSafetyHitMarginToLandAreaEdge;

wxString PointDiagnostic(double lat, double lon);
int GetPendingRequestCount();
bool ServicePendingRequests(int max_requests, int max_milliseconds,
                            SegmentSafetyRequestServiceResult *result);
bool PrepareRawTiles(const long *lat_tiles, const long *lon_tiles,
                     int tile_count, bool require_depth,
                     SegmentSafetyResult *result);
bool PrepareHazardSnapshot(double min_lat, double min_lon, double max_lat,
                           double max_lon, bool enable_fast_path,
                           bool shadow_compare,
                           const SegmentSafetyOptions *options,
                           SegmentSafetyResult *result);
bool CheckSegment(double lat1, double lon1, double lat2, double lon2,
                  const SegmentSafetyOptions *options,
                  SegmentSafetyResult *result);
bool PrepareCorridor(const double *latitudes, const double *longitudes,
                     const int *point_counts, int polyline_count,
                     double corridor_margin_nm, int fine_tile_halo,
                     const SegmentSafetyOptions *options,
                     SegmentSafetyResult *result);
bool PrepareGridForSegment(double lat1, double lon1, double lat2, double lon2,
                           double safety_margin_nm,
                           SegmentSafetyResult *result);
bool PrepareRouteMaskForPolylines(const double *latitudes,
                                  const double *longitudes,
                                  const int *point_counts, int polyline_count,
                                  double corridor_margin_nm,
                                  const SegmentSafetyOptions *options,
                                  SegmentSafetyResult *result);
void ReleasePins();
bool RegisterTileCache(const SegmentSafetyTileCacheCallbacks *callbacks);
bool GetChartIdentity(std::string *identity);
int GetChartInfoCount();
bool GetChartInfo(int ordinal, SegmentSafetyChartInfo *chart_info);
bool GetChartCoverageTiles(const int *chart_db_indices, int chart_count,
                           double tile_degrees, long *lat_tiles,
                           long *lon_tiles, int tile_capacity, int *tile_count,
                           bool *complete);
bool SetPersistentCacheEnabled(bool enabled);
bool GetPersistentCacheEnabled();
bool SavePersistentCache();
bool ClearPersistentCache();

}  // namespace ocpn::chart_safety

#endif  // GUI_CHART_SAFETY_API_H_
