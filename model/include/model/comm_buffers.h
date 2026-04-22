/***************************************************************************
 *   Copyright (C) 2024 Alec Leamas                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Various input/output buffers.
 */

#ifndef COMM_BUFFERS_H_
#define COMM_BUFFERS_H_

#include <deque>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
#else
#include <cstdint>
#endif

class BufferError : public std::logic_error {
public:
  BufferError(const std::string& why) : logic_error(why) {};
};

/** Synchronized buffer for outbound, line oriented data. */
class OutputBuffer {
public:
  /** Insert line in buffer. */
  void Put(const std::string line);

  /**
   * Retrieve a line in buffer.
   * @return true if data was available and fetched into line, else false.
   */
  bool Get(std::string& line);

private:
  std::deque<std::string> m_buffer;
  std::mutex m_mutex;
};

/** Assembles input characters to lines. */
class LineBuffer {
public:
  /** Add a single character. */
  void Put(uint8_t ch);

  /** Return true if a line is available to be returned by GetLine(). */
  bool HasLine() const;

  /** Retrieve a line from buffer, return empty line if none available. */
  std::vector<uint8_t> GetLine();

  LineBuffer() : m_line_count(0), m_last_ch('\n') {}

private:
  std::vector<uint8_t> m_buff;
  int m_line_count;
  uint8_t m_last_ch;
};

/** Assemble characters to NMEA0183 sentences. */
class N0183Buffer {
public:
  /** Add a single character, possibly making a sentence available */
  void Put(uint8_t ch);

  /** Return true if a sentence is available to be returned by GetSentence() */
  bool HasSentence() const { return !m_lines.empty(); }

  /**
   * Retrieve a sentence from buffer
   * @return Next available sentence in buffer or "" if none available
   */
  std::string GetSentence();

  N0183Buffer() : m_state(State::PrefixWait) {}

private:
  std::deque<std::string> m_lines;
  std::vector<uint8_t> m_line;
  enum class State { PrefixWait, Data, CsDigit1, CsDigit2 } m_state;
};

/** Fixed size, synchronized FIFO buffer. Some methods throws BufferError. */
template <class T>
class CircularBuffer {
public:
  explicit CircularBuffer(size_t size)
      : m_buf(std::unique_ptr<T[]>(new T[size])),
        m_max_size(size),
        m_head(0),
        m_tail(0),
        m_full(false) {}

  /** Reset internal state, ditch possible contained data. */
  void Reset() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_head = 0;
    m_tail = 0;
    m_full = false;
  }

  /** Return buffer max size */
  size_t Capacity() const noexcept { return m_max_size; }

  /** Return actual size */
  size_t Size() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t size = m_max_size;
    if (!m_full)
      size = m_head >= m_tail ? m_head - m_tail : m_head + m_max_size - m_tail;
    return size;
  }

  /** Return true if buffer is empty */
  bool IsEmpty() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return (!m_full && (m_head == m_tail));
  }

  /** Return true if buffer is full. */
  bool IsFull() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_full;
  }

  /**
   * Add item to buffer without throwing exceptions.
   * @return true if success i.e., buffer is not full.
   */
  bool SafePut(const T& item) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_full) return false;
    DoPut(item);
    return true;
  }

  /** Add item to buffer; throw BufferError if full. */
  void Put(const T& item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_full) throw BufferError("Put(): full buffer");
    DoPut(item);
  }

  /** Get item from buff; throw BufferError if empty. */
  T Get() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_full && m_head == m_tail) throw BufferError("Get(): empty buffer");
    return DoGet();
  }

  /**
   * Retrieve item from buffer without throwing exceptions.
   * @return true if success i.e., buffer is not empty.
   */
  bool Get(T& item) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_full && m_head == m_tail) return false;
    item = DoGet();
    return true;
  }

  const T& Peek() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_full && m_head == m_tail) throw BufferError("Peek(): empty buffer");
    return m_buf[m_tail];
  }

  bool Peek(T& item) const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_full && m_head == m_tail) return false;
    item = m_buf[m_tail];
    return true;
  }

private:
  mutable std::mutex m_mutex;
  std::unique_ptr<T[]> m_buf;
  const size_t m_max_size;
  size_t m_head;
  size_t m_tail;
  bool m_full;

  void DoPut(const T& item) {
    m_buf[m_head] = item;
    if (m_full) m_tail = (m_tail + 1) % m_max_size;
    m_head = (m_head + 1) % m_max_size;
    m_full = m_head == m_tail;
  }

  // Read data and advance the tail (we now have a free space)
  T DoGet() {
    auto val = m_buf[m_tail];
    m_full = false;
    m_tail = (m_tail + 1) % m_max_size;
    return val;
  }
};

#endif  // COMM_BUFFERS_H_
