/******************************************************************************
 * $Id: dashboard_pi.h, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 * expanded: Bernd Cirotzki 2023 (special colour design)
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
 ***************************************************************************
 */

#ifndef _DASHBOARDPI_H_
#define _DASHBOARDPI_H_

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#define PLUGIN_VERSION_MAJOR 1
#define PLUGIN_VERSION_MINOR 2

#define MY_API_VERSION_MAJOR 1
#define MY_API_VERSION_MINOR 8

#include <wx/notebook.h>
#include <wx/fileconf.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/fontpicker.h>
#include <wx/jsonval.h>

// wx2.9 #include <wx/wrapsizer.h>
#include "../../../include/ocpn_plugin.h"

#ifdef __OCPN__ANDROID__
#include <wx/qt/private/wxQtGesture.h>
#endif

#include "nmea0183/nmea0183.h"
#include "instrument.h"
#include "speedometer.h"
#include "compass.h"
#include "wind.h"
#include "rudder_angle.h"
#include "gps.h"
#include "depth.h"
#include "altitude.h"
#include "clock.h"
#include "wind_history.h"
#include "baro_history.h"
#include "from_ownship.h"
#include "iirfilter.h"
#include <wx/clrpicker.h>
#include <wx/statline.h>

#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif

class DashboardWindow;
class DashboardWindowContainer;
class DashboardInstrumentContainer;

// Request default positioning of toolbar tool
#define DASHBOARD_TOOL_POSITION -1
// Set watchdog to comply with OCPN WD.
#define gps_watchdog_timeout_ticks GetGlobalWatchdogTimoutSeconds()
// SignalK motor & environ instr defaults 30 sec update frequency
#define no_nav_watchdog_timeout_ticks 40
#define GEODESIC_RAD2DEG(r) ((r) * (180.0 / M_PI))
#define MS2KNOTS(r) ((r) * (1.9438444924406))
#define KELVIN2C(r) ((r) - (273.15))
#define PA2HPA(r) ((r) / (100))
#define METERS2NM(r) ((r) / (1852))

#define wxFontPickerCtrl OCPNFontButton
class OCPNFontButton;

WX_DEFINE_ARRAY(InstrumentProperties*, wxArrayOfInstrumentProperties);

class EditDialog : public wxDialog
{
private:

protected:
    wxStaticText* m_staticText1;
    wxStaticText* m_staticText5;
    wxStaticText* m_staticText2;
    wxStaticText* m_staticText6;
    wxStaticText* m_staticText3;
    wxStaticText* m_staticText4;
    wxStaticLine* m_staticline1;
    wxStaticLine* m_staticline2;
    wxStaticText* m_staticText7;
    wxStaticText* m_staticText9;
    wxStaticText* m_staticText10;
    wxStdDialogButtonSizer* m_sdbSizer3;
    wxButton* m_sdbSizer3OK;
    wxButton* m_sdbSizer3Cancel;

    // Virtual event handlers, override them in your derived class
    virtual void OnSetdefault(wxCommandEvent& event);

public:
    wxFontPickerCtrl* m_fontPicker2;
    wxColourPickerCtrl* m_colourPicker1;
    wxFontPickerCtrl* m_fontPicker4;
    wxColourPickerCtrl* m_colourPicker2;
    wxFontPickerCtrl* m_fontPicker5;
    wxFontPickerCtrl* m_fontPicker6;
    wxColourPickerCtrl* m_colourPicker3;
    wxColourPickerCtrl* m_colourPicker4;
    wxButton* m_button1;

    EditDialog(wxWindow* parent, InstrumentProperties& Properties, wxWindowID id = wxID_ANY, const wxString& title = wxT("Edit Instrument"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(-1, -1), long style = wxDEFAULT_DIALOG_STYLE);

    ~EditDialog();
};

class DashboardWindowContainer {
public:
  DashboardWindowContainer(DashboardWindow *dashboard_window, wxString name,
                           wxString caption, wxString orientation,
                           wxArrayInt inst, wxArrayOfInstrumentProperties inProberty) {
    m_pDashboardWindow = dashboard_window;
    m_sName = name;
    m_sCaption = caption;
    m_sOrientation = orientation;
    m_aInstrumentList = inst;
    m_aInstrumentPropertyList = inProberty;
    m_bIsVisible = false;
    m_bIsDeleted = false;
  }

  ~DashboardWindowContainer() {
      for (unsigned int i = 0; i < m_aInstrumentPropertyList.GetCount(); i++)
      {
          InstrumentProperties* Inst = m_aInstrumentPropertyList.Item(i);
          delete Inst;
      }
  }
  DashboardWindow *m_pDashboardWindow;
  bool m_bIsVisible;
  bool m_bIsDeleted;
  bool m_bPersVisible;  // Persists visibility, even when Dashboard tool is
                        // toggled off.
  wxString m_sName;
  wxString m_sCaption;
  wxString m_sOrientation;
  wxArrayInt m_aInstrumentList;
  wxArrayOfInstrumentProperties m_aInstrumentPropertyList;
};

class DashboardInstrumentContainer {
public:
  DashboardInstrumentContainer(int id, DashboardInstrument *instrument,
                               CapType capa) {
    m_ID = id;
    m_pInstrument = instrument;
    m_cap_flag = capa;
  }
  ~DashboardInstrumentContainer() { delete m_pInstrument; }

  DashboardInstrument *m_pInstrument;
  int m_ID;
  CapType m_cap_flag;
};

//    Dynamic arrays of pointers need explicit macros in wx261
#ifdef __WX261
WX_DEFINE_ARRAY_PTR(DashboardWindowContainer *, wxArrayOfDashboard);
WX_DEFINE_ARRAY_PTR(DashboardInstrumentContainer *, wxArrayOfInstrument);
#else
WX_DEFINE_ARRAY(DashboardWindowContainer *, wxArrayOfDashboard);
WX_DEFINE_ARRAY(DashboardInstrumentContainer *, wxArrayOfInstrument);
#endif

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

class dashboard_pi : public wxTimer, opencpn_plugin_18 {
public:
  dashboard_pi(void *ppimgr);
  ~dashboard_pi(void);

  //    The required PlugIn Methods
  int Init(void);
  bool DeInit(void);

  void Notify();

  int GetAPIVersionMajor();
  int GetAPIVersionMinor();
  int GetPlugInVersionMajor();
  int GetPlugInVersionMinor();
  wxBitmap *GetPlugInBitmap();
  wxString GetCommonName();
  wxString GetShortDescription();
  wxString GetLongDescription();

  //    The optional method overrides
  void SetNMEASentence(wxString &sentence);
  void SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix);
  void SetCursorLatLon(double lat, double lon);
  int GetToolbarToolCount(void);
  void OnToolbarToolCallback(int id);
  void ShowPreferencesDialog(wxWindow *parent);
  void SetColorScheme(PI_ColorScheme cs);
  void OnPaneClose(wxAuiManagerEvent &event);
  void UpdateAuiStatus(void);
  bool SaveConfig(void);
  void PopulateContextMenu(wxMenu *menu);
  void ShowDashboard(size_t id, bool visible);
  int GetToolbarItemId() { return m_toolbar_item_id; }
  int GetDashboardWindowShownCount();
  void SetPluginMessage(wxString &message_id, wxString &message_body);

private:
  bool LoadConfig(void);
  void LoadFont(wxFont **target, wxString native_info);

  void ApplyConfig(void);
  void SendSentenceToAllInstruments(DASH_CAP st, double value, wxString unit);
  void SendSatInfoToAllInstruments(int cnt, int seq, wxString talk,
                                   SAT_INFO sats[4]);
  void SendUtcTimeToAllInstruments(wxDateTime value);

  void CalculateAndUpdateTWDS(double awsKnots, double awaDegrees);

  void ParseSignalK(wxString &msg);
  void handleSKUpdate(wxJSONValue &update);
  void updateSKItem(wxJSONValue &item, wxString &talker, wxString &sfixtime);

  void HandleN2K_127245(ObservedEvt ev);
  void HandleN2K_127257(ObservedEvt ev);
  void HandleN2K_128259(ObservedEvt ev);
  void HandleN2K_128267(ObservedEvt ev);
  void HandleN2K_128275(ObservedEvt ev);
  void HandleN2K_129029(ObservedEvt ev);
  void HandleN2K_129540(ObservedEvt ev);
  void HandleN2K_130306(ObservedEvt ev);
  void HandleN2K_130310(ObservedEvt ev);
  void HandleN2K_130313(ObservedEvt ev);
  std::shared_ptr<ObservableListener> listener_127245;
  std::shared_ptr<ObservableListener> listener_127257;
  std::shared_ptr<ObservableListener> listener_128259;
  std::shared_ptr<ObservableListener> listener_128267;
  std::shared_ptr<ObservableListener> listener_128275;
  std::shared_ptr<ObservableListener> listener_129029;
  std::shared_ptr<ObservableListener> listener_129540;
  std::shared_ptr<ObservableListener> listener_130306;
  std::shared_ptr<ObservableListener> listener_130310;
  std::shared_ptr<ObservableListener> listener_130313;


  std::string prio127245;
  std::string prio127257;
  std::string prio128259;
  std::string prio128267;
  std::string prio129029;
  std::string prioN2kPGNsat;
  std::string prio130306;

  wxString m_self;

  wxFileConfig *m_pconfig;
  wxAuiManager *m_pauimgr;
  int m_toolbar_item_id;

  wxArrayOfDashboard m_ArrayOfDashboardWindow;
  int m_show_id;
  int m_hide_id;

  NMEA0183 m_NMEA0183;  // Used to parse NMEA Sentences
  short mPriPosition, mPriCOGSOG, mPriHeadingM, mPriHeadingT;
  short mPriVar, mPriDateTime, mPriAWA, mPriTWA, mPriDepth;
  short mPriSTW, mPriWTP, mPriATMP, mPriWDN, mPriSatStatus;
  short mPriMDA, mPriHUM;
  // Prio: Pos from O, SK gnss.satellites, GGA sats in use, SK gnss
  // satellitesinView, GSV sats in view
  short mPriSatUsed, mPriAlt, mPriRSA, mPriPitchRoll;
  double mVar;
  // FFU
  int mSatsInUse;
  int mSatsInView;
  double mHdm;
  wxDateTime mUTCDateTime;
  int m_config_version;
  wxString m_VDO_accumulator;
  int mHDx_Watchdog;
  int mHDT_Watchdog;
  int mSatsUsed_Wdog;
  int mSatStatus_Wdog;
  int mVar_Watchdog;
  int mMWVA_Watchdog;
  int mMWVT_Watchdog;
  int mDPT_DBT_Watchdog;
  int mSTW_Watchdog;
  int mWTP_Watchdog;
  int mRSA_Watchdog;
  int mVMG_Watchdog;
  int mVMGW_Watchdog;
  int mUTC_Watchdog;
  int mATMP_Watchdog;
  int mWDN_Watchdog;
  int mMDA_Watchdog;
  int mPITCH_Watchdog;
  int mHEEL_Watchdog;
  int mALT_Watchdog;
  int mLOG_Watchdog;
  int mTrLOG_Watchdog;
  int mHUM_Watchdog;

  iirfilter mSOGFilter;
  iirfilter mCOGFilter;
  // protected:
  //      DECLARE_EVENT_TABLE();
};

class DashboardPreferencesDialog : public wxDialog {
public:
  DashboardPreferencesDialog(wxWindow *pparent, wxWindowID id,
                             wxArrayOfDashboard config);
  ~DashboardPreferencesDialog() {}

  void OnCloseDialog(wxCloseEvent &event);
  void OnDashboardSelected(wxListEvent &event);
  void OnDashboardAdd(wxCommandEvent &event);
  void OnDashboardDelete(wxCommandEvent &event);
  void OnInstrumentSelected(wxListEvent &event);
  void OnInstrumentAdd(wxCommandEvent &event);
  void OnInstrumentEdit(wxCommandEvent &event);
  void OnInstrumentDelete(wxCommandEvent &event);
  void OnInstrumentUp(wxCommandEvent &event);
  void OnInstrumentDown(wxCommandEvent &event);
  void OnDashboarddefaultFont(wxCommandEvent& event);  
  void SaveDashboardConfig();
  void RecalculateSize(void);

  wxArrayOfDashboard m_Config;
  wxFontPickerCtrl *m_pFontPickerTitle;
  wxFontPickerCtrl *m_pFontPickerData;
  wxFontPickerCtrl *m_pFontPickerLabel;
  wxFontPickerCtrl *m_pFontPickerSmall;
  wxSpinCtrl *m_pSpinSpeedMax;
  wxSpinCtrl *m_pSpinCOGDamp;
  wxSpinCtrl *m_pSpinSOGDamp;
  wxChoice *m_pChoiceUTCOffset;
  wxChoice *m_pChoiceSpeedUnit;
  wxChoice *m_pChoiceDepthUnit;
  wxSpinCtrlDouble *m_pSpinDBTOffset;
  wxChoice *m_pChoiceDistanceUnit;
  wxChoice *m_pChoiceWindSpeedUnit;
  wxCheckBox *m_pUseTrueWinddata;
  wxChoice *m_pChoiceTempUnit;

private:
  void UpdateDashboardButtonsState(void);
  void UpdateButtonsState(void);
  int curSel;
  wxListCtrl *m_pListCtrlDashboards;
  wxBitmapButton *m_pButtonAddDashboard;
  wxBitmapButton *m_pButtonDeleteDashboard;
  wxPanel *m_pPanelDashboard;
  wxTextCtrl *m_pTextCtrlCaption;
  wxCheckBox *m_pCheckBoxIsVisible;
  wxChoice *m_pChoiceOrientation;
  wxListCtrl *m_pListCtrlInstruments;
  wxButton *m_pButtonAdd;
  wxButton *m_pButtonEdit;
  wxButton *m_pButtonDelete;
  wxButton *m_pButtonUp;
  wxButton *m_pButtonDown;
  wxButton *m_pButtondefaultFont;
};

class AddInstrumentDlg : public wxDialog {
public:
  AddInstrumentDlg(wxWindow *pparent, wxWindowID id);
  ~AddInstrumentDlg() {}

  unsigned int GetInstrumentAdded();

private:
  wxListCtrl *m_pListCtrlInstruments;
};

enum { ID_DASHBOARD_WINDOW };

enum {
  ID_DASH_PREFS = 999,
  ID_DASH_VERTICAL,
  ID_DASH_HORIZONTAL,
  ID_DASH_RESIZE,
  ID_DASH_UNDOCK
};

class DashboardWindow : public wxWindow {
public:
  DashboardWindow(wxWindow *pparent, wxWindowID id, wxAuiManager *auimgr,
                  dashboard_pi *plugin, int orient,
                  DashboardWindowContainer *mycont);
  ~DashboardWindow();

  void SetColorScheme(PI_ColorScheme cs);
  void SetSizerOrientation(int orient);
  int GetSizerOrientation();
  void OnSize(wxSizeEvent &evt);
  void OnContextMenu(wxContextMenuEvent &evt);
  void OnContextMenuSelect(wxCommandEvent &evt);

  void OnMouseEvent(wxMouseEvent &event);

#ifdef __OCPN__ANDROID__
  void OnEvtPinchGesture(wxQT_PinchGestureEvent &event);
  void OnEvtPanGesture(wxQT_PanGestureEvent &event);
#endif

  bool isInstrumentListEqual(const wxArrayInt &list);
  void SetInstrumentList(wxArrayInt list, wxArrayOfInstrumentProperties* InstrumentPropertyList);
  void SendSentenceToAllInstruments(DASH_CAP st, double value, wxString unit);
  void SendSatInfoToAllInstruments(int cnt, int seq, wxString talk,
                                   SAT_INFO sats[4]);
  void SendUtcTimeToAllInstruments(wxDateTime value);
  void ChangePaneOrientation(int orient, bool updateAUImgr);
  /*TODO: OnKeyPress pass event to main window or disable focus*/

  DashboardWindowContainer *m_Container;

  bool m_binPinch;
  bool m_binPan;

  wxPoint m_resizeStartPoint;
  wxSize m_resizeStartSize;
  bool m_binResize;
  bool m_binResize2;

private:
  wxAuiManager *m_pauimgr;
  dashboard_pi *m_plugin;

  // wx2.9      wxWrapSizer*          itemBoxSizer;
  wxBoxSizer *itemBoxSizer;
  wxArrayOfInstrument m_ArrayOfInstrument;

  wxButton *m_tButton;
};

#include "wx/button.h"
#include "wx/fontdata.h"

//-----------------------------------------------------------------------------
// OCPNFontButton: a button which brings up a wxFontDialog
//-----------------------------------------------------------------------------

class OCPNFontButton : public wxButton {
public:
  OCPNFontButton() {}
  OCPNFontButton(wxWindow *parent, wxWindowID id,
                 const wxFontData &initial,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 long style = wxFONTBTN_DEFAULT_STYLE,
                 const wxValidator &validator = wxDefaultValidator,
                 const wxString &name = wxFontPickerWidgetNameStr) {
    Create(parent, id, initial, pos, size, style, validator, name);
  }

  virtual wxColour GetSelectedColour() const { return m_data.GetColour(); }
  virtual void SetSelectedFont(const wxFont &font) {
      m_data.SetChosenFont(font);
      m_selectedFont = m_data.GetChosenFont();
      UpdateFont();
  }
  virtual void SetSelectedColour(const wxColour &colour) {
    m_data.SetColour(colour);
    UpdateFont();
  }

  virtual ~OCPNFontButton() {}

public:  // API extensions specific for OCPNFontButton
  // user can override this to init font data in a different way
  // virtual void InitFontData();

  // returns the font data shown in wxFontDialog
  wxFontData *GetFontData() { return &m_data; }

  // get the font chosen
  wxFont GetSelectedFont() const { return m_selectedFont; }

public:
  bool Create(wxWindow *parent, wxWindowID id,
              const wxFontData &initial,
              const wxPoint &pos = wxDefaultPosition,
              const wxSize &size = wxDefaultSize,
              long style = wxFONTBTN_DEFAULT_STYLE,
              const wxValidator &validator = wxDefaultValidator,
              const wxString &name = wxFontPickerWidgetNameStr);

  void OnButtonClick(wxCommandEvent &);

protected:
  void UpdateFont();

  wxFontData m_data;

  wxFont m_selectedFont;

private:
  DECLARE_DYNAMIC_CLASS(OCPNFontButton)
};

#endif
