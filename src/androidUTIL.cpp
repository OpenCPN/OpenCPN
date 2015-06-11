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

class androidUtilHandler;

JavaVM *java_vm;
JNIEnv* jenv;
bool     b_androidBusyShown;

QString g_qtStyleSheet;



extern MyFrame                  *gFrame;
extern const wxEventType wxEVT_OCPN_DATASTREAM;
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

#define ANDROID_EVENT_TIMER 4389
#define ACTION_RESIZE_PERSISTENTS       1

class androidUtilHandler : public wxEvtHandler
{
 public:
     androidUtilHandler();
    ~androidUtilHandler() {}
    
    void onTimerEvent(wxTimerEvent &event);
    wxTimer     m_eventTimer;
    int         m_action;
    
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
            if(pRoutePropDialog){
                bool bshown = pRoutePropDialog->IsShown();
                pRoutePropDialog->Hide();
                pRoutePropDialog->RecalculateSize();
                if(bshown){
                    pRoutePropDialog->Show();
                }
            }
            

            // Track Props
            if(pTrackPropDialog){
                bool bshown = pTrackPropDialog->IsShown();
                pTrackPropDialog->Hide();
                pTrackPropDialog->RecalculateSize();
                if(bshown){
                    pTrackPropDialog->Show();
                }
            }
            
            
            // Mark Props
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
            
            // Route Manager dialog
            if(pRouteManagerDialog){
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
 
            
        default:
            break;
    }
    
}


bool androidUtilInit( void )
{
    g_androidUtilHandler = new androidUtilHandler();
    
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
        
        gFrame->TriggerResize(new_size);

        if(g_androidUtilHandler){
            
            g_androidUtilHandler->m_action = ACTION_RESIZE_PERSISTENTS;
            g_androidUtilHandler->m_eventTimer.Start(200, wxTIMER_ONE_SHOT);
        }
        
//        gFrame->DestroyPersistentDialogs();
        
//        wxSizeEvent ev(new_size);
        
//        wxEvtHandler *evh = dynamic_cast<wxEvtHandler*>(cc1);
        
//        evh->AddPendingEvent(ev);
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
        
//        g_bSleep = true;
        
        return 98;
    }
}

extern "C"{
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onStart(JNIEnv *env, jobject obj)
    {
        qDebug() << "onStart";
        
//        g_bSleep = false;;
        
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
    QAndroidJniObject data = activity.callObjectMethod(method, "(Ljava/lang/String;)Ljava/lang/String;", p);
    
    jstring s = data.object<jstring>();
    
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
    
    return return_string;
    
}


bool androidGetMemoryStatus( int *mem_total, int *mem_used )
{
    
    if(g_start_time.GetTicks() > 1435723200 )
        exit(0);
    
    //  On android, We arbitrarilly declare that we have used 50% of available memory.
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
            
    }
    
    double ldpi = 160. * density;
    double maxDim = wxMax(::wxGetDisplaySize().x, ::wxGetDisplaySize().y);
    ret = (maxDim / ldpi) * 25.4;
 
    msg.Printf(_T("Android Auto Display Size (mm, est.): %g"), ret);
    wxLogMessage(msg);
    
    
//     wxString istr = return_string.BeforeFirst('.');
//     
//     long ldpi;
//     if( istr.ToLong(&ldpi)){
//         ret = (::wxGetDisplaySize().x/(double)ldpi) * 25.4;
//     }

    return ret;
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

    wxSize sz_wx = ::wxGetDisplaySize();               // default, probably reasonable, but maybe not accurate
    qDebug() << sz_wx.x << sz_wx.y << sz_ret.x << sz_ret.y;
    
    return sz_ret;
    
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


bool LoadQtStyleSheet(wxString &sheet_file)
{
    if(wxFileExists( sheet_file )){
        //        QApplication qApp = getqApp();
        if(qApp){
            QString file(sheet_file.c_str());
            QFile File(file);
            File.open(QFile::ReadOnly);
            g_qtStyleSheet = QLatin1String(File.readAll());
            
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

QString getQtStyleSheet( void )
{
    return g_qtStyleSheet;
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

#if 0
wxString MyFrame::BuildAndroidSettingsString( void )
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
        result += _T("prefb_preservescale:") + wxString(g_bPreserveScaleOnX == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_smoothzp:") + wxString(g_bsmoothpanzoom == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_showgrid:") + wxString(g_bDisplayGrid == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_showoutlines:") + wxString(g_bShowOutlines == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_showdepthunits:") + wxString(g_bShowDepthUnits == 1 ? _T("1;") : _T("0;"));
        result += _T("prefb_showskewnu:") + wxString(g_bskew_comp == 1 ? _T("1;") : _T("0;"));

    // Some other assorted values
        result += _T("prefs_navmode:") + wxString(g_bCourseUp == 0 ? _T("North Up;") : _T("Course Up;"));



    return result;

}

bool MyFrame::DoAndroidPreferences( void )
{
    wxString settings = BuildAndroidSettingsString();

    if( InvokeJNIPreferences(settings)){
    //  Start a timer to poll for results of Android native references dialog
        m_PrefTimer.Start(500);
    }

    return true;
}


void MyFrame::OnPreferencesResultTimer( wxTimerEvent &event)
{
    //  Polling the native activity to see when the Preferences activity is done

    //  Get a reference to the running native activity
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                              "activity", "()Landroid/app/Activity;");

    if ( !activity.isValid() ){
        qDebug() << "OnPreferencesResultTimer : Activity is not valid";
        return;
    }

    //  Call the desired method
    QAndroidJniObject data = activity.callObjectMethod("checkAndroidSettings", "()Ljava/lang/String;");

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


    //  If the string has no contents, the settings activity has not completed
    if(!return_string.Length())
        return;


    //  Activity finished, so stop polling
    m_PrefTimer.Stop();


    bool bPrevQuilt = g_bQuiltEnable;

    //    Capture the name and index of the currently open chart
    wxString chart_file_name;
    if( cc1->GetQuiltMode() ) {
        int dbi = cc1->GetQuiltRefChartdbIndex();
        chart_file_name = ChartData->GetDBChartFileName( dbi );
    } else
        if( Current_Ch )
            chart_file_name = Current_Ch->GetFullPath();


    //  Parse the resulting settings string
    wxLogMessage( return_string );

    wxStringTokenizer tk(return_string, _T(";"));
    while ( tk.HasMoreTokens() )
    {
        wxString token = tk.GetNextToken();
        wxLogMessage(token);

        wxString val = token.AfterFirst(':');
        wxLogMessage(val);

        if(token.StartsWith( _T("prefb_lookahead"))){
            g_bLookAhead = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_quilt"))){
            g_bQuiltEnable = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_preservescale"))){
            g_bPreserveScaleOnX = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_smoothzp"))){
            g_bsmoothpanzoom = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showgrid"))){
            g_bDisplayGrid = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showoutlines"))){
            g_bShowOutlines = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showdepthunits"))){
            g_bShowDepthUnits = val.IsSameAs(_T("1"));
        }
        else if(token.StartsWith( _T("prefb_showskewnu"))){
            g_bskew_comp = val.IsSameAs(_T("1"));
        }
       else if(token.StartsWith( _T("prefs_navmode"))){
            g_bCourseUp = val.IsSameAs(_T("Course Up"));
        }

    }

     // And apply the changes
    pConfig->UpdateSettings();

//    if( bPrevQuilt != g_bQuiltEnable )
    {
        cc1->SetQuiltMode( g_bQuiltEnable );
        SetupQuiltMode();
    }

    if( g_bCourseUp ) {
        //    Stuff the COGAvg table in case COGUp is selected
        double stuff = 0.;
        if( !wxIsNaN(gCog) ) stuff = gCog;
        if( g_COGAvgSec > 0 ) {
            for( int i = 0; i < g_COGAvgSec; i++ )
                COGTable[i] = stuff;
        }

        g_COGAvg = stuff;

        DoCOGSet();
    }

    //    Stuff the Filter tables
    double stuffcog = 0.;
    double stuffsog = 0.;
    if( !wxIsNaN(gCog) ) stuffcog = gCog;
    if( !wxIsNaN(gSog) ) stuffsog = gSog;

    for( int i = 0; i < MAX_COGSOG_FILTER_SECONDS; i++ ) {
        COGFilterTable[i] = stuffcog;
        SOGFilterTable[i] = stuffsog;
    }
    m_COGFilterLast = stuffcog;

    SetChartUpdatePeriod( cc1->GetVP() );              // Pick up changes to skew compensator

    //    Capture the index of the currently open chart, after any database update
    int dbii = ChartData->FinddbIndex( chart_file_name );

    //    Reload all charts
    ChartsRefresh( dbii, cc1->GetVP(), true );

    if(stats){
        stats->Show(g_bShowChartBar);
        if(g_bShowChartBar){
            stats->Move(0,0);
            stats->RePosition();
 //           gFrame->Raise();
            DoChartUpdate();
            UpdateControlBar();
            Refresh();
        }
    }


}

#endif

#if 0
bool MyFrame::InvokeJNIPreferences( wxString &initial_settings)
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
#endif


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


