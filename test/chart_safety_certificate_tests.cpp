/* GPL-2.0-or-later. Private production certificate-cache harness. */
#include <gtest/gtest.h>
#include "../gui/src/chart_safety_core.cpp"
#include <wx/init.h>
#include <limits>
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
