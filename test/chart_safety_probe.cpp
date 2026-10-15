// SPDX-License-Identifier: GPL-2.0-or-later
// Test-only plugin: exercises the public API in an isolated real host.
#include "ocpn_plugin.h"
#include <wx/app.h>
#include <wx/timer.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include <limits>
#include <thread>

class Probe final : public opencpn_plugin_122, public wxTimer {
public:
  explicit Probe(void* manager) : opencpn_plugin_122(manager) {}
  int Init() override {
    if (!std::getenv("CHART_SAFETY_PROBE_OUTPUT")) return 0;
    auto host = GetHostApi();
    auto* api = dynamic_cast<HostApi123*>(host.get());
    if (api) {
      HostApi123::SegmentSafetyOptions options{};
      options.struct_size = sizeof(options);
      options.check_land = 1;
      HostApi123::SegmentSafetyResult result{};
      result.struct_size = sizeof(result);
      api->CheckSegmentSafety(28.4, -16.1, 28.4, -16.09, &options, &result);
      startup_status = result.status;
    }
    return WANTS_CONFIG | WANTS_LATE_INIT | WANTS_OVERLAY_CALLBACK;
  }
  bool DeInit() override {
    Stop();
    return true;
  }
  void LateInit() override {
    late_initialized = true;
    StartOnce(2000);
  }
  bool RenderOverlay(wxDC&, PlugIn_ViewPort*) override {
    overlay_called = true;
    return false;
  }
  int startup_status{-1};
  bool late_initialized{false};
  bool overlay_called{false};
  int GetAPIVersionMajor() override { return 1; }
  int GetAPIVersionMinor() override { return 22; }
  int GetPlugInVersionMajor() override { return 1; }
  int GetPlugInVersionMinor() override { return 0; }
  wxString GetCommonName() override { return "ChartSafetyProbe"; }
  wxString GetShortDescription() override {
    return "Isolated chart-safety API test";
  }
  wxString GetLongDescription() override { return GetShortDescription(); }
  void Notify() override {
    nlohmann::json report;
    auto host = GetHostApi();
    auto* api122 = dynamic_cast<HostApi122*>(host.get());
    report["api121"] = dynamic_cast<HostApi121*>(host.get()) != nullptr;
    report["api122"] = api122 != nullptr;
    report["late_initialized"] = late_initialized;
    report["overlay_called"] = overlay_called;
    if (api122) report["active_messages"] = api122->GetActiveMessages().size();
    auto* api = dynamic_cast<HostApi123*>(host.get());
    report["api123"] = api != nullptr;
    report["startup_status"] = startup_status;
    if (api) {
      report["chart_count"] = api->GetSegmentSafetyChartInfoCount();
      std::string identity;
      report["chart_identity_available"] =
          api->GetSegmentSafetyChartIdentity(&identity);
      auto unsupported = +[](void*, PlugInChartBase*,
                             const HostApi123::ChartSafetyProviderRequest*,
                             HostApi123::ChartSafetyProviderResult*) {
        return HostApi123::kChartSafetyProviderUnsupported;
      };
      HostApi123::ChartSafetyProviderCallbacks callbacks{};
      callbacks.struct_size = sizeof(callbacks);
      callbacks.query = unsupported;
      report["registered"] =
          api->RegisterChartSafetyProvider("ChartSafetyProbe", &callbacks);
      report["unknown_owner_rejected"] =
          !api->RegisterChartSafetyProvider("NoSuchPlugin", &callbacks);
      report["unregistered"] =
          api->RegisterChartSafetyProvider("ChartSafetyProbe", nullptr);
      HostApi123::SegmentSafetyOptions options{};
      options.struct_size = sizeof(options);
      options.check_land = 1;
      options.force_authoritative_fine_validation = 1;
      HostApi123::SegmentSafetyResult invalid{};
      invalid.struct_size = sizeof(invalid);
      report["invalid_rejected"] =
          !api->CheckSegmentSafety(std::numeric_limits<double>::quiet_NaN(), 0,
                                   0, 0, &options, &invalid);
      report["invalid_status"] = invalid.status;
      const auto check =
          [&](const char* name,
              std::vector<HostApi123::NavigationSafetyPosition> points,
              bool depth, double margin, double draft) {
            HostApi123::NavigationSafetyProfile profile;
            profile.check_depth = depth;
            profile.land_margin_nm = margin;
            profile.vessel_draft_m = draft;
            HostApi123::NavigationRouteSafetyResult result;
            const bool accepted =
                api->CheckNavigationRoute(profile, points, &result);
            nlohmann::json entry{{"accepted", accepted},
                                 {"complete", result.complete},
                                 {"safe", result.safe},
                                 {"legs", result.segments.size()}};
            for (const auto& segment : result.segments)
              entry["segments"].push_back({{"status", segment.status},
                                           {"source", segment.source},
                                           {"fallback", segment.used_fallback},
                                           {"message", segment.message}});
            report["routes"][name] = entry;
          };
      check("tenerife_land", {{28.4, -16.6}, {28.4, -16.5}}, false, 0, 0);
      check("tenerife_water", {{28.4, -16.1}, {28.4, -16.09}}, false, 0, 0);
      check("tenerife_depth", {{28.4, -16.1}, {28.4, -16.09}}, true, 0, 2.8);
      check("tenerife_unreachable_depth", {{28.4, -16.1}, {28.4, -16.09}}, true,
            0, 10000);
      check("pacific_water", {{0, -150}, {0.01, -150}}, false, 0, 0);
      check("mixed_route", {{28.4, -16.1}, {28.4, -16.5}, {28.4, -16.6}}, false,
            0, 0);
      check("polar_missing_depth", {{89.8, -150}, {89.81, -150}}, true, 0, 2.8);
      HostApi123::NavigationRouteSafetyResult worker_first, worker_second;
      bool worker_register_rejected = false;
      const std::vector<HostApi123::NavigationSafetyPosition> worker_points{
          {25, -30}, {25.005, -30}};
      std::thread worker([&] {
        api->CheckNavigationRoute({}, worker_points, &worker_first);
        worker_register_rejected =
            !api->RegisterChartSafetyProvider("ChartSafetyProbe", &callbacks);
      });
      worker.join();
      report["worker"]["initial_complete"] = worker_first.complete;
      report["worker"]["initial_status"] =
          worker_first.segments.empty() ? -1 : worker_first.segments[0].status;
      report["worker"]["registration_rejected"] = worker_register_rejected;
      report["worker"]["pending_requests"] =
          api->GetPendingSegmentSafetyRequestCount();
      HostApi123::SegmentSafetyRequestServiceResult serviced{};
      serviced.struct_size = sizeof(serviced);
      api->ServicePendingSegmentSafetyRequests(100, 1000, &serviced);
      std::thread retry([&] {
        api->CheckNavigationRoute({}, worker_points, &worker_second);
      });
      retry.join();
      report["worker"]["retry_complete"] = worker_second.complete;
      report["worker"]["retry_safe"] = worker_second.safe;
      report["worker"]["retry_status"] = worker_second.segments.empty()
                                             ? -1
                                             : worker_second.segments[0].status;
      HostApi123::NavigationRouteSafetyResult bad;
      report["invalid_route_rejected"] =
          !api->CheckNavigationRoute({}, {}, &bad);
    }
    const char* output = std::getenv("CHART_SAFETY_PROBE_OUTPUT");
    if (output) std::ofstream(output) << report.dump(2) << '\n';
    if (wxTheApp->GetTopWindow()) wxTheApp->GetTopWindow()->Close();
  }
};
extern "C" DECL_EXP opencpn_plugin* create_pi(void* manager) {
  return new Probe(manager);
}
extern "C" DECL_EXP void destroy_pi(opencpn_plugin* plugin) { delete plugin; }
