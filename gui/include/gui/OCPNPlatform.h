/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Platform specific support utilities
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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

#ifndef OCPNPLATFORM_H
#define OCPNPLATFORM_H

#include <cstdio>
#include <string>
#include <vector>

// if wxWidgets headers have not been included, include them now
#ifndef _WX_DEFS_H_
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#endif  // _WX_DEFS_H_

#include <wx/bmpbuttn.h>
#include <wx/clrpicker.h>
#include <wx/colordlg.h>
#include <wx/colour.h>
#include <wx/colourdata.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/gdicmn.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/validate.h>
#include <wx/window.h>

#include "model/base_platform.h"

class MyConfig;
class ArrayOfCDI;

//--------------------------------------------------------------------------
//      Per-Platform Utility support
//--------------------------------------------------------------------------

// #ifdef __WXQT__
// extern bool LoadQtStyleSheet(wxString &sheet_file);
// extern QString getQtStyleSheet( void );
// #endif

/**
 * Provides platform-specific support utilities for OpenCPN. Extends
 * BasePlatform to implement platform-specific functionality for various aspects
 * of the application, including device support, initialization, display
 * handling, and file operations, and more.
 */
class OCPNPlatform : public BasePlatform {
public:
  OCPNPlatform();
  virtual ~OCPNPlatform();

  //      Internal Device Support
  static bool hasInternalGPS(wxString profile = _T(""));  // GPS

  static bool hasInternalBT(wxString profile = _T(""));  // Bluetooth
  bool startBluetoothScan();
  wxArrayString getBluetoothScanResults();
  bool stopBluetoothScan();

  //  Per-Platform initialization support

  //  Called from MyApp() immediately upon entry to MyApp::OnInit()
  static void Initialize_1(void);

  //  Called from MyApp() immediately before creation of MyFrame()
  void Initialize_2(void);

  //  Called from MyApp()::OnInit() just after gFrame is created, so gFrame is
  //  available
  void Initialize_3(void);

  //  Called from MyApp() just before end of MyApp::OnInit()
  static void Initialize_4(void);

  static void OnExit_1(void);
  static void OnExit_2(void);

  void SetDefaultOptions(void);
  void SetUpgradeOptions(wxString vString, wxString vStringConfig);

  void applyExpertMode(bool mode);

  //--------------------------------------------------------------------------
  //      Platform Display Support
  //--------------------------------------------------------------------------
  virtual void ShowBusySpinner(void);
  virtual void HideBusySpinner(void);
  double getFontPointsperPixel(void);
  /**
   * Get the display size in logical pixels.
   *
   * Returns the display dimensions in logical pixels, which may differ from
   * physical pixels on high-DPI displays. For example:
   * - On a MacBook Pro 16" Retina: returns 1792x1120 (logical) rather than
   * 4096x2560 (physical).
   * - On standard displays: logical pixels equal physical pixels.
   *
   * @note On Retina/HiDPI displays, multiply by GetContentScaleFactor() to get
   * physical pixels.
   * @return wxSize containing the width and height in logical pixels.
   */
  wxSize getDisplaySize();
  /**
   * Get the width of the screen in millimeters.
   */
  double GetDisplaySizeMM();
  double GetDisplayAreaCM2();
  virtual double GetDisplayDPmm();

  /**
   * Set the width of the monitor in millimeters.
   */
  void SetDisplaySizeMM(size_t monitor, double size);
  unsigned int GetSelectRadiusPix();
  double GetToolbarScaleFactor(int GUIScaleFactor);
  double GetCompassScaleFactor(int GUIScaleFactor);

  wxFileDialog *AdjustFileDialogFont(wxWindow *container, wxFileDialog *dlg);
  wxDirDialog *AdjustDirDialogFont(wxWindow *container, wxDirDialog *dlg);

  void PositionAISAlert(wxWindow *alert_window);
  float GetChartScaleFactorExp(float scale_linear);
  float GetMarkScaleFactorExp(float scale_linear);
  // float GetDIPScaleFactor();
  int GetStatusBarFieldCount();
  bool GetFullscreen();
  bool SetFullscreen(bool bFull);
  bool AllowAlertDialog(const wxString &class_name);
  double GetDisplayDensityFactor();
  double m_pt_per_pixel;
  long GetDefaultToolbarOrientation();

  //--------------------------------------------------------------------------
  //      Per-Platform file/directory support
  //--------------------------------------------------------------------------

  MyConfig *GetConfigObject();
  wxString GetSupplementalLicenseString();

  int DoFileSelectorDialog(wxWindow *parent, wxString *file_spec,
                           wxString Title, wxString initDir,
                           wxString suggestedName, wxString wildcard);
  int DoDirSelectorDialog(wxWindow *parent, wxString *file_spec, wxString Title,
                          wxString initDir, bool b_addFiles = true);

  //--------------------------------------------------------------------------
  //      Per-Platform Utility support
  //--------------------------------------------------------------------------
  void setChartTypeMaskSel(int mask, wxString &indicator);
  bool isPlatformCapable(int flag);

  int platformApplyPrivateSettingsString(wxString settings,
                                         ArrayOfCDI *pDirArray);
  void platformLaunchDefaultBrowser(wxString URL);

  void SetLocaleSearchPrefixes(void);
  wxString GetDefaultSystemLocale();

#if wxUSE_XLOCALE
  wxString GetAdjustedAppLocale();
  wxString ChangeLocale(wxString &newLocaleID, wxLocale *presentLocale,
                        wxLocale **newLocale);
#endif

  //--------------------------------------------------------------------------
  //      Per-Platform OpenGL support
  //--------------------------------------------------------------------------
  bool BuildGLCaps(void *pbuf);
  bool IsGLCapable();

private:
  wxString m_SData_Dir;
};

//      Private colourPicker control
//--------------------------------------------------------------------------

class OCPNColourPickerCtrl : public wxBitmapButton {
public:
  OCPNColourPickerCtrl();
  OCPNColourPickerCtrl(wxWindow *parent, wxWindowID id,
                       const wxColour &initial = *wxBLACK,
                       const wxPoint &pos = wxDefaultPosition,
                       const wxSize &size = wxDefaultSize, long style = 0,
                       const wxValidator &validator = wxDefaultValidator,
                       const wxString &name = _T(""));

  bool Create(wxWindow *parent, wxWindowID id,
              const wxColour &initial = *wxBLACK,
              const wxPoint &pos = wxDefaultPosition,
              const wxSize &size = wxDefaultSize, long style = 0,
              const wxValidator &validator = wxDefaultValidator,
              const wxString &name = _T(""));

  void OnButtonClick(wxCommandEvent &WXUNUSED(ev));
  void InitColourData();
  void SetColour(wxColour &c);
  wxColour GetColour(void);

protected:
  virtual void UpdateColour();
  wxSize DoGetBestSize() const;

  void OnPaint(wxPaintEvent &event);

  DECLARE_EVENT_TABLE();

private:
  wxBitmap m_bitmap;
  wxColour m_colour;
  wxColourData ms_data;
};

#endif  // guard
