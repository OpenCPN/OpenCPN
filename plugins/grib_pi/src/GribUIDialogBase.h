///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  5 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////
/**
 * \file
 * Base User Interface Components for GRIB Plugin.
 *
 * Defines the foundational UI elements and structure for the GRIB plugin
 * including:
 * - Control bar for timeline navigation
 * - Data display controls and settings
 * - Request dialogs for downloading GRIB files
 * - Table views for detailed data inspection
 * - Settings and preferences panels
 *
 * Key Components:
 * - Timeline controls for forecast navigation
 * - Parameter selection and visualization options
 * - Geographic position tracking
 * - Animation playback controls
 * - File management interface
 * - Settings configuration panels
 * - Data table visualization
 *
 * This base UI framework provides the structure for all user interaction with
 * GRIB weather data, supporting both basic and advanced usage scenarios while
 * maintaining consistency across the interface.
 */
#ifndef __GRIBUIDIALOGBASE_H__
#define __GRIBUIDIALOGBASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/scrolwin.h>
#include <wx/radiobut.h>
#include <wx/statbmp.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/treectrl.h>
#include <wx/html/htmlwin.h>
#include <wx/tglbtn.h>
#include "CustomGrid.h"
#include "XyGribPanel.h"

///////////////////////////////////////////////////////////////////////////

// GRIBUICtrlBarBase
#define CONTROL_BAR 1000
#define ID_BTNPREV 1001
#define ID_CTRLTIME 1002
#define ID_BTNNEXT 1003
#define ID_CTRLALTITUDE 1004
#define ID_BTNNOW 1005
#define ID_BTNZOOMTC 1006
#define ID_BTNSHOWCDATA 1007
#define ID_BTNPLAY 1008
#define ID_TIMELINE 1009
#define ID_BTNOPENFILE 1010
#define ID_BTNSETTING 1011
#define ID_BTNREQUEST \
  1012  //!< ID of button for requesting/downloading GRIB data.
// GRIBUICDataBase
#define CURSOR_DATA 1013
#define ID_CB_WIND 1014
#define ID_CB_WIND_GUSTS 1015
#define ID_CB_PRESSURE 1016
#define ID_CB_WAVES 1017
#define ID_CB_CURRENT 1018
#define ID_CB_RAINFALL 1019
#define ID_CB_CLOUD_COVER 1020
#define ID_CB_AIR_TEMP 1021
#define ID_CB_SEA_TEMP 1022
#define ID_CB_CAPE 1023
#define ID_CB_COMP_REFL 1024
// GribSettingsDialogBase
#define BARBFIXSPACING 1025
#define BARBMINSPACING 1026
#define DIRFIXSPACING 1027
#define DIRMINSPACING 1028
#define NUMFIXSPACING 1029
#define NUMMINSPACING 1030
#define AC0 1031
#define AC1 1032
#define NW0 1033
#define NW1 1034
#define ZC0 1035
#define ZC1 1036
#define SCD0 1037
#define SCD1 1038
#define PB0 1039
#define PB1 1040
#define TL0 1041
#define TL1 1042
#define OF0 1043
#define OF1 1044
#define STS0 1045
#define STS1 1046
#define RQ0 1047
#define RQ1 1048
// GribRequestDialogBase
#define MAXLAT 1049
#define MAXLON 1050
#define MINLAT 1051
#define MINLON 1052
#define AUTOSELECT 1053  //!< Automatic selection mode.
#define MANSELECT 1054   //!< Manual selection mode.
#define SAVEDZONE 1055   //!< Save zone.

///////////////////////////////////////////////////////////////////////////////
/// Class ProjectBoatPanel
///////////////////////////////////////////////////////////////////////////////
class ProjectBoatPanel : public wxPanel {
private:
protected:
  wxCheckBox* m_cbProjectPosition;
  wxStaticText* m_stCourse;
  wxTextCtrl* m_tCourse;
  wxStaticText* m_stSpeed;
  wxTextCtrl* m_tSpeed;
  wxStaticText* m_stSpeedUnit;

public:
  ProjectBoatPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxSize(580, 40),
                   long style = wxTAB_TRAVERSAL,
                   const wxString& name = wxEmptyString);

  ~ProjectBoatPanel();

  double GetCourse();
  double GetSpeed();
  bool ProjectionEnabled();
  void SetCourse(const double course) {
    m_tCourse->SetValue(!wxIsNaN(course) ? wxString::FromDouble(course)
                                         : "0.0");
  };
  void SetSpeed(const double speed) {
    m_tSpeed->SetValue(!wxIsNaN(speed) ? wxString::FromDouble(speed) : "6.0");
  };
  void EnableProjection(bool enabled) {
    m_cbProjectPosition->SetValue(enabled);
  };
};

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBUICtrlBarBase
///////////////////////////////////////////////////////////////////////////////
class GRIBUICtrlBarBase : public wxDialog {
private:
protected:
  wxFlexGridSizer* m_fgCtrlBarSizer;
  wxBitmapButton* m_bpPrev;
  wxChoice* m_cRecordForecast;
  wxBitmapButton* m_bpNext;
  wxBitmapButton* m_bpNow;
  wxBitmapButton* m_bpZoomToCenter;
  wxBitmapButton* m_bpShowCursorData;
  wxBitmapButton* m_bpPlay;
  wxBitmapButton* m_bpOpenFile;
  wxBitmapButton* m_bpSettings;
  wxBitmapButton* m_bpRequest;
  wxFlexGridSizer* m_fgCDataSizer;
  wxFlexGridSizer* m_fgCtrlGrabberSize;
  ProjectBoatPanel* m_ProjectBoatPanel;
  double m_ScaledFactor;

  // Virtual event handlers, overide them in your derived class
  virtual void OnClose(wxCloseEvent& event) { event.Skip(); }
  virtual void OnMouseEvent(wxMouseEvent& event) { event.Skip(); }
  virtual void OnPaint(wxPaintEvent& event) { event.Skip(); }
  virtual void OnSize(wxSizeEvent& event) { event.Skip(); }
  virtual void OnPrev(wxCommandEvent& event) { event.Skip(); }
  virtual void OnRecordForecast(wxCommandEvent& event) { event.Skip(); }
  virtual void OnNext(wxCommandEvent& event) { event.Skip(); }
  virtual void OnAltitude(wxCommandEvent& event) { event.Skip(); }
  virtual void OnNow(wxCommandEvent& event) { event.Skip(); }
  virtual void OnZoomToCenterClick(wxCommandEvent& event) { event.Skip(); }
  virtual void OnShowCursorData(wxCommandEvent& event) { event.Skip(); }
  virtual void OnPlayStop(wxCommandEvent& event) { event.Skip(); }
  virtual void OnTimeline(wxScrollEvent& event) { event.Skip(); }
  virtual void OnOpenFile(wxCommandEvent& event) { event.Skip(); }
  virtual void OnSettings(wxCommandEvent& event) { event.Skip(); }
  virtual void OnRequestForecastData(wxCommandEvent& event) { event.Skip(); }
  virtual void OnCompositeDialog(wxCommandEvent& event) { event.Skip(); }

public:
  wxBitmapButton* m_bpAltitude;
  wxSlider* m_sTimeline;

  GRIBUICtrlBarBase(wxWindow* parent, wxWindowID id = CONTROL_BAR,
                    const wxString& title = wxEmptyString,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxDEFAULT_DIALOG_STYLE | wxSYSTEM_MENU,
                    double scale_factor = 1.0);
  ~GRIBUICtrlBarBase();

  void SetScaleFactor(double factor) { m_ScaledFactor = factor; }
  double GetScaleFactor() { return m_ScaledFactor; }
  static wxBitmap GetScaledBitmap(wxBitmap bitmap, const wxString svgFileName,
                                  double scale_factor);
};

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBUICDataBase
///////////////////////////////////////////////////////////////////////////////
class GRIBUICDataBase : public wxDialog {
private:
protected:
  wxFlexGridSizer* m_fgCdataSizer;

public:
  GRIBUICDataBase(wxWindow* parent, wxWindowID id = CURSOR_DATA,
                  const wxString& title = _("GRIB Display Control"),
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxSize(-1, -1),
                  long style = wxSYSTEM_MENU | wxNO_BORDER);
  ~GRIBUICDataBase();
};

///////////////////////////////////////////////////////////////////////////////
/// Class CursorDataBase
///////////////////////////////////////////////////////////////////////////////
class CursorDataBase : public wxPanel {
private:
protected:
  wxFlexGridSizer* fgSizer30;
  wxStaticText* m_stTrackingText;
  wxFlexGridSizer* m_fgTrackingControls;
  wxTextCtrl* m_tcWindSpeed;
  wxTextCtrl* m_tcWindSpeedBf;
  wxTextCtrl* m_tcWindDirection;
  wxTextCtrl* m_tcWindGust;
  wxTextCtrl* m_tcPressure;
  wxTextCtrl* m_tcWaveHeight;
  wxTextCtrl* m_tcWavePeriode;
  wxTextCtrl* m_tcWaveDirection;
  wxTextCtrl* m_tcCurrentVelocity;
  wxTextCtrl* m_tcCurrentDirection;
  wxTextCtrl* m_tcPrecipitation;
  wxTextCtrl* m_tcCloud;
  wxTextCtrl* m_tcAirTemperature;
  wxTextCtrl* m_tcSeaTemperature;
  wxTextCtrl* m_tcCAPE;
  wxTextCtrl* m_tcReflC;
  wxCheckBox* m_cbAltitude;
  wxTextCtrl* m_tcAltitude;
  wxCheckBox* m_cbTemp;
  wxTextCtrl* m_tcTemp;
  wxCheckBox* m_cbRelHumid;
  wxTextCtrl* m_tcRelHumid;

  // Virtual event handlers, overide them in your derived class
  virtual void OnMouseEvent(wxMouseEvent& event) { event.Skip(); }
  virtual void OnCBAny(wxCommandEvent& event) { event.Skip(); }
  virtual void OnMenuCallBack(wxMouseEvent& event) { event.Skip(); }

public:
  wxCheckBox* m_cbWind;
  wxCheckBox* m_cbWindGust;
  wxCheckBox* m_cbPressure;
  wxCheckBox* m_cbWave;
  wxCheckBox* m_cbCurrent;
  wxCheckBox* m_cbPrecipitation;
  wxCheckBox* m_cbCloud;
  wxCheckBox* m_cbAirTemperature;
  wxCheckBox* m_cbSeaTemperature;
  wxCheckBox* m_cbCAPE;
  wxCheckBox* m_cbReflC;

  CursorDataBase(wxWindow* parent, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxSize(-1, -1),
                 long style = wxNO_BORDER | wxTAB_TRAVERSAL);
  ~CursorDataBase();
};

///////////////////////////////////////////////////////////////////////////////
/// Class GribSettingsDialogBase
///////////////////////////////////////////////////////////////////////////////
class GribSettingsDialogBase : public wxDialog {
private:
protected:
  wxFlexGridSizer* m_fgSetDataSizer;
  wxChoice* m_cDataType;
  wxChoice* m_cDataUnits;
  wxCheckBox* m_cbBarbedArrows;
  wxFlexGridSizer* m_fgBarbedData1;
  wxChoice* m_cBarbedColours;
  /**
   * Flag to determine barbed arrow layout mode.
   *
   * When true, wind/current barbs are drawn using a fixed spacing grid where
   * arrows are evenly distributed across the screen at intervals specified
   * by m_iBarbArrSpacing. This mode creates a regular pattern of barbs.
   */
  wxCheckBox* m_cBarbArrFixSpac;
  wxCheckBox* m_cBarbArrMinSpac;
  wxFlexGridSizer* m_fgBarbedData2;
  wxCheckBox* m_cBarbedVisibility;
  /**
   * Spacing between barbed arrows in fixed spacing mode.
   *
   * This value defines the pixel distance between adjacent wind/current barbs
   * when using fixed spacing mode (m_bBarbArrFixSpac = true). The spacing
   * is measured in screen pixels, but is converted to geographic coordinates
   * during rendering to ensure proper movement when panning.
   *
   * Higher values create a sparser grid with fewer barbs, while lower values
   * create a denser grid with more barbs. The actual on-screen distance
   * includes this spacing plus the size of the barbed arrow itself.
   */
  wxSpinCtrl* m_sBarbArrSpacing;
  wxCheckBox* m_cbIsoBars;
  wxFlexGridSizer* m_fIsoBarSpacing;
  wxStaticText* m_tIsoBarSpacing;
  wxSpinCtrl* m_sIsoBarSpacing;
  wxFlexGridSizer* m_fIsoBarVisibility;
  wxCheckBox* m_sIsoBarVisibility;
  wxCheckBox* m_cbAbbrIsoBarsNumbers;
  wxCheckBox* m_cbDirectionArrows;
  wxFlexGridSizer* m_fgDirArrData1;
  wxChoice* m_cDirectionArrowForm;
  wxCheckBox* m_cDirArrFixSpac;
  wxCheckBox* m_cDirArrMinSpac;
  wxFlexGridSizer* m_fgDirArrData2;
  wxChoice* m_cDirectionArrowSize;
  wxSpinCtrl* m_sDirArrSpacing;
  wxCheckBox* m_cbOverlayMap;
  wxStaticText* m_tOverlayColors;
  wxChoice* m_cOverlayColors;
  wxCheckBox* m_cbNumbers;
  wxFlexGridSizer* m_fgNumData1;
  wxCheckBox* m_cNumFixSpac;
  wxCheckBox* m_cNumMinSpac;
  wxSpinCtrl* m_sNumbersSpacing;
  wxCheckBox* m_cbParticles;
  wxStaticText* m_ctParticles;
  wxSlider* m_sParticleDensity;
  wxStaticText* m_staticText24;
  wxSlider* m_sTransparency;
  wxFlexGridSizer* m_fgSetPlaybackSizer;
  wxStaticText* m_staticText26;
  wxChoice* m_cLoopStartPoint;
  wxSlider* m_sUpdatesPerSecond;
  wxStaticText* m_tSlicesPerUpdate;
  wxFlexGridSizer* m_fgSetGuiSizer;
  wxRadioButton* m_rbCurDataAttaWCap;
  wxRadioButton* m_rbCurDataAttaWoCap;
  wxRadioButton* m_rbCurDataIsolHoriz;
  wxRadioButton* m_rbCurDataIsolVertic;
  wxStaticBitmap* m_biAltitude;
  wxStaticBitmap* m_biNow;
  wxStaticBitmap* m_biZoomToCenter;
  wxStaticBitmap* m_biShowCursorData;
  wxStaticBitmap* m_biPlay;
  wxStaticBitmap* m_biTimeSlider;
  wxStaticBitmap* m_biOpenFile;
  wxStaticBitmap* m_biSettings;
  wxStaticBitmap* m_biRequest;

  // Virtual event handlers, overide them in your derived class
  virtual void OnPageChange(wxNotebookEvent& event) { event.Skip(); }
  virtual void OnDataTypeChoice(wxCommandEvent& event) { event.Skip(); }
  virtual void OnUnitChange(wxCommandEvent& event) { event.Skip(); }
  virtual void OnSpacingModeChange(wxCommandEvent& event) { event.Skip(); }
  virtual void OnTransparencyChange(wxScrollEvent& event) { event.Skip(); }
  virtual void OnIntepolateChange(wxCommandEvent& event) { event.Skip(); }
  virtual void OnCtrlandDataStyleChanged(wxCommandEvent& event) {
    event.Skip();
  }
  virtual void OnApply(wxCommandEvent& event) { event.Skip(); }

public:
  wxNotebook* m_nSettingsBook;
  wxCheckBox* m_cLoopMode;
  wxCheckBox* m_cInterpolate;
  wxChoice* m_sSlicesPerUpdate;
  wxStdDialogButtonSizer* m_sButton;
  wxButton* m_sButtonOK;
  wxButton* m_sButtonApply;
  wxButton* m_sButtonCancel;

  GribSettingsDialogBase(wxWindow* parent, wxWindowID id = wxID_ANY,
                         const wxString& title = _("Settings"),
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& size = wxSize(-1, -1),
                         long style = wxDEFAULT_DIALOG_STYLE);
  ~GribSettingsDialogBase();
};

///////////////////////////////////////////////////////////////////////////////
/// Class GribPreferencesDialogBase
///////////////////////////////////////////////////////////////////////////////
class GribPreferencesDialogBase : public wxDialog {
private:
protected:
  // Virtual event handlers, overide them in your derived class
  virtual void OnStartOptionChange(wxCommandEvent& event) { event.Skip(); }
  virtual void OnOKClick(wxCommandEvent& event) { event.Skip(); }
  void OnDirSelClick(wxCommandEvent& event);

public:
  wxCheckBox* m_cbUseHiDef;
  wxCheckBox* m_cbUseGradualColors;
  wxCheckBox* m_cbCopyFirstCumulativeRecord;
  wxCheckBox* m_cbCopyMissingWaveRecord;
  wxCheckBox* m_cbDrawBarbedArrowHead;
  wxCheckBox* m_cZoomToCenterAtInit;
  wxRadioBox* m_rbLoadOptions;
  wxRadioBox* m_rbStartOptions;
  wxString m_grib_dir_sel;
  wxTextCtrl* m_textDirectory;

#ifdef __WXMSW__
  wxSlider* m_sIconSizeFactor;
#endif

  GribPreferencesDialogBase(wxWindow* parent, wxWindowID id = wxID_ANY,
                            const wxString& title = _("Preferences"),
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            long style = wxDEFAULT_DIALOG_STYLE);
  ~GribPreferencesDialogBase();
};

///////////////////////////////////////////////////////////////////////////////
/// Class GribRequestSettingBase
///////////////////////////////////////////////////////////////////////////////
class GribRequestSettingBase : public wxDialog {
private:
  wxStaticBoxSizer* createAreaSelectionSection(wxWindow* parent,
                                               GRIBUICtrlBarBase* ctrlBar);
  void createWorldPanel();
  void createLocalModelsPanel();
  void createEmailPanel();

protected:
  wxNotebook* m_notebookGetGrib;
  wxPanel* m_panelWorld;
  wxHtmlWindow* m_htmlWinWorld;
  wxStaticText* m_staticTextInfo;
  wxStaticText* m_stForecastLength;
  wxChoice* m_chForecastLength;
  wxStaticText* m_stECMWFResolution;
  wxChoice* m_chECMWFResolution;
  wxButton* m_btnDownloadWorld;
  wxPanel* m_panelLocalModels;
  wxTreeCtrl* m_SourcesTreeCtrl1;
  wxStaticText* m_stLocalDownloadInfo;
  wxHtmlWindow* m_htmlInfoWin;
  wxButton* m_buttonUpdateCatalog;
  wxButton* m_btnDownloadLocal;
  wxPanel* m_panelEmail;
  wxFlexGridSizer* m_fgScrollSizer;
  wxFlexGridSizer* m_pSenderSizer;
  wxTextCtrl* m_pSenderAddress;
  wxChoice* m_pMailTo;
  wxChoice* m_pModel;
  wxCheckBox* m_cMovingGribEnabled;
  wxFlexGridSizer* m_fgMovingParams;
  wxSpinCtrl* m_sMovingSpeed;
  wxSpinCtrl* m_sMovingCourse;
  wxStaticText* m_sCourseUnit;
  wxFlexGridSizer* m_fgLog;
  wxTextCtrl* m_pLogin;
  wxTextCtrl* m_pCode;
  wxChoice* m_pResolution;
  wxStaticText* m_tResUnit;
  wxChoice* m_pInterval;
  wxChoice* m_pTimeRange;
  wxStaticText* m_staticText21;
  /**
   * Radio button selected to indicate the download area is based on
   * the visible area of the chart in the canvas which is currently
   * in focus.
   */
  wxRadioButton* m_rbCurrentView;
  /**
   * Radio button selected to indicate the download area is based on
   * the area selected by the user.
   */
  wxRadioButton* m_rbManualSelect;
  wxBitmapToggleButton* m_bpManualSelection;
  wxFlexGridSizer* fgZoneCoordinatesSizer;
  wxCheckBox* m_cUseSavedZone;
  /** A spinner for the max latitude of the bounding box for downloads. */
  wxSpinCtrl* m_spMaxLat;
  wxStaticText* m_stMaxLatNS;
  wxStaticText* m_staticText36;
  /** A spinner for the max longitude of the bounding box for downloads. */
  wxSpinCtrl* m_spMaxLon;
  wxStaticText* m_stMaxLonEW;
  /** A spinner for the min latitude of the bounding box for downloads. */
  wxSpinCtrl* m_spMinLat;
  wxStaticText* m_stMinLatNS;
  /** A spinner for the min longitude of the bounding box for downloads. */
  wxSpinCtrl* m_spMinLon;
  wxStaticText* m_stMinLonEW;
  wxCheckBox* m_pWind;
  wxCheckBox* m_pPress;
  wxCheckBox* m_pWindGust;
  wxCheckBox* m_pRainfall;
  wxCheckBox* m_pCloudCover;
  wxCheckBox* m_pAirTemp;
  wxCheckBox* m_pCAPE;
  wxCheckBox* m_pReflectivity;
  wxCheckBox* m_pSeaTemp;
  wxCheckBox* m_pCurrent;
  wxCheckBox* m_pWaves;
  wxChoice* m_pWModel;
  wxCheckBox* m_pAltitudeData;
  wxFlexGridSizer* m_fgAltitudeData;
  wxCheckBox* m_p850hpa;
  wxCheckBox* m_p700hpa;
  wxCheckBox* m_p500hpa;
  wxCheckBox* m_p300hpa;
  wxTextCtrl* m_MailImage;
  wxFlexGridSizer* m_fgFixedSizer;
  wxStaticText* m_tFileSize;
  wxStaticText* m_tLimit;
  /** Button to Send a download request through e-mail. */
  wxButton* m_rButtonYes;
  /** Button to Save the "download request" configuration. */
  wxButton* m_rButtonApply;
  /** Button to Cancel a request to download, close the dialog without saving
   * the configuration. */
  wxButton* m_rButtonCancel;
  XyGribPanel* m_xygribPanel;

  // Virtual event handlers, override them in your derived class
  virtual void OnClose(wxCloseEvent& event) { event.Skip(); }
  virtual void OnNotebookPageChanged(wxNotebookEvent& event) { event.Skip(); }
  virtual void OnWorldLengthChoice(wxCommandEvent& event) { event.Skip(); }
  virtual void OnWorldResolutionChoice(wxCommandEvent& event) { event.Skip(); }
  virtual void OnWorldDownload(wxCommandEvent& event) { event.Skip(); }
  virtual void OnLocalTreeItemExpanded(wxTreeEvent& event) { event.Skip(); }
  virtual void OnLocalTreeSelChanged(wxTreeEvent& event) { event.Skip(); }
  virtual void OnUpdateLocalCatalog(wxCommandEvent& event) { event.Skip(); }
  virtual void OnDownloadLocal(wxCommandEvent& event) { event.Skip(); }
  virtual void OnTopChange(wxCommandEvent& event) { event.Skip(); }
  virtual void OnMovingClick(wxCommandEvent& event) { event.Skip(); }
  virtual void OnAnySpinChange(wxSpinEvent& event) { event.Skip(); }
  virtual void OnAnyChange(wxCommandEvent& event) { event.Skip(); }
  virtual void OnTimeRangeChange(wxCommandEvent& event) { event.Skip(); }
  virtual void OnZoneSelectionModeChange(wxCommandEvent& event) {
    event.Skip();
  }
  virtual void OnCoordinatesChange(wxSpinEvent& event) { event.Skip(); }
  /**
   * Callback invoked when the user clicks the "OK" button in the "download"
   * dialog.
   */
  virtual void OnOK(wxCommandEvent& event) { event.Skip(); }
  /**
   * Callback invoked when the user clicks the "Cancel" button in the "download"
   * dialog.
   */
  virtual void OnCancel(wxCommandEvent& event) { event.Skip(); }
  /**
   * Callback invoked when the user clicks the "Send" button in the "e-mail"
   * tab.
   */
  virtual void OnSendMaiL(wxCommandEvent& event) { event.Skip(); }
  virtual void OnXyGribDownloadButton(wxCommandEvent& event) { event.Skip(); }
  virtual void OnXyGribAtmModelChoice(wxCommandEvent& event) { event.Skip(); }
  virtual void OnXyGribWaveModelChoice(wxCommandEvent& event) { event.Skip(); }
  virtual void OnXyGribConfigChange(wxCommandEvent& event) { event.Skip(); }
  // Save configuration before closing
  virtual void SaveConfig() {};

public:
  wxScrolledWindow* m_sScrolledDialog;

  GribRequestSettingBase(GRIBUICtrlBarBase* parent, wxWindowID id = wxID_ANY,
                         const wxString& title = _("Get forecast..."),
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& size = wxSize(-1, -1),
                         long style = wxDEFAULT_DIALOG_STYLE);
  ~GribRequestSettingBase();
};

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBTableBase
///////////////////////////////////////////////////////////////////////////////
class GRIBTableBase : public wxDialog {
private:
protected:
  wxStaticText* m_pPositionText;
  wxStaticText* m_pCursorPosition;

  // Virtual event handlers, overide them in your derived class
  virtual void OnClose(wxCloseEvent& event) { event.Skip(); }
  virtual void OnClick(wxGridEvent& event) { event.Skip(); }
  virtual void OnRangeClick(wxGridRangeSelectEvent& event) { event.Skip(); }
  virtual void OnOKButton(wxCommandEvent& event) { event.Skip(); }

public:
  CustomGrid* m_pGribTable;
  wxButton* m_pButtonTableOK;
  int m_pIndex;

  GRIBTableBase(wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxString& title = _("Grib Data Table"),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxSize(-1, -1),
                long style = wxDEFAULT_DIALOG_STYLE | wxMAXIMIZE_BOX |
                             wxRESIZE_BORDER);
  ~GRIBTableBase();
};

#endif  //__GRIBUIDIALOGBASE_H__
