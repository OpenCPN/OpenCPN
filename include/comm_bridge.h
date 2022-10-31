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

#ifndef _COMM_BRIDGE_H
#define _COMM_BRIDGE_H

#include <memory>
#include <string>
#include <unordered_map>

#include <wx/event.h>
#include <wx/log.h>
#include <wx/timer.h>

#include "comm_decoder.h"
#include "comm_navmsg.h"


typedef struct{
  std::string pcclass;
  int active_priority;
  std::string active_source;
  std::string active_identifier;
  int active_source_address;
} PriorityContainer;

typedef struct {
  int position_watchdog;
  int variation_watchdog;
  int heading_watchdog;
  int velocity_watchdog;
  int satellite_watchdog;

} Watchdogs;

class CommBridge : public wxEvtHandler {
public:
  CommBridge();

  ~CommBridge();

  bool Initialize();
  void InitCommListeners();

  bool HandleN2K_129029(std::shared_ptr<const Nmea2000Msg> n2k_msg);
  bool HandleN2K_129025(std::shared_ptr<const Nmea2000Msg> n2k_msg);
  bool HandleN2K_129026(std::shared_ptr<const Nmea2000Msg> n2k_msg);
  bool HandleN2K_127250(std::shared_ptr<const Nmea2000Msg> n2k_msg);
  bool HandleN2K_129540(std::shared_ptr<const Nmea2000Msg> n2k_msg);

  bool HandleN0183_RMC(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_HDT(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_HDG(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_HDM(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_VTG(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_GSV(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_GGA(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_GLL(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN0183_AIVDO(std::shared_ptr<const Nmea0183Msg> n0183_msg);

  bool HandleSignalK(std::shared_ptr<const SignalkMsg> sK_msg);

  void OnDriverStateChange();

  void OnWatchdogTimer(wxTimerEvent& event);
  bool EvalPriority(std::shared_ptr <const NavMsg> msg,
                            PriorityContainer& active_priority,
                            std::unordered_map<std::string, int>& priority_map);
  std::string GetPriorityKey(std::shared_ptr <const NavMsg> msg);

  std::vector<std::string> GetPriorityMaps();
  void ApplyPriorityMaps(std::vector<std::string> new_maps);
  bool LoadConfig( void );
  bool SaveConfig( void );

  Watchdogs m_watchdogs;
  wxTimer m_watchdog_timer;

  //  comm event listeners
  ObservableListener listener_N2K_129029;
  ObservableListener listener_N2K_129025;
  ObservableListener listener_N2K_129026;
  ObservableListener listener_N2K_127250;
  ObservableListener listener_N2K_129540;

  ObservableListener listener_N0183_RMC;
  ObservableListener listener_N0183_HDT;
  ObservableListener listener_N0183_HDG;
  ObservableListener listener_N0183_HDM;
  ObservableListener listener_N0183_VTG;
  ObservableListener listener_N0183_GSV;
  ObservableListener listener_N0183_GGA;
  ObservableListener listener_N0183_GLL;
  ObservableListener listener_N0183_AIVDO;

  ObservableListener listener_SignalK;

  ObservableListener driver_change_listener;

  CommDecoder m_decoder;

private:
  void PresetWatchdogs();
  void MakeHDTFromHDM();
  void InitializePriorityContainers();
  void PresetPriorityContainers();

  std::string GetPriorityMap(std::unordered_map<std::string, int> &map);
  void ApplyPriorityMap(std::unordered_map<std::string, int>& priority_map,
                        wxString &new_prio, int category);
  void ClearPriorityMaps();
  void PresetPriorityContainer(PriorityContainer &pc,
                        const std::unordered_map<std::string, int> &priority_map);
  void SelectNextLowerPriority(const std::unordered_map<std::string, int> &map,
                                         PriorityContainer &pc);

  PriorityContainer active_priority_position;
  PriorityContainer active_priority_velocity;
  PriorityContainer active_priority_heading;
  PriorityContainer active_priority_variation;
  PriorityContainer active_priority_satellites;

  std::unordered_map<std::string, int> priority_map_position;
  std::unordered_map<std::string, int> priority_map_velocity;
  std::unordered_map<std::string, int> priority_map_heading;
  std::unordered_map<std::string, int> priority_map_variation;
  std::unordered_map<std::string, int> priority_map_satellites;

  DECLARE_EVENT_TABLE()
};

#endif  // _COMM_BRIDGE_H
