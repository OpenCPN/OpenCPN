/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Low-level utility functions for socketcan support.
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2024 by David Register, Alec Leamas                     *
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

#ifndef _COMMCANUTIL_H
#define _COMMCANUTIL_H

#include <memory>
#include <string>

#include <wx/datetime.h>

#if !defined(__WXMSW__) && !defined(__WXMAC__)
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

#ifdef __WXMSW__
#define CAN_MAX_DLEN 8

struct can_frame {
  uint32_t can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
  uint8_t can_dlc; /* frame payload length in byte (0 .. 8) */
  uint8_t data[CAN_MAX_DLEN];
};
#endif

#if defined(__WXMAC__)
#define CAN_MAX_DLEN 8

struct can_frame {
  uint32_t can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
  uint8_t can_dlc; /* frame payload length in byte (0 .. 8) */
  uint8_t data[CAN_MAX_DLEN];
};
#endif

unsigned long BuildCanID(int priority, int source, int destination, int pgn);
bool IsFastMessagePGN(unsigned pgn);

/// CAN v2.0 29 bit header as used by NMEA 2000
class CanHeader {
public:
  CanHeader();
  /** Construct a CanHeader by parsing a frame */
  CanHeader(can_frame frame);

  /** Return true if header reflects a multipart fast message. */
  bool IsFastMessage() const;

  unsigned char priority;
  unsigned char source;
  unsigned char destination;
  int pgn;
};

/** Track fast message fragments eventually forming complete messages. */
class FastMessageMap {
public:
  class Entry {
  public:
    Entry()
        : time_arrived(wxDateTime::Now()),
          sid(0),
          expected_length(0),
          cursor(0) {}

    wxDateTime time_arrived;  ///< time of last fragment.

    /// Can header, used to "map" the incoming fast message fragments
    CanHeader header;

    /// Sequence identifier, used to check if a received message is the
    /// next message in the sequence
    unsigned int sid;

    unsigned int expected_length;  ///< total data length from first frame
    unsigned int cursor;  ///< cursor into the current position in data.
    std::vector<unsigned char> data;  ///< Received data
  };

  FastMessageMap() : dropped_frames(0), last_gc_run(wxDateTime::Now()) {}

  Entry operator[](int i) const { return entries[i]; }  /// Getter
  Entry& operator[](int i) { return entries[i]; }       /// Setter

  /** Return index to entry matching header and sid or -1 if not found. */
  int FindMatchingEntry(const CanHeader header, const unsigned char sid);

  /** Allocate a new, fresh entry and return index to it. */
  int AddNewEntry(void);

  /** Insert a new entry, first part of a multipart message. */
  bool InsertEntry(const CanHeader header, const unsigned char* data,
                   int index);

  /** Append fragment to existing multipart message. */
  bool AppendEntry(const CanHeader hdr, const unsigned char* data, int index);

  /** Remove entry at pos. */
  void Remove(int pos);

  std::vector<Entry> entries;

private:
  bool IsEntryExpired(unsigned int i);
  int GarbageCollector(void);
  void CheckGc();

  int dropped_frames;
  wxDateTime last_gc_run;
  wxDateTime dropped_frame_time;
};

#endif  // guard
