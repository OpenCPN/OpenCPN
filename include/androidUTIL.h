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

#ifndef ANDROIDUTIL_H
#define ANDROIDUTIL_H

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "OCPN_Sound.h"

class ArrayOfCDI;

#include <QString>
#include <string>

// Enumerators for OCPN menu actions requested by Android UI
#define OCPN_ACTION_FOLLOW 0x1000
#define OCPN_ACTION_ROUTE 0x1001
#define OCPN_ACTION_RMD 0x1002
#define OCPN_ACTION_SETTINGS_BASIC 0x1003
#define OCPN_ACTION_SETTINGS_EXPERT 0x1004
#define OCPN_ACTION_TRACK_TOGGLE 0x1005
#define OCPN_ACTION_MOB 0x1006
#define OCPN_ACTION_TIDES_TOGGLE 0x1007
#define OCPN_ACTION_CURRENTS_TOGGLE 0x1008
#define OCPN_ACTION_ENCTEXT_TOGGLE 0x1009
#define OCPN_ACTION_TRACK_ON 0x100a
#define OCPN_ACTION_TRACK_OFF 0x100b
#define OCPN_ACTION_ENCSOUNDINGS_TOGGLE 0x100c
#define OCPN_ACTION_ENCLIGHTS_TOGGLE 0x100d
#define OCPN_ACTION_DOWNLOAD_VALID 0x100e

#define GPS_OFF 0
#define GPS_ON 1
#define GPS_PROVIDER_AVAILABLE 2
#define GPS_SHOWPREFERENCES 3

extern bool androidUtilInit(void);

extern wxString androidGetDeviceInfo();
extern void CheckMigrateCharts();

extern bool androidGetMemoryStatus(int *mem_total, int *mem_used);

extern double GetAndroidDisplaySize();
extern double getAndroidDPmm();
extern wxSize getAndroidDisplayDimensions(void);
extern double getAndroidDisplayDensity();
extern int getAndroidActionBarHeight();
extern void androidConfirmSizeCorrection();
extern void androidForceFullRepaint(bool bskipConfirm = false);
extern int androidGetVersionCode();
extern wxString androidGetVersionName();

extern bool LoadQtStyleSheet(wxString &sheet_file);
extern QString getQtStyleSheet(void);

extern void androidShowBusyIcon();
extern void androidHideBusyIcon();
extern void androidEnableBackButton(bool benable);
extern void androidEnableBackButtonCheck(bool benable);
extern void androidEnableOptionItems(bool benable);

extern wxString androidGetSupplementalLicense(void);

extern bool androidStartGPS(wxEvtHandler *consumer);
extern bool androidStopGPS();
extern wxString androidGPSService(int parm);
extern bool androidDeviceHasGPS();

extern bool androidDeviceHasBlueTooth();
extern bool androidStartBluetoothScan();
extern bool androidStopBluetoothScan();
extern wxArrayString androidGetBluetoothScanResults();
extern bool androidStartBT(wxEvtHandler *consumer, wxString mac_address);
extern bool androidStopBT();
extern bool androidSendBTMessage(wxString &payload);

extern wxArrayString *androidGetSerialPortsArray(void);
extern bool androidStartUSBSerial(wxString &portname, wxString baudRate,
                                  wxEvtHandler *consumer);
extern bool androidStopUSBSerial(wxString &portname);
extern bool androidWriteSerial(wxString &portname, wxString &message);

extern bool DoAndroidPreferences(void);
extern int androidFileChooser(wxString *result, const wxString &initDir,
                              const wxString &title, const wxString &suggestion,
                              const wxString &wildcard, bool dirOnly = false,
                              bool addFiles = false);

extern void androidSetChartTypeMaskSel(int mask, wxString &indicator);
extern void androidSetRouteAnnunciator(bool viz);
extern void androidSetFollowTool(int state, bool forceUpdate = false);
extern void androidSetTrackTool(bool bactive);

extern wxString androidGetHomeDir();
extern wxString
androidGetPrivateDir();  // Used for logfile, config file, and the like
extern wxString
androidGetSharedDir();  // Used for assets like uidata, s57data, etc
extern wxString
androidGetCacheDir();  // Used for raster_texture_cache, mmsitoname.csv, etc
extern wxString androidGetExtStorageDir();  // Used for Chart storage, typically
extern wxString androidGetDownloadDirectory();

extern int startAndroidFileDownload(const wxString &url,
                                    const wxString &destination,
                                    wxEvtHandler *evh, long *dl_ID);
extern int queryAndroidFileDownload(long dl_ID, wxString *result);
extern void finishAndroidFileDownload();
extern void cancelAndroidFileDownload(long dl_ID);
extern int validateAndroidWriteLocation(const wxString &destination);

extern wxString doAndroidPOST(const wxString &url, wxString &parms,
                              int timeoutMsec);

extern wxString getFontQtStylesheet(wxFont *font);
extern wxSize getAndroidConfigSize();
void resizeAndroidPersistents();
bool AndroidSecureCopyFile(wxString in, wxString out);

class AndroidSound;
bool androidPlaySound(const wxString soundfile, AndroidSound* sound);

bool androidGetFullscreen();
bool androidSetFullscreen(bool bFull);
void androidDisableFullScreen();
void androidRestoreFullScreen();

void androidLaunchHelpView();
void androidTerminate();

void androidTestCPP();
int androidGetTZOffsetMins();

int androidApplySettingsString(wxString settings, ArrayOfCDI *pACDI);

bool androidShowDisclaimer(wxString title, wxString msg);
bool androidShowSimpleOKDialog(std::string title, std::string msg);
bool androidShowSimpleYesNoDialog(wxString title, wxString msg);
bool androidInstallPlaystoreHelp();

void androidLaunchBrowser(wxString URL);
bool androidCheckOnline();

unsigned int androidColorPicker(unsigned int initialColor);

wxArrayString androidTraverseDir(wxString dir, wxString filespec);
void androidEnableOptionsMenu(bool bEnable);

void prepareAndroidStyleSheets();
QString getAdjustedDialogStyleSheet();
QString getListBookStyleSheet();
QString getScrollBarsStyleSheet();
void setChoiceStyleSheet(wxChoice *win, int refDim);
void setMenuStyleSheet(wxMenu *win, const wxFont &font);
QString prepareAndroidSliderStyleSheet(int sliderWidth);

void androidDisplayTimedToast(wxString message, int timeMillisec);
void androidCancelTimedToast();
void androidDisplayToast(wxString message);

void androidEnableRotation(void);
void androidDisableRotation(void);
int androidGetScreenOrientation();

void androidEnableMulticast(bool benable);
void androidLastCall();
wxString androidGetIpV4Address(void);

//      SVG Support
wxBitmap loadAndroidSVG(const wxString filename, unsigned int width,
                        unsigned int height);

wxString androidGetAndroidSystemLocale();
bool androidIsDirWritable( wxString dir );
wxArrayString GetConfigChartDirectories();

class InProgressIndicator: public wxGauge
{
    DECLARE_EVENT_TABLE()

public:
    InProgressIndicator();
    InProgressIndicator(wxWindow* parent, wxWindowID id, int range,
                        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                        long style = wxGA_HORIZONTAL, const wxValidator& validator = wxDefaultValidator, const wxString& name = "inprogress");

    ~InProgressIndicator();

    void OnTimer(wxTimerEvent &evt);
    void Start();
    void Stop();


    wxTimer m_timer;
    int msec;
    bool m_bAlive;
};


///////////////////////////////////////////////////////////////////////////////
/// Class MigrateAssistantDialog
///////////////////////////////////////////////////////////////////////////////
#define ID_MIGRATE_OK 8791
#define ID_MIGRATE_CANCEL 8792
#define ID_MIGRATE_START 8793
#define ID_MIGRATE_CONTINUE 8794
#define MIGRATION_STATUS_TIMER 8795

class MigrateAssistantDialog : public wxDialog {
public:
  explicit MigrateAssistantDialog(wxWindow *parent,
                          bool bskipScan,
                          wxWindowID id = wxID_ANY,
                          const wxString &caption = wxEmptyString,
                          const wxPoint &pos = wxDefaultPosition,
                          const wxSize &size = wxDefaultSize, long style = 0);
  ~MigrateAssistantDialog(void);

  void CreateControls(void);
  void OnMigrateCancelClick(wxCommandEvent &event);
  void OnMigrateOKClick(wxCommandEvent &event);
  void OnMigrateClick(wxCommandEvent &event);
  void OnMigrate1Click(wxCommandEvent &event);
  void OnCtlUpdated(wxCommandEvent &event);
  void onPermissionGranted(wxString);
  void setStatus( wxString s ){ m_statusText->SetLabel(s); }
  void onTimerEvent(wxTimerEvent &event);
  void FinishMigration();

  wxButton *m_CancelButton, *m_OKButton;
  wxButton *m_migrateButton, *m_migrateButton1;
  wxStaticText *m_infoText, *m_infoDirs, *m_migrateStep1, *m_statusText;;
	wxRadioButton *m_radioSDCard, *m_radioInternal;
  InProgressIndicator *m_ipGauge;
  wxStaticBoxSizer *statusSizer;

  wxArrayString m_migrateDirs;
  wxString m_Status;
  wxString m_permissionResult;
  wxTimer m_statusTimer;
  wxString m_migrateSourceFolder;
  wxString m_migrateDestinationFolder;  // something like "/storage/emulated/0/Android/data/org.opencpn.opencpn/files/Charts
                                        // or             "/storage/xxx-yyyy/Android/data/org.opencpn.opencpn/files/Charts
  bool m_bsdcard;
  bool m_bskipScan;

private:

  DECLARE_EVENT_TABLE()
};


#endif  // guard
