#include "config.h"

#include <chrono>

#include <wx/app.h>
#include <wx/fileconf.h>
#include <wx/event.h>
#include <wx/evtloop.h>

#include <gtest/gtest.h>

#include "model/base_platform.h"
#include "model/config_vars.h"
#include "model/navmsg_filter.h"
#include "model/logger.h"
#include "model/ocpn_utils.h"

using namespace std::literals::chrono_literals;

static std::string s_result;

static wxLogStderr default_log;

TEST(Filter, Create) {
  using Direction = NavmsgStatus::Direction;
  using Accepted = NavmsgStatus::Accepted;
  using State = NavmsgStatus::State;

  wxLog::SetActiveTarget(&default_log);
  NavmsgFilter filter;
  filter.m_name = "test-1";
  filter.m_description = "A useless test filter";

  filter.buses.insert(NavAddr::Bus::Plugin);
  filter.buses.insert(NavAddr::Bus::N0183);
  filter.buses.insert(NavAddr::Bus::N2000);

  filter.directions.insert(Direction::kHandled);
  filter.directions.insert(Direction::kOutput);

  filter.status.insert(State::kOk);

  filter.include_msg.insert("GGA");
  filter.include_msg.insert("VDM");

  filter.accepted.insert(NavmsgStatus::Accepted::kOk);

  filter.pgns.insert(27211);
  filter.src_pgns.insert(27213);

  std::string string = filter.to_string();
  std::ofstream stream("filter.json");
  stream << string << "\n";

  filter = NavmsgFilter::Parse(string);
  ASSERT_TRUE(filter.buses.find(NavAddr::Bus::N0183) != filter.buses.end());
  ASSERT_TRUE(filter.buses.find(NavAddr::Bus::Signalk) == filter.buses.end());
  ASSERT_TRUE(filter.directions.find(Direction::kInput) ==
              filter.directions.end());
  ASSERT_TRUE(filter.directions.find(Direction::kOutput) !=
              filter.directions.end());

  ASSERT_TRUE(filter.accepted.find(Accepted::kOk) != filter.accepted.end());
  ASSERT_TRUE(filter.accepted.find(Accepted::kFilteredNoOutput) ==
              filter.accepted.end());

  ASSERT_TRUE(filter.status.find(State::kOk) != filter.status.end());
  ASSERT_TRUE(filter.status.find(State::kMalformed) == filter.status.end());

  ASSERT_TRUE(filter.include_msg.find("GGA") != filter.include_msg.end());
  ASSERT_TRUE(filter.include_msg.find("CGN") == filter.include_msg.end());

  ASSERT_TRUE(filter.pgns.find(27211) != filter.pgns.end());
  ASSERT_TRUE(filter.pgns.find(27212) == filter.pgns.end());

  ASSERT_TRUE(filter.src_pgns.find(27213) != filter.src_pgns.end());
  ASSERT_TRUE(filter.src_pgns.find(27214) == filter.src_pgns.end());
}

TEST(Filter, ListSystemFilters) {
  auto filters = NavmsgFilter::GetFilters(FILTER_DIR);
  ASSERT_EQ(filters.size(), 7);
}
