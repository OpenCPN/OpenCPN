#include "config.h"

#include <chrono>

#include <gtest/gtest.h>

#include "model/base_platform.h"
#include "model/comm_out_queue.h"
#include "model/logger.h"
#include "model/ocpn_utils.h"

using namespace std::literals::chrono_literals;

static bool bool_result0;
static std::string s_result;

static const char* const GPGGA = "$GPGGA 00";
static const char* const GPGGL = "$GPGGL 00";

TEST(Buffer, Single) {
  CommOutQueueSingle queue;
  for (int i = 0; i < 10; i++) queue.push_back(GPGGA);
  EXPECT_EQ(queue.size(), 1);
  EXPECT_EQ(queue.pop(), GPGGA);
  EXPECT_EQ(queue.size(), 0);
}

TEST(Buffer, Size_3 ) {
  CommOutQueue queue(3);
  for (int i = 0; i < 100; i++) queue.push_back(GPGGA);
  EXPECT_EQ(queue.size(), 3);
  for (int i = 0; i < 3; i++) queue.push_back(GPGGL);
  EXPECT_EQ(queue.size(), 6);
  EXPECT_EQ(queue.pop(), GPGGA);
  EXPECT_EQ(queue.pop(), GPGGA);
  EXPECT_EQ(queue.pop(), GPGGA);
  EXPECT_EQ(queue.pop(), GPGGL);
  EXPECT_EQ(queue.pop(), GPGGL);
  EXPECT_EQ(queue.pop(), GPGGL);
  EXPECT_EQ(queue.size(), 0);
}
