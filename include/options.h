/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Options Dialog
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 */


#ifndef _OPTIONS_H_
#define _OPTIONS_H_


#include <wx/listbook.h>
#include <wx/dirctrl.h>
#include <wx/spinctrl.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/collpane.h>

#include "pluginmanager.h"

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
#include "scrollingdialog.h"
#endif

#include "datastream.h"

//      Forward Declarations
class wxGenericDirCtrl;
class MyConfig;
class ChartGroupsUI;
class ConnectionParams;
class SentenceListDlg;

#define ID_DIALOG 10001
#define SYMBOL_OPTIONS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
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
    ID_BUTTONFONTCHOOSE,
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
    ID_SHIPICONTYPE,
    ID_OUTLINECHECKBOX1,
    ID_PANEL,
    ID_PANEL2,
    ID_PANEL3,
    ID_PANELADVANCED,
    ID_PANELAIS,
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
    ID_TEXTCTRL,
    ID_TEXTCTRL1,
    ID_TIDESELECTED,
    ID_TRACKCHECKBOX,
    ID_TRACKHILITE,
    ID_TRANSTOOLBARCHECKBOX,
    ID_UPDCHECKBOX,
    ID_VECTORCHECKBOX1,
    ID_ZTCCHECKBOX,
    ID_DELETECHECKBOX,
    xID_OK
};

//    Define an int bit field for dialog return value
//    To indicate which types of settings have changed
#define     GENERIC_CHANGED    1
#define     S52_CHANGED        2
#define     FONT_CHANGED       4
#define     FORCE_UPDATE       8
#define     VISIT_CHARTS      16
#define     LOCALE_CHANGED    32
#define     TOOLBAR_CHANGED   64
#define     CHANGE_CHARTS    128
#define     SCAN_UPDATE      256
#define     GROUPS_CHANGED   512
#define     STYLE_CHANGED   1024
#define     TIDES_CHANGED   2048

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

WX_DECLARE_OBJARRAY(wxGenericDirCtrl *, ArrayOfDirCtrls);

#ifndef bert// wxCHECK_VERSION(2, 9, 0)
class options: public wxDialog
#else
class options: public wxScrollingDialog
#endif
{
    DECLARE_DYNAMIC_CLASS( options )
    DECLARE_EVENT_TABLE()

public:
    options();
    options( MyFrame* parent, wxWindowID id = SYMBOL_OPTIONS_IDNAME, const wxString& caption =
            SYMBOL_OPTIONS_TITLE, const wxPoint& pos = SYMBOL_OPTIONS_POSITION, const wxSize& size =
            SYMBOL_OPTIONS_SIZE, long style = SYMBOL_OPTIONS_STYLE );

    ~options();

    bool Create( MyFrame* parent, wxWindowID id = SYMBOL_OPTIONS_IDNAME, const wxString& caption =
            SYMBOL_OPTIONS_TITLE, const wxPoint& pos = SYMBOL_OPTIONS_POSITION, const wxSize& size =
            SYMBOL_OPTIONS_SIZE, long style = SYMBOL_OPTIONS_STYLE );

    void Init();

    wxWindow* GetContentWindow() const;

    void CreateControls();
    size_t CreatePanel( wxString title );
    wxScrolledWindow *AddPage( size_t parent, wxString title );
    bool DeletePage( wxScrolledWindow *page );
    void SetColorScheme( ColorScheme cs );

    void SetInitChartDir( wxString &dir )
    {
        m_init_chart_dir = dir;
    }
    void SetInitialSettings();

    void SetCurrentDirList( ArrayOfCDI p )
    {
        m_CurrentDirList = p;
    }
    void SetWorkDirListPtr( ArrayOfCDI *p )
    {
        m_pWorkDirList = p;
    }
    ArrayOfCDI* GetWorkDirListPtr()
    {
        return m_pWorkDirList;
    }

    void SetConfigPtr( MyConfig *p )
    {
        m_pConfig = p;
    }
    void OnDebugcheckbox1Click( wxCommandEvent& event );
    void OnDirctrlSelChanged( wxTreeEvent& event );
    void OnButtonaddClick( wxCommandEvent& event );
    void OnButtondeleteClick( wxCommandEvent& event );
    void OnRadioboxSelected( wxCommandEvent& event );
    void OnApplyClick( wxCommandEvent& event );
    void OnXidOkClick( wxCommandEvent& event );
    void OnCancelClick( wxCommandEvent& event );
    void OnChooseFont( wxCommandEvent& event );
    void OnDisplayCategoryRadioButton( wxCommandEvent& event );
    void OnButtonClearClick( wxCommandEvent& event );
    void OnButtonSelectClick( wxCommandEvent& event );
    void OnPageChange( wxListbookEvent& event );
    void OnButtonSelectSound( wxCommandEvent& event );
    void OnButtonTestSound( wxCommandEvent& event );
    void OnShowGpsWindowCheckboxClick( wxCommandEvent& event );
    void OnZTCCheckboxClick( wxCommandEvent& event );
    void OnRadarringSelect( wxCommandEvent& event );
    void OnShipTypeSelect( wxCommandEvent& event );
    void OnButtonGroups( wxCommandEvent& event );
    void OnInsertTideDataLocation( wxCommandEvent &event );
    void OnRemoveTideDataLocation( wxCommandEvent &event );
    void OnCharHook( wxKeyEvent& event );
    void OnChartsPageChange( wxListbookEvent& event );
    
    void UpdateWorkArrayFromTextCtl();

// Should we show tooltips?
    static bool ShowToolTips();

    wxListbook*             m_pListbook;
    size_t                  m_pageDisplay, m_pageConnections, m_pageCharts, m_pageShips, m_pageUI, m_pagePlugins;
    int                     lastPage;
    wxPoint                 lastWindowPos;
    wxSize                  lastWindowSize;
    wxButton*               m_ApplyButton;
    wxButton*               m_OKButton;
    wxButton*               m_CancelButton;

    ChartGroupArray         *m_pGroupArray;
    int                     m_groups_changed;

//    For General Options
    wxCheckBox              *pDebugShowStat;
    wxCheckBox              *pPrintShowIcon;
    wxCheckBox              *pCDOOutlines;
    wxCheckBox              *pSDepthUnits;
    wxCheckBox              *pSDisplayGrid;
    wxCheckBox              *pAutoAnchorMark;
    wxCheckBox              *pCDOQuilting;
    wxCheckBox              *pCBRaster;
    wxCheckBox              *pCBVector;
    wxCheckBox              *pCBCM93;
    wxCheckBox              *pCBCourseUp;
    wxTextCtrl              *pCOGUPUpdateSecs;
    wxCheckBox              *pCBLookAhead;
    wxTextCtrl              *m_pText_OSCOG_Predictor;
    wxChoice                *m_pShipIconType;
    wxCheckBox              *pSkewComp;
    wxCheckBox              *pOpenGL;
    wxCheckBox              *pSmoothPanZoom;
    wxCheckBox              *pFullScreenQuilt;
    wxChoice                *m_pcTCDatasets;
    int                      k_tides;

//    For GPS Page
    wxListCtrl* m_lcSources;
    wxButton* m_buttonAdd;
    wxButton* m_buttonRemove;
    wxStaticBoxSizer* sbSizerConnectionProps;
    wxRadioButton* m_rbTypeSerial;
    wxRadioButton* m_rbTypeNet;
    wxGridSizer* gSizerNetProps;
    wxStaticText* m_stNetProto;
    wxRadioButton* m_rbNetProtoTCP;
    wxRadioButton* m_rbNetProtoUDP;
    wxRadioButton* m_rbNetProtoGPSD;
    wxStaticText* m_stNetAddr;
    wxTextCtrl* m_tNetAddress;
    wxStaticText* m_stNetPort;
    wxTextCtrl* m_tNetPort;
    wxGridSizer* gSizerSerProps;
    wxStaticText* m_stSerPort;
    wxComboBox* m_comboPort;
    wxStaticText* m_stSerBaudrate;
    wxChoice* m_choiceBaudRate;
    wxStaticText* m_stSerProtocol;
    wxChoice* m_choiceSerialProtocol;
    wxStaticText* m_stPriority;
    wxChoice* m_choicePriority;
    wxCheckBox* m_cbCheckCRC;
    wxCheckBox* m_cbGarminHost;
    wxCheckBox* m_cbGarminUploadHost;
    wxCheckBox* m_cbFurunoGP3X;
    wxCheckBox* m_cbNMEADebug;
    wxCheckBox* m_cbFilterSogCog;
    wxStaticText* m_stFilterSec;
    wxTextCtrl* m_tFilterSec;
    wxRadioButton* m_rbIAccept;
    wxRadioButton* m_rbIIgnore;
    wxTextCtrl* m_tcInputStc;
    wxButton* m_btnInputStcList;
    wxCheckBox* m_cbOutput;
    wxRadioButton* m_rbOAccept;
    wxRadioButton* m_rbOIgnore;
    wxTextCtrl* m_tcOutputStc;
    wxButton* m_btnOutputStcList;
    wxStdDialogButtonSizer* m_sdbSizerDlgButtons;
    wxButton* m_sdbSizerDlgButtonsOK;
    wxButton* m_sdbSizerDlgButtonsApply;
    wxButton* m_sdbSizerDlgButtonsCancel;
    wxStaticBoxSizer* sbSizerInFilter;
    wxStaticBoxSizer* sbSizerOutFilter;

    SentenceListDlg* m_stcdialog_in;
    SentenceListDlg* m_stcdialog_out;
    
    // Virtual event handlers, overide them in your derived class
    void OnSelectDatasource( wxListEvent& event );
    void OnAddDatasourceClick( wxCommandEvent& event );
    void OnRemoveDatasourceClick( wxCommandEvent& event );
    void OnTypeSerialSelected( wxCommandEvent& event );
    void OnTypeNetSelected( wxCommandEvent& event );
    void OnNetProtocolSelected( wxCommandEvent& event );
    void OnBaudrateChoice( wxCommandEvent& event ) { OnConnValChange(event); }
    void OnProtocolChoice( wxCommandEvent& event ) { OnConnValChange(event); }
    void OnCrcCheck( wxCommandEvent& event ) { OnValChange(event); }
    void OnRbAcceptInput( wxCommandEvent& event );
    void OnRbIgnoreInput( wxCommandEvent& event );
    void OnBtnIStcs( wxCommandEvent& event );
    void OnCbOutput( wxCommandEvent& event ) { OnConnValChange(event); }
    void OnRbOutput( wxCommandEvent& event );
    void OnBtnOStcs( wxCommandEvent& event );
    void OnConnValChange( wxCommandEvent& event );
    void OnValChange( wxCommandEvent& event );
    void OnUploadFormatChange( wxCommandEvent& event );
    bool connectionsaved;

//    For "S57" page
    wxFlexGridSizer         *vectorPanel;
    wxScrolledWindow        *ps57Ctl;
    wxCheckListBox          *ps57CtlListBox;
    wxRadioBox              *pDispCat;
    wxButton                *itemButtonClearList;
    wxButton                *itemButtonSelectList;
    wxRadioBox              *pPointStyle;
    wxRadioBox              *pBoundStyle;
    wxRadioBox              *p24Color;
    wxCheckBox              *pCheck_SOUNDG;
    wxCheckBox              *pCheck_META;
    wxCheckBox              *pCheck_SHOWIMPTEXT;
    wxCheckBox              *pCheck_SCAMIN;
    wxCheckBox              *pCheck_ATONTEXT;
    wxCheckBox              *pCheck_LDISTEXT;
    wxCheckBox              *pCheck_XLSECTTEXT;
    wxCheckBox              *pCheck_DECLTEXT;
    wxTextCtrl              *m_ShallowCtl;
    wxTextCtrl              *m_SafetyCtl;
    wxTextCtrl              *m_DeepCtl;
    wxRadioBox              *pDepthUnitSelect;
    wxSlider                *m_pSlider_CM93_Zoom;
    wxCheckBox              *pSEnableCM93Offset;
    int                       k_vectorcharts;

//    For "Charts" page
    wxStaticBoxSizer          *activeSizer;
    wxBoxSizer                *chartPanel;
    wxTextCtrl                *pSelCtl;
    wxListBox                 *pActiveChartsList;
    wxStaticBox               *itemActiveChartStaticBox;
    wxCheckBox                *pUpdateCheckBox;
    wxCheckBox                *pScanCheckBox;
    int                       k_charts;

//    For "AIS" Page
    wxCheckBox                *m_pCheck_CPA_Max;
    wxTextCtrl                *m_pText_CPA_Max;
    wxCheckBox                *m_pCheck_CPA_Warn;
    wxTextCtrl                *m_pText_CPA_Warn;
    wxCheckBox                *m_pCheck_CPA_WarnT;
    wxTextCtrl                *m_pText_CPA_WarnT;
    wxCheckBox                *m_pCheck_Mark_Lost;
    wxTextCtrl                *m_pText_Mark_Lost;
    wxCheckBox                *m_pCheck_Remove_Lost;
    wxTextCtrl                *m_pText_Remove_Lost;
    wxCheckBox                *m_pCheck_Show_COG;
    wxTextCtrl                *m_pText_COG_Predictor;
    wxCheckBox                *m_pCheck_Show_Tracks;
    wxTextCtrl                *m_pText_Track_Length;
    wxCheckBox                *m_pCheck_Show_Moored;
    wxTextCtrl                *m_pText_Moored_Speed;
    wxCheckBox                *m_pCheck_AlertDialog;
    wxCheckBox                *m_pCheck_AlertAudio;
    wxCheckBox                *m_pCheck_Alert_Moored;
    wxCheckBox                *m_pCheck_Rollover_Class;
    wxCheckBox                *m_pCheck_Rollover_COG;
    wxCheckBox                *m_pCheck_Rollover_CPA;
    wxCheckBox                *m_pCheck_Ack_Timout;
    wxTextCtrl                *m_pText_ACK_Timeout;
    wxCheckBox                *m_pCheck_Show_Area_Notices;
    wxCheckBox                *m_pCheck_Draw_Target_Size;
    wxCheckBox                *m_pCheck_ShowAllCPA;
//    For Ship page
    wxFlexGridSizer*        realSizes;
    wxTextCtrl              *m_pOSLength;
    wxTextCtrl              *m_pOSWidth;
    wxTextCtrl              *m_pOSGPSOffsetX;
    wxTextCtrl              *m_pOSGPSOffsetY;
    wxTextCtrl              *m_pOSMinSize;
    wxStaticBoxSizer        *dispOptions;
    wxScrolledWindow        *itemPanelShip;
    wxBoxSizer              *ownShip;

//    For Fonts page
    wxBoxSizer              *m_itemBoxSizerFontPanel;
    wxComboBox              *m_itemFontElementListBox;
    wxComboBox              *m_itemStyleListBox;
    wxComboBox              *m_itemLangListBox;
    bool                    m_bVisitLang;

//    For "AIS Options"
    wxComboBox              *m_itemAISListBox;

//    For "PlugIns" Panel
    PluginListPanel         *m_pPlugInCtrl;
    int                     k_plugins;

    wxChoice                *pNavAidRadarRingsNumberVisible;
    wxFlexGridSizer         *radarGrid;
    wxTextCtrl              *pNavAidRadarRingsStep;
    wxChoice                *m_itemRadarRingsUnits;
    wxCheckBox              *pWayPointPreventDragging;
    wxCheckBox              *pConfirmObjectDeletion;
    wxCheckBox              *pEnableZoomToCursor;
    wxCheckBox              *pPreserveScale;
    wxCheckBox              *pPlayShipsBells;
    wxCheckBox              *pFullScreenToolbar;
    wxCheckBox              *pTransparentToolbar;
    wxChoice                *pSDMMFormat;

    wxCheckBox              *pTrackShowIcon;
    wxCheckBox              *pTrackDaily;
    wxCheckBox              *pTrackHighlite;
    wxChoice                *pTrackPrecision;
    wxTextCtrl              *m_pText_TP_Secs;
    wxTextCtrl              *m_pText_TP_Dist;

    wxCheckBox*             pSettingsCB1;

    ArrayOfCDI              m_CurrentDirList;
    ArrayOfCDI              *m_pWorkDirList;

    MyConfig                *m_pConfig;

    wxArrayPtrVoid          OBJLBoxArray;
    wxString                m_init_chart_dir;
    MyFrame                 *pParent;

    wxArrayString           *m_pSerialArray;

private:
    void CreatePanel_AIS( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );
    void CreatePanel_Ownship( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );
    void CreatePanel_NMEA( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );
    void CreatePanel_ChartsLoad( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );
    void CreatePanel_VectorCharts( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );
    void CreatePanel_TidesCurrents( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );
    void CreatePanel_ChartGroups( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );
    void CreatePanel_Display( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );
    void CreatePanel_UI( size_t parent, int border_size, int group_item_spacing,
            wxSize small_button_size );

    int m_returnChanges;
    wxListBox *tcDataSelected;
    std::vector<int> marinersStdXref;
    ChartGroupsUI *groupsPanel;
    wxImageList *m_topImgList;

    wxScrolledWindow *m_pNMEAForm;
    void ShowNMEACommon( bool visible );
    void ShowNMEASerial( bool visible );
    void ShowNMEANet( bool visible );
    void SetNMEAFormToSerial();
    void SetNMEAFormToNet();
    void SetConnectionParams(ConnectionParams *cp);
    void SetDSFormRWStates();
    void FillSourceList();
    ConnectionParams *SaveConnectionParams();
};

class ChartGroupsUI: public wxScrolledWindow {

    DECLARE_EVENT_TABLE()

public:
    ChartGroupsUI( wxWindow* parent );
    ~ChartGroupsUI();

    void CreatePanel( size_t parent, int border_size, int group_item_spacing, wxSize small_button_size );
    void CompletePanel( void );
    void SetDBDirs( ArrayOfCDI &array ) { m_db_dirs = array; }
    void SetGroupArray( ChartGroupArray *pGroupArray ) { m_pGroupArray = pGroupArray; }
    void SetInitialSettings();
    void CompleteInitialSettings();
    void PopulateTrees();
    
    void PopulateTreeCtrl( wxTreeCtrl *ptc, const wxArrayString &dir_array, const wxColour &col,
            wxFont *pFont = NULL );
    wxTreeCtrl *AddEmptyGroupPage( const wxString& label );

    void BuildNotebookPages( ChartGroupArray *pGroupArray );
    ChartGroupArray* CloneChartGroupArray( ChartGroupArray* s );
    void EmptyChartGroupArray( ChartGroupArray* s );

    void OnNodeExpanded( wxTreeEvent& event );
    void OnAvailableSelection( wxTreeEvent& event );
    void OnInsertChartItem( wxCommandEvent &event );
    void OnRemoveChartItem( wxCommandEvent &event );
    void OnGroupPageChange( wxNotebookEvent& event );
    void OnNewGroup( wxCommandEvent &event );
    void OnDeleteGroup( wxCommandEvent &event );

    bool modified;
    bool m_UIcomplete;
    bool m_settingscomplete;
    bool m_treespopulated;
    
    
private:
    int FindGroupBranch( ChartGroup *pGroup, wxTreeCtrl *ptree, wxTreeItemId item,
            wxString *pbranch_adder );

    wxWindow *pParent;

    wxFlexGridSizer *groupsSizer;
    wxButton *m_pAddButton;
    wxButton *m_pRemoveButton;
    wxButton *m_pNewGroupButton;
    wxButton *m_pDeleteGroupButton;
    int m_border_size;
    int m_group_item_spacing;

    wxGenericDirCtrl *allAvailableCtl;
    wxGenericDirCtrl *defaultAllCtl;
    wxTreeCtrl* m_pActiveChartsTree;
    wxTreeCtrl* lastSelectedCtl;
    wxTreeItemId lastDeletedItem;
    wxNotebook* m_GroupNB;
    ArrayOfCDI m_db_dirs;
    int m_GroupSelectedPage;
    wxFont *iFont;

    ArrayOfDirCtrls m_DirCtrlArray;

    ChartGroupArray *m_pGroupArray;
};

static int lang_list[] = {
            wxLANGUAGE_DEFAULT,
            wxLANGUAGE_ABKHAZIAN,
            wxLANGUAGE_AFAR,
            wxLANGUAGE_AFRIKAANS,
            wxLANGUAGE_ALBANIAN,
            wxLANGUAGE_AMHARIC,
            wxLANGUAGE_ARABIC,
            wxLANGUAGE_ARABIC_ALGERIA,
            wxLANGUAGE_ARABIC_BAHRAIN,
            wxLANGUAGE_ARABIC_EGYPT,
            wxLANGUAGE_ARABIC_IRAQ,
            wxLANGUAGE_ARABIC_JORDAN,
            wxLANGUAGE_ARABIC_KUWAIT,
            wxLANGUAGE_ARABIC_LEBANON,
            wxLANGUAGE_ARABIC_LIBYA,
            wxLANGUAGE_ARABIC_MOROCCO,
            wxLANGUAGE_ARABIC_OMAN,
            wxLANGUAGE_ARABIC_QATAR,
            wxLANGUAGE_ARABIC_SAUDI_ARABIA,
            wxLANGUAGE_ARABIC_SUDAN,
            wxLANGUAGE_ARABIC_SYRIA,
            wxLANGUAGE_ARABIC_TUNISIA,
            wxLANGUAGE_ARABIC_UAE,
            wxLANGUAGE_ARABIC_YEMEN,
            wxLANGUAGE_ARMENIAN,
            wxLANGUAGE_ASSAMESE,
            wxLANGUAGE_AYMARA,
            wxLANGUAGE_AZERI,
            wxLANGUAGE_AZERI_CYRILLIC,
            wxLANGUAGE_AZERI_LATIN,
            wxLANGUAGE_BASHKIR,
            wxLANGUAGE_BASQUE,
            wxLANGUAGE_BELARUSIAN,
            wxLANGUAGE_BENGALI,
            wxLANGUAGE_BHUTANI,
            wxLANGUAGE_BIHARI,
            wxLANGUAGE_BISLAMA,
            wxLANGUAGE_BRETON,
            wxLANGUAGE_BULGARIAN,
            wxLANGUAGE_BURMESE,
            wxLANGUAGE_CAMBODIAN,
            wxLANGUAGE_CATALAN,
//            wxLANGUAGE_CHINESE,
//            wxLANGUAGE_CHINESE_SIMPLIFIED,
//            wxLANGUAGE_CHINESE_TRADITIONAL,
//            wxLANGUAGE_CHINESE_HONGKONG,
//            wxLANGUAGE_CHINESE_MACAU,
//            wxLANGUAGE_CHINESE_SINGAPORE,
            wxLANGUAGE_CHINESE_TAIWAN,
            wxLANGUAGE_CORSICAN,
            wxLANGUAGE_CROATIAN,
            wxLANGUAGE_CZECH,
            wxLANGUAGE_DANISH,
            wxLANGUAGE_DUTCH,
            wxLANGUAGE_DUTCH_BELGIAN,
            wxLANGUAGE_ENGLISH,
            wxLANGUAGE_ENGLISH_UK,
            wxLANGUAGE_ENGLISH_US,
            wxLANGUAGE_ENGLISH_AUSTRALIA,
            wxLANGUAGE_ENGLISH_BELIZE,
            wxLANGUAGE_ENGLISH_BOTSWANA,
            wxLANGUAGE_ENGLISH_CANADA,
            wxLANGUAGE_ENGLISH_CARIBBEAN,
            wxLANGUAGE_ENGLISH_DENMARK,
            wxLANGUAGE_ENGLISH_EIRE,
            wxLANGUAGE_ENGLISH_JAMAICA,
            wxLANGUAGE_ENGLISH_NEW_ZEALAND,
            wxLANGUAGE_ENGLISH_PHILIPPINES,
            wxLANGUAGE_ENGLISH_SOUTH_AFRICA,
            wxLANGUAGE_ENGLISH_TRINIDAD,
            wxLANGUAGE_ENGLISH_ZIMBABWE,
            wxLANGUAGE_ESPERANTO,
            wxLANGUAGE_ESTONIAN,
            wxLANGUAGE_FAEROESE,
            wxLANGUAGE_FARSI,
            wxLANGUAGE_FIJI,
            wxLANGUAGE_FINNISH,
            wxLANGUAGE_FRENCH,
            wxLANGUAGE_FRENCH_BELGIAN,
            wxLANGUAGE_FRENCH_CANADIAN,
            wxLANGUAGE_FRENCH_LUXEMBOURG,
            wxLANGUAGE_FRENCH_MONACO,
            wxLANGUAGE_FRENCH_SWISS,
            wxLANGUAGE_FRISIAN,
            wxLANGUAGE_GALICIAN,
            wxLANGUAGE_GEORGIAN,
            wxLANGUAGE_GERMAN,
            wxLANGUAGE_GERMAN_AUSTRIAN,
            wxLANGUAGE_GERMAN_BELGIUM,
            wxLANGUAGE_GERMAN_LIECHTENSTEIN,
            wxLANGUAGE_GERMAN_LUXEMBOURG,
            wxLANGUAGE_GERMAN_SWISS,
            wxLANGUAGE_GREEK,
            wxLANGUAGE_GREENLANDIC,
            wxLANGUAGE_GUARANI,
            wxLANGUAGE_GUJARATI,
            wxLANGUAGE_HAUSA,
            wxLANGUAGE_HEBREW,
            wxLANGUAGE_HINDI,
            wxLANGUAGE_HUNGARIAN,
            wxLANGUAGE_ICELANDIC,
            wxLANGUAGE_INDONESIAN,
            wxLANGUAGE_INTERLINGUA,
            wxLANGUAGE_INTERLINGUE,
            wxLANGUAGE_INUKTITUT,
            wxLANGUAGE_INUPIAK,
            wxLANGUAGE_IRISH,
            wxLANGUAGE_ITALIAN,
            wxLANGUAGE_ITALIAN_SWISS,
            wxLANGUAGE_JAPANESE,
            wxLANGUAGE_JAVANESE,
            wxLANGUAGE_KANNADA,
            wxLANGUAGE_KASHMIRI,
            wxLANGUAGE_KASHMIRI_INDIA,
            wxLANGUAGE_KAZAKH,
            wxLANGUAGE_KERNEWEK,
            wxLANGUAGE_KINYARWANDA,
            wxLANGUAGE_KIRGHIZ,
            wxLANGUAGE_KIRUNDI,
//            wxLANGUAGE_KONKANI,
            wxLANGUAGE_KOREAN,
            wxLANGUAGE_KURDISH,
            wxLANGUAGE_LAOTHIAN,
            wxLANGUAGE_LATIN,
            wxLANGUAGE_LATVIAN,
            wxLANGUAGE_LINGALA,
            wxLANGUAGE_LITHUANIAN,
            wxLANGUAGE_MACEDONIAN,
            wxLANGUAGE_MALAGASY,
            wxLANGUAGE_MALAY,
            wxLANGUAGE_MALAYALAM,
            wxLANGUAGE_MALAY_BRUNEI_DARUSSALAM,
            wxLANGUAGE_MALAY_MALAYSIA,
            wxLANGUAGE_MALTESE,
//            wxLANGUAGE_MANIPURI,
            wxLANGUAGE_MAORI,
            wxLANGUAGE_MARATHI,
            wxLANGUAGE_MOLDAVIAN,
            wxLANGUAGE_MONGOLIAN,
            wxLANGUAGE_NAURU,
            wxLANGUAGE_NEPALI,
            wxLANGUAGE_NEPALI_INDIA,
            wxLANGUAGE_NORWEGIAN_BOKMAL,
            wxLANGUAGE_NORWEGIAN_NYNORSK,
            wxLANGUAGE_OCCITAN,
            wxLANGUAGE_ORIYA,
            wxLANGUAGE_OROMO,
            wxLANGUAGE_PASHTO,
            wxLANGUAGE_POLISH,
            wxLANGUAGE_PORTUGUESE,
            wxLANGUAGE_PORTUGUESE_BRAZILIAN,
            wxLANGUAGE_PUNJABI,
            wxLANGUAGE_QUECHUA,
            wxLANGUAGE_RHAETO_ROMANCE,
            wxLANGUAGE_ROMANIAN,
            wxLANGUAGE_RUSSIAN,
            wxLANGUAGE_RUSSIAN_UKRAINE,
            wxLANGUAGE_SAMOAN,
            wxLANGUAGE_SANGHO,
            wxLANGUAGE_SANSKRIT,
            wxLANGUAGE_SCOTS_GAELIC,
            wxLANGUAGE_SERBIAN,
            wxLANGUAGE_SERBIAN_CYRILLIC,
            wxLANGUAGE_SERBIAN_LATIN,
            wxLANGUAGE_SERBO_CROATIAN,
            wxLANGUAGE_SESOTHO,
            wxLANGUAGE_SETSWANA,
            wxLANGUAGE_SHONA,
            wxLANGUAGE_SINDHI,
            wxLANGUAGE_SINHALESE,
            wxLANGUAGE_SISWATI,
            wxLANGUAGE_SLOVAK,
            wxLANGUAGE_SLOVENIAN,
            wxLANGUAGE_SOMALI,
            wxLANGUAGE_SPANISH,
            wxLANGUAGE_SPANISH_ARGENTINA,
            wxLANGUAGE_SPANISH_BOLIVIA,
            wxLANGUAGE_SPANISH_CHILE,
            wxLANGUAGE_SPANISH_COLOMBIA,
            wxLANGUAGE_SPANISH_COSTA_RICA,
            wxLANGUAGE_SPANISH_DOMINICAN_REPUBLIC,
            wxLANGUAGE_SPANISH_ECUADOR,
            wxLANGUAGE_SPANISH_EL_SALVADOR,
            wxLANGUAGE_SPANISH_GUATEMALA,
            wxLANGUAGE_SPANISH_HONDURAS,
            wxLANGUAGE_SPANISH_MEXICAN,
//            wxLANGUAGE_SPANISH_MODERN,
            wxLANGUAGE_SPANISH_NICARAGUA,
            wxLANGUAGE_SPANISH_PANAMA,
            wxLANGUAGE_SPANISH_PARAGUAY,
            wxLANGUAGE_SPANISH_PERU,
            wxLANGUAGE_SPANISH_PUERTO_RICO,
            wxLANGUAGE_SPANISH_URUGUAY,
            wxLANGUAGE_SPANISH_US,
            wxLANGUAGE_SPANISH_VENEZUELA,
            wxLANGUAGE_SUNDANESE,
            wxLANGUAGE_SWAHILI,
            wxLANGUAGE_SWEDISH,
            wxLANGUAGE_SWEDISH_FINLAND,
            wxLANGUAGE_TAGALOG,
            wxLANGUAGE_TAJIK,
            wxLANGUAGE_TAMIL,
            wxLANGUAGE_TATAR,
            wxLANGUAGE_TELUGU,
            wxLANGUAGE_THAI,
            wxLANGUAGE_TIBETAN,
            wxLANGUAGE_TIGRINYA,
            wxLANGUAGE_TONGA,
            wxLANGUAGE_TSONGA,
            wxLANGUAGE_TURKISH,
            wxLANGUAGE_TURKMEN,
            wxLANGUAGE_TWI,
            wxLANGUAGE_UIGHUR,
            wxLANGUAGE_UKRAINIAN,
            wxLANGUAGE_URDU,
            wxLANGUAGE_URDU_INDIA,
            wxLANGUAGE_URDU_PAKISTAN,
            wxLANGUAGE_UZBEK,
            wxLANGUAGE_UZBEK_CYRILLIC,
            wxLANGUAGE_UZBEK_LATIN,
            wxLANGUAGE_VIETNAMESE,
            wxLANGUAGE_VOLAPUK,
            wxLANGUAGE_WELSH,
            wxLANGUAGE_WOLOF,
            wxLANGUAGE_XHOSA,
            wxLANGUAGE_YIDDISH,
            wxLANGUAGE_YORUBA,
            wxLANGUAGE_ZHUANG,
            wxLANGUAGE_ZULU
            };

///////////////////////////////////////////////////////////////////////////////
/// Class SentenceListDlg
///////////////////////////////////////////////////////////////////////////////
class SentenceListDlg : public wxDialog
{
    private:
        wxArrayString m_sentences;
        void FillSentences();
        ListType m_type;
        FilterDirection m_dir;

    protected:
        wxCheckListBox* m_clbSentences;
        wxButton* m_btnAdd;
        wxButton* m_btnDel;
        wxButton* m_btnCheckAll;
        wxButton* m_btnClearAll;
        wxStdDialogButtonSizer* m_sdbSizer4;
        wxButton* m_sdbSizer4OK;
        wxButton* m_sdbSizer4Cancel;
        wxArrayString standard_sentences;
        wxStaticBox *m_pclbBox;
        
    // Virtual event handlers, overide them in your derived class
        void OnStcSelect( wxCommandEvent& event );
        void OnAddClick( wxCommandEvent& event );
        void OnDeleteClick( wxCommandEvent& event );
        void OnCancelClick( wxCommandEvent& event );
        void OnOkClick( wxCommandEvent& event );
        void OnCLBSelect( wxCommandEvent& event );
        void OnCLBToggle( wxCommandEvent& event );
        void OnCheckAllClick( wxCommandEvent& event );
        void OnClearAllClick( wxCommandEvent& event );
        
    public:

    SentenceListDlg( FilterDirection dir,
                     wxWindow* parent,
                     wxWindowID id = wxID_ANY,
                     const wxString& title = _("Sentence Filter"),
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxSize( 280,420 ),
                     long style = wxDEFAULT_DIALOG_STYLE );
    ~SentenceListDlg();
    void SetSentenceList(wxArrayString sentences);
    wxString GetSentencesAsText();
    void BuildSentenceArray();
    void SetType(int io, ListType type);
};
#endif
    // _OPTIONS_H_
