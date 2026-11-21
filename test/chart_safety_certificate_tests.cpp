/* GPL-2.0-or-later. Private production certificate-cache harness. */
#include <gtest/gtest.h>
#include "../gui/src/chart_safety_core.cpp"
#include <wx/init.h>
#include <limits>
using namespace ocpn::chart_safety;
using namespace ocpn::chart_safety::detail;
extern "C" int __wrap_main(int argc, char** argv) {
  wxApp::SetInstance(new wxApp());
  if (!wxEntryStart(argc, argv)) return 2;
  delete wxLog::SetActiveTarget(new wxLogStderr());
  ::testing::InitGoogleTest(&argc, argv);
  const int result = RUN_ALL_TESTS();
  wxEntryCleanup();
  return result;
}

namespace {
struct CertificateCacheFixture {
  decltype(s_segment_safety_persistent_certified_safe_cache) original =
      std::move(s_segment_safety_persistent_certified_safe_cache);
  wxString identity = s_segment_safety_chart_identity;
  wxString catalogue = s_segment_safety_chart_catalog_identity;
  bool enabled = s_segment_safety_persistent_cache_enabled;
  bool loaded = s_segment_safety_persistent_cache_loaded;
  bool dirty = s_segment_safety_persistent_cache_dirty;
  wxString path = wxFileName::CreateTempFileName("ocpn-certificate-test-");
  CertificateCacheFixture() {
    s_segment_safety_chart_identity = "certificate-round-trip";
    s_segment_safety_chart_catalog_identity = "certificate-catalogue";
    s_segment_safety_persistent_cache_enabled = true;
    Reset();
  }
  ~CertificateCacheFixture() {
    wxRemoveFile(path);
    if (wxFileExists(path + ".tmp")) wxRemoveFile(path + ".tmp");
    s_segment_safety_persistent_certified_safe_cache = std::move(original);
    s_segment_safety_chart_identity = identity;
    s_segment_safety_chart_catalog_identity = catalogue;
    s_segment_safety_persistent_cache_enabled = enabled;
    s_segment_safety_persistent_cache_loaded = loaded;
    s_segment_safety_persistent_cache_dirty = dirty;
  }
  void Reset() {
    s_segment_safety_persistent_cache_loaded = false;
    s_segment_safety_persistent_cache_dirty = false;
    s_segment_safety_persistent_certified_safe_cache.clear();
  }
  CachedSegmentSafetyCoarseRouteMaskCell Cell(long lat = 180, long lon = -65) {
    CachedSegmentSafetyCoarseRouteMaskCell cell;
    cell.lat_cell = lat;
    cell.lon_cell = lon;
    cell.min_lat = lat * cell.degrees;
    cell.min_lon = lon * cell.degrees;
    cell.state = kCoarseCertifiedSafe;
    cell.source = kSourceCm93;
    cell.check_depth = true;
    cell.minimum_depth_m = 10;
    cell.safety_margin_nm = 1;
    return cell;
  }
  std::string Key(const CachedSegmentSafetyCoarseRouteMaskCell& cell) {
    return (s_segment_safety_chart_identity + ":" +
            wxString::FromUTF8(SegmentSafetyCoarseRouteMaskKeyForGroup(
                                   cell.group_index, cell.lat_cell,
                                   cell.lon_cell, cell.safety_margin_nm,
                                   cell.check_depth, cell.minimum_depth_m)
                                   .c_str()))
        .ToStdString();
  }
  wxJSONValue Entry(const CachedSegmentSafetyCoarseRouteMaskCell& cell) {
    wxJSONValue entry;
    entry["key"] = wxString::FromUTF8(Key(cell).c_str());
    entry["certified_safe"] = true;
    entry["group_index"] = cell.group_index;
    entry["lat_cell"] = (wxInt64)cell.lat_cell;
    entry["lon_cell"] = (wxInt64)cell.lon_cell;
    entry["min_lat"] = cell.min_lat;
    entry["min_lon"] = cell.min_lon;
    entry["coarse_degrees"] = cell.degrees;
    entry["check_depth"] = cell.check_depth;
    entry["minimum_depth_m"] = cell.minimum_depth_m;
    entry["safety_margin_nm"] = cell.safety_margin_nm;
    entry["source"] = (int)cell.source;
    entry["persistent_cache_allowed"] = cell.persistent_cache_allowed;
    return entry;
  }
  void Write(const wxJSONValue& entries) {
    wxJSONValue root;
    root["format_version"] = kPersistentCacheVersion;
    root["chart_catalog_identity"] = s_segment_safety_chart_catalog_identity;
    root["route_mask_algorithm_version"] = kRouteMaskAlgorithmVersion;
    root["entries"] = entries;
    wxFileOutputStream out(path);
    wxJSONWriter writer;
    writer.Write(root, out);
    ASSERT_TRUE(out.IsOk());
  }
};
}  // namespace

TEST(ChartCertificateCache, SaveLoadSavePreservesIntegralAndFractionalNumbers) {
  CertificateCacheFixture fixture;
  std::vector<CachedSegmentSafetyCoarseRouteMaskCell> expected = {
      fixture.Cell(180, -65), fixture.Cell(-170, -570), fixture.Cell(181, -66),
      fixture.Cell(0, 0)};
  expected[2].safety_margin_nm = .4;
  expected[2].minimum_depth_m = 3.75;
  for (const auto& cell : expected)
    s_segment_safety_persistent_certified_safe_cache[fixture.Key(cell)] = cell;
  for (int round = 0; round < 3; ++round) {
    s_segment_safety_persistent_cache_dirty = true;
    ASSERT_TRUE(SegmentSafetyPersistentCertifiedCacheSaveTo(fixture.path));
    fixture.Reset();
    ASSERT_TRUE(SegmentSafetyPersistentCacheLoadLocked(fixture.path));
    ASSERT_EQ(s_segment_safety_persistent_certified_safe_cache.size(),
              expected.size());
    for (const auto& cell : expected) {
      const auto found = s_segment_safety_persistent_certified_safe_cache.find(
          fixture.Key(cell));
      ASSERT_NE(found, s_segment_safety_persistent_certified_safe_cache.end());
      EXPECT_DOUBLE_EQ(found->second.min_lat, cell.min_lat);
      EXPECT_DOUBLE_EQ(found->second.min_lon, cell.min_lon);
      EXPECT_DOUBLE_EQ(found->second.minimum_depth_m, cell.minimum_depth_m);
      EXPECT_DOUBLE_EQ(found->second.safety_margin_nm, cell.safety_margin_nm);
    }
  }
}

TEST(ChartCertificateCache, LegacyMissingCoordinateFieldsUseCanonicalDefaults) {
  CertificateCacheFixture fixture;
  auto cell = fixture.Cell();
  cell.group_index = 3;
  auto entry = fixture.Entry(cell);
  for (auto name : {"min_lat", "min_lon", "coarse_degrees", "source",
                    "persistent_cache_allowed"})
    entry.Remove(name);
  wxJSONValue entries(wxJSONTYPE_ARRAY);
  entries.Append(entry);
  fixture.Write(entries);
  ASSERT_TRUE(SegmentSafetyPersistentCacheLoadLocked(fixture.path));
  ASSERT_EQ(s_segment_safety_persistent_certified_safe_cache.size(), 1u);
  auto loaded =
      s_segment_safety_persistent_certified_safe_cache.at(fixture.Key(cell));
  EXPECT_EQ(loaded.group_index, 3);
  EXPECT_DOUBLE_EQ(loaded.min_lat, 36);
  EXPECT_DOUBLE_EQ(loaded.min_lon, -13);
  EXPECT_DOUBLE_EQ(loaded.minimum_depth_m, 10);
}

TEST(ChartCertificateCache, InvalidEntriesCannotBecomeOrChangeAProof) {
  CertificateCacheFixture fixture;
  auto valid = fixture.Entry(fixture.Cell());
  std::vector<wxJSONValue> invalid;
  const auto change = [&](const char* field, const wxJSONValue& value) {
    auto entry = valid;
    entry[field] = value;
    invalid.push_back(entry);
  };
  change("min_lon", wxString("-13"));
  change("min_lon", -12.0);
  change("min_lat", .00001);
  change("lat_cell", 180.5);
  change("lat_cell", 450);
  change("lon_cell", 900);
  change("group_index", -1);
  change("coarse_degrees", .4);
  change("check_depth", 1);
  change("certified_safe", 1);
  change("minimum_depth_m", -1);
  change("minimum_depth_m", 1e30);
  change("safety_margin_nm", -1);
  change("safety_margin_nm", true);
  change("source", (int)kSourceNone);
  change("persistent_cache_allowed", wxString("true"));
  change("grid_resolution_degrees", .01);
  change("coarse_factor", 8);
  change("key",
         wxString("other-chart:0:180:-65:cr0.20000000:f4:m1000:d1:1000"));
  auto plugin = valid;
  plugin["source"] = (int)kSourcePluginVector;
  plugin["persistent_cache_allowed"] = false;
  invalid.push_back(plugin);
  wxJSONValue entries(wxJSONTYPE_ARRAY);
  entries.Append(valid);
  for (const auto& entry : invalid) entries.Append(entry);
  fixture.Write(entries);
  ASSERT_TRUE(SegmentSafetyPersistentCacheLoadLocked(fixture.path));
  ASSERT_EQ(s_segment_safety_persistent_certified_safe_cache.size(), 1u);
  EXPECT_EQ(s_segment_safety_persistent_entries_ignored, (long)invalid.size());
  for (size_t i = 0; i < invalid.size(); ++i) {
    SCOPED_TRACE(i);
    fixture.Reset();
    wxJSONValue one(wxJSONTYPE_ARRAY);
    one.Append(invalid[i]);
    fixture.Write(one);
    ASSERT_TRUE(SegmentSafetyPersistentCacheLoadLocked(fixture.path));
    EXPECT_TRUE(s_segment_safety_persistent_certified_safe_cache.empty());
  }
}

TEST(ChartCertificateCache, SaveAndStoreRejectNonfiniteOrInconsistentRecords) {
  CertificateCacheFixture fixture;
  auto valid = fixture.Cell();
  auto bad = fixture.Cell(181, -65);
  bad.min_lat = 1.778636325e-322;
  EXPECT_FALSE(SegmentSafetyValidCertificate(bad));
  bad.min_lon = std::numeric_limits<double>::quiet_NaN();
  SegmentSafetyPersistentStoreCertifiedSafe(bad);
  EXPECT_TRUE(s_segment_safety_persistent_certified_safe_cache.empty());
  s_segment_safety_persistent_certified_safe_cache[fixture.Key(valid)] = valid;
  s_segment_safety_persistent_certified_safe_cache[fixture.Key(bad)] = bad;
  s_segment_safety_persistent_certified_safe_cache["wrong-key"] = valid;
  s_segment_safety_persistent_cache_dirty = true;
  ASSERT_TRUE(SegmentSafetyPersistentCertifiedCacheSaveTo(fixture.path));
  fixture.Reset();
  ASSERT_TRUE(SegmentSafetyPersistentCacheLoadLocked(fixture.path));
  EXPECT_EQ(s_segment_safety_persistent_certified_safe_cache.size(), 1u);
  fixture.Reset();
  s_segment_safety_persistent_cache_dirty = true;
  ASSERT_TRUE(SegmentSafetyPersistentCertifiedCacheSaveTo(fixture.path));
  fixture.Reset();
  EXPECT_TRUE(SegmentSafetyPersistentCacheLoadLocked(fixture.path));
  EXPECT_TRUE(s_segment_safety_persistent_certified_safe_cache.empty());
}

TEST(ChartCertificateCache, AlreadyMalformedCacheFailsClosed) {
  CertificateCacheFixture fixture;
  {
    std::ofstream out(fixture.path.ToStdString());
    out << "{\"format_version\":3,\"entries\":[{\"min_lon\":-nan}]}";
  }
  EXPECT_FALSE(SegmentSafetyPersistentCacheLoadLocked(fixture.path));
  EXPECT_TRUE(s_segment_safety_persistent_certified_safe_cache.empty());
}

TEST(ChartCertificateCache, CatalogueChangesAcrossRestartInvalidateTheFile) {
  CertificateCacheFixture fixture;
  wxJSONValue entries(wxJSONTYPE_ARRAY);
  entries.Append(fixture.Entry(fixture.Cell()));
  fixture.Write(entries);
  s_segment_safety_chart_catalog_identity = "updated-catalogue";
  EXPECT_FALSE(SegmentSafetyPersistentCacheLoadLocked(fixture.path));
  EXPECT_TRUE(s_segment_safety_persistent_certified_safe_cache.empty());
}

TEST(ChartTileHelperCodec, RoundTripRetainsHazardsDepthAndDatelineIdentity) {
  CachedPointSafetyGridTile tile;
  tile.group_index = SegmentSafetyCurrentGroupIndex();
  tile.lat_tile = -400;
  tile.lon_tile = -3600;
  tile.rows = tile.cols = 41;
  tile.source = kSourceCm93;
  snprintf(tile.chart_path, sizeof(tile.chart_path), "CM93.A");
  snprintf(tile.dependency_identity, sizeof(tile.dependency_identity),
           "fixture-dependency");
  tile.hazard_flags.assign(1681, kHazardNone);
  tile.has_depth.assign(1681, 1);
  tile.min_depth_m.assign(1681, 40.f);
  tile.hazard_flags[7] = kHazardLand;
  tile.hazard_flags[8] = kHazardDrying;
  std::stringstream stream;
  ASSERT_TRUE(WriteHelperTile(stream, tile));
  CachedPointSafetyGridTile decoded;
  ASSERT_TRUE(ReadHelperTile(stream, &decoded));
  EXPECT_EQ(decoded.lon_tile, -3600);
  EXPECT_EQ(decoded.land_count, 1);
  EXPECT_EQ(decoded.drying_count, 1);
  EXPECT_EQ(decoded.min_depth_m, tile.min_depth_m);
  EXPECT_EQ(decoded.hazard_flags, tile.hazard_flags);
  EXPECT_STREQ(decoded.dependency_identity, "fixture-dependency");
  tile.hazard_flags[10] = 0x8000;
  std::stringstream invalid;
  ASSERT_TRUE(WriteHelperTile(invalid, tile));
  EXPECT_FALSE(ReadHelperTile(invalid, &decoded));
  tile.hazard_flags[10] = 0;
  tile.source = kSourcePluginVector;
  std::stringstream unsupported;
  ASSERT_TRUE(WriteHelperTile(unsupported, tile));
  EXPECT_FALSE(ReadHelperTile(unsupported, &decoded));
  tile.persistent_cache_allowed = true;
  std::stringstream permitted;
  ASSERT_TRUE(WriteHelperTile(permitted, tile));
  EXPECT_TRUE(ReadHelperTile(permitted, &decoded));
  EXPECT_TRUE(decoded.persistent_cache_allowed);
  std::stringstream truncated(stream.str().substr(0, 200));
  EXPECT_FALSE(ReadHelperTile(truncated, &decoded));
}

#if defined(__linux__) && !defined(__ANDROID__)
TEST(ChartTileHelpers, RejectsStaleCatalogueWithoutPublishingClearWater) {
  StopSegmentSafetyTileHelpers();
  const auto root = wxFileName::CreateTempFileName("chart-helper-reject-");
  wxRemoveFile(root);
  ASSERT_TRUE(wxFileName::Mkdir(root));
  s_tile_helpers.root = root;
  s_tile_helpers.identity = s_segment_safety_chart_identity;
  s_tile_helpers.local_catalogue = s_segment_safety_chart_catalog_identity;
  s_tile_helpers.catalogue = "expected-catalogue";
  s_tile_helpers.group = SegmentSafetyCurrentGroupIndex();
  s_tile_helpers.memory_checked = std::chrono::steady_clock::now();
  s_tile_helpers.lanes.push_back(
      {root, 0, {{-600, -2600}}, std::chrono::steady_clock::now()});
  const auto before = s_segment_safety_grid_cache.size();
  {
    std::ofstream out((root + "/response.bin").ToStdString(), std::ios::binary);
    ASSERT_TRUE(SegmentSafetyWriteBinary(out, uint32_t{0x54494c31u}));
    ASSERT_TRUE(SegmentSafetyWriteBinaryString(
        out, s_tile_helpers.identity.ToStdString()));
    ASSERT_TRUE(SegmentSafetyWriteBinaryString(out, "stale-catalogue"));
    ASSERT_TRUE(SegmentSafetyWriteBinary(out, uint32_t{0}));
  }
  PollSegmentSafetyTileHelpers();
  EXPECT_TRUE(s_tile_helpers.disabled);
  EXPECT_EQ(s_tile_helpers.unsupported.count({-600, -2600}), 1u);
  EXPECT_EQ(s_segment_safety_grid_cache.size(), before);
  StopSegmentSafetyTileHelpers();
}

TEST(ChartTileHelpers,
     MissingRequiredDepthFallsBackInsteadOfRequeueingForever) {
  StopSegmentSafetyTileHelpers();
  const auto root = wxFileName::CreateTempFileName("chart-helper-depth-");
  wxRemoveFile(root);
  ASSERT_TRUE(wxFileName::Mkdir(root));
  s_tile_helpers.root = root;
  s_tile_helpers.identity = s_segment_safety_chart_identity;
  s_tile_helpers.local_catalogue = s_segment_safety_chart_catalog_identity;
  s_tile_helpers.catalogue = "same-catalogue";
  s_tile_helpers.group = SegmentSafetyCurrentGroupIndex();
  s_tile_helpers.memory_checked = std::chrono::steady_clock::now();
  s_tile_helpers.lanes.push_back(
      {root, 0, {{-600, -2600}}, std::chrono::steady_clock::now(), true});
  CachedPointSafetyGridTile tile;
  tile.group_index = SegmentSafetyCurrentGroupIndex();
  tile.lat_tile = -600;
  tile.lon_tile = -2600;
  tile.rows = tile.cols = 41;
  tile.source = kSourceCm93;
  tile.depth_complete = false;
  snprintf(tile.chart_path, sizeof(tile.chart_path), "CM93.A");
  snprintf(tile.dependency_identity, sizeof(tile.dependency_identity),
           "depth-unproven");
  tile.hazard_flags.assign(1681, 0);
  tile.has_depth.assign(1681, 0);
  tile.min_depth_m.assign(1681, 0);
  const auto before = s_segment_safety_grid_cache.size();
  {
    std::ofstream out((root + "/response.bin").ToStdString(), std::ios::binary);
    ASSERT_TRUE(SegmentSafetyWriteBinary(out, uint32_t{0x54494c31u}));
    ASSERT_TRUE(SegmentSafetyWriteBinaryString(
        out, s_tile_helpers.identity.ToStdString()));
    ASSERT_TRUE(SegmentSafetyWriteBinaryString(out, "same-catalogue"));
    ASSERT_TRUE(SegmentSafetyWriteBinary(out, uint32_t{1}));
    ASSERT_TRUE(WriteHelperTile(out, tile));
  }
  PollSegmentSafetyTileHelpers();
  EXPECT_EQ(s_tile_helpers.imported, 0u);
  EXPECT_EQ(s_tile_helpers.unsupported.count({-600, -2600}), 1u);
  EXPECT_FALSE(TileHelperPending(-600, -2600));
  EXPECT_EQ(s_segment_safety_grid_cache.size(), before);
  StopSegmentSafetyTileHelpers();
}
#endif

TEST(ChartTileHelpers, AdmissionRequiresRepeatedExpensiveLocalBuilds) {
  TileHelperCostSamples costs;
  EXPECT_FALSE(costs.WorthOffloading());
  costs.Add(600);
  costs.Add(1);
  costs.Add(2);
  costs.Add(1);
  EXPECT_FALSE(costs.WorthOffloading());
  for (int n = 0; n < 8; ++n) costs.Add(30);
  EXPECT_TRUE(costs.WorthOffloading());
  for (int n = 0; n < 8; ++n) costs.Add(2);
  EXPECT_FALSE(costs.WorthOffloading());
}
TEST(ChartTileHelpers, AdmissionSeparatesDepthRegionsAndCatalogue) {
  s_tile_helper_costs.clear();
  for (int n = 0; n < 4; ++n) RecordTileHelperBuildCost(-1, -1, true, 30);
  EXPECT_TRUE(TileHelperCostAllows(-2, -2, true));
  EXPECT_FALSE(TileHelperCostAllows(0, -2, true));
  EXPECT_FALSE(TileHelperCostAllows(-2, -2, false));
  const auto original = s_segment_safety_chart_catalog_identity;
  s_segment_safety_chart_catalog_identity += "-changed";
  EXPECT_FALSE(TileHelperCostAllows(-2, -2, true));
  s_segment_safety_chart_catalog_identity = original;
  RefreshTileHelperCostIdentity();
}
#if defined(__linux__) && !defined(__ANDROID__)
TEST(ChartTileHelpers, CachedLiveMaskDoesNotProbeUnrelatedPendingRequests) {
  StopSegmentSafetyTileHelpers();
  auto saved_grid = std::move(s_segment_safety_grid_cache);
  auto saved_pending = std::move(s_segment_safety_pending_route_mask_requests);
  const auto saved_callbacks = s_segment_safety_external_tile_cache;
  const bool saved_enabled = s_tile_helpers_enabled;
  const unsigned saved_limit = s_tile_helper_process_limit;
  const auto saved_prepared = s_tile_helper_prepared;
  wxString saved_flag;
  const bool had_flag = wxGetEnv("OCPN_CHART_TILE_HELPERS", &saved_flag);
  wxSetEnv("OCPN_CHART_TILE_HELPERS", "1");
  s_tile_helpers_enabled = true;
  s_tile_helper_process_limit = 2;
  int lookups = 0;
  s_segment_safety_external_tile_cache = {};
  s_segment_safety_external_tile_cache.context = &lookups;
  s_segment_safety_external_tile_cache.lookup = [](void* context, long, long,
                                                   bool, SegmentSafetyTile*) {
    ++*static_cast<int*>(context);
    return false;
  };
  SegmentSafetyRouteMaskRequest request;
  request.lat_tile = 700;
  request.lon_tile = 2000;
  request.safety_margin_nm = 0.4;
  request.check_depth = false;
  for (int dy = -1; dy <= 1; ++dy)
    for (int dx = -1; dx <= 1; ++dx) {
      CachedPointSafetyGridTile tile;
      tile.built = true;
      s_segment_safety_grid_cache[SegmentSafetyGridTileKeyForIndices(
          700 + dy, 2000 + dx)] = tile;
    }
  for (int n = 0; n < 32; ++n) {
    auto other = request;
    other.lat_tile = -600 + n;
    other.lon_tile = -2000;
    s_segment_safety_pending_route_mask_requests[std::to_string(n)] = other;
  }
  EXPECT_FALSE(DeferSegmentSafetyTileRequest(request));
  EXPECT_EQ(lookups, 0);
  EXPECT_TRUE(s_tile_helpers.lanes.empty());
  s_segment_safety_grid_cache.erase(
      SegmentSafetyGridTileKeyForIndices(700, 2000));
  s_tile_helpers.lanes.push_back(
      {"", 0, {{700, 2000}}, std::chrono::steady_clock::now()});
  EXPECT_TRUE(DeferSegmentSafetyTileRequest(request));
  EXPECT_EQ(lookups, 0);
  wxSetEnv("OCPN_CHART_TILE_HELPERS", "auto");
  EXPECT_FALSE(DeferSegmentSafetyTileRequest(request));
  EXPECT_EQ(lookups, 0);
  wxSetEnv("OCPN_CHART_TILE_HELPERS", "1");
  bool depth = true;
  const auto keys = BackgroundTileHelperKeys(&depth);
  EXPECT_FALSE(depth);
  EXPECT_FALSE(keys.empty());
  EXPECT_LE(keys.size(), 18u);
  EXPECT_EQ(lookups, 0);
  if (!keys.empty()) {
    const auto first = *keys.begin();
    s_tile_helper_prepared.emplace(SegmentSafetyCurrentGroupIndex(),
                                   first.first, first.second);
    EXPECT_EQ(BackgroundTileHelperKeys(&depth).count(first), 0u);
  }
  s_tile_helper_prepared = saved_prepared;
  SetHelperProcessLimitCompat(0);
  EXPECT_TRUE(s_tile_helpers.lanes.empty());
  EXPECT_FALSE(DeferSegmentSafetyTileRequest(request));
  s_tile_helper_process_limit = saved_limit;
  s_segment_safety_grid_cache = std::move(saved_grid);
  s_segment_safety_pending_route_mask_requests = std::move(saved_pending);
  s_segment_safety_external_tile_cache = saved_callbacks;
  s_tile_helpers_enabled = saved_enabled;
  if (had_flag)
    wxSetEnv("OCPN_CHART_TILE_HELPERS", saved_flag);
  else
    wxUnsetEnv("OCPN_CHART_TILE_HELPERS");
}
#endif

namespace {
struct GridEvictionFixture {
  decltype(s_segment_safety_grid_cache) grid =
      std::move(s_segment_safety_grid_cache);
  decltype(s_segment_safety_pinned_grid_keys) pins =
      std::move(s_segment_safety_pinned_grid_keys);
  decltype(s_segment_safety_pinned_route_mask_keys) masks =
      std::move(s_segment_safety_pinned_route_mask_keys);
  decltype(s_segment_safety_grid_eviction) index =
      std::move(s_segment_safety_grid_eviction);
  decltype(s_segment_safety_external_tile_cache) callbacks =
      s_segment_safety_external_tile_cache;
  bool persistent = s_segment_safety_persistent_cache_enabled;
  decltype(s_segment_safety_grid_cache_evictions) evictions =
      s_segment_safety_grid_cache_evictions;
  GridEvictionFixture() {
    s_segment_safety_grid_cache.clear();
    s_segment_safety_pinned_grid_keys.clear();
    s_segment_safety_pinned_route_mask_keys.clear();
    s_segment_safety_grid_eviction.Reset();
    s_segment_safety_external_tile_cache = {};
    s_segment_safety_persistent_cache_enabled = false;
  }
  ~GridEvictionFixture() {
    s_segment_safety_grid_cache = std::move(grid);
    s_segment_safety_pinned_grid_keys = std::move(pins);
    s_segment_safety_pinned_route_mask_keys = std::move(masks);
    s_segment_safety_grid_eviction = std::move(index);
    s_segment_safety_external_tile_cache = callbacks;
    s_segment_safety_persistent_cache_enabled = persistent;
    s_segment_safety_grid_cache_evictions = evictions;
  }
};
CachedPointSafetyGridTile CacheFixtureTile(long lat, long lon) {
  CachedPointSafetyGridTile tile;
  tile.built = true;
  tile.lat_tile = lat;
  tile.lon_tile = lon;
  tile.group_index = SegmentSafetyCurrentGroupIndex();
  tile.rows = tile.cols = 1;
  tile.classes = {kPointWater};
  tile.hazard_flags = {kHazardNone};
  tile.has_depth = {1};
  tile.min_depth_m = {200.0f};
  tile.has_drying = {0};
  tile.water_count = 1;
  tile.depth_complete = true;
  tile.source = kSourceCm93;
  return tile;
}
}  // namespace
TEST(ChartGridEviction, OverflowPinsKeepPayloadsAndEvictionOrder) {
  GridEvictionFixture fixture;
  std::set<std::pair<long, long>> coords;
  for (int i = 0; i < 4200; ++i) coords.emplace(900 + i / 64, -1400 + i % 64);
  PinSegmentSafetyRouteMaskTiles(coords, 0, false, 0);
  for (auto xy : coords) {
    auto tile = CacheFixtureTile(xy.first, xy.second);
    StoreSegmentSafetyGridTile(
        SegmentSafetyGridTileKeyForIndices(xy.first, xy.second), tile);
  }
  EXPECT_EQ(s_segment_safety_grid_cache.size(), coords.size());
  for (int i = 0; i < 40; ++i) {
    auto key = SegmentSafetyGridTileKeyForIndices(1200, -1000 + i);
    StoreSegmentSafetyGridTile(key, CacheFixtureTile(1200, -1000 + i));
    ASSERT_EQ(s_segment_safety_grid_cache.size(), coords.size() + 1);
    if (i) {
      EXPECT_FALSE(s_segment_safety_grid_cache.count(
          SegmentSafetyGridTileKeyForIndices(1200, -1001 + i)));
    }
  }
  for (auto xy : coords) {
    CachedPointSafetyGridTile tile;
    ASSERT_TRUE(LookupSegmentSafetyGridTile(
        SegmentSafetyGridTileKeyForIndices(xy.first, xy.second), &tile));
    EXPECT_EQ(tile.classes, std::vector<unsigned char>{kPointWater});
    EXPECT_EQ(tile.min_depth_m, std::vector<float>{200.0f});
  }
  ReleasePins();
  ASSERT_EQ(s_segment_safety_grid_cache.size(), kMaxGridTiles);
  const auto victim = s_segment_safety_grid_cache.begin()->first;
  StoreSegmentSafetyGridTile(SegmentSafetyGridTileKeyForIndices(1300, -1200),
                             CacheFixtureTile(1300, -1200));
  EXPECT_FALSE(s_segment_safety_grid_cache.count(victim));
}
TEST(ChartGridEviction, LatePinRemovesTheSameFirstEligibleKey) {
  GridEvictionFixture fixture;
  for (size_t i = 0; i <= kMaxGridTiles; ++i) {
    long lat = 900 + i / 64, lon = -1400 + i % 64;
    StoreSegmentSafetyGridTile(SegmentSafetyGridTileKeyForIndices(lat, lon),
                               CacheFixtureTile(lat, lon));
  }
  ASSERT_EQ(s_segment_safety_grid_cache.size(), kMaxGridTiles);
  const auto protected_key = s_segment_safety_grid_cache.begin()->first;
  const auto protected_tile = s_segment_safety_grid_cache.begin()->second;
  const auto victim = std::next(s_segment_safety_grid_cache.begin())->first;
  PinSegmentSafetyRouteMaskTiles(
      {{protected_tile.lat_tile, protected_tile.lon_tile}}, 0, false, 0);
  StoreSegmentSafetyGridTile(SegmentSafetyGridTileKeyForIndices(1300, -1100),
                             CacheFixtureTile(1300, -1100));
  EXPECT_TRUE(s_segment_safety_grid_cache.count(protected_key));
  EXPECT_FALSE(s_segment_safety_grid_cache.count(victim));
}

#include "../gui/src/chart_safety_compat_abi.h"
extern "C" bool PlugIn_RegisterSegmentSafetyTileCache(
    const PlugInSegmentSafetyTileCacheCallbacks* callbacks);
TEST(ChartSafetyOptionalAbi,
     AdaptsIntegerCallbacksWithoutFunctionPointerCasts) {
  const auto saved = s_segment_safety_external_tile_cache;
  struct State {
    int depth{-1};
    int stores{0};
    int identities{0};
    int changes{0};
  } state;
  PlugInSegmentSafetyTileCacheCallbacks callbacks{};
  callbacks.struct_size = sizeof(callbacks);
  callbacks.context = &state;
  callbacks.lookup = [](void* context, long lat, long lon, int depth,
                        PlugInSegmentSafetyTile* tile) {
    static_cast<State*>(context)->depth = depth;
    tile->lat_tile = lat;
    tile->lon_tile = lon;
    tile->rows = 7;
    return 2;  // Every nonzero legacy success must become typed true.
  };
  callbacks.store = [](void* context, const PlugInSegmentSafetyTile* tile) {
    if (tile->rows == 7) ++static_cast<State*>(context)->stores;
  };
  callbacks.identity_changed = [](void* context, const char*) {
    ++static_cast<State*>(context)->identities;
  };
  callbacks.dependencies_changed = [](void* context) {
    ++static_cast<State*>(context)->changes;
  };
  ASSERT_TRUE(PlugIn_RegisterSegmentSafetyTileCache(&callbacks));
  const auto typed = s_segment_safety_external_tile_cache;
  SegmentSafetyTile tile{};
  tile.struct_size = sizeof(tile);
  EXPECT_TRUE(typed.lookup(typed.context, 12, -13, true, &tile));
  EXPECT_EQ(state.depth, 1);
  EXPECT_EQ(tile.lat_tile, 12);
  EXPECT_EQ(tile.lon_tile, -13);
  typed.store(typed.context, &tile);
  typed.dependencies_changed(typed.context);
  EXPECT_EQ(state.stores, 1);
  EXPECT_EQ(state.changes, 1);
  EXPECT_EQ(state.identities, 1);
  EXPECT_TRUE(typed.lookup(typed.context, 12, -13, false, &tile));
  EXPECT_EQ(state.depth, 0);
  ASSERT_TRUE(PlugIn_RegisterSegmentSafetyTileCache(nullptr));
  EXPECT_EQ(s_segment_safety_external_tile_cache.lookup, nullptr);
  s_segment_safety_external_tile_cache = saved;
}
