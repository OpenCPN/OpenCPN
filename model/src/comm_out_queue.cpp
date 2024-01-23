#include  <algorithm>
#include  <cassert>
#include  <vector>

#include "model/comm_out_queue.h"

static long GetNmeaType(const std::string& line) {
  auto id = line.substr(1, 6);
  for (int i = 0; i < 6; i += 1) id[i] &= 127;
  return id[0] * id[1] * id[2] * id[3] * id[4] * id[5];
}

CommOutQueue::CommOutQueue(int size) : m_size(size - 1) {
  assert(size >=1 && "Illegal buffer size");
}

void CommOutQueue::push_back(const std::string& line) {
  BufferItem item(line);
  auto match = [item] (const BufferItem& it) { return it.type == item.type; };
  std::lock_guard<std::mutex> lock(m_mutex);
  int found = std::count_if(m_buffer.begin(), m_buffer.end(), match);
  if (found > m_size) {
    // overflow: too many of these kind of messages
    // are still not processed. Drop so we keep m_size of them.
    int matches = 0;
    auto match_cnt = [&] (const BufferItem& it) {
        return it.type == item.type && matches++ >= m_size; };
    m_buffer.erase(std::remove_if(m_buffer.begin(), m_buffer.end(), match_cnt),
                   m_buffer.end());
  }
  m_buffer.push_back(item);
  return;
}

CommOutQueue::BufferItem::BufferItem(const std::string& _line)
    : type(GetNmeaType(_line)), line(_line) {}

CommOutQueue::BufferItem::BufferItem(const BufferItem& other)
    : type(other.type), line(other.line) {}

std::string CommOutQueue::pop() {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto item = m_buffer[0];
  m_buffer.erase(m_buffer.begin());
  return item.line;
}

int CommOutQueue::size() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_buffer.size();
}

void CommOutQueueSingle::push_back(const std::string& line) {
  std::lock_guard<std::mutex> lock(m_mutex);
  BufferItem item(line);
  auto match = [&item](const BufferItem& it) { return it.type == item.type; };
  auto found = std::find_if(m_buffer.begin(), m_buffer.end(), match);
  if (found != m_buffer.end()) {
    // overflow: this kind of message is still not processed. Drop it
    m_buffer.erase(std::remove_if(found, m_buffer.end(), match),
                   m_buffer.end());
  }
  m_buffer.push_back(item);
  return;
}
