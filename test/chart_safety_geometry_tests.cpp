#include <gtest/gtest.h>
#include <climits>
#include <limits>
#include <random>
#include "gui/chart_safety_geometry.h"

using namespace ocpn::chart_safety;

TEST(ChartSafetyGeometry, TinyReefBetweenSamplesStillIntersectsCell) {
  EXPECT_TRUE(
      TriangleIntersectsCell({.1, .1}, {.2, .1}, {.1, .2}, {-.5, -.5, .5, .5}));
  EXPECT_FALSE(PointInTriangle({0, 0}, {.1, .1}, {.2, .1}, {.1, .2}));
}
TEST(ChartSafetyGeometry, PointAndLineDangersAreNeverAreaOnly) {
  for (auto name : {"UWTROC", "WRECKS", "OBSTRN"})
    EXPECT_TRUE(IsIsolatedDanger(name));
  EXPECT_FALSE(IsIsolatedDanger("DEPARE"));
  EXPECT_TRUE(SegmentIntersectsCell({-2, .1}, {2, .1}, {-.5, -.5, .5, .5}));
  EXPECT_FALSE(SegmentIntersectsCell({-2, 1}, {2, 1}, {-.5, -.5, .5, .5}));
}
TEST(ChartSafetyGeometry, SharedEdgeAndCornerRemainHazards) {
  EXPECT_TRUE(TriangleIntersectsCell({0, 0}, {1, 0}, {0, 1}, {1, 0, 2, 1}));
  EXPECT_TRUE(SegmentIntersectsCell({-1, -1}, {0, 0}, {0, 0, 1, 1}));
}
TEST(ChartSafetyGeometry, PositiveDepthProofNeedsWholeBoxInOneTriangle) {
  EXPECT_TRUE(
      TriangleContainsCell({-10, -10}, {10, -10}, {0, 20}, {-.1, -.1, .1, .1}));
  EXPECT_FALSE(TriangleContainsCell({0, 0}, {10, 0}, {0, 10}, {1, 1, 9, 9}));
  EXPECT_FALSE(TriangleContainsCell({0, 0}, {10, 0}, {0, 10}, {0, 0, 1, 1}));
}
TEST(ChartSafetyGeometry, DegenerateTrianglesNeverCertifyWater) {
  EXPECT_FALSE(TriangleContainsCell({0, 0}, {5, 5}, {10, 10}, {4, 4, 6, 6}));
  EXPECT_FALSE(TriangleIntersectsCell({0, 0}, {5, 5}, {10, 10}, {0, 9, 1, 10}));
  EXPECT_TRUE(TriangleIntersectsCell({0, 0}, {5, 5}, {10, 10}, {4, 4, 6, 6}));
}
TEST(ChartSafetyGeometry, HoleWhollyInsideRegionPreventsCertificate) {
  std::vector<std::vector<Point>> rings = {
      {{-10, -10}, {10, -10}, {10, 10}, {-10, 10}},
      {{.1, .1}, {.2, .1}, {.2, .2}, {.1, .2}}};
  EXPECT_TRUE(BoundaryMayIntersectCell(rings, {-1, -1, 1, 1}));
  EXPECT_FALSE(PointInsideRings(rings, {.15, .15}));
  EXPECT_TRUE(PointInsideRings(rings, {0, 0}));
}
TEST(ChartSafetyGeometry, FourClearCornersCannotHideTinyCoverageHole) {
  std::vector<std::vector<Point>> hole = {
      {{.1, .1}, {.2, .1}, {.2, .2}, {.1, .2}}};
  EXPECT_TRUE(BoundaryMayIntersectCell(hole, {-1, -1, 1, 1}));
}
TEST(ChartSafetyGeometry, RemoteVertexAxisDoesNotCreateCoverage) {
  const std::vector<std::vector<Point>> rings = {
      {{10, 0}, {11, 0}, {11, 1}, {10, 1}}};
  EXPECT_FALSE(PointInsideRings(rings, {0, 0}));
  EXPECT_FALSE(PointInsideRings(rings, {10, 5}));
}
TEST(ChartSafetyGeometry, MissingAndNonFiniteGeometryCannotProveClear) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  EXPECT_FALSE(
      TriangleContainsCell({nan, 0}, {1, 0}, {0, 1}, {.1, .1, .2, .2}));
  EXPECT_TRUE(
      TriangleIntersectsCell({nan, 0}, {1, 0}, {0, 1}, {.1, .1, .2, .2}));
  EXPECT_TRUE(BoundaryMayIntersectCell({}, {0, 0, 1, 1}));
  EXPECT_FALSE(PointInsideRings({{{0, 0}, {1, nan}, {0, 1}}}, {.1, .1}));
}
// An independent integer separating-axis oracle checks all rectangle and
// triangle edge normals. It does not call the implementation's edge tests.
static bool Oracle(Point a, Point b, Point c, CellBox box) {
  const Point corners[] = {{box.min_x, box.min_y},
                           {box.max_x, box.min_y},
                           {box.max_x, box.max_y},
                           {box.min_x, box.max_y}};
  const Point triangle[] = {a, b, c};
  const Point axes[] = {{1, 0},
                        {0, 1},
                        {a.y - b.y, b.x - a.x},
                        {b.y - c.y, c.x - b.x},
                        {c.y - a.y, a.x - c.x}};
  for (auto axis : axes) {
    long long tmin = LLONG_MAX, tmax = LLONG_MIN, bmin = LLONG_MAX,
              bmax = LLONG_MIN;
    for (auto p : triangle) {
      long long v = p.x * axis.x + p.y * axis.y;
      tmin = std::min(tmin, v);
      tmax = std::max(tmax, v);
    }
    for (auto p : corners) {
      long long v = p.x * axis.x + p.y * axis.y;
      bmin = std::min(bmin, v);
      bmax = std::max(bmax, v);
    }
    if (tmax < bmin || bmax < tmin) return false;
  }
  return true;
}
TEST(ChartSafetyGeometry, FiftyThousandIndependentIntersectionComparisons) {
  std::mt19937 random(20261007);
  std::uniform_int_distribution<int> coordinate(-100, 100);
  for (int i = 0; i < 50000; ++i) {
    Point a{double(coordinate(random)), double(coordinate(random))};
    Point b{double(coordinate(random)), double(coordinate(random))};
    Point c{double(coordinate(random)), double(coordinate(random))};
    if (Cross(a, b, c) == 0) continue;
    int x = coordinate(random), y = coordinate(random);
    CellBox box{double(x), double(y), double(x + 3), double(y + 4)};
    ASSERT_EQ(TriangleIntersectsCell(a, b, c, box), Oracle(a, b, c, box))
        << "case " << i;
  }
}

TEST(ChartSafetyGeometry, TessellationInternalDiagonalDoesNotPreventCoverage) {
  PreparedGeometry g;
  g.valid = true;
  g.triangles = {{{Point{0, 0}, Point{2, 0}, Point{2, 2}}},
                 {{Point{0, 0}, Point{2, 2}, Point{0, 2}}}};
  BuildTriangleBoundary(&g);
  EXPECT_TRUE(TessellationContainsCell(g, {.9, .9, 1.1, 1.1}));
  EXPECT_FALSE(TessellationContainsCell(g, {.9, .9, 2.1, 2.1}));
}
TEST(ChartSafetyGeometry, DuplicateTrianglesCannotEraseBoundaryEvidence) {
  PreparedGeometry g;
  g.valid = true;
  g.triangles = {{{Point{0, 0}, Point{2, 0}, Point{0, 2}}},
                 {{Point{0, 0}, Point{2, 0}, Point{0, 2}}}};
  BuildTriangleBoundary(&g);
  EXPECT_FALSE(g.boundary_valid);
  EXPECT_FALSE(TessellationContainsCell(g, {-.1, -.1, .1, .1}));
}
TEST(ChartSafetyGeometry, TinyTessellatedHoleCannotBecomeWholeAreaWater) {
  PreparedGeometry g;
  g.valid = true;
  const Point outer[] = {{0, 0}, {4, 0}, {4, 4}, {0, 4}};
  const Point inner[] = {
      {1.99, 1.99}, {2.01, 1.99}, {2.01, 2.01}, {1.99, 2.01}};
  for (int i = 0; i < 4; ++i) {
    int j = (i + 1) % 4;
    g.triangles.push_back({outer[i], outer[j], inner[j]});
    g.triangles.push_back({outer[i], inner[j], inner[i]});
  }
  BuildTriangleBoundary(&g);
  ASSERT_TRUE(g.boundary_valid);
  EXPECT_FALSE(TessellationContainsCell(g, {1, 1, 3, 3}));
  EXPECT_TRUE(TessellationContainsCell(g, {.5, .5, 1, 1}));
}

TEST(ChartSafetyGeometry,
     TenThousandIndependentRectangleAndHoleCoverageComparisons) {
  std::mt19937 rng(1937);
  std::uniform_int_distribution<int> coordinate(-10, 110), length(1, 20);
  for (bool hole : {false, true}) {
    PreparedGeometry g;
    g.valid = true;
    const Point outer[] = {{0, 0}, {100, 0}, {100, 100}, {0, 100}};
    const Point inner[] = {{40, 40}, {60, 40}, {60, 60}, {40, 60}};
    if (hole)
      for (int i = 0; i < 4; ++i) {
        int j = (i + 1) % 4;
        g.triangles.push_back({outer[i], outer[j], inner[j]});
        g.triangles.push_back({outer[i], inner[j], inner[i]});
      }
    else {
      g.triangles.push_back({outer[0], outer[1], outer[2]});
      g.triangles.push_back({outer[0], outer[2], outer[3]});
    }
    BuildTriangleBoundary(&g);
    ASSERT_TRUE(g.boundary_valid);
    for (int n = 0; n < 5000; ++n) {
      const double x = coordinate(rng), y = coordinate(rng);
      CellBox box{x, y, x + length(rng), y + length(rng)};
      const bool inside =
          box.min_x > 0 && box.max_x < 100 && box.min_y > 0 && box.max_y < 100;
      const bool outside_hole =
          box.max_x < 40 || box.min_x > 60 || box.max_y < 40 || box.min_y > 60;
      const bool expected = inside && (!hole || outside_hole);
      ASSERT_EQ(TessellationContainsCell(g, box), expected);
    }
  }
}

TEST(ChartSafetyGeometry, DegenerateOutlyingEdgeCannotCertifyArea) {
  PreparedGeometry g;
  g.valid = true;
  g.triangles = {{{Point{0, 0}, Point{2, 0}, Point{0, 2}}},
                 {{Point{10, 10}, Point{11, 11}, Point{12, 12}}}};
  BuildTriangleBoundary(&g);
  EXPECT_FALSE(TessellationContainsCell(g, {10.9, 10.9, 11.1, 11.1}));
  g.triangles[0][0].x = std::numeric_limits<double>::quiet_NaN();
  BuildTriangleBoundary(&g);
  EXPECT_FALSE(g.boundary_valid);
}

TEST(ChartSafetyGeometry,
     TenThousandIndexedPointQueriesMatchIndependentFullScan) {
  PreparedGeometry g;
  g.valid = true;
  g.kind = PreparedGeometry::kPoints;
  std::mt19937 rng(8103);
  std::uniform_real_distribution<double> lon(-180, 180), lat(-70, 70),
      offset(-.002, .002);
  for (int i = 0; i < 1000; ++i) g.points.push_back({lon(rng), lat(rng)});
  g.points.push_back({179.999999, .01});
  g.points.push_back({-179.999999, -.01});
  ASSERT_TRUE(BuildPointIndex(&g));
  ASSERT_FALSE(g.point_bins.empty());
  for (int n = 0; n < 10000; ++n) {
    const auto target = g.points[n % g.points.size()];
    const double x = target.x + offset(rng), y = target.y + offset(rng),
                 r = .0009575;
    CellBox box{x - r, y - r, x + r, y + r};
    std::vector<size_t> expected, actual;
    for (size_t i = 0; i < g.points.size(); ++i) {
      auto p = g.points[i];
      p.x = x + std::remainder(p.x - x, 360.0);
      if (box.Contains(p)) expected.push_back(i);
    }
    VisitPointsInCell(g, box, [&](size_t i) {
      actual.push_back(i);
      return false;
    });
    std::sort(actual.begin(), actual.end());
    ASSERT_EQ(actual, expected);
  }
}
TEST(ChartSafetyGeometry, IndexedSoundingDepthUsesOnlyMatchingPoints) {
  PreparedGeometry g;
  g.valid = true;
  g.soundings = true;
  for (int i = 0; i < 64; ++i) {
    g.points.push_back({double(i), 1});
    g.depths.push_back(1000);
  }
  g.points.push_back({.0254, .0254});
  g.depths.push_back(.25);
  g.points.push_back({179.9999, .025});
  g.depths.push_back(std::numeric_limits<double>::quiet_NaN());
  ASSERT_TRUE(BuildPointIndex(&g));
  double minimum = 0;
  bool unknown = false;
  EXPECT_TRUE(SoundingMinDepth(g, .025, .025, .001, &minimum, &unknown));
  EXPECT_DOUBLE_EQ(minimum, .25);
  EXPECT_FALSE(unknown);
  EXPECT_FALSE(SoundingMinDepth(g, .025, -179.9999, .001, &minimum, &unknown));
  EXPECT_TRUE(unknown);
}
