#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <iostream>


class PerfCounter {
  public:
    PerfCounter() : msgs_in(0), msgs_out(0), bytes_in(0), bytes_out(0), bps_in(0), mps_in(0), bps_out(0), mps_out(0), in_out_delay_us(0), overflow_msgs(0), in_queue(0) {}
    void in(const size_t bytes, bool ok) {
      auto t1 = std::chrono::steady_clock::now();
      std::chrono::duration<double, std::micro> us_time = t1 - last_in;
      bps_in = 0.95 * bps_in + 0.05 * bytes * 1000000 / us_time.count();
      mps_in = 0.95 * bps_in + 0.05 * 1000000 / us_time.count();
      msgs_in++;
      bytes_in += bytes;
      last_in = t1;
      if(!ok) {
        overflow_msgs++;
      }
      in_queue++;
    }

    void out(const size_t bytes, std::chrono::time_point<std::chrono::steady_clock> in_ts) {
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

std::ostream& operator <<(std::ostream& os, const PerfCounter& pc) 
{
  os << "{";
  os << "msgs_in: " << pc.msgs_in << ", ";
  os << "msgs_out: " << pc.msgs_out << ", ";
  os << "bytes_in: " << pc.bytes_in << ", ";
  os << "bytes_out: " << pc.bytes_out << ", ";
  os << "bps_in: " << pc.bps_in << ", ";
  os << "mps_in: " << pc.mps_in << ", ";
  os << "bps_out: " << pc.bps_out << ", ";
  os << "mps_out: " << pc.mps_out << ", ";
  os << "in_out_delay_us: " << pc.in_out_delay_us << ", ";
  os << "overflow_msgs: " << pc.overflow_msgs << ", ";
  os << "in_queue: " << pc.in_queue;
  os << "}";
  return os;
};

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
  int size() const;

  /**
   * Create a buffer which stores at most message_count items of each
   * message.
   */
  CommOutQueue(int message_count);

  CommOutQueue() : CommOutQueue(1) {}

  // Disable copying and assignment
  CommOutQueue(const CommOutQueue& other) = delete;
  CommOutQueue& operator=(const CommOutQueue&) = delete;

protected:
  struct BufferItem {
    uint64_t type;
    std::string line;
    BufferItem(const std::string& line);
    BufferItem(const BufferItem& other);
    std::chrono::time_point<std::chrono::steady_clock> ts;
  };

  std::vector<BufferItem> m_buffer;
  mutable std::mutex m_mutex;
  int m_size;
};

/** A  CommOutQueue limited to one message of each kind. */
class CommOutQueueSingle : public CommOutQueue {
public:
  CommOutQueueSingle() : CommOutQueue(1) {}

  /** Insert line of NMEA0183 data in buffer. */
  bool push_back(const std::string& line) override;
};

/** Add unit test measurements to CommOutQueue. */
#include <unordered_map>

class MeasuredCommOutQueue : public CommOutQueue {
public:
  MeasuredCommOutQueue(int size) : CommOutQueue(size), push_time(0), pop_time(0) {}

  bool push_back(const std::string& line) override;
  std::string pop() override;

  std::unordered_map<unsigned long, PerfCounter> msg_perf;
  PerfCounter perf;
  double push_time;
  double pop_time;
};
std::ostream& operator <<(std::ostream& os, const MeasuredCommOutQueue& q) 
{
  os << "{";
  os << "push_time: " << q.push_time << ", ";
  os << "pop_time: " << q.pop_time << ", ";
  os << "perf: " << q.perf << ", ";
  os << "msg_perf: [";
  for(const auto& kv : q.msg_perf) {
    os << kv.first << ": " << kv.second << ", ";
  }
  os << "]";
  os << "}";
  return os;
};