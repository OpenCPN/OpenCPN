/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Multiplexer Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifdef __MSVC__
#include "winsock2.h"
#include "wx/msw/winundef.h"
#endif

#include "config.h"

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#if defined(HAVE_READLINK) && !defined(HAVE_LIBGEN_H)
#error Using readlink(3) requires libgen.h which cannot be found.
#endif

#include "wx/wx.h"
#include "multiplexer.h"
#include "NMEALogWindow.h"
#include "conn_params.h"
#include "comm_drv_registry.h"
#include "comm_drv_n0183_serial.h"
#include "comm_drv_n0183_net.h"

#ifdef __linux__
#include "udev_rule_mgr.h"
#endif

extern wxString g_GPS_Ident;
extern bool g_b_legacy_input_filter_behaviour;

wxDEFINE_EVENT(EVT_N0183_MUX, ObservedEvt);

#ifdef HAVE_READLINK

static std::string do_readlink(const char *link) {
  // Strip possible Serial: or Usb: prefix:
  const char *colon = strchr(link, ':');
  const char *path = colon ? colon + 1 : link;

  char target[PATH_MAX + 1] = {0};
  int r = readlink(path, target, sizeof(target));
  if (r == -1 && (errno == EINVAL || errno == ENOENT)) {
    // Not a a symlink
    return path;
  }
  if (r == -1) {
    wxLogDebug("Error reading device link %s: %s", path, strerror(errno));
    return path;
  }
  if (*target == '/') {
    return target;
  }
  char buff[PATH_MAX + 1];
  memcpy(buff, path, std::min(strlen(path) + 1, (size_t)PATH_MAX));
  return std::string(dirname(buff)) + "/" + target;
}

static bool is_same_device(const char *port1, const char *port2) {
  std::string dev1 = do_readlink(port1);
  std::string dev2 = do_readlink(port2);
  return dev1 == dev2;
}

#else  // HAVE_READLINK

static bool inline is_same_device(const char *port1, const char *port2) {
  return strcmp(port1, port2) == 0;
}

#endif  // HAVE_READLINK

Multiplexer::Multiplexer() {
  auto &msgbus = NavMsgBus::GetInstance();

  m_listener_N0183_all =
      msgbus.GetListener(EVT_N0183_MUX, this, Nmea0183Msg::MessageKey("ALL"));
  Bind(EVT_N0183_MUX, [&](ObservedEvt ev) {
    auto ptr = ev.GetSharedPtr();
    auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
    HandleN0183(n0183_msg);
  });

  if (g_GPS_Ident.IsEmpty()) g_GPS_Ident = wxT("Generic");
}

Multiplexer::~Multiplexer() {}

void Multiplexer::LogOutputMessageColor(const wxString &msg,
                                        const wxString &stream_name,
                                        const wxString &color) {
  if (NMEALogWindow::Get().Active()) {
    wxDateTime now = wxDateTime::Now();
    wxString ss;
#ifndef __WXQT__  //  Date/Time on Qt are broken, at least for android
    ss = now.FormatISOTime();
#endif
    ss.Prepend(_T("--> "));
    ss.Append(_T(" ("));
    ss.Append(stream_name);
    ss.Append(_T(") "));
    ss.Append(msg);
    ss.Prepend(color);

    NMEALogWindow::Get().Add(ss);
  }
}

void Multiplexer::LogOutputMessage(const wxString &msg, wxString stream_name,
                                   bool b_filter) {
  if (b_filter)
    LogOutputMessageColor(msg, stream_name, _T("<CORAL>"));
  else
    LogOutputMessageColor(msg, stream_name, _T("<BLUE>"));
}

void Multiplexer::LogInputMessage(const wxString &msg,
                                  const wxString &stream_name, bool b_filter,
                                  bool b_error) {
  if (NMEALogWindow::Get().Active()) {
    wxDateTime now = wxDateTime::Now();
    wxString ss;
#ifndef __WXQT__  //  Date/Time on Qt are broken, at least for android
    ss = now.FormatISOTime();
#endif
    ss.Append(_T(" ("));
    ss.Append(stream_name);
    ss.Append(_T(") "));
    ss.Append(msg);
    if (b_error) {
      ss.Prepend(_T("<RED>"));
    } else {
      if (b_filter)
        if (g_b_legacy_input_filter_behaviour)
          ss.Prepend(_T("<CORAL>"));
        else
          ss.Prepend(_T("<MAROON>"));
      else
        ss.Prepend(_T("<GREEN>"));
    }

    NMEALogWindow::Get().Add(ss);
  }
}

void Multiplexer::HandleN0183(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  // Find the driver that originated this message

  const auto& drivers = CommDriverRegistry::getInstance().GetDrivers();
  auto target_driver = FindDriver(drivers, n0183_msg->source->iface);

  wxString fmsg;

  // Send to the Debug Window, if open
  //  Special formatting for non-printable characters helps debugging NMEA
  //  problems
  if (NMEALogWindow::Get().Active()) {
    std::string str = n0183_msg->payload;

    // Get the params for the driver sending this message
      ConnectionParams params;
      auto drv_serial =
          std::dynamic_pointer_cast<CommDriverN0183Serial>(target_driver);
      if (drv_serial) {
        params = drv_serial->GetParams();
      } else {
        auto drv_net = std::dynamic_pointer_cast<CommDriverN0183Net>(target_driver);
        if (drv_net) {
          params = drv_net->GetParams();
        }
      }

    // Check to see if the message passes the source's input filter
    bool bpass = true;
    bpass = params.SentencePassesFilter(n0183_msg->payload.c_str(),
                                        FILTER_INPUT);

    bool b_error = false;
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
      if (isprint(*it))
        fmsg += *it;
      else {
        wxString bin_print;
        bin_print.Printf(_T("<0x%02X>"), *it);
        fmsg += bin_print;
        if ((*it != 0x0a) && (*it != 0x0d)) b_error = true;
      }
    }

    // FIXME (dave)  Flag checksum errors, but fix and process the sentence anyway
    //std::string goodMessage(message);
    //bool checksumOK = CheckSumCheck(event.GetNMEAString());
    //if (!checksumOK) {
    //goodMessage = stream->FixChecksum(goodMessage);
    //goodEvent->SetNMEAString(goodMessage);
    //}


    wxString port(n0183_msg->source->iface);
    LogInputMessage(fmsg, port, !bpass, b_error);
  }

  // Do not mux-out anything coming from a "virtual" or plugin stream
  if (!target_driver)
    return;

  // Perform multiplexer output functions
  for (auto& driver : drivers) {
    if (driver->bus == NavAddr::Bus::N0183) {
      ConnectionParams params;
      auto drv_serial =
          std::dynamic_pointer_cast<CommDriverN0183Serial>(driver);
      if (drv_serial) {
        params = drv_serial->GetParams();
      } else {
        auto drv_net = std::dynamic_pointer_cast<CommDriverN0183Net>(driver);
        if (drv_net) {
          params = drv_net->GetParams();
        }
      }

      // FIXME (dave)   Think about this.  Probably OK to drop this undocumented legacy setting
      //if ((g_b_legacy_input_filter_behaviour && !bpass) || bpass) {

      //  Allow re-transmit on same port (if type is SERIAL),
      //  or any any other NMEA0183 port supporting output
      if (params.Type == SERIAL || driver->iface != target_driver->iface) {
        if (params.IOSelect == DS_TYPE_INPUT_OUTPUT ||
            params.IOSelect == DS_TYPE_OUTPUT)
        {
          bool bout_filter = true;
          bool bxmit_ok = true;
          if (params.SentencePassesFilter(n0183_msg->payload.c_str(),
                                          FILTER_OUTPUT)) {
            driver->SendMessage(n0183_msg,
                                std::make_shared<NavAddr0183>(driver->iface));
            bout_filter = false;
          }

          // Send to the Debug Window, if open
          if (!bout_filter) {
            if (bxmit_ok)
              LogOutputMessageColor(fmsg, driver->iface, _T("<BLUE>"));
            else
              LogOutputMessageColor(fmsg, driver->iface, _T("<RED>"));
          } else
             LogOutputMessageColor(fmsg, driver->iface, _T("<CORAL>"));

        }
      }
    }
  }
}

