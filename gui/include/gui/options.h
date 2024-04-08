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
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <memory>

#include <wx/listbook.h>
#include <wx/dirctrl.h>
#include <wx/spinctrl.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/collpane.h>
#include <wx/clrpicker.h>
#include <wx/colourdata.h>
#include "connections_dialog.h"

#if wxUSE_TIMEPICKCTRL
#include <wx/timectrl.h>
#endif

#ifdef __WXGTK__
// wxTimePickerCtrl is completely broken in Gnome based desktop environments as
// of wxGTK 3.0
#include "time_textbox.h"
#endif

#include <vector>

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/frame.h>
#else
#include "scrollingdialog.h"
#endif

#include "chartdbs.h"
#include "pluginmanager.h"  // FIXME: Refactor

#ifndef __OCPN__ANDROID__
#define __OCPN__OPTIONS_USE_LISTBOOK__
#endif

// Forward Declarations
class wxGenericDirCtrl;
class MyConfig;
class ChartGroupsUI;
//class ConnectionParams;
class PluginListPanel;
class ChartGroupArray;
class ChartGroup;
class MMSI_Props_Panel;
class MmsiProperties;
class OCPNCheckedListCtrl;
class CanvasConfigSelect;
class OCPNIconCombo;
class OCPNColourPickerCtrl;
class OCPNChartDirPanel;
class OCPNSoundPanel;

#define ID_DIALOG 10001
#define SYMBOL_OPTIONS_STYLE \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
#define SYMBOL_OPTIONS_TITLE _("Options")
#define SYMBOL_OPTIONS_IDNAME ID_DIALOG
#define SYMBOL_OPTIONS_SIZE wxSize(500, 500)
#define SYMBOL_OPTIONS_POSITION wxDefaultPosition

enum {
  ID_APPLY = 10000,
  ID_ATONTEXTCHECKBOX,
  ID_AUTOANCHORMARKBOX1,
  ID_BELLSCHECKBOX,
  ID_BUTTONADD,
  ID_BUTTONDELETE,
  ID_BUTTONCOMPRESS,
  ID_BUTTONFONTCHOOSE,
  ID_BUTTONECDISHELP,
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
  ID_SUPERSCAMINCHECKBOX,
  ID_SCANCHECKBOX,
  ID_SDMMFORMATCHOICE,
  ID_DISTANCEUNITSCHOICE,
  ID_SPEEDUNITSCHOICE,
  ID_WINDSPEEDUNITCHOICE,
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
  ID_SCREENCONFIG1,
  ID_SCREENCONFIG2,
  ID_CONFIGEDIT_OK,
  ID_CONFIGEDIT_CANCEL,
  ID_ZOOMBUTTONS,
  ID_RASTERZOOM,
  ID_ANCHORALERTAUDIO,
  ID_AISALERTAUDIO,
  ID_AISALERTDIALOG,
  ID_TEMPUNITSCHOICE,
  ID_BUTTONMIGRATE
};

/* Define an int bit field for dialog return value
 * to indicate which types of settings have changed */
#define GENERIC_CHANGED 1
#define S52_CHANGED 1 << 1
#define FONT_CHANGED 1 << 2
#define FORCE_UPDATE 1 << 3
#define VISIT_CHARTS 1 << 4
#define LOCALE_CHANGED 1 << 5
#define TOOLBAR_CHANGED 1 << 6
#define CHANGE_CHARTS 1 << 7
#define SCAN_UPDATE 1 << 8
#define GROUPS_CHANGED 1 << 9
#define STYLE_CHANGED 1 << 10
#define TIDES_CHANGED 1 << 11
#define GL_CHANGED 1 << 12
#define REBUILD_RASTER_CACHE 1 << 13
#define NEED_NEW_OPTIONS 1 << 14
#define PARSE_ENC 1 << 15
#define CONFIG_CHANGED 1 << 16

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
  explicit options(wxWindow *parent, wxWindowID id = SYMBOL_OPTIONS_IDNAME,
                   const wxString &caption = SYMBOL_OPTIONS_TITLE,
                   const wxPoint &pos = SYMBOL_OPTIONS_POSITION,
                   const wxSize &size = SYMBOL_OPTIONS_SIZE,
                   long style = SYMBOL_OPTIONS_STYLE);

  ~options(void);
#if wxCHECK_VERSION(3, 0, 0)
  bool SendIdleEvents(wxIdleEvent &event);
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
  void RecalculateSize(int hint_x, int hint_y);

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
  void OnButtonmigrateClick(wxCommandEvent &event);
  void OnButtonEcdisHelp(wxCommandEvent &event);
  void OnRadioboxSelected(wxCommandEvent &event);
  void OnApplyClick(wxCommandEvent &event);
  void OnXidOkClick(wxCommandEvent &event);
  void OnCancelClick(wxCommandEvent &event);
  void OnChooseFont(wxCommandEvent &event);
  void OnFontChoice(wxCommandEvent &event);
  void OnCPAWarnClick(wxCommandEvent &event);
  void OnSyncCogPredClick(wxCommandEvent &event);
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
  void OnButtonSetStd(wxCommandEvent &event);

  void OnPageChange(wxListbookEvent &event);
  void OnTopNBPageChange(wxNotebookEvent &event);
  void OnSubNBPageChange(wxNotebookEvent &event);
  void DoOnPageChange(size_t page);

  wxString SelectSoundFile();
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

  void UpdateWorkArrayFromDisplayPanel(void);
  ArrayOfCDI GetSelectedChartDirs();
  ArrayOfCDI GetUnSelectedChartDirs();
  void SetDirActionButtons();

  void OnCreateConfig(wxCommandEvent &event);
  void OnEditConfig(wxCommandEvent &event);
  void OnDeleteConfig(wxCommandEvent &event);
  void OnApplyConfig(wxCommandEvent &event);
  void SetConfigButtonState();
  void ClearConfigList();
  void BuildConfigList();
  void OnConfigMouseSelected(wxMouseEvent &event);
  void OnDialogInit(wxInitDialogEvent& event);


  bool GetNeedNew() { return m_bneedNew; }
  void SetNeedNew(bool bnew) { m_bneedNew = bnew; }
  int GetScrollRate() { return m_scrollRate; }
  void SetForceNewToolbarOnCancel(bool val) { m_bForceNewToolbaronCancel = val; }

  wxArrayString *GetSerialArray(){ return m_pSerialArray; }

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
  wxCheckBox *pOZScaleVector, *pToolbarAutoHideCB, *pInlandEcdis, *pRollover;
  wxCheckBox *pZoomButtons, *pChartBarEX;
  wxTextCtrl *pCOGUPUpdateSecs, *m_pText_OSCOG_Predictor, *pScreenMM;
  wxTextCtrl *pToolbarHideSecs, *m_pText_OSHDT_Predictor;

  wxTextCtrl *pCmdSoundString;

  wxChoice *m_pShipIconType, *m_pcTCDatasets;
  wxSlider *m_pSlider_Zoom_Raster, *m_pSlider_GUI_Factor, *m_pSlider_Chart_Factor,
      *m_pSlider_Ship_Factor, *m_pSlider_Text_Factor, *m_pSlider_ENCText_Factor;
  wxSlider *m_pMouse_Zoom_Slider;
  wxSlider *m_pSlider_Zoom_Vector;
  wxSlider *m_pSlider_CM93_Zoom;
  // LIVE ETA OPTION
  wxCheckBox *pSLiveETA;
  wxTextCtrl *pSDefaultBoatSpeed;

  wxRadioButton *pCBCourseUp, *pCBNorthUp, *pRBSizeAuto, *pRBSizeManual;
  int k_tides;

  // For the Display\Units page
  wxStaticText *itemStaticTextUserVar;
  wxStaticText *itemStaticTextUserVar2;
  wxButton *m_configDeleteButton, *m_configApplyButton;

  void OnAISRolloverClick(wxCommandEvent &event);
  void UpdateChartDirList();

  void OnCanvasConfigSelectClick(int ID, bool selected);


  bool b_haveWMM;
  bool b_oldhaveWMM;
  ColorScheme m_cs;

  // For "S57" page
  wxBoxSizer *vectorPanel;
  wxScrolledWindow *ps57Ctl;

  OCPNCheckedListCtrl *ps57CtlListBox;

  wxChoice *pDispCat, *pPointStyle, *pBoundStyle, *p24Color;
  wxButton *itemButtonClearList, *itemButtonSelectList, *itemButtonSetStd;
  wxCheckBox *pCheck_SOUNDG, *pCheck_META, *pCheck_SHOWIMPTEXT;
  wxCheckBox *pCheck_SCAMIN, *pCheck_SuperSCAMIN;
  wxCheckBox *pCheck_ATONTEXT, *pCheck_LDISTEXT;
  wxCheckBox *pCheck_XLSECTTEXT, *pCheck_DECLTEXT, *pCheck_NATIONALTEXT;
  wxCheckBox *pSEnableCM93Offset;
  wxTextCtrl *m_ShallowCtl, *m_SafetyCtl, *m_DeepCtl;
  wxStaticText *m_depthUnitsShal, *m_depthUnitsSafe, *m_depthUnitsDeep;
  int k_vectorcharts;

  // For "Units" page
  wxChoice *pSDMMFormat, *pDistanceFormat, *pSpeedFormat, *pDepthUnitSelect,
      *pTempFormat, *pWindSpeedFormat;
  wxCheckBox *pCBTrueShow, *pCBMagShow;
  wxTextCtrl *pMagVar;

  // For "Charts" page
  wxStaticBoxSizer *activeSizer;
  wxBoxSizer *chartPanel;
  wxTextCtrl *pSelCtl;

  ArrayOfCDI ActiveChartArray;

  wxStaticBox *itemActiveChartStaticBox;
  wxCheckBox *pUpdateCheckBox, *pScanCheckBox;
  wxButton *pParseENCButton;
  wxButton *m_removeBtn, *m_compressBtn;
  wxButton *m_migrateBtn;
  int k_charts;
  int m_nCharWidthMax;
  wxBoxSizer *boxSizerCharts;
  wxScrolledWindow *m_scrollWinChartList;
  wxScrolledWindow* chartPanelWin;
  wxBoxSizer* cmdButtonSizer;
  wxStaticBox* loadedBox;
  std::vector<OCPNChartDirPanel *> panelVector;
  wxArrayString activeChartList;

  // For the "Charts->Display Options" page
  wxScrolledWindow *m_ChartDisplayPage;

  // For the "AIS" page
  wxCheckBox *m_pCheck_CPA_Max, *m_pCheck_CPA_Warn, *m_pCheck_CPA_WarnT;
  wxCheckBox *m_pCheck_Mark_Lost, *m_pCheck_Remove_Lost;
  wxCheckBox *m_pCheck_Show_COG, *m_pCheck_Sync_OCOG_ACOG;
  wxCheckBox *m_pCheck_Show_Tracks, *m_pCheck_Hide_Moored,
      *m_pCheck_Scale_Priority;
  wxCheckBox *m_pCheck_AlertDialog, *m_pCheck_AlertAudio;
  wxCheckBox *m_pCheck_Alert_Moored, *m_pCheck_Rollover_Class;
  wxCheckBox *m_pCheck_Rollover_COG, *m_pCheck_Rollover_CPA;
  wxCheckBox *m_pCheck_Ack_Timout, *m_pCheck_Show_Area_Notices;
  wxCheckBox *m_pCheck_Draw_Target_Size, *m_pCheck_Draw_Realtime_Prediction;
  wxCheckBox *m_pCheck_Show_Target_Name;
  wxChoice *m_pWplAction;
  wxCheckBox *m_pCheck_use_Wpl, *m_pCheck_ShowAllCPA;
  wxTextCtrl *m_pText_CPA_Max, *m_pText_CPA_Warn, *m_pText_CPA_WarnT;
  wxTextCtrl *m_pText_Mark_Lost, *m_pText_Remove_Lost, *m_pText_COG_Predictor;
  wxTextCtrl *m_pText_Track_Length, *m_pText_Moored_Speed,
      *m_pText_Scale_Priority;
  wxTextCtrl *m_pText_ACK_Timeout, *m_pText_Show_Target_Name_Scale;
  wxTextCtrl *m_pText_RealtPred_Speed;

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
  wxCheckBox *pShowshipToActive;
  wxChoice *m_shipToActiveStyle, *m_shipToActiveColor;

  OCPNIconCombo *pWaypointDefaultIconChoice;
  OCPNIconCombo *pRoutepointDefaultIconChoice;
  wxCheckBox *pScaMinChckB, *pScaMinOverruleChckB;
  wxTextCtrl *m_pText_ScaMin;

  // For the font page
  wxBoxSizer *m_itemBoxSizerFontPanel;
  wxChoice *m_itemFontElementListBox, *m_itemStyleListBox, *m_itemLangListBox;
  wxStaticText *m_textSample;
  bool m_bVisitLang;

  // For "AIS Options"
  wxComboBox *m_itemAISListBox;

  // For "PlugIns" Panel
  PluginListPanel *m_pPlugInCtrl;
  AddPluginPanel *m_AddPluginPanel;
  CatalogMgrPanel *m_PluginCatalogMgrPanel;
  wxScrolledWindow *itemPanelPlugins;
  wxBoxSizer *itemBoxSizerPanelPlugins;
  wxFlexGridSizer *radarGrid, *waypointradarGrid;
  wxChoice *pNavAidRadarRingsNumberVisible, *pWaypointRangeRingsNumber;
  OCPNColourPickerCtrl *m_colourOwnshipRangeRingColour;
  wxChoice *m_itemRadarRingsUnits, *m_itemWaypointRangeRingsUnits;
  OCPNColourPickerCtrl *m_colourTrackLineColour;
  ;
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
  wxStaticText *pStatic_CallSign;

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
  wxWindow *pParent;

  int k_plugins;
  bool m_bForceNewToolbaronCancel;

  // Sounds panel

  OCPNSoundPanel *m_soundPanelAnchor;
  OCPNSoundPanel *m_soundPanelAIS;
  OCPNSoundPanel *m_soundPanelSART;
  OCPNSoundPanel *m_soundPanelDSC;
  void OnUXAudioEnableButtonClickAIS(wxCommandEvent &event);
  void OnUXAudioEnableButtonClickSART(wxCommandEvent &event);
  void OnUXAudioEnableButtonClickDSC(wxCommandEvent &event);

  /** Notified with a OCPN_Sound* pointer when sound has completed. */
  EventVar m_on_sound_done;
  ObsListener m_sound_done_listener;

private:
  void Init(void);
  void CreatePanel_MMSI(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_AIS(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_Ownship(size_t parent, int border_size,
                           int group_item_spacing);
  void CreatePanel_NMEA(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_ChartsLoad(size_t parent, int border_size,
                              int group_item_spacing);
  void CreatePanel_VectorCharts(size_t parent, int border_size,
                                int group_item_spacing);
  void CreatePanel_TidesCurrents(size_t parent, int border_size,
                                 int group_item_spacing);
  void CreatePanel_ChartGroups(size_t parent, int border_size,
                               int group_item_spacing);
  void CreatePanel_Display(size_t parent, int border_size,
                           int group_item_spacing);
  void CreatePanel_UI(size_t parent, int border_size, int group_item_spacing);
  void CreatePanel_Units(size_t parent, int border_size,
                         int group_item_spacing);
  void CreatePanel_Sounds(size_t parent, int border_size,
                          int group_item_spacing);
  void CreatePanel_Advanced(size_t parent, int border_size,
                            int group_item_spacing);
  void CreatePanel_Configs(size_t parent, int border_size,
                           int group_item_spacing);
  void CreatePanel_Routes(size_t parent, int border_size,
                          int group_item_spacing);

  void OnAlertEnableButtonClick(wxCommandEvent &event);
  void OnAlertAudioEnableButtonClick(wxCommandEvent &event);

  void UpdateTemplateTitleText();
  void CheckDeviceAccess(wxString &path);
  int m_returnChanges;
  wxListCtrl *tcDataSelected;
  std::vector<int> marinersStdXref;
  ChartGroupsUI *groupsPanel;
  wxImageList *m_topImgList;

  wxCheckBox* m_persist_active_route_chkbox;
  wxScrolledWindow *m_pNMEAForm;
  void resetMarStdList(bool bsetConfig, bool bsetStd);

  ObservableListener compat_os_listener;

  int m_screenConfig;

  wxNotebookPage *m_groupsPage;
  wxFont *dialogFont;
  wxFont smallFont;
  //  wxFont *dialogFont;
  wxSize m_small_button_size;

  bool m_bcompact;
  int m_fontHeight, m_scrollRate;
  bool m_bfontChanged;
  bool m_bVectorInit;

  wxBoxSizer *m_boxSizerConfigs;
  wxColour m_panelBackgroundUnselected;
  wxString m_selectedConfigPanelGUID;
  wxSize m_colourPickerDefaultSize;

  wxSize m_sliderSize;
  bool m_bneedNew;

  std::shared_ptr<ConnectionsDialog>comm_dialog;

  DECLARE_EVENT_TABLE()
};

class CanvasConfigSelect : public wxPanel {
public:
  CanvasConfigSelect(wxWindow *parent, options *parentOptions, int id,
                     wxBitmap &bmp, const wxPoint &pos = wxDefaultPosition,
                     const wxSize &size = wxDefaultSize);
  ~CanvasConfigSelect();

  void OnMouseSelected(wxMouseEvent &event);
  void SetSelected(bool selected);
  void OnPaint(wxPaintEvent &event);

  bool GetSelected() { return m_bSelected; }

private:
  options *m_parentOptions;
  bool m_bSelected;
  wxColour m_boxColour;
  wxBitmap m_bmpNormal;
  int m_borderWidth;

  DECLARE_EVENT_TABLE()
};

class ChartGroupsUI :  public wxScrolledWindow {
public:
  ChartGroupsUI(wxWindow *parent);
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
  explicit MMSIEditDialog(MmsiProperties *props, wxWindow *parent,
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

  MmsiProperties *m_props;
  wxTextCtrl *m_MMSICtl, m_ShipNameCtl;  // Has ToDo take away?
  wxRadioButton *m_rbTypeTrackDefault, *m_rbTypeTrackAlways;
  wxRadioButton *m_rbTypeTrackNever;
  wxCheckBox *m_cbTrackPersist, *m_IgnoreButton, *m_MOBButton, *m_VDMButton,
      *m_FollowerButton;
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

class ConfigCreateDialog : private Uncopyable, public wxDialog {
public:
  explicit ConfigCreateDialog(wxWindow *parent, wxWindowID id = wxID_ANY,
                              const wxString &caption = wxEmptyString,
                              const wxPoint &pos = wxDefaultPosition,
                              const wxSize &size = wxDefaultSize,
                              long style = 0);
  ~ConfigCreateDialog(void);

  void SetColorScheme(ColorScheme cs);
  void CreateControls(void);
  void OnConfigEditCancelClick(wxCommandEvent &event);
  void OnConfigEditOKClick(wxCommandEvent &event);
  wxString GetCreatedTemplateGUID() { return m_createdTemplateGUID; }

  wxTextCtrl *m_TitleCtl, *m_DescriptionCtl;
  wxButton *m_CancelButton, *m_OKButton;
  wxString m_createdTemplateGUID;
  DECLARE_EVENT_TABLE()
};

#endif
// _OPTIONS_H_
