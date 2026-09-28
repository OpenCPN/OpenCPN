/***************************************************************************
 * Copyright (C) 2026 OpenCPN contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 ***************************************************************************/
#include "navigation_safety.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace ocpn::chart_safety {
namespace {
bool NonNegative(double value) { return std::isfinite(value) && value >= 0; }
bool ValidPosition(const HostApi123::NavigationSafetyPosition& point) {
  return std::isfinite(point.latitude) && std::isfinite(point.longitude) &&
         point.latitude >= -90 && point.latitude <= 90 &&
         point.longitude >= -180 && point.longitude <= 180;
}
bool KnownStatus(int status) {
  return status >= HostApi123::kSegmentSafetySafe &&
         status <= HostApi123::kSegmentSafetyPendingData;
}
}  // namespace

bool ValidSegmentInput(double lat1, double lon1, double lat2, double lon2,
                       const HostApi123::SegmentSafetyOptions* options) {
  if (!std::isfinite(lat1) || !std::isfinite(lat2) || std::abs(lat1) > 90 ||
      std::abs(lat2) > 90 || !std::isfinite(lon1) || !std::isfinite(lon2))
    return false;
  if (!options) return true;
  if (options->struct_size < static_cast<int>(sizeof(int))) return false;
  // Older size-tagged callers need not provide all options. Validate only
  // fields which the caller actually supplied, before any clamping or casts.
  if (options->struct_size >=
          static_cast<int>(
              offsetof(HostApi123::SegmentSafetyOptions, safety_margin_nm) +
              sizeof(options->safety_margin_nm)) &&
      !NonNegative(options->safety_margin_nm))
    return false;
  if (options->struct_size >=
          static_cast<int>(
              offsetof(HostApi123::SegmentSafetyOptions, minimum_depth_m) +
              sizeof(options->minimum_depth_m)) &&
      !NonNegative(options->minimum_depth_m))
    return false;
  return true;
}

bool MakeNavigationOptions(const HostApi123::NavigationSafetyProfile& profile,
                           HostApi123::SegmentSafetyOptions* options) {
  if (!options || (!profile.check_land && !profile.check_depth) ||
      !NonNegative(profile.land_margin_nm) ||
      !NonNegative(profile.vessel_draft_m) ||
      !NonNegative(profile.under_keel_clearance_m) ||
      !NonNegative(profile.minimum_charted_depth_m))
    return false;
  const double required_depth =
      profile.vessel_draft_m + profile.under_keel_clearance_m;
  if (!std::isfinite(required_depth)) return false;
  *options = {};
  options->struct_size = sizeof(*options);
  options->check_land = profile.check_land;
  options->check_depth = profile.check_depth;
  options->safety_margin_nm = profile.land_margin_nm;
  options->minimum_depth_m =
      std::max(profile.minimum_charted_depth_m, required_depth);
  options->allow_gshhs_fallback = profile.allow_gshhs_fallback;
  // Whole-route validation must not accept an approximate search shortcut.
  options->force_authoritative_fine_validation = 1;
  return true;
}

bool CheckNavigationRoute(
    const HostApi123::NavigationSafetyProfile& profile,
    const std::vector<HostApi123::NavigationSafetyPosition>& positions,
    const SegmentQuery& query,
    HostApi123::NavigationRouteSafetyResult* result) {
  if (!result) return false;
  *result = {};
  HostApi123::SegmentSafetyOptions options{};
  if (!query || positions.size() < 2 ||
      !MakeNavigationOptions(profile, &options) ||
      !std::all_of(positions.begin(), positions.end(), ValidPosition))
    return false;

  result->valid_input = true;
  result->complete = true;
  result->safe = true;
  result->segments.reserve(positions.size() - 1);
  for (size_t index = 0; index + 1 < positions.size(); ++index) {
    HostApi123::SegmentSafetyResult segment{};
    segment.struct_size = sizeof(segment);
    segment.status = HostApi123::kSegmentSafetyError;
    const bool queried =
        query(positions[index].latitude, positions[index].longitude,
              positions[index + 1].latitude, positions[index + 1].longitude,
              &options, &segment);
    if (!queried || !KnownStatus(segment.status)) {
      segment.status = HostApi123::kSegmentSafetyError;
      std::strncpy(segment.message, "chart query failed",
                   sizeof(segment.message));
    }
    // NoData, unknown depth and deferred work are incomplete, never safe.
    const bool incomplete =
        segment.status == HostApi123::kSegmentSafetyNoData ||
        segment.status == HostApi123::kSegmentSafetyUnknownDepth ||
        segment.status == HostApi123::kSegmentSafetyPendingData ||
        segment.status == HostApi123::kSegmentSafetyError;
    result->complete = result->complete && !incomplete;
    result->safe =
        result->safe && segment.status == HostApi123::kSegmentSafetySafe;
    result->segments.push_back(segment);
  }
  return true;
}
}  // namespace ocpn::chart_safety
