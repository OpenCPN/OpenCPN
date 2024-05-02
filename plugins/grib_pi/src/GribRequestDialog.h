/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Friends
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 */

#ifndef __GRIBREQUESTDIALOG_H__
#define __GRIBREQUESTDIALOG_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "GribUIDialogBase.h"
#include "GribUIDialog.h"
#include "pi_ocpndc.h"
#include "wx/jsonreader.h"

enum GribDownloadType { WORLD, LOCAL, LOCAL_CATALOG, XYGRIB, NONE };

const std::string CATALOG_URL =
    "https://raw.githubusercontent.com/chartcatalogs/gribcatalog/main/"
    "sources.json";

//----------------------------------------------------------------------------------------------------------
//    Request setting Specification
//----------------------------------------------------------------------------------------------------------
class GribRequestSetting : public GribRequestSettingBase {
public:
  GribRequestSetting(GRIBUICtrlBar &parent);

  ~GribRequestSetting();

  void OnClose(wxCloseEvent &event) override;
  void SetVpSize(PlugIn_ViewPort *vp);
  void OnVpChange(PlugIn_ViewPort *vp);
  bool MouseEventHook(wxMouseEvent &event);
  bool RenderZoneOverlay(wxDC &dc);
  bool RenderGlZoneOverlay();
  bool DoRenderZoneOverlay();
  void SetRequestDialogSize();
  void StopGraphicalZoneSelection();
  void Save() {
    wxCommandEvent evt;
    OnSaveMail(evt);
  }

  wxString m_RequestConfigBase;
  wxString m_MailToAddresses;
  int m_RenderZoneOverlay;

  wxPoint m_StartPoint;
  PlugIn_ViewPort *m_Vp;
  double m_Lat;
  double m_Lon;

private:
  void HighlightArea(double latmax, double lonmax, double latmin,
                     double lonmin);
  void ReadLocalCatalog();
  void FillTreeCtrl(wxJSONValue &data);
  void ApplyRequestConfig(unsigned rs, unsigned it, unsigned tr);
  wxString WriteMail();
  int EstimateFileSize(double *size);

  void InitRequestConfig();
  void OnExit(wxCommandEvent &event) {
    wxCloseEvent evt;
    OnClose(evt);
  }
  void OnTopChange(wxCommandEvent &event) override;
  void OnMovingClick(wxCommandEvent &event) override;
  void OnAnyChange(wxCommandEvent &event) override;
  void OnAnySpinChange(wxSpinEvent &event) override {
    wxCommandEvent evt;
    OnAnyChange(evt);
  }
  void OnNotebookPageChanged(wxNotebookEvent &event) override {
    HighlightArea(0, 0, 0, 0);
  }
  void OnTimeRangeChange(wxCommandEvent &event) override;
  void OnSendMaiL(wxCommandEvent &event) override;
  void OnSaveMail(wxCommandEvent &event) override;
  void OnZoneSelectionModeChange(wxCommandEvent &event) override;
  void OnCancel(wxCommandEvent &event) override {
    wxCloseEvent evt;
    OnClose(evt);
  }
  void OnCoordinatesChange(wxSpinEvent &event) override;
  void OnMouseEventTimer(wxTimerEvent &event);
  void SetCoordinatesText();
  void OnWorldLengthChoice(wxCommandEvent &event) override { event.Skip(); }
  void OnWorldResolutionChoice(wxCommandEvent &event) override { event.Skip(); }
  void OnWorldDownload(wxCommandEvent &event) override;
  void OnLocalTreeItemExpanded(wxTreeEvent &event) override { event.Skip(); }
  void OnLocalTreeSelChanged(wxTreeEvent &event) override;
  void OnUpdateLocalCatalog(wxCommandEvent &event) override;
  void OnDownloadLocal(wxCommandEvent &event) override;
  void onDLEvent(OCPN_downloadEvent &ev);
  void EnableDownloadButtons();

  void AddXyGribGFSUrlParams(wxString &urlStr);
  wxString BuildXyGribUrl();
  wxString BuildGribFileName();
  void OnXyGribDownloadButton(wxCommandEvent &event) override;

  GRIBUICtrlBar &m_parent;

  wxDC *m_pdc;
  pi_ocpnDC *m_oDC;  // Used for selection overlay on GL

  wxTimer m_tMouseEventTimer;
  wxTimer m_tMouseClickTimer;
  wxMouseEvent m_SingleClickEvent;

  bool IsZYGRIB;
  bool IsGFS;
  int m_MailError_Nb;
  int m_SendMethod;
  bool m_AllowSend;
  bool m_IsMaxLong;
  double m_displayScale;
  bool m_connected;
  bool m_downloading;
  long m_download_handle;
  bool m_bTransferSuccess;
  bool m_canceled;
  bool m_bLocal_source_selected;
  GribDownloadType m_downloadType;
};

#endif
