#ifndef COMM__OUT_QUEUE_H__
#define COMM__OUT_QUEUE_H__

#include <chrono>
#include <cstdint>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std::literals::chrono_literals;

class PerfCounter {
public:
  PerfCounter()
      : msgs_in(0),
        msgs_out(0),
        bytes_in(0),
        bytes_out(0),
        bps_in(0),
        mps_in(0),
        bps_out(0),
        mps_out(0),
        in_out_delay_us(0),
        overflow_msgs(0),
        in_queue(0) {}

  void in(const size_t bytes, bool ok) {
    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::micro> us_time = t1 - last_in;
    bps_in = 0.95 * bps_in + 0.05 * bytes * 1000000 / us_time.count();
    mps_in = 0.95 * bps_in + 0.05 * 1000000 / us_time.count();
    msgs_in++;
    bytes_in += bytes;
    last_in = t1;
    if (!ok) {
      overflow_msgs++;
    }
    in_queue++;
  }

  void out(const size_t bytes,
           std::chrono::time_point<std::chrono::steady_clock> in_ts) {
    auto t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::micro> us_time = t1 - last_in;
    bps_out = 0.95 * bps_out + 0.05 * bytes * 1000000 / us_time.count();
    mps_out = 0.95 * bps_out + 0.05 * 1000000 / us_time.count();
    us_time = t1 - in_ts;
    in_out_delay_us = 0.95 * in_out_delay_us + 0.05 * us_time.count();
    msgs_out++;
    bytes_out += bytes;
    last_out = t1;
    in_queue--;
  }

  size_t msgs_in;
  size_t msgs_out;
  size_t bytes_in;
  size_t bytes_out;
  uint32_t bps_in;
  double mps_in;
  uint32_t bps_out;
  double mps_out;
  size_t in_out_delay_us;
  size_t overflow_msgs;
  size_t in_queue;
  std::chrono::time_point<std::chrono::steady_clock> last_in;
  std::chrono::time_point<std::chrono::steady_clock> last_out;
};

std::ostream& operator<<(std::ostream& os, const PerfCounter& pc);

/**
 *  Queue of NMEA0183 messages which only holds a limited amount
 *  of each message type.
 */
class CommOutQueue {
public:
  /**
   * Insert valid line of NMEA0183 data in buffer.
   * @return false on errors including invalid input, else true.
   */
  virtual bool push_back(const std::string& line);

  /**
   * Return  next line to send and remove it from buffer,
   * throws exception if empty.
   */
  virtual std::string pop();

  /** Return number of lines in queue. */
  virtual int size() const;

  /**
   * Create a buffer which stores at most max_buffered items of each
   * message, applying rate limits if messages are entered "too" fast
   *
   * @param max_buffered Max number of messages of a type kept in buffer
   *                     without discarding data as overrun.
   * @param min_msg_gap  minimum time between two messages of the same
   *                     type without applying rate limits.
   */
  CommOutQueue(unsigned max_buffered,
               std::chrono::duration<unsigned, std::milli> min_msg_gap);
  /**
   * Create a buffer which stores at most max_buffered items of each
   * message
   */
  CommOutQueue(unsigned max_buffered) : CommOutQueue(max_buffered, 0ms) {}

  /**
   * Default buffer, allows 10 buffered messages of each type, applies
   * rate limits when repeated with less than 600 ms intervals
   */
  CommOutQueue() : CommOutQueue(12) {}

  // Disable copying and assignment
  CommOutQueue(const CommOutQueue& other) = delete;
  CommOutQueue& operator=(const CommOutQueue&) = delete;

  virtual ~CommOutQueue() = default;

protected:
  struct BufferItem {
    uint64_t type;
    std::string line;
    BufferItem(const std::string& line);
    BufferItem(const BufferItem& other);
    std::chrono::time_point<std::chrono::steady_clock> stamp;
  };

  std::vector<BufferItem> m_buffer;
  mutable std::mutex m_mutex;
  int m_size;
  using duration_ms = std::chrono::duration<unsigned, std::milli>;
  duration_ms m_min_msg_gap;
  bool m_overrun_reported;
  std::set<uint64_t> m_rate_limits_logged;
  ;
};

/** A  CommOutQueue limited to one message of each kind. */
class CommOutQueueSingle : public CommOutQueue {
public:
  CommOutQueueSingle() : CommOutQueue(1, 0ms) {}

  /** Insert line of NMEA0183 data in buffer. */
  bool push_back(const std::string& line) override;
};

/** Add unit test measurements to CommOutQueue. */

class MeasuredCommOutQueue : public CommOutQueue {
public:
  MeasuredCommOutQueue(unsigned max_buffered,
                       std::chrono::duration<unsigned, std::milli> min_msg_gap)
      : CommOutQueue(max_buffered, min_msg_gap), push_time(0), pop_time(0) {}

  MeasuredCommOutQueue(unsigned max_buffered)
      : MeasuredCommOutQueue(max_buffered, 0ms) {}

  bool push_back(const std::string& line) override;

  std::string pop() override;

  std::unordered_map<unsigned long, PerfCounter> msg_perf;

  PerfCounter perf;
  double push_time;
  double pop_time;
};

/** Simple FIFO queue without added logic. */
class DummyCommOutQueue : public CommOutQueue {
public:
  DummyCommOutQueue() {};

  bool push_back(const std::string& line) override {
    std::lock_guard<std::mutex> lock(m_mutex);
    buff.insert(buff.begin(), line);
    return true;
  }

  std::string pop() override {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (buff.size() <= 0)
      throw std::underflow_error("Attempt to pop() from empty buffer");
    auto line = buff.back();
    buff.pop_back();
    return line;
  }

  int size() const override {
    std::lock_guard<std::mutex> lock(m_mutex);
    return buff.size();
  }

private:
  mutable std::mutex m_mutex;
  std::vector<std::string> buff;
};

std::ostream& operator<<(std::ostream& os, const MeasuredCommOutQueue& q);

#endif  //  COMM__OUT_QUEUE_H__
