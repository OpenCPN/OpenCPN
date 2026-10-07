/* GPL-2.0-or-later. Private GUI/core integration harness.
 * Compiled in place of ocpn_plugin_gui.cpp by the prototype qualification
 * script, so tests exercise its private production classifier without adding
 * test exports or changing the plugin ABI. Linked against the complete host. */
#include <gtest/gtest.h>
#include "../gui/src/chart_safety_raster.cpp"
using namespace ocpn::chart_safety;
using namespace ocpn::chart_safety::detail;
#include "mygeom.h"
#include <wx/init.h>

namespace {
class FixtureChart : public cm93chart {
public:
  FixtureChart() {
    m_RAZBuilt = true;
    s_segment_safety_point_cache.clear();
  }
  ~FixtureChart() {
    for (int i = 0; i < PRIO_NUM; ++i)
      for (int j = 0; j < LUPNAME_NUM; ++j) razRules[i][j] = nullptr;
  }
  S57Obj *Add(const char *name, int primitive, int style = 4) {
    auto obj = std::make_unique<S57Obj>(name);
    obj->Primitive_type = static_cast<GeoPrim_t>(primitive);
    obj->Scamin = 1;
    obj->m_DisplayCat = OTHER;  // deliberately hidden at normal scale
    auto rule = std::unique_ptr<ObjRazRules>(new ObjRazRules{});
    rule->obj = obj.get();
    rule->next = razRules[0][style];
    razRules[0][style] = rule.get();
    auto *result = obj.get();
    objects.push_back(std::move(obj));
    rules.push_back(std::move(rule));
    return result;
  }
  void Point(S57Obj *obj, double lat = .0254, double lon = .0254) {
    obj->npt = 1;
    obj->x_rate = obj->y_rate = 1;
    obj->x_origin = obj->y_origin = 0;
    toSM(lat, lon, ref_lat, ref_lon, &obj->x, &obj->y);
    obj->BBObj.Set(lat, lon, lat, lon);
  }
  void Line(S57Obj *obj) {
    obj->npt = 2;
    obj->geoPt = static_cast<pt *>(calloc(2, sizeof(pt)));
    obj->x_rate = obj->y_rate = 1;
    obj->x_origin = obj->y_origin = 0;
    double x, y;
    toSM(.0254, .024, ref_lat, ref_lon, &x, &y);
    obj->geoPt[0].x = x;
    obj->geoPt[0].y = y;
    toSM(.0254, .027, ref_lat, ref_lon, &x, &y);
    obj->geoPt[1].x = x;
    obj->geoPt[1].y = y;
    obj->BBObj.Set(.0254, .024, .0254, .027);
  }
  void Triangle(S57Obj *obj, bool ocean = false) {
    const double lats[3] = {ocean ? -1.0 : .0252, ocean ? -1.0 : .0252,
                            ocean ? 2.0 : .0255};
    const double lons[3] = {ocean ? -1.0 : .0252, ocean ? 2.0 : .0255,
                            ocean ? -1.0 : .0252};
    obj->pPolyTessGeo = new PolyTessGeo;
    auto *group = new PolyTriGroup;
    group->m_bSMSENC = true;
    group->data_type = DATA_TYPE_DOUBLE;
    auto *triangle = new TriPrim;
    triangle->p_next = nullptr;
    triangle->type = PTG_TRIANGLES;
    triangle->nVert = 3;
    triangle->p_vertex = static_cast<double *>(calloc(6, sizeof(double)));
    for (int i = 0; i < 3; ++i)
      toSM(lats[i], lons[i], ref_lat, ref_lon, &triangle->p_vertex[2 * i],
           &triangle->p_vertex[2 * i + 1]);
    group->tri_prim_head = triangle;
    obj->pPolyTessGeo->Set_PolyTriGroup_head(group);
    obj->pPolyTessGeo->Set_OK(true);
    obj->BBObj.Set(
        *std::min_element(lats, lats + 3), *std::min_element(lons, lons + 3),
        *std::max_element(lats, lats + 3), *std::max_element(lons, lons + 3));
  }
  void Coverage(double minlat, double minlon, double maxlat, double maxlon) {
    auto c = std::make_unique<M_COVR_Desc>();
    c->m_nvertices = 4;
    c->pvertices = new float_2Dpt[4];
    c->pvertices[0].x = minlon;
    c->pvertices[0].y = minlat;
    c->pvertices[1].x = maxlon;
    c->pvertices[1].y = minlat;
    c->pvertices[2].x = maxlon;
    c->pvertices[2].y = maxlat;
    c->pvertices[3].x = minlon;
    c->pvertices[3].y = maxlat;
    m_pcovr_array_loaded.Add(c.get());
    coverage.push_back(std::move(c));
  }
  std::vector<std::unique_ptr<M_COVR_Desc>> coverage;
  void Ocean() {
    auto *obj = Add("DEPARE", GEO_AREA);
    Triangle(obj, true);
    obj->AddDoubleAttribute("DRVAL1", 1000);
  }
  std::vector<std::unique_ptr<S57Obj>> objects;
  std::vector<std::unique_ptr<ObjRazRules>> rules;
};
SegmentSafetyResult Classify(FixtureChart &chart) {
  auto rules = CollectCm93SafetyTileRules(&chart, 0, 0);
  ViewPort vp;
  SegmentSafetyResult result{};
  result.struct_size = sizeof(result);
  InitSegmentSafetyResult(&result);
  SegmentSafetySource source;
  auto cls = ChartPointSafetyClassAtPreparedCm93(
      &chart, 123, .025, .025, &vp, &source, nullptr, &result, rules);
  result.status = cls;
  return result;
}
TEST(ChartSafetyNative, CoverageProofUsesCompleteOriginalPolygons) {
  FixtureChart chart;
  chart.Coverage(-1, 359, 2, 362);
  EXPECT_EQ(chart.SafetyCoverageBoxRelation(.024, .026, .024, .026), 2);
  EXPECT_EQ(chart.SafetyCoverageBoxRelation(-1.1, -.9, .024, .026), 1);
  EXPECT_EQ(chart.SafetyCoverageBoxRelation(10, 11, .024, .026), 0);
  chart.coverage[0]->user_xoff = 1;
  EXPECT_EQ(chart.SafetyCoverageBoxRelation(.024, .026, .024, .026), -1);
}
TEST(ChartSafetyNative,
     ExtractedHazardGeometrySurvivesChartMutationAndDestruction) {
  std::shared_ptr<const ocpn::chart_safety::PreparedGeometry> geometry;
  {
    FixtureChart chart;
    auto *danger = chart.Add("WRECKS", GEO_AREA);
    chart.Triangle(danger);
    geometry = chart.PrepareSafetyObjectGeometry(danger);
    danger->pPolyTessGeo->Get_PolyTriGroup_head()->tri_prim_head->p_vertex[0] =
        1e10;
  }
  EXPECT_EQ(s57chart::PreparedSafetyBoxRelation(*geometry, .024, .026, .024,
                                                .026, false),
            1);
}
TEST(ChartSafetyNative, DeepUniformAreaHasGeometricProof) {
  FixtureChart chart;
  chart.Ocean();
  double minimum = 0;
  EXPECT_TRUE(Cm93UniformDepthProof(CollectCm93SafetyTileRules(&chart, 0, 0),
                                    &minimum));
  EXPECT_DOUBLE_EQ(minimum, 1000);
}
TEST(ChartSafetyNative, GeographicPrefilterCannotHideIsolatedDangers) {
  for (auto name : {"UWTROC", "WRECKS", "OBSTRN"})
    for (auto primitive : {GEO_POINT, GEO_LINE, GEO_AREA}) {
      FixtureChart chart;
      auto *danger = chart.Add(name, primitive);
      if (primitive == GEO_POINT)
        chart.Point(danger);
      else if (primitive == GEO_LINE)
        chart.Line(danger);
      else
        chart.Triangle(danger);
      EXPECT_TRUE(s57chart::SafetyObjectMayIntersectBox(danger, .024, .026,
                                                        .024, .026));
      EXPECT_EQ(s57chart::SafetyObjectMayIntersectBox(danger, 10, 11, 10, 11),
                primitive == GEO_POINT);
      // Point bounds are not authoritative. An inconsistent point box must
      // not suppress the actual nearby coordinate.
      danger->BBObj.Set(10, 10, 11, 11);
      if (primitive == GEO_POINT) {
        EXPECT_TRUE(s57chart::SafetyObjectMayIntersectBox(danger, .024, .026,
                                                          .024, .026));
      }
    }
  S57Obj unresolved("OBSTRN");
  unresolved.Primitive_type = GEO_AREA;
  EXPECT_TRUE(s57chart::SafetyObjectMayIntersectBox(&unresolved, .024, .026,
                                                    .024, .026));
  unresolved.BBObj.Set(.02, 359.98, .03, 360.03);
  EXPECT_TRUE(s57chart::SafetyObjectMayIntersectBox(&unresolved, .024, .026,
                                                    -.01, .01));
}
TEST(ChartSafetyNative, EveryIsolatedDangerPrimitiveOverridesDeepWater) {
  for (auto name : {"UWTROC", "WRECKS", "OBSTRN"})
    for (auto primitive : {GEO_POINT, GEO_LINE, GEO_AREA}) {
      SCOPED_TRACE(std::string(name) +
                   " primitive=" + std::to_string(primitive));
      FixtureChart chart;
      chart.Ocean();
      auto *danger = chart.Add(name, primitive,
                               primitive == GEO_POINT  ? 1
                               : primitive == GEO_LINE ? 2
                                                       : 4);
      if (primitive == GEO_POINT)
        chart.Point(danger);
      else if (primitive == GEO_LINE)
        chart.Line(danger);
      else
        chart.Triangle(danger);
      danger->AddDoubleAttribute("VALSOU", 1.0);
      auto result = Classify(chart);
      EXPECT_EQ(result.status, kPointWater);
      EXPECT_EQ(result.has_depth, 1);
      EXPECT_DOUBLE_EQ(result.min_depth_m, 1);
      EXPECT_NE(std::string(result.depth_source_attribute).find("VALSOU"),
                std::string::npos);
      double minimum = 0;
      EXPECT_FALSE(Cm93UniformDepthProof(
          CollectCm93SafetyTileRules(&chart, 0, 0), &minimum));
      auto *raw = chart.GetSafetyRulesAtLatLon(.025, .025, .001);
      ASSERT_NE(raw, nullptr);
      EXPECT_GE(raw->GetCount(), 2u);
      delete raw;
    }
}
TEST(ChartSafetyNative, UnknownDangerDepthCannotBorrowDeepSurroundings) {
  for (auto name : {"UWTROC", "WRECKS", "OBSTRN"})
    for (auto primitive : {GEO_POINT, GEO_LINE, GEO_AREA}) {
      SCOPED_TRACE(std::string(name) +
                   " primitive=" + std::to_string(primitive));
      FixtureChart chart;
      chart.Ocean();
      auto *danger = chart.Add(name, primitive,
                               primitive == GEO_POINT  ? 1
                               : primitive == GEO_LINE ? 2
                                                       : 4);
      if (primitive == GEO_POINT)
        chart.Point(danger);
      else if (primitive == GEO_LINE)
        chart.Line(danger);
      else
        chart.Triangle(danger);
      auto result = Classify(chart);
      EXPECT_EQ(result.has_depth, 0);
      EXPECT_EQ(result.status, kPointNoData);
      EXPECT_NE(std::string(result.depth_source_attribute).find("unresolved"),
                std::string::npos);
    }
}
TEST(ChartSafetyNative, DeepKnownRockStillForbidsWholeTileShortcut) {
  FixtureChart chart;
  chart.Ocean();
  auto *rock = chart.Add("UWTROC", GEO_POINT, 0);
  chart.Point(rock);
  rock->AddDoubleAttribute("VALSOU", 50);
  double minimum = 0;
  EXPECT_FALSE(Cm93UniformDepthProof(CollectCm93SafetyTileRules(&chart, 0, 0),
                                     &minimum));
  EXPECT_DOUBLE_EQ(Classify(chart).min_depth_m, 50);
}
TEST(ChartSafetyNative, TinyLandAndDryingAreasBetweenSamplesAreRetained) {
  for (bool land : {true, false}) {
    FixtureChart chart;
    chart.Ocean();
    auto *hazard = chart.Add(land ? "LNDARE" : "OBSTRN", GEO_AREA);
    chart.Triangle(hazard);
    if (!land) hazard->AddIntegerAttribute("WATLEV", 4);
    EXPECT_EQ(Classify(chart).status, land ? kPointLand : kPointDrying);
  }
}
TEST(ChartSafetyNative, TinyShallowDepthAreaBetweenSamplesIsRetained) {
  FixtureChart chart;
  chart.Ocean();
  auto *reef = chart.Add("DEPARE", GEO_AREA);
  chart.Triangle(reef);
  reef->AddDoubleAttribute("DRVAL1", .5);
  EXPECT_DOUBLE_EQ(Classify(chart).min_depth_m, .5);
  double minimum = 0;
  EXPECT_FALSE(Cm93UniformDepthProof(CollectCm93SafetyTileRules(&chart, 0, 0),
                                     &minimum));
}
TEST(ChartSafetyNative, AllAreaTriangleEncodingsRetainIsolatedDangers) {
  for (auto name : {"UWTROC", "WRECKS", "OBSTRN"})
    for (auto type : {PTG_TRIANGLES, PTG_TRIANGLE_FAN, PTG_TRIANGLE_STRIP})
      for (bool use_float : {false, true}) {
        FixtureChart chart;
        chart.Ocean();
        auto *danger = chart.Add(name, GEO_AREA);
        chart.Triangle(danger);
        danger->AddDoubleAttribute("VALSOU", .25);
        auto *group = danger->pPolyTessGeo->Get_PolyTriGroup_head();
        auto *triangle = group->tri_prim_head;
        triangle->type = type;
        if (use_float) {
          auto *vertices = static_cast<float *>(calloc(6, sizeof(float)));
          for (int i = 0; i < 6; ++i) vertices[i] = triangle->p_vertex[i];
          free(triangle->p_vertex);
          triangle->p_vertex = reinterpret_cast<double *>(vertices);
          group->data_type = DATA_TYPE_FLOAT;
        }
        EXPECT_DOUBLE_EQ(Classify(chart).min_depth_m, .25);
      }
}
TEST(ChartSafetyNative, MissingDangerGeometryCannotBecomeClearWater) {
  FixtureChart chart;
  chart.Ocean();
  auto *danger = chart.Add("OBSTRN", GEO_AREA);
  danger->BBObj.Set(.025, .025, .026, .026);
  EXPECT_EQ(Classify(chart).status, kPointNoData);
}
TEST(ChartSafetyNative, DateLinePointAndAreaUseQueryLongitudeBranch) {
  FixtureChart chart;
  auto *point = chart.Add("UWTROC", GEO_POINT, 1);
  chart.Point(point, .025, 179.9999);
  EXPECT_GT(
      chart.SafetyObjectBoxRelation(point, .024, .026, -180.001, -179.999), 0);
  auto *area = chart.Add("WRECKS", GEO_AREA);
  chart.Triangle(area, true);
  // Shift a native chart to its equivalent 0..360 branch.
  area->x_origin = 360 * DEGREE * WGS84_semimajor_axis_meters * mercator_k0;
  area->x_rate = area->y_rate = 1;
  area->y_origin = 0;
  auto *group = area->pPolyTessGeo->Get_PolyTriGroup_head();
  group->m_bSMSENC = false;
  area->BBObj.Set(-1, 359, 2, 362);
  EXPECT_EQ(chart.SafetyObjectBoxRelation(area, .024, .026, .024, .026), 2);
}
TEST(ChartSafetyNative, ShallowSoundingOverridesAreaButCannotCertifyCoverage) {
  FixtureChart chart;
  chart.Ocean();
  auto *obj = chart.Add("SOUNDG", GEO_POINT, 1);
  obj->npt = 1;
  obj->geoPtMulti = static_cast<double *>(calloc(2, sizeof(double)));
  obj->geoPtz = static_cast<double *>(calloc(3, sizeof(double)));
  obj->geoPtMulti[0] = .0254;
  obj->geoPtMulti[1] = .0254;
  obj->geoPtz[2] = .75;
  auto result = Classify(chart);
  EXPECT_EQ(result.has_depth, 1);
  EXPECT_DOUBLE_EQ(result.min_depth_m, .75);
  double minimum = 0;
  EXPECT_FALSE(Cm93UniformDepthProof(CollectCm93SafetyTileRules(&chart, 0, 0),
                                     &minimum));
}
TEST(ChartSafetyNative, NonFiniteDangerDepthRemainsUnknown) {
  FixtureChart chart;
  chart.Ocean();
  auto *rock = chart.Add("UWTROC", GEO_POINT, 1);
  chart.Point(rock);
  rock->AddDoubleAttribute("VALSOU", std::numeric_limits<double>::quiet_NaN());
  EXPECT_EQ(Classify(chart).has_depth, 0);
}
}  // namespace
extern "C" int __wrap_main(int argc, char **argv) {
  wxApp::SetInstance(new wxApp());
  if (!wxEntryStart(argc, argv)) return 2;
  ::testing::InitGoogleTest(&argc, argv);
  const int result = RUN_ALL_TESTS();
  wxEntryCleanup();
  return result;
}

TEST(ChartSafetyNative, DeepSoundingsCannotWeakenUniformAreaMinimum) {
  FixtureChart chart;
  chart.Ocean();
  auto *obj = chart.Add("SOUNDG", GEO_POINT, 1);
  obj->npt = 1;
  obj->geoPtMulti = static_cast<double *>(calloc(2, sizeof(double)));
  obj->geoPtz = static_cast<double *>(calloc(3, sizeof(double)));
  obj->geoPtMulti[0] = .0254;
  obj->geoPtMulti[1] = .0254;
  obj->geoPtz[2] = 1500;
  double minimum = 0;
  EXPECT_TRUE(Cm93UniformDepthProof(CollectCm93SafetyTileRules(&chart, 0, 0),
                                    &minimum));
  EXPECT_DOUBLE_EQ(minimum, 1000);
  obj->geoPtz[2] = std::numeric_limits<double>::quiet_NaN();
  EXPECT_FALSE(Cm93UniformDepthProof(CollectCm93SafetyTileRules(&chart, 0, 0),
                                     &minimum));
}

TEST(ChartSafetyNative, TypedProviderVisitorPreservesUnknownDangerDepth) {
  for (bool known : {false, true}) {
    SegmentSafetyPluginBatchGroup group(1);
    group.active[0] = 1;
    const uint64_t hits = 1;
    ChartSafetyFeature area{};
    area.struct_size = sizeof(area);
    area.flags = HostApi123::kChartSafetyFeatureHasDepth;
    area.minimum_depth_m = 1000;
    SegmentSafetyPluginBatchVisit(&group, &area, &hits, 1);
    ChartSafetyFeature danger{};
    danger.struct_size = sizeof(danger);
    danger.flags = known ? HostApi123::kChartSafetyFeatureHasDepth
                         : HostApi123::kChartSafetyFeatureUnknownDangerDepth;
    danger.minimum_depth_m = 1;
    SegmentSafetyPluginBatchVisit(&group, &danger, &hits, 1);
    EXPECT_EQ(group.unknown_danger_depth[0], known ? 0 : 1);
    EXPECT_FLOAT_EQ(group.min_depth_m[0], known ? 1 : 1000);
  }
}
