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

#include <algorithm>
#include <vector>

#include "model/comm_can_util.h"

static const int kNotFound = -1;

/// Number of fast messsages stored triggering Garbage Collection.
static const int kGcThreshold = 100;

/// Max time between garbage collection runs.
static const int kGcIntervalSecs = 10;

/// Max entry age before garbage collected
static const int kEntryMaxAgeSecs = 100;

typedef struct can_frame CanFrame;


unsigned long BuildCanID(int priority, int source, int destination, int pgn) {
  // build CanID
  unsigned long cid = 0;
  unsigned char pf = (unsigned char) (pgn >> 8);
  if (pf < 240){
    cid = ((unsigned long)(priority & 0x7))<<26 | pgn<<8 | ((unsigned long)destination)<<8 | (unsigned long)source;
  }
  else {
    cid = ((unsigned long)(priority & 0x7))<<26 | pgn<<8 | (unsigned long)source;
  }
  return cid;
}


// CanHeader implementation

CanHeader::CanHeader(const CanFrame frame) {
  unsigned char buf[4];
  buf[0] = frame.can_id & 0xFF;
  buf[1] = (frame.can_id >> 8) & 0xFF;
  buf[2] = (frame.can_id >> 16) & 0xFF;
  buf[3] = (frame.can_id >> 24) & 0xFF;

  source = buf[0];
  destination = buf[2] < 240 ? buf[1] : 255;
  pgn = (buf[3] & 0x01) << 16 | (buf[2] << 8) | (buf[2] < 240 ? 0 : buf[1]);
  priority = (buf[3] & 0x1c) >> 2;
}


bool CanHeader::IsFastMessage() const {
  static const std::vector<unsigned> haystack = {
      // All known multiframe fast messages
      65240u,  126208u, 126464u, 126996u, 126998u, 127233u, 127237u, 127489u,
      127496u, 127506u, 128275u, 129029u, 129038u, 129039u, 129040u, 129041u,
      129284u, 129285u, 129540u, 129793u, 129794u, 129795u, 129797u, 129798u,
      129801u, 129802u, 129808u, 129809u, 129810u, 130065u, 130074u, 130323u,
      130577u, 130820u, 130822u, 130824u};

  unsigned needle = static_cast<unsigned>(pgn);
  auto found = std::find_if(haystack.begin(), haystack.end(),
                            [needle](unsigned i) { return i == needle; });
  return found != haystack.end();
}

//  FastMessage implementation

bool FastMessageMap::IsEntryExpired(unsigned int i) {
    return (wxDateTime::Now() - entries[i].time_arrived
              > wxTimeSpan(0, 0, kEntryMaxAgeSecs));
}

void FastMessageMap::CheckGc() {
  bool last_run_over_age = (wxDateTime::Now() - last_gc_run) > wxTimeSpan(0, 0, kGcIntervalSecs);
  if (last_run_over_age || entries.size() > kGcThreshold) {
    GarbageCollector();
    last_gc_run = wxDateTime::Now();
  }
}

int FastMessageMap::FindMatchingEntry(const CanHeader header,
                                      const unsigned char sid) {
  for (unsigned i = 0; i < entries.size(); i++) {
    if (((sid & 0xE0) == (entries[i].sid & 0xE0)) &&
        (entries[i].header.pgn == header.pgn) &&
        (entries[i].header.source == header.source) &&
        (entries[i].header.destination == header.destination)) {
      return i;
    }
  }
  return kNotFound;
}

int FastMessageMap::AddNewEntry(void) {
  entries.push_back(Entry());
  return entries.size() - 1;
}

int FastMessageMap::GarbageCollector(void) {
  std::vector<unsigned> stale_entries;
  for (unsigned i = 0; i < entries.size(); i++) {
    //if (entries[i].IsExpired()) stale_entries.push_back(i);
    if (IsEntryExpired(i)) stale_entries.push_back(i);
  }
  for (auto i : stale_entries) Remove(i);
  return stale_entries.size();
}

bool FastMessageMap::InsertEntry(const CanHeader header,
                                 const unsigned char* data, int index) {
  // first message of fast packet
  // data[0] Sequence Identifier (sid)
  // data[1] Length of data bytes
  // data[2..7] 6 data bytes

  CheckGc();
  // Ensure that this is indeed the first frame of a fast message
  if ((data[0] & 0x1F) == 0) {
    int total_data_len;  // will also include padding as we memcpy all of the
                         // frame, because I'm lazy
    total_data_len = static_cast<unsigned int>(data[1]);
    total_data_len += 7 - ((total_data_len - 6) % 7);

    entries[index].sid = static_cast<unsigned int>(data[0]);
    entries[index].expected_length = static_cast<unsigned int>(data[1]);
    entries[index].header = header;
    entries[index].time_arrived = wxDateTime::Now();

    entries[index].data.resize(total_data_len);
    memcpy(&entries[index].data[0], &data[2], 6);
    // First frame of a multi-frame Fast Message contains six data bytes.
    // Position the cursor ready for next message
    entries[index].cursor = 6;

    // Fusion, using fast messages to sends frames less than eight bytes
    return entries[index].expected_length <= 6;
  }
  return false;
  // No further processing is performed if this is not a start frame.
  // A start frame may have been dropped and we received a subsequent frame
}

bool FastMessageMap::AppendEntry(const CanHeader header,
                                 const unsigned char* data, int position) {
  // Check that this is the next message in the sequence
  if ((entries[position].sid + 1) == data[0]) {
    memcpy(&entries[position].data[entries[position].cursor], &data[1], 7);
    entries[position].sid = data[0];
    // Subsequent messages contains seven data bytes (last message may be padded
    // with 0xFF)
    entries[position].cursor += 7;
    // Is this the last message ?
    return entries[position].cursor >= entries[position].expected_length;
  } else if ((data[0] & 0x1F) == 0) {
    // We've found a matching entry, however this is a start frame, therefore
    // we've missed an end frame, and now we have a start frame with the same id
    // (top 3 bits). The id has obviously rolled over. Should really double
    // check that (data[0] & 0xE0) Clear the entry as we don't want to leak
    // memory, prior to inserting a start frame
    entries.erase(entries.begin() + position);
    position = AddNewEntry();
    // And now insert it
    InsertEntry(header, data, position);
    // FIXME (dave) Should update the dropped frame stats
    return true;
  } else {
    // This is not the next frame in the sequence and not a start frame
    // We've dropped an intermedite frame, so free the slot and do no further
    // processing
    entries.erase(entries.begin() + position);
    // Dropped Frame Statistics
    if (dropped_frames == 0) {
      dropped_frame_time = wxDateTime::Now();
      dropped_frames += 1;
    } else {
      dropped_frames += 1;
    }
    // FIXME (dave)
    //     if ((dropped_frames > CONST_DROPPEDFRAME_THRESHOLD) &&
    //     (wxDateTime::Now() < (dropped_frame_time +
    //     wxTimeSpan::Seconds(CONST_DROPPEDFRAME_PERIOD) ) ) ) {
    //       wxLogError(_T("TwoCan Device, Dropped Frames rate exceeded"));
    //       wxLogError(wxString::Format(_T("Frame: Source: %d Destination: %d
    //       Priority: %d PGN: %d"),header.source, header.destination,
    //       header.priority, header.pgn)); dropped_frames = 0;
    //     }
    return false;
  }
}

void FastMessageMap::Remove(int pos) { entries.erase(entries.begin() + pos); }

