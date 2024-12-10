/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022  Alec Leamas                                       *
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

/**
 *  \file
 *  Implement various ocpn_plugin.h methods.
 */

#include <setjmp.h>

#include <wx/event.h>
#include <wx/jsonval.h>
#include <wx/jsonreader.h>
#include <wx/jsonwriter.h>

#include "model/comm_appmsg.h"
#include "model/comm_navmsg_bus.h"
#include "model/plugin_loader.h"

#include "model/plugin_comm.h"

#include "ocpn_plugin.h"

#ifndef _WIN32

static struct sigaction sa_all_PIM_previous;
static sigjmp_buf env_PIM;

static void catch_signals_PIM(int signo) {
  switch (signo) {
    case SIGSEGV:
      siglongjmp(env_PIM, 1);  // jump back to the setjmp() point
      break;

    default:
      break;
  }
}

#endif

void SendMessageToAllPlugins(const wxString& message_id,
                             const wxString& message_body) {
  auto msg = std::make_shared<PluginMsg>(
      PluginMsg(message_id.ToStdString(), message_body.ToStdString()));
  NavMsgBus::GetInstance().Notify(msg);

  // decouple 'const wxString &' and 'wxString &' to keep bin
  wxString decouple_message_id(message_id);
  wxString decouple_message_body(message_body);

  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_PLUGIN_MESSAGING) {
        switch (pic->m_api_version) {
          case 106: {
            auto* ppi = dynamic_cast<opencpn_plugin_16*>(pic->m_pplugin);
            if (ppi)
              ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
            break;
          }
          case 107: {
            auto* ppi = dynamic_cast<opencpn_plugin_17*>(pic->m_pplugin);
            if (ppi)
              ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
            break;
          }
          case 108:
          case 109:
          case 110:
          case 111:
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117:
          case 118:
          case 119: {
            auto* ppi = dynamic_cast<opencpn_plugin_18*>(pic->m_pplugin);
            if (ppi)
              ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

void SendJSONMessageToAllPlugins(const wxString& message_id, wxJSONValue v) {
  wxJSONWriter w;
  wxString out;
  w.Write(v, out);
  SendMessageToAllPlugins(message_id, out);
  wxLogDebug(message_id);
  wxLogDebug(out);
}

void SendAISSentenceToAllPlugIns(const wxString& sentence) {
  // decouple 'const wxString &' to keep interface.
  wxString decouple_sentence(sentence);
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_AIS_SENTENCES)
        pic->m_pplugin->SetAISSentence(decouple_sentence);
    }
  }
}

void SendPositionFixToAllPlugIns(GenericPosDatEx* ppos) {
  //    Send basic position fix
  PlugIn_Position_Fix pfix;
  pfix.Lat = ppos->kLat;
  pfix.Lon = ppos->kLon;
  pfix.Cog = ppos->kCog;
  pfix.Sog = ppos->kSog;
  pfix.Var = ppos->kVar;
  pfix.FixTime = ppos->FixTime;
  pfix.nSats = ppos->nSats;

  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_NMEA_EVENTS)
        if (pic->m_pplugin) pic->m_pplugin->SetPositionFix(pfix);
    }
  }

  //    Send extended position fix to PlugIns at API 108 and later
  PlugIn_Position_Fix_Ex pfix_ex;
  pfix_ex.Lat = ppos->kLat;
  pfix_ex.Lon = ppos->kLon;
  pfix_ex.Cog = ppos->kCog;
  pfix_ex.Sog = ppos->kSog;
  pfix_ex.Var = ppos->kVar;
  pfix_ex.FixTime = ppos->FixTime;
  pfix_ex.nSats = ppos->nSats;
  pfix_ex.Hdt = ppos->kHdt;
  pfix_ex.Hdm = ppos->kHdm;

  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_NMEA_EVENTS) {
        switch (pic->m_api_version) {
          case 108:
          case 109:
          case 110:
          case 111:
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117:
          case 118:
          case 119: {
            auto* ppi = dynamic_cast<opencpn_plugin_18*>(pic->m_pplugin);
            if (ppi) ppi->SetPositionFixEx(pfix_ex);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

void SendActiveLegInfoToAllPlugIns(const ActiveLegDat* leg_info) {
  Plugin_Active_Leg_Info leg;
  leg.Btw = leg_info->Btw;
  leg.Dtw = leg_info->Dtw;
  leg.wp_name = leg_info->wp_name;
  leg.Xte = leg_info->Xte;
  leg.arrival = leg_info->arrival;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_NMEA_EVENTS) {
        switch (pic->m_api_version) {
          case 108:
          case 109:
          case 110:
          case 111:
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
            break;
          case 117:
          case 118:
          case 119: {
            auto* ppi = dynamic_cast<opencpn_plugin_117*>(pic->m_pplugin);
            if (ppi) ppi->SetActiveLegInfo(leg);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

bool SendMouseEventToPlugins(wxMouseEvent& event) {
  bool bret = false;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_MOUSE_EVENTS) {
        switch (pic->m_api_version) {
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117:
          case 118:
          case 119: {
            auto* ppi = dynamic_cast<opencpn_plugin_112*>(pic->m_pplugin);
            if (ppi && ppi->MouseEventHook(event)) bret = true;
            break;
          }
          default:
            break;
        }
      }
    }
  }
  return bret;
}

bool SendKeyEventToPlugins(wxKeyEvent& event) {
  bool bret = false;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_KEYBOARD_EVENTS) {
        {
          switch (pic->m_api_version) {
            case 113:
            case 114:
            case 115:
            case 116:
            case 117:
            case 118:
            case 119: {
              auto* ppi = dynamic_cast<opencpn_plugin_113*>(pic->m_pplugin);
              if (ppi && ppi->KeyboardEventHook(event)) bret = true;
              break;
            }
            default:
              break;
          }
        }
      }
    }
  }

  return bret;
}

void SendPreShutdownHookToPlugins() {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_PRESHUTDOWN_HOOK) {
        switch (pic->m_api_version) {
          case 119: {
            auto* ppi = dynamic_cast<opencpn_plugin_119*>(pic->m_pplugin);
            if (ppi) ppi->PreShutdownHook();
            break;
          }
          default:
            break;
        }
      }
    }
  }
}

void SendCursorLatLonToAllPlugIns(double lat, double lon) {
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_CURSOR_LATLON)
        if (pic->m_pplugin) pic->m_pplugin->SetCursorLatLon(lat, lon);
    }
  }
}

void SendNMEASentenceToAllPlugIns(const wxString& sentence) {
  // decouple 'const wxString &' to keep plugin interface.
  wxString decouple_sentence(sentence);
#ifndef __WXMSW__
  // Set up a framework to catch (some) sigsegv faults from plugins.
  sigaction(SIGSEGV, NULL, &sa_all_PIM_previous);  // save existing
                                                   // action for this signal
  struct sigaction temp;
  sigaction(SIGSEGV, NULL, &temp);  // inspect existing action for this signal

  temp.sa_handler = catch_signals_PIM;  // point to my handler
  sigemptyset(&temp.sa_mask);           // make the blocking set
                                        // empty, so that all
                                        // other signals will be
                                        // unblocked during my handler
  temp.sa_flags = 0;
  sigaction(SIGSEGV, &temp, NULL);
#endif
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_NMEA_SENTENCES) {
#ifndef __WXMSW__
        if (sigsetjmp(env_PIM, 1)) {
          //  Something in the "else" code block faulted.
          // Probably safest to assume that all variables in this method are
          // trash... So, simply clean up and return.
          sigaction(SIGSEGV, &sa_all_PIM_previous, NULL);
          // reset signal handler
          return;
        } else
#endif
        {
          // volatile int *x = 0;
          //*x = 0;
          if (pic->m_pplugin)
            pic->m_pplugin->SetNMEASentence(decouple_sentence);
        }
      }
    }
  }
#ifndef __WXMSW__
  sigaction(SIGSEGV, &sa_all_PIM_previous, NULL);  // reset signal handler
#endif
}

int GetJSONMessageTargetCount() {
  int rv = 0;
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state &&
        (pic->m_cap_flag & WANTS_PLUGIN_MESSAGING))
      rv++;
  }
  return rv;
}

void SendVectorChartObjectInfo(const wxString& chart, const wxString& feature,
                               const wxString& objname, double& lat,
                               double& lon, double& scale, int& nativescale) {
  wxString decouple_chart(chart);
  wxString decouple_feature(feature);
  wxString decouple_objname(objname);
  auto plugin_array = PluginLoader::getInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = (*plugin_array)[i];
    if (pic->m_enabled && pic->m_init_state) {
      if (pic->m_cap_flag & WANTS_VECTOR_CHART_OBJECT_INFO) {
        switch (pic->m_api_version) {
          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117:
          case 118:
          case 119: {
            auto* ppi = dynamic_cast<opencpn_plugin_112*>(pic->m_pplugin);
            if (ppi)
              ppi->SendVectorChartObjectInfo(decouple_chart, decouple_feature,
                                             decouple_objname, lat, lon, scale,
                                             nativescale);
            break;
          }
          default:
            break;
        }
      }
    }
  }
}
