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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

/** \file comm_buffers.h Line-oriented input/output buffers. */

#ifndef _COMM_BUFFERS_H__
#define _COMM_BUFFERS_H__

#include <deque>
#include <mutex>
#include <string>
#include <vector>

#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
#else
#include <stdint.h>
#endif

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

/** Assembles input characters to  lines. */
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

#endif  // _COMM_BUFFERS_H__
