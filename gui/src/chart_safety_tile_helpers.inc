// SPDX-License-Identifier: GPL-2.0-or-later
// Native CM93/S-57 and explicitly cache-permitted plugin classifications.
// Unsupported or non-shareable provider data retains the ordinary host path.
using HelperTileKey = std::pair<long, long>;
bool s_tile_helpers_enabled = false;
unsigned s_tile_helper_process_limit = 0;
// Process round trips cost tens of milliseconds even for a one-millisecond
// offshore proof. Learn local extraction costs before deciding to offload.
// Keep depth and non-depth observations separate; a cold chart load alone must
// not make an otherwise cheap region look expensive.
struct TileHelperCostSamples {
  std::vector<int> values;
  size_t next{};
  void Add(int milliseconds) {
    if (values.size() < 8)
      values.push_back(std::max(0, milliseconds));
    else {
      values[next] = std::max(0, milliseconds);
      next = (next + 1) % 8;
    }
  }
  bool WorthOffloading() const {
    if (values.size() < 4) return false;
    auto ordered = values;
    std::sort(ordered.begin(), ordered.end());
    return ordered[(ordered.size() - 1) / 2] >= 18;
  }
};
using TileHelperCostKey = std::tuple<int, long, long, bool>;
std::map<TileHelperCostKey, TileHelperCostSamples> s_tile_helper_costs;
wxString s_tile_helper_cost_identity;
std::set<std::tuple<int, long, long>> s_tile_helper_prepared;
TileHelperCostKey TileHelperCostKeyFor(long lat, long lon, bool depth) {
  // Raw tiles cover 0.05 degrees: twenty tiles make a one-degree region.
  return {SegmentSafetyCurrentGroupIndex(),
          static_cast<long>(std::floor(lat / 20.0)),
          static_cast<long>(std::floor(lon / 20.0)), depth};
}
void RefreshTileHelperCostIdentity() {
  if (s_tile_helper_cost_identity == s_segment_safety_chart_catalog_identity)
    return;
  s_tile_helper_costs.clear();
  s_tile_helper_prepared.clear();
  s_tile_helper_cost_identity = s_segment_safety_chart_catalog_identity;
}
void RecordTileHelperBuildCost(long lat, long lon, bool depth,
                               int milliseconds) {
  if (!wxThread::IsMain()) return;
  RefreshTileHelperCostIdentity();
  if (s_tile_helper_prepared.size() >= 65536)
    s_tile_helper_prepared.erase(s_tile_helper_prepared.begin());
  s_tile_helper_prepared.emplace(SegmentSafetyCurrentGroupIndex(), lat, lon);
  const auto key = TileHelperCostKeyFor(lat, lon, depth);
  if (s_tile_helper_costs.size() >= 1024 && !s_tile_helper_costs.count(key))
    s_tile_helper_costs.erase(s_tile_helper_costs.begin());
  s_tile_helper_costs[key].Add(milliseconds);
}
bool TileHelperCostAllows(long lat, long lon, bool depth) {
  wxString flag;
  // Explicit override is useful for provider qualification and on/off controls.
  if (wxGetEnv("OCPN_CHART_TILE_HELPERS", &flag)) {
    if (flag == "0") return false;
    if (flag == "1") return true;
  }
  RefreshTileHelperCostIdentity();
  const auto found =
      s_tile_helper_costs.find(TileHelperCostKeyFor(lat, lon, depth));
  return found != s_tile_helper_costs.end() && found->second.WorthOffloading();
}

bool TransferableHelperTile(const CachedPointSafetyGridTile& t) {
  return t.built &&
         (t.source == kSourceCm93 || t.source == kSourceVectorChart ||
          (t.source == kSourcePluginVector && t.persistent_cache_allowed));
}
bool WriteHelperTile(std::ostream& out, const CachedPointSafetyGridTile& t) {
  const int32_t values[] = {t.group_index,
                            static_cast<int32_t>(t.lat_tile),
                            static_cast<int32_t>(t.lon_tile),
                            t.rows,
                            t.cols,
                            t.chart_db_index,
                            t.chart_scale,
                            static_cast<int32_t>(t.source),
                            t.depth_complete ? 1 : 0,
                            t.persistent_cache_allowed ? 1 : 0};
  for (auto value : values)
    if (!SegmentSafetyWriteBinary(out, value)) return false;
  if (!SegmentSafetyWriteBinaryString(out, t.chart_path) ||
      !SegmentSafetyWriteBinaryString(out, t.dependency_identity))
    return false;
  const size_t cells = static_cast<size_t>(t.rows) * t.cols;
  if (cells != 1681 || t.hazard_flags.size() != cells ||
      t.has_depth.size() != cells || t.min_depth_m.size() != cells)
    return false;
  out.write(reinterpret_cast<const char*>(t.hazard_flags.data()),
            cells * sizeof(uint16_t));
  out.write(reinterpret_cast<const char*>(t.has_depth.data()), cells);
  out.write(reinterpret_cast<const char*>(t.min_depth_m.data()),
            cells * sizeof(float));
  return out.good();
}
bool ReadHelperTile(std::istream& in, CachedPointSafetyGridTile* t) {
  int32_t v[10];
  for (auto& value : v)
    if (!SegmentSafetyReadBinary(in, &value)) return false;
  std::string path, dependency;
  if (v[0] != SegmentSafetyCurrentGroupIndex() || v[1] < -1800 ||
      v[1] >= 1800 || v[2] < -3600 || v[2] >= 3600 || v[3] != 41 ||
      v[4] != 41 ||
      (v[7] != kSourceCm93 && v[7] != kSourceVectorChart &&
       v[7] != kSourcePluginVector) ||
      (v[8] != 0 && v[8] != 1) || (v[9] != 0 && v[9] != 1) ||
      (v[7] == kSourcePluginVector && v[9] != 1) ||
      !SegmentSafetyReadBinaryString(in, &path) ||
      path.size() >= sizeof(t->chart_path) ||
      !SegmentSafetyReadBinaryString(in, &dependency) || dependency.empty() ||
      dependency.size() >= sizeof(t->dependency_identity))
    return false;
  t->group_index = v[0];
  t->lat_tile = v[1];
  t->lon_tile = v[2];
  t->rows = v[3];
  t->cols = v[4];
  t->chart_db_index = v[5];
  t->chart_scale = v[6];
  t->source = static_cast<SegmentSafetySource>(v[7]);
  t->depth_complete = v[8] != 0;
  t->persistent_cache_allowed = v[9] != 0;
  t->min_lat = t->lat_tile * kGridTileDegrees;
  t->min_lon = t->lon_tile * kGridTileDegrees;
  t->resolution = kGridResolutionDegrees;
  snprintf(t->chart_path, sizeof(t->chart_path), "%s", path.c_str());
  snprintf(t->dependency_identity, sizeof(t->dependency_identity), "%s",
           dependency.c_str());
  const size_t cells = 1681;
  t->hazard_flags.resize(cells);
  t->has_depth.resize(cells);
  t->min_depth_m.resize(cells);
  in.read(reinterpret_cast<char*>(t->hazard_flags.data()),
          cells * sizeof(uint16_t));
  in.read(reinterpret_cast<char*>(t->has_depth.data()), cells);
  in.read(reinterpret_cast<char*>(t->min_depth_m.data()),
          cells * sizeof(float));
  if (!in.good()) return false;
  t->classes.resize(cells);
  t->has_drying.resize(cells);
  const uint16_t valid =
      kHazardLand | kHazardDrying | kHazardNoChart | kHazardUnknownClass;
  for (size_t n = 0; n < cells; ++n) {
    const auto h = t->hazard_flags[n];
    if ((h & ~valid) || t->has_depth[n] > 1 ||
        (t->has_depth[n] && !std::isfinite(t->min_depth_m[n])))
      return false;
    t->hazard_summary_flags |= h;
    t->has_drying[n] = (h & kHazardDrying) != 0;
    if (h & kHazardLand) {
      t->classes[n] = kPointLand;
      ++t->land_count;
    } else if (h & kHazardDrying) {
      t->classes[n] = kPointDrying;
      ++t->drying_count;
    } else if (h & (kHazardNoChart | kHazardUnknownClass)) {
      t->classes[n] = kPointNoData;
      ++t->unknown_count;
    } else {
      t->classes[n] = kPointWater;
      ++t->water_count;
    }
  }
  t->built = true;
  return true;
}

#if defined(__linux__) && !defined(__ANDROID__)
std::uint64_t AvailableTileHelperMemoryKiB() {
  std::ifstream memory("/proc/meminfo");
  std::string line;
  std::uint64_t available = 0;
  while (std::getline(memory, line)) {
    if (line.compare(0, 13, "MemAvailable:") == 0) {
      std::istringstream value(line.substr(13));
      value >> available;
      break;
    }
  }
  std::ifstream limit_file("/sys/fs/cgroup/memory.max"),
      used_file("/sys/fs/cgroup/memory.current");
  std::string limit;
  std::uint64_t used = 0;
  if ((limit_file >> limit) && limit != "max" && (used_file >> used)) {
    try {
      const auto maximum = std::stoull(limit);
      available = std::min<std::uint64_t>(
          available, maximum > used ? (maximum - used) / 1024 : 0);
    } catch (...) {
      return 0;
    }
  }
  return available;
}
struct TileHelperLane {
  wxString directory;
  long pid{};
  std::set<HelperTileKey> requested;
  std::chrono::steady_clock::time_point submitted{};
  bool require_depth{};
};
struct TileHelpers {
  wxString root, identity, catalogue, local_catalogue;
  int group{};
  bool disabled{};
  unsigned long imported{}, rejected{}, batches{};
  std::chrono::steady_clock::time_point memory_checked{};
  std::set<HelperTileKey> unsupported;
  std::vector<TileHelperLane> lanes;
} s_tile_helpers;

void StopSegmentSafetyTileHelpers() {
  for (auto& lane : s_tile_helpers.lanes)
    if (lane.pid > 0) wxKill(lane.pid, wxSIGTERM, nullptr, wxKILL_CHILDREN);
  if (!s_tile_helpers.lanes.empty())
    wxLogMessage(
        "WR_TILE_HELPERS stopped imported=%lu rejected=%lu batches=%lu",
        s_tile_helpers.imported, s_tile_helpers.rejected,
        s_tile_helpers.batches);
  wxString keep;
  if (!s_tile_helpers.root.empty() &&
      !(wxGetEnv("OCPN_CHART_TILE_HELPER_KEEP", &keep) && keep == "1"))
    wxFileName::Rmdir(s_tile_helpers.root, wxPATH_RMDIR_RECURSIVE);
  s_tile_helpers = {};
}
void PollSegmentSafetyTileHelpers() {
  if (s_tile_helpers.lanes.empty()) return;
  if (s_tile_helpers.identity != s_segment_safety_chart_identity ||
      s_tile_helpers.local_catalogue !=
          s_segment_safety_chart_catalog_identity ||
      s_tile_helpers.group != SegmentSafetyCurrentGroupIndex()) {
    StopSegmentSafetyTileHelpers();
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  if (now - s_tile_helpers.memory_checked > std::chrono::seconds(1)) {
    s_tile_helpers.memory_checked = now;
    if (AvailableTileHelperMemoryKiB() < 512 * 1024) {
      StopSegmentSafetyTileHelpers();
      s_tile_helpers.disabled = true;
      wxLogMessage("WR_TILE_HELPERS fallback reason=memory_pressure");
      return;
    }
  }
  for (auto& lane : s_tile_helpers.lanes) {
    if (lane.requested.empty()) continue;
    const wxString response = lane.directory + "/response.bin";
    if (!wxFileExists(response)) {
      if (!wxProcess::Exists(lane.pid)) {
        s_tile_helpers.unsupported.insert(lane.requested.begin(),
                                          lane.requested.end());
        lane.requested.clear();
        s_tile_helpers.disabled = true;
        wxLogMessage("WR_TILE_HELPERS fallback reason=worker_exited");
        continue;
      }
      if (std::chrono::steady_clock::now() - lane.submitted >
          std::chrono::seconds(30)) {
        s_tile_helpers.unsupported.insert(lane.requested.begin(),
                                          lane.requested.end());
        lane.requested.clear();
        s_tile_helpers.disabled = true;
        wxKill(lane.pid, wxSIGTERM, nullptr, wxKILL_CHILDREN);
        lane.pid = 0;
        wxLogMessage("WR_TILE_HELPERS fallback reason=worker_timeout");
      }
      continue;
    }
    wxStopWatch import_timer;
    const auto batch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::steady_clock::now() - lane.submitted)
                              .count();
    std::ifstream in(response.ToStdString(), std::ios::binary);
    std::string identity, catalogue;
    uint32_t count{}, magic{};
    bool ok = SegmentSafetyReadBinary(in, &magic) && magic == 0x54494c31u &&
              SegmentSafetyReadBinaryString(in, &identity) &&
              SegmentSafetyReadBinaryString(in, &catalogue) &&
              SegmentSafetyReadBinary(in, &count) && count <= 64 &&
              identity == s_tile_helpers.identity.ToStdString() &&
              catalogue == s_tile_helpers.catalogue.ToStdString();
    std::vector<CachedPointSafetyGridTile> tiles;
    std::set<HelperTileKey> returned;
    for (uint32_t n = 0; ok && n < count; ++n) {
      CachedPointSafetyGridTile t;
      ok = ReadHelperTile(in, &t) &&
           lane.requested.count({t.lat_tile, t.lon_tile}) &&
           returned.insert({t.lat_tile, t.lon_tile}).second;
      if (ok) tiles.push_back(std::move(t));
    }
    ok = ok && in.peek() == std::char_traits<char>::eof();
    unsigned imported = 0;
    if (!ok) s_tile_helpers.disabled = true;
    if (ok)
      for (auto& t : tiles) {
        // Check the actual local dependencies and provider precedence. A helper
        // result can never turn absent or stale chart evidence into clear sea.
        if ((lane.require_depth && !t.depth_complete) ||
            !SegmentSafetyTileDependencyIsCurrent(t) ||
            !SegmentSafetyCachedTileProviderIsCurrent(t))
          continue;
        StoreSegmentSafetyGridTile(
            SegmentSafetyGridTileKeyForIndices(t.lat_tile, t.lon_tile), t);
        SegmentSafetyExternalTileCacheStore(t);
        if (s_tile_helper_prepared.size() >= 65536)
          s_tile_helper_prepared.erase(s_tile_helper_prepared.begin());
        s_tile_helper_prepared.emplace(t.group_index, t.lat_tile, t.lon_tile);
        lane.requested.erase({t.lat_tile, t.lon_tile});
        ++imported;
      }
    s_tile_helpers.imported += imported;
    s_tile_helpers.rejected += lane.requested.size();
    s_tile_helpers.unsupported.insert(lane.requested.begin(),
                                      lane.requested.end());
    wxLogMessage(
        "WR_TILE_HELPERS delivered=%u fallback=%zu valid=%d batch_ms=%lld "
        "gui_import_ms=%ld",
        imported, lane.requested.size(), ok ? 1 : 0,
        static_cast<long long>(batch_ms), import_timer.Time());
    lane.requested.clear();
    in.close();
    wxRemoveFile(response);
  }
}
bool StartSegmentSafetyTileHelpers() {
  if (!s_tile_helpers_enabled || !s_tile_helper_process_limit ||
      s_tile_helpers.disabled)
    return false;
  if (!s_tile_helpers.lanes.empty()) return true;
  wxString flag;
  if (wxGetEnv("OCPN_CHART_TILE_WORKER_DIR", &flag) ||
      (wxGetEnv("OCPN_CHART_TILE_HELPERS", &flag) && flag == "0"))
    return false;
  const auto host = wxStandardPaths::Get().GetExecutablePath();
  const auto tool =
      wxFileName(host).GetPathWithSep() + "opencpn-chart-tile-worker";
  if (!g_Platform || !wxIsExecutable(tool)) return false;
  wxString python, display;
  if (!wxFindFileInPath(&python, wxGetenv("PATH"), "python3") ||
      !wxIsExecutable(python))
    return false;
  if ((!wxFindFileInPath(&display, wxGetenv("PATH"), "Xvfb") ||
       !wxIsExecutable(display)) &&
      !wxIsExecutable(wxFileName(host).GetPathWithSep() +
                      "opencpn-verifier-xvfb"))
    return false;
  const auto available = AvailableTileHelperMemoryKiB();
  if (available < 1536 * 1024) return false;
  const unsigned lanes = std::min(
      s_tile_helper_process_limit,
      available >= 3072 * 1024 && wxThread::GetCPUCount() >= 4 ? 2u : 1u);
  auto root = wxFileName::CreateTempFileName("ocpn-chart-tiles-");
  if (root.empty()) return false;
  wxRemoveFile(root);
  if (!wxFileName::Mkdir(root, 0700)) return false;
  s_tile_helpers.root = root;
  s_tile_helpers.identity = s_segment_safety_chart_identity;
  s_tile_helpers.local_catalogue = s_segment_safety_chart_catalog_identity;
  s_tile_helpers.catalogue = SegmentSafetyChartCatalogIdentity(false);
  s_tile_helpers.group = SegmentSafetyCurrentGroupIndex();
  s_tile_helpers.memory_checked = std::chrono::steady_clock::now();
  const auto tool_arg = tool.ToStdString(wxConvUTF8),
             host_arg = host.ToStdString(wxConvUTF8);
  const auto profile = g_Platform->GetPrivateDataDir().ToStdString(wxConvUTF8);
  const auto basemap = gWorldMapLocation.ToStdString(wxConvUTF8);
  const auto group = std::to_string(s_tile_helpers.group);
  for (unsigned n = 0; n < lanes; ++n) {
    const auto directory = root + wxString::Format("/%u", n);
    wxFileName::Mkdir(directory, 0700);
    const auto dir_arg = directory.ToStdString(wxConvUTF8);
    const char* argv[] = {tool_arg.c_str(), host_arg.c_str(), profile.c_str(),
                          basemap.c_str(),  group.c_str(),    dir_arg.c_str(),
                          nullptr};
    const auto pid = wxExecute(argv, wxEXEC_ASYNC | wxEXEC_MAKE_GROUP_LEADER);
    if (pid > 0) s_tile_helpers.lanes.push_back({directory, pid, {}, {}});
  }
  wxLogMessage("WR_TILE_HELPERS started lanes=%zu directory=%s",
               s_tile_helpers.lanes.size(), root);
  return !s_tile_helpers.lanes.empty();
}
void ScheduleTileHelperBatch(std::set<HelperTileKey> needed,
                             bool require_depth) {
  for (auto& lane : s_tile_helpers.lanes) {
    if (!lane.requested.empty() || needed.empty()) continue;
    wxJSONValue job;
    lane.require_depth = require_depth;
    job["depth"] = require_depth;
    wxJSONValue rows(wxJSONTYPE_ARRAY);
    const auto idle =
        std::count_if(s_tile_helpers.lanes.begin(), s_tile_helpers.lanes.end(),
                      [](const auto& item) { return item.requested.empty(); });
    const auto quota =
        std::min<std::size_t>(16, (needed.size() + idle - 1) / idle);
    while (!needed.empty() && lane.requested.size() < quota) {
      auto key = *needed.begin();
      needed.erase(needed.begin());
      wxJSONValue row(wxJSONTYPE_ARRAY);
      row.Append(static_cast<int>(key.first));
      row.Append(static_cast<int>(key.second));
      rows.Append(row);
      lane.requested.insert(key);
    }
    job["tiles"] = rows;
    wxJSONWriter writer;
    wxString text;
    writer.Write(job, text);
    const auto path = lane.directory + "/request.json";
    {
      std::ofstream out((path + ".new").ToStdString());
      out << text.ToStdString();
    }
    wxRenameFile(path + ".new", path, true);
    lane.submitted = std::chrono::steady_clock::now();
    ++s_tile_helpers.batches;
  }
}

bool TileHelperPending(long lat, long lon) {
  if (s_tile_helpers.disabled) return false;
  const HelperTileKey key{
      lat, ocpn::chart_safety::CanonicalLongitudeIndex(lon, 3600)};
  for (const auto& lane : s_tile_helpers.lanes)
    if (lane.requested.count(key)) return true;
  return false;
}

int QueueRawTileHelpers(const long* lat, const long* lon, int count,
                        bool depth) {
  if (!wxThread::IsMain() || !lat || !lon || count < 1 || count > 64 ||
      (!s_tile_helpers_enabled || !s_tile_helper_process_limit))
    return 0;
  PollSegmentSafetyTileHelpers();
  std::set<HelperTileKey> needed;
  for (int n = 0; n < count; ++n) {
    HelperTileKey key{
        lat[n], ocpn::chart_safety::CanonicalLongitudeIndex(lon[n], 3600)};
    if (key.first < -1800 || key.first >= 1800 ||
        s_tile_helpers.unsupported.count(key) ||
        TileHelperPending(key.first, key.second))
      continue;
    if (!TileHelperCostAllows(key.first, key.second, depth)) continue;
    CachedPointSafetyGridTile tile;
    const auto cache_key =
        SegmentSafetyGridTileKeyForIndices(key.first, key.second);
    if (LookupSegmentSafetyGridTile(cache_key, &tile) &&
        (!depth || tile.depth_complete))
      continue;
    if (SegmentSafetyExternalTileCacheLookup(key.first, key.second, depth,
                                             &tile) &&
        SegmentSafetyTileDependencyIsCurrent(tile) &&
        SegmentSafetyCachedTileProviderIsCurrent(tile)) {
      StoreSegmentSafetyGridTile(cache_key, tile);
      continue;
    }
    if (SegmentSafetyTileChartCandidates(key.first * kGridTileDegrees,
                                         key.second * kGridTileDegrees,
                                         (key.first + 1) * kGridTileDegrees,
                                         (key.second + 1) * kGridTileDegrees)
            .empty())
      continue;
    needed.insert(key);
  }
  for (auto it = needed.begin(); it != needed.end();)
    if (!TileHelperCostAllows(it->first, it->second, depth))
      it = needed.erase(it);
    else
      ++it;
  // Do not start private chart processes for a lone speculative request.
  if (!needed.empty() &&
      (!s_tile_helpers.lanes.empty() || needed.size() >= 8) &&
      StartSegmentSafetyTileHelpers())
    ScheduleTileHelperBatch(std::move(needed), depth);
  int pending = 0;
  for (int n = 0; n < count; ++n)
    pending += TileHelperPending(lat[n], lon[n]) ? 1 : 0;
  return pending;
}

bool DeferSegmentSafetyTileRequest(
    const SegmentSafetyRouteMaskRequest& request) {
  // Diagnostic forced mode retains blocking delivery for transfer controls.
  // Normal live search keeps the parent moving while helpers prefetch.
  wxString force;
  if (!wxGetEnv("OCPN_CHART_TILE_HELPERS", &force) || force != "1")
    return false;
  if (request.safety_margin_nm > 1.0) return false;
  if (!s_tile_helpers_enabled || !s_tile_helper_process_limit ||
      s_tile_helpers.disabled)
    return false;
  // Bulk raw preparation already supplies a bounded lookahead. Live mask
  // delivery must inspect only its own nine source tiles: repeatedly probing
  // the whole pending queue makes catalogue/cache checks dominate the GUI
  // service budget, even when the requested mask itself is cheap.
  std::set<HelperTileKey> needed;
  bool require_depth = request.check_depth;
  {
    const auto& r = request;
    for (int dy = -1; dy <= 1; ++dy)
      for (int dx = -1; dx <= 1; ++dx) {
        HelperTileKey key{
            r.lat_tile + dy,
            ocpn::chart_safety::CanonicalLongitudeIndex(r.lon_tile + dx, 3600)};
        if (key.first < -1800 || key.first >= 1800 ||
            s_tile_helpers.unsupported.count(key))
          continue;
        // In-flight work has no result to look up yet. Do not repeat provider
        // and catalogue probes on every GUI poll while the helper computes it.
        if (TileHelperPending(key.first, key.second)) continue;
        if (!TileHelperCostAllows(key.first, key.second, require_depth))
          continue;
        CachedPointSafetyGridTile existing;
        const auto cache_key =
            SegmentSafetyGridTileKeyForIndices(key.first, key.second);
        if (LookupSegmentSafetyGridTile(cache_key, &existing) &&
            (!require_depth || existing.depth_complete))
          continue;
        if (SegmentSafetyExternalTileCacheLookup(key.first, key.second,
                                                 require_depth, &existing) &&
            SegmentSafetyTileDependencyIsCurrent(existing) &&
            SegmentSafetyCachedTileProviderIsCurrent(existing)) {
          StoreSegmentSafetyGridTile(cache_key, existing);
          continue;
        }
        const auto candidates = SegmentSafetyTileChartCandidates(
            key.first * kGridTileDegrees, key.second * kGridTileDegrees,
            (key.first + 1) * kGridTileDegrees,
            (key.second + 1) * kGridTileDegrees);
        if (candidates.empty()) {
          s_tile_helpers.unsupported.insert(key);
          continue;
        }
        bool in_flight = false;
        for (const auto& lane : s_tile_helpers.lanes)
          in_flight |= lane.requested.count(key) != 0;
        if (!in_flight) needed.insert(key);
      }
  }
  for (auto it = needed.begin(); it != needed.end();)
    if (!TileHelperCostAllows(it->first, it->second, require_depth))
      it = needed.erase(it);
    else
      ++it;
  if (!needed.empty() &&
      (!s_tile_helpers.lanes.empty() || needed.size() >= 8) &&
      StartSegmentSafetyTileHelpers())
    ScheduleTileHelperBatch(std::move(needed), require_depth);
  if (request.safety_margin_nm > 1.0) return false;
  for (const auto& lane : s_tile_helpers.lanes)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dx = -1; dx <= 1; ++dx)
        if (lane.requested.count({request.lat_tile + dy,
                                  ocpn::chart_safety::CanonicalLongitudeIndex(
                                      request.lon_tile + dx, 3600)}))
          return true;
  return false;
}
// Examine only metadata for a bounded set of real queued demand. In particular,
// do not call the external tile cache or chart catalogue here: those probes can
// cost more than an already-cached mask. The parent validates every imported
// payload as usual and retains all authoritative work for its current request.
std::set<HelperTileKey> BackgroundTileHelperKeys(bool* require_depth) {
  std::vector<SegmentSafetyRouteMaskRequest> requests;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    for (const auto& entry : s_segment_safety_pending_route_mask_requests) {
      if (requests.size() == 2) break;
      if (entry.second.group_index == SegmentSafetyCurrentGroupIndex() &&
          !entry.second.force_authoritative_fine &&
          entry.second.safety_margin_nm <= 1.0)
        requests.push_back(entry.second);
    }
  }
  *require_depth = false;
  for (const auto& request : requests) *require_depth |= request.check_depth;
  RefreshTileHelperCostIdentity();
  std::set<HelperTileKey> needed;
  for (const auto& request : requests)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dx = -1; dx <= 1; ++dx) {
        const HelperTileKey key{request.lat_tile + dy,
                                ocpn::chart_safety::CanonicalLongitudeIndex(
                                    request.lon_tile + dx, 3600)};
        if (key.first < -1800 || key.first >= 1800 ||
            s_tile_helpers.unsupported.count(key) ||
            TileHelperPending(key.first, key.second) ||
            s_tile_helper_prepared.count(
                {SegmentSafetyCurrentGroupIndex(), key.first, key.second}) ||
            !TileHelperCostAllows(key.first, key.second, *require_depth) ||
            LookupSegmentSafetyGridTile(
                SegmentSafetyGridTileKeyForIndices(key.first, key.second),
                nullptr))
          continue;
        needed.insert(key);
      }
  return needed;
}
void PrefetchPendingSegmentSafetyTiles() {
  wxString flag;
  if (!s_tile_helpers_enabled || !s_tile_helper_process_limit ||
      s_tile_helpers.disabled ||
      (wxGetEnv("OCPN_CHART_TILE_HELPERS", &flag) &&
       (flag == "0" || flag == "1")))
    return;
  if (!s_tile_helpers.lanes.empty() &&
      std::none_of(s_tile_helpers.lanes.begin(), s_tile_helpers.lanes.end(),
                   [](const auto& lane) { return lane.requested.empty(); }))
    return;
  bool require_depth = false;
  auto needed = BackgroundTileHelperKeys(&require_depth);
  if (needed.empty() || (s_tile_helpers.lanes.empty() && needed.size() < 8) ||
      !StartSegmentSafetyTileHelpers())
    return;
  ScheduleTileHelperBatch(std::move(needed), require_depth);
}

#else
void StopSegmentSafetyTileHelpers() {}
void PollSegmentSafetyTileHelpers() {}
void PrefetchPendingSegmentSafetyTiles() {}
bool DeferSegmentSafetyTileRequest(const SegmentSafetyRouteMaskRequest&) {
  return false;
}
bool TileHelperPending(long, long) { return false; }
int QueueRawTileHelpers(const long*, const long*, int, bool) { return 0; }
#endif

int QueueTileHelpersCompat(const long* lat, const long* lon, int count,
                           int depth) {
  return QueueRawTileHelpers(lat, lon, count, depth != 0);
}
bool TileHelperPendingCompat(long lat, long lon) {
  if (!wxThread::IsMain()) return false;
  PollSegmentSafetyTileHelpers();
  return TileHelperPending(lat, lon);
}

void SetHelperProcessLimitCompat(int limit) {
  if (!wxThread::IsMain()) return;
  const auto next = static_cast<unsigned>(std::clamp(limit, 0, 6));
  if (next != s_tile_helper_process_limit) StopSegmentSafetyTileHelpers();
  s_tile_helper_process_limit = next;
}

void EnableTileHelpersCompat(int enabled) {
  if (!wxThread::IsMain()) return;
  s_tile_helpers_enabled = enabled != 0;
  if (!s_tile_helpers_enabled) StopSegmentSafetyTileHelpers();
}

bool RunTileWorkerCompat(const char* directory) {
#if defined(__linux__) && !defined(__ANDROID__)
  wxString worker;
  if (!wxThread::IsMain() || !directory ||
      !wxGetEnv("OCPN_CHART_TILE_WORKER_DIR", &worker) ||
      worker != wxString::FromUTF8(directory))
    return false;
  const auto root = wxString::FromUTF8(directory);
  if (!wxDirExists(root)) return false;
  SegmentSafetyRefreshPersistentChartIdentity();
  const auto identity = s_segment_safety_chart_identity;
  const auto catalogue = SegmentSafetyChartCatalogIdentity(false);
  std::ofstream((root + "/ready").ToStdString()) << "1\n";
  while (wxDirExists(root) && !wxFileExists(root + "/stop")) {
    const auto path = root + "/request.json";
    if (!wxFileExists(path)) {
      wxMilliSleep(10);
      continue;
    }
    wxFileInputStream input(path);
    wxJSONReader reader;
    wxJSONValue job;
    std::vector<CachedPointSafetyGridTile> tiles;
    const bool valid = reader.Parse(input, &job) == 0 &&
                       job["tiles"].IsArray() && job["tiles"].Size() <= 64 &&
                       job["depth"].IsBool();
    std::set<HelperTileKey> requested;
    if (valid)
      for (int n = 0; n < job["tiles"].Size(); ++n) {
        auto row = job["tiles"][n];
        if (row.IsArray() && row.Size() == 2 && row[0].IsInt() &&
            row[1].IsInt() && row[0].AsInt() >= -1800 &&
            row[0].AsInt() < 1800 && row[1].AsInt() >= -3600 &&
            row[1].AsInt() < 3600)
          requested.emplace(row[0].AsInt(), row[1].AsInt());
      }
    SegmentSafetyCoreStats stats;
    if (!requested.empty())
      PrebuildSegmentSafetyPluginVectorGridTiles(requested, &stats,
                                                 job["depth"].AsBool());
    if (valid)
      for (int n = 0; n < job["tiles"].Size(); ++n) {
        auto row = job["tiles"][n];
        if (!row.IsArray() || row.Size() != 2 || !row[0].IsInt() ||
            !row[1].IsInt())
          continue;
        const long lat = row[0].AsInt(), lon = row[1].AsInt();
        if (lat < -1800 || lat >= 1800 || lon < -3600 || lon >= 3600) continue;
        if (!EnsureSegmentSafetyGridTile(lat, lon, nullptr, nullptr,
                                         job["depth"].AsBool()))
          continue;
        CachedPointSafetyGridTile tile;
        if (LookupSegmentSafetyGridTile(
                SegmentSafetyGridTileKeyForIndices(lat, lon), &tile) &&
            TransferableHelperTile(tile))
          tiles.push_back(std::move(tile));
      }
    const auto response = root + "/response.bin";
    std::ofstream out((response + ".new").ToStdString(), std::ios::binary);
    bool ok =
        SegmentSafetyWriteBinary(out, uint32_t{0x54494c31u}) &&
        SegmentSafetyWriteBinaryString(out, identity.ToStdString()) &&
        SegmentSafetyWriteBinaryString(out, catalogue.ToStdString()) &&
        SegmentSafetyWriteBinary(out, static_cast<uint32_t>(tiles.size()));
    for (const auto& tile : tiles) ok = ok && WriteHelperTile(out, tile);
    out.close();
    wxRemoveFile(path);
    if (!ok || !wxRenameFile(response + ".new", response, true)) return false;
  }
  return true;
#else
  return false;
#endif
}
