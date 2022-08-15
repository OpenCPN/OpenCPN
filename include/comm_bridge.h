/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 *
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#include <memory>
#include <string>
#include <unordered_map>

#include "observable_msg.h"
#include "comm_navmsg.h"
#include "comm_decoder.h"

#ifndef _COMM_BRIDGE_H
#define _COMM_BRIDGE_H

typedef struct{
  int active_priority;
  std::string active_source;
} PriorityContainer;


class CommBridge : public wxEvtHandler {
public:
  CommBridge();

  ~CommBridge();

  bool Initialize();
  void InitCommListeners();

  bool HandleN2K_129029(std::shared_ptr<const Nmea2000Msg> n2k_msg);
  bool HandleN2K_129026(std::shared_ptr<const Nmea2000Msg> n2k_msg);

  bool HandleN0183_RMC(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_HDT(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_HDG(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_HDM(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_VTG(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_GSV(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_GGA(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_GLL(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_AIVDO(std::shared_ptr<const Nmea0183Msg> n0183_msg);

  void OnWatchdogTimer(wxTimerEvent& event);
  void PresetWatchdogs();
  void MakeHDTFromHDM();
  void InitializePriorityContainers();
  bool EvalPriorityPosition(std::string priority_key,
                       std::string source,
                       std::shared_ptr <const NavMsg> msg);

  Watchdogs m_watchdogs;
  wxTimer m_watchdog_timer;

  //  comm event listeners
  ObservedVarListener listener_N2K_129029;
  ObservedVarListener listener_N2K_129026;

  ObservedVarListener listener_N0183_RMC;
  ObservedVarListener listener_N0183_HDT;
  ObservedVarListener listener_N0183_HDG;
  ObservedVarListener listener_N0183_HDM;
  ObservedVarListener listener_N0183_VTG;
  ObservedVarListener listener_N0183_GSV;
  ObservedVarListener listener_N0183_GGA;
  ObservedVarListener listener_N0183_GLL;
  ObservedVarListener listener_N0183_AIVDO;

  CommDecoder m_decoder;

  PriorityContainer position_priority;
  std::unordered_map<std::string, int> pos_priority_map;

  DECLARE_EVENT_TABLE()
};

#endif  // _COMM_BRIDGE_H
