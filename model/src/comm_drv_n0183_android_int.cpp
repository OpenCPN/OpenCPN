/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_n0183_android_int.h -- Nmea 0183 driver.
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2023 by David Register, Alec Leamas                     *
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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <mutex>
#include <queue>
#include <vector>

#include <wx/event.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "config.h"
#include "model/comm_drv_n0183_android_int.h"
#include "model/comm_navmsg_bus.h"
#include "model/comm_drv_registry.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

typedef enum DS_ENUM_BUFFER_STATE {
  DS_RX_BUFFER_EMPTY,
  DS_RX_BUFFER_FULL
} _DS_ENUM_BUFFER_STATE;

class CommDriverN0183AndroidInt;  // fwd

#define MAX_OUT_QUEUE_MESSAGE_LENGTH 100

template <typename T>
class n0183_atomic_queue {
public:
  size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queque.size();
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queque.empty();
  }

  const T& front() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queque.front();
  }

  void push(const T& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queque.push(value);
  }

  void pop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queque.pop();
  }

private:
  std::queue<T> m_queque;
  mutable std::mutex m_mutex;
};

#define OUT_QUEUE_LENGTH 20
#define MAX_OUT_QUEUE_MESSAGE_LENGTH 100

wxDEFINE_EVENT(wxEVT_COMMDRIVER_N0183_ANDROID_INT,
               CommDriverN0183AndroidIntEvent);

CommDriverN0183AndroidIntEvent::CommDriverN0183AndroidIntEvent(
    wxEventType commandType, int id = 0)
    : wxEvent(id, commandType) {};

CommDriverN0183AndroidIntEvent::~CommDriverN0183AndroidIntEvent() {};

void CommDriverN0183AndroidIntEvent::SetPayload(
    std::shared_ptr<std::vector<unsigned char>> data) {
  m_payload = data;
}
std::shared_ptr<std::vector<unsigned char>>
CommDriverN0183AndroidIntEvent::GetPayload() {
  return m_payload;
}

// required for sending with wxPostEvent()
wxEvent* CommDriverN0183AndroidIntEvent::Clone() const {
  CommDriverN0183AndroidIntEvent* newevent =
      new CommDriverN0183AndroidIntEvent(*this);
  newevent->m_payload = this->m_payload;
  return newevent;
};

template <class T>
class circular_buffer {
public:
  explicit circular_buffer(size_t size)
      : buf_(std::unique_ptr<T[]>(new T[size])), max_size_(size) {}

  void reset();
  size_t capacity() const;
  size_t size() const;

  bool empty() const {
    // if head and tail are equal, we are empty
    return (!full_ && (head_ == tail_));
  }

  bool full() const {
    // If tail is ahead the head by 1, we are full
    return full_;
  }

  void put(T item) {
    std::lock_guard<std::mutex> lock(mutex_);
    buf_[head_] = item;
    if (full_) tail_ = (tail_ + 1) % max_size_;

    head_ = (head_ + 1) % max_size_;

    full_ = head_ == tail_;
  }

  T get() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (empty()) return T();

    // Read data and advance the tail (we now have a free space)
    auto val = buf_[tail_];
    full_ = false;
    tail_ = (tail_ + 1) % max_size_;

    return val;
  }

private:
  std::mutex mutex_;
  std::unique_ptr<T[]> buf_;
  size_t head_ = 0;
  size_t tail_ = 0;
  const size_t max_size_;
  bool full_ = 0;
};

CommDriverN0183AndroidInt::CommDriverN0183AndroidInt(
    const ConnectionParams* params, DriverListener& listener)
    : CommDriverN0183(NavAddr::Bus::N0183, params->GetStrippedDSPort()),
      m_bok(false),
      m_portstring(params->GetDSPort()),
      m_params(*params),
      m_listener(listener) {
  this->attributes["commPort"] = params->Port.ToStdString();
  this->attributes["userComment"] = params->UserComment.ToStdString();
  this->attributes["ioDirection"] = DsPortTypeToString(params->IOSelect);

  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_N0183_ANDROID_INT,
       &CommDriverN0183AndroidInt::handle_N0183_MSG, this);

  Open();
}

CommDriverN0183AndroidInt::~CommDriverN0183AndroidInt() { Close(); }

bool CommDriverN0183AndroidInt::Open() {
  androidStartGPS(this);
  return true;
}

void CommDriverN0183AndroidInt::Close() {
  wxLogMessage(
      wxString::Format(_T("Closing NMEA Driver %s"), m_portstring.c_str()));

  androidStopGPS();

  Unbind(wxEVT_COMMDRIVER_N0183_ANDROID_INT,
         &CommDriverN0183AndroidInt::handle_N0183_MSG, this);
}

bool CommDriverN0183AndroidInt::SendMessage(
    std::shared_ptr<const NavMsg> msg, std::shared_ptr<const NavAddr> addr) {
  return false;
}

void CommDriverN0183AndroidInt::handle_N0183_MSG(
    CommDriverN0183AndroidIntEvent& event) {
  auto p = event.GetPayload();
  std::vector<unsigned char>* payload = p.get();

  // Extract the NMEA0183 sentence
  std::string full_sentence = std::string(payload->begin(), payload->end());

  if ((full_sentence[0] == '$') || (full_sentence[0] == '!')) {  // Sanity check
    std::string identifier;
    // We notify based on full message, including the Talker ID
    identifier = full_sentence.substr(1, 5);

    // notify message listener and also "ALL" N0183 messages, to support plugin
    // API using original talker id
    auto msg = std::make_shared<const Nmea0183Msg>(identifier, full_sentence,
                                                   GetAddress());
    auto msg_all = std::make_shared<const Nmea0183Msg>(*msg, "ALL");

    if (m_params.SentencePassesFilter(full_sentence, FILTER_INPUT))
      m_listener.Notify(std::move(msg));

    m_listener.Notify(std::move(msg_all));
  }
}
