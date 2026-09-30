/***************************************************************************
 * Copyright (C) 2026 OpenCPN contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 ***************************************************************************/
#ifndef GUI_NAVIGATION_SAFETY_H_
#define GUI_NAVIGATION_SAFETY_H_

#include <functional>
#include "ocpn_plugin.h"

namespace ocpn::chart_safety {

using SegmentQuery = std::function<bool(double, double, double, double,
                                        const HostApi123::SegmentSafetyOptions*,
                                        HostApi123::SegmentSafetyResult*)>;

bool MakeNavigationOptions(const HostApi123::NavigationSafetyProfile& profile,
                           HostApi123::SegmentSafetyOptions* options);
bool ValidSegmentInput(double lat1, double lon1, double lat2, double lon2,
                       const HostApi123::SegmentSafetyOptions* options);
bool CheckNavigationRoute(
    const HostApi123::NavigationSafetyProfile& profile,
    const std::vector<HostApi123::NavigationSafetyPosition>& positions,
    const SegmentQuery& query, HostApi123::NavigationRouteSafetyResult* result);

}  // namespace ocpn::chart_safety
#endif
