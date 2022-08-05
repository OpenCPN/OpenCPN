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
#include <memory>

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/progdlg.h>
#include <wx/radiobox.h>
#include <wx/listbox.h>
#include <wx/imaglist.h>
#include <wx/display.h>
#include <wx/choice.h>
#include <wx/dirdlg.h>
#include <wx/clrpicker.h>
#include <wx/stdpaths.h>
#include "wx/tokenzr.h"
#include <wx/mediactrl.h>
#include "wx/dir.h"
#include "wx/odcombo.h"
#include <wx/statline.h>
#include <wx/regex.h>
#include <wx/textwrapper.h>
#include "SignalKDataStream.h"

#if wxCHECK_VERSION(2, 9, \
                    4) /* does this work in 2.8 too.. do we need a test? */
#include <wx/renderer.h>
#endif
#if defined(__WXGTK__) || defined(__WXQT__)
#include <wx/colordlg.h>
#endif

#include "config.h"

#include "dychart.h"
#include "chart1.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
extern GLuint g_raster_format;
#endif

#include "chartdbs.h"
#include "options.h"
#include "styles.h"
#include "datastream.h"
#include "multiplexer.h"
#include "FontMgr.h"
#include "OCPN_Sound.h"
#include "SoundFactory.h"
#include "SystemCmdSound.h"

#include "SystemCmdSound.h"

#include "NMEALogWindow.h"
#include "wx28compat.h"
#include "routeman.h"
#include "chcanv.h"
#include "MarkInfo.h"

#include "ais.h"
#include "AIS_Decoder.h"
#include "AIS_Target_Data.h"

#include "usb_devices.h"
#include "navutil.h"

#include "s52plib.h"
#include "s52utils.h"
#include "cm93.h"

#ifdef __linux__
#include "udev_rule_mgr.h"
#endif

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#ifdef __OCPN__ANDROID__
#include <QtWidgets/QScroller>
#endif

#include "OCPNPlatform.h"
#include "ConfigMgr.h"

#include "SignalKDataStream.h"
#include "observable.h"
#include "ser_ports.h"
#include "svg_utils.h"

#if !defined(__WXOSX__)
#define SLIDER_STYLE wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS
#else
#define SLIDER_STYLE wxSL_HORIZONTAL | wxSL_AUTOTICKS
#endif

wxString GetOCPNKnownLanguage(const wxString lang_canonical,
                              wxString& lang_dir);
wxString GetOCPNKnownLanguage(const wxString lang_canonical);

extern OCPNPlatform* g_Platform;

extern MyFrame* gFrame;
extern WayPointman* pWayPointMan;
extern bool g_bSoftwareGL;
extern bool g_bShowFPS;

extern bool g_bShowOutlines;
extern bool g_bShowChartBar;
extern bool g_bShowDepthUnits;
extern bool g_bskew_comp;
extern bool g_bopengl;
extern bool g_bsmoothpanzoom;
extern bool g_bShowTrue, g_bShowMag;
extern double g_UserVar;
extern double gVar;
extern int g_chart_zoom_modifier;
extern int g_chart_zoom_modifier_vector;
extern int g_NMEAAPBPrecision;
extern wxString g_TalkerIdText;
extern int g_nDepthUnitDisplay;
extern bool g_bUIexpert;

extern wxString* pInit_Chart_Dir;
extern wxArrayOfConnPrm* g_pConnectionParams;
extern Multiplexer* g_pMUX;
extern bool g_bfilter_cogsog;
extern int g_COGFilterSec;
extern int g_SOGFilterSec;

extern PlugInManager* g_pi_manager;
extern ocpnStyle::StyleManager* g_StyleManager;

extern bool g_bDisplayGrid;

// LIVE ETA OPTION
extern bool g_bShowLiveETA;
extern double g_defaultBoatSpeed;
extern double g_defaultBoatSpeedUserUnit;

//    AIS Global configuration
extern bool g_bCPAMax;
extern double g_CPAMax_NM;
extern bool g_bCPAWarn;
extern double g_CPAWarn_NM;
extern bool g_bTCPA_Max;
extern double g_TCPA_Max;
extern bool g_bMarkLost;
extern double g_MarkLost_Mins;
extern bool g_bRemoveLost;
extern double g_RemoveLost_Mins;
extern bool g_bShowCOG;
extern double g_ShowCOG_Mins;
extern bool g_bSyncCogPredictors;
extern bool g_bAISShowTracks;
extern double g_AISShowTracks_Mins;
extern double g_ShowMoored_Kts;
extern bool g_bHideMoored;
extern bool g_bAllowShowScaled;
extern int g_ShowScaled_Num;
extern bool g_bAIS_CPA_Alert;
extern bool g_bAIS_CPA_Alert_Audio;
extern wxString g_sAIS_Alert_Sound_File;
extern bool g_bAIS_CPA_Alert_Suppress_Moored;
extern bool g_bShowAreaNotices;
extern bool g_bDrawAISSize;
extern bool g_bDrawAISRealtime;
extern double g_AIS_RealtPred_Kts;
extern bool g_bShowAISName;
extern int g_Show_Target_Name_Scale;
extern bool g_bWplUsePosition;
extern int g_WplAction;

extern int g_iNavAidRadarRingsNumberVisible;
extern float g_fNavAidRadarRingsStep;
extern int g_pNavAidRadarRingsStepUnits;
extern int g_iWaypointRangeRingsNumber;
extern float g_fWaypointRangeRingsStep;
extern int g_iWaypointRangeRingsStepUnits;
extern wxColour g_colourWaypointRangeRingsColour;
extern bool g_bWayPointPreventDragging;
extern wxColour g_colourOwnshipRangeRingsColour;
extern bool g_bShowShipToActive;
extern int g_shipToActiveStyle;
extern int g_shipToActiveColor;

extern bool g_own_ship_sog_cog_calc;
extern int g_own_ship_sog_cog_calc_damp_sec;

extern bool g_bPreserveScaleOnX;
extern bool g_bPlayShipsBells;

extern wxString g_CmdSoundString;

extern int g_iSoundDeviceIndex;
extern bool g_bFullscreenToolbar;
extern bool g_bTransparentToolbar;
extern bool g_bTransparentToolbarInOpenGLOK;

extern int g_OwnShipIconType;
extern double g_n_ownship_length_meters;
extern double g_n_ownship_beam_meters;
extern double g_n_gps_antenna_offset_y;
extern double g_n_gps_antenna_offset_x;
extern int g_n_ownship_min_mm;
extern double g_n_arrival_circle_radius;

extern bool g_bEnableZoomToCursor;
extern bool g_bTrackDaily;
extern int g_track_rotate_time;
extern int g_track_rotate_time_type;
extern bool g_bHighliteTracks;
extern double g_TrackDeltaDistance;
extern double g_TrackDeltaDistance;
extern int g_nTrackPrecision;
extern wxColour g_colourTrackLineColour;

extern int g_iSDMMFormat;
extern int g_iDistanceFormat;
extern int g_iSpeedFormat;
extern int g_iTempFormat;

extern bool g_bAdvanceRouteWaypointOnArrivalOnly;

extern int g_cm93_zoom_factor;

extern int g_COGAvgSec;

extern bool g_bCourseUp;
extern bool g_bLookAhead;

extern double g_ownship_predictor_minutes;
extern double g_ownship_HDTpredictor_miles;

extern bool g_bAISRolloverShowClass;
extern bool g_bAISRolloverShowCOG;
extern bool g_bAISRolloverShowCPA;

extern bool g_bAIS_ACK_Timeout;
extern double g_AckTimeout_Mins;

extern bool g_bQuiltEnable;
extern bool g_bFullScreenQuilt;
extern bool g_bConfirmObjectDelete;
extern wxString g_GPS_Ident;
extern bool g_bGarminHostUpload;
extern wxString g_compatOS;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
extern wxLocale* plocale_def_lang;
#endif

extern double g_mouse_zoom_sensitivity;
extern int g_mouse_zoom_sensitivity_ui;

extern OcpnSound* g_anchorwatch_sound;
extern wxString g_anchorwatch_sound_file;
extern wxString g_DSC_sound_file;
extern wxString g_SART_sound_file;
extern wxString g_AIS_sound_file;
extern bool g_bAIS_GCPA_Alert_Audio;
extern bool g_bAIS_SART_Alert_Audio;
extern bool g_bAIS_DSC_Alert_Audio;
extern bool g_bAnchor_Alert_Audio;

extern bool g_bMagneticAPB;

extern bool g_fog_overzoom;
extern bool g_oz_vector_scale;
extern bool g_bShowStatusBar;

extern s52plib* ps52plib;

extern wxString g_locale;
extern bool g_bportable;
extern bool g_bdisable_opengl;

extern ChartGroupArray* g_pGroupArray;
extern ocpnStyle::StyleManager* g_StyleManager;

#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;
#endif

extern bool g_bGLexpert;
//    Some constants
#define ID_CHOICE_NMEA wxID_HIGHEST + 1

extern wxArrayString TideCurrentDataSet;
extern wxString g_TCData_Dir;

extern AIS_Decoder* g_pAIS;

options* g_pOptions;

extern bool g_bShowMenuBar;
extern bool g_bShowCompassWin;

extern bool g_btouch;
extern bool g_bresponsive;
extern bool g_bAutoHideToolbar;
extern int g_nAutoHideToolbar;
extern int g_GUIScaleFactor;
extern int g_ChartScaleFactor;
extern float g_ChartScaleFactorExp;
extern bool g_bRollover;
extern int g_ShipScaleFactor;
extern float g_ShipScaleFactorExp;
extern int g_ENCSoundingScaleFactor;
extern bool g_bShowMuiZoomButtons;

extern double g_config_display_size_mm;
extern bool g_config_display_size_manual;
extern bool g_bInlandEcdis;
extern unsigned int g_canvasConfig;
extern bool g_useMUI;
extern wxString g_lastAppliedTemplateGUID;
extern wxString g_default_wp_icon;
extern wxString g_default_routepoint_icon;
extern int g_iWpt_ScaMin;
extern bool g_bUseWptScaMin;
bool g_bOverruleScaMin;
extern int osMajor, osMinor;
extern bool g_bShowMuiZoomButtons;
extern MyConfig* pConfig;

#ifdef __OCPN__ANDROID__
extern int g_Android_SDK_Version;
extern MigrateAssistantDialog* g_migrateDialog;
#endif

extern wxString GetShipNameFromFile(int);

WX_DEFINE_ARRAY_PTR(ChartCanvas*, arrayofCanvasPtr);
extern arrayofCanvasPtr g_canvasArray;

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
    wxLANGUAGE_DUTCH_BELGIAN, wxLANGUAGE_ENGLISH_UK, wxLANGUAGE_ENGLISH_US,
    wxLANGUAGE_ENGLISH_AUSTRALIA, wxLANGUAGE_ENGLISH_BELIZE,
    wxLANGUAGE_ENGLISH_BOTSWANA, wxLANGUAGE_ENGLISH_CANADA,
    wxLANGUAGE_ENGLISH_CARIBBEAN, wxLANGUAGE_ENGLISH_DENMARK,
    wxLANGUAGE_ENGLISH_EIRE, wxLANGUAGE_ENGLISH_JAMAICA,
    wxLANGUAGE_ENGLISH_NEW_ZEALAND, wxLANGUAGE_ENGLISH_PHILIPPINES,
    wxLANGUAGE_ENGLISH_SOUTH_AFRICA, wxLANGUAGE_ENGLISH_TRINIDAD,
    wxLANGUAGE_ENGLISH_ZIMBABWE, wxLANGUAGE_ESPERANTO, wxLANGUAGE_ESTONIAN,
    wxLANGUAGE_FAEROESE, wxLANGUAGE_FARSI, wxLANGUAGE_FIJI, wxLANGUAGE_FINNISH,
    wxLANGUAGE_FRENCH, wxLANGUAGE_FRENCH_BELGIAN, wxLANGUAGE_FRENCH_CANADIAN,
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

#ifdef __OCPN__ANDROID__
void prepareSlider(wxSlider* slider) {
  slider->GetHandle()->setStyleSheet(
      prepareAndroidSliderStyleSheet(slider->GetSize().x));
  slider->GetHandle()->setAttribute(Qt::WA_AcceptTouchEvents);
  slider->GetHandle()->grabGesture(Qt::PanGesture);
  slider->GetHandle()->grabGesture(Qt::SwipeGesture);
}
#endif

// sort callback for Connections list  Sort by priority.
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortConnectionOnPriority(wxIntPtr item1, wxIntPtr item2,
                                        wxIntPtr list)
#else
int wxCALLBACK SortConnectionOnPriority(long item1, long item2, long list)
#endif
{
  wxListCtrl* lc = reinterpret_cast<wxListCtrl*>(list);

  wxListItem it1, it2;
  it1.SetId(lc->FindItem(-1, item1));
  it1.SetColumn(3);
  it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

  it2.SetId(lc->FindItem(-1, item2));
  it2.SetColumn(3);
  it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

  lc->GetItem(it1);
  lc->GetItem(it2);

#ifdef __WXOSX__
  return it1.GetText().CmpNoCase(it2.GetText());
#else
  return it2.GetText().CmpNoCase(it1.GetText());
#endif
}

//////////////////////////////////////////////////////////////////////////////////////

class ChartDirPanelHardBreakWrapper : public wxTextWrapper {
public:
  ChartDirPanelHardBreakWrapper(wxWindow* win, const wxString& text,
                                int widthMax) {
    m_lineCount = 0;

    // Replace all spaces in the string with a token character '^'
    wxString textMod = text;
    textMod.Replace(" ", "^");

    // Replace all path separators with spaces
    wxString sep = wxFileName::GetPathSeparator();
    textMod.Replace(sep, " ");

    Wrap(win, textMod, widthMax);

    // walk the output array, repairing the substitutions
    for (size_t i = 0; i < m_array.GetCount(); i++) {
      wxString a = m_array[i];
      a.Replace(" ", sep);
      if (m_array.GetCount() > 1) {
        if (i < m_array.GetCount() - 1) a += sep;
      }
      a.Replace("^", " ");
      m_array[i] = a;
    }
  }
  wxString const& GetWrapped() const { return m_wrapped; }
  int const GetLineCount() const { return m_lineCount; }
  wxArrayString GetLineArray() { return m_array; }

protected:
  virtual void OnOutputLine(const wxString& line) {
    m_wrapped += line;
    m_array.Add(line);
  }
  virtual void OnNewLine() {
    m_wrapped += '\n';
    m_lineCount++;
  }

private:
  wxString m_wrapped;
  int m_lineCount;
  wxArrayString m_array;
};

class OCPNChartDirPanel : public wxPanel {
public:
  OCPNChartDirPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                    const wxSize& size, ChartDirInfo& cdi);
  ~OCPNChartDirPanel();

  void DoChartSelected();
  void SetSelected(bool selected);
  void OnPaint(wxPaintEvent& event);
  // void OnEraseBackground( wxEraseEvent &event );
  void OnSize(wxSizeEvent& event);
  ChartDirInfo GetCDI() { return m_cdi; }
  int GetUnselectedHeight() { return m_unselectedHeight; }
  int GetRefHeight() { return m_refHeight; }
  bool IsSelected() { return m_bSelected; }
  void OnClickDown(wxMouseEvent& event);
  void OnClickUp(wxMouseEvent& event);

private:
  // shopPanel *m_pContainer;
  bool m_bSelected;
  wxColour m_boxColour;
  int m_unselectedHeight;
  wxString m_pChartDir;
  int m_refHeight;
  ChartDirInfo m_cdi;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(OCPNChartDirPanel, wxPanel)
EVT_PAINT(OCPNChartDirPanel::OnPaint)
// EVT_ERASE_BACKGROUND( OCPNChartDirPanel::OnEraseBackground)
EVT_SIZE(OCPNChartDirPanel::OnSize)
END_EVENT_TABLE()

OCPNChartDirPanel::OCPNChartDirPanel(wxWindow* parent, wxWindowID id,
                                     const wxPoint& pos, const wxSize& size,
                                     ChartDirInfo& cdi)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE) {
  m_pChartDir = cdi.fullpath;

  // On Android, shorten the displayed path name by removing well-known prefix
  if (cdi.fullpath.StartsWith(
          "/storage/emulated/0/Android/data/org.opencpn.opencpn/files"))
    m_pChartDir = "..." + cdi.fullpath.Mid(58);

  m_cdi = cdi;
  m_bSelected = false;

  m_refHeight = GetCharHeight();

  m_unselectedHeight = 2 * m_refHeight;

  // #ifdef __OCPN__ANDROID__
  //     m_unselectedHeight = 2 * m_refHeight;
  // #endif

  SetMinSize(wxSize(-1, m_unselectedHeight));

  Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(OCPNChartDirPanel::OnClickDown),
          NULL, this);
#ifdef __OCPN__ANDROID__
  Connect(wxEVT_LEFT_UP, wxMouseEventHandler(OCPNChartDirPanel::OnClickUp),
          NULL, this);
#endif
}

OCPNChartDirPanel::~OCPNChartDirPanel() {}

static wxStopWatch swclick;
#ifdef __OCPN__ANDROID__
static int downx, downy;
#endif

void OCPNChartDirPanel::OnClickDown(wxMouseEvent& event) {
#ifdef __OCPN__ANDROID__
  swclick.Start();
  event.GetPosition(&downx, &downy);
#else
  DoChartSelected();
#endif
}

void OCPNChartDirPanel::OnClickUp(wxMouseEvent& event) {
#ifdef __OCPN__ANDROID__
  qDebug() << swclick.Time();
  if (swclick.Time() < 200) {
    int upx, upy;
    event.GetPosition(&upx, &upy);
    if ((fabs(upx - downx) < GetCharWidth()) &&
        (fabs(upy - downy) < GetCharWidth())) {
      DoChartSelected();
    }
  }
  swclick.Start();
#endif
}

void OCPNChartDirPanel::DoChartSelected() {
  if (!m_bSelected) {
    SetSelected(true);
    //        m_pContainer->SelectChart( this );
  } else {
    SetSelected(false);
    //        m_pContainer->SelectChart( (OCPNChartDirPanel*)NULL );
  }
}

void OCPNChartDirPanel::SetSelected(bool selected) {
  m_bSelected = selected;
  wxColour colour;

  if (selected) {
    GetGlobalColor(_T("UIBCK"), &colour);
    m_boxColour = colour;
  } else {
    GetGlobalColor(_T("DILG0"), &colour);
    m_boxColour = colour;
  }

  Refresh(true);

  g_pOptions->SetDirActionButtons();
}

// void OCPNChartDirPanel::OnEraseBackground( wxEraseEvent &event )
// {
// }

void OCPNChartDirPanel::OnSize(wxSizeEvent& event) {
  if (m_pChartDir.Length()) {
    int x, y;
    GetClientSize(&x, &y);

    ChartDirPanelHardBreakWrapper wrapper(this, m_pChartDir, x * 9 / 10);
    wxArrayString nameWrapped = wrapper.GetLineArray();

    SetMinSize(wxSize(-1, (nameWrapped.GetCount() + 1) * m_refHeight));
  }

  event.Skip();
}

void OCPNChartDirPanel::OnPaint(wxPaintEvent& event) {
  int width, height;
  GetSize(&width, &height);
  wxPaintDC dc(this);

  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(wxBrush(GetBackgroundColour()));
  dc.DrawRectangle(GetVirtualSize());

  wxColour c;

  wxString nameString = m_pChartDir;
  ChartDirPanelHardBreakWrapper wrapper(this, nameString, width * 9 / 10);
  wxArrayString nameWrapped = wrapper.GetLineArray();

  if (height < (int)(nameWrapped.GetCount() + 1) * m_refHeight) {
    SetMinSize(wxSize(-1, (nameWrapped.GetCount() + 1) * m_refHeight));
    GetParent()->GetSizer()->Layout();
  }

  if (m_bSelected) {
    dc.SetBrush(wxBrush(m_boxColour));

    GetGlobalColor(_T ( "UITX1" ), &c);
    dc.SetPen(wxPen(wxColor(0xCE, 0xD5, 0xD6), 3));

    dc.DrawRoundedRectangle(0, 0, width - 1, height - 1, height / 10);

    int offset = height / 10;
    int text_x = offset * 2;

    wxFont* dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
    dc.SetFont(*dFont);

    dc.SetTextForeground(wxColour(64, 64, 64));

    int yd = height * 20 / 100;
    for (size_t i = 0; i < nameWrapped.GetCount(); i++) {
      if (i == 0)
        dc.DrawText(nameWrapped[i], text_x, yd);
      else
        dc.DrawText(nameWrapped[i], text_x + GetCharWidth(), yd);
      yd += GetCharHeight();
    }
  }  // selected
  else {
    dc.SetBrush(wxBrush(m_boxColour));

    GetGlobalColor(_T ( "GREY1" ), &c);
    dc.SetPen(wxPen(c, 1));

    int offset = height / 10;
    dc.DrawRoundedRectangle(offset, offset, width - (2 * offset),
                            height - (2 * offset), height / 10);

    int text_x = offset * 2;

    wxFont* dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
    dc.SetFont(*dFont);

    dc.SetTextForeground(wxColour(64, 64, 64));

    int yd = height * 20 / 100;
    for (size_t i = 0; i < nameWrapped.GetCount(); i++) {
      if (i == 0)
        dc.DrawText(nameWrapped[i], text_x, yd);
      else
        dc.DrawText(nameWrapped[i], text_x + GetCharWidth(), yd);
      yd += GetCharHeight();
    }

  }  // not selected
}

/////////////////////////////////////////////////////////////////////////////////////


static bool LoadAllPlugIns(bool load_enabled) {
  g_Platform->ShowBusySpinner();
  bool b = PluginLoader::getInstance()->LoadAllPlugIns(load_enabled);
  g_Platform->HideBusySpinner();
  return b;
}

WX_DECLARE_LIST(wxCheckBox, CBList);

class OCPNCheckedListCtrl : public wxScrolledWindow {
public:
  OCPNCheckedListCtrl() {}

  OCPNCheckedListCtrl(wxWindow* parent, wxWindowID id = -1,
                      const wxPoint& pt = wxDefaultPosition,
                      const wxSize& sz = wxDefaultSize,
                      long style = wxHSCROLL | wxVSCROLL,
                      const wxString& name = _T("scrolledWindow")) {
    Create(parent, id, pt, sz, style, name);
  }

  bool Create(wxWindow* parent, wxWindowID id = -1,
              const wxPoint& pt = wxDefaultPosition,
              const wxSize& sz = wxDefaultSize,
              long style = wxHSCROLL | wxVSCROLL,
              const wxString& name = _T("scrolledWindow"));

  virtual ~OCPNCheckedListCtrl() {}

  unsigned int Append(wxString& label, bool benable = true);
  unsigned int GetCount() { return m_list.GetCount(); }

  void Clear();
  void Check(int index, bool val);
  bool IsChecked(int index);

private:
  wxBoxSizer* m_sizer;

  CBList m_list;
};

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(CBList);

bool OCPNCheckedListCtrl::Create(wxWindow* parent, wxWindowID id,
                                 const wxPoint& pt, const wxSize& sz,
                                 long style, const wxString& name) {
  if (!wxScrolledWindow::Create(parent, id, pt, sz, style, name)) return FALSE;

#ifdef __OCPN__ANDROID__
  GetHandle()->setObjectName("OCPNCheckedListCtrl");
  GetHandle()->setStyleSheet(getAdjustedDialogStyleSheet());
#endif

  SetScrollRate(0, 2);
  m_sizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_sizer);

  return TRUE;
}

unsigned int OCPNCheckedListCtrl::Append(wxString& label, bool benable) {
  wxCheckBox* cb = new wxCheckBox(this, wxID_ANY, label);
  cb->Enable(benable);
  cb->SetValue(!benable);
  m_sizer->Add(cb);
  m_sizer->Layout();

  m_list.Append(cb);

  return m_list.GetCount() - 1;
}

void OCPNCheckedListCtrl::Check(int index, bool val) {
  CBList::Node* node = m_list.Item(index);
  wxCheckBox* cb = node->GetData();

  if (cb) cb->SetValue(val);
}

bool OCPNCheckedListCtrl::IsChecked(int index) {
  CBList::Node* node = m_list.Item(index);
  wxCheckBox* cb = node->GetData();

  if (cb)
    return cb->GetValue();
  else
    return false;
}

void OCPNCheckedListCtrl::Clear() {
  for (unsigned int i = 0; i < m_list.GetCount(); i++) {
    wxCheckBox* cb = m_list[i];
    delete cb;
  }
  m_list.Clear();
  Scroll(0, 0);
}

// Helper for conditional file name separator
void appendOSDirSlash(wxString* pString);

extern ArrayOfMMSIProperties g_MMSI_Props_Array;

///////////////////////////////////////////////////////////////////////////////
/// Class ConfigCreateDialog
///////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ConfigCreateDialog, wxDialog)
EVT_BUTTON(ID_CONFIGEDIT_CANCEL, ConfigCreateDialog::OnConfigEditCancelClick)
EVT_BUTTON(ID_CONFIGEDIT_OK, ConfigCreateDialog::OnConfigEditOKClick)
END_EVENT_TABLE()

ConfigCreateDialog::ConfigCreateDialog(wxWindow* parent, wxWindowID id,
                                       const wxString& caption,
                                       const wxPoint& pos, const wxSize& size,
                                       long style)
    : wxDialog(parent, id, caption, pos, size,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
  CreateControls();
  GetSizer()->SetSizeHints(this);
  Centre();
}

ConfigCreateDialog::~ConfigCreateDialog(void) {}

void ConfigCreateDialog::CreateControls(void) {
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  mainSizer->Add(new wxStaticText(this, wxID_STATIC, _("Title")), 0,
                 wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  m_TitleCtl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                              wxSize(40 * GetCharHeight(), -1), 0);
  mainSizer->Add(m_TitleCtl, 0,
                 wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

  mainSizer->AddSpacer(2 * GetCharHeight());

  mainSizer->Add(new wxStaticText(this, wxID_STATIC, _("Description")), 0,
                 wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  m_DescriptionCtl =
      new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxSize(-1, 6 * GetCharHeight()), wxTE_MULTILINE);
  mainSizer->Add(m_DescriptionCtl, 0,
                 wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

  mainSizer->AddSpacer(2 * GetCharHeight());

  mainSizer->Add(
      new wxStaticText(
          this, wxID_STATIC,
          _("Create a private configuration template based on current settings.\n\
This template will be saved, and may be selected for further use at any time.\n\
               ")),
      0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  mainSizer->AddSpacer(2 * GetCharHeight());

  wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(this, ID_CONFIGEDIT_CANCEL, _("Cancel"));
  btnSizer->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(this, ID_CONFIGEDIT_OK, _("OK"));
  btnSizer->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();
}

void ConfigCreateDialog::OnConfigEditCancelClick(wxCommandEvent& event) {
  EndModal(wxID_CANCEL);
}

void ConfigCreateDialog::OnConfigEditOKClick(wxCommandEvent& event) {
  const wxString& title = m_TitleCtl->GetValue();
  const wxString& desc = m_DescriptionCtl->GetValue();
  m_createdTemplateGUID =
      ConfigMgr::Get().CreateNamedConfig(title, desc, _T(""));
  EndModal(wxID_OK);
}

///////////////////////////////////////////////////////////////////////////////
/// Class MMSIEditDialog
///////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MMSIEditDialog, wxDialog)
EVT_BUTTON(ID_MMSIEDIT_CANCEL, MMSIEditDialog::OnMMSIEditCancelClick)
EVT_BUTTON(ID_MMSIEDIT_OK, MMSIEditDialog::OnMMSIEditOKClick)
END_EVENT_TABLE()

MMSIEditDialog::MMSIEditDialog(MMSIProperties* props, wxWindow* parent,
                               wxWindowID id, const wxString& caption,
                               const wxPoint& pos, const wxSize& size,
                               long style)
    : wxDialog(parent, id, caption, pos, size,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_props(props) {
  CreateControls();
  GetSizer()->SetSizeHints(this);
  Centre();
}

MMSIEditDialog::~MMSIEditDialog(void) { delete m_MMSICtl; }

void MMSIEditDialog::CreateControls(void) {
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  wxStaticBox* mmsiBox =
      new wxStaticBox(this, wxID_ANY, _("MMSI Extended Properties"));

  wxStaticBoxSizer* mmsiSizer = new wxStaticBoxSizer(mmsiBox, wxVERTICAL);
  mainSizer->Add(mmsiSizer, 0, wxEXPAND | wxALL, 5);

  mmsiSizer->Add(new wxStaticText(this, wxID_STATIC, _("MMSI")), 0,
                 wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  m_MMSICtl = new wxTextCtrl(this, ID_MMSI_CTL, wxEmptyString,
                             wxDefaultPosition, wxSize(180, -1), 0);
  mmsiSizer->Add(m_MMSICtl, 0,
                 wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

  wxStaticBoxSizer* trackSizer = new wxStaticBoxSizer(
      new wxStaticBox(this, wxID_ANY, _("Tracking")), wxVERTICAL);

  wxGridSizer* gridSizer = new wxGridSizer(0, 3, 0, 0);

  m_rbTypeTrackDefault =
      new wxRadioButton(this, wxID_ANY, _("Default tracking"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  m_rbTypeTrackDefault->SetValue(TRUE);
  gridSizer->Add(m_rbTypeTrackDefault, 0, wxALL, 5);

  m_rbTypeTrackAlways = new wxRadioButton(this, wxID_ANY, _("Always track"));
  gridSizer->Add(m_rbTypeTrackAlways, 0, wxALL, 5);

  m_rbTypeTrackNever = new wxRadioButton(this, wxID_ANY, _(" Never track"));
  gridSizer->Add(m_rbTypeTrackNever, 0, wxALL, 5);

  m_cbTrackPersist = new wxCheckBox(this, wxID_ANY, _("Persistent"));
  gridSizer->Add(m_cbTrackPersist, 0, wxALL, 5);

  trackSizer->Add(gridSizer, 0, wxEXPAND, 0);
  mmsiSizer->Add(trackSizer, 0, wxEXPAND, 0);

  m_IgnoreButton = new wxCheckBox(this, wxID_ANY, _("Ignore this MMSI"));
  mmsiSizer->Add(m_IgnoreButton, 0, wxEXPAND, 5);

  m_MOBButton = new wxCheckBox(this, wxID_ANY,
                               _("Handle this MMSI as SART/PLB(AIS) MOB."));
  mmsiSizer->Add(m_MOBButton, 0, wxEXPAND, 5);

  m_VDMButton =
      new wxCheckBox(this, wxID_ANY, _("Convert AIVDM to AIVDO for this MMSI"));
  mmsiSizer->Add(m_VDMButton, 0, wxEXPAND, 5);

  m_FollowerButton = new wxCheckBox(
      this, wxID_ANY, _("This MMSI is my Follower - No CPA Alert"));
  mmsiSizer->Add(m_FollowerButton, 0, wxEXPAND, 5);

  wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(this, ID_MMSIEDIT_CANCEL, _("Cancel"));
  btnSizer->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(this, ID_MMSIEDIT_OK, _("OK"));
  btnSizer->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();

  //  Set initial values...
  wxString sMMSI;
  if (m_props->MMSI > 0)
    sMMSI.Printf(_T("%d"), m_props->MMSI);
  else
    sMMSI = _T("");
  m_MMSICtl->AppendText(sMMSI);

  switch (m_props->TrackType) {
    case TRACKTYPE_ALWAYS:
      m_rbTypeTrackAlways->SetValue(TRUE);
      break;
    case TRACKTYPE_NEVER:
      m_rbTypeTrackNever->SetValue(TRUE);
      break;
    default:
      break;
  }

  m_cbTrackPersist->SetValue(m_props->m_bPersistentTrack);
  m_IgnoreButton->SetValue(m_props->m_bignore);
  m_MOBButton->SetValue(m_props->m_bMOB);
  m_VDMButton->SetValue(m_props->m_bVDM);
  m_FollowerButton->SetValue(m_props->m_bFollower);

  SetColorScheme(GLOBAL_COLOR_SCHEME_RGB);
}

void MMSIEditDialog::SetColorScheme(ColorScheme cs) { DimeControl(this); }

void MMSIEditDialog::OnMMSIEditCancelClick(wxCommandEvent& event) {
  EndModal(wxID_CANCEL);
}

void MMSIEditDialog::Persist() {
  if (m_props) {
    if (m_rbTypeTrackDefault->GetValue())
      m_props->TrackType = TRACKTYPE_DEFAULT;
    else if (m_rbTypeTrackAlways->GetValue())
      m_props->TrackType = TRACKTYPE_ALWAYS;
    else
      m_props->TrackType = TRACKTYPE_NEVER;

    m_props->m_bignore = m_IgnoreButton->GetValue();
    m_props->m_bMOB = m_MOBButton->GetValue();
    m_props->m_bVDM = m_VDMButton->GetValue();
    m_props->m_bFollower = m_FollowerButton->GetValue();
    m_props->m_bPersistentTrack = m_cbTrackPersist->GetValue();
    if (m_props->m_ShipName == wxEmptyString) {
      AIS_Target_Data* proptarget =
          g_pAIS->Get_Target_Data_From_MMSI(m_props->MMSI);
      if (proptarget) {
        wxString s = proptarget->GetFullName();
        m_props->m_ShipName = s;
      } else {
        wxString GetShipNameFromFile(int);
        m_props->m_ShipName = GetShipNameFromFile(m_props->MMSI);
      }
    }
  }
}

void MMSIEditDialog::OnMMSIEditOKClick(wxCommandEvent& event) {
  // Update the MMSIProperties by the passed pointer
  if (m_props) {
    long nmmsi;
    m_MMSICtl->GetValue().ToLong(&nmmsi);
    m_props->MMSI = nmmsi;
    Persist();

    if (m_MMSICtl->GetValue().Length() != 9) {
      OCPNMessageDialog* dlg = new OCPNMessageDialog(
          this,
          _("An MMSI Id is generally a number of nine digits.\nPlease check "
            "your entries and cancel if necessary."),
          _("OpenCPN Info"), wxOK | wxCANCEL);

      dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
          Persist();
        }
        EndModal(retcode);
      });
    } else {
      EndModal(wxID_OK);
    }
  }
}

void MMSIEditDialog::OnCtlUpdated(wxCommandEvent& event) {}

BEGIN_EVENT_TABLE(MMSIListCtrl, wxListCtrl)
EVT_LIST_ITEM_SELECTED(ID_MMSI_PROPS_LIST, MMSIListCtrl::OnListItemClick)
EVT_LIST_ITEM_ACTIVATED(ID_MMSI_PROPS_LIST, MMSIListCtrl::OnListItemActivated)
EVT_LIST_ITEM_RIGHT_CLICK(ID_MMSI_PROPS_LIST,
                          MMSIListCtrl::OnListItemRightClick)
EVT_MENU(ID_DEF_MENU_MMSI_EDIT, MMSIListCtrl::PopupMenuHandler)
EVT_MENU(ID_DEF_MENU_MMSI_DELETE, MMSIListCtrl::PopupMenuHandler)
END_EVENT_TABLE()

MMSIListCtrl::MMSIListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                           const wxSize& size, long style)
    : wxListCtrl(parent, id, pos, size, style) {
  m_parent = parent;
}

MMSIListCtrl::~MMSIListCtrl(void) {}

wxString MMSIListCtrl::OnGetItemText(long item, long column) const {
  wxString ret;
  MMSIProperties* props = g_MMSI_Props_Array[item];

  if (!props) return ret;
  switch (column) {
    case mlMMSI:
      if (props->MMSI > 0) ret = wxString::Format(_T( "%d" ), props->MMSI);
      break;
    case mlTrackMode:
      switch (props->TrackType) {
        case TRACKTYPE_DEFAULT:
          ret = _("Default");
          break;
        case TRACKTYPE_ALWAYS:
          ret = _("Always");
          break;
        case TRACKTYPE_NEVER:
          ret = _("Never");
          break;
        default:
          ret = _T( "???" );
      }
      if (props->m_bPersistentTrack)
        ret.Append(_T( ", " )).Append(_("Persistent"));
      break;
    case mlIgnore:
      if (props->m_bignore) ret = _T( "X" );
      break;
    case mlMOB:
      if (props->m_bMOB) ret = _T( "X" );
      break;
    case mlVDM:
      if (props->m_bVDM) ret = _T( "X" );
      break;
    case mlFollower:
      if (props->m_bFollower) ret = _T("X");
      break;
    case mlShipName:
      ret = props->m_ShipName;
      break;
    default:
      ret = _T( "??" );
      break;
  }
  return ret;
}

void MMSIListCtrl::OnListItemClick(wxListEvent& event) {}

void MMSIListCtrl::OnListItemActivated(wxListEvent& event) {
  MMSIProperties* props = g_MMSI_Props_Array.Item(event.GetIndex());
  MMSIProperties* props_new = new MMSIProperties(*props);

  MMSIEditDialog* pd =
      new MMSIEditDialog(props_new, m_parent, -1, _("Edit MMSI Properties"),
                         wxDefaultPosition, wxSize(200, 200));

  if (pd->ShowModal() == wxID_OK) {
    g_MMSI_Props_Array.RemoveAt(event.GetIndex());
    delete props;
    g_MMSI_Props_Array.Insert(props_new, event.GetIndex());
  } else
    delete props_new;

  pd->Destroy();
}

void MMSIListCtrl::OnListItemRightClick(wxListEvent& event) {
  m_context_item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (m_context_item == wxNOT_FOUND) return;
  wxMenu* menu = new wxMenu(_("MMSI Properties"));
  wxMenuItem* item_edit =
      new wxMenuItem(menu, ID_DEF_MENU_MMSI_EDIT, _("Edit") + _T("..."));
  menu->Append(item_edit);
  wxMenuItem* item_delete =
      new wxMenuItem(menu, ID_DEF_MENU_MMSI_DELETE, _("Delete"));
  menu->Append(item_delete);

#ifdef __WXMSW__
  wxFont* qFont = GetOCPNScaledFont(_("Menu"));
  item_edit->SetFont(*qFont);
  item_delete->SetFont(*qFont);
#endif

  wxPoint p = ScreenToClient(wxGetMousePosition());
  PopupMenu(menu, p.x, p.y);

  SetItemCount(g_MMSI_Props_Array.GetCount());
  Refresh(TRUE);
}

void MMSIListCtrl::PopupMenuHandler(wxCommandEvent& event) {
  int context_item = m_context_item;
  MMSIProperties* props = g_MMSI_Props_Array[context_item];

  if (!props) return;

  switch (event.GetId()) {
    case ID_DEF_MENU_MMSI_EDIT: {
      MMSIProperties* props_new = new MMSIProperties(*props);
      MMSIEditDialog* pd =
          new MMSIEditDialog(props_new, m_parent, -1, _("Edit MMSI Properties"),
                             wxDefaultPosition, wxSize(200, 200));

      if (pd->ShowModal() == wxID_OK) {
        g_MMSI_Props_Array.RemoveAt(context_item);
        delete props;
        props_new->m_ShipName = GetShipNameFromFile(props_new->MMSI);
        g_MMSI_Props_Array.Insert(props_new, context_item);
      } else {
        delete props_new;
      }
      pd->Destroy();
      break;
    }
    case ID_DEF_MENU_MMSI_DELETE:
      g_MMSI_Props_Array.RemoveAt(context_item);
      delete props;
      break;
  }
}

MMSI_Props_Panel::MMSI_Props_Panel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
              wxBORDER_NONE) {
  m_pparent = parent;

  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(topSizer);

  wxString MMSI_props_column_spec = _T("120;120;100;100;100;100;100;100");
  //  Parse the global column width string as read from config file
  wxStringTokenizer tkz(MMSI_props_column_spec, _T(";"));
  wxString s_width = tkz.GetNextToken();
  int width;
  long lwidth;

  m_pListCtrlMMSI = new MMSIListCtrl(
      this, ID_MMSI_PROPS_LIST, wxDefaultPosition, wxSize(-1, -1),
      wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES |
          wxBORDER_SUNKEN | wxLC_VIRTUAL);
  // wxImageList* imglist = new wxImageList(16, 16, TRUE, 2);

  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
  // imglist->Add(style->GetIcon(_T( "sort_asc" )));
  // imglist->Add(style->GetIcon(_T( "sort_desc" )));

  // m_pListCtrlMMSI->AssignImageList( imglist, wxIMAGE_LIST_SMALL );
  int dx = GetCharWidth();

  width = dx * 5;
  if (s_width.ToLong(&lwidth)) {
    width = wxMax(dx * 2, lwidth);
    width = wxMin(width, dx * 13);
  }
  m_pListCtrlMMSI->InsertColumn(tlMMSI, _("MMSI"), wxLIST_FORMAT_LEFT, width);

  s_width = tkz.GetNextToken();
  width = dx * 12;
  if (s_width.ToLong(&lwidth)) {
    width = wxMax(dx * 2, lwidth);
    width = wxMin(width, dx * 25);
  }
  m_pListCtrlMMSI->InsertColumn(tlCLASS, _("Track Mode"), wxLIST_FORMAT_CENTER,
                                width);

  s_width = tkz.GetNextToken();
  width = dx * 8;
  if (s_width.ToLong(&lwidth)) {
    width = wxMax(dx * 2, lwidth);
    width = wxMin(width, dx * 10);
  }
  m_pListCtrlMMSI->InsertColumn(tlTYPE, _("Ignore"), wxLIST_FORMAT_CENTER,
                                width);

  s_width = tkz.GetNextToken();
  width = dx * 8;
  if (s_width.ToLong(&lwidth)) {
    width = wxMax(dx * 2, lwidth);
    width = wxMin(width, dx * 10);
  }
  m_pListCtrlMMSI->InsertColumn(tlTYPE, _("MOB"), wxLIST_FORMAT_CENTER, width);

  s_width = tkz.GetNextToken();
  width = dx * 8;
  if (s_width.ToLong(&lwidth)) {
    width = wxMax(dx * 2, lwidth);
    width = wxMin(width, dx * 15);
  }
  m_pListCtrlMMSI->InsertColumn(tlTYPE, _("VDM->VDO"), wxLIST_FORMAT_CENTER,
                                width);

  s_width = tkz.GetNextToken();
  width = dx * 8;
  if (s_width.ToLong(&lwidth)) {
    width = wxMax(dx * 2, lwidth);
    width = wxMin(width, dx * 30);
  }
  m_pListCtrlMMSI->InsertColumn(tlTYPE, _("Ship name"), wxLIST_FORMAT_CENTER,
                                width);

  s_width = tkz.GetNextToken();
  width = dx * 8;
  if (s_width.ToLong(&lwidth)) {
    width = wxMax(dx * 2, lwidth);
    width = wxMin(width, dx * 10);
  }
  m_pListCtrlMMSI->InsertColumn(tlTYPE, _("Follower"), wxLIST_FORMAT_CENTER,
                                width);  // Has

  topSizer->Add(m_pListCtrlMMSI, 1, wxEXPAND | wxALL, 0);

  m_pButtonNew = new wxButton(this, wxID_ANY, _("New..."), wxDefaultPosition,
                              wxSize(200, -1));
  m_pButtonNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(MMSI_Props_Panel::OnNewButton),
                        NULL, this);
  topSizer->Add(m_pButtonNew, 0, wxALIGN_RIGHT | wxALL, 0);

  topSizer->Layout();

  //  This is silly, but seems to be required for __WXMSW__ build
  //  If not done, the SECOND invocation of the panel fails to expand the list
  //  to the full wxSizer size....
  SetSize(GetSize().x, GetSize().y - 1);

  SetColorScheme(GLOBAL_COLOR_SCHEME_RGB);
}

MMSI_Props_Panel::~MMSI_Props_Panel(void) {}

void MMSI_Props_Panel::OnNewButton(wxCommandEvent& event) {
  MMSIProperties* props = new MMSIProperties(-1);

  MMSIEditDialog* pd =
      new MMSIEditDialog(props, m_parent, -1, _("Add MMSI Properties"),
                         wxDefaultPosition, wxSize(200, 200));

  DimeControl(pd);
  pd->ShowWindowModalThenDo([this, pd, props](int retcode) {
    if (retcode == wxID_OK) {
      g_MMSI_Props_Array.Add(props);
    } else {
      delete props;
    }
    UpdateMMSIList();
  });
}

void MMSI_Props_Panel::UpdateMMSIList(void) {
  // Capture the MMSI of the curently selected list item
  long selItemID = wxNOT_FOUND;
  m_pListCtrlMMSI->GetNextItem(selItemID, wxLIST_NEXT_ALL,
                               wxLIST_STATE_SELECTED);

  int selMMSI = wxNOT_FOUND;
  if (selItemID != wxNOT_FOUND) selMMSI = g_MMSI_Props_Array[selItemID]->MMSI;

  m_pListCtrlMMSI->SetItemCount(g_MMSI_Props_Array.GetCount());

  // Restore selected item
  long item_sel = wxNOT_FOUND;
  if (selItemID != wxNOT_FOUND && selMMSI != wxNOT_FOUND) {
    for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
      if (g_MMSI_Props_Array[i]->MMSI == selMMSI) {
        item_sel = i;
        break;
      }
    }
  }

  if (g_MMSI_Props_Array.GetCount() > 0)
    m_pListCtrlMMSI->SetItemState(item_sel,
                                  wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                  wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

#ifdef __WXMSW__
  m_pListCtrlMMSI->Refresh(FALSE);
#endif
}

void MMSI_Props_Panel::SetColorScheme(ColorScheme cs) { DimeControl(this); }

// WX_DECLARE_OBJARRAY(wxBitmap,      ArrayOfBitmaps);
//#include <wx/arrimpl.cpp>
// WX_DEFINE_OBJARRAY(ArrayOfBitmaps);

class OCPNFatCombo : public wxOwnerDrawnComboBox {
public:
  OCPNFatCombo();

  OCPNFatCombo(wxWindow* parent, wxWindowID id, const wxString& value = _T(""),
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize, int n = 0,
               const wxString choices[] = NULL, long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = _T("OCPNFatCombo"));

  ~OCPNFatCombo();

  void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
  wxCoord OnMeasureItem(size_t item) const;
  wxCoord OnMeasureItemWidth(size_t item) const;
  bool SetFont(const wxFont& font);

  int Append(const wxString& item, wxBitmap bmp);
  void Clear(void);

  const wxFont* dfont;

private:
  int itemHeight;
  ArrayOfBitmaps bmpArray;
};

OCPNFatCombo::OCPNFatCombo() : wxOwnerDrawnComboBox() {}

OCPNFatCombo::OCPNFatCombo(wxWindow* parent, wxWindowID id,
                           const wxString& value, const wxPoint& pos,
                           const wxSize& size, int n, const wxString choices[],
                           long style, const wxValidator& validator,
                           const wxString& name)
    : wxOwnerDrawnComboBox(parent, id, value, pos, size, n, choices, style,
                           validator, name) {
  double fontHeight =
      GetFont().GetPointSize() / g_Platform->getFontPointsperPixel();
  itemHeight = (int)wxRound(fontHeight);
  SetPopupMaxHeight(::wxGetDisplaySize().y / 2);
}

OCPNFatCombo::~OCPNFatCombo() {}

bool OCPNFatCombo::SetFont(const wxFont& font) {
  dfont = &font;
  return true;
}

void OCPNFatCombo::OnDrawItem(wxDC& dc, const wxRect& rect, int item,
                              int flags) const {
  int offset_x = 10;
  //    dc.DrawBitmap(bmpArray.Item(item), rect.x, rect.y + (rect.height -
  //    bmpHeight)/2, true);
  dc.SetFont(*dfont);

  wxColor bg = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);
  wxBrush br = wxBrush(bg);
  wxBrush sv = dc.GetBrush();
  dc.SetBrush(br);
  dc.DrawRectangle(0, 0, rect.width, rect.height);
  dc.SetBrush(sv);
  dc.SetTextForeground(wxColour(0, 0, 0));

  if (flags & wxODCB_PAINTING_CONTROL) {
    wxString text = GetValue();
    int margin_x = 2;
    dc.DrawText(text, rect.x + margin_x + offset_x,
                (rect.height - dc.GetCharHeight()) / 2 + rect.y);
  } else {
    dc.SetBackground(wxBrush(br));
    dc.Clear();
    dc.DrawText(GetVListBoxComboPopup()->GetString(item), rect.x + 2 + offset_x,
                (rect.height - dc.GetCharHeight()) / 2 + rect.y);
  }
}

wxCoord OCPNFatCombo::OnMeasureItem(size_t item) const {
  if (item < bmpArray.GetCount())
    return wxMax(itemHeight, bmpArray.Item(item).GetHeight());

  return itemHeight * 12 / 10;
}

wxCoord OCPNFatCombo::OnMeasureItemWidth(size_t item) const { return -1; }

int OCPNFatCombo::Append(const wxString& item, wxBitmap bmp) {
  bmpArray.Add(bmp);
  int idx = wxOwnerDrawnComboBox::Append(item);

  return idx;
}

void OCPNFatCombo::Clear(void) {
  wxOwnerDrawnComboBox::Clear();
  bmpArray.Clear();
}

BEGIN_EVENT_TABLE(options, wxDialog)
EVT_INIT_DIALOG(options::OnDialogInit)
EVT_CHECKBOX(ID_DEBUGCHECKBOX1, options::OnDebugcheckbox1Click)
EVT_BUTTON(ID_BUTTONADD, options::OnButtonaddClick)
EVT_BUTTON(ID_BUTTONDELETE, options::OnButtondeleteClick)
EVT_BUTTON(ID_PARSEENCBUTTON, options::OnButtonParseENC)
EVT_BUTTON(ID_BUTTONCOMPRESS, options::OnButtoncompressClick)
EVT_BUTTON(ID_BUTTONMIGRATE, options::OnButtonmigrateClick)
EVT_BUTTON(ID_TCDATAADD, options::OnInsertTideDataLocation)
EVT_BUTTON(ID_TCDATADEL, options::OnRemoveTideDataLocation)
EVT_BUTTON(ID_APPLY, options::OnApplyClick)
EVT_BUTTON(xID_OK, options::OnXidOkClick)
EVT_BUTTON(wxID_CANCEL, options::OnCancelClick)
EVT_BUTTON(ID_BUTTONFONTCHOOSE, options::OnChooseFont)
EVT_CHOICE(ID_CHOICE_FONTELEMENT, options::OnFontChoice)
EVT_CLOSE(options::OnClose)

#if defined(__WXGTK__) || defined(__WXQT__)
EVT_BUTTON(ID_BUTTONFONTCOLOR, options::OnChooseFontColor)
#endif
#ifdef ocpnUSE_GL
EVT_BUTTON(ID_OPENGLOPTIONS, options::OnOpenGLOptions)
#endif
EVT_CHOICE(ID_RADARDISTUNIT, options::OnDisplayCategoryRadioButton)
EVT_CHOICE(ID_DEPTHUNITSCHOICE, options::OnUnitsChoice)
EVT_BUTTON(ID_CLEARLIST, options::OnButtonClearClick)
EVT_BUTTON(ID_SELECTLIST, options::OnButtonSelectClick)
EVT_BUTTON(ID_SETSTDLIST, options::OnButtonSetStd)
EVT_CHECKBOX(ID_SHOWGPSWINDOW, options::OnShowGpsWindowCheckboxClick)
EVT_CHOICE(ID_SHIPICONTYPE, options::OnShipTypeSelect)
EVT_CHOICE(ID_RADARRINGS, options::OnRadarringSelect)
EVT_CHOICE(ID_OPWAYPOINTRANGERINGS, options::OnWaypointRangeRingSelect)
EVT_CHAR_HOOK(options::OnCharHook)
EVT_TIMER(ID_BT_SCANTIMER, options::onBTScanTimer)

END_EVENT_TABLE()

options::options(MyFrame* parent, wxWindowID id, const wxString& caption,
                 const wxPoint& pos, const wxSize& size, long style) {
  Init();

  pParent = parent;

  long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
  SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

  wxDialog::Create(parent, id, caption, pos, size, wstyle);
  SetFont(*dialogFont);

  CreateControls();
  RecalculateSize();

  // Protect against unreasonable small size
  // And also handle the empty config file init case.
  if (((size.x < 200) || (size.y < 200)) && !g_bresponsive) Fit();

  Center();

  wxDEFINE_EVENT(EVT_COMPAT_OS_CHANGE, wxCommandEvent);
  GlobalVar<wxString> compat_os(&g_compatOS);
  compat_os_listener = compat_os.get_listener(this, EVT_COMPAT_OS_CHANGE);
  Bind(EVT_COMPAT_OS_CHANGE, [&](wxCommandEvent&) {
    PluginLoader::getInstance()->LoadAllPlugIns(false);
    m_pPlugInCtrl->ReloadPluginPanels();
  });
}

options::~options(void) {
  wxNotebook* nb =
      dynamic_cast<wxNotebook*>(m_pListbook->GetPage(m_pageCharts));
  if (nb)
    nb->Disconnect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                   wxListbookEventHandler(options::OnChartsPageChange), NULL,
                   this);

  groupsPanel->EmptyChartGroupArray(m_pGroupArray);
  delete groupsPanel;

  delete m_pSerialArray;
  delete m_pGroupArray;
  delete m_topImgList;

  // Take care of the plugin manager...

  delete m_pPlugInCtrl;
  if (g_pi_manager) g_pi_manager->SetListPanelPtr(NULL);
#ifndef __OCPN__ANDROID__
  delete m_PluginCatalogMgrPanel;
#endif
}

// with AIS it's called very often
#if wxCHECK_VERSION(3, 0, 0)
bool options::SendIdleEvents(wxIdleEvent& event) {
  if (IsShown()) return wxDialog::SendIdleEvents(event);
  return false;
}
#endif

void options::RecalculateSize(void) {
  if (!g_bresponsive) {
    m_nCharWidthMax = GetSize().x / GetCharWidth();
    return;
#if 0
    wxSize canvas_size = gFrame->GetSize();
    wxSize fitted_size = GetSize();

    fitted_size.x = wxMin(fitted_size.x, canvas_size.x);
    fitted_size.y = wxMin(fitted_size.y, canvas_size.y);

    SetSize(fitted_size);

    Fit();

    return;
#endif
  }

  wxSize esize;
  esize.x = GetCharWidth() * 110;
  esize.y = GetCharHeight() * 40;

  wxSize dsize = GetParent()->GetSize();  // GetClientSize();
  esize.y = wxMin(esize.y, dsize.y - 0 /*(2 * GetCharHeight())*/);
  esize.x = wxMin(esize.x, dsize.x - 0 /*(2 * GetCharHeight())*/);
  SetSize(esize);

  wxSize fsize = GetSize();
  wxSize canvas_size = GetParent()->GetSize();
  wxPoint screen_pos = GetParent()->GetScreenPosition();
  int xp = (canvas_size.x - fsize.x) / 2;
  int yp = (canvas_size.y - fsize.y) / 2;
  Move(screen_pos.x + xp, screen_pos.y + yp);

  m_nCharWidthMax = GetSize().x / GetCharWidth();
}

void options::Init(void) {
  m_pWorkDirList = NULL;

  pShowStatusBar = NULL;
  pShowMenuBar = NULL;
  pShowCompassWin = NULL;
  pSelCtl = NULL;
  // pActiveChartsList = NULL;
  m_scrollWinChartList = NULL;
  ps57CtlListBox = NULL;
  pDispCat = NULL;
  m_pSerialArray = NULL;
  pUpdateCheckBox = NULL;
  pParseENCButton = NULL;
  k_charts = 0;
  k_vectorcharts = 0;
  k_plugins = 0;
  k_tides = 0;
  m_pConfig = NULL;

  pSoundDeviceIndex = NULL;

  pCBNorthUp = NULL;
  pCBCourseUp = NULL;
  pCBLookAhead = NULL;
  pCDOQuilting = NULL;
  pPreserveScale = NULL;
  pSmoothPanZoom = NULL;
  pEnableZoomToCursor = NULL;
  pSDisplayGrid = NULL;
  pCDOOutlines = NULL;
  pSDepthUnits = NULL;
  pSLiveETA = NULL;
  pSDefaultBoatSpeed = NULL;

  activeSizer = NULL;
  itemActiveChartStaticBox = NULL;

  pCheck_SOUNDG = NULL;
  pCheck_META = NULL;
  pCheck_SHOWIMPTEXT = NULL;
  pCheck_SCAMIN = NULL;
  pCheck_ATONTEXT = NULL;
  pCheck_LDISTEXT = NULL;
  pCheck_XLSECTTEXT = NULL;

  m_bVisitLang = FALSE;
  m_itemFontElementListBox = NULL;
  m_textSample = NULL;
  m_topImgList = NULL;

  m_pListbook = NULL;
  m_pGroupArray = NULL;
  m_groups_changed = 0;

  m_pageDisplay = -1;
  m_pageConnections = -1;
  m_pageCharts = -1;
  m_pageShips = -1;
  m_pageUI = -1;
  m_pagePlugins = -1;
  m_pageConnections = -1;

  m_buttonScanBT = 0;
  m_stBTPairs = 0;
  m_choiceBTDataSources = 0;

  auto loader = PluginLoader::getInstance();
  b_haveWMM = loader && loader->IsPlugInAvailable(_T("WMM"));
  b_oldhaveWMM = b_haveWMM;

  lastPage = 0;
  m_bneedNew = false;

  m_bForceNewToolbaronCancel = false;

  m_cs = (ColorScheme)0;

  // for deferred loading
  m_pPlugInCtrl = NULL;
  m_PluginCatalogMgrPanel = NULL;
  m_pNMEAForm = NULL;
  mSelectedConnection = NULL;

#ifdef __OCPN__ANDROID__
  m_scrollRate = 1;
#else
  m_scrollRate = 15;
#endif

  m_BTScanTimer.SetOwner(this, ID_BT_SCANTIMER);
  m_BTscanning = 0;

  dialogFont = GetOCPNScaledFont(_("Dialog"));

  dialogFontPlus = new wxFont(
      *dialogFont);  // we can't use Smaller() because wx2.8 doesn't support it
  dialogFontPlus->SetPointSize((dialogFontPlus->GetPointSize() * 1.2) +
                               0.5);  // + 0.5 to round instead of truncate
  dialogFontPlus->SetWeight(wxFONTWEIGHT_BOLD);

  m_bVectorInit = false;

  // This variable is used by plugin callback function AddOptionsPage
  g_pOptions = this;

  pCmdSoundString = NULL;

  m_sconfigSelect_single = NULL;
  m_sconfigSelect_twovertical = NULL;

  wxScreenDC dc;
  dc.SetFont(*dialogFont);
  int width, height;
  dc.GetTextExtent(_T("H"), &width, &height, NULL, NULL, dialogFont);

  m_colourPickerDefaultSize = wxSize(4 * height, height * 2);

  m_bcompact = false;

  // wxSize dSize = g_Platform->getDisplaySize();
  // if ( dSize.x < width * 40)
  //   m_bcompact = true;

  double dsizemm = g_Platform->GetDisplaySizeMM();
  if (dsizemm < 80)  // Probably and Android Phone, portrait mode
    m_bcompact = true;
}

#if defined(__GNUC__) && __GNUC__ < 8
// Handle old gcc C++-11 bugs, remove when builders updated to gcc >= 8.1.1.

static const wxString BAD_ACCESS_MSG = _(
    "The device selected is not accessible; opencpn will likely not be able\n"
    "to use this device as-is. You might want to exit OpenCPN, reboot and\n"
    "retry after creating a file called /etc/udev/rules.d/70-opencpn.rules\n"
    "with the following contents:\n\n"
    "            KERNEL==\"ttyUSB*\", MODE=\"0666\"\n"
    "            KERNEL==\"ttyACM*\", MODE=\"0666\"\n"
    "            KERNEL==\"ttyS*\", MODE=\"0666\"\n\n"
    "For more info, see the file LINUX_DEVICES.md in the distribution docs.\n");

#else

static const wxString BAD_ACCESS_MSG = _(R"(
The device selected is not accessible; opencpn will likely not be able
to use this device as-is. You might want to exit OpenCPN, reboot and
retry after creating a file called /etc/udev/rules.d/70-opencpn.rules
with the following contents:

            KERNEL=="ttyUSB*", MODE="0666"
            KERNEL=="ttyACM*", MODE="0666"
            KERNEL=="ttyS*", MODE="0666"

For more info, see the file LINUX_DEVICES.md in the distribution docs.
)");

#endif  // defined(__GNUC__) && __GNUC__ < 8

void options::OnDialogInit(wxInitDialogEvent& event) {}

void options::CheckDeviceAccess(/*[[maybe_unused]]*/ wxString& path) {}

size_t options::CreatePanel(const wxString& title) {
  size_t id = m_pListbook->GetPageCount();
  /* This is the default empty content for any top tab.
     It'll be replaced when we call AddPage */
  wxPanel* panel = new wxPanel(m_pListbook, wxID_ANY, wxDefaultPosition,
                               wxDefaultSize, wxTAB_TRAVERSAL, title);
  m_pListbook->AddPage(panel, title, FALSE, id);
  return id;
}

wxScrolledWindow* options::AddPage(size_t parent, const wxString& title) {
  if (parent > m_pListbook->GetPageCount() - 1) {
    wxLogMessage(
        wxString::Format(
            _T("Warning: invalid parent in options::AddPage( %d, "), parent) +
        title + _T(" )"));
    return NULL;
  }
  wxNotebookPage* page = m_pListbook->GetPage(parent);
  wxNotebook* nb;
  wxScrolledWindow* sw;

  int style = wxVSCROLL | wxTAB_TRAVERSAL;
  if ((nb = dynamic_cast<wxNotebook*>(page))) {
    sw = new wxScrolledWindow(page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              style);
    sw->SetScrollRate(m_scrollRate, m_scrollRate);
    nb->AddPage(sw, title);
  } else if ((sw = dynamic_cast<wxScrolledWindow*>(page))) {
    wxString toptitle = m_pListbook->GetPageText(parent);
    wxNotebook* nb = new wxNotebook(m_pListbook, wxID_ANY, wxDefaultPosition,
                                    wxDefaultSize, wxNB_TOP);
    /* Only remove the tab from listbook, we still have original content in
     * {page} */
    m_pListbook->InsertPage(parent, nb, toptitle, FALSE, parent);
    m_pListbook->SetSelection(0);  // avoid gtk assertions
    m_pListbook->RemovePage(parent + 1);
    wxString previoustitle = page->GetName();
    page->Reparent(nb);
    nb->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                wxNotebookEventHandler(options::OnSubNBPageChange), NULL, this);

    nb->AddPage(page, previoustitle);
    /* wxNotebookPage is hidden under wxGTK after RemovePage/Reparent
     * we must explicitely Show() it */
    page->Show();
    sw = new wxScrolledWindow(nb, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              style);
    sw->SetScrollRate(m_scrollRate, m_scrollRate);
    nb->AddPage(sw, title);
    nb->ChangeSelection(0);
  } else {  // This is the default content, we can replace it now
    sw = new wxScrolledWindow(m_pListbook, wxID_ANY, wxDefaultPosition,
                              wxDefaultSize, style, title);
    sw->SetScrollRate(m_scrollRate, m_scrollRate);
    wxString toptitle = m_pListbook->GetPageText(parent);
    m_pListbook->InsertPage(parent, sw, toptitle, FALSE, parent);
    m_pListbook->SetSelection(0);  // avoid gtk assertions
    m_pListbook->DeletePage(parent + 1);
  }

#ifdef __OCPN__ANDROID__
  sw->GetHandle()->setStyleSheet(getAdjustedDialogStyleSheet());
#endif

  return sw;
}

bool options::DeletePluginPage(wxScrolledWindow* page) {
  for (size_t i = 0; i < m_pListbook->GetPageCount(); i++) {
    wxNotebookPage* pg = m_pListbook->GetPage(i);
    wxNotebook* nb = dynamic_cast<wxNotebook*>(pg);

    if (nb) {
      for (size_t j = 0; j < nb->GetPageCount(); j++) {
        wxNotebookPage* spg = nb->GetPage(j);
        if (spg == page) {
          nb->DeletePage(j);
          if (nb->GetPageCount() != 1) return TRUE;
          /* There's only one page, remove inner notebook */
          spg = nb->GetPage(0);
          spg->Reparent(m_pListbook);
          nb->RemovePage(0);
          wxString toptitle = m_pListbook->GetPageText(i);
          m_pListbook->DeletePage(i);
          m_pListbook->InsertPage(i, spg, toptitle, FALSE, i);
          return TRUE;
        }
      }
    } else if (pg == page) {
      /* There's only one page, replace it with empty panel */
      m_pListbook->DeletePage(i);
      wxPanel* panel = new wxPanel(m_pListbook);
      wxString toptitle = m_pListbook->GetPageText(i);
      m_pListbook->InsertPage(i, panel, toptitle, FALSE, i);
      return TRUE;
    }
  }
  return FALSE;
}

void options::CreatePanel_NMEA_Compact(size_t parent, int border_size,
                                       int group_item_spacing) {
  m_pNMEAForm = AddPage(parent, _("NMEA"));

  wxBoxSizer* bSizer4 = new wxBoxSizer(wxVERTICAL);
  m_pNMEAForm->SetSizer(bSizer4);
  m_pNMEAForm->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizerOuterContainer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer* sbSizerGeneral;
  sbSizerGeneral = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("General")), wxVERTICAL);

  wxBoxSizer* bSizer151;
  bSizer151 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer161;
  bSizer161 = new wxBoxSizer(wxVERTICAL);

  m_cbFilterSogCog =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Filter NMEA COG/SOG"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbFilterSogCog->SetValue(g_bfilter_cogsog);
  bSizer161->Add(m_cbFilterSogCog, 0, wxALL, 5);

  wxFlexGridSizer* bSizer171 = new wxFlexGridSizer(0, 2, 0, 0);
  bSizer171->SetFlexibleDirection(wxBOTH);
  bSizer171->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stFilterSec =
      new wxStaticText(m_pNMEAForm, wxID_ANY, _("Filter period (sec)"),
                       wxDefaultPosition, wxDefaultSize, 0);
  // m_stFilterSec->Wrap( -1 );

  int nspace = 5;
#ifdef __WXGTK__
  nspace = 9;
#endif
  bSizer171->Add(m_stFilterSec, 1, wxALL, nspace);

  m_tFilterSec = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxSize(100, -1), 0);
  wxString sfilt;
  sfilt.Printf(_T( "%d" ), g_COGFilterSec);
  m_tFilterSec->SetValue(sfilt);
  bSizer171->Add(m_tFilterSec, 1, wxALL, 4);

  bSizer161->Add(bSizer171, 1, wxEXPAND, 5);

  int cb_space = 2;
  m_cbNMEADebug =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Show NMEA Debug Window"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbNMEADebug->SetValue(NMEALogWindow::Get().Active());
  bSizer161->Add(m_cbNMEADebug, 0, wxALL, cb_space);

  m_cbFurunoGP3X =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Use Furuno GP3X for uploads"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbFurunoGP3X->SetValue(g_GPS_Ident == _T( "FurunoGP3X" ));
  bSizer161->Add(m_cbFurunoGP3X, 0, wxALL, cb_space);

  m_cbGarminUploadHost =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Use GRMN for uploads"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbGarminUploadHost->SetValue(g_bGarminHostUpload);
  bSizer161->Add(m_cbGarminUploadHost, 0, wxALL, cb_space);

  m_cbAPBMagnetic =
      new wxCheckBox(m_pNMEAForm, wxID_ANY,
                     _("Use magnetic bearings in output sentence ECAPB"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbAPBMagnetic->SetValue(g_bMagneticAPB);
  bSizer161->Add(m_cbAPBMagnetic, 0, wxALL, cb_space);

  bSizer151->Add(bSizer161, 1, wxEXPAND, 5);
  sbSizerGeneral->Add(bSizer151, 1, wxEXPAND, 5);
  bSizerOuterContainer->Add(sbSizerGeneral, 0, wxALL | wxEXPAND, 5);

  //  Connections listbox, etc
  wxStaticBoxSizer* sbSizerLB = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Data Connections")),
      wxVERTICAL);

  wxBoxSizer* bSizer17;
  bSizer17 = new wxBoxSizer(wxVERTICAL);

  // m_lcSources = new wxListCtrl(m_pNMEAForm, wxID_ANY, wxDefaultPosition,
  //                             wxSize(300, m_fontHeight * 2), wxLC_REPORT |
  //                             wxLC_SINGLE_SEL);
  // bSizer17->Add(m_lcSources, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizer18;
  bSizer18 = new wxBoxSizer(wxHORIZONTAL);
  bSizer17->Add(bSizer18, 0, wxEXPAND, 5);

  m_buttonAdd = new wxButton(m_pNMEAForm, wxID_ANY, _("Add") + _T("..."),
                             wxDefaultPosition, wxDefaultSize, 0);
  bSizer18->Add(m_buttonAdd, 0, wxALL, 5);

  m_buttonRemove = new wxButton(m_pNMEAForm, wxID_ANY, _("Remove"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_buttonRemove->Enable(FALSE);
  bSizer18->Add(m_buttonRemove, 0, wxALL, 5);

  sbSizerLB->Add(bSizer17, 1, wxEXPAND, 5);
  bSizerOuterContainer->Add(sbSizerLB, 0, wxEXPAND, 5);

  //  Connections Properties
  sbSizerConnectionProps = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Properties")), wxVERTICAL);

  wxFlexGridSizer* bSizer15 = new wxFlexGridSizer(0, 2, 0, 0);
  bSizer15->SetFlexibleDirection(wxBOTH);
  bSizer15->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  sbSizerConnectionProps->Add(bSizer15, 0, wxEXPAND, 0);

  m_rbTypeSerial =
      new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Serial"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_rbTypeSerial->SetValue(TRUE);
  bSizer15->Add(m_rbTypeSerial, 0, wxALL, 5);

  m_rbTypeNet = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Network"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer15->Add(m_rbTypeNet, 0, wxALL, 5);

  if (OCPNPlatform::hasInternalGPS()) {
    m_rbTypeInternalGPS =
        new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Built-in GPS"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15->Add(m_rbTypeInternalGPS, 0, wxALL, 5);
  } else
    m_rbTypeInternalGPS = NULL;

  // has built-in Bluetooth
  if (OCPNPlatform::hasInternalBT()) {
    m_rbTypeInternalBT =
        new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Built-in Bluetooth"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15->Add(m_rbTypeInternalBT, 0, wxALL, 5);

    m_buttonScanBT =
        new wxButton(m_pNMEAForm, wxID_ANY, _("BT Scan") + _T("..."),
                     wxDefaultPosition, wxDefaultSize);
    m_buttonScanBT->Hide();
    sbSizerConnectionProps->Add(m_buttonScanBT, 0, wxALL, 5);

    wxBoxSizer* bSizer15a = new wxBoxSizer(wxHORIZONTAL);
    sbSizerConnectionProps->Add(bSizer15a, 0, wxEXPAND, 5);

    m_stBTPairs = new wxStaticText(m_pNMEAForm, wxID_ANY, _("BT Sources"),
                                   wxDefaultPosition, wxDefaultSize, 0);
    // m_stBTPairs->Wrap( -1 );
    // m_stBTPairs->Hide();
    bSizer15a->Add(m_stBTPairs, 0, wxALL, 5);

    wxArrayString mt;
    mt.Add(_T( "unscanned" ));
    m_choiceBTDataSources = new wxChoice(m_pNMEAForm, wxID_ANY,
                                         wxDefaultPosition, wxDefaultSize, mt);
    bSizer15a->Add(m_choiceBTDataSources, 1, wxEXPAND | wxTOP, 5);

#if 0
        m_BTscan_results.Clear();
        m_BTscan_results.Add(_T("None"));

        m_BTscan_results = g_Platform->getBluetoothScanResults();
        m_choiceBTDataSources->Clear();
        m_choiceBTDataSources->Append(m_BTscan_results[0]);  // scan status

        unsigned int i=1;
        while( (i+1) < m_BTscan_results.GetCount()){
            wxString item1 = m_BTscan_results[i] + _T(";");
            wxString item2 = m_BTscan_results.Item(i+1);
            m_choiceBTDataSources->Append(item1 + item2);

            i += 2;
        }

        if( m_BTscan_results.GetCount() > 1){
            m_choiceBTDataSources->SetSelection( 1 );
        }
#endif

    m_choiceBTDataSources->Hide();
    m_buttonScanBT->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                            wxCommandEventHandler(options::OnScanBTClick), NULL,
                            this);
  } else
    m_rbTypeInternalBT = NULL;

  wxBoxSizer* gSizerNetPropsV = new wxBoxSizer(wxVERTICAL);
  sbSizerConnectionProps->Add(gSizerNetPropsV, 0, wxEXPAND, 5);

  m_stNetProto = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Protocol"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  // m_stNetProto->Wrap( -1 );
  gSizerNetPropsV->Add(m_stNetProto, 0, wxALL, 5);

  wxBoxSizer* bSizer16;
  bSizer16 = new wxBoxSizer(wxHORIZONTAL);
  gSizerNetPropsV->Add(bSizer16, 1, wxEXPAND, 5);

  m_rbNetProtoTCP =
      new wxRadioButton(m_pNMEAForm, wxID_ANY, _("TCP"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_rbNetProtoTCP->Enable(TRUE);

  bSizer16->Add(m_rbNetProtoTCP, 0, wxALL, 5);

  m_rbNetProtoUDP = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("UDP"),
                                      wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoUDP->Enable(TRUE);

  bSizer16->Add(m_rbNetProtoUDP, 0, wxALL, 5);

  m_rbNetProtoGPSD = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("GPSD"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoGPSD->SetValue(TRUE);
  bSizer16->Add(m_rbNetProtoGPSD, 0, wxALL, 5);

  m_rbNetProtoSignalK = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Signal K"),
                                          wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoSignalK->Enable(TRUE);
  bSizer16->Add(m_rbNetProtoSignalK, 0, wxALL, 5);

  wxFlexGridSizer* fgSizer1a = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer1a->SetFlexibleDirection(wxBOTH);
  fgSizer1a->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
  gSizerNetPropsV->Add(fgSizer1a, 0, wxEXPAND, 5);

  m_stNetAddr = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Address"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  fgSizer1a->Add(m_stNetAddr, 0, wxALL, 5);

  m_tNetAddress =
      new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxSize(m_fontHeight * 4, -1), 0);
  fgSizer1a->Add(m_tNetAddress, 1, wxTOP | wxALIGN_RIGHT, 5);

  m_stNetPort = new wxStaticText(m_pNMEAForm, wxID_ANY, _("DataPort"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  fgSizer1a->Add(m_stNetPort, 1, wxALL, 5);

  m_tNetPort = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString,
                              wxDefaultPosition, wxSize(200, -1), 0);
  fgSizer1a->Add(m_tNetPort, 1, wxTOP | wxALIGN_RIGHT, 5);

  gSizerSerProps = new wxGridSizer(0, 1, 0, 0);

  wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer1->SetFlexibleDirection(wxBOTH);
  fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
  gSizerSerProps->Add(fgSizer1, 0, wxEXPAND, 5);

  m_stSerPort = new wxStaticText(m_pNMEAForm, wxID_ANY, _("DataPort"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  // m_stSerPort->Wrap( -1 );
  fgSizer1->Add(m_stSerPort, 0, wxALL, 5);

  m_comboPort = new wxComboBox(m_pNMEAForm, wxID_ANY, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
  fgSizer1->Add(m_comboPort, 0, wxEXPAND | wxTOP, 5);

  m_stSerBaudrate = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Baudrate"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  // m_stSerBaudrate->Wrap( -1 );
  fgSizer1->Add(m_stSerBaudrate, 0, wxALL, 5);

  wxString m_choiceBaudRateChoices[] = {
      _("150"),    _("300"),    _("600"),    _("1200"),  _("2400"),
      _("4800"),   _("9600"),   _("19200"),  _("38400"), _("57600"),
      _("115200"), _("230400"), _("460800"), _("921600")};
  int m_choiceBaudRateNChoices =
      sizeof(m_choiceBaudRateChoices) / sizeof(wxString);
  m_choiceBaudRate =
      new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceBaudRateNChoices, m_choiceBaudRateChoices, 0);
  m_choiceBaudRate->SetSelection(0);
  fgSizer1->Add(m_choiceBaudRate, 1, wxEXPAND | wxTOP, 5);

  m_stSerProtocol = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Protocol"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  // m_stSerProtocol->Wrap( -1 );
  fgSizer1->Add(m_stSerProtocol, 0, wxALL, 5);

  wxString m_choiceSerialProtocolChoices[] = {"NMEA 0183", "NMEA 2000"};
  int m_choiceSerialProtocolNChoices =
      sizeof(m_choiceSerialProtocolChoices) / sizeof(wxString);
  m_choiceSerialProtocol = new wxChoice(
      m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      m_choiceSerialProtocolNChoices, m_choiceSerialProtocolChoices, 0);
  m_choiceSerialProtocol->SetSelection(0);
  m_choiceSerialProtocol->Enable(TRUE);

  fgSizer1->Add(m_choiceSerialProtocol, 1, wxEXPAND | wxTOP, 5);
  m_stPriority = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Priority"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  // m_stPriority->Wrap( -1 );
  fgSizer1->Add(m_stPriority, 0, wxALL, 5);

  wxString m_choicePriorityChoices[] = {_("0"), _("1"), _("2"), _("3"), _("4"),
                                        _("5"), _("6"), _("7"), _("8"), _("9")};
  int m_choicePriorityNChoices =
      sizeof(m_choicePriorityChoices) / sizeof(wxString);
  m_choicePriority =
      new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choicePriorityNChoices, m_choicePriorityChoices, 0);
  m_choicePriority->SetSelection(9);
  fgSizer1->Add(m_choicePriority, 0, wxEXPAND | wxTOP, 5);

  wxBoxSizer* fgSizer5 = new wxBoxSizer(wxVERTICAL);

  m_cbCheckCRC = new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Control checksum"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_cbCheckCRC->SetValue(TRUE);
  m_cbCheckCRC->SetToolTip(
      _("If checked, only the sentences with a valid checksum are passed "
        "through"));
  fgSizer5->Add(m_cbCheckCRC, 0, wxALL, 5);

  m_cbGarminHost =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Use GRMN mode for input"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbGarminHost->SetValue(FALSE);
  fgSizer5->Add(m_cbGarminHost, 0, wxALL, 5);
#ifndef USE_GARMINHOST
// m_cbGarminHost->Hide();
#endif

  m_cbInput =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Receive Input on this Port"),
                     wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_cbInput, 0, wxALL, 5);

  m_cbOutput = new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Output on this port"),
                              wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_cbOutput, 0, wxALL, 5);

  wxFlexGridSizer* fgSizer5a = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer5a->SetFlexibleDirection(wxBOTH);
  fgSizer5a->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stTalkerIdText = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Talker ID"),
                                      wxDefaultPosition, wxDefaultSize, 0);
  // m_stTalkerIdText->Wrap( -1 );
  fgSizer5a->Add(m_stTalkerIdText, 0, wxALL, 5);

  m_TalkerIdText = new wxTextCtrl(m_pNMEAForm, ID_OPTEXTCTRL, _T(""),
                                  wxDefaultPosition, wxSize(50, -1), 0);
  m_TalkerIdText->SetMaxLength(2);
  fgSizer5a->Add(m_TalkerIdText, 0, wxALIGN_LEFT | wxALL, group_item_spacing);

  m_stPrecision =
      new wxStaticText(m_pNMEAForm, wxID_ANY, _("APB bearing precision"),
                       wxDefaultPosition, wxDefaultSize, 0);

  // m_stPrecision->Wrap( -1 );
  fgSizer5a->Add(m_stPrecision, 0, wxALL, 5);

  wxString m_choicePrecisionChoices[] = {_T("x"), _T("x.x"), _T("x.xx"),
                                         _T("x.xxx"), _T("x.xxxx")};
  int m_choicePrecisionNChoices =
      sizeof(m_choicePrecisionChoices) / sizeof(wxString);
  m_choicePrecision =
      new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choicePrecisionNChoices, m_choicePrecisionChoices, 0);
  m_choicePrecision->SetSelection(g_NMEAAPBPrecision);
  fgSizer5a->Add(m_choicePrecision, 0, wxALL, 5);

  sbSizerConnectionProps->Add(gSizerSerProps, 0, wxEXPAND, 5);
  sbSizerConnectionProps->Add(fgSizer5, 0, wxEXPAND, 5);
  sbSizerConnectionProps->Add(fgSizer5a, 0, wxEXPAND, 5);

  sbSizerInFilter = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Input filtering")), wxVERTICAL);

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxVERTICAL);

  m_rbIAccept =
      new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Accept only sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer9->Add(m_rbIAccept, 0, wxALL, 5);

  m_rbIIgnore = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Ignore sentences"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer9->Add(m_rbIIgnore, 0, wxALL, 5);

  sbSizerInFilter->Add(bSizer9, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);

  m_tcInputStc =
      new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxSize(-1, -1), wxTE_READONLY);
  bSizer11->Add(m_tcInputStc, 1, wxALL, 5);

  m_btnInputStcList =
      new wxButton(m_pNMEAForm, wxID_ANY, _T("..."), wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizer11->Add(m_btnInputStcList, 0, wxALL, 5);

  sbSizerInFilter->Add(bSizer11, 0, wxEXPAND, 5);
  sbSizerConnectionProps->Add(sbSizerInFilter, 0, wxEXPAND, 5);

  sbSizerOutFilter = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Output filtering")),
      wxVERTICAL);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxVERTICAL);

  m_rbOAccept =
      new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Transmit sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer10->Add(m_rbOAccept, 0, wxALL, 5);

  m_rbOIgnore = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Drop sentences"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer10->Add(m_rbOIgnore, 0, wxALL, 5);

  sbSizerOutFilter->Add(bSizer10, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer12;
  bSizer12 = new wxBoxSizer(wxHORIZONTAL);

  m_tcOutputStc =
      new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxSize(-1, -1), wxTE_READONLY);
  bSizer12->Add(m_tcOutputStc, 1, wxALL, 5);

  m_btnOutputStcList =
      new wxButton(m_pNMEAForm, wxID_ANY, _T("..."), wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizer12->Add(m_btnOutputStcList, 0, wxALL, 5);

  sbSizerOutFilter->Add(bSizer12, 0, wxEXPAND, 5);
  sbSizerConnectionProps->Add(sbSizerOutFilter, 0, wxEXPAND, 5);

  bSizerOuterContainer->Add(sbSizerConnectionProps, 1, wxALL | wxEXPAND, 5);

  bSizer4->Add(bSizerOuterContainer, 1, wxEXPAND, 5);

  // Connect Events
  //   m_lcSources->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
  //                        wxListEventHandler(options::OnSelectDatasource),
  //                        NULL, this);
  m_buttonAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(options::OnAddDatasourceClick),
                       NULL, this);
  m_buttonRemove->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(options::OnRemoveDatasourceClick), NULL, this);

  m_rbTypeSerial->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                          wxCommandEventHandler(options::OnTypeSerialSelected),
                          NULL, this);
  m_rbTypeNet->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnTypeNetSelected), NULL,
                       this);

  if (m_rbTypeInternalGPS)
    m_rbTypeInternalGPS->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(options::OnTypeGPSSelected), NULL, this);
  if (m_rbTypeInternalBT)
    m_rbTypeInternalBT->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(options::OnTypeBTSelected), NULL, this);

  m_rbNetProtoTCP->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoUDP->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoGPSD->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoSignalK->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
  m_tNetAddress->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                         wxCommandEventHandler(options::OnConnValChange), NULL,
                         this);
  m_tNetPort->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                      wxCommandEventHandler(options::OnConnValChange), NULL,
                      this);
  m_comboPort->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED,
                       wxCommandEventHandler(options::OnConnValChange), NULL,
                       this);
  m_comboPort->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                       wxCommandEventHandler(options::OnConnValChange), NULL,
                       this);
  m_choiceBaudRate->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                            wxCommandEventHandler(options::OnBaudrateChoice),
                            NULL, this);
  m_choiceSerialProtocol->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(options::OnProtocolChoice), NULL, this);
  m_choicePriority->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                            wxCommandEventHandler(options::OnConnValChange),
                            NULL, this);
  m_cbCheckCRC->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                        wxCommandEventHandler(options::OnCrcCheck), NULL, this);
  m_cbGarminHost->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                          wxCommandEventHandler(options::OnUploadFormatChange),
                          NULL, this);
  m_cbGarminUploadHost->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(options::OnUploadFormatChange), NULL, this);
  m_cbFurunoGP3X->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                          wxCommandEventHandler(options::OnUploadFormatChange),
                          NULL, this);
  m_rbIAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnRbAcceptInput), NULL,
                       this);
  m_rbIIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnRbIgnoreInput), NULL,
                       this);
  m_tcInputStc->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                        wxCommandEventHandler(options::OnConnValChange), NULL,
                        this);
  m_btnInputStcList->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                             wxCommandEventHandler(options::OnBtnIStcs), NULL,
                             this);
  m_cbInput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                     wxCommandEventHandler(options::OnCbInput), NULL, this);
  m_cbOutput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                      wxCommandEventHandler(options::OnCbOutput), NULL, this);
  m_rbOAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnRbOutput), NULL, this);
  m_rbOIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnRbOutput), NULL, this);
  m_tcOutputStc->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                         wxCommandEventHandler(options::OnConnValChange), NULL,
                         this);
  m_btnOutputStcList->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(options::OnBtnOStcs), NULL,
                              this);
  m_cbCheckCRC->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                        wxCommandEventHandler(options::OnCrcCheck), NULL,
                        this);
  pOpenGL->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                   wxCommandEventHandler(options::OnGLClicked), NULL, this);

  m_cbNMEADebug->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(options::OnShowGpsWindowCheckboxClick), NULL, this);
  m_cbFilterSogCog->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                            wxCommandEventHandler(options::OnValChange), NULL,
                            this);
  m_tFilterSec->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                        wxCommandEventHandler(options::OnValChange), NULL,
                        this);
  m_cbAPBMagnetic->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                           wxCommandEventHandler(options::OnValChange), NULL,
                           this);

#if 0
  m_lcSources->Connect(wxEVT_LEFT_DOWN,
                       wxMouseEventHandler(options::OnConnectionToggleEnableMouse),
                       NULL, this);
#if wxCHECK_VERSION(2, 9, 0)
    m_lcSources->Connect(wxEVT_LIST_ITEM_ACTIVATED,
                       wxListEventHandler(options::OnConnectionToggleEnable),
                       NULL, this);
#endif

  wxString columns[] = {_("On"),   _("Type"), _("Data Port"),   _("Prio"),
                        _("Parm"), _("I/O"),  _("Filters")};
  for (int i = 0; i < 7; ++i) {
    wxListItem col;
    col.SetId(i);
    col.SetText(columns[i]);
    m_lcSources->InsertColumn(i, col);
  }

  //  Build the image list
  wxImageList* imglist = new wxImageList(16, 16, TRUE, 1);
  wxBitmap unchecked_bmp(16, 16), checked_bmp(16, 16);
  wxMemoryDC renderer_dc;

  // Unchecked
  renderer_dc.SelectObject(unchecked_bmp);
  renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(
      GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
  renderer_dc.Clear();
  wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16),
                                       0);

  // Checked
  renderer_dc.SelectObject(checked_bmp);
  renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(
      GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
  renderer_dc.Clear();
  wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16),
                                       wxCONTROL_CHECKED);

  // Deselect the renderer Object
  renderer_dc.SelectObject(wxNullBitmap);

  imglist->Add(unchecked_bmp);
  imglist->Add(checked_bmp);
  m_lcSources->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

  m_lcSources->Refresh();
#endif
  FillSourceList();

  ShowNMEACommon(FALSE);
  ShowNMEASerial(FALSE);
  ShowNMEANet(FALSE);
  connectionsaved = TRUE;
}

void options::CreatePanel_NMEA(size_t parent, int border_size,
                               int group_item_spacing) {
  m_pNMEAForm = AddPage(parent, _("NMEA"));

  wxBoxSizer* bSizer4 = new wxBoxSizer(wxVERTICAL);
  m_pNMEAForm->SetSizer(bSizer4);
  m_pNMEAForm->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizerOuterContainer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer* sbSizerGeneral;
  sbSizerGeneral = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("General")), wxVERTICAL);

  wxBoxSizer* bSizer151;
  bSizer151 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer161;
  bSizer161 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer171;
  bSizer171 = new wxBoxSizer(wxHORIZONTAL);

  m_cbFilterSogCog = new wxCheckBox(m_pNMEAForm, wxID_ANY,
                                    _("Filter NMEA Course and Speed data"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_cbFilterSogCog->SetValue(g_bfilter_cogsog);
  bSizer171->Add(m_cbFilterSogCog, 0, wxALL, 5);

  m_stFilterSec =
      new wxStaticText(m_pNMEAForm, wxID_ANY, _("Filter period (sec)"),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_stFilterSec->Wrap(-1);

  int nspace = 5;
#ifdef __WXGTK__
  nspace = 9;
#endif
  bSizer171->Add(m_stFilterSec, 0, wxALL, nspace);

  m_tFilterSec = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize, 0);
  wxString sfilt;
  sfilt.Printf(_T("%d"), g_COGFilterSec);
  m_tFilterSec->SetValue(sfilt);
  bSizer171->Add(m_tFilterSec, 0, wxALL, 4);
  bSizer161->Add(bSizer171, 1, wxEXPAND, 5);

  int cb_space = 2;
  m_cbNMEADebug =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Show NMEA Debug Window"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbNMEADebug->SetValue(NMEALogWindow::Get().Active());
  bSizer161->Add(m_cbNMEADebug, 0, wxALL, cb_space);

  m_cbFurunoGP3X =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Format uploads for Furuno GP3X"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbFurunoGP3X->SetValue(g_GPS_Ident == _T( "FurunoGP3X" ));
  bSizer161->Add(m_cbFurunoGP3X, 0, wxALL, cb_space);

  m_cbGarminUploadHost = new wxCheckBox(
      m_pNMEAForm, wxID_ANY, _("Use Garmin GRMN (Host) mode for uploads"),
      wxDefaultPosition, wxDefaultSize, 0);
  m_cbGarminUploadHost->SetValue(g_bGarminHostUpload);
  bSizer161->Add(m_cbGarminUploadHost, 0, wxALL, cb_space);

  m_cbAPBMagnetic =
      new wxCheckBox(m_pNMEAForm, wxID_ANY,
                     _("Use magnetic bearings in output sentence ECAPB"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbAPBMagnetic->SetValue(g_bMagneticAPB);
  bSizer161->Add(m_cbAPBMagnetic, 0, wxALL, cb_space);

  bSizer151->Add(bSizer161, 1, wxEXPAND, 5);
  sbSizerGeneral->Add(bSizer151, 1, wxEXPAND, 5);
  bSizerOuterContainer->Add(sbSizerGeneral, 0, wxALL | wxEXPAND, 5);

  //  Connections listbox, etc
  wxStaticBoxSizer* sbSizerLB = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Data Connections")),
      wxVERTICAL);

  /*
    wxBoxSizer* bSizer17;
    bSizer17 = new wxBoxSizer(wxVERTICAL);

    m_lcSources = new wxListCtrl(m_pNMEAForm, wxID_ANY, wxDefaultPosition,
                                 wxSize(-1, 150), wxLC_REPORT |
    wxLC_SINGLE_SEL); bSizer17->Add(m_lcSources, 1, wxALL | wxEXPAND, 5);
  */

  wxPanel* cPanel =
      new wxPanel(m_pNMEAForm, wxID_ANY, wxDefaultPosition,
                  wxDLG_UNIT(this, wxSize(-1, -1)), wxBG_STYLE_ERASE);
  sbSizerLB->Add(cPanel, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer* boxSizercPanel = new wxBoxSizer(wxVERTICAL);
  cPanel->SetSizer(boxSizercPanel);

  m_scrollWinConnections = new wxScrolledWindow(
      cPanel, wxID_ANY, wxDefaultPosition, wxDLG_UNIT(this, wxSize(-1, 80)),
      wxBORDER_RAISED | wxVSCROLL | wxBG_STYLE_ERASE);
  m_scrollWinConnections->SetScrollRate(5, 5);
  boxSizercPanel->Add(m_scrollWinConnections, 0, wxALL | wxEXPAND, 5);

  boxSizerConnections = new wxBoxSizer(wxVERTICAL);
  m_scrollWinConnections->SetSizer(boxSizerConnections);

  bSizerOuterContainer->Add(sbSizerLB, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer18;
  bSizer18 = new wxBoxSizer(wxHORIZONTAL);
  sbSizerLB->Add(bSizer18, 1, wxEXPAND, 5);

  m_buttonAdd = new wxButton(m_pNMEAForm, wxID_ANY, _("Add Connection"),
                             wxDefaultPosition, wxDefaultSize, 0);
  bSizer18->Add(m_buttonAdd, 0, wxALL, 5);

  m_buttonRemove = new wxButton(m_pNMEAForm, wxID_ANY, _("Remove Connection"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_buttonRemove->Enable(FALSE);
  bSizer18->Add(m_buttonRemove, 0, wxALL, 5);

  //   wxBoxSizer* bSizer19 = new wxBoxSizer(wxHORIZONTAL);
  //   sbSizerLB->Add(bSizer19, 1, wxEXPAND, 5);
  //
  wxFont* dFont = GetOCPNScaledFont_PlugIn(_("Dialog"));
  double font_size = dFont->GetPointSize() * 17 / 16;
  wxFont* bFont = wxTheFontList->FindOrCreateFont(
      font_size, dFont->GetFamily(), dFont->GetStyle(), wxFONTWEIGHT_BOLD);
  //
  //   m_stEditCon = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Edit Selected
  //   Connection")); m_stEditCon->SetFont(*bFont); bSizer19->Add(m_stEditCon,
  //   0, wxALL | wxEXPAND | wxALIGN_CENTER_HORIZONTAL, 5);
  //

  //  Connections Properties
  m_sbConnEdit =
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Edit Selected Connection"));
  m_sbConnEdit->SetFont(*bFont);

  sbSizerConnectionProps = new wxStaticBoxSizer(m_sbConnEdit, wxVERTICAL);

  wxBoxSizer* bSizer15;
  bSizer15 = new wxBoxSizer(wxHORIZONTAL);

  sbSizerConnectionProps->Add(bSizer15, 0, wxTOP | wxEXPAND, 5);

  m_rbTypeSerial =
      new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Serial"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_rbTypeSerial->SetValue(TRUE);
  bSizer15->Add(m_rbTypeSerial, 0, wxALL, 5);

  m_rbTypeNet = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Network"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer15->Add(m_rbTypeNet, 0, wxALL, 5);

  if (OCPNPlatform::hasInternalGPS()) {
    m_rbTypeInternalGPS =
        new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Built-in GPS"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15->Add(m_rbTypeInternalGPS, 0, wxALL, 5);
  } else
    m_rbTypeInternalGPS = NULL;

  // has built-in Bluetooth
  if (OCPNPlatform::hasInternalBT()) {
    m_rbTypeInternalBT =
        new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Built-in Bluetooth SPP"),
                          wxDefaultPosition, wxDefaultSize, 0);
    bSizer15->Add(m_rbTypeInternalBT, 0, wxALL, 5);

    m_buttonScanBT = new wxButton(m_pNMEAForm, wxID_ANY, _("BT Scan"),
                                  wxDefaultPosition, wxDefaultSize);
    m_buttonScanBT->Hide();

    wxBoxSizer* bSizer15a = new wxBoxSizer(wxHORIZONTAL);
    sbSizerConnectionProps->Add(bSizer15a, 0, wxEXPAND, 5);

    bSizer15a->Add(m_buttonScanBT, 0, wxALL, 5);

    m_stBTPairs =
        new wxStaticText(m_pNMEAForm, wxID_ANY, _("Bluetooth Data Sources"),
                         wxDefaultPosition, wxDefaultSize, 0);
    m_stBTPairs->Wrap(-1);
    m_stBTPairs->Hide();
    bSizer15a->Add(m_stBTPairs, 0, wxALL, 5);

    wxArrayString mt;
    mt.Add(_T( "unscanned" ));
    m_choiceBTDataSources = new wxChoice(m_pNMEAForm, wxID_ANY,
                                         wxDefaultPosition, wxDefaultSize, mt);

#if 0
        m_BTscan_results.Clear();
        m_BTscan_results.Add(_T("None"));

        m_BTscan_results = g_Platform->getBluetoothScanResults();
        m_choiceBTDataSources->Clear();
        m_choiceBTDataSources->Append(m_BTscan_results[0]);  // scan status

        unsigned int i=1;
        while( (i+1) < m_BTscan_results.GetCount()){
            wxString item1 = m_BTscan_results[i] + _T(";");
            wxString item2 = m_BTscan_results.Item(i+1);
            m_choiceBTDataSources->Append(item1 + item2);

            i += 2;
    }

    if( m_BTscan_results.GetCount() > 1){
        m_choiceBTDataSources->SetSelection( 1 );
    }
#endif

    m_choiceBTDataSources->Hide();
    bSizer15a->Add(m_choiceBTDataSources, 1, wxEXPAND | wxTOP, 5);

    m_buttonScanBT->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                            wxCommandEventHandler(options::OnScanBTClick), NULL,
                            this);
  } else
    m_rbTypeInternalBT = NULL;

  gSizerNetProps = new wxGridSizer(0, 2, 0, 0);

  m_stNetProto = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Protocol"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_stNetProto->Wrap(-1);
  gSizerNetProps->Add(m_stNetProto, 0, wxALL, 5);

  wxBoxSizer* bSizer16;
  bSizer16 = new wxBoxSizer(wxHORIZONTAL);

  m_rbNetProtoTCP =
      new wxRadioButton(m_pNMEAForm, wxID_ANY, _("TCP"), wxDefaultPosition,
                        wxDefaultSize, wxRB_GROUP);
  m_rbNetProtoTCP->Enable(TRUE);

  bSizer16->Add(m_rbNetProtoTCP, 0, wxALL, 5);

  m_rbNetProtoUDP = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("UDP"),
                                      wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoUDP->Enable(TRUE);

  bSizer16->Add(m_rbNetProtoUDP, 0, wxALL, 5);

  m_rbNetProtoGPSD = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("GPSD"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoGPSD->SetValue(TRUE);
  bSizer16->Add(m_rbNetProtoGPSD, 0, wxALL, 5);

  m_rbNetProtoSignalK = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Signal K"),
                                          wxDefaultPosition, wxDefaultSize, 0);
  m_rbNetProtoSignalK->Enable(TRUE);
  bSizer16->Add(m_rbNetProtoSignalK, 0, wxALL, 5);

  gSizerNetProps->Add(bSizer16, 1, wxEXPAND, 5);

  m_stNetAddr = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Address"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stNetAddr->Wrap(-1);
  gSizerNetProps->Add(m_stNetAddr, 0, wxALL, 5);

  m_tNetAddress = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, 0);
  gSizerNetProps->Add(m_tNetAddress, 0, wxEXPAND | wxTOP, 5);

  m_stNetPort = new wxStaticText(m_pNMEAForm, wxID_ANY, _("DataPort"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stNetPort->Wrap(-1);
  gSizerNetProps->Add(m_stNetPort, 0, wxALL, 5);

  m_tNetPort = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString,
                              wxDefaultPosition, wxDefaultSize, 0);
  gSizerNetProps->Add(m_tNetPort, 1, wxEXPAND | wxTOP, 5);

  //  User Comments
  m_stNetComment = new wxStaticText(m_pNMEAForm, wxID_ANY, _("User Comment"),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_stNetComment->Wrap(-1);
  gSizerNetProps->Add(m_stNetComment, 0, wxALL, 5);

  m_tNetComment = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString,
                                 wxDefaultPosition, wxDefaultSize, 0);
  gSizerNetProps->Add(m_tNetComment, 1, wxEXPAND | wxTOP, 5);

  sbSizerConnectionProps->Add(gSizerNetProps, 0, wxEXPAND, 5);

  gSizerSerProps = new wxGridSizer(0, 1, 0, 0);

  wxFlexGridSizer* fgSizer1;
  fgSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
  fgSizer1->SetFlexibleDirection(wxBOTH);
  fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stSerPort = new wxStaticText(m_pNMEAForm, wxID_ANY, _("DataPort"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stSerPort->Wrap(-1);
  fgSizer1->Add(m_stSerPort, 0, wxALL, 5);

  m_comboPort = new wxComboBox(m_pNMEAForm, wxID_ANY, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
  fgSizer1->Add(m_comboPort, 0, wxEXPAND | wxTOP, 5);

  m_stSerBaudrate = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Baudrate"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  m_stSerBaudrate->Wrap(-1);
  fgSizer1->Add(m_stSerBaudrate, 0, wxALL, 5);

  wxString m_choiceBaudRateChoices[] = {
      _("150"),    _("300"),    _("600"),    _("1200"),  _("2400"),
      _("4800"),   _("9600"),   _("19200"),  _("38400"), _("57600"),
      _("115200"), _("230400"), _("460800"), _("921600")};
  int m_choiceBaudRateNChoices =
      sizeof(m_choiceBaudRateChoices) / sizeof(wxString);
  m_choiceBaudRate =
      new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceBaudRateNChoices, m_choiceBaudRateChoices, 0);
  m_choiceBaudRate->SetSelection(0);
  fgSizer1->Add(m_choiceBaudRate, 1, wxEXPAND | wxTOP, 5);

  m_stSerProtocol = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Protocol"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  m_stSerProtocol->Wrap(-1);
  fgSizer1->Add(m_stSerProtocol, 0, wxALL, 5);

  wxString m_choiceSerialProtocolChoices[] = {_("NMEA 0183"), _("NMEA 2000")};
  int m_choiceSerialProtocolNChoices =
      sizeof(m_choiceSerialProtocolChoices) / sizeof(wxString);
  m_choiceSerialProtocol = new wxChoice(
      m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      m_choiceSerialProtocolNChoices, m_choiceSerialProtocolChoices, 0);
  m_choiceSerialProtocol->SetSelection(0);
  m_choiceSerialProtocol->Enable(TRUE);
  fgSizer1->Add(m_choiceSerialProtocol, 1, wxEXPAND | wxTOP, 5);

  m_stPriority = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Priority"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_stPriority->Wrap(-1);
  fgSizer1->Add(m_stPriority, 0, wxALL, 5);

  wxString m_choicePriorityChoices[] = {_("0"), _("1"), _("2"), _("3"), _("4"),
                                        _("5"), _("6"), _("7"), _("8"), _("9")};
  int m_choicePriorityNChoices =
      sizeof(m_choicePriorityChoices) / sizeof(wxString);
  m_choicePriority =
      new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choicePriorityNChoices, m_choicePriorityChoices, 0);
  m_choicePriority->SetSelection(9);
  fgSizer1->Add(m_choicePriority, 0, wxEXPAND | wxTOP, 5);

  gSizerSerProps->Add(fgSizer1, 0, wxEXPAND, 5);

  wxFlexGridSizer* fgSizer5;
  fgSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizer5->SetFlexibleDirection(wxBOTH);
  fgSizer5->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  //  User Comments
  m_stSerialComment = new wxStaticText(m_pNMEAForm, wxID_ANY, _("User Comment"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_stSerialComment->Wrap(-1);
  fgSizer5->Add(m_stSerialComment, 0, wxALL, 5);

  m_tSerialComment = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_tSerialComment, 1, wxEXPAND | wxTOP, 5);

  m_cbCheckCRC = new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Control checksum"),
                                wxDefaultPosition, wxDefaultSize, 0);
  m_cbCheckCRC->SetValue(TRUE);
  m_cbCheckCRC->SetToolTip(
      _("If checked, only the sentences with a valid checksum are passed "
        "through"));
  fgSizer5->Add(m_cbCheckCRC, 0, wxALL, 5);

  m_cbGarminHost = new wxCheckBox(m_pNMEAForm, wxID_ANY,
                                  _("Use Garmin (GRMN) mode for input"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_cbGarminHost->SetValue(FALSE);
  fgSizer5->Add(m_cbGarminHost, 0, wxALL, 5);
#ifndef USE_GARMINHOST
  m_cbGarminHost->Hide();
#endif

  m_cbInput =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Receive Input on this Port"),
                     wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_cbInput, 0, wxALL, 5);

  m_cbOutput =
      new wxCheckBox(m_pNMEAForm, wxID_ANY,
                     wxString::Format(_T("%s (%s)"), _("Output on this port"),
                                      _("as autopilot or NMEA repeater")),
                     wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_cbOutput, 0, wxALL, 5);

  m_stTalkerIdText = new wxStaticText(
      m_pNMEAForm, wxID_ANY,
      wxString::Format(_T("%s (%s)"), _("Talker ID"), _("blank = default ID")),
      wxDefaultPosition, wxDefaultSize, 0);
  m_stTalkerIdText->Wrap(-1);
  fgSizer5->Add(m_stTalkerIdText, 0, wxALL, 5);

  m_TalkerIdText = new wxTextCtrl(m_pNMEAForm, ID_OPTEXTCTRL, _T( "" ),
                                  wxDefaultPosition, wxSize(50, -1), 0);
  m_TalkerIdText->SetMaxLength(2);
  fgSizer5->Add(m_TalkerIdText, 0, wxALIGN_LEFT | wxALL, group_item_spacing);

  m_stPrecision =
      new wxStaticText(m_pNMEAForm, wxID_ANY, _("APB bearing precision"),
                       wxDefaultPosition, wxDefaultSize, 0);

  m_stPrecision->Wrap(-1);
  fgSizer5->Add(m_stPrecision, 0, wxALL, 5);

  wxString m_choicePrecisionChoices[] = {_("x"), _("x.x"), _("x.xx"),
                                         _("x.xxx"), _("x.xxxx")};
  int m_choicePrecisionNChoices =
      sizeof(m_choicePrecisionChoices) / sizeof(wxString);
  m_choicePrecision =
      new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choicePrecisionNChoices, m_choicePrecisionChoices, 0);
  m_choicePrecision->SetSelection(g_NMEAAPBPrecision);
  fgSizer5->Add(m_choicePrecision, 0, wxALL, 5);

  // signalK discovery enable
  m_cbCheckSKDiscover =
      new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Automatic server discovery"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_cbCheckSKDiscover->SetValue(TRUE);
  m_cbCheckSKDiscover->SetToolTip(
      _("If checked, signal K server will be discovered automatically"));
  fgSizer5->Add(m_cbCheckSKDiscover, 0, wxALL, 5);

  // signal K "Discover now" button
  m_ButtonSKDiscover = new wxButton(m_pNMEAForm, wxID_ANY, _("Discover now..."),
                                    wxDefaultPosition, wxDefaultSize, 0);
  m_ButtonSKDiscover->Hide();
  fgSizer5->Add(m_ButtonSKDiscover, 0, wxALL, 5);

  // signalK Server Status
  m_StaticTextSKServerStatus = new wxStaticText(
      m_pNMEAForm, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0);
  fgSizer5->Add(m_StaticTextSKServerStatus, 0, wxALL, 5);

  sbSizerConnectionProps->Add(gSizerSerProps, 0, wxEXPAND, 5);
  sbSizerConnectionProps->Add(fgSizer5, 0, wxEXPAND, 5);

  sbSizerInFilter = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Input filtering")), wxVERTICAL);

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxHORIZONTAL);

  m_rbIAccept =
      new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Accept only sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer9->Add(m_rbIAccept, 0, wxALL, 5);

  m_rbIIgnore = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Ignore sentences"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer9->Add(m_rbIIgnore, 0, wxALL, 5);

  sbSizerInFilter->Add(bSizer9, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);

  m_tcInputStc =
      new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizer11->Add(m_tcInputStc, 1, wxALL | wxEXPAND, 5);

  m_btnInputStcList =
      new wxButton(m_pNMEAForm, wxID_ANY, _T("..."), wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizer11->Add(m_btnInputStcList, 0, wxALL, 5);

  sbSizerInFilter->Add(bSizer11, 0, wxEXPAND, 5);

  sbSizerConnectionProps->Add(sbSizerInFilter, 0, wxEXPAND, 5);

  sbSizerOutFilter = new wxStaticBoxSizer(
      new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Output filtering")),
      wxVERTICAL);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxHORIZONTAL);

  m_rbOAccept =
      new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Transmit sentences"),
                        wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  bSizer10->Add(m_rbOAccept, 0, wxALL, 5);

  m_rbOIgnore = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Drop sentences"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  bSizer10->Add(m_rbOIgnore, 0, wxALL, 5);

  sbSizerOutFilter->Add(bSizer10, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer12;
  bSizer12 = new wxBoxSizer(wxHORIZONTAL);

  m_tcOutputStc =
      new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  bSizer12->Add(m_tcOutputStc, 1, wxALL | wxEXPAND, 5);

  m_btnOutputStcList =
      new wxButton(m_pNMEAForm, wxID_ANY, _T("..."), wxDefaultPosition,
                   wxDefaultSize, wxBU_EXACTFIT);
  bSizer12->Add(m_btnOutputStcList, 0, wxALL, 5);

  sbSizerOutFilter->Add(bSizer12, 0, wxEXPAND, 5);
  sbSizerConnectionProps->Add(sbSizerOutFilter, 0, wxEXPAND, 5);

  bSizerOuterContainer->Add(sbSizerConnectionProps, 1, wxALL | wxEXPAND, 5);

  bSizer4->Add(bSizerOuterContainer, 1, wxEXPAND, 5);

  // Connect Events
  //   m_lcSources->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
  //                        wxListEventHandler(options::OnSelectDatasource),
  //                        NULL, this);
  m_buttonAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(options::OnAddDatasourceClick),
                       NULL, this);
  m_buttonRemove->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(options::OnRemoveDatasourceClick), NULL, this);

  m_rbTypeSerial->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                          wxCommandEventHandler(options::OnTypeSerialSelected),
                          NULL, this);
  m_rbTypeNet->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnTypeNetSelected), NULL,
                       this);

  if (m_rbTypeInternalGPS)
    m_rbTypeInternalGPS->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(options::OnTypeGPSSelected), NULL, this);
  if (m_rbTypeInternalBT)
    m_rbTypeInternalBT->Connect(
        wxEVT_COMMAND_RADIOBUTTON_SELECTED,
        wxCommandEventHandler(options::OnTypeBTSelected), NULL, this);

  m_rbNetProtoTCP->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoUDP->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoGPSD->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
  m_rbNetProtoSignalK->Connect(
      wxEVT_COMMAND_RADIOBUTTON_SELECTED,
      wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
  m_tNetAddress->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                         wxCommandEventHandler(options::OnConnValChange), NULL,
                         this);
  m_tNetPort->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                      wxCommandEventHandler(options::OnConnValChange), NULL,
                      this);
  m_comboPort->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED,
                       wxCommandEventHandler(options::OnConnValChange), NULL,
                       this);
  m_comboPort->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                       wxCommandEventHandler(options::OnConnValChange), NULL,
                       this);
  m_choiceBaudRate->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                            wxCommandEventHandler(options::OnBaudrateChoice),
                            NULL, this);
  m_choiceSerialProtocol->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(options::OnProtocolChoice), NULL, this);
  m_choicePriority->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
                            wxCommandEventHandler(options::OnConnValChange),
                            NULL, this);
  m_cbCheckCRC->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                        wxCommandEventHandler(options::OnCrcCheck), NULL, this);
  m_cbGarminHost->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                          wxCommandEventHandler(options::OnUploadFormatChange),
                          NULL, this);
  m_cbGarminUploadHost->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(options::OnUploadFormatChange), NULL, this);
  m_cbFurunoGP3X->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                          wxCommandEventHandler(options::OnUploadFormatChange),
                          NULL, this);
  m_cbCheckSKDiscover->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                               wxCommandEventHandler(options::OnConnValChange),
                               NULL, this);
  m_ButtonSKDiscover->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(options::OnDiscoverButton),
                              NULL, this);

  m_rbIAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnRbAcceptInput), NULL,
                       this);
  m_rbIIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnRbIgnoreInput), NULL,
                       this);
  m_tcInputStc->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                        wxCommandEventHandler(options::OnConnValChange), NULL,
                        this);
  m_btnInputStcList->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                             wxCommandEventHandler(options::OnBtnIStcs), NULL,
                             this);
  m_cbInput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                     wxCommandEventHandler(options::OnCbInput), NULL, this);
  m_cbOutput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                      wxCommandEventHandler(options::OnCbOutput), NULL, this);
  m_rbOAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnRbOutput), NULL, this);
  m_rbOIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                       wxCommandEventHandler(options::OnRbOutput), NULL, this);
  m_tcOutputStc->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                         wxCommandEventHandler(options::OnConnValChange), NULL,
                         this);
  m_btnOutputStcList->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(options::OnBtnOStcs), NULL,
                              this);
  m_cbCheckCRC->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                        wxCommandEventHandler(options::OnConnValChange), NULL,
                        this);

  pOpenGL->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                   wxCommandEventHandler(options::OnGLClicked), NULL, this);

  m_cbNMEADebug->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(options::OnShowGpsWindowCheckboxClick), NULL, this);
  m_cbFilterSogCog->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                            wxCommandEventHandler(options::OnValChange), NULL,
                            this);
  m_tFilterSec->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                        wxCommandEventHandler(options::OnValChange), NULL,
                        this);
  m_cbAPBMagnetic->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                           wxCommandEventHandler(options::OnValChange), NULL,
                           this);

  m_tNetComment->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                         wxCommandEventHandler(options::OnConnValChange), NULL,
                         this);
  m_tSerialComment->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                            wxCommandEventHandler(options::OnConnValChange),
                            NULL, this);

#if 0
//   m_lcSources->Connect(wxEVT_LEFT_DOWN,
//                        wxMouseEventHandler(options::OnConnectionToggleEnableMouse),
//                        NULL, this);
// #if wxCHECK_VERSION(2, 9, 0)
//   m_lcSources->Connect(wxEVT_LIST_ITEM_ACTIVATED,
//                        wxListEventHandler(options::OnConnectionToggleEnable),
//                        NULL, this);
// #endif

  wxString columns[] = {_("Enable"),   _("Type"),       _("DataPort"),
                        _("Priority"), _("Parameters"), _("Connection"),
                        _("Filters")};
  for (int i = 0; i < 7; ++i) {
    wxListItem col;
    col.SetId(i);
    col.SetText(columns[i]);
//    m_lcSources->InsertColumn(i, col);
  }

  //  Build the image list
  wxImageList* imglist = new wxImageList(16, 16, TRUE, 1);
  wxBitmap unchecked_bmp(16, 16), checked_bmp(16, 16);
  wxMemoryDC renderer_dc;

  // Unchecked
  renderer_dc.SelectObject(unchecked_bmp);
  renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(
      GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
  renderer_dc.Clear();
  wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16),
                                       0);

  // Checked
  renderer_dc.SelectObject(checked_bmp);
  renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(
      GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
  renderer_dc.Clear();
  wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16),
                                       wxCONTROL_CHECKED);

  // Deselect the renderer Object
  renderer_dc.SelectObject(wxNullBitmap);

  imglist->Add(unchecked_bmp);
  imglist->Add(checked_bmp);
//  m_lcSources->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

//  m_lcSources->Refresh();
#endif
  FillSourceList();

  ShowNMEACommon(true);
  ShowNMEASerial(true);
  ShowNMEANet(true);
  connectionsaved = TRUE;
}

#if 0
void options::EnableItem(const long index) {
  if (index == wxNOT_FOUND) {
    ClearNMEAForm();
    m_buttonRemove->Disable();
  } else {
    ConnectionParams* conn =
        g_pConnectionParams->Item(m_lcSources->GetItemData(index));
    if (!conn) return;
    conn->bEnabled = !conn->bEnabled;
    m_connection_enabled = conn->bEnabled;
    // Mark as changed
    conn->b_IsSetup = FALSE;
    m_lcSources->SetItemImage(index, conn->bEnabled);
  }
}

void options::OnConnectionToggleEnable(wxListEvent& event) {
  EnableItem(event.GetIndex());
  gFrame->RefreshAllCanvas();;
}

void options::OnConnectionToggleEnableMouse(wxMouseEvent& event) {
#if 0
  int flags;
  long index = m_lcSources->HitTest(event.GetPosition(), flags);
  if (index == wxNOT_FOUND || event.GetX() < m_lcSources->GetColumnWidth(0))
    EnableItem(index);
#endif
  // Allow wx to process...
  event.Skip();
}
#endif

void options::EnableConnection(ConnectionParams* conn, bool value) {
  if (conn) {
    conn->bEnabled = value;
    conn->b_IsSetup = FALSE;  // trigger a rebuild/takedown of the connection
    m_connection_enabled = conn->bEnabled;
  }
}

void options::CreatePanel_Ownship(size_t parent, int border_size,
                                  int group_item_spacing) {
  itemPanelShip = AddPage(parent, _("Own ship"));

  ownShip = new wxBoxSizer(wxVERTICAL);
  itemPanelShip->SetSizer(ownShip);

  // OwnShip Display options
  wxStaticBox* osdBox =
      new wxStaticBox(itemPanelShip, wxID_ANY, _("Display Options"));
  dispOptions = new wxStaticBoxSizer(osdBox, wxVERTICAL);
  ownShip->Add(dispOptions, 0, wxTOP | wxALL | wxEXPAND, border_size);

  wxFlexGridSizer* dispOptionsGrid =
      new wxFlexGridSizer(0, 2, group_item_spacing, group_item_spacing);
  dispOptionsGrid->AddGrowableCol(1);
  dispOptions->Add(dispOptionsGrid, 0, wxALL | wxEXPAND, border_size);

  wxStaticText* pStatic_OSCOG_Predictor = new wxStaticText(
      itemPanelShip, wxID_ANY, _("COG Predictor Length (min)"));
  dispOptionsGrid->Add(pStatic_OSCOG_Predictor, 0);

  m_pText_OSCOG_Predictor = new wxTextCtrl(itemPanelShip, wxID_ANY);
  dispOptionsGrid->Add(m_pText_OSCOG_Predictor, 0, wxALIGN_RIGHT);

  wxStaticText* pStatic_OSHDT_Predictor = new wxStaticText(
      itemPanelShip, wxID_ANY, _("Heading Predictor Length (NMi)"));
  dispOptionsGrid->Add(pStatic_OSHDT_Predictor, 0);

  m_pText_OSHDT_Predictor = new wxTextCtrl(itemPanelShip, wxID_ANY);
  dispOptionsGrid->Add(m_pText_OSHDT_Predictor, 0, wxALIGN_RIGHT);

  wxStaticText* iconTypeTxt =
      new wxStaticText(itemPanelShip, wxID_ANY, _("Ship Icon Type"));
  dispOptionsGrid->Add(iconTypeTxt, 0);

  wxString iconTypes[] = {_("Default"), _("Real Scale Bitmap"),
                          _("Real Scale Vector")};

  m_pShipIconType = new wxChoice(
      itemPanelShip, ID_SHIPICONTYPE, wxDefaultPosition,
      wxSize(GetCharWidth() * 20, GetCharHeight() * 2), 3, iconTypes);
  dispOptionsGrid->Add(m_pShipIconType, 0,
                       wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxTOP,
                       group_item_spacing);

  realSizes = new wxFlexGridSizer(5, 2, group_item_spacing, group_item_spacing);
  realSizes->AddGrowableCol(1);

  dispOptions->Add(realSizes, 0, wxEXPAND | wxLEFT, 30);

  realSizes->Add(
      new wxStaticText(itemPanelShip, wxID_ANY, _("Length Over All (m)")), 1,
      wxALIGN_LEFT);
  m_pOSLength = new wxTextCtrl(itemPanelShip, 1);
  realSizes->Add(m_pOSLength, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

  realSizes->Add(
      new wxStaticText(itemPanelShip, wxID_ANY, _("Width Over All (m)")), 1,
      wxALIGN_LEFT);
  m_pOSWidth = new wxTextCtrl(itemPanelShip, wxID_ANY);
  realSizes->Add(m_pOSWidth, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

  realSizes->Add(
      new wxStaticText(itemPanelShip, wxID_ANY, _("GPS Offset from Bow (m)")),
      1, wxALIGN_LEFT);
  m_pOSGPSOffsetY = new wxTextCtrl(itemPanelShip, wxID_ANY);
  realSizes->Add(m_pOSGPSOffsetY, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

  realSizes->Add(new wxStaticText(itemPanelShip, wxID_ANY,
                                  _("GPS Offset from Midship (m)")),
                 1, wxALIGN_LEFT);
  m_pOSGPSOffsetX = new wxTextCtrl(itemPanelShip, wxID_ANY);
  realSizes->Add(m_pOSGPSOffsetX, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

  realSizes->Add(
      new wxStaticText(itemPanelShip, wxID_ANY, _("Minimum Screen Size (mm)")),
      1, wxALIGN_LEFT);
  m_pOSMinSize = new wxTextCtrl(itemPanelShip, wxID_ANY);
  realSizes->Add(m_pOSMinSize, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

  // Radar rings
  wxFlexGridSizer* rrSelect =
      new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
  rrSelect->AddGrowableCol(1);
  dispOptions->Add(rrSelect, 0, wxLEFT | wxRIGHT | wxEXPAND, border_size);

  wxStaticText* rrTxt =
      new wxStaticText(itemPanelShip, wxID_ANY, _("Show range rings"));
  rrSelect->Add(rrTxt, 1, wxEXPAND | wxALL, group_item_spacing);

  wxString rrAlt[] = {_("None"), _T( "1" ), _T( "2" ), _T( "3" ),
                      _T( "4" ), _T( "5" ), _T( "6" ), _T( "7" ),
                      _T( "8" ), _T( "9" ), _T( "10" )};
  pNavAidRadarRingsNumberVisible =
      new wxChoice(itemPanelShip, ID_RADARRINGS, wxDefaultPosition,
                   m_pShipIconType->GetSize(), 11, rrAlt);
  rrSelect->Add(pNavAidRadarRingsNumberVisible, 0, wxALIGN_RIGHT | wxALL,
                group_item_spacing);

  radarGrid = new wxFlexGridSizer(0, 2, group_item_spacing, group_item_spacing);
  radarGrid->AddGrowableCol(1);
  dispOptions->Add(radarGrid, 0, wxLEFT | wxEXPAND, 30);

  wxStaticText* distanceText =
      new wxStaticText(itemPanelShip, wxID_STATIC, _("Distance between rings"));
  radarGrid->Add(distanceText, 1, wxEXPAND | wxALL, group_item_spacing);

  pNavAidRadarRingsStep = new wxTextCtrl(itemPanelShip, ID_OPTEXTCTRL, _T(""),
                                         wxDefaultPosition, wxSize(100, -1), 0);
  radarGrid->Add(pNavAidRadarRingsStep, 0, wxALIGN_RIGHT | wxALL,
                 group_item_spacing);

  wxStaticText* unitText =
      new wxStaticText(itemPanelShip, wxID_STATIC, _("Distance Unit"));
  radarGrid->Add(unitText, 1, wxEXPAND | wxALL, group_item_spacing);

  wxString pDistUnitsStrings[] = {_("Nautical miles"), _("Kilometers")};
  m_itemRadarRingsUnits =
      new wxChoice(itemPanelShip, ID_RADARDISTUNIT, wxDefaultPosition,
                   m_pShipIconType->GetSize(), 2, pDistUnitsStrings);
  radarGrid->Add(m_itemRadarRingsUnits, 0, wxALIGN_RIGHT | wxALL, border_size);

  wxStaticText* colourText =
      new wxStaticText(itemPanelShip, wxID_STATIC, _("Range Ring Colour"));
  radarGrid->Add(colourText, 1, wxEXPAND | wxALL, group_item_spacing);

  m_colourOwnshipRangeRingColour = new OCPNColourPickerCtrl(
      itemPanelShip, wxID_ANY, *wxRED, wxDefaultPosition,
      m_colourPickerDefaultSize, 0, wxDefaultValidator,
      _T( "ID_COLOUROSRANGECOLOUR" ));
  radarGrid->Add(m_colourOwnshipRangeRingColour, 0, wxALIGN_RIGHT, border_size);

  // ship to active
  wxFlexGridSizer* shipToActiveGrid =
      new wxFlexGridSizer(1, 5, group_item_spacing, group_item_spacing);
  shipToActiveGrid->AddGrowableCol(1);
  dispOptions->Add(shipToActiveGrid, 0, wxALL | wxEXPAND, border_size);
  pShowshipToActive = new wxCheckBox(itemPanelShip, wxID_ANY,
                                     _("Show direction to Active Waypoint"));
  shipToActiveGrid->Add(pShowshipToActive, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT,
                        border_size);

  wxStaticText* shipToActiveText1 =
      new wxStaticText(itemPanelShip, wxID_STATIC, _("Style"));
  shipToActiveGrid->Add(shipToActiveText1, 1,
                        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, border_size);

  wxString LineStyleChoices[] = {_("Default"),    _("Solid"),
                                 _("Dot"),        _("Long dash"),
                                 _("Short dash"), _("Dot dash")};
  int LineStyleNChoices = sizeof(LineStyleChoices) / sizeof(wxString);
  m_shipToActiveStyle =
      new wxChoice(itemPanelShip, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   LineStyleNChoices, LineStyleChoices, 0);
  m_shipToActiveStyle->SetSelection(0);
  shipToActiveGrid->Add(m_shipToActiveStyle, 0, wxALL, 5);

  wxStaticText* shipToActiveText2 =
      new wxStaticText(itemPanelShip, wxID_STATIC, _("Color"));
  shipToActiveGrid->Add(shipToActiveText2, 1,
                        wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, border_size);

  wxString m_LineColorChoices[] = {
      ("Default color"), _("Black"),     _("Dark Red"),     _("Dark Green"),
      _("Dark Yellow"),  _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
      _("Light Gray"),   _("Dark Gray"), _("Red"),          _("Green"),
      _("Yellow"),       _("Blue"),      _("Magenta"),      _("Cyan"),
      _("White")};
  int LineColorNChoices = sizeof(m_LineColorChoices) / sizeof(wxString);
  m_shipToActiveColor =
      new wxChoice(itemPanelShip, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   LineColorNChoices, m_LineColorChoices, 0);
  m_shipToActiveColor->SetSelection(0);
  shipToActiveGrid->Add(m_shipToActiveColor, 0, wxALL, 5);

  //  Tracks
  wxStaticBox* trackText =
      new wxStaticBox(itemPanelShip, wxID_ANY, _("Tracks"));
  wxStaticBoxSizer* trackSizer = new wxStaticBoxSizer(trackText, wxVERTICAL);
  wxBoxSizer* trackSizer1 = new wxBoxSizer(wxHORIZONTAL);
  ownShip->Add(trackSizer, 0, wxGROW | wxALL, border_size);

  pTrackDaily = new wxCheckBox(itemPanelShip, ID_DAILYCHECKBOX,
                               _("Automatic Daily Tracks at midnight"));

  trackSizer1->Add(pTrackDaily, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT,
                   border_size);

  trackSizer1->Add(0, 0, 1, wxEXPAND, 0);

#if wxCHECK_VERSION(2, 9, 0)
#if wxUSE_TIMEPICKCTRL
  pTrackDaily->SetLabel(_("Automatic Daily Tracks at"));
#ifdef __WXGTK__
  pTrackRotateTime =
      new TimeCtrl(itemPanelShip, ID_TRACKROTATETIME,
                   wxDateTime((time_t)g_track_rotate_time).ToUTC(),
                   wxDefaultPosition, wxDefaultSize, 0);
#else
  pTrackRotateTime =
      new wxTimePickerCtrl(itemPanelShip, ID_TRACKROTATETIME,
                           wxDateTime((time_t)g_track_rotate_time).ToUTC(),
                           wxDefaultPosition, wxDefaultSize, 0);
#endif
  trackSizer1->Add(pTrackRotateTime, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT,
                   border_size);
#endif
#endif

  pTrackRotateComputerTime =
      new wxRadioButton(itemPanelShip, ID_TRACKROTATECOMPUTER, _("Computer"),
                        wxDefaultPosition, wxDefaultSize, 0);
  trackSizer1->Add(pTrackRotateComputerTime, 0,
                   wxALIGN_CENTER_VERTICAL | wxRIGHT, border_size);

  pTrackRotateUTC =
      new wxRadioButton(itemPanelShip, ID_TRACKROTATEUTC, _("UTC"),
                        wxDefaultPosition, wxDefaultSize, 0);
  trackSizer1->Add(pTrackRotateUTC, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT,
                   border_size);

  pTrackRotateLMT =
      new wxRadioButton(itemPanelShip, ID_TRACKROTATELMT, _("LMT"),
                        wxDefaultPosition, wxDefaultSize, 0);
  trackSizer1->Add(pTrackRotateLMT, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT,
                   border_size);

  trackSizer->Add(trackSizer1, 1, wxEXPAND | wxALL, border_size);

  wxFlexGridSizer* hTrackGrid =
      new wxFlexGridSizer(1, 3, group_item_spacing, group_item_spacing);
  hTrackGrid->AddGrowableCol(1);
  trackSizer->Add(hTrackGrid, 0, wxALL | wxEXPAND, border_size);

  pTrackHighlite =
      new wxCheckBox(itemPanelShip, ID_TRACKHILITE, _("Highlight Tracks"));
  hTrackGrid->Add(pTrackHighlite, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,
                  border_size);
  wxStaticText* trackColourText =
      new wxStaticText(itemPanelShip, wxID_STATIC, _("Track Colour"));
  hTrackGrid->Add(trackColourText, 1, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
                  border_size);
  m_colourTrackLineColour = new OCPNColourPickerCtrl(
      itemPanelShip, wxID_STATIC, *wxRED, wxDefaultPosition,
      m_colourPickerDefaultSize, 0, wxDefaultValidator,
      _T( "ID_COLOURTRACKCOLOUR" ));
  hTrackGrid->Add(m_colourTrackLineColour, 1, wxALIGN_RIGHT, border_size);

  wxFlexGridSizer* pTrackGrid =
      new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
  pTrackGrid->AddGrowableCol(1);
  trackSizer->Add(pTrackGrid, 0, wxALL | wxEXPAND, border_size);

  wxStaticText* tpText =
      new wxStaticText(itemPanelShip, wxID_STATIC, _("Tracking Precision"));
  pTrackGrid->Add(tpText, 1, wxEXPAND | wxALL, group_item_spacing);

  wxString trackAlt[] = {_("Low"), _("Medium"), _("High")};
  pTrackPrecision = new wxChoice(itemPanelShip, wxID_ANY, wxDefaultPosition,
                                 m_pShipIconType->GetSize(), 3, trackAlt);
  pTrackGrid->Add(pTrackPrecision, 0, wxALIGN_RIGHT | wxALL,
                  group_item_spacing);

  //  Calculate values
  wxStaticBox* ownshipcalcText =
      new wxStaticBox(itemPanelShip, wxID_ANY, _("Calculate values"));
  wxStaticBoxSizer* ownshipcalcSizer =
      new wxStaticBoxSizer(ownshipcalcText, wxVERTICAL);
  ownShip->Add(ownshipcalcSizer, 0, wxTOP | wxALL | wxEXPAND, border_size);

  wxFlexGridSizer* dispOwnShipCalcOptionsGrid =
      new wxFlexGridSizer(2, 2, group_item_spacing, group_item_spacing);
  ownshipcalcSizer->Add(dispOwnShipCalcOptionsGrid, 0, wxTOP | wxALL | wxEXPAND,
                        border_size);

  dispOwnShipCalcOptionsGrid->AddGrowableCol(1);

  pSogCogFromLLCheckBox =
      new wxCheckBox(itemPanelShip, ID_SOGCOGFROMLLCHECKBOX,
                     _("Calculate SOG and COG from position changes"));
  dispOwnShipCalcOptionsGrid->Add(pSogCogFromLLCheckBox, 1, wxALL, 5);
  dispOwnShipCalcOptionsGrid->AddSpacer(0);

  wxStaticText* SogCogFromLLDampIntText = new wxStaticText(
      itemPanelShip, wxID_STATIC, _("Min seconds between updates"));
  dispOwnShipCalcOptionsGrid->Add(SogCogFromLLDampIntText, 1, wxEXPAND | wxALL,
                                  group_item_spacing);

  pSogCogFromLLDampInterval = new wxSpinCtrl(
      itemPanelShip, ID_SOGCOGDAMPINTTEXTCTRL, wxEmptyString, wxDefaultPosition,
      wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0);

  dispOwnShipCalcOptionsGrid->Add(pSogCogFromLLDampInterval, 0,
                                  wxALIGN_RIGHT | wxALL, group_item_spacing);

  // DimeControl(itemPanelShip);
}

void options::CreatePanel_Routes(size_t parent, int border_size,
                                 int group_item_spacing) {
  itemPanelRoutes = AddPage(parent, _("Routes/Points"));

  Routes = new wxBoxSizer(wxVERTICAL);
  itemPanelRoutes->SetSizer(Routes);

  wxString pDistUnitsStrings[] = {_("Nautical miles"), _("Kilometers")};

  wxString rrAlt[] = {_("None"), _T( "1" ), _T( "2" ), _T( "3" ),
                      _T( "4" ), _T( "5" ), _T( "6" ), _T( "7" ),
                      _T( "8" ), _T( "9" ), _T( "10" )};

  //  Routes
  wxStaticBox* routeText =
      new wxStaticBox(itemPanelRoutes, wxID_ANY, _("New Routes"));
  wxStaticBoxSizer* routeSizer = new wxStaticBoxSizer(routeText, wxVERTICAL);
  Routes->Add(routeSizer, 0, wxGROW | wxALL, border_size);

  routeSizer->AddSpacer(5);

  // Default ICON
  wxFlexGridSizer* routepointiconSelect =
      new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
  routepointiconSelect->AddGrowableCol(1);
  routeSizer->Add(routepointiconSelect, 0, wxLEFT | wxRIGHT | wxEXPAND,
                  border_size);

  wxStaticText* routepointiconTxt =
      new wxStaticText(itemPanelRoutes, wxID_ANY, _("Waypoint default icon"));
  routepointiconSelect->Add(routepointiconTxt, 1, wxEXPAND | wxALL,
                            group_item_spacing);

  pRoutepointDefaultIconChoice = new OCPNIconCombo(
      itemPanelRoutes, wxID_ANY, _("Combo!"), wxDefaultPosition, wxDefaultSize,
      0, NULL, wxCB_READONLY);
  routepointiconSelect->Add(pRoutepointDefaultIconChoice, 0,
                            wxALIGN_RIGHT | wxALL, group_item_spacing);

  pRoutepointDefaultIconChoice->SetPopupMaxHeight(::wxGetDisplaySize().y / 2);

  //  Accomodate scaling of icon
  int min_size = GetCharHeight() * 2;
  min_size = wxMax(min_size, (32 * g_ChartScaleFactorExp) + 4);
  pRoutepointDefaultIconChoice->SetMinSize(
      wxSize(GetCharHeight() * 15, min_size));

  routeSizer->AddSpacer(5);

  wxFlexGridSizer* pRouteGrid =
      new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
  pRouteGrid->AddGrowableCol(1);
  routeSizer->Add(pRouteGrid, 0, wxALL | wxEXPAND, border_size);

  wxStaticText* raText = new wxStaticText(
      itemPanelRoutes, wxID_STATIC, _("Waypoint Arrival Circle Radius (NMi)"));
  pRouteGrid->Add(raText, 1, wxEXPAND | wxALL, group_item_spacing);

  m_pText_ACRadius = new wxTextCtrl(itemPanelRoutes, -1);
  pRouteGrid->Add(m_pText_ACRadius, 0, wxALL | wxALIGN_RIGHT,
                  group_item_spacing);

  pAdvanceRouteWaypointOnArrivalOnly =
      new wxCheckBox(itemPanelRoutes, ID_DAILYCHECKBOX,
                     _("Advance route waypoint on arrival only"));
  routeSizer->Add(pAdvanceRouteWaypointOnArrivalOnly, 0, wxALL, 5);

#ifdef __WXGTK__
  Routes->AddSpacer(8 * group_item_spacing);
  wxStaticLine* pln =
      new wxStaticLine(itemPanelRoutes, wxID_ANY, wxDefaultPosition,
                       wxDefaultSize, wxLI_HORIZONTAL);
  Routes->Add(pln, 0, wxEXPAND);
#endif

  //  Waypoints
  wxStaticBox* waypointText =
      new wxStaticBox(itemPanelRoutes, wxID_ANY, _("New Waypoints"));
  wxStaticBoxSizer* waypointSizer =
      new wxStaticBoxSizer(waypointText, wxVERTICAL);
  Routes->Add(waypointSizer, 0, wxTOP | wxALL | wxEXPAND, border_size);

  waypointSizer->AddSpacer(5);

  // Default ICON
  wxFlexGridSizer* waypointiconSelect =
      new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
  waypointiconSelect->AddGrowableCol(1);
  waypointSizer->Add(waypointiconSelect, 0, wxLEFT | wxRIGHT | wxEXPAND,
                     border_size);

  wxStaticText* waypointiconTxt =
      new wxStaticText(itemPanelRoutes, wxID_ANY, _("Waypoint default icon"));
  waypointiconSelect->Add(waypointiconTxt, 1, wxEXPAND | wxALL,
                          group_item_spacing);

  pWaypointDefaultIconChoice = new OCPNIconCombo(
      itemPanelRoutes, wxID_ANY, _("Combo!"), wxDefaultPosition, wxDefaultSize,
      0, NULL, wxCB_READONLY);
  waypointiconSelect->Add(pWaypointDefaultIconChoice, 0, wxALIGN_RIGHT | wxALL,
                          group_item_spacing);

  pWaypointDefaultIconChoice->SetPopupMaxHeight(::wxGetDisplaySize().y / 2);

  //  Accomodate scaling of icon
  int rmin_size = GetCharHeight() * 2;
  min_size = wxMax(rmin_size, (32 * g_ChartScaleFactorExp) + 4);
  pWaypointDefaultIconChoice->SetMinSize(
      wxSize(GetCharHeight() * 15, rmin_size));

  waypointSizer->AddSpacer(5);
  // ScaMin
  wxFlexGridSizer* ScaMinSizer =
      new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
  ScaMinSizer->AddGrowableCol(1);
  waypointSizer->Add(ScaMinSizer, 0, wxLEFT | wxRIGHT | wxEXPAND, border_size);
  pScaMinChckB =
      new wxCheckBox(itemPanelRoutes, wxID_ANY,
                     _("Show waypoints only at a chartscale greater than 1 :"));
  ScaMinSizer->Add(pScaMinChckB, 0);
  m_pText_ScaMin = new wxTextCtrl(itemPanelRoutes, -1);
  ScaMinSizer->Add(m_pText_ScaMin, 0, wxALL | wxALIGN_RIGHT,
                   group_item_spacing);

  // Overrule the Scamin settings
  pScaMinOverruleChckB =
      new wxCheckBox(itemPanelRoutes, wxID_ANY,
                     _("Override the settings for chartscale based visibility "
                       "and show always"));
  waypointSizer->Add(pScaMinOverruleChckB, 0, wxALL, 5);

  waypointSizer->AddSpacer(5);
  // Range Rings
  wxFlexGridSizer* waypointrrSelect =
      new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
  waypointrrSelect->AddGrowableCol(1);
  waypointSizer->Add(waypointrrSelect, 0, wxLEFT | wxRIGHT | wxEXPAND,
                     border_size);

  wxStaticText* waypointrrTxt =
      new wxStaticText(itemPanelRoutes, wxID_ANY, _("Waypoint range rings"));
  waypointrrSelect->Add(waypointrrTxt, 1, wxEXPAND | wxALL, group_item_spacing);

  pWaypointRangeRingsNumber =
      new wxChoice(itemPanelRoutes, ID_OPWAYPOINTRANGERINGS, wxDefaultPosition,
                   m_pShipIconType->GetSize(), 11, rrAlt);
  waypointrrSelect->Add(pWaypointRangeRingsNumber, 0, wxALIGN_RIGHT | wxALL,
                        group_item_spacing);

  waypointradarGrid =
      new wxFlexGridSizer(0, 2, group_item_spacing, group_item_spacing);
  waypointradarGrid->AddGrowableCol(1);
  waypointSizer->Add(waypointradarGrid, 0, wxLEFT | wxEXPAND, 30);

  wxStaticText* waypointdistanceText = new wxStaticText(
      itemPanelRoutes, wxID_STATIC, _("Distance between rings"));
  waypointradarGrid->Add(waypointdistanceText, 1, wxEXPAND | wxALL,
                         group_item_spacing);

  pWaypointRangeRingsStep =
      new wxTextCtrl(itemPanelRoutes, ID_OPTEXTCTRL, _T(""), wxDefaultPosition,
                     wxSize(100, -1), 0);
  waypointradarGrid->Add(pWaypointRangeRingsStep, 0, wxALIGN_RIGHT | wxALL,
                         group_item_spacing);

  wxStaticText* waypointunitText =
      new wxStaticText(itemPanelRoutes, wxID_STATIC, _("Distance Unit"));
  waypointradarGrid->Add(waypointunitText, 1, wxEXPAND | wxALL,
                         group_item_spacing);

  m_itemWaypointRangeRingsUnits =
      new wxChoice(itemPanelRoutes, ID_RADARDISTUNIT, wxDefaultPosition,
                   m_pShipIconType->GetSize(), 2, pDistUnitsStrings);
  waypointradarGrid->Add(m_itemWaypointRangeRingsUnits, 0,
                         wxALIGN_RIGHT | wxALL, border_size);

  wxStaticText* waypointrangeringsColour = new wxStaticText(
      itemPanelRoutes, wxID_STATIC, _("Waypoint Range Ring Colours"));
  waypointradarGrid->Add(waypointrangeringsColour, 1, wxEXPAND | wxALL, 1);

  m_colourWaypointRangeRingsColour = new OCPNColourPickerCtrl(
      itemPanelRoutes, wxID_ANY, *wxRED, wxDefaultPosition,
      m_colourPickerDefaultSize, 0, wxDefaultValidator,
      _T( "ID_COLOURWAYPOINTRANGERINGSCOLOUR" ));
  waypointradarGrid->Add(m_colourWaypointRangeRingsColour, 0,
                         wxALIGN_RIGHT | wxALL, 1);

  // Control Options

#ifdef __WXGTK__
  Routes->AddSpacer(8 * group_item_spacing);
  wxStaticLine* pln1 =
      new wxStaticLine(itemPanelRoutes, wxID_ANY, wxDefaultPosition,
                       wxDefaultSize, wxLI_HORIZONTAL);
  Routes->Add(pln1, 0, wxEXPAND);
#endif

  wxStaticBox* waypointControl =
      new wxStaticBox(itemPanelRoutes, wxID_ANY, _("Control Options"));
  wxStaticBoxSizer* ControlSizer =
      new wxStaticBoxSizer(waypointControl, wxVERTICAL);
  Routes->Add(ControlSizer, 0, wxTOP | wxALL | wxEXPAND, border_size);

  ControlSizer->AddSpacer(5);

  pWayPointPreventDragging = new wxCheckBox(
      itemPanelRoutes, ID_DRAGGINGCHECKBOX,
      _("Lock Waypoints (Unless waypoint property dialog visible)"));
  pWayPointPreventDragging->SetValue(FALSE);
  ControlSizer->Add(pWayPointPreventDragging, verticleInputFlags);

  pConfirmObjectDeletion =
      new wxCheckBox(itemPanelRoutes, ID_DELETECHECKBOX,
                     _("Confirm deletion of tracks and routes"));
  pConfirmObjectDeletion->SetValue(FALSE);
  ControlSizer->Add(pConfirmObjectDeletion, verticleInputFlags);
  ControlSizer->AddSpacer(5);

  // Fill the default waypoint icon selector combo box
  pWaypointDefaultIconChoice->Clear();
  //      Iterate on the Icon Descriptions, filling in the combo control
  bool fillCombo = true;

  if (fillCombo) {
    for (int i = 0; i < pWayPointMan->GetNumIcons(); i++) {
      wxString* ps = pWayPointMan->GetIconDescription(i);
      wxBitmap bmp = pWayPointMan->GetIconBitmapForList(i, 2 * GetCharHeight());

      pWaypointDefaultIconChoice->Append(*ps, bmp);
    }
  }

  // find the correct item in the combo box
  int iconToSelect = -1;
  for (int i = 0; i < pWayPointMan->GetNumIcons(); i++) {
    if (*pWayPointMan->GetIconKey(i) == g_default_wp_icon) {
      iconToSelect = i;
      pWaypointDefaultIconChoice->Select(iconToSelect);
      break;
    }
  }

  // Fill the default Routepoint icon selector combo box
  pRoutepointDefaultIconChoice->Clear();
  //      Iterate on the Icon Descriptions, filling in the combo control
  fillCombo = true;

  if (fillCombo) {
    for (int i = 0; i < pWayPointMan->GetNumIcons(); i++) {
      wxString* ps = pWayPointMan->GetIconDescription(i);
      wxBitmap bmp = pWayPointMan->GetIconBitmapForList(i, 2 * GetCharHeight());

      pRoutepointDefaultIconChoice->Append(*ps, bmp);
    }
  }

  // find the correct item in the combo box
  iconToSelect = -1;
  for (int i = 0; i < pWayPointMan->GetNumIcons(); i++) {
    if (*pWayPointMan->GetIconKey(i) == g_default_routepoint_icon) {
      iconToSelect = i;
      pRoutepointDefaultIconChoice->Select(iconToSelect);
      break;
    }
  }

  // DimeControl(itemPanelRoutes);
}

void options::CreatePanel_ChartsLoad(size_t parent, int border_size,
                                     int group_item_spacing) {
  chartPanelWin = AddPage(m_pageCharts, _("Chart Files"));

  chartPanel = new wxBoxSizer(wxVERTICAL);
  chartPanelWin->SetSizer(chartPanel);

  loadedBox = new wxStaticBox(chartPanelWin, wxID_ANY, _("Directories"));
  activeSizer = new wxStaticBoxSizer(loadedBox, wxHORIZONTAL);
  chartPanel->Add(activeSizer, 1, wxALL | wxEXPAND, border_size);

  m_scrollWinChartList = new wxScrolledWindow(
      chartPanelWin, wxID_ANY, wxDefaultPosition,
      wxDLG_UNIT(this, wxSize(-1, -1)), wxBORDER_RAISED | wxVSCROLL);

  activeSizer->Add(m_scrollWinChartList, 1, wxALL | wxEXPAND, 5);

#ifndef __OCPN__ANDROID__
  m_scrollRate = 5;
#else
  m_scrollRate = 1;
#endif
  m_scrollWinChartList->SetScrollRate(m_scrollRate, m_scrollRate);

  boxSizerCharts = new wxBoxSizer(wxVERTICAL);
  m_scrollWinChartList->SetSizer(boxSizerCharts);

  cmdButtonSizer = new wxBoxSizer(wxVERTICAL);
  activeSizer->Add(cmdButtonSizer, 0, wxALL, border_size);

  wxString b1 = _("Add Directory...");
  wxString b2 = _("Remove Selected");
  wxString b3 = _("Compress Selected");

  if (m_bcompact) {
    b1 = _("Add..");
    b2 = _("Remove");
    b3 = _("Compress");
  }

  wxButton* addBtn = new wxButton(chartPanelWin, ID_BUTTONADD, b1);
  cmdButtonSizer->Add(addBtn, 1, wxALL | wxEXPAND, group_item_spacing);

  cmdButtonSizer->AddSpacer(GetCharHeight());

  m_removeBtn = new wxButton(chartPanelWin, ID_BUTTONDELETE, b2);
  cmdButtonSizer->Add(m_removeBtn, 1, wxALL | wxEXPAND, group_item_spacing);
  m_removeBtn->Disable();

  cmdButtonSizer->AddSpacer(GetCharHeight());

#ifdef OCPN_USE_LZMA
  m_compressBtn = new wxButton(chartPanelWin, ID_BUTTONCOMPRESS, b3);
  cmdButtonSizer->Add(m_compressBtn, 1, wxALL | wxEXPAND, group_item_spacing);
  m_compressBtn->Disable();
#else
  m_compressBtn = NULL;
#endif

#ifdef __OCPN__ANDROID__
  if (g_Android_SDK_Version >= 30) {
    m_migrateBtn =
        new wxButton(chartPanelWin, ID_BUTTONMIGRATE, _("Migrate Charts.."));
    cmdButtonSizer->Add(m_migrateBtn, 1, wxALL | wxEXPAND, group_item_spacing);
  }
#endif

  cmdButtonSizer->AddSpacer(GetCharHeight());

  wxStaticBox* itemStaticBoxUpdateStatic =
      new wxStaticBox(chartPanelWin, wxID_ANY, _("Update Control"));
  wxStaticBoxSizer* itemStaticBoxSizerUpdate =
      new wxStaticBoxSizer(itemStaticBoxUpdateStatic, wxVERTICAL);
  chartPanel->Add(itemStaticBoxSizerUpdate, 0, wxGROW | wxALL, 5);

  wxFlexGridSizer* itemFlexGridSizerUpdate = new wxFlexGridSizer(1);
  itemFlexGridSizerUpdate->SetFlexibleDirection(wxHORIZONTAL);

  pScanCheckBox = new wxCheckBox(chartPanelWin, ID_SCANCHECKBOX,
                                 _("Scan Charts and Update Database"));
  itemFlexGridSizerUpdate->Add(pScanCheckBox, 1, wxALL, 5);

  pUpdateCheckBox = new wxCheckBox(chartPanelWin, ID_UPDCHECKBOX,
                                   _("Force Full Database Rebuild"));
  itemFlexGridSizerUpdate->Add(pUpdateCheckBox, 1, wxALL, 5);

  pParseENCButton = new wxButton(chartPanelWin, ID_PARSEENCBUTTON,
                                 _("Prepare all ENC Charts"));
  itemFlexGridSizerUpdate->Add(pParseENCButton, 1, wxALL, 5);

  itemStaticBoxSizerUpdate->Add(itemFlexGridSizerUpdate, 1, wxEXPAND, 5);

  // Currently loaded chart dirs
  ActiveChartArray.Clear();
  for (size_t i = 0; i < m_CurrentDirList.GetCount(); i++) {
    ActiveChartArray.Add(m_CurrentDirList[i]);
  }

  UpdateChartDirList();

  chartPanel->Layout();
}

void options::UpdateChartDirList() {
  // Clear the sizer, and delete all the child panels
  m_scrollWinChartList->GetSizer()->Clear(true);
  m_scrollWinChartList->ClearBackground();

  panelVector.clear();

  // Add new panels
  for (size_t i = 0; i < ActiveChartArray.GetCount(); i++) {
    OCPNChartDirPanel* chartPanel =
        new OCPNChartDirPanel(m_scrollWinChartList, wxID_ANY, wxDefaultPosition,
                              wxSize(-1, -1), ActiveChartArray[i]);
    chartPanel->SetSelected(false);

    m_scrollWinChartList->GetSizer()->Add(chartPanel, 0, wxEXPAND | wxALL, 0);

    panelVector.push_back(chartPanel);
  }

  m_scrollWinChartList->GetSizer()->Layout();

  chartPanelWin->ClearBackground();
  chartPanelWin->Layout();

  // There are some problems with wxScrolledWindow after add/removing items.
  // Typically, the problem is that blank space remains at the top of the
  // scrollable range of the window.
  // Workarounds here...
  // n.b. according to wx docs, none of this should be necessary...
#ifdef __OCPN__ANDROID__
  // This works on Android, but seems pretty drastic
  wxSize sza = GetSize();
  sza.y -= 1;
  SetSize(sza);
#else
  // This works, except on Android
  m_scrollWinChartList->GetParent()->Layout();
#endif

  m_scrollWinChartList->Scroll(0, 0);
}

void options::UpdateTemplateTitleText() {
  if (!m_templateTitleText) return;

  wxString activeTitle;
  if (!g_lastAppliedTemplateGUID.IsEmpty()) {
    activeTitle = ConfigMgr::Get().GetTemplateTitle(g_lastAppliedTemplateGUID);

    bool configCompare =
        ConfigMgr::Get().CheckTemplateGUID(g_lastAppliedTemplateGUID);
    if (!configCompare) activeTitle += _(" [Modified]");
    m_templateTitleText->SetLabel(activeTitle);
  } else
    m_templateTitleText->SetLabel(_("None"));
}

void options::CreatePanel_Configs(size_t parent, int border_size,
                                  int group_item_spacing) {
  m_DisplayConfigsPage = AddPage(parent, _("Templates"));

  // if (m_bcompact) {
  //}
  // else
  {
    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    m_DisplayConfigsPage->SetSizer(wrapperSizer);

    // Template management

    wxStaticBox* templateStatusBox =
        new wxStaticBox(m_DisplayConfigsPage, wxID_ANY, _("Template Status"));
    m_templateStatusBoxSizer =
        new wxStaticBoxSizer(templateStatusBox, wxHORIZONTAL);
    wrapperSizer->Add(m_templateStatusBoxSizer, 1, wxALL | wxEXPAND,
                      border_size);

    wxBoxSizer* statSizer = new wxBoxSizer(wxVERTICAL);
    m_templateStatusBoxSizer->Add(statSizer, 0, wxALL | wxEXPAND, border_size);

    m_staticTextLastAppled = new wxStaticText(
        m_DisplayConfigsPage, wxID_ANY, _("Last Applied Template Title:"));
    m_staticTextLastAppled->Hide();

    statSizer->Add(m_staticTextLastAppled);

    m_templateTitleText =
        new wxStaticText(m_DisplayConfigsPage, wxID_ANY, wxEmptyString);
    statSizer->Add(m_templateTitleText);
    m_templateTitleText->Hide();

    UpdateTemplateTitleText();

    wxStaticBox* configsBox =
        new wxStaticBox(m_DisplayConfigsPage, wxID_ANY, _("Saved Templates"));
    wxStaticBoxSizer* configSizer =
        new wxStaticBoxSizer(configsBox, wxHORIZONTAL);
    wrapperSizer->Add(configSizer, 4, wxALL | wxEXPAND, border_size);

    wxPanel* cPanel = new wxPanel(m_DisplayConfigsPage, wxID_ANY);
    configSizer->Add(cPanel, 1, wxALL | wxEXPAND, border_size);

    wxBoxSizer* boxSizercPanel = new wxBoxSizer(wxVERTICAL);
    cPanel->SetSizer(boxSizercPanel);

    m_scrollWinConfigList =
        new wxScrolledWindow(cPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                             wxBORDER_RAISED | wxVSCROLL);
    m_scrollWinConfigList->SetScrollRate(1, 1);
    boxSizercPanel->Add(m_scrollWinConfigList, 0, wxALL | wxEXPAND,
                        border_size);
    m_scrollWinConfigList->SetMinSize(wxSize(-1, 15 * GetCharHeight()));

    m_boxSizerConfigs = new wxBoxSizer(wxVERTICAL);
    m_scrollWinConfigList->SetSizer(m_boxSizerConfigs);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxVERTICAL);
    configSizer->Add(btnSizer);

    //    Add the "Insert/Remove" buttons
    wxButton* createButton =
        new wxButton(m_DisplayConfigsPage, wxID_ANY, _("Create Config..."));
    btnSizer->Add(createButton, 1, wxALL | wxEXPAND, group_item_spacing);
    createButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                          wxCommandEventHandler(options::OnCreateConfig), NULL,
                          this);

    // wxButton* editButton = new wxButton(m_DisplayConfigsPage, wxID_ANY,
    // _("Edit Config...")); btnSizer->Add(editButton, 1, wxALL | wxEXPAND,
    // group_item_spacing); editButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
    // wxCommandEventHandler(options::OnEditConfig), NULL, this);

    m_configDeleteButton = new wxButton(m_DisplayConfigsPage, wxID_ANY,
                                        _("Delete Selected Config..."));
    btnSizer->Add(m_configDeleteButton, 1, wxALL | wxEXPAND,
                  group_item_spacing);
    m_configDeleteButton->Connect(
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(options::OnDeleteConfig), NULL, this);

    m_configApplyButton = new wxButton(m_DisplayConfigsPage, wxID_ANY,
                                       _("Apply Selected Config"));
    btnSizer->Add(m_configApplyButton, 1, wxALL | wxEXPAND, group_item_spacing);
    m_configApplyButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                                 wxCommandEventHandler(options::OnApplyConfig),
                                 NULL, this);

    // Populate the configs list from the ConfigMgr
    ClearConfigList();
    BuildConfigList();

    SetConfigButtonState();
  }
}

void options::ClearConfigList() {
  if (m_scrollWinConfigList) {
    wxWindowList kids = m_scrollWinConfigList->GetChildren();
    for (unsigned int i = 0; i < kids.GetCount(); i++) {
      wxWindowListNode* node = kids.Item(i);
      wxWindow* win = node->GetData();
      wxPanel* pcp = wxDynamicCast(win, wxPanel);
      if (pcp) {
        ConfigPanel* cPanel = wxDynamicCast(pcp, ConfigPanel);
        if (cPanel) {
          cPanel->Destroy();
        }
      }
    }
  }
  SetConfigButtonState();
}

void options::BuildConfigList() {
  wxArrayString configGUIDs = ConfigMgr::Get().GetConfigGUIDArray();

  for (size_t i = 0; i < configGUIDs.GetCount(); i++) {
    wxPanel* pp =
        ConfigMgr::Get().GetConfigPanel(m_scrollWinConfigList, configGUIDs[i]);
    if (pp) {
      m_panelBackgroundUnselected = pp->GetBackgroundColour();
      m_boxSizerConfigs->Add(pp, 1, wxEXPAND);
      pp->Connect(wxEVT_LEFT_DOWN,
                  wxMouseEventHandler(options::OnConfigMouseSelected), NULL,
                  this);

      //  Set mouse handler for children of the panel, too.
      wxWindowList kids = pp->GetChildren();
      for (unsigned int i = 0; i < kids.GetCount(); i++) {
        wxWindowListNode* node = kids.Item(i);
        wxWindow* win = node->GetData();
        win->Connect(wxEVT_LEFT_DOWN,
                     wxMouseEventHandler(options::OnConfigMouseSelected), NULL,
                     this);
      }
    }
  }

  m_boxSizerConfigs->Layout();

  m_selectedConfigPanelGUID = _T("");
  SetConfigButtonState();
}

void options::SetConfigButtonState() {
  m_configDeleteButton->Enable(!m_selectedConfigPanelGUID.IsEmpty());
  if (m_selectedConfigPanelGUID.StartsWith(
          _T("11111111")))  // Cannot delete "Recovery" template
    m_configDeleteButton->Disable();
  m_configApplyButton->Enable(!m_selectedConfigPanelGUID.IsEmpty());
}

void options::OnCreateConfig(wxCommandEvent& event) {
  ConfigCreateDialog* pd = new ConfigCreateDialog(
      this, -1, _("Create Config"), wxDefaultPosition, wxSize(200, 200));
  DimeControl(pd);
  pd->ShowWindowModalThenDo([this, pd](int retcode) {
    if (retcode == wxID_OK) {
      g_lastAppliedTemplateGUID = pd->GetCreatedTemplateGUID();
      UpdateTemplateTitleText();

      ClearConfigList();
      BuildConfigList();
      m_DisplayConfigsPage->Layout();
    }
    SetConfigButtonState();
  });
}

void options::OnEditConfig(wxCommandEvent& event) {}

void options::OnDeleteConfig(wxCommandEvent& event) {
  if (m_selectedConfigPanelGUID.IsEmpty()) return;

  ConfigMgr::Get().DeleteConfig(m_selectedConfigPanelGUID);
  m_selectedConfigPanelGUID = _T("");

  ClearConfigList();
  BuildConfigList();

  m_DisplayConfigsPage->Layout();
  SetConfigButtonState();
}

void options::OnApplyConfig(wxCommandEvent& event) {
  if (m_selectedConfigPanelGUID.IsEmpty()) return;

  // Record a few special items
  wxString currentLocale = g_locale;

  //  Apply any changed settings other than the target config template.
  wxCommandEvent evt;
  evt.SetId(ID_APPLY);
  OnApplyClick(evt);

  // Then Apply the target config template
  bool bApplyStat = ConfigMgr::Get().ApplyConfigGUID(m_selectedConfigPanelGUID);
  if (bApplyStat) {
    //        OCPNMessageBox(this, _("Configuration successfully applied."),
    //        _("OpenCPN Info"), wxOK);
    g_lastAppliedTemplateGUID = m_selectedConfigPanelGUID;
    wxString activeTitle =
        ConfigMgr::Get().GetTemplateTitle(g_lastAppliedTemplateGUID);
    m_templateTitleText->SetLabel(activeTitle);
    m_templateTitleText->Show();
    m_staticTextLastAppled->Show();
    m_templateStatusBoxSizer->Layout();
  } else
    OCPNMessageBox(this, _("Problem applying selected configuration."),
                   _("OpenCPN Info"), wxOK);

  //  Clear all selections
  if (m_scrollWinConfigList) {
    wxWindowList kids = m_scrollWinConfigList->GetChildren();
    for (unsigned int i = 0; i < kids.GetCount(); i++) {
      wxWindowListNode* node = kids.Item(i);
      wxWindow* win = node->GetData();
      wxPanel* pcp = wxDynamicCast(win, wxPanel);
      if (pcp) {
        ConfigPanel* cPanel = wxDynamicCast(pcp, ConfigPanel);
        if (cPanel) {
          cPanel->SetBackgroundColour(m_panelBackgroundUnselected);
        }
      }
    }
  }
  m_selectedConfigPanelGUID = wxEmptyString;

  m_returnChanges |= CONFIG_CHANGED;

  if (!currentLocale.IsSameAs(g_locale)) m_returnChanges |= LOCALE_CHANGED;

  Finish();
}

void options::OnConfigMouseSelected(wxMouseEvent& event) {
  wxPanel* selectedPanel = NULL;
  wxObject* obj = event.GetEventObject();
  if (obj) {
    wxPanel* panel = wxDynamicCast(obj, wxPanel);
    if (panel) {
      selectedPanel = panel;
    }
    // Clicked on child?
    else {
      wxWindow* win = wxDynamicCast(obj, wxWindow);
      if (win) {
        wxPanel* parentpanel = wxDynamicCast(win->GetParent(), wxPanel);
        if (parentpanel) {
          selectedPanel = parentpanel;
        }
      }
    }

    if (m_scrollWinConfigList) {
      wxWindowList kids = m_scrollWinConfigList->GetChildren();
      for (unsigned int i = 0; i < kids.GetCount(); i++) {
        wxWindowListNode* node = kids.Item(i);
        wxWindow* win = node->GetData();
        wxPanel* panel = wxDynamicCast(win, wxPanel);
        if (panel) {
          if (panel == selectedPanel) {
            wxColour colour;
            GetGlobalColor(_T("UIBCK"), &colour);
            panel->SetBackgroundColour(colour);
            ConfigPanel* cPanel = wxDynamicCast(panel, ConfigPanel);
            if (cPanel) m_selectedConfigPanelGUID = cPanel->GetConfigGUID();
          } else
            panel->SetBackgroundColour(m_panelBackgroundUnselected);

          panel->Refresh(true);
        }
      }
    }
    m_DisplayConfigsPage->Layout();
    SetConfigButtonState();
  }
}

void options::CreatePanel_Advanced(size_t parent, int border_size,
                                   int group_item_spacing) {
  m_ChartDisplayPage = AddPage(parent, _("Advanced"));

  if (m_bcompact) {
    wxSize sz = g_Platform->getDisplaySize();
    double dpmm = g_Platform->GetDisplayDPmm();

    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    m_ChartDisplayPage->SetSizer(wrapperSizer);

    wxBoxSizer* itemBoxSizerUI = wrapperSizer;

    // spacer
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    // Chart Display Options
    wxBoxSizer* boxCharts = new wxBoxSizer(wxVERTICAL);
    itemBoxSizerUI->Add(boxCharts, groupInputFlags);

    pSkewComp = new wxCheckBox(m_ChartDisplayPage, ID_SKEWCOMPBOX,
                               _("De-skew Raster Charts"));
    boxCharts->Add(pSkewComp, inputFlags);

    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    // OpenGL Options
    wxBoxSizer* OpenGLSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizerUI->Add(OpenGLSizer, 0, 0, 0);

    pOpenGL = new wxCheckBox(m_ChartDisplayPage, ID_OPENGLBOX,
                             _("Use Accelerated Graphics (OpenGL)"));
    OpenGLSizer->Add(pOpenGL, inputFlags);
    pOpenGL->Enable(!g_bdisable_opengl);

    wxButton* bOpenGL = new wxButton(m_ChartDisplayPage, ID_OPENGLOPTIONS,
                                     _("OpenGL Options") + _T("..."));
    OpenGLSizer->Add(bOpenGL, inputFlags);
    bOpenGL->Enable(!g_bdisable_opengl);

#ifdef __OCPN__ANDROID__
    pOpenGL->Hide();
    bOpenGL->Hide();
#endif

    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    //  Course Up display update period
    wxStaticText* crat = new wxStaticText(m_ChartDisplayPage, wxID_ANY,
                                          _("Chart Rotation Averaging Time"));
    crat->Wrap(-1);
    wrapperSizer->Add(crat, 0,
                      wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL,
                      group_item_spacing);

    wxBoxSizer* pCOGUPFilterRow = new wxBoxSizer(wxHORIZONTAL);
    wrapperSizer->Add(pCOGUPFilterRow, 0, wxALL | wxEXPAND, group_item_spacing);

    pCOGUPUpdateSecs =
        new wxTextCtrl(m_ChartDisplayPage, ID_OPTEXTCTRL, _T(""),
                       wxDefaultPosition, wxSize(sz.x / 5, -1), wxTE_RIGHT);
    pCOGUPFilterRow->Add(pCOGUPUpdateSecs, 0, wxALIGN_RIGHT | wxALL,
                         group_item_spacing);

    pCOGUPFilterRow->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("seconds")),
        inputFlags);

    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    // Chart Zoom Scale Weighting
    wxStaticText* zoomTextHead = new wxStaticText(
        m_ChartDisplayPage, wxID_ANY, _("Chart Zoom/Scale Weighting"));
    zoomTextHead->Wrap(-1);
    itemBoxSizerUI->Add(
        zoomTextHead, 0,
        wxALL | wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL,
        group_item_spacing);
    itemBoxSizerUI->Add(0, border_size * 1);

    itemBoxSizerUI->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("Raster")),
        inputFlags);

    m_pSlider_Zoom =
        new wxSlider(m_ChartDisplayPage, ID_RASTERZOOM, 0, -5, 5,
                     wxDefaultPosition, m_sliderSize, SLIDER_STYLE);

#ifdef __OCPN__ANDROID__
    prepareSlider(m_pSlider_Zoom);
#endif

    itemBoxSizerUI->Add(m_pSlider_Zoom, inputFlags);

    itemBoxSizerUI->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("Vector")),
        inputFlags);

    m_pSlider_Zoom_Vector =
        new wxSlider(m_ChartDisplayPage, ID_VECZOOM, 0, -5, 5,
                     wxDefaultPosition, m_sliderSize, SLIDER_STYLE);

#ifdef __OCPN__ANDROID__
    prepareSlider(m_pSlider_Zoom_Vector);
#endif

    itemBoxSizerUI->Add(m_pSlider_Zoom_Vector, inputFlags);

    itemBoxSizerUI->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("CM93 Detail level")),
        inputFlags);
    m_pSlider_CM93_Zoom =
        new wxSlider(m_ChartDisplayPage, ID_CM93ZOOM, 0,
                     -CM93_ZOOM_FACTOR_MAX_RANGE, CM93_ZOOM_FACTOR_MAX_RANGE,
                     wxDefaultPosition, m_sliderSize, SLIDER_STYLE);

#ifdef __OCPN__ANDROID__
    prepareSlider(m_pSlider_CM93_Zoom);
#endif

    itemBoxSizerUI->Add(m_pSlider_CM93_Zoom, 0, wxALL, border_size);

    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    //  Display size/DPI
    itemBoxSizerUI->Add(new wxStaticText(m_ChartDisplayPage, wxID_ANY,
                                         _("Physical Screen Width")),
                        inputFlags);
    wxBoxSizer* pDPIRow = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerUI->Add(pDPIRow, 0, wxEXPAND);

    pRBSizeAuto = new wxRadioButton(m_ChartDisplayPage, wxID_ANY, _("Auto"));
    pDPIRow->Add(pRBSizeAuto, inputFlags);
    pDPIRow->AddSpacer(10);
    pRBSizeManual = new wxRadioButton(m_ChartDisplayPage,
                                      ID_SIZEMANUALRADIOBUTTON, _("Manual:"));
    pDPIRow->Add(pRBSizeManual, inputFlags);

    wxBoxSizer* pmmRow = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerUI->Add(pmmRow, 0, wxEXPAND);

    pScreenMM =
        new wxTextCtrl(m_ChartDisplayPage, ID_OPTEXTCTRL, _T(""),
                       wxDefaultPosition, wxSize(sz.x / 5, -1), wxTE_RIGHT);
    pmmRow->Add(pScreenMM, 0, wxALIGN_RIGHT | wxALL, group_item_spacing);

    pmmRow->Add(new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("mm")),
                inputFlags);

    pRBSizeAuto->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                         wxCommandEventHandler(options::OnSizeAutoButton), NULL,
                         this);
    pRBSizeManual->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                           wxCommandEventHandler(options::OnSizeManualButton),
                           NULL, this);

  }

  else {
    wxFlexGridSizer* itemBoxSizerUI = new wxFlexGridSizer(2);
    itemBoxSizerUI->SetHGap(border_size);
    //    itemBoxSizerUI->AddGrowableCol( 0, 1 );
    //    itemBoxSizerUI->AddGrowableCol( 1, 1 );
    //    m_ChartDisplayPage->SetSizer( itemBoxSizerUI );

    // wxFlexGridSizer grows wrongly in wx2.8, so we need to centre it in
    // another sizer instead of letting it grow.
    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    m_ChartDisplayPage->SetSizer(wrapperSizer);
    wrapperSizer->Add(itemBoxSizerUI, 1, wxALL | wxALIGN_CENTER, border_size);

    // spacer
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    // Chart Display Options
    itemBoxSizerUI->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("Chart Display")),
        groupLabelFlags);
    wxBoxSizer* boxCharts = new wxBoxSizer(wxVERTICAL);
    itemBoxSizerUI->Add(boxCharts, groupInputFlags);

    pSkewComp = new wxCheckBox(m_ChartDisplayPage, ID_SKEWCOMPBOX,
                               _("Show Skewed Raster Charts as North-Up"));
    boxCharts->Add(pSkewComp, verticleInputFlags);

    //     pFullScreenQuilt = new wxCheckBox(m_ChartDisplayPage,
    //     ID_FULLSCREENQUILT,
    //                                       _("Disable Full Screen Quilting"));
    //     boxCharts->Add(pFullScreenQuilt, verticleInputFlags);

    //     pOverzoomEmphasis =
    //         new wxCheckBox(m_ChartDisplayPage, ID_FULLSCREENQUILT,
    //                        _("Suppress blur/fog effects on overzoom"));
    //     boxCharts->Add(pOverzoomEmphasis, verticleInputFlags);
    //
    //     pOZScaleVector =
    //         new wxCheckBox(m_ChartDisplayPage, ID_FULLSCREENQUILT,
    //                        _("Suppress scaled vector charts on overzoom"));
    //     boxCharts->Add(pOZScaleVector, verticleInputFlags);

    // spacer
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    //  Course Up display update period
    itemBoxSizerUI->Add(new wxStaticText(m_ChartDisplayPage, wxID_ANY,
                                         _("Chart Rotation Averaging Time")),
                        labelFlags);
    wxBoxSizer* pCOGUPFilterRow = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerUI->Add(pCOGUPFilterRow, 0, wxALL | wxEXPAND,
                        group_item_spacing);

    pCOGUPUpdateSecs =
        new wxTextCtrl(m_ChartDisplayPage, ID_OPTEXTCTRL, _T(""),
                       wxDefaultPosition, wxSize(50, -1), wxTE_RIGHT);
    pCOGUPFilterRow->Add(pCOGUPUpdateSecs, 0, wxALL, group_item_spacing);

    pCOGUPFilterRow->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("seconds")),
        inputFlags);

    // spacer
    itemBoxSizerUI->Add(0, border_size * 8);
    itemBoxSizerUI->Add(0, border_size * 8);

    // Chart Zoom Scale Weighting
    wxStaticText* zoomTextHead = new wxStaticText(
        m_ChartDisplayPage, wxID_ANY, _("Chart Zoom/Scale Weighting"));

    itemBoxSizerUI->Add(zoomTextHead, labelFlags);
    itemBoxSizerUI->Add(0, border_size * 1);
    itemBoxSizerUI->Add(0, border_size * 1);

    wxStaticText* zoomText = new wxStaticText(
        m_ChartDisplayPage, wxID_ANY,
        _("With a lower value, the same zoom level shows a less detailed chart.\n\
With a higher value, the same zoom level shows a more detailed chart."));

    smallFont = *dialogFont;  // we can't use Smaller() because
                              // wx2.8 doesn't support it
    smallFont.SetPointSize((smallFont.GetPointSize() / 1.2) +
                           0.5);  // + 0.5 to round instead of truncate
    zoomText->SetFont(smallFont);
    itemBoxSizerUI->Add(zoomText, 0, wxALL | wxEXPAND, group_item_spacing);

    // spacer
    /*itemBoxSizerUI->Add(0, border_size * 8); itemBoxSizerUI->Add(0,
     * border_size * 8);*/

    // wxSize sz = g_Platform->getDisplaySize();

    itemBoxSizerUI->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("Raster")),
        labelFlags);
    m_pSlider_Zoom =
        new wxSlider(m_ChartDisplayPage, ID_RASTERZOOM, 0, -5, 5,
                     wxDefaultPosition, m_sliderSize, SLIDER_STYLE);

#ifdef __OCPN__ANDROID__
    prepareSlider(m_pSlider_Zoom);
#endif

    itemBoxSizerUI->Add(m_pSlider_Zoom, inputFlags);

    itemBoxSizerUI->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("Vector")),
        labelFlags);
    m_pSlider_Zoom_Vector =
        new wxSlider(m_ChartDisplayPage, ID_VECZOOM, 0, -5, 5,
                     wxDefaultPosition, m_sliderSize, SLIDER_STYLE);

#ifdef __OCPN__ANDROID__
    prepareSlider(m_pSlider_Zoom_Vector);
#endif

    itemBoxSizerUI->Add(m_pSlider_Zoom_Vector, inputFlags);

    // Spacer
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    itemBoxSizerUI->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("CM93 Detail level")),
        labelFlags);
    m_pSlider_CM93_Zoom =
        new wxSlider(m_ChartDisplayPage, ID_CM93ZOOM, 0,
                     -CM93_ZOOM_FACTOR_MAX_RANGE, CM93_ZOOM_FACTOR_MAX_RANGE,
                     wxDefaultPosition, m_sliderSize, SLIDER_STYLE);

#ifdef __OCPN__ANDROID__
    prepareSlider(m_pSlider_CM93_Zoom);
#endif

    itemBoxSizerUI->Add(m_pSlider_CM93_Zoom, 0, wxALL, border_size);

    // spacer
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    //  Display size/DPI
    itemBoxSizerUI->Add(new wxStaticText(m_ChartDisplayPage, wxID_ANY,
                                         _("Physical Screen Width")),
                        labelFlags);
    wxBoxSizer* pDPIRow = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerUI->Add(pDPIRow, 0, wxEXPAND);

    pRBSizeAuto = new wxRadioButton(m_ChartDisplayPage, wxID_ANY, _("Auto"));
    pDPIRow->Add(pRBSizeAuto, inputFlags);
    pDPIRow->AddSpacer(10);
    pRBSizeManual = new wxRadioButton(m_ChartDisplayPage,
                                      ID_SIZEMANUALRADIOBUTTON, _("Manual:"));
    pDPIRow->Add(pRBSizeManual, inputFlags);

    pScreenMM = new wxTextCtrl(m_ChartDisplayPage, ID_OPTEXTCTRL, _T(""),
                               wxDefaultPosition, wxSize(3 * m_fontHeight, -1),
                               wxTE_RIGHT);
    pDPIRow->Add(pScreenMM, 0, wxALL, group_item_spacing);

    pDPIRow->Add(new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("mm")),
                 inputFlags);

    pRBSizeAuto->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                         wxCommandEventHandler(options::OnSizeAutoButton), NULL,
                         this);
    pRBSizeManual->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                           wxCommandEventHandler(options::OnSizeManualButton),
                           NULL, this);

    // spacer
    itemBoxSizerUI->Add(0, border_size * 3);
    itemBoxSizerUI->Add(0, border_size * 3);

    // OpenGL Options
    itemBoxSizerUI->Add(
        new wxStaticText(m_ChartDisplayPage, wxID_ANY, _("Graphics")),
        labelFlags);
    wxBoxSizer* OpenGLSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerUI->Add(OpenGLSizer, 0, 0, 0);

    pOpenGL = new wxCheckBox(m_ChartDisplayPage, ID_OPENGLBOX,
                             _("Use Accelerated Graphics (OpenGL)"));
    OpenGLSizer->Add(pOpenGL, inputFlags);
    pOpenGL->Enable(!g_bdisable_opengl);

#ifdef __OCPN__ANDROID__
    pOpenGL->Disable();
#endif

    wxButton* bOpenGL = new wxButton(m_ChartDisplayPage, ID_OPENGLOPTIONS,
                                     _("Options") + _T("..."));
    OpenGLSizer->Add(bOpenGL, inputFlags);
    bOpenGL->Enable(!g_bdisable_opengl);

    itemBoxSizerUI->Add(0, border_size * 3);
    /*
        pTransparentToolbar =
            new wxCheckBox(m_ChartDisplayPage, ID_TRANSTOOLBARCHECKBOX,
                           _("Enable Transparent Toolbar"));
        itemBoxSizerUI->Add(pTransparentToolbar, 0, wxALL, border_size);
        if (g_bopengl && !g_bTransparentToolbarInOpenGLOK)
          pTransparentToolbar->Disable();
    */
  }
}

void options::CreatePanel_VectorCharts(size_t parent, int border_size,
                                       int group_item_spacing) {
  ps57Ctl = AddPage(parent, _("Vector Chart Display"));

  if (!m_bcompact) {
    vectorPanel = new wxBoxSizer(wxHORIZONTAL);
    ps57Ctl->SetSizer(vectorPanel);

    // 1st column, all options except Mariner's Standard
    wxFlexGridSizer* optionsColumn = new wxFlexGridSizer(2);
    optionsColumn->SetHGap(border_size);
    optionsColumn->AddGrowableCol(0, 2);
    optionsColumn->AddGrowableCol(1, 3);
    vectorPanel->Add(optionsColumn, 3, wxALL | wxEXPAND, border_size);

    // spacer
    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _T("")));
    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _T("")));

    if (!g_useMUI) {
      // display category
      optionsColumn->Add(
          new wxStaticText(ps57Ctl, wxID_ANY, _("Display Category")),
          labelFlags);
      wxString pDispCatStrings[] = {_("Base"), _("Standard"), _("All"),
                                    _("Mariner's Standard")};
      pDispCat = new wxChoice(ps57Ctl, ID_RADARDISTUNIT, wxDefaultPosition,
                              wxDefaultSize, 4, pDispCatStrings);
      optionsColumn->Add(pDispCat, 0, wxALL, 2);
    }

    // spacer
    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _T("")));
    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _T("")));

    // display options
    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Display")),
                       groupLabelFlags);

    wxBoxSizer* miscSizer = new wxBoxSizer(wxVERTICAL);
    optionsColumn->Add(miscSizer, groupInputFlags);

    if (!g_useMUI) {
      pCheck_SOUNDG =
          new wxCheckBox(ps57Ctl, ID_SOUNDGCHECKBOX, _("Depth Soundings"));
      pCheck_SOUNDG->SetValue(FALSE);
      miscSizer->Add(pCheck_SOUNDG, verticleInputFlags);
    }

    pCheck_META = new wxCheckBox(ps57Ctl, ID_METACHECKBOX,
                                 _("Chart Information Objects"));
    pCheck_META->SetValue(FALSE);
    miscSizer->Add(pCheck_META, verticleInputFlags);

    if (!g_useMUI) {
      optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Buoys/Lights")),
                         groupLabelFlags);

      wxBoxSizer* lightSizer = new wxBoxSizer(wxVERTICAL);
      optionsColumn->Add(lightSizer, groupInputFlags);

      pCheck_ATONTEXT =
          new wxCheckBox(ps57Ctl, ID_ATONTEXTCHECKBOX, _("Buoy/Light Labels"));
      pCheck_ATONTEXT->SetValue(FALSE);
      lightSizer->Add(pCheck_ATONTEXT, verticleInputFlags);

      pCheck_LDISTEXT =
          new wxCheckBox(ps57Ctl, ID_LDISTEXTCHECKBOX, _("Light Descriptions"));
      pCheck_LDISTEXT->SetValue(FALSE);
      lightSizer->Add(pCheck_LDISTEXT, verticleInputFlags);

      pCheck_XLSECTTEXT = new wxCheckBox(ps57Ctl, ID_LDISTEXTCHECKBOX,
                                         _("Extended Light Sectors"));
      pCheck_XLSECTTEXT->SetValue(FALSE);
      lightSizer->Add(pCheck_XLSECTTEXT, verticleInputFlags);
    }

    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Chart Texts")),
                       groupLabelFlags);

    wxBoxSizer* textSizer = new wxBoxSizer(wxVERTICAL);
    optionsColumn->Add(textSizer, groupInputFlags);

    pCheck_NATIONALTEXT = new wxCheckBox(ps57Ctl, ID_NATIONALTEXTCHECKBOX,
                                         _("National text on chart"));
    pCheck_NATIONALTEXT->SetValue(FALSE);
    textSizer->Add(pCheck_NATIONALTEXT, verticleInputFlags);

    pCheck_SHOWIMPTEXT =
        new wxCheckBox(ps57Ctl, ID_IMPTEXTCHECKBOX, _("Important Text Only"));
    pCheck_SHOWIMPTEXT->SetValue(FALSE);
    textSizer->Add(pCheck_SHOWIMPTEXT, verticleInputFlags);

    pCheck_DECLTEXT =
        new wxCheckBox(ps57Ctl, ID_DECLTEXTCHECKBOX, _("De-Cluttered Text"));
    pCheck_DECLTEXT->SetValue(FALSE);
    textSizer->Add(pCheck_DECLTEXT, verticleInputFlags);

    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Chart Detail")),
                       labelFlags);
    pCheck_SCAMIN = new wxCheckBox(ps57Ctl, ID_SCAMINCHECKBOX,
                                   _("Reduced Detail at Small Scale"));
    pCheck_SCAMIN->SetValue(FALSE);
    optionsColumn->Add(pCheck_SCAMIN, inputFlags);

    // spacer
    optionsColumn->Add(0, border_size * 4);
    optionsColumn->Add(0, border_size * 4);

    // graphics options
    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Graphics Style")),
                       labelFlags);
    wxString pPointStyleStrings[] = {
        _("Paper Chart"),
        _("Simplified"),
    };
    pPointStyle = new wxChoice(ps57Ctl, ID_RADARDISTUNIT, wxDefaultPosition,
                               wxDefaultSize, 2, pPointStyleStrings);
    optionsColumn->Add(pPointStyle, inputFlags);

    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Boundaries")),
                       labelFlags);
    wxString pBoundStyleStrings[] = {
        _("Plain"),
        _("Symbolized"),
    };
    pBoundStyle = new wxChoice(ps57Ctl, ID_RADARDISTUNIT, wxDefaultPosition,
                               wxDefaultSize, 2, pBoundStyleStrings);
    optionsColumn->Add(pBoundStyle, inputFlags);

    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Colors")),
                       labelFlags);
    wxString pColorNumStrings[] = {
        _("2 Color"),
        _("4 Color"),
    };
    p24Color = new wxChoice(ps57Ctl, ID_RADARDISTUNIT, wxDefaultPosition,
                            wxDefaultSize, 2, pColorNumStrings);
    optionsColumn->Add(p24Color, inputFlags);

    // spacer
    optionsColumn->Add(0, border_size * 4);
    optionsColumn->Add(0, border_size * 4);

    // depth options
    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Shallow Depth")),
                       labelFlags);
    wxBoxSizer* depShalRow = new wxBoxSizer(wxHORIZONTAL);
    optionsColumn->Add(depShalRow);
    m_ShallowCtl =
        new wxTextCtrl(ps57Ctl, ID_OPTEXTCTRL, _T(""), wxDefaultPosition,
                       wxSize(60, -1), wxTE_RIGHT);
    depShalRow->Add(m_ShallowCtl, inputFlags);
    m_depthUnitsShal = new wxStaticText(ps57Ctl, wxID_ANY, _("meters"));
    depShalRow->Add(m_depthUnitsShal, inputFlags);

    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Safety Depth")),
                       labelFlags);
    wxBoxSizer* depSafeRow = new wxBoxSizer(wxHORIZONTAL);
    optionsColumn->Add(depSafeRow);
    m_SafetyCtl = new wxTextCtrl(ps57Ctl, ID_OPTEXTCTRL, _T(""),
                                 wxDefaultPosition, wxSize(60, -1), wxTE_RIGHT);
    depSafeRow->Add(m_SafetyCtl, inputFlags);
    m_depthUnitsSafe = new wxStaticText(ps57Ctl, wxID_ANY, _("meters"));
    depSafeRow->Add(m_depthUnitsSafe, inputFlags);

    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Deep Depth")),
                       labelFlags);
    wxBoxSizer* depDeepRow = new wxBoxSizer(wxHORIZONTAL);
    optionsColumn->Add(depDeepRow);
    m_DeepCtl = new wxTextCtrl(ps57Ctl, ID_OPTEXTCTRL, _T(""),
                               wxDefaultPosition, wxSize(60, -1), wxTE_RIGHT);
    depDeepRow->Add(m_DeepCtl, inputFlags);
    m_depthUnitsDeep = new wxStaticText(ps57Ctl, wxID_ANY, _("meters"));
    depDeepRow->Add(m_depthUnitsDeep, inputFlags);

    // 2nd column, Display Category / Mariner's Standard options
    wxBoxSizer* dispSizer = new wxBoxSizer(wxVERTICAL);
    vectorPanel->Add(dispSizer, 2, wxALL | wxEXPAND, border_size);

    wxStaticBox* marinersBox =
        new wxStaticBox(ps57Ctl, wxID_ANY, _("User Standard Objects"));
    wxStaticBoxSizer* marinersSizer =
        new wxStaticBoxSizer(marinersBox, wxVERTICAL);
    dispSizer->Add(marinersSizer, 1, wxALL | wxEXPAND, border_size);

    ps57CtlListBox = new OCPNCheckedListCtrl(
        ps57Ctl, ID_CHECKLISTBOX, wxDefaultPosition, wxSize(250, 350));
    marinersSizer->Add(ps57CtlListBox, 1, wxALL | wxEXPAND, group_item_spacing);

    wxBoxSizer* btnRow1 = new wxBoxSizer(wxHORIZONTAL);
    itemButtonSelectList =
        new wxButton(ps57Ctl, ID_SELECTLIST, _("Select All"));
    btnRow1->Add(itemButtonSelectList, 1, wxALL | wxEXPAND, group_item_spacing);
    itemButtonClearList = new wxButton(ps57Ctl, ID_CLEARLIST, _("Clear All"));
    btnRow1->Add(itemButtonClearList, 1, wxALL | wxEXPAND, group_item_spacing);
    marinersSizer->Add(btnRow1);

    wxBoxSizer* btnRow2 = new wxBoxSizer(wxHORIZONTAL);
    itemButtonSetStd =
        new wxButton(ps57Ctl, ID_SETSTDLIST, _("Reset to STANDARD"));
    btnRow2->Add(itemButtonSetStd, 1, wxALL | wxEXPAND, group_item_spacing);
    marinersSizer->Add(btnRow2);

  }

  else {  // compact
    vectorPanel = new wxBoxSizer(wxVERTICAL);
    ps57Ctl->SetSizer(vectorPanel);

    wxBoxSizer* optionsColumn = vectorPanel;

    // spacer
    optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _T("")));

    // display category
    if (!g_useMUI) {
      optionsColumn->Add(
          new wxStaticText(ps57Ctl, wxID_ANY, _("Display Category")),
          inputFlags);
      wxString pDispCatStrings[] = {_("Base"), _("Standard"), _("All"),
                                    _("Mariner's Standard")};
      pDispCat = new wxChoice(ps57Ctl, ID_RADARDISTUNIT, wxDefaultPosition,
                              wxSize(350, -1), 4, pDispCatStrings);
      optionsColumn->Add(pDispCat, 0, wxALL, 2);

      // spacer
      optionsColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _T("")));
    }

    // display options

    wxBoxSizer* miscSizer = new wxBoxSizer(wxVERTICAL);
    optionsColumn->Add(miscSizer, groupInputFlags);

    pCheck_SOUNDG =
        new wxCheckBox(ps57Ctl, ID_SOUNDGCHECKBOX, _("Depth Soundings"));
    pCheck_SOUNDG->SetValue(FALSE);
    miscSizer->Add(pCheck_SOUNDG, inputFlags);

    pCheck_META = new wxCheckBox(ps57Ctl, ID_METACHECKBOX,
                                 _("Chart Information Objects"));
    pCheck_META->SetValue(FALSE);
    miscSizer->Add(pCheck_META, inputFlags);

    wxBoxSizer* lightSizer = new wxBoxSizer(wxVERTICAL);
    optionsColumn->Add(lightSizer, groupInputFlags);

    pCheck_ATONTEXT =
        new wxCheckBox(ps57Ctl, ID_ATONTEXTCHECKBOX, _("Buoy/Light Labels"));
    pCheck_ATONTEXT->SetValue(FALSE);
    lightSizer->Add(pCheck_ATONTEXT, inputFlags);

    pCheck_LDISTEXT =
        new wxCheckBox(ps57Ctl, ID_LDISTEXTCHECKBOX, _("Light Descriptions"));
    pCheck_LDISTEXT->SetValue(FALSE);
    lightSizer->Add(pCheck_LDISTEXT, inputFlags);

    pCheck_XLSECTTEXT = new wxCheckBox(ps57Ctl, ID_LDISTEXTCHECKBOX,
                                       _("Extended Light Sectors"));
    pCheck_XLSECTTEXT->SetValue(FALSE);
    lightSizer->Add(pCheck_XLSECTTEXT, inputFlags);

    wxBoxSizer* textSizer = new wxBoxSizer(wxVERTICAL);
    optionsColumn->Add(textSizer, groupInputFlags);

    pCheck_NATIONALTEXT = new wxCheckBox(ps57Ctl, ID_NATIONALTEXTCHECKBOX,
                                         _("National text on chart"));
    pCheck_NATIONALTEXT->SetValue(FALSE);
    textSizer->Add(pCheck_NATIONALTEXT, inputFlags);

    pCheck_SHOWIMPTEXT =
        new wxCheckBox(ps57Ctl, ID_IMPTEXTCHECKBOX, _("Important Text Only"));
    pCheck_SHOWIMPTEXT->SetValue(FALSE);
    textSizer->Add(pCheck_SHOWIMPTEXT, inputFlags);

    pCheck_DECLTEXT =
        new wxCheckBox(ps57Ctl, ID_DECLTEXTCHECKBOX, _("De-Cluttered Text"));
    pCheck_DECLTEXT->SetValue(FALSE);
    textSizer->Add(pCheck_DECLTEXT, inputFlags);

    pCheck_SCAMIN = new wxCheckBox(ps57Ctl, ID_SCAMINCHECKBOX,
                                   _("Reduced Detail at Small Scale"));
    pCheck_SCAMIN->SetValue(FALSE);
    optionsColumn->Add(pCheck_SCAMIN, inputFlags);

    // spacer
    optionsColumn->Add(0, border_size * 4);
    optionsColumn->Add(0, border_size * 4);

    // graphics options

    wxFlexGridSizer* StyleColumn = new wxFlexGridSizer(2);
    StyleColumn->SetHGap(border_size);
    StyleColumn->AddGrowableCol(0, 2);
    StyleColumn->AddGrowableCol(1, 3);
    optionsColumn->Add(StyleColumn);

    StyleColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Graphics Style")),
                     inputFlags);
    wxString pPointStyleStrings[] = {
        _("Paper Chart"),
        _("Simplified"),
    };
    pPointStyle = new wxChoice(ps57Ctl, ID_RADARDISTUNIT, wxDefaultPosition,
                               wxSize(m_fontHeight * 3, m_fontHeight), 2,
                               pPointStyleStrings);
#ifdef __OCPN__ANDROID__
    setChoiceStyleSheet(pPointStyle, m_fontHeight * 8 / 10);
#endif

    StyleColumn->Add(pPointStyle, inputFlags);

    StyleColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Boundaries")),
                     inputFlags);
    wxString pBoundStyleStrings[] = {
        _("Plain"),
        _("Symbolized"),
    };
    pBoundStyle = new wxChoice(ps57Ctl, ID_RADARDISTUNIT, wxDefaultPosition,
                               wxSize(m_fontHeight * 3, m_fontHeight), 2,
                               pBoundStyleStrings);
#ifdef __OCPN__ANDROID__
    setChoiceStyleSheet(pBoundStyle, m_fontHeight * 8 / 10);
#endif

    StyleColumn->Add(pBoundStyle, inputFlags);

    StyleColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Colors")),
                     inputFlags);

    wxString pColorNumStrings[] = {
        _("2 Color"),
        _("4 Color"),
    };
    p24Color = new wxChoice(ps57Ctl, ID_RADARDISTUNIT, wxDefaultPosition,
                            wxSize(m_fontHeight * 3, m_fontHeight), 2,
                            pColorNumStrings);
#ifdef __OCPN__ANDROID__
    setChoiceStyleSheet(p24Color, m_fontHeight * 8 / 10);
#endif
    StyleColumn->Add(p24Color, inputFlags);

    // spacer
    optionsColumn->Add(0, border_size * 4);
    optionsColumn->Add(0, border_size * 4);

    wxFlexGridSizer* DepthColumn = new wxFlexGridSizer(3);
    DepthColumn->SetHGap(border_size);
    DepthColumn->AddGrowableCol(0, 3);
    DepthColumn->AddGrowableCol(1, 2);
    DepthColumn->AddGrowableCol(2, 3);
    optionsColumn->Add(DepthColumn);

    // depth options
    DepthColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Shallow Depth")),
                     inputFlags);
    m_ShallowCtl =
        new wxTextCtrl(ps57Ctl, ID_OPTEXTCTRL, _T(""), wxDefaultPosition,
                       wxSize(m_fontHeight * 2, m_fontHeight), wxTE_RIGHT);
    DepthColumn->Add(m_ShallowCtl, inputFlags);
    m_depthUnitsShal = new wxStaticText(ps57Ctl, wxID_ANY, _("meters"));
    DepthColumn->Add(m_depthUnitsShal, inputFlags);

    DepthColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Safety Depth")),
                     inputFlags);
    m_SafetyCtl =
        new wxTextCtrl(ps57Ctl, ID_OPTEXTCTRL, _T(""), wxDefaultPosition,
                       wxSize(m_fontHeight * 2, m_fontHeight), wxTE_RIGHT);
    DepthColumn->Add(m_SafetyCtl, inputFlags);
    m_depthUnitsSafe = new wxStaticText(ps57Ctl, wxID_ANY, _("meters"));
    DepthColumn->Add(m_depthUnitsSafe, inputFlags);

    DepthColumn->Add(new wxStaticText(ps57Ctl, wxID_ANY, _("Deep Depth")),
                     inputFlags);
    m_DeepCtl =
        new wxTextCtrl(ps57Ctl, ID_OPTEXTCTRL, _T(""), wxDefaultPosition,
                       wxSize(m_fontHeight * 2, m_fontHeight), wxTE_CENTER);
    DepthColumn->Add(m_DeepCtl, inputFlags);
    m_depthUnitsDeep = new wxStaticText(ps57Ctl, wxID_ANY, _("meters"));
    DepthColumn->Add(m_depthUnitsDeep, inputFlags);

    // spacer
    optionsColumn->Add(0, border_size * 4);
    optionsColumn->Add(0, border_size * 4);

    //  Display Category / Mariner's Standard options
    wxBoxSizer* dispSizer = new wxBoxSizer(wxVERTICAL);
    vectorPanel->Add(dispSizer, 2, wxALL | wxEXPAND, border_size);

    wxStaticBox* marinersBox =
        new wxStaticBox(ps57Ctl, wxID_ANY, _("User Standard Objects"));
    wxStaticBoxSizer* marinersSizer =
        new wxStaticBoxSizer(marinersBox, wxVERTICAL);
    dispSizer->Add(marinersSizer, 1, wxALL | wxEXPAND, border_size);

    wxBoxSizer* btnRow1 = new wxBoxSizer(wxHORIZONTAL);
    itemButtonSelectList =
        new wxButton(ps57Ctl, ID_SELECTLIST, _("Select All"));
    btnRow1->Add(itemButtonSelectList, 1, wxALL | wxEXPAND, group_item_spacing);
    itemButtonClearList = new wxButton(ps57Ctl, ID_CLEARLIST, _("Clear All"));
    btnRow1->Add(itemButtonClearList, 1, wxALL | wxEXPAND, group_item_spacing);
    marinersSizer->Add(btnRow1);

    wxBoxSizer* btnRow2 = new wxBoxSizer(wxHORIZONTAL);
    itemButtonSetStd =
        new wxButton(ps57Ctl, ID_SETSTDLIST, _("Reset to STANDARD"));
    btnRow2->Add(itemButtonSetStd, 1, wxALL | wxEXPAND, group_item_spacing);
    marinersSizer->Add(btnRow2);

    // #if defined(__WXMSW__) || defined(__WXOSX__)
    //     wxString* ps57CtlListBoxStrings = NULL;
    //     ps57CtlListBox = new wxCheckListBox(
    //         ps57Ctl, ID_CHECKLISTBOX, wxDefaultPosition, wxSize(250, 350), 0,
    //         ps57CtlListBoxStrings, wxLB_SINGLE | wxLB_HSCROLL | wxLB_SORT);
    // #else
    ps57CtlListBox = new OCPNCheckedListCtrl(
        ps57Ctl, ID_CHECKLISTBOX, wxDefaultPosition, wxSize(250, 350));
    // #endif

    marinersSizer->Add(ps57CtlListBox, 1, wxALL | wxEXPAND, group_item_spacing);
  }
}

void options::CreatePanel_TidesCurrents(size_t parent, int border_size,
                                        int group_item_spacing) {
  wxScrolledWindow* tcPanel = AddPage(parent, _("Tides && Currents"));

  wxBoxSizer* mainHBoxSizer = new wxBoxSizer(wxVERTICAL);
  tcPanel->SetSizer(mainHBoxSizer);

  wxStaticBox* tcBox = new wxStaticBox(tcPanel, wxID_ANY, _("Active Datasets"));
  wxStaticBoxSizer* tcSizer = new wxStaticBoxSizer(tcBox, wxHORIZONTAL);
  mainHBoxSizer->Add(tcSizer, 1, wxALL | wxEXPAND, border_size);

  tcDataSelected =
      new wxListCtrl(tcPanel, ID_TIDESELECTED, wxDefaultPosition,
                     wxSize(100, -1), wxLC_REPORT | wxLC_NO_HEADER);

  tcSizer->Add(tcDataSelected, 1, wxALL | wxEXPAND, border_size);

  //  Populate Selection List Control with the contents
  //  of the Global static array
  tcDataSelected->DeleteAllItems();

  // Add first column
  wxListItem col0;
  col0.SetId(0);
  col0.SetText(_T(""));
  col0.SetWidth(500);
  col0.SetAlign(wxLIST_FORMAT_LEFT);

  tcDataSelected->InsertColumn(0, col0);

  for (unsigned int id = 0; id < TideCurrentDataSet.Count(); id++) {
    wxListItem li;
    li.SetId(id);
    long idx = tcDataSelected->InsertItem(li);

    wxString setName = TideCurrentDataSet[id];
    tcDataSelected->SetItem(id, 0, setName);
  }

  //    Add the "Insert/Remove" buttons
  wxButton* insertButton =
      new wxButton(tcPanel, ID_TCDATAADD, _("Add Dataset..."));
  wxButton* removeButton =
      new wxButton(tcPanel, ID_TCDATADEL, _("Remove Selected"));

  wxBoxSizer* btnSizer = new wxBoxSizer(wxVERTICAL);
  tcSizer->Add(btnSizer);

  btnSizer->Add(insertButton, 1, wxALL | wxEXPAND, group_item_spacing);
  btnSizer->Add(removeButton, 1, wxALL | wxEXPAND, group_item_spacing);
}

void options::CreatePanel_ChartGroups(size_t parent, int border_size,
                                      int group_item_spacing) {
  // Special case for adding the tab here. We know this page has multiple tabs,
  // and we have the actual widgets in a separate class (because of its
  // complexity)

  wxWindow* chartsPage = m_pListbook->GetPage(parent);
  groupsPanel = new ChartGroupsUI(chartsPage);

  groupsPanel->m_panel = AddPage(parent, _("Chart Groups"));

  groupsPanel->CreatePanel(parent, border_size, group_item_spacing);
}

void ChartGroupsUI::CreatePanel(size_t parent, int border_size,
                                int group_item_spacing) {
  modified = FALSE;
  m_border_size = border_size;
  m_group_item_spacing = group_item_spacing;

  m_UIcomplete = FALSE;

  CompletePanel();
}

void ChartGroupsUI::CompletePanel(void) {
  m_topSizer = new wxBoxSizer(wxVERTICAL);
  m_panel->SetSizer(m_topSizer);

  //    The chart file/dir tree
  wxStaticText* allChartsLabel =
      new wxStaticText(m_panel, wxID_ANY, _("All Available Charts"));
  m_topSizer->Add(allChartsLabel, 0, wxTOP | wxRIGHT | wxLEFT, m_border_size);

  wxBoxSizer* sizerCharts = new wxBoxSizer(wxHORIZONTAL);
  m_topSizer->Add(sizerCharts, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* activeListSizer = new wxBoxSizer(wxVERTICAL);
  sizerCharts->Add(activeListSizer, 1, wxALL | wxEXPAND, 5);

#ifdef __OCPN__ANDROID__
  allAvailableCtl =
      new wxGenericDirCtrl(m_panel, ID_GROUPAVAILABLE, _T(""),
                           wxDefaultPosition, wxDefaultSize, wxVSCROLL);
#else
  allAvailableCtl =
      new wxGenericDirCtrl(m_panel, ID_GROUPAVAILABLE, _T(""),
                           wxDefaultPosition, wxDefaultSize, wxVSCROLL);
#endif
  activeListSizer->Add(allAvailableCtl, 1, wxEXPAND);

  m_pAddButton = new wxButton(m_panel, ID_GROUPINSERTDIR, _("Add"));
  m_pAddButton->Disable();
  m_pRemoveButton = new wxButton(m_panel, ID_GROUPREMOVEDIR, _("Remove Chart"));
  m_pRemoveButton->Disable();

  m_pAddButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(ChartGroupsUI::OnInsertChartItem),
                        NULL, this);
  m_pRemoveButton->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartGroupsUI::OnRemoveChartItem), NULL, this);

  wxBoxSizer* addRemove = new wxBoxSizer(wxVERTICAL);
  sizerCharts->Add(addRemove, 0, wxALL | wxEXPAND, m_border_size);
  addRemove->Add(m_pAddButton, 0, wxALL | wxEXPAND, m_group_item_spacing);

  sizerCharts->AddSpacer(20);  // Avoid potential scrollbar

  //    Add the Groups notebook control
  wxStaticText* groupsLabel =
      new wxStaticText(m_panel, wxID_ANY, _("Chart Groups"));
  m_topSizer->Add(groupsLabel, 0, wxTOP | wxRIGHT | wxLEFT, m_border_size);

  wxBoxSizer* sizerGroups = new wxBoxSizer(wxHORIZONTAL);
  m_topSizer->Add(sizerGroups, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* nbSizer = new wxBoxSizer(wxVERTICAL);
  sizerGroups->Add(nbSizer, 1, wxALL | wxEXPAND, m_border_size);
  m_GroupNB = new wxNotebook(m_panel, ID_GROUPNOTEBOOK, wxDefaultPosition,
                             wxDefaultSize, wxNB_TOP);
  nbSizer->Add(m_GroupNB, 1, wxEXPAND);

  m_GroupNB->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                     wxNotebookEventHandler(ChartGroupsUI::OnGroupPageChange),
                     NULL, this);

  //    Add default (always present) Default Chart Group
  wxPanel* allActiveGroup =
      new wxPanel(m_GroupNB, -1, wxDefaultPosition, wxDefaultSize);
  m_GroupNB->AddPage(allActiveGroup, _("All Charts"));

  wxBoxSizer* page0BoxSizer = new wxBoxSizer(wxHORIZONTAL);
  allActiveGroup->SetSizer(page0BoxSizer);

  defaultAllCtl = new wxGenericDirCtrl(allActiveGroup, -1, _T(""),
                                       wxDefaultPosition, wxDefaultSize);

  //    Set the Font for the All Active Chart Group tree to be italic, dimmed
  iFont = new wxFont(*dialogFont);
  iFont->SetStyle(wxFONTSTYLE_ITALIC);
  iFont->SetWeight(wxFONTWEIGHT_LIGHT);

  page0BoxSizer->Add(defaultAllCtl, 1, wxALIGN_TOP | wxALL | wxEXPAND);

  m_DirCtrlArray.Add(defaultAllCtl);

  //    Add the Chart Group (page) "New" and "Delete" buttons
  m_pNewGroupButton =
      new wxButton(m_panel, ID_GROUPNEWGROUP, _("New Group..."));
  m_pDeleteGroupButton =
      new wxButton(m_panel, ID_GROUPDELETEGROUP, _("Delete Group"));
  m_pDeleteGroupButton->Disable();  // for default "all Charts" group

  m_pNewGroupButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                             wxCommandEventHandler(ChartGroupsUI::OnNewGroup),
                             NULL, this);
  m_pDeleteGroupButton->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartGroupsUI::OnDeleteGroup), NULL, this);

  wxBoxSizer* newDeleteGrp = new wxBoxSizer(wxVERTICAL);
  sizerGroups->Add(newDeleteGrp, 0, wxALL, m_border_size);

  newDeleteGrp->AddSpacer(25);
  newDeleteGrp->Add(m_pNewGroupButton, 0, wxALL | wxEXPAND,
                    m_group_item_spacing);
  newDeleteGrp->AddSpacer(15);
  newDeleteGrp->Add(m_pDeleteGroupButton, 0, wxALL | wxEXPAND,
                    m_group_item_spacing);
  newDeleteGrp->AddSpacer(25);
  newDeleteGrp->Add(m_pRemoveButton, 0, wxALL | wxEXPAND, m_group_item_spacing);

  sizerGroups->AddSpacer(20);  // Avoid potential scrollbar

  // Connect this last, otherwise handler is called before all objects are
  // initialized.
  m_panel->Connect(wxEVT_COMMAND_TREE_SEL_CHANGED,
                   wxTreeEventHandler(ChartGroupsUI::OnAvailableSelection),
                   NULL, this);

  m_UIcomplete = TRUE;
}

void options::CreatePanel_Display(size_t parent, int border_size,
                                  int group_item_spacing) {
  pDisplayPanel = AddPage(parent, _("General"));

  if (!m_bcompact) {
    wxFlexGridSizer* generalSizer = new wxFlexGridSizer(2);
    generalSizer->SetHGap(border_size);
    //    generalSizer->AddGrowableCol( 0, 1 );
    //    generalSizer->AddGrowableCol( 1, 1 );
    //    pDisplayPanel->SetSizer( generalSizer );

    // wxFlexGridSizer grows wrongly in wx2.8, so we need to centre it in
    // another sizer instead of letting it grow.
    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    pDisplayPanel->SetSizer(wrapperSizer);
    wrapperSizer->Add(generalSizer, 1, wxALL | wxALIGN_CENTER, border_size);

    // spacer
    generalSizer->Add(0, border_size * 4);
    generalSizer->Add(0, border_size * 4);

    if (!g_useMUI) {
      // Nav Mode
      generalSizer->Add(
          new wxStaticText(pDisplayPanel, wxID_ANY, _("Navigation Mode")),
          groupLabelFlags);
      wxBoxSizer* boxNavMode = new wxBoxSizer(wxVERTICAL);
      generalSizer->Add(boxNavMode, groupInputFlags);

      wxBoxSizer* rowOrientation = new wxBoxSizer(wxHORIZONTAL);
      boxNavMode->Add(rowOrientation);

      pCBNorthUp = new wxRadioButton(pDisplayPanel, wxID_ANY, _("North Up"));
      rowOrientation->Add(pCBNorthUp, inputFlags);
      pCBCourseUp =
          new wxRadioButton(pDisplayPanel, ID_COURSEUPCHECKBOX, _("Course Up"));
      rowOrientation->Add(pCBCourseUp,
                          wxSizerFlags(0)
                              .Align(wxALIGN_CENTRE_VERTICAL)
                              .Border(wxLEFT, group_item_spacing * 2));

      pCBLookAhead = new wxCheckBox(pDisplayPanel, ID_CHECK_LOOKAHEAD,
                                    _("Look Ahead Mode"));
      boxNavMode->Add(pCBLookAhead, verticleInputFlags);

      // spacer
      generalSizer->Add(0, border_size * 4);
      generalSizer->Add(0, border_size * 4);

      // Control Options
      generalSizer->Add(
          new wxStaticText(pDisplayPanel, wxID_ANY, _("Chart Display")),
          groupLabelFlags);
      wxBoxSizer* boxCharts = new wxBoxSizer(wxVERTICAL);
      generalSizer->Add(boxCharts, groupInputFlags);

      pCDOQuilting = new wxCheckBox(pDisplayPanel, ID_QUILTCHECKBOX1,
                                    _("Enable Chart Quilting"));
      boxCharts->Add(pCDOQuilting, verticleInputFlags);

      pPreserveScale =
          new wxCheckBox(pDisplayPanel, ID_PRESERVECHECKBOX,
                         _("Preserve scale when switching charts"));
      boxCharts->Add(pPreserveScale, verticleInputFlags);

      // spacer
      generalSizer->Add(0, border_size * 4);
      generalSizer->Add(0, border_size * 4);
    }

    // Control Options
    generalSizer->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("Controls")),
                      groupLabelFlags);
    wxBoxSizer* boxCtrls = new wxBoxSizer(wxVERTICAL);
    generalSizer->Add(boxCtrls, groupInputFlags);

    pSmoothPanZoom = new wxCheckBox(pDisplayPanel, ID_SMOOTHPANZOOMBOX,
                                    _("Smooth Panning / Zooming"));
    boxCtrls->Add(pSmoothPanZoom, verticleInputFlags);

    pEnableZoomToCursor =
        new wxCheckBox(pDisplayPanel, ID_ZTCCHECKBOX, _("Zoom to Cursor"));
    pEnableZoomToCursor->SetValue(FALSE);
    boxCtrls->Add(pEnableZoomToCursor, verticleInputFlags);

    // spacer
    generalSizer->Add(0, border_size * 4);
    generalSizer->Add(0, border_size * 4);

    if (!g_useMUI) {
      // Display Options
      generalSizer->Add(
          new wxStaticText(pDisplayPanel, wxID_ANY, _("Display Features")),
          groupLabelFlags);
      wxBoxSizer* boxDisp = new wxBoxSizer(wxVERTICAL);
      generalSizer->Add(boxDisp, groupInputFlags);

      pSDisplayGrid =
          new wxCheckBox(pDisplayPanel, ID_CHECK_DISPLAYGRID, _("Show Grid"));
      boxDisp->Add(pSDisplayGrid, verticleInputFlags);

      pCDOOutlines = new wxCheckBox(pDisplayPanel, ID_OUTLINECHECKBOX1,
                                    _("Show Chart Outlines"));
      boxDisp->Add(pCDOOutlines, verticleInputFlags);

      pSDepthUnits = new wxCheckBox(pDisplayPanel, ID_SHOWDEPTHUNITSBOX1,
                                    _("Show Depth Units"));
      boxDisp->Add(pSDepthUnits, verticleInputFlags);
    }

    // CUSTOMIZATION - LIVE ETA OPTION
    // -------------------------------
    // Add a checkbox to activate live ETA option in status bar, and
    // Add a text field to set default boat speed (for calculation when
    // no GPS or when the boat is at the harbor).

    // Spacer
    generalSizer->Add(0, border_size * 4);
    generalSizer->Add(0, border_size * 4);

    // New menu status bar
    generalSizer->Add(
        new wxStaticText(pDisplayPanel, wxID_ANY, _("Status Bar Option")),
        groupLabelFlags);
    wxBoxSizer* boxDispStatusBar = new wxBoxSizer(wxVERTICAL);
    generalSizer->Add(boxDispStatusBar, groupInputFlags);

    // Add option for live ETA
    pSLiveETA = new wxCheckBox(pDisplayPanel, ID_CHECK_LIVEETA,
                               _("Live ETA at Cursor"));
    boxDispStatusBar->Add(pSLiveETA, verticleInputFlags);

    // Add text input for default boat speed
    // (for calculation, in case GPS speed is null)
    wxBoxSizer* defaultBoatSpeedSizer = new wxBoxSizer(wxHORIZONTAL);
    boxDispStatusBar->Add(defaultBoatSpeedSizer, wxALL, group_item_spacing);
    defaultBoatSpeedSizer->Add(
        new wxStaticText(pDisplayPanel, wxID_ANY, _("Default Boat Speed ")),
        groupLabelFlagsHoriz);
    pSDefaultBoatSpeed =
        new wxTextCtrl(pDisplayPanel, ID_DEFAULT_BOAT_SPEED, _T(""),
                       wxDefaultPosition, wxSize(50, -1), wxTE_RIGHT);
    defaultBoatSpeedSizer->Add(pSDefaultBoatSpeed, 0, wxALIGN_CENTER_VERTICAL,
                               group_item_spacing);

    // --------------------------------------
    // END OF CUSTOMIZATION - LIVE ETA OPTION

#ifndef __OCPN__ANDROID__
    // MultiChart selection panel
    if (g_Platform->GetDisplayAreaCM2() > 100) {
      wxStaticBox* itemStaticBoxScreenConfig =
          new wxStaticBox(pDisplayPanel, wxID_ANY, _("Canvas Layout"));
      wxStaticBoxSizer* itemStaticBoxSizerScreenConfig =
          new wxStaticBoxSizer(itemStaticBoxScreenConfig, wxHORIZONTAL);
      wrapperSizer->Add(itemStaticBoxSizerScreenConfig, 1, wxALL | wxEXPAND, 5);

      //  The standard screen configs...
      wxString iconDir = g_Platform->GetSharedDataDir();
      appendOSDirSlash(&iconDir);
      iconDir.append(_T("uidata"));
      appendOSDirSlash(&iconDir);
      iconDir.append(_T("MUI_flat"));
      appendOSDirSlash(&iconDir);
      int bmpSize = GetCharHeight() * 3;

      wxBitmap bmp =
          LoadSVG(iconDir + _T("MUI_Sconfig_1.svg"), bmpSize, bmpSize);
      m_sconfigSelect_single =
          new CanvasConfigSelect(pDisplayPanel, this, ID_SCREENCONFIG1, bmp);
      itemStaticBoxSizerScreenConfig->Add(m_sconfigSelect_single, 0,
                                          wxALIGN_LEFT);

      itemStaticBoxSizerScreenConfig->AddSpacer(GetCharHeight());

      bmp = LoadSVG(iconDir + _T("MUI_Sconfig_2.svg"), bmpSize, bmpSize);
      m_sconfigSelect_twovertical =
          new CanvasConfigSelect(pDisplayPanel, this, ID_SCREENCONFIG2, bmp);
      itemStaticBoxSizerScreenConfig->Add(m_sconfigSelect_twovertical, 0,
                                          wxALIGN_LEFT);

      itemStaticBoxSizerScreenConfig->AddSpacer(GetCharHeight());
    }
#endif

  } else {  // compact follows
    wxFlexGridSizer* generalSizer = new wxFlexGridSizer(2);
    generalSizer->SetHGap(border_size);
    //    generalSizer->AddGrowableCol( 0, 1 );
    //    generalSizer->AddGrowableCol( 1, 1 );
    //    pDisplayPanel->SetSizer( generalSizer );

    // wxFlexGridSizer grows wrongly in wx2.8, so we need to centre it in
    // another sizer instead of letting it grow.
    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    pDisplayPanel->SetSizer(wrapperSizer);
    wrapperSizer->Add(generalSizer, 1, wxALL | wxALIGN_CENTER, border_size);

    // spacer
    generalSizer->Add(0, border_size * 4);
    generalSizer->Add(0, border_size * 4);

    if (!g_useMUI) {
      // Nav Mode
      generalSizer->Add(
          new wxStaticText(pDisplayPanel, wxID_ANY, _("Navigation Mode")),
          groupLabelFlags);
      wxBoxSizer* boxNavMode = new wxBoxSizer(wxVERTICAL);
      generalSizer->Add(boxNavMode, groupInputFlags);

      wxBoxSizer* rowOrientation = new wxBoxSizer(wxHORIZONTAL);
      boxNavMode->Add(rowOrientation);

      pCBNorthUp = new wxRadioButton(pDisplayPanel, wxID_ANY, _("North Up"));
      rowOrientation->Add(pCBNorthUp, inputFlags);
      pCBCourseUp =
          new wxRadioButton(pDisplayPanel, ID_COURSEUPCHECKBOX, _("Course Up"));
      rowOrientation->Add(pCBCourseUp,
                          wxSizerFlags(0)
                              .Align(wxALIGN_CENTRE_VERTICAL)
                              .Border(wxLEFT, group_item_spacing * 2));

      pCBLookAhead = new wxCheckBox(pDisplayPanel, ID_CHECK_LOOKAHEAD,
                                    _("Look Ahead Mode"));
      boxNavMode->Add(pCBLookAhead, verticleInputFlags);

      // spacer
      generalSizer->Add(0, border_size * 4);
      generalSizer->Add(0, border_size * 4);

      // Control Options
      generalSizer->Add(
          new wxStaticText(pDisplayPanel, wxID_ANY, _("Chart Display")),
          groupLabelFlags);
      wxBoxSizer* boxCharts = new wxBoxSizer(wxVERTICAL);
      generalSizer->Add(boxCharts, groupInputFlags);

      pCDOQuilting = new wxCheckBox(pDisplayPanel, ID_QUILTCHECKBOX1,
                                    _("Enable Chart Quilting"));
      boxCharts->Add(pCDOQuilting, verticleInputFlags);

      pPreserveScale =
          new wxCheckBox(pDisplayPanel, ID_PRESERVECHECKBOX,
                         _("Preserve scale when switching charts"));
      boxCharts->Add(pPreserveScale, verticleInputFlags);

      // spacer
      generalSizer->Add(0, border_size * 4);
      generalSizer->Add(0, border_size * 4);
    }

    // Control Options
    generalSizer->Add(new wxStaticText(pDisplayPanel, wxID_ANY, _("Controls")),
                      groupLabelFlags);
    wxBoxSizer* boxCtrls = new wxBoxSizer(wxVERTICAL);
    generalSizer->Add(boxCtrls, groupInputFlags);

    pSmoothPanZoom = new wxCheckBox(pDisplayPanel, ID_SMOOTHPANZOOMBOX,
                                    _("Smooth Panning / Zooming"));
    boxCtrls->Add(pSmoothPanZoom, verticleInputFlags);
    pEnableZoomToCursor =
        new wxCheckBox(pDisplayPanel, ID_ZTCCHECKBOX, _("Zoom to Cursor"));
    pEnableZoomToCursor->SetValue(FALSE);
    boxCtrls->Add(pEnableZoomToCursor, verticleInputFlags);

#ifdef __OCPN__ANDROID__
    pSmoothPanZoom->Hide();
    pEnableZoomToCursor->Hide();
#endif

    // spacer
    generalSizer->Add(0, border_size * 4);
    generalSizer->Add(0, border_size * 4);

    if (!g_useMUI) {
      // Display Options
      generalSizer->Add(
          new wxStaticText(pDisplayPanel, wxID_ANY, _("Display Features")),
          groupLabelFlags);
      wxBoxSizer* boxDisp = new wxBoxSizer(wxVERTICAL);
      generalSizer->Add(boxDisp, groupInputFlags);

      pSDisplayGrid =
          new wxCheckBox(pDisplayPanel, ID_CHECK_DISPLAYGRID, _("Show Grid"));
      boxDisp->Add(pSDisplayGrid, verticleInputFlags);

      pCDOOutlines = new wxCheckBox(pDisplayPanel, ID_OUTLINECHECKBOX1,
                                    _("Show Chart Outlines"));
      boxDisp->Add(pCDOOutlines, verticleInputFlags);

      pSDepthUnits = new wxCheckBox(pDisplayPanel, ID_SHOWDEPTHUNITSBOX1,
                                    _("Show Depth Units"));
      boxDisp->Add(pSDepthUnits, verticleInputFlags);
    }

    // CUSTOMIZATION - LIVE ETA OPTION
    // -------------------------------
    // Add a checkbox to activate live ETA option in status bar, and
    // Add a text field to set default boat speed (for calculation when
    // no GPS or when the boat is at the harbor).

    // Spacer
    generalSizer->Add(0, border_size * 4);
    generalSizer->Add(0, border_size * 4);

    // New menu status bar
    generalSizer->Add(
        new wxStaticText(pDisplayPanel, wxID_ANY, _("Status Bar")),
        groupLabelFlags);
    wxBoxSizer* boxDispStatusBar = new wxBoxSizer(wxVERTICAL);
    generalSizer->Add(boxDispStatusBar, groupInputFlags);

    // Add option for live ETA
    pSLiveETA = new wxCheckBox(pDisplayPanel, ID_CHECK_LIVEETA,
                               _("Live ETA at Cursor"));
    boxDispStatusBar->Add(pSLiveETA, verticleInputFlags);

    // Add text input for default boat speed
    // (for calculation, in case GPS speed is null)
    wxBoxSizer* defaultBoatSpeedSizer = new wxBoxSizer(wxHORIZONTAL);
    boxDispStatusBar->Add(defaultBoatSpeedSizer, wxALL, group_item_spacing);
    defaultBoatSpeedSizer->Add(
        new wxStaticText(pDisplayPanel, wxID_ANY, _("Default Boat Speed ")),
        groupLabelFlagsHoriz);
    pSDefaultBoatSpeed =
        new wxTextCtrl(pDisplayPanel, ID_DEFAULT_BOAT_SPEED, _T(""),
                       wxDefaultPosition, wxSize(50, -1), wxTE_RIGHT);
    defaultBoatSpeedSizer->Add(pSDefaultBoatSpeed, 0, wxALIGN_CENTER_VERTICAL,
                               group_item_spacing);

    // --------------------------------------
    // END OF CUSTOMIZATION - LIVE ETA OPTION

#ifndef __OCPN__ANDROID__
    if (g_Platform->GetDisplayAreaCM2() > 100) {
      // MultiChart selection panel
      wxStaticBox* itemStaticBoxScreenConfig =
          new wxStaticBox(pDisplayPanel, wxID_ANY, _("Canvas Layout"));
      wxStaticBoxSizer* itemStaticBoxSizerScreenConfig =
          new wxStaticBoxSizer(itemStaticBoxScreenConfig, wxHORIZONTAL);
      wrapperSizer->Add(itemStaticBoxSizerScreenConfig, 1, wxALL | wxEXPAND, 5);

      //  The standard screen configs...
      wxString iconDir = g_Platform->GetSharedDataDir();
      appendOSDirSlash(&iconDir);
      iconDir.append(_T("uidata"));
      appendOSDirSlash(&iconDir);
      iconDir.append(_T("MUI_flat"));
      appendOSDirSlash(&iconDir);
      int bmpSize = GetCharHeight() * 3;

      wxBitmap bmp =
          LoadSVG(iconDir + _T("MUI_Sconfig_1.svg"), bmpSize, bmpSize);
      m_sconfigSelect_single =
          new CanvasConfigSelect(pDisplayPanel, this, ID_SCREENCONFIG1, bmp);
      itemStaticBoxSizerScreenConfig->Add(m_sconfigSelect_single, 0,
                                          wxALIGN_LEFT);

      itemStaticBoxSizerScreenConfig->AddSpacer(GetCharHeight());

      bmp = LoadSVG(iconDir + _T("MUI_Sconfig_2.svg"), bmpSize, bmpSize);
      m_sconfigSelect_twovertical =
          new CanvasConfigSelect(pDisplayPanel, this, ID_SCREENCONFIG2, bmp);
      itemStaticBoxSizerScreenConfig->Add(m_sconfigSelect_twovertical, 0,
                                          wxALIGN_LEFT);

      itemStaticBoxSizerScreenConfig->AddSpacer(GetCharHeight());
    }
#endif
  }
}

void options::CreatePanel_Units(size_t parent, int border_size,
                                int group_item_spacing) {
  wxScrolledWindow* panelUnits = AddPage(parent, _("Units"));

  if (m_bcompact) {
    wxFlexGridSizer* unitsSizer = new wxFlexGridSizer(2);
    unitsSizer->SetHGap(border_size);

    // wxFlexGridSizer grows wrongly in wx2.8, so we need to centre it in
    // another sizer instead of letting it grow.
    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    panelUnits->SetSizer(wrapperSizer);

    wrapperSizer->Add(1, border_size * 24);
    wrapperSizer->Add(unitsSizer, 1, wxALL | wxALIGN_CENTER, border_size);

    // spacer
    unitsSizer->Add(0, border_size * 4);
    unitsSizer->Add(0, border_size * 4);

    // distance units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Distance")),
                    labelFlags);
    wxString pDistanceFormats[] = {_("Nautical miles"), _("Statute miles"),
                                   _("Kilometers"), _("Meters")};
    int m_DistanceFormatsNChoices = sizeof(pDistanceFormats) / sizeof(wxString);
    pDistanceFormat =
        new wxChoice(panelUnits, ID_DISTANCEUNITSCHOICE, wxDefaultPosition,
                     wxSize(m_fontHeight * 4, -1), m_DistanceFormatsNChoices,
                     pDistanceFormats);
#ifdef __OCPN__ANDROID__
    setChoiceStyleSheet(pDistanceFormat, m_fontHeight * 8 / 10);
#endif
    unitsSizer->Add(pDistanceFormat, inputFlags);

    // speed units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Speed")),
                    labelFlags);
    wxString pSpeedFormats[] = {_("Knots"), _("Mph"), _("km/h"), _("m/s")};
    int m_SpeedFormatsNChoices = sizeof(pSpeedFormats) / sizeof(wxString);
    pSpeedFormat = new wxChoice(panelUnits, ID_SPEEDUNITSCHOICE,
                                wxDefaultPosition, wxSize(m_fontHeight * 4, -1),
                                m_SpeedFormatsNChoices, pSpeedFormats);
#ifdef __OCPN__ANDROID__
    setChoiceStyleSheet(pSpeedFormat, m_fontHeight * 8 / 10);
#endif
    unitsSizer->Add(pSpeedFormat, inputFlags);

    // depth units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Depth")),
                    labelFlags);
    wxString pDepthUnitStrings[] = {
        _("Feet"),
        _("Meters"),
        _("Fathoms"),
    };
    pDepthUnitSelect =
        new wxChoice(panelUnits, ID_DEPTHUNITSCHOICE, wxDefaultPosition,
                     wxSize(m_fontHeight * 4, -1), 3, pDepthUnitStrings);
#ifdef __OCPN__ANDROID__
    setChoiceStyleSheet(pDepthUnitSelect, m_fontHeight * 8 / 10);
#endif
    unitsSizer->Add(pDepthUnitSelect, inputFlags);

    // temperature units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Temperature")),
                    labelFlags);
    wxString pTempUnitStrings[] = {
        _("Celsius"),
        _("Fahrenheit"),
        _("Kelvin"),
    };
    pTempFormat =
        new wxChoice(panelUnits, ID_TEMPUNITSCHOICE, wxDefaultPosition,
                     wxSize(m_fontHeight * 4, -1), 3, pTempUnitStrings);
#ifdef __OCPN__ANDROID__
    setChoiceStyleSheet(pTempFormat, m_fontHeight * 8 / 10);
#endif
    unitsSizer->Add(pTempFormat, inputFlags);

    // spacer
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _T("")));
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _T("")));

    // lat/long units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Lat/Long")),
                    labelFlags);
    wxString pSDMMFormats[] = {_("Degrees, Decimal Minutes"),
                               _("Decimal Degrees"),
                               _("Degrees, Minutes, Seconds")};
    int m_SDMMFormatsNChoices = sizeof(pSDMMFormats) / sizeof(wxString);
    pSDMMFormat = new wxChoice(panelUnits, ID_SDMMFORMATCHOICE,
                               wxDefaultPosition, wxSize(m_fontHeight * 4, -1),
                               m_SDMMFormatsNChoices, pSDMMFormats);
#ifdef __OCPN__ANDROID__
    setChoiceStyleSheet(pSDMMFormat, m_fontHeight * 8 / 10);
#endif
    unitsSizer->Add(pSDMMFormat, inputFlags);

    // spacer
    unitsSizer->Add(0, border_size * 4);
    unitsSizer->Add(0, border_size * 4);

    // bearings (magnetic/true, variation)
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Bearings")),
                    groupLabelFlags);

    //  "Mag Heading" checkbox
    pCBTrueShow =
        new wxCheckBox(panelUnits, ID_TRUESHOWCHECKBOX, _("Show true"));
    unitsSizer->Add(pCBTrueShow, 0, wxALL, group_item_spacing);
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _T("")));

    pCBMagShow =
        new wxCheckBox(panelUnits, ID_MAGSHOWCHECKBOX, _("Show magnetic"));
    unitsSizer->Add(pCBMagShow, 0, wxALL, group_item_spacing);
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _T("")));

    //  Mag Heading user variation

    wxStaticBox* itemStaticBoxVar =
        new wxStaticBox(panelUnits, wxID_ANY, _T(""));

    wxStaticBoxSizer* itemStaticBoxSizerVar =
        new wxStaticBoxSizer(itemStaticBoxVar, wxVERTICAL);
    wrapperSizer->Add(itemStaticBoxSizerVar, 0, wxALL | wxEXPAND, 5);

    itemStaticBoxSizerVar->Add(0, border_size * 4);

    itemStaticTextUserVar =
        new wxStaticText(panelUnits, wxID_ANY, _("Assumed magnetic variation"));
    itemStaticBoxSizerVar->Add(itemStaticTextUserVar, 1, wxEXPAND | wxALL,
                               group_item_spacing);

    wxBoxSizer* magVarSizer = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizerVar->Add(magVarSizer, 1, wxEXPAND | wxALL,
                               group_item_spacing);

    pMagVar = new wxTextCtrl(panelUnits, ID_OPTEXTCTRL, _T(""),
                             wxDefaultPosition, wxSize(150, -1), wxTE_RIGHT);
    magVarSizer->AddSpacer(100);

    magVarSizer->Add(pMagVar, 0, wxALIGN_CENTRE_VERTICAL, group_item_spacing);

    itemStaticTextUserVar2 =
        new wxStaticText(panelUnits, wxID_ANY, _("deg (-W, +E)"));

    magVarSizer->Add(itemStaticTextUserVar2, 0, wxALL | wxALIGN_CENTRE_VERTICAL,
                     group_item_spacing);

    itemStaticBoxSizerVar->Add(0, border_size * 40);

  } else {
    wxFlexGridSizer* unitsSizer = new wxFlexGridSizer(2);
    unitsSizer->SetHGap(border_size);

    // wxFlexGridSizer grows wrongly in wx2.8, so we need to centre it in
    // another sizer instead of letting it grow.
    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    panelUnits->SetSizer(wrapperSizer);
    wrapperSizer->Add(unitsSizer, 1, wxALL | wxALIGN_CENTER, border_size);

    // spacer
    unitsSizer->Add(0, border_size * 4);
    unitsSizer->Add(0, border_size * 4);

    // distance units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Distance")),
                    labelFlags);
    wxString pDistanceFormats[] = {_("Nautical miles"), _("Statute miles"),
                                   _("Kilometers"), _("Meters")};
    int m_DistanceFormatsNChoices = sizeof(pDistanceFormats) / sizeof(wxString);
    pDistanceFormat = new wxChoice(panelUnits, ID_DISTANCEUNITSCHOICE,
                                   wxDefaultPosition, wxDefaultSize,
                                   m_DistanceFormatsNChoices, pDistanceFormats);
    unitsSizer->Add(pDistanceFormat, inputFlags);

    // speed units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Speed")),
                    labelFlags);
    wxString pSpeedFormats[] = {_("Knots"), _("Mph"), _("km/h"), _("m/s")};
    int m_SpeedFormatsNChoices = sizeof(pSpeedFormats) / sizeof(wxString);
    pSpeedFormat =
        new wxChoice(panelUnits, ID_SPEEDUNITSCHOICE, wxDefaultPosition,
                     wxDefaultSize, m_SpeedFormatsNChoices, pSpeedFormats);
    unitsSizer->Add(pSpeedFormat, inputFlags);

    // depth units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Depth")),
                    labelFlags);
    wxString pDepthUnitStrings[] = {
        _("Feet"),
        _("Meters"),
        _("Fathoms"),
    };
    pDepthUnitSelect =
        new wxChoice(panelUnits, ID_DEPTHUNITSCHOICE, wxDefaultPosition,
                     wxDefaultSize, 3, pDepthUnitStrings);
    unitsSizer->Add(pDepthUnitSelect, inputFlags);

    // temperature units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Temperature")),
                    labelFlags);
    wxString pTempUnitStrings[] = {
        _("Celsius"),
        _("Fahrenheit"),
        _("Kelvin"),
    };
    pTempFormat =
        new wxChoice(panelUnits, ID_TEMPUNITSCHOICE, wxDefaultPosition,
                     wxDefaultSize, 3, pTempUnitStrings);
    unitsSizer->Add(pTempFormat, inputFlags);

    // spacer
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _T("")));
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _T("")));

    // lat/long units
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Lat/Long")),
                    labelFlags);
    wxString pSDMMFormats[] = {_("Degrees, Decimal Minutes"),
                               _("Decimal Degrees"),
                               _("Degrees, Minutes, Seconds")};
    int m_SDMMFormatsNChoices = sizeof(pSDMMFormats) / sizeof(wxString);
    pSDMMFormat =
        new wxChoice(panelUnits, ID_SDMMFORMATCHOICE, wxDefaultPosition,
                     wxDefaultSize, m_SDMMFormatsNChoices, pSDMMFormats);
    unitsSizer->Add(pSDMMFormat, inputFlags);

    // spacer
    unitsSizer->Add(0, border_size * 4);
    unitsSizer->Add(0, border_size * 4);

    // bearings (magnetic/true, variation)
    unitsSizer->Add(new wxStaticText(panelUnits, wxID_ANY, _("Bearings")),
                    groupLabelFlags);

    wxBoxSizer* bearingsSizer = new wxBoxSizer(wxVERTICAL);
    unitsSizer->Add(bearingsSizer, 0, 0, 0);

    //  "Mag Heading" checkbox
    pCBTrueShow = new wxCheckBox(panelUnits, ID_TRUESHOWCHECKBOX,
                                 _("Show true bearings and headings"));
    bearingsSizer->Add(pCBTrueShow, 0, wxALL, group_item_spacing);
    pCBMagShow = new wxCheckBox(panelUnits, ID_MAGSHOWCHECKBOX,
                                _("Show magnetic bearings and headings."));
    bearingsSizer->Add(pCBMagShow, 0, wxALL, group_item_spacing);

    bearingsSizer->AddSpacer(10);

    //  Mag Heading user variation
    wxBoxSizer* magVarSizer = new wxBoxSizer(wxHORIZONTAL);
    bearingsSizer->Add(magVarSizer, 0, wxALL, group_item_spacing);

    itemStaticTextUserVar =
        new wxStaticText(panelUnits, wxID_ANY, wxEmptyString);
    itemStaticTextUserVar->SetLabel(
        _("WMM Plugin calculated magnetic variation"));

    magVarSizer->Add(itemStaticTextUserVar, 0, wxALL | wxALIGN_CENTRE_VERTICAL,
                     group_item_spacing);

    pMagVar = new wxTextCtrl(panelUnits, ID_OPTEXTCTRL, _T(""),
                             wxDefaultPosition, wxSize(50, -1), wxTE_RIGHT);
    magVarSizer->Add(pMagVar, 0, wxALIGN_CENTRE_VERTICAL, group_item_spacing);

    itemStaticTextUserVar2 =
        new wxStaticText(panelUnits, wxID_ANY, _("deg (-W, +E)"));
    magVarSizer->Add(itemStaticTextUserVar2, 0, wxALL | wxALIGN_CENTRE_VERTICAL,
                     group_item_spacing);

    bearingsSizer->AddSpacer(10);

    wxStaticText* varText =
        new wxStaticText(panelUnits, wxID_ANY,
                         _(" To set the magnetic variation manually,\n you "
                           "must disable the WMM plugin."));
    smallFont = *dialogFont;
    smallFont.SetPointSize((smallFont.GetPointSize() / 1.2) +
                           0.5);  // + 0.5 to round instead of truncate
    varText->SetFont(smallFont);

    bearingsSizer->Add(varText);
  }
}

class OCPNSoundPanel: public wxPanel
{
public:
    OCPNSoundPanel( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                    wxString title,
                    wxString checkLegend,
                    wxString selectLegend,
                    wxString *pSoundFile
                  );

    ~OCPNSoundPanel() {};

    void OnButtonSelectSound(wxCommandEvent& event);
    void OnButtonTestSound(wxCommandEvent& event);
    wxString SelectSoundFile();
    void SetSoundFileLabel(wxString file);
    wxCheckBox *GetCheckBox(){ return m_pCheck_Sound; }

    wxCheckBox *m_pCheck_Sound;
    wxSize m_small_button_size;
    wxString m_sound_file;
    wxString *m_pSoundFile;
    wxStaticText *m_AudioFileNameText;

    wxButton* SelSound;
    wxButton* TestSound;

    DECLARE_EVENT_TABLE()
};

#define ID_SELECTSOUND 9341
#define ID_TESTSOUND 9342


BEGIN_EVENT_TABLE(OCPNSoundPanel, wxPanel)
EVT_BUTTON(ID_SELECTSOUND, OCPNSoundPanel::OnButtonSelectSound)
EVT_BUTTON(ID_TESTSOUND, OCPNSoundPanel::OnButtonTestSound)

END_EVENT_TABLE()

OCPNSoundPanel::OCPNSoundPanel( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
              wxString title,
              wxString checkLegend,
              wxString selectLegend,
              wxString *pSoundFile)
:wxPanel(parent, id, pos, size, wxBORDER_NONE)
{
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont( *pif );

  m_pSoundFile = pSoundFile;
  if(pSoundFile)
    m_sound_file = *pSoundFile;

  int border_size = 4;
  int group_item_spacing = 2;

  int font_size_y, font_descent, font_lead;
  GetTextExtent(_T("0"), NULL, &font_size_y, &font_descent, &font_lead);
  m_small_button_size =
      wxSize(-1, (int)(1.6 * (font_size_y + font_descent)));


  wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(wrapperSizer);

  wxStaticBox* StaticBox1 =
        new wxStaticBox(this, wxID_ANY, title);
  wxStaticBoxSizer* StaticBoxSizer1 =
        new wxStaticBoxSizer(StaticBox1, wxVERTICAL);
  wrapperSizer->Add(StaticBoxSizer1, 0, wxALL | wxEXPAND, border_size);

  m_pCheck_Sound = new wxCheckBox(this, ID_ANCHORALERTAUDIO,
                                          checkLegend);
  StaticBoxSizer1->Add(m_pCheck_Sound, 1, wxALL, group_item_spacing);

  // Sound file
  wxString LabelWrapped;

#ifdef __OCPN__ANDROID__
  // Count the likely number of lines in the wrapped file name
  // to set the default wxStaticText size.
  ChartDirPanelHardBreakWrapper wrapper(this, m_sound_file, g_pOptions->GetSize().x * 8 / 10);
  wxArrayString LabelWrappedArray = wrapper.GetLineArray();
  for (unsigned int i=0 ; i < LabelWrappedArray.GetCount() ; i++){
     LabelWrapped += "\n";
  }
  LabelWrapped += "\n";
#endif

  m_AudioFileNameText = new wxStaticText(this, wxID_ANY, LabelWrapped);
  m_AudioFileNameText->Wrap(-1);
  StaticBoxSizer1->Add(m_AudioFileNameText, 0, wxALL | wxEXPAND, border_size);

  SetSoundFileLabel(m_sound_file);

  //  Select/Test sound
  wxFlexGridSizer* soundSizer1 = new wxFlexGridSizer(3);
  soundSizer1->SetHGap(border_size * 2);
  StaticBoxSizer1->Add(soundSizer1, 1, wxALL | wxLEFT, border_size);

  SelSound = new wxButton(
        this, ID_SELECTSOUND, selectLegend,
        wxDefaultPosition, m_small_button_size, 0);
  soundSizer1->Add(SelSound, 0, wxALL | wxALIGN_RIGHT,
                     group_item_spacing);
  soundSizer1->AddSpacer(group_item_spacing * 4);
  TestSound = new wxButton(
        this, ID_TESTSOUND, _("Test"),
        wxDefaultPosition, m_small_button_size, 0);
  soundSizer1->Add(TestSound, 0, wxALL | wxALIGN_RIGHT,
                     group_item_spacing);

}

void OCPNSoundPanel::SetSoundFileLabel(wxString file)
{
  wxString soundLabel = wxString( " " + _("Audio file name:") + "\n " +
                file);

  ChartDirPanelHardBreakWrapper wrapper(this, soundLabel, g_pOptions->GetSize().x * 8 / 10);
  wxArrayString LabelWrappedArray = wrapper.GetLineArray();
  wxString LabelWrapped;
  for (unsigned int i=0 ; i < LabelWrappedArray.GetCount() ; i++){
    if (i == 0)
      LabelWrapped += LabelWrappedArray[i].BeforeFirst('/');
    else
      LabelWrapped += LabelWrappedArray[i];
    LabelWrapped += "\n";
  }

  m_AudioFileNameText->SetLabel(LabelWrapped);
  Layout();
}



wxString OCPNSoundPanel::SelectSoundFile() {
  wxString sound_dir = g_Platform->GetSharedDataDir();
  sound_dir.Append(_T("sounds"));
  wxString sel_file;
  int response;

#ifndef __OCPN__ANDROID__
  wxFileDialog* popenDialog = new wxFileDialog(
      NULL, _("Select Sound File"), sound_dir, wxEmptyString,
      _T("WAV files (*.wav)|*.wav|All files (*.*)|*.*"), wxFD_OPEN);
  if (g_bresponsive)
    popenDialog = g_Platform->AdjustFileDialogFont(this, popenDialog);

  response = popenDialog->ShowModal();
  sel_file = popenDialog->GetPath();
  delete popenDialog;

#else
  response =
      g_Platform->DoFileSelectorDialog(this, &sel_file, _("Select Sound File"),
                                       sound_dir, wxEmptyString, wxT("*.*"));
#endif

  wxString rv;
  if (response == wxID_OK)
    return g_Platform->NormalizePath(sel_file);
  else
    return _T("");
}

void OCPNSoundPanel::OnButtonSelectSound(wxCommandEvent& event) {
  wxString sel_file = SelectSoundFile();

  if (!sel_file.IsEmpty()) {
    m_sound_file = g_Platform->NormalizePath(sel_file);
    if(m_pSoundFile)
      *m_pSoundFile = m_sound_file;   // Update global variable

    SetSoundFileLabel(m_sound_file);
    //g_anchorwatch_sound->Stop();
  }
}

void OCPNSoundPanel::OnButtonTestSound(wxCommandEvent& event) {
  auto sound = SoundFactory();
  auto cmd_sound = dynamic_cast<SystemCmdSound*>(sound);
  if (cmd_sound) cmd_sound->SetCmd(g_CmdSoundString.mb_str());
  sound->SetFinishedCallback([sound](void*) { delete sound; });
  sound->Load(m_sound_file, g_iSoundDeviceIndex);
  sound->Play();
}






void options::CreatePanel_Sounds(size_t parent, int border_size,
                                 int group_item_spacing) {

    wxScrolledWindow* panelSounds = AddPage(parent, _("Sounds"));

    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    panelSounds->SetSizer(wrapperSizer);

    //  Anchor Alarm
    m_soundPanelAnchor = new OCPNSoundPanel( panelSounds, wxID_ANY,
                                                        wxDefaultPosition, wxDefaultSize,
                                                        _("Anchor Alarm"),
                                                        _("Play Sound on Anchor Alarm."),
                                                        _("Select Anchor Alarm Sound"),
                                                        &g_anchorwatch_sound_file
                                        );
    wrapperSizer->Add(m_soundPanelAnchor, 1, wxALL | wxEXPAND, border_size);


    // AIS Alert
    m_soundPanelAIS = new OCPNSoundPanel( panelSounds, wxID_ANY,
                                                        wxDefaultPosition, wxDefaultSize,
                                                        _("AIS Alert"),
                                                        _("Play Sound on AIS Alert."),
                                                        _("Select AIS Alert Sound"),
                                                        &g_AIS_sound_file
                                        );
    wrapperSizer->Add(m_soundPanelAIS, 1, wxALL | wxEXPAND, border_size);


    // SART Alert
    m_soundPanelSART = new OCPNSoundPanel( panelSounds, wxID_ANY,
                                                        wxDefaultPosition, wxDefaultSize,
                                                        _("SART Alert"),
                                                        _("Play Sound on AIS SART Alert."),
                                                        _("Select AIS SART Alert Sound"),
                                                        &g_SART_sound_file
                                        );
    wrapperSizer->Add(m_soundPanelSART, 1, wxALL | wxEXPAND, border_size);

    // DSC Call
    m_soundPanelDSC = new OCPNSoundPanel( panelSounds, wxID_ANY,
                                                        wxDefaultPosition, wxDefaultSize,
                                                        _("DSC Alert"),
                                                        _("Play Sound on DSC notification."),
                                                        _("Select DSC notification Sound"),
                                                        &g_DSC_sound_file
                                        );
    wrapperSizer->Add(m_soundPanelDSC, 1, wxALL | wxEXPAND, border_size);
}

void options::CreatePanel_MMSI(size_t parent, int border_size,
                               int group_item_spacing) {
  wxScrolledWindow* panelMMSI = AddPage(parent, _("MMSI Properties"));

  wxBoxSizer* MMSISizer = new wxBoxSizer(wxVERTICAL);
  panelMMSI->SetSizer(MMSISizer);

  // MMSI list control (panel)
  wxStaticBox* itemStaticBoxMMSI =
      new wxStaticBox(panelMMSI, wxID_ANY, _("MMSI Properties"));
  wxStaticBoxSizer* itemStaticBoxSizerMMSI =
      new wxStaticBoxSizer(itemStaticBoxMMSI, wxVERTICAL);
  MMSISizer->Add(itemStaticBoxSizerMMSI, 0, wxALL | wxEXPAND, border_size);

  MMSI_Props_Panel* pPropsPanel = new MMSI_Props_Panel(panelMMSI);

  pPropsPanel->UpdateMMSIList();

  itemStaticBoxSizerMMSI->Add(pPropsPanel, 0, wxALL | wxEXPAND, border_size);

  panelMMSI->Layout();
}

void options::CreatePanel_AIS(size_t parent, int border_size,
                              int group_item_spacing) {
  wxScrolledWindow* panelAIS = AddPage(parent, _("AIS Targets"));

  wxBoxSizer* aisSizer = new wxBoxSizer(wxVERTICAL);
  panelAIS->SetSizer(aisSizer);

  //      CPA Box
  wxStaticBox* itemStaticBoxCPA =
      new wxStaticBox(panelAIS, wxID_ANY, _("CPA Calculation"));
  wxStaticBoxSizer* itemStaticBoxSizerCPA =
      new wxStaticBoxSizer(itemStaticBoxCPA, wxVERTICAL);
  aisSizer->Add(itemStaticBoxSizerCPA, 0, wxALL | wxEXPAND, border_size);

  wxFlexGridSizer* pCPAGrid = new wxFlexGridSizer(2);
  pCPAGrid->AddGrowableCol(1);
  itemStaticBoxSizerCPA->Add(pCPAGrid, 0, wxALL | wxEXPAND, border_size);

  m_pCheck_CPA_Max = new wxCheckBox(
      panelAIS, -1,
      _("No (T)CPA Alerts if target range is greater than (NMi)"));
  pCPAGrid->Add(m_pCheck_CPA_Max, 0, wxALL, group_item_spacing);

  m_pText_CPA_Max = new wxTextCtrl(panelAIS, -1);
  pCPAGrid->Add(m_pText_CPA_Max, 0, wxALL | wxALIGN_RIGHT, group_item_spacing);

  m_pCheck_CPA_Warn =
      new wxCheckBox(panelAIS, -1, _("Warn if CPA less than (NMi)"));
  pCPAGrid->Add(m_pCheck_CPA_Warn, 0, wxALL, group_item_spacing);

  m_pText_CPA_Warn =
      new wxTextCtrl(panelAIS, -1, _T(""), wxDefaultPosition, wxSize(-1, -1));
  pCPAGrid->Add(m_pText_CPA_Warn, 0, wxALL | wxALIGN_RIGHT, group_item_spacing);

  m_pCheck_CPA_Warn->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                             wxCommandEventHandler(options::OnCPAWarnClick),
                             NULL, this);

  m_pCheck_CPA_WarnT =
      new wxCheckBox(panelAIS, -1, _("...and TCPA is less than (min)"));
  pCPAGrid->Add(m_pCheck_CPA_WarnT, 0, wxALL, group_item_spacing);

  m_pText_CPA_WarnT = new wxTextCtrl(panelAIS, -1);
  pCPAGrid->Add(m_pText_CPA_WarnT, 0, wxALL | wxALIGN_RIGHT,
                group_item_spacing);

  //      Lost Targets
  wxStaticBox* lostBox = new wxStaticBox(panelAIS, wxID_ANY, _("Lost Targets"));
  wxStaticBoxSizer* lostSizer = new wxStaticBoxSizer(lostBox, wxVERTICAL);
  aisSizer->Add(lostSizer, 0, wxALL | wxEXPAND, 3);

  wxFlexGridSizer* pLostGrid = new wxFlexGridSizer(2);
  pLostGrid->AddGrowableCol(1);
  lostSizer->Add(pLostGrid, 0, wxALL | wxEXPAND, border_size);

  m_pCheck_Mark_Lost =
      new wxCheckBox(panelAIS, -1, _("Mark targets as lost after (min)"));
  pLostGrid->Add(m_pCheck_Mark_Lost, 1, wxALL, group_item_spacing);

  m_pText_Mark_Lost = new wxTextCtrl(panelAIS, -1);
  pLostGrid->Add(m_pText_Mark_Lost, 1, wxALL | wxALIGN_RIGHT,
                 group_item_spacing);

  m_pCheck_Remove_Lost =
      new wxCheckBox(panelAIS, -1, _("Remove lost targets after (min)"));
  pLostGrid->Add(m_pCheck_Remove_Lost, 1, wxALL, group_item_spacing);

  m_pText_Remove_Lost = new wxTextCtrl(panelAIS, -1);
  pLostGrid->Add(m_pText_Remove_Lost, 1, wxALL | wxALIGN_RIGHT,
                 group_item_spacing);

  if (g_bInlandEcdis) lostSizer->Hide(pLostGrid, true);

  //      Display
  wxStaticBox* displBox = new wxStaticBox(panelAIS, wxID_ANY, _("Display"));
  wxStaticBoxSizer* displSizer = new wxStaticBoxSizer(displBox, wxHORIZONTAL);
  aisSizer->Add(displSizer, 0, wxALL | wxEXPAND, border_size);

  wxFlexGridSizer* pDisplayGrid = new wxFlexGridSizer(2);
  pDisplayGrid->AddGrowableCol(1);
  displSizer->Add(pDisplayGrid, 1, wxALL | wxEXPAND, border_size);

  m_pCheck_Show_COG = new wxCheckBox(
      panelAIS, -1, _("Show target COG predictor arrow, length (min)"));
  pDisplayGrid->Add(m_pCheck_Show_COG, 1, wxALL | wxEXPAND, group_item_spacing);

  m_pText_COG_Predictor = new wxTextCtrl(panelAIS, -1);
  pDisplayGrid->Add(m_pText_COG_Predictor, 1, wxALL | wxALIGN_RIGHT,
                    group_item_spacing);

  m_pCheck_Sync_OCOG_ACOG = new wxCheckBox(
    panelAIS, -1, _("Sync AIS arrow length with own ship's COG predictor"));
  pDisplayGrid->Add(m_pCheck_Sync_OCOG_ACOG, 1, wxALL, group_item_spacing);
  m_pCheck_Sync_OCOG_ACOG->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                             wxCommandEventHandler(options::OnSyncCogPredClick),
                             NULL, this);

  wxStaticText* pStatic_Dummy4a = new wxStaticText(panelAIS, -1, _T(""));
  pDisplayGrid->Add(pStatic_Dummy4a, 1, wxALL, group_item_spacing);

  m_pCheck_Show_Tracks =
      new wxCheckBox(panelAIS, -1, _("Show target tracks, length (min)"));
  pDisplayGrid->Add(m_pCheck_Show_Tracks, 1, wxALL, group_item_spacing);

  m_pText_Track_Length = new wxTextCtrl(panelAIS, -1);
  pDisplayGrid->Add(m_pText_Track_Length, 1, wxALL | wxALIGN_RIGHT,
                    group_item_spacing);

  m_pCheck_Hide_Moored = new wxCheckBox(
      panelAIS, -1, _("Suppress anchored/moored targets, speed max (kn)"));
  pDisplayGrid->Add(m_pCheck_Hide_Moored, 1, wxALL, group_item_spacing);

  m_pText_Moored_Speed = new wxTextCtrl(panelAIS, -1);
  pDisplayGrid->Add(m_pText_Moored_Speed, 1, wxALL | wxALIGN_RIGHT,
                    group_item_spacing);

  m_pCheck_Draw_Realtime_Prediction = new wxCheckBox(
      panelAIS, -1, _("Draw AIS realtime prediction, target speed min (kn)"));
  pDisplayGrid->Add(m_pCheck_Draw_Realtime_Prediction, 1, wxALL,
                    group_item_spacing);

  m_pText_RealtPred_Speed = new wxTextCtrl(panelAIS, -1);
  pDisplayGrid->Add(m_pText_RealtPred_Speed, 1, wxALL | wxALIGN_RIGHT,
                    group_item_spacing);

  m_pCheck_Scale_Priority = new wxCheckBox(
      panelAIS, -1,
      _("Allow attenuation of less critical targets if more than ... targets"));
  pDisplayGrid->Add(m_pCheck_Scale_Priority, 1, wxALL, group_item_spacing);

  m_pText_Scale_Priority = new wxTextCtrl(panelAIS, -1);
  pDisplayGrid->Add(m_pText_Scale_Priority, 1, wxALL | wxALIGN_RIGHT,
                    group_item_spacing);

  m_pCheck_Show_Area_Notices = new wxCheckBox(
      panelAIS, -1, _("Show area notices (from AIS binary messages)"));
  pDisplayGrid->Add(m_pCheck_Show_Area_Notices, 1, wxALL, group_item_spacing);

  wxStaticText* pStatic_Dummy5 = new wxStaticText(panelAIS, -1, _T(""));
  pDisplayGrid->Add(pStatic_Dummy5, 1, wxALL, group_item_spacing);

  m_pCheck_Draw_Target_Size =
      new wxCheckBox(panelAIS, -1, _("Show AIS targets real size"));
  pDisplayGrid->Add(m_pCheck_Draw_Target_Size, 1, wxALL, group_item_spacing);

  wxStaticText* pStatic_Dummy6 = new wxStaticText(panelAIS, -1, _T(""));
  pDisplayGrid->Add(pStatic_Dummy6, 1, wxALL, group_item_spacing);

  m_pCheck_Show_Target_Name = new wxCheckBox(
      panelAIS, -1, _("Show names with AIS targets at scale greater than 1:"));
  pDisplayGrid->Add(m_pCheck_Show_Target_Name, 1, wxALL, group_item_spacing);

  m_pText_Show_Target_Name_Scale = new wxTextCtrl(panelAIS, -1);
  pDisplayGrid->Add(m_pText_Show_Target_Name_Scale, 1, wxALL | wxALIGN_RIGHT,
                    group_item_spacing);

  m_pCheck_use_Wpl = new wxCheckBox(
      panelAIS, -1, _("Use WPL position messages. Action when received:"));
  pDisplayGrid->Add(m_pCheck_use_Wpl, 1, wxALL, group_item_spacing);

  wxString Wpl_Action[] = {_("APRS position report"), _("Create mark")};
  m_pWplAction = new wxChoice(panelAIS, wxID_ANY, wxDefaultPosition,
                              m_pShipIconType->GetSize(), 2, Wpl_Action);
  pDisplayGrid->Add(m_pWplAction, 0, wxALIGN_RIGHT | wxALL, group_item_spacing);

  // Rollover
  wxStaticBox* rolloverBox = new wxStaticBox(panelAIS, wxID_ANY, _("Rollover"));
  wxStaticBoxSizer* rolloverSizer =
      new wxStaticBoxSizer(rolloverBox, wxVERTICAL);
  aisSizer->Add(rolloverSizer, 0, wxALL | wxEXPAND, border_size);

  pRollover = new wxCheckBox(panelAIS, ID_ROLLOVERBOX,
                             _("Enable route/AIS info block"));
  rolloverSizer->Add(pRollover, 1, wxALL, 2 * group_item_spacing);

  pRollover->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                     wxCommandEventHandler(options::OnAISRolloverClick), NULL,
                     this);

  pStatic_CallSign =
      new wxStaticText(panelAIS, -1, _("\"Ship Name\" MMSI (Call Sign)"));
  rolloverSizer->Add(pStatic_CallSign, 1, wxALL, 2 * group_item_spacing);

  m_pCheck_Rollover_Class =
      new wxCheckBox(panelAIS, -1, _("[Class] Type (Status)"));
  rolloverSizer->Add(m_pCheck_Rollover_Class, 1, wxALL, 2 * group_item_spacing);

  m_pCheck_Rollover_COG = new wxCheckBox(panelAIS, -1, _("SOG COG"));
  rolloverSizer->Add(m_pCheck_Rollover_COG, 1, wxALL, 2 * group_item_spacing);

  m_pCheck_Rollover_CPA = new wxCheckBox(panelAIS, -1, _("CPA TCPA"));
  rolloverSizer->Add(m_pCheck_Rollover_CPA, 1, wxALL, 2 * group_item_spacing);

  //      Alert Box
  wxStaticBox* alertBox =
      new wxStaticBox(panelAIS, wxID_ANY, _("CPA/TCPA Alerts"));
  wxStaticBoxSizer* alertSizer = new wxStaticBoxSizer(alertBox, wxVERTICAL);
  aisSizer->Add(alertSizer, 0, wxALL | wxEXPAND, group_item_spacing);

  wxFlexGridSizer* pAlertGrid = new wxFlexGridSizer(2);
  pAlertGrid->AddGrowableCol(1);
  alertSizer->Add(pAlertGrid, 0, wxALL | wxEXPAND, group_item_spacing);

  m_pCheck_AlertDialog = new wxCheckBox(panelAIS, ID_AISALERTDIALOG,
                                        _("Show CPA/TCPA Alert Dialog"));
  pAlertGrid->Add(m_pCheck_AlertDialog, 0, wxALL, group_item_spacing);

  m_pCheck_AlertDialog->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(options::OnAlertEnableButtonClick), NULL, this);

  //   wxButton* m_SelSound =
  //       new wxButton(panelAIS, ID_AISALERTSELECTSOUND, _("Select Alert
  //       Sound"),
  //                    wxDefaultPosition, m_small_button_size, 0);
  //   pAlertGrid->Add(m_SelSound, 0, wxALL | wxALIGN_RIGHT,
  //   group_item_spacing);

  wxStaticText* pStatic_Dummy5a = new wxStaticText(panelAIS, -1, _T(""));
  pAlertGrid->Add(pStatic_Dummy5a, 1, wxALL, group_item_spacing);

  m_pCheck_AlertAudio = new wxCheckBox(
      panelAIS, ID_AISALERTAUDIO,
      _("Play Sound on CPA/TCPA Alerts and DSC/SART emergencies."));
  pAlertGrid->Add(m_pCheck_AlertAudio, 0, wxALL, group_item_spacing);

  m_pCheck_AlertAudio->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(options::OnAlertAudioEnableButtonClick), NULL,
      this);

  //   wxButton* m_pPlay_Sound =
  //       new wxButton(panelAIS, ID_AISALERTTESTSOUND, _("Test Alert Sound"),
  //                    wxDefaultPosition, m_small_button_size, 0);
  //   pAlertGrid->Add(m_pPlay_Sound, 0, wxALL | wxALIGN_RIGHT,
  //   group_item_spacing);

  wxStaticText* pStatic_Dummy5b = new wxStaticText(panelAIS, -1, _T(""));
  pAlertGrid->Add(pStatic_Dummy5b, 1, wxALL, group_item_spacing);

  m_pCheck_Alert_Moored = new wxCheckBox(
      panelAIS, -1, _("Suppress Alerts for anchored/moored targets"));
  pAlertGrid->Add(m_pCheck_Alert_Moored, 1, wxALL, group_item_spacing);

  wxStaticText* pStatic_Dummy2 = new wxStaticText(panelAIS, -1, _T(""));
  pAlertGrid->Add(pStatic_Dummy2, 1, wxALL, group_item_spacing);

  m_pCheck_Ack_Timout = new wxCheckBox(
      panelAIS, -1, _("Enable Target Alert Acknowledge timeout (min)"));
  pAlertGrid->Add(m_pCheck_Ack_Timout, 1, wxALL, group_item_spacing);

  m_pText_ACK_Timeout = new wxTextCtrl(panelAIS, -1);
  pAlertGrid->Add(m_pText_ACK_Timeout, 1, wxALL | wxALIGN_RIGHT,
                  group_item_spacing);

  panelAIS->Layout();
}

class MouseZoomSlider : public wxSlider {
public:
  MouseZoomSlider(wxWindow* parent, wxSize size)
      : wxSlider(parent, wxID_ANY, 10, 1, 100, wxDefaultPosition, size,
                 SLIDER_STYLE) {
    Show();
#ifdef __OCPN__ANDROID__
    GetHandle()->setStyleSheet(getQtStyleSheet());
#endif
  }
};

void options::CreatePanel_UI(size_t parent, int border_size,
                             int group_item_spacing) {
  wxScrolledWindow* itemPanelFont = AddPage(parent, _("General Options"));

  m_itemBoxSizerFontPanel = new wxBoxSizer(wxVERTICAL);
  itemPanelFont->SetSizer(m_itemBoxSizerFontPanel);

  wxBoxSizer* langStyleBox = new wxBoxSizer(wxHORIZONTAL);
  m_itemBoxSizerFontPanel->Add(langStyleBox, 0, wxEXPAND | wxALL, border_size);

  wxStaticBox* itemLangStaticBox =
      new wxStaticBox(itemPanelFont, wxID_ANY, _("Language"));
  wxStaticBoxSizer* itemLangStaticBoxSizer =
      new wxStaticBoxSizer(itemLangStaticBox, wxVERTICAL);

  langStyleBox->Add(itemLangStaticBoxSizer, 1, wxEXPAND | wxALL, border_size);

  m_itemLangListBox = new wxChoice(itemPanelFont, ID_CHOICE_LANG);

  itemLangStaticBoxSizer->Add(m_itemLangListBox, 0, wxEXPAND | wxALL,
                              border_size);
#ifdef __OCPN__ANDROID__
  m_itemLangListBox->Disable();
#endif

  wxStaticBox* itemFontStaticBox =
      new wxStaticBox(itemPanelFont, wxID_ANY, _("Fonts"));

  wxSize fontChoiceSize = wxSize(-1, -1);

  int fLayout = wxHORIZONTAL;
#ifdef __OCPN__ANDROID__
  // Compensate for very narrow displays on Android
  if (m_nCharWidthMax < 40) fLayout = wxVERTICAL;

  // Need to set wxChoice vertical size explicitely in Android
  fontChoiceSize = wxSize(-1, m_fontHeight * 3 / 4);
#endif

  wxStaticBoxSizer* itemFontStaticBoxSizer =
      new wxStaticBoxSizer(itemFontStaticBox, fLayout);
  m_itemBoxSizerFontPanel->Add(itemFontStaticBoxSizer, 0, wxEXPAND | wxALL,
                               border_size);

  m_itemFontElementListBox =
      new wxChoice(itemPanelFont, ID_CHOICE_FONTELEMENT, wxDefaultPosition,
                   fontChoiceSize, 0, NULL, wxCB_SORT);

  int nFonts = FontMgr::Get().GetNumFonts();
  for (int it = 0; it < nFonts; it++) {
    const wxString& t = FontMgr::Get().GetDialogString(it);

    if (FontMgr::Get().GetConfigString(it).StartsWith(g_locale)) {
      m_itemFontElementListBox->Append(t);
    }
  }

  if (nFonts) m_itemFontElementListBox->SetSelection(0);

  itemFontStaticBoxSizer->Add(m_itemFontElementListBox, 0, wxALL, border_size);

  wxButton* itemFontChooseButton =
      new wxButton(itemPanelFont, ID_BUTTONFONTCHOOSE, _("Choose Font..."),
                   wxDefaultPosition, wxDefaultSize, 0);
  itemFontStaticBoxSizer->Add(itemFontChooseButton, 0, wxALL, border_size);
#if defined(__WXGTK__) || defined(__WXQT__)
  wxButton* itemFontColorButton =
      new wxButton(itemPanelFont, ID_BUTTONFONTCOLOR, _("Choose Font Color..."),
                   wxDefaultPosition, wxDefaultSize, 0);
  itemFontStaticBoxSizer->Add(itemFontColorButton, 0, wxALL, border_size);
#endif
  m_textSample = new wxStaticText(itemPanelFont, wxID_ANY, _("Sample"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  itemFontStaticBoxSizer->Add(m_textSample, 0, wxALL, border_size);
  wxCommandEvent e;
  OnFontChoice(e);

#if 0
  wxStaticBox* itemStyleStaticBox =
      new wxStaticBox(itemPanelFont, wxID_ANY, _("Toolbar and Window Style"));
  wxStaticBoxSizer* itemStyleStaticBoxSizer =
      new wxStaticBoxSizer(itemStyleStaticBox, wxVERTICAL);
  langStyleBox->Add(itemStyleStaticBoxSizer, 1, wxEXPAND | wxALL, border_size);

  m_itemStyleListBox = new wxChoice(itemPanelFont, ID_STYLESCOMBOBOX);

  wxArrayPtrVoid styles = g_StyleManager->GetArrayOfStyles();
  for (unsigned int i = 0; i < styles.Count(); i++) {
    ocpnStyle::Style* style = (ocpnStyle::Style*)(styles[i]);
    m_itemStyleListBox->Append(style->name);
  }
  m_itemStyleListBox->SetStringSelection(
      g_StyleManager->GetCurrentStyle()->name);
  itemStyleStaticBoxSizer->Add(m_itemStyleListBox, 1, wxEXPAND | wxALL,
                               border_size);
#endif
  wxStaticBox* miscOptionsBox =
      new wxStaticBox(itemPanelFont, wxID_ANY, _("Interface Options"));
  wxStaticBoxSizer* miscOptions =
      new wxStaticBoxSizer(miscOptionsBox, wxVERTICAL);
  m_itemBoxSizerFontPanel->Add(miscOptions, 0, wxALL | wxEXPAND, border_size);

  pShowStatusBar =
      new wxCheckBox(itemPanelFont, ID_DEBUGCHECKBOX1, _("Show Status Bar"));
  pShowStatusBar->SetValue(FALSE);
  miscOptions->Add(pShowStatusBar, 0, wxALL, border_size);

#ifndef __WXOSX__
  pShowMenuBar = new wxCheckBox(itemPanelFont, wxID_ANY, _("Show Menu Bar"));
  pShowMenuBar->SetValue(FALSE);
  miscOptions->Add(pShowMenuBar, 0, wxALL, border_size);
#endif

#ifdef __OCPN__ANDROID__
  pShowMenuBar->Hide();
#endif

  pShowChartBar = new wxCheckBox(itemPanelFont, wxID_ANY, _("Show Chart Bar"));
  pShowChartBar->SetValue(g_bShowChartBar);
  miscOptions->Add(pShowChartBar, 0, wxALL, border_size);

  pShowCompassWin = new wxCheckBox(itemPanelFont, wxID_ANY,
                                   _("Show Compass/GPS Status Window"));
  pShowCompassWin->SetValue(FALSE);
  miscOptions->Add(pShowCompassWin, 0, wxALL, border_size);

  wxBoxSizer* pToolbarAutoHide = new wxBoxSizer(wxHORIZONTAL);
  miscOptions->Add(pToolbarAutoHide, 0, wxALL | wxEXPAND, group_item_spacing);

  pToolbarAutoHideCB =
      new wxCheckBox(itemPanelFont, wxID_ANY, _("Enable Toolbar auto-hide"));
  pToolbarAutoHide->Add(pToolbarAutoHideCB, 0, wxALL, group_item_spacing);

  pToolbarHideSecs =
      new wxTextCtrl(itemPanelFont, ID_OPTEXTCTRL, _T(""), wxDefaultPosition,
                     wxSize(50, -1), wxTE_RIGHT);
  pToolbarAutoHide->Add(pToolbarHideSecs, 0, wxALL, group_item_spacing);

  pToolbarAutoHide->Add(new wxStaticText(itemPanelFont, wxID_ANY, _("seconds")),
                        group_item_spacing);

  wxBoxSizer* pShipsBellsSizer = new wxBoxSizer(wxHORIZONTAL);
  miscOptions->Add(pShipsBellsSizer, 0, wxALL, group_item_spacing);
  // Sound options
  pPlayShipsBells =
      new wxCheckBox(itemPanelFont, ID_BELLSCHECKBOX, _("Play Ships Bells"));
  pShipsBellsSizer->Add(pPlayShipsBells, 0, wxALL | wxEXPAND, border_size);

#ifndef __OCPN__ANDROID__
  if (g_bUIexpert && (bool)dynamic_cast<SystemCmdSound*>(SoundFactory())) {
    wxBoxSizer* pSoundSizer = new wxBoxSizer(wxVERTICAL);
    pShipsBellsSizer->Add(pSoundSizer, 0, wxALL | wxEXPAND, group_item_spacing);
    pCmdSoundString =
        new wxTextCtrl(itemPanelFont, wxID_ANY, _T( "" ), wxDefaultPosition,
                       wxSize(450, -1), wxTE_LEFT);
    pSoundSizer->Add(
        new wxStaticText(itemPanelFont, wxID_ANY, _("Audio Play command:")), 0,
        wxALIGN_CENTER_HORIZONTAL | wxALL);
    pSoundSizer->Add(pCmdSoundString, 1, wxEXPAND | wxALIGN_LEFT, border_size);
  }
#endif

  auto sound = std::unique_ptr<OcpnSound>(SoundFactory());
  int deviceCount = sound->DeviceCount();
  wxLogMessage("options: got device count: %d", deviceCount);
  if (deviceCount >= 1) {
    wxArrayString labels;
    for (int i = 0; i < deviceCount; i += 1) {
      wxString label(sound->GetDeviceInfo(i));
      if (label == "") {
        std::ostringstream stm;
        stm << i;
        label = _("Unknown device :") + stm.str();
      }
      if (!sound->IsOutputDevice(i)) {
        std::ostringstream stm;
        stm << i;
        label = _("Input device :") + stm.str();
      }
      labels.Add(label);
    }

    //  if sound device index is uninitialized, set to "default", if found.
    // Otherwise, set to 0
    int iDefault = labels.Index("default");

    if (g_iSoundDeviceIndex == -1) {
      if (iDefault >= 0)
        g_iSoundDeviceIndex = iDefault;
      else
        g_iSoundDeviceIndex = 0;
    }

    pSoundDeviceIndex = new wxChoice();
    if (pSoundDeviceIndex) {
      pSoundDeviceIndex->Create(itemPanelFont, wxID_ANY, wxDefaultPosition,
                                wxDefaultSize, labels);
      pSoundDeviceIndex->SetSelection(g_iSoundDeviceIndex);
      pSoundDeviceIndex->Show();
      wxFlexGridSizer* pSoundDeviceIndexGrid = new wxFlexGridSizer(2);
      miscOptions->Add(pSoundDeviceIndexGrid, 0, wxALL | wxEXPAND,
                       group_item_spacing);

      stSoundDeviceIndex =
          new wxStaticText(itemPanelFont, wxID_STATIC, _("Sound Device"));
      pSoundDeviceIndexGrid->Add(stSoundDeviceIndex, 0, wxALL, 5);
      pSoundDeviceIndexGrid->Add(pSoundDeviceIndex, 0, wxALL, border_size);
    }
  }
#ifdef __OCPN__ANDROID__
  stSoundDeviceIndex->Hide();
  pSoundDeviceIndex->Hide();
#endif

  //  Mobile/Touchscreen checkboxes
  pMobile = new wxCheckBox(itemPanelFont, ID_MOBILEBOX,
                           _("Enable Touchscreen interface"));
  miscOptions->Add(pMobile, 0, wxALL, border_size);

  pResponsive = new wxCheckBox(itemPanelFont, ID_REPONSIVEBOX,
                               _("Enable Scaled Graphics interface"));
  miscOptions->Add(pResponsive, 0, wxALL, border_size);

  //  This two options are always needed for Android
#ifdef __OCPN__ANDROID__
  pMobile->Hide();
  pResponsive->Hide();
#endif

  pZoomButtons =
      new wxCheckBox(itemPanelFont, ID_ZOOMBUTTONS, _("Show Zoom buttons"));
  miscOptions->Add(pZoomButtons, 0, wxALL, border_size);
#ifndef __OCPN__ANDROID__
  pZoomButtons->Hide();
#endif

  pInlandEcdis = new wxCheckBox(itemPanelFont, ID_INLANDECDISBOX,
                                _("Use Inland ECDIS V2.3"));
  miscOptions->Add(pInlandEcdis, 0, wxALL, border_size);

#ifdef __OCPN__ANDROID__
  pInlandEcdis->Hide();
#endif

  miscOptions->AddSpacer(10);

  wxFlexGridSizer* sliderSizer;
  sliderSizer = new wxFlexGridSizer(0, 2, 0, 0);
  sliderSizer->AddGrowableCol(1);
  sliderSizer->SetFlexibleDirection(wxBOTH);
  sliderSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_pSlider_GUI_Factor =
      new wxSlider(itemPanelFont, wxID_ANY, 0, -5, 5, wxDefaultPosition,
                   m_sliderSize, SLIDER_STYLE);
  m_pSlider_GUI_Factor->Hide();
  sliderSizer->Add(new wxStaticText(itemPanelFont, wxID_ANY,
                                    _("User Interface scale factor")),
                   inputFlags);
  sliderSizer->Add(m_pSlider_GUI_Factor, 0, wxALL, border_size);
  m_pSlider_GUI_Factor->Show();

#ifdef __OCPN__ANDROID__
  prepareSlider(m_pSlider_GUI_Factor);
#endif

  m_pSlider_Chart_Factor =
      new wxSlider(itemPanelFont, wxID_ANY, 0, -5, 5, wxDefaultPosition,
                   m_sliderSize, SLIDER_STYLE);
  m_pSlider_Chart_Factor->Hide();
  sliderSizer->Add(
      new wxStaticText(itemPanelFont, wxID_ANY, _("Chart Object scale factor")),
      inputFlags);
  sliderSizer->Add(m_pSlider_Chart_Factor, 0, wxALL, border_size);
  m_pSlider_Chart_Factor->Show();

#ifdef __OCPN__ANDROID__
  prepareSlider(m_pSlider_Chart_Factor);
#endif

  m_pSlider_Ship_Factor =
      new wxSlider(itemPanelFont, wxID_ANY, 0, -5, 5, wxDefaultPosition,
                   m_sliderSize, SLIDER_STYLE);
  m_pSlider_Ship_Factor->Hide();
  sliderSizer->Add(
      new wxStaticText(itemPanelFont, wxID_ANY, _("Ship scale factor")),
      inputFlags);
  sliderSizer->Add(m_pSlider_Ship_Factor, 0, wxALL, border_size);
  m_pSlider_Ship_Factor->Show();

#ifdef __OCPN__ANDROID__
  m_pSlider_Ship_Factor->GetHandle()->setStyleSheet(getQtStyleSheet());
#endif

  m_pSlider_Text_Factor =
      new wxSlider(itemPanelFont, wxID_ANY, 0, -5, 5, wxDefaultPosition,
                   m_sliderSize, SLIDER_STYLE);
  m_pSlider_Text_Factor->Hide();
  sliderSizer->Add(
      new wxStaticText(itemPanelFont, wxID_ANY, _("ENC Sounding factor")),
      inputFlags);
  sliderSizer->Add(m_pSlider_Text_Factor, 0, wxALL, border_size);
  m_pSlider_Text_Factor->Show();

#ifdef __OCPN__ANDROID__
  m_pSlider_Text_Factor->GetHandle()->setStyleSheet(getQtStyleSheet());
#endif

  sliderSizer->Add(
      new wxStaticText(itemPanelFont, wxID_ANY, "Mouse wheel zoom sensitivity"),
      inputFlags);
  m_pMouse_Zoom_Slider = new MouseZoomSlider(itemPanelFont, m_sliderSize);
  sliderSizer->Add(m_pMouse_Zoom_Slider, 0, wxALL, border_size);

  miscOptions->Add(sliderSizer, 0, wxEXPAND, 5);
  miscOptions->AddSpacer(20);
}

void options::OnAlertEnableButtonClick(wxCommandEvent& event) {
  if (event.IsChecked()) {
    m_pCheck_AlertAudio->Enable();
    m_soundPanelAIS->GetCheckBox()->Enable();
    m_soundPanelSART->GetCheckBox()->Enable();
    m_soundPanelDSC->GetCheckBox()->Enable();
  } else {
    m_pCheck_AlertAudio->Disable();
    m_soundPanelAIS->GetCheckBox()->Disable();
    m_soundPanelSART->GetCheckBox()->Disable();
    m_soundPanelDSC->GetCheckBox()->Disable();
  }
}

void options::OnAlertAudioEnableButtonClick(wxCommandEvent& event) {
  if (event.IsChecked()) {
    m_soundPanelAIS->GetCheckBox()->Enable();
    m_soundPanelSART->GetCheckBox()->Enable();
    m_soundPanelDSC->GetCheckBox()->Enable();
  } else {
    m_soundPanelAIS->GetCheckBox()->Disable();
    m_soundPanelSART->GetCheckBox()->Disable();
    m_soundPanelDSC->GetCheckBox()->Disable();
  }
}

void options::CreateListbookIcons() {
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

  if (!g_bresponsive) {
    m_topImgList = new wxImageList(40, 40, TRUE, 1);

#if wxCHECK_VERSION(2, 8, 12)
    m_topImgList->Add(style->GetIcon(_T("Display")));
    m_topImgList->Add(style->GetIcon(_T("Charts")));
    m_topImgList->Add(style->GetIcon(_T("Connections")));
    m_topImgList->Add(style->GetIcon(_T("Ship")));
    m_topImgList->Add(style->GetIcon(_T("UI")));
    m_topImgList->Add(style->GetIcon(_T("Plugins")));
#else
    wxBitmap bmp;
    wxImage img;
    bmp = style->GetIcon(_T("Display"));
    img = bmp.ConvertToImage();
    img.ConvertAlphaToMask(128);
    bmp = wxBitmap(img);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("Charts"));
    img = bmp.ConvertToImage();
    img.ConvertAlphaToMask(128);
    bmp = wxBitmap(img);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("Connections"));
    img = bmp.ConvertToImage();
    img.ConvertAlphaToMask(128);
    bmp = wxBitmap(img);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("Ship"));
    img = bmp.ConvertToImage();
    img.ConvertAlphaToMask(128);
    bmp = wxBitmap(img);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("UI"));
    img = bmp.ConvertToImage();
    img.ConvertAlphaToMask(128);
    bmp = wxBitmap(img);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("Plugins"));
    img = bmp.ConvertToImage();
    img.ConvertAlphaToMask(128);
    bmp = wxBitmap(img);
    m_topImgList->Add(bmp);
#endif
  } else {
    wxBitmap bmps;
    bmps = style->GetIcon(_T("Display"));
    int base_size = bmps.GetWidth();
    double tool_size = base_size;

    double premult = 1.0;

    // unless overridden by user, we declare the "best" size
    // to be roughly 6 mm square.
    double target_size = 6.0;  // mm

    double basic_tool_size_mm = tool_size / g_Platform->GetDisplayDPmm();
    premult = target_size / basic_tool_size_mm;

    // Adjust the scale factor using the global GUI scale parameter
    double postmult = exp(g_GUIScaleFactor * (0.693 / 5.0));  //  exp(2)
    postmult = wxMin(postmult, 3.0);
    postmult = wxMax(postmult, 1.0);

    int sizeTab = base_size * postmult * premult;

    m_topImgList = new wxImageList(sizeTab, sizeTab, TRUE, 1);

    wxBitmap bmp;
    wxImage img, simg;
    bmp = style->GetIcon(_T("Display"));
    img = bmp.ConvertToImage();
    simg = img.Scale(sizeTab, sizeTab);
    bmp = wxBitmap(simg);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("Charts"));
    img = bmp.ConvertToImage();
    simg = img.Scale(sizeTab, sizeTab);
    bmp = wxBitmap(simg);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("Connections"));
    img = bmp.ConvertToImage();
    simg = img.Scale(sizeTab, sizeTab);
    bmp = wxBitmap(simg);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("Ship"));
    img = bmp.ConvertToImage();
    simg = img.Scale(sizeTab, sizeTab);
    bmp = wxBitmap(simg);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("UI"));
    img = bmp.ConvertToImage();
    simg = img.Scale(sizeTab, sizeTab);
    bmp = wxBitmap(simg);
    m_topImgList->Add(bmp);
    bmp = style->GetIcon(_T("Plugins"));
    img = bmp.ConvertToImage();
    simg = img.Scale(sizeTab, sizeTab);
    bmp = wxBitmap(simg);
    m_topImgList->Add(bmp);
  }
}

void options::CreateControls(void) {
  int border_size = 4;
  // use for items within one group, with Add(...wxALL)
  int group_item_spacing = 2;

  int font_size_y, font_descent, font_lead;
  GetTextExtent(_T("0"), NULL, &font_size_y, &font_descent, &font_lead);
  m_fontHeight = font_size_y + font_descent + font_lead;

#ifdef __OCPN__ANDROID__
  m_sliderSize =
      wxSize(wxMin(m_fontHeight * 8, g_Platform->getDisplaySize().x / 2),
             m_fontHeight * 8 / 10);
#else
  m_sliderSize =
      wxSize(wxMin(m_fontHeight * 8, g_Platform->getDisplaySize().x / 2),
             m_fontHeight * 2);
#endif

  m_small_button_size =
      wxSize(-1, (int)(1.2 * (font_size_y + font_descent /*+ font_lead*/)));

  m_nCharWidthMax = GetSize().x / GetCharWidth();

  // Some members (pointers to controls) need to initialized
  pEnableZoomToCursor = NULL;
  pSmoothPanZoom = NULL;

  // Check the display size.
  // If "small", adjust some factors to squish out some more white space
  int width, height;
  ::wxDisplaySize(&width, &height);

  if (!g_bresponsive && height <= 800) {
    border_size = 2;
    group_item_spacing = 1;
  }

  labelFlags =
      wxSizerFlags(0).Align(wxALIGN_RIGHT).Border(wxALL, group_item_spacing);
  inputFlags = wxSizerFlags(0)
                   .Align(wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL)
                   .Border(wxALL, group_item_spacing);
  verticleInputFlags =
      wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxALL, group_item_spacing);
  groupLabelFlags = wxSizerFlags(0)
                        .Align(wxALIGN_RIGHT | wxALIGN_TOP)
                        .Border(wxALL, group_item_spacing);
  groupLabelFlagsHoriz =
      wxSizerFlags(0).Align(wxALIGN_TOP).Border(wxALL, group_item_spacing);
  groupInputFlags = wxSizerFlags(0)
                        .Align(wxALIGN_LEFT | wxALIGN_TOP)
                        .Border(wxBOTTOM, group_item_spacing * 2)
                        .Expand();

#ifdef __WXGTK__
  groupLabelFlags.Border(wxTOP, group_item_spacing + 3);
#endif

  options* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

#ifdef __OCPN__ANDROID__
  //  Set Dialog Font by custom crafted Qt Stylesheet.
  wxFont* qFont = dialogFont;

  wxString wqs = getFontQtStylesheet(qFont);
  wxCharBuffer sbuf = wqs.ToUTF8();
  QString qsb = QString(sbuf.data());

  QString qsbq = getQtStyleSheet();  // basic scrollbars, etc

  itemDialog1->GetHandle()->setStyleSheet(qsb +
                                          qsbq);  // Concatenated style sheets

#endif

  int flags = 0;

#ifdef __OCPN__OPTIONS_USE_LISTBOOK__
  flags = wxLB_TOP;
  m_pListbook = new wxListbook(itemDialog1, ID_NOTEBOOK, wxDefaultPosition,
                               wxSize(-1, -1), flags);
  m_pListbook->Connect(wxEVT_COMMAND_LISTBOOK_PAGE_CHANGED,
                       wxListbookEventHandler(options::OnPageChange), NULL,
                       this);
#else
  flags = wxNB_TOP;
  m_pListbook = new wxNotebook(itemDialog1, ID_NOTEBOOK, wxDefaultPosition,
                               wxSize(-1, -1), flags);
  m_pListbook->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                       wxNotebookEventHandler(options::OnTopNBPageChange), NULL,
                       this);
#endif

#ifdef __OCPN__ANDROID__
  //  In wxQT, we can dynamically style the little scroll buttons on a small
  //  display, to make them bigger
  m_pListbook->GetHandle()->setStyleSheet(getListBookStyleSheet());

#endif

#ifdef __WXMSW__
  //  Windows clips the width of listbook selectors to about twice icon size
  //  This makes the text render with ellipses if too large

  //  So, Measure and reduce the Font size on ListBook(ListView) selectors
  //  to allow text layout without ellipsis...
  wxBitmap tbmp = g_StyleManager->GetCurrentStyle()->GetIcon(_T("Display"));
  wxScreenDC sdc;
  int text_width = tbmp.GetWidth();
  if (sdc.IsOk())
    sdc.GetTextExtent(_("Connections"), &text_width, NULL, NULL, NULL,
                      dialogFont);

  if (text_width > tbmp.GetWidth() * 2) {
    wxListView* lv = m_pListbook->GetListView();
    wxFont* qFont = dialogFont;  // to get type, weight, etc...

    wxFont* sFont = FontMgr::Get().FindOrCreateFont(
        10, qFont->GetFamily(), qFont->GetStyle(), qFont->GetWeight());
    lv->SetFont(*sFont);
  }
#endif

  CreateListbookIcons();

  m_pListbook->SetImageList(m_topImgList);
  itemBoxSizer2->Add(m_pListbook, 1, wxALL | wxEXPAND, border_size);

  wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(buttons, 0, wxALIGN_RIGHT | wxALL, border_size);

  m_OKButton = new wxButton(itemDialog1, xID_OK, _("OK"));
  m_OKButton->SetDefault();
  buttons->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size);

  m_CancelButton = new wxButton(itemDialog1, wxID_CANCEL, _("Cancel"));
  buttons->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size);

  m_ApplyButton = new wxButton(itemDialog1, ID_APPLY, _("Apply"));
  buttons->Add(m_ApplyButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size);

  m_pageDisplay = CreatePanel(_("Display"));
  CreatePanel_Display(m_pageDisplay, border_size, group_item_spacing);
  CreatePanel_Units(m_pageDisplay, border_size, group_item_spacing);
  CreatePanel_Advanced(m_pageDisplay, border_size, group_item_spacing);
  CreatePanel_Configs(m_pageDisplay, border_size, group_item_spacing);

  m_pageCharts = CreatePanel(_("Charts"));
  CreatePanel_ChartsLoad(m_pageCharts, border_size, group_item_spacing);
  CreatePanel_VectorCharts(m_pageCharts, border_size, group_item_spacing);
  // ChartGroups must be created after ChartsLoad and must be at least third
  CreatePanel_ChartGroups(m_pageCharts, border_size, group_item_spacing);
  RecalculateSize();
  CreatePanel_TidesCurrents(m_pageCharts, border_size, group_item_spacing);

  wxNotebook* nb =
      dynamic_cast<wxNotebook*>(m_pListbook->GetPage(m_pageCharts));
  if (nb) {
#ifdef __OCPN__OPTIONS_USE_LISTBOOK__
    nb->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                wxListbookEventHandler(options::OnChartsPageChange), NULL,
                this);
#else
    nb->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
                wxNotebookEventHandler(options::OnChartsPageChange), NULL,
                this);

#endif
  }

  m_pageConnections = CreatePanel(_("Connections"));
#ifndef __OCPN__ANDROID__
  CreatePanel_NMEA(m_pageConnections, border_size, group_item_spacing);
#else
  CreatePanel_NMEA(m_pageConnections, border_size, group_item_spacing);
// CreatePanel_NMEA_Compact(m_pageConnections, border_size, group_item_spacing);
#endif

  //    SetDefaultConnectionParams();

  m_pageShips = CreatePanel(_("Ships"));
  CreatePanel_Ownship(m_pageShips, border_size, group_item_spacing);
  CreatePanel_AIS(m_pageShips, border_size, group_item_spacing);
#ifndef __OCPN__ANDROID__
  CreatePanel_MMSI(m_pageShips, border_size, group_item_spacing);
#endif

  CreatePanel_Routes(m_pageShips, border_size, group_item_spacing);

  m_pageUI = CreatePanel(_("User Interface"));
  CreatePanel_UI(m_pageUI, border_size, group_item_spacing);
  CreatePanel_Sounds(m_pageUI, border_size, group_item_spacing);

  m_pagePlugins = CreatePanel(_("Plugins"));
  itemPanelPlugins = AddPage(m_pagePlugins, _("Plugins"));

  itemBoxSizerPanelPlugins = new wxBoxSizer(wxVERTICAL);
  itemPanelPlugins->SetSizer(itemBoxSizerPanelPlugins);

  //      PlugIns can add panels, too
  if (g_pi_manager) g_pi_manager->NotifySetupOptions();

  SetColorScheme(static_cast<ColorScheme>(0));

  // Set the maximum size of the entire settings dialog
  // leaving a slightly larger border for larger displays.
  int marginx = 10;
  int marginy = 40;
  if (width > 800) {
    marginx = 100;
    marginy = 100;
  }

  SetSizeHints(-1, -1, width - marginx, height - marginy);

  //  The s57 chart panel is the one which controls the minimum width required
  //  to avoid horizontal scroll bars
  vectorPanel->SetSizeHints(ps57Ctl);
}

void options::SetInitialPage(int page_sel, int sub_page) {
  if (page_sel < (int)m_pListbook->GetPageCount())
    m_pListbook->SetSelection(page_sel);
  else
    m_pListbook->SetSelection(0);

  if (sub_page >= 0) {
    for (size_t i = 0; i < m_pListbook->GetPageCount(); i++) {
      wxNotebookPage* pg = m_pListbook->GetPage(i);
      wxNotebook* nb = dynamic_cast<wxNotebook*>(pg);
      if (nb) {
        if (i == (size_t)page_sel) {
          if (sub_page < (int)nb->GetPageCount())
            nb->SetSelection(sub_page);
          else
            nb->SetSelection(0);
        } else
          nb->ChangeSelection(0);
      }
    }
  }
}

void options::SetColorScheme(ColorScheme cs) {
  DimeControl(this);

#ifdef __OCPN__OPTIONS_USE_LISTBOOK__
  wxListView* lv = m_pListbook->GetListView();
  lv->SetBackgroundColour(GetBackgroundColour());

  if (m_cs != cs) {
    delete m_topImgList;
    CreateListbookIcons();
    m_pListbook->SetImageList(m_topImgList);

    m_cs = cs;
  }

#endif
}

void options::OnAISRolloverClick(wxCommandEvent& event) {
  m_pCheck_Rollover_Class->Enable(event.IsChecked());
  m_pCheck_Rollover_COG->Enable(event.IsChecked());
  m_pCheck_Rollover_CPA->Enable(event.IsChecked());
  pStatic_CallSign->Enable(event.IsChecked());
}

void options::OnCanvasConfigSelectClick(int ID, bool selected) {
  switch (ID) {
    case ID_SCREENCONFIG1:
      if (m_sconfigSelect_twovertical)
        m_sconfigSelect_twovertical->SetSelected(false);
      m_screenConfig = 0;
      break;

    case ID_SCREENCONFIG2:
      if (m_sconfigSelect_single) m_sconfigSelect_single->SetSelected(false);
      m_screenConfig = 1;
      break;
  }
}

void options::SetInitialSettings(void) {
  wxString s;

  m_returnChanges = 0;  // reset the flags
  m_bfontChanged = false;

  b_oldhaveWMM = b_haveWMM;
  auto loader = PluginLoader::getInstance();
  b_haveWMM = loader && loader->IsPlugInAvailable(_T("WMM"));

  // Canvas configuration
  switch (g_canvasConfig) {
    case 0:
    default:
      if (m_sconfigSelect_single) m_sconfigSelect_single->SetSelected(true);
      if (m_sconfigSelect_twovertical)
        m_sconfigSelect_twovertical->SetSelected(false);
      break;
    case 1:
      if (m_sconfigSelect_single) m_sconfigSelect_single->SetSelected(false);
      if (m_sconfigSelect_twovertical)
        m_sconfigSelect_twovertical->SetSelected(true);
      break;
  }
  m_screenConfig = g_canvasConfig;

  // Initial Charts Load

  ActiveChartArray.Clear();
  for (size_t i = 0; i < m_CurrentDirList.GetCount(); i++) {
    ActiveChartArray.Add(m_CurrentDirList[i]);
  }

  // ChartGroups
  if (m_pWorkDirList) {
    UpdateWorkArrayFromDisplayPanel();
    groupsPanel->SetDBDirs(*m_pWorkDirList);

    // Make a deep copy of the current global Group Array
    groupsPanel->EmptyChartGroupArray(m_pGroupArray);
    delete m_pGroupArray;
    m_pGroupArray = groupsPanel->CloneChartGroupArray(g_pGroupArray);
    groupsPanel->SetGroupArray(m_pGroupArray);
    groupsPanel->SetInitialSettings();
  }

  if (m_pConfig) {
    pShowStatusBar->SetValue(g_bShowStatusBar);
#ifndef __WXOSX__
    pShowMenuBar->SetValue(g_bShowMenuBar);
#endif
    pShowCompassWin->SetValue(g_bShowCompassWin);
  }

  s.Printf(_T("%d"), g_COGAvgSec);
  pCOGUPUpdateSecs->SetValue(s);

  if (pCDOOutlines) pCDOOutlines->SetValue(g_bShowOutlines);
  if (pCDOQuilting) pCDOQuilting->SetValue(g_bQuiltEnable);
  //  if(pFullScreenQuilt) pFullScreenQuilt->SetValue(!g_bFullScreenQuilt);
  if (pSDepthUnits) pSDepthUnits->SetValue(g_bShowDepthUnits);
  if (pSkewComp) pSkewComp->SetValue(g_bskew_comp);
  pMobile->SetValue(g_btouch);
  pResponsive->SetValue(g_bresponsive);
  pRollover->SetValue(g_bRollover);
  m_pCheck_Rollover_Class->Enable(g_bRollover);
  m_pCheck_Rollover_COG->Enable(g_bRollover);
  m_pCheck_Rollover_CPA->Enable(g_bRollover);
  pStatic_CallSign->Enable(g_bRollover);

  pZoomButtons->SetValue(g_bShowMuiZoomButtons);

  // pOverzoomEmphasis->SetValue(!g_fog_overzoom);
  // pOZScaleVector->SetValue(!g_oz_vector_scale);
  pInlandEcdis->SetValue(g_bInlandEcdis);
  pOpenGL->SetValue(g_bopengl);
  if (pSmoothPanZoom) pSmoothPanZoom->SetValue(g_bsmoothpanzoom);
  pCBTrueShow->SetValue(g_bShowTrue);
  pCBMagShow->SetValue(g_bShowMag);

  int oldLength = itemStaticTextUserVar->GetLabel().Length();

  // disable input for variation if WMM is available
  if (b_haveWMM) {
    itemStaticTextUserVar->SetLabel(
        _("WMM Plugin calculated magnetic variation"));
    wxString s;
    s.Printf(_T("%4.1f"), gVar);
    pMagVar->SetValue(s);
  } else {
    itemStaticTextUserVar->SetLabel(_("User set magnetic variation"));
    wxString s;
    s.Printf(_T("%4.1f"), g_UserVar);
    pMagVar->SetValue(s);
  }

  int newLength = itemStaticTextUserVar->GetLabel().Length();

  // size hack to adjust change in static text size
  if ((newLength != oldLength) || (b_oldhaveWMM != b_haveWMM)) {
    wxSize sz = GetSize();
    SetSize(sz.x + 1, sz.y);
    SetSize(sz);
  }

  itemStaticTextUserVar2->Enable(!b_haveWMM);
  pMagVar->Enable(!b_haveWMM);

  if (pSDisplayGrid) pSDisplayGrid->SetValue(g_bDisplayGrid);

  // LIVE ETA OPTION

  // Checkbox
  if (pSLiveETA) pSLiveETA->SetValue(g_bShowLiveETA);

  // Defaut boat speed text input field
  // Speed always in knots, and converted to user speed unit
  wxString stringDefaultBoatSpeed;
  if (!g_defaultBoatSpeed || !g_defaultBoatSpeedUserUnit) {
    g_defaultBoatSpeed = 6.0;
    g_defaultBoatSpeedUserUnit = toUsrSpeed(g_defaultBoatSpeed, -1);
  }
  stringDefaultBoatSpeed.Printf(_T("%d"), (int)g_defaultBoatSpeedUserUnit);
  if (pSDefaultBoatSpeed) pSDefaultBoatSpeed->SetValue(stringDefaultBoatSpeed);

  // END LIVE ETA OPTION

  if (pCBCourseUp) pCBCourseUp->SetValue(g_bCourseUp);
  if (pCBNorthUp) pCBNorthUp->SetValue(!g_bCourseUp);
  if (pCBLookAhead) pCBLookAhead->SetValue(g_bLookAhead);

  if (fabs(wxRound(g_ownship_predictor_minutes) - g_ownship_predictor_minutes) >
      1e-4)
    s.Printf(_T("%6.2f"), g_ownship_predictor_minutes);
  else
    s.Printf(_T("%4.0f"), g_ownship_predictor_minutes);
  m_pText_OSCOG_Predictor->SetValue(s);

  if (fabs(wxRound(g_ownship_HDTpredictor_miles) -
           g_ownship_HDTpredictor_miles) > 1e-4)
    s.Printf(_T("%6.2f"), g_ownship_HDTpredictor_miles);
  else
    s.Printf(_T("%4.0f"), g_ownship_HDTpredictor_miles);
  m_pText_OSHDT_Predictor->SetValue(s);

  m_pShipIconType->SetSelection(g_OwnShipIconType);
  wxCommandEvent eDummy;
  OnShipTypeSelect(eDummy);
  m_pOSLength->SetValue(
      wxString::Format(_T("%.1f"), g_n_ownship_length_meters));
  m_pOSWidth->SetValue(wxString::Format(_T("%.1f"), g_n_ownship_beam_meters));
  m_pOSGPSOffsetX->SetValue(
      wxString::Format(_T("%.1f"), g_n_gps_antenna_offset_x));
  m_pOSGPSOffsetY->SetValue(
      wxString::Format(_T("%.1f"), g_n_gps_antenna_offset_y));
  m_pOSMinSize->SetValue(wxString::Format(_T("%d"), g_n_ownship_min_mm));
  m_pText_ACRadius->SetValue(
      wxString::Format(_T("%.3f"), g_n_arrival_circle_radius));

  wxString buf;
  if (g_iNavAidRadarRingsNumberVisible > 10)
    g_iNavAidRadarRingsNumberVisible = 10;
  pNavAidRadarRingsNumberVisible->SetSelection(
      g_iNavAidRadarRingsNumberVisible);
  buf.Printf(_T("%.3f"), g_fNavAidRadarRingsStep);
  pNavAidRadarRingsStep->SetValue(buf);
  m_itemRadarRingsUnits->SetSelection(g_pNavAidRadarRingsStepUnits);
  m_colourOwnshipRangeRingColour->SetColour(g_colourOwnshipRangeRingsColour);

  pScaMinChckB->SetValue(g_bUseWptScaMin);
  m_pText_ScaMin->SetValue(wxString::Format(_T("%i"), g_iWpt_ScaMin));
  pScaMinOverruleChckB->SetValue(g_bOverruleScaMin);

  OnRadarringSelect(eDummy);

  if (g_iWaypointRangeRingsNumber > 10) g_iWaypointRangeRingsNumber = 10;
  pWaypointRangeRingsNumber->SetSelection(g_iWaypointRangeRingsNumber);
  buf.Printf(_T("%.3f"), g_fWaypointRangeRingsStep);
  pWaypointRangeRingsStep->SetValue(buf);
  m_itemWaypointRangeRingsUnits->SetSelection(g_iWaypointRangeRingsStepUnits);
  m_colourWaypointRangeRingsColour->SetColour(g_colourWaypointRangeRingsColour);
  OnWaypointRangeRingSelect(eDummy);
  pShowshipToActive->SetValue(g_bShowShipToActive);
  m_shipToActiveStyle->SetSelection(g_shipToActiveStyle);
  m_shipToActiveColor->SetSelection(g_shipToActiveColor);

  pWayPointPreventDragging->SetValue(g_bWayPointPreventDragging);
  pConfirmObjectDeletion->SetValue(g_bConfirmObjectDelete);

  pSogCogFromLLCheckBox->SetValue(g_own_ship_sog_cog_calc);
  pSogCogFromLLDampInterval->SetValue(g_own_ship_sog_cog_calc_damp_sec);

  if (pEnableZoomToCursor) pEnableZoomToCursor->SetValue(g_bEnableZoomToCursor);

  if (pPreserveScale) pPreserveScale->SetValue(g_bPreserveScaleOnX);
  pPlayShipsBells->SetValue(g_bPlayShipsBells);

  if (g_bUIexpert && pCmdSoundString)
    pCmdSoundString->SetValue(g_CmdSoundString);

  if (pSoundDeviceIndex) pSoundDeviceIndex->SetSelection(g_iSoundDeviceIndex);
  //    pFullScreenToolbar->SetValue( g_bFullscreenToolbar );
  // pTransparentToolbar->SetValue(g_bTransparentToolbar);
  pSDMMFormat->Select(g_iSDMMFormat);
  pDistanceFormat->Select(g_iDistanceFormat);
  pSpeedFormat->Select(g_iSpeedFormat);
  pTempFormat->Select(g_iTempFormat);

  pAdvanceRouteWaypointOnArrivalOnly->SetValue(
      g_bAdvanceRouteWaypointOnArrivalOnly);

  pTrackDaily->SetValue(g_bTrackDaily);
  pTrackRotateLMT->SetValue(g_track_rotate_time_type == TIME_TYPE_LMT);
  pTrackRotateUTC->SetValue(g_track_rotate_time_type == TIME_TYPE_UTC);
  pTrackRotateComputerTime->SetValue(g_track_rotate_time_type ==
                                     TIME_TYPE_COMPUTER);
  pTrackHighlite->SetValue(g_bHighliteTracks);
  m_colourTrackLineColour->SetColour(g_colourTrackLineColour);
  pTrackPrecision->SetSelection(g_nTrackPrecision);

  m_soundPanelAnchor->GetCheckBox()->SetValue(g_bAnchor_Alert_Audio);

  //    AIS Parameters
  //      CPA Box
  m_pCheck_CPA_Max->SetValue(g_bCPAMax);

  s.Printf(_T("%4.1f"), g_CPAMax_NM);
  m_pText_CPA_Max->SetValue(s);

  m_pCheck_CPA_Warn->SetValue(g_bCPAWarn);

  s.Printf(_T("%4.1f"), g_CPAWarn_NM);
  m_pText_CPA_Warn->SetValue(s);

  if (m_pCheck_CPA_Warn->GetValue()) {
    m_pCheck_CPA_WarnT->Enable();
    m_pCheck_CPA_WarnT->SetValue(g_bTCPA_Max);
  } else
    m_pCheck_CPA_WarnT->Disable();

  s.Printf(_T("%4.0f"), g_TCPA_Max);
  m_pText_CPA_WarnT->SetValue(s);

  //      Lost Targets
  m_pCheck_Mark_Lost->SetValue(g_bMarkLost);

  s.Printf(_T("%4.0f"), g_MarkLost_Mins);
  m_pText_Mark_Lost->SetValue(s);

  m_pCheck_Remove_Lost->SetValue(g_bRemoveLost);

  s.Printf(_T("%4.0f"), g_RemoveLost_Mins);
  m_pText_Remove_Lost->SetValue(s);

  //      Display
  m_pCheck_Show_COG->SetValue(g_bShowCOG);

  s.Printf(_T("%4.0f"), g_ShowCOG_Mins);
  m_pText_COG_Predictor->SetValue(s);

  m_pCheck_Sync_OCOG_ACOG->SetValue(g_bSyncCogPredictors);
  if(g_bSyncCogPredictors) m_pText_COG_Predictor->Disable();

  m_pCheck_Show_Tracks->SetValue(g_bAISShowTracks);

  s.Printf(_T("%4.0f"), g_AISShowTracks_Mins);
  m_pText_Track_Length->SetValue(s);

  m_pCheck_Hide_Moored->SetValue(g_bHideMoored);

  s.Printf(_T("%4.1f"), g_ShowMoored_Kts);
  m_pText_Moored_Speed->SetValue(s);

  m_pCheck_Draw_Realtime_Prediction->SetValue(g_bDrawAISRealtime);

  s.Printf(_T("%4.1f"), g_AIS_RealtPred_Kts);
  m_pText_RealtPred_Speed->SetValue(s);

  m_pCheck_Scale_Priority->SetValue(g_bAllowShowScaled);

  s.Printf(_T("%i"), g_ShowScaled_Num);
  m_pText_Scale_Priority->SetValue(s);

  m_pCheck_Show_Area_Notices->SetValue(g_bShowAreaNotices);

  m_pCheck_Draw_Target_Size->SetValue(g_bDrawAISSize);
  m_pCheck_Draw_Realtime_Prediction->SetValue(g_bDrawAISRealtime);

  m_pCheck_Show_Target_Name->SetValue(g_bShowAISName);

  s.Printf(_T("%d"), g_Show_Target_Name_Scale);
  m_pText_Show_Target_Name_Scale->SetValue(s);

  m_pCheck_use_Wpl->SetValue(g_bWplUsePosition);
  m_pWplAction->SetSelection(g_WplAction);

  // Alerts
  m_pCheck_AlertDialog->SetValue(g_bAIS_CPA_Alert);
  if (m_pCheck_AlertDialog->GetValue()) {
    m_pCheck_AlertAudio->Enable();
    m_pCheck_AlertAudio->SetValue(g_bAIS_CPA_Alert_Audio);
  } else {
    m_pCheck_AlertAudio->Disable();
    m_pCheck_AlertAudio->SetValue(false);
  }

  m_pCheck_Alert_Moored->SetValue(g_bAIS_CPA_Alert_Suppress_Moored);

  m_pCheck_Ack_Timout->SetValue(g_bAIS_ACK_Timeout);
  s.Printf(_T("%4.0f"), g_AckTimeout_Mins);
  m_pText_ACK_Timeout->SetValue(s);

  // Sounds
  if (m_pCheck_AlertDialog
          ->GetValue()) {  // AIS Alert sound only if Alert dialog is enabled

    m_soundPanelAIS->GetCheckBox()->Enable(m_pCheck_AlertAudio->GetValue());
    m_soundPanelAIS->GetCheckBox()->SetValue(g_bAIS_GCPA_Alert_Audio);
    m_soundPanelSART->GetCheckBox()->Enable(m_pCheck_AlertAudio->GetValue());
    m_soundPanelSART->GetCheckBox()->SetValue(g_bAIS_SART_Alert_Audio);
    m_soundPanelDSC->GetCheckBox()->Enable(m_pCheck_AlertAudio->GetValue());
    m_soundPanelDSC->GetCheckBox()->SetValue(g_bAIS_DSC_Alert_Audio);
  } else {
    m_soundPanelAIS->GetCheckBox()->Disable();
    m_soundPanelAIS->GetCheckBox()->SetValue(false);
    m_soundPanelSART->GetCheckBox()->Disable();
    m_soundPanelSART->GetCheckBox()->SetValue(false);
    m_soundPanelDSC->GetCheckBox()->Disable();
    m_soundPanelDSC->GetCheckBox()->SetValue(false);
  }

  // Rollover
  m_pCheck_Rollover_Class->SetValue(g_bAISRolloverShowClass);
  m_pCheck_Rollover_COG->SetValue(g_bAISRolloverShowCOG);
  m_pCheck_Rollover_CPA->SetValue(g_bAISRolloverShowCPA);

  m_pSlider_Zoom->SetValue(g_chart_zoom_modifier);
  m_pSlider_Zoom_Vector->SetValue(g_chart_zoom_modifier_vector);

  m_pSlider_GUI_Factor->SetValue(g_GUIScaleFactor);
  m_pSlider_Chart_Factor->SetValue(g_ChartScaleFactor);
  m_pSlider_Ship_Factor->SetValue(g_ShipScaleFactor);
  m_pSlider_Text_Factor->SetValue(g_ENCSoundingScaleFactor);
  m_pMouse_Zoom_Slider->SetValue(g_mouse_zoom_sensitivity_ui);
  wxString screenmm;

  if (!g_config_display_size_manual) {
    pRBSizeAuto->SetValue(TRUE);
    screenmm.Printf(_T("%d"), int(g_Platform->GetDisplaySizeMM()));
    pScreenMM->Disable();
  } else {
    screenmm.Printf(_T("%d"), int(g_config_display_size_mm));
    pRBSizeManual->SetValue(TRUE);
  }

  pScreenMM->SetValue(screenmm);

  m_TalkerIdText->SetValue(g_TalkerIdText.MakeUpper());

  pDepthUnitSelect->SetSelection(g_nDepthUnitDisplay);
  UpdateOptionsUnits();  // sets depth values using the user's unit preference

  SetInitialVectorSettings();

  pToolbarAutoHideCB->SetValue(g_bAutoHideToolbar);

  s.Printf(_T("%d"), g_nAutoHideToolbar);
  pToolbarHideSecs->SetValue(s);

  m_cbNMEADebug->SetValue(false);
  if (NMEALogWindow::Get().GetTTYWindow()) {
    if (NMEALogWindow::Get().GetTTYWindow()->IsShown()) {
      m_cbNMEADebug->SetValue(true);
    }
  }

  //  Serial ports

  delete m_pSerialArray;
  m_pSerialArray = NULL;
  m_pSerialArray = EnumerateSerialPorts();

  if (m_pSerialArray) {
    m_comboPort->Clear();
    for (size_t i = 0; i < m_pSerialArray->Count(); i++) {
      m_comboPort->Append(m_pSerialArray->Item(i));
    }
  }

  //  On some platforms, the global connections list may be changed outside of
  //  the options dialog. Pick up any changes here, and re-populate the dialog
  //  list.
  FillSourceList();

  //  Reset the touch flag...
  connectionsaved = true;

  SetSelectedConnectionPanel(nullptr);

  m_bForceNewToolbaronCancel = false;
}

void options::resetMarStdList(bool bsetConfig, bool bsetStd) {
  if (ps57CtlListBox) {
    //    S52 Primary Filters
    ps57CtlListBox->Clear();
    marinersStdXref.clear();

    for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount();
         iPtr++) {
      OBJLElement* pOLE = (OBJLElement*)(ps52plib->pOBJLArray->Item(iPtr));

      wxString item;
      if (iPtr < ps52plib->OBJLDescriptions.size()) {
        item = ps52plib->OBJLDescriptions[iPtr];
      } else {
        item = wxString(pOLE->OBJLName, wxConvUTF8);
      }

      // Find the most conservative Category, among Point, Area, and Line LUPs
      DisCat cat = OTHER;

      DisCat catp = ps52plib->findLUPDisCat(pOLE->OBJLName, SIMPLIFIED);
      DisCat cata = ps52plib->findLUPDisCat(pOLE->OBJLName, PLAIN_BOUNDARIES);
      DisCat catl = ps52plib->findLUPDisCat(pOLE->OBJLName, LINES);

      if ((catp == DISPLAYBASE) || (cata == DISPLAYBASE) ||
          (catl == DISPLAYBASE))
        cat = DISPLAYBASE;
      else if ((catp == STANDARD) || (cata == STANDARD) || (catl == STANDARD))
        cat = STANDARD;

      bool benable = true;
      if (cat > 0) benable = cat != DISPLAYBASE;

      // The ListBox control will insert entries in sorted order, which means
      // we need to
      // keep track of already inserted items that gets pushed down the line.
      int newpos = ps57CtlListBox->Append(item, benable);
      marinersStdXref.push_back(newpos);
      for (size_t i = 0; i < iPtr; i++) {
        if (marinersStdXref[i] >= newpos) marinersStdXref[i]++;
      }

      bool bviz = 0;
      if (bsetConfig) bviz = !(pOLE->nViz == 0);

      if (cat == DISPLAYBASE) bviz = true;

      if (bsetStd) {
        if (cat == STANDARD) {
          bviz = true;
        }
      }

      ps57CtlListBox->Check(newpos, bviz);
    }
    //  Force the wxScrolledWindow to recalculate its scroll bars
    wxSize s = ps57CtlListBox->GetSize();
    ps57CtlListBox->SetSize(s.x, s.y - 1);
  }
}

void options::SetInitialVectorSettings(void) {
  m_pSlider_CM93_Zoom->SetValue(g_cm93_zoom_factor);

  //    Diplay Category
  if (ps52plib) {
    m_bVectorInit = true;
    resetMarStdList(true, false);

#ifdef __OCPN__ANDROID__
    ps57CtlListBox->GetHandle()->setStyleSheet(getAdjustedDialogStyleSheet());
#endif

    int nset = 2;  // default OTHER
    switch (ps52plib->GetDisplayCategory()) {
      case (DISPLAYBASE):
        nset = 0;
        break;
      case (STANDARD):
        nset = 1;
        break;
      case (OTHER):
        nset = 2;
        break;
      case (MARINERS_STANDARD):
        nset = 3;
        break;
      default:
        nset = 3;
        break;
    }

    if (pDispCat) pDispCat->SetSelection(nset);

    // Enable the UserStandard object list if either canvas is in
    // MARINERS_STANDARD display category
    bool benableMarStd = false;
    // .. for each canvas...
    for (unsigned int i = 0; i < g_canvasArray.GetCount(); i++) {
      ChartCanvas* cc = g_canvasArray.Item(i);
      if (cc) {
        if (cc->GetENCDisplayCategory() == MARINERS_STANDARD)
          benableMarStd = true;
      }
    }

    //        if(g_useMUI)
    //           benableMarStd = true;

    if (ps57CtlListBox) ps57CtlListBox->Enable(benableMarStd);
    itemButtonClearList->Enable(benableMarStd);
    itemButtonSelectList->Enable(benableMarStd);
    itemButtonSetStd->Enable(benableMarStd);

    //  Other Display Filters
    if (pCheck_SOUNDG) pCheck_SOUNDG->SetValue(ps52plib->m_bShowSoundg);
    if (pCheck_ATONTEXT) pCheck_ATONTEXT->SetValue(ps52plib->m_bShowAtonText);
    if (pCheck_LDISTEXT) pCheck_LDISTEXT->SetValue(ps52plib->m_bShowLdisText);
    if (pCheck_XLSECTTEXT)
      pCheck_XLSECTTEXT->SetValue(ps52plib->m_bExtendLightSectors);

    pCheck_META->SetValue(ps52plib->m_bShowMeta);
    pCheck_SHOWIMPTEXT->SetValue(ps52plib->m_bShowS57ImportantTextOnly);
    pCheck_SCAMIN->SetValue(ps52plib->m_bUseSCAMIN);
    pCheck_DECLTEXT->SetValue(ps52plib->m_bDeClutterText);
    pCheck_NATIONALTEXT->SetValue(ps52plib->m_bShowNationalTexts);

    // Chart Display Style
    if (ps52plib->m_nSymbolStyle == PAPER_CHART)
      pPointStyle->SetSelection(0);
    else
      pPointStyle->SetSelection(1);

    if (ps52plib->m_nBoundaryStyle == PLAIN_BOUNDARIES)
      pBoundStyle->SetSelection(0);
    else
      pBoundStyle->SetSelection(1);

    if (S52_getMarinerParam(S52_MAR_TWO_SHADES) == 1.0)
      p24Color->SetSelection(0);
    else
      p24Color->SetSelection(1);

    UpdateOptionsUnits();  // sets depth values using the user's unit preference
  }
}

void options::UpdateOptionsUnits(void) {
  int depthUnit = pDepthUnitSelect->GetSelection();

  depthUnit = wxMax(depthUnit, 0);
  depthUnit = wxMin(depthUnit, 2);

  // depth unit conversion factor
  float conv = 1;
  if (depthUnit == 0)  // feet
    conv = 0.3048f;    // international definiton of 1 foot is 0.3048 metres
  else if (depthUnit == 2)  // fathoms
    conv = 0.3048f * 6;     // 1 fathom is 6 feet

  // set depth input values

  // set depth unit labels
  wxString depthUnitStrings[] = {_("feet"), _("meters"), _("fathoms")};
  wxString depthUnitString = depthUnitStrings[depthUnit];
  m_depthUnitsShal->SetLabel(depthUnitString);
  m_depthUnitsSafe->SetLabel(depthUnitString);
  m_depthUnitsDeep->SetLabel(depthUnitString);

  wxString s;
  s.Printf(_T( "%6.2f" ), S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR) / conv);
  s.Trim(FALSE);
  m_ShallowCtl->SetValue(s);

  s.Printf(_T( "%6.2f" ), S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR) / conv);
  s.Trim(FALSE);
  m_SafetyCtl->SetValue(s);

  s.Printf(_T( "%6.2f" ), S52_getMarinerParam(S52_MAR_DEEP_CONTOUR) / conv);
  s.Trim(FALSE);
  m_DeepCtl->SetValue(s);
  /*
    int oldLength = itemStaticTextUserVar->GetLabel().Length();

    //disable input for variation if WMM is available
    if(b_haveWMM){
        itemStaticTextUserVar->SetLabel(_("WMM Plugin calculated magnetic
    variation")); wxString s; s.Printf(_T("%4.1f"), gVar); pMagVar->SetValue(s);
    }
    else{
        itemStaticTextUserVar->SetLabel(_("User set magnetic variation"));
        wxString s;
        s.Printf(_T("%4.1f"), g_UserVar);
        pMagVar->SetValue(s);
    }

    int newLength = itemStaticTextUserVar->GetLabel().Length();

    // size hack to adjust change in static text size
    if( (newLength != oldLength) || (b_oldhaveWMM != b_haveWMM) ){
      wxSize sz = GetSize();
      SetSize(sz.x+1, sz.y);
      SetSize(sz);
    }

    itemStaticTextUserVar2->Enable(!b_haveWMM);
    pMagVar->Enable(!b_haveWMM);
  */
}

void options::OnSizeAutoButton(wxCommandEvent& event) {
  wxString screenmm = wxString::Format(
      _T( "%d" ), static_cast<int>(g_Platform->GetDisplaySizeMM()));
  pScreenMM->SetValue(screenmm);
  pScreenMM->Disable();
  g_config_display_size_manual = FALSE;
}

void options::OnSizeManualButton(wxCommandEvent& event) {
  wxString screenmm = wxString::Format(
      _T( "%d" ), static_cast<int>(g_config_display_size_mm > 0
                                       ? g_config_display_size_mm
                                       : g_Platform->GetDisplaySizeMM()));
  pScreenMM->SetValue(screenmm);
  pScreenMM->Enable();
  g_config_display_size_manual = TRUE;
}

void options::OnUnitsChoice(wxCommandEvent& event) { UpdateOptionsUnits(); }

void options::OnCPAWarnClick(wxCommandEvent& event) {
  if (m_pCheck_CPA_Warn->GetValue()) {
    m_pCheck_CPA_WarnT->Enable();
  } else {
    m_pCheck_CPA_WarnT->SetValue(FALSE);
    m_pCheck_CPA_WarnT->Disable();
  }
}

void options::OnSyncCogPredClick(wxCommandEvent &event) {
  if (m_pCheck_Sync_OCOG_ACOG->GetValue()) {
    m_pText_COG_Predictor->SetValue(m_pText_OSCOG_Predictor->GetValue());
    m_pText_COG_Predictor->Disable();
  }
  else {
    wxString s;
    s.Printf(_T("%4.0f"), g_ShowCOG_Mins);
    m_pText_COG_Predictor->SetValue(s);
    m_pText_COG_Predictor->Enable();
  }
}

void options::OnShowGpsWindowCheckboxClick(wxCommandEvent& event) {
  if (!m_cbNMEADebug->GetValue()) {
    NMEALogWindow::Get().DestroyWindow();
  } else {
    NMEALogWindow::Get().Create(pParent, 35);

    // Try to ensure that the log window is a least a little bit visible
    wxRect logRect(
        NMEALogWindow::Get().GetPosX(), NMEALogWindow::Get().GetPosY(),
        NMEALogWindow::Get().GetSizeW(), NMEALogWindow::Get().GetSizeH());

    if (GetRect().Contains(logRect)) {
      NMEALogWindow::Get().SetPos(
          GetRect().x / 2,
          (GetRect().y + (GetRect().height - logRect.height) / 2));
      NMEALogWindow::Get().Move();
    }

    Raise();
  }
}

void options::OnShipTypeSelect(wxCommandEvent& event) {
  realSizes->ShowItems(m_pShipIconType->GetSelection() != 0);
  dispOptions->Layout();
  ownShip->Layout();
  itemPanelShip->Layout();
  itemPanelShip->Refresh();
  event.Skip();
}

void options::OnRadarringSelect(wxCommandEvent& event) {
  radarGrid->ShowItems(pNavAidRadarRingsNumberVisible->GetSelection() != 0);
  dispOptions->Layout();
  ownShip->Layout();
  itemPanelShip->Layout();
  itemPanelShip->Refresh();
  event.Skip();
}

void options::OnWaypointRangeRingSelect(wxCommandEvent& event) {
  waypointradarGrid->ShowItems(pWaypointRangeRingsNumber->GetSelection() != 0);
  dispOptions->Layout();
  Routes->Layout();
  itemPanelRoutes->Layout();
  itemPanelRoutes->Refresh();
  event.Skip();
}

void options::OnGLClicked(wxCommandEvent& event) {
  //   if (!g_bTransparentToolbarInOpenGLOK)
  //     pTransparentToolbar->Enable(!pOpenGL->GetValue());
}

void options::OnOpenGLOptions(wxCommandEvent& event) {
#ifdef ocpnUSE_GL
  OpenGLOptionsDlg dlg(this);

  if (dlg.ShowModal() == wxID_OK) {
    if (gFrame->GetPrimaryCanvas()->GetglCanvas()) {
      g_GLOptions.m_bUseAcceleratedPanning = g_bGLexpert
                                                 ? dlg.GetAcceleratedPanning()
                                                 : gFrame->GetPrimaryCanvas()
                                                       ->GetglCanvas()
                                                       ->CanAcceleratePanning();
    }

    g_bShowFPS = dlg.GetShowFPS();
    g_bSoftwareGL = dlg.GetSoftwareGL();

    g_GLOptions.m_GLPolygonSmoothing = dlg.GetPolygonSmoothing();
    g_GLOptions.m_GLLineSmoothing = dlg.GetLineSmoothing();

    if (g_bGLexpert) {
      // user defined
      g_GLOptions.m_bTextureCompressionCaching =
          dlg.GetTextureCompressionCaching();
      g_GLOptions.m_iTextureMemorySize = dlg.GetTextureMemorySize();
    } else {
      // caching is on if textures are compressed
      g_GLOptions.m_bTextureCompressionCaching = dlg.GetTextureCompression();
    }

    if (g_bopengl && g_glTextureManager &&
        g_GLOptions.m_bTextureCompression != dlg.GetTextureCompression()) {
      // new g_GLoptions setting is needed in callees
      g_GLOptions.m_bTextureCompression = dlg.GetTextureCompression();

      if (gFrame->GetPrimaryCanvas()->GetglCanvas()) {
        ::wxBeginBusyCursor();
        gFrame->GetPrimaryCanvas()->GetglCanvas()->SetupCompression();
        g_glTextureManager->ClearAllRasterTextures();
        ::wxEndBusyCursor();
      }
    } else
      g_GLOptions.m_bTextureCompression = dlg.GetTextureCompression();
  }

  if (dlg.GetRebuildCache()) {
    m_returnChanges = REBUILD_RASTER_CACHE;
    Finish();
  }
#endif
}

void options::OnChartDirListSelect(wxCommandEvent& event) {
#if 0
  bool selected = (pActiveChartsList->GetSelectedItemCount() > 0);
  m_removeBtn->Enable(selected);
  if (m_compressBtn) m_compressBtn->Enable(selected);
#endif
}

void options::OnDisplayCategoryRadioButton(wxCommandEvent& event) {
  if (!g_useMUI) {
    if (pDispCat) {
      const bool select = pDispCat->GetSelection() == 3;
      ps57CtlListBox->Enable(select);
      itemButtonClearList->Enable(select);
      itemButtonSelectList->Enable(select);
      itemButtonSetStd->Enable(select);
    }
  }
  event.Skip();
}

void options::OnButtonClearClick(wxCommandEvent& event) {
  resetMarStdList(false, false);

  //   int nOBJL = ps57CtlListBox->GetCount();
  //   for (int iPtr = 0; iPtr < nOBJL; iPtr++){
  //           ps57CtlListBox->Check(iPtr, FALSE);
  //   }
  event.Skip();
}

void options::OnButtonSelectClick(wxCommandEvent& event) {
  int nOBJL = ps57CtlListBox->GetCount();
  for (int iPtr = 0; iPtr < nOBJL; iPtr++) ps57CtlListBox->Check(iPtr, TRUE);

  event.Skip();
}

void options::OnButtonSetStd(wxCommandEvent& event) {
  resetMarStdList(false, true);

  event.Skip();
}

bool options::ShowToolTips(void) { return TRUE; }

void options::OnCharHook(wxKeyEvent& event) {
  if (event.GetKeyCode() == WXK_RETURN &&
      event.GetModifiers() == wxMOD_CONTROL) {
    wxCommandEvent okEvent;
    okEvent.SetId(xID_OK);
    okEvent.SetEventType(wxEVT_COMMAND_BUTTON_CLICKED);
    GetEventHandler()->AddPendingEvent(okEvent);
  }
  event.Skip();
}

void options::OnButtonaddClick(wxCommandEvent& event) {
  wxString selDir;
  int dresult = g_Platform->DoDirSelectorDialog(
      this, &selDir, _("Add a directory containing chart files"),
      *pInit_Chart_Dir, false);  // no add files allowed

  if (dresult != wxID_CANCEL) AddChartDir(selDir);

  event.Skip();
}

void options::AddChartDir(const wxString& dir) {
  wxFileName dirname = wxFileName(dir);
  pInit_Chart_Dir->Empty();

  wxString dirAdd;
  if (g_bportable) {
    wxFileName f(dir);
    f.MakeRelativeTo(g_Platform->GetHomeDir());
    dirAdd = f.GetFullPath();
  } else {
    pInit_Chart_Dir->Append(dirname.GetPath());
    dirAdd = dir;
  }

  ChartDirInfo cdi;
  cdi.fullpath = dirAdd;
  ActiveChartArray.Add(cdi);

  UpdateChartDirList();

  k_charts |= CHANGE_CHARTS;

  pScanCheckBox->Disable();
}

void options::UpdateDisplayedChartDirList(ArrayOfCDI p) {
  // Called by pluginmanager after adding single chart to database

  ActiveChartArray.Clear();
  for (size_t i = 0; i < p.GetCount(); i++) {
    ActiveChartArray.Add(p[i]);
  }

  UpdateChartDirList();
}

void options::UpdateWorkArrayFromDisplayPanel(void) {
  wxString dirname;
  int n = ActiveChartArray.GetCount();
  if (m_pWorkDirList) {
    m_pWorkDirList->Clear();
    for (int i = 0; i < n; i++) {
      dirname = ActiveChartArray[i].fullpath;
      if (!dirname.IsEmpty()) {
        //    This is a fix for OSX, which appends EOL to results of
        //    GetLineText()
        while ((dirname.Last() == wxChar(_T('\n'))) ||
               (dirname.Last() == wxChar(_T('\r'))))
          dirname.RemoveLast();

        //    scan the current array to find a match
        //    if found, add the info to the work list, preserving the magic
        //    number
        //    If not found, make a new ChartDirInfo, and add it
        bool b_added = FALSE;
        //                        if(m_pCurrentDirList)
        {
          int nDir = m_CurrentDirList.GetCount();

          for (int i = 0; i < nDir; i++) {
            if (m_CurrentDirList[i].fullpath == dirname) {
              ChartDirInfo cdi = m_CurrentDirList[i];
              m_pWorkDirList->Add(cdi);
              b_added = TRUE;
              break;
            }
          }
        }
        if (!b_added) {
          ChartDirInfo cdin;
          cdin.fullpath = dirname;
          m_pWorkDirList->Add(cdin);
        }
      }
    }
  }
}

ConnectionParams* options::CreateConnectionParamsFromSelectedItem(void) {
  if (!m_bNMEAParams_shown) return NULL;

  //  Special encoding for deleted connection
  if (m_rbTypeSerial->GetValue() && m_comboPort->GetValue() == _T("Deleted" ))
    return NULL;

  //  We check some values here for consistency.
  //  If necessary, set defaults so user will see some result, however wrong...

  //  DataStreams should be Input, Output, or Both
  if (!(m_cbInput->GetValue() || m_cbOutput->GetValue())) {
    m_cbInput->SetValue(true);
  }

  if (m_rbTypeSerial->GetValue() && m_comboPort->GetValue() == wxEmptyString) {
    m_comboPort->Select(0);
  }
  //  TCP, GPSD and UDP require port field to be set.
  //  TCP clients, GPSD and UDP output sockets require an address
  else if (m_rbTypeNet->GetValue()) {
    if (wxAtoi(m_tNetPort->GetValue()) == 0) {
      m_tNetPort->SetValue(_T("2947"));  // reset to default
    }
    if (m_tNetAddress->GetValue() == wxEmptyString) {
      m_tNetAddress->SetValue(_T("0.0.0.0"));
    }
  }

  ConnectionParams* pConnectionParams = new ConnectionParams();

  UpdateConnectionParamsFromSelectedItem(pConnectionParams);

  ConnectionParamsPanel* pPanel = new ConnectionParamsPanel(
      m_scrollWinConnections, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      pConnectionParams, this);
  pPanel->SetSelected(false);
  boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxALL, 0);
  pConnectionParams->m_optionsPanel = pPanel;

  return pConnectionParams;
}

ConnectionParams* options::UpdateConnectionParamsFromSelectedItem(
    ConnectionParams* pConnectionParams) {
  pConnectionParams->Valid = TRUE;
  if (m_rbTypeSerial->GetValue())
    pConnectionParams->Type = SERIAL;
  else if (m_rbTypeNet->GetValue())
    pConnectionParams->Type = NETWORK;
  else if (m_rbTypeInternalGPS && m_rbTypeInternalGPS->GetValue())
    pConnectionParams->Type = INTERNAL_GPS;
  else if (m_rbTypeInternalBT && m_rbTypeInternalBT->GetValue())
    pConnectionParams->Type = INTERNAL_BT;

  //  Save the existing addr/port to allow closing of existing port
  pConnectionParams->LastNetworkAddress = pConnectionParams->NetworkAddress;
  pConnectionParams->LastNetworkPort = pConnectionParams->NetworkPort;
  pConnectionParams->LastNetProtocol = pConnectionParams->NetProtocol;

  pConnectionParams->NetworkAddress = m_tNetAddress->GetValue();
  pConnectionParams->NetworkPort = wxAtoi(m_tNetPort->GetValue());
  if (m_rbNetProtoTCP->GetValue())
    pConnectionParams->NetProtocol = TCP;
  else if (m_rbNetProtoUDP->GetValue())
    pConnectionParams->NetProtocol = UDP;
  else if (m_rbNetProtoGPSD->GetValue())
    pConnectionParams->NetProtocol = GPSD;
  else if (m_rbNetProtoSignalK->GetValue())
    pConnectionParams->NetProtocol = SIGNALK;
  else
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;

  pConnectionParams->Baudrate = wxAtoi(m_choiceBaudRate->GetStringSelection());
  pConnectionParams->Protocol = (DataProtocol)m_choiceSerialProtocol->GetSelection();
  pConnectionParams->Priority = wxAtoi(m_choicePriority->GetStringSelection());
  pConnectionParams->ChecksumCheck = m_cbCheckCRC->GetValue();
  pConnectionParams->AutoSKDiscover = m_cbCheckSKDiscover->GetValue();
  pConnectionParams->Garmin = m_cbGarminHost->GetValue();
  pConnectionParams->InputSentenceList =
      wxStringTokenize(m_tcInputStc->GetValue(), _T(","));
  if (m_rbIAccept->GetValue())
    pConnectionParams->InputSentenceListType = WHITELIST;
  else
    pConnectionParams->InputSentenceListType = BLACKLIST;
  if (m_cbInput->GetValue()) {
    if (m_cbOutput->GetValue()) {
      pConnectionParams->IOSelect = DS_TYPE_INPUT_OUTPUT;
    } else {
      pConnectionParams->IOSelect = DS_TYPE_INPUT;
    }
  } else
    pConnectionParams->IOSelect = DS_TYPE_OUTPUT;

  pConnectionParams->OutputSentenceList =
      wxStringTokenize(m_tcOutputStc->GetValue(), _T(","));
  if (m_rbOAccept->GetValue())
    pConnectionParams->OutputSentenceListType = WHITELIST;
  else
    pConnectionParams->OutputSentenceListType = BLACKLIST;
  pConnectionParams->Port = m_comboPort->GetValue().BeforeFirst(' ');
#if defined(__linux__) && !defined(__OCPN__ANDROID__)
  if (pConnectionParams->Type == SERIAL)
    CheckSerialAccess(this, pConnectionParams->Port.ToStdString());
#endif

  if ((pConnectionParams->Type != INTERNAL_GPS) &&
      (pConnectionParams->Type != INTERNAL_BT))
    CheckDeviceAccess(pConnectionParams->Port);

  pConnectionParams->bEnabled = m_connection_enabled;
  pConnectionParams->b_IsSetup = FALSE;

  if (pConnectionParams->Type == INTERNAL_GPS) {
    pConnectionParams->NetworkAddress = _T("");
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
  }

  if (pConnectionParams->Type == INTERNAL_BT) {
    wxString parms = m_choiceBTDataSources->GetStringSelection();
    wxStringTokenizer tkz(parms, _T(";"));
    wxString name = tkz.GetNextToken();
    wxString mac = tkz.GetNextToken();

    pConnectionParams->NetworkAddress = name;
    pConnectionParams->Port = mac;
    pConnectionParams->NetworkPort = 0;
    pConnectionParams->NetProtocol = PROTO_UNDEFINED;
    pConnectionParams->Baudrate = 0;
    //        pConnectionParams->SetAuxParameterStr(m_choiceBTDataSources->GetStringSelection());
  }

  if (pConnectionParams->Type == SERIAL) {
    pConnectionParams->UserComment = m_tSerialComment->GetValue();
  } else if (pConnectionParams->Type == NETWORK) {
    pConnectionParams->UserComment = m_tNetComment->GetValue();
  }

  return pConnectionParams;
}

void options::OnApplyClick(wxCommandEvent& event) {
  //::wxBeginBusyCursor();
  StopBTScan();

  // Start with the stuff that requires intelligent validation.

  if (m_pShipIconType->GetSelection() > 0) {
    double n_ownship_length_meters;
    double n_ownship_beam_meters;
    double n_gps_antenna_offset_y;
    double n_gps_antenna_offset_x;
    long n_ownship_min_mm;
    m_pOSLength->GetValue().ToDouble(&n_ownship_length_meters);
    m_pOSWidth->GetValue().ToDouble(&n_ownship_beam_meters);
    m_pOSGPSOffsetX->GetValue().ToDouble(&n_gps_antenna_offset_x);
    m_pOSGPSOffsetY->GetValue().ToDouble(&n_gps_antenna_offset_y);
    m_pOSMinSize->GetValue().ToLong(&n_ownship_min_mm);
    wxString msg;
    if (n_ownship_length_meters <= 0)
      msg += _("\n - your ship's length must be > 0");
    if (n_ownship_beam_meters <= 0)
      msg += _("\n - your ship's beam must be > 0");
    if (fabs(n_gps_antenna_offset_x) > n_ownship_beam_meters / 2.0)
      msg += _(
          "\n - your GPS offset from midship must be within your ship's beam");
    if (n_gps_antenna_offset_y < 0 ||
        n_gps_antenna_offset_y > n_ownship_length_meters)
      msg +=
          _("\n - your GPS offset from bow must be within your ship's length");
    if (n_ownship_min_mm <= 0 || n_ownship_min_mm > 100)
      msg += _("\n - your minimum ship icon size must be between 1 and 100 mm");
    if (!msg.IsEmpty()) {
      msg.Prepend(_("The settings for own ship real size are not correct:"));
      OCPNMessageBox(this, msg, _("OpenCPN info"), wxICON_ERROR | wxOK);
      ::wxEndBusyCursor();
      event.SetInt(wxID_STOP);
      return;
    }
    g_n_ownship_length_meters = n_ownship_length_meters;
    g_n_ownship_beam_meters = n_ownship_beam_meters;
    g_n_gps_antenna_offset_y = n_gps_antenna_offset_y;
    g_n_gps_antenna_offset_x = n_gps_antenna_offset_x;
    g_n_ownship_min_mm = static_cast<int>(n_ownship_min_mm);
  }
  g_OwnShipIconType = m_pShipIconType->GetSelection();
  g_bShowShipToActive = pShowshipToActive->GetValue();
  g_shipToActiveStyle = m_shipToActiveStyle->GetSelection();
  g_shipToActiveColor = m_shipToActiveColor->GetSelection();

  m_pText_ACRadius->GetValue().ToDouble(&g_n_arrival_circle_radius);
  g_n_arrival_circle_radius =
      wxClip(g_n_arrival_circle_radius, 0.001, 0.6);  // Correct abnormally

  wxString* icon_name =
      pWayPointMan->GetIconKey(pWaypointDefaultIconChoice->GetSelection());
  if (icon_name && icon_name->Length()) g_default_wp_icon = *icon_name;

  icon_name =
      pWayPointMan->GetIconKey(pRoutepointDefaultIconChoice->GetSelection());
  if (icon_name && icon_name->Length()) g_default_routepoint_icon = *icon_name;

  g_bUseWptScaMin = pScaMinChckB->GetValue();
  g_iWpt_ScaMin = wxAtoi(m_pText_ScaMin->GetValue());
  g_bOverruleScaMin = pScaMinOverruleChckB->GetValue();

  //  Any Font changes?
  if (m_bfontChanged) m_returnChanges |= FONT_CHANGED;

  // Handle Chart Tab
  UpdateWorkArrayFromDisplayPanel();

  groupsPanel->SetDBDirs(*m_pWorkDirList);  // update the Groups tab
  groupsPanel->m_treespopulated = FALSE;

  int k_force = FORCE_UPDATE;
  if (pUpdateCheckBox) {
    if (!pUpdateCheckBox->GetValue()) k_force = 0;
    pUpdateCheckBox->Enable();
    pUpdateCheckBox->SetValue(FALSE);
  } else {
    k_force = 0;
  }

  m_returnChanges |= k_force;

  int k_scan = SCAN_UPDATE;
  if (pScanCheckBox) {
    if (!pScanCheckBox->GetValue()) k_scan = 0;
    pScanCheckBox->Enable();
    pScanCheckBox->SetValue(FALSE);
  } else {
    k_scan = 0;
  }

  m_returnChanges |= k_scan;

  // Chart Groups

  if (groupsPanel->modified) {
    groupsPanel->EmptyChartGroupArray(g_pGroupArray);
    delete g_pGroupArray;
    g_pGroupArray = groupsPanel->CloneChartGroupArray(m_pGroupArray);
    m_returnChanges |= GROUPS_CHANGED;
  }

  // Handle Settings Tab
  if (m_pConfig) {
    g_bShowStatusBar = pShowStatusBar->GetValue();
#ifndef __WXOSX__
    g_bShowMenuBar = pShowMenuBar->GetValue();
#endif
    g_bShowCompassWin = pShowCompassWin->GetValue();
  }

  g_bShowChartBar = pShowChartBar->GetValue();

  wxString screenmm = pScreenMM->GetValue();
  long mm = -1;
  screenmm.ToLong(&mm);
  g_config_display_size_mm = mm > 0 ? mm : -1;
  g_config_display_size_manual = pRBSizeManual->GetValue();

  // Connections page.
  g_bfilter_cogsog = m_cbFilterSogCog->GetValue();

  long filter_val = 1;
  m_tFilterSec->GetValue().ToLong(&filter_val);
  g_COGFilterSec =
      wxMin(static_cast<int>(filter_val), MAX_COGSOG_FILTER_SECONDS);
  g_COGFilterSec = wxMax(g_COGFilterSec, 1);
  g_SOGFilterSec = g_COGFilterSec;

  g_bMagneticAPB = m_cbAPBMagnetic->GetValue();
  g_NMEAAPBPrecision = m_choicePrecision->GetCurrentSelection();

  // NMEA Source
  //  If the stream selected exists, capture some of its existing parameters
  //  to facility identification and allow stop and restart of the stream
  wxString lastAddr;
  int lastPort = 0;
  NetworkProtocol lastNetProtocol = PROTO_UNDEFINED;

  if (mSelectedConnection) {
    ConnectionParams* cpo = mSelectedConnection;
    lastAddr = cpo->NetworkAddress;
    lastPort = cpo->NetworkPort;
    lastNetProtocol = cpo->NetProtocol;
  }

  if (!connectionsaved) {
    size_t nCurrentPanelCount = g_pConnectionParams->GetCount();
    ConnectionParams* cp = NULL;
    int old_priority = -1;
    {
      if (mSelectedConnection) {
        cp = mSelectedConnection;
        old_priority = cp->Priority;
        UpdateConnectionParamsFromSelectedItem(cp);
        cp->b_IsSetup = false;

        // delete g_pConnectionParams->Item(itemIndex)->m_optionsPanel;
        // old_priority = g_pConnectionParams->Item(itemIndex)->Priority;
        // g_pConnectionParams->RemoveAt(itemIndex);
        // g_pConnectionParams->Insert(cp, itemIndex);
        // mSelectedConnection = cp;
        // cp->m_optionsPanel->SetSelected( true );
      } else {
        cp = CreateConnectionParamsFromSelectedItem();
        if (cp) g_pConnectionParams->Add(cp);
      }

      //  Record the previous parameters, if any
      if (cp) {
        cp->LastNetProtocol = lastNetProtocol;
        cp->LastNetworkAddress = lastAddr;
        cp->LastNetworkPort = lastPort;
      }

      if (g_pConnectionParams->GetCount() != nCurrentPanelCount)
        FillSourceList();
      else if (old_priority >= 0) {
        if (old_priority != cp->Priority)  // need resort
          UpdateSourceList(true);
        else
          UpdateSourceList(false);
      }

      connectionsaved = TRUE;
    }
    //     else {
    //       ::wxEndBusyCursor();
    //       if (m_bNMEAParams_shown) event.SetInt(wxID_STOP);
    //     }

    SetSelectedConnectionPanel(nullptr);
  }

  // Recreate datastreams that are new, or have been edited
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
    ConnectionParams* cp = g_pConnectionParams->Item(i);

    if (cp->b_IsSetup) continue;

    // Stream is new, or edited

    // Terminate and remove any existing stream with the same port name
    StopAndRemoveCommDriver(cp->GetDSPort());

    /DataStream* pds_existing = g_pMUX->FindStream(cp->GetDSPort());
    /if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);

    //  Try to stop any previous stream to avoid orphans
    pds_existing = g_pMUX->FindStream(cp->GetLastDSPort());
    if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);

    //  This for Bluetooth, which has strange parameters
    if (cp->Type == INTERNAL_BT) {
      pds_existing = g_pMUX->FindStream(cp->GetPortStr());
      if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);
    }

    // Internal BlueTooth driver stacks commonly need a time delay to purge
    // their buffers, etc. before restating with new parameters...
    if (cp->Type == INTERNAL_BT) wxSleep(1);

    if (!cp->bEnabled) continue;
    g_pMUX->AddStream(makeDataStream(g_pMUX, cp));
    cp->b_IsSetup = TRUE;
  }

  g_bGarminHostUpload = m_cbGarminUploadHost->GetValue();
  g_GPS_Ident =
      m_cbFurunoGP3X->GetValue() ? _T( "FurunoGP3X" ) : _T( "Generic" );

  // End of Connections page
  if (pCDOOutlines) g_bShowOutlines = pCDOOutlines->GetValue();
  if (pSDisplayGrid) g_bDisplayGrid = pSDisplayGrid->GetValue();

  if (pCDOQuilting) {
    bool temp_bquilting = pCDOQuilting->GetValue();
    //   if (!g_bQuiltEnable && temp_bquilting)
    //     cc1->ReloadVP(); /* compose the quilt */
    g_bQuiltEnable = temp_bquilting;
  }
  //  g_bFullScreenQuilt = !pFullScreenQuilt->GetValue();

  if (pSDepthUnits) g_bShowDepthUnits = pSDepthUnits->GetValue();
  g_bskew_comp = pSkewComp->GetValue();
  g_btouch = pMobile->GetValue();
  g_bresponsive = pResponsive->GetValue();
  g_bRollover = pRollover->GetValue();
  g_bShowMuiZoomButtons = pZoomButtons->GetValue();

  g_bAutoHideToolbar = pToolbarAutoHideCB->GetValue();

  long hide_val = 10;
  pToolbarHideSecs->GetValue().ToLong(&hide_val);
  g_nAutoHideToolbar = wxMin(static_cast<int>(hide_val), 100);
  g_nAutoHideToolbar = wxMax(g_nAutoHideToolbar, 2);

  // g_fog_overzoom = !pOverzoomEmphasis->GetValue();
  // g_oz_vector_scale = !pOZScaleVector->GetValue();

  g_bsmoothpanzoom = pSmoothPanZoom->GetValue();
#ifdef __OCPN__ANDROID__
  g_bsmoothpanzoom = false;
#endif
  if (pSmoothPanZoom) g_bsmoothpanzoom = pSmoothPanZoom->GetValue();
#ifdef __OCPN__ANDROID__
  g_bsmoothpanzoom = false;
#endif

  long update_val = 1;
  pCOGUPUpdateSecs->GetValue().ToLong(&update_val);
  g_COGAvgSec = wxMin(static_cast<int>(update_val), MAX_COG_AVERAGE_SECONDS);

  // TODO if (g_bCourseUp != pCBCourseUp->GetValue()) gFrame->ToggleCourseUp();

  if (pCBLookAhead) g_bLookAhead = pCBLookAhead->GetValue();

  g_bShowTrue = pCBTrueShow->GetValue();
  g_bShowMag = pCBMagShow->GetValue();

  auto loader = PluginLoader::getInstance();
  b_haveWMM = loader && loader->IsPlugInAvailable(_T("WMM"));
  if (!b_haveWMM && !b_oldhaveWMM) {
    pMagVar->GetValue().ToDouble(&g_UserVar);
    gVar = g_UserVar;
  }

  m_pText_OSCOG_Predictor->GetValue().ToDouble(&g_ownship_predictor_minutes);
  m_pText_OSHDT_Predictor->GetValue().ToDouble(&g_ownship_HDTpredictor_miles);

  double temp_dbl;
  g_iNavAidRadarRingsNumberVisible =
      pNavAidRadarRingsNumberVisible->GetSelection();
  if (pNavAidRadarRingsStep->GetValue().ToDouble(&temp_dbl))
    g_fNavAidRadarRingsStep = temp_dbl;
  g_pNavAidRadarRingsStepUnits = m_itemRadarRingsUnits->GetSelection();
  g_iWaypointRangeRingsNumber = pWaypointRangeRingsNumber->GetSelection();
  if (pWaypointRangeRingsStep->GetValue().ToDouble(&temp_dbl))
    g_fWaypointRangeRingsStep = temp_dbl;
  g_iWaypointRangeRingsStepUnits =
      m_itemWaypointRangeRingsUnits->GetSelection();
  g_colourWaypointRangeRingsColour =
      m_colourWaypointRangeRingsColour->GetColour();
  g_colourWaypointRangeRingsColour =
      wxColour(g_colourWaypointRangeRingsColour.Red(),
               g_colourWaypointRangeRingsColour.Green(),
               g_colourWaypointRangeRingsColour.Blue());
  g_bWayPointPreventDragging = pWayPointPreventDragging->GetValue();
  g_own_ship_sog_cog_calc = pSogCogFromLLCheckBox->GetValue();
  g_own_ship_sog_cog_calc_damp_sec = pSogCogFromLLDampInterval->GetValue();

  g_bConfirmObjectDelete = pConfirmObjectDeletion->GetValue();

  if (pPreserveScale) g_bPreserveScaleOnX = pPreserveScale->GetValue();

  if (g_bUIexpert && pCmdSoundString) {
    g_CmdSoundString = pCmdSoundString->GetValue();
    if (wxIsEmpty(g_CmdSoundString)) {
      g_CmdSoundString = wxString(OCPN_SOUND_CMD);
      pCmdSoundString->SetValue(g_CmdSoundString);
    }
  }

  g_bPlayShipsBells = pPlayShipsBells->GetValue();
  if (pSoundDeviceIndex)
    g_iSoundDeviceIndex = pSoundDeviceIndex->GetSelection();
  // g_bTransparentToolbar = pTransparentToolbar->GetValue();
  g_iSDMMFormat = pSDMMFormat->GetSelection();
  g_iDistanceFormat = pDistanceFormat->GetSelection();
  g_iSpeedFormat = pSpeedFormat->GetSelection();
  g_iTempFormat = pTempFormat->GetSelection();

  // LIVE ETA OPTION
  if (pSLiveETA) g_bShowLiveETA = pSLiveETA->GetValue();
  if (pSDefaultBoatSpeed)
    pSDefaultBoatSpeed->GetValue().ToDouble(&g_defaultBoatSpeedUserUnit);
  g_defaultBoatSpeed = fromUsrSpeed(g_defaultBoatSpeedUserUnit);

  g_bAdvanceRouteWaypointOnArrivalOnly =
      pAdvanceRouteWaypointOnArrivalOnly->GetValue();

  g_colourTrackLineColour = m_colourTrackLineColour->GetColour();
  g_colourTrackLineColour =
      wxColour(g_colourTrackLineColour.Red(), g_colourTrackLineColour.Green(),
               g_colourTrackLineColour.Blue());
  g_nTrackPrecision = pTrackPrecision->GetSelection();

  g_bTrackDaily = pTrackDaily->GetValue();

  g_track_rotate_time = 0;
#if wxCHECK_VERSION(2, 9, 0)
#if wxUSE_TIMEPICKCTRL
  int h, m, s;
  if (pTrackRotateTime->GetTime(&h, &m, &s))
    g_track_rotate_time = h * 3600 + m * 60 + s;
#endif
#endif

  if (pTrackRotateUTC->GetValue())
    g_track_rotate_time_type = TIME_TYPE_UTC;
  else if (pTrackRotateLMT->GetValue())
    g_track_rotate_time_type = TIME_TYPE_LMT;
  else
    g_track_rotate_time_type = TIME_TYPE_COMPUTER;

  g_bHighliteTracks = pTrackHighlite->GetValue();

  if (pEnableZoomToCursor)
    g_bEnableZoomToCursor = pEnableZoomToCursor->GetValue();
#ifdef __OCPN__ANDROID__
  g_bEnableZoomToCursor = false;
#endif

  g_colourOwnshipRangeRingsColour = m_colourOwnshipRangeRingColour->GetColour();
  g_colourOwnshipRangeRingsColour =
      wxColour(g_colourOwnshipRangeRingsColour.Red(),
               g_colourOwnshipRangeRingsColour.Green(),
               g_colourOwnshipRangeRingsColour.Blue());

  // Sounds
   g_bAIS_GCPA_Alert_Audio = m_soundPanelAIS->GetCheckBox()->GetValue();
   g_bAIS_SART_Alert_Audio = m_soundPanelSART->GetCheckBox()->GetValue();
   g_bAIS_DSC_Alert_Audio = m_soundPanelDSC->GetCheckBox()->GetValue();
   g_bAnchor_Alert_Audio = m_soundPanelAnchor->GetCheckBox()->GetValue();

  // AIS Parameters
  //   CPA Box
  g_bCPAMax = m_pCheck_CPA_Max->GetValue();
  m_pText_CPA_Max->GetValue().ToDouble(&g_CPAMax_NM);
  g_bCPAWarn = m_pCheck_CPA_Warn->GetValue();
  m_pText_CPA_Warn->GetValue().ToDouble(&g_CPAWarn_NM);
  g_bTCPA_Max = m_pCheck_CPA_WarnT->GetValue();
  m_pText_CPA_WarnT->GetValue().ToDouble(&g_TCPA_Max);

  //   Lost Targets
  g_bMarkLost = m_pCheck_Mark_Lost->GetValue();
  m_pText_Mark_Lost->GetValue().ToDouble(&g_MarkLost_Mins);
  g_bRemoveLost = m_pCheck_Remove_Lost->GetValue();
  m_pText_Remove_Lost->GetValue().ToDouble(&g_RemoveLost_Mins);

  //   Display
  g_bShowCOG = m_pCheck_Show_COG->GetValue();
  // If synchronized with own ship predictor
  g_bSyncCogPredictors = m_pCheck_Sync_OCOG_ACOG->GetValue();
  if (g_bSyncCogPredictors) {
    m_pText_COG_Predictor->SetValue(m_pText_OSCOG_Predictor->GetValue());
  }
  m_pText_COG_Predictor->GetValue().ToDouble(&g_ShowCOG_Mins);

  g_bAISShowTracks = m_pCheck_Show_Tracks->GetValue();
  m_pText_Track_Length->GetValue().ToDouble(&g_AISShowTracks_Mins);

  //   Update all the current targets
  if (g_pAIS) {
    for (const auto& it : g_pAIS->GetTargetList()) {
      AIS_Target_Data* pAISTarget = it.second;
      if (NULL != pAISTarget) {
        pAISTarget->b_show_track = g_bAISShowTracks;
        // Check for exceptions in MMSI properties
        for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
          if (pAISTarget->MMSI == g_MMSI_Props_Array[i]->MMSI) {
            MMSIProperties *props = g_MMSI_Props_Array[i];
            if (TRACKTYPE_NEVER == props->TrackType) {
              pAISTarget->b_show_track = false;
              break;
            }
            else if (TRACKTYPE_ALWAYS == props->TrackType) {
              pAISTarget->b_show_track = true;
              break;
            }
            else
              break;
          }
        }
      }
    }
  }

  g_bHideMoored = m_pCheck_Hide_Moored->GetValue();
  m_pText_Moored_Speed->GetValue().ToDouble(&g_ShowMoored_Kts);

  g_bDrawAISRealtime = m_pCheck_Draw_Realtime_Prediction->GetValue();
  m_pText_RealtPred_Speed->GetValue().ToDouble(&g_AIS_RealtPred_Kts);

  g_bAllowShowScaled = m_pCheck_Scale_Priority->GetValue();
  long l;
  m_pText_Scale_Priority->GetValue().ToLong(&l);
  g_ShowScaled_Num = (int)l;

  g_bShowAreaNotices = m_pCheck_Show_Area_Notices->GetValue();
  g_bDrawAISSize = m_pCheck_Draw_Target_Size->GetValue();
  g_bShowAISName = m_pCheck_Show_Target_Name->GetValue();
  long ais_name_scale = 5000;
  m_pText_Show_Target_Name_Scale->GetValue().ToLong(&ais_name_scale);
  g_Show_Target_Name_Scale = (int)wxMax(5000, ais_name_scale);
  g_bWplUsePosition = m_pCheck_use_Wpl->GetValue();
  g_WplAction = m_pWplAction->GetSelection();

  //   Alert
  g_bAIS_CPA_Alert = m_pCheck_AlertDialog->GetValue();
  g_bAIS_CPA_Alert_Audio = m_pCheck_AlertAudio->GetValue();
  g_bAIS_CPA_Alert_Suppress_Moored = m_pCheck_Alert_Moored->GetValue();

  g_bAIS_ACK_Timeout = m_pCheck_Ack_Timout->GetValue();
  m_pText_ACK_Timeout->GetValue().ToDouble(&g_AckTimeout_Mins);

  //   Rollover
  g_bAISRolloverShowClass = m_pCheck_Rollover_Class->GetValue();
  g_bAISRolloverShowCOG = m_pCheck_Rollover_COG->GetValue();
  g_bAISRolloverShowCPA = m_pCheck_Rollover_CPA->GetValue();

  g_chart_zoom_modifier = m_pSlider_Zoom->GetValue();
  g_chart_zoom_modifier_vector = m_pSlider_Zoom_Vector->GetValue();
  g_cm93_zoom_factor = m_pSlider_CM93_Zoom->GetValue();
  g_GUIScaleFactor = m_pSlider_GUI_Factor->GetValue();
  g_ChartScaleFactor = m_pSlider_Chart_Factor->GetValue();
  g_ChartScaleFactorExp =
      g_Platform->getChartScaleFactorExp(g_ChartScaleFactor);
  g_ShipScaleFactor = m_pSlider_Ship_Factor->GetValue();
  g_ShipScaleFactorExp = g_Platform->getChartScaleFactorExp(g_ShipScaleFactor);
  g_ENCSoundingScaleFactor = m_pSlider_Text_Factor->GetValue();
  g_mouse_zoom_sensitivity_ui = m_pMouse_Zoom_Slider->GetValue();
  g_mouse_zoom_sensitivity =
      MouseZoom::ui_to_config(g_mouse_zoom_sensitivity_ui);

  //  Only reload the icons if user has actually visted the UI page
  // if(m_bVisitLang)
  //  pWayPointMan->ReloadAllIcons();

  g_NMEAAPBPrecision = m_choicePrecision->GetCurrentSelection();

  g_TalkerIdText = m_TalkerIdText->GetValue().MakeUpper();

  if (g_bopengl != pOpenGL->GetValue()) m_returnChanges |= GL_CHANGED;
  g_bopengl = pOpenGL->GetValue();

  //   Handle Vector Charts Tab
  int depthUnit = pDepthUnitSelect->GetSelection();
  g_nDepthUnitDisplay = depthUnit;

  //  Process the UserStandard display list, noting if any changes were made
  bool bUserStdChange = false;

  int nOBJL = ps57CtlListBox->GetCount();

  for (int iPtr = 0; iPtr < nOBJL; iPtr++) {
    int itemIndex = -1;
    for (size_t i = 0; i < marinersStdXref.size(); i++) {
      if (marinersStdXref[i] == iPtr) {
        itemIndex = i;
        break;
      }
    }
    assert(itemIndex >= 0);
    OBJLElement* pOLE = (OBJLElement*)(ps52plib->pOBJLArray->Item(itemIndex));
    if (pOLE->nViz != (int)(ps57CtlListBox->IsChecked(iPtr)))
      bUserStdChange = true;
    pOLE->nViz = ps57CtlListBox->IsChecked(iPtr);
  }

  if (ps52plib) {
    // Take a snapshot of the S52 config right now,
    // for later comparison
    ps52plib->GenerateStateHash();
    long stateHash = ps52plib->GetStateHash();

    if (m_returnChanges & GL_CHANGED) {
      // Do this now to handle the screen refresh that is automatically
      // generated on Windows at closure of the options dialog...
      ps52plib->FlushSymbolCaches();
      // some CNSY depends on renderer (e.g. CARC)
      ps52plib->ClearCNSYLUPArray();
      ps52plib->GenerateStateHash();
    }

    if (pDispCat) {
      enum _DisCat nset = OTHER;
      switch (pDispCat->GetSelection()) {
        case 0:
          nset = DISPLAYBASE;
          break;
        case 1:
          nset = STANDARD;
          break;
        case 2:
          nset = OTHER;
          break;
        case 3:
          nset = MARINERS_STANDARD;
          break;
      }
      ps52plib->SetDisplayCategory(nset);
    }

    if (pCheck_SOUNDG) ps52plib->m_bShowSoundg = pCheck_SOUNDG->GetValue();
    if (pCheck_ATONTEXT)
      ps52plib->m_bShowAtonText = pCheck_ATONTEXT->GetValue();
    if (pCheck_LDISTEXT)
      ps52plib->m_bShowLdisText = pCheck_LDISTEXT->GetValue();
    if (pCheck_XLSECTTEXT)
      ps52plib->m_bExtendLightSectors = pCheck_XLSECTTEXT->GetValue();

    ps52plib->m_bShowMeta = pCheck_META->GetValue();
    ps52plib->m_bDeClutterText = pCheck_DECLTEXT->GetValue();
    ps52plib->m_bShowNationalTexts = pCheck_NATIONALTEXT->GetValue();
    ps52plib->m_bShowS57ImportantTextOnly = pCheck_SHOWIMPTEXT->GetValue();
    ps52plib->m_bUseSCAMIN = pCheck_SCAMIN->GetValue();

    ps52plib->m_nSymbolStyle =
        pPointStyle->GetSelection() == 0 ? PAPER_CHART : SIMPLIFIED;

    ps52plib->m_nBoundaryStyle = pBoundStyle->GetSelection() == 0
                                     ? PLAIN_BOUNDARIES
                                     : SYMBOLIZED_BOUNDARIES;
    ps52plib->m_nSoundingFactor = m_pSlider_Text_Factor->GetValue();

    S52_setMarinerParam(S52_MAR_TWO_SHADES,
                        (p24Color->GetSelection() == 0) ? 1.0 : 0.0);

    // Depths
    double dval;
    float conv = 1;

    if (depthUnit == 0)  // feet
      conv = 0.3048f;    // international definiton of 1 foot is 0.3048 metres
    else if (depthUnit == 2)  // fathoms
      conv = 0.3048f * 6;     // 1 fathom is 6 feet

    if (m_SafetyCtl->GetValue().ToDouble(&dval)) {
      S52_setMarinerParam(S52_MAR_SAFETY_DEPTH,
                          dval * conv);  // controls sounding display
      S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR,
                          dval * conv);  // controls colour
    }

    if (m_ShallowCtl->GetValue().ToDouble(&dval))
      S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval * conv);

    if (m_DeepCtl->GetValue().ToDouble(&dval))
      S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval * conv);

    ps52plib->UpdateMarinerParams();
    ps52plib->m_nDepthUnitDisplay = depthUnit;

    ps52plib->GenerateStateHash();

    // Detect a change to S52 library config
    if ((stateHash != ps52plib->GetStateHash()) || bUserStdChange)
      m_returnChanges |= S52_CHANGED;
  }

// User Interface Panel
#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
  if (m_bVisitLang) {
    wxString new_canon = _T("en_US");
    wxString lang_sel = m_itemLangListBox->GetStringSelection();

    int nLang = sizeof(lang_list) / sizeof(int);
    for (int it = 0; it < nLang; it++) {
      const wxLanguageInfo* pli = wxLocale::GetLanguageInfo(lang_list[it]);
      if (pli) {
        wxString lang_canonical = pli->CanonicalName;
        wxString test_string = GetOCPNKnownLanguage(lang_canonical);
        if (lang_sel == test_string) {
          new_canon = lang_canonical;
          break;
        }
      }
    }

    wxString locale_old = g_locale;
    g_locale = new_canon;

    if (g_locale != locale_old) m_returnChanges |= LOCALE_CHANGED;

    wxString oldStyle = g_StyleManager->GetCurrentStyle()->name;
    // g_StyleManager->SetStyleNextInvocation(
    // m_itemStyleListBox->GetStringSelection());
    if (g_StyleManager->GetStyleNextInvocation() != oldStyle) {
      m_returnChanges |= STYLE_CHANGED;
    }
    wxSizeEvent nullEvent;
    gFrame->OnSize(nullEvent);
  }
#endif
  if (g_bInlandEcdis != pInlandEcdis->GetValue()) {  // InlandEcdis changed
    g_bInlandEcdis = pInlandEcdis->GetValue();
    SwitchInlandEcdisMode(g_bInlandEcdis);
    m_returnChanges |= TOOLBAR_CHANGED;
  }
  // PlugIn Manager Panel

  // Pick up any changes to selections
  if (PluginLoader::getInstance()->UpdatePlugIns())
    m_returnChanges |= TOOLBAR_CHANGED;

  // And keep config in sync
  if (m_pPlugInCtrl) m_pPlugInCtrl->UpdatePluginsOrder();
  g_pi_manager->UpdateConfig();

  // PlugIns may have added panels
  if (g_pi_manager) g_pi_manager->CloseAllPlugInPanels((int)wxOK);

  m_returnChanges |= GENERIC_CHANGED | k_vectorcharts | k_charts |
                     m_groups_changed | k_plugins | k_tides;

  // Pick up all the entries in the Tide/current DataSelected control
  // and update the global static array
  TideCurrentDataSet.Clear();
  int nEntry = tcDataSelected->GetItemCount();
  for (int i = 0; i < nEntry; i++) {
    wxString setName = tcDataSelected->GetItemText(i);
    TideCurrentDataSet.Add(setName);
  }

  if (event.GetId() != ID_APPLY)  // only on ID_OK
    g_canvasConfig = m_screenConfig;

  if (event.GetId() == ID_APPLY) {
    gFrame->ProcessOptionsDialog(m_returnChanges, m_pWorkDirList);
    m_CurrentDirList =
        *m_pWorkDirList;  // Perform a deep copy back to main database.

    //  We can clear a few flag bits on "Apply", so they won't be recognised at
    //  the "OK" click. Their actions have already been accomplished once...
    m_returnChanges &= ~(CHANGE_CHARTS | FORCE_UPDATE | SCAN_UPDATE);
    k_charts = 0;

    gFrame->RefreshAllCanvas();
  }

  // Some layout changes requiring a new options instance?
  if (m_bneedNew) m_returnChanges |= NEED_NEW_OPTIONS;

  //  Record notice of any changes to last applied template
  UpdateTemplateTitleText();

  if (::wxIsBusy())  // FIXME: Not sure why this is needed here
    ::wxEndBusyCursor();
}

void options::OnXidOkClick(wxCommandEvent& event) {
  // When closing the form with Ctrl-Enter sometimes we get double events, the
  // second is empty??
  if (event.GetEventObject() == NULL) return;

  OnApplyClick(event);
  if (event.GetInt() == wxID_STOP) return;

  Finish();
}

void options::Finish(void) {
  //  Required to avoid intermittent crash on wxGTK
  m_pListbook->ChangeSelection(0);
  for (size_t i = 0; i < m_pListbook->GetPageCount(); i++) {
    wxNotebookPage* pg = m_pListbook->GetPage(i);
    wxNotebook* nb = dynamic_cast<wxNotebook*>(pg);
    if (nb) nb->ChangeSelection(0);
  }

  lastWindowPos = GetPosition();
  lastWindowSize = GetSize();

  pConfig->SetPath("/Settings");
  pConfig->Write("OptionsSizeX", lastWindowSize.x);
  pConfig->Write("OptionsSizeY", lastWindowSize.y);

  SetReturnCode(m_returnChanges);
  EndModal(m_returnChanges);
}

ArrayOfCDI options::GetSelectedChartDirs() {
  ArrayOfCDI rv;
  for (size_t i = 0; i < panelVector.size(); i++) {
    if (panelVector[i]->IsSelected()) {
      rv.Add(panelVector[i]->GetCDI());
    }
  }

  return rv;
}

ArrayOfCDI options::GetUnSelectedChartDirs() {
  ArrayOfCDI rv;
  for (size_t i = 0; i < panelVector.size(); i++) {
    if (!panelVector[i]->IsSelected()) {
      rv.Add(panelVector[i]->GetCDI());
    }
  }

  return rv;
}

void options::SetDirActionButtons() {
  ArrayOfCDI selArray = GetSelectedChartDirs();
  if (selArray.GetCount())
    m_removeBtn->Enable();
  else
    m_removeBtn->Disable();
}

void options::OnButtondeleteClick(wxCommandEvent& event) {
  ArrayOfCDI unselArray = GetUnSelectedChartDirs();
  ActiveChartArray.Clear();
  for (size_t i = 0; i < unselArray.GetCount(); i++) {
    ActiveChartArray.Add(unselArray[i]);
  }

  UpdateChartDirList();

  UpdateWorkArrayFromDisplayPanel();

#if 0
  if (m_pWorkDirList) {
    pActiveChartsList->DeleteAllItems();
    for (size_t id = 0; id < m_pWorkDirList->GetCount(); id++) {
      wxString dirname = m_pWorkDirList->Item(id).fullpath;
      wxListItem li;
      li.SetId(id);
      li.SetAlign(wxLIST_FORMAT_LEFT);
      li.SetText(dirname);
      li.SetColumn(0);
      long idx = pActiveChartsList->InsertItem(li);
    }
  }
#endif

  k_charts |= CHANGE_CHARTS;

  pScanCheckBox->Disable();

  event.Skip();
}

void options::OnButtonParseENC(wxCommandEvent& event) {
  gFrame->GetPrimaryCanvas()->EnablePaint(false);

  extern void ParseAllENC(wxWindow * parent);

  ParseAllENC(g_pOptions);

  ViewPort vp;
  gFrame->ChartsRefresh();

  gFrame->GetPrimaryCanvas()->EnablePaint(true);
}

#ifdef OCPN_USE_LZMA
#include <lzma.h>

static bool compress(lzma_stream* strm, FILE* infile, FILE* outfile) {
  // This will be LZMA_RUN until the end of the input file is reached.
  // This tells lzma_code() when there will be no more input.
  lzma_action action = LZMA_RUN;

  // Buffers to temporarily hold uncompressed input
  // and compressed output.
  uint8_t inbuf[BUFSIZ];
  uint8_t outbuf[BUFSIZ];

  // Initialize the input and output pointers. Initializing next_in
  // and avail_in isn't really necessary when we are going to encode
  // just one file since LZMA_STREAM_INIT takes care of initializing
  // those already. But it doesn't hurt much and it will be needed
  // if encoding more than one file like we will in 02_decompress.c.
  //
  // While we don't care about strm->total_in or strm->total_out in this
  // example, it is worth noting that initializing the encoder will
  // always reset total_in and total_out to zero. But the encoder
  // initialization doesn't touch next_in, avail_in, next_out, or
  // avail_out.
  strm->next_in = NULL;
  strm->avail_in = 0;
  strm->next_out = outbuf;
  strm->avail_out = sizeof(outbuf);

  // Loop until the file has been successfully compressed or until
  // an error occurs.
  while (true) {
    // Fill the input buffer if it is empty.
    if (strm->avail_in == 0 && !feof(infile)) {
      strm->next_in = inbuf;
      strm->avail_in = fread(inbuf, 1, sizeof(inbuf), infile);

      if (ferror(infile)) {
        fprintf(stderr, "Read error: %s\n", strerror(errno));
        return false;
      }

      // Once the end of the input file has been reached,
      // we need to tell lzma_code() that no more input
      // will be coming and that it should finish the
      // encoding.
      if (feof(infile)) action = LZMA_FINISH;
    }

    // Tell liblzma do the actual encoding.
    //
    // This reads up to strm->avail_in bytes of input starting
    // from strm->next_in. avail_in will be decremented and
    // next_in incremented by an equal amount to match the
    // number of input bytes consumed.
    //
    // Up to strm->avail_out bytes of compressed output will be
    // written starting from strm->next_out. avail_out and next_out
    // will be incremented by an equal amount to match the number
    // of output bytes written.
    //
    // The encoder has to do internal buffering, which means that
    // it may take quite a bit of input before the same data is
    // available in compressed form in the output buffer.
    lzma_ret ret = lzma_code(strm, action);

    // If the output buffer is full or if the compression finished
    // successfully, write the data from the output bufffer to
    // the output file.
    if (strm->avail_out == 0 || ret == LZMA_STREAM_END) {
      // When lzma_code() has returned LZMA_STREAM_END,
      // the output buffer is likely to be only partially
      // full. Calculate how much new data there is to
      // be written to the output file.
      size_t write_size = sizeof(outbuf) - strm->avail_out;

      if (fwrite(outbuf, 1, write_size, outfile) != write_size) {
        fprintf(stderr, "Write error: %s\n", strerror(errno));
        return false;
      }

      // Reset next_out and avail_out.
      strm->next_out = outbuf;
      strm->avail_out = sizeof(outbuf);
    }

    // Normally the return value of lzma_code() will be LZMA_OK
    // until everything has been encoded.
    if (ret != LZMA_OK) {
      // Once everything has been encoded successfully, the
      // return value of lzma_code() will be LZMA_STREAM_END.
      //
      // It is important to check for LZMA_STREAM_END. Do not
      // assume that getting ret != LZMA_OK would mean that
      // everything has gone well.
      if (ret == LZMA_STREAM_END) return true;

      // It's not LZMA_OK nor LZMA_STREAM_END,
      // so it must be an error code. See lzma/base.h
      // (src/liblzma/api/lzma/base.h in the source package
      // or e.g. /usr/include/lzma/base.h depending on the
      // install prefix) for the list and documentation of
      // possible values. Most values listen in lzma_ret
      // enumeration aren't possible in this example.
      const char* msg;
      switch (ret) {
        case LZMA_MEM_ERROR:
          msg = "Memory allocation failed";
          break;

        case LZMA_DATA_ERROR:
          // This error is returned if the compressed
          // or uncompressed size get near 8 EiB
          // (2^63 bytes) because that's where the .xz
          // file format size limits currently are.
          // That is, the possibility of this error
          // is mostly theoretical unless you are doing
          // something very unusual.
          //
          // Note that strm->total_in and strm->total_out
          // have nothing to do with this error. Changing
          // those variables won't increase or decrease
          // the chance of getting this error.
          msg = "File size limits exceeded";
          break;

        default:
          // This is most likely LZMA_PROG_ERROR, but
          // if this program is buggy (or liblzma has
          // a bug), it may be e.g. LZMA_BUF_ERROR or
          // LZMA_OPTIONS_ERROR too.
          //
          // It is inconvenient to have a separate
          // error message for errors that should be
          // impossible to occur, but knowing the error
          // code is important for debugging. That's why
          // it is good to print the error code at least
          // when there is no good error message to show.
          msg = "Unknown error, possibly a bug";
          break;
      }

      wxLogMessage(_T("LZMA Encoder error: %s (error code %u)\n"), msg, ret);
      return false;
    }
  }
}
#endif

static bool CompressChart(wxString in, wxString out) {
#ifdef OCPN_USE_LZMA
  FILE* infile = fopen(in.mb_str(), "rb");
  if (!infile) return false;

  FILE* outfile = fopen(out.mb_str(), "wb");
  if (!outfile) {
    fclose(infile);
    return false;
  }

  lzma_stream strm = LZMA_STREAM_INIT;
  bool success = false;
  if (lzma_easy_encoder(&strm, LZMA_PRESET_DEFAULT, LZMA_CHECK_CRC64) ==
      LZMA_OK)
    success = compress(&strm, infile, outfile);

  lzma_end(&strm);
  fclose(infile);
  fclose(outfile);

  return success;
#endif
  return false;
}

void options::OnButtonmigrateClick(wxCommandEvent& event) {
#ifdef __OCPN__ANDROID__

  // Run the chart migration assistant
  g_migrateDialog =
      new MigrateAssistantDialog(gFrame, true);  // skip Folder scan
  g_migrateDialog->SetSize(gFrame->GetSize());
  g_migrateDialog->Centre();
  g_migrateDialog->Raise();
  g_migrateDialog->ShowModal();
#endif
}

void options::OnButtoncompressClick(wxCommandEvent& event) {
#if 0
  wxArrayInt pListBoxSelections;
  long item = -1;
  for (;;) {
    item = pActiveChartsList->GetNextItem(item, wxLIST_NEXT_ALL,
                                          wxLIST_STATE_SELECTED);
    if (item == -1) break;
    //pListBoxSelections.Add((int)item);
    item = -1;  // Restart
  }

  if (OCPNMessageBox(this, _("Compression will alter chart files on disk.\n\
This may make them incompatible with other programs or older versions of OpenCPN.\n\
Compressed charts may take slightly longer to load and display on some systems.\n\
They can be decompressed again using unxz or 7 zip programs."),
                     _("OpenCPN Warning"),
                     wxYES | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING) !=
      wxID_YES)
    return;

  wxArrayString filespecs;
  filespecs.Add("*.kap");
  filespecs.Add("*.KAP");
  filespecs.Add("*.000");

  // should we verify we are in a cm93 directory for these?
  filespecs.Add("*.A"), filespecs.Add("*.B"), filespecs.Add("*.C"),
      filespecs.Add("*.D");
  filespecs.Add("*.E"), filespecs.Add("*.F"), filespecs.Add("*.G"),
      filespecs.Add("*.Z");

  wxGenericProgressDialog prog1(
      _("OpenCPN Compress Charts"), wxEmptyString,
      filespecs.GetCount() * pListBoxSelections.GetCount() + 1, this,
      wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME |
          wxPD_REMAINING_TIME | wxPD_CAN_SKIP);

  //    Make sure the dialog is big enough to be readable
  wxSize sz = prog1.GetSize();
  sz.x = gFrame->GetClientSize().x * 8 / 10;
  prog1.SetSize(sz);

  wxArrayString charts;
  for (unsigned int i = 0; i < pListBoxSelections.GetCount(); i++) {
    wxString dirname = pActiveChartsList->GetItemText(pListBoxSelections[i]);
    if (dirname.IsEmpty()) continue;
    //    This is a fix for OSX, which appends EOL to results of
    //    GetLineText()
    while ((dirname.Last() == wxChar(_T('\n'))) ||
           (dirname.Last() == wxChar(_T('\r'))))
      dirname.RemoveLast();

    if (!wxDir::Exists(dirname)) continue;

    wxDir dir(dirname);
    wxArrayString FileList;
    for (unsigned int j = 0; j < filespecs.GetCount(); j++) {
      dir.GetAllFiles(dirname, &FileList, filespecs[j]);
      bool skip = false;
      prog1.Update(i * filespecs.GetCount() + j, dirname + filespecs[j], &skip);
      if (skip) return;
    }

    for (unsigned int j = 0; j < FileList.GetCount(); j++)
      charts.Add(FileList[j]);
  }
  prog1.Hide();

  if (charts.GetCount() == 0) {
    OCPNMessageBox(this, _("No charts found to compress."), _("OpenCPN Info"));
    return;
  }

  // TODO: make this use threads
  unsigned long total_size = 0, total_compressed_size = 0, count = 0;
  wxGenericProgressDialog prog(
      _("OpenCPN Compress Charts"), wxEmptyString, charts.GetCount() + 1, this,
      wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME |
          wxPD_REMAINING_TIME | wxPD_CAN_SKIP);

  prog.SetSize(sz);

  for (unsigned int i = 0; i < charts.GetCount(); i++) {
    bool skip = false;
    prog.Update(i, charts[i], &skip);
    if (skip) break;

    wxString compchart = charts[i] + _T(".xz");
    if (CompressChart(charts[i], compchart)) {
      total_size += wxFileName::GetSize(charts[i]).ToULong();
      total_compressed_size += wxFileName::GetSize(compchart).ToULong();
      wxRemoveFile(charts[i]);
      count++;
    }
  }

  // report statistics
  double total_size_mb = total_size / 1024.0 / 1024.0;
  double total_compressed_size_mb = total_compressed_size / 1024.0 / 1024.0;
  OCPNMessageBox(
      this,
      wxString::Format(_("compressed %ld charts\nfrom %.1fMB to %.1fMB\nsaved "
                         "%.1fMB (%.1f%%)"),
                       count, total_size_mb, total_compressed_size_mb,
                       total_size_mb - total_compressed_size_mb,
                       (1 - total_compressed_size_mb / total_size_mb) * 100.0),
      _("OpenCPN Info"));

  UpdateWorkArrayFromTextCtl();

  if (m_pWorkDirList) {
    pActiveChartsList->DeleteAllItems();
    for (size_t id = 0; id < m_pWorkDirList->GetCount(); id++) {
      wxString dirname = m_pWorkDirList->Item(id).fullpath;
      wxListItem li;
      li.SetId(id);
      li.SetAlign(wxLIST_FORMAT_LEFT);
      li.SetText(dirname);
      li.SetColumn(0);
      long idx = pActiveChartsList->InsertItem(li);
    }
  }

  k_charts |= CHANGE_CHARTS;

  pScanCheckBox->Disable();

  event.Skip();
#endif
}

void options::OnDebugcheckbox1Click(wxCommandEvent& event) { event.Skip(); }

void options::OnCancelClick(wxCommandEvent& event) {
  m_pListbook->ChangeSelection(0);

  lastWindowPos = GetPosition();
  lastWindowSize = GetSize();

  if (g_pi_manager) g_pi_manager->CloseAllPlugInPanels((int)wxCANCEL);

  pConfig->SetPath("/Settings");
  pConfig->Write("OptionsSizeX", lastWindowSize.x);
  pConfig->Write("OptionsSizeY", lastWindowSize.y);

  int rv = 0;
  if (m_bForceNewToolbaronCancel) rv = TOOLBAR_CHANGED;
  EndModal(rv);
}

void options::OnClose(wxCloseEvent& event) {
  //      PlugIns may have added panels
  if (g_pi_manager) g_pi_manager->CloseAllPlugInPanels((int)wxOK);

  m_pListbook->ChangeSelection(0);

  lastWindowPos = GetPosition();
  lastWindowSize = GetSize();

  pConfig->SetPath("/Settings");
  pConfig->Write("OptionsSizeX", lastWindowSize.x);
  pConfig->Write("OptionsSizeY", lastWindowSize.y);

  EndModal(0);
}

void options::OnFontChoice(wxCommandEvent& event) {
  wxString sel_text_element = m_itemFontElementListBox->GetStringSelection();

  wxFont* pif = FontMgr::Get().GetFont(sel_text_element);
  wxColour init_color = FontMgr::Get().GetFontColor(sel_text_element);

  m_textSample->SetFont(*pif);
  m_textSample->SetForegroundColour(init_color);
  m_itemBoxSizerFontPanel->Layout();
  event.Skip();
}

void options::OnChooseFont(wxCommandEvent& event) {
#ifdef __OCPN__ANDROID__
  androidDisableRotation();
#endif

  wxString sel_text_element = m_itemFontElementListBox->GetStringSelection();
  wxFontData font_data;

  wxFont* pif = FontMgr::Get().GetFont(sel_text_element);
  wxColour init_color = FontMgr::Get().GetFontColor(sel_text_element);

  wxFontData init_font_data;
  if (pif) init_font_data.SetInitialFont(*pif);
  init_font_data.SetColour(init_color);

#ifdef __WXGTK__
  wxFontDialog dg(this, init_font_data);
#else
  wxFontDialog dg(pParent, init_font_data);
#endif

  wxFont* qFont = dialogFont;
  dg.SetFont(*qFont);

#ifdef __WXQT__
  // Make sure that font dialog will fit on the screen without scrolling
  // We do this by setting the dialog font size "small enough" to show "n" lines
  wxSize proposed_size = GetSize();
  float n_lines = 30;
  float font_size = dialogFont->GetPointSize();

  if ((proposed_size.y / font_size) < n_lines) {
    float new_font_size = proposed_size.y / n_lines;
    wxFont* smallFont = new wxFont(*dialogFont);
    smallFont->SetPointSize(new_font_size);
    dg.SetFont(*smallFont);
  }
#endif

  if (g_bresponsive) {
    dg.SetSize(GetSize());
    dg.Centre();
  }

  int retval = dg.ShowModal();
  if (wxID_CANCEL != retval) {
    font_data = dg.GetFontData();
    wxFont font = font_data.GetChosenFont();
    wxFont* psfont = new wxFont(font);
    wxColor color = font_data.GetColour();
    FontMgr::Get().SetFont(sel_text_element, psfont, color);
    pParent->UpdateAllFonts();
    m_bfontChanged = true;
    OnFontChoice(event);
  }

#ifdef __OCPN__ANDROID__
  androidEnableRotation();
#endif

  event.Skip();
}

#if defined(__WXGTK__) || defined(__WXQT__)
void options::OnChooseFontColor(wxCommandEvent& event) {
  wxString sel_text_element = m_itemFontElementListBox->GetStringSelection();

  wxColourData colour_data;

  wxFont* pif = FontMgr::Get().GetFont(sel_text_element);
  wxColour init_color = FontMgr::Get().GetFontColor(sel_text_element);

#ifdef __OCPN__ANDROID__
  androidDisableRotation();

  unsigned int cco = 0;
  cco |= 0xff;
  cco = cco << 8;
  cco |= init_color.Red();
  cco = cco << 8;
  cco |= init_color.Green();
  cco = cco << 8;
  cco |= init_color.Blue();
  unsigned int cc = androidColorPicker(cco);

  wxColor cn;
  unsigned char blue = (unsigned char)cc % 256;
  unsigned char green = (unsigned char)(cc >> 8) % 256;
  ;
  unsigned char red = (unsigned char)(cc >> 16) % 256;
  cn.Set(red, green, blue);

  FontMgr::Get().SetFont(sel_text_element, pif, cn);

  pParent->UpdateAllFonts();
  m_bfontChanged = true;

  androidEnableRotation();
#else
  wxScrolledWindow* sw =
      new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(400, 400));

  wxColourData init_colour_data;
  init_colour_data.SetColour(init_color);

  wxColourDialog dg(sw, &init_colour_data);

  int retval = dg.ShowModal();
  if (wxID_CANCEL != retval) {
    colour_data = dg.GetColourData();

    wxColor color = colour_data.GetColour();
    FontMgr::Get().SetFont(sel_text_element, pif, color);

    pParent->UpdateAllFonts();
    m_bfontChanged = true;
    OnFontChoice(event);
  }

  sw->Destroy();
#endif
  event.Skip();
}
#endif

void options::OnChartsPageChange(wxListbookEvent& event) {
  unsigned int i = event.GetSelection();

  UpdateChartDirList();

  //    User selected Chart Groups Page?
  //    If so, build the remaining UI elements
  if (2 == i) {  // 2 is the index of "Chart Groups" page
    if (!groupsPanel->m_UIcomplete) groupsPanel->CompletePanel();

    if (!groupsPanel->m_settingscomplete) {
      ::wxBeginBusyCursor();
      groupsPanel->CompleteInitialSettings();
      ::wxEndBusyCursor();
    } else if (!groupsPanel->m_treespopulated) {
      groupsPanel->PopulateTrees();
      groupsPanel->m_treespopulated = TRUE;
    }
  } else if (1 == i) {  // Vector charts panel
    LoadS57();
    if (!m_bVectorInit) {
      SetInitialVectorSettings();
      UpdateOptionsUnits();  // sets depth values, overriding defaults
    }
  }

  event.Skip();  // Allow continued event processing
}

void options::OnPageChange(wxListbookEvent& event) {
  DoOnPageChange(event.GetSelection());
}

void options::OnSubNBPageChange(wxNotebookEvent& event) {
  // In the case where wxNotebooks are nested, we need to identify the subpage
  // But otherwise do nothing
  if (event.GetEventObject()) {
    if (dynamic_cast<wxWindow*>(event.GetEventObject())) {
      wxWindow* win = dynamic_cast<wxWindow*>(event.GetEventObject());
      wxWindow* parent = dynamic_cast<wxWindow*>(win->GetParent());
      if (dynamic_cast<wxNotebook*>(parent)) {
        lastSubPage = event.GetSelection();
        return;
      }
      if (dynamic_cast<wxListbook*>(parent)) {
        lastSubPage = event.GetSelection();
        return;
      }
    }
  }
}

void options::OnTopNBPageChange(wxNotebookEvent& event) {
  DoOnPageChange(event.GetSelection());
}

void options::DoOnPageChange(size_t page) {
  unsigned int i = page;

  //  Sometimes there is a (-1) page selected.
  if (page > 10) return;

  lastPage = i;

#ifndef __OCPN__ANDROID__
  if (0 == i) {  // Display
    if (m_sconfigSelect_single) m_sconfigSelect_single->Refresh(true);
    if (m_sconfigSelect_twovertical) m_sconfigSelect_twovertical->Refresh(true);
  }
#endif

  //    User selected Chart Page?
  //    If so, build the "Charts" page variants
  if (1 == i) {  // 2 is the index of "Charts" page
    k_charts = VISIT_CHARTS;
    UpdateChartDirList();
  }

  else if (m_pageUI == i) {  // 5 is the index of "User Interface" page
    if (!m_itemLangListBox) return;
#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)

    if (!m_bVisitLang) {
      ::wxBeginBusyCursor();

      int current_language = plocale_def_lang->GetLanguage();
      wxString current_sel = wxLocale::GetLanguageName(current_language);

      current_sel = GetOCPNKnownLanguage(g_locale);

      int nLang = sizeof(lang_list) / sizeof(int);

#ifdef __WXMSW__
      // always add us english
      m_itemLangListBox->Append(_T("English (U.S.)"));

      wxString lang_dir = g_Platform->GetSharedDataDir() + _T("share/locale/");
      for (int it = 1; it < nLang; it++) {
        if (wxLocale::IsAvailable(lang_list[it])) {
          wxLocale ltest(lang_list[it], 0);
          ltest.AddCatalog(_T("opencpn"));
          if (!ltest.IsLoaded(_T("opencpn"))) continue;

          // Defaults
          wxString loc_lang_name = wxLocale::GetLanguageName(lang_list[it]);
          wxString widgets_lang_name = loc_lang_name;
          wxString lang_canonical =
              wxLocale::GetLanguageInfo(lang_list[it])->CanonicalName;

          //  Make opencpn substitutions
          wxString lang_suffix;
          loc_lang_name = GetOCPNKnownLanguage(lang_canonical, lang_suffix);

          //  Look explicitely to see if .mo is available
          wxString test_dir = lang_dir + lang_suffix;
          if (!wxDir::Exists(test_dir)) continue;

          m_itemLangListBox->Append(loc_lang_name);
        }
      }
#else
      wxArrayString lang_array;

      // always add us english
      lang_array.Add(_T("en_US"));

      for (int it = 0; it < nLang; it++) {
        {
          wxLog::EnableLogging(
              FALSE);  // avoid "Cannot set locale to..." log message

          wxLocale ltest(lang_list[it], 0);
#if wxCHECK_VERSION(2, 9, 0)
#ifdef __WXGTK__
          ltest.AddCatalogLookupPathPrefix(
              wxStandardPaths::Get().GetInstallPrefix() +
              _T( "/share/locale" ));
#endif
#endif
          ltest.AddCatalog(_T("opencpn"));

          wxLog::EnableLogging(TRUE);

          if (ltest.IsLoaded(_T("opencpn"))) {
            wxString s0 =
                wxLocale::GetLanguageInfo(lang_list[it])->CanonicalName;
            wxString sl = wxLocale::GetLanguageName(lang_list[it]);
            if (wxNOT_FOUND == lang_array.Index(s0)) lang_array.Add(s0);
          }
        }
      }

      for (unsigned int i = 0; i < lang_array.GetCount(); i++) {
        //  Make opencpn substitutions
        wxString loc_lang_name = GetOCPNKnownLanguage(lang_array[i]);
        m_itemLangListBox->Append(loc_lang_name);
      }
#endif

      // BUGBUG
      //  Remember that wxLocale ctor has the effect of changing the system
      //  locale, including the "C" libraries.
      //  It should then also happen that the locale should be switched back to
      //  ocpn initial load setting
      //  upon the dtor of the above wxLocale instantiations....
      //  wxWidgets may do so internally, but there seems to be no effect upon
      //  the system libraries, so that
      //  functions like strftime() do not revert to the correct locale setting.
      //  Also, the catalog for the application is not reloaded by the ctor, so
      //  we must reload them directly
      //  So as workaround, we reset the locale explicitely.

      delete plocale_def_lang;
      plocale_def_lang = new wxLocale(current_language);

      setlocale(LC_NUMERIC, "C");
      plocale_def_lang->AddCatalog(_T("opencpn"));

      m_itemLangListBox->SetStringSelection(current_sel);

      //      Initialize Language tab
      const wxLanguageInfo* pli = wxLocale::FindLanguageInfo(g_locale);
      if (pli) {
        wxString clang = pli->Description;
        //                        m_itemLangListBox->SetValue(clang);
      }

      m_bVisitLang = TRUE;

      ::wxEndBusyCursor();
    }
#endif
  } else if (m_pagePlugins == i) {  // 7 is the index of "Plugins" page
    // load the disabled plugins finally because the user might want to enable
    // them
    auto loader = PluginLoader::getInstance();
    if (LoadAllPlugIns(false)) {
      delete m_pPlugInCtrl;
      m_pPlugInCtrl = NULL;
      delete m_PluginCatalogMgrPanel;
      m_PluginCatalogMgrPanel = NULL;
      itemBoxSizerPanelPlugins->Clear();
    }

    if (!m_pPlugInCtrl) {
      //      Build the PlugIn Manager Panel
      ::wxBeginBusyCursor();

      m_pPlugInCtrl =
          new PluginListPanel(itemPanelPlugins, ID_PANELPIM, wxDefaultPosition,
                              wxDefaultSize,
                              PluginLoader::getInstance()->GetPlugInArray());
      m_pPlugInCtrl->SetScrollRate(m_scrollRate, m_scrollRate);
      itemBoxSizerPanelPlugins->Add(m_pPlugInCtrl, 01,
                                    wxEXPAND | wxGROW | wxALL, 4);
      if (g_pi_manager) g_pi_manager->SetListPanelPtr(m_pPlugInCtrl);

      m_PluginCatalogMgrPanel = new CatalogMgrPanel(itemPanelPlugins);
      m_PluginCatalogMgrPanel->SetListPanelPtr(m_pPlugInCtrl);

      itemBoxSizerPanelPlugins->Add(m_PluginCatalogMgrPanel, 0,
                                    wxEXPAND | wxALL, 4);
      itemBoxSizerPanelPlugins->Layout();

      //  Update the PlugIn page to reflect the state of individual selections
      m_pPlugInCtrl->UpdateSelections();

      ::wxEndBusyCursor();

      wxDEFINE_EVENT(EVT_COMPAT_OS_CHANGE, wxCommandEvent);
      GlobalVar<wxString> compat_os(&g_compatOS);
    }
    k_plugins = TOOLBAR_CHANGED;
  }
}

// void options::OnNMEASourceChoice( wxCommandEvent& event )
//{
/*TODO
    int i = event.GetSelection();
    wxString src( m_itemNMEAListBox->GetString( i ) );
    if( ( src.Upper().Find( _T("GPSD") ) != wxNOT_FOUND )
            || ( src.Upper().Find( _T("LIBGPS") ) != wxNOT_FOUND ) ) {
        m_itemNMEA_TCPIP_StaticBox->Enable();
        m_itemNMEA_TCPIP_Source->Enable();

        m_itemNMEA_TCPIP_Source->Clear();
        m_itemNMEA_TCPIP_Source->WriteText( _T("localhost") ); // default

        wxString source;
        source = *pNMEADataSource;
        if( source.Upper().StartsWith( _T("GPSD") ) ||
   source.Upper().StartsWith( _T("LIBGPS") ) ) {
            wxString ip;
            ip = source.AfterFirst( ':' );

            if( ip.Len() ) {
                m_itemNMEA_TCPIP_Source->Clear();
                m_itemNMEA_TCPIP_Source->WriteText( ip );
            }
        }
    } else {
        m_itemNMEA_TCPIP_StaticBox->Disable();
        m_itemNMEA_TCPIP_Source->Disable();
    }
*/
//}

wxString options::SelectSoundFile() {
  wxString sound_dir = g_Platform->GetSharedDataDir();
  sound_dir.Append(_T("sounds"));
  wxString sel_file;
  int response;

#ifndef __OCPN__ANDROID__
  wxFileDialog* popenDialog = new wxFileDialog(
      NULL, _("Select Sound File"), sound_dir, wxEmptyString,
      _T("WAV files (*.wav)|*.wav|All files (*.*)|*.*"), wxFD_OPEN);
  if (g_bresponsive)
    popenDialog = g_Platform->AdjustFileDialogFont(this, popenDialog);

  response = popenDialog->ShowModal();
  sel_file = popenDialog->GetPath();
  delete popenDialog;

#else
  response =
      g_Platform->DoFileSelectorDialog(this, &sel_file, _("Select Sound File"),
                                       sound_dir, wxEmptyString, wxT("*.*"));
#endif

  if (response == wxID_OK)
    return g_Platform->NormalizePath(sel_file);
  else
    return _T("");
}

void options::OnButtonSelectSound(wxCommandEvent& event) {
  wxString sel_file = SelectSoundFile();

  if (!sel_file.IsEmpty()) {
    g_sAIS_Alert_Sound_File = g_Platform->NormalizePath(sel_file);
  }
}

void options::OnButtonTestSound(wxCommandEvent& event) {
  auto sound = SoundFactory();
  auto cmd_sound = dynamic_cast<SystemCmdSound*>(sound);
  if (cmd_sound) cmd_sound->SetCmd(g_CmdSoundString.mb_str());
  sound->SetFinishedCallback([sound](void*) { delete sound; });
  sound->Load(g_sAIS_Alert_Sound_File, g_iSoundDeviceIndex);
  sound->Play();
}

wxString GetOCPNKnownLanguage(wxString lang_canonical, wxString& lang_dir) {
  wxString return_string;
  wxString dir_suffix;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)

  if (lang_canonical == _T("en_US")) {
    dir_suffix = _T("en");
    return_string = wxString("English (U.S.)", wxConvUTF8);
  } else if (lang_canonical == _T("cs_CZ")) {
    dir_suffix = _T("cs");
    return_string = wxString("Čeština", wxConvUTF8);
  } else if (lang_canonical == _T("da_DK")) {
    dir_suffix = _T("da");
    return_string = wxString("Dansk", wxConvUTF8);
  } else if (lang_canonical == _T("de_DE")) {
    dir_suffix = _T("de");
    return_string = wxString("Deutsch", wxConvUTF8);
  } else if (lang_canonical == _T("et_EE")) {
    dir_suffix = _T("et");
    return_string = wxString("Eesti", wxConvUTF8);
  } else if (lang_canonical == _T("es_ES")) {
    dir_suffix = _T("es");
    return_string = wxString("Español", wxConvUTF8);
  } else if (lang_canonical == _T("fr_FR")) {
    dir_suffix = _T("fr");
    return_string = wxString("Français", wxConvUTF8);
  } else if (lang_canonical == _T("it_IT")) {
    dir_suffix = _T("it");
    return_string = wxString("Italiano", wxConvUTF8);
  } else if (lang_canonical == _T("nl_NL")) {
    dir_suffix = _T("nl");
    return_string = wxString("Nederlands", wxConvUTF8);
  } else if (lang_canonical == _T("pl_PL")) {
    dir_suffix = _T("pl");
    return_string = wxString("Polski", wxConvUTF8);
  } else if (lang_canonical == _T("pt_PT")) {
    dir_suffix = _T("pt_PT");
    return_string = wxString("Português", wxConvUTF8);
  } else if (lang_canonical == _T("pt_BR")) {
    dir_suffix = _T("pt_BR");
    return_string = wxString("Português Brasileiro", wxConvUTF8);
  } else if (lang_canonical == _T("ru_RU")) {
    dir_suffix = _T("ru");
    return_string = wxString("Русский", wxConvUTF8);
  } else if (lang_canonical == _T("sv_SE")) {
    dir_suffix = _T("sv");
    return_string = wxString("Svenska", wxConvUTF8);
  } else if (lang_canonical == _T("fi_FI")) {
    dir_suffix = _T("fi_FI");
    return_string = wxString("Suomi", wxConvUTF8);
  } else if (lang_canonical == _T("nb_NO")) {
    dir_suffix = _T("nb_NO");
    return_string = wxString("Norsk", wxConvUTF8);
  } else if (lang_canonical == _T("tr_TR")) {
    dir_suffix = _T("tr_TR");
    return_string = wxString("Türkçe", wxConvUTF8);
  } else if (lang_canonical == _T("el_GR")) {
    dir_suffix = _T("el_GR");
    return_string = wxString("Ελληνικά", wxConvUTF8);
  } else if (lang_canonical == _T("hu_HU")) {
    dir_suffix = _T("hu_HU");
    return_string = wxString("Magyar", wxConvUTF8);
  } else if (lang_canonical == _T("zh_TW")) {
    dir_suffix = _T("zh_TW");
    return_string = wxString("正體字", wxConvUTF8);
  } else if (lang_canonical == _T("zh_CN")) {
    dir_suffix = _T("zh_CN");
    return_string = wxString("Simplified Chinese", wxConvUTF8);
  } else if (lang_canonical == _T("ca_ES")) {
    dir_suffix = _T("ca_ES");
    return_string = wxString("Catalan", wxConvUTF8);
  } else if (lang_canonical == _T("gl_ES")) {
    dir_suffix = _T("gl_ES");
    return_string = wxString("Galician", wxConvUTF8);
  } else if (lang_canonical == _T("ja_JP")) {
    dir_suffix = _T("ja_JP");
    return_string = wxString("Japanese", wxConvUTF8);
  } else if (lang_canonical == _T("ar_SA")) {
    dir_suffix = _T("ar_SA");
    return_string = wxString("Arabic", wxConvUTF8);
  } else if (lang_canonical == _T("vi_VN")) {
    dir_suffix = _T("vi_VN");
    return_string = wxString("Vietnamese", wxConvUTF8);
  } else if (lang_canonical == _T("he_IL")) {
    dir_suffix = _T("he_IL");
    return_string = wxString("Hebrew", wxConvUTF8);
  } else if (lang_canonical == _T("en_GB")) {
    dir_suffix = _T("en_GB");
    return_string = wxString("English (U.K.)", wxConvUTF8);
  } else {
    dir_suffix = lang_canonical;
    const wxLanguageInfo* info = wxLocale::FindLanguageInfo(lang_canonical);
    if (info)
      return_string = info->Description;
    else
      return_string = lang_canonical;
  }

  lang_dir = dir_suffix;
#endif
  return return_string;
}

wxString GetOCPNKnownLanguage(const wxString lang_canonical) {
  wxString lang_dir;
  return GetOCPNKnownLanguage(lang_canonical, lang_dir);
}

ChartGroupArray* ChartGroupsUI::CloneChartGroupArray(ChartGroupArray* s) {
  ChartGroupArray* d = new ChartGroupArray;
  for (unsigned int i = 0; i < s->GetCount(); i++) {
    ChartGroup* psg = s->Item(i);
    ChartGroup* pdg = new ChartGroup;
    pdg->m_group_name = psg->m_group_name;
    pdg->m_element_array.reserve(psg->m_element_array.size());

    for (auto& elem : psg->m_element_array)
      pdg->m_element_array.push_back(elem);

    d->Add(pdg);
  }
  return d;
}

void ChartGroupsUI::EmptyChartGroupArray(ChartGroupArray* s) {
  if (!s) return;

  // ChartGroups don't need anything special for delete, just calling the
  // destructor is enough.
  WX_CLEAR_ARRAY(*s);
}

//    Chart Groups dialog implementation
BEGIN_EVENT_TABLE(ChartGroupsUI, wxEvtHandler)
EVT_TREE_ITEM_EXPANDED(wxID_TREECTRL, ChartGroupsUI::OnNodeExpanded)
EVT_NOTEBOOK_PAGE_CHANGED(
    wxID_ANY,
    ChartGroupsUI::OnGroupPageChange)  // This should work under Windows :-(
END_EVENT_TABLE()

ChartGroupsUI::ChartGroupsUI(wxWindow* parent) {
  m_GroupSelectedPage = -1;
  m_pActiveChartsTree = 0;
  pParent = parent;
  lastSelectedCtl = NULL;
  allAvailableCtl = NULL;
  defaultAllCtl = NULL;
  iFont = NULL;
  m_pAddButton = NULL;
  m_pRemoveButton = NULL;
  m_pDeleteGroupButton = NULL;
  m_pNewGroupButton = NULL;
  m_pGroupArray = NULL;
  m_GroupNB = NULL;
  modified = false;
  m_UIcomplete = false;
  m_treespopulated = false;
  dialogFont = GetOCPNScaledFont(_("Dialog"));
}

ChartGroupsUI::~ChartGroupsUI(void) {
  m_DirCtrlArray.Clear();
  m_GroupNB->Disconnect(
      wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
      wxNotebookEventHandler(ChartGroupsUI::OnGroupPageChange), NULL, this);

  delete iFont;
}

void ChartGroupsUI::SetInitialSettings(void) {
  m_settingscomplete = FALSE;
  m_treespopulated = FALSE;
}

void ChartGroupsUI::PopulateTrees(void) {
  //    Fill in the "Active chart" tree control
  //    from the options dialog "Active Chart Directories" list
  wxArrayString dir_array;
  int nDir = m_db_dirs.GetCount();
  for (int i = 0; i < nDir; i++) {
    wxString dirname = m_db_dirs[i].fullpath;
    if (!dirname.IsEmpty()) dir_array.Add(dirname);
  }

  PopulateTreeCtrl(allAvailableCtl->GetTreeCtrl(), dir_array, wxColour(0, 0, 0),
                   dialogFont);
  m_pActiveChartsTree = allAvailableCtl->GetTreeCtrl();

  //    Fill in the Page 0 tree control
  //    from the options dialog "Active Chart Directories" list
  wxArrayString dir_array0;
  int nDir0 = m_db_dirs.GetCount();
  for (int i = 0; i < nDir0; i++) {
    wxString dirname = m_db_dirs[i].fullpath;
    if (!dirname.IsEmpty()) dir_array0.Add(dirname);
  }
  PopulateTreeCtrl(defaultAllCtl->GetTreeCtrl(), dir_array0,
                   wxColour(128, 128, 128), iFont);
}

void ChartGroupsUI::CompleteInitialSettings(void) {
  PopulateTrees();

  BuildNotebookPages(m_pGroupArray);

  m_panel->GetSizer()->Layout();

  m_settingscomplete = TRUE;
  m_treespopulated = TRUE;
}

void ChartGroupsUI::PopulateTreeCtrl(wxTreeCtrl* ptc,
                                     const wxArrayString& dir_array,
                                     const wxColour& col, wxFont* pFont) {
  ptc->DeleteAllItems();

  wxDirItemData* rootData = new wxDirItemData(_T("Dummy"), _T("Dummy"), TRUE);
  wxString rootName;
  rootName = _T("Dummy");
  wxTreeItemId m_rootId = ptc->AddRoot(rootName, 3, -1, rootData);
  ptc->SetItemHasChildren(m_rootId);

  wxString dirname;
  int nDir = dir_array.GetCount();
  for (int i = 0; i < nDir; i++) {
    wxString dirname = dir_array[i];
    if (!dirname.IsEmpty()) {
      wxDirItemData* dir_item = new wxDirItemData(dirname, dirname, TRUE);
      wxTreeItemId id = ptc->AppendItem(m_rootId, dirname, 0, -1, dir_item);

      // wxWidgets bug workaraound (Ticket #10085)
      ptc->SetItemText(id, dirname);
      if (pFont) ptc->SetItemFont(id, *pFont);

        // On MacOS, use the default system dialog color, to honor Dark mode.
#ifndef __WXOSX__
      ptc->SetItemTextColour(id, col);
#endif
      ptc->SetItemHasChildren(id);
    }
  }
}

void ChartGroupsUI::OnInsertChartItem(wxCommandEvent& event) {
  wxString insert_candidate = allAvailableCtl->GetPath();
  if (!insert_candidate.IsEmpty()) {
    if (m_DirCtrlArray.GetCount()) {
      wxGenericDirCtrl* pDirCtrl = (m_DirCtrlArray[m_GroupSelectedPage]);
      ChartGroup* pGroup = m_pGroupArray->Item(m_GroupSelectedPage - 1);
      if (pDirCtrl) {
        wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();
        if (ptree) {
          if (ptree->IsEmpty()) {
            wxDirItemData* rootData =
                new wxDirItemData(wxEmptyString, wxEmptyString, TRUE);
            wxString rootName = _T("Dummy");
            wxTreeItemId rootId = ptree->AddRoot(rootName, 3, -1, rootData);

            ptree->SetItemHasChildren(rootId);
          }

          wxTreeItemId root_Id = ptree->GetRootItem();
          wxDirItemData* dir_item =
              new wxDirItemData(insert_candidate, insert_candidate, TRUE);
          wxTreeItemId id =
              ptree->AppendItem(root_Id, insert_candidate, 0, -1, dir_item);
          if (wxDir::Exists(insert_candidate)) ptree->SetItemHasChildren(id);
        }

        pGroup->m_element_array.push_back({insert_candidate});
      }
    }
  }
  modified = TRUE;
  allAvailableCtl->GetTreeCtrl()->UnselectAll();
  m_pAddButton->Disable();

  wxGenericDirCtrl* pDirCtrl = (m_DirCtrlArray[m_GroupSelectedPage]);
  if (pDirCtrl) {
    wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();
    if (ptree) ptree->Refresh();
  }
}

void ChartGroupsUI::OnRemoveChartItem(wxCommandEvent& event) {
  if (m_DirCtrlArray.GetCount()) {
    wxGenericDirCtrl* pDirCtrl = (m_DirCtrlArray[m_GroupSelectedPage]);
    ChartGroup* pGroup = m_pGroupArray->Item(m_GroupSelectedPage - 1);

    if (pDirCtrl) {
      wxString sel_item = pDirCtrl->GetPath();

      wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();
      if (ptree && ptree->GetCount()) {
        wxTreeItemId id = ptree->GetSelection();
        lastDeletedItem = id;
        if (id.IsOk()) {
          wxString branch_adder;
          int group_item_index =
              FindGroupBranch(pGroup, ptree, id, &branch_adder);
          if (group_item_index >= 0) {
            ChartGroupElement &pelement =
                pGroup->m_element_array[group_item_index];
            bool b_duplicate = FALSE;
            for (unsigned int k = 0; k < pelement.m_missing_name_array.size();
                 k++) {
              if (pelement.m_missing_name_array[k] == sel_item) {
                b_duplicate = TRUE;
                break;
              }
            }
            if (!b_duplicate) {
              pelement.m_missing_name_array.Add(sel_item);
            }

            //    Special case...
            //    If the selection is a branch itself,
            //    Then delete from the tree, and delete from the group
            if (branch_adder == _T("")) {
              ptree->Delete(id);
              pGroup->m_element_array.erase(pGroup->m_element_array.begin() +
                                            group_item_index);
            } else {
              ptree->SetItemTextColour(id, wxColour(128, 128, 128));
              //   what about toggle back?
            }
          }
        }
        modified = TRUE;
        lastSelectedCtl->Unselect();
        lastSelectedCtl = 0;
        m_pRemoveButton->Disable();

        wxGenericDirCtrl* pDirCtrl = (m_DirCtrlArray[m_GroupSelectedPage]);
        if (pDirCtrl) {
          wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();
          if (ptree) ptree->Refresh();
        }
      }
    }
  }
  event.Skip();
}

void ChartGroupsUI::OnGroupPageChange(wxNotebookEvent& event) {
  m_GroupSelectedPage = event.GetSelection();
  allAvailableCtl->GetTreeCtrl()->UnselectAll();
  if (lastSelectedCtl) {
    lastSelectedCtl->UnselectAll();
    lastSelectedCtl = 0;
  }
  m_pRemoveButton->Disable();
  m_pAddButton->Disable();

  // Disable delete option for "All Charts" group
  if (m_GroupSelectedPage == 0) {
    if (m_pDeleteGroupButton) m_pDeleteGroupButton->Disable();
  } else {
    if (m_pDeleteGroupButton) m_pDeleteGroupButton->Enable();
  }
}

void ChartGroupsUI::OnAvailableSelection(wxTreeEvent& event) {
  wxObject* evtObj = event.GetEventObject();
  if (allAvailableCtl && (evtObj == allAvailableCtl->GetTreeCtrl())) {
    wxTreeItemId item = allAvailableCtl->GetTreeCtrl()->GetSelection();
    if (item && item.IsOk() && m_GroupSelectedPage > 0) {
      m_pAddButton->Enable();
    } else {
      m_pAddButton->Disable();
    }
  } else {
    lastSelectedCtl = dynamic_cast<wxTreeCtrl*>(evtObj);
    if (!lastSelectedCtl) goto out;
    wxTreeItemId item = lastSelectedCtl->GetSelection();
    if (item && item.IsOk() && m_GroupSelectedPage > 0) {
      // We need a trick for wxGTK here, since it gives us a Selection
      // event with the just deleted element after OnRemoveChartItem()
      wxGenericDirCtrl* dirCtrl =
          dynamic_cast<wxGenericDirCtrl*>(lastSelectedCtl->GetParent());
      if (!dirCtrl) goto out;
      wxString itemPath = dirCtrl->GetPath();
      if (!itemPath.IsEmpty()) m_pRemoveButton->Enable();
    } else {
      m_pRemoveButton->Disable();
    }
  }

out:
  event.Skip();
}

void ChartGroupsUI::OnNewGroup(wxCommandEvent& event) {
  wxTextEntryDialog* pd = new wxTextEntryDialog();
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  pd->SetFont(*qFont);

  pd->Create(m_panel, _("Enter Group Name"), _("New Chart Group"));

#ifdef __OCPN__ANDROID__
  androidDisableRotation();
#endif

  if (pd->ShowModal() == wxID_OK) {
    if (pd->GetValue().Length()) {
      AddEmptyGroupPage(pd->GetValue());
      ChartGroup* pGroup = new ChartGroup;
      pGroup->m_group_name = pd->GetValue();
      m_pGroupArray->Add(pGroup);

      m_GroupSelectedPage =
          m_GroupNB->GetPageCount() - 1;  // select the new page
      m_GroupNB->ChangeSelection(m_GroupSelectedPage);
      m_pDeleteGroupButton->Enable();
      modified = TRUE;
    }
  }
  delete pd;

#ifdef __OCPN__ANDROID__
  androidEnableRotation();
#endif
}

void ChartGroupsUI::OnDeleteGroup(wxCommandEvent& event) {
  if (0 != m_GroupSelectedPage) {
    m_DirCtrlArray.RemoveAt(m_GroupSelectedPage);
    if (m_pGroupArray) m_pGroupArray->RemoveAt(m_GroupSelectedPage - 1);
    m_GroupNB->DeletePage(m_GroupSelectedPage);
    modified = TRUE;
  }
  if (m_GroupSelectedPage <= 0) {
    m_pAddButton->Disable();
    m_pDeleteGroupButton->Disable();
  }
}

int ChartGroupsUI::FindGroupBranch(ChartGroup* pGroup, wxTreeCtrl* ptree,
                                   wxTreeItemId item, wxString* pbranch_adder) {
  wxString branch_name;
  wxString branch_adder;

  wxTreeItemId current_node = item;
  while (current_node.IsOk()) {
    wxTreeItemId parent_node = ptree->GetItemParent(current_node);
    if (!parent_node) break;

    if (parent_node == ptree->GetRootItem()) {
      branch_name = ptree->GetItemText(current_node);
      break;
    }

    branch_adder.Prepend(ptree->GetItemText(current_node));
    branch_adder.Prepend(wxString(wxFILE_SEP_PATH));

    current_node = ptree->GetItemParent(current_node);
  }

  // Find the index and element pointer of the target branch in the Group
  unsigned int target_item_index = -1;

  for (unsigned int i = 0; i < pGroup->m_element_array.size(); i++) {
    const wxString &target = pGroup->m_element_array[i].m_element_name;
    if (branch_name == target) {
      target_item_index = i;
      break;
    }
  }

  if (pbranch_adder) *pbranch_adder = branch_adder;

  return target_item_index;
}

void ChartGroupsUI::OnNodeExpanded(wxTreeEvent& event) {
  wxTreeItemId node = event.GetItem();

  if (m_GroupSelectedPage <= 0) return;
  wxGenericDirCtrl* pDirCtrl = (m_DirCtrlArray[m_GroupSelectedPage]);
  ChartGroup* pGroup = m_pGroupArray->Item(m_GroupSelectedPage - 1);
  if (!pDirCtrl) return;

  wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();
  wxString branch_adder;
  int target_item_index = FindGroupBranch(pGroup, ptree, node, &branch_adder);
  if (target_item_index < 0) return;
  const ChartGroupElement& target_element =
      pGroup->m_element_array[target_item_index];
  const wxString &branch_name = target_element.m_element_name;

  // Walk the children of the expanded node, marking any items which appear in
  // the "missing" list
  if (!target_element.m_missing_name_array.GetCount()) return;
  wxString full_root = branch_name;
  full_root += branch_adder;
  full_root += wxString(wxFILE_SEP_PATH);

  wxTreeItemIdValue cookie;
  wxTreeItemId child = ptree->GetFirstChild(node, cookie);
  while (child.IsOk()) {
    wxString target_string = full_root;
    target_string += ptree->GetItemText(child);

    for (unsigned int k = 0;
         k < target_element.m_missing_name_array.GetCount(); k++) {
      if (target_element.m_missing_name_array[k] == target_string) {
        ptree->SetItemTextColour(child, wxColour(128, 128, 128));
        break;
      }
    }
    child = ptree->GetNextChild(node, cookie);
  }
}

void ChartGroupsUI::BuildNotebookPages(ChartGroupArray* pGroupArray) {
  ClearGroupPages();

  for (unsigned int i = 0; i < pGroupArray->GetCount(); i++) {
    ChartGroup* pGroup = pGroupArray->Item(i);
    wxTreeCtrl* ptc = AddEmptyGroupPage(pGroup->m_group_name);

    wxString itemname;
    int nItems = pGroup->m_element_array.size();
    for (int i = 0; i < nItems; i++) {
      const wxString &itemname = pGroup->m_element_array[i].m_element_name;
      if (!itemname.IsEmpty()) {
        wxDirItemData* dir_item = new wxDirItemData(itemname, itemname, TRUE);
        wxTreeItemId id =
            ptc->AppendItem(ptc->GetRootItem(), itemname, 0, -1, dir_item);

        if (wxDir::Exists(itemname)) ptc->SetItemHasChildren(id);
      }
    }
  }
}

wxTreeCtrl* ChartGroupsUI::AddEmptyGroupPage(const wxString& label) {
  wxGenericDirCtrl* GroupDirCtl =
      new wxGenericDirCtrl(m_GroupNB, wxID_ANY, _T("TESTDIR"));
  m_GroupNB->AddPage(GroupDirCtl, label);

  wxTreeCtrl* ptree = GroupDirCtl->GetTreeCtrl();
  ptree->DeleteAllItems();

  wxDirItemData* rootData =
      new wxDirItemData(wxEmptyString, wxEmptyString, TRUE);
  wxString rootName = _T("Dummy");
  wxTreeItemId rootId = ptree->AddRoot(rootName, 3, -1, rootData);
  ptree->SetItemHasChildren(rootId);

  m_DirCtrlArray.Add(GroupDirCtl);

  return ptree;
}

void ChartGroupsUI::ClearGroupPages() {
  if (m_GroupNB->GetPageCount() == 0) return;

  for (unsigned int i = m_GroupNB->GetPageCount() - 1; i > 0; i--) {
    m_DirCtrlArray.RemoveAt(i);
    m_GroupNB->DeletePage(i);
  }
}

void options::OnInsertTideDataLocation(wxCommandEvent& event) {
  wxString sel_file;
  int response = wxID_CANCEL;

#ifndef __OCPN__ANDROID__
  wxFileDialog* popenDialog = new wxFileDialog(
      NULL, _("Select Tide/Current Data"), g_TCData_Dir, wxT(""),
      wxT("Tide/Current Data files (*.IDX; *.TCD)|*.IDX;*.idx;*.TCD;*.tcd|All ")
          wxT("files (*.*)|*.*"),
      wxFD_OPEN);
  if (g_bresponsive)
    popenDialog = g_Platform->AdjustFileDialogFont(this, popenDialog);

  response = popenDialog->ShowModal();
  sel_file = popenDialog->GetPath();
  delete popenDialog;

#else
  wxString path;
  response = g_Platform->DoFileSelectorDialog(this, &path,
                                              _("Select Tide/Current Data"),
                                              g_TCData_Dir, _T(""), wxT("*.*"));
  sel_file = path;
#endif

  if (response == wxID_OK) {
    wxListItem li;
    int id = tcDataSelected->GetItemCount();  // next index
    li.SetId(id);
    long idx = tcDataSelected->InsertItem(li);
    tcDataSelected->SetItem(id, 0, g_Platform->NormalizePath(sel_file));

    //    Record the currently selected directory for later use
    wxFileName fn(sel_file);
    wxString data_dir = fn.GetPath();
    g_TCData_Dir = g_Platform->NormalizePath(data_dir);
  }
}

void options::OnRemoveTideDataLocation(wxCommandEvent& event) {
  long item = -1;
  for (;;) {
    item = tcDataSelected->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    if (item == -1) break;
    tcDataSelected->DeleteItem(item);
    item = -1;  // Restart
  }
}

void options::OnValChange(wxCommandEvent& event) { event.Skip(); }

void options::OnScanBTClick(wxCommandEvent& event) {
  if (m_BTscanning)
    StopBTScan();
  else {
    m_btNoChangeCounter = 0;
    m_btlastResultCount = 0;

    m_BTScanTimer.Start(1000, wxTIMER_CONTINUOUS);
    g_Platform->startBluetoothScan();
    m_BTscanning = 1;
    if (m_buttonScanBT) {
      m_buttonScanBT->SetLabel(_("Stop Scan"));
    }
  }
}

void options::onBTScanTimer(wxTimerEvent& event) {
  if (m_BTscanning) {
    m_BTscanning++;

    m_BTscan_results = g_Platform->getBluetoothScanResults();

    m_choiceBTDataSources->Clear();
    m_choiceBTDataSources->Append(m_BTscan_results[0]);  // scan status

    unsigned int i = 1;
    while ((i + 1) < m_BTscan_results.GetCount()) {
      wxString item1 = m_BTscan_results[i] + _T(";");
      wxString item2 = m_BTscan_results.Item(i + 1);
      m_choiceBTDataSources->Append(item1 + item2);

      i += 2;
    }

    if (m_BTscan_results.GetCount() > 1) {
      m_choiceBTDataSources->SetSelection(1);
    }

    //  Watch for changes.  When no changes occur after n seconds, stop the scan
    if (m_btNoChangeCounter > 5) StopBTScan();

    if ((int)m_BTscan_results.GetCount() == m_btlastResultCount)
      m_btNoChangeCounter++;
    else
      m_btNoChangeCounter = 0;

    m_btlastResultCount = m_BTscan_results.GetCount();

    // Absolute fallback
    if (m_BTscanning >= 15) {
      StopBTScan();
    }
  } else {
  }
  return;
}

void options::StopBTScan(void) {
  m_BTScanTimer.Stop();

  g_Platform->stopBluetoothScan();

  m_BTscanning = 0;

  if (m_buttonScanBT) {
    m_buttonScanBT->SetLabel(_("BT Scan"));
    m_buttonScanBT->Enable();
  }
}

void options::OnConnValChange(wxCommandEvent& event) {
  connectionsaved = FALSE;
  event.Skip();
}

void options::OnTypeSerialSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToSerial();
}

void options::OnTypeNetSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToNet();
}

void options::OnTypeGPSSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToGPS();
}

void options::OnTypeBTSelected(wxCommandEvent& event) {
  OnConnValChange(event);
  SetNMEAFormToBT();
}

void options::OnUploadFormatChange(wxCommandEvent& event) {
  if (event.GetEventObject() == m_cbGarminUploadHost && event.IsChecked())
    m_cbFurunoGP3X->SetValue(FALSE);
  else if (event.GetEventObject() == m_cbFurunoGP3X && event.IsChecked())
    m_cbGarminUploadHost->SetValue(FALSE);

  OnConnValChange(event);
  event.Skip();
}

void options::ShowNMEACommon(bool visible) {
  m_rbTypeSerial->Show(visible);
  m_rbTypeNet->Show(visible);
  if (m_rbTypeInternalGPS) m_rbTypeInternalGPS->Show(visible);
  if (m_rbTypeInternalBT) m_rbTypeInternalBT->Show(visible);
  m_rbIAccept->Show(visible);
  m_rbIIgnore->Show(visible);
  m_rbOAccept->Show(visible);
  m_rbOIgnore->Show(visible);
  m_tcInputStc->Show(visible);
  m_btnInputStcList->Show(visible);
  m_tcOutputStc->Show(visible);
  m_btnOutputStcList->Show(visible);
  m_cbInput->Show(visible);
  m_cbOutput->Show(visible);
  m_stPrecision->Show(visible);
  m_choicePrecision->Show(visible);
  m_choicePriority->Show(visible);
  m_stPriority->Show(visible);
  m_stPrecision->Show(visible);
  m_stTalkerIdText->Show(visible);
  m_TalkerIdText->Show(visible);
  m_cbCheckCRC->Show(visible);
  m_cbCheckSKDiscover->Show(visible);
  m_ButtonSKDiscover->Show(visible);
  if (visible) {
    const bool output = m_cbOutput->IsChecked();
    m_stPrecision->Enable(output);
    m_choicePrecision->Enable(output);
    m_stTalkerIdText->Enable(output);
    m_TalkerIdText->Enable(output);
  } else {
    sbSizerOutFilter->SetDimension(0, 0, 0, 0);
    sbSizerInFilter->SetDimension(0, 0, 0, 0);
    sbSizerConnectionProps->SetDimension(0, 0, 0, 0);
    m_sbConnEdit->SetLabel(_T(""));
  }
  m_bNMEAParams_shown = visible;
}

void options::ShowNMEANet(bool visible) {
  m_stNetAddr->Show(visible);
  m_tNetAddress->Show(visible);
  m_stNetPort->Show(visible);
  m_tNetPort->Show(visible);
  m_stNetProto->Show(visible);
  m_rbNetProtoSignalK->Show(visible);
  m_rbNetProtoGPSD->Show(visible);
  m_rbNetProtoTCP->Show(visible);
  m_rbNetProtoUDP->Show(visible);
  m_stNetComment->Show(visible);
  m_tNetComment->Show(visible);
}

void options::ShowNMEASerial(bool visible) {
  m_stSerBaudrate->Show(visible);
  m_choiceBaudRate->Show(visible);
  m_stSerPort->Show(visible);
  m_comboPort->Show(visible);
  m_stSerProtocol->Show(visible);
  m_choiceSerialProtocol->Show(visible);
  m_cbGarminHost->Show(visible);
  m_stSerialComment->Show(visible);
  m_tSerialComment->Show(visible);
}

void options::ShowNMEAGPS(bool visible) {}

void options::ShowNMEABT(bool visible) {
  if (visible) {
    if (m_buttonScanBT) m_buttonScanBT->Show();
    if (m_stBTPairs) m_stBTPairs->Show();
    if (m_choiceBTDataSources) {
      if (m_choiceBTDataSources->GetCount() > 1)
        m_choiceBTDataSources->SetSelection(1);
      m_choiceBTDataSources->Show();
    }
  } else {
    if (m_buttonScanBT) m_buttonScanBT->Hide();
    if (m_stBTPairs) m_stBTPairs->Hide();
    if (m_choiceBTDataSources) m_choiceBTDataSources->Hide();
  }
  m_tcOutputStc->Show(visible);
  m_btnOutputStcList->Show(visible);
  m_cbOutput->Show(visible);
}

void options::SetNMEAFormToSerial(void) {
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(TRUE);

  m_pNMEAForm->FitInside();
  // Fit();
  RecalculateSize();
  SetDSFormRWStates();
}

void options::SetNMEAFormToNet(void) {
  ShowNMEACommon(TRUE);
  ShowNMEANet(TRUE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  m_pNMEAForm->FitInside();
  // Fit();
  RecalculateSize();
  SetDSFormRWStates();
}

void options::SetNMEAFormToGPS(void) {
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(TRUE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  m_pNMEAForm->FitInside();
  // Fit();
  RecalculateSize();
  SetDSFormRWStates();
}

void options::SetNMEAFormToBT(void) {
  m_rbNetProtoUDP->SetValue(true);
  ShowNMEACommon(TRUE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(TRUE);
  ShowNMEASerial(FALSE);
  m_pNMEAForm->FitInside();
  // Fit();
  RecalculateSize();
  SetDSFormRWStates();
}

void options::ClearNMEAForm(void) {
  ShowNMEACommon(FALSE);
  ShowNMEANet(FALSE);
  ShowNMEAGPS(FALSE);
  ShowNMEABT(FALSE);
  ShowNMEASerial(FALSE);
  m_pNMEAForm->FitInside();
  // Fit();
  RecalculateSize();
}

wxString StringArrayToString(wxArrayString arr) {
  wxString ret = wxEmptyString;
  for (size_t i = 0; i < arr.Count(); i++) {
    if (i > 0) ret.Append(_T(","));
    ret.Append(arr[i]);
  }
  return ret;
}

void options::SetDSFormOptionVizStates(void) {
  m_cbInput->Show();
  m_cbOutput->Show();
  m_cbCheckCRC->Show();
  m_stPrecision->Show();
  m_choicePrecision->Show();
  m_stTalkerIdText->Show();
  m_TalkerIdText->Show();
  sbSizerInFilter->GetStaticBox()->Show();
  m_rbIAccept->Show();
  m_rbIIgnore->Show();
  sbSizerOutFilter->GetStaticBox()->Show();
  m_rbOAccept->Show();
  m_rbOIgnore->Show();
  m_tcInputStc->Show();
  m_btnInputStcList->Show();
  m_tcOutputStc->Show();
  m_btnOutputStcList->Show();
  m_cbCheckSKDiscover->Show();
  m_ButtonSKDiscover->Show();
  m_StaticTextSKServerStatus->Show();

  if (m_rbTypeSerial->GetValue()) {
    m_cbCheckSKDiscover->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();

  } else if (m_rbNetProtoGPSD->GetValue()) {
    m_cbCheckSKDiscover->Hide();
    m_cbInput->Hide();
    m_cbOutput->Hide();
    sbSizerOutFilter->GetStaticBox()->Hide();
    m_rbOAccept->Hide();
    m_rbOIgnore->Hide();
    m_tcOutputStc->Hide();
    m_btnOutputStcList->Hide();
    m_stPrecision->Hide();
    m_choicePrecision->Hide();
    m_stTalkerIdText->Hide();
    m_TalkerIdText->Hide();
    m_cbCheckSKDiscover->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();

  } else if (m_rbNetProtoSignalK->GetValue()) {
    // here
    m_cbInput->Hide();
    m_cbOutput->Hide();
    m_cbCheckCRC->Hide();
    m_stPrecision->Hide();
    m_choicePrecision->Hide();
    m_stTalkerIdText->Hide();
    m_TalkerIdText->Hide();
    sbSizerInFilter->GetStaticBox()->Hide();
    m_rbIAccept->Hide();
    m_rbIIgnore->Hide();
    sbSizerOutFilter->GetStaticBox()->Hide();
    m_rbOAccept->Hide();
    m_rbOIgnore->Hide();
    m_tcInputStc->Hide();
    m_btnInputStcList->Hide();
    m_tcOutputStc->Hide();
    m_btnOutputStcList->Hide();

  } else {
    m_cbCheckSKDiscover->Hide();
    m_ButtonSKDiscover->Hide();
    m_StaticTextSKServerStatus->Hide();
  }
}

void options::SetDSFormRWStates(void) {
  if (m_rbTypeSerial->GetValue()) {
    m_cbInput->Enable(FALSE);
    m_cbOutput->Enable(TRUE);
    m_rbOAccept->Enable(TRUE);
    m_rbOIgnore->Enable(TRUE);
    m_btnOutputStcList->Enable(TRUE);
  } else if (m_rbNetProtoGPSD->GetValue()) {
    if (m_tNetPort->GetValue() == wxEmptyString)
      m_tNetPort->SetValue(_T("2947"));
    m_cbInput->SetValue(TRUE);
    m_cbInput->Enable(FALSE);
    m_cbOutput->SetValue(FALSE);
    m_cbOutput->Enable(FALSE);
    m_rbOAccept->Enable(FALSE);
    m_rbOIgnore->Enable(FALSE);
    m_btnOutputStcList->Enable(FALSE);
  } else if (m_rbNetProtoSignalK->GetValue()) {
    if (m_tNetPort->GetValue() == wxEmptyString)
      m_tNetPort->SetValue(_T("3000"));
    m_cbInput->SetValue(TRUE);
    m_cbInput->Enable(FALSE);
    m_cbOutput->SetValue(FALSE);
    m_cbOutput->Enable(FALSE);
    m_rbOAccept->Enable(FALSE);
    m_rbOIgnore->Enable(FALSE);
    UpdateDiscoverStatus(wxEmptyString);

  } else {
    if (m_tNetPort->GetValue() == wxEmptyString)
      m_tNetPort->SetValue(_T("10110"));
    m_cbInput->Enable(TRUE);
    m_cbOutput->Enable(TRUE);
    m_rbOAccept->Enable(TRUE);
    m_rbOIgnore->Enable(TRUE);
    m_btnOutputStcList->Enable(TRUE);
  }

  SetDSFormOptionVizStates();
  m_pNMEAForm->FitInside();
  // Fit();
  // RecalculateSize();
}

void options::SetConnectionParams(ConnectionParams* cp) {
  if (wxNOT_FOUND == m_comboPort->FindString(cp->Port))
    m_comboPort->Append(cp->Port);

  m_comboPort->Select(m_comboPort->FindString(cp->Port));

  m_cbCheckCRC->SetValue(cp->ChecksumCheck);
  m_cbGarminHost->SetValue(cp->Garmin);
  m_cbInput->SetValue(cp->IOSelect != DS_TYPE_OUTPUT);
  m_cbOutput->SetValue(cp->IOSelect != DS_TYPE_INPUT);
  m_cbCheckSKDiscover->SetValue(cp->AutoSKDiscover);

  if (cp->InputSentenceListType == WHITELIST)
    m_rbIAccept->SetValue(TRUE);
  else
    m_rbIIgnore->SetValue(TRUE);
  if (cp->OutputSentenceListType == WHITELIST)
    m_rbOAccept->SetValue(TRUE);
  else
    m_rbOIgnore->SetValue(TRUE);
  m_tcInputStc->SetValue(StringArrayToString(cp->InputSentenceList));
  m_tcOutputStc->SetValue(StringArrayToString(cp->OutputSentenceList));
  m_choiceBaudRate->Select(
      m_choiceBaudRate->FindString(wxString::Format(_T( "%d" ), cp->Baudrate)));
  m_choiceSerialProtocol->Select(cp->Protocol);  // TODO
  m_choicePriority->Select(
      m_choicePriority->FindString(wxString::Format(_T( "%d" ), cp->Priority)));
  m_tNetAddress->SetValue(cp->NetworkAddress);

  if (cp->NetworkPort == 0)
    m_tNetPort->SetValue(wxEmptyString);
  else
    m_tNetPort->SetValue(wxString::Format(wxT("%i"), cp->NetworkPort));

  if (cp->NetProtocol == TCP)
    m_rbNetProtoTCP->SetValue(TRUE);
  else if (cp->NetProtocol == UDP)
    m_rbNetProtoUDP->SetValue(TRUE);
  else if (cp->NetProtocol == GPSD)
    m_rbNetProtoGPSD->SetValue(TRUE);
  else if (cp->NetProtocol == SIGNALK)
    m_rbNetProtoSignalK->SetValue(TRUE);
  else
    m_rbNetProtoGPSD->SetValue(TRUE);

  if (cp->Type == SERIAL) {
    m_rbTypeSerial->SetValue(TRUE);
    SetNMEAFormToSerial();
  } else if (cp->Type == NETWORK) {
    m_rbTypeNet->SetValue(TRUE);
    SetNMEAFormToNet();
  } else if (cp->Type == INTERNAL_GPS) {
    if (m_rbTypeInternalGPS) m_rbTypeInternalGPS->SetValue(TRUE);
    SetNMEAFormToGPS();
  } else if (cp->Type == INTERNAL_BT) {
    if (m_rbTypeInternalBT) m_rbTypeInternalBT->SetValue(TRUE);
    SetNMEAFormToBT();

    // Preset the source selector
    wxString bts = cp->NetworkAddress + _T(";") + cp->GetPortStr();
    m_choiceBTDataSources->Clear();
    m_choiceBTDataSources->Append(bts);
    m_choiceBTDataSources->SetSelection(0);
  } else
    ClearNMEAForm();

  if (cp->Type == SERIAL)
    m_tSerialComment->SetValue(cp->UserComment);
  else if (cp->Type == NETWORK)
    m_tNetComment->SetValue(cp->UserComment);

  m_connection_enabled = cp->bEnabled;

  // Reset touch flag
  connectionsaved = true;
}

void options::SetDefaultConnectionParams(void) {
  if (m_comboPort && !m_comboPort->IsListEmpty()) {
    m_comboPort->Select(0);
    m_comboPort->SetValue(wxEmptyString);  // These two broke it
  }
  m_cbCheckCRC->SetValue(TRUE);
  m_cbGarminHost->SetValue(FALSE);
  m_cbInput->SetValue(TRUE);
  m_cbOutput->SetValue(FALSE);
  m_rbIAccept->SetValue(TRUE);
  m_rbOAccept->SetValue(TRUE);
  m_tcInputStc->SetValue(wxEmptyString);
  m_tcOutputStc->SetValue(wxEmptyString);
  m_choiceBaudRate->Select(m_choiceBaudRate->FindString(_T( "4800" )));
  //    m_choiceSerialProtocol->Select( cp->Protocol ); // TODO
  m_choicePriority->Select(m_choicePriority->FindString(_T( "1" )));

  m_tNetAddress->SetValue(_T("0.0.0.0"));

  m_tNetComment->SetValue(wxEmptyString);
  m_tSerialComment->SetValue(wxEmptyString);

  bool bserial = TRUE;
#ifdef __WXGTK__
  bserial = FALSE;
#endif

#ifdef __WXOSX__
  bserial = FALSE;
#endif

#ifdef __OCPN__ANDROID__
  if (m_rbTypeInternalGPS) {
    m_rbTypeInternalGPS->SetValue(true);
    SetNMEAFormToGPS();
  } else {
    m_rbTypeNet->SetValue(true);
    SetNMEAFormToNet();
  }

#else
  m_rbTypeSerial->SetValue(bserial);
  m_rbTypeNet->SetValue(!bserial);
  bserial ? SetNMEAFormToSerial() : SetNMEAFormToNet();
#endif

  m_connection_enabled = TRUE;

  // Reset touch flag
  connectionsaved = false;
}

bool options::SortSourceList(void) {
  if (g_pConnectionParams->Count() < 2) return false;

  std::vector<int> ivec;
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) ivec.push_back(i);

  bool did_sort = false;
  bool did_swap = true;
  while (did_swap) {
    did_swap = false;
    for (size_t j = 1; j < ivec.size(); j++) {
      ConnectionParams* c1 = g_pConnectionParams->Item(ivec[j]);
      ConnectionParams* c2 = g_pConnectionParams->Item(ivec[j - 1]);

      if (c1->Priority > c2->Priority) {
        int t = ivec[j - 1];
        ivec[j - 1] = ivec[j];
        ivec[j] = t;
        did_swap = true;
        did_sort = true;
      }
    }
  }

  // if(did_sort)
  {
    boxSizerConnections = new wxBoxSizer(wxVERTICAL);
    m_scrollWinConnections->SetSizer(boxSizerConnections);

    for (size_t i = 0; i < ivec.size(); i++) {
      ConnectionParamsPanel* pPanel =
          g_pConnectionParams->Item(ivec[i])->m_optionsPanel;
      boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxALL, 0);
    }
  }

  return did_sort;
}

void options::FillSourceList(void) {
  m_buttonRemove->Enable(FALSE);

  // Add new panels as necessary
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
    if (!g_pConnectionParams->Item(i)->m_optionsPanel) {
      ConnectionParamsPanel* pPanel = new ConnectionParamsPanel(
          m_scrollWinConnections, wxID_ANY, wxDefaultPosition, wxDefaultSize,
          g_pConnectionParams->Item(i), this);
      pPanel->SetSelected(false);
      boxSizerConnections->Add(pPanel, 0, wxEXPAND | wxALL, 0);
      g_pConnectionParams->Item(i)->m_optionsPanel = pPanel;
    } else {
      g_pConnectionParams->Item(i)->m_optionsPanel->Update(
          g_pConnectionParams->Item(i));
    }
  }
  SortSourceList();
  m_scrollWinConnections->Layout();

  mSelectedConnection = NULL;
  m_buttonAdd->Enable(true);
  m_buttonAdd->Show();
}

void options::UpdateSourceList(bool bResort) {
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
    ConnectionParams* cp = g_pConnectionParams->Item(i);
    ConnectionParamsPanel* panel = cp->m_optionsPanel;
    if (panel) panel->Update(g_pConnectionParams->Item(i));
  }

  if (bResort) {
    SortSourceList();
  }

  m_scrollWinConnections->Layout();
}

void options::OnAddDatasourceClick(wxCommandEvent& event) {
  //  Unselect all panels
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++)
    g_pConnectionParams->Item(i)->m_optionsPanel->SetSelected(false);

  connectionsaved = FALSE;
  SetDefaultConnectionParams();

  m_sbConnEdit->SetLabel(_("Configure new connection"));

  m_buttonRemove->Hide();  // Disable();
  m_buttonAdd->Hide();     // Disable();

  SetDSFormRWStates();

  RecalculateSize();

  //  Scroll the panel to allow the user to see more of the NMEA parameter
  //  settings area
  wxPoint buttonPosition = m_buttonAdd->GetPosition();
  m_pNMEAForm->Scroll(-1, buttonPosition.y / m_scrollRate);
}

void options::OnRemoveDatasourceClick(wxCommandEvent& event) {
  if (mSelectedConnection) {
    // Find the index
    int index = -1;
    ConnectionParams* cp = NULL;
    for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
      cp = g_pConnectionParams->Item(i);
      if (mSelectedConnection == cp) {
        index = i;
        break;
      }
    }

    if ((index >= 0) && (cp)) {
      delete g_pConnectionParams->Item(index)->m_optionsPanel;
      g_pConnectionParams->RemoveAt(index);

      DataStream* pds_existing = g_pMUX->FindStream(cp->GetDSPort());
      if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);
      // delete mSelectedConnection->m_optionsPanel;
      mSelectedConnection = NULL;
    }
  }

  //  Mark connection deleted
  m_rbTypeSerial->SetValue(TRUE);
  m_comboPort->SetValue(_T( "Deleted" ));

  FillSourceList();
  ShowNMEACommon(FALSE);
  ShowNMEANet(FALSE);
  ShowNMEASerial(FALSE);
}

void options::OnSelectDatasource(wxListEvent& event) {
  SetConnectionParams(g_pConnectionParams->Item(event.GetData()));
  m_buttonRemove->Enable();
  m_buttonRemove->Show();
  event.Skip();
}

void options::SetSelectedConnectionPanel(ConnectionParamsPanel* panel) {
  //  Only one panel can be selected at any time
  //  Clear any selections

  if (mSelectedConnection && mSelectedConnection->m_optionsPanel)
    mSelectedConnection->m_optionsPanel->SetSelected(false);

  if (panel) {
    mSelectedConnection = panel->m_pConnectionParams;
    panel->SetSelected(true);
    SetConnectionParams(mSelectedConnection);
    m_buttonRemove->Enable();
    m_buttonRemove->Show();
    m_buttonAdd->Disable();
    m_sbConnEdit->SetLabel(_("Edit Selected Connection"));

  } else {
    mSelectedConnection = NULL;
    m_buttonRemove->Disable();
    m_buttonAdd->Enable();
    m_buttonAdd->Show();
    m_sbConnEdit->SetLabel(_T(""));
    ClearNMEAForm();
  }
}

void options::OnDiscoverButton(wxCommandEvent& event) {
  wxString ip;
  int port;
  std::string serviceIdent =
      std::string("_signalk-ws._tcp.local.");  // Works for node.js server

  g_Platform->ShowBusySpinner();

  if (SignalKDataStream::DiscoverSKServer(serviceIdent, ip, port,
                                          1))  // 1 second scan
  {
    m_tNetAddress->SetValue(ip);
    m_tNetPort->SetValue(wxString::Format(wxT("%i"), port));
    UpdateDiscoverStatus(_("Signal K server available."));
  } else {
    UpdateDiscoverStatus(_("Signal K server not found."));
  }
  g_Platform->HideBusySpinner();

  event.Skip();
}

void options::UpdateDiscoverStatus(wxString stat) {
  m_StaticTextSKServerStatus->SetLabel(stat);
}

void options::OnBtnIStcs(wxCommandEvent& event) {
  const ListType type = m_rbIAccept->GetValue() ? WHITELIST : BLACKLIST;
  const wxArrayString list =
      wxStringTokenize(m_tcInputStc->GetValue(), _T( "," ));
  SentenceListDlg dlg(this, FILTER_INPUT, type, list);

  if (dlg.ShowModal() == wxID_OK) m_tcInputStc->SetValue(dlg.GetSentences());
}

void options::OnBtnOStcs(wxCommandEvent& event) {
  const ListType type = m_rbOAccept->GetValue() ? WHITELIST : BLACKLIST;
  const wxArrayString list =
      wxStringTokenize(m_tcOutputStc->GetValue(), _T( "," ));
  SentenceListDlg dlg(this, FILTER_OUTPUT, type, list);

  if (dlg.ShowModal() == wxID_OK) m_tcOutputStc->SetValue(dlg.GetSentences());
}

void options::OnNetProtocolSelected(wxCommandEvent& event) {
  if (m_rbNetProtoGPSD->GetValue()) {
    if (m_tNetPort->GetValue().IsEmpty()) m_tNetPort->SetValue(_T( "2947" ));
  } else if (m_rbNetProtoUDP->GetValue()) {
    if (m_tNetPort->GetValue().IsEmpty()) m_tNetPort->SetValue(_T( "10110" ));
    if (m_tNetAddress->GetValue().IsEmpty())
      m_tNetAddress->SetValue(_T( "0.0.0.0" ));
  } else if (m_rbNetProtoSignalK->GetValue()) {
    if (m_tNetPort->GetValue().IsEmpty()) m_tNetPort->SetValue(_T( "8375" ));
  } else if (m_rbNetProtoTCP->GetValue()) {
    if (m_tNetPort->GetValue().IsEmpty()) m_tNetPort->SetValue(_T( "10110" ));
  }

  SetDSFormRWStates();
  OnConnValChange(event);
}

void options::OnRbAcceptInput(wxCommandEvent& event) { OnConnValChange(event); }
void options::OnRbIgnoreInput(wxCommandEvent& event) { OnConnValChange(event); }

void options::OnRbOutput(wxCommandEvent& event) { OnConnValChange(event); }

void options::OnCbInput(wxCommandEvent& event) { OnConnValChange(event); }

void options::OnCbOutput(wxCommandEvent& event) {
  OnConnValChange(event);
  const bool checked = m_cbOutput->IsChecked();
  m_stPrecision->Enable(checked);
  m_choicePrecision->Enable(checked);
  m_stTalkerIdText->Enable(checked);
  m_TalkerIdText->Enable(checked);
}

SentenceListDlg::SentenceListDlg(wxWindow* parent, FilterDirection dir,
                                 ListType type, const wxArrayString& list)
    : wxDialog(parent, wxID_ANY, _("Sentence Filter"), wxDefaultPosition,
               wxSize(280, 420)),
      m_type(type),
      m_dir(dir),
      m_sentences(NMEA0183().GetRecognizedArray()) {
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* secondSizer = new wxBoxSizer(wxHORIZONTAL);
  wxStaticBox* pclbBox = new wxStaticBox(this, wxID_ANY, GetBoxLabel());
  wxStaticBoxSizer* stcSizer = new wxStaticBoxSizer(pclbBox, wxVERTICAL);
  m_clbSentences = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition,
                                      wxDefaultSize, m_sentences);
  wxBoxSizer* btnEntrySizer = new wxBoxSizer(wxVERTICAL);
  wxButton* btnCheckAll = new wxButton(this, wxID_ANY, _("Select All"));
  wxButton* btnClearAll = new wxButton(this, wxID_ANY, _("Clear All"));
  wxButton* btnAdd = new wxButton(this, wxID_ANY, _("Add"));
  m_btnDel = new wxButton(this, wxID_ANY, _("Delete"));
  m_btnDel->Disable();
  wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
  wxButton* btnOK = new wxButton(this, wxID_OK);
  wxButton* btnCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));

  secondSizer->Add(stcSizer, 1, wxALL | wxEXPAND, 5);
  stcSizer->Add(m_clbSentences, 1, wxALL | wxEXPAND, 5);
  btnEntrySizer->Add(btnCheckAll, 0, wxALL, 5);
  btnEntrySizer->Add(btnClearAll, 0, wxALL, 5);
  btnEntrySizer->AddSpacer(1);
  btnEntrySizer->Add(btnAdd, 0, wxALL, 5);
  btnEntrySizer->Add(m_btnDel, 0, wxALL, 5);
  secondSizer->Add(btnEntrySizer, 0, wxALL | wxEXPAND, 5);
  mainSizer->Add(secondSizer, 1, wxEXPAND, 5);
  btnSizer->AddButton(btnOK);
  btnSizer->AddButton(btnCancel);
  btnSizer->Realize();
  mainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

  SetSizer(mainSizer);
  mainSizer->SetSizeHints(this);
  Centre();

  // Connect Events
  btnAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                  wxCommandEventHandler(SentenceListDlg::OnAddClick), NULL,
                  this);
  m_btnDel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                    wxCommandEventHandler(SentenceListDlg::OnDeleteClick), NULL,
                    this);
  m_clbSentences->Connect(wxEVT_COMMAND_LISTBOX_SELECTED,
                          wxCommandEventHandler(SentenceListDlg::OnCLBSelect),
                          NULL, this);
  btnCheckAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(SentenceListDlg::OnCheckAllClick),
                       NULL, this);
  btnClearAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(SentenceListDlg::OnClearAllClick),
                       NULL, this);

  Populate(list);
}

wxString SentenceListDlg::GetBoxLabel(void) const {
  if (m_dir == FILTER_OUTPUT)
    return m_type == WHITELIST ? _("Transmit sentences") : _("Drop sentences");
  else
    return m_type == WHITELIST ? _("Accept only sentences")
                               : _("Ignore sentences");
}

void SentenceListDlg::Populate(const wxArrayString& list) {
  if (m_dir == FILTER_OUTPUT) {
    m_sentences.Add(_T("ECRMB"));
    m_sentences.Add(_T("ECRMC"));
    m_sentences.Add(_T("ECAPB"));
  }
  m_sentences.Add(_T("AIVDM"));
  m_sentences.Add(_T("AIVDO"));
  m_sentences.Add(_T("FRPOS"));
  m_sentences.Add(_T("CD"));
  m_clbSentences->Clear();
  m_clbSentences->InsertItems(m_sentences, 0);

  if (list.Count() == 0) {
    for (size_t i = 0; i < m_clbSentences->GetCount(); ++i)
      m_clbSentences->Check(i, m_type == WHITELIST);
  } else {
    m_clbSentences->InsertItems(list, m_sentences.GetCount());
    for (size_t i = 0; i < list.Count(); ++i) {
      int item = m_clbSentences->FindString(list[i]);
      if (item != wxNOT_FOUND) m_clbSentences->Check(item);
    }
  }
}

wxString SentenceListDlg::GetSentences(void) {
  wxArrayString retString;
  for (size_t i = 0; i < m_clbSentences->GetCount(); i++) {
    if (m_clbSentences->IsChecked(i))
      retString.Add(m_clbSentences->GetString(i));
  }
  return StringArrayToString(retString);
}

void SentenceListDlg::OnCLBSelect(wxCommandEvent& e) {
  // Only activate the "Delete" button if the selection is not in the standard
  // list
  m_btnDel->Enable(m_sentences.Index(e.GetString()) == wxNOT_FOUND);
}

void SentenceListDlg::OnAddClick(wxCommandEvent& event) {
#ifdef __OCPN__ANDROID__
  androidDisableRotation();
#endif

  wxTextEntryDialog textdlg(
      this,
      _("Enter the NMEA sentence (2, 3 or 5 characters)\n  or a valid REGEX "
        "expression (6 characters or longer)"),
      _("Enter the NMEA sentence"));

  textdlg.SetTextValidator(wxFILTER_ASCII);
  int result = textdlg.ShowModal();

#ifdef __OCPN__ANDROID__
  androidEnableRotation();
#endif

  if (result == wxID_CANCEL) return;
  wxString stc = textdlg.GetValue();

  if (stc.Length() == 2 || stc.Length() == 3 || stc.Length() == 5) {
    m_clbSentences->Append(stc);
    m_clbSentences->Check(m_clbSentences->FindString(stc));
    return;
  } else if (stc.Length() < 2) {
    OCPNMessageBox(
        this,
        _("An NMEA sentence is generally 3 characters long (like RMC, GGA etc.)\n \
          It can also have a two letter prefix identifying the source, or TALKER, of the message.\n \
          The whole sentences then looks like GPGGA or AITXT.\n \
          You may filter out all the sentences with certain TALKER prefix (like GP, AI etc.).\n \
          The filter also accepts Regular Expressions (REGEX) with 6 or more characters. \n\n"),
        _("OpenCPN Info"));
    return;
  }

  else {
    // Verify that a longer text entry is a valid RegEx
    wxRegEx r(stc);
    if (r.IsValid()) {
      m_clbSentences->Append(stc);
      m_clbSentences->Check(m_clbSentences->FindString(stc));
      return;
    } else {
      OCPNMessageBox(this, _("REGEX syntax error: \n") + stc,
                     _("OpenCPN Info"));
      return;
    }
  }
}

void SentenceListDlg::OnDeleteClick(wxCommandEvent& event) {
  m_clbSentences->Delete(m_clbSentences->GetSelection());
}

void SentenceListDlg::OnClearAllClick(wxCommandEvent& event) {
  for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
    m_clbSentences->Check(i, FALSE);
}

void SentenceListDlg::OnCheckAllClick(wxCommandEvent& event) {
  for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
    m_clbSentences->Check(i, TRUE);
}

// OpenGLOptionsDlg
enum { ID_BUTTON_REBUILD, ID_BUTTON_CLEAR };

#ifdef ocpnUSE_GL
BEGIN_EVENT_TABLE(OpenGLOptionsDlg, wxDialog)
EVT_BUTTON(ID_BUTTON_REBUILD, OpenGLOptionsDlg::OnButtonRebuild)
EVT_BUTTON(ID_BUTTON_CLEAR, OpenGLOptionsDlg::OnButtonClear)
END_EVENT_TABLE()

OpenGLOptionsDlg::OpenGLOptionsDlg(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, _T( "OpenGL Options" ), wxDefaultPosition,
               wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
#ifdef __WXOSX__
                   | wxSTAY_ON_TOP
#endif
               ),
      m_brebuild_cache(FALSE) {

  wxFont* dialogFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*dialogFont);

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  wxFlexGridSizer* flexSizer = new wxFlexGridSizer(2);

  m_cbTextureCompression =
      new wxCheckBox(this, wxID_ANY,
                     g_bGLexpert ? _("Texture Compression")
                                 : _("Texture Compression with Caching"));
  m_cbTextureCompressionCaching =
      new wxCheckBox(this, wxID_ANY, _("Texture Compression Caching"));
  m_memorySize =
      new wxStaticText(this, wxID_ANY, _("Texture Memory Size (MB)"));
  m_sTextureMemorySize = new wxSpinCtrl(this);
  m_sTextureMemorySize->SetRange(1, 16384);
  m_cacheSize =
      new wxStaticText(this, wxID_ANY, _("Size: ") + GetTextureCacheSize());
  wxButton* btnRebuild =
      new wxButton(this, ID_BUTTON_REBUILD, _("Rebuild Texture Cache"));
  wxButton* btnClear =
      new wxButton(this, ID_BUTTON_CLEAR, _("Clear Texture Cache"));
  btnRebuild->Enable(g_GLOptions.m_bTextureCompressionCaching);
  if (!g_bopengl || g_raster_format == GL_RGB) btnRebuild->Disable();
  btnClear->Enable(g_GLOptions.m_bTextureCompressionCaching);
  m_cbShowFPS = new wxCheckBox(this, wxID_ANY, _("Show FPS"));
  m_cbPolygonSmoothing = new wxCheckBox(this, wxID_ANY, _("Polygon Smoothing"));
  m_cbLineSmoothing = new wxCheckBox(this, wxID_ANY, _("Line Smoothing"));
  m_cbSoftwareGL =
      new wxCheckBox(this, wxID_ANY, _("Software OpenGL (restart OpenCPN)"));
  m_cbUseAcceleratedPanning =
      new wxCheckBox(this, wxID_ANY, _("Use Accelerated Panning"));

  flexSizer->Add(new wxStaticText(this, wxID_ANY, _("Texture Settings")), 0,
                 wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
  flexSizer->Add(m_cbTextureCompression, 0, wxALL | wxEXPAND, 5);
  flexSizer->AddSpacer(0);
  flexSizer->Add(m_cbTextureCompressionCaching, 0, wxALL | wxEXPAND, 5);
  flexSizer->Add(m_memorySize, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
  flexSizer->Add(m_sTextureMemorySize, 0, wxALL | wxEXPAND, 5);
  flexSizer->Add(new wxStaticText(this, wxID_ANY, _("Texture Cache")), 0,
                 wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
  flexSizer->Add(m_cacheSize, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL, 5);
  flexSizer->AddSpacer(0);
  flexSizer->Add(btnRebuild, 0, wxALL | wxEXPAND, 5);
  flexSizer->AddSpacer(0);
  flexSizer->Add(btnClear, 0, wxALL | wxEXPAND, 5);
  flexSizer->Add(new wxStaticText(this, wxID_ANY, _("Miscellaneous")), 0,
                 wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);
  flexSizer->Add(m_cbPolygonSmoothing, 0, wxALL | wxEXPAND, 5);
  flexSizer->AddSpacer(0);
  flexSizer->Add(m_cbLineSmoothing, 0, wxALL | wxEXPAND, 5);
  flexSizer->AddSpacer(0);
  flexSizer->Add(m_cbShowFPS, 0, wxALL | wxEXPAND, 5);
  flexSizer->AddSpacer(0);
  flexSizer->Add(m_cbSoftwareGL, 0, wxALL | wxEXPAND, 5);
  flexSizer->AddSpacer(0);
  flexSizer->Add(m_cbUseAcceleratedPanning, 0, wxALL | wxEXPAND, 5);
  flexSizer->AddGrowableCol(1);
  mainSizer->Add(flexSizer, 0, wxALL | wxEXPAND, 5);

  wxStdDialogButtonSizer* btnSizer = new wxStdDialogButtonSizer();
  btnSizer->AddButton(new wxButton(this, wxID_OK));
  btnSizer->AddButton(new wxButton(this, wxID_CANCEL, _("Cancel")));
  btnSizer->Realize();

  mainSizer->AddStretchSpacer();
  mainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

  Populate();

  SetSizer(mainSizer);
  mainSizer->SetSizeHints(this);
  Centre();
}

bool OpenGLOptionsDlg::GetAcceleratedPanning(void) const {
  return m_cbUseAcceleratedPanning->GetValue();
}

bool OpenGLOptionsDlg::GetTextureCompression(void) const {
  return m_cbTextureCompression->GetValue();
}

bool OpenGLOptionsDlg::GetPolygonSmoothing(void) const {
  return m_cbPolygonSmoothing->GetValue();
}

bool OpenGLOptionsDlg::GetLineSmoothing(void) const {
  return m_cbLineSmoothing->GetValue();
}

bool OpenGLOptionsDlg::GetShowFPS(void) const {
  return m_cbShowFPS->GetValue();
}

bool OpenGLOptionsDlg::GetSoftwareGL(void) const {
  return m_cbSoftwareGL->GetValue();
}

bool OpenGLOptionsDlg::GetTextureCompressionCaching(void) const {
  return m_cbTextureCompressionCaching->GetValue();
}

bool OpenGLOptionsDlg::GetRebuildCache(void) const { return m_brebuild_cache; }

int OpenGLOptionsDlg::GetTextureMemorySize(void) const {
  return m_sTextureMemorySize->GetValue();
}

void OpenGLOptionsDlg::Populate(void) {
  extern PFNGLCOMPRESSEDTEXIMAGE2DPROC s_glCompressedTexImage2D;
  extern bool b_glEntryPointsSet;

  m_cbTextureCompression->SetValue(g_GLOptions.m_bTextureCompression);
  /* disable caching if unsupported */
  if (b_glEntryPointsSet && !s_glCompressedTexImage2D) {
    g_GLOptions.m_bTextureCompressionCaching = FALSE;
    m_cbTextureCompression->Disable();
    m_cbTextureCompression->SetValue(FALSE);
  }

  m_cbTextureCompressionCaching->Show(g_bGLexpert);
  m_memorySize->Show(g_bGLexpert);
  m_sTextureMemorySize->Show(g_bGLexpert);
  if (g_bGLexpert) {
    m_cbTextureCompressionCaching->SetValue(
        g_GLOptions.m_bTextureCompressionCaching);
    m_sTextureMemorySize->SetValue(g_GLOptions.m_iTextureMemorySize);
  }
  m_cbShowFPS->SetValue(g_bShowFPS);
  m_cbPolygonSmoothing->SetValue(g_GLOptions.m_GLPolygonSmoothing);
  m_cbLineSmoothing->SetValue(g_GLOptions.m_GLLineSmoothing);

#if defined(__UNIX__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)
  if (gFrame->GetPrimaryCanvas()->GetglCanvas()) {
    if (gFrame->GetPrimaryCanvas()
            ->GetglCanvas()
            ->GetVersionString()
            .Upper()
            .Find(_T( "MESA" )) != wxNOT_FOUND)
      m_cbSoftwareGL->SetValue(g_bSoftwareGL);
  }
#else
  m_cbSoftwareGL->Hide();
#endif

  wxFont* dialogFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*dialogFont);

  if (g_bGLexpert) {
    if (gFrame->GetPrimaryCanvas()->GetglCanvas()) {
      if (gFrame->GetPrimaryCanvas()->GetglCanvas()->CanAcceleratePanning()) {
        m_cbUseAcceleratedPanning->Enable();
        m_cbUseAcceleratedPanning->SetValue(
            g_GLOptions.m_bUseAcceleratedPanning);
      } else {
        m_cbUseAcceleratedPanning->SetValue(FALSE);
        m_cbUseAcceleratedPanning->Disable();
      }
    } else {
      m_cbUseAcceleratedPanning->SetValue(g_GLOptions.m_bUseAcceleratedPanning);
    }
  } else {
    m_cbUseAcceleratedPanning->SetValue(g_GLOptions.m_bUseAcceleratedPanning);
    m_cbUseAcceleratedPanning->Disable();
  }
}

void OpenGLOptionsDlg::OnButtonRebuild(wxCommandEvent& event) {
  if (g_GLOptions.m_bTextureCompressionCaching) {
    m_brebuild_cache = TRUE;
    EndModal(wxID_CANCEL);
  }
}

void OpenGLOptionsDlg::OnButtonClear(wxCommandEvent& event) {
  if (g_bopengl && g_glTextureManager) {
    ::wxBeginBusyCursor();
    g_glTextureManager->ClearAllRasterTextures();

    wxString path = g_Platform->GetPrivateDataDir();
    appendOSDirSlash(&path);
    path.append(_T("raster_texture_cache"));

    if (::wxDirExists(path)) {
      wxArrayString files;
      size_t nfiles = wxDir::GetAllFiles(path, &files);
      for (unsigned int i = 0; i < files.GetCount(); i++)
        ::wxRemoveFile(files[i]);
    }

    m_cacheSize->SetLabel(_("Size: ") + GetTextureCacheSize());
    ::wxEndBusyCursor();
  }
}

wxString OpenGLOptionsDlg::GetTextureCacheSize(void) {
  wxString path = g_Platform->GetPrivateDataDir();
  appendOSDirSlash(&path);
  path.append(_T("raster_texture_cache"));
  long long total = 0;

  if (::wxDirExists(path)) {
    wxArrayString files;
    size_t nfiles = wxDir::GetAllFiles(path, &files);
    for (unsigned int i = 0; i < files.GetCount(); i++)
      total += wxFile(files[i]).Length();
  }
  double mb = total / (1024.0 * 1024.0);
  if (mb < 10000.0) return wxString::Format(_T( "%.1f MB" ), mb);
  mb = mb / 1024.0;
  return wxString::Format(_T( "%.1f GB" ), mb);
}

//-------------------------------------------------------------------------------------------------
//  CanvasConfig selection panel
//-------------------------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CanvasConfigSelect, wxPanel)
EVT_PAINT(CanvasConfigSelect::OnPaint)
END_EVENT_TABLE()

CanvasConfigSelect::CanvasConfigSelect(wxWindow* parent, options* parentOptions,
                                       wxWindowID id, wxBitmap& bmp,
                                       const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE) {
  m_parentOptions = parentOptions;
  m_bmpNormal = bmp;
  m_borderWidth = 5;
  SetSelected(false);

  int refHeight = GetCharHeight();
  // SetMinSize(wxSize(-1, 5 * refHeight));
  SetMinSize(wxSize(bmp.GetSize().x + m_borderWidth * 2,
                    bmp.GetSize().y + m_borderWidth * 2));

  Connect(wxEVT_LEFT_DOWN,
          wxMouseEventHandler(CanvasConfigSelect::OnMouseSelected), NULL, this);
}

CanvasConfigSelect::~CanvasConfigSelect() {}

void CanvasConfigSelect::OnMouseSelected(wxMouseEvent& event) {
  if (!m_bSelected) {
    SetSelected(true);
  } else {
    SetSelected(false);
  }
  if (m_parentOptions)
    m_parentOptions->OnCanvasConfigSelectClick(GetId(), GetSelected());
}

void CanvasConfigSelect::SetSelected(bool selected) {
  m_bSelected = selected;
  wxColour colour;

  if (selected) {
    GetGlobalColor(_T("UIBCK"), &colour);
    m_boxColour = colour;
  } else {
    GetGlobalColor(_T("DILG0"), &colour);
    m_boxColour = colour;
  }

  Refresh(true);
}

void CanvasConfigSelect::OnPaint(wxPaintEvent& event) {
  int width, height;
  GetSize(&width, &height);
  wxPaintDC dc(this);

  dc.SetBackground(*wxLIGHT_GREY);

  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(wxBrush(GetBackgroundColour()));
  dc.DrawRectangle(GetVirtualSize());

  wxColour c;

  if (m_bSelected) {
    dc.SetBrush(wxBrush(m_boxColour));

    GetGlobalColor(_T ( "UITX1" ), &c);
    dc.SetPen(wxPen(wxColor(0xCE, 0xD5, 0xD6), 3));

    dc.DrawRoundedRectangle(0, 0, width - 1, height - 1, height / 10);
  } else {
    dc.SetBrush(wxBrush(m_boxColour));

    GetGlobalColor(_T ( "UITX1" ), &c);
    dc.SetPen(wxPen(wxColor(0xCE, 0xD5, 0xD6), 3));

    dc.DrawRoundedRectangle(0, 0, width - 1, height - 1, height / 10);
  }

  dc.DrawBitmap(m_bmpNormal, m_borderWidth, m_borderWidth, false);
}

#endif  // ocpnUSE_GL
