/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Options Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */


#ifndef _OPTIONS_H_
#define _OPTIONS_H_


#include "wx/notebook.h"
#include "wx/dirctrl.h"
#include "wx/spinctrl.h"
#include <wx/listctrl.h>
#include "pluginmanager.h"

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
#include "scrollingdialog.h"
#endif


//      Forward Declarations
class wxGenericDirCtrl;
class MyConfig;

#define ID_DIALOG 10001
#define SYMBOL_OPTIONS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_OPTIONS_TITLE _("Options")
#define SYMBOL_OPTIONS_IDNAME ID_DIALOG
#define SYMBOL_OPTIONS_SIZE wxSize(500, 500)
#define SYMBOL_OPTIONS_POSITION wxDefaultPosition

enum {
        ID_TEXTCTRL1 =            10000,
        ID_NOTEBOOK,
        ID_TEXTCTRL,
        ID_LISTBOX,
        ID_PANEL,
        ID_DIRCTRL,
        ID_BUTTONDELETE,
        ID_PANEL2,
        ID_BUTTONREBUILD,
        xID_OK,
        ID_DEBUGCHECKBOX1,
        ID_PRINTCHECKBOX1,
        ID_CHECK_DISPLAYGRID,
        ID_OUTLINECHECKBOX1,
        ID_BUTTONADD,
        ID_PANEL3,
        ID_CHECKLISTBOX,
        ID_RADIOBOX,
        ID_PANELFONT,
        ID_CHOICE_FONTELEMENT,
        ID_BUTTONFONTCHOOSE,
        ID_CLEARLIST,
        ID_SELECTLIST,
        ID_CHOICE_AIS,
        ID_CHOICE_AP,
        ID_SOUNDGCHECKBOX,
        ID_METACHECKBOX,
        ID_TEXTCHECKBOX,
        ID_IMPTEXTCHECKBOX,
        ID_SCAMINCHECKBOX,
        ID_SHOWDEPTHUNITSBOX1,
        ID_AUTOANCHORMARKBOX1,
        ID_PANELAIS,
        ID_ATONTEXTCHECKBOX,
        ID_LDISTEXTCHECKBOX,
        ID_DECLTEXTCHECKBOX,
        ID_PANELADVANCED,
        ID_GPXCHECKBOX,
        ID_DRAGGINGCHECKBOX,
        ID_ZTCCHECKBOX,
        ID_PRESERVECHECKBOX,
	  ID_BELLSCHECKBOX,             // pjotrc 2010.01.31
        ID_AISALERTDIALOG,
        ID_AISALERTAUDIO,
        ID_AISALERTSELECTSOUND,
        ID_AISALERTTESTSOUND,
        ID_TRACKCHECKBOX,
        ID_CM93ZOOM,
        ID_PANELPIM,
        ID_GRIBCHECKBOX,
        ID_UPDCHECKBOX,
        ID_SHOWGPSWINDOW,
        ID_CHOICE_LANG,
        ID_QUILTCHECKBOX1,
        ID_RASTERCHECKBOX1,
        ID_VECTORCHECKBOX1,
        ID_CM93CHECKBOX1,
        ID_COURSEUPCHECKBOX,
        ID_CHOICE_NMEA_BAUD,
        ID_CHECK_LOOKAHEAD,
        ID_SKEWCOMPBOX,
        ID_GARMINHOST,
        ID_FILTERNMEA,
        ID_FULLSCREENQUILT


};

//    Define an int bit field for dialog return value
//    To indicate which types of settings have changed
#define     GENERIC_CHANGED   1
#define     S52_CHANGED       2
#define     FONT_CHANGED      4
#define     FORCE_UPDATE      8
#define     VISIT_CHARTS      16
#define     LOCALE_CHANGED    32
#define     TOOLBAR_CHANGED   64



#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

#ifndef bert// wxCHECK_VERSION(2, 9, 0)
class options: public wxDialog
#else
class options: public wxScrollingDialog
#endif
{
    DECLARE_DYNAMIC_CLASS( options )
    DECLARE_EVENT_TABLE()

public:
    options( );
    options( MyFrame* parent, wxWindowID id = SYMBOL_OPTIONS_IDNAME, const wxString& caption = SYMBOL_OPTIONS_TITLE,
           const wxPoint& pos = SYMBOL_OPTIONS_POSITION, const wxSize& size = SYMBOL_OPTIONS_SIZE, long style = SYMBOL_OPTIONS_STYLE);

    ~options( );

    bool Create( MyFrame* parent, wxWindowID id = SYMBOL_OPTIONS_IDNAME, const wxString& caption = SYMBOL_OPTIONS_TITLE,
             const wxPoint& pos = SYMBOL_OPTIONS_POSITION, const wxSize& size = SYMBOL_OPTIONS_SIZE,
             long style = SYMBOL_OPTIONS_STYLE);

    void Init();

    wxWindow* GetContentWindow() const;

    void CreateControls();
    void SetColorScheme(ColorScheme cs);

    void SetInitChartDir(wxString &dir){ m_init_chart_dir = dir;}
    void SetInitialSettings();

    void SetCurrentDirListPtr(ArrayOfCDI *p)  {m_pCurrentDirList = p;}
    void SetWorkDirListPtr(ArrayOfCDI *p)  {m_pWorkDirList = p;}

    void SetConfigPtr(MyConfig *p)  {m_pConfig = p;}
    void OnDebugcheckbox1Click( wxCommandEvent& event );
    void OnDirctrlSelChanged( wxTreeEvent& event );
    void OnButtonaddClick( wxCommandEvent& event );
    void OnButtondeleteClick( wxCommandEvent& event );
    void OnRadioboxSelected( wxCommandEvent& event );
    void OnXidOkClick( wxCommandEvent& event );
    void OnCancelClick( wxCommandEvent& event );
    void OnChooseFont( wxCommandEvent& event );
    void OnDisplayCategoryRadioButton( wxCommandEvent& event);
    void OnButtonClearClick( wxCommandEvent& event );
    void OnButtonSelectClick( wxCommandEvent& event );
    void OnPageChange(wxNotebookEvent& event);
    void OnNMEASourceChoice(wxCommandEvent& event);
    void OnButtonSelectSound(wxCommandEvent& event);
    void OnButtonTestSound(wxCommandEvent& event);
    void OnShowGpsWindowCheckboxClick( wxCommandEvent& event );

    void SetControlColors(wxWindow *ctrl, ColorScheme cs);

    void UpdateWorkArrayFromTextCtl();
    void CreateChartsPage();
    void PopulateChartsPage();

// Should we show tooltips?
    static bool ShowToolTips();

    wxNotebook*             itemNotebook4;
    wxButton*               m_OKButton;
    wxButton*               m_CancelButton;

//    For General Options
    wxScrolledWindow        *itemPanel5;
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
    wxCheckBox              *pCBLookAhead;
    wxTextCtrl              *m_pText_OSCOG_Predictor;
    wxCheckBox              *pSkewComp;
    wxCheckBox              *pFullScreenQuilt;

//    For GPS Page
    wxScrolledWindow        *itemPanelGPS;
    wxCheckBox              *pShowGPSWin;
    wxCheckBox              *pGarminHost;
    wxCheckBox              *pFilterNMEA;
    wxTextCtrl              *pFilterSecs;

//    For "S57" page
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
    wxCheckBox              *pCheck_DECLTEXT;
    wxTextCtrl              *m_ShallowCtl;
    wxTextCtrl              *m_SafetyCtl;
    wxTextCtrl              *m_DeepCtl;
    wxRadioBox              *pDepthUnitSelect;
    wxSlider                *m_pSlider_CM93_Zoom;
    wxCheckBox              *pSEnableCM93Offset;

//    For "Charts" page
    wxScrolledWindow          *itemPanel9;
    wxStaticBoxSizer          *itemStaticBoxSizer11;
    wxBoxSizer                *itemBoxSizer10;
    wxGenericDirCtrl          *pDirCtl;
    wxTextCtrl                *pSelCtl;
    wxListBox                 *pListBox;
    wxStaticBox               *itemActiveChartStaticBox;
    wxCheckBox                *pUpdateCheckBox;
    int                       k_charts;

//    For "NMEA Options" Box
    wxStaticBox             *m_itemNMEA_TCPIP_StaticBox;
    wxStaticBoxSizer        *m_itemNMEA_TCPIP_StaticBoxSizer;
    wxTextCtrl              *m_itemNMEA_TCPIP_Source;
    wxComboBox              *m_itemNMEAListBox;
    wxComboBox              *m_itemNMEAAutoListBox;
    wxComboBox              *m_itemNMEABaudListBox;

#ifdef USE_WIFI_CLIENT
//    For "WiFi Options" Box
    wxStaticBox             *m_itemWIFI_TCPIP_StaticBox;
    wxStaticBoxSizer        *m_itemWIFI_TCPIP_StaticBoxSizer;
    wxTextCtrl              *m_itemWIFI_TCPIP_Source;
#endif

//    For "AIS" Page
    wxScrolledWindow          *itemPanelAIS;

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

//    For Fonts page
    wxScrolledWindow        *itemPanelFont;
    wxBoxSizer              *m_itemBoxSizerFontPanel;
    wxComboBox              *m_itemFontElementListBox;
    wxComboBox              *m_itemLangListBox;
    bool                    m_bVisitLang;

//    For "AIS Options"
    wxComboBox              *m_itemAISListBox;

//    For "PlugIns" Panel
    PluginListPanel         *m_pPlugInCtrl;

//    For "Etc." Page
    wxScrolledWindow        *itemPanelAdvanced;
    wxCheckBox              *pNavAidShowRadarRings;
    wxTextCtrl              *pNavAidRadarRingsNumberVisible;
    wxTextCtrl              *pNavAidRadarRingsStep;
    wxRadioBox              *m_itemNavAidRadarRingsStepUnitsRadioBox;
    wxCheckBox              *pWayPointPreventDragging;
    wxCheckBox              *pEnableZoomToCursor;
    wxCheckBox              *pPreserveScale;
    wxCheckBox		    *pPlayShipsBells;  // pjotrc 2010.02.09

    wxCheckBox              *pTrackShowIcon;
    wxRadioButton           *m_pCheck_Trackpoint_time;
    wxRadioButton           *m_pCheck_Trackpoint_distance;
    wxTextCtrl              *m_pText_TP_Secs;
    wxTextCtrl              *m_pText_TP_Dist;

    wxCheckBox*             pSettingsCB1;

    ArrayOfCDI              *m_pCurrentDirList;
    ArrayOfCDI              *m_pWorkDirList;

    MyConfig                *m_pConfig;

    wxArrayPtrVoid          OBJLBoxArray;
    wxString                m_init_chart_dir;
    MyFrame                 *pParent;

    wxArrayString           *m_pSerialArray;
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
            wxLANGUAGE_CHINESE,
            wxLANGUAGE_CHINESE_SIMPLIFIED,
            wxLANGUAGE_CHINESE_TRADITIONAL,
            wxLANGUAGE_CHINESE_HONGKONG,
            wxLANGUAGE_CHINESE_MACAU,
            wxLANGUAGE_CHINESE_SINGAPORE,
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

#endif
    // _OPTIONS_H_
