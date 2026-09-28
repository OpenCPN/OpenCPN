# Draft chart-safety service for HostApi123

## Scope of this milestone

This branch implements the shared chart-checking foundation discussed in
[issue #5401](https://github.com/OpenCPN/OpenCPN/issues/5401) and overlaps the
profile/segment/route-validation part of
[PR #4464](https://github.com/OpenCPN/OpenCPN/pull/4464).

The earlier proposal is a design reference. Its distance queries, route checker
and route planner are declarations, not implementations ready to reuse. This
branch uses the existing, separately tested chart service and adds its own
profile-to-query conversion and whole-route validation. No code from the
navigation_apis proposal is imported.

Implemented:

- Chart-backed segment checks for land, drying areas, charted hazards and depth.
- Native S-57 and CM93 semantic extraction and an explicit chart-plugin provider
  interface. Raster imagery cannot certify depth or navigable water.
- Provider registration bound to plugin lifetime. Providers operate on the GUI
  thread; worker queries use immutable derived tiles and deferred requests.
- A navigation profile with explicit land margin, draft, under-keel clearance
  and minimum charted depth. Required depth is the larger of the explicit
  minimum and draft plus clearance.
- Whole-route validation, with one result per leg and explicit complete/safe
  flags. Every leg is visited, including legs after missing data or an unsafe
  leg. NoData, unknown depth, pending work and errors cannot certify a route.
- Rejection of non-finite coordinates and invalid allowances. Early calls while
  the chart database is rebuilding return NoData without storing false coverage
  results in the tile cache.

Profiles are caller-owned values. The host does not silently choose defaults,
apply a day/night context or persist a vessel profile. No existing route is
changed or inserted by validation.

## API compatibility

Published HostApi122 and all preceding declarations are unchanged. GetHostApi()
returns a private implementation of the developing HostApi123 interface. Plugin
API_VERSION_MINOR remains the published value 22; this change does not declare
API 1.23 released.

Compile consumers/providers against the matching development header. Use a
checked `dynamic_cast<HostApi123*>` with appropriate ownership: retain both the owning unique_ptr and the cast pointer for the same lifetime. A failed
cast means the optional chart service is unavailable. Stock 5.14.2 supports the
published base interface and loads the coordinated plugins; it does not provide
these new operations.

HostApi123 inherits HostApi122, whose virtual functions are defined in the host.
Consequently these development plugins require a host with published API 1.22;
a lower plugin API declaration does not remove that load-time dependency.

This development baseline also needs the already released API 1.22 callback
routing fixes from upstream commit `5b805e33d4601b478d1172724384c1956869f62e`
(Complete update to plugin API v122). The changes are carried forward from that
upstream implementation and cover late initialization, overlays, messages,
position fixes, active legs, input hooks, shutdown, follow state and options.

## Calling route validation

```cpp
auto host = GetHostApi();
auto* safety = dynamic_cast<HostApi123*>(host.get());
if (!safety) {
  // Chart-backed validation is unavailable on this host.
  return;
}
HostApi123::NavigationSafetyProfile profile;
profile.check_depth = true;
profile.land_margin_nm = 0.1;
profile.vessel_draft_m = 2.0;
profile.under_keel_clearance_m = 0.8;
std::vector<HostApi123::NavigationSafetyPosition> points{
    {28.4, -16.10}, {28.4, -16.09}};
HostApi123::NavigationRouteSafetyResult result;
if (!safety->CheckNavigationRoute(profile, points, &result) ||
    !result.complete || !result.safe) {
  // Present the per-leg results and do not certify this route as safe.
}
```

Pending worker checks require bounded GUI-thread service followed by a retry.
Do not run a long synchronous route validation on the GUI thread. Prepare a
corridor, service deferred tiles, validate on a worker, then present the result.
The final validation requests the fine chart service rather than accepting a
coarse search certificate.

Depth values refer to chart datum. Tide, squat, waves and other allowances are
not inferred. Unknown or absent depths remain unknown. Permitting GSHHS is an
explicit profile choice and does not provide a depth guarantee. CM93 is reported
as a separate, degraded chart source and does not supersede a more detailed
licensed/native vector chart covering a point.

## Further stages of #4464

This milestone does not implement a route-generating algorithm, nearest-hazard
or safety-distance queries, profile storage/CRUD, automatically selected context
margins, hard/soft constraint optimization, AIS collision predictions,
user-defined dynamic hazards, port/starboard passage constraints, bridge/air
draft checks, tides or traffic-scheme interpretation. These require separate
contracts, algorithms and test fixtures. Their absence is not represented as a
successful check.

Chart hazard coverage is also explicit: the current semantic classifiers cover
LNDARE, DEPARE/DRGARE, drying/WATLEV classifications and applicable WRECKS,
UWTROC and OBSTRN attributes. This is not a claim to interpret every S-57 hazard
class or every source of navigation risk.

## Verification

Configure and build with `-DOCPN_BUILD_TEST=ON`. Run focused tests from the test
subdirectory (upstream does not register the tests at the build root):

```sh
cmake -S . -B build -DOCPN_BUILD_TEST=ON
cmake --build build --parallel 4
ctest --test-dir build/test -R 'ChartSafety|NavigationSafety' --output-on-failure
```

The focused tests exercise conservative boundary depths, unknown values, tile
coverage holes and limits, batch planning, profile allowances, invalid routes,
final-leg checks, incomplete/failed queries, deferred retries and dateline legs.
The coordinated Weather Routing consumer has its own independent engine/cache
suite. Real-host startup/load and native chart queries are tested in isolated
profiles, separately from the stock compatibility control.

Platform builds and API review remain gates before this development interface
can be treated as ready for release. Linux startup or a unit suite alone cannot
establish Windows/macOS/Android behavior. The provider retains the previously
validated o-charts extraction implementation; this port checks its build and
load integration rather than repeating the licensed-chart qualification.

### Optional real-host probe

On Linux, `cmake --build build --target chart_safety_probe_pi` builds a test-only
plugin. It is excluded from normal builds and installation. Use it only in a
disposable profile: it writes the JSON file specified by
`CHART_SAFETY_PROBE_OUTPUT` and closes that test host after the checks. Enable
`libchart_safety_probe_pi.so` in that profile and set its plugin directory via
`OPENCPN_PLUGIN_DIRS`.

The probe checks the published parent interfaces, optional capability discovery,
API 1.22 late-initialization/overlay dispatch, provider ownership/thread guards,
invalid input and deferred worker requests. Its geographical assertions use the
local CM93 2015 fixture around Tenerife and polar coverage gaps. Chart data is
not distributed with this patch; other fixtures require corresponding expected
results. Clear only the disposable profile's chart-safety caches before a cold
worker test. A warm cache can legitimately bypass PendingData.

The September 28 local run passed 23 focused core tests, 203 coordinated Weather
Routing tests, stock 5.14.2 plugin compatibility, and the CM93 probe cases. Four
existing loopback tests segfaulted on both the stock control and this candidate;
this is not a claim that the full upstream suite is green. The o-charts provider
built and loaded. Its retained extraction code had already passed licensed-chart
testing before this port; those geographical checks were not repeated here.
The local helper dependency was supplied privately to the test instance. No
cross-platform result is implied.
