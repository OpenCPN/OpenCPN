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
#include <wx/clrpicker.h>
#include <wx/colourdata.h>
#include <wx/colordlg.h>

#include <stdio.h>

class MyConfig;
class ArrayOfCDI;

typedef struct {
    char    tsdk[20];
    char    hn[20];
    char    msdk[20];
} PlatSpec;

//--------------------------------------------------------------------------
//      Per-Platform Utility support
//--------------------------------------------------------------------------

// #ifdef __WXQT__
// extern bool LoadQtStyleSheet(wxString &sheet_file);
// extern QString getQtStyleSheet( void );
// #endif


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
    void Initialize_2( void );
    
    //  Called from MyApp()::OnInit() just after gFrame is created, so gFrame is available
    void Initialize_3( void );

    //  Called from MyApp() just before end of MyApp::OnInit()
    static void Initialize_4( void );
    
    static void OnExit_1( void );
    static void OnExit_2( void );
    

    void SetDefaultOptions( void );
    void SetUpgradeOptions( wxString vString, wxString vStringConfig );

    void applyExpertMode(bool mode);
    
//--------------------------------------------------------------------------
//      Platform Display Support
//--------------------------------------------------------------------------
    static void ShowBusySpinner( void );
    static void HideBusySpinner( void );
    double getFontPointsperPixel( void );
    wxSize getDisplaySize();
    double GetDisplaySizeMM();
    double GetDisplayAreaCM2();

    void SetDisplaySizeMM( double size );
    double GetDisplayDPmm();
    unsigned int GetSelectRadiusPix();
    double GetToolbarScaleFactor( int GUIScaleFactor );
    double GetCompassScaleFactor( int GUIScaleFactor );
    
    wxFileDialog *AdjustFileDialogFont(wxWindow *container, wxFileDialog *dlg);
    wxDirDialog  *AdjustDirDialogFont(wxWindow *container,  wxDirDialog *dlg);

    void PositionAISAlert( wxWindow *alert_window);
    float getChartScaleFactorExp( float scale_linear );
    int GetStatusBarFieldCount();
    bool GetFullscreen();
    bool SetFullscreen( bool bFull );
    bool AllowAlertDialog(const wxString& class_name);
    double GetDisplayDensityFactor();
    double m_pt_per_pixel;
    long GetDefaultToolbarOrientation();
    
//--------------------------------------------------------------------------
//      Per-Platform file/directory support
//--------------------------------------------------------------------------

    wxStandardPaths& GetStdPaths();
    wxString &GetHomeDir();
    wxString &GetExePath();
    wxString &GetSharedDataDir();
    wxString &GetPrivateDataDir();
    wxString GetWritableDocumentsDir();

    /** The original in-tree plugin directory, sometimes not user-writable.*/
    wxString &GetPluginDir();

    /**
     * Base directory for user writable windows plugins, reflects
     * winPluginDir option, defaults to %LOCALAPPDATA%/opencpn.
     **/
     wxString GetWinPluginBaseDir();

    /**
     * Return ';'-separated list of base directories for plugin data. The
     * list always includes the main installation directory. Some platforms
     * prepends this with user-writable path(s), each of which ending in
     * "opencpn". All paths are guaranteed to exist.
     *
     * For Linux, return paths ending in .../share. 
     */
    wxString GetPluginDataPath();

    /** Return true if ocpn is running in a flatpak sandbox. */
    bool isFlatpacked() { return m_isFlatpacked; }

    wxString &GetConfigFileName();
    wxString *GetPluginDirPtr();
    wxString *GetSharedDataDirPtr();
    wxString *GetPrivateDataDirPtr();
    wxString &GetLogFileName(){ return mlog_file; }
    MyConfig *GetConfigObject();
    wxString GetSupplementalLicenseString();
    wxString NormalizePath(const wxString &full_path); //Adapt for portable use
    
    int DoFileSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir,
                                wxString suggestedName, wxString wildcard);
    int DoDirSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir, bool b_addFiles = true);
    
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
    void DoHelpDialog( void );

    int platformApplyPrivateSettingsString( wxString settings, ArrayOfCDI *pDirArray);
    void platformLaunchDefaultBrowser( wxString URL );

    void SetLocaleSearchPrefixes( void );
    wxString GetDefaultSystemLocale();
    
#if wxUSE_XLOCALE    
    wxString GetAdjustedAppLocale();
    wxString ChangeLocale(wxString &newLocaleID, wxLocale *presentLocale, wxLocale** newLocale);
#endif
    
    
//--------------------------------------------------------------------------
//      Per-Platform OpenGL support
//--------------------------------------------------------------------------
    bool BuildGLCaps( void *pbuf );
    bool IsGLCapable();

private:
    bool        GetWindowsMonitorSize( int *width, int *height);
    
    wxString    m_homeDir;
    wxString    m_exePath;
    wxString    m_SData_Dir;
    wxString    m_PrivateDataDir;
    wxString    m_PluginsDir;
    wxString    m_config_file_name;
    wxString    m_pluginDataPath;
    
    wxString    mlog_file;
    FILE        *flog;
    wxLog       *m_Oldlogger;
    wxString    large_log_message;
    wxSize      m_displaySize;
    wxSize      m_displaySizeMM;
    int         m_displaySizeMMOverride;
    
    int         m_monitorWidth, m_monitorHeight;
    bool        m_bdisableWindowsDisplayEnum;
    bool        m_isFlatpacked;
};


//--------------------------------------------------------------------------
//      Private colourPicker control
//--------------------------------------------------------------------------

class OCPNColourPickerCtrl : public wxBitmapButton
{
public:
    OCPNColourPickerCtrl();
    OCPNColourPickerCtrl(wxWindow *parent,
                   wxWindowID id,
                   const wxColour& initial = *wxBLACK,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = _T(""));

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxColour& initial = *wxBLACK,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = _T(""));

    void OnButtonClick(wxCommandEvent& WXUNUSED(ev));
    void InitColourData();
    void SetColour( wxColour& c);
    wxColour GetColour( void );
    
protected:
    virtual void UpdateColour();
    wxSize DoGetBestSize() const;

private:
    wxBitmap m_bitmap;
    wxColour m_colour;
    wxColourData ms_data;

};





#endif          //guard
