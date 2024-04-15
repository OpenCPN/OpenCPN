#include "config.h"

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#if (defined(__clang_major__) && (__clang_major__ < 15))   // MacOS 1.13
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#else
#include <filesystem>
#include <utility>
namespace fs = std::filesystem;
#endif

#include <wx/app.h>

#include <gtest/gtest.h>

#include "model/base_platform.h"
#include "model/comm_drv_registry.h"
#include "model/comm_out_queue.h"
#include "model/logger.h"
#include "model/ocpn_utils.h"

#include "observable.h"

using namespace std::literals::chrono_literals;

static bool bool_result0;
static std::string s_result;

static const char* const GPGGA = "$GPGGA 00";
static const char* const GPGGL = "$GPGGL 00";

class OverrunEvent : public wxAppConsole {
public:

  OverrunEvent() {
    bool result = false;
    ObsListener listener;
    listener.Init(CommDriverRegistry::GetInstance().evt_comm_overrun,
		  [&](ObservedEvt&) { result = true; });

    CommOutQueue queue(3);
    for (int i = 0; i < 20; i++) queue.push_back(GPGGL);
    EXPECT_EQ(queue.size(), 3);
    EXPECT_TRUE(HasPendingEvents());
    ProcessPendingEvents();
    EXPECT_TRUE(result);
  };
};


TEST(Buffer, Single) {
  CommOutQueueSingle queue;
  for (int i = 0; i < 10; i++) queue.push_back(GPGGA);
  EXPECT_EQ(queue.size(), 1);
  EXPECT_EQ(queue.pop(), GPGGA);
  EXPECT_EQ(queue.size(), 0);
  EXPECT_THROW({ queue.pop(); }, std::underflow_error);
  EXPECT_FALSE(queue.push_back("foo"));
}

TEST(Buffer, Size_3 ) {
  CommOutQueue queue(3);

  for (int i = 0; i < 20; i++) queue.push_back(GPGGL);
  EXPECT_EQ(queue.size(), 3);

  std::string line(GPGGA);
  for (int i = 0; i < 100; i++) {
    std::string line(GPGGA);
    ocpn::replace(line, "00", std::to_string(i));
    queue.push_back(line);
  }
  EXPECT_EQ(queue.size(), 6);

  EXPECT_EQ(queue.pop(), GPGGL);
  EXPECT_EQ(queue.pop(), GPGGL);
  EXPECT_EQ(queue.pop(), GPGGL);
  EXPECT_EQ(queue.pop(), "$GPGGA 97");
  EXPECT_EQ(queue.pop(), "$GPGGA 98");
  EXPECT_EQ(queue.pop(), "$GPGGA 99");
  EXPECT_EQ(queue.size(), 0);
  EXPECT_THROW({ queue.pop(); }, std::underflow_error);
}



TEST(Buffer, Hakefjord) {
  const auto path = fs::path(TESTDATA) / "Hakefjord.log";
  std::ifstream stream(path.string());
  MeasuredCommOutQueue queue(3);
  for (std::string line; std::getline(stream, line); ) {
    queue.push_back(line);
  }
  RecordProperty("buffer size", std::to_string(queue.size()));
  for (int i = 0; i < 3; i++) queue.push_back(GPGGA);
  std::string line;
  do { line = queue.pop(); } while (!ocpn::startswith(line, "$GPGGA"));
  EXPECT_EQ(line, GPGGA);
  do { line = queue.pop(); } while (!ocpn::startswith(line, "$GPGGA"));
  EXPECT_EQ(line, GPGGA);
  do { line = queue.pop(); } while (!ocpn::startswith(line, "$GPGGA"));
  EXPECT_EQ(line, GPGGA) ;
  RecordProperty("push_time", std::to_string(queue.push_time));
  // writes to test_detail.xml if invoked with --gtest_output.xml
}

TEST(Buffer, RateLimit1) {
  CommOutQueue queue(20, 1ms);
  for (int i = 0; i < 20; i++) {
    queue.push_back(GPGGL);
    std::this_thread::sleep_for(2ms);
  }
  EXPECT_EQ(queue.size(), 20);
}

#ifndef __APPLE__
// The MacOS builders seems to have a lot of "too" long sleeps.
// Disable for now.
TEST(Buffer, RateLimit2) {
  CommOutQueue queue(20, 5ms);
  for (int i = 0; i < 20; i++) {
    queue.push_back(GPGGL);
    std::this_thread::sleep_for(2ms);
  }
  EXPECT_EQ(queue.size(), 1);
  // might fail due to OS gitter i. e., sleep takes "too" long
}
#endif

TEST(Buffer, RateAndSizeLimit) {
  CommOutQueue queue(10, 1ms);
  for (int i = 0; i < 20; i++) {
    queue.push_back(GPGGL);
    std::this_thread::sleep_for(2ms);
  }
  EXPECT_EQ(queue.size(), 10);
  // might fail due to OS gitter i. e., sleep takes "too" long
}

TEST(Buffer, OverrunEvent) {
  OverrunEvent event;
}
