/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Android support utilities
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
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/tokenzr.h>
#include <wx/aui/aui.h>
#include <wx/fontpicker.h>
#include <wx/filepicker.h>
#include <wx/zipstrm.h>

#include <QtAndroidExtras/QAndroidJniObject>
#include <QWidget>
#include <QMenu>
#include <QApplication>

#include "config.h"
#include "dychart.h"
#include "androidUTIL.h"
#include "OCPN_DataStreamEvent.h"
#include "chart1.h"
#include "AISTargetQueryDialog.h"
#include "AISTargetAlertDialog.h"
#include "AISTargetListDialog.h"
#include "TrackPropDlg.h"
#include "S57QueryDialog.h"
#include "options.h"
#include "routemanagerdialog.h"
#include "chartdb.h"
#include "s52plib.h"
#include "s52utils.h"
#include "s52s57.h"
#include "navutil.h"
#include "TCWin.h"
#include "ocpn_plugin.h"
#include "about.h"
#include "OCPNPlatform.h"
#include "multiplexer.h"
#include "chartdbs.h"
#include "glChartCanvas.h"
#include "chcanv.h"
#include "MarkInfo.h"
#include "RoutePropDlgImpl.h"
#include "MUIBar.h"
#include "toolbar.h"
#include "NavObjectCollection.h"
#include "toolbar.h"
#include "iENCToolbar.h"
#include "Select.h"
#include "routeman.h"
#include "CanvasOptions.h"
#include "SerialDataStream.h"
#include <wchar.h>

const wxString AndroidSuppLicense =
wxT("<br><br>The software included in this product contains copyrighted software that is licensed under the GPL.")
wxT("A copy of that license is shown above.")
wxT("You may obtain the complete Corresponding Source code from us for ")
wxT("a period of three years after our last shipment of this product, ")
wxT("by sending a money order or check for $5 to:<br><br>")
wxT("GPL Compliance Division<br>")
wxT("Dyad Inc.<br>")
wxT("31 Ocean Reef Dr<br>")
wxT("# C101-449<br>")
wxT("Key Largo, FL 33037-5282<br>")
wxT("United States<br><br>")
wxT("Please write “source for OpenCPN Version {insert version here} in the memo line of your payment.<br><br>");


#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif



class androidUtilHandler;



extern MyFrame                  *gFrame;
extern const wxEventType wxEVT_OCPN_DATASTREAM;
//extern const wxEventType wxEVT_DOWNLOAD_EVENT;

wxEvtHandler                    *s_pAndroidNMEAMessageConsumer;
wxEvtHandler                    *s_pAndroidBTNMEAMessageConsumer;

extern AISTargetAlertDialog      *g_pais_alert_dialog_active;
extern AISTargetQueryDialog      *g_pais_query_dialog_active;
extern AISTargetListDialog       *g_pAISTargetList;
//extern MarkInfoImpl              *pMarkPropDialog;
extern RoutePropDlgImpl          *pRoutePropDialog;
extern TrackPropDlg              *pTrackPropDialog;
extern S57QueryDialog            *g_pObjectQueryDialog;
extern options                   *g_options;
extern bool                       g_bSleep;
androidUtilHandler               *g_androidUtilHandler;
extern wxDateTime                 g_start_time;
extern RouteManagerDialog        *pRouteManagerDialog;
extern about                     *g_pAboutDlgLegacy;
extern bool                      g_bFullscreen;
extern OCPNPlatform              *g_Platform;

// Static globals
extern ChartDB                   *ChartData;
extern MyConfig                  *pConfig;


//   Preferences globals
extern bool             g_bShowOutlines;
extern bool             g_bShowChartBar;
extern bool             g_bShowDepthUnits;
extern bool             g_bskew_comp;
extern bool             g_bopengl;
extern bool             g_bsmoothpanzoom;
extern bool             g_bShowMag;
extern double           g_UserVar;
extern int              g_chart_zoom_modifier;
extern int              g_NMEAAPBPrecision;
extern wxString         g_TalkerIdText;

extern wxString         *pInit_Chart_Dir;
extern wxArrayOfConnPrm *g_pConnectionParams;
extern bool             g_bfilter_cogsog;
extern int              g_COGFilterSec;
extern int              g_SOGFilterSec;


extern bool             g_bDisplayGrid;

//    AIS Global configuration
extern bool             g_bCPAMax;
extern double           g_CPAMax_NM;
extern bool             g_bCPAWarn;
extern double           g_CPAWarn_NM;
extern bool             g_bTCPA_Max;
extern double           g_TCPA_Max;
extern bool             g_bMarkLost;
extern double           g_MarkLost_Mins;
extern bool             g_bRemoveLost;
extern double           g_RemoveLost_Mins;
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bAISShowTracks;
extern double           g_AISShowTracks_Mins;
extern bool             g_bHideMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bAIS_CPA_Alert;
extern bool             g_bAIS_CPA_Alert_Audio;
extern wxString         g_sAIS_Alert_Sound_File;
extern bool             g_bAIS_CPA_Alert_Suppress_Moored;
extern bool             g_bShowAreaNotices;
extern bool             g_bDrawAISSize;
extern bool             g_bShowAISName;
extern int              g_Show_Target_Name_Scale;
extern bool             g_bWplIsAprsPosition;

extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern int              g_iWaypointRangeRingsNumber;
extern float            g_fWaypointRangeRingsStep;
extern int              g_iWaypointRangeRingsStepUnits;
extern wxColour         g_colourWaypointRangeRingsColour;
extern bool             g_bWayPointPreventDragging;

extern bool             g_bPreserveScaleOnX;
extern bool             g_bPlayShipsBells;
extern int              g_iSoundDeviceIndex;
extern bool             g_bFullscreenToolbar;

extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern int              g_n_ownship_min_mm;
extern double           g_n_arrival_circle_radius;

extern bool             g_bEnableZoomToCursor;
extern bool             g_bTrackDaily;
extern bool             g_bHighliteTracks;
extern double           g_TrackIntervalSeconds;
extern double           g_TrackDeltaDistance;
extern double           g_TrackDeltaDistance;
extern int              g_nTrackPrecision;

extern int              g_iSDMMFormat;
extern int              g_iDistanceFormat;
extern int              g_iSpeedFormat;

extern bool             g_bAdvanceRouteWaypointOnArrivalOnly;

extern int              g_cm93_zoom_factor;

extern int              g_COGAvgSec;

extern bool             g_bCourseUp;
extern bool             g_bLookAhead;

extern double           g_ownship_predictor_minutes;
extern double           g_ownship_HDTpredictor_miles;
extern double           gLat, gLon, gCog, gSog, gHdt, gVar;

extern bool             g_bAISRolloverShowClass;
extern bool             g_bAISRolloverShowCOG;
extern bool             g_bAISRolloverShowCPA;

extern bool             g_bAIS_ACK_Timeout;
extern double           g_AckTimeout_Mins;

extern bool             g_bQuiltEnable;
extern bool             g_bFullScreenQuilt;
extern bool             g_bConfirmObjectDelete;
extern wxString         g_GPS_Ident;
extern bool             g_bGarminHostUpload;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
extern wxLocale         *plocale_def_lang;
#endif

//extern OCPN_Sound        g_anchorwatch_sound;
extern bool             g_bMagneticAPB;

extern bool             g_fog_overzoom;
extern double           g_overzoom_emphasis_base;
extern bool             g_oz_vector_scale;
extern bool             g_bShowStatusBar;

extern ocpnGLOptions    g_GLOptions;


extern s52plib          *ps52plib;

extern wxString         g_locale;
extern bool             g_bportable;
extern bool             g_bdisable_opengl;

extern ChartGroupArray  *g_pGroupArray;


extern bool             g_bUIexpert;
//    Some constants
#define ID_CHOICE_NMEA  wxID_HIGHEST + 1

//extern wxArrayString *EnumerateSerialPorts(void);           // in chart1.cpp

extern wxArrayString    TideCurrentDataSet;
extern wxString         g_TCData_Dir;

extern AIS_Decoder      *g_pAIS;
extern bool             g_bserial_access_checked;

extern options          *g_pOptions;

extern bool             g_btouch;
extern bool             g_bresponsive;
extern bool             g_bAutoHideToolbar;
extern int              g_nAutoHideToolbar;
extern int              g_GUIScaleFactor;
extern int              g_ChartScaleFactor;

extern double           g_config_display_size_mm;
extern float            g_ChartScaleFactorExp;
extern bool             g_config_display_size_manual;

extern Multiplexer      *g_pMUX;
extern bool             b_inCloseWindow;
extern bool             g_config_display_size_manual;
extern MarkInfoDlg      *g_pMarkInfoDialog;
extern PlugInManager    *g_pi_manager;
extern iENCToolbar      *g_iENCToolbar;
extern int              g_iENCToolbarPosX;
extern int              g_iENCToolbarPosY;
extern ocpnFloatingToolbarDialog *g_MainToolbar;
extern int              g_maintoolbar_x;
extern int              g_maintoolbar_y;
extern long             g_maintoolbar_orient;
extern int              g_restore_stackindex;
extern int              g_restore_dbindex;
extern ChartStack       *pCurrentStack;
extern Select           *pSelect;
extern WayPointman      *pWayPointMan;
extern bool             g_bCruising;
extern RoutePoint       *pAnchorWatchPoint1;
extern RoutePoint       *pAnchorWatchPoint2;
extern bool             g_bAutoAnchorMark;
extern wxAuiManager     *g_pauimgr;
extern wxString         g_AisTargetList_perspective;
extern wxLog            *g_logger;

extern ocpnFloatingToolbarDialog *g_MainToolbar;

WX_DEFINE_ARRAY_PTR(ChartCanvas*, arrayofCanvasPtr);
extern arrayofCanvasPtr g_canvasArray;

wxString callActivityMethod_vs(const char *method);
wxString callActivityMethod_is(const char *method, int parm);


//      Globals, accessible only to this module

JavaVM *java_vm;
JNIEnv* global_jenv;
bool     b_androidBusyShown;
double   g_androidDPmm;
double   g_androidDensity;

bool            g_bExternalApp;

wxString        g_androidFilesDir;
wxString        g_androidCacheDir;
wxString        g_androidExtFilesDir;
wxString        g_androidExtCacheDir;
wxString        g_androidExtStorageDir;

int             g_mask;
int             g_sel;
int             g_ActionBarHeight;
int             g_follow_state;
bool            g_track_active;
bool            bGPSEnabled;

wxSize          config_size;

bool            s_bdownloading;
wxString        s_requested_url;
wxEvtHandler    *s_download_evHandler;
bool            g_running;
bool            g_bstress1;
extern int      g_GUIScaleFactor;

wxString        g_deviceInfo;

int             s_androidMemTotal;
int             s_androidMemUsed;
bool            g_backEnabled;
bool            g_bFullscreenSave;
bool            s_optionsActive;

extern int ShowNavWarning();
extern bool     g_btrackContinuous;

int doAndroidPersistState();

bool            bInConfigChange;
AudioDoneCallback s_soundCallBack;

//      Some dummy devices to ensure plugins have static access to these classes not used elsewhere
wxFontPickerEvent       g_dummy_wxfpe;

#define ANDROID_EVENT_TIMER 4389
#define ANDROID_STRESS_TIMER 4388
#define ANDROID_RESIZE_TIMER 4387

#define ACTION_NONE                     -1
#define ACTION_RESIZE_PERSISTENTS       1
#define ACTION_FILECHOOSER_END          3
#define ACTION_COLORDIALOG_END          4
#define ACTION_POSTASYNC_END            5

#define SCHEDULED_EVENT_CLEAN_EXIT      5498

class androidUtilHandler : public wxEvtHandler
{
 public:
     androidUtilHandler();
    ~androidUtilHandler() {}
    
    void onTimerEvent(wxTimerEvent &event);
    void onStressTimer(wxTimerEvent &event);
    void OnResizeTimer(wxTimerEvent &event);
    void OnScheduledEvent( wxCommandEvent& event );

    wxString GetStringResult(){ return m_stringResult; }
    
    wxTimer     m_eventTimer;
    int         m_action;
    bool        m_done;
    wxString    m_stringResult;
    wxTimer     m_stressTimer;
    wxTimer     m_resizeTimer;
    int         timer_sequence;
    int         m_bskipConfirm;
    DECLARE_EVENT_TABLE()
};

const char  wxMessageBoxCaptionStr [] = "Message";

#define _GNU_SOURCE 1
extern "C" const char* __gnu_basename(const char* path) {
  const char* last_slash = strrchr(path, '/');
  return (last_slash != nullptr) ? last_slash + 1 : path;
}

extern "C" char* __gnu_strerror_r(int error_number, char* buf, size_t buf_len) {
 // ErrnoRestorer errno_restorer; // The glibc strerror_r doesn't set errno if it truncates...
  //strerror_r(error_number, buf, buf_len);
  strncpy(buf, strerror(errno), buf_len-1);  
  return buf; // ...and just returns whatever fit.
}

extern "C" int __register_atfork(void (*prepare) (void), void (*parent) (void), void (*child) (void), void *dso_handle)
{
    return pthread_atfork(prepare,parent,child);
}


char *fgets_unlocked(char *s, int n, FILE *stream)
{
    return fgets(s, n, stream);
}

size_t fwrite_unlocked(const void *ptr, size_t size, size_t n, FILE *stream)
{
    return 0;
}
size_t fread_unlocked(void *ptr, size_t size, size_t n, FILE *stream)
{
    return 0;
}

int fputc_unlocked(int c, FILE *stream)
{
    return 0;
}


double wcstod_l(const wchar_t* s, wchar_t** end_ptr, locale_t) {
  return wcstod(s, end_ptr);
}

long wcstol_l(const wchar_t* s, wchar_t** end_ptr, int base, locale_t) {
  return wcstol(s, end_ptr, base);
}

unsigned long wcstoul_l(const wchar_t* s, wchar_t** end_ptr, int base, locale_t) {
  return wcstoul(s, end_ptr, base);
}

BEGIN_EVENT_TABLE ( androidUtilHandler, wxEvtHandler )
EVT_TIMER ( ANDROID_EVENT_TIMER, androidUtilHandler::onTimerEvent )
EVT_TIMER ( ANDROID_RESIZE_TIMER, androidUtilHandler::OnResizeTimer )
EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, androidUtilHandler::OnScheduledEvent )

END_EVENT_TABLE()

androidUtilHandler::androidUtilHandler()
{
    m_eventTimer.SetOwner( this, ANDROID_EVENT_TIMER );
    m_stressTimer.SetOwner( this, ANDROID_STRESS_TIMER );
    m_resizeTimer.SetOwner(this, ANDROID_RESIZE_TIMER);
 
    m_bskipConfirm = false;
    
    // We do a few little dummy class accesses here, to cause the static link to wxWidgets to bring in some
    // class members required by some plugins, that would be missing otherwise.
    
    wxRegion a(0,0,1,1);
    wxRegion b(0,0,2,2);
    bool c = a.IsEqual(b);
    
    wxFilePickerCtrl *pfpc = new wxFilePickerCtrl();
    
    wxZipEntry *entry = new wxZipEntry();
    
    wxRadioBox *box = new wxRadioBox();
    delete box;

    
}

       
void androidUtilHandler::onTimerEvent(wxTimerEvent &event)
{
//    qDebug() << "onTimerEvent";

    switch(m_action){
        case ACTION_RESIZE_PERSISTENTS:            //  Handle rotation/resizing of persistent dialogs

            // AIS Target Query
            if( g_pais_query_dialog_active ) {
                qDebug() << "AISB";

                bool bshown = g_pais_query_dialog_active->IsShown();
                g_pais_query_dialog_active->Hide();
                g_pais_query_dialog_active->RecalculateSize();
                if(bshown){
                    qDebug() << "AISC";
                    g_pais_query_dialog_active->Show();
                    g_pais_query_dialog_active->Raise();
                }
            }

            // Route Props
            if(RoutePropDlgImpl::getInstanceFlag()){
                bool bshown = pRoutePropDialog->IsShown();
                if(bshown){
                    pRoutePropDialog->Hide();
                    pRoutePropDialog->RecalculateSize();
                    pRoutePropDialog->Show();
                }
                else{
                    pRoutePropDialog->Destroy();
                    pRoutePropDialog = NULL;
                }
            }

            // Track Props
            if(TrackPropDlg::getInstanceFlag()){
                bool bshown = pTrackPropDialog->IsShown();
                if(bshown){
                    pTrackPropDialog->Hide();
                    pTrackPropDialog->RecalculateSize();
                    pTrackPropDialog->Show();
                }
                else{
                    pTrackPropDialog->Destroy();
                    pTrackPropDialog = NULL;
                }
            }
            
            // Mark Props
            
            if(g_pMarkInfoDialog){
                bool bshown = g_pMarkInfoDialog->IsShown();
                g_pMarkInfoDialog->Hide();
                g_pMarkInfoDialog->RecalculateSize();
                if(bshown){
                    if(g_pMarkInfoDialog->m_SaveDefaultDlg){
                        g_pMarkInfoDialog->m_SaveDefaultDlg->Destroy();
                        g_pMarkInfoDialog->m_SaveDefaultDlg = NULL;
                    }
                    g_pMarkInfoDialog->Show();
                }
                
            }
            
            // ENC Object Query
            if(g_pObjectQueryDialog){
                bool bshown = g_pObjectQueryDialog->IsShown();
                g_pObjectQueryDialog->Hide();
                g_pObjectQueryDialog->RecalculateSize();
                if(bshown){
                    g_pObjectQueryDialog->Show();
                } 
            }
            
            
            // AIS Target List dialog
            if(g_pAISTargetList){
                qDebug() << "ATLA";
                bool bshown = g_pAISTargetList->IsShown();
                g_pAISTargetList->Hide();
                g_pAISTargetList->RecalculateSize();
                if(bshown){
                    qDebug() << "ATLB";
                    g_pAISTargetList->Show();
                    g_pAISTargetList->Raise();
                }
            }
 
            // Tide/Current window
            if( gFrame->GetPrimaryCanvas()->getTCWin()){
                bool bshown = gFrame->GetPrimaryCanvas()->getTCWin()->IsShown();
                gFrame->GetPrimaryCanvas()->getTCWin()->Hide();
                gFrame->GetPrimaryCanvas()->getTCWin()->RecalculateSize();
                if(bshown){
                    gFrame->GetPrimaryCanvas()->getTCWin()->Show();
                    gFrame->GetPrimaryCanvas()->getTCWin()->Refresh();
                }
            }
            
            // Route Manager dialog
            if(RouteManagerDialog::getInstanceFlag()){
                bool bshown = pRouteManagerDialog->IsShown();
                if(bshown){
                    pRouteManagerDialog->Hide();
                    pRouteManagerDialog->RecalculateSize();
                    pRouteManagerDialog->Show();
                }
                else{
                    pRouteManagerDialog->Destroy();
                    pRouteManagerDialog = NULL;
                }
                    
            }
            
            // About dialog
            if(g_pAboutDlgLegacy){
                bool bshown = g_pAboutDlgLegacy->IsShown();
                if(bshown){
                    g_pAboutDlgLegacy->Hide();
                    g_pAboutDlgLegacy->RecalculateSize();
                    g_pAboutDlgLegacy->Show();
                }
            }
            
            bInConfigChange = false;
            
            break;
 
        case ACTION_FILECHOOSER_END:            //  Handle polling of android Dialog
            {
                //qDebug() << "chooser poll";
                //  Get a reference to the running FileChooser
                QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                "activity", "()Landroid/app/Activity;");
                
                if ( !activity.isValid() ){
                    //qDebug() << "onTimerEvent : Activity is not valid";
                    return;
                }
 
                //  Call the method which tracks the completion of the Intent.
                QAndroidJniObject data = activity.callObjectMethod("isFileChooserFinished", "()Ljava/lang/String;");
                
                jstring s = data.object<jstring>();
                
                JNIEnv* jenv;
                
                //  Need a Java environment to decode the resulting string
                if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
                    //qDebug() << "GetEnv failed.";
                }
                else {
                    
                    // The string coming back will be one of:
                    //  "no"   ......Intent not done yet.
                    //  "cancel:"   .. user cancelled intent.
                    //  "file:{file_name}"  .. user selected this file, fully qualified.
                    if(!s){
                        //qDebug() << "isFileChooserFinished returned null";
                    }
                    else if( (jenv)->GetStringLength( s )){
                        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
//                        qDebug() << "isFileChooserFinished returned " << ret_string;
                        if( !strncmp(ret_string, "cancel:", 7) ){
                            m_done = true;
                            m_stringResult = _T("cancel:");
                        }
                        else if( !strncmp(ret_string, "file:", 5) ){
                            m_done = true;
                            m_stringResult = wxString(ret_string, wxConvUTF8);
                        } 
                    }
                }
                
                
                break;
            }

        case ACTION_COLORDIALOG_END:            //  Handle polling of android Dialog
            {
                //qDebug() << "colorpicker poll";
                //  Get a reference to the running FileChooser
                QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                                       "activity", "()Landroid/app/Activity;");
                
                if ( !activity.isValid() ){
                    //qDebug() << "onTimerEvent : Activity is not valid";
                    return;
                }
                
                //  Call the method which tracks the completion of the Intent.
                QAndroidJniObject data = activity.callObjectMethod("isColorPickerDialogFinished", "()Ljava/lang/String;");
                
                jstring s = data.object<jstring>();
                
                JNIEnv* jenv;
                
                //  Need a Java environment to decode the resulting string
                if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
                    //qDebug() << "GetEnv failed.";
                }
                else {
                    
                    // The string coming back will be one of:
                    //  "no"   ......Dialog not done yet.
                    //  "cancel:"   .. user cancelled Dialog.
                    //  "color: ".
                    if(!s){
                        qDebug() << "isColorPickerDialogFinished returned null";
                    }
                    else if( (jenv)->GetStringLength( s )){
                        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
                        //qDebug() << "isColorPickerDialogFinished returned " << ret_string;
                        if( !strncmp(ret_string, "cancel:", 7) ){
                            m_done = true;
                            m_stringResult = _T("cancel:");
                        }
                        else if( !strncmp(ret_string, "color:", 6) ){
                            m_done = true;
                            m_stringResult = wxString(ret_string, wxConvUTF8);
                        } 
                    }
                }
                
                
                break;
            }
            
        case ACTION_POSTASYNC_END:            //  Handle polling of android async POST task end
            {
                //qDebug() << "colorpicker poll";
                //  Get a reference to the running FileChooser
                QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                                       "activity", "()Landroid/app/Activity;");
                
                if ( !activity.isValid() ){
                    //qDebug() << "onTimerEvent : Activity is not valid";
                    return;
                }
                
                //  Call the method which tracks the completion of the POST async task.
                QAndroidJniObject data = activity.callObjectMethod("checkPostAsync", "()Ljava/lang/String;");
                
                jstring s = data.object<jstring>();
                
                JNIEnv* jenv;
                
                //  Need a Java environment to decode the resulting string
                if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
                    //qDebug() << "GetEnv failed.";
                }
                else {
                    
                    // The string coming back will be either:
                    //  "ACTIVE"   ......Post command not done yet.
                    //  A valid XML response body.
                    if(!s){
                        qDebug() << "checkPostAsync returned null";
                    }
                    else if( (jenv)->GetStringLength( s )){
                        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
                        qDebug() << "checkPostAsync returned " << ret_string;
                        if( strncmp(ret_string, "ACTIVE", 6) ){         // Must be done....
                            m_done = true;
                            m_stringResult =  wxString(ret_string, wxConvUTF8);
                        }
                    }
                }
                
                
                break;
            }
            
            
        default:
            break;
    }
    
}

void androidUtilHandler::OnResizeTimer(wxTimerEvent &event)
{
    if(timer_sequence == 0){
    //  On QT, we need to clear the status bar item texts to prevent the status bar from
    //  growing the parent frame due to unexpected width changes.
//         if( m_pStatusBar != NULL ){
//             int widths[] = { 2,2,2,2,2 };
//            m_pStatusBar->SetStatusWidths( m_StatusBarFieldCount, widths );
// 
//             for(int i=0 ; i <  m_pStatusBar->GetFieldsCount() ; i++){
//                 m_pStatusBar->SetStatusText(_T(""), i);
//             }
//         }
        qDebug() << "sequence 0";

        timer_sequence++;
        //  This timer step needs to be long enough to allow Java induced size change to take effect
        //  in another thread.
        m_resizeTimer.Start(1000, wxTIMER_ONE_SHOT);
        return;
    }



    if(timer_sequence == 1){
        qDebug() << "sequence 1" << config_size.x;
        gFrame->SetSize(config_size);
        timer_sequence++;
        if(!m_bskipConfirm)
            m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
        m_bskipConfirm = false;
        return;
    }

    if(timer_sequence == 2){
        timer_sequence++;
        m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
        return;
    }

    if(timer_sequence == 3){
        qDebug() << "sequence 3";
        androidConfirmSizeCorrection();

        timer_sequence++;
        m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
        return;
    }
    
    if(timer_sequence == 4){
        qDebug() << "sequence 4";
        
        //  Raise the resized options dialog.
        //  This has no effect if the dialog is not already shown.
        if(g_options)
            g_options->Raise();

        resizeAndroidPersistents();
        return;
    }

}









int stime;

void androidUtilHandler::onStressTimer(wxTimerEvent &event){

    g_GUIScaleFactor = -5;
    g_ChartScaleFactor = -5;
    gFrame->SetGPSCompassScale();
    
    s_androidMemUsed  = 80;
    
    g_GLOptions.m_bTextureCompression = 0;
    g_GLOptions.m_bTextureCompressionCaching = 0;
    
    if(600 == stime++) androidTerminate();
    
}

void androidUtilHandler::OnScheduledEvent( wxCommandEvent& event )
{
    switch( event.GetId() ){
        
        case SCHEDULED_EVENT_CLEAN_EXIT:
//             gFrame->FrameTimer1.Stop();
//             gFrame->FrameCOGTimer.Stop();
// 
//             doAndroidPersistState();
//             androidTerminate();
            break;

        case ID_CMD_TRIGGER_RESIZE:
            qDebug() << "Trigger Resize";
            timer_sequence = 0;
            m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
            bInConfigChange = true;
            break;
 
        case ID_CMD_SOUND_FINISHED:
            //qDebug() << "Trigger SoundFinished";
            if(s_soundCallBack){
               s_soundCallBack(0);              // No user data
               s_soundCallBack = 0;
            }
            break;
 
/* 
        case ID_CMD_STOP_RESIZE:    
           // Stop any underway timer chain
            qDebug() << "Stop Resize";
            m_resizeTimer.Stop();
            m_eventTimer.Stop();
            timer_sequence = 0;
            bInConfigChange = false;
            break;
            */
           
            
        default:
            break;
    }
}



bool androidUtilInit( void )
{
    qDebug() << "androidUtilInit()";
    
    g_androidUtilHandler = new androidUtilHandler();

    //  Initialize some globals
    
    s_androidMemTotal  = 100;
    s_androidMemUsed  = 50;
    
    wxString dirs = callActivityMethod_vs("getSystemDirs");
    qDebug() << "dirs: " << dirs.mb_str();
    
    wxStringTokenizer tk(dirs, _T(";"));
    if( tk.HasMoreTokens() ){
        wxString token = tk.GetNextToken();
        if(wxNOT_FOUND != token.Find(_T("EXTAPP")))
            g_bExternalApp = true;
        
        token = tk.GetNextToken();              
        g_androidFilesDir = token;              // used for "home dir"
        token = tk.GetNextToken();              
        g_androidCacheDir = token;
        token = tk.GetNextToken();              
        g_androidExtFilesDir = token;           // used as PrivateDataDir, "/storage/emulated/0/Android/data/org.opencpn.opencpn/files"
                                                // if app has been moved to sdcard, this gives like (on Android 6)
                                                // /storage/2385-1BF8/Android/data/org.opencpn.opencpn/files
        token = tk.GetNextToken();              
        g_androidExtCacheDir = token;
        token = tk.GetNextToken();              
        g_androidExtStorageDir = token;
        
    }
    
    g_mask = -1;
    g_sel = -1;
    
    
    wxStringTokenizer tku(g_androidExtFilesDir, _T("/") );
    while( tku.HasMoreTokens() )
    {
        wxString s1 = tku.GetNextToken();
    
        if(s1.Find(_T("org.")) != wxNOT_FOUND){
            if(s1 != _T("org.opencpn.opencpn") ) g_bstress1 = true;
        }
    }
                
    if(g_bstress1){            
        g_androidUtilHandler->Connect( g_androidUtilHandler->m_stressTimer.GetId(), wxEVT_TIMER, wxTimerEventHandler( androidUtilHandler::onStressTimer ), NULL, g_androidUtilHandler );
        g_androidUtilHandler->m_stressTimer.Start(1000, wxTIMER_CONTINUOUS);
    }
    
    return true;
}

void androidPrepareShutdown()
{
    // This method is called just before the android Java side activity 
    // posts a final "BACK" button key to the Qt subsystem.
    // Provides a way to configure the running app for a clean shutdown.
    
    if(gFrame->GetPrimaryCanvas()){
        gFrame->GetPrimaryCanvas()->ShowMUIBar(false);
    }
}


wxSize getAndroidConfigSize()
{
    return config_size;
}

void resizeAndroidPersistents()
{
    
     if(g_androidUtilHandler){
         g_androidUtilHandler->m_action = ACTION_RESIZE_PERSISTENTS;
         g_androidUtilHandler->m_eventTimer.Start(100, wxTIMER_ONE_SHOT);
     }
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    java_vm = vm;
    
    // Get JNI Env for all function calls
    if (vm->GetEnv( (void **) &global_jenv, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    
    return JNI_VERSION_1_6;
}

void sendNMEAMessageEvent(wxString &msg)
{
    wxCharBuffer abuf = msg.ToUTF8();
    if( abuf.data() ){                            // OK conversion?
        std::string s(abuf.data());              
//    qDebug() << tstr;
        OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
        Nevent.SetNMEAString( s );
        Nevent.SetStream( NULL );
        if(s_pAndroidNMEAMessageConsumer)
            s_pAndroidNMEAMessageConsumer->AddPendingEvent(Nevent);
    }
}
    
    

//      OCPNNativeLib
//      This is a set of methods which can be called from the android activity context.

extern "C"{
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_test(JNIEnv *env, jobject obj)
{
    //qDebug() << "test";
    
    return 55;
}
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_processSailTimer(JNIEnv *env, jobject obj, double WindAngleMagnetic, double WindSpeedKnots)
    {
        //  The NMEA message target handler may not be setup yet, if no connections are defined or enabled.
        //  But we may want to synthesize messages from the Java app, even without a definite connection, and we want to process these messages too.
        //  So assume that the global MUX, if present, will handle these synthesized messages.
        if( !s_pAndroidNMEAMessageConsumer && g_pMUX ) 
            s_pAndroidNMEAMessageConsumer = g_pMUX;
        
        
        double wind_angle_mag = 0;
        double apparent_wind_angle = 0;
        
        double app_windSpeed = 0;
        double true_windSpeed = 0;
        double true_windDirection = 0;
        
        {
            {
                
                // Need to correct the Magnetic wind angle to True
                // TODO  Punt for mow
                double variation = gVar;
                //qDebug() << "gVar" << gVar;
                
                //  What to use for TRUE ownship head?
                //TODO Look for HDT message contents, if available
                double osHead = gCog;
                bool buseCOG = true;
                //qDebug() << "gHdt" << gHdt;
                
                if( !wxIsNaN(gHdt) ){
                    osHead = gHdt;
                    buseCOG = false;
                }
                
                // What SOG to use?
                double osSog = gSog;
                
                wind_angle_mag = WindAngleMagnetic;
                app_windSpeed = WindSpeedKnots;
                
                // Compute the apparent wind angle
                // If using gCog for ownship head, require speed to be > 0.2 knots
                // If not useing cGog for head, assume we must be using a true heading sensor, so always valid
                if( !wxIsNaN(osHead) && ( (!buseCOG)  ||  (buseCOG && osSog > 0.2) ) ){
                    apparent_wind_angle = wind_angle_mag - (osHead - variation);
                }
                else{
                    apparent_wind_angle = 0;
                }
                if(apparent_wind_angle < 0)
                    apparent_wind_angle += 360.;
                if(apparent_wind_angle > 360.)
                    apparent_wind_angle -= 360.;
                
                
                //  Using the "Law of cosines", compute the true wind speed
                if( !wxIsNaN(osSog) ){
                        true_windSpeed = sqrt( (osSog * osSog) + (app_windSpeed * app_windSpeed) - (2 * osSog * app_windSpeed * cos(apparent_wind_angle * PI / 180.)) );
                }
                else{
                        true_windSpeed = app_windSpeed;
                }
                    
                    // Rearranging the Law of cosines, we calculate True Wind Direction
                if( ( !wxIsNaN(osSog) ) && ( !wxIsNaN(osHead) )  && ( osSog > 0.2)  &&  (true_windSpeed > 1) ){
                        double acosTW = ((osSog * osSog) + (true_windSpeed * true_windSpeed) - (app_windSpeed * app_windSpeed)) / (2 * osSog * true_windSpeed);
                        
                        double twd0 = acos( acosTW) *  ( 180. / PI );
                        
                        // OK on the beat...
                        if(apparent_wind_angle > 180.){
                            true_windDirection = osHead + 180 + twd0;
                        }
                        else{
                            true_windDirection = osHead + 180 - twd0;
                        }                    
                }
                else{
                        true_windDirection = wind_angle_mag + variation;
                }
                    
                if(true_windDirection < 0)
                        true_windDirection += 360.;
                if(true_windDirection > 360.)
                        true_windDirection -= 360.;
                    
                //qDebug() << wind_angle_mag << app_windSpeed << apparent_wind_angle << true_windSpeed << true_windDirection;
                    
                if( s_pAndroidNMEAMessageConsumer ) {
                        
                        NMEA0183        parser;
                    
                        // Now make some NMEA messages
                        // We dont want to pass the incoming MWD message thru directly, since it is not really correct.  The angle is correct, but the speed is relative.
                        //  Make a new MWD sentence with calculated values
                        parser.TalkerID = _T("OS");
                        
                        // MWD
                        SENTENCE sntd;
                        parser.Mwd.WindAngleTrue = true_windDirection; 
                        parser.Mwd.WindAngleMagnetic = wind_angle_mag;
                        parser.Mwd.WindSpeedKnots = true_windSpeed;
                        parser.Mwd.WindSpeedms = true_windSpeed * 0.5144;           // convert kts to m/s
                        parser.Mwd.Write( sntd );
                        sendNMEAMessageEvent(sntd.Sentence);
                        
                        // Now make two MWV sentences
                        // Apparent
                        SENTENCE snt;
                        parser.Mwv.WindAngle = apparent_wind_angle;
                        parser.Mwv.WindSpeed = app_windSpeed;
                        parser.Mwv.WindSpeedUnits = _T("N");
                        parser.Mwv.Reference = _T("R");
                        parser.Mwv.IsDataValid = NTrue;
                        parser.Mwv.Write( snt );
                        sendNMEAMessageEvent(snt.Sentence);
                        
                        // True
                        SENTENCE sntt;
                        double true_relHead = 0;
                        if( !wxIsNaN(osHead) && ( (!buseCOG)  ||  (buseCOG && osSog > 0.2) ) )
                            true_relHead = true_windDirection - osHead;
                        
                        if(true_relHead < 0)
                            true_relHead += 360.;
                        if(true_relHead > 360.)
                            true_relHead -= 360.;
                        
                        parser.Mwv.WindAngle = true_relHead;
                        parser.Mwv.WindSpeed = true_windSpeed;
                        parser.Mwv.WindSpeedUnits = _T("N");
                        parser.Mwv.Reference = _T("T");
                        parser.Mwv.IsDataValid = NTrue;
                        parser.Mwv.Write( sntt );
                        sendNMEAMessageEvent(sntt.Sentence);
                        
                }
            }
        }
        
        return 52;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_processNMEA(JNIEnv *env, jobject obj, jstring nmea_string)
    {
        //  The NMEA message target handler may not be setup yet, if no connections are defined or enabled.
        //  But we may get synthesized messages from the Java app, even without a definite connection, and we want to process these messages too.
        //  So assume that the global MUX, if present, will handle these messages.
        wxEvtHandler  *consumer = s_pAndroidNMEAMessageConsumer;
        
        if( !consumer && g_pMUX ) 
            consumer = g_pMUX;
                
            
        const char *string = env->GetStringUTFChars(nmea_string, NULL);

        //qDebug() << "ProcessNMEA: " << string;
        
        char tstr[200];
        strncpy(tstr, string, 190);
        strcat(tstr, "\r\n");
        
        if( consumer ) {
            OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
            Nevent.SetNMEAString( tstr );
            Nevent.SetStream( NULL );
                
            consumer->AddPendingEvent(Nevent);
        }
        
        return 66;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_processBTNMEA(JNIEnv *env, jobject obj, jstring nmea_string)
    {
        const char *string = env->GetStringUTFChars(nmea_string, NULL);
        wxString wstring = wxString(string, wxConvUTF8);
        
        char tstr[200];
        strncpy(tstr, string, 190);
        strcat(tstr, "\r\n");
        
        if( s_pAndroidBTNMEAMessageConsumer ) {
            OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
            Nevent.SetNMEAString( tstr );
            Nevent.SetStream( NULL );
            
            s_pAndroidBTNMEAMessageConsumer->AddPendingEvent(Nevent);
        }
        
        return 77;
    }
}


extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onConfigChange(JNIEnv *env, jobject obj)
    {
        qDebug() << "onConfigChange";

        wxLogMessage(_T("onConfigChange"));
        GetAndroidDisplaySize();
        
        wxSize new_size = getAndroidDisplayDimensions();
        qDebug() << "NewSize: " << new_size.x << new_size.y;
        config_size = new_size;
        
//         wxCommandEvent evts(wxEVT_COMMAND_MENU_SELECTED);
//         evts.SetId( ID_CMD_STOP_RESIZE );
//             g_androidUtilHandler->AddPendingEvent(evts);

        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        evt.SetId( ID_CMD_TRIGGER_RESIZE );
            g_androidUtilHandler->AddPendingEvent(evt);
 

        return 77;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onMouseWheel(JNIEnv *env, jobject obj, int dir)
    {
        
        wxMouseEvent evt(wxEVT_MOUSEWHEEL);
        evt.m_wheelRotation = dir;

        if(gFrame->GetPrimaryCanvas()){
            gFrame->GetPrimaryCanvas()->GetEventHandler()->AddPendingEvent(evt);
        }
        
        return 77;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onMenuKey(JNIEnv *env, jobject obj)
    {

//         if(g_MainToolbar){
//             g_MainToolbar->Show( !g_MainToolbar->IsShown() );
//         }
            
        return 88;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onStop(JNIEnv *env, jobject obj)
    {
        qDebug() << "onStop";
        wxLogMessage(_T("onStop"));
        
        
        //  App may be summarily killed after this point due to OOM condition.
        //  So we need to persist some dynamic data.
        if(pConfig){
        
        //  Persist the config file, especially to capture the viewport location,scale etc.
            pConfig->UpdateSettings();
        
        //  There may be unsaved objects at this point, and a navobj.xml.changes restore file
        //  We commit the navobj deltas, and flush the restore file 
        //  Pass flag "true" to also recreate a new empty "changes" file    
            pConfig->UpdateNavObj( true );

        }
        
        g_running = false;

        qDebug() << "onStop return 98";
        return 98;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onStart(JNIEnv *env, jobject obj)
    {
        qDebug() << "onStart";
        if(g_logger)
            wxLogMessage(_T("onStart"));
        
        if(g_bstress1) ShowNavWarning();
        
        g_running = true;

        return 99;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onPause(JNIEnv *env, jobject obj)
    {
        qDebug() << "onPause";
        wxLogMessage(_T("onPause"));
        g_bSleep = true;
        
        callActivityMethod_is("setTrackContinuous", (int)g_btrackContinuous);

        if(!g_btrackContinuous)
            androidGPSService( GPS_OFF );
        
        return 97;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onResume(JNIEnv *env, jobject obj)
    {
        qDebug() << "onResume";
        if(g_logger)
            wxLogMessage(_T("onResume"));
        
        int ret = 96;
        
        g_bSleep = false;
        
        if(bGPSEnabled)
            androidGPSService( GPS_ON );
        
        wxCommandEvent evt0(wxEVT_COMMAND_MENU_SELECTED);
        evt0.SetId( ID_CMD_CLOSE_ALL_DIALOGS );
        if(gFrame && gFrame->GetEventHandler())
            gFrame->GetEventHandler()->AddPendingEvent(evt0);
        
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        evt.SetId( ID_CMD_INVALIDATE );
        if(gFrame)
            gFrame->GetEventHandler()->AddPendingEvent(evt);

        //  Check screen orientation is sensible
        int orient = androidGetScreenOrientation();
        qDebug() << "Orient: " << orient;
        if(gFrame && gFrame->GetPrimaryCanvas()){
            qDebug() << "Size: " << gFrame->GetSize().x << gFrame->GetSize().y;
            qDebug() << "CanvasSize: " << gFrame->GetPrimaryCanvas()->GetSize().x << gFrame->GetPrimaryCanvas()->GetSize().y;
            
            if(gFrame->GetSize().y > gFrame->GetSize().x){
                qDebug() << "gFrame is Portrait";
                if((orient == 2) || (orient == 4)){
                    qDebug() << "NEEDS RESIZE";
                    GetAndroidDisplaySize();
                    wxSize new_size = getAndroidDisplayDimensions();
                    qDebug() << "NewSize: " << new_size.x << new_size.y;
                    config_size = new_size;
        
                    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
                    evt.SetId( ID_CMD_TRIGGER_RESIZE );
                    if(g_androidUtilHandler)
                        g_androidUtilHandler->AddPendingEvent(evt);
                }
            }
            else{
                qDebug() << "gFrame is Landscape";
                if((orient == 1) || (orient == 3)){
                    qDebug() << "NEEDS RESIZE";
                    GetAndroidDisplaySize();
                    wxSize new_size = getAndroidDisplayDimensions();
                    qDebug() << "NewSize: " << new_size.x << new_size.y;
                    config_size = new_size;
        
                    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
                    evt.SetId( ID_CMD_TRIGGER_RESIZE );
                    if(g_androidUtilHandler)
                        g_androidUtilHandler->AddPendingEvent(evt);
                }
            }
        }

        return ret;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onDestroy(JNIEnv *env, jobject obj)
    {
        qDebug() << "onDestroy";
        wxLogMessage(_T("onDestroy"));
        
        if(pConfig){
            //  Persist the config file, especially to capture the viewport location,scale, locale etc.
            pConfig->UpdateSettings();
        }
        
        g_running = false;
        
        return 98;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_selectChartDisplay(JNIEnv *env, jobject obj, int type, int family)
    {
        
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        if(type == CHART_TYPE_CM93COMP){
            evt.SetId( ID_CMD_SELECT_CHART_TYPE );
            evt.SetExtraLong( CHART_TYPE_CM93COMP);
        }
        else{
            evt.SetId( ID_CMD_SELECT_CHART_FAMILY );
            evt.SetExtraLong( family);
        }
        
        if(gFrame)
            gFrame->GetEventHandler()->AddPendingEvent(evt);
        
        return 74;
    }
}
    
extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_invokeCmdEventCmdString(JNIEnv *env, jobject obj, int cmd_id, jstring s)
    {
         const char *sparm;
        wxString wx_sparm;
        JNIEnv* jenv;
        
        //  Need a Java environment to decode the string parameter
        if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
            //qDebug() << "GetEnv failed.";
        }
        else {
            sparm = (jenv)->GetStringUTFChars(s, NULL);
            wx_sparm = wxString(sparm, wxConvUTF8);
        }
 
        //qDebug() << "invokeCmdEventCmdString" << cmd_id << s;
        
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        evt.SetId( cmd_id );
        evt.SetString( wx_sparm);
        
        if(gFrame){
            qDebug() << "add event" << cmd_id << wx_sparm.mbc_str();
            gFrame->GetEventHandler()->AddPendingEvent(evt);
        }
        else
            qDebug() << "No frame for EventCmdString";

        
        return 71;
    }
}
    
        
extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_invokeMenuItem(JNIEnv *env, jobject obj, int item)
    {
        if(!gFrame)                     // App Frame not yet set up, on slow devices
            return 71;
        
        wxString msg1;
        msg1.Printf(_T("invokeMenuItem: %d"), item);
        wxLogMessage(msg1);
        
        // If in Route Create, disable all other menu items
         if( gFrame && (gFrame->GetFocusCanvas()->m_routeState > 1 ) && (OCPN_ACTION_ROUTE != item) ) {
             wxLogMessage(_T("invokeMenuItem A"));
             return 72;
         }

        wxLogMessage(_T("invokeMenuItem B"));
        
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        
        switch(item){
            case OCPN_ACTION_FOLLOW:
                evt.SetId( ID_MENU_NAV_FOLLOW );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_ROUTE:
                evt.SetId( ID_MENU_ROUTE_NEW );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_RMD:
                evt.SetId( ID_MENU_ROUTE_MANAGER );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_SETTINGS_BASIC:
                evt.SetId( ID_MENU_SETTINGS_BASIC );
                wxLogMessage(_T("invokeMenuItem OCPN_ACTION_SETTINGS_BASIC"));
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_TRACK_TOGGLE:
                evt.SetId( ID_MENU_NAV_TRACK );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_MOB:
                evt.SetId( ID_MENU_MARK_MOB );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_TIDES_TOGGLE:
                evt.SetId( ID_MENU_SHOW_TIDES );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_CURRENTS_TOGGLE:
                evt.SetId( ID_MENU_SHOW_CURRENTS );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_ENCTEXT_TOGGLE:
                evt.SetId( ID_MENU_ENC_TEXT );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_ENCSOUNDINGS_TOGGLE:
                evt.SetId( ID_MENU_ENC_SOUNDINGS );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_ENCLIGHTS_TOGGLE:
                evt.SetId( ID_MENU_ENC_LIGHTS );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            case OCPN_ACTION_PREPARE_SHUTDOWN:
                evt.SetId( ID_MENU_PREPARE_SHUTDOWN );
                gFrame->GetEventHandler()->AddPendingEvent(evt);
                break;
                
            default:
                break;
        }
        
        return 73;
    }
}


extern "C"{
    JNIEXPORT jstring JNICALL Java_org_opencpn_OCPNNativeLib_getVPCorners(JNIEnv *env, jobject obj)
    {
//        qDebug() << "getVPCorners";
        
        wxString s;
        
        if(gFrame->GetPrimaryCanvas()){
            LLBBox vbox;
            vbox = gFrame->GetPrimaryCanvas()->GetVP().GetBBox();
            s.Printf(_T("%g;%g;%g;%g;"), vbox.GetMaxLat(), vbox.GetMaxLon(), vbox.GetMinLat(), vbox.GetMinLon());  
        }
                    
        jstring ret = (env)->NewStringUTF(s.c_str());
        
        return ret;
    }
        
}       

extern "C"{
    JNIEXPORT jstring JNICALL Java_org_opencpn_OCPNNativeLib_getVPS(JNIEnv *env, jobject obj)
    {
        wxString s;
        
        if(gFrame->GetPrimaryCanvas()){
            ViewPort vp = gFrame->GetPrimaryCanvas()->GetVP();
            s.Printf(_T("%g;%g;%g;%g;%g;"), vp.clat, vp.clon, vp.view_scale_ppm, gLat, gLon);  
            
        }
        
        jstring ret = (env)->NewStringUTF(s.c_str());
        
        return ret;
    }
    
}       

extern "C"{
    JNIEXPORT int JNICALL Java_org_opencpn_OCPNNativeLib_getTLWCount(JNIEnv *env, jobject obj)
    {
        int ret = 0;
        wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
        while (node)
        {
            wxWindow* win = node->GetData();
            if(win->IsShown() && !win->IsKindOf( CLASSINFO( CanvasOptions )))
                ret++;
            
            node = node->GetNext();
        }
        return ret;
    }
}       

extern "C"{
    JNIEXPORT int JNICALL Java_org_opencpn_OCPNNativeLib_notifyFullscreenChange(JNIEnv *env, jobject obj, bool bFull)
    {
        g_bFullscreen = bFull;
        return 1;
    }    
}       


extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_setDownloadStatus(JNIEnv *env, jobject obj, int status, jstring url)
    {
//        qDebug() << "setDownloadStatus";
 
        const char *sparm;
        wxString wx_sparm;
        JNIEnv* jenv;
        
        //  Need a Java environment to decode the string parameter
        if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
            //qDebug() << "GetEnv failed.";
        }
        else {
            sparm = (jenv)->GetStringUTFChars(url, NULL);
            wx_sparm = wxString(sparm, wxConvUTF8);
        }
        
        if(s_bdownloading && wx_sparm.IsSameAs(s_requested_url) ){
            
//            qDebug() << "Maybe mine...";
            //  We simply pass the event on to the core download manager methods,
            //  with parameters crafted to the event
            OCPN_downloadEvent ev(wxEVT_DOWNLOAD_EVENT, 0);
            
            OCPN_DLCondition dl_condition = OCPN_DL_EVENT_TYPE_UNKNOWN;
            OCPN_DLStatus dl_status = OCPN_DL_UNKNOWN;
            
            //  Translate Android status values to OCPN 
            switch (status){
                case 16:                                // STATUS_FAILED
                    dl_condition = OCPN_DL_EVENT_TYPE_END;
                    dl_status = OCPN_DL_FAILED;
                    break;
                    
                case 8:                                 // STATUS_SUCCESSFUL
                    dl_condition = OCPN_DL_EVENT_TYPE_END;
                    dl_status = OCPN_DL_NO_ERROR;
                    break;
                    
                case 4:                                 //  STATUS_PAUSED
                case 2:                                 //  STATUS_RUNNING 
                case 1:                                 //  STATUS_PENDING
                   dl_condition = OCPN_DL_EVENT_TYPE_PROGRESS;
                   dl_status = OCPN_DL_NO_ERROR;
            }
                   
            ev.setDLEventCondition( dl_condition );
            ev.setDLEventStatus( dl_status );
            
            if(s_download_evHandler){
//                qDebug() << "Sending event...";
                s_download_evHandler->AddPendingEvent(ev);
            }
            
            
        }
       
        
        return 77;
    }
    
}       

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_sendPluginMessage(JNIEnv *env, jobject obj, jstring msgID, jstring msg)
    {
        const char *sparm;
        wxString MsgID;
        wxString Msg;
        JNIEnv* jenv;
        
        //  Need a Java environment to decode the string parameter
        if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
            //qDebug() << "GetEnv failed.";
        }
        else {
            sparm = (jenv)->GetStringUTFChars(msgID, NULL);
            MsgID = wxString(sparm, wxConvUTF8);
            
            sparm = (jenv)->GetStringUTFChars(msg, NULL);
            Msg = wxString(sparm, wxConvUTF8);
            
        }
        
        SendPluginMessage( MsgID, Msg );
        
        return 74;
    }
}

void androidTerminate(){
    callActivityMethod_vs("terminateApp");
}


bool CheckPendingJNIException()
{
    JNIEnv* jenv;
    
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) 
        return true;

    if( (jenv)->ExceptionCheck() == JNI_TRUE ) {

        // Handle exception here.
        (jenv)->ExceptionDescribe(); // writes to logcat
        (jenv)->ExceptionClear();
        
        return false;           // There was a pending exception, but cleared OK
                                // interesting discussion:  http://blog.httrack.com/blog/2013/08/23/catching-posix-signals-on-android/
    }
    
    return false;
    
}


wxString callActivityMethod_vs(const char *method)
{
    if(CheckPendingJNIException())
        return _T("NOK");
    
    JNIEnv* jenv;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod(method, "()Ljava/lang/String;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    jstring s = data.object<jstring>();
    //qDebug() << s;
    
    if(s){
        //  Need a Java environment to decode the resulting string
        if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
            //qDebug() << "GetEnv failed.";
        }
        else {
            const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
            return_string = wxString(ret_string, wxConvUTF8);
        }
    }
    
    return return_string;
}



wxString callActivityMethod_is(const char *method, int parm)
{
    if(CheckPendingJNIException())
        return _T("NOK");
    JNIEnv* jenv;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod(method, "(I)Ljava/lang/String;", parm);
    if(CheckPendingJNIException())
        return _T("NOK");
    
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
    
}

wxString callActivityMethod_iis(const char *method, int parm1, int parm2)
{
    if(CheckPendingJNIException())
        return _T("NOK");
   
    JNIEnv* jenv;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod(method, "(II)Ljava/lang/String;", parm1, parm2);
    if(CheckPendingJNIException())
        return _T("NOK");
    
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
    
}

wxString callActivityMethod_ss(const char *method, wxString parm)
{
    if(CheckPendingJNIException())
        return _T("NOK");
    JNIEnv* jenv;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return return_string;
    }

    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
        return _T("jenv Error");
    }
    
    jstring p = (jenv)->NewStringUTF(parm.c_str());
    
    
    //  Call the desired method
    //qDebug() << "Calling method_ss";
    //qDebug() << method;
    
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;)Ljava/lang/String;", p);
    
    (jenv)->DeleteLocalRef(p);
    
    if(CheckPendingJNIException())
        return _T("NOK");
    
    //qDebug() << "Back from method_ss";
    
    jstring s = data.object<jstring>();
    
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
    
}

wxString callActivityMethod_s2s(const char *method, wxString parm1, wxString parm2)
{
    if(CheckPendingJNIException())
        return _T("NOK");
    JNIEnv* jenv;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
        return _T("jenv Error");
    }
    
    wxCharBuffer p1b = parm1.ToUTF8();
    jstring p1 = (jenv)->NewStringUTF(p1b.data());
    
    wxCharBuffer p2b = parm2.ToUTF8();
    jstring p2 = (jenv)->NewStringUTF(p2b.data());
    
    //  Call the desired method
    //qDebug() << "Calling method_s2s" << " (" << method << ")";
    
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1, p2);

    (jenv)->DeleteLocalRef(p1);
    (jenv)->DeleteLocalRef(p2);
    
    if(CheckPendingJNIException())
        return _T("NOK");
    
    //qDebug() << "Back from method_s2s";
    
    jstring s = data.object<jstring>();
    
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
    
}

wxString callActivityMethod_s3s(const char *method, wxString parm1, wxString parm2, wxString parm3)
{
    if(CheckPendingJNIException())
        return _T("NOK");
    JNIEnv* jenv;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    if ( !activity.isValid() ){
        return return_string;
    }
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        return _T("jenv Error");
    }
    
    wxCharBuffer p1b = parm1.ToUTF8();
    jstring p1 = (jenv)->NewStringUTF(p1b.data());
    
    wxCharBuffer p2b = parm2.ToUTF8();
    jstring p2 = (jenv)->NewStringUTF(p2b.data());
    
    wxCharBuffer p3b = parm3.ToUTF8();
    jstring p3 = (jenv)->NewStringUTF(p3b.data());
    
    //  Call the desired method
    //qDebug() << "Calling method_s3s" << " (" << method << ")";
    
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
                                                       p1, p2, p3);
    (jenv)->DeleteLocalRef(p1);
    (jenv)->DeleteLocalRef(p2);
    (jenv)->DeleteLocalRef(p3);
    
    if(CheckPendingJNIException())
        return _T("NOK");

    //qDebug() << "Back from method_s3s";
        
    jstring s = data.object<jstring>();
        
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
        
    return return_string;
        
}


wxString callActivityMethod_s4s(const char *method, wxString parm1, wxString parm2, wxString parm3, wxString parm4)
{
    if(CheckPendingJNIException())
        return _T("NOK");
    JNIEnv* jenv;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
        return _T("jenv Error");
    }
    
    wxCharBuffer p1b = parm1.ToUTF8();
    jstring p1 = (jenv)->NewStringUTF(p1b.data());
    
    wxCharBuffer p2b = parm2.ToUTF8();
    jstring p2 = (jenv)->NewStringUTF(p2b.data());
    
    wxCharBuffer p3b = parm3.ToUTF8();
    jstring p3 = (jenv)->NewStringUTF(p3b.data());
    
    wxCharBuffer p4b = parm4.ToUTF8();
    jstring p4 = (jenv)->NewStringUTF(p4b.data());

    //const char *ts = (jenv)->GetStringUTFChars(p2, NULL);
    //qDebug() << "Test String p2" << ts;
    
    //  Call the desired method
    //qDebug() << "Calling method_s4s" << " (" << method << ")";
    
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
                                                       p1, p2, p3, p4);
    (jenv)->DeleteLocalRef(p1);
    (jenv)->DeleteLocalRef(p2);
    (jenv)->DeleteLocalRef(p3);
    (jenv)->DeleteLocalRef(p4);
    
    if(CheckPendingJNIException())
        return _T("NOK");
    
    //qDebug() << "Back from method_s4s";
    
    jstring s = data.object<jstring>();
    
     if( (jenv)->GetStringLength( s )){
         const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
         return_string = wxString(ret_string, wxConvUTF8);
     }
    
    return return_string;
    
}

wxString callActivityMethod_s2s2i(const char *method, wxString parm1, wxString parm2, int parm3, int parm4)
{
    if(CheckPendingJNIException())
        return _T("NOK");
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    if ( !activity.isValid() ){
        return return_string;
    }
    
    //  Need a Java environment to decode the resulting string
    JNIEnv* jenv;
    
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
        return _T("jenv Error");
    }
    
    wxCharBuffer p1b = parm1.ToUTF8();
    jstring p1 = (jenv)->NewStringUTF(p1b.data());

    wxCharBuffer p2b = parm2.ToUTF8();
    jstring p2 = (jenv)->NewStringUTF(p2b.data());
    
    //qDebug() << "Calling method_s2s2i" << " (" << method << ")";
    //qDebug() << parm3 << parm4;
    
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;Ljava/lang/String;II)Ljava/lang/String;",
                                                       p1, p2, parm3, parm4);
    
    (jenv)->DeleteLocalRef(p1);
    (jenv)->DeleteLocalRef(p2);
    
    if(CheckPendingJNIException())
        return _T("NOK");
    
    jstring s = data.object<jstring>();
        
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
             return_string = wxString(ret_string, wxConvUTF8);
     }
        
    return return_string;
        
}


wxString callActivityMethod_ssi(const char *method, wxString parm1, int parm2)
{
    if(CheckPendingJNIException())
        return _T("NOK");
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return _T("NOK");
    
    if ( !activity.isValid() ){
        return return_string;
    }
    
    //  Need a Java environment to decode the resulting string
    JNIEnv* jenv;
    
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
        return _T("jenv Error");
    }
    
    wxCharBuffer p1b = parm1.ToUTF8();
    jstring p1 = (jenv)->NewStringUTF(p1b.data());

    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;I)Ljava/lang/String;", p1, parm2);
    
    (jenv)->DeleteLocalRef(p1);
    
    if(CheckPendingJNIException())
        return _T("NOK");
    
    jstring s = data.object<jstring>();
        
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
             return_string = wxString(ret_string, wxConvUTF8);
    }
        
    return return_string;
        
}

wxString androidGetAndroidSystemLocale()
{
    return callActivityMethod_vs("getAndroidLocaleString");
}   
    
bool androidGetFullscreen()
{
    wxString s = callActivityMethod_vs("getFullscreen");
    
    return s == _T("YES");
}

bool androidSetFullscreen( bool bFull )
{
    callActivityMethod_is("setFullscreen", (int)bFull);
    
    return true;
}

void androidDisableFullScreen()
{
    if(g_bFullscreen){
        g_bFullscreenSave = true;
        androidSetFullscreen( false );
    }
}

void androidRestoreFullScreen()
{
    if(g_bFullscreenSave){
        g_bFullscreenSave = false;
        androidSetFullscreen( true );
    }
}

int androidGetScreenOrientation(){
    wxString s = callActivityMethod_vs("getScreenOrientation");
    long result = -1;
    s.ToLong(&result);
    return result;
}
    
void androidLaunchHelpView()
{
    qDebug() << "androidLaunchHelpView ";
    wxString val = callActivityMethod_vs("isHelpAvailable");
    if(val.IsSameAs(_T("YES"))){
        callActivityMethod_vs("launchHelpBook");
    }
    else{
        wxString msg = _("OpenCPN Help is not installed.\nWould you like to install from Google PlayStore now?");
        if(androidShowSimpleYesNoDialog( _T("OpenCPN"), msg ))
            androidInstallPlaystoreHelp();
    }
}

void androidLaunchBrowser( wxString URL )
{
    qDebug() << "androidLaunchBrowser";
    callActivityMethod_ss("launchWebView", URL);
}

void androidDisplayTimedToast(wxString message, int timeMillisec)
{
    callActivityMethod_ssi("showTimedToast", message, timeMillisec);
}

void androidCancelTimedToast()
{
    callActivityMethod_vs("cancelTimedToast");
}

void androidDisplayToast(wxString message)
{
    callActivityMethod_ss("showToast", message);
}

void androidEnableRotation( void )
{
    callActivityMethod_vs("EnableRotation");
}

void androidDisableRotation( void )
{
    callActivityMethod_vs("DisableRotation");
}

bool androidShowDisclaimer( wxString title, wxString msg )
{
    if(CheckPendingJNIException())
        return false;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return false;
    
    if ( !activity.isValid() )
        return false;
    
    JNIEnv* jenv;
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) 
        return false;
    
    
    wxCharBuffer p1b = title.ToUTF8();
    jstring p1 = (jenv)->NewStringUTF(p1b.data());
    
    // Convert for wxString-UTF8  to jstring-UTF16
    wxWCharBuffer b = msg.wc_str();
    jstring p2 = (jenv)->NewString( (jchar *)b.data(), msg.Len() * 2);
    
    QAndroidJniObject data = activity.callObjectMethod( "disclaimerDialog", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1, p2);
    
    (jenv)->DeleteLocalRef(p1);
    (jenv)->DeleteLocalRef(p2);
    
    if(CheckPendingJNIException())
        return false;
        
    jstring s = data.object<jstring>();
        
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
        
        
    return (return_string == _T("OK"));
}

bool androidShowSimpleOKDialog( wxString title, wxString msg )
{
    if(CheckPendingJNIException())
        return false;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return false;
    
    if ( !activity.isValid() )
        return false;
    
    JNIEnv* jenv;
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) 
        return false;
    
    
    wxCharBuffer p1b = title.ToUTF8();
    jstring p1 = (jenv)->NewStringUTF(p1b.data());
    
    // Convert for wxString-UTF8  to jstring-UTF16
    wxWCharBuffer b = msg.wc_str();
    jstring p2 = (jenv)->NewString( (jchar *)b.data(), msg.Len() * 2);
    
    QAndroidJniObject data = activity.callObjectMethod( "simpleOKDialog", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1, p2);
    
    (jenv)->DeleteLocalRef(p1);
    (jenv)->DeleteLocalRef(p2);
    
    if(CheckPendingJNIException())
        return false;
        
    jstring s = data.object<jstring>();
        
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
        
        
    return (return_string == _T("OK"));
}

bool androidShowSimpleYesNoDialog( wxString title, wxString msg )
{
    if(CheckPendingJNIException())
        return false;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return false;
    
    if ( !activity.isValid() )
        return false;
    
    JNIEnv* jenv;
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) 
        return false;
    
    
    wxCharBuffer p1b = title.ToUTF8();
    jstring p1 = (jenv)->NewStringUTF(p1b.data());
    
    // Convert for wxString-UTF8  to jstring-UTF16
    wxWCharBuffer b = msg.wc_str();
    jstring p2 = (jenv)->NewString( (jchar *)b.data(), msg.Len() * 2);
    
    QAndroidJniObject data = activity.callObjectMethod( "simpleYesNoDialog", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1, p2);
    
    (jenv)->DeleteLocalRef(p1);
    (jenv)->DeleteLocalRef(p2);
    
    if(CheckPendingJNIException())
        return false;
        
    jstring s = data.object<jstring>();
        
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
        
        
    return (return_string == _T("YES"));
}

bool androidInstallPlaystoreHelp()
{
   qDebug() << "androidInstallPlaystoreHelp";
 //  return false;
   
   if(CheckPendingJNIException())
        return false;
    
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if(CheckPendingJNIException())
        return false;
    
    if ( !activity.isValid() )
        return false;
    
    JNIEnv* jenv;
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) 
        return false;
    
    
    QAndroidJniObject data = activity.callObjectMethod( "installPlaystoreHelp", "()Ljava/lang/String;");
    
    if(CheckPendingJNIException())
        return false;
        
    jstring s = data.object<jstring>();
        
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }

    return (return_string == _T("OK"));
}


int androidGetTZOffsetMins()
{
    // Get the TZ offset (from UTC) of the local machine, in minutes.  Includes DST, if applicable
    wxString result = callActivityMethod_vs("getAndroidTZOffsetMinutes");
    qDebug() << "androidGetTZOffsetMins result: " << result.mb_str();
    long value = 0;
    result.ToLong(&value);
    return (int)value;
}   

extern PlatSpec android_plat_spc;

wxString androidGetDeviceInfo()
{
    if(!g_deviceInfo.Length())
        g_deviceInfo = callActivityMethod_vs("getDeviceInfo");
    
    wxStringTokenizer tkz(g_deviceInfo, _T("\n"));
    while( tkz.HasMoreTokens() )
    {
        wxString s1 = tkz.GetNextToken();
        if(wxNOT_FOUND != s1.Find(_T("OS API Level"))){
            int a = s1.Find(_T("{"));
            if(wxNOT_FOUND != a){
                wxString b = s1.Mid(a+1, 2);
                memset(android_plat_spc.msdk, 0, sizeof(android_plat_spc.msdk));
                strncpy(android_plat_spc.msdk, b.c_str(), 2);
            }
        }
        if(wxNOT_FOUND != s1.Find(_T("opencpn"))){
            strcpy(&android_plat_spc.hn[0], s1.c_str());
        }
    }
    
    return g_deviceInfo;
}

wxString androidGetHomeDir()
{
    return g_androidFilesDir + _T("/");
}

wxString androidGetPrivateDir()                 // Used for logfile, config file, navobj, and the like
{
    if(g_bExternalApp){
        
        // should check storage availability
#if 0
/* Checks if external storage is available for read and write */
        public boolean isExternalStorageWritable() {
            String state = Environment.getExternalStorageState();
            if (Environment.MEDIA_MOUNTED.equals(state)) {
                return true;
            }
            return false;
        }
        
        /* Checks if external storage is available to at least read */
        public boolean isExternalStorageReadable() {
            String state = Environment.getExternalStorageState();
            if (Environment.MEDIA_MOUNTED.equals(state) ||
                Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
                return true;
                }
                return false;
        }
#endif        
        return g_androidExtFilesDir;
    }

    // We choose to use the ExtFiles directory always , so that the contents of logfiles, navobjs, etc.
    // may always be accessible by simple Android File Explorers...
    return g_androidExtFilesDir;    
}

wxString androidGetSharedDir()                 // Used for assets like uidata, s57data, etc
{
//    if(g_bExternalApp){
//        if(g_androidExtFilesDir.Length())
//            return g_androidExtFilesDir + _T("/");
//    }
    
    return g_androidExtFilesDir + _T("/");
}

wxString androidGetCacheDir()                 // Used for raster_texture_cache, mmsitoname.csv, etc
{
//    if(g_bExternalApp){
//        if(g_androidExtCacheDir.Length())
//            return g_androidExtCacheDir;
//    }
    
    return g_androidExtCacheDir;
}

// Android notes:
/* Note: don't be confused by the word "external" here.
 * This directory can better be thought as media/shared storage.
 * It is a filesystem that can hold a relatively large amount of data
 * and that is shared across all applications (does not enforce permissions).
 * Traditionally this is an SD card, but it may also be implemented as built-in storage
 * in a device that is distinct from the protected internal storage
 * and can be mounted as a filesystem on a computer.
 */

wxString androidGetExtStorageDir()                 // Used for Chart storage, typically
{
    return g_androidExtStorageDir;
}

extern void androidSetRouteAnnunciator(bool viz)
{
    callActivityMethod_is("setRouteAnnunciator", viz?1:0);
}

extern void androidSetFollowTool(int state, bool forceUpdate)
{
//    qDebug() << "setFollowIconState" << bactive;
    
    if( (g_follow_state != state) || forceUpdate)
        callActivityMethod_is("setFollowIconState", state);
    
    g_follow_state = state;
}

extern void androidSetTrackTool(bool bactive)
{
    if(g_track_active != bactive)
        callActivityMethod_is("setTrackIconState", bactive?1:0);
    
    g_track_active = bactive;
}


void androidSetChartTypeMaskSel( int mask, wxString &indicator)
{
    int sel = 0;
    if(wxNOT_FOUND != indicator.Find( _T("raster")))
        sel = 1;
    else if(wxNOT_FOUND != indicator.Find( _T("vector")))
        sel = 2;
    else if(wxNOT_FOUND != indicator.Find( _T("cm93")))
        sel = 4;

    if((g_mask != mask) || (g_sel != sel)){
//        qDebug() << "androidSetChartTypeMaskSel" << mask << sel;
        callActivityMethod_iis("configureNavSpinnerTS", mask, sel);
        g_mask = mask;
        g_sel = sel;
    }
}       


void androidEnableBackButton(bool benable)
{
    callActivityMethod_is("setBackButtonState", benable?1:0);
    g_backEnabled = benable;
}

void androidEnableBackButtonCheck(bool benable)
{
    if(g_backEnabled != benable)
        androidEnableBackButton(benable);
}


bool androidGetMemoryStatus( int *mem_total, int *mem_used )
{
    
    //  On android, We arbitrarily declare that we have used 50% of available memory.
    if(mem_total)
        *mem_total = s_androidMemTotal * 1024;
    if(mem_used)
        *mem_used = s_androidMemUsed * 1024;
    return true;
    
#if 0
    
    //  Get a reference to the running native activity
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
        return false;
    }

    unsigned long android_processID = wxGetProcessId();
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("getMemInfo", "(I)Ljava/lang/String;", (int)android_processID);
    
//    wxString return_string;
    jstring s = data.object<jstring>();
    
    int mu = 50;
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        mu = atoi(ret_string);
        
    }
    
    if(mem_used)
        *mem_used = mu;

        
    return true;
#endif    
}

double GetAndroidDisplaySize()
{

    double ret = 200.;          // sane default
    
    //  Get a reference to the running native activity
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return false;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("getDisplayMetrics", "()Ljava/lang/String;");
    
    wxString return_string;
    jstring s = data.object<jstring>();
    
    JNIEnv* jenv;
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    //  Return string may have commas instead of periods, if using Euro locale
    //  We just fix it here...
    return_string.Replace( _T(","), _T(".") );

    wxSize screen_size = wxSize(1,2);
    
    if(QApplication::desktop()){                // Desktop might not yet be initialized
        screen_size = ::wxGetDisplaySize();
    }
        
    wxString msg;
    
    //int ssx, ssy;
    //::wxDisplaySize(&ssx, &ssy);
    //msg.Printf(_T("wxDisplaySize(): %d %d"), ssx, ssy);
    //wxLogMessage(msg);
    
    double density = 1.0;
    long androidWidth = 2;
    long androidHeight = 1;
    long androidDmWidth = 2;
    long androidDmHeight = 1;
    long abh = 1;
    
    wxStringTokenizer tk(return_string, _T(";"));
    if( tk.HasMoreTokens() ){
        wxString token = tk.GetNextToken();     // xdpi
        token = tk.GetNextToken();              // density
        
        token.ToDouble( &density );

        token = tk.GetNextToken();              // ldpi
        
        token = tk.GetNextToken();              // width
        token.ToLong( &androidWidth );
        token = tk.GetNextToken();              // height - statusBarHeight
        token = tk.GetNextToken();              // width
        token = tk.GetNextToken();              // height
        token.ToLong( &androidHeight );
        
        token = tk.GetNextToken();              // dm.widthPixels
        token.ToLong( &androidDmWidth );
        token = tk.GetNextToken();              // dm.heightPixels
        token.ToLong( &androidDmHeight );
        
        token = tk.GetNextToken();              // actionBarHeight
        token.ToLong( &abh );

    }
    
    double ldpi = 160. * density;
    if(ldpi < 160)
        ldpi = 160.;
    
    // Find the max dimension among all possibilities
    double maxDim = wxMax(screen_size.x, screen_size.y);
    maxDim = wxMax(maxDim, androidHeight);
    maxDim = wxMax(maxDim, androidWidth);
    
    ret = (maxDim / ldpi) * 25.4;

    if(ret < 75){               // 3 inches is too small....
        double ret_bad = ret;
        ret = 100;
        msg.Printf(_T("WARNING: Android Auto Display Size OVERRIDE_TOO_SMALL: %g  ldpi: %g  density: %g correctedsize: %g "), ret_bad, ldpi, density, ret);
    }
    else if(ret > 400){         // Too large
        double ret_bad = ret;
        ret = 400;
        msg.Printf(_T("WARNING: Android Auto Display Size OVERRIDE_TOO_LARGE: %g  ldpi: %g  density: %g corrected size: %g"), ret_bad, ldpi, density, ret);
    }
    else{        
        msg.Printf(_T("Android Auto Display Size (mm, est.): %g   ldpi: %g  density: %g"), ret, ldpi, density);
    }
    
    //  Save some items as global statics for convenience
    g_androidDPmm = ldpi / 25.4;
    g_androidDensity = density;
    g_ActionBarHeight = wxMax(abh, 50);
    
    //qDebug() << "GetAndroidDisplaySize" << ldpi << g_androidDPmm;

    return ret;
}

int getAndroidActionBarHeight()
{
    return g_ActionBarHeight;
}

double getAndroidDPmm()
{
    // Returns an estimate based on the pixel density reported
    GetAndroidDisplaySize();
    
    //qDebug() << "getAndroidDPmm" << g_androidDPmm;
    
    // User override?
    if(g_config_display_size_manual && (g_config_display_size_mm > 0) ){
        double maxDim = wxMax(::wxGetDisplaySize().x, ::wxGetDisplaySize().y);
        double size_mm = g_config_display_size_mm;
        size_mm = wxMax(size_mm, 50);
        size_mm = wxMin(size_mm, 400);
        double ret = maxDim / size_mm;
        //qDebug() << "getAndroidDPmm override" << maxDim << size_mm << g_config_display_size_mm;
        
        return ret;
    }
        
        
    if(g_androidDPmm > 0.01)
        return g_androidDPmm;
    else
        return 160. / 25.4;
}

double getAndroidDisplayDensity()
{
    if( g_androidDensity < 0.01){
        GetAndroidDisplaySize();
    }
    
//    qDebug() << "g_androidDensity" << g_androidDensity;
    
    if(g_androidDensity > 0.01)
        return g_androidDensity;
    else
        return 1.0;
}
    

wxSize getAndroidDisplayDimensions( void )
{
    wxSize sz_ret = ::wxGetDisplaySize();               // default, probably reasonable, but maybe not accurate
    
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return sz_ret;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("getDisplayMetrics", "()Ljava/lang/String;");
    
    wxString return_string;
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    JNIEnv* jenv;
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    //167.802994;1.000000;160;1024;527;1024;552;1024;552;56
     wxStringTokenizer tk(return_string, _T(";"));
    if( tk.HasMoreTokens() ){
        wxString token = tk.GetNextToken();     // xdpi
        token = tk.GetNextToken();              // density
        token = tk.GetNextToken();              // densityDPI
        
        token = tk.GetNextToken();
        long a = 1000;
        token.ToLong( &a );
        sz_ret.x = a;
        
        token = tk.GetNextToken();
        long b = 1000;        
        token.ToLong( &b );
        sz_ret.y = b;
        
        token = tk.GetNextToken();              
        token = tk.GetNextToken();
        
        token = tk.GetNextToken();
        token = tk.GetNextToken();
        
        long abh = 0;
        token = tk.GetNextToken();              //  ActionBar height, if shown
        token.ToLong( &abh );
        sz_ret.y -= abh;
    }

    return sz_ret;
    
}

void androidConfirmSizeCorrection()
{
    //  There is some confusion about the ActionBar size during configuration changes.
    //  We need to confirm the calculated display size, and fix it if necessary.
    //  This happens during staged resize events 
 
    wxLogMessage(_T("androidConfirmSizeCorrection"));
    wxSize targetSize = getAndroidDisplayDimensions();
    qDebug() << "Confirming" << targetSize.y << config_size.y;
    if(config_size != targetSize){
        qDebug() << "Correcting";
        gFrame->SetSize(targetSize);
        config_size = targetSize;
    }
}
        
void androidForceFullRepaint( bool b_skipConfirm)
{
    
        wxLogMessage(_T("androidForceFullRepaint"));
        wxSize targetSize = getAndroidDisplayDimensions();
        wxSize tempSize = targetSize;
        tempSize.y--;
        gFrame->SetSize(tempSize);
       
        GetAndroidDisplaySize();
        
        wxSize new_size = getAndroidDisplayDimensions();
        config_size = new_size;
        
        g_androidUtilHandler->m_bskipConfirm = b_skipConfirm;
        
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        evt.SetId( ID_CMD_TRIGGER_RESIZE );
        if(gFrame && gFrame->GetEventHandler()){
            g_androidUtilHandler->AddPendingEvent(evt);
        }
        
}       

void androidShowBusyIcon()
{
    if(b_androidBusyShown)
        return;

    //qDebug() << "ShowBusy";
        
    //  Get a reference to the running native activity
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("showBusyCircle", "()Ljava/lang/String;");
    
    b_androidBusyShown = true;
}

void androidHideBusyIcon()
{
    if(!b_androidBusyShown)
        return;
    
    //  Get a reference to the running native activity
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("hideBusyCircle", "()Ljava/lang/String;");

    b_androidBusyShown = false;
}

int androidGetVersionCode()
{
       //  Get a reference to the running native activity
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return false;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("getAndroidVersionCode", "()Ljava/lang/String;");
    
    wxString return_string;
    jstring s = data.object<jstring>();
    
    JNIEnv* jenv;
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    long rv;
    return_string.ToLong(&rv);
    
    return rv;
}

wxString androidGetVersionName()
{
       //  Get a reference to the running native activity
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return _T("ERROR");
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("getAndroidVersionName", "()Ljava/lang/String;");
    
    wxString return_string;
    jstring s = data.object<jstring>();
    
    JNIEnv* jenv;
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
}

//---------------------------------------------------------------
//      GPS Device Support
//---------------------------------------------------------------
bool androidDeviceHasGPS()
{
    wxString query = androidGPSService( GPS_PROVIDER_AVAILABLE );
    wxLogMessage( query);
    
    bool result = query.Upper().IsSameAs(_T("YES"));
    if(result){
        //qDebug() << "Android Device has internal GPS";
        wxLogMessage(_T("Android Device has internal GPS"));
    }
    else{
        //qDebug() << "Android Device has NO internal GPS";
        wxLogMessage(_T("Android Device has NO internal GPS"));
    }
    return result;
}

bool androidStartNMEA(wxEvtHandler *consumer)
{
    s_pAndroidNMEAMessageConsumer = consumer;

    //qDebug() << "androidStartNMEA";
    wxString s;
    
    s = androidGPSService( GPS_ON );
    wxLogMessage(s);
    if(s.Upper().Find(_T("DISABLED")) != wxNOT_FOUND){
        OCPNMessageBox(NULL,
                       _("Your android device has an internal GPS, but it is disabled.\n\
                       Please visit android Settings/Location dialog to enable GPS"),
                        _T("OpenCPN"), wxOK );        
        
        androidStopNMEA();
        return false;
    }
    else
        bGPSEnabled = true;
    
    return true;
}

bool androidStopNMEA()
{
    s_pAndroidNMEAMessageConsumer = NULL;
    
    wxString s = androidGPSService( GPS_OFF );
    
    bGPSEnabled = false;
    
    return true;
}


wxString androidGPSService(int parm)
{
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return _T("Activity is not valid");
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("queryGPSServer", "(I)Ljava/lang/String;", parm);
    
    wxString return_string;
    jstring s = data.object<jstring>();
    
    if( s == NULL )
        return return_string;
    
    //  Need a Java environment to decode the resulting string
    JNIEnv* jenv;
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
     return return_string;
}


bool androidDeviceHasBlueTooth()
{
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        //qDebug() << "Activity is not valid";
        return _T("Activity is not valid");
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("hasBluetooth", "(I)Ljava/lang/String;", 0);
    
    wxString query;
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    JNIEnv* jenv;
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        //qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        query = wxString(ret_string, wxConvUTF8);
    }
    
    bool result = query.Upper().IsSameAs(_T("YES"));
    
    if(result){
        //qDebug() << "Android Device has internal Bluetooth";
        wxLogMessage(_T("Android Device has internal Bluetooth"));
    }
    else{
        //qDebug() << "Android Device has NO internal Bluetooth";
        wxLogMessage(_T("Android Device has NO internal Bluetooth"));
    }
    
    return result;
}

bool androidStartBluetoothScan()
{
    wxString result = callActivityMethod_is("startBlueToothScan", 0);
    
    return true;
    
}

bool androidStopBluetoothScan()
{
    wxString result = callActivityMethod_is("stopBlueToothScan", 0);
    
    return true;
    
}

bool androidStartBT(wxEvtHandler *consumer, wxString mac_address )
{
    s_pAndroidBTNMEAMessageConsumer = consumer;
    
    if(mac_address.Find(':') ==  wxNOT_FOUND)   //  does not look like a mac address
        return false;
    
    wxString result = callActivityMethod_ss("startBTService", mac_address);
    
    return true;
}
    
bool androidStopBT()
{
    //qDebug() << "androidStopBT";
    
    s_pAndroidBTNMEAMessageConsumer = NULL;
    
    wxString result = callActivityMethod_is("stopBTService", 0);
        
    return true;
}
    
    
wxArrayString androidGetBluetoothScanResults()
{
    wxArrayString ret_array;

    wxString result = callActivityMethod_is("getBlueToothScanResults", 0);
    
    wxStringTokenizer tk(result, _T(";"));
    while ( tk.HasMoreTokens() )
    {
        wxString token = tk.GetNextToken();
        ret_array.Add(token);
    }
    
    if(!ret_array.GetCount())
        ret_array.Add(_("Nothing found"));
    
    return ret_array;
}

bool androidSendBTMessage( wxString &payload )
{
    wxString result = callActivityMethod_ss("sendBTMessage", payload);

    return true;
}

bool androidCheckOnline()
{
    wxString val = callActivityMethod_vs("isNetworkAvailable");
    return val.IsSameAs(_T("YES"));
}

wxArrayString *androidGetSerialPortsArray( void )
{
  
    wxArrayString *pret_array = new wxArrayString;
    wxString result = callActivityMethod_is("scanSerialPorts", 0);
    
    wxStringTokenizer tk(result, _T(";"));
    while ( tk.HasMoreTokens() )
    {
        wxString token = tk.GetNextToken();
        pret_array->Add(token);
    }
    
    return pret_array;
}

bool androidStartUSBSerial(wxString &portname, wxString baudRate, wxEvtHandler *consumer)
{
    wxString result = callActivityMethod_s2s("startSerialPort", portname, baudRate);
    
    s_pAndroidNMEAMessageConsumer = consumer;
    
    return true;
}

bool androidStopUSBSerial(wxString &portname)
{
    s_pAndroidNMEAMessageConsumer = NULL;
    
    //  If app is closing down, the USB serial ports will go away automatically.
    //  So no need here.
    //  In fact, stopSerialPort() causes an occasional error when closing app.
    //  Dunno why, difficult to debug.
    if(!b_inCloseWindow)
        wxString result = callActivityMethod_ss("stopSerialPort", portname);
    
    return true;
}

bool androidWriteSerial(wxString &portname, wxString& message)
{
    wxString result = callActivityMethod_s2s("writeSerialPort", portname, message);
    return true;
}
   

int androidFileChooser( wxString *result, const wxString &initDir, const wxString &title,
                        const wxString &suggestion, const wxString &wildcard, bool dirOnly, bool addFile)
{
    wxString tresult;
    
    //  Start a timer to poll for results 
    if(g_androidUtilHandler){
        g_androidUtilHandler->m_eventTimer.Stop();
        g_androidUtilHandler->m_done = false;

        wxString activityResult;
        if(dirOnly)
            activityResult = callActivityMethod_s2s2i("DirChooserDialog", initDir, title, addFile, 0);
        
        else
            activityResult = callActivityMethod_s4s("FileChooserDialog", initDir, title, suggestion, wildcard);
        
        if(activityResult == _T("OK") ){
            //qDebug() << "ResultOK, starting spin loop";
            g_androidUtilHandler->m_action = ACTION_FILECHOOSER_END;
            g_androidUtilHandler->m_eventTimer.Start(1000, wxTIMER_CONTINUOUS);
        
        //  Spin, waiting for result
            while(!g_androidUtilHandler->m_done){
                wxMilliSleep(50);
                wxSafeYield(NULL, true);
            }
        
            //qDebug() << "out of spin loop";
            g_androidUtilHandler->m_action = ACTION_NONE;
            g_androidUtilHandler->m_eventTimer.Stop();
        
        
            tresult = g_androidUtilHandler->GetStringResult();
            
            if( tresult.StartsWith(_T("cancel:")) ){
                //qDebug() << "Cancel1";
                return wxID_CANCEL;
            }
            else if( tresult.StartsWith(_T("file:")) ){
                if(result){
                    *result = tresult.AfterFirst(':');
                    //qDebug() << "OK";
                    return wxID_OK;
                }
                else{
                    //qDebug() << "Cancel2";
                    return wxID_CANCEL;
                }
            }
        }
        else{
            //qDebug() << "Result NOT OK";
        }
        
    }

    return wxID_CANCEL;
}

    

bool InvokeJNIPreferences( wxString &initial_settings)
{
    bool ret = true;
    wxCharBuffer abuf = initial_settings.ToUTF8();
    if( !abuf.data() )
        return false;
    
    //  Create the method parameter(s)
        QAndroidJniObject param1 = QAndroidJniObject::fromString(abuf.data());
        
        //  Get a reference to the running native activity
        QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
        "activity", "()Landroid/app/Activity;");
        
        if ( !activity.isValid() ){
            //qDebug() << "Activity is not valid";
            return false;
        }
        
        //  Call the desired method
        activity.callObjectMethod("doAndroidSettings", "(Ljava/lang/String;)Ljava/lang/String;", param1.object<jstring>());
        
        
        return ret;
}

wxString BuildAndroidSettingsString( void )
{
    wxString result;

    //  Start with chart dirs
    if( ChartData){
        wxArrayString chart_dir_array = ChartData->GetChartDirArrayString();

        for(unsigned int i=0 ; i < chart_dir_array.GetCount() ; i++){
            result += _T("ChartDir:");
            result += chart_dir_array[i];
            result += _T(";");
        }
    }

    //  Now the simple Boolean parameters
        result += _T("prefb_lookahead:") + wxString(g_bLookAhead == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_quilt:") + wxString(g_bQuiltEnable == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_showgrid:") + wxString(g_bDisplayGrid == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_showoutlines:") + wxString(g_bShowOutlines == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_showdepthunits:") + wxString(g_bShowDepthUnits == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_lockwp:") + wxString(g_bWayPointPreventDragging == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_confirmdelete:") + wxString(g_bConfirmObjectDelete == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_expertmode:") + wxString(g_bUIexpert == 1 ? _T("1;") : _T("0;"));

        if(ps52plib){
            result += _T("prefb_showlightldesc:") + wxString(ps52plib->m_bShowLdisText == 1 ? _T("1;") : _T("0;"));
            result += _T("prefb_showimptext:") + wxString(ps52plib->m_bShowS57ImportantTextOnly == 1 ? _T("1;") : _T("0;"));
            result += _T("prefb_showSCAMIN:") + wxString(ps52plib->m_bUseSCAMIN == 1 ? _T("1;") : _T("0;"));
            result += _T("prefb_showsound:") + wxString(ps52plib->m_bShowSoundg == 1 ? _T("1;") : _T("0;"));
            result += _T("prefb_showATONLabels:") + wxString(ps52plib->m_bShowAtonText == 1 ? _T("1;") : _T("0;"));
        }
    // Some other assorted values
        result += _T("prefs_navmode:") + wxString(g_bCourseUp == 0 ? _T("North Up;") : _T("Course Up;"));
        result += _T("prefs_chartInitDir:") + *pInit_Chart_Dir + _T(";");

        wxString s;
        double sf = (g_GUIScaleFactor * 10) + 50.;
        s.Printf( _T("%3.0f;"), sf );
        s.Trim(false);
        result += _T("prefs_UIScaleFactor:") + s;
        
        sf = (g_ChartScaleFactor * 10) + 50.;
        s.Printf( _T("%3.0f;"), sf );
        s.Trim(false);
        result += _T("prefs_chartScaleFactor:") + s;
        
        if(ps52plib){
            wxString nset = _T("Base");
            switch( ps52plib->GetDisplayCategory() ){
                case ( DISPLAYBASE ):
                    nset = _T("Base;");
                    break;
                case ( STANDARD ):
                    nset = _T("Standard;");
                    break;
                case ( OTHER ):
                    nset = _T("All;");
                    break;
                case ( MARINERS_STANDARD ):
                    nset = _T("Mariner Standard;");
                    break;
                default:
                    nset = _T("Base;");
                    break;
            }
            result += _T("prefs_displaycategory:") + nset;
            
    
            if( ps52plib->m_nSymbolStyle == PAPER_CHART )
                nset = _T("Paper Chart;");
            else
                nset = _T("Simplified;");
            result += _T("prefs_vectorgraphicsstyle:") + nset;
            
            if( ps52plib->m_nBoundaryStyle == PLAIN_BOUNDARIES )
                nset = _T("Plain;");
            else
                nset = _T("Symbolized;");
            result += _T("prefs_vectorboundarystyle:") + nset;
            
            if( S52_getMarinerParam( S52_MAR_TWO_SHADES ) == 1.0 )
                nset = _T("2;");
            else
                nset = _T("4;");
            result += _T("prefs_vectorchartcolors:") + nset;
            
            // depth unit conversion factor
          
            float conv = 1;
            int depthUnit = ps52plib->m_nDepthUnitDisplay;
            if ( depthUnit == 0 ) // feet
                conv = 0.3048f; // international definiton of 1 foot is 0.3048 metres
                else if ( depthUnit == 2 ) // fathoms
                conv = 0.3048f * 6; // 1 fathom is 6 feet
                
            s.Printf( _T("%4.2f;"), S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR ) / conv );
            s.Trim(false);
            result += _T("prefs_shallowdepth:") + s;
            
            s.Printf( _T("%4.2f;"), S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR ) / conv );
            s.Trim(false);
            result += _T("prefs_safetydepth:") + s;
            
            s.Printf( _T("%4.2f;"), S52_getMarinerParam( S52_MAR_DEEP_CONTOUR ) / conv );
            s.Trim(false);
            result += _T("prefs_deepdepth:") + s;
    
            //  Scale slider range from -5 -- 5 in OCPN options.
            //  On Android, the range is 0 -- 100
            //  So, convert
        }
        
        // Connections
        
        // Internal GPS.
        for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
        {
            ConnectionParams *cp = g_pConnectionParams->Item(i);
            if(INTERNAL_GPS == cp->Type){
                result += _T("prefb_internalGPS:");
                result += cp->bEnabled ? _T("1;") : _T("0;");
            }                    
            if(SERIAL == cp->Type){
                if(wxNOT_FOUND != cp->GetPortStr().Find(_T("PL2303"))){
                    result += _T("prefb_PL2303:");
                    result += cp->bEnabled ? _T("1;") : _T("0;");
                }
                else if(wxNOT_FOUND != cp->GetPortStr().Find(_T("dAISy"))){
                    result += _T("prefb_dAISy:");
                    result += cp->bEnabled ? _T("1;") : _T("0;");
                }
                else if(wxNOT_FOUND != cp->GetPortStr().Find(_T("FT232R"))){
                    result += _T("prefb_FT232R:");
                    result += cp->bEnabled ? _T("1;") : _T("0;");
                }
                else if(wxNOT_FOUND != cp->GetPortStr().Find(_T("FT231X"))){
                    result += _T("prefb_FT231X:");
                    result += cp->bEnabled ? _T("1;") : _T("0;");
                }
                else if(wxNOT_FOUND != cp->GetPortStr().Find(_T("USBDP"))){
                    result += _T("prefb_USBDP:");
                    result += cp->bEnabled ? _T("1;") : _T("0;");
                }
            }                    
        }
    
    wxLogMessage(_T("BuildAndroidSettingsString: ") + result);
    
    return result;
}

const wxString AUSBNames[] = { _T("AUSBSerial:Prolific_PL2303"), _T("AUSBSerial:FTDI_FT232R"), _T("AUSBSerial:FTDI_FT231X"), _T("AUSBSerial:dAISy"),
                _T("AUSBSerial:USBDP"), _T("LASTENTRY") };
const wxString AUSBPrefs[] = { _T("prefb_PL2303"),               _T("prefb_FT232R"),           _T("prefb_FT231X"),           _T("prefb_dAISy"),
                _T("prefb_USBDP"),         _T("LASTENTRY") };


int androidApplySettingsString( wxString settings, ArrayOfCDI *pACDI)
{
    
    //  Parse the passed settings string
    bool bproc_InternalGPS = false;
    bool benable_InternalGPS = false;
    
    int rr = GENERIC_CHANGED;
    
    // extract chart directories
        
    if(ChartData){
        wxStringTokenizer tkd(settings, _T(";"));
        while ( tkd.HasMoreTokens() ){
            wxString token = tkd.GetNextToken();
            
            if(token.StartsWith( _T("ChartDir"))){
                wxString dir = token.AfterFirst(':');
                if(dir.Length()){
                    ChartDirInfo cdi;
                    cdi.fullpath = dir.Trim();
                    cdi.magic_number = ChartData->GetMagicNumberCached(dir.Trim());
                    pACDI->Add(cdi);
                }
            }
        }
        
        // Scan for changes
        if(!ChartData->CompareChartDirArray( *pACDI )){
            rr |= VISIT_CHARTS;
            rr |= CHANGE_CHARTS;
            wxLogMessage(_T("Chart Dir List change detected"));
        }
    }
    
    
    wxStringTokenizer tk(settings, _T(";"));
    while ( tk.HasMoreTokens() )
    {
        wxString token = tk.GetNextToken();
        wxString val = token.AfterFirst(':');
        
        //  Binary switches
        
        if(token.StartsWith( _T("prefb_lookahead"))){
            g_bLookAhead = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_quilt"))){
            g_bQuiltEnable = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_lockwp"))){
            g_bWayPointPreventDragging = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showdepthunits"))){
            g_bShowDepthUnits = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_confirmdelete"))){
            g_bConfirmObjectDelete = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showgrid"))){
            g_bDisplayGrid = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showoutlines"))){
            g_bShowOutlines = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_expertmode"))){
            g_bUIexpert = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_internalGPS"))){
            bproc_InternalGPS = true;
            benable_InternalGPS = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefs_navmode"))){
            g_bCourseUp = val.IsSameAs(_T("Course Up"));
        }
        else if(token.StartsWith( _T("prefb_trackOnPause"))){
            g_btrackContinuous = val.IsSameAs(_T("1"));
        }
        
        
        //  Strings, etc.
        
        else if(token.StartsWith( _T("prefs_UIScaleFactor"))){
            double a;
            if(val.ToDouble(&a))
                g_GUIScaleFactor = wxRound( (a / 10.) - 5.);
        }
        
        else if(token.StartsWith( _T("prefs_chartScaleFactor"))){
            double a;
            if(val.ToDouble(&a)){
                g_ChartScaleFactor = wxRound( (a / 10.) - 5.);
                g_ChartScaleFactorExp = g_Platform->getChartScaleFactorExp( g_ChartScaleFactor );
            }
        }
        
        else if(token.StartsWith( _T("prefs_chartInitDir"))){
            *pInit_Chart_Dir = val;
        }
        
        if(ps52plib){
            float conv = 1;
            int depthUnit = ps52plib->m_nDepthUnitDisplay;
            if ( depthUnit == 0 ) // feet
                conv = 0.3048f; // international definiton of 1 foot is 0.3048 metres
                else if ( depthUnit == 2 ) // fathoms
                conv = 0.3048f * 6; // 1 fathom is 6 feet
                
                if(token.StartsWith( _T("prefb_showsound"))){
                    bool old_val = ps52plib->m_bShowSoundg;
                    ps52plib->m_bShowSoundg = val.IsSameAs(_T("1"));
                    if(old_val != ps52plib->m_bShowSoundg)
                        rr |= S52_CHANGED;
                }
                else if(token.StartsWith( _T("prefb_showSCAMIN"))){
                    bool old_val = ps52plib->m_bUseSCAMIN;
                    ps52plib->m_bUseSCAMIN = val.IsSameAs(_T("1"));
                    if(old_val != ps52plib->m_bUseSCAMIN)
                        rr |= S52_CHANGED;
                }
                else if(token.StartsWith( _T("prefb_showimptext"))){
                    bool old_val = ps52plib->m_bShowS57ImportantTextOnly;
                    ps52plib->m_bShowS57ImportantTextOnly = val.IsSameAs(_T("1"));
                    if(old_val != ps52plib->m_bShowS57ImportantTextOnly)
                        rr |= S52_CHANGED;
                }
                else if(token.StartsWith( _T("prefb_showlightldesc"))){
                    bool old_val = ps52plib->m_bShowLdisText;
                    ps52plib->m_bShowLdisText = val.IsSameAs(_T("1"));
                    if(old_val != ps52plib->m_bShowLdisText)
                        rr |= S52_CHANGED;
                }
                else if(token.StartsWith( _T("prefb_showATONLabels"))){
                    bool old_val = ps52plib->m_bShowAtonText;
                    ps52plib->m_bShowAtonText = val.IsSameAs(_T("1"));
                    if(old_val != ps52plib->m_bShowAtonText)
                        rr |= S52_CHANGED;
                }
                
                else if(token.StartsWith( _T("prefs_displaycategory"))){
                    _DisCat old_nset = ps52plib->GetDisplayCategory();
                    
                    _DisCat nset = DISPLAYBASE;
                    if(wxNOT_FOUND != val.Lower().Find(_T("base")))
                        nset = DISPLAYBASE;
                    else if(wxNOT_FOUND != val.Lower().Find(_T("mariner")))
                        nset = MARINERS_STANDARD;
                    else if(wxNOT_FOUND != val.Lower().Find(_T("standard")))
                        nset = STANDARD;
                    else if(wxNOT_FOUND != val.Lower().Find(_T("all")))
                        nset = OTHER;
                    
                    if(nset != old_nset){
                        rr |= S52_CHANGED;
                        ps52plib-> SetDisplayCategory( nset );
                    }
                }
                
                else if(token.StartsWith( _T("prefs_shallowdepth"))){
                    double old_dval = S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR );
                    double dval;
                    if(val.ToDouble(&dval)){
                        if(fabs(dval - old_dval) > .001){
                            S52_setMarinerParam( S52_MAR_SHALLOW_CONTOUR, dval * conv );
                            rr |= S52_CHANGED;
                        }
                    }
                }
                
                else if(token.StartsWith( _T("prefs_safetydepth"))){
                    double old_dval = S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR );
                    double dval;
                    if(val.ToDouble(&dval)){
                        if(fabs(dval - old_dval) > .001){
                            S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval * conv );
                            rr |= S52_CHANGED;
                        }
                    }
                }
                
                else if(token.StartsWith( _T("prefs_deepdepth"))){
                    double old_dval = S52_getMarinerParam( S52_MAR_DEEP_CONTOUR );
                    double dval;
                    if(val.ToDouble(&dval)){
                        if(fabs(dval - old_dval) > .001){
                            S52_setMarinerParam( S52_MAR_DEEP_CONTOUR, dval * conv );
                            rr |= S52_CHANGED;
                        }
                    }
                }
                
                else if(token.StartsWith( _T("prefs_vectorgraphicsstyle"))){
                    LUPname old_LUP = ps52plib->m_nSymbolStyle;
                    
                    if(wxNOT_FOUND != val.Lower().Find(_T("paper")))
                        ps52plib->m_nSymbolStyle = PAPER_CHART;
                    else if(wxNOT_FOUND != val.Lower().Find(_T("simplified")))
                        ps52plib->m_nSymbolStyle = SIMPLIFIED;
                    
                    if(old_LUP != ps52plib->m_nSymbolStyle)
                        rr |= S52_CHANGED;
                    
                }
                
                else if(token.StartsWith( _T("prefs_vectorboundarystyle"))){
                    LUPname old_LUP = ps52plib->m_nBoundaryStyle;
                    
                    if(wxNOT_FOUND != val.Lower().Find(_T("plain")))
                        ps52plib->m_nBoundaryStyle = PLAIN_BOUNDARIES;
                    else if(wxNOT_FOUND != val.Lower().Find(_T("symbolized")))
                        ps52plib->m_nBoundaryStyle = SYMBOLIZED_BOUNDARIES;
                    
                    if(old_LUP != ps52plib->m_nBoundaryStyle)
                        rr |= S52_CHANGED;
                    
                }
                
                else if(token.StartsWith( _T("prefs_vectorchartcolors"))){
                    double old_dval = S52_getMarinerParam( S52_MAR_TWO_SHADES );
                    
                    if(wxNOT_FOUND != val.Lower().Find(_T("2")))
                        S52_setMarinerParam( S52_MAR_TWO_SHADES, 1. );
                    else if(wxNOT_FOUND != val.Lower().Find(_T("4")))
                        S52_setMarinerParam( S52_MAR_TWO_SHADES, 0. );
                    
                    double new_dval = S52_getMarinerParam( S52_MAR_TWO_SHADES );
                    if(fabs(new_dval - old_dval) > .1){
                        rr |= S52_CHANGED;
                    }
                }
        }
    }
    
    // Process Internal GPS Connection
    if(g_pConnectionParams && bproc_InternalGPS){
        
        //  Does the connection already exist?
        ConnectionParams *pExistingParams = NULL;
        ConnectionParams *cp = NULL;
        
        for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
        {
            ConnectionParams *xcp = g_pConnectionParams->Item(i);
            if(INTERNAL_GPS == xcp->Type){
                pExistingParams = xcp;
                cp = xcp;
                break;
            }
        }
        
        bool b_action = true;
        if(pExistingParams){
            if(pExistingParams->bEnabled == benable_InternalGPS)
                b_action = false;                    // nothing to do...
                else
                    cp->bEnabled = benable_InternalGPS;
        }
        else if(benable_InternalGPS){           //  Need a new Params
            // make a generic config string for InternalGPS.
            wxString sGPS = _T("2;3;;0;0;;0;1;0;0;;0;;1;0;0;0;0");          // 17 parms
            ConnectionParams *new_params = new ConnectionParams(sGPS);
            
            new_params->bEnabled = benable_InternalGPS;
            g_pConnectionParams->Add(new_params);
            cp = new_params;
        }
        
        
        if(b_action && cp){                               // something to do?

            // Terminate and remove any existing stream with the same port name
            DataStream *pds_existing = g_pMUX->FindStream( cp->GetDSPort() );
            if(pds_existing)
                g_pMUX->StopAndRemoveStream( pds_existing );
            
            
            if( cp->bEnabled ) {
                dsPortType port_type = cp->IOSelect;
                DataStream *dstr = makeSerialDataStream(g_pMUX,
                                                   cp->Type,
                                                   cp->GetDSPort(),
                                                   wxString::Format(wxT("%i"), cp->Baudrate),
                                                   port_type,
                                                   cp->Priority,
                                                   cp->Garmin);

#if 0                
                DataStream *dstr = new DataStream( g_pMUX,
                                                   cp->Type,
                                                   cp->GetDSPort(),
                                                   wxString::Format(wxT("%i"), cp->Baudrate),
                                                                    port_type,
                                                                    cp->Priority,
                                                                    cp->Garmin);
#endif                
                                                   dstr->SetInputFilter(cp->InputSentenceList);
                                                   dstr->SetInputFilterType(cp->InputSentenceListType);
                                                   dstr->SetOutputFilter(cp->OutputSentenceList);
                                                   dstr->SetOutputFilterType(cp->OutputSentenceListType);
                                                   dstr->SetChecksumCheck(cp->ChecksumCheck);
                                                   
                                                   g_pMUX->AddStream(dstr);
                                                   
                                                   cp->b_IsSetup = true;
            }
        }
    }
    
    // Process USB Serial Connections
    bool b_newGlobalSettings = false;
    if(g_pConnectionParams){
        
        int i = 0;
        while( wxNOT_FOUND == AUSBPrefs[i].Find(_T("LASTENTRY")) ){
            wxStringTokenizer tk(settings, _T(";"));
            while ( tk.HasMoreTokens() )
            {
                wxString token = tk.GetNextToken();
                wxString pref = token.BeforeFirst(':');
                wxString val = token.AfterFirst(':');
                wxString extraString;
                
                
                bool benabled = false;
                
                if(pref.IsSameAs(AUSBPrefs[i])){

                    wxLogMessage(_T("pref: ") + pref);
                    wxLogMessage(_T("val: ") + val);
                    
                    if(pref.Contains(_T("USBDP"))){
                        extraString = val.AfterFirst(':');
                        wxLogMessage(_T("extra: ") + extraString);
                    }
                            
                    wxLogMessage(_T("found pref ") + pref);
                    
                    //  Does the connection already exist?
                    ConnectionParams *pExistingParams = NULL;
                    ConnectionParams *cp = NULL;
                    
                    wxString target = AUSBNames[i] + _T("-") + extraString;
                    
                    for ( unsigned int j = 0; j < g_pConnectionParams->Count(); j++ )
                    {
                        ConnectionParams *xcp = g_pConnectionParams->Item(j);
                        wxLogMessage( _T("    Checking: ") + target + " .. " +xcp->GetDSPort());
                        
                        if( (SERIAL == xcp->Type) && (target.IsSameAs(xcp->GetDSPort().AfterFirst(':'))) ){
                            pExistingParams = xcp;
                            cp = xcp;
                            benabled = val.BeforeFirst(':').IsSameAs(_T("1"));
                            break;
                        }
                    }
                    
                    
                    bool b_action = true;
                    if(pExistingParams){
                        wxLogMessage(_T("Using existing connection  ") + target);
                        
                        if(pExistingParams->bEnabled == benabled){
                            b_action = false;                    // nothing to do...
                        }
                        else
                            cp->bEnabled = benabled;
                    }
                    else if(val.BeforeFirst(':').IsSameAs(_T("1"))){           //  Need a new Params
                        // make a generic config string.
                        //0;1;;0;0;/dev/ttyS0;4800;1;0;0;;0;;1;0;0;0;0        17 parms
                        
                        wxString sSerial = _T("0;1;;0;0;");
                        sSerial += AUSBNames[i];
                        sSerial += _T("-") + extraString;
                        sSerial += _T(";4800;1;0;0;;0;;1;0;0;0;0");
                        
                        wxLogMessage(_T("Adding connection  ") + sSerial);
                        
                        ConnectionParams *new_params = new ConnectionParams(sSerial);
                        
                        new_params->bEnabled = true;
                        g_pConnectionParams->Add(new_params);
                        cp = new_params;
                        rr |= NEED_NEW_OPTIONS;
                    }
                    
                    
                    
                    
                    if(b_action && cp){                               // something to do?
                        rr |= NEED_NEW_OPTIONS;
                        
                        // Terminate and remove any existing stream with the same port name
                        DataStream *pds_existing = g_pMUX->FindStream( cp->GetDSPort() );
                        if(pds_existing)
                            g_pMUX->StopAndRemoveStream( pds_existing );
                        
                        
                        if( cp->bEnabled ) {
                            dsPortType port_type = cp->IOSelect;
#if 0
                            DataStream *dstr = new DataStream( g_pMUX,
                                                               cp->Type,
                                                               cp->GetDSPort(),
                                                               wxString::Format(wxT("%i"), cp->Baudrate),
                                                               port_type,
                                                               cp->Priority,
                                                               cp->Garmin);
#endif                            
                            DataStream *dstr = makeSerialDataStream(g_pMUX,
                                                   cp->Type,
                                                   cp->GetDSPort(),
                                                   wxString::Format(wxT("%i"), cp->Baudrate),
                                                   port_type,
                                                   cp->Priority,
                                                   cp->Garmin);

                            dstr->SetInputFilter(cp->InputSentenceList);
                            dstr->SetInputFilterType(cp->InputSentenceListType);
                            dstr->SetOutputFilter(cp->OutputSentenceList);
                            dstr->SetOutputFilterType(cp->OutputSentenceListType);
                            dstr->SetChecksumCheck(cp->ChecksumCheck);
                            
                            g_pMUX->AddStream(dstr);
                            
                            cp->b_IsSetup = true;
                        }
                    }
                }
            }   // found pref
            
            i++;
        }       // while
    }
    
    return rr;
}
    
    

bool DoAndroidPreferences( void )
{
    wxLogMessage(_T("Start DoAndroidPreferences"));
    
    wxString settings = BuildAndroidSettingsString();

    wxLogMessage(_T("Call InvokeJNIPreferences"));
    InvokeJNIPreferences(settings);

    return true;
}

wxString doAndroidPOST( const wxString &url, wxString &parms, int timeoutMsec)
{
    //  Start a timer to poll for results 
    if(g_androidUtilHandler){
        g_androidUtilHandler->m_eventTimer.Stop();
        g_androidUtilHandler->m_done = false;

        androidShowBusyIcon();
        
        wxString stimeout;
        stimeout.Printf(_T("%d"), timeoutMsec);
        wxString result = callActivityMethod_s3s( "doHttpPostAsync", url, parms, stimeout );
        
        if(result == _T("OK") ){
            qDebug() << "doHttpPostAsync ResultOK, starting spin loop";
            g_androidUtilHandler->m_action = ACTION_POSTASYNC_END;
            g_androidUtilHandler->m_eventTimer.Start(500, wxTIMER_CONTINUOUS);
            
            //  Spin, waiting for result
            while(!g_androidUtilHandler->m_done){
                wxMilliSleep(50);
                wxSafeYield(NULL, true);
            }
            
            qDebug() << "out of spin loop";
            g_androidUtilHandler->m_action = ACTION_NONE;
            g_androidUtilHandler->m_eventTimer.Stop();
            androidHideBusyIcon();
            
            wxString presult = g_androidUtilHandler->GetStringResult();
            
            return presult;
        }
        else{
            qDebug() << "doHttpPostAsync Result NOT OK";
            androidHideBusyIcon();
        }
    }
        
    return wxEmptyString;    
}
    
    
int startAndroidFileDownload( const wxString &url, const wxString& destination, wxEvtHandler *evh, long *dl_id )
{
//    if(evh)
    {
        s_bdownloading = true;
        s_requested_url = url;
        s_download_evHandler = evh;
    
        wxString result = callActivityMethod_s2s( "downloadFile", url, destination );

        androidShowBusyIcon();
        
        if( result.IsSameAs(_T("NOK")) )
            return 1;                       // general error
            
  //      wxLogMessage(_T("downloads2s result: ") + result);
        long dl_ID;
        wxStringTokenizer tk(result, _T(";"));
        if( tk.HasMoreTokens() ){
            wxString token = tk.GetNextToken();
            if(token.IsSameAs(_T("OK"))){
                token = tk.GetNextToken();
                token.ToLong(&dl_ID);
                *dl_id = dl_ID;
  //              qDebug() << dl_ID;
                return 0;
            }
        }
    }
    
    return -1;
}

int queryAndroidFileDownload( long dl_ID, wxString *result )
{
//    qDebug() << dl_ID;
    
    wxString stat = callActivityMethod_is( "getDownloadStatus", (int)dl_ID );
    if(result)
        *result = stat;
    
//    wxLogMessage( _T("queryAndroidFileDownload: ") + stat); 
    
    if( stat.IsSameAs(_T("NOK")) )
        return 1;                       // general error
    else
        return 0;
    
}

void finishAndroidFileDownload( void )
{
    s_bdownloading = false;
    s_requested_url.Clear();
    s_download_evHandler = NULL;
    androidHideBusyIcon();
    
    return;
}


void cancelAndroidFileDownload( long dl_ID )
{
    wxString stat = callActivityMethod_is( "cancelDownload", (int)dl_ID );
}


bool AndroidUnzip(wxString& zipFile, wxString& destDir, int nStrip, bool bRemoveZip)
{
    wxString ns;
    ns.Printf(_T("%d"), nStrip);

    wxString br;
    br.Printf(_T("%d"), bRemoveZip);
    
    wxString stat = callActivityMethod_s4s( "unzipFile", zipFile, destDir, ns, br  );
    
    if(wxNOT_FOUND == stat.Find(_T("OK")))
        return false;
    
    qDebug() << "unzip start";
    
    bool bDone = false;
    while (!bDone){
        wxMilliSleep(1000);
        wxSafeYield(NULL, true);
        
        qDebug() << "unzip poll";
        
        wxString result = callActivityMethod_ss( "getUnzipStatus", _T("") );
        if(wxNOT_FOUND != result.Find(_T("DONE")))
            bDone = true;
    }
    qDebug() << "unzip done";
    
    return true;    
    
}

    
wxString getFontQtStylesheet(wxFont *font)
{
    // wxString classes = _T("QLabel, QPushButton, QTreeWidget, QTreeWidgetItem, QCheckBox");
    wxString classes = _T("QWidget ");
    
    wxString qstyle = classes + _T("{  font-family: ") + font->GetFaceName() + _T(";font-style: ");
    switch(font->GetStyle()){
        case wxFONTSTYLE_ITALIC:
            qstyle += _T("italic;");
            break;
        case wxFONTSTYLE_NORMAL:
        default:
            qstyle += _T("normal;");
            break;
    }
    qstyle += _T("font-weight: ");
    switch(font->GetWeight()){
        case wxFONTWEIGHT_BOLD:
            qstyle += _T("bold;");
            break;
        case wxFONTWEIGHT_LIGHT:
            qstyle += _T("light;");
            break;
        case wxFONTWEIGHT_NORMAL:
        default:
            qstyle += _T("normal;");
            break;
    }
    
    qstyle += _T("font-size: ");
    wxString fontSize;
    fontSize.Printf(_T("%dpt }"), font->GetPointSize());
    qstyle += fontSize;

    //  Oddity here....
    //  If this line is active, this particular style is applied to ListCtrl() in PlugIns,
    //  But not TreeCtrl.....
    //  ????
    //qstyle += _T("QTreeWidget::item{ border-color:red; border-style:outset; border-width:2px; color:black; }");
    
    return qstyle;
    
}

    

bool androidPlaySound( wxString soundfile, AudioDoneCallback callBack )
{
    //qDebug() << "androidPlay";
    s_soundCallBack = callBack;    
    wxString result = callActivityMethod_ss("playSound", soundfile);
    
    return true;
}
    
extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onSoundFinished(JNIEnv *env, jobject obj)
    {
        qDebug() << "onSoundFinished";
        
        if(s_soundCallBack){
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
            evt.SetId( ID_CMD_SOUND_FINISHED );
            if(g_androidUtilHandler)
                g_androidUtilHandler->AddPendingEvent(evt);
        }

            
        return 98;
    }
}

wxString androidGetSupplementalLicense( void )
{
    wxString result = callActivityMethod_vs("getGMAPILicense");
    
    result += AndroidSuppLicense;
    
    return result;
}




wxArrayString androidTraverseDir( wxString dir, wxString filespec)
{
    wxArrayString result;
    if(strncmp(android_plat_spc.msdk, "17", 2))   // skip unless running Android 4.2.2, especially Samsung...
        return result;
    
    wxString ir = callActivityMethod_s2s("getAllFilesWithFilespec", dir, filespec);
    
    wxStringTokenizer tk(ir, _T(";"));
    while( tk.HasMoreTokens() ){
        result.Add(tk.GetNextToken());
    }
    
    return result;
}
        
void androidEnableOptionsMenu( bool bEnable )
{
    callActivityMethod_is("enableOptionsMenu", bEnable?1:0);
}

        
//    Android specific style sheet management

//  ------------Runtime modified globals
QString qtStyleSheetDialog;
QString qtStyleSheetListBook;
QString qtStyleSheetScrollbars;

//--------------Stylesheet prototypes

//  Generic dialog stylesheet
//  Typically adjusted at runtime for display density

QString qtStyleSheetDialogProto = "\
QSlider::groove\
{\
    border: 1px solid #999999;\
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E6E6E6, stop:1 #EEEEEE);\
}\
QSlider::groove:disabled\
{\
    background: #efefef;\
}\
\
QSlider::handle\
{\
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #7cb0e9, stop:1 #7cb0e9);\
    border: 1px solid #5c5c5c;\
    border-radius: 3px;\
    width: 80px;\
    height: 45px;\
}\
\
QSlider::handle:disabled\
{\
    background: #D3D0CD;\
}\
\
QScrollBar:horizontal {\
    border: 0px solid grey;\
    background-color: transparent;\
    height: 4px;\
    margin: 0px 1px 0 1px;\
}\
QScrollBar::handle:horizontal {\
    background-color: rgb(150, 150, 150);\
    min-width: 20px;\
}\
QScrollBar::add-line:horizontal {\
    border: 0px solid grey;\
    background: transparent;\
    width: 1px;\
    subcontrol-position: right;\
    subcontrol-origin: margin;\
}\
\
QScrollBar::sub-line:horizontal {\
    border: 0px solid grey;\
    background: transparent;\
    width: 1px;\
    subcontrol-position: left;\
    subcontrol-origin: margin;\
}\
\
QScrollBar:vertical {\
    border: 0px solid grey;\
    background-color: transparent;\
    width: 4px;\
    margin: 1px 0px 1px 0px;\
}\
QScrollBar::handle:vertical {\
    background-color: rgb(150, 150, 150);\
    min-height: 20px;\
}\
QScrollBar::add-line:vertical {\
    border: 0px solid grey;\
    background: transparent;\
    height: 1px;\
    subcontrol-position: top;\
    subcontrol-origin: margin;\
}\
\
QScrollBar::sub-line:vertical {\
    border: 0px solid grey;\
    background: transparent;\
    height: 1px;\
    subcontrol-position: bottom;\
    subcontrol-origin: margin;\
}\
\
QTreeWidget QScrollBar:vertical {\
    border: 0px solid grey;\
    background-color: rgb(240, 240, 240);\
    width: 35px;\
    margin: 1px 0px 1px 0px;\
}\
QTreeWidget QScrollBar::handle:vertical {\
    background-color: rgb(200, 200, 200);\
    min-height: 20px;\
    border-radius: 10px;\
}\
QTreeWidget QScrollBar::add-line:vertical {\
    border: 0px solid grey;\
    background: #32CC99;\
    height: 0px;\
    subcontrol-position: top;\
    subcontrol-origin: margin;\
}\
\
QTreeWidget QScrollBar::sub-line:vertical {\
    border: 0px solid grey;\
    background: #32CC99;\
    height: 0px;\
    subcontrol-position: bottom;\
    subcontrol-origin: margin;\
}\
\
QTreeWidget QScrollBar:horizontal {\
    border: 0px solid grey;\
    background-color: rgb(240, 240, 240);\
    height: 35px;\
    margin: 0px 1px 0 1px;\
}\
QTreeWidget QScrollBar::handle:horizontal {\
    background-color: rgb(200, 200, 200);\
    min-width: 20px;\
    border-radius: 10px;\
}\
QTreeWidget QScrollBar::add-line:horizontal {\
    border: 0px solid grey;\
    background: #32CC99;\
    width: 0px;\
    subcontrol-position: right;\
    subcontrol-origin: margin;\
}\
\
QTreeWidget QScrollBar::sub-line:horizontal {\
    border: 0px solid grey;\
    background: #32CC99;\
    width: 0px;\
    subcontrol-position: left;\
    subcontrol-origin: margin;\
}\
\
#OCPNCheckedListCtrl QScrollBar::vertical {\
    border: 0px solid grey;\
    background-color: rgb(240, 240, 240);\
    width: 45px;\
    margin: 1px 0px 1px 0px;\
}\
#OCPNCheckedListCtrl QScrollBar::handle:vertical {\
    background-color: rgb(180, 180, 180);\
    min-height: 45px;\
    border-radius: 6px;\
}\
#OCPNCheckedListCtrl QScrollBar::add-line:vertical {\
    border: 0px solid grey;\
    background: #32CC99;\
    height: 0px;\
    subcontrol-position: top;\
    subcontrol-origin: margin;\
}\
\
#OCPNCheckedListCtrl QScrollBar::sub-line:vertical {\
    border: 0px solid grey;\
    background: #32CC99;\
    height: 0px;\
    subcontrol-position: bottom;\
    subcontrol-origin: margin;\
}";



QString qtStyleSheetScrollbarsProto ="\
QScrollBar:horizontal {\
    border: 0px solid grey;\
    background-color: transparent;\
    height: 35px;\
    margin: 0px 1px 0 1px;\
}\
QScrollBar::handle:horizontal {\
    background-color: #7cb0e9;\
    min-width: 20px;\
}\
QScrollBar::add-line:horizontal {\
    border: 0px solid grey;\
    background: transparent;\
    width: 1px;\
    subcontrol-position: right;\
    subcontrol-origin: margin;\
}\
\
QScrollBar::sub-line:horizontal {\
    border: 0px solid grey;\
    background: transparent;\
    width: 1px;\
    subcontrol-position: left;\
    subcontrol-origin: margin;\
}\
\
QScrollBar:vertical {\
    border: 0px solid grey;\
    background-color: transparent;\
    width: 35px;\
    margin: 1px 0px 1px 0px;\
}\
QScrollBar::handle:vertical {\
    background-color: #7cb0e9;\
    min-height: 20px;\
}\
QScrollBar::add-line:vertical {\
    border: 0px solid grey;\
    background: transparent;\
    height: 1px;\
    subcontrol-position: top;\
    subcontrol-origin: margin;\
}\
\
QScrollBar::sub-line:vertical {\
    border: 0px solid grey;\
    background: transparent;\
    height: 1px;\
    subcontrol-position: bottom;\
    subcontrol-origin: margin;\
}";



std::string prepareStyleIcon( wxString icon_file, int size )
{
    wxString data_locn = g_Platform->GetSharedDataDir();
    data_locn.Append( _T("styles/") );

    wxString file = data_locn + icon_file;
    
    wxImage Image(file, wxBITMAP_TYPE_PNG);
    wxImage scaledImage = Image.Scale( size, size, wxIMAGE_QUALITY_HIGH );
    
    wxString save_file = g_Platform->GetPrivateDataDir() + _T("/") + icon_file;
    scaledImage.SaveFile(save_file, wxBITMAP_TYPE_PNG);

    wxCharBuffer buf = save_file.ToUTF8();
    std::string ret(buf);
    return ret;
}
    
QString prepareAndroidSliderStyleSheet(int sliderWidth)
{
    QString qtStyleSheetSlider;
    
    
    //  Create and fix up the qtStyleSheetDialog for generic dialog
    
    // adjust the Slider specification
    
    int slider_handle_width = wxMax(g_Platform->GetDisplayDPmm() * 6, sliderWidth / 5);
    
    char sb[600];
    snprintf(sb, sizeof(sb),
    "QSlider::groove { border: 1px solid #999999;  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E6E6E6, stop:1 #EEEEEE); } \
    QSlider::groove:disabled { background: #efefef; } \
    QSlider::handle { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #7cb0e9, stop:1 #7cb0e9); border: 1px solid #5c5c5c; \
    border-radius: 3px; width: %dpx; height: 45px; } \
    QSlider::handle:disabled { background: #D3D0CD;}", slider_handle_width);
    
    qtStyleSheetSlider.append(sb);
    
    return qtStyleSheetSlider;
}

    
void prepareAndroidStyleSheets()
{
    
    //  Create and fix up the qtStyleSheetDialog for generic dialog
    qtStyleSheetDialog.clear();
    qtStyleSheetDialog.append(qtStyleSheetDialogProto);
    
    // add the Slider specification
    
    int slider_handle_width = g_Platform->GetDisplayDPmm() * 6;
    
    char sb[400];
    snprintf(sb, sizeof(sb),
    "QSlider::groove { border: 1px solid #999999;  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E6E6E6, stop:1 #EEEEEE); } \
    QSlider::groove:disabled { background: #efefef; } \
    QSlider::handle { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #7cb0e9, stop:1 #7cb0e9); border: 1px solid #5c5c5c; \
    border-radius: 3px; width: %dpx; height: 45px; } \
    QSlider::handle:disabled { background: #D3D0CD;}", slider_handle_width);
    
    qtStyleSheetDialog.append(sb);
    
    
    
    // add the checkbox specification
    int cbSize = 30 * getAndroidDisplayDensity();
    char cb[400];
    
    // icons
     // Checked box
    std::string cbs = prepareStyleIcon(_T("chek_full.png"), cbSize);
    //  Empty box
    std::string ucbs = prepareStyleIcon(_T("chek_empty.png"), cbSize);
    
    snprintf(cb, sizeof(cb), "QCheckBox { spacing: 25px;}\
    QCheckBox::indicator { width: %dpx;   height: %dpx;}\
    QCheckBox::indicator:checked {image: url(%s);}\
    QCheckBox::indicator:unchecked {image: url(%s);}", cbSize, cbSize, cbs.c_str(), ucbs.c_str());
    
    qtStyleSheetDialog.append(cb);

    
    //   The qTabBar buttons as in a listbook
    qtStyleSheetListBook.clear();
    
    // compute the tabbar button size
    int tbbSize = 50 * getAndroidDisplayDensity();
    char tbb[400];
    

    std::string tbbl = prepareStyleIcon(_T("tabbar_button_left.png"), tbbSize);
    std::string tbbr = prepareStyleIcon(_T("tabbar_button_right.png"), tbbSize);
    
    snprintf(tbb, sizeof(tbb), "QTabBar::scroller { width: %dpx; }\
    QTabBar QToolButton::right-arrow { image: url(%s); }\
    QTabBar QToolButton::left-arrow { image: url(%s); }", tbbSize, tbbr.c_str(), tbbl.c_str());
    
    qtStyleSheetListBook.append(tbb);

    
    // A simple stylesheet with scrollbars only
    qtStyleSheetScrollbars.clear();
    qtStyleSheetScrollbars.append(qtStyleSheetScrollbarsProto);
}    
    
void setChoiceStyleSheet( wxChoice *win, int refDim)
{
    //qDebug() << "refDim" << refDim;
    
    float fontDimFloat = ((float)refDim) * 0.5;
    int fontDim = (int)fontDimFloat;
    int pixRadius = refDim / 4;
    
    QString styleString;
    char sb[1400];
    
    
     //  This one control the appearance of the "un-dropped" control.
    snprintf(sb, sizeof(sb),
             "QComboBox { font-size: %dpx; font-weight: bold; min-height: %dpx; color: rgb(0,0,0); background-color: rgb(250,250,250); }", fontDim, refDim );
    styleString.append(sb);
    
    // This one controls the color and style of the drop list items
    snprintf(sb, sizeof(sb),
             "QComboBox QListView::item { color: rgb(0,0,0); background-color: rgb(95, 163, 237); }");
    styleString.append(sb);
    
    
    // This one controls the drop list font
    snprintf(sb, sizeof(sb),
             "QComboBox QAbstractItemView { font-size: %dpx; font-weight: bold;}", fontDim);
    styleString.append(sb);
    
    // This one is necessary to set min height of drop list items, otherwise they are squished.
    snprintf(sb, sizeof(sb),
             "QComboBox QAbstractItemView::item {  min-height: %dpx; border: 10px outset darkgray; border-radius: %dpx;  }", refDim, pixRadius);
    styleString.append(sb); 
    
    //qDebug() << styleString;
    
    //win->GetHandle()->setView(new QListView());         // Magic
    win->GetHandle()->setStyleSheet(styleString);
 
    
}


void setMenuStyleSheet( wxMenu *win, const wxFont& font)
{
    if(!win)
        return;
    
    int points = font.GetPointSize();
    int fontPix = points / g_Platform->getFontPointsperPixel();
     
    //qDebug() << points << g_Platform->getFontPointsperPixel() << fontPix;
    
    QString styleString;
    char sb[1400];
    
    snprintf(sb, sizeof(sb),
             "QMenu { font: bold %dpx; }", fontPix );
    styleString.append(sb);

    snprintf(sb, sizeof(sb),
             "QMenu::separator { height: 4px; background: lightblue; margin-left: 10px; margin-right: 5px; }");
    styleString.append(sb);
    
    
    //qDebug() << styleString;
    
    win->GetHandle()->setStyleSheet(styleString);
}


QString getAdjustedDialogStyleSheet()
{
    return qtStyleSheetDialog;
}

QString getListBookStyleSheet()
{
    return qtStyleSheetListBook;
}

QString getScrollBarsStyleSheet()
{
    return qtStyleSheetScrollbars;
}


//      SVG Support
wxBitmap loadAndroidSVG( const wxString filename, unsigned int width, unsigned int height )
{
    wxCharBuffer abuf = filename.ToUTF8();
    if( abuf.data() ){                            // OK conversion?
        std::string s(abuf.data());              
        //qDebug() << "loadAndroidSVG" << s.c_str();
    }
    else{
        qDebug() << "loadAndroidSVG FAIL";
    }        
    
    // Destination file location
    wxString save_file_dir = g_Platform->GetPrivateDataDir() + _T("/") + _T("icons");
    if( !wxDirExists(save_file_dir) )
        wxMkdir( save_file_dir);
    
    wxFileName fsvg(filename);
    wxFileName fn(save_file_dir + _T("/") + fsvg.GetFullName());
    fn.SetExt(_T("png"));

/*
       //Caching does not work well, since we always build each icon twice.
    if(fn.FileExists()){
        wxBitmap bmp_test(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
        if(bmp_test.IsOk()){
            if((bmp_test.GetWidth() == (int)width) && (bmp_test.GetHeight() == (int)height))
                return bmp_test;
        }
    }
*/            
        
    wxString val = callActivityMethod_s2s2i("buildSVGIcon", filename, fn.GetFullPath(), width, height);
    if( val == _T("OK") ){
//        qDebug() << "OK";
        
        return wxBitmap(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
    }
    else{
        return wxBitmap(width, height);
    }
}

void androidTestCPP()
{
    callActivityMethod_vs("callFromCpp");
}

unsigned int androidColorPicker( unsigned int initialColor)
{
    if(g_androidUtilHandler){
        g_androidUtilHandler->m_eventTimer.Stop();
        g_androidUtilHandler->m_done = false;
        
        wxString val = callActivityMethod_is("doColorPickerDialog", initialColor);
    
   
        if(val == _T("OK") ){
            //qDebug() << "ResultOK, starting spin loop";
            g_androidUtilHandler->m_action = ACTION_COLORDIALOG_END;
            g_androidUtilHandler->m_eventTimer.Start(1000, wxTIMER_CONTINUOUS);
            
            //  Spin, waiting for result
            while(!g_androidUtilHandler->m_done){
                wxMilliSleep(50);
                wxSafeYield(NULL, true);
            }
            
            //qDebug() << "out of spin loop";
            g_androidUtilHandler->m_action = ACTION_NONE;
            g_androidUtilHandler->m_eventTimer.Stop();
            
            
            wxString tresult = g_androidUtilHandler->GetStringResult();
            
            if( tresult.StartsWith(_T("cancel:")) ){
                //qDebug() << "Cancel1";
                return initialColor;
            }
            else if( tresult.StartsWith(_T("color:")) ){
                wxString color = tresult.AfterFirst(':');
                long a;
                color.ToLong(&a);
                unsigned int b = a;
                
                //char cc[30];
                //sprintf(cc, "%0X", b);
                //qDebug() << "OK " << cc;
                
                return b;
            }
        }
        else{
            qDebug() << "Result NOT OK";
        }
    }
    return 0;
}

bool AndroidSecureCopyFile(wxString in, wxString out)
{
    bool bret = true;
    
    wxString result = callActivityMethod_s2s("SecureFileCopy", in, out);
 
    if(wxNOT_FOUND == result.Find(_T("OK")))
        bret = false;
    
    return bret;
}

int doAndroidPersistState()
{
    qDebug() << "doAndroidPersistState() starting...";
    wxLogMessage( _T("doAndroidPersistState() starting...") );

    // We save perspective before closing to restore position next time
    // Pane is not closed so the child is not notified (OnPaneClose)
    if(g_pauimgr){
        if( g_pAISTargetList ) {
            wxAuiPaneInfo &pane = g_pauimgr->GetPane( g_pAISTargetList );
            g_AisTargetList_perspective = g_pauimgr->SavePaneInfo( pane );
            g_pauimgr->DetachPane( g_pAISTargetList );

            pConfig->SetPath( _T ( "/AUI" ) );
            pConfig->Write( _T ( "AUIPerspective" ), g_pauimgr->SavePerspective() );
        }
    }


 
    //    Deactivate the PlugIns, allowing them to save state
    if( g_pi_manager ) {
        g_pi_manager->DeactivateAllPlugIns();
    }

    /*
     Automatically drop an anchorage waypoint, if enabled
     On following conditions:
     1.  In "Cruising" mode, meaning that speed has at some point exceeded 3.0 kts.
     2.  Current speed is less than 0.5 kts.
     3.  Opencpn has been up at least 30 minutes
     4.  And, of course, opencpn is going down now.
     5.  And if there is no anchor watch set on "anchor..." icon mark           // pjotrc 2010.02.15
     */
    if( g_bAutoAnchorMark ) {
        bool watching_anchor = false;                                           // pjotrc 2010.02.15
        if( pAnchorWatchPoint1 )                                               // pjotrc 2010.02.15
        watching_anchor = ( pAnchorWatchPoint1->GetIconName().StartsWith( _T("anchor") ) ); // pjotrc 2010.02.15
        if( pAnchorWatchPoint2 )                                               // pjotrc 2010.02.15
        watching_anchor |= ( pAnchorWatchPoint2->GetIconName().StartsWith( _T("anchor") ) ); // pjotrc 2010.02.15

        wxDateTime now = wxDateTime::Now();
        wxTimeSpan uptime = now.Subtract( g_start_time );

        if( !watching_anchor && ( g_bCruising ) && ( gSog < 0.5 )
                && ( uptime.IsLongerThan( wxTimeSpan( 0, 30, 0, 0 ) ) ) )     // pjotrc 2010.02.15
                {
            //    First, delete any single anchorage waypoint closer than 0.25 NM from this point
            //    This will prevent clutter and database congestion....

            wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
            while( node ) {
                RoutePoint *pr = node->GetData();
                if( pr->GetName().StartsWith( _T("Anchorage") ) ) {
                    double a = gLat - pr->m_lat;
                    double b = gLon - pr->m_lon;
                    double l = sqrt( ( a * a ) + ( b * b ) );

                    // caveat: this is accurate only on the Equator
                    if( ( l * 60. * 1852. ) < ( .25 * 1852. ) ) {
                        pConfig->DeleteWayPoint( pr );
                        pSelect->DeleteSelectablePoint( pr, SELTYPE_ROUTEPOINT );
                        delete pr;
                        break;
                    }
                }

                node = node->GetNext();
            }

            wxString name = now.Format();
            name.Prepend( _("Anchorage created ") );
            RoutePoint *pWP = new RoutePoint( gLat, gLon, _T("anchorage"), name, _T("") );
            pWP->m_bShowName = false;
            pWP->m_bIsolatedMark = true;

            pConfig->AddNewWayPoint( pWP, -1 );       // use auto next num
        }
    }

    if( gFrame->GetPrimaryCanvas()->GetpCurrentStack() ) {
        g_restore_stackindex = gFrame->GetPrimaryCanvas()->GetpCurrentStack()->CurrentStackEntry;
        g_restore_dbindex = gFrame->GetPrimaryCanvas()->GetpCurrentStack()->GetCurrentEntrydbIndex();
        if(gFrame->GetPrimaryCanvas() && gFrame->GetPrimaryCanvas()->GetQuiltMode())
            g_restore_dbindex = gFrame->GetPrimaryCanvas()->GetQuiltReferenceChartIndex();
    }

    if( g_MainToolbar ) {
        wxPoint tbp = g_MainToolbar->GetPosition();
        wxPoint tbp_incanvas = gFrame->GetPrimaryCanvas()->ScreenToClient( tbp );
        g_maintoolbar_x = tbp_incanvas.x;
        g_maintoolbar_y = tbp_incanvas.y;
        g_maintoolbar_orient = g_MainToolbar->GetOrient();
    }

    if(g_iENCToolbar){
        wxPoint locn = g_iENCToolbar->GetPosition();
        wxPoint tbp_incanvas = gFrame->GetPrimaryCanvas()->ScreenToClient( locn );
        g_iENCToolbarPosY = tbp_incanvas.y;
        g_iENCToolbarPosX = tbp_incanvas.x;
    }
    
    pConfig->UpdateSettings();
    pConfig->UpdateNavObj();

    delete pConfig->m_pNavObjectChangesSet;

    //Remove any leftover Routes and Waypoints from config file as they were saved to navobj before
    pConfig->DeleteGroup( _T ( "/Routes" ) );
    pConfig->DeleteGroup( _T ( "/Marks" ) );
    pConfig->Flush();

    delete pConfig;             // All done
    pConfig = NULL;


    //    Unload the PlugIns
    //      Note that we are waiting until after the canvas is destroyed,
    //      since some PlugIns may have created children of canvas.
    //      Such a PlugIn must stay intact for the canvas dtor to call DestoryChildren()

    if(ChartData)
        ChartData->PurgeCachePlugins();

    if( g_pi_manager ) {
        g_pi_manager->UnLoadAllPlugIns();
        delete g_pi_manager;
        g_pi_manager = NULL;
    }

    wxLogMessage( _T("doAndroidPersistState() finished cleanly.") );
    qDebug() << "doAndroidPersistState() finished cleanly.";

    wxLogMessage( _T("Closing logfile, Terminating App.") );
    
    wxLog::FlushActive();
    g_Platform->CloseLogFile();

    return 0;
}




extern "C"{
    JNIEXPORT int JNICALL Java_org_opencpn_OCPNNativeLib_ScheduleCleanExit(JNIEnv *env, jobject obj)
    {
        qDebug() << "Java_org_opencpn_OCPNNativeLib_ScheduleCleanExit";
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        evt.SetId( SCHEDULED_EVENT_CLEAN_EXIT );
        if(g_androidUtilHandler ){
            g_androidUtilHandler->AddPendingEvent(evt);
        }
        
        return 1;
    }
}       


#define REPLACE_NL_LANGINFO 0

/* nl_langinfo() replacement: query locale dependent information.
   Copyright (C) 2007-2012 Free Software Foundation, Inc.
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
#include <config.h>
/* Specification.  */
#include <langinfo.h>
#if REPLACE_NL_LANGINFO
/* Override nl_langinfo with support for added nl_item values.  */
# include <locale.h>
# include <string.h>
# undef nl_langinfo
char *
rpl_nl_langinfo (nl_item item)
{
  switch (item)
    {
# if GNULIB_defined_CODESET
    case CODESET:
      {
        const char *locale;
        static char buf[2 + 10 + 1];
        locale = setlocale (LC_CTYPE, NULL);
        if (locale != NULL && locale[0] != '\0')
          {
            /* If the locale name contains an encoding after the dot, return
               it.  */
            const char *dot = strchr (locale, '.');
            if (dot != NULL)
              {
                const char *modifier;
                dot++;
                /* Look for the possible @... trailer and remove it, if any.  */
                modifier = strchr (dot, '@');
                if (modifier == NULL)
                  return dot;
                if (modifier - dot < sizeof (buf))
                  {
                    memcpy (buf, dot, modifier - dot);
                    buf [modifier - dot] = '\0';
                    return buf;
                  }
              }
          }
        return "";
      }
# endif
# if GNULIB_defined_T_FMT_AMPM
    case T_FMT_AMPM:
      return "%I:%M:%S %p";
# endif
# if GNULIB_defined_ERA
    case ERA:
      /* The format is not standardized.  In glibc it is a sequence of strings
         of the form "direction:offset:start_date:end_date:era_name:era_format"
         with an empty string at the end.  */
      return "";
    case ERA_D_FMT:
      /* The %Ex conversion in strftime behaves like %x if the locale does not
         have an alternative time format.  */
      item = D_FMT;
      break;
    case ERA_D_T_FMT:
      /* The %Ec conversion in strftime behaves like %c if the locale does not
         have an alternative time format.  */
      item = D_T_FMT;
      break;
    case ERA_T_FMT:
      /* The %EX conversion in strftime behaves like %X if the locale does not
         have an alternative time format.  */
      item = T_FMT;
      break;
    case ALT_DIGITS:
      /* The format is not standardized.  In glibc it is a sequence of 10
         strings, appended in memory.  */
      return "\0\0\0\0\0\0\0\0\0\0";
# endif
# if GNULIB_defined_YESEXPR || !FUNC_NL_LANGINFO_YESEXPR_WORKS
    case YESEXPR:
      return "^[yY]";
    case NOEXPR:
      return "^[nN]";
# endif
    default:
      break;
    }
  return nl_langinfo (item);
}
#else
/* Provide nl_langinfo from scratch.  */
# if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
/* Native Windows platforms.  */
#  define WIN32_LEAN_AND_MEAN  /* avoid including junk */
#  include <windows.h>
#  include <stdio.h>
# else
/* An old Unix platform without locales, such as Linux libc5 or BeOS.  */
# endif
# include <locale.h>
extern "C" 
char *
nl_langinfo (nl_item item)
{
  switch (item)
    {
    /* nl_langinfo items of the LC_CTYPE category */
    case CODESET:
# if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
      {
        static char buf[2 + 10 + 1];
        /* The Windows API has a function returning the locale's codepage as
           a number.  */
        sprintf (buf, "CP%u", GetACP ());
        return buf;
      }
# elif defined __BEOS__
      return "UTF-8";
# else
      return (char *)"ISO-8859-1";
# endif
    /* nl_langinfo items of the LC_NUMERIC category */
    case RADIXCHAR:
      return localeconv () ->decimal_point;
    case THOUSEP:
      return localeconv () ->thousands_sep;
    /* nl_langinfo items of the LC_TIME category.
       TODO: Really use the locale.  */
    case D_T_FMT:
    case ERA_D_T_FMT:
      return (char *)"%a %b %e %H:%M:%S %Y";
    case D_FMT:
    case ERA_D_FMT:
      return (char *)"%m/%d/%y";
    case T_FMT:
    case ERA_T_FMT:
      return (char *)"%H:%M:%S";
    case T_FMT_AMPM:
      return (char *)"%I:%M:%S %p";
    case AM_STR:
      return (char *)"AM";
    case PM_STR:
      return (char *)"PM";
    case DAY_1:
      return (char *)"Sunday";
    case DAY_2:
      return (char *)"Monday";
    case DAY_3:
      return (char *)"Tuesday";
    case DAY_4:
      return (char *)"Wednesday";
    case DAY_5:
      return (char *)"Thursday";
    case DAY_6:
      return (char *)"Friday";
    case DAY_7:
      return (char *)"Saturday";
    case ABDAY_1:
      return (char *)"Sun";
    case ABDAY_2:
      return (char *)"Mon";
    case ABDAY_3:
      return (char *)"Tue";
    case ABDAY_4:
      return (char *)"Wed";
    case ABDAY_5:
      return (char *)"Thu";
    case ABDAY_6:
      return (char *)"Fri";
    case ABDAY_7:
      return (char *)"Sat";
    case MON_1:
      return (char *)"January";
    case MON_2:
      return (char *)"February";
    case MON_3:
      return (char *)"March";
    case MON_4:
      return (char *)"April";
    case MON_5:
      return (char *)"May";
    case MON_6:
      return (char *)"June";
    case MON_7:
      return (char *)"July";
    case MON_8:
      return (char *)"August";
    case MON_9:
      return (char *)"September";
    case MON_10:
      return (char *)"October";
    case MON_11:
      return (char *)"November";
    case MON_12:
      return (char *)"December";
    case ABMON_1:
      return (char *)"Jan";
    case ABMON_2:
      return (char *)"Feb";
    case ABMON_3:
      return (char *)"Mar";
    case ABMON_4:
      return (char *)"Apr";
    case ABMON_5:
      return (char *)"May";
    case ABMON_6:
      return (char *)"Jun";
    case ABMON_7:
      return (char *)"Jul";
    case ABMON_8:
      return (char *)"Aug";
    case ABMON_9:
      return (char *)"Sep";
    case ABMON_10:
      return (char *)"Oct";
    case ABMON_11:
      return (char *)"Nov";
    case ABMON_12:
      return (char *)"Dec";
    case ERA:
      return (char *)"";
    case ALT_DIGITS:
      return (char *)"\0\0\0\0\0\0\0\0\0\0";
    /* nl_langinfo items of the LC_MONETARY category
       TODO: Really use the locale. */
    case CRNCYSTR:
      return (char *)"-";
    /* nl_langinfo items of the LC_MESSAGES category
       TODO: Really use the locale. */
    case YESEXPR:
      return (char *)"^[yY]";
    case NOEXPR:
      return (char *)"^[nN]";
    default:
      return (char *)"";
    }
}
#endif

/* 
 * Copyright (C) 2014, Galois, Inc.
 * This sotware is distributed under a standard, three-clause BSD license.
 * Please see the file LICENSE, distributed with this software, for specific
 * terms and conditions.
 */
#include <stdlib.h>

#define isdigit(c) (c >= '0' && c <= '9')

double atof(const char *s)
{
  // This function stolen from either Rolf Neugebauer or Andrew Tolmach. 
  // Probably Rolf.
  double a = 0.0;
  int e = 0;
  int c;
  while ((c = *s++) != '\0' && isdigit(c)) {
    a = a*10.0 + (c - '0');
  }
  if (c == '.') {
    while ((c = *s++) != '\0' && isdigit(c)) {
      a = a*10.0 + (c - '0');
      e = e-1;
    }
  }
  if (c == 'e' || c == 'E') {
    int sign = 1;
    int i = 0;
    c = *s++;
    if (c == '+')
      c = *s++;
    else if (c == '-') {
      c = *s++;
      sign = -1;
    }
    while (isdigit(c)) {
      i = i*10 + (c - '0');
      c = *s++;
    }
    e += i*sign;
  }
  while (e > 0) {
    a *= 10.0;
    e--;
  }
  while (e < 0) {
    a *= 0.1;
    e++;
  }
  return a;
}

#if 0
// The methods below are required to link correctly on Android devices running API 19/20
//  This will, of course, require a separate APK build to support API 19/20 (Android Version 4.4, KitKat)
//  OpenCPN is not supported for API < 19

long long int wcstoll (const wchar_t* str, wchar_t** endptr, int base){
    return 0;
}

unsigned long long wcstoull( const wchar_t* str, wchar_t** str_end, int base ){
    return 0;
}

unsigned long wcstoul( const wchar_t* str, wchar_t** str_end, int base ){
    return 0;
}

long wcstol( const wchar_t* str, wchar_t** str_end, int base ){
    return 0;
}

/*-
 * Copyright (c) 2002 Tim J. Robbins
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
//__FBSDID("$FreeBSD$");

#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

/*
 * Convert a string to a double-precision number.
 *
 * This is the wide-character counterpart of strtod(). So that we do not
 * have to duplicate the code of strtod() here, we convert the supplied
 * wide character string to multibyte and call strtod() on the result.
 * This assumes that the multibyte encoding is compatible with ASCII
 * for at least the digits, radix character and letters.
 */
double
wcstod(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
        static /*const*/ mbstate_t initial;
        mbstate_t mbs;
        double val;
        char *buf, *end;
        const wchar_t *wcp;
        size_t len;

        while (iswspace(*nptr))
                nptr++;

        /*
         * Convert the supplied numeric wide char. string to multibyte.
         *
         * We could attempt to find the end of the numeric portion of the
         * wide char. string to avoid converting unneeded characters but
         * choose not to bother; optimising the uncommon case where
         * the input string contains a lot of text after the number
         * duplicates a lot of strtod()'s functionality and slows down the
         * most common cases.
         */
        wcp = nptr;
        mbs = initial;
        if ((len = wcsrtombs(NULL, &wcp, 0, &mbs)) == (size_t)-1) {
                if (endptr != NULL)
                        *endptr = (wchar_t *)nptr;
                return (0.0);
        }
        if ((buf = (char *)malloc(len + 1)) == NULL)
                return (0.0);
        mbs = initial;
        wcsrtombs(buf, &wcp, len + 1, &mbs);

        /* Let strtod() do most of the work for us. */
        val = strtod(buf, &end);

        /*
         * We only know where the number ended in the _multibyte_
         * representation of the string. If the caller wants to know
         * where it ended, count multibyte characters to find the
         * corresponding position in the wide char string.
         */
        if (endptr != NULL)
                /* XXX Assume each wide char is one byte. */
                *endptr = (wchar_t *)nptr + (end - buf);

        free(buf);

        return (val);
}

int vswscanf (const wchar_t* ws, const wchar_t* format, va_list arg){
    return 1;
}

extern "C" int rand() {
  return 0; //random();
}
extern "C" void srand(unsigned int seed) {
  return; // srandom(seed);
}



// extern "C" struct lconv* localeconv (void)
// {
//     
//     return 0;
// }

/* Copyright (C) 1991-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
#include <locale.h>
//#include "localeinfo.h"
/* Return monetary and numeric information about the current locale.  */
extern "C"
struct lconv *
localeconv (void)
{
  static struct lconv result;
  result.decimal_point = (char *) "."; //_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT);
#if 0
  result.thousands_sep = (char *) _NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP);
  result.grouping = (char *) _NL_CURRENT (LC_NUMERIC, GROUPING);
  if (*result.grouping == '\177' || *result.grouping == '\377')
    result.grouping = (char *) "";
  result.int_curr_symbol = (char *) _NL_CURRENT (LC_MONETARY, INT_CURR_SYMBOL);
  result.currency_symbol = (char *) _NL_CURRENT (LC_MONETARY, CURRENCY_SYMBOL);
  result.mon_decimal_point = (char *) _NL_CURRENT (LC_MONETARY,
                                                   MON_DECIMAL_POINT);
  result.mon_thousands_sep = (char *) _NL_CURRENT (LC_MONETARY,
                                                   MON_THOUSANDS_SEP);
  result.mon_grouping = (char *) _NL_CURRENT (LC_MONETARY, MON_GROUPING);
  if (*result.mon_grouping == '\177' || *result.mon_grouping == '\377')
    result.mon_grouping = (char *) "";
  result.positive_sign = (char *) _NL_CURRENT (LC_MONETARY, POSITIVE_SIGN);
  result.negative_sign = (char *) _NL_CURRENT (LC_MONETARY, NEGATIVE_SIGN);
#define INT_ELEM(member, element) \
  result.member = *(char *) _NL_CURRENT (LC_MONETARY, element);                      \
  if (result.member == '\377') result.member = CHAR_MAX
  INT_ELEM (int_frac_digits, INT_FRAC_DIGITS);
  INT_ELEM (frac_digits, FRAC_DIGITS);
  INT_ELEM (p_cs_precedes, P_CS_PRECEDES);
  INT_ELEM (p_sep_by_space, P_SEP_BY_SPACE);
  INT_ELEM (n_cs_precedes, N_CS_PRECEDES);
  INT_ELEM (n_sep_by_space, N_SEP_BY_SPACE);
  INT_ELEM (p_sign_posn, P_SIGN_POSN);
  INT_ELEM (n_sign_posn, N_SIGN_POSN);
  INT_ELEM (int_p_cs_precedes, INT_P_CS_PRECEDES);
  INT_ELEM (int_p_sep_by_space, INT_P_SEP_BY_SPACE);
  INT_ELEM (int_n_cs_precedes, INT_N_CS_PRECEDES);
  INT_ELEM (int_n_sep_by_space, INT_N_SEP_BY_SPACE);
  INT_ELEM (int_p_sign_posn, INT_P_SIGN_POSN);
  INT_ELEM (int_n_sign_posn, INT_N_SIGN_POSN);
#endif  
  return &result;
}

locale_t newlocale(int category_mask, const char *locale, locale_t base){
    return (locale_t)0;
}

void freelocale(locale_t locobj){
    return;
}

char *setlocale(int category, const char *locale){
    return (char *)"";
}


#undef _FORTIFY_SOURCE
#include <sys/select.h>

extern "C" int __FD_ISSET_chk(int fd, const fd_set* set, size_t s) {
  return FD_ISSET(fd, set);
}

extern "C" void __FD_SET_chk(int fd, fd_set* set, size_t s) {
  FD_SET(fd, set);
}

extern "C" void __FD_CLR_chk(int fd, fd_set* set, size_t s) {
  FD_CLR(fd, set);
}

int sigismember(const sigset_t *set, int signum) {
  /* Signal numbers start at 1, but bit positions start at 0. */
  int bit = signum - 1;
  const unsigned long *local_set = (const unsigned long *)set;
  if (set == NULL || bit < 0 || bit >= (int)(8 * sizeof(sigset_t))) {
    errno = EINVAL;
    return -1;
  }
  return (int)((local_set[bit / LONG_BIT] >> (bit % LONG_BIT)) & 1);
}

int sigaddset(sigset_t *set, int signum) {
  /* Signal numbers start at 1, but bit positions start at 0. */
  int bit = signum - 1;
  unsigned long *local_set = (unsigned long *)set;
  if (set == NULL || bit < 0 || bit >= (int)(8 * sizeof(sigset_t))) {
    errno = EINVAL;
    return -1;
  }
  local_set[bit / LONG_BIT] |= 1UL << (bit % LONG_BIT);
  return 0;
}

int sigdelset(sigset_t *set, int signum) {
  /* Signal numbers start at 1, but bit positions start at 0. */
  int bit = signum - 1;
  unsigned long *local_set = (unsigned long *)set;
  if (set == NULL || bit < 0 || bit >= (int)(8 * sizeof(sigset_t))) {
    errno = EINVAL;
    return -1;
  }
  local_set[bit / LONG_BIT] &= ~(1UL << (bit % LONG_BIT));
  return 0;
}

int sigemptyset(sigset_t *set) {
  if (set == NULL) {
    errno = EINVAL;
    return -1;
  }
  memset(set, 0, sizeof(sigset_t));
  return 0;
}

extern "C" int
pthread_mutex_timedlock (pthread_mutex_t *mutex,  const struct timespec *timeout)
{
 struct timeval timenow;
 struct timespec sleepytime;
 int retcode;
 
 /* This is just to avoid a completely busy wait */
 sleepytime.tv_sec = 0;
 sleepytime.tv_nsec = 10000000; /* 10ms */
 
 while ((retcode = pthread_mutex_trylock (mutex)) == EBUSY) {
  gettimeofday (&timenow, NULL);
  
  if (timenow.tv_sec >= timeout->tv_sec &&
      (timenow.tv_usec * 1000) >= timeout->tv_nsec) {
   return ETIMEDOUT;
  }
  
  nanosleep (&sleepytime, NULL);
 }
 
 return retcode;
}

FILE *stderr = (FILE *)2;
FILE *stdout = (FILE *)1;

#define MAP_FAILED      ((void *)-1)

extern "C" void* mmap64(void* __addr, size_t __size, int __prot, int __flags, int __fd,  off64_t __offset) {
  const int __mmap2_shift = 12; // 2**12 == 4096
  if (__offset < 0 || (__offset & ((1UL << __mmap2_shift) - 1)) != 0) {
    errno = EINVAL;
    return MAP_FAILED;
  }

  // prevent allocations large enough for `end - start` to overflow
  size_t __rounded = __BIONIC_ALIGN(__size, PAGE_SIZE);
  if (__rounded < __size || __rounded > PTRDIFF_MAX) {
    errno = ENOMEM;
    return MAP_FAILED;
  }

  extern void* __mmap2(void* __addr, size_t __size, int __prot, int __flags, int __fd, size_t __offset);
  return __mmap2(__addr, __size, __prot, __flags, __fd, __offset >> __mmap2_shift);
}

#endif
