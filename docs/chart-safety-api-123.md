# Chart-aware routing support for xWeatherRouting 1.41.0

## Scope of the combined proposal

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

The chart-safety support includes adaptive aligned fine-tile lookahead, reuse of decoded
CM93 rules while the loaded cell is unchanged, and mixed provider/CM93 boundary
recovery. Only complete fine coverage can promote a coarse search certificate;
depth and final validation retain fine checks.

Native S-57/CM93 extraction is independent of chart display styles. Prepared
geometry retains tiny land/shallow/drying areas, point/line/area isolated dangers,
dateline geometry, local soundings and unresolved geometry. Unknown hazard depth
cannot borrow a safe depth from surrounding water. Soundings provide local depth
evidence and cannot certify coverage. The semantic cache identity changes when
these classifier semantics change.

The open-water starting layer uses complete coverage, hazard geometry and a
conservative depth lower bound. Import verifies regions against the installed
chart catalogue; unknown, changed or unproved regions retain detailed checks.
An imported layer never supplies a depth or safety guarantee by itself. The
host can also derive bounded open-water regions on demand. Final validation
continues to use authoritative fine checks.

Set `OCPN_CHART_SAFETY_REUSE_DETAILED_REGIONS=1` to enable restart reuse of
failed CM93 open-water proof attempts when persistent chart caching is on.
The host accepts the cache when the chart catalogue and source file fingerprint
match. Cache hits still go through the existing detailed checks. The host
saves new entries with its persistent caches.

Additional chart helpers are opt-in and bounded by CPU, available memory and a
zero-through-six process limit. On desktop Linux, isolated helpers can prepare
expensive detailed tiles or verify a starting-layer import. The parent validates
every returned payload and its chart identity. Unsupported platforms/providers,
missing helper prerequisites and failed/stale replies retain normal in-process
checks. Chart objects are never shared between processes or routing workers.

Derived certificate format 4 rejects older, malformed or mismatched catalogue
proofs. Integral and fractional JSON coordinates are read numerically and checked
against canonical tile coordinates and the policy key. Valid cached tiles remain
persistent; loaded chart changes invalidate their identity. CM93 cell loading
uses a batch append cursor to retain the same rule priority, style and natural
order without repeatedly walking the accumulated object list.

The implementation is split into core, raster, facade, compatibility and
navigation modules, retaining the HostApi123 provider registry. The optional
compatibility adapter lets the existing 1.41.0 plugin resolve its chart functions
dynamically. Legacy integer cache callbacks are explicitly converted to the typed
boolean callbacks; function pointers are never reinterpreted.

Profiles are caller-owned values. The host does not silently choose defaults,
apply a day/night context or persist a vessel profile. No existing route is
changed or inserted by validation.

## API compatibility

Published HostApi122 and all preceding declarations are unchanged. GetHostApi()
returns a private implementation of the developing HostApi123 interface. Plugin
API_VERSION_MINOR remains the published value 22; this change does not declare
API 1.23 released.

New consumers/providers compile against the matching development header and
retain the owning `unique_ptr` for the lifetime of a checked `HostApi123*`.
HostApi123 is an unstable developing interface, so a matching coordinated host
is required for its methods. It does not change the released API version.

xWeatherRouting 1.41.0 keeps its published API 1.21 dependency and dynamically
resolves the optional compatibility entrypoints. The same plugin binary loads
on unmodified stock OpenCPN: ordinary GRIB, climatology and GSHHG routing do not
require this proposal. A request for unavailable chart safety is reported as
unavailable rather than silently certified or relaxed. New typed HostApi123
consumers additionally require the matching developing host and published API
1.22 support inherited by HostApi123.

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

The current consumer is xWeatherRouting 1.41.0, with 2.8.0 providing the
improved-UX development variant. Both retain optional dynamically resolved
chart operations and their existing stock-host loading requirements. The older
HostApi123 consumer/provider branches are review fixtures for the typed API;
they are not qualification of the current plugin version.

The core application retains its upstream warning settings. The focused
native fixtures use AddressSanitizer; the standalone shoreline predicate test
also uses UndefinedBehaviorSanitizer. GTK leak detection is disabled in the
local address-checking harness. Platform builds and real-host consumer checks
remain separate gates.

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

### Private production fixtures

After a completed Linux Makefiles host build, link the production chart and
cache implementations into private test executables without adding test
exports to the API:

```sh
python3 test/build_chart_safety_native_tests.py --build-dir build --output-dir native-tests
python3 test/build_chart_safety_native_tests.py --build-dir build --output-dir cache-tests --certificate-cache
# Use a private virtual display and disposable runtime directories.
DISPLAY=:188 native-tests/native-fixtures
DISPLAY=:188 cache-tests/certificate-fixtures
python3 test/gshhs_intersection_regression.py --sanitize
```

The chart harness covers native CM93/S-57 hazards, depths, coverage, dateline
traversal, open-water proofs and exact batch append order. The cache harness
covers numeric save/load/save, invalid/stale certificate rejection, eviction
under pinned pressure, helper replies/admission/fallback and legacy-to-typed
callback conversion. The normal test target also compiles frozen consumer
layout assertions for all fields crossing the optional ABI boundary.

The standalone shoreline test extracts the production intersection predicate
and checks 2,000,012 cases against independent integer geometry. It covers
endpoint reversal, collinear disjoint segments and near-parallel lines.

The real-host probe checks plugin/API lifecycle and geographical results using
local CM93 charts. A matching Weather Routing binary must additionally be
exercised on both an enhanced host and an unmodified stock host, including
explicit chart-unavailable results, cancellation, ordinary weather routing and
clean shutdown. Enhanced-host behavior, stock compatibility, licensed-provider
qualification and platform builds are recorded separately. Raw chart data and
personal profiles are not distributed with these tests.
