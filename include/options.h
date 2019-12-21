/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Options Dialog
 * Author:   David Register
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
 **************************************************************************/

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#ifdef __MINGW32__
#undef IPV6STRICT    // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <wx/listbook.h>
#include <wx/dirctrl.h>
#include <wx/spinctrl.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/collpane.h>
#include <wx/clrpicker.h>
#include <wx/colourdata.h>

#if wxUSE_TIMEPICKCTRL
 #include <wx/timectrl.h>
#endif

#ifdef __WXGTK__
//wxTimePickerCtrl is completely broken in Gnome based desktop environments as of wxGTK 3.0
#include "time_textbox.h"
#endif

#include <vector>

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/frame.h>
#else
#include "scrollingdialog.h"
#endif

#include "datastream.h"
#include "pluginmanager.h"  // FIXME: Refactor

#ifndef __OCPN__ANDROID__
#define __OCPN__OPTIONS_USE_LISTBOOK__
#endif

// Forward Declarations
class wxGenericDirCtrl;
class MyConfig;
class ChartGroupsUI;
class ConnectionParams;
class PluginListPanel;
class ChartGroupArray;
class ChartGroup;
class MMSI_Props_Panel;
class MMSIProperties;
class OCPNCheckedListCtrl;
class CanvasConfigSelect;
class OCPNIconCombo;
class OCPNColourPickerCtrl;

#define ID_DIALOG 10001
#define SYMBOL_OPTIONS_STYLE \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
#define SYMBOL_OPTIONS_TITLE _("Options")
#define SYMBOL_OPTIONS_IDNAME ID_DIALOG
#define SYMBOL_OPTIONS_SIZE wxSize(500, 500)
#define SYMBOL_OPTIONS_POSITION wxDefaultPosition

enum {
  ID_AISALERTAUDIO = 10000,
  ID_AISALERTDIALOG,
  ID_AISALERTSELECTSOUND,
  ID_AISALERTTESTSOUND,
  ID_APPLY,
  ID_ATONTEXTCHECKBOX,
  ID_AUTOANCHORMARKBOX1,
  ID_BELLSCHECKBOX,
  ID_BUTTONADD,
  ID_BUTTONDELETE,
  ID_BUTTONCOMPRESS,
  ID_BUTTONFONTCHOOSE,
  ID_BUTTONFONTCOLOR,
  ID_BUTTONGROUP,
  ID_BUTTONREBUILD,
  ID_BUTTONTCDATA,
  ID_CHECKLISTBOX,
  ID_CHECK_DISPLAYGRID,
  ID_CHECK_LOOKAHEAD,
  ID_CHOICE_AIS,
  ID_CHOICE_AP,
  ID_CHOICE_FONTELEMENT,
  ID_CHOICE_LANG,
  ID_CHOICE_NMEA_BAUD,
  ID_CLEARLIST,
  ID_CM93CHECKBOX1,
  ID_CM93ZOOM,
  ID_COURSEUPCHECKBOX,
  ID_DAILYCHECKBOX,
  ID_DEBUGCHECKBOX1,
  ID_DECLTEXTCHECKBOX,
  ID_DIRCTRL,
  ID_DRAGGINGCHECKBOX,
  ID_FILTERNMEA,
  ID_FSTOOLBARCHECKBOX,
  ID_FULLSCREENQUILT,
  ID_GARMINHOST,
  ID_RADARRINGS,
  ID_GRIBCHECKBOX,
  ID_GROUPAVAILABLE,
  ID_GROUPDELETEGROUP,
  ID_GROUPINSERTDIR,
  ID_GROUPNEWGROUP,
  ID_GROUPNOTEBOOK,
  ID_GROUPREMOVEDIR,
  ID_IMPTEXTCHECKBOX,
  ID_LDISTEXTCHECKBOX,
  ID_LISTBOX,
  ID_METACHECKBOX,
  ID_NOTEBOOK,
  ID_OPENGLBOX,
  ID_OPENGLOPTIONS,
  ID_SHIPICONTYPE,
  ID_OUTLINECHECKBOX1,
  ID_PANEL,
  ID_PANEL2,
  ID_PANEL3,
  ID_PANELADVANCED,
  ID_PANELAIS,
  ID_PANELMMSI,
  ID_PANELFONT,
  ID_PANELPIM,
  ID_PRESERVECHECKBOX,
  ID_PRINTCHECKBOX1,
  ID_QUILTCHECKBOX1,
  ID_RADARDISTUNIT,
  ID_RASTERCHECKBOX1,
  ID_SCAMINCHECKBOX,
  ID_SCANCHECKBOX,
  ID_SDMMFORMATCHOICE,
  ID_DISTANCEUNITSCHOICE,
  ID_SPEEDUNITSCHOICE,
  ID_DEPTHUNITSCHOICE,
  ID_SELECTLIST,
  ID_SHOWDEPTHUNITSBOX1,
  ID_SHOWGPSWINDOW,
  ID_SKEWCOMPBOX,
  ID_SMOOTHPANZOOMBOX,
  ID_SOUNDGCHECKBOX,
  ID_STYLESCOMBOBOX,
  ID_TCDATAADD,
  ID_TCDATADEL,
  ID_TEXTCHECKBOX,
  ID_OPTEXTCTRL,
  ID_TIDESELECTED,
  ID_TRACKCHECKBOX,
  ID_TRACKHILITE,
  ID_TRANSTOOLBARCHECKBOX,
  ID_UPDCHECKBOX,
  ID_PARSEENCBUTTON,
  ID_VECTORCHECKBOX1,
  ID_ZTCCHECKBOX,
  ID_DELETECHECKBOX,
  ID_NATIONALTEXTCHECKBOX,
  ID_TRUESHOWCHECKBOX,
  ID_MAGSHOWCHECKBOX,
  ID_MAGAPBCHECKBOX,
  ID_MOBILEBOX,
  ID_REPONSIVEBOX,
  ID_SIZEMANUALRADIOBUTTON,
  ID_OPWAYPOINTRANGERINGS,
  xID_OK,
  ID_BT_SCANTIMER,
  ID_TRACKROTATETIME,
  ID_TRACKROTATEUTC,
  ID_TRACKROTATELMT,
  ID_TRACKROTATECOMPUTER,
  ID_SETSTDLIST,
  ID_VECZOOM,
  ID_INLANDECDISBOX,
  ID_ROLLOVERBOX,
  ID_SOGCOGFROMLLCHECKBOX,
  ID_SOGCOGDAMPINTTEXTCTRL,
  // LIVE ETA OPTION
  ID_CHECK_LIVEETA,
  ID_DEFAULT_BOAT_SPEED,
  ID_DARKDECORATIONSBOX,
  ID_SCREENCONFIG1,
  ID_SCREENCONFIG2,
  ID_CONFIGEDIT_OK,
  ID_CONFIGEDIT_CANCEL,
  ID_ZOOMBUTTONS
};

/* Define an int bit field for dialog return value
 * to indicate which types of settings have changed */
#define GENERIC_CHANGED 1
#define S52_CHANGED 2
#define FONT_CHANGED 4
#define FORCE_UPDATE 8
#define VISIT_CHARTS 16
#define LOCALE_CHANGED 32
#define TOOLBAR_CHANGED 64
#define CHANGE_CHARTS 128
#define SCAN_UPDATE 256
#define GROUPS_CHANGED 512
#define STYLE_CHANGED 1024
#define TIDES_CHANGED 2048
#define GL_CHANGED 4096
#define REBUILD_RASTER_CACHE 8192
#define NEED_NEW_OPTIONS 16384
#define PARSE_ENC 32768
#define CONFIG_CHANGED 8192 * 2

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_ARRAY_PTR(wxGenericDirCtrl *, ArrayOfDirCtrls);

class Uncopyable {
 protected:
  Uncopyable(void) {}
  ~Uncopyable(void) {}

 private:
  Uncopyable(const Uncopyable &);
  Uncopyable &operator=(const Uncopyable &);
};

#ifndef bert  // wxCHECK_VERSION(2, 9, 0)
class options : private Uncopyable,
                public wxDialog
#else
class options : private Uncopyable,
                public wxScrollingDialog
#endif
                {
 public:
  explicit options(MyFrame *parent, wxWindowID id = SYMBOL_OPTIONS_IDNAME,
                   const wxString &caption = SYMBOL_OPTIONS_TITLE,
                   const wxPoint &pos = SYMBOL_OPTIONS_POSITION,
                   const wxSize &size = SYMBOL_OPTIONS_SIZE,
                   long style = SYMBOL_OPTIONS_STYLE);

  ~options(void);
#if wxCHECK_VERSION(3,0,0)
  bool SendIdleEvents(wxIdleEvent &event );
#endif  
  void SetInitialPage(int page_sel, int sub_page = -1);
  void Finish(void);

  void OnClose(wxCloseEvent &event);

  void CreateListbookIcons();
  void CreateControls(void);
  size_t CreatePanel(const wxString &title);
  wxScrolledWindow *AddPage(size_t parent, const wxString &title);
  bool DeletePluginPage(wxScrolledWindow *page);
  void SetColorScheme(ColorScheme cs);
  void RecalculateSize(void);

  void SetInitChartDir(const wxString &dir) { m_init_chart_dir = dir; }
  void SetInitialSettings(void);
  void SetInitialVectorSettings(void);
  
  void SetCurrentDirList(ArrayOfCDI p) { m_CurrentDirList = p; }
  void SetWorkDirListPtr(ArrayOfCDI *p) { m_pWorkDirList = p; }
  ArrayOfCDI *GetWorkDirListPtr(void) { return m_pWorkDirList; }

  void AddChartDir(const wxString &dir);

  void UpdateDisplayedChartDirList(ArrayOfCDI p);
  void UpdateOptionsUnits(void);

  void SetConfigPtr(MyConfig *p) { m_pConfig = p; }
  void OnDebugcheckbox1Click(wxCommandEvent &event);
  void OnDirctrlSelChanged(wxTreeEvent &event);
  void OnButtonaddClick(wxCommandEvent &event);
  void OnButtondeleteClick(wxCommandEvent &event);
  void OnButtonParseENC(wxCommandEvent &event);
  void OnButtoncompressClick(wxCommandEvent &event);
  void OnRadioboxSelected(wxCommandEvent &event);
  void OnApplyClick(wxCommandEvent &event);
  void OnXidOkClick(wxCommandEvent &event);
  void OnCancelClick(wxCommandEvent &event);
  void OnChooseFont(wxCommandEvent &event);
  void OnFontChoice(wxCommandEvent &event);
  void OnCPAWarnClick(wxCommandEvent &event);
  void OnSizeAutoButton(wxCommandEvent &event);
  void OnSizeManualButton(wxCommandEvent &event);

#if defined(__WXGTK__) || defined(__WXQT__)
  void OnChooseFontColor(wxCommandEvent &event);
#endif
  void OnGLClicked(wxCommandEvent &event);
  void OnOpenGLOptions(wxCommandEvent &event);
  void OnDisplayCategoryRadioButton(wxCommandEvent &event);
  void OnButtonClearClick(wxCommandEvent &event);
  void OnButtonSelectClick(wxCommandEvent &event);
  void OnButtonSetStd(wxCommandEvent& event);
      
  void OnPageChange(wxListbookEvent &event);
  void OnNBPageChange(wxNotebookEvent &event);
  void DoOnPageChange(size_t page);

  void OnButtonSelectSound(wxCommandEvent &event);
  void OnButtonTestSound(wxCommandEvent &event);
  void OnShowGpsWindowCheckboxClick(wxCommandEvent &event);
  void OnZTCCheckboxClick(wxCommandEvent &event);
  void OnRadarringSelect(wxCommandEvent &event);
  void OnWaypointRangeRingSelect(wxCommandEvent &event);
  void OnShipTypeSelect(wxCommandEvent &event);
  void OnButtonGroups(wxCommandEvent &event);
  void OnInsertTideDataLocation(wxCommandEvent &event);
  void OnRemoveTideDataLocation(wxCommandEvent &event);
  void OnCharHook(wxKeyEvent &event);
  void OnChartsPageChange(wxListbookEvent &event);
  void OnChartDirListSelect(wxCommandEvent &event);
  void OnUnitsChoice(wxCommandEvent &event);
  void OnScanBTClick(wxCommandEvent &event);
  void onBTScanTimer(wxTimerEvent &event);
  void StopBTScan(void);

  void UpdateWorkArrayFromTextCtl(void);

  void OnCreateConfig( wxCommandEvent &event);
  void OnEditConfig( wxCommandEvent &event);
  void OnDeleteConfig( wxCommandEvent &event);
  void OnApplyConfig( wxCommandEvent &event);
  void SetConfigButtonState();
  void ClearConfigList();
  void BuildConfigList();
  void OnConfigMouseSelected( wxMouseEvent &event);

  void SetSelectedConnectionPanel( ConnectionParamsPanel *panel );
  
  // Should we show tooltips?
  static bool ShowToolTips(void);

#ifdef __OCPN__OPTIONS_USE_LISTBOOK__
  wxListbook *m_pListbook;
#else
  wxNotebook *m_pListbook;
#endif

  size_t m_pageDisplay, m_pageConnections, m_pageCharts, m_pageShips;
  size_t m_pageUI, m_pagePlugins;
  int lastPage, lastSubPage;
  wxPoint lastWindowPos;
  wxSize lastWindowSize;
  wxButton *m_ApplyButton, *m_OKButton, *m_CancelButton;

  ChartGroupArray *m_pGroupArray;
  int m_groups_changed;

  // Sizer flags
  wxSizerFlags inputFlags, verticleInputFlags, labelFlags, groupInputFlags;
  wxSizerFlags groupLabelFlags, groupLabelFlagsHoriz;

  // For general options
  wxScrolledWindow *pDisplayPanel;
  wxCheckBox *pShowStatusBar, *pShowMenuBar, *pShowChartBar, *pShowCompassWin;
  wxCheckBox *pPrintShowIcon, *pCDOOutlines, *pSDepthUnits, *pSDisplayGrid;
  wxCheckBox *pAutoAnchorMark, *pCDOQuilting, *pCBRaster, *pCBVector;
  wxCheckBox *pCBCM93, *pCBLookAhead, *pSkewComp, *pOpenGL, *pSmoothPanZoom;
  wxCheckBox *pFullScreenQuilt, *pMobile, *pResponsive, *pOverzoomEmphasis;
//  wxCheckBox *pOZScaleVector, *pToolbarAutoHideCB, *pInlandEcdis, *pRollover;
  wxCheckBox *pOZScaleVector, *pToolbarAutoHideCB, *pInlandEcdis, *pRollover, *pDarkDecorations;
  wxCheckBox *pZoomButtons;
  wxTextCtrl *pCOGUPUpdateSecs, *m_pText_OSCOG_Predictor, *pScreenMM;
  wxTextCtrl *pToolbarHideSecs, *m_pText_OSHDT_Predictor;

  wxTextCtrl *pCmdSoundString;

  wxChoice *m_pShipIconType, *m_pcTCDatasets;
  wxSlider *m_pSlider_Zoom, *m_pSlider_GUI_Factor, *m_pSlider_Chart_Factor, *m_pSlider_Ship_Factor;
  wxSlider *m_pSlider_Zoom_Vector;
  // LIVE ETA OPTION
  wxCheckBox *pSLiveETA;
  wxTextCtrl *pSDefaultBoatSpeed;
  
  wxRadioButton *pCBCourseUp, *pCBNorthUp, *pRBSizeAuto, *pRBSizeManual;
  int k_tides;

  // For the GPS page
  wxButton *m_buttonAdd, *m_buttonRemove, *m_buttonScanBT, *m_btnInputStcList;
  wxButton *m_btnOutputStcList, *m_sdbSizerDlgButtonsOK;
  wxButton *m_sdbSizerDlgButtonsApply, *m_sdbSizerDlgButtonsCancel;
  wxStaticBoxSizer *sbSizerConnectionProps, *sbSizerInFilter;
  wxStaticBoxSizer *sbSizerOutFilter;
  wxRadioButton *m_rbTypeSerial, *m_rbTypeNet, *m_rbTypeInternalGPS;
  wxRadioButton *m_rbTypeInternalBT, *m_rbNetProtoTCP, *m_rbNetProtoUDP;
  wxRadioButton *m_rbNetProtoGPSD, *m_rbIAccept, *m_rbIIgnore, *m_rbOAccept;
  wxRadioButton *m_rbOIgnore;
  wxStaticText *m_stBTPairs, *m_stNetProto, *m_stNetAddr, *m_stNetPort;
  wxStaticText *m_stSerPort, *m_stSerBaudrate, *m_stSerProtocol;
  wxStaticText *m_stPriority, *m_stFilterSec, *m_stPrecision;
  wxStaticText *m_stTalkerIdText;
  wxStaticText *m_stNetComment, *m_stSerialComment;
  wxTextCtrl *m_tNetComment, *m_tSerialComment;
  wxStaticBox *m_sbConnEdit;
  wxChoice *m_choiceBTDataSources, *m_choiceBaudRate, *m_choiceSerialProtocol;
  wxChoice *m_choicePriority, *m_choicePrecision;
  wxScrolledWindow *m_scrollWinConnections; 
  wxBoxSizer *boxSizerConnections;
  ConnectionParams *mSelectedConnection;
  
  // For the Display\Units page
  wxStaticText* itemStaticTextUserVar;
  wxStaticText* itemStaticTextUserVar2;
  
  wxGridSizer *gSizerNetProps, *gSizerSerProps;
  wxTextCtrl *m_tNetAddress, *m_tNetPort, *m_tFilterSec, *m_tcInputStc;
  wxTextCtrl *m_tcOutputStc, *m_TalkerIdText;
  wxCheckBox *m_cbCheckCRC, *m_cbGarminHost, *m_cbGarminUploadHost;
  wxCheckBox *m_cbFurunoGP3X, *m_cbNMEADebug, *m_cbFilterSogCog, *m_cbInput;
  wxCheckBox *m_cbOutput, *m_cbAPBMagnetic;
  wxComboBox *m_comboPort;
  wxStdDialogButtonSizer *m_sdbSizerDlgButtons;
  wxButton *m_configDeleteButton, *m_configApplyButton;
  
  void OnSelectDatasource(wxListEvent &event);
  void OnAddDatasourceClick(wxCommandEvent &event);
  void OnRemoveDatasourceClick(wxCommandEvent &event);

  void OnTypeSerialSelected(wxCommandEvent &event);
  void OnTypeNetSelected(wxCommandEvent &event);
  void OnTypeGPSSelected(wxCommandEvent &event);
  void OnTypeBTSelected(wxCommandEvent &event);

  void OnNetProtocolSelected(wxCommandEvent &event);
  void OnBaudrateChoice(wxCommandEvent &event) { OnConnValChange(event); }
  void OnProtocolChoice(wxCommandEvent &event) { OnConnValChange(event); }
  void OnCrcCheck(wxCommandEvent &event) { OnValChange(event); }
  void OnRbAcceptInput(wxCommandEvent &event);
  void OnRbIgnoreInput(wxCommandEvent &event);
  void OnBtnIStcs(wxCommandEvent &event);
  void OnCbInput(wxCommandEvent &event);
  void OnCbOutput(wxCommandEvent &event);
  void OnRbOutput(wxCommandEvent &event);
  void OnBtnOStcs(wxCommandEvent &event);
  void OnConnValChange(wxCommandEvent &event);
  void OnValChange(wxCommandEvent &event);
  void OnUploadFormatChange(wxCommandEvent &event);
  void EnableConnection( ConnectionParams *conn, bool value);

  
  void OnCanvasConfigSelectClick( int ID, bool selected);
  
  bool connectionsaved;
  bool m_connection_enabled;

  bool b_haveWMM;
  bool b_oldhaveWMM;
  ColorScheme m_cs;
  
  // For "S57" page
  wxBoxSizer *vectorPanel;
  wxScrolledWindow *ps57Ctl;

// #if defined(__WXMSW__) || defined(__WXOSX__)
//   wxCheckListBox *ps57CtlListBox;
// #else
  OCPNCheckedListCtrl *ps57CtlListBox;
// #endif

  wxChoice *pDispCat, *pPointStyle, *pBoundStyle, *p24Color;
  wxButton *itemButtonClearList, *itemButtonSelectList, *itemButtonSetStd;
  wxCheckBox *pCheck_SOUNDG, *pCheck_META, *pCheck_SHOWIMPTEXT;
  wxCheckBox *pCheck_SCAMIN, *pCheck_ATONTEXT, *pCheck_LDISTEXT;
  wxCheckBox *pCheck_XLSECTTEXT, *pCheck_DECLTEXT, *pCheck_NATIONALTEXT;
  wxCheckBox *pSEnableCM93Offset;
  wxTextCtrl *m_ShallowCtl, *m_SafetyCtl, *m_DeepCtl;
  wxStaticText *m_depthUnitsShal, *m_depthUnitsSafe, *m_depthUnitsDeep;
  wxSlider *m_pSlider_CM93_Zoom;
  int k_vectorcharts;

  // For "Units" page
  wxChoice *pSDMMFormat, *pDistanceFormat, *pSpeedFormat, *pDepthUnitSelect;
  wxCheckBox *pCBTrueShow, *pCBMagShow;
  wxTextCtrl *pMagVar;

  // For "Charts" page
  wxStaticBoxSizer *activeSizer;
  wxBoxSizer *chartPanel;
  wxTextCtrl *pSelCtl;
  wxListBox *pActiveChartsList;
  wxStaticBox *itemActiveChartStaticBox;
  wxCheckBox *pUpdateCheckBox, *pScanCheckBox;
  wxButton *pParseENCButton;
  wxButton *m_removeBtn, *m_compressBtn;
  int k_charts;
  int m_nCharWidthMax;
  
  // For the "Charts->Display Options" page
  wxScrolledWindow *m_ChartDisplayPage;

  // For the "AIS" page
  wxCheckBox *m_pCheck_CPA_Max, *m_pCheck_CPA_Warn, *m_pCheck_CPA_WarnT;
  wxCheckBox *m_pCheck_Mark_Lost, *m_pCheck_Remove_Lost, *m_pCheck_Show_COG;
  wxCheckBox *m_pCheck_Show_Tracks, *m_pCheck_Hide_Moored, *m_pCheck_Scale_Priority;
  wxCheckBox *m_pCheck_AlertDialog, *m_pCheck_AlertAudio;
  wxCheckBox *m_pCheck_Alert_Moored, *m_pCheck_Rollover_Class;
  wxCheckBox *m_pCheck_Rollover_COG, *m_pCheck_Rollover_CPA;
  wxCheckBox *m_pCheck_Ack_Timout, *m_pCheck_Show_Area_Notices;
  wxCheckBox *m_pCheck_Draw_Target_Size, *m_pCheck_Show_Target_Name;
  wxCheckBox *m_pCheck_Wpl_Aprs, *m_pCheck_ShowAllCPA;
  wxTextCtrl *m_pText_CPA_Max, *m_pText_CPA_Warn, *m_pText_CPA_WarnT;
  wxTextCtrl *m_pText_Mark_Lost, *m_pText_Remove_Lost, *m_pText_COG_Predictor;
  wxTextCtrl *m_pText_Track_Length, *m_pText_Moored_Speed, *m_pText_Scale_Priority;
  wxTextCtrl *m_pText_ACK_Timeout, *m_pText_Show_Target_Name_Scale;

  // For Display->Configs page...
  wxScrolledWindow *m_DisplayConfigsPage;
  
  CanvasConfigSelect *m_sconfigSelect_single;
  CanvasConfigSelect *m_sconfigSelect_twovertical;
  
  // For Configuration Template panel
  wxScrolledWindow *m_scrollWinConfigList;
  wxStaticText *m_templateTitleText;
  wxStaticText *m_staticTextLastAppled;
  wxStaticBoxSizer *m_templateStatusBoxSizer;
  
  // For the ship page
  wxFlexGridSizer *realSizes;
  wxTextCtrl *m_pOSLength, *m_pOSWidth, *m_pOSGPSOffsetX, *m_pOSGPSOffsetY;
  wxTextCtrl *m_pOSMinSize, *m_pText_ACRadius;
  wxStaticBoxSizer *dispOptions, *dispWaypointOptions;
  wxScrolledWindow *itemPanelShip, *itemPanelRoutes;
  wxBoxSizer *ownShip, *Routes;

  OCPNIconCombo *pWaypointDefaultIconChoice;
  OCPNIconCombo *pRoutepointDefaultIconChoice;
  wxCheckBox    *pScaMinChckB, *pScaMinOverruleChckB;
  wxTextCtrl*   m_pText_ScaMin;
  
  // For the font page
  wxBoxSizer *m_itemBoxSizerFontPanel;
  wxChoice *m_itemFontElementListBox, *m_itemStyleListBox, *m_itemLangListBox;
  wxStaticText *m_textSample;
  bool m_bVisitLang;

  // For "AIS Options"
  wxComboBox *m_itemAISListBox;

  // For "PlugIns" Panel
  PluginListPanel *m_pPlugInCtrl;
  AddPluginPanel* m_AddPluginPanel;
  wxScrolledWindow *itemPanelPlugins;
  wxBoxSizer *itemBoxSizerPanelPlugins;
  wxFlexGridSizer *radarGrid, *waypointradarGrid;
  wxChoice *pNavAidRadarRingsNumberVisible, *pWaypointRangeRingsNumber;
  OCPNColourPickerCtrl *m_colourOwnshipRangeRingColour;
  wxChoice *m_itemRadarRingsUnits, *m_itemWaypointRangeRingsUnits;
  OCPNColourPickerCtrl *m_colourTrackLineColour;;
  wxChoice *pTrackPrecision;
  wxTextCtrl *pNavAidRadarRingsStep, *pWaypointRangeRingsStep;
  wxCheckBox *pSogCogFromLLCheckBox;
  wxSpinCtrl *pSogCogFromLLDampInterval;
  wxTextCtrl *m_pText_TP_Secs, *m_pText_TP_Dist;
  wxCheckBox *pWayPointPreventDragging, *pConfirmObjectDeletion;
  wxCheckBox *pEnableZoomToCursor, *pPreserveScale, *pPlayShipsBells;
  wxCheckBox *pTransparentToolbar;
  wxCheckBox *pAdvanceRouteWaypointOnArrivalOnly, *pTrackShowIcon;
  wxCheckBox *pTrackDaily, *pTrackHighlite;
#if wxCHECK_VERSION(2, 9, 0)
#if wxUSE_TIMEPICKCTRL  
#ifdef __WXGTK__
  TimeCtrl *pTrackRotateTime;
#else
  wxTimePickerCtrl *pTrackRotateTime;
#endif  
#endif
#endif  
  wxRadioButton *pTrackRotateComputerTime, *pTrackRotateUTC, *pTrackRotateLMT;
  OCPNColourPickerCtrl *m_colourWaypointRangeRingsColour;
  wxChoice *pSoundDeviceIndex;
  wxStaticText *stSoundDeviceIndex;
  
  wxArrayPtrVoid OBJLBoxArray;
  wxString m_init_chart_dir;
  wxArrayString *m_pSerialArray;

  ArrayOfCDI m_CurrentDirList, *m_pWorkDirList;
  MyConfig *m_pConfig;
  MyFrame *pParent;

  int k_plugins;

 private:
  void Init(void);
  void CreatePanel_MMSI(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_AIS(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_Ownship(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_NMEA(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_NMEA_Compact(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_ChartsLoad(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_VectorCharts(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_TidesCurrents(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_ChartGroups(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_Display(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_UI(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_Units(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_Advanced(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_Configs(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_Routes(size_t parent, int border_size, int group_item_spacing);

  void UpdateTemplateTitleText();
  void CheckDeviceAccess(wxString &path);
  int m_returnChanges;
  wxListBox *tcDataSelected;
  std::vector<int> marinersStdXref;
  ChartGroupsUI *groupsPanel;
  wxImageList *m_topImgList;

  wxScrolledWindow *m_pNMEAForm;
  void ShowNMEACommon(bool visible);

  void ShowNMEASerial(bool visible);
  void ShowNMEANet(bool visible);
  void ShowNMEAGPS(bool visible);
  void ShowNMEABT(bool visible);

  void SetNMEAFormToSerial(void);
  void SetNMEAFormToNet(void);
  void SetNMEAFormToGPS(void);
  void SetNMEAFormToBT(void);

  void ClearNMEAForm(void);
  bool m_bNMEAParams_shown;
  
  void resetMarStdList(bool bsetConfig, bool bsetStd);
  
  void SetConnectionParams(ConnectionParams *cp);
  void SetDefaultConnectionParams(void);
  void SetDSFormRWStates();
  void FillSourceList();
  void UpdateSourceList( bool bResort );
  bool SortSourceList(void);

  ConnectionParams *CreateConnectionParamsFromSelectedItem();
  ConnectionParams *UpdateConnectionParamsFromSelectedItem(ConnectionParams *pConnectionParams);

  int m_screenConfig;
  
  wxNotebookPage *m_groupsPage;
  wxFont  *dialogFont, *dialogFontPlus;
  wxFont smallFont;
//  wxFont *dialogFont;
  wxSize m_small_button_size;
  wxTimer m_BTScanTimer;
  wxArrayString m_BTscan_results;

  bool m_bcompact;
  int m_fontHeight, m_scrollRate, m_BTscanning, m_btNoChangeCounter;
  int m_btlastResultCount;
  bool m_bfontChanged;
  bool m_bVectorInit;
  
  wxBoxSizer *m_boxSizerConfigs;
  wxColour m_panelBackgroundUnselected;
  wxString m_selectedConfigPanelGUID;
  wxSize  m_colourPickerDefaultSize;

  wxSize m_sliderSize;
  
  DECLARE_EVENT_TABLE()
};

class CanvasConfigSelect: public wxPanel
{
public:
    CanvasConfigSelect( wxWindow *parent, options *parentOptions, int id, wxBitmap &bmp, 
                        const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize );
    ~CanvasConfigSelect();
    
    void OnMouseSelected( wxMouseEvent &event );
    void SetSelected( bool selected );
    void OnPaint( wxPaintEvent &event );
    
    bool GetSelected(){ return m_bSelected; }
    
private:
    options *m_parentOptions;
    bool m_bSelected;
    wxColour m_boxColour;
    wxBitmap m_bmpNormal;
    int m_borderWidth;
    
    DECLARE_EVENT_TABLE()
};

class ChartGroupsUI : private Uncopyable, public wxScrolledWindow {
 public:
  explicit ChartGroupsUI(wxWindow *parent);
  ~ChartGroupsUI(void);

  void CreatePanel(size_t parent, int border_size, int group_item_spacing);
  void CompletePanel(void);
  void SetDBDirs(ArrayOfCDI &array) { m_db_dirs = array; }
  void SetGroupArray(ChartGroupArray *pGroupArray) {
    m_pGroupArray = pGroupArray;
  }
  void SetInitialSettings(void);
  void CompleteInitialSettings(void);
  void PopulateTrees(void);
  void PopulateTreeCtrl(wxTreeCtrl *ptc, const wxArrayString &dir_array,
                        const wxColour &col, wxFont *pFont = NULL);
  void BuildNotebookPages(ChartGroupArray *pGroupArray);
  void EmptyChartGroupArray(ChartGroupArray *s);
  void ClearGroupPages();
  

  void OnNodeExpanded(wxTreeEvent &event);
  void OnAvailableSelection(wxTreeEvent &event);
  void OnInsertChartItem(wxCommandEvent &event);
  void OnRemoveChartItem(wxCommandEvent &event);
  void OnGroupPageChange(wxNotebookEvent &event);
  void OnNewGroup(wxCommandEvent &event);
  void OnDeleteGroup(wxCommandEvent &event);

  ChartGroupArray *CloneChartGroupArray(ChartGroupArray *s);
  wxTreeCtrl *AddEmptyGroupPage(const wxString &label);

  bool modified, m_UIcomplete, m_settingscomplete, m_treespopulated;

  wxScrolledWindow *m_panel;
  
 private:
  int FindGroupBranch(ChartGroup *pGroup, wxTreeCtrl *ptree, wxTreeItemId item,
                      wxString *pbranch_adder);

  wxWindow *pParent;
  wxFlexGridSizer *groupsSizer;
  wxButton *m_pAddButton, *m_pRemoveButton, *m_pNewGroupButton;
  wxButton *m_pDeleteGroupButton;
  wxGenericDirCtrl *allAvailableCtl, *defaultAllCtl;
  wxTreeCtrl *m_pActiveChartsTree, *lastSelectedCtl;
  wxTreeItemId lastDeletedItem;
  wxNotebook *m_GroupNB;
  wxFont *iFont;
  wxFont *dialogFont;

  ArrayOfCDI m_db_dirs;
  ArrayOfDirCtrls m_DirCtrlArray;
  ChartGroupArray *m_pGroupArray;

  int m_border_size, m_group_item_spacing, m_GroupSelectedPage;
  
  wxBoxSizer *m_topSizer;

  DECLARE_EVENT_TABLE()
};

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
static int lang_list[] = {
    wxLANGUAGE_DEFAULT, wxLANGUAGE_ABKHAZIAN, wxLANGUAGE_AFAR,
    wxLANGUAGE_AFRIKAANS, wxLANGUAGE_ALBANIAN, wxLANGUAGE_AMHARIC,
    wxLANGUAGE_ARABIC, wxLANGUAGE_ARABIC_ALGERIA, wxLANGUAGE_ARABIC_BAHRAIN,
    wxLANGUAGE_ARABIC_EGYPT, wxLANGUAGE_ARABIC_IRAQ, wxLANGUAGE_ARABIC_JORDAN,
    wxLANGUAGE_ARABIC_KUWAIT, wxLANGUAGE_ARABIC_LEBANON,
    wxLANGUAGE_ARABIC_LIBYA, wxLANGUAGE_ARABIC_MOROCCO, wxLANGUAGE_ARABIC_OMAN,
    wxLANGUAGE_ARABIC_QATAR, wxLANGUAGE_ARABIC_SAUDI_ARABIA,
    wxLANGUAGE_ARABIC_SUDAN, wxLANGUAGE_ARABIC_SYRIA, wxLANGUAGE_ARABIC_TUNISIA,
    //    wxLANGUAGE_ARABIC_UAE,
    wxLANGUAGE_ARABIC_YEMEN, wxLANGUAGE_ARMENIAN, wxLANGUAGE_ASSAMESE,
    wxLANGUAGE_AYMARA, wxLANGUAGE_AZERI, wxLANGUAGE_AZERI_CYRILLIC,
    wxLANGUAGE_AZERI_LATIN, wxLANGUAGE_BASHKIR, wxLANGUAGE_BASQUE,
    wxLANGUAGE_BELARUSIAN, wxLANGUAGE_BENGALI, wxLANGUAGE_BHUTANI,
    wxLANGUAGE_BIHARI, wxLANGUAGE_BISLAMA, wxLANGUAGE_BRETON,
    wxLANGUAGE_BULGARIAN, wxLANGUAGE_BURMESE, wxLANGUAGE_CAMBODIAN,
    wxLANGUAGE_CATALAN,
    //    wxLANGUAGE_CHINESE,
    //    wxLANGUAGE_CHINESE_SIMPLIFIED,
    //    wxLANGUAGE_CHINESE_TRADITIONAL,
    //    wxLANGUAGE_CHINESE_HONGKONG,
    //    wxLANGUAGE_CHINESE_MACAU,
    //    wxLANGUAGE_CHINESE_SINGAPORE,
    wxLANGUAGE_CHINESE_TAIWAN, wxLANGUAGE_CORSICAN, wxLANGUAGE_CROATIAN,
    wxLANGUAGE_CZECH, wxLANGUAGE_DANISH, wxLANGUAGE_DUTCH,
    wxLANGUAGE_DUTCH_BELGIAN, wxLANGUAGE_ENGLISH, wxLANGUAGE_ENGLISH_UK,
    wxLANGUAGE_ENGLISH_US, wxLANGUAGE_ENGLISH_AUSTRALIA,
    wxLANGUAGE_ENGLISH_BELIZE, wxLANGUAGE_ENGLISH_BOTSWANA,
    wxLANGUAGE_ENGLISH_CANADA, wxLANGUAGE_ENGLISH_CARIBBEAN,
    wxLANGUAGE_ENGLISH_DENMARK, wxLANGUAGE_ENGLISH_EIRE,
    wxLANGUAGE_ENGLISH_JAMAICA, wxLANGUAGE_ENGLISH_NEW_ZEALAND,
    wxLANGUAGE_ENGLISH_PHILIPPINES, wxLANGUAGE_ENGLISH_SOUTH_AFRICA,
    wxLANGUAGE_ENGLISH_TRINIDAD, wxLANGUAGE_ENGLISH_ZIMBABWE,
    wxLANGUAGE_ESPERANTO, wxLANGUAGE_ESTONIAN, wxLANGUAGE_FAEROESE,
    wxLANGUAGE_FARSI, wxLANGUAGE_FIJI, wxLANGUAGE_FINNISH, wxLANGUAGE_FRENCH,
    wxLANGUAGE_FRENCH_BELGIAN, wxLANGUAGE_FRENCH_CANADIAN,
    wxLANGUAGE_FRENCH_LUXEMBOURG, wxLANGUAGE_FRENCH_MONACO,
    wxLANGUAGE_FRENCH_SWISS, wxLANGUAGE_FRISIAN, wxLANGUAGE_GALICIAN,
    wxLANGUAGE_GEORGIAN, wxLANGUAGE_GERMAN, wxLANGUAGE_GERMAN_AUSTRIAN,
    wxLANGUAGE_GERMAN_BELGIUM, wxLANGUAGE_GERMAN_LIECHTENSTEIN,
    wxLANGUAGE_GERMAN_LUXEMBOURG, wxLANGUAGE_GERMAN_SWISS, wxLANGUAGE_GREEK,
    wxLANGUAGE_GREENLANDIC, wxLANGUAGE_GUARANI, wxLANGUAGE_GUJARATI,
    wxLANGUAGE_HAUSA, wxLANGUAGE_HEBREW, wxLANGUAGE_HINDI, wxLANGUAGE_HUNGARIAN,
    wxLANGUAGE_ICELANDIC, wxLANGUAGE_INDONESIAN, wxLANGUAGE_INTERLINGUA,
    wxLANGUAGE_INTERLINGUE, wxLANGUAGE_INUKTITUT, wxLANGUAGE_INUPIAK,
    wxLANGUAGE_IRISH, wxLANGUAGE_ITALIAN, wxLANGUAGE_ITALIAN_SWISS,
    wxLANGUAGE_JAPANESE, wxLANGUAGE_JAVANESE, wxLANGUAGE_KANNADA,
    wxLANGUAGE_KASHMIRI, wxLANGUAGE_KASHMIRI_INDIA, wxLANGUAGE_KAZAKH,
    wxLANGUAGE_KERNEWEK, wxLANGUAGE_KINYARWANDA, wxLANGUAGE_KIRGHIZ,
    wxLANGUAGE_KIRUNDI,
    //    wxLANGUAGE_KONKANI,
    wxLANGUAGE_KOREAN, wxLANGUAGE_KURDISH, wxLANGUAGE_LAOTHIAN,
    wxLANGUAGE_LATIN, wxLANGUAGE_LATVIAN, wxLANGUAGE_LINGALA,
    wxLANGUAGE_LITHUANIAN, wxLANGUAGE_MACEDONIAN, wxLANGUAGE_MALAGASY,
    wxLANGUAGE_MALAY, wxLANGUAGE_MALAYALAM, wxLANGUAGE_MALAY_BRUNEI_DARUSSALAM,
    wxLANGUAGE_MALAY_MALAYSIA, wxLANGUAGE_MALTESE,
    //    wxLANGUAGE_MANIPURI,
    wxLANGUAGE_MAORI, wxLANGUAGE_MARATHI, wxLANGUAGE_MOLDAVIAN,
    wxLANGUAGE_MONGOLIAN, wxLANGUAGE_NAURU, wxLANGUAGE_NEPALI,
    wxLANGUAGE_NEPALI_INDIA, wxLANGUAGE_NORWEGIAN_BOKMAL,
    wxLANGUAGE_NORWEGIAN_NYNORSK, wxLANGUAGE_OCCITAN, wxLANGUAGE_ORIYA,
    wxLANGUAGE_OROMO, wxLANGUAGE_PASHTO, wxLANGUAGE_POLISH,
    wxLANGUAGE_PORTUGUESE, wxLANGUAGE_PORTUGUESE_BRAZILIAN, wxLANGUAGE_PUNJABI,
    wxLANGUAGE_QUECHUA, wxLANGUAGE_RHAETO_ROMANCE, wxLANGUAGE_ROMANIAN,
    wxLANGUAGE_RUSSIAN, wxLANGUAGE_RUSSIAN_UKRAINE, wxLANGUAGE_SAMOAN,
    wxLANGUAGE_SANGHO, wxLANGUAGE_SANSKRIT, wxLANGUAGE_SCOTS_GAELIC,
    wxLANGUAGE_SERBIAN, wxLANGUAGE_SERBIAN_CYRILLIC, wxLANGUAGE_SERBIAN_LATIN,
    wxLANGUAGE_SERBO_CROATIAN, wxLANGUAGE_SESOTHO, wxLANGUAGE_SETSWANA,
    wxLANGUAGE_SHONA, wxLANGUAGE_SINDHI, wxLANGUAGE_SINHALESE,
    wxLANGUAGE_SISWATI, wxLANGUAGE_SLOVAK, wxLANGUAGE_SLOVENIAN,
    wxLANGUAGE_SOMALI, wxLANGUAGE_SPANISH, wxLANGUAGE_SPANISH_ARGENTINA,
    wxLANGUAGE_SPANISH_BOLIVIA, wxLANGUAGE_SPANISH_CHILE,
    wxLANGUAGE_SPANISH_COLOMBIA, wxLANGUAGE_SPANISH_COSTA_RICA,
    wxLANGUAGE_SPANISH_DOMINICAN_REPUBLIC, wxLANGUAGE_SPANISH_ECUADOR,
    wxLANGUAGE_SPANISH_EL_SALVADOR, wxLANGUAGE_SPANISH_GUATEMALA,
    wxLANGUAGE_SPANISH_HONDURAS, wxLANGUAGE_SPANISH_MEXICAN,
    //    wxLANGUAGE_SPANISH_MODERN,
    wxLANGUAGE_SPANISH_NICARAGUA, wxLANGUAGE_SPANISH_PANAMA,
    wxLANGUAGE_SPANISH_PARAGUAY, wxLANGUAGE_SPANISH_PERU,
    wxLANGUAGE_SPANISH_PUERTO_RICO, wxLANGUAGE_SPANISH_URUGUAY,
    wxLANGUAGE_SPANISH_US, wxLANGUAGE_SPANISH_VENEZUELA, wxLANGUAGE_SUNDANESE,
    wxLANGUAGE_SWAHILI, wxLANGUAGE_SWEDISH, wxLANGUAGE_SWEDISH_FINLAND,
    wxLANGUAGE_TAGALOG, wxLANGUAGE_TAJIK, wxLANGUAGE_TAMIL, wxLANGUAGE_TATAR,
    wxLANGUAGE_TELUGU, wxLANGUAGE_THAI, wxLANGUAGE_TIBETAN, wxLANGUAGE_TIGRINYA,
    wxLANGUAGE_TONGA, wxLANGUAGE_TSONGA, wxLANGUAGE_TURKISH, wxLANGUAGE_TURKMEN,
    wxLANGUAGE_TWI, wxLANGUAGE_UIGHUR, wxLANGUAGE_UKRAINIAN, wxLANGUAGE_URDU,
    wxLANGUAGE_URDU_INDIA, wxLANGUAGE_URDU_PAKISTAN, wxLANGUAGE_UZBEK,
    wxLANGUAGE_UZBEK_CYRILLIC, wxLANGUAGE_UZBEK_LATIN, wxLANGUAGE_VIETNAMESE,
    wxLANGUAGE_VOLAPUK, wxLANGUAGE_WELSH, wxLANGUAGE_WOLOF, wxLANGUAGE_XHOSA,
    wxLANGUAGE_YIDDISH, wxLANGUAGE_YORUBA, wxLANGUAGE_ZHUANG, wxLANGUAGE_ZULU};
#endif

class SentenceListDlg : private Uncopyable, public wxDialog {
 public:
  explicit SentenceListDlg(wxWindow *parent, FilterDirection dir, ListType type,
                           const wxArrayString &list);
  wxString GetSentences(void);

 private:
  void OnAddClick(wxCommandEvent &event);
  void OnDeleteClick(wxCommandEvent &event);
  void OnCLBSelect(wxCommandEvent &event);
  void OnCheckAllClick(wxCommandEvent &event);
  void OnClearAllClick(wxCommandEvent &event);

  void Populate(const wxArrayString &list);
  wxString GetBoxLabel(void) const;

  wxCheckListBox *m_clbSentences;
  wxButton *m_btnDel;

  ListType m_type;
  FilterDirection m_dir;
  wxArrayString m_sentences;
};

#ifdef ocpnUSE_GL
class OpenGLOptionsDlg : private Uncopyable, public wxDialog {
 public:
  explicit OpenGLOptionsDlg(wxWindow *parent);
  bool GetAcceleratedPanning(void) const;
  bool GetTextureCompression(void) const;
  bool GetPolygonSmoothing(void) const;
  bool GetLineSmoothing(void) const;
  bool GetShowFPS(void) const;
  bool GetSoftwareGL(void) const;
  bool GetTextureCompressionCaching(void) const;
  bool GetRebuildCache(void) const;
  int GetTextureMemorySize(void) const;

 private:
  void Populate(void);
  void OnButtonRebuild(wxCommandEvent &event);
  void OnButtonClear(wxCommandEvent &event);
  wxString GetTextureCacheSize(void);

  wxCheckBox *m_cbUseAcceleratedPanning, *m_cbTextureCompression;
  wxCheckBox *m_cbTextureCompressionCaching, *m_cbShowFPS, *m_cbSoftwareGL,
      *m_cbPolygonSmoothing, *m_cbLineSmoothing;
  wxSpinCtrl *m_sTextureDimension, *m_sTextureMemorySize;
  wxStaticText *m_cacheSize, *m_memorySize;

  bool m_brebuild_cache;

  DECLARE_EVENT_TABLE()
};
#endif

#define ID_MMSI_PROPS_LIST 10073

enum {
    mlMMSI = 0,
    mlTrackMode,
    mlIgnore,
    mlMOB,
    mlVDM,
    mlFollower,
    mlShipName
};  // MMSIListCtrl Columns;

class MMSIListCtrl : private Uncopyable, public wxListCtrl {
 public:
  explicit MMSIListCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                        const wxSize &size, long style);
  ~MMSIListCtrl(void);

  wxString OnGetItemText(long item, long column) const;
  void OnListItemClick(wxListEvent &event);
  void OnListItemActivated(wxListEvent &event);
  void OnListItemRightClick(wxListEvent &event);
  void PopupMenuHandler(wxCommandEvent &event);

  wxWindow *m_parent;
  int m_context_item;
  
  DECLARE_EVENT_TABLE()
};

#define ID_MMSIEDIT_OK 8191
#define ID_MMSIEDIT_CANCEL 8192
#define ID_MMSI_CTL 8193
#define ID_DEF_MENU_MMSI_EDIT 8194
#define ID_DEF_MENU_MMSI_DELETE 8195

class MMSIEditDialog : private Uncopyable, public wxDialog {
 public:
  explicit MMSIEditDialog(MMSIProperties *props, wxWindow *parent,
                          wxWindowID id = wxID_ANY,
                          const wxString &caption = wxEmptyString,
                          const wxPoint &pos = wxDefaultPosition,
                          const wxSize &size = wxDefaultSize, long style = 0);
  ~MMSIEditDialog(void);

  void SetColorScheme(ColorScheme cs);
  void CreateControls(void);
  void OnMMSIEditCancelClick(wxCommandEvent &event);
  void OnMMSIEditOKClick(wxCommandEvent &event);
  void OnCtlUpdated(wxCommandEvent &event);

  MMSIProperties *m_props;
  wxTextCtrl *m_MMSICtl, m_ShipNameCtl; //Has ToDo take away?
  wxRadioButton *m_rbTypeTrackDefault, *m_rbTypeTrackAlways;
  wxRadioButton *m_rbTypeTrackNever;
  wxCheckBox *m_cbTrackPersist, *m_IgnoreButton, *m_MOBButton, *m_VDMButton, *m_FollowerButton;
  wxButton *m_CancelButton, *m_OKButton;

private:
    void Persist();

  DECLARE_EVENT_TABLE()
};

class MMSI_Props_Panel : private Uncopyable, public wxPanel {
 public:
  explicit MMSI_Props_Panel(wxWindow *parent);
  ~MMSI_Props_Panel();

  void OnNewButton(wxCommandEvent &event);
  void SetColorScheme(ColorScheme cs);
  void UpdateMMSIList(void);

  MMSIListCtrl *m_pListCtrlMMSI;
  wxButton *m_pButtonNew;

 private:
  wxWindow *m_pparent;
};

class ConfigCreateDialog : private Uncopyable, public wxDialog
{
public:
    explicit ConfigCreateDialog(wxWindow *parent, wxWindowID id = wxID_ANY,
                            const wxString &caption = wxEmptyString,
                            const wxPoint &pos = wxDefaultPosition,
                            const wxSize &size = wxDefaultSize, long style = 0);
    ~ConfigCreateDialog(void);
    
    void SetColorScheme(ColorScheme cs);
    void CreateControls(void);
    void OnConfigEditCancelClick(wxCommandEvent& event);
    void OnConfigEditOKClick(wxCommandEvent& event);
    wxString GetCreatedTemplateGUID(){ return m_createdTemplateGUID; }
    
    wxTextCtrl *m_TitleCtl, *m_DescriptionCtl; 
    wxButton *m_CancelButton, *m_OKButton;
    wxString m_createdTemplateGUID;
    DECLARE_EVENT_TABLE()
};

#endif
// _OPTIONS_H_
