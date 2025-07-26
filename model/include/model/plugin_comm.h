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

/**
 * \file
 *  Tools to send data to plugins.
 */

#ifndef PLUGIN__COMM_H
#define PLUGIN__COMM_H

#include <wx/event.h>
#include <wx/jsonval.h>
#include <wx/string.h>

#include "model/ocpn_types.h"

void SendMessageToAllPlugins(const wxString& message_id,
                             const wxString& message_body);

void SendJSONMessageToAllPlugins(const wxString& message_id, wxJSONValue v);

void SendAISSentenceToAllPlugIns(const wxString& sentence);

void SendPositionFixToAllPlugIns(GenericPosDatEx* ppos);

void SendActiveLegInfoToAllPlugIns(const ActiveLegDat* leg_info);

bool SendMouseEventToPlugins(wxMouseEvent& event);

bool SendKeyEventToPlugins(wxKeyEvent& event);

void SendPreShutdownHookToPlugins();

void SendCursorLatLonToAllPlugIns(double lat, double lon);

/**
 * Distribute a NMEA 0183 sentence to all plugins that have registered interest
 * by setting the WANTS_NMEA_SENTENCES capability flag.
 *
 * The sentence will only be sent to plugins that:
 * - Are enabled
 * - Have been initialized
 * - Have declared WANTS_NMEA_SENTENCES in their Init() return flags
 *
 * @param sentence A complete NMEA 0183 sentence including delimiters and
 * checksum.
 *
 * @note For handling NMEA/SignalK messages, a newer recommended message API is
 * available: \htmlonly <a
 * href="https://opencpn-manuals.github.io/main/opencpn-dev/plugin-messaging.html">Plugin
 * Message API Documentation</a> \endhtmlonly
 */
void SendNMEASentenceToAllPlugIns(const wxString& sentence);

int GetJSONMessageTargetCount();

void SendVectorChartObjectInfo(const wxString& chart, const wxString& feature,
                               const wxString& objname, double& lat,
                               double& lon, double& scale, int& nativescale);

/**
 * Interface for core components to handle messages before they are sent to
 * plugins.
 *
 * This allows core components to intercept and process messages that would
 * otherwise be sent to plugins, enabling them to handle specific messages
 * internally.
 */
class CoreMessageHandler {
public:
  /** Handle a message before it is sent to plugins.
   *
   * @param message_id The ID of the message
   * @param message_body The body of the message
   * @return true if the message was handled, false otherwise
   */
  virtual bool HandleMessage(const wxString& message_id,
                             const wxString& message_body) = 0;
};

/**
 * Register a core message handler for a specific message ID.
 * This allows core components to handle messages before they are sent to
 * plugins.
 *
 * @param message_id The ID of the message to handle
 * @param handler The handler that will process the message
 */
void RegisterCoreMessageHandler(const wxString& message_id,
                                CoreMessageHandler* handler);

#endif  //  PLUGIN__COMM_H
