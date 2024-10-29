/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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

/** \file comm_buffers.cpp  Implement comm_buffers.h */

#include "model/comm_buffers.h"

#include <algorithm>

void OutputBuffer::Put(const std::string line) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_buffer.push_back(line);
}

bool OutputBuffer::Get(std::string& line) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_buffer.size() == 0) {
    return false;
  }
  line = m_buffer[0];
  m_buffer.pop_front();
  return true;
}

void LineBuffer::Put(uint8_t ch) {
  if (m_last_ch == '\n' && ch == '\r') {
    // Replace possible <lf><cr> with <lf>
    m_last_ch = 0;
  } else if (m_last_ch == '\r' && ch == '\n') {
    // Replace possible <cr><lf> with <lf>
    if (m_buff.size() > 0) m_buff[m_buff.size() - 1] = '\n';
    m_last_ch = 0;
  } else {
    m_buff.push_back(ch);
    m_last_ch = ch;
  }
  if (ch == '\n') m_line_count += 1;
}

bool LineBuffer::HasLine() const { return m_line_count > 0; }

std::vector<uint8_t> LineBuffer::GetLine() {
  using namespace std;
  auto nl_pos = find(m_buff.begin(), m_buff.end(), '\n');
  if (nl_pos == m_buff.end()) {
    return vector<uint8_t>();
  }
  auto line = vector<uint8_t>(m_buff.begin(), nl_pos);
  m_buff = vector<uint8_t>(nl_pos + 1, m_buff.end());

  m_line_count -= 1;
  return line;
}

std::string N0183Buffer::GetSentence() {
  if (m_lines.empty()) return "";
  auto sentence = m_lines.front();
  m_lines.pop_front();
  return sentence;
}

void N0183Buffer::Put(uint8_t ch) {
  switch (m_state) {
    case State::PrefixWait:
      // Wait until start of message
      if (ch == '$' || ch == '!') {
        m_line.clear();
        m_line.push_back(ch);
        m_state = State::Data;
      }
      break;
    case State::Data:
      // Collect data into m_line until a '*' is found
      if (std::isprint(ch)) {
        m_line.push_back(ch);
        if (ch == '*') m_state = State::CsDigit1;
      } else {
        // Malformed? Garbage input?
        m_state = State::PrefixWait;
      }
      break;
    case State::CsDigit1:
      // Collect first checksum digit
      if (std::isxdigit(ch)) {
        m_line.push_back(ch);
        m_state = State::CsDigit2;
      } else {
        m_state = State::PrefixWait;
      }
      break;
    case State::CsDigit2:
      // Collect last checksum digit, push m_line to m_lines if OK
      if (std::isxdigit(ch)) {
        m_line.push_back(ch);
        m_lines.emplace_back(m_line.begin(), m_line.end());
      }
      m_state = State::PrefixWait;
      break;
  }
}
