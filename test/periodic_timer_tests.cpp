// SPDX-License-Identifier: GPL-2.0-or-later
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include <gtest/gtest.h>

#include "model/periodic_timer.h"

using namespace std::chrono_literals;

namespace {

// This state outlives the timer, including when the baseline code incorrectly
// permits owner teardown while a callback is still running. The tests detect
// that ordering without dereferencing freed memory.
struct CallbackState {
  std::mutex mutex;
  std::condition_variable changed;
  bool entered = false;
  bool release = false;
  bool finished = false;
  bool stop_started = false;
  bool stop_returned = false;
  bool finished_when_stop_returned = false;
  bool owner_teardown = false;
  bool callback_observed_teardown = false;

  template <typename Predicate>
  bool Wait(Predicate predicate, std::chrono::milliseconds timeout = 2s) {
    std::unique_lock lock(mutex);
    return changed.wait_for(lock, timeout, predicate);
  }

  void Release() {
    std::lock_guard lock(mutex);
    release = true;
    changed.notify_all();
  }
};

class BlockingTimer : public PeriodicTimer {
public:
  explicit BlockingTimer(const std::shared_ptr<CallbackState>& state)
      : PeriodicTimer(50ms), m_state(state) {}

  ~BlockingTimer() override { Stop(); }

private:
  void Notify() override {
    // Do not access the timer object after announcing callback entry: the
    // baseline destructor may already have begun before Release() is called.
    const auto state = m_state;
    std::unique_lock lock(state->mutex);
    state->entered = true;
    state->changed.notify_all();
    state->changed.wait(lock, [&] { return state->release; });
    state->callback_observed_teardown = state->owner_teardown;
    state->finished = true;
    state->changed.notify_all();
  }

  std::shared_ptr<CallbackState> m_state;
};

class DriverLikeOwner {
public:
  explicit DriverLikeOwner(const std::shared_ptr<CallbackState>& state)
      : m_state(state), m_timer(state) {
    m_timer.Start();
  }

  ~DriverLikeOwner() {
    {
      std::lock_guard lock(m_state->mutex);
      m_state->stop_started = true;
      m_state->changed.notify_all();
    }
    m_timer.Stop();
    // Model a driver's destructor/Close() releasing resources which Notify()
    // needs. The shared test state itself is kept alive to avoid actual UAF.
    std::lock_guard lock(m_state->mutex);
    m_state->owner_teardown = true;
    m_state->changed.notify_all();
  }

private:
  std::shared_ptr<CallbackState> m_state;
  BlockingTimer m_timer;
};

class CountingTimer : public PeriodicTimer {
public:
  explicit CountingTimer(std::chrono::milliseconds interval = 10ms)
      : PeriodicTimer(interval) {}

  ~CountingTimer() override { Stop(); }

  bool WaitForCallbacks(int count) {
    std::unique_lock lock(m_mutex);
    return m_changed.wait_for(lock, 2s, [&] { return m_count >= count; });
  }

  size_t WorkerCount() {
    std::lock_guard lock(m_mutex);
    return m_workers.size();
  }

private:
  void Notify() override {
    std::lock_guard lock(m_mutex);
    ++m_count;
    m_workers.insert(std::this_thread::get_id());
    m_changed.notify_all();
  }

  std::mutex m_mutex;
  std::condition_variable m_changed;
  int m_count = 0;
  std::set<std::thread::id> m_workers;
};

}  // namespace

TEST(PeriodicTimer, ConstructionDoesNotStartWorker) {
  CountingTimer timer;
  std::this_thread::sleep_for(50ms);
  EXPECT_EQ(timer.WorkerCount(), 0u);
  timer.Start();
  EXPECT_TRUE(timer.WaitForCallbacks(1));
}

TEST(PeriodicTimer, RepeatedStartUsesOneWorker) {
  CountingTimer timer;
  timer.Start();
  ASSERT_TRUE(timer.WaitForCallbacks(1));
  timer.Start();
  EXPECT_TRUE(timer.WaitForCallbacks(3));
  timer.Stop();
  EXPECT_EQ(timer.WorkerCount(), 1u);
}

TEST(PeriodicTimer, StopBeforeStartAndRepeatedStopReturnPromptly) {
  CountingTimer timer(1h);
  auto stopper = std::async(std::launch::async, [&] {
    timer.Stop();
    timer.Stop();
    timer.Start();
    timer.Stop();
    timer.Stop();
  });
  EXPECT_EQ(stopper.wait_for(1s), std::future_status::ready);
  stopper.get();
  EXPECT_EQ(timer.WorkerCount(), 0u);
}

TEST(PeriodicTimer, CanRestartAfterStop) {
  CountingTimer timer;
  timer.Start();
  ASSERT_TRUE(timer.WaitForCallbacks(1));
  timer.Stop();
  timer.Start();
  EXPECT_TRUE(timer.WaitForCallbacks(2));
}

TEST(PeriodicTimer, RapidStartAndStopDoesNotLoseWakeup) {
  CountingTimer timer(1h);
  for (int i = 0; i < 200; ++i) {
    timer.Start();
    timer.Stop();
  }
  EXPECT_EQ(timer.WorkerCount(), 0u);
}

TEST(PeriodicTimer, StopWaitsForRunningCallback) {
  const auto state = std::make_shared<CallbackState>();
  BlockingTimer timer(state);
  timer.Start();
  const bool entered = state->Wait([&] { return state->entered; });
  if (!entered) state->Release();
  ASSERT_TRUE(entered);

  std::thread stopper([&] {
    {
      std::lock_guard lock(state->mutex);
      state->stop_started = true;
      state->changed.notify_all();
    }
    timer.Stop();
    std::lock_guard lock(state->mutex);
    state->finished_when_stop_returned = state->finished;
    state->stop_returned = true;
    state->changed.notify_all();
  });
  EXPECT_TRUE(state->Wait([&] { return state->stop_started; }));
  const bool returned_early =
      state->Wait([&] { return state->stop_returned; }, 300ms);
  state->Release();
  stopper.join();

  EXPECT_FALSE(returned_early) << "Stop returned while Notify was blocked";
  EXPECT_TRUE(state->finished_when_stop_returned);
}

TEST(PeriodicTimer, OwnerTeardownWaitsForRunningCallback) {
  const auto state = std::make_shared<CallbackState>();
  auto owner = std::make_unique<DriverLikeOwner>(state);
  const bool entered = state->Wait([&] { return state->entered; });
  if (!entered) state->Release();
  ASSERT_TRUE(entered);

  std::thread destroyer([&] { owner.reset(); });
  EXPECT_TRUE(state->Wait([&] { return state->stop_started; }));
  const bool teardown_early =
      state->Wait([&] { return state->owner_teardown; }, 300ms);
  state->Release();
  destroyer.join();

  EXPECT_FALSE(teardown_early) << "Owner teardown began before Notify finished";
  EXPECT_FALSE(state->callback_observed_teardown);
}

TEST(PeriodicTimer, ConcurrentStopsWaitForRunningCallback) {
  const auto state = std::make_shared<CallbackState>();
  BlockingTimer timer(state);
  timer.Start();
  const bool entered = state->Wait([&] { return state->entered; });
  if (!entered) state->Release();
  ASSERT_TRUE(entered);

  auto first = std::async(std::launch::async, [&] { timer.Stop(); });
  auto second = std::async(std::launch::async, [&] { timer.Stop(); });
  const auto first_status = first.wait_for(100ms);
  const auto second_status = second.wait_for(100ms);
  state->Release();
  first.get();
  second.get();
  EXPECT_EQ(first_status, std::future_status::timeout);
  EXPECT_EQ(second_status, std::future_status::timeout);
  EXPECT_TRUE(state->finished);
}

TEST(PeriodicTimer, DerivedDestructorWaitsForRunningCallback) {
  const auto state = std::make_shared<CallbackState>();
  auto timer = std::make_unique<BlockingTimer>(state);
  timer->Start();
  const bool entered = state->Wait([&] { return state->entered; });
  if (!entered) state->Release();
  ASSERT_TRUE(entered);

  auto destroyer = std::async(std::launch::async, [&] { timer.reset(); });
  const auto status = destroyer.wait_for(300ms);
  state->Release();
  destroyer.get();
  EXPECT_EQ(status, std::future_status::timeout);
  EXPECT_TRUE(state->finished);
}
