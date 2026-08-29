#include <array>
#include <limits>

#include <gtest/gtest.h>

#include "gui/chart_safety_depth.h"

namespace {

using ocpn::chart_safety::ConservativeBoundaryDepth;
using ocpn::chart_safety::DepthProbe;
using ocpn::chart_safety::DepthProbeClass;

std::array<DepthProbe, 4> SafeProbes() {
  return {{{DepthProbeClass::kWater, true, 12.0},
           {DepthProbeClass::kWater, true, 7.5},
           {DepthProbeClass::kWater, true, 19.0},
           {DepthProbeClass::kWater, true, 8.0}}};
}

TEST(ChartSafetyDepth, UsesMinimumOfFourValidWaterProbes) {
  const auto depth = ConservativeBoundaryDepth(SafeProbes());

  ASSERT_TRUE(depth.has_value());
  EXPECT_DOUBLE_EQ(*depth, 7.5);
}

TEST(ChartSafetyDepth, MissingDepthRemainsUnknown) {
  auto probes = SafeProbes();
  probes[2].has_depth = false;

  EXPECT_FALSE(ConservativeBoundaryDepth(probes).has_value());
}

TEST(ChartSafetyDepth, LandDryingAndNoDataRemainUnsafe) {
  for (const DepthProbeClass point_class : {
           DepthProbeClass::kLand, DepthProbeClass::kDrying,
           DepthProbeClass::kNoData}) {
    auto probes = SafeProbes();
    probes[1].point_class = point_class;
    EXPECT_FALSE(ConservativeBoundaryDepth(probes).has_value());
  }
}

TEST(ChartSafetyDepth, NonFiniteDepthRemainsUnknown) {
  auto probes = SafeProbes();
  probes[0].depth_m = std::numeric_limits<double>::quiet_NaN();

  EXPECT_FALSE(ConservativeBoundaryDepth(probes).has_value());
}

}  // namespace
