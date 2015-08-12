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

#include <QtAndroidExtras/QAndroidJniObject>

#include "dychart.h"
#include "androidUTIL.h"
#include "OCPN_DataStreamEvent.h"
#include "chart1.h"
#include "AISTargetQueryDialog.h"
#include "AISTargetAlertDialog.h"
#include "AISTargetListDialog.h"
#include "routeprop.h"
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

class androidUtilHandler;



extern MyFrame                  *gFrame;
extern const wxEventType wxEVT_OCPN_DATASTREAM;
extern const wxEventType wxEVT_DOWNLOAD_EVENT;

wxEvtHandler                    *s_pAndroidNMEAMessageConsumer;
wxEvtHandler                    *s_pAndroidBTNMEAMessageConsumer;

extern AISTargetAlertDialog      *g_pais_alert_dialog_active;
extern AISTargetQueryDialog      *g_pais_query_dialog_active;
extern AISTargetListDialog       *g_pAISTargetList;
extern MarkInfoImpl              *pMarkPropDialog;
extern RouteProp                 *pRoutePropDialog;
extern TrackPropDlg              *pTrackPropDialog;
extern MarkInfoImpl              *pMarkInfoDialog;
extern S57QueryDialog            *g_pObjectQueryDialog;
extern options                   *g_options;
extern bool                       g_bSleep;
androidUtilHandler               *g_androidUtilHandler;
extern wxDateTime                 g_start_time;
extern RouteManagerDialog        *pRouteManagerDialog;
extern ChartCanvas               *cc1;

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
extern bool             g_bShowMoored;
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
extern bool             g_bTransparentToolbar;
extern bool             g_bTransparentToolbarInOpenGLOK;

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



//#ifdef USE_S57
extern s52plib          *ps52plib;
//#endif

extern wxString         g_locale;
extern bool             g_bportable;
extern bool             g_bdisable_opengl;
extern wxString         *pHome_Locn;

extern ChartGroupArray  *g_pGroupArray;


extern bool             g_bUIexpert;
//    Some constants
#define ID_CHOICE_NMEA  wxID_HIGHEST + 1

extern wxArrayString *EnumerateSerialPorts(void);           // in chart1.cpp

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

wxString callActivityMethod_vs(const char *method);


//      Globals, accessible only to this module

JavaVM *java_vm;
JNIEnv* jenv;
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
bool            g_follow_active;
bool            g_track_active;

wxSize          config_size;

bool            s_bdownloading;
wxString        s_requested_url;
wxEvtHandler    *s_download_evHandler;

#define ANDROID_EVENT_TIMER 4389

#define ACTION_NONE                     -1
#define ACTION_RESIZE_PERSISTENTS       1
#define ACTION_FILECHOOSER_END          3

class androidUtilHandler : public wxEvtHandler
{
 public:
     androidUtilHandler();
    ~androidUtilHandler() {}
    
    void onTimerEvent(wxTimerEvent &event);
    
    wxString GetStringResult(){ return m_stringResult; }
    
    wxTimer     m_eventTimer;
    int         m_action;
    bool        m_done;
    wxString    m_stringResult;
    
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE ( androidUtilHandler, wxEvtHandler )
EVT_TIMER ( ANDROID_EVENT_TIMER, androidUtilHandler::onTimerEvent )
END_EVENT_TABLE()

androidUtilHandler::androidUtilHandler()
{
    m_eventTimer.SetOwner( this, ANDROID_EVENT_TIMER );
    
}


void androidUtilHandler::onTimerEvent(wxTimerEvent &event)
{
    qDebug() << "onTimerEvent";

    switch(m_action){
        case ACTION_RESIZE_PERSISTENTS:            //  Handle rotation/resizing of persistent dialogs

            // AIS Target Query
            if( g_pais_query_dialog_active ) {
                bool bshown = g_pais_query_dialog_active->IsShown();
                g_pais_query_dialog_active->Hide();
                g_pais_query_dialog_active->RecalculateSize();
                if(bshown){
                    g_pais_query_dialog_active->Show();
                    g_pais_query_dialog_active->Raise();
                }
            }
            
            // Route Props
            if(RouteProp::getInstanceFlag()){
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
            if(MarkInfoImpl::getInstanceFlag()){
                bool bshown = pMarkPropDialog->IsShown();
                if(bshown){
                    pMarkPropDialog->Hide();
                    pMarkPropDialog->RecalculateSize();
                    pMarkPropDialog->Show();
                }
                else{
                    pMarkPropDialog->Destroy();
                    pMarkPropDialog = NULL;
                }
            }
            
            if(pMarkPropDialog){
                bool bshown = pMarkPropDialog->IsShown();
                pMarkPropDialog->Hide();
                pMarkPropDialog->RecalculateSize();
                if(bshown){
                    pMarkPropDialog->Show();
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
            
            // Options dialog
            if(g_options){
                bool bshown = g_options->IsShown();
                g_options->Hide();
                g_options->RecalculateSize();
                if(bshown){
                    g_options->ShowModal();
                }
            }
            
            // AIS Target List dialog
            if(g_pAISTargetList){
                bool bshown = g_pAISTargetList->IsShown();
                g_pAISTargetList->Hide();
                g_pAISTargetList->RecalculateSize();
                if(bshown){
                    g_pAISTargetList->Show();
                }
            }
            
            // Tide/Current window
            if(cc1->getTCWin()){
                bool bshown = cc1->getTCWin()->IsShown();
                cc1->getTCWin()->Hide();
                cc1->getTCWin()->RecalculateSize();
                if(bshown){
                    cc1->getTCWin()->Show();
                    cc1->getTCWin()->Refresh();
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
            
            break;
 
            
        case ACTION_FILECHOOSER_END:            //  Handle polling of android Dialog
            {
                qDebug() << "chooser poll";
                //  Get a reference to the running FileChooser
                QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                "activity", "()Landroid/app/Activity;");
                
                if ( !activity.isValid() ){
                    qDebug() << "onTimerEvent : Activity is not valid";
                    return;
                }
 
                //  Call the method which tracks the completion of the Intent.
                QAndroidJniObject data = activity.callObjectMethod("isFileChooserFinished", "()Ljava/lang/String;");
                
                jstring s = data.object<jstring>();
                
                //  Need a Java environment to decode the resulting string
                if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
                    qDebug() << "GetEnv failed.";
                }
                else {
                    
                    // The string coming back will be one of:
                    //  "no"   ......Intent not done yet.
                    //  "cancel:"   .. user cancelled intent.
                    //  "file:{file_name}"  .. user selected this file, fully qualified.
                    if(!s){
                        qDebug() << "isFileChooserFinished returned null";
                    }
                    else if( (jenv)->GetStringLength( s )){
                        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
                        qDebug() << "isFileChooserFinished returned " << ret_string;
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
        default:
            break;
    }
    
}


bool androidUtilInit( void )
{
    g_androidUtilHandler = new androidUtilHandler();

    //  Initialize some globals
    wxString dirs = callActivityMethod_vs("getSystemDirs");
    wxStringTokenizer tk(dirs, _T(";"));
    if( tk.HasMoreTokens() ){
        wxString token = tk.GetNextToken();
        if(wxNOT_FOUND != token.Find(_T("EXTAPP")))
            g_bExternalApp = true;
        
        token = tk.GetNextToken();              
        g_androidFilesDir = token;
        token = tk.GetNextToken();              
        g_androidCacheDir = token;
        token = tk.GetNextToken();              
        g_androidExtFilesDir = token;
        token = tk.GetNextToken();              
        g_androidExtCacheDir = token;
        token = tk.GetNextToken();              
        g_androidExtStorageDir = token;
        
    }
    
    g_mask = -1;
    g_sel = -1;
    
    return true;
}


jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    qDebug() << "JNI_OnLoad";
    java_vm = vm;
    
    // Get JNI Env for all function calls
    if (vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
        return -1;
    }
    
#if 0
    // Find the class calling native function
    jclass NativeUsb = (*env)->FindClass(env, "com/venky/Home");
    if (class_home == NULL) {
        LOG_D ("FindClass failed : No class found.");
        return -1;
}

// Register native method for getUsbPermission
JNINativeMethod nm[1] = {
    { "getPermission", "(Landroid/app/Activity;)I", get_permission}
};

if ((*env)->RegisterNatives(env, NativeUsb, nm , 1)) {
    LOG_D ("RegisterNatives Failed.");
    return -1;
}
#endif
    return JNI_VERSION_1_6;
}



//      OCPNNativeLib
//      This is a set of methods which can be called from the android activity context.

extern "C"{
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_test(JNIEnv *env, jobject obj)
{
    qDebug() << "test";
    
    return 55;
}
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_processNMEA(JNIEnv *env, jobject obj, jstring nmea_string)
    {
        const char *string = env->GetStringUTFChars(nmea_string, NULL);
//        wxString wstring = wxString(string, wxConvUTF8);
        
//        qDebug() << "processNMEA" << string;
 
        char tstr[200];
        strncpy(tstr, string, 190);
        strcat(tstr, "\r\n");
        
        if( s_pAndroidNMEAMessageConsumer ) {
            OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
            Nevent.SetNMEAString( tstr );
            Nevent.SetStream( NULL );
                
            s_pAndroidNMEAMessageConsumer->AddPendingEvent(Nevent);
        }
        
        return 66;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_processBTNMEA(JNIEnv *env, jobject obj, jstring nmea_string)
    {
        const char *string = env->GetStringUTFChars(nmea_string, NULL);
        wxString wstring = wxString(string, wxConvUTF8);
        
        qDebug() << "processNMEA" << string;
        
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
        GetAndroidDisplaySize();
        
        wxSize new_size = getAndroidDisplayDimensions();
        qDebug() << "onConfigChange" << new_size.x << new_size.y;
        
        config_size = new_size;
        gFrame->TriggerResize(new_size);

        if(g_androidUtilHandler){
            
            g_androidUtilHandler->m_action = ACTION_RESIZE_PERSISTENTS;
            g_androidUtilHandler->m_eventTimer.Start(200, wxTIMER_ONE_SHOT);
        }
        
        return 77;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onMenuKey(JNIEnv *env, jobject obj)
    {
        qDebug() << "onMenuKey";

        gFrame->ToggleToolbar();
            
        return 88;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onStop(JNIEnv *env, jobject obj)
    {
        qDebug() << "onStop";
        
        //  App may be summarily killed after this point due to OOM condition.
        //  So we need to persist some dynamic data.
        if(pConfig){
            qDebug() << "startPersist";
        
        //  Persist the config file, especially to capture the viewport location,scale etc.
            pConfig->UpdateSettings();
        
        //  There may be unsaved objects at this point, and a navobj.xml.changes restore file
        //  We commit the navobj deltas, and flush the restore file 
            pConfig->UpdateNavObj();

            qDebug() << "endPersist";
        }
        
        return 98;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onStart(JNIEnv *env, jobject obj)
    {
        qDebug() << "onStart";
        
        // Set initial ActionBar item states
        androidSetFollowTool(cc1->m_bFollow);
        androidSetRouteAnnunciator( false );
        
        return 99;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onPause(JNIEnv *env, jobject obj)
    {
        qDebug() << "onPause";
        
        g_bSleep = true;
        
        
        
        return 97;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onResume(JNIEnv *env, jobject obj)
    {
        qDebug() << "onResume";
        
        g_bSleep = false;
        
        return 96;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_selectChartDisplay(JNIEnv *env, jobject obj, int type, int family)
    {
        qDebug() << "selectChartDisplay" << type << family;
        
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        if(type == CHART_TYPE_CM93COMP){
            evt.SetId( ID_CMD_SELECT_CHART_TYPE );
            evt.SetExtraLong( CHART_TYPE_CM93COMP);
        }
        else{
            evt.SetId( ID_CMD_SELECT_CHART_FAMILY );
            evt.SetExtraLong( family);
        }
        
        if(gFrame){
            qDebug() << "add event" << type << family;
            gFrame->GetEventHandler()->AddPendingEvent(evt);
        }

        
        return 74;
    }
}
    
extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_invokeCmdEventCmdString(JNIEnv *env, jobject obj, int cmd_id, jstring s)
    {
        const char *sparm;
        wxString wx_sparm;
        //  Need a Java environment to decode the string parameter
        if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
            qDebug() << "GetEnv failed.";
        }
        else {
            sparm = (jenv)->GetStringUTFChars(s, NULL);
            wx_sparm = wxString(sparm, wxConvUTF8);
        }
        
        qDebug() << "invokeCmdEventCmdString" << cmd_id << s;
        
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        evt.SetId( cmd_id );
        evt.SetString( wx_sparm);
        
        if(gFrame){
            qDebug() << "add event" << cmd_id << s;
            gFrame->GetEventHandler()->AddPendingEvent(evt);
        }

        
        return 71;
    }
}
    
        
extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_invokeMenuItem(JNIEnv *env, jobject obj, int item)
    {
        qDebug() << "invokeMenuItem" << item;
        
        // If in Route Create, disable all other menu items
        if( (gFrame->nRoute_State > 1 ) && (OCPN_ACTION_ROUTE != item) ) {
            return 72;
        }
            
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
                
            default:
                break;
        }
        
        return 73;
    }
}


extern "C"{
    JNIEXPORT jstring JNICALL Java_org_opencpn_OCPNNativeLib_getVPCorners(JNIEnv *env, jobject obj)
    {
        qDebug() << "getVPCorners";
        
        LLBBox vbox;
        if(cc1){
            vbox = cc1->GetVP().GetBBox();
        }
            
        wxString s;
        s.Printf(_T("%g;%g;%g;%g;"), vbox.GetMaxY(), vbox.GetMaxX(), vbox.GetMinY(), vbox.GetMinX());  
                    
//        jstring ret = (env)->NewStringUTF("40.1; -85; 39; -86;");
        jstring ret = (env)->NewStringUTF(s.c_str());
        
        return ret;
    }
        
}       


extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_setDownloadStatus(JNIEnv *env, jobject obj, int status, jstring url)
    {
        qDebug() << "setDownloadStatus";
 
        const char *sparm;
        wxString wx_sparm;
        
        //  Need a Java environment to decode the string parameter
        if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
            qDebug() << "GetEnv failed.";
        }
        else {
            sparm = (jenv)->GetStringUTFChars(url, NULL);
            wx_sparm = wxString(sparm, wxConvUTF8);
        }
        
        if(s_bdownloading && wx_sparm.IsSameAs(s_requested_url) ){
            
            qDebug() << "Maybe mine...";
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
                qDebug() << "Sending event...";
                s_download_evHandler->AddPendingEvent(ev);
            }
            
            
        }
       
        
        return 77;
    }
    
}       


        

wxString callActivityMethod_vs(const char *method)
{
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod(method, "()Ljava/lang/String;");
    
    jstring s = data.object<jstring>();
    qDebug() << s;
    
    if(s){
        //  Need a Java environment to decode the resulting string
        if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
            qDebug() << "GetEnv failed.";
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
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod(method, "(I)Ljava/lang/String;", parm);
    
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
    
}

wxString callActivityMethod_iis(const char *method, int parm1, int parm2)
{
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod(method, "(II)Ljava/lang/String;", parm1, parm2);
    
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
    
}

wxString callActivityMethod_ss(const char *method, wxString parm)
{
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
        return return_string;
    }

    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
        return _T("jenv Error");
    }
    
    jstring p = (jenv)->NewStringUTF(parm.c_str());
    
    
    //  Call the desired method
    qDebug() << "Calling method_ss";
    qDebug() << method;
    
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;)Ljava/lang/String;", p);

    qDebug() << "Back from method_ss";
    
    jstring s = data.object<jstring>();
    
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
    
}

wxString callActivityMethod_s2s(const char *method, wxString parm1, wxString parm2)
{
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
        return _T("jenv Error");
    }
    
    jstring p1 = (jenv)->NewStringUTF(parm1.c_str());
    jstring p2 = (jenv)->NewStringUTF(parm2.c_str());
    
    
    //  Call the desired method
    qDebug() << "Calling method_s2s" << " (" << method << ")";
    
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1, p2);
    
    qDebug() << "Back from method_s2s";
    
    jstring s = data.object<jstring>();
    
    if( (jenv)->GetStringLength( s )){
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    return return_string;
    
}

wxString callActivityMethod_s4s(const char *method, wxString parm1, wxString parm2, wxString parm3, wxString parm4)
{
    wxString return_string;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
        return return_string;
    }
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
        return _T("jenv Error");
    }
    
    jstring p1 = (jenv)->NewStringUTF(parm1.c_str());
    jstring p2 = (jenv)->NewStringUTF(parm2.c_str());
    jstring p3 = (jenv)->NewStringUTF(parm3.c_str());
    jstring p4 = (jenv)->NewStringUTF(parm4.c_str());

    const char *ts = (jenv)->GetStringUTFChars(p2, NULL);
    
    qDebug() << "Test String p2" << ts;
    
    //  Call the desired method
    qDebug() << "Calling method_s4s" << " (" << method << ")";
    
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
                                                       p1, p2, p3, p4);
    
    qDebug() << "Back from method_s4s";
    
    jstring s = data.object<jstring>();
    
     if( (jenv)->GetStringLength( s )){
         const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
         return_string = wxString(ret_string, wxConvUTF8);
     }
    
    return return_string;
    
}


wxString androidGetDeviceInfo()
{
    wxString info = callActivityMethod_vs("getDeviceInfo");
    
    return info;
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
        if(g_androidExtFilesDir.Length())
            return g_androidExtFilesDir;
    }

    return _T("/mnt/sdcard/opencpn"); //g_androidFilesDir;
}

wxString androidGetSharedDir()                 // Used for assets like uidata, s57data, etc
{
    if(g_bExternalApp){
        if(g_androidExtFilesDir.Length())
            return g_androidExtFilesDir + _T("/");
    }
    
    return g_androidFilesDir + _T("/");
}

wxString androidGetCacheDir()                 // Used for raster_texture_cache, mmsitoname.csv, etc
{
    if(g_bExternalApp){
        if(g_androidExtCacheDir.Length())
            return g_androidExtCacheDir;
    }
    
    return g_androidCacheDir;
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

extern void androidSetFollowTool(bool bactive)
{
//    qDebug() << "setFollowIconState" << bactive;
    
    if(g_follow_active != bactive)
        callActivityMethod_is("setFollowIconState", bactive?1:0);
    
    g_follow_active = bactive;
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
        qDebug() << "androidSetChartTypeMaskSel" << mask << sel;
        callActivityMethod_iis("configureNavSpinnerTS", mask, sel);
        g_mask = mask;
        g_sel = sel;
    }
}       


void androidEnableBackButton(bool benable)
{
    callActivityMethod_is("setBackButtonState", benable?1:0);
}


bool androidGetMemoryStatus( int *mem_total, int *mem_used )
{
    
    if(g_start_time.GetTicks() > 1441080000 )
        exit(0);
    
    //  On android, We arbitrarily declare that we have used 50% of available memory.
    if(mem_total)
        *mem_total = 100 * 1024;
    if(mem_used)
        *mem_used = 50 * 1024;
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
        qDebug() << "Activity is not valid";
        return false;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("getDisplayMetrics", "()Ljava/lang/String;");
    
    wxString return_string;
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    wxLogMessage(_T("Metrics:") + return_string);
    wxSize screen_size = ::wxGetDisplaySize();
    wxString msg;
    msg.Printf(_T("wxGetDisplaySize(): %d %d"), screen_size.x, screen_size.y);
    wxLogMessage(msg);
    
    double density = 1.0;
    wxStringTokenizer tk(return_string, _T(";"));
    if( tk.HasMoreTokens() ){
        wxString token = tk.GetNextToken();     // xdpi
        token = tk.GetNextToken();              // density
        
        long b = ::wxGetDisplaySize().y;        
        token.ToDouble( &density );

        token = tk.GetNextToken();              // ldpi
        
        token = tk.GetNextToken();              // width
        token = tk.GetNextToken();              // height - statusBarHeight
        token = tk.GetNextToken();              // width
        token = tk.GetNextToken();              // height
        token = tk.GetNextToken();              // dm.widthPixels
        token = tk.GetNextToken();              // dm.heightPixels
 
        token = tk.GetNextToken();              // actionBarHeight
        long abh;
        token.ToLong( &abh );
        g_ActionBarHeight = wxMax(abh, 50);

//        qDebug() << "g_ActionBarHeight" << abh << g_ActionBarHeight;
        
    }
    
    double ldpi = 160. * density;
    
    double maxDim = wxMax(::wxGetDisplaySize().x, ::wxGetDisplaySize().y);
    ret = (maxDim / ldpi) * 25.4;
 
    msg.Printf(_T("Android Auto Display Size (mm, est.): %g"), ret);
    wxLogMessage(msg);
    
    //  Save some items as global statics for convenience
    g_androidDPmm = ldpi / 25.4;
    g_androidDensity = density;

//    qDebug() << "g_androidDPmm" << g_androidDPmm;
//    qDebug() << "Auto Display Size (mm)" << ret;
//    qDebug() << "ldpi" << ldpi;
    
    
//     wxString istr = return_string.BeforeFirst('.');
//     
//     long ldpi;
//     if( istr.ToLong(&ldpi)){
//         ret = (::wxGetDisplaySize().x/(double)ldpi) * 25.4;
//     }

    return ret;
}

int getAndroidActionBarHeight()
{
    return g_ActionBarHeight;
}

double getAndroidDPmm()
{
    // Returns an estimate based on the pixel density reported
    if( g_androidDPmm < 0.01){
        GetAndroidDisplaySize();
    }
    
    // User override?
    if(g_config_display_size_mm > 0){
        double maxDim = wxMax(::wxGetDisplaySize().x, ::wxGetDisplaySize().y);
        double size_mm = g_config_display_size_mm;
        size_mm = wxMax(size_mm, 50);
        size_mm = wxMin(size_mm, 400);
        double ret = maxDim / size_mm;
        qDebug() << "getAndroidDPmm override" << maxDim << size_mm << g_config_display_size_mm;
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
        qDebug() << "Activity is not valid";
        return sz_ret;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("getDisplayMetrics", "()Ljava/lang/String;");
    
    wxString return_string;
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
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
        long a = ::wxGetDisplaySize().x;        // default is wxWidgets idea
        if(token.ToLong( &a ))
            sz_ret.x = a;
        
        token = tk.GetNextToken();
        long b = ::wxGetDisplaySize().y;        
        if(token.ToLong( &b ))
            sz_ret.y = b;
        
        token = tk.GetNextToken();              
        token = tk.GetNextToken();
        
        token = tk.GetNextToken();
        token = tk.GetNextToken();
        
        long abh = 0;
        token = tk.GetNextToken();              //  ActionBar height, if shown
        if(token.ToLong( &abh ))
            sz_ret.y -= abh;
            
        
        
    }

    qDebug() << sz_ret.x << sz_ret.y;
    
    return sz_ret;
    
}

void androidConfirmSizeCorrection()
{
    //  There is some confusion about the ActionBar size during configuration changes.
    //  We need to confirm the calculated display size, and fix it if necessary.
    //  This happens during staged resize events processed by gFrame->TriggerResize()
    
    wxSize targetSize = getAndroidDisplayDimensions();
//    qDebug() << "Confirming" << targetSize.y << config_size.y;
    if(config_size != targetSize){
//        qDebug() << "Correcting";
        gFrame->SetSize(targetSize);
        config_size = targetSize;
    }
}
        
void androidForceFullRepaint()
{
        wxSize targetSize = getAndroidDisplayDimensions();
        wxSize tempSize = targetSize;
        tempSize.y--;
        gFrame->SetSize(tempSize);
        gFrame->SetSize(targetSize);
}       

void androidShowBusyIcon()
{
    if(b_androidBusyShown)
        return;
    
    //  Get a reference to the running native activity
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
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
        qDebug() << "Activity is not valid";
        return;
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("hideBusyCircle", "()Ljava/lang/String;");

    b_androidBusyShown = false;
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
        qDebug() << "Android Device has internal GPS";
        wxLogMessage(_T("Android Device has internal GPS"));
    }
    else{
        qDebug() << "Android Device has NO internal GPS";
        wxLogMessage(_T("Android Device has NO internal GPS"));
    }
    return result;
}

bool androidStartNMEA(wxEvtHandler *consumer)
{
    s_pAndroidNMEAMessageConsumer = consumer;

    qDebug() << "androidStartNMEA";
    wxString s;
    
    s = androidGPSService( GPS_ON );
    wxLogMessage(s);
    if(s.Upper().Find(_T("DISABLED")) != wxNOT_FOUND){
        OCPNMessageBox(NULL,
                       _("Your android device has an internal GPS, but it is disabled.\n\
                       Please visit android Settings/Location dialog to enabled GPS"),
                        _T("OpenCPN"), wxOK );        
        
        androidStopNMEA();
        return false;
    }
    
    return true;
}

bool androidStopNMEA()
{
    s_pAndroidNMEAMessageConsumer = NULL;
    
    wxString s = androidGPSService( GPS_OFF );
    
    return true;
}


wxString androidGPSService(int parm)
{
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity", "()Landroid/app/Activity;");
    
    if ( !activity.isValid() ){
        qDebug() << "Activity is not valid";
        return _T("Activity is not valid");
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("queryGPSServer", "(I)Ljava/lang/String;", parm);
    
    wxString return_string;
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
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
        qDebug() << "Activity is not valid";
        return _T("Activity is not valid");
    }
    
    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("hasBluetooth", "(I)Ljava/lang/String;", 0);
    
    wxString query;
    jstring s = data.object<jstring>();
    
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        query = wxString(ret_string, wxConvUTF8);
    }
    
    bool result = query.Upper().IsSameAs(_T("YES"));
    
    if(result){
        qDebug() << "Android Device has internal Bluetooth";
        wxLogMessage(_T("Android Device has internal Bluetooth"));
    }
    else{
        qDebug() << "Android Device has NO internal Bluetooth";
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

int androidFileChooser( wxString *result, const wxString &initDir, const wxString &title,
                        const wxString &suggestion, const wxString &wildcard, bool dirOnly)
{
    wxString tresult;
    
    //  Start a timer to poll for results 
    if(g_androidUtilHandler){
        g_androidUtilHandler->m_eventTimer.Stop();
        g_androidUtilHandler->m_done = false;

        wxString activityResult;
        if(dirOnly)
            activityResult = callActivityMethod_s2s("DirChooserDialog", initDir, title);
        else
            activityResult = callActivityMethod_s4s("FileChooserDialog", initDir, title, suggestion, wildcard);
        
        if(activityResult == _T("OK") ){
            qDebug() << "ResultOK, starting spin loop";
            g_androidUtilHandler->m_action = ACTION_FILECHOOSER_END;
            g_androidUtilHandler->m_eventTimer.Start(1000, wxTIMER_CONTINUOUS);
        
        //  Spin, waiting for result
            while(!g_androidUtilHandler->m_done){
                wxMilliSleep(50);
                wxSafeYield(NULL, true);
            }
        
            qDebug() << "out of spin loop";
            g_androidUtilHandler->m_action = ACTION_NONE;
            g_androidUtilHandler->m_eventTimer.Stop();
        
        
            tresult = g_androidUtilHandler->GetStringResult();
            
            if( tresult.StartsWith(_T("cancel:")) ){
                qDebug() << "Cancel1";
                return wxID_CANCEL;
            }
            else if( tresult.StartsWith(_T("file:")) ){
                if(result){
                    *result = tresult.AfterFirst(':');
                    qDebug() << "OK";
                    return wxID_OK;
                }
                else{
                    qDebug() << "Cancel2";
                    return wxID_CANCEL;
                }
            }
        }
        else{
            qDebug() << "Result NOT OK";
        }
        
    }

    return wxID_CANCEL;
}

    
#if 0
void invokeApp( void )
{
    void runApplication(const QString &packageName, const QString &className)
{
    qDebug() << "Start app: " <<packageName <<", "<<className;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
                "org/qtproject/qt5/android/QtNative", "activity",
                "()Landroid/app/Activity;");   //activity is valid

    if ( activity.isValid() )
    {
        // Equivalent to Jave code: 'Intent intent = new Intent();'
        QAndroidJniObject intent("android/content/Intent","()V");

        if ( intent.isValid() )
        {
            QAndroidJniObject jPackageName = QAndroidJniObject::fromString(packageName);
            QAndroidJniObject jClassName = QAndroidJniObject::fromString(className);

            if ( jPackageName.isValid() && jClassName.isValid() )
            {
                // Equivalent to Jave code: 'intent.setClassName("com.android.settings", "com.android.settings.DevelopmentSettings");'
                intent.callObjectMethod("setClassName",
                                        "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                                        jPackageName.object<jstring>(),jClassName.object<jstring>());

                jint flag = QAndroidJniObject::getStaticField<jint>(
                            "android/content/Intent",
                            "FLAG_ACTIVITY_NEW_TASK");

                intent.callObjectMethod("setFlags", "(I)V",flag);

                // Equivalent to Jave code: 'startActivity(intent);'
                QAndroidJniEnvironment env;
                activity.callObjectMethod(
                            "startActivity",
                            "(Landroid/content/Intent;)V",
                            intent.object<jobject>());
                if (env->ExceptionCheck()) {
                    qDebug() << "Intent not found!";
                    env->ExceptionClear(); // TODO: stupid method! Remove this!
                }
            } else {
                qDebug() << "Action is not valid";
            }
        } else {
            qDebug() << "Intent is not valid";
        }
    } else {
        qDebug() << "Activity is not valid";
    }
}


}
#endif

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
            qDebug() << "Activity is not valid";
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
            result += chart_dir_array.Item(i);
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
    //         if ( depthUnit == 0 ) // feet
    //             conv = 0.3048f; // international definiton of 1 foot is 0.3048 metres
    //         else if ( depthUnit == 2 ) // fathoms
    //             conv = 0.3048f * 6; // 1 fathom is 6 feet
            
            s.Printf( _T("%4.0f;"), S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR ) / conv );
            s.Trim(false);
            result += _T("prefs_shallowdepth:") + s;
            
            s.Printf( _T("%4.0f;"), S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR ) / conv );
            s.Trim(false);
            result += _T("prefs_safetydepth:") + s;
            
            s.Printf( _T("%4.0f;"), S52_getMarinerParam( S52_MAR_DEEP_CONTOUR ) / conv );
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
                break;                  // there can only be one entry for type INTERNAL_GPS
            }                    
        }
    
    wxLogMessage(result);
    
    return result;
}

bool DoAndroidPreferences( void )
{
    qDebug() << "Start AndroidPreferences";
    
    wxString settings = BuildAndroidSettingsString();

    InvokeJNIPreferences(settings);

    return true;
}


int startAndroidFileDownload( const wxString &url, const wxString& destination, wxEvtHandler *evh, long *dl_id )
{
    if(evh){
        s_bdownloading = true;
        s_requested_url = url;
        s_download_evHandler = evh;
    
        wxString result = callActivityMethod_s2s( "downloadFile", url, destination );

        wxLogMessage(_T("downloads2s result: ") + result);
        long dl_ID;
        wxStringTokenizer tk(result, _T(";"));
        if( tk.HasMoreTokens() ){
            wxString token = tk.GetNextToken();
            if(token.IsSameAs(_T("OK"))){
                token = tk.GetNextToken();
                token.ToLong(&dl_ID);
                *dl_id = dl_ID;
                qDebug() << dl_ID;
                return 0;
            }
        }
    }
    
    return -1;
}

int queryAndroidFileDownload( long dl_ID, wxString *result )
{
    qDebug() << dl_ID;
    
    wxString stat = callActivityMethod_is( "getDownloadStatus", (int)dl_ID );
    *result = stat;
    
    wxLogMessage( _T("queryAndroidFileDownload: ") + stat); 
    
    return 0;
    
}

void finishAndroidFileDownload( void )
{
    s_bdownloading = false;
    s_requested_url.Clear();
    s_download_evHandler = NULL;
    
    return;
}


void cancelAndroidFileDownload( long dl_ID )
{
    wxString stat = callActivityMethod_is( "cancelDownload", (int)dl_ID );
}




#if 0
    // This is a way to invoke Android Settings dialog
        QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                               "activity", "()Landroid/app/Activity;");

        if ( activity.isValid() )
        {
            QAndroidJniObject intent("android/content/Intent","()V");
            if ( intent.isValid() )
            {
                QAndroidJniObject param1 = QAndroidJniObject::fromString("com.android.settings");
                QAndroidJniObject param2 = QAndroidJniObject::fromString("com.android.settings.DevelopmentSettings");

                if ( param1.isValid() && param2.isValid() )
                {
                    intent.callObjectMethod("setClassName","(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;", param1.object<jobject>(),param2.object<jobject>());

                    activity.callObjectMethod("startActivity","(Landroid/content/Intent;)V",intent.object<jobject>());
                }
            }
        }
#endif




















#if 0
    JNIEnv *env;
    JavaVM* lJavaVM = java_vm;     // static, comes from JNI_OnLoad
    app->activity->vm->AttachCurrentThread(&env, NULL);

    jobject lNativeActivity = app->activity->clazz;
    jclass intentClass = env->FindClass("android/content/Intent");
    jstring actionString =env->NewStringUTF("org.opencpn.opencpn.Settings");

    jmethodID newIntent = env->GetMethodID(intentClass, "<init>", "()V");
    jobject intent = env->AllocObject(intentClass);
    env->CallVoidMethod(intent, newIntent);

    jmethodID setAction = env->GetMethodID(intentClass, "setAction","(Ljava/lang/String;)Landroid/content/Intent;");
    env->CallObjectMethod(intent, setAction, actionString);

    jclass activityClass = env->FindClass("android/app/Activity");
    jmethodID startActivity = env->GetMethodID(activityClass,"startActivity", "(Landroid/content/Intent;)V");
    jobject intentObject = env->NewObject(intentClass,newIntent);
    env->CallVoidMethod(intentObject, setAction,actionString);
    env->CallVoidMethod(lNativeActivity, startActivity, intentObject);

    app->activity->vm->DetachCurrentThread();
#endif


