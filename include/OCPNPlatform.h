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

#ifndef OCPNPLATFORM_H
#define OCPNPLATFORM_H

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/log.h>
#include <wx/stdpaths.h>

#include <stdio.h>

class MyConfig;

typedef struct {
    char    tsdk[20];
    char    hn[20];
    char    msdk[20];
} PlatSpec;

//--------------------------------------------------------------------------
//      Per-Platform Utility support
//--------------------------------------------------------------------------

#ifdef __WXQT__
extern bool LoadQtStyleSheet(wxString &sheet_file);
extern QString getQtStyleSheet( void );
#endif


class OCPNPlatform
{
public:    
    OCPNPlatform();
    ~OCPNPlatform();


//      Internal Device Support
    static bool hasInternalGPS(wxString profile = _T(""));      // GPS

    static bool hasInternalBT(wxString profile = _T(""));       // Bluetooth
    bool startBluetoothScan();
    wxArrayString getBluetoothScanResults();
    bool stopBluetoothScan();
    
//  Per-Platform initialization support    
    
    //  Called from MyApp() immediately upon entry to MyApp::OnInit()
    static void Initialize_1( void );
    
    //  Called from MyApp() immediately before creation of MyFrame()
    static void Initialize_2( void );
    
    //  Called from MyApp() just before end of MyApp::OnInit()
    static void Initialize_3( void );
    
    static void OnExit_1( void );
    static void OnExit_2( void );
    

    void SetDefaultOptions( void );

    void applyExpertMode(bool mode);
    
//--------------------------------------------------------------------------
//      Platform Display Support
//--------------------------------------------------------------------------
    static void ShowBusySpinner( void );
    static void HideBusySpinner( void );
    double getFontPointsperPixel( void );
    wxSize getDisplaySize();
    double GetDisplaySizeMM();
    double GetDisplayDPmm();
    double GetToolbarScaleFactor( int GUIScaleFactor );
    double GetCompassScaleFactor( int GUIScaleFactor );
    void onStagedResizeFinal();
    
    wxFileDialog *AdjustFileDialogFont(wxWindow *container, wxFileDialog *dlg);
    wxDirDialog  *AdjustDirDialogFont(wxWindow *container,  wxDirDialog *dlg);

    void PositionAISAlert( wxWindow *alert_window);
    float getChartScaleFactorExp( float scale_linear );
    int GetStatusBarFieldCount();
    bool GetFullscreen();
    bool SetFullscreen( bool bFull );
    double GetDisplayDensityFactor();
//--------------------------------------------------------------------------
//      Per-Platform file/directory support
//--------------------------------------------------------------------------

    wxStandardPaths& GetStdPaths();
    wxString &GetHomeDir();
    wxString &GetExePath();
    wxString &GetSharedDataDir();
    wxString &GetPrivateDataDir();
    wxString GetWritableDocumentsDir();
    wxString &GetPluginDir();
    wxString &GetConfigFileName();
    wxString *GetPluginDirPtr();
    wxString *GetSharedDataDirPtr();
    wxString *GetPrivateDataDirPtr();
    wxString &GetLogFileName(){ return mlog_file; }
    MyConfig *GetConfigObject();
    wxString GetSupplementalLicenseString();
    
    int DoFileSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir,
                                wxString suggestedName, wxString wildcard);
    int DoDirSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir);
    
    bool InitializeLogFile( void );
    void CloseLogFile( void );
    wxString    &GetLargeLogMessage( void ){ return large_log_message; }
    FILE        *GetLogFilePtr(){ return flog; }

    
    
//--------------------------------------------------------------------------
//      Per-Platform Utility support
//--------------------------------------------------------------------------
    void setChartTypeMaskSel(int mask, wxString &indicator);
    bool isPlatformCapable( int flag);
#define PLATFORM_CAP_PLUGINS   1
#define PLATFORM_CAP_FASTPAN   2
    void LaunchLocalHelp();
    
private:
    wxString    m_homeDir;
    wxString    m_exePath;
    wxString    m_SData_Dir;
    wxString    m_PrivateDataDir;
    wxString    m_PluginsDir;
    wxString    m_config_file_name;
    
    wxString    mlog_file;
    FILE        *flog;
    wxLog       *m_Oldlogger;
    wxString    large_log_message;
    
    

};


#endif          //guard
