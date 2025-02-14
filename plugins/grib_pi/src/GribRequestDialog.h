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
 * Enumeration defining the states of the GRIB zone selection overlay rendering.
 */
enum ZoneSelectionRenderState {
  RENDER_NONE = 0,      // No selection zone to render
  RENDER_COMPLETE = 1,  // Selection is complete, render the final zone
  RENDER_DRAWING = 2    // User is actively drawing the selection zone
};

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
  /**
   * Callback invoked when the view port under mouse has changed.
   * This is the viewport where the mouse cursor is currently located.
   */
  void OnVpUnderMouseChange(PlugIn_ViewPort *vp);
  /**
   * Callback invoked when the focused view port has changed,
   * such as in multi-chart mode when user switches viewport focus.
   */
  void OnVpWithFocusChange(PlugIn_ViewPort *vp);
  /**
   * Intercepts mouse events to handle GRIB area selection.
   *
   * We only handle mouse events when the Shift key is pressed to avoid
   * conflicting with OpenCPN core chart panning, which uses normal left-click
   * drag. This lets users:
   * - Pan the chart normally with left-click drag.
   * - Select GRIB download area with Shift + left-click drag.
   *
   * The selection states flow:
   * 1. User holds Shift and left-clicks: Start selection (DRAW_SELECTION)
   * 2. User drags with Shift still held: Update selection rectangle
   * 3. User releases left button: Complete selection (COMPLETE_SELECTION)
   *
   * @param event The intercepted mouse event.
   * @return true if event was handled, false to pass event to chart.
   */
  bool MouseEventHook(wxMouseEvent &event);
  /**
   * Renders the GRIB area selection overlay using standard device context.
   * Called by OpenCPN when in standard graphics mode. Passes the DC to
   * DoRenderZoneOverlay() for actual rendering.
   *
   * @param dc Device context to draw on
   * @return true if overlay should be rendered, false if nothing to draw
   */
  bool RenderZoneOverlay(wxDC &dc);
  /**
   * Renders the GRIB area selection overlay using OpenGL.
   * Called by OpenCPN when in OpenGL mode. Sets up OpenGL context
   * and delegates actual rendering to DoRenderZoneOverlay().
   *
   * @return true if overlay should be rendered, false if nothing to draw
   */
  bool RenderGlZoneOverlay();
  /**
   * Draws the GRIB area selection overlay on the chart.
   * This includes the rectangular selection zone and an information label
   * showing the coordinates and estimated file size.
   * Supports both standard wxDC and OpenGL rendering paths.
   *
   * @return true if drawing was successful
   */
  bool DoRenderZoneOverlay();
  void SetRequestDialogSize();
  void StopGraphicalZoneSelection();
  void UpdateAreaSelectionState();
  /**
   * Get the minimum latitude of the bounding box for the download request.
   *
   * The bouding box is defined by the user during manual zone selection
   * or by the visible area of the chart in focus.
   *
   * @return The minimum latitude of the bounding box.
   */
  double GetMinLat() const;
  /**
   * Get the maximum latitude of the bounding box for the download request.
   *
   * The bouding box is defined by the user during manual zone selection
   * or by the visible area of the chart in focus.
   *
   * @return The maximum latitude of the bounding box.
   */
  double GetMaxLat() const;
  /**
   * Get the minimum longitude of the bounding box for the download request.
   *
   * The bouding box is defined by the user during manual zone selection
   * or by the visible area of the chart in focus.
   *
   * @return The minimum longitude of the bounding box.
   */
  double GetMinLon() const;
  /**
   * Get the maximum longitude of the bounding box for the download request.
   *
   * The bouding box is defined by the user during manual zone selection
   * or by the visible area of the chart in focus.
   *
   * @return The maximum longitude of the bounding box.
   */
  double GetMaxLon() const;

  int GetBoundingBoxCanvasIndex() const { return m_boundingBoxCanvasIndex; }

  void Save() {
    wxCommandEvent evt;
    OnOK(evt);
  }

  wxString m_RequestConfigBase;
  wxString m_MailToAddresses;
  /**
   * Current state of the bounding box overlay rendering.
   * Controls whether and how the selection zone is displayed on the chart.
   * @see ZoneSelectionRenderState
   */
  ZoneSelectionRenderState m_RenderSelectionZoneState;

  /**
   * Starting point of the bounding box in physical pixels.
   * Set when user begins dragging to select a bounding box. Acts as the anchor
   * point for drawing the bounding box rectangle.
   */
  wxPoint m_StartPoint;
  /** The latitude at the starting point of the bounding box. */
  double m_StartLat;
  /** The longitude at the starting point of the bounding box. */
  double m_StartLon;

  /**
   * The viewport currently in focus.
   *
   * @note In multi-canvas mode, this may be different from the viewport under
   * the mouse cursor.
   */
  PlugIn_ViewPort *m_VpFocus;
  /** The viewport under the mouse. */
  PlugIn_ViewPort *m_VpMouse;
  /** The latitude at the mouse cursor while drawing a bounding box. */
  double m_Lat;
  /** The longitude at the mouse cursor while drawing a bounding box. */
  double m_Lon;

private:
  void HighlightArea(double latmax, double lonmax, double latmin,
                     double lonmin);
  void ReadLocalCatalog();
  void FillTreeCtrl(wxJSONValue &data);
  void ApplyRequestConfig(unsigned rs, unsigned it, unsigned tr);
  wxString WriteMail();
  int EstimateFileSize(double *size);

  /** Save the "download" configuration to disk. */
  void SaveConfig() override;
  /** Load the "download" configuration from disk and initialize the dialog
   * widgets based on the configuration that was loaded from disk. */
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
  void OnOK(wxCommandEvent &event) override;
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
  /** Index of the canvas where the bounding box is drawn during manual zone
   * selection. */
  int m_boundingBoxCanvasIndex;
};

#endif
