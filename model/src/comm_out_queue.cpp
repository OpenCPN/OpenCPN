#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "model/comm_out_queue.h"

static inline unsigned long ShiftChar(unsigned char ch, int shift) {
  return static_cast<unsigned long>(ch) << shift;
}

static unsigned long GetNmeaType(const std::string& line) {
  auto id = line.substr(1, 5);
  unsigned long result = 0;
  for (int i = 5; i >= 0; i--) result += ShiftChar(id[i], i * 8);
  return result;
}

CommOutQueue::BufferItem::BufferItem(const std::string& _line)
    : type(GetNmeaType(_line)), line(_line) {}

CommOutQueue::BufferItem::BufferItem(const BufferItem& other)
    : type(other.type), line(other.line) {}

CommOutQueue::CommOutQueue(int size) : m_size(size - 1) {
  assert(size >= 1 && "Illegal buffer size");
}

bool CommOutQueue::push_back(const std::string& line) {
  if (line.size() < 7 ) return false;
  BufferItem item(line);
  auto match = [item](const BufferItem& it) { return it.type == item.type; };
  std::lock_guard<std::mutex> lock(m_mutex);
  int found = std::count_if(m_buffer.begin(), m_buffer.end(), match);
  if (found > m_size) {
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
  if (line.size() < 7 ) return false;
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
  using std::chrono::high_resolution_clock;

  auto t1 = high_resolution_clock::now();
  bool ok = CommOutQueue::push_back(line);
  auto t2 = high_resolution_clock::now();
  duration<double, std::micro> us_time = t2 - t1;

  push_time = 0.95 * push_time + 0.05 * us_time.count();  // LP filter.
  return ok;
}
