/***************************************************************************
 * Copyright (C) 2026 OpenCPN contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 ***************************************************************************/

#ifndef GUI_CHART_SAFETY_DEPTH_H
#define GUI_CHART_SAFETY_DEPTH_H

#include <array>
#include <cmath>
#include <optional>

namespace ocpn::chart_safety {

enum class DepthProbeClass { kNoData, kWater, kLand, kDrying };

struct DepthProbe {
  DepthProbeClass point_class{DepthProbeClass::kNoData};
  bool has_depth{false};
  double depth_m{0.0};
};

/**
 * Resolve a depth-area polygon-boundary sample without weakening fail-closed
 * chart safety.
 *
 * A depth is returned only when all four samples surrounding the unresolved
 * point are chart-covered water with finite depths.  The minimum surrounding
 * depth is used so a boundary between depth areas remains conservative.
 */
inline std::optional<double> ConservativeBoundaryDepth(
    const std::array<DepthProbe, 4>& probes) {
  std::optional<double> minimum;
  for (const DepthProbe& probe : probes) {
    if (probe.point_class != DepthProbeClass::kWater || !probe.has_depth ||
        !std::isfinite(probe.depth_m))
      return std::nullopt;
    if (!minimum || probe.depth_m < *minimum) minimum = probe.depth_m;
  }
  return minimum;
}

}  // namespace ocpn::chart_safety

#endif  // GUI_CHART_SAFETY_DEPTH_H
