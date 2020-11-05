/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Hakan Svensson
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "EmulatorReceive.h"
#include "RadarFactory.h"

#define SCALE_RAW_TO_DEGREES(raw) ((raw) * (double)DEGREES_PER_ROTATION / EMULATOR_SPOKES)
#define SCALE_DEGREES_TO_RAW(angle) ((int)((angle) * (double)EMULATOR_SPOKES / DEGREES_PER_ROTATION))

PLUGIN_BEGIN_NAMESPACE

/*
 * This file not only contains the radar receive threads, it is also
 * the only unit that understands what the radar returned data looks like.
 * The rest of the plugin uses a (slightly) abstract definition of the radar.
 */

#define MILLIS_PER_SELECT 250
#define SECONDS_SELECT(x) ((x)*MILLISECONDS_PER_SECOND / MILLIS_PER_SELECT)

/*
 * Called once a second. Emulate a radar return that is
 * at the current desired auto_range.
 * Speed is 24 images per minute, e.g. 1/2.5 of a full
 * image.
 */

void EmulatorReceive::EmulateFakeBuffer(void) {
  time_t now = time(0);
  uint8_t data[EMULATOR_MAX_SPOKE_LEN];

  wxCriticalSectionLocker lock(m_ri->m_exclusive);

  m_ri->m_radar_timeout = now + WATCHDOG_TIMEOUT;

  int state = m_ri->m_state.GetValue();

  if (state != RADAR_TRANSMIT) {
    if (state == RADAR_OFF) {
      m_ri->m_state.Update(RADAR_STANDBY);
    }
    return;
  }

  m_ri->m_statistics.packets++;
  m_ri->m_data_timeout = now + WATCHDOG_TIMEOUT;

  m_next_rotation = (m_next_rotation + 1) % EMULATOR_SPOKES;

  int scanlines_in_packet = EMULATOR_SPOKES * 24 / 60 * MILLIS_PER_SELECT / MILLISECONDS_PER_SECOND;
  int range_meters = m_ri->m_range.GetValue();

  const int *ranges;
  size_t count = RadarFactory::GetRadarRanges(RT_EMULATOR, M_SETTINGS.range_units, &ranges);

  if (range_meters < ranges[0]) {
    range_meters = ranges[0];
    m_ri->m_range.Update(range_meters);
  }
  if (range_meters > ranges[count - 1]) {
    range_meters = ranges[count - 1];
    m_ri->m_range.Update(range_meters);
  }

  int spots = 0;

  for (int scanline = 0; scanline < scanlines_in_packet; scanline++) {
    int angle = m_next_spoke;
    m_next_spoke = MOD_SPOKES(m_next_spoke + 1);
    m_ri->m_statistics.spokes++;

    if (range_meters == ranges[count - 1]) {
      // New pattern suited for arpa / guard zone detection
      CLEAR_STRUCT(data);
      if (scanline < 8) {
        for (size_t range = 384; range < 410; range++) {
          data[range] = 255;
          spots++;
        }
      }
    } else {
      // The blotchy pattern
      // Invent a pattern. Outermost ring, then a square pattern
      for (size_t range = 0; range < sizeof(data); range++) {
        size_t bit = range >> 7;
        // use bit 'bit' of angle_raw
        uint8_t colour = (((angle + m_next_rotation) >> 5) & (2 << bit)) > 0 ? (range / 2) : 0;
        if (range > sizeof(data) - 10) {
          colour = ((angle + m_next_rotation) % EMULATOR_SPOKES) <= 8 ? 255 : 0;
        }
        data[range] = colour;
        if (colour > 0) {
          spots++;
        }
      }
    }

    int hdt = SCALE_DEGREES_TO_SPOKES(m_pi->GetHeadingTrue());
    int bearing = MOD_SPOKES(angle + hdt);

    wxLongLong time_rec = wxGetUTCTimeMillis();
    m_ri->ProcessRadarSpoke(angle, bearing, data, sizeof(data), range_meters, time_rec);
  }

  LOG_VERBOSE(wxT("radar_pi: emulating %d spokes at range %d with %d spots"), scanlines_in_packet, range_meters, spots);
}

/*
 * Entry
 *
 * Called by wxThread when the new thread is running.
 * It should remain running until Shutdown is called.
 */
void *EmulatorReceive::Entry(void) {
  int r = 0;
  NetworkAddress fake(127, 0, 0, 10, 3333);

  LOG_VERBOSE(wxT("radar_pi: EmulatorReceive thread %s starting"), m_ri->m_name.c_str());

  m_ri->DetectedRadar(fake, fake);

  while (!m_shutdown) {
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = (long)(MILLIS_PER_SELECT * 1000);

    fd_set fdin;
    FD_ZERO(&fdin);

    int maxFd = INVALID_SOCKET;
    if (m_receive_socket != INVALID_SOCKET) {
      FD_SET(m_receive_socket, &fdin);
      maxFd = MAX(m_receive_socket, maxFd);
    }

    r = select(maxFd + 1, &fdin, 0, 0, &tv);
    if (r > 0) {
      if (m_receive_socket != INVALID_SOCKET && FD_ISSET(m_receive_socket, &fdin)) {
        uint8_t data[10];
        sockaddr_in rx_addr;

        socklen_t rx_len = sizeof(rx_addr);
        r = recvfrom(m_receive_socket, (char *)data, sizeof(data), 0, (struct sockaddr *)&rx_addr, &rx_len);
        if (r > 0) {
          LOG_VERBOSE(wxT("radar_pi: %s received stop instruction"), m_ri->m_name.c_str());
          break;
        }
      }
    }

    EmulateFakeBuffer();

  }  // endless loop until thread destroy

  LOG_VERBOSE(wxT("radar_pi: %s receive thread stopping"), m_ri->m_name.c_str());
  return 0;
}

// Called from the main thread to stop this thread.
// We send a simple one byte message to the thread so that it awakens from the select() call with
// this message ready for it to be read on 'm_receive_socket'. See the constructor in EmulatorReceive.h
// for the setup of these two sockets.

void EmulatorReceive::Shutdown() {
  m_shutdown = true;
  if (m_send_socket != INVALID_SOCKET) {
    if (send(m_send_socket, "!", 1, MSG_DONTROUTE) > 0) {
      LOG_VERBOSE(wxT("radar_pi: %s requested receive thread to stop"), m_ri->m_name.c_str());
      return;
    }
  }
  LOG_INFO(wxT("radar_pi: %s receive thread will take long time to stop"), m_ri->m_name.c_str());
}

wxString EmulatorReceive::GetInfoStatus() { return _("OK"); }

PLUGIN_END_NAMESPACE
