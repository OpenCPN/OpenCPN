/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Configuration wizard
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010-2024 by David S. Register                          *
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
 ***************************************************************************
 *
 *
 *
 */

#ifndef _WIZ_UI_H__
#define _WIZ_UI_H__

#include "wiz_ui_proto.h"
#include "model/conn_params.h"
#include "navutil.h"

#include <cstdint>

struct USBDevice {
  std::string name;
  uint16_t vid;
  uint16_t pid;
  DataProtocol protocol;
  int baudrate;
};

enum class NMEA0183Flavor { CRC, NO_CRC, INVALID };

const std::vector<uint32_t> Speeds = {4800,  9600,   19200,  38400,
                                      57600, 115200, 230400, 460800};

// Well known ports used by various NMEA to IP gateways
// 10110 - "Standard" for NMEA0183 over IP
// 2000 - Quark Elec default for both TCP and UDP
// 1456-1458 - YD (See https://www.yachtd.com/downloads/ydnr02.pdf)
// 39150/TCP - Vesper Cortex
const std::vector<uint16_t> UDPPorts = {10110, 2000, 1458};
const std::vector<uint16_t> TCPPorts = {10110, 2000, 1456, 1457, 39150};

const std::vector<USBDevice> known_usb_devices = {
    {"Actisense NGT-1", 0x0403, 0xd9aa, DataProtocol::PROTO_NMEA2000, 115200},
    {"ShipModul MiniPlex", 0x0403, 0xfd4b, DataProtocol::PROTO_NMEA0183,
     460800}};

class FirstUseWizImpl : public FirstUseWiz {
public:
  FirstUseWizImpl(wxWindow* parent, MyConfig *pConfig,
                  wxWindowID id = wxID_ANY,
                  const wxString& title = _("OpenCPN Initial Configuration"),
                  const wxBitmap& bitmap = wxNullBitmap,
                  const wxPoint& pos = wxDefaultPosition,
                  long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER |
                               wxSTAY_ON_TOP);
  ~FirstUseWizImpl();

  void OnWizardPageChanging(wxWizardEvent& event) { event.Skip(); }
  void OnWizardPageChanged(wxWizardEvent& event) { event.Skip(); }
  void OnWizardPageShown(wxWizardEvent& event);
  void OnWizardCancel(wxWizardEvent& event) { event.Skip(); }
  void OnWizardHelp(wxWizardEvent& event) { event.Skip(); }
  void OnWizardFinished(wxWizardEvent& event);
  void m_btnRescanSourcesOnButtonClick(wxCommandEvent& event) {
    EnumerateDatasources();
  }
  void m_btnAddChartDirOnButtonClick(wxCommandEvent& event);

  bool Run() {
    FitToPage(m_pages[m_pages.Count() - 1]);
    return RunWizard(m_pages[0]);
  }

  void EnumerateDatasources();
  void EnumerateUSB();
  void EnumerateUDP();
  void EnumerateTCP();
  void EnumerateCAN();
  void EnumerateSignalK();
  void EnumerateGPSD();

private:
  MyConfig *m_pConfig;
  std::vector<ConnectionParams> m_detected_connections;
  NMEA0183Flavor SeemsN0183(std::string& data);
  bool SeemsN2000(std::string& data);

  inline void SetControlEnable(int id, bool state)
  {
    wxWindow *win = wxWindow::FindWindowById(id);
    if(win) win->Enable(state);
  }
};

#endif  //_WIZ_UI_H__