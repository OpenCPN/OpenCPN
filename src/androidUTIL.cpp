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


JavaVM *java_vm;
JNIEnv* jenv;
bool     b_androidBusyShown;


extern MyFrame                  *gFrame;
extern const wxEventType wxEVT_OCPN_DATASTREAM;
wxEvtHandler                    *s_pAndroidNMEAMessageConsumer;


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
 
        char tstr[100];
        strncpy(tstr, string, 99);
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
    JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onConfigChange(JNIEnv *env, jobject obj)
    {
        qDebug() << "onConfigChange";
        GetAndroidDisplaySize();
        
        wxSize new_size = getAndroidDisplayDimensions();
        qDebug() << "onConfigChange" << new_size.x << new_size.y;
        
        gFrame->TriggerResize(new_size);
        
//        wxSizeEvent ev(new_size);
        
//        wxEvtHandler *evh = dynamic_cast<wxEvtHandler*>(cc1);
        
//        evh->AddPendingEvent(ev);
        return 77;
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


bool androidGetMemoryStatus( int *mem_total, int *mem_used )
{
    
    if(mem_total)
        *mem_total = 100 * 1024;
    if(mem_used)
        *mem_used = 50 * 1024;
    
#if 0
        
        
    unsigned long android_processID = wxGetProcessId();
        
        QAndroidJniObject data = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/bindings/QtActivity",
        "callFromCpp",
        "(I)Ljava/lang/String;",
        (int) android_processID);
        
        //   jint x = data.object<jint>();
        //   int x = reinterpret_cast<int>(data.object<int>());
        jstring f = data.object<jstring>();
        
        
        //     jint x = QAndroidJniObject::callStaticObjectMethod<jint>("org/qtproject/qt5/appActivity",
        //                                                                        "callFromCpp");
        
        if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
            qDebug() << "GetEnv failed.";
            return -1;
}

int mu = 0;
const char *ret_val = (jenv)->GetStringUTFChars(f, NULL);
if (ret_val != NULL) {
    
    qDebug() << "Mem" << ret_val;
    
    mu = atoi(ret_val);
    
    (jenv)->ReleaseStringUTFChars(f, ret_val);
}

if(mem_total)
    *mem_total = 100 * 1024;
if(mem_used)
    *mem_used = mu;

//    if(mem_used)
    //        qDebug() << "Mem Status" << (*mem_used) / 1024  ;
    #endif

    
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
    
    int mu = 100;
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv( (void **) &jenv, JNI_VERSION_1_6) != JNI_OK) {
        qDebug() << "GetEnv failed.";
    }
    else {
        const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
        mu = atoi(ret_string);
        
//        return_string = wxString(ret_string, wxConvUTF8);
    }
    
    if(mem_used)
        *mem_used = mu;

//    if(mem_used)
//        qDebug() << "Mem Status" << (*mem_used) / 1024  ;
        
    return true;
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
    
    
    wxString istr = return_string.BeforeFirst('.');
    
    long ldpi;
    if( istr.ToLong(&ldpi)){
        ret = (::wxGetDisplaySize().x/(double)ldpi) * 25.4;
    }

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
    
     wxStringTokenizer tk(return_string, _T(";"));
    if( tk.HasMoreTokens() ){
        wxString token = tk.GetNextToken();     // xdpi
        token = tk.GetNextToken();
        long a = ::wxGetDisplaySize().x;        // default is wxWidgets idea
        if(token.ToLong( &a ))
            sz_ret.x = a;
        
        token = tk.GetNextToken();
        long b = ::wxGetDisplaySize().y;        
        if(token.ToLong( &b ))
            sz_ret.y = b;
    }
    
    return sz_ret;
    
}

void androidShowBusyIcon()
{
    if(b_androidBusyShown)
        return;
    qDebug() << "Showit";
    
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
    
    qDebug() << "Hideit";
    
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
            QString StyleSheet = QLatin1String(File.readAll());
            
            qApp->setStyleSheet(StyleSheet);
            
            return true;
        }
        else
            return false;
    }
    else
        return false;
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


