// Research-only integration tests: production CommDecoder and N2KParser.
// Do not propose upstream changes without checking the independent PGN 129025
// path and per-source failover semantics.
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include "N2kMessages.h"
#include "model/comm_decoder.h"

namespace {
std::vector<unsigned char> ToActisenseFrame(const tN2kMsg& msg) {
  // Matches the frame layout consumed by N2KParser::MakeN2kMsg.
  std::vector<unsigned char> frame;
  frame.push_back(0x93);
  frame.push_back(static_cast<unsigned char>(msg.DataLen + 11));
  frame.push_back(msg.Priority);
  for (int n = 0; n < 3; ++n)
    frame.push_back(static_cast<unsigned char>((msg.PGN >> (8 * n)) & 0xff));
  frame.push_back(msg.Destination);
  frame.push_back(msg.Source);
  for (int n = 0; n < 4; ++n)
    frame.push_back(static_cast<unsigned char>((msg.MsgTime >> (8 * n)) & 0xff));
  frame.push_back(static_cast<unsigned char>(msg.DataLen));
  for (int n = 0; n < msg.DataLen; ++n) frame.push_back(msg.Data[n]);
  frame.push_back(0);  // ignored framing checksum in this parser
  return frame;
}

NavData EmptyNavData() {
  NavData n{};
  n.gLat = std::numeric_limits<double>::quiet_NaN();
  n.gLon = std::numeric_limits<double>::quiet_NaN();
  n.n_satellites = -1;
  return n;
}
}  // namespace

TEST(GNSSValidityResearch, ExplicitlyInvalidMethodsCannotUpdatePosition) {
  CommDecoder decoder;
  for (int code : {0, 14, 15}) {
    SCOPED_TRACE(code);
    tN2kMsg msg;
    SetN2kPGN129029(msg, 7, 20000, 1.0, 35.6812, 139.7671, 12.0,
                    N2kGNSSt_GPS, static_cast<tN2kGNSSmethod>(code),
                    7, 0.8);
    auto data = EmptyNavData();
    EXPECT_FALSE(decoder.DecodePGN129029(ToActisenseFrame(msg), data));
    EXPECT_TRUE(std::isnan(data.gLat));
    EXPECT_TRUE(std::isnan(data.gLon));
  }
}

TEST(GNSSValidityResearch, NormalAndRtkMethodsStillUpdatePosition) {
  CommDecoder decoder;
  for (int code : {1, 2, 3, 4, 5}) {
    SCOPED_TRACE(code);
    tN2kMsg msg;
    SetN2kPGN129029(msg, 7, 20000, 1.0, 35.6812, 139.7671, 12.0,
                    N2kGNSSt_GPS, static_cast<tN2kGNSSmethod>(code),
                    7, 0.8);
    auto data = EmptyNavData();
    ASSERT_TRUE(decoder.DecodePGN129029(ToActisenseFrame(msg), data));
    EXPECT_NEAR(data.gLat, 35.6812, 1e-7);
    EXPECT_NEAR(data.gLon, 139.7671, 1e-7);
  }
}

TEST(GNSSValidityResearch, RapidPositionRemainsIndependent) {
  CommDecoder decoder;
  tN2kMsg msg;
  SetN2kPGN129025(msg, 35.6812, 139.7671);
  auto data = EmptyNavData();
  EXPECT_TRUE(decoder.DecodePGN129025(ToActisenseFrame(msg), data));
  EXPECT_NEAR(data.gLat, 35.6812, 1e-7);
  EXPECT_NEAR(data.gLon, 139.7671, 1e-7);
}
