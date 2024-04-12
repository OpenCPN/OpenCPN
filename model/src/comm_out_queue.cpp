#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <sys/types.h>

#include "model/comm_drv_registry.h"
#include "model/comm_out_queue.h"
#include "model/logger.h"

// Both arm and intel are little endian, but better safe than sorry:
#if __BYTE_ORDER == __LITTLE_ENDIAN
static const uint64_t kFirstFiveBytes = 0x000000ffffffffff;
#else
static const uint64_t kFirstFiveBytes = 0xffffffffff000000;
#endif

#define PUBX 190459303248   //"PUBX,"
#define STALK 323401897043  //"STALK"

/**
 * Return bytes 1..5 in line as an uint64_t with exceptions for u-blox GNSS and
 * converted Seatalk.
 * Note: Some vendor extension messages also do not have 5
 * character talker ID + message ID, but 6 (PMGNST, PRWIZCH, PSMDST). This is
 * probably not critical as the last character seems to not be making any
 * difference in producing a unique ID for the messages commonly seen.
 */
static inline uint64_t GetNmeaType(const std::string& line) {
  size_t skipchars = 1;
  if (line[0] == 0x5c) {  // Starts with the tag block '\', we need to skip it
                          // and then also the start delimiter
    skipchars = line.find(',', 1);
    if (skipchars == std::string::npos) {
      skipchars = 1;  // This should never happen, there is no end of the tag
                      // block, but just in case...
    }
  }
  uint64_t result = *reinterpret_cast<const uint64_t*>(&line[skipchars]);
  uint64_t result5 = result & kFirstFiveBytes;
  if (result5 == PUBX || result5 == STALK) {
    /* PUBX from possibly high-speed u-blox GNSS receivers that are sure to
       overload slow connections has a 2 digit zero-padded numerical message ID
       in the first field Similar with STALK, the two digit Seatalk message ID
       is in the first field Both fit nicely into 8 bytes though... */
    return result;
  } else {
    return result5;
  }
}

static void ReportOverrun(const std::string& msg, bool overrun_reported) {
  auto& registry = CommDriverRegistry::GetInstance();
  std::string s;
  if (msg.length() < 6) s = msg; else s = msg.substr(0, 5);
  DEBUG_LOG << "CommOutQueue: Overrun on: " << msg;
  if (!overrun_reported) registry.evt_comm_overrun.Notify(msg);
}


CommOutQueue::BufferItem::BufferItem(const std::string& _line)
    : type(GetNmeaType(_line)),
      line(_line),
      ts(std::chrono::steady_clock::now()) {}

CommOutQueue::BufferItem::BufferItem(const BufferItem& other)
    : type(other.type),
      line(other.line),
      ts(std::chrono::steady_clock::now()) {}

CommOutQueue::CommOutQueue(int size)
    : m_size(size - 1),
      m_overrun_reported(false) {
  assert(size >= 1 && "Illegal buffer size");
}

bool CommOutQueue::push_back(const std::string& line) {
  if (line.size() < 7) return false;
  BufferItem item(line);
  auto match = [item](const BufferItem& it) { return it.type == item.type; };
  std::lock_guard<std::mutex> lock(m_mutex);
  int found = std::count_if(m_buffer.begin(), m_buffer.end(), match);
  if (found > m_size) {
    if (!m_overrun_reported) {
      ReportOverrun(line, m_overrun_reported);
      m_overrun_reported = true;
    }
    // overflow: too many of these kind of messages
    // are still not processed. Drop so we keep m_size of them.
    int matches = 0;
    auto match_cnt = [&](const BufferItem& it) {
      return it.type == item.type && matches++ >= m_size;
    };
    m_buffer.erase(std::remove_if(m_buffer.begin(), m_buffer.end(), match_cnt),
                   m_buffer.end());
  }
  m_buffer.insert(m_buffer.begin(), item);
  return true;
}

std::string CommOutQueue::pop() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_buffer.size() <= 0)
    throw std::underflow_error("Attempt to pop() from empty buffer");
  auto item = m_buffer.back();
  m_buffer.pop_back();
  return item.line;
}

int CommOutQueue::size() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_buffer.size();
}

bool CommOutQueueSingle::push_back(const std::string& line) {
  if (line.size() < 7) return false;
  BufferItem item(line);
  auto match = [&item](const BufferItem& it) { return it.type == item.type; };

  std::lock_guard<std::mutex> lock(m_mutex);
  auto found = std::find_if(m_buffer.begin(), m_buffer.end(), match);
  if (found != m_buffer.end()) {
    // overflow: this kind of message is still not processed. Drop it
    m_buffer.erase(std::remove_if(found, m_buffer.end(), match),
                   m_buffer.end());
  }
  m_buffer.push_back(item);
  return true;
}

bool MeasuredCommOutQueue::push_back(const std::string& line) {
  using std::chrono::duration;
  using std::chrono::steady_clock;

  auto t1 = steady_clock::now();
  bool ok = CommOutQueue::push_back(line);
  msg_perf[GetNmeaType(line)].in(line.size(), ok);
  perf.in(line.size(), ok);
  auto t2 = steady_clock::now();
  duration<double, std::micro> us_time = t2 - t1;

  push_time = 0.95 * push_time + 0.05 * us_time.count();  // LP filter.
  return ok;
}

std::string MeasuredCommOutQueue::pop() {
  using std::chrono::duration;
  using std::chrono::steady_clock;

  auto t1 = steady_clock::now();
  // auto msg = CommOutQueue::pop(); // We need to update the perf counters,
  // can't just pop() here
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_buffer.size() <= 0)
    throw std::underflow_error("Attempt to pop() from empty buffer");
  auto item = m_buffer.back();
  m_buffer.pop_back();
  perf.out(item.line.size(), item.ts);
  msg_perf[item.type].out(item.line.size(), item.ts);
  auto t2 = steady_clock::now();
  duration<double, std::micro> us_time = t2 - t1;
  us_time = t2 - t1;

  pop_time = 0.95 * pop_time + 0.05 * us_time.count();  // LP filter.
  return item.line;
}

std::ostream& operator<<(std::ostream& os, const MeasuredCommOutQueue& q) {
  os << "{";
  os << "push_time: " << q.push_time << ", ";
  os << "pop_time: " << q.pop_time << ", ";
  os << "perf: " << q.perf << ", ";
  os << "msg_perf: [";
  for (const auto& kv : q.msg_perf) {
    os << kv.first << ": " << kv.second << ", ";
  }
  os << "]";
  os << "}";
  return os;
};

std::ostream& operator<<(std::ostream& os, const PerfCounter& pc) {
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
