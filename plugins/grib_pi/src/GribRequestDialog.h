/***************************************************************************
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
/**
 * \file
 * GRIB Weather Data Request and Download Management.
 *
 * Provides comprehensive functionality for requesting and downloading GRIB
 * weather forecast data from multiple sources:
 *
 * Download Sources:
 * - Global models (GFS, ECMWF, etc.)
 * - XyGrib specialized marine weather service
 * - Local GRIB catalogs and repositories
 * - Email-based request services
 *
 * Key Features:
 * - Interactive geographic zone selection
 * - Multiple forecast model support
 * - Resolution and time range configuration
 * - File size estimation and validation
 * - Download progress tracking
 * - Catalog management and updates
 * - Email request template generation
 *
 * The interface provides visual feedback during zone selection and supports
 * both manual coordinate entry and graphical region selection. It handles
 * different download methods including direct HTTP(S) downloads and email-based
 * requests with size and coverage validation.
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

enum GribDownloadType {
  WORLD,          ///< Global forecast downloads (e.g., GFS)
  LOCAL,          ///< Downloads from local sources
  LOCAL_CATALOG,  ///< Downloads from configured catalogs
  XYGRIB,         ///< Downloads from XyGrib service
  NONE            ///< No download source selected
};

const std::string CATALOG_URL =
    "https://raw.githubusercontent.com/chartcatalogs/gribcatalog/main/"
    "sources.json";

#define XYGRIB_MAX_DOWNLOADABLE_GRIB_SIZE_MB 10

/**
 * Manages GRIB file request configuration and downloads.
 *
 * This class provides:
 * - User interface for request configuration
 * - Geographic zone selection
 * - Multiple download source support
 * - File size estimation
 * - Request validation
 * - Visual feedback during selection
 */
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

  // Xygrib internal methods
  void InitializeXygribDialog();
  wxString BuildXyGribUrl();
  wxString BuildGribFileName();
  // XyGrib GUI callbacks
  void OnXyGribDownloadButton(wxCommandEvent &event) override;
  void OnXyGribAtmModelChoice(wxCommandEvent &event) override;
  void OnXyGribWaveModelChoice(wxCommandEvent &event) override;
  void OnXyGribConfigChange(wxCommandEvent &event) override;
  // Manage XyGrib UI Configuration
  void ApplyXyGribConfiguration();
  void MemorizeXyGribConfiguration();
  // Calculate estimated size of GRIB file
  void UpdateGribSizeEstimate();

  // Index of currently selected XyGrib atmospheric model
  int m_selectedAtmModelIndex;
  // Index of currently selected XyGrib wave model
  int m_selectedWaveModelIndex;
  // Last size estimation of the GRIB file
  int m_gribSizeEstimate;

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
