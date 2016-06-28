/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Platform specific support utilities
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

#include <wx/app.h>
#include <wx/apptrait.h>
#include "wx/stdpaths.h"
#include <wx/filename.h>

#include "dychart.h"
#include "OCPNPlatform.h"

#include "chart1.h"
#include "cutil.h"
#include "styles.h"
#include "navutil.h"
#include "ConnectionParams.h"
#include "FontMgr.h"
#include "s52s57.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

// Include CrashRpt Header
#ifdef OCPN_USE_CRASHRPT
#include "CrashRpt.h"
#endif
#ifdef __MSVC__
#include <new.h>
#endif

#ifdef LINUX_CRASHRPT
#include "crashprint.h"
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>
#endif

#ifdef __WXMSW__
#include <windows.h>
#include <winioctl.h>
#include <initguid.h>
#include "setupapi.h"                   // presently stored in opencpn/src
#endif

#ifdef __WXOSX__
#include "macutils.h"
#endif

DECLARE_APP(MyApp)

void appendOSDirSlash( wxString* pString );


#ifndef __WXMSW__
struct sigaction          sa_all;
struct sigaction          sa_all_old;
extern sigjmp_buf env;                    // the context saved by sigsetjmp();
#endif


extern OCPNPlatform              *g_Platform;

extern int                       quitflag;
extern MyFrame                   *gFrame;
extern bool                      g_bportable;
extern wxString                  OpenCPNVersion;

extern MyConfig                  *pConfig;

extern ocpnStyle::StyleManager* g_StyleManager;

extern bool                      g_bshowToolbar;
extern bool                      g_bBasicMenus;
extern bool                      g_bUIexpert;

extern bool                      g_bshowToolbar;
extern bool                      g_bBasicMenus;

extern bool                      g_bShowOutlines;
extern bool                      g_bShowDepthUnits;
extern bool                      g_bDisplayGrid;  // Flag indicating weather the lat/lon grid should be displayed
extern bool                      g_bShowChartBar;
extern bool                      g_bShowActiveRouteHighway;
extern int                       g_nNMEADebug;
extern int                       g_nAWDefault;
extern int                       g_nAWMax;
extern bool                      g_bPlayShipsBells;
extern bool                      g_bFullscreenToolbar;
extern bool                      g_bShowLayers;
extern bool                      g_bTransparentToolbar;
extern bool                      g_bPermanentMOBIcon;
extern bool                      g_bTempShowMenuBar;

extern int                       g_iSDMMFormat;
extern int                       g_iDistanceFormat;
extern int                       g_iSpeedFormat;

extern int                       g_iNavAidRadarRingsNumberVisible;
extern float                     g_fNavAidRadarRingsStep;
extern int                       g_pNavAidRadarRingsStepUnits;
extern int                       g_iWaypointRangeRingsNumber;
extern float                     g_fWaypointRangeRingsStep;
extern int                       g_iWaypointRangeRingsStepUnits;
extern wxColour                  g_colourWaypointRangeRingsColour;
extern bool                      g_bWayPointPreventDragging;
extern bool                      g_bConfirmObjectDelete;

// AIS Global configuration
extern bool                      g_bShowAIS;
extern bool                      g_bCPAMax;
extern double                    g_CPAMax_NM;
extern bool                      g_bCPAWarn;
extern double                    g_CPAWarn_NM;
extern bool                      g_bTCPA_Max;
extern double                    g_TCPA_Max;
extern bool                      g_bMarkLost;
extern double                    g_MarkLost_Mins;
extern bool                      g_bRemoveLost;
extern double                    g_RemoveLost_Mins;
extern bool                      g_bShowCOG;
extern double                    g_ShowCOG_Mins;
extern bool                      g_bAISShowTracks;
extern double                    g_AISShowTracks_Mins;
extern bool                      g_bHideMoored;
extern double                    g_ShowMoored_Kts;
extern wxString                  g_sAIS_Alert_Sound_File;
extern bool                      g_bAIS_CPA_Alert_Suppress_Moored;
extern bool                      g_bAIS_ACK_Timeout;
extern double                    g_AckTimeout_Mins;
extern bool                      g_bShowAreaNotices;
extern bool                      g_bDrawAISSize;
extern bool                      g_bShowAISName;
extern int                       g_Show_Target_Name_Scale;
extern bool                      g_bWplIsAprsPosition;

extern int                       gps_watchdog_timeout_ticks;
extern int                       sat_watchdog_timeout_ticks;

extern int                       gGPS_Watchdog;
extern bool                      bGPSValid;

extern int                       gHDx_Watchdog;
extern int                       gHDT_Watchdog;
extern int                       gVAR_Watchdog;
extern bool                      g_bHDT_Rx;
extern bool                      g_bVAR_Rx;

extern int                       gSAT_Watchdog;
extern int                       g_SatsInView;
extern bool                      g_bSatValid;

extern bool                      g_bDebugCM93;
extern bool                      g_bDebugS57;

extern bool                      g_bfilter_cogsog;
extern int                       g_COGFilterSec;
extern int                       g_SOGFilterSec;

extern int                       g_ChartUpdatePeriod;
extern int                       g_SkewCompUpdatePeriod;

extern int                       g_lastClientRectx;
extern int                       g_lastClientRecty;
extern int                       g_lastClientRectw;
extern int                       g_lastClientRecth;
extern double                    g_display_size_mm;
extern double                    g_config_display_size_mm;
extern bool                      g_config_display_size_manual;

extern bool                     g_bTrackDaily;
extern double                   g_PlanSpeed;
extern bool                     g_bFullScreenQuilt;
extern bool                     g_bQuiltEnable;
extern bool                     g_bskew_comp;

extern bool                     g_bopengl;
extern bool                     g_btouch;
extern bool                     g_bresponsive;
extern bool                     g_bShowStatusBar;
extern int                      g_cm93_zoom_factor;
extern int                      g_GUIScaleFactor;
extern wxArrayOfConnPrm         *g_pConnectionParams;
extern bool                     g_fog_overzoom;
extern double                   g_overzoom_emphasis_base;
extern bool                     g_oz_vector_scale;
extern int                      g_nTrackPrecision;
extern wxString                 g_toolbarConfig;
extern bool                     g_bPreserveScaleOnX;

#ifdef ocpnUSE_GL
extern ocpnGLOptions            g_GLOptions;
#endif
extern int                      g_default_font_size;

wxLog       *g_logger;
bool         g_bEmailCrashReport;
extern int                       g_ais_alert_dialog_x, g_ais_alert_dialog_y;
extern int                       g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
extern wxLocale                  *plocale_def_lang;
#endif





//  OCPN Platform implementation

OCPNPlatform::OCPNPlatform()
{
}

OCPNPlatform::~OCPNPlatform()
{
}

//--------------------------------------------------------------------------
//      Per-Platform Initialization support
//--------------------------------------------------------------------------
#ifdef __WXMSW__
int MyNewHandler( size_t size )
{
    //  Pass to wxWidgets Main Loop handler
    throw std::bad_alloc();
    
    return 0;
}
#endif

//-----------------------------------------------------------------------
//      Signal Handlers
//-----------------------------------------------------------------------
#ifndef __WXMSW__

//These are the signals possibly expected
//      SIGUSR1
//      Raised externally to cause orderly termination of application
//      Intended to act just like pushing the "EXIT" button

//      SIGSEGV
//      Some undefined segfault......

void
catch_signals(int signo)
{
    switch(signo)
    {
        case SIGUSR1:
            quitflag++;                             // signal to the timer loop
            break;
            
        case SIGSEGV:
            siglongjmp(env, 1);// jump back to the setjmp() point
            break;
            
        case SIGTERM:
            gFrame->Close();
            break;
            
        default:
            break;
    }
    
}
#endif

#ifdef OCPN_USE_CRASHRPT
// Define the crash callback
int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
{
    //  Flush log file
    if( g_logger)
        g_logger->Flush();
    
    return CR_CB_DODEFAULT;
}
#endif



//  Called from MyApp() immediately upon entry to MyApp::OnInit()
void OCPNPlatform::Initialize_1( void )
{
    
#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
    // Install Windows crash reporting
    
    CR_INSTALL_INFO info;
    memset(&info, 0, sizeof(CR_INSTALL_INFO));
    info.cb = sizeof(CR_INSTALL_INFO);
    info.pszAppName = _T("OpenCPN");
    
    info.pszAppVersion = OpenCPNVersion.c_str();
    
    int type = MiniDumpWithDataSegs;  // Include the data sections from all loaded modules.
    // This results in the inclusion of global variables
    
    type |=  MiniDumpNormal;// | MiniDumpWithPrivateReadWriteMemory | MiniDumpWithIndirectlyReferencedMemory;
    info.uMiniDumpType = (MINIDUMP_TYPE)type;
    
    // Install all available exception handlers....
    info.dwFlags = CR_INST_ALL_POSSIBLE_HANDLERS;
    
    //  Except memory allocation failures
    info.dwFlags &= ~CR_INST_NEW_OPERATOR_ERROR_HANDLER;
    
    //  Allow user to attach files
    info.dwFlags |= CR_INST_ALLOW_ATTACH_MORE_FILES;
    
    //  Allow user to add more info
    info.dwFlags |= CR_INST_SHOW_ADDITIONAL_INFO_FIELDS;
    
    
    // URL for sending error reports over HTTP.
    if(1/*g_bEmailCrashReport*/){
        info.pszEmailTo = _T("opencpn@bigdumboat.com");
        info.pszSmtpProxy = _T("mail.bigdumboat.com:587");
        info.pszUrl = _T("http://bigdumboat.com/crashrpt/ocpn_crashrpt.php");
        info.uPriorities[CR_HTTP] = 1;  // First try send report over HTTP
    }
    else{
        info.dwFlags |= CR_INST_DONT_SEND_REPORT;
        info.uPriorities[CR_HTTP] = CR_NEGATIVE_PRIORITY;       // don't send at all
    }
    
    info.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY;  // Second try send report over SMTP
    info.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY; //1; // Third try send report over Simple MAPI
    
    wxStandardPaths& crash_std_path = g_Platform->GetStdPaths();
    
    wxString crash_rpt_save_locn = crash_std_path.GetConfigDir();
    if( g_bportable ) {
        wxFileName exec_path_crash( crash_std_path.GetExecutablePath() );
        crash_rpt_save_locn = exec_path_crash.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
    }
    
    wxString locn = crash_rpt_save_locn + _T("\\CrashReports");
    
    if(!wxDirExists( locn ) )
        wxMkdir( locn );
    
    if(wxDirExists( locn ) ){
        wxCharBuffer buf = locn.ToUTF8();
        wchar_t wlocn[256];
        if(buf && (locn.Length() < sizeof(wlocn)) ){
            MultiByteToWideChar( 0, 0, buf.data(), -1, wlocn, sizeof(wlocn)-1);
            info.pszErrorReportSaveDir = (LPCWSTR)wlocn;
        }
    }
    
    // Provide privacy policy URL
    wxFileName exec_path_crash( crash_std_path.GetExecutablePath() );
    wxString policy_file =  exec_path_crash.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
    policy_file += _T("PrivacyPolicy.txt");
    policy_file.Prepend(_T("file:"));
    
    info.pszPrivacyPolicyURL = policy_file.c_str();;
    
    int nResult = crInstall(&info);
    if(nResult!=0) {
        TCHAR buff[256];
        crGetLastErrorMsg(buff, 256);
        MessageBox(NULL, buff, _T("crInstall error, Crash Reporting disabled."), MB_OK);
    }
    
    // Establish the crash callback function
    crSetCrashCallback( CrashCallback, NULL );
    
    // Take screenshot of the app window at the moment of crash
    crAddScreenshot2(CR_AS_PROCESS_WINDOWS|CR_AS_USE_JPEG_FORMAT, 95);
    
    //  Mark some files to add to the crash report
    wxString home_data_crash = crash_std_path.GetConfigDir();
    if( g_bportable ) {
        wxFileName f( crash_std_path.GetExecutablePath() );
        home_data_crash = f.GetPath();
    }
    appendOSDirSlash( &home_data_crash );
    
    wxString config_crash = _T("opencpn.ini");
    config_crash.Prepend( home_data_crash );
    crAddFile2( config_crash.c_str(), NULL, NULL, CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE );
    
    wxString log_crash = _T("opencpn.log");
    log_crash.Prepend( home_data_crash );
    crAddFile2( log_crash.c_str(), NULL, NULL, CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE );
    
#endif
#endif

    
#ifdef LINUX_CRASHRPT
#if wxUSE_ON_FATAL_EXCEPTION
    // fatal exceptions handling
    wxHandleFatalExceptions (true);
#endif
#endif

#ifdef __MSVC__
    //  Invoke my own handler for failures of malloc/new
    _set_new_handler( MyNewHandler );
    //  configure malloc to call the New failure handler on failure
    _set_new_mode(1);
#endif    
    
    //    On MSW, force the entire process to run on one CPU core only
    //    This resolves some difficulty with wxThread syncronization
#if 0
#ifdef __WXMSW__
    //Gets the current process handle
    HANDLE hProc = GetCurrentProcess();
    DWORD procMask;
    DWORD sysMask;
    HANDLE hDup;
    DuplicateHandle( hProc, hProc, hProc, &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS );
    
    //Gets the current process affinity mask
    GetProcessAffinityMask( hDup, &procMask, &sysMask );
    
    // Take a simple approach, and assume up to 4 processors
    DWORD newMask;
    if( ( procMask & 1 ) == 1 ) newMask = 1;
    else
        if( ( procMask & 2 ) == 2 ) newMask = 2;
        else
            if( ( procMask & 4 ) == 4 ) newMask = 4;
            else
                if( ( procMask & 8 ) == 8 ) newMask = 8;
                
                //Set te affinity mask for the process
                BOOL res = SetProcessAffinityMask( hDup, (DWORD_PTR) newMask );
            
            if( res == 0 ) {
                //Error setting affinity mask!!
            }
#endif
#endif
            
#ifdef __MSVC__
            
            //    Handle any Floating Point Exceptions which may leak thru from other
            //    processes.  The exception filter is in cutil.c
            //    Seems to only happen for W98
            
            wxPlatformInfo Platform;
            if( Platform.GetOperatingSystemId() == wxOS_WINDOWS_9X ) SetUnhandledExceptionFilter (&MyUnhandledExceptionFilter);
#endif
            
#ifdef __WXMSW__
            //     _CrtSetBreakAlloc(25503);
#endif
            

#ifndef __WXMSW__
            //      Setup Linux SIGNAL handling, for external program control
            
            //      Build the sigaction structure
            sa_all.sa_handler = catch_signals;// point to my handler
            sigemptyset(&sa_all.sa_mask);// make the blocking set
            // empty, so that all
            // other signals will be
            // unblocked during my handler
            sa_all.sa_flags = 0;
            
            sigaction(SIGUSR1, NULL, &sa_all_old);// save existing action for this signal
            
            //      Register my request for some signals
            sigaction(SIGUSR1, &sa_all, NULL);
            
            sigaction(SIGUSR1, NULL, &sa_all_old);// inspect existing action for this signal
            
            sigaction(SIGTERM, &sa_all, NULL);
            sigaction(SIGTERM, NULL, &sa_all_old);
#endif

#ifdef __OCPN__ANDROID__
    androidUtilInit( );
#endif            
            
}

//  Called from MyApp() immediately before creation of MyFrame()
//  Config is known to be loaded and stable
//  Log is available
void OCPNPlatform::Initialize_2( void )
{
#ifdef __OCPN__ANDROID__
    wxLogMessage(androidGetDeviceInfo());
#endif    
}

//  Called from MyApp() just before end of MyApp::OnInit()
void OCPNPlatform::Initialize_3( void ){
}

void OCPNPlatform::OnExit_1( void ){
}
    
void OCPNPlatform::OnExit_2( void ){
    
#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
        // Uninstall Windows crash reporting
    crUninstall();
#endif
#endif
    
}


//      Setup default global options when config file is unavailable,
//      as on initial startup after new install
//      The global config object (pConfig) is available, so direct updates are also allowed

void OCPNPlatform::SetDefaultOptions( void )
{
    //  General options, applied to all platforms
    g_bShowOutlines = true;
    
    g_CPAMax_NM = 20.;
    g_CPAWarn_NM = 2.;
    g_TCPA_Max = 30.;
    g_bMarkLost = true;
    g_MarkLost_Mins = 8;
    g_bRemoveLost = true;
    g_RemoveLost_Mins = 10;
    g_bShowCOG = true;
    g_ShowCOG_Mins = 6;
    g_bHideMoored = false;
    g_ShowMoored_Kts = 0.2;
    g_bTrackDaily = false;
    g_PlanSpeed = 6.;
    g_bFullScreenQuilt = true;
    g_bQuiltEnable = true;
    g_bskew_comp = false;
    g_bShowAreaNotices = false;
    g_bDrawAISSize = false;
    g_bShowAISName = false;
    g_nTrackPrecision = 2;
    g_bPreserveScaleOnX = true;
    
    // Initial S52/S57 options
    if(pConfig){
        pConfig->SetPath( _T ( "/Settings/GlobalState" ) );
        pConfig->Write( _T ( "bShowS57Text" ), false );
        pConfig->Write( _T ( "bShowS57ImportantTextOnly" ), false );
        pConfig->Write( _T ( "nDisplayCategory" ), (int)(_DisCat)STANDARD );
        pConfig->Write( _T ( "nSymbolStyle" ), (int)(_LUPname)PAPER_CHART );
        pConfig->Write( _T ( "nBoundaryStyle" ), (int)(_LUPname)PLAIN_BOUNDARIES );
        
        pConfig->Write( _T ( "bShowSoundg" ), false );
        pConfig->Write( _T ( "bShowMeta" ), false );
        pConfig->Write( _T ( "bUseSCAMIN" ), true );
        pConfig->Write( _T ( "bShowAtonText" ), false );
        pConfig->Write( _T ( "bShowLightDescription" ), false );
        pConfig->Write( _T ( "bExtendLightSectors" ), true );
        pConfig->Write( _T ( "bDeClutterText" ), true );
        pConfig->Write( _T ( "bShowNationalText" ), false );
        
        pConfig->Write( _T ( "S52_MAR_SAFETY_CONTOUR" ), 5 );
        pConfig->Write( _T ( "S52_MAR_SHALLOW_CONTOUR" ), 2 );
        pConfig->Write( _T ( "S52_MAR_DEEP_CONTOUR" ), 10 );
        pConfig->Write( _T ( "S52_MAR_TWO_SHADES" ), 0  );
        pConfig->Write( _T ( "S52_DEPTH_UNIT_SHOW" ), 1 );
     }
    
    
#ifdef __WXMSW__
    //  Enable some default PlugIns, and their default options
    if(pConfig){
        pConfig->SetPath( _T ( "/PlugIns/chartdldr_pi.dll" ) );
        pConfig->Write( _T ( "bEnabled" ), true );

        pConfig->SetPath( _T ( "/PlugIns/wmm_pi.dll" ) );
        pConfig->Write( _T ( "bEnabled" ), true );
        
        pConfig->SetPath ( _T ( "/Settings/WMM" ) );
        pConfig->Write ( _T ( "ShowIcon" ), true );
        pConfig->Write ( _T ( "ShowLiveIcon" ), true );
        
    }
#endif

#ifdef __WXOSX__
//  Enable some default PlugIns, and their default options
    if(pConfig){
        pConfig->SetPath( _T ( "/PlugIns/libchartdldr_pi.dylib" ) );
        pConfig->Write( _T ( "bEnabled" ), true );
        
        pConfig->SetPath( _T ( "/PlugIns/libwmm_pi.dylib" ) );
        pConfig->Write( _T ( "bEnabled" ), true );
        
        pConfig->SetPath ( _T ( "/Settings/WMM" ) );
        pConfig->Write ( _T ( "ShowIcon" ), true );
        pConfig->Write ( _T ( "ShowLiveIcon" ), true );
        
    }
#endif

#ifdef __LINUX__
//  Enable some default PlugIns, and their default options
    if(pConfig){
        pConfig->SetPath( _T ( "/PlugIns/libchartdldr_pi.so" ) );
        pConfig->Write( _T ( "bEnabled" ), true );
        
        pConfig->SetPath( _T ( "/PlugIns/libwmm_pi.so" ) );
        pConfig->Write( _T ( "bEnabled" ), true );
        
        pConfig->SetPath ( _T ( "/Settings/WMM" ) );
        pConfig->Write ( _T ( "ShowIcon" ), true );
        pConfig->Write ( _T ( "ShowLiveIcon" ), true );
        
    }
#endif

        
#ifdef __OCPN__ANDROID__
    
#ifdef ocpnUSE_GL
    g_bopengl = true;
    g_GLOptions.m_bTextureCompression = 1;
    g_GLOptions.m_bTextureCompressionCaching = 1;
#endif
    
    g_btouch = true;
    g_bresponsive = true;
    g_default_font_size = 18;            //  This is pretty close to TextAppearance.Medium
    g_bUIexpert = true;         
    
    g_bShowStatusBar = true;
    g_cm93_zoom_factor = -5;
    g_oz_vector_scale = false;
    g_fog_overzoom = false;
    
    g_GUIScaleFactor = 0;               // nominal
    
    //  Suppress most tools, especially those that appear in the Basic menus.
    //  Of course, they may be re-enabled by experts...
    g_toolbarConfig = _T("......X........X..XXXXXXXXXXX");
    g_bPermanentMOBIcon = false;
    
    wxString sGPS = _T("2;3;;0;0;;0;1;0;0;;0;;1;0;0;0;0");          // 17 parms
    ConnectionParams *new_params = new ConnectionParams(sGPS);
    
    new_params->bEnabled = true;
    g_pConnectionParams->Add(new_params);
    
    //  Enable some default PlugIns, and their default options
    
    if(pConfig){
        pConfig->SetPath( _T ( "/PlugIns/libchartdldr_pi.so" ) );
        pConfig->Write( _T ( "bEnabled" ), true );
        
        pConfig->SetPath( _T ( "/PlugIns/libwmm_pi.so" ) );
        pConfig->Write( _T ( "bEnabled" ), true );
        
        pConfig->SetPath ( _T ( "/Settings/WMM" ) );
        pConfig->Write ( _T ( "ShowIcon" ), false );
   
        pConfig->SetPath( _T ( "/PlugIns/libgrib_pi.so" ) );
        pConfig->Write( _T ( "bEnabled" ), true );
        
        pConfig->SetPath( _T ( "/PlugIns/GRIB" ) );
        pConfig->Write ( _T ( "GRIBCtrlBarPosX" ), 0 );
        pConfig->Write ( _T ( "GRIBCtrlBarPosY" ), 90 );
        
        pConfig->SetPath ( _T ( "/Settings/GRIB" ) );
        pConfig->Write ( _T ( "CursorDataShown" ), 0 );
        
        
        pConfig->SetPath ( _T ( "/Settings/QTFonts" ) );

        //Status Bar
        wxString str = _T("en_US-b25a3899");
        wxString pval = _T("StatusBar:Roboto,26,-1,5,75,0,0,0,0,0:rgb(0, 0, 0)");
        
        pConfig->Write (str, pval );
        FontMgr::Get().LoadFontNative( &str, &pval );
    }
        
        
    
#endif
    
    
    
}


void OCPNPlatform::applyExpertMode(bool mode)
{
#ifdef __OCPN__ANDROID__
    g_bshowToolbar = mode;               // no toolbar unless in exprt mode
    g_bBasicMenus = !mode;              //  simplified context menus in basic mode
#endif

}
        
    
wxString OCPNPlatform::GetSupplementalLicenseString()
{
    wxString lic;
#ifdef __OCPN__ANDROID__
    lic = androidGetSupplementalLicense();
#endif    
    return lic;
}
    
//--------------------------------------------------------------------------
//      Per-Platform file/directory support
//--------------------------------------------------------------------------

wxStandardPaths& OCPNPlatform::GetStdPaths()
{
#ifndef __OCPN__ANDROID__    
    return *dynamic_cast<wxStandardPaths*>(&(wxGetApp().GetTraits()->GetStandardPaths()));
#else 
//    return *dynamic_cast<wxStandardPaths*>(&wxApp::GetTraits()->GetStandardPaths());
    return *dynamic_cast<wxStandardPaths*>(&(wxTheApp->GetTraits())->GetStandardPaths());
#endif    
    
}


wxString &OCPNPlatform::GetHomeDir()
{
    if(m_homeDir.IsEmpty()){

        //      Establish a "home" location
 //       wxStandardPaths& std_path = *dynamic_cast<wxStandardPaths*>(&wxApp::GetTraits()->GetStandardPaths());
        wxStandardPaths& std_path = GetStdPaths();
        //        wxStandardPaths &std_path = ( wxStandardPaths) wxGetApp().GetTraits()->GetStandardPaths();
        
        //TODO  Why is the following preferred?  Will not compile with gcc...
//    wxStandardPaths& std_path = wxApp::GetTraits()->GetStandardPaths();

#ifdef __unix__
        std_path.SetInstallPrefix(wxString(PREFIX, wxConvUTF8));
#endif

#ifdef __WXMSW__
        m_homeDir =  std_path.GetConfigDir();   // on w98, produces "/windows/Application Data"
#else
        m_homeDir = std_path.GetUserConfigDir();
#endif

#ifdef __OCPN__ANDROID__
        m_homeDir =  androidGetHomeDir();
#endif

		if( g_bportable ) {
			wxFileName path(GetExePath());
			m_homeDir = path.GetPath();
		}
        
#ifdef  __WXOSX__
        appendOSDirSlash(&m_homeDir);
        m_homeDir.Append(_T("opencpn"));
#endif            

        appendOSDirSlash( &m_homeDir );
    }
    
    return m_homeDir;
}

wxString &OCPNPlatform::GetExePath()
{
    if(m_exePath.IsEmpty()){
        
        wxStandardPaths& std_path = GetStdPaths();
        m_exePath = std_path.GetExecutablePath();
    }
    
    return m_exePath;
}


wxString OCPNPlatform::GetWritableDocumentsDir()
{
    wxString dir;
    
#ifdef __OCPN__ANDROID__
    dir = androidGetExtStorageDir();                 // Used for Chart storage, typically
#else
    wxStandardPaths& std_path = GetStdPaths();
    dir = std_path.GetDocumentsDir();
#endif    
    return dir;
}


wxString &OCPNPlatform::GetSharedDataDir()
{
    if(m_SData_Dir.IsEmpty()){
        //      Establish a "shared data" location
        /*  From the wxWidgets documentation...
         * 
         *     wxStandardPaths::GetDataDir
         *     wxString GetDataDir() const
         *     Return the location of the applications global, i.e. not user-specific, data files.
         * Unix: prefix/share/appname
         * Windows: the directory where the executable file is located
         * Mac: appname.app/Contents/SharedSupport bundle subdirectory
         */
        wxStandardPaths& std_path = GetStdPaths();
        m_SData_Dir = std_path.GetDataDir();
        appendOSDirSlash( &m_SData_Dir );
        
#ifdef __OCPN__ANDROID__
        m_SData_Dir = androidGetSharedDir();
#endif
        
        if( g_bportable )
            m_SData_Dir = GetHomeDir();
    }
    
    return m_SData_Dir;
    
}

wxString &OCPNPlatform::GetPrivateDataDir()
{
    if(m_PrivateDataDir.IsEmpty()){
        //      Establish the prefix of the location of user specific data files
        wxStandardPaths& std_path = GetStdPaths();
        
#ifdef __WXMSW__
        m_PrivateDataDir = GetHomeDir();                     // should be {Documents and Settings}\......
#elif defined __WXOSX__
        m_PrivateDataDir = std_path.GetUserConfigDir();     // should be ~/Library/Preferences
        appendOSDirSlash(&m_PrivateDataDir);
        m_PrivateDataDir.Append(_T("opencpn"));
#else
        m_PrivateDataDir = std_path.GetUserDataDir();       // should be ~/.opencpn
#endif
        
        if( g_bportable )
            m_PrivateDataDir = GetHomeDir();
        
#ifdef __OCPN__ANDROID__
        m_PrivateDataDir = androidGetPrivateDir();
#endif
    }
    
    return m_PrivateDataDir;
}


wxString &OCPNPlatform::GetPluginDir()
{
    if(m_PluginsDir.IsEmpty()){

        wxStandardPaths& std_path = GetStdPaths();
        
        //  Get the PlugIns directory location
        m_PluginsDir = std_path.GetPluginsDir();   // linux:   {prefix}/lib/opencpn
        // Mac:     appname.app/Contents/PlugIns
#ifdef __WXMSW__
        m_PluginsDir += _T("\\plugins");             // Windows: {exe dir}/plugins
#endif
        
        if( g_bportable ) {
            m_PluginsDir = GetHomeDir();
            m_PluginsDir += _T("plugins");
        }
        
#ifdef __OCPN__ANDROID__
        // something like: data/data/org.opencpn.opencpn
        wxFileName fdir = wxFileName::DirName(std_path.GetUserConfigDir());
        fdir.RemoveLastDir();
        m_PluginsDir = fdir.GetPath();
        
#endif        
        
        
    }
    
    return m_PluginsDir;
}


wxString &OCPNPlatform::GetConfigFileName()
{
    if(m_config_file_name.IsEmpty()){
        //      Establish the location of the config file
        wxStandardPaths& std_path = GetStdPaths();
        
#ifdef __WXMSW__
        m_config_file_name = _T("opencpn.ini");
        m_config_file_name.Prepend( GetHomeDir() );
        
#elif defined __WXOSX__
        m_config_file_name = std_path.GetUserConfigDir(); // should be ~/Library/Preferences
        appendOSDirSlash(&m_config_file_name);
        m_config_file_name.Append(_T("opencpn"));
        appendOSDirSlash(&m_config_file_name);
        m_config_file_name.Append(_T("opencpn.ini"));
#else
        m_config_file_name = std_path.GetUserDataDir(); // should be ~/.opencpn
        appendOSDirSlash(&m_config_file_name);
        m_config_file_name.Append(_T("opencpn.conf"));
#endif
        
        if( g_bportable ) {
            m_config_file_name = GetHomeDir();
#ifdef __WXMSW__
            m_config_file_name += _T("opencpn.ini");
#elif defined __WXOSX__
            m_config_file_name +=_T("opencpn.ini");
#else
            m_config_file_name += _T("opencpn.conf");
#endif
            
        }
        
#ifdef __OCPN__ANDROID__
        m_config_file_name = androidGetPrivateDir();
        appendOSDirSlash(&m_config_file_name);
        m_config_file_name += _T("opencpn.conf");
#endif
        
    }
    
    return m_config_file_name;
}

wxString *OCPNPlatform::GetPluginDirPtr()
{
    return &m_PluginsDir;
}
wxString *OCPNPlatform::GetSharedDataDirPtr()
{
    return &m_SData_Dir;
}
wxString *OCPNPlatform::GetPrivateDataDirPtr()
{
    return &m_PrivateDataDir;
}

int OCPNPlatform::DoFileSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir,
                          wxString suggestedName, wxString wildcard)
{
    wxString file;
    int result = wxID_CANCEL;

#ifdef __OCPN__ANDROID__
    //  Verify that initDir is traversable, fix it if not...
    wxString idir = initDir;
    if(initDir.StartsWith(_T("/data/data")))                 // not good, provokes a crash usually...
        idir = GetWritableDocumentsDir();
    
    result = androidFileChooser(&file, idir, Title, suggestedName, wildcard);
    if(file_spec)
        *file_spec = file;
#else
    long flag = wxFD_DEFAULT_STYLE;
    if(suggestedName.Length()){                 // new file
        flag = wxFD_SAVE;
    }
        
    wxString mask = wildcard;
    if( wxNOT_FOUND != mask.Find(_T("gpx")) )
        mask.Prepend( _T("GPX files (*.gpx)|") );
    
    wxFileDialog *psaveDialog = new wxFileDialog( parent, Title, initDir, suggestedName, mask, flag );

//    Try to reduce the dialog size, and scale fonts down, if necessary.
//     if(g_bresponsive && parent)
//         psaveDialog = g_Platform->AdjustFileDialogFont(parent, psaveDialog);
    
#ifdef __WXOSX__
    if(parent)
        parent->HideWithEffect(wxSHOW_EFFECT_BLEND );
#endif
            
     result = psaveDialog->ShowModal();
            
#ifdef __WXOSX__
    if(parent)
        parent->ShowWithEffect(wxSHOW_EFFECT_BLEND );
#endif

    if(file_spec)
        *file_spec = psaveDialog->GetPath();
    delete psaveDialog;
        
#endif
    
    return result;
}

int OCPNPlatform::DoDirSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir)
{
    wxString dir;
    int result = wxID_CANCEL;
    
#ifdef __OCPN__ANDROID__
    //  Verify that initDir is traversable, fix it if not...
    wxString idir = initDir;
    if(initDir.StartsWith(_T("/data/data")))                 // not good, provokes a crash usually...
        idir = GetWritableDocumentsDir();
    
    result = androidFileChooser(&dir, idir, Title, _T(""), _T(""), true);    // Directories only
    if(file_spec)
        *file_spec = dir;
#else
    wxDirDialog *dirSelector = new wxDirDialog( parent, Title, initDir, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    dirSelector->SetFont(*qFont);

//    Try to reduce the dialog size, and scale fonts down, if necessary.
//     if(g_bresponsive && parent)
//         dirSelector = AdjustDirDialogFont(parent, dirSelector);
    
#ifdef __WXOSX__
    if(parent)
        parent->HideWithEffect(wxSHOW_EFFECT_BLEND );
#endif
            
    result = dirSelector->ShowModal();
            
#ifdef __WXOSX__
    if(parent)
        parent->ShowWithEffect(wxSHOW_EFFECT_BLEND );
#endif

    if( result == wxID_CANCEL ){
    }
    else{
        if(file_spec){
            *file_spec = dirSelector->GetPath();
        }
    }
    
    delete dirSelector;
#endif
    
    return result;
}


bool OCPNPlatform::InitializeLogFile( void )
{
    //      Establish Log File location
    mlog_file = GetPrivateDataDir();
    appendOSDirSlash( &mlog_file );
    
#ifdef  __WXOSX__
    
    wxFileName LibPref(mlog_file);          // starts like "~/Library/Preferences/opencpn"
    LibPref.RemoveLastDir();// takes off "opencpn"
    LibPref.RemoveLastDir();// takes off "Preferences"
    
    mlog_file = LibPref.GetFullPath();
    appendOSDirSlash(&mlog_file);
    
    mlog_file.Append(_T("Logs/"));// so, on OS X, opencpn.log ends up in ~/Library/Logs
                                   // which makes it accessible to Applications/Utilities/Console....
#endif

    
    // create the opencpn "home" directory if we need to
    wxFileName wxHomeFiledir( GetHomeDir() );
    if( true != wxHomeFiledir.DirExists( wxHomeFiledir.GetPath() ) )
        if( !wxHomeFiledir.Mkdir( wxHomeFiledir.GetPath() ) ) {
            wxASSERT_MSG(false,_T("Cannot create opencpn home directory"));
            return false;
    }
        
        // create the opencpn "log" directory if we need to
    wxFileName wxLogFiledir( mlog_file );
    if( true != wxLogFiledir.DirExists( wxLogFiledir.GetPath() ) ) {
        if( !wxLogFiledir.Mkdir( wxLogFiledir.GetPath() ) ) {
            wxASSERT_MSG(false,_T("Cannot create opencpn log directory"));
            return false;
        }
    }
    
    mlog_file.Append( _T("opencpn.log") );
    wxString logit = mlog_file;

#ifdef __OCPN__ANDROID__
    wxCharBuffer abuf = mlog_file.ToUTF8();  qDebug() << "logfile " << abuf.data();
#endif        
    
        //  Constrain the size of the log file
    if( ::wxFileExists( mlog_file ) ) {
            if( wxFileName::GetSize( mlog_file ) > 1000000 ) {
                wxString oldlog = mlog_file;
                oldlog.Append( _T(".log") );
                //  Defer the showing of this messagebox until the system locale is established.
                large_log_message = ( _("Old log will be moved to opencpn.log.log") );
                ::wxRenameFile( mlog_file, oldlog );
            }
    }
        
#ifdef __OCPN__ANDROID__
        //  Force new logfile for each instance
        // TODO Remove this behaviour on Release
    if( ::wxFileExists( mlog_file ) ){
        ::wxRemoveFile( mlog_file );
    }
#endif
        
    flog = fopen( mlog_file.mb_str(), "a" );
    g_logger = new wxLogStderr( flog );
        
#ifdef __OCPN__ANDROID__
        //  Trouble printing timestamp
    g_logger->SetTimestamp((const char *)NULL);
#endif
        
#if defined(__WXGTK__) || defined(__WXOSX__)
    g_logger->SetTimestamp(_T("%H:%M:%S %Z"));
#endif
        
    m_Oldlogger = wxLog::SetActiveTarget( g_logger );

    return true;
    
}

void OCPNPlatform::CloseLogFile( void)
{
    if( g_logger ) {
        wxLog::SetActiveTarget( m_Oldlogger );
        delete g_logger;
    }
}



MyConfig *OCPNPlatform::GetConfigObject()
{
    MyConfig *result = NULL;

    result = new MyConfig( wxString( _T("") ), wxString( _T("") ), GetConfigFileName() );

    return result;
}







//--------------------------------------------------------------------------
//      Internal GPS Support
//--------------------------------------------------------------------------

bool OCPNPlatform::hasInternalGPS(wxString profile)
{
    
#ifdef __OCPN__ANDROID__
    bool t = androidDeviceHasGPS();
//    qDebug() << "androidDeviceHasGPS" << t;
    return t;
#else

    return false;
    
#endif    
}


//--------------------------------------------------------------------------
//      Platform Display Support
//--------------------------------------------------------------------------

void OCPNPlatform::ShowBusySpinner( void )
{
#ifdef __OCPN__ANDROID__
    androidShowBusyIcon();
#else 
    #if wxCHECK_VERSION(2, 9, 0 )
    if( !::wxIsBusy() ){
        ::wxBeginBusyCursor();
    }
    #endif
#endif    
}

void OCPNPlatform::HideBusySpinner( void )
{
#ifdef __OCPN__ANDROID__
    androidHideBusyIcon();
#else
    #if wxCHECK_VERSION(2, 9, 0 )
    if( ::wxIsBusy() )
    {
        ::wxEndBusyCursor();
    }
    #endif
#endif    
}

double OCPNPlatform::GetDisplayDensityFactor()
{
#ifdef __OCPN__ANDROID__
    return getAndroidDisplayDensity();
#else
    return 1.0;
#endif
}
    

int OCPNPlatform::GetStatusBarFieldCount()
{
#ifdef __OCPN__ANDROID__
    int count = 1;
    
    //  Make a horizontal measurement...
    wxScreenDC dc;
    wxFont* templateFont = FontMgr::Get().GetFont( _("StatusBar"), 0 );
    dc.SetFont(*templateFont);
    
    wxSize sz = dc.GetTextExtent(_T("WWWWWW"));
    double font_size_pix = (double)sz.x / 6.0;
    
    wxSize dispSize = getDisplaySize();
    
    double nChars = dispSize.x / font_size_pix;
    
    if(nChars < 40)
        count = 1;
    else
        count = 2;
    
    return count;
    
#else
    return STAT_FIELD_COUNT;            // default
#endif

}


double OCPNPlatform::getFontPointsperPixel( void )
{
    double pt_per_pixel = 1.0;
    
#ifdef __OCPN__ANDROID__
    // On Android, this calculation depends on the density bucket in use.
    //  Also uses some magic numbers...
    //  For reference, see http://pixplicity.com/dp-px-converter/
    pt_per_pixel = 14.0 / (31.11 * getAndroidDisplayDensity()) ;
    
#else    
    //  Make a measurement...
    wxScreenDC dc;
    
    wxFont *f = FontMgr::Get().FindOrCreateFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE,
                                                wxString( _T ( "" ) ), wxFONTENCODING_SYSTEM );
    dc.SetFont(*f);
    
    wxSize sz = dc.GetTextExtent(_T("H"));
    pt_per_pixel = 12.0 / (double)sz.y;
#endif
    
    return pt_per_pixel;
    
    
}

wxSize OCPNPlatform::getDisplaySize()
{
#ifdef __OCPN__ANDROID__
    return getAndroidDisplayDimensions();
#else
    return (::wxGetDisplaySize());               // default, for most platforms
#endif

}

double  OCPNPlatform::GetDisplaySizeMM()
{
    double ret = wxGetDisplaySizeMM().GetWidth();
    
#ifdef __WXMSW__    
    int w,h;
    
    bool GetWindowsMonitorSize( int *w, int *h );
       
    if( GetWindowsMonitorSize( &w, &h) ){
        if(w > 100)             // sanity check
            ret = w;
    }
#endif

#ifdef __WXOSX__
    ret = GetMacMonitorSize();
#endif
    
#ifdef __OCPN__ANDROID__
    ret = GetAndroidDisplaySize();
#endif    
    
    wxString msg;
    msg.Printf(_T("Detected display size (horizontal): %d mm"), (int) ret);
    wxLogMessage(msg);
    
    return ret;
}

double OCPNPlatform::GetDisplayDPmm()
{
#ifdef __OCPN__ANDROID__
    return getAndroidDPmm();
#else
    double r = ::wxGetDisplaySize().x;            // dots
    return r / GetDisplaySizeMM();
#endif    
}

void OCPNPlatform::onStagedResizeFinal()
{
#ifdef __OCPN__ANDROID__
    androidConfirmSizeCorrection();
#endif
    
}

bool OCPNPlatform::GetFullscreen()
{
    bool bret = false;
#ifdef __OCPN__ANDROID__
    bret = androidGetFullscreen();
#else
    
#endif

    return bret;
}

bool OCPNPlatform::SetFullscreen( bool bFull )
{
    bool bret = false;
#ifdef __OCPN__ANDROID__
    bret = androidSetFullscreen( bFull );
#else
#endif
    
    return bret;
}


void OCPNPlatform::PositionAISAlert(wxWindow *alert_window)
{
#ifndef __OCPN__ANDROID__    
    if(alert_window){
        alert_window->SetSize(g_ais_alert_dialog_x, g_ais_alert_dialog_y, g_ais_alert_dialog_sx, g_ais_alert_dialog_sy );
    }
#else
    if(alert_window){
        alert_window->SetSize(g_ais_alert_dialog_x, g_ais_alert_dialog_y, g_ais_alert_dialog_sx, g_ais_alert_dialog_sy );
        alert_window->Centre();
    }
    
#endif
}



wxDirDialog* OCPNPlatform::AdjustDirDialogFont(wxWindow *container, wxDirDialog* dlg)
{
    wxDirDialog* ret_dlg = dlg;
    
        dlg->Show();
        dlg->SetSize( container->GetSize());
        dlg->Centre();
        
        wxSize sds = dlg->GetSize();
        wxSize ss = container->GetSize();
        
        
        if(sds.x > ss.x){
            dlg->Hide();
            
            wxString msg = dlg->GetMessage();
            wxString default_dir = dlg->GetPath();
            
            delete dlg;
            
            ret_dlg = new wxDirDialog( NULL, msg, default_dir, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );
            
            
            wxFont *dialogFont = GetOCPNScaledFont(_("Dialog"));
            wxFont *smallFont = new wxFont( * dialogFont ); 
            smallFont->SetPointSize( (smallFont->GetPointSize() / 2) + 0.5 ); // + 0.5 to round instead of truncate
            ret_dlg->SetFont( * smallFont );
            
            ret_dlg->SetSize( container->GetSize());
            ret_dlg->Centre();
            
        }
        ret_dlg->Hide();
        
    return ret_dlg;
}
        
wxFileDialog* OCPNPlatform::AdjustFileDialogFont(wxWindow *container, wxFileDialog* dlg)
{
            wxFileDialog* ret_dlg = dlg;
            
            dlg->Show();
            dlg->SetSize( container->GetSize());
            dlg->Centre();
            
            wxSize sds = dlg->GetSize();
            wxSize ss = container->GetSize();
            
            
            if(sds.x > ss.x){
                dlg->Hide();
                
                wxString msg = dlg->GetMessage();
                wxString default_dir = dlg->GetDirectory();
                wxString default_file = dlg->GetFilename();
                wxString wildcard = dlg->GetWildcard();
                
                delete dlg;
                
                ret_dlg = new wxFileDialog( NULL, msg, default_dir, default_file,  wildcard, wxFD_OPEN );
                
                
                wxFont *dialogFont = GetOCPNScaledFont(_("Dialog"));
                wxFont *smallFont = new wxFont( * dialogFont ); 
                smallFont->SetPointSize( (smallFont->GetPointSize() / 2) + 0.5 ); // + 0.5 to round instead of truncate
                ret_dlg->SetFont( * smallFont );
                
                ret_dlg->SetSize( container->GetSize());
                ret_dlg->Centre();
                
            }
            ret_dlg->Hide();
            
            return ret_dlg;
}
        
double OCPNPlatform::GetToolbarScaleFactor( int GUIScaleFactor )
{
    double rv = 1.0;
#ifdef __OCPN__ANDROID__

    // We try to arrange matters so that at GUIScaleFactor=0, the tool icons are approximately 9 mm in size
    // and that the value may range from 0.5 -> 2.0
    
    //  Get the basic size of a tool icon
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    wxSize style_tool_size = style->GetToolSize();
    double tool_size = style_tool_size.x;
    
    // unless overridden by user, we declare the "best" tool size
    // to be roughly the same as the ActionBar height.
    //  This may be approximated in a device orientation-independent way as:
    //   40pixels * DENSITY
    double premult = 1.0;
    if( g_config_display_size_manual && (g_config_display_size_mm > 0) ){
        double target_size = 9.0;                // mm
    
        double basic_tool_size_mm = tool_size / GetDisplayDPmm();
        premult = target_size / basic_tool_size_mm;
        
    }
    else{
        premult = wxMax(40 * getAndroidDisplayDensity(), 50) / tool_size;
    }            
    
    //Adjust the scale factor using the global GUI scale parameter
    double postmult =  exp( GUIScaleFactor * (0.693 / 5.0) );       //  exp(2)
    
//        qDebug() << "parmsF" << GUIScaleFactor << premult << postmult;
    
    rv = premult * postmult;
    rv = wxMin(rv, 3.0);      //  Clamp at 3.0
    
#else
    double premult = 1.0;

    if(g_bresponsive){
    //  Get the basic size of a tool icon
        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        wxSize style_tool_size = style->GetToolSize();
        double tool_size = style_tool_size.x;

    // unless overridden by user, we declare the "best" tool size
    // to be roughly 9 mm square.
        double target_size = 9.0;                // mm

        double basic_tool_size_mm = tool_size / GetDisplayDPmm();
        premult = target_size / basic_tool_size_mm;
    }

    //Adjust the scale factor using the global GUI scale parameter
    double postmult =  exp( GUIScaleFactor * (0.693 / 5.0) );       //  exp(2)
    
    
    rv = premult * postmult;
    rv = wxMin(rv, 3.0);      //  Clamp at 3.0
    rv = wxMax(rv, 0.5);      //  and at 0.5  

#endif

    return rv;
}

double OCPNPlatform::GetCompassScaleFactor( int GUIScaleFactor )
{
    double rv = 1.0;
#ifdef __OCPN__ANDROID__
    
    // We try to arrange matters so that at GUIScaleFactor=0, the compass icon is approximately 9 mm in size
    // and that the value may range from 0.5 -> 2.0
    
    if(g_bresponsive ){
        //  Get the basic size of a tool icon
        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        wxSize style_tool_size = style->GetToolSize();
        double compass_size = style_tool_size.x;
        
        // We declare the "nominal best" icon size
        // to be roughly the same as the ActionBar height.
        //  This may be approximated in a device orientation-independent way as:
        //   28pixels * DENSITY
        double premult = wxMax(28 * getAndroidDisplayDensity(), 50) / compass_size;
        
        //Adjust the scale factor using the global GUI scale parameter
        double postmult =  exp( GUIScaleFactor * (0.693 / 5.0) );       //  exp(2)
        rv = wxMin(rv, 1.5);      //  Clamp at 1.5
        
        rv = premult * postmult;
        rv = wxMin(rv, 3.0);      //  Clamp at 3.0
    }
    
    
    
#else
    double premult = 1.0;

    if(g_bresponsive ){
        
        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
        wxSize style_tool_size = style->GetToolSize();
        double compass_size = style_tool_size.x;

        // We declare the "best" tool size to be roughly 6 mm.
        double target_size = 6.0;                // mm
        
        double basic_tool_size_mm = compass_size / GetDisplayDPmm();
        premult = target_size / basic_tool_size_mm;
    }
        
    double postmult =  exp( GUIScaleFactor * (0.693 / 5.0) );       //  exp(2)

    rv = premult * postmult;
    rv = wxMin(rv, 3.0);      //  Clamp at 3.0
    rv = wxMax(rv, 0.5);
    
#endif
    
    return rv;
}

float OCPNPlatform::getChartScaleFactorExp( float scale_linear )
{
    double factor = 1.0;
#ifndef __OCPN__ANDROID__
    factor =  exp( scale_linear * (0.693 / 5.0) );       //  exp(2)

#else
    // the idea here is to amplify the scale factor for higher density displays, in a measured way....
    factor =  exp( scale_linear * (0.693 / 5.0) * getAndroidDisplayDensity());
#endif
    
    factor = wxMax(factor, .5);
    factor = wxMin(factor, 4.);
    

    return factor;
}

        
#ifdef __WXMSW__

#define NAME_SIZE 128

const GUID GUID_CLASS_MONITOR = {0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18};

// Assumes hDevRegKey is valid
bool GetMonitorSizeFromEDID(const HKEY hDevRegKey, int *WidthMm, int *HeightMm)
{
    DWORD dwType, AcutalValueNameLength = NAME_SIZE;
    TCHAR valueName[NAME_SIZE];
    
    BYTE EDIDdata[1024];
    DWORD edidsize=sizeof(EDIDdata);
    
    for (LONG i = 0, retValue = ERROR_SUCCESS; retValue != ERROR_NO_MORE_ITEMS; ++i)
    {
        retValue = RegEnumValue ( hDevRegKey, i, &valueName[0],
                                  &AcutalValueNameLength, NULL, &dwType,
                                  EDIDdata, // buffer
                                  &edidsize); // buffer size
        
        if (retValue != ERROR_SUCCESS || 0 != _tcscmp(valueName,_T("EDID")))
            continue;
        
        *WidthMm  = ((EDIDdata[68] & 0xF0) << 4) + EDIDdata[66];
        *HeightMm = ((EDIDdata[68] & 0x0F) << 8) + EDIDdata[67];
        
        return true; // valid EDID found
    }
    
    return false; // EDID not found
}

bool GetSizeForDevID(wxString &TargetDevID, int *WidthMm, int *HeightMm)
{
    HDEVINFO devInfo = SetupDiGetClassDevsEx(
        &GUID_CLASS_MONITOR, //class GUID
        NULL, //enumerator
        NULL, //HWND
        DIGCF_PRESENT, // Flags //DIGCF_ALLCLASSES|
        NULL, // device info, create a new one.
        NULL, // machine name, local machine
        NULL);// reserved
    
    if (NULL == devInfo)
        return false;
    
    bool bRes = false;
    
    for (ULONG i=0; ERROR_NO_MORE_ITEMS != GetLastError(); ++i)
    {
        SP_DEVINFO_DATA devInfoData;
        memset(&devInfoData,0,sizeof(devInfoData));
        devInfoData.cbSize = sizeof(devInfoData);
        
        if (SetupDiEnumDeviceInfo(devInfo,i,&devInfoData))
        {
            wchar_t    Instance[80];
            SetupDiGetDeviceInstanceId(devInfo, &devInfoData, Instance, MAX_PATH, NULL);
            wxString instance(Instance);
            if(instance.Upper().Find( TargetDevID.Upper() ) == wxNOT_FOUND )
                continue;
            
            HKEY hDevRegKey = SetupDiOpenDevRegKey(devInfo,&devInfoData,
                                                   DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            
            if(!hDevRegKey || (hDevRegKey == INVALID_HANDLE_VALUE))
                continue;
            
            bRes = GetMonitorSizeFromEDID(hDevRegKey, WidthMm, HeightMm);
            
            RegCloseKey(hDevRegKey);
        }
    }
    SetupDiDestroyDeviceInfoList(devInfo);
    return bRes;
}

bool GetWindowsMonitorSize( int *width, int *height)
{
    int WidthMm = 0;
    int HeightMm = 0;
    
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(dd);
    DWORD dev = 0; // device index
    int id = 1; // monitor number, as used by Display Properties > Settings
    
    wxString DeviceID;
    bool bFoundDevice = false;
    while (EnumDisplayDevices(0, dev, &dd, 0) && !bFoundDevice)
    {
        DISPLAY_DEVICE ddMon;
        ZeroMemory(&ddMon, sizeof(ddMon));
        ddMon.cb = sizeof(ddMon);
        DWORD devMon = 0;
        
        while (EnumDisplayDevices(dd.DeviceName, devMon, &ddMon, 0) && !bFoundDevice)
        {
            if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE &&
                !(ddMon.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
            {
                DeviceID = wxString(ddMon.DeviceID, wxConvUTF8);
                DeviceID = DeviceID.Mid (8);
                DeviceID = DeviceID.Mid (0, DeviceID.Find ( '\\' ));
                
                bFoundDevice = GetSizeForDevID(DeviceID, &WidthMm, &HeightMm);
            }
            devMon++;
            
            ZeroMemory(&ddMon, sizeof(ddMon));
            ddMon.cb = sizeof(ddMon);
        }
        
        ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);
        dev++;
    }
    
    if(width)
        *width = WidthMm;
    if(height)
        *height = HeightMm;
    
    return bFoundDevice;
}


#endif




//--------------------------------------------------------------------------
//      Internal Bluetooth Support
//--------------------------------------------------------------------------

bool OCPNPlatform::hasInternalBT(wxString profile)
{
#ifdef __OCPN__ANDROID__
    bool t = androidDeviceHasBlueTooth();
//    qDebug() << "androidDeviceHasBluetooth" << t;
    return t;
#else
    
    return false;
#endif    
}

bool OCPNPlatform::startBluetoothScan()
{
#ifdef __OCPN__ANDROID__
    return androidStartBluetoothScan();
#else
    
    return false;
#endif    
}

bool OCPNPlatform::stopBluetoothScan()
{
#ifdef __OCPN__ANDROID__
    return androidStopBluetoothScan();
#else
    
    return false;
#endif    
}


wxArrayString OCPNPlatform::getBluetoothScanResults()
{
    wxArrayString ret_val;
#ifdef __OCPN__ANDROID__
    return androidGetBluetoothScanResults();
#else

    ret_val.Add(_T("line 1"));
    ret_val.Add(_T("line 2"));
    ret_val.Add(_T("line 3"));
    return ret_val;
    
#endif    
    
}


//--------------------------------------------------------------------------
//      Per-Platform Utility support
//--------------------------------------------------------------------------

void OCPNPlatform::setChartTypeMaskSel(int mask, wxString &indicator)
{
#ifdef __OCPN__ANDROID__
    return androidSetChartTypeMaskSel(mask, indicator);
#endif
    
}

#ifdef __WXQT__
QString g_qtStyleSheet;

bool LoadQtStyleSheet(wxString &sheet_file)
{
    if(wxFileExists( sheet_file )){
        //        QApplication qApp = getqApp();
        if(qApp){
            QString file(sheet_file.c_str());
            QFile File(file);
            File.open(QFile::ReadOnly);
            g_qtStyleSheet = QLatin1String(File.readAll());
            
 //           qApp->setStyleSheet(g_qtStyleSheet);
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

#endif


PlatSpec android_plat_spc;

bool OCPNPlatform::isPlatformCapable( int flag){

#ifndef __OCPN__ANDROID__
    return true;
#else
    if(flag == PLATFORM_CAP_PLUGINS){
        long platver;
        wxString tsdk(android_plat_spc.msdk);
        if(tsdk.ToLong(&platver)){
            if(platver >= 11)
                return true;
        }
    }
    else if(flag == PLATFORM_CAP_FASTPAN){
        long platver;
        wxString tsdk(android_plat_spc.msdk);
        if(tsdk.ToLong(&platver)){
            if(platver >= 14)
                return true;
        }
    }
    
    return false;
#endif    
}    
    
    
void OCPNPlatform::LaunchLocalHelp( void ) {
 
#ifdef __OCPN__ANDROID__
    androidLaunchHelpView();
#else
    wxString def_lang_canonical = _T("en_US");
    
    #if wxUSE_XLOCALE
    if(plocale_def_lang)
        def_lang_canonical = plocale_def_lang->GetCanonicalName();
    #endif
        
        wxString help_locn = g_Platform->GetSharedDataDir() + _T("doc/help_");
        
        wxString help_try = help_locn + def_lang_canonical + _T(".html");
        
        if( ! ::wxFileExists( help_try ) ) {
            help_try = help_locn + _T("en_US") + _T(".html");
            
            if( ! ::wxFileExists( help_try ) ) {
                help_try = help_locn + _T("web") + _T(".html");
            }
            
            if( ! ::wxFileExists( help_try ) ) return;
        }
        
        wxLaunchDefaultBrowser(wxString( _T("file:///") ) + help_try );
#endif        
}


