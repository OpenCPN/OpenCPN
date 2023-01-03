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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <sstream>

#include <wx/tokenzr.h>
#include <wx/aui/aui.h>
#include <wx/config.h>
#include <wx/fontpicker.h>
#include <wx/filepicker.h>
#include <wx/zipstrm.h>
#include <wx/textwrapper.h>

#include <QtAndroidExtras/QAndroidJniObject>

#include "config.h"
#include "dychart.h"
#include "androidUTIL.h"
//#include "OCPN_DataStreamEvent.h"
#include "AISTargetQueryDialog.h"
#include "AISTargetAlertDialog.h"
#include "AISTargetListDialog.h"
#include "TrackPropDlg.h"
#include "S57QueryDialog.h"
#include "options.h"
#include "own_ship.h"
#include "plugin_loader.h"
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
#include "logger.h"
#include "multiplexer.h"
#include "chartdbs.h"
#include "glChartCanvas.h"
#include "chcanv.h"
#include "MarkInfo.h"
#include "RoutePropDlgImpl.h"
#include "MUIBar.h"
#include "toolbar.h"
#include "nav_object_database.h"
#include "nmea0183.h"
#include "toolbar.h"
#include "iENCToolbar.h"
#include "select.h"
#include "routeman.h"
#include "CanvasOptions.h"
//#include "SerialDataStream.h"
#include "gui_lib.h"
#include "AndroidSound.h"
#include "idents.h"
#include "config_vars.h"

#ifdef HAVE_DIRENT_H
#include "dirent.h"
#endif

const wxString AndroidSuppLicense = wxT(
    "<br><br>The software included in this product contains copyrighted "
    "software that is licensed under the GPL.")
    wxT("A copy of that license is shown above.") wxT(
        "You may obtain the complete Corresponding Source code from us for ")
        wxT("a period of three years after our last shipment of this product, ")
            wxT("by sending a money order or check for $5 to:<br><br>")
                wxT("GPL Compliance Division<br>") wxT("Dyad Inc.<br>")
                    wxT("31 Ocean Reef Dr<br>") wxT("# C101-449<br>")
                        wxT("Key Largo, FL 33037-5282<br>")
                            wxT("United States<br><br>")
                                wxT("Please write “source for OpenCPN Version "
                                    "{insert version here} in the memo line of "
                                    "your payment.<br><br>");

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

class androidUtilHandler;
class PlugInManager;

extern MyFrame *gFrame;
extern const wxEventType wxEVT_OCPN_DATASTREAM;
// extern const wxEventType wxEVT_DOWNLOAD_EVENT;

wxEvtHandler *s_pAndroidNMEAMessageConsumer;
wxEvtHandler *s_pAndroidBTNMEAMessageConsumer;

extern AISTargetAlertDialog *g_pais_alert_dialog_active;
extern AISTargetQueryDialog *g_pais_query_dialog_active;
extern AISTargetListDialog *g_pAISTargetList;
// extern MarkInfoImpl              *pMarkPropDialog;
extern RoutePropDlgImpl *pRoutePropDialog;
extern TrackPropDlg *pTrackPropDialog;
extern S57QueryDialog *g_pObjectQueryDialog;
extern options *g_options;
extern bool g_bSleep;
androidUtilHandler *g_androidUtilHandler;
extern wxDateTime g_start_time;
extern RouteManagerDialog *pRouteManagerDialog;
extern about *g_pAboutDlgLegacy;
extern bool g_bFullscreen;
extern OCPNPlatform *g_Platform;

// Static globals
extern ChartDB *ChartData;
extern MyConfig *pConfig;
extern wxConfigBase *pBaseConfig;
extern wxConfigBase *pBaseConfig;

//   Preferences globals
extern bool g_bShowOutlines;
extern bool g_bShowChartBar;
extern bool g_bShowDepthUnits;
extern bool g_bskew_comp;
extern bool g_bopengl;
extern bool g_bsmoothpanzoom;
extern bool g_bShowMag;
extern int g_chart_zoom_modifier_raster;
extern int g_NMEAAPBPrecision;

extern wxString *pInit_Chart_Dir;
extern wxArrayOfConnPrm *g_pConnectionParams;
extern bool g_bfilter_cogsog;
extern int g_COGFilterSec;
extern int g_SOGFilterSec;

extern bool g_bDisplayGrid;

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
extern bool g_bAISShowTracks;
extern double g_AISShowTracks_Mins;
extern bool g_bHideMoored;
extern double g_ShowMoored_Kts;
extern bool g_bAIS_CPA_Alert;
extern bool g_bAIS_CPA_Alert_Audio;
extern wxString g_sAIS_Alert_Sound_File;
extern bool g_bAIS_CPA_Alert_Suppress_Moored;
extern bool g_bShowAreaNotices;
extern bool g_bDrawAISSize;
extern bool g_bShowAISName;
extern int g_Show_Target_Name_Scale;
extern bool g_bWplIsAprsPosition;

extern int g_iNavAidRadarRingsNumberVisible;
extern float g_fNavAidRadarRingsStep;
extern int g_pNavAidRadarRingsStepUnits;
extern int g_iWaypointRangeRingsNumber;
extern float g_fWaypointRangeRingsStep;
extern int g_iWaypointRangeRingsStepUnits;
extern wxColour g_colourWaypointRangeRingsColour;
extern bool g_bWayPointPreventDragging;

extern bool g_bPreserveScaleOnX;
extern bool g_bPlayShipsBells;
extern int g_iSoundDeviceIndex;
extern bool g_bFullscreenToolbar;

extern int g_OwnShipIconType;
extern double g_n_ownship_length_meters;
extern double g_n_ownship_beam_meters;
extern double g_n_gps_antenna_offset_y;
extern double g_n_gps_antenna_offset_x;
extern int g_n_ownship_min_mm;
extern double g_n_arrival_circle_radius;

extern bool g_bEnableZoomToCursor;
extern bool g_bTrackDaily;
extern bool g_bHighliteTracks;
extern double g_TrackIntervalSeconds;
extern double g_TrackDeltaDistance;
extern double g_TrackDeltaDistance;
extern int g_nTrackPrecision;

extern int g_iSDMMFormat;
extern int g_iDistanceFormat;
extern int g_iSpeedFormat;

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

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
extern wxLocale *plocale_def_lang;
#endif

// extern OCPN_Sound        g_anchorwatch_sound;
extern bool g_bMagneticAPB;

extern bool g_fog_overzoom;
extern double g_overzoom_emphasis_base;
extern bool g_oz_vector_scale;
extern bool g_bShowStatusBar;

extern ocpnGLOptions g_GLOptions;

extern s52plib *ps52plib;

extern wxString g_locale;
extern bool g_bportable;
extern bool g_bdisable_opengl;

extern ChartGroupArray *g_pGroupArray;

extern bool g_bUIexpert;
//    Some constants
#define ID_CHOICE_NMEA wxID_HIGHEST + 1

// extern wxArrayString *EnumerateSerialPorts(void);           // in chart1.cpp

extern wxArrayString TideCurrentDataSet;
extern wxString g_TCData_Dir;

extern AisDecoder *g_pAIS;

extern options *g_pOptions;

extern bool g_btouch;
extern bool g_bresponsive;
extern bool g_bAutoHideToolbar;
extern int g_nAutoHideToolbar;
extern int g_GUIScaleFactor;
extern int g_ChartScaleFactor;

extern double g_config_display_size_mm;
extern float g_ChartScaleFactorExp;
extern bool g_config_display_size_manual;

extern Multiplexer *g_pMUX;
extern bool b_inCloseWindow;
extern bool g_config_display_size_manual;
extern MarkInfoDlg *g_pMarkInfoDialog;
extern PlugInManager *g_pi_manager;
extern iENCToolbar *g_iENCToolbar;
extern int g_iENCToolbarPosX;
extern int g_iENCToolbarPosY;
extern ocpnFloatingToolbarDialog *g_MainToolbar;
extern int g_maintoolbar_x;
extern int g_maintoolbar_y;
extern long g_maintoolbar_orient;
extern int g_restore_stackindex;
extern int g_restore_dbindex;
extern ChartStack *pCurrentStack;
extern Select *pSelect;
extern WayPointman *pWayPointMan;
extern bool g_bCruising;
extern RoutePoint *pAnchorWatchPoint1;
extern RoutePoint *pAnchorWatchPoint2;
extern bool g_bAutoAnchorMark;
extern wxAuiManager *g_pauimgr;
extern wxString g_AisTargetList_perspective;

extern ocpnFloatingToolbarDialog *g_MainToolbar;

WX_DEFINE_ARRAY_PTR(ChartCanvas *, arrayofCanvasPtr);
extern arrayofCanvasPtr g_canvasArray;

wxString callActivityMethod_vs(const char *method);
wxString callActivityMethod_is(const char *method, int parm);

//      Globals, accessible only to this module

JavaVM *java_vm;
JNIEnv *global_jenv;
bool b_androidBusyShown;
double g_androidDPmm;
double g_androidDensity;

bool g_bExternalApp;

wxString g_androidFilesDir;
wxString g_androidCacheDir;
wxString g_androidExtFilesDir;
wxString g_androidExtCacheDir;
wxString g_androidExtStorageDir;
wxString g_androidGetFilesDirs0;
wxString g_androidGetFilesDirs1;
wxString g_androidDownloadDirectory;


int g_mask;
int g_sel;
int g_ActionBarHeight;
int g_follow_state;
bool g_track_active;
bool bGPSEnabled;

wxSize config_size;

bool s_bdownloading;
wxString s_requested_url;
wxEvtHandler *s_download_evHandler;
wxString s_download_destination;

bool g_running;
bool g_bstress1;
extern int g_GUIScaleFactor;

wxString g_deviceInfo;

int s_androidMemTotal;
int s_androidMemUsed;
bool g_backEnabled;
bool g_bFullscreenSave;
bool s_optionsActive;

extern int ShowNavWarning();
extern bool g_btrackContinuous;
extern wxString ChartListFileName;

int doAndroidPersistState();

bool bInConfigChange;
AudioDoneCallback s_soundCallBack;
void *s_soundData;

bool g_detect_smt590;
int g_orientation;
int g_Android_SDK_Version;
MigrateAssistantDialog *g_migrateDialog;

//      Some dummy devices to ensure plugins have static access to these classes
//      not used elsewhere
wxFontPickerEvent g_dummy_wxfpe;

#define ANDROID_EVENT_TIMER 4389
#define ANDROID_STRESS_TIMER 4388
#define ANDROID_RESIZE_TIMER 4387

#define ACTION_NONE -1
#define ACTION_RESIZE_PERSISTENTS 1
#define ACTION_FILECHOOSER_END 3
#define ACTION_COLORDIALOG_END 4
#define ACTION_POSTASYNC_END 5
#define ACTION_SAF_PERMISSION_END 6

#define SCHEDULED_EVENT_CLEAN_EXIT 5498

class androidUtilHandler : public wxEvtHandler {
public:
  androidUtilHandler();
  ~androidUtilHandler() {}

  void onTimerEvent(wxTimerEvent &event);
  void onStressTimer(wxTimerEvent &event);
  void OnResizeTimer(wxTimerEvent &event);
  void OnScheduledEvent(wxCommandEvent &event);

  wxString GetStringResult() { return m_stringResult; }
  void LoadAuxClasses();

  wxTimer m_eventTimer;
  int m_action;
  bool m_done;
  wxString m_stringResult;
  wxTimer m_stressTimer;
  wxTimer m_resizeTimer;
  int timer_sequence;
  int m_bskipConfirm;
  bool m_migratePermissionSetDone;

  DECLARE_EVENT_TABLE()
};

const char wxMessageBoxCaptionStr[] = "Message";

BEGIN_EVENT_TABLE(androidUtilHandler, wxEvtHandler)
EVT_TIMER(ANDROID_EVENT_TIMER, androidUtilHandler::onTimerEvent)
EVT_TIMER(ANDROID_RESIZE_TIMER, androidUtilHandler::OnResizeTimer)
EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
            androidUtilHandler::OnScheduledEvent)

END_EVENT_TABLE()

androidUtilHandler::androidUtilHandler() {
  m_eventTimer.SetOwner(this, ANDROID_EVENT_TIMER);
  m_stressTimer.SetOwner(this, ANDROID_STRESS_TIMER);
  m_resizeTimer.SetOwner(this, ANDROID_RESIZE_TIMER);

  m_bskipConfirm = false;

  LoadAuxClasses();
}

void androidUtilHandler::LoadAuxClasses()
{
  // We do a few little dummy class accesses here, to cause the static link to
  // wxWidgets to bring in some class members required by some plugins, that
  // would be missing otherwise.

  wxRegion a(0, 0, 1, 1);
  wxRegion b(0, 0, 2, 2);
  bool c = a.IsEqual(b);

  wxFilePickerCtrl *pfpc = new wxFilePickerCtrl();

  wxZipEntry *entry = new wxZipEntry();

  wxSplitterWindow *swin = new wxSplitterWindow();

}

void androidUtilHandler::onTimerEvent(wxTimerEvent &event) {
  qDebug() << "onTimerEvent" << m_action;

  switch (m_action) {
    case ACTION_RESIZE_PERSISTENTS:  //  Handle rotation/resizing of persistent
                                     //  dialogs

      // AIS Target Query
      if (g_pais_query_dialog_active) {
        qDebug() << "AISB";

        bool bshown = g_pais_query_dialog_active->IsShown();
        g_pais_query_dialog_active->Hide();
        g_pais_query_dialog_active->RecalculateSize();
        if (bshown) {
          qDebug() << "AISC";
          g_pais_query_dialog_active->Show();
          g_pais_query_dialog_active->Raise();
        }
      }

      // Route Props
      if (RoutePropDlgImpl::getInstanceFlag()) {
        bool bshown = pRoutePropDialog->IsShown();
        if (bshown) {
          pRoutePropDialog->Hide();
          pRoutePropDialog->RecalculateSize();
          pRoutePropDialog->Show();
        } else {
          pRoutePropDialog->Destroy();
          pRoutePropDialog = NULL;
        }
      }

      // Track Props
      if (TrackPropDlg::getInstanceFlag()) {
        bool bshown = pTrackPropDialog->IsShown();
        if (bshown) {
          pTrackPropDialog->Hide();
          pTrackPropDialog->RecalculateSize();
          pTrackPropDialog->Show();
        } else {
          pTrackPropDialog->Destroy();
          pTrackPropDialog = NULL;
        }
      }

      // Mark Props

      if (g_pMarkInfoDialog) {
        bool bshown = g_pMarkInfoDialog->IsShown();
        g_pMarkInfoDialog->Hide();
        g_pMarkInfoDialog->RecalculateSize();
        if (bshown) {
          if (g_pMarkInfoDialog->m_SaveDefaultDlg) {
            g_pMarkInfoDialog->m_SaveDefaultDlg->Destroy();
            g_pMarkInfoDialog->m_SaveDefaultDlg = NULL;
          }
          g_pMarkInfoDialog->Show();
        }
      }

      // ENC Object Query
      if (g_pObjectQueryDialog) {
        bool bshown = g_pObjectQueryDialog->IsShown();
        g_pObjectQueryDialog->Hide();
        g_pObjectQueryDialog->RecalculateSize();
        if (bshown) {
          g_pObjectQueryDialog->Show();
        }
      }

      // AIS Target List dialog
      if (g_pAISTargetList) {
        qDebug() << "ATLA";
        bool bshown = g_pAISTargetList->IsShown();
        g_pAISTargetList->Hide();
        g_pAISTargetList->RecalculateSize();
        if (bshown) {
          qDebug() << "ATLB";
          g_pAISTargetList->Show();
          g_pAISTargetList->Raise();
        }
      }

      // Tide/Current window
      if (gFrame->GetPrimaryCanvas()->getTCWin()) {
        bool bshown = gFrame->GetPrimaryCanvas()->getTCWin()->IsShown();
        gFrame->GetPrimaryCanvas()->getTCWin()->Hide();
        gFrame->GetPrimaryCanvas()->getTCWin()->RecalculateSize();
        if (bshown) {
          gFrame->GetPrimaryCanvas()->getTCWin()->Show();
          gFrame->GetPrimaryCanvas()->getTCWin()->Refresh();
        }
      }

      // Route Manager dialog
      if (RouteManagerDialog::getInstanceFlag()) {
        bool bshown = pRouteManagerDialog->IsShown();
        if (bshown) {
          pRouteManagerDialog->Hide();
          pRouteManagerDialog->RecalculateSize();
          pRouteManagerDialog->Show();
        } else {
          pRouteManagerDialog->Destroy();
          pRouteManagerDialog = NULL;
        }
      }

      // About dialog
      if (g_pAboutDlgLegacy) {
        bool bshown = g_pAboutDlgLegacy->IsShown();
        if (bshown) {
          g_pAboutDlgLegacy->Hide();
          g_pAboutDlgLegacy->RecalculateSize();
          g_pAboutDlgLegacy->Show();
        }
      }

      if (g_options) {
        g_options->RecalculateSize();
      }

      bInConfigChange = false;

      break;

    case ACTION_FILECHOOSER_END:  //  Handle polling of android Dialog
    {
      // qDebug() << "chooser poll";
      //  Get a reference to the running FileChooser
      QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
          "org/qtproject/qt5/android/QtNative", "activity",
          "()Landroid/app/Activity;");

      if (!activity.isValid()) {
        // qDebug() << "onTimerEvent : Activity is not valid";
        return;
      }

      //  Call the method which tracks the completion of the Intent.
      QAndroidJniObject data = activity.callObjectMethod(
          "isFileChooserFinished", "()Ljava/lang/String;");

      jstring s = data.object<jstring>();

      JNIEnv *jenv;

      //  Need a Java environment to decode the resulting string
      if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
        // qDebug() << "GetEnv failed.";
      } else {
        // The string coming back will be one of:
        //  "no"   ......Intent not done yet.
        //  "cancel:"   .. user cancelled intent.
        //  "file:{file_name}"  .. user selected this file, fully qualified.
        if (!s) {
          // qDebug() << "isFileChooserFinished returned null";
        } else if ((jenv)->GetStringLength(s)) {
          const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
          //                        qDebug() << "isFileChooserFinished returned
          //                        " << ret_string;
          if (!strncmp(ret_string, "cancel:", 7)) {
            m_done = true;
            m_stringResult = _T("cancel:");
          } else if (!strncmp(ret_string, "file:", 5)) {
            m_done = true;
            m_stringResult = wxString(ret_string, wxConvUTF8);
          }
        }
      }

      break;
    }

    case ACTION_COLORDIALOG_END:  //  Handle polling of android Dialog
    {
      // qDebug() << "colorpicker poll";
      //  Get a reference to the running FileChooser
      QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
          "org/qtproject/qt5/android/QtNative", "activity",
          "()Landroid/app/Activity;");

      if (!activity.isValid()) {
        // qDebug() << "onTimerEvent : Activity is not valid";
        return;
      }

      //  Call the method which tracks the completion of the Intent.
      QAndroidJniObject data = activity.callObjectMethod(
          "isColorPickerDialogFinished", "()Ljava/lang/String;");

      jstring s = data.object<jstring>();

      JNIEnv *jenv;

      //  Need a Java environment to decode the resulting string
      if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
        // qDebug() << "GetEnv failed.";
      } else {
        // The string coming back will be one of:
        //  "no"   ......Dialog not done yet.
        //  "cancel:"   .. user cancelled Dialog.
        //  "color: ".
        if (!s) {
          qDebug() << "isColorPickerDialogFinished returned null";
        } else if ((jenv)->GetStringLength(s)) {
          const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
          // qDebug() << "isColorPickerDialogFinished returned " << ret_string;
          if (!strncmp(ret_string, "cancel:", 7)) {
            m_done = true;
            m_stringResult = _T("cancel:");
          } else if (!strncmp(ret_string, "color:", 6)) {
            m_done = true;
            m_stringResult = wxString(ret_string, wxConvUTF8);
          }
        }
      }

      break;
    }

    case ACTION_POSTASYNC_END:  //  Handle polling of android async POST task
                                //  end
    {
      // qDebug() << "colorpicker poll";
      //  Get a reference to the running FileChooser
      QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
          "org/qtproject/qt5/android/QtNative", "activity",
          "()Landroid/app/Activity;");

      if (!activity.isValid()) {
        // qDebug() << "onTimerEvent : Activity is not valid";
        return;
      }

      //  Call the method which tracks the completion of the POST async task.
      QAndroidJniObject data =
          activity.callObjectMethod("checkPostAsync", "()Ljava/lang/String;");

      jstring s = data.object<jstring>();

      JNIEnv *jenv;

      //  Need a Java environment to decode the resulting string
      if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
        // qDebug() << "GetEnv failed.";
      } else {
        // The string coming back will be either:
        //  "ACTIVE"   ......Post command not done yet.
        //  A valid XML response body.
        if (!s) {
          qDebug() << "checkPostAsync returned null";
        } else if ((jenv)->GetStringLength(s)) {
          const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
          qDebug() << "checkPostAsync returned " << ret_string;
          if (strncmp(ret_string, "ACTIVE", 6)) {  // Must be done....
            m_done = true;
            m_stringResult = wxString(ret_string, wxConvUTF8);
          }
        }
      }


      break;
    }

    case ACTION_SAF_PERMISSION_END:  //  Handle android SAF Dialog
    {
      qDebug() << "SAF permission chooser poll";

      QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
          "org/qtproject/qt5/android/QtNative", "activity",
          "()Landroid/app/Activity;");

      if (!activity.isValid()) {
        // qDebug() << "onTimerEvent : Activity is not valid";
        return;
      }

      //  Call the method which tracks the completion of the activity.
      QAndroidJniObject data = activity.callObjectMethod(
          "isSAFChooserFinished", "()Ljava/lang/String;");

      jstring s = data.object<jstring>();

      JNIEnv *jenv;

      //  Need a Java environment to decode the resulting string
      if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
        // qDebug() << "GetEnv failed.";
      } else {
        // The string coming back will be one of:
        //  "no"   ......Intent not done yet.
        //  "cancel:"   .. user cancelled intent.
        //  "file:{file_name}"  .. user selected this file, fully qualified.
        if (!s) {
          // qDebug() << "isFileChooserFinished returned null";
        } else if ((jenv)->GetStringLength(s)) {
          const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
          //qDebug() << "isFileChooserFinished returned" << ret_string;
          if (!strncmp(ret_string, "cancel:", 7)) {
            m_migratePermissionSetDone = true;
            m_stringResult = _T("cancel:");
          } else if (!strncmp(ret_string, "file:", 5)) {
            m_migratePermissionSetDone = true;
            m_stringResult = wxString(ret_string, wxConvUTF8);
          }
        }

        if(m_migratePermissionSetDone){
          g_androidUtilHandler->m_action = ACTION_NONE;
          g_androidUtilHandler->m_eventTimer.Stop();

          if(g_migrateDialog)
            g_migrateDialog->onPermissionGranted(m_stringResult);
        }
      }

      break;
    }



    default:
      break;
  }
}

void androidUtilHandler::OnResizeTimer(wxTimerEvent &event) {
  if (timer_sequence == 0) {
    //  On QT, we need to clear the status bar item texts to prevent the status
    //  bar from growing the parent frame due to unexpected width changes.
    //         if( m_pStatusBar != NULL ){
    //             int widths[] = { 2,2,2,2,2 };
    //            m_pStatusBar->SetStatusWidths( m_StatusBarFieldCount, widths
    //            );
    //
    //             for(int i=0 ; i <  m_pStatusBar->GetFieldsCount() ; i++){
    //                 m_pStatusBar->SetStatusText(_T(""), i);
    //             }
    //         }
    qDebug() << "sequence 0";

    timer_sequence++;
    //  This timer step needs to be long enough to allow Java induced size
    //  change to take effect in another thread. The results will be checked in
    //  sequence 1.
    m_resizeTimer.Start(1000, wxTIMER_ONE_SHOT);
    return;
  }

  if (timer_sequence == 1) {
    qDebug() << "sequence 1";

    qDebug() << "****config_size: " << config_size.x << config_size.y;

    wxSize szt = gFrame->GetSize();
    qDebug() << "****Frame Size: " << szt.x << szt.y;

    // Some Android devices do not correctly process the config change, and
    // properly resize the app. A slower forced config change is then necessary,
    // with lots of steps.

    // However, if we can detect the ones that do properly resize the app Frame,
    // we can skip all this.

    wxSize new_size = getAndroidDisplayDimensions();
    qDebug() << "****NewSize: " << new_size.x << new_size.y;

    if ((g_orientation == 1) || (g_orientation == 3)) {  // Portrait
      if (szt.x < szt.y)                                 // OK
        return;
    } else if ((g_orientation == 2) || (g_orientation == 4)) {  // Landscape
      if (szt.x > szt.y)                                        // OK
        return;
    }

    qDebug() << "****Force config change";
    gFrame->SetSize(config_size);
    timer_sequence++;
    if (!m_bskipConfirm) m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
    m_bskipConfirm = false;
    return;
  }

  if (timer_sequence == 2) {
    qDebug() << "sequence 2";
    timer_sequence++;
    m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
    return;
  }

  if (timer_sequence == 3) {
    qDebug() << "sequence 3";
    androidConfirmSizeCorrection();

    timer_sequence++;
    m_resizeTimer.Start(10, wxTIMER_ONE_SHOT);
    return;
  }

  if (timer_sequence == 4) {
    qDebug() << "sequence 4";

    //  Raise the resized options dialog.
    //  This has no effect if the dialog is not already shown.
    if (g_options) g_options->Raise();

    resizeAndroidPersistents();
    return;
  }
}

int stime;

void androidUtilHandler::onStressTimer(wxTimerEvent &event) {
  g_GUIScaleFactor = -5;
  g_ChartScaleFactor = -5;
  gFrame->SetGPSCompassScale();

  s_androidMemUsed = 80;

  g_GLOptions.m_bTextureCompression = 0;
  g_GLOptions.m_bTextureCompressionCaching = 0;

  if (600 == stime++) androidTerminate();
}

void androidUtilHandler::OnScheduledEvent(wxCommandEvent &event) {
  switch (event.GetId()) {
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
      // qDebug() << "Trigger SoundFinished";
      if (s_soundCallBack) {
        s_soundCallBack(s_soundData);  // Wirh user data
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

bool androidUtilInit(void) {
  qDebug() << "androidUtilInit()";

  g_androidUtilHandler = new androidUtilHandler();

  //  Initialize some globals

  s_androidMemTotal = 100;
  s_androidMemUsed = 50;

  wxString dirs = callActivityMethod_vs("getSystemDirs");
  qDebug() << "dirs: " << dirs.mb_str();

  wxStringTokenizer tk(dirs, _T(";"));
  if (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();
    if (wxNOT_FOUND != token.Find(_T("EXTAPP"))) g_bExternalApp = true;

    token = tk.GetNextToken();
    g_androidFilesDir = token;  // used for "home dir"
    token = tk.GetNextToken();
    g_androidCacheDir = token;
    token = tk.GetNextToken();
    g_androidExtFilesDir =
        token;  // used as PrivateDataDir,
                // "/storage/emulated/0/Android/data/org.opencpn.opencpn/files"
                // if app has been moved to sdcard, this gives like (on Android
                // 6) /storage/2385-1BF8/Android/data/org.opencpn.opencpn/files
    token = tk.GetNextToken();
    g_androidExtCacheDir = token;
    token = tk.GetNextToken();
    g_androidExtStorageDir = token;

    token = tk.GetNextToken();
    g_androidGetFilesDirs0 = token;
    token = tk.GetNextToken();
    g_androidGetFilesDirs1 = token;

    token = tk.GetNextToken();
    g_androidDownloadDirectory = token;

  }

  g_mask = -1;
  g_sel = -1;

  wxStringTokenizer tku(g_androidExtFilesDir, _T("/"));
  while (tku.HasMoreTokens()) {
    wxString s1 = tku.GetNextToken();

    if (s1.Find(_T("org.")) != wxNOT_FOUND) {
      if (s1 != _T("org.opencpn.opencpn")) g_bstress1 = true;
    }
  }

  if (g_bstress1) {
    g_androidUtilHandler->Connect(
        g_androidUtilHandler->m_stressTimer.GetId(), wxEVT_TIMER,
        wxTimerEventHandler(androidUtilHandler::onStressTimer), NULL,
        g_androidUtilHandler);
    g_androidUtilHandler->m_stressTimer.Start(1000, wxTIMER_CONTINUOUS);
  }

  return true;
}


wxSize getAndroidConfigSize() { return config_size; }

void resizeAndroidPersistents() {
  qDebug() << "resizeAndroidPersistents()";

  if (g_androidUtilHandler) {
    g_androidUtilHandler->m_action = ACTION_RESIZE_PERSISTENTS;
    g_androidUtilHandler->m_eventTimer.Start(100, wxTIMER_ONE_SHOT);
  }
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  java_vm = vm;

  // Get JNI Env for all function calls
  if (vm->GetEnv((void **)&global_jenv, JNI_VERSION_1_6) != JNI_OK) {
    return -1;
  }

  return JNI_VERSION_1_6;
}

void sendNMEAMessageEvent(wxString &msg) {
  //FIXME (dave)
#if 0
  wxCharBuffer abuf = msg.ToUTF8();
  if (abuf.data()) {  // OK conversion?
    std::string s(abuf.data());
    //    qDebug() << tstr;
    OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
    Nevent.SetNMEAString(s);
    Nevent.SetStream(NULL);
    if (s_pAndroidNMEAMessageConsumer)
      s_pAndroidNMEAMessageConsumer->AddPendingEvent(Nevent);
  }
#endif
}

//      OCPNNativeLib
//      This is a set of methods which can be called from the android activity
//      context.

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_test(JNIEnv *env,
                                                           jobject obj) {
  // qDebug() << "test";

  return 55;
}
}

extern "C" {
JNIEXPORT jint JNICALL
    Java_org_opencpn_OCPNNativeLib_onSoundDone(JNIEnv *env,
                                              jobject obj,
                                              long soundPtr) {
        auto sound = reinterpret_cast<AndroidSound*>(soundPtr);
        DEBUG_LOG << "on SoundDone, ptr: " << soundPtr;
        sound->OnSoundDone();
        return 57;
}
}


extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_processSailTimer(
    JNIEnv *env, jobject obj, double WindAngleMagnetic, double WindSpeedKnots) {
  //  The NMEA message target handler may not be setup yet, if no connections
  //  are defined or enabled. But we may want to synthesize messages from the
  //  Java app, even without a definite connection, and we want to process these
  //  messages too. So assume that the global MUX, if present, will handle these
  //  synthesized messages.
  if (!s_pAndroidNMEAMessageConsumer && g_pMUX)
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
      // qDebug() << "gVar" << gVar;

      //  What to use for TRUE ownship head?
      // TODO Look for HDT message contents, if available
      double osHead = gCog;
      bool buseCOG = true;
      // qDebug() << "gHdt" << gHdt;

      if (!wxIsNaN(gHdt)) {
        osHead = gHdt;
        buseCOG = false;
      }

      // What SOG to use?
      double osSog = gSog;

      wind_angle_mag = WindAngleMagnetic;
      app_windSpeed = WindSpeedKnots;

      // Compute the apparent wind angle
      // If using gCog for ownship head, require speed to be > 0.2 knots
      // If not useing cGog for head, assume we must be using a true heading
      // sensor, so always valid
      if (!wxIsNaN(osHead) && ((!buseCOG) || (buseCOG && osSog > 0.2))) {
        apparent_wind_angle = wind_angle_mag - (osHead - variation);
      } else {
        apparent_wind_angle = 0;
      }
      if (apparent_wind_angle < 0) apparent_wind_angle += 360.;
      if (apparent_wind_angle > 360.) apparent_wind_angle -= 360.;

      //  Using the "Law of cosines", compute the true wind speed
      if (!wxIsNaN(osSog)) {
        true_windSpeed = sqrt(
            (osSog * osSog) + (app_windSpeed * app_windSpeed) -
            (2 * osSog * app_windSpeed * cos(apparent_wind_angle * PI / 180.)));
      } else {
        true_windSpeed = app_windSpeed;
      }

      // Rearranging the Law of cosines, we calculate True Wind Direction
      if ((!wxIsNaN(osSog)) && (!wxIsNaN(osHead)) && (osSog > 0.2) &&
          (true_windSpeed > 1)) {
        double acosTW = ((osSog * osSog) + (true_windSpeed * true_windSpeed) -
                         (app_windSpeed * app_windSpeed)) /
                        (2 * osSog * true_windSpeed);

        double twd0 = acos(acosTW) * (180. / PI);

        // OK on the beat...
        if (apparent_wind_angle > 180.) {
          true_windDirection = osHead + 180 + twd0;
        } else {
          true_windDirection = osHead + 180 - twd0;
        }
      } else {
        true_windDirection = wind_angle_mag + variation;
      }

      if (true_windDirection < 0) true_windDirection += 360.;
      if (true_windDirection > 360.) true_windDirection -= 360.;

      // qDebug() << wind_angle_mag << app_windSpeed << apparent_wind_angle <<
      // true_windSpeed << true_windDirection;

      if (s_pAndroidNMEAMessageConsumer) {
        NMEA0183 parser;

        // Now make some NMEA messages
        // We dont want to pass the incoming MWD message thru directly, since it
        // is not really correct.  The angle is correct, but the speed is
        // relative.
        //  Make a new MWD sentence with calculated values
        parser.TalkerID = _T("OS");

        // MWD
        SENTENCE sntd;
        parser.Mwd.WindAngleTrue = true_windDirection;
        parser.Mwd.WindAngleMagnetic = wind_angle_mag;
        parser.Mwd.WindSpeedKnots = true_windSpeed;
        parser.Mwd.WindSpeedms = true_windSpeed * 0.5144;  // convert kts to m/s
        parser.Mwd.Write(sntd);
        sendNMEAMessageEvent(sntd.Sentence);

        // Now make two MWV sentences
        // Apparent
        SENTENCE snt;
        parser.Mwv.WindAngle = apparent_wind_angle;
        parser.Mwv.WindSpeed = app_windSpeed;
        parser.Mwv.WindSpeedUnits = _T("N");
        parser.Mwv.Reference = _T("R");
        parser.Mwv.IsDataValid = NTrue;
        parser.Mwv.Write(snt);
        sendNMEAMessageEvent(snt.Sentence);

        // True
        SENTENCE sntt;
        double true_relHead = 0;
        if (!wxIsNaN(osHead) && ((!buseCOG) || (buseCOG && osSog > 0.2)))
          true_relHead = true_windDirection - osHead;

        if (true_relHead < 0) true_relHead += 360.;
        if (true_relHead > 360.) true_relHead -= 360.;

        parser.Mwv.WindAngle = true_relHead;
        parser.Mwv.WindSpeed = true_windSpeed;
        parser.Mwv.WindSpeedUnits = _T("N");
        parser.Mwv.Reference = _T("T");
        parser.Mwv.IsDataValid = NTrue;
        parser.Mwv.Write(sntt);
        sendNMEAMessageEvent(sntt.Sentence);
      }
    }
  }

  return 52;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_processNMEA(
    JNIEnv *env, jobject obj, jstring nmea_string) {
  //  The NMEA message target handler may not be setup yet, if no connections
  //  are defined or enabled. But we may get synthesized messages from the Java
  //  app, even without a definite connection, and we want to process these
  //  messages too. So assume that the global MUX, if present, will handle these
  //  messages.
  wxEvtHandler *consumer = s_pAndroidNMEAMessageConsumer;

  if (!consumer && g_pMUX) consumer = g_pMUX;

  const char *string = env->GetStringUTFChars(nmea_string, NULL);

  // qDebug() << "ProcessNMEA: " << string;

  char tstr[200];
  strncpy(tstr, string, 190);
  strcat(tstr, "\r\n");

  // FIXME (dave)
//   if (consumer) {
//     OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
//     Nevent.SetNMEAString(tstr);
//     Nevent.SetStream(NULL);
//
//     consumer->AddPendingEvent(Nevent);
//   }

  return 66;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_processBTNMEA(
    JNIEnv *env, jobject obj, jstring nmea_string) {
  const char *string = env->GetStringUTFChars(nmea_string, NULL);
  wxString wstring = wxString(string, wxConvUTF8);

  char tstr[200];
  strncpy(tstr, string, 190);
  strcat(tstr, "\r\n");

//FIXME (dave)
//   if (s_pAndroidBTNMEAMessageConsumer) {
//     OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
//     Nevent.SetNMEAString(tstr);
//     Nevent.SetStream(NULL);
//
//     s_pAndroidBTNMEAMessageConsumer->AddPendingEvent(Nevent);
//   }

  return 77;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onConfigChange(
    JNIEnv *env, jobject obj, int orientation) {
  g_orientation = orientation;
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
  evt.SetId(ID_CMD_TRIGGER_RESIZE);
  g_androidUtilHandler->AddPendingEvent(evt);

  return 77;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onMouseWheel(JNIEnv *env,
                                                                   jobject obj,
                                                                   int dir) {
  wxMouseEvent evt(wxEVT_MOUSEWHEEL);
  evt.m_wheelRotation = dir;

  if (gFrame->GetPrimaryCanvas()) {
    gFrame->GetPrimaryCanvas()->GetEventHandler()->AddPendingEvent(evt);
  }

  return 77;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onMenuKey(JNIEnv *env,
                                                                jobject obj) {
  //         if(g_MainToolbar){
  //             g_MainToolbar->Show( !g_MainToolbar->IsShown() );
  //         }

  return 88;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onStop(JNIEnv *env,
                                                             jobject obj) {
  qDebug() << "onStop";
  wxLogMessage(_T("onStop"));

  //  App may be summarily killed after this point due to OOM condition.
  //  So we need to persist some dynamic data.
  if (pConfig) {
    //  Persist the config file, especially to capture the viewport
    //  location,scale etc.
    pConfig->UpdateSettings();

    //  There may be unsaved objects at this point, and a navobj.xml.changes
    //  restore file We commit the navobj deltas, and flush the restore file
    //  Pass flag "true" to also recreate a new empty "changes" file
    pConfig->UpdateNavObj(true);
  }

  g_running = false;

  qDebug() << "onStop return 98";
  return 98;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onStart(JNIEnv *env,
                                                              jobject obj) {
  qDebug() << "onStart";
  wxLogMessage(_T("onStart"));

  if (g_bstress1) ShowNavWarning();

  g_running = true;

  return 99;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onPause(JNIEnv *env,
                                                              jobject obj) {
  qDebug() << "onPause";
  wxLogMessage(_T("onPause"));
  g_bSleep = true;

  callActivityMethod_is("setTrackContinuous", (int)g_btrackContinuous);

  if (!g_btrackContinuous) androidGPSService(GPS_OFF);

  return 97;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onResume(JNIEnv *env,
                                                               jobject obj) {
  qDebug() << "onResume";
  wxLogMessage(_T("onResume"));

  int ret = 96;

  g_bSleep = false;

  if (bGPSEnabled) androidGPSService(GPS_ON);

  wxCommandEvent evt0(wxEVT_COMMAND_MENU_SELECTED);
  evt0.SetId(ID_CMD_CLOSE_ALL_DIALOGS);
  if (gFrame && gFrame->GetEventHandler())
    gFrame->GetEventHandler()->AddPendingEvent(evt0);

  wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
  evt.SetId(ID_CMD_INVALIDATE);
  if (gFrame) gFrame->GetEventHandler()->AddPendingEvent(evt);

  //  Check screen orientation is sensible
  int orient = androidGetScreenOrientation();
  qDebug() << "Orient: " << orient;
  if (gFrame && gFrame->GetPrimaryCanvas()) {
    qDebug() << "Size: " << gFrame->GetSize().x << gFrame->GetSize().y;
    qDebug() << "CanvasSize: " << gFrame->GetPrimaryCanvas()->GetSize().x
             << gFrame->GetPrimaryCanvas()->GetSize().y;

    if (gFrame->GetSize().y > gFrame->GetSize().x) {
      qDebug() << "gFrame is Portrait";
      if ((orient == 2) || (orient == 4)) {
        qDebug() << "NEEDS RESIZE";
        GetAndroidDisplaySize();
        wxSize new_size = getAndroidDisplayDimensions();
        qDebug() << "NewSize: " << new_size.x << new_size.y;
        config_size = new_size;

        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        evt.SetId(ID_CMD_TRIGGER_RESIZE);
        if (g_androidUtilHandler) g_androidUtilHandler->AddPendingEvent(evt);
      }
    } else {
      qDebug() << "gFrame is Landscape";
      if ((orient == 1) || (orient == 3)) {
        qDebug() << "NEEDS RESIZE";
        GetAndroidDisplaySize();
        wxSize new_size = getAndroidDisplayDimensions();
        qDebug() << "NewSize: " << new_size.x << new_size.y;
        config_size = new_size;

        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
        evt.SetId(ID_CMD_TRIGGER_RESIZE);
        if (g_androidUtilHandler) g_androidUtilHandler->AddPendingEvent(evt);
      }
    }
  }

  return ret;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_onDestroy(JNIEnv *env,
                                                                jobject obj) {
  qDebug() << "onDestroy";
  wxLogMessage(_T("onDestroy"));

  if (pConfig) {
    //  Persist the config file, especially to capture the viewport
    //  location,scale, locale etc.
    pConfig->UpdateSettings();
  }

  g_running = false;

  return 98;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_selectChartDisplay(
    JNIEnv *env, jobject obj, int type, int family) {
  wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
  if (type == CHART_TYPE_CM93COMP) {
    evt.SetId(ID_CMD_SELECT_CHART_TYPE);
    evt.SetExtraLong(CHART_TYPE_CM93COMP);
  } else {
    evt.SetId(ID_CMD_SELECT_CHART_FAMILY);
    evt.SetExtraLong(family);
  }

  if (gFrame) gFrame->GetEventHandler()->AddPendingEvent(evt);

  return 74;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_invokeCmdEventCmdString(
    JNIEnv *env, jobject obj, int cmd_id, jstring s) {
  const char *sparm;
  wxString wx_sparm;
  JNIEnv *jenv;

  //  Need a Java environment to decode the string parameter
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    sparm = (jenv)->GetStringUTFChars(s, NULL);
    wx_sparm = wxString(sparm, wxConvUTF8);
  }

  // qDebug() << "invokeCmdEventCmdString" << cmd_id << s;

  wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
  evt.SetId(cmd_id);
  evt.SetString(wx_sparm);

  if (gFrame) {
    qDebug() << "add event" << cmd_id << wx_sparm.mbc_str();
    gFrame->GetEventHandler()->AddPendingEvent(evt);
  } else
    qDebug() << "No frame for EventCmdString";

  return 71;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_invokeMenuItem(
    JNIEnv *env, jobject obj, int item) {
  if (!gFrame)  // App Frame not yet set up, on slow devices
    return 71;

  wxString msg1;
  msg1.Printf(_T("invokeMenuItem: %d"), item);
  wxLogMessage(msg1);

  // If in Route Create, disable all other menu items
  if (gFrame && (gFrame->GetFocusCanvas()->m_routeState > 1) &&
      (OCPN_ACTION_ROUTE != item)) {
    wxLogMessage(_T("invokeMenuItem A"));
    return 72;
  }

  wxLogMessage(_T("invokeMenuItem B"));

  wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);

  switch (item) {
    case OCPN_ACTION_FOLLOW:
      evt.SetId(ID_MENU_NAV_FOLLOW);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_ROUTE:
      evt.SetId(ID_MENU_ROUTE_NEW);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_RMD:
      evt.SetId(ID_MENU_ROUTE_MANAGER);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_SETTINGS_BASIC:
      evt.SetId(ID_MENU_SETTINGS_BASIC);
      wxLogMessage(_T("invokeMenuItem OCPN_ACTION_SETTINGS_BASIC"));
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_TRACK_TOGGLE:
      evt.SetId(ID_MENU_NAV_TRACK);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_MOB:
      evt.SetId(ID_MENU_MARK_MOB);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_TIDES_TOGGLE:
      evt.SetId(ID_MENU_SHOW_TIDES);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_CURRENTS_TOGGLE:
      evt.SetId(ID_MENU_SHOW_CURRENTS);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_ENCTEXT_TOGGLE:
      evt.SetId(ID_MENU_ENC_TEXT);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_ENCSOUNDINGS_TOGGLE:
      evt.SetId(ID_MENU_ENC_SOUNDINGS);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    case OCPN_ACTION_ENCLIGHTS_TOGGLE:
      evt.SetId(ID_MENU_ENC_LIGHTS);
      gFrame->GetEventHandler()->AddPendingEvent(evt);
      break;

    default:
      break;
  }

  return 73;
}
}

extern "C" {
JNIEXPORT jstring JNICALL
Java_org_opencpn_OCPNNativeLib_getVPCorners(JNIEnv *env, jobject obj) {
  //        qDebug() << "getVPCorners";

  wxString s;

  if (gFrame->GetPrimaryCanvas()) {
    LLBBox vbox;
    vbox = gFrame->GetPrimaryCanvas()->GetVP().GetBBox();
    s.Printf(_T("%g;%g;%g;%g;"), vbox.GetMaxLat(), vbox.GetMaxLon(),
             vbox.GetMinLat(), vbox.GetMinLon());
  }

  jstring ret = (env)->NewStringUTF(s.c_str());

  return ret;
}
}

extern "C" {
JNIEXPORT jstring JNICALL Java_org_opencpn_OCPNNativeLib_getVPS(JNIEnv *env,
                                                                jobject obj) {
  wxString s;

  if (gFrame->GetPrimaryCanvas()) {
    ViewPort vp = gFrame->GetPrimaryCanvas()->GetVP();
    s.Printf(_T("%g;%g;%g;%g;%g;"), vp.clat, vp.clon, vp.view_scale_ppm, gLat,
             gLon);
  }

  jstring ret = (env)->NewStringUTF(s.c_str());

  return ret;
}
}

extern "C" {
JNIEXPORT int JNICALL Java_org_opencpn_OCPNNativeLib_getTLWCount(JNIEnv *env,
                                                                 jobject obj) {
  int ret = 0;
  wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
  while (node) {
    wxWindow *win = node->GetData();
    if (win->IsShown() && !win->IsKindOf(CLASSINFO(CanvasOptions))) ret++;

    node = node->GetNext();
  }
  return ret;
}
}

extern "C" {
JNIEXPORT int JNICALL Java_org_opencpn_OCPNNativeLib_notifyFullscreenChange(
    JNIEnv *env, jobject obj, bool bFull) {
  g_bFullscreen = bFull;
  return 1;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_setDownloadStatus(
    JNIEnv *env, jobject obj, int status, jstring url) {
  //        qDebug() << "setDownloadStatus";

  const char *sparm;
  wxString wx_sparm;
  JNIEnv *jenv;

  //  Need a Java environment to decode the string parameter
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    sparm = (jenv)->GetStringUTFChars(url, NULL);
    wx_sparm = wxString(sparm, wxConvUTF8);
  }

  if (s_bdownloading && wx_sparm.IsSameAs(s_requested_url)) {
    //            qDebug() << "Maybe mine...";
    //  We simply pass the event on to the core download manager methods,
    //  with parameters crafted to the event
    OCPN_downloadEvent ev(wxEVT_DOWNLOAD_EVENT, 0);

    OCPN_DLCondition dl_condition = OCPN_DL_EVENT_TYPE_UNKNOWN;
    OCPN_DLStatus dl_status = OCPN_DL_UNKNOWN;

    //  Translate Android status values to OCPN
    switch (status) {
      case 16:  // STATUS_FAILED
        dl_condition = OCPN_DL_EVENT_TYPE_END;
        dl_status = OCPN_DL_FAILED;
        break;

      case 8:  // STATUS_SUCCESSFUL
        dl_condition = OCPN_DL_EVENT_TYPE_END;
        dl_status = OCPN_DL_NO_ERROR;
        break;

      case 4:  //  STATUS_PAUSED
      case 2:  //  STATUS_RUNNING
      case 1:  //  STATUS_PENDING
        dl_condition = OCPN_DL_EVENT_TYPE_PROGRESS;
        dl_status = OCPN_DL_NO_ERROR;
    }

    ev.setDLEventCondition(dl_condition);
    ev.setDLEventStatus(dl_status);

    if (s_download_evHandler) {
      //                qDebug() << "Sending event...";
      s_download_evHandler->AddPendingEvent(ev);
    }
  }

  return 77;
}
}

extern "C" {
JNIEXPORT jint JNICALL Java_org_opencpn_OCPNNativeLib_sendPluginMessage(
    JNIEnv *env, jobject obj, jstring msgID, jstring msg) {
  const char *sparm;
  wxString MsgID;
  wxString Msg;
  JNIEnv *jenv;

  //  Need a Java environment to decode the string parameter
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    sparm = (jenv)->GetStringUTFChars(msgID, NULL);
    MsgID = wxString(sparm, wxConvUTF8);

    sparm = (jenv)->GetStringUTFChars(msg, NULL);
    Msg = wxString(sparm, wxConvUTF8);
  }

  SendPluginMessage(MsgID, Msg);

  return 74;
}
}

void androidTerminate() { callActivityMethod_vs("terminateApp"); }

bool CheckPendingJNIException() {
  JNIEnv *jenv;

  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) return true;

  if ((jenv)->ExceptionCheck() == JNI_TRUE) {
    // Handle exception here.
    (jenv)->ExceptionDescribe();  // writes to logcat
    (jenv)->ExceptionClear();

    return false;  // There was a pending exception, but cleared OK
                   // interesting discussion:
                   // http://blog.httrack.com/blog/2013/08/23/catching-posix-signals-on-android/
  }

  return false;
}

wxString callActivityMethod_vs(const char *method) {
  if (CheckPendingJNIException()) return _T("NOK");

  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod(method, "()Ljava/lang/String;");
  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();
  // qDebug() << s;

  if (s) {
    //  Need a Java environment to decode the resulting string
    if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
      // qDebug() << "GetEnv failed.";
    } else {
      const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
      return_string = wxString(ret_string, wxConvUTF8);
    }
  }

  return return_string;
}

wxString callActivityMethod_is(const char *method, int parm) {
  if (CheckPendingJNIException()) return _T("NOK");
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod(method, "(I)Ljava/lang/String;", parm);
  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_iis(const char *method, int parm1, int parm2) {
  if (CheckPendingJNIException()) return _T("NOK");

  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod(method, "(II)Ljava/lang/String;", parm1, parm2);
  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_ss(const char *method, wxString parm) {
  if (CheckPendingJNIException()) return _T("NOK");
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  jstring p = (jenv)->NewStringUTF(parm.c_str());

  //  Call the desired method
  // qDebug() << "Calling method_ss";
  // qDebug() << method;

  QAndroidJniObject data = activity.callObjectMethod(
      method, "(Ljava/lang/String;)Ljava/lang/String;", p);

  (jenv)->DeleteLocalRef(p);

  if (CheckPendingJNIException()) return _T("NOK");

  // qDebug() << "Back from method_ss";

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_s2s(const char *method, const wxString parm1,
                                const wxString parm2) {
  if (CheckPendingJNIException()) return _T("NOK");
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  wxCharBuffer p1b = parm1.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  wxCharBuffer p2b = parm2.ToUTF8();
  jstring p2 = (jenv)->NewStringUTF(p2b.data());

  //  Call the desired method
  // qDebug() << "Calling method_s2s" << " (" << method << ")";

  QAndroidJniObject data = activity.callObjectMethod(
      method, "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1,
      p2);

  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);

  if (CheckPendingJNIException()) return _T("NOK");

  // qDebug() << "Back from method_s2s";

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_s3s(const char *method, wxString parm1,
                                wxString parm2, wxString parm3) {
  if (CheckPendingJNIException()) return _T("NOK");
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    return _T("jenv Error");
  }

  wxCharBuffer p1b = parm1.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  wxCharBuffer p2b = parm2.ToUTF8();
  jstring p2 = (jenv)->NewStringUTF(p2b.data());

  wxCharBuffer p3b = parm3.ToUTF8();
  jstring p3 = (jenv)->NewStringUTF(p3b.data());

  //  Call the desired method
  // qDebug() << "Calling method_s3s" << " (" << method << ")";

  QAndroidJniObject data =
      activity.callObjectMethod(method,
                                "(Ljava/lang/String;Ljava/lang/String;Ljava/"
                                "lang/String;)Ljava/lang/String;",
                                p1, p2, p3);
  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);
  (jenv)->DeleteLocalRef(p3);

  if (CheckPendingJNIException()) return _T("NOK");

  // qDebug() << "Back from method_s3s";

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_s4s(const char *method, wxString parm1,
                                wxString parm2, wxString parm3,
                                wxString parm4) {
  if (CheckPendingJNIException()) return _T("NOK");
  JNIEnv *jenv;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
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

  // const char *ts = (jenv)->GetStringUTFChars(p2, NULL);
  // qDebug() << "Test String p2" << ts;

  //  Call the desired method
  // qDebug() << "Calling method_s4s" << " (" << method << ")";

  QAndroidJniObject data = activity.callObjectMethod(
      method,
      "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/"
      "String;)Ljava/lang/String;",
      p1, p2, p3, p4);
  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);
  (jenv)->DeleteLocalRef(p3);
  (jenv)->DeleteLocalRef(p4);

  if (CheckPendingJNIException()) return _T("NOK");

  // qDebug() << "Back from method_s4s";

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_s2s2i(const char *method, wxString parm1,
                                  wxString parm2, int parm3, int parm4) {
  if (CheckPendingJNIException()) return _T("NOK");

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  JNIEnv *jenv;

  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  wxCharBuffer p1b = parm1.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  wxCharBuffer p2b = parm2.ToUTF8();
  jstring p2 = (jenv)->NewStringUTF(p2b.data());

  // qDebug() << "Calling method_s2s2i" << " (" << method << ")";
  // qDebug() << parm3 << parm4;

  QAndroidJniObject data = activity.callObjectMethod(
      method, "(Ljava/lang/String;Ljava/lang/String;II)Ljava/lang/String;", p1,
      p2, parm3, parm4);

  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);

  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString callActivityMethod_ssi(const char *method, wxString parm1, int parm2) {
  if (CheckPendingJNIException()) return _T("NOK");

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return _T("NOK");

  if (!activity.isValid()) {
    return return_string;
  }

  //  Need a Java environment to decode the resulting string
  JNIEnv *jenv;

  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
    return _T("jenv Error");
  }

  wxCharBuffer p1b = parm1.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  QAndroidJniObject data = activity.callObjectMethod(
      method, "(Ljava/lang/String;I)Ljava/lang/String;", p1, parm2);

  (jenv)->DeleteLocalRef(p1);

  if (CheckPendingJNIException()) return _T("NOK");

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

wxString androidGetAndroidSystemLocale() {
  return callActivityMethod_vs("getAndroidLocaleString");
}

bool androidGetFullscreen() {
  wxString s = callActivityMethod_vs("getFullscreen");

  return s == _T("YES");
}

bool androidSetFullscreen(bool bFull) {
  callActivityMethod_is("setFullscreen", (int)bFull);

  return true;
}

void androidDisableFullScreen() {
  if (g_bFullscreen) {
    g_bFullscreenSave = true;
    androidSetFullscreen(false);
  }
}

void androidRestoreFullScreen() {
  if (g_bFullscreenSave) {
    g_bFullscreenSave = false;
    androidSetFullscreen(true);
  }
}

int androidGetScreenOrientation() {
  wxString s = callActivityMethod_vs("getScreenOrientation");
  long result = -1;
  s.ToLong(&result);
  return result;
}

void androidLaunchHelpView() {
  qDebug() << "androidLaunchHelpView ";
  wxString val = callActivityMethod_vs("isHelpAvailable");
  if (val.IsSameAs(_T("YES"))) {
    callActivityMethod_vs("launchHelpBook");
  } else {
    wxString msg =
        _("OpenCPN Help is not installed.\nWould you like to install from "
          "Google PlayStore now?");
    if (androidShowSimpleYesNoDialog(_T("OpenCPN"), msg))
      androidInstallPlaystoreHelp();
  }
}

void androidLaunchBrowser(wxString URL) {
  qDebug() << "androidLaunchBrowser";
  callActivityMethod_ss("launchWebView", URL);
}

void androidDisplayTimedToast(wxString message, int timeMillisec) {
  callActivityMethod_ssi("showTimedToast", message, timeMillisec);
}

void androidCancelTimedToast() { callActivityMethod_vs("cancelTimedToast"); }

void androidDisplayToast(wxString message) {
  callActivityMethod_ss("showToast", message);
}

void androidEnableRotation(void) {
  //    if(g_detect_smt590)
  //        return;

  callActivityMethod_vs("EnableRotation");
}

void androidDisableRotation(void) { callActivityMethod_vs("DisableRotation"); }

bool androidShowDisclaimer(wxString title, wxString msg) {
  if (CheckPendingJNIException()) return false;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return false;

  if (!activity.isValid()) return false;

  JNIEnv *jenv;

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) return false;

  wxCharBuffer p1b = title.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  // Convert for wxString-UTF8  to jstring-UTF16
  wxWCharBuffer b = msg.wc_str();
  jstring p2 = (jenv)->NewString((jchar *)b.data(), msg.Len() * 2);

  QAndroidJniObject data = activity.callObjectMethod(
      "disclaimerDialog",
      "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1, p2);

  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);

  if (CheckPendingJNIException()) return false;

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return (return_string == _T("OK"));
}

bool androidShowSimpleOKDialog(wxString title, wxString msg) {
  if (CheckPendingJNIException()) return false;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return false;

  if (!activity.isValid()) return false;

  JNIEnv *jenv;

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) return false;

  wxCharBuffer p1b = title.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  // Convert for wxString-UTF8  to jstring-UTF16
  wxWCharBuffer b = msg.wc_str();
  jstring p2 = (jenv)->NewString((jchar *)b.data(), msg.Len() * 2);

  QAndroidJniObject data = activity.callObjectMethod(
      "simpleOKDialog",
      "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1, p2);

  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);

  if (CheckPendingJNIException()) return false;

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return (return_string == _T("OK"));
}

bool androidShowSimpleYesNoDialog(wxString title, wxString msg) {
  if (CheckPendingJNIException()) return false;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return false;

  if (!activity.isValid()) return false;

  JNIEnv *jenv;

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) return false;

  wxCharBuffer p1b = title.ToUTF8();
  jstring p1 = (jenv)->NewStringUTF(p1b.data());

  // Convert for wxString-UTF8  to jstring-UTF16
  wxWCharBuffer b = msg.wc_str();
  jstring p2 = (jenv)->NewString((jchar *)b.data(), msg.Len() * 2);

  QAndroidJniObject data = activity.callObjectMethod(
      "simpleYesNoDialog",
      "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", p1, p2);

  (jenv)->DeleteLocalRef(p1);
  (jenv)->DeleteLocalRef(p2);

  if (CheckPendingJNIException()) return false;

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return (return_string == _T("YES"));
}

bool androidInstallPlaystoreHelp() {
  qDebug() << "androidInstallPlaystoreHelp";
  //  return false;

  if (CheckPendingJNIException()) return false;

  wxString return_string;
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (CheckPendingJNIException()) return false;

  if (!activity.isValid()) return false;

  JNIEnv *jenv;

  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) return false;

  QAndroidJniObject data =
      activity.callObjectMethod("installPlaystoreHelp", "()Ljava/lang/String;");

  if (CheckPendingJNIException()) return false;

  jstring s = data.object<jstring>();

  if ((jenv)->GetStringLength(s)) {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return (return_string == _T("OK"));
}

int androidGetTZOffsetMins() {
  // Get the TZ offset (from UTC) of the local machine, in minutes.  Includes
  // DST, if applicable
  wxString result = callActivityMethod_vs("getAndroidTZOffsetMinutes");
  qDebug() << "androidGetTZOffsetMins result: " << result.mb_str();
  long value = 0;
  result.ToLong(&value);
  return (int)value;
}

extern PlatSpec android_plat_spc;

wxString androidGetDeviceInfo() {
  if (!g_deviceInfo.Length())
    g_deviceInfo = callActivityMethod_vs("getDeviceInfo");

  wxStringTokenizer tkz(g_deviceInfo, _T("\n"));
  while (tkz.HasMoreTokens()) {
    wxString s1 = tkz.GetNextToken();
    if (wxNOT_FOUND != s1.Find(_T("OS API Level"))) {
      int a = s1.Find(_T("{"));
      if (wxNOT_FOUND != a) {
        wxString b = s1.Mid(a + 1, 2);
        memset(android_plat_spc.msdk, 0, sizeof(android_plat_spc.msdk));
        strncpy(android_plat_spc.msdk, b.c_str(), 2);
        g_Android_SDK_Version = atoi( android_plat_spc.msdk );
      }
    }
    if (wxNOT_FOUND != s1.Find(_T("opencpn"))) {
      strcpy(&android_plat_spc.hn[0], s1.c_str());
    }

    // Look for some specific device identifiers, for special processing as
    // implemented.

    // (1) Samsung SM-T590 running Android/10{29}
    if (wxNOT_FOUND != s1.Find(_T("SM-T590"))) {
      if (!strncmp(android_plat_spc.msdk, "29",
                   2))  // Assumes API comes before Model/Product.
        g_detect_smt590 = true;
    }
  }

  return g_deviceInfo;
}

bool androidIsDirWritable( wxString dir )
{
  if (g_Android_SDK_Version < 30)
    return true;
  else{
    // This is theorectically most accurate, but slow to execute
    //wxString result = callActivityMethod_ss("isDirWritable", dir);
    //return (result.IsSameAs("YES"));

    // This is a practical alternative, for things like chart storage qualification.
    return (dir.Contains("org.opencpn.opencpn"));
  }
}

wxString androidGetHomeDir() { return g_androidFilesDir + _T("/"); }

wxString
androidGetPrivateDir()  // Used for logfile, config file, navobj, and the like
{
  if (g_bExternalApp) {
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

  // We choose to use the ExtFiles directory always , so that the contents of
  // logfiles, navobjs, etc. may always be accessible by simple Android File
  // Explorers...
  return g_androidExtFilesDir;
}

wxString androidGetSharedDir()  // Used for assets like uidata, s57data, etc
{
  //    if(g_bExternalApp){
  //        if(g_androidExtFilesDir.Length())
  //            return g_androidExtFilesDir + _T("/");
  //    }

  return g_androidExtFilesDir + _T("/");
}

wxString
androidGetCacheDir()  // Used for raster_texture_cache, mmsitoname.csv, etc
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
 * Traditionally this is an SD card, but it may also be implemented as built-in
 * storage in a device that is distinct from the protected internal storage and
 * can be mounted as a filesystem on a computer.
 */

wxString androidGetExtStorageDir()  // Used for Chart storage, typically
{
  if (g_Android_SDK_Version >= 30)
    return g_androidExtFilesDir;      // Scoped storage model
  else
    return g_androidExtStorageDir;
}

extern void androidSetRouteAnnunciator(bool viz) {
  callActivityMethod_is("setRouteAnnunciator", viz ? 1 : 0);
}

extern void androidSetFollowTool(int state, bool forceUpdate) {
  //    qDebug() << "setFollowIconState" << bactive;

  if ((g_follow_state != state) || forceUpdate)
    callActivityMethod_is("setFollowIconState", state);

  g_follow_state = state;
}

extern void androidSetTrackTool(bool bactive) {
  if (g_track_active != bactive)
    callActivityMethod_is("setTrackIconState", bactive ? 1 : 0);

  g_track_active = bactive;
}

void androidSetChartTypeMaskSel(int mask, wxString &indicator) {
  int sel = 0;
  if (wxNOT_FOUND != indicator.Find(_T("raster")))
    sel = 1;
  else if (wxNOT_FOUND != indicator.Find(_T("vector")))
    sel = 2;
  else if (wxNOT_FOUND != indicator.Find(_T("cm93")))
    sel = 4;

  if ((g_mask != mask) || (g_sel != sel)) {
    //        qDebug() << "androidSetChartTypeMaskSel" << mask << sel;
    callActivityMethod_iis("configureNavSpinnerTS", mask, sel);
    g_mask = mask;
    g_sel = sel;
  }
}

void androidEnableBackButton(bool benable) {
  callActivityMethod_is("setBackButtonState", benable ? 1 : 0);
  g_backEnabled = benable;
}

void androidEnableBackButtonCheck(bool benable) {
  if (g_backEnabled != benable) androidEnableBackButton(benable);
}

void androidEnableOptionItems(bool benable) {
  callActivityMethod_is("enableOptionItemAction", benable ? 1 : 0);
}

void androidEnableMulticast(bool benable) {
  callActivityMethod_is("enableMulticast", benable ? 1 : 0);
}

void androidLastCall(void) {
  CheckMigrateCharts();
  callActivityMethod_is("lastCallOnInit", 1);
}

bool androidGetMemoryStatus(int *mem_total, int *mem_used) {
  //  On android, We arbitrarily declare that we have used 50% of available
  //  memory.
  if (mem_total) *mem_total = s_androidMemTotal * 1024;
  if (mem_used) *mem_used = s_androidMemUsed * 1024;
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

double GetAndroidDisplaySize() {
  double ret = 200.;  // sane default

  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return false;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod("getDisplayMetrics", "()Ljava/lang/String;");

  wxString return_string;
  jstring s = data.object<jstring>();

  JNIEnv *jenv;
  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  //  Return string may have commas instead of periods, if using Euro locale
  //  We just fix it here...
  return_string.Replace(_T(","), _T("."));

  wxSize screen_size = wxSize(1, 2);

  if (QApplication::desktop()) {  // Desktop might not yet be initialized
    screen_size = ::wxGetDisplaySize();
  }

  wxString msg;

  // int ssx, ssy;
  //::wxDisplaySize(&ssx, &ssy);
  // msg.Printf(_T("wxDisplaySize(): %d %d"), ssx, ssy);
  // wxLogMessage(msg);

  double density = 1.0;
  long androidWidth = 2;
  long androidHeight = 1;
  long androidDmWidth = 2;
  long androidDmHeight = 1;
  long abh = 1;

  wxStringTokenizer tk(return_string, _T(";"));
  if (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();  // xdpi
    token = tk.GetNextToken();           // density

    token.ToDouble(&density);

    token = tk.GetNextToken();  // ldpi

    token = tk.GetNextToken();  // width
    token.ToLong(&androidWidth);
    token = tk.GetNextToken();  // height - statusBarHeight
    token = tk.GetNextToken();  // width
    token = tk.GetNextToken();  // height
    token.ToLong(&androidHeight);

    token = tk.GetNextToken();  // dm.widthPixels
    token.ToLong(&androidDmWidth);
    token = tk.GetNextToken();  // dm.heightPixels
    token.ToLong(&androidDmHeight);

    token = tk.GetNextToken();  // actionBarHeight
    token.ToLong(&abh);
  }

  double ldpi = 160. * density;
  if (ldpi < 160) ldpi = 160.;

  // Find the max dimension among all possibilities
//   double maxDim = wxMax(screen_size.x, screen_size.y);
//   maxDim = wxMax(maxDim, androidHeight);
//   maxDim = wxMax(maxDim, androidWidth);

  double maxDim = screen_size.x;
  maxDim = wxMax(maxDim, androidWidth);

  ret = (maxDim / ldpi) * 25.4;

  if (ret < 50) {  // 2 inches wide is too small....
    double ret_bad = ret;
    ret = 100;
    msg.Printf(
        _T("WARNING: Android Auto Display Size OVERRIDE_TOO_SMALL: %g  ldpi: ")
        _T("%g  density: %g correctedsize: %g "),
        ret_bad, ldpi, density, ret);
  } else if (ret > 400) {  // Too large
    double ret_bad = ret;
    ret = 400;
    msg.Printf(
        _T("WARNING: Android Auto Display Size OVERRIDE_TOO_LARGE: %g  ldpi: ")
        _T("%g  density: %g corrected size: %g"),
        ret_bad, ldpi, density, ret);
  } else {
    msg.Printf(
        _T("Android Auto Display Size (mm, est.): %g   ldpi: %g  density: %g"),
        ret, ldpi, density);
  }

  //  Save some items as global statics for convenience
  g_androidDPmm = ldpi / 25.4;
  g_androidDensity = density;
  g_ActionBarHeight = wxMax(abh, 50);

  // qDebug() << "GetAndroidDisplaySize" << ldpi << g_androidDPmm;

  return ret;
}

int getAndroidActionBarHeight() { return g_ActionBarHeight; }

double getAndroidDPmm() {
  // Returns an estimate based on the pixel density reported
  GetAndroidDisplaySize();

  // qDebug() << "getAndroidDPmm" << g_androidDPmm;

  // User override?
  if (g_config_display_size_manual && (g_config_display_size_mm > 0)) {
    double maxDim = wxMax(::wxGetDisplaySize().x, ::wxGetDisplaySize().y);
    double size_mm = g_config_display_size_mm;
    size_mm = wxMax(size_mm, 50);
    size_mm = wxMin(size_mm, 400);
    double ret = maxDim / size_mm;
    // qDebug() << "getAndroidDPmm override" << maxDim << size_mm <<
    // g_config_display_size_mm;

    return ret;
  }

  if (g_androidDPmm > 0.01)
    return g_androidDPmm;
  else
    return 160. / 25.4;
}

double getAndroidDisplayDensity() {
  if (g_androidDensity < 0.01) {
    GetAndroidDisplaySize();
  }

  //    qDebug() << "g_androidDensity" << g_androidDensity;

  if (g_androidDensity > 0.01)
    return g_androidDensity;
  else
    return 1.0;
}

wxSize getAndroidDisplayDimensions(void) {
  wxSize sz_ret = ::wxGetDisplaySize();  // default, probably reasonable, but
                                         // maybe not accurate

  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return sz_ret;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod("getDisplayMetrics", "()Ljava/lang/String;");

  wxString return_string;
  jstring s = data.object<jstring>();

  //  Need a Java environment to decode the resulting string
  JNIEnv *jenv;
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  // 167.802994;1.000000;160;1024;527;1024;552;1024;552;56
  wxStringTokenizer tk(return_string, _T(";"));
  if (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();  // xdpi
    token = tk.GetNextToken();           // density
    token = tk.GetNextToken();           // densityDPI

    token = tk.GetNextToken();
    long a = 1000;
    if (token.ToLong(&a)) sz_ret.x = a;

    token = tk.GetNextToken();
    long b = 1000;
    if (token.ToLong(&b)) sz_ret.y = b;
    token = tk.GetNextToken();
    token = tk.GetNextToken();

    token = tk.GetNextToken();
    token = tk.GetNextToken();

    long abh = 0;
    token = tk.GetNextToken();  //  ActionBar height, if shown
    if (token.ToLong(&abh)) sz_ret.y -= abh;
  }

  // Samsung sm-t590/Android 10 has some display problems in portrait mode.....
  if (g_detect_smt590) {
    if (sz_ret.x < sz_ret.y) sz_ret.y = 1650;
  }

  // qDebug() << "getAndroidDisplayDimensions" << sz_ret.x << sz_ret.y;

  return sz_ret;
}

void androidConfirmSizeCorrection() {
  //  There is some confusion about the ActionBar size during configuration
  //  changes. We need to confirm the calculated display size, and fix it if
  //  necessary. This happens during staged resize events

  wxLogMessage(_T("androidConfirmSizeCorrection"));
  wxSize targetSize = getAndroidDisplayDimensions();
  qDebug() << "Confirming" << targetSize.y << config_size.y;
  if (config_size != targetSize) {
    qDebug() << "Correcting";
    gFrame->SetSize(targetSize);
    config_size = targetSize;
  }
}

void androidForceFullRepaint(bool b_skipConfirm) {
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
  evt.SetId(ID_CMD_TRIGGER_RESIZE);
  if (gFrame && gFrame->GetEventHandler()) {
    g_androidUtilHandler->AddPendingEvent(evt);
  }
}

void androidShowBusyIcon() {
  if (b_androidBusyShown) return;

  // qDebug() << "ShowBusy";

  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");
  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod("showBusyCircle", "()Ljava/lang/String;");

  b_androidBusyShown = true;
}

void androidHideBusyIcon() {
  if (!b_androidBusyShown) return;

  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return;
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod("hideBusyCircle", "()Ljava/lang/String;");

  b_androidBusyShown = false;
}

int androidGetVersionCode() {
  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return false;
  }

  //  Call the desired method
  QAndroidJniObject data = activity.callObjectMethod("getAndroidVersionCode",
                                                     "()Ljava/lang/String;");

  wxString return_string;
  jstring s = data.object<jstring>();

  JNIEnv *jenv;
  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  long rv;
  return_string.ToLong(&rv);

  return rv;
}

wxString androidGetVersionName() {
  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return _T("ERROR");
  }

  //  Call the desired method
  QAndroidJniObject data = activity.callObjectMethod("getAndroidVersionName",
                                                     "()Ljava/lang/String;");

  wxString return_string;
  jstring s = data.object<jstring>();

  JNIEnv *jenv;
  //  Need a Java environment to decode the resulting string
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

//---------------------------------------------------------------
//      GPS Device Support
//---------------------------------------------------------------
bool androidDeviceHasGPS() {
  wxString query = androidGPSService(GPS_PROVIDER_AVAILABLE);
  wxLogMessage(query);

  bool result = query.Upper().IsSameAs(_T("YES"));
  if (result) {
    // qDebug() << "Android Device has internal GPS";
    wxLogMessage(_T("Android Device has internal GPS"));
  } else {
    // qDebug() << "Android Device has NO internal GPS";
    wxLogMessage(_T("Android Device has NO internal GPS"));
  }
  return result;
}

bool androidStartNMEA(wxEvtHandler *consumer) {
  s_pAndroidNMEAMessageConsumer = consumer;

  // qDebug() << "androidStartNMEA";
  wxString s;

  s = androidGPSService(GPS_ON);
  wxLogMessage(s);
  if (s.Upper().Find(_T("DISABLED")) != wxNOT_FOUND) {
    OCPNMessageBox(
        NULL, _("Your android device has an internal GPS, but it is disabled.\n\
                       Please visit android Settings/Location dialog to enable GPS"),
        _T("OpenCPN"), wxOK);

    androidStopNMEA();
    return false;
  } else
    bGPSEnabled = true;

  return true;
}

bool androidStopNMEA() {
  s_pAndroidNMEAMessageConsumer = NULL;

  wxString s = androidGPSService(GPS_OFF);

  bGPSEnabled = false;

  return true;
}

wxString androidGPSService(int parm) {
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return _T("Activity is not valid");
  }

  //  Call the desired method
  QAndroidJniObject data = activity.callObjectMethod(
      "queryGPSServer", "(I)Ljava/lang/String;", parm);

  wxString return_string;
  jstring s = data.object<jstring>();

  if (s == NULL) return return_string;

  //  Need a Java environment to decode the resulting string
  JNIEnv *jenv;
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    return_string = wxString(ret_string, wxConvUTF8);
  }

  return return_string;
}

bool androidDeviceHasBlueTooth() {
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return _T("Activity is not valid");
  }

  //  Call the desired method
  QAndroidJniObject data =
      activity.callObjectMethod("hasBluetooth", "(I)Ljava/lang/String;", 0);

  wxString query;
  jstring s = data.object<jstring>();

  //  Need a Java environment to decode the resulting string
  JNIEnv *jenv;
  if (java_vm->GetEnv((void **)&jenv, JNI_VERSION_1_6) != JNI_OK) {
    // qDebug() << "GetEnv failed.";
  } else {
    const char *ret_string = (jenv)->GetStringUTFChars(s, NULL);
    query = wxString(ret_string, wxConvUTF8);
  }

  bool result = query.Upper().IsSameAs(_T("YES"));

  if (result) {
    // qDebug() << "Android Device has internal Bluetooth";
    wxLogMessage(_T("Android Device has internal Bluetooth"));
  } else {
    // qDebug() << "Android Device has NO internal Bluetooth";
    wxLogMessage(_T("Android Device has NO internal Bluetooth"));
  }

  return result;
}

bool androidStartBluetoothScan() {
  wxString result = callActivityMethod_is("startBlueToothScan", 0);

  return true;
}

bool androidStopBluetoothScan() {
  wxString result = callActivityMethod_is("stopBlueToothScan", 0);

  return true;
}

bool androidStartBT(wxEvtHandler *consumer, wxString mac_address) {
  s_pAndroidBTNMEAMessageConsumer = consumer;

  if (mac_address.Find(':') ==
      wxNOT_FOUND)  //  does not look like a mac address
    return false;

  wxString result = callActivityMethod_ss("startBTService", mac_address);

  return true;
}

bool androidStopBT() {
  // qDebug() << "androidStopBT";

  s_pAndroidBTNMEAMessageConsumer = NULL;

  wxString result = callActivityMethod_is("stopBTService", 0);

  return true;
}

wxArrayString androidGetBluetoothScanResults() {
  wxArrayString ret_array;

  wxString result = callActivityMethod_is("getBlueToothScanResults", 0);

  wxStringTokenizer tk(result, _T(";"));
  while (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();
    ret_array.Add(token);
  }

  if (!ret_array.GetCount()) ret_array.Add(_("Nothing found"));

  return ret_array;
}

bool androidSendBTMessage(wxString &payload) {
  wxString result = callActivityMethod_ss("sendBTMessage", payload);

  return true;
}

bool androidCheckOnline() {
  wxString val = callActivityMethod_vs("isNetworkAvailable");
  return val.IsSameAs(_T("YES"));
}

wxArrayString *androidGetSerialPortsArray(void) {
  wxArrayString *pret_array = new wxArrayString;
  wxString result = callActivityMethod_is("scanSerialPorts", 0);

  wxStringTokenizer tk(result, _T(";"));
  while (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();
    pret_array->Add(token);
  }

  return pret_array;
}

bool androidStartUSBSerial(wxString &portname, wxString baudRate,
                           wxEvtHandler *consumer) {
  wxString result =
      callActivityMethod_s2s("startSerialPort", portname, baudRate);

  s_pAndroidNMEAMessageConsumer = consumer;

  return true;
}

bool androidStopUSBSerial(wxString &portname) {
  s_pAndroidNMEAMessageConsumer = NULL;

  //  If app is closing down, the USB serial ports will go away automatically.
  //  So no need here.
  //  In fact, stopSerialPort() causes an occasional error when closing app.
  //  Dunno why, difficult to debug.
  if (!b_inCloseWindow)
    wxString result = callActivityMethod_ss("stopSerialPort", portname);

  return true;
}

bool androidWriteSerial(wxString &portname, wxString &message) {
  wxString result =
      callActivityMethod_s2s("writeSerialPort", portname, message);
  return true;
}

int androidFileChooser(wxString *result, const wxString &initDir,
                       const wxString &title, const wxString &suggestion,
                       const wxString &wildcard, bool dirOnly, bool addFile) {
  wxString tresult;

  //  Start a timer to poll for results
  if (g_androidUtilHandler) {
    g_androidUtilHandler->m_eventTimer.Stop();
    g_androidUtilHandler->m_done = false;

    wxString activityResult;
    if (dirOnly)
      activityResult = callActivityMethod_s2s2i("DirChooserDialog", initDir,
                                                title, addFile, 0);

    else
      activityResult = callActivityMethod_s4s("FileChooserDialog", initDir,
                                              title, suggestion, wildcard);

    if (activityResult == _T("OK")) {
      // qDebug() << "ResultOK, starting spin loop";
      g_androidUtilHandler->m_action = ACTION_FILECHOOSER_END;
      g_androidUtilHandler->m_eventTimer.Start(1000, wxTIMER_CONTINUOUS);

      //  Spin, waiting for result
      while (!g_androidUtilHandler->m_done) {
        wxMilliSleep(50);
        wxSafeYield(NULL, true);
      }

      // qDebug() << "out of spin loop";
      g_androidUtilHandler->m_action = ACTION_NONE;
      g_androidUtilHandler->m_eventTimer.Stop();

      tresult = g_androidUtilHandler->GetStringResult();

      if (tresult.StartsWith(_T("cancel:"))) {
        // qDebug() << "Cancel1";
        return wxID_CANCEL;
      } else if (tresult.StartsWith(_T("file:"))) {
        if (result) {
          *result = tresult.AfterFirst(':');
          // qDebug() << "OK";
          return wxID_OK;
        } else {
          // qDebug() << "Cancel2";
          return wxID_CANCEL;
        }
      }
    } else {
      // qDebug() << "Result NOT OK";
    }
  }

  return wxID_CANCEL;
}

bool InvokeJNIPreferences(wxString &initial_settings) {
  bool ret = true;
  wxCharBuffer abuf = initial_settings.ToUTF8();
  if (!abuf.data()) return false;

  //  Create the method parameter(s)
  QAndroidJniObject param1 = QAndroidJniObject::fromString(abuf.data());

  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    // qDebug() << "Activity is not valid";
    return false;
  }

  //  Call the desired method
  activity.callObjectMethod("doAndroidSettings",
                            "(Ljava/lang/String;)Ljava/lang/String;",
                            param1.object<jstring>());

  return ret;
}

wxString BuildAndroidSettingsString(void) {
  wxString result;

  //  Start with chart dirs
  if (ChartData) {
    wxArrayString chart_dir_array = ChartData->GetChartDirArrayString();

    for (unsigned int i = 0; i < chart_dir_array.GetCount(); i++) {
      result += _T("ChartDir:");
      result += chart_dir_array[i];
      result += _T(";");
    }
  }

  //  Now the simple Boolean parameters
  result += _T("prefb_lookahead:") +
            wxString(g_bLookAhead == 1 ? _T("1;") : _T("0;"));
  result +=
      _T("prefb_quilt:") + wxString(g_bQuiltEnable == 1 ? _T("1;") : _T("0;"));
  result += _T("prefb_showgrid:") +
            wxString(g_bDisplayGrid == 1 ? _T("1;") : _T("0;"));
  result += _T("prefb_showoutlines:") +
            wxString(g_bShowOutlines == 1 ? _T("1;") : _T("0;"));
  result += _T("prefb_showdepthunits:") +
            wxString(g_bShowDepthUnits == 1 ? _T("1;") : _T("0;"));
  result += _T("prefb_lockwp:") +
            wxString(g_bWayPointPreventDragging == 1 ? _T("1;") : _T("0;"));
  result += _T("prefb_confirmdelete:") +
            wxString(g_bConfirmObjectDelete == 1 ? _T("1;") : _T("0;"));
  result += _T("prefb_expertmode:") +
            wxString(g_bUIexpert == 1 ? _T("1;") : _T("0;"));

  if (ps52plib) {
    result += _T("prefb_showlightldesc:") +
              wxString(ps52plib->m_bShowLdisText == 1 ? _T("1;") : _T("0;"));
    result += _T("prefb_showimptext:") +
              wxString(ps52plib->m_bShowS57ImportantTextOnly == 1 ? _T("1;")
                                                                  : _T("0;"));
    result += _T("prefb_showSCAMIN:") +
              wxString(ps52plib->m_bUseSCAMIN == 1 ? _T("1;") : _T("0;"));
    result += _T("prefb_showsound:") +
              wxString(ps52plib->m_bShowSoundg == 1 ? _T("1;") : _T("0;"));
    result += _T("prefb_showATONLabels:") +
              wxString(ps52plib->m_bShowAtonText == 1 ? _T("1;") : _T("0;"));
  }
  // Some other assorted values
  result += _T("prefs_navmode:") +
            wxString(g_bCourseUp == 0 ? _T("North Up;") : _T("Course Up;"));
  result += _T("prefs_chartInitDir:") + *pInit_Chart_Dir + _T(";");

  wxString s;
  double sf = (g_GUIScaleFactor * 10) + 50.;
  s.Printf(_T("%3.0f;"), sf);
  s.Trim(false);
  result += _T("prefs_UIScaleFactor:") + s;

  sf = (g_ChartScaleFactor * 10) + 50.;
  s.Printf(_T("%3.0f;"), sf);
  s.Trim(false);
  result += _T("prefs_chartScaleFactor:") + s;

  if (ps52plib) {
    wxString nset = _T("Base");
    switch (ps52plib->GetDisplayCategory()) {
      case (DISPLAYBASE):
        nset = _T("Base;");
        break;
      case (STANDARD):
        nset = _T("Standard;");
        break;
      case (OTHER):
        nset = _T("All;");
        break;
      case (MARINERS_STANDARD):
        nset = _T("Mariner Standard;");
        break;
      default:
        nset = _T("Base;");
        break;
    }
    result += _T("prefs_displaycategory:") + nset;

    if (ps52plib->m_nSymbolStyle == PAPER_CHART)
      nset = _T("Paper Chart;");
    else
      nset = _T("Simplified;");
    result += _T("prefs_vectorgraphicsstyle:") + nset;

    if (ps52plib->m_nBoundaryStyle == PLAIN_BOUNDARIES)
      nset = _T("Plain;");
    else
      nset = _T("Symbolized;");
    result += _T("prefs_vectorboundarystyle:") + nset;

    if (S52_getMarinerParam(S52_MAR_TWO_SHADES) == 1.0)
      nset = _T("2;");
    else
      nset = _T("4;");
    result += _T("prefs_vectorchartcolors:") + nset;

    // depth unit conversion factor

    float conv = 1;
    int depthUnit = ps52plib->m_nDepthUnitDisplay;
    if (depthUnit == 0)  // feet
      conv = 0.3048f;    // international definiton of 1 foot is 0.3048 metres
    else if (depthUnit == 2)  // fathoms
      conv = 0.3048f * 6;     // 1 fathom is 6 feet

    s.Printf(_T("%4.2f;"), S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR) / conv);
    s.Trim(false);
    result += _T("prefs_shallowdepth:") + s;

    s.Printf(_T("%4.2f;"), S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR) / conv);
    s.Trim(false);
    result += _T("prefs_safetydepth:") + s;

    s.Printf(_T("%4.2f;"), S52_getMarinerParam(S52_MAR_DEEP_CONTOUR) / conv);
    s.Trim(false);
    result += _T("prefs_deepdepth:") + s;

    //  Scale slider range from -5 -- 5 in OCPN options.
    //  On Android, the range is 0 -- 100
    //  So, convert
  }

  // Connections

  // Internal GPS.
  for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
    ConnectionParams *cp = g_pConnectionParams->Item(i);
    if (INTERNAL_GPS == cp->Type) {
      result += _T("prefb_internalGPS:");
      result += cp->bEnabled ? _T("1;") : _T("0;");
    }
    if (SERIAL == cp->Type) {
      if (wxNOT_FOUND != cp->GetPortStr().Find(_T("PL2303"))) {
        result += _T("prefb_PL2303:");
        result += cp->bEnabled ? _T("1;") : _T("0;");
      } else if (wxNOT_FOUND != cp->GetPortStr().Find(_T("dAISy"))) {
        result += _T("prefb_dAISy:");
        result += cp->bEnabled ? _T("1;") : _T("0;");
      } else if (wxNOT_FOUND != cp->GetPortStr().Find(_T("FT232R"))) {
        result += _T("prefb_FT232R:");
        result += cp->bEnabled ? _T("1;") : _T("0;");
      } else if (wxNOT_FOUND != cp->GetPortStr().Find(_T("FT231X"))) {
        result += _T("prefb_FT231X:");
        result += cp->bEnabled ? _T("1;") : _T("0;");
      } else if (wxNOT_FOUND != cp->GetPortStr().Find(_T("USBDP"))) {
        result += _T("prefb_USBDP:");
        result += cp->bEnabled ? _T("1;") : _T("0;");
      }
    }
  }

  wxLogMessage(_T("BuildAndroidSettingsString: ") + result);

  return result;
}

const wxString AUSBNames[] = {
    _T("AUSBSerial:Prolific_PL2303"), _T("AUSBSerial:FTDI_FT232R"),
    _T("AUSBSerial:FTDI_FT231X"),     _T("AUSBSerial:dAISy"),
    _T("AUSBSerial:USBDP"),           _T("LASTENTRY")};
const wxString AUSBPrefs[] = {_T("prefb_PL2303"), _T("prefb_FT232R"),
                              _T("prefb_FT231X"), _T("prefb_dAISy"),
                              _T("prefb_USBDP"),  _T("LASTENTRY")};

int androidApplySettingsString(wxString settings, ArrayOfCDI *pACDI) {
  //  Parse the passed settings string
  bool bproc_InternalGPS = false;
  bool benable_InternalGPS = false;

  int rr = GENERIC_CHANGED;

  // extract chart directories

  if (ChartData) {
    wxStringTokenizer tkd(settings, _T(";"));
    while (tkd.HasMoreTokens()) {
      wxString token = tkd.GetNextToken();

      if (token.StartsWith(_T("ChartDir"))) {
        wxString dir = token.AfterFirst(':');
        if (dir.Length()) {
          ChartDirInfo cdi;
          cdi.fullpath = dir.Trim();
          cdi.magic_number = ChartData->GetMagicNumberCached(dir.Trim());
          pACDI->Add(cdi);
        }
      }
    }

    // Scan for changes
    if (!ChartData->CompareChartDirArray(*pACDI)) {
      rr |= VISIT_CHARTS;
      rr |= CHANGE_CHARTS;
      wxLogMessage(_T("Chart Dir List change detected"));
    }
  }

  wxStringTokenizer tk(settings, _T(";"));
  while (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();
    wxString val = token.AfterFirst(':');

    //  Binary switches

    if (token.StartsWith(_T("prefb_lookahead"))) {
      g_bLookAhead = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefb_quilt"))) {
      g_bQuiltEnable = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefb_lockwp"))) {
      g_bWayPointPreventDragging = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefb_showdepthunits"))) {
      g_bShowDepthUnits = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefb_confirmdelete"))) {
      g_bConfirmObjectDelete = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefb_showgrid"))) {
      g_bDisplayGrid = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefb_showoutlines"))) {
      g_bShowOutlines = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefb_expertmode"))) {
      g_bUIexpert = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefb_internalGPS"))) {
      bproc_InternalGPS = true;
      benable_InternalGPS = val.IsSameAs(_T("1"));
    } else if (token.StartsWith(_T("prefs_navmode"))) {
      g_bCourseUp = val.IsSameAs(_T("Course Up"));
    } else if (token.StartsWith(_T("prefb_trackOnPause"))) {
      g_btrackContinuous = val.IsSameAs(_T("1"));
    }

    //  Strings, etc.

    else if (token.StartsWith(_T("prefs_UIScaleFactor"))) {
      double a;
      if (val.ToDouble(&a)) g_GUIScaleFactor = wxRound((a / 10.) - 5.);
    }

    else if (token.StartsWith(_T("prefs_chartScaleFactor"))) {
      double a;
      if (val.ToDouble(&a)) {
        g_ChartScaleFactor = wxRound((a / 10.) - 5.);
        g_ChartScaleFactorExp =
            g_Platform->getChartScaleFactorExp(g_ChartScaleFactor);
      }
    }

    else if (token.StartsWith(_T("prefs_chartInitDir"))) {
      *pInit_Chart_Dir = val;
    }

    if (ps52plib) {
      float conv = 1;
      int depthUnit = ps52plib->m_nDepthUnitDisplay;
      if (depthUnit == 0)  // feet
        conv = 0.3048f;    // international definiton of 1 foot is 0.3048 metres
      else if (depthUnit == 2)  // fathoms
        conv = 0.3048f * 6;     // 1 fathom is 6 feet

      if (token.StartsWith(_T("prefb_showsound"))) {
        bool old_val = ps52plib->m_bShowSoundg;
        ps52plib->m_bShowSoundg = val.IsSameAs(_T("1"));
        if (old_val != ps52plib->m_bShowSoundg) rr |= S52_CHANGED;
      } else if (token.StartsWith(_T("prefb_showSCAMIN"))) {
        bool old_val = ps52plib->m_bUseSCAMIN;
        ps52plib->m_bUseSCAMIN = val.IsSameAs(_T("1"));
        if (old_val != ps52plib->m_bUseSCAMIN) rr |= S52_CHANGED;
      } else if (token.StartsWith(_T("prefb_showimptext"))) {
        bool old_val = ps52plib->m_bShowS57ImportantTextOnly;
        ps52plib->m_bShowS57ImportantTextOnly = val.IsSameAs(_T("1"));
        if (old_val != ps52plib->m_bShowS57ImportantTextOnly) rr |= S52_CHANGED;
      } else if (token.StartsWith(_T("prefb_showlightldesc"))) {
        bool old_val = ps52plib->m_bShowLdisText;
        ps52plib->m_bShowLdisText = val.IsSameAs(_T("1"));
        if (old_val != ps52plib->m_bShowLdisText) rr |= S52_CHANGED;
      } else if (token.StartsWith(_T("prefb_showATONLabels"))) {
        bool old_val = ps52plib->m_bShowAtonText;
        ps52plib->m_bShowAtonText = val.IsSameAs(_T("1"));
        if (old_val != ps52plib->m_bShowAtonText) rr |= S52_CHANGED;
      }

      else if (token.StartsWith(_T("prefs_displaycategory"))) {
        _DisCat old_nset = ps52plib->GetDisplayCategory();

        _DisCat nset = DISPLAYBASE;
        if (wxNOT_FOUND != val.Lower().Find(_T("base")))
          nset = DISPLAYBASE;
        else if (wxNOT_FOUND != val.Lower().Find(_T("mariner")))
          nset = MARINERS_STANDARD;
        else if (wxNOT_FOUND != val.Lower().Find(_T("standard")))
          nset = STANDARD;
        else if (wxNOT_FOUND != val.Lower().Find(_T("all")))
          nset = OTHER;

        if (nset != old_nset) {
          rr |= S52_CHANGED;
          ps52plib->SetDisplayCategory(nset);
        }
      }

      else if (token.StartsWith(_T("prefs_shallowdepth"))) {
        double old_dval = S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR);
        double dval;
        if (val.ToDouble(&dval)) {
          if (fabs(dval - old_dval) > .001) {
            S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval * conv);
            rr |= S52_CHANGED;
          }
        }
      }

      else if (token.StartsWith(_T("prefs_safetydepth"))) {
        double old_dval = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);
        double dval;
        if (val.ToDouble(&dval)) {
          if (fabs(dval - old_dval) > .001) {
            S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, dval * conv);
            rr |= S52_CHANGED;
          }
        }
      }

      else if (token.StartsWith(_T("prefs_deepdepth"))) {
        double old_dval = S52_getMarinerParam(S52_MAR_DEEP_CONTOUR);
        double dval;
        if (val.ToDouble(&dval)) {
          if (fabs(dval - old_dval) > .001) {
            S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval * conv);
            rr |= S52_CHANGED;
          }
        }
      }

      else if (token.StartsWith(_T("prefs_vectorgraphicsstyle"))) {
        LUPname old_LUP = ps52plib->m_nSymbolStyle;

        if (wxNOT_FOUND != val.Lower().Find(_T("paper")))
          ps52plib->m_nSymbolStyle = PAPER_CHART;
        else if (wxNOT_FOUND != val.Lower().Find(_T("simplified")))
          ps52plib->m_nSymbolStyle = SIMPLIFIED;

        if (old_LUP != ps52plib->m_nSymbolStyle) rr |= S52_CHANGED;

      }

      else if (token.StartsWith(_T("prefs_vectorboundarystyle"))) {
        LUPname old_LUP = ps52plib->m_nBoundaryStyle;

        if (wxNOT_FOUND != val.Lower().Find(_T("plain")))
          ps52plib->m_nBoundaryStyle = PLAIN_BOUNDARIES;
        else if (wxNOT_FOUND != val.Lower().Find(_T("symbolized")))
          ps52plib->m_nBoundaryStyle = SYMBOLIZED_BOUNDARIES;

        if (old_LUP != ps52plib->m_nBoundaryStyle) rr |= S52_CHANGED;

      }

      else if (token.StartsWith(_T("prefs_vectorchartcolors"))) {
        double old_dval = S52_getMarinerParam(S52_MAR_TWO_SHADES);

        if (wxNOT_FOUND != val.Lower().Find(_T("2")))
          S52_setMarinerParam(S52_MAR_TWO_SHADES, 1.);
        else if (wxNOT_FOUND != val.Lower().Find(_T("4")))
          S52_setMarinerParam(S52_MAR_TWO_SHADES, 0.);

        double new_dval = S52_getMarinerParam(S52_MAR_TWO_SHADES);
        if (fabs(new_dval - old_dval) > .1) {
          rr |= S52_CHANGED;
        }
      }
    }
  }

  // Process Internal GPS Connection
  if (g_pConnectionParams && bproc_InternalGPS) {
    //  Does the connection already exist?
    ConnectionParams *pExistingParams = NULL;
    ConnectionParams *cp = NULL;

    for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
      ConnectionParams *xcp = g_pConnectionParams->Item(i);
      if (INTERNAL_GPS == xcp->Type) {
        pExistingParams = xcp;
        cp = xcp;
        break;
      }
    }

    bool b_action = true;
    if (pExistingParams) {
      if (pExistingParams->bEnabled == benable_InternalGPS)
        b_action = false;  // nothing to do...
      else
        cp->bEnabled = benable_InternalGPS;
    } else if (benable_InternalGPS) {  //  Need a new Params
      // make a generic config string for InternalGPS.
      wxString sGPS = _T("2;3;;0;0;;0;1;0;0;;0;;1;0;0;0;0");  // 17 parms
      ConnectionParams *new_params = new ConnectionParams(sGPS);

      new_params->bEnabled = benable_InternalGPS;
      g_pConnectionParams->Add(new_params);
      cp = new_params;
    }

    if (b_action && cp) {  // something to do?
//FIXME (dave)
#if 0

      // Terminate and remove any existing stream with the same port name
      DataStream *pds_existing = g_pMUX->FindStream(cp->GetDSPort());
      if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);

      if (cp->bEnabled) {
        dsPortType port_type = cp->IOSelect;
        DataStream *dstr =
            makeSerialDataStream(g_pMUX, cp->Type, cp->GetDSPort(),
                                 wxString::Format(wxT("%i"), cp->Baudrate),
                                 port_type, cp->Priority, cp->Garmin);

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
#endif
    }
  }

  // Process USB Serial Connections
  bool b_newGlobalSettings = false;
  if (g_pConnectionParams) {
    int i = 0;
    while (wxNOT_FOUND == AUSBPrefs[i].Find(_T("LASTENTRY"))) {
      wxStringTokenizer tk(settings, _T(";"));
      while (tk.HasMoreTokens()) {
        wxString token = tk.GetNextToken();
        wxString pref = token.BeforeFirst(':');
        wxString val = token.AfterFirst(':');
        wxString extraString;

        bool benabled = false;

        if (pref.IsSameAs(AUSBPrefs[i])) {
          wxLogMessage(_T("pref: ") + pref);
          wxLogMessage(_T("val: ") + val);

          if (pref.Contains(_T("USBDP"))) {
            extraString = val.AfterFirst(':');
            wxLogMessage(_T("extra: ") + extraString);
          }

          wxLogMessage(_T("found pref ") + pref);

          //  Does the connection already exist?
          ConnectionParams *pExistingParams = NULL;
          ConnectionParams *cp = NULL;

          wxString target = AUSBNames[i] + _T("-") + extraString;

          for (unsigned int j = 0; j < g_pConnectionParams->Count(); j++) {
            ConnectionParams *xcp = g_pConnectionParams->Item(j);
            wxLogMessage(_T("    Checking: ") + target + " .. " +
                         xcp->GetDSPort());

            if ((SERIAL == xcp->Type) &&
                (target.IsSameAs(xcp->GetDSPort().AfterFirst(':')))) {
              pExistingParams = xcp;
              cp = xcp;
              benabled = val.BeforeFirst(':').IsSameAs(_T("1"));
              break;
            }
          }

          bool b_action = true;
          if (pExistingParams) {
            wxLogMessage(_T("Using existing connection  ") + target);

            if (pExistingParams->bEnabled == benabled) {
              b_action = false;  // nothing to do...
            } else
              cp->bEnabled = benabled;
          } else if (val.BeforeFirst(':').IsSameAs(
                         _T("1"))) {  //  Need a new Params
            // make a generic config string.
            // 0;1;;0;0;/dev/ttyS0;4800;1;0;0;;0;;1;0;0;0;0        17 parms

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

          if (b_action && cp) {  // something to do?
//FIXME (dave)
#if 0
            rr |= NEED_NEW_OPTIONS;

            // Terminate and remove any existing stream with the same port name
            DataStream *pds_existing = g_pMUX->FindStream(cp->GetDSPort());
            if (pds_existing) g_pMUX->StopAndRemoveStream(pds_existing);

            if (cp->bEnabled) {
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
              DataStream *dstr = makeSerialDataStream(
                  g_pMUX, cp->Type, cp->GetDSPort(),
                  wxString::Format(wxT("%i"), cp->Baudrate), port_type,
                  cp->Priority, cp->Garmin);

              dstr->SetInputFilter(cp->InputSentenceList);
              dstr->SetInputFilterType(cp->InputSentenceListType);
              dstr->SetOutputFilter(cp->OutputSentenceList);
              dstr->SetOutputFilterType(cp->OutputSentenceListType);
              dstr->SetChecksumCheck(cp->ChecksumCheck);

              g_pMUX->AddStream(dstr);
              cp->b_IsSetup = true;
            }
#endif
          }
        }
      }  // found pref

      i++;
    }  // while
  }

  return rr;
}

bool DoAndroidPreferences(void) {
  wxLogMessage(_T("Start DoAndroidPreferences"));

  wxString settings = BuildAndroidSettingsString();

  wxLogMessage(_T("Call InvokeJNIPreferences"));
  InvokeJNIPreferences(settings);

  return true;
}

wxString doAndroidPOST(const wxString &url, wxString &parms, int timeoutMsec) {
  //  Start a timer to poll for results
  if (g_androidUtilHandler) {
    g_androidUtilHandler->m_eventTimer.Stop();
    g_androidUtilHandler->m_done = false;

    androidShowBusyIcon();

    wxString stimeout;
    stimeout.Printf(_T("%d"), timeoutMsec);
    wxString result =
        callActivityMethod_s3s("doHttpPostAsync", url, parms, stimeout);

    if (result == _T("OK")) {
      qDebug() << "doHttpPostAsync ResultOK, starting spin loop";
      g_androidUtilHandler->m_action = ACTION_POSTASYNC_END;
      g_androidUtilHandler->m_eventTimer.Start(500, wxTIMER_CONTINUOUS);

      //  Spin, waiting for result
      while (!g_androidUtilHandler->m_done) {
        wxMilliSleep(50);
        wxSafeYield(NULL, true);
      }

      qDebug() << "out of spin loop";
      g_androidUtilHandler->m_action = ACTION_NONE;
      g_androidUtilHandler->m_eventTimer.Stop();
      androidHideBusyIcon();

      wxString presult = g_androidUtilHandler->GetStringResult();

      return presult;
    } else {
      qDebug() << "doHttpPostAsync Result NOT OK";
      androidHideBusyIcon();
    }
  }

  return wxEmptyString;
}

int validateAndroidWriteLocation(const wxString &destination) {
  // validate the destination, as it might be on SDCard
  wxString val_result =
      callActivityMethod_s2s2i("validateWriteLocation", destination, _T(""),
                               OCPN_ACTION_DOWNLOAD_VALID, 0);
  if (val_result.IsSameAs(_T("Pending")))
    return 0;  //  SAF Dialog is going to run
  else
    return 1;  // All well.
}

int startAndroidFileDownload(const wxString &url, const wxString &destination,
                             wxEvtHandler *evh, long *dl_id) {
  //    if(evh)
  {
    s_bdownloading = true;
    s_requested_url = url;
    s_download_evHandler = evh;
    s_download_destination = destination;

    wxString result = callActivityMethod_s2s("downloadFile", url, destination);

    androidShowBusyIcon();

    if (result.IsSameAs(_T("NOK"))) return 1;  // general error

    //      wxLogMessage(_T("downloads2s result: ") + result);
    long dl_ID;
    wxStringTokenizer tk(result, _T(";"));
    if (tk.HasMoreTokens()) {
      wxString token = tk.GetNextToken();
      if (token.IsSameAs(_T("OK"))) {
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

int queryAndroidFileDownload(long dl_ID, wxString *result) {
  //    qDebug() << dl_ID;

  wxString stat = callActivityMethod_is("getDownloadStatus", (int)dl_ID);
  if (result) *result = stat;

  //    wxLogMessage( _T("queryAndroidFileDownload: ") + stat);

  if (stat.IsSameAs(_T("NOK")))
    return 1;  // general error
  else
    return 0;
}

void finishAndroidFileDownload(void) {
  s_bdownloading = false;
  s_requested_url.Clear();
  s_download_evHandler = NULL;
  s_download_destination.Clear();
  androidHideBusyIcon();

  return;
}

void cancelAndroidFileDownload(long dl_ID) {
  wxString stat = callActivityMethod_is("cancelDownload", (int)dl_ID);
}

bool AndroidUnzip(wxString &zipFile, wxString &destDir, int nStrip,
                  bool bRemoveZip) {
  wxString ns;
  ns.Printf(_T("%d"), nStrip);

  wxString br;
  br.Printf(_T("%d"), bRemoveZip);

  wxString stat = callActivityMethod_s4s("unzipFile", zipFile, destDir, ns, br);

  if (wxNOT_FOUND == stat.Find(_T("OK"))) return false;

  qDebug() << "unzip start";

  bool bDone = false;
  while (!bDone) {
    wxMilliSleep(1000);
    wxSafeYield(NULL, true);

    qDebug() << "unzip poll";

    wxString result = callActivityMethod_ss("getUnzipStatus", _T(""));
    if (wxNOT_FOUND != result.Find(_T("DONE"))) bDone = true;
  }
  qDebug() << "unzip done";

  return true;
}

wxString getFontQtStylesheet(wxFont *font) {
  // wxString classes = _T("QLabel, QPushButton, QTreeWidget, QTreeWidgetItem,
  // QCheckBox");
  wxString classes = _T("QWidget ");

  wxString qstyle = classes + _T("{  font-family: ") + font->GetFaceName() +
                    _T(";font-style: ");
  switch (font->GetStyle()) {
    case wxFONTSTYLE_ITALIC:
      qstyle += _T("italic;");
      break;
    case wxFONTSTYLE_NORMAL:
    default:
      qstyle += _T("normal;");
      break;
  }
  qstyle += _T("font-weight: ");
  switch (font->GetWeight()) {
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
  //  If this line is active, this particular style is applied to ListCtrl() in
  //  PlugIns, But not TreeCtrl.....
  //  ????
  // qstyle += _T("QTreeWidget::item{ border-color:red; border-style:outset;
  // border-width:2px; color:black; }");

  return qstyle;
}

bool androidPlaySound(const wxString soundfile, AndroidSound* sound) {
  DEBUG_LOG << "androidPlaySound";
  std::ostringstream oss;
  oss << sound;
  wxString wxSound(oss.str());
  wxString result = callActivityMethod_s2s("playSound", soundfile, wxSound.Mid(2));
  return true;
}

extern "C" {
JNIEXPORT jint JNICALL
Java_org_opencpn_OCPNNativeLib_onSoundFinished(JNIEnv *env, jobject obj) {
  qDebug() << "onSoundFinished";

  if (s_soundCallBack) {
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
    evt.SetId(ID_CMD_SOUND_FINISHED);
    if (g_androidUtilHandler) g_androidUtilHandler->AddPendingEvent(evt);
  }

  return 98;
}
}

wxString androidGetSupplementalLicense(void) {
  wxString result = callActivityMethod_vs("getGMAPILicense");

  result += AndroidSuppLicense;

  return result;
}

wxArrayString androidTraverseDir(wxString dir, wxString filespec) {
  wxArrayString result;
  if (g_Android_SDK_Version != 17)  // skip unless running Android 4.2.2, especially Samsung...
    return result;

  wxString ir =
      callActivityMethod_s2s("getAllFilesWithFilespec", dir, filespec);

  wxStringTokenizer tk(ir, _T(";"));
  while (tk.HasMoreTokens()) {
    result.Add(tk.GetNextToken());
  }

  return result;
}

void androidEnableOptionsMenu(bool bEnable) {
  callActivityMethod_is("enableOptionsMenu", bEnable ? 1 : 0);
}

//    Android specific style sheet management

//  ------------Runtime modified globals
QString qtStyleSheetDialog;
QString qtStyleSheetListBook;
QString qtStyleSheetScrollbars;

//--------------Stylesheet prototypes

//  Generic dialog stylesheet
//  Typically adjusted at runtime for display density

QString qtStyleSheetDialogProto =
    "\
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

QString qtStyleSheetScrollbarsProto =
    "\
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

std::string prepareStyleIcon(wxString icon_file, int size) {
  wxString data_locn = g_Platform->GetSharedDataDir();
  data_locn.Append(_T("styles/"));

  wxString file = data_locn + icon_file;

  wxImage Image(file, wxBITMAP_TYPE_PNG);
  wxImage scaledImage = Image.Scale(size, size, wxIMAGE_QUALITY_HIGH);

  wxString save_file = g_Platform->GetPrivateDataDir() + _T("/") + icon_file;
  scaledImage.SaveFile(save_file, wxBITMAP_TYPE_PNG);

  wxCharBuffer buf = save_file.ToUTF8();
  std::string ret(buf);
  return ret;
}

QString prepareAndroidSliderStyleSheet(int sliderWidth) {
  QString qtStyleSheetSlider;

  //  Create and fix up the qtStyleSheetDialog for generic dialog

  // adjust the Slider specification

  int slider_handle_width =
      wxMax(g_Platform->GetDisplayDPmm() * 6, sliderWidth / 5);

  char sb[600];
  snprintf(
      sb, sizeof(sb),
      "QSlider::groove { border: 1px solid #999999;  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E6E6E6, stop:1 #EEEEEE); } \
    QSlider::groove:disabled { background: #efefef; } \
    QSlider::handle { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #7cb0e9, stop:1 #7cb0e9); border: 1px solid #5c5c5c; \
    border-radius: 3px; width: %dpx; height: 45px; } \
    QSlider::handle:disabled { background: #D3D0CD;}",
      slider_handle_width);

  qtStyleSheetSlider.append(sb);

  return qtStyleSheetSlider;
}

void prepareAndroidStyleSheets() {
  //  Create and fix up the qtStyleSheetDialog for generic dialog
  qtStyleSheetDialog.clear();
  qtStyleSheetDialog.append(qtStyleSheetDialogProto);

  // add the Slider specification

  int slider_handle_width = g_Platform->GetDisplayDPmm() * 6;

  char sb[400];
  snprintf(
      sb, sizeof(sb),
      "QSlider::groove { border: 1px solid #999999;  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E6E6E6, stop:1 #EEEEEE); } \
    QSlider::groove:disabled { background: #efefef; } \
    QSlider::handle { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #7cb0e9, stop:1 #7cb0e9); border: 1px solid #5c5c5c; \
    border-radius: 3px; width: %dpx; height: 45px; } \
    QSlider::handle:disabled { background: #D3D0CD;}",
      slider_handle_width);

  qtStyleSheetDialog.append(sb);

  // add the checkbox specification
  int cbSize = 30 * getAndroidDisplayDensity();
  char cb[400];

  // icons
  // Checked box
  std::string cbs = prepareStyleIcon(_T("chek_full.png"), cbSize);
  //  Empty box
  std::string ucbs = prepareStyleIcon(_T("chek_empty.png"), cbSize);

  snprintf(cb, sizeof(cb),
           "QCheckBox { spacing: 25px;}\
    QCheckBox::indicator { width: %dpx;   height: %dpx;}\
    QCheckBox::indicator:checked {image: url(%s);}\
    QCheckBox::indicator:unchecked {image: url(%s);}",
           cbSize, cbSize, cbs.c_str(), ucbs.c_str());

  qtStyleSheetDialog.append(cb);

  //   The qTabBar buttons as in a listbook
  qtStyleSheetListBook.clear();

  // compute the tabbar button size
  int tbbSize = 50 * getAndroidDisplayDensity();
  char tbb[400];

  std::string tbbl = prepareStyleIcon(_T("tabbar_button_left.png"), tbbSize);
  std::string tbbr = prepareStyleIcon(_T("tabbar_button_right.png"), tbbSize);

  snprintf(tbb, sizeof(tbb),
           "QTabBar::scroller { width: %dpx; }\
    QTabBar QToolButton::right-arrow { image: url(%s); }\
    QTabBar QToolButton::left-arrow { image: url(%s); }",
           tbbSize, tbbr.c_str(), tbbl.c_str());

  qtStyleSheetListBook.append(tbb);

  // A simple stylesheet with scrollbars only
  qtStyleSheetScrollbars.clear();
  qtStyleSheetScrollbars.append(qtStyleSheetScrollbarsProto);
}

void setChoiceStyleSheet(wxChoice *win, int refDim) {
  // qDebug() << "refDim" << refDim;

  float fontDimFloat = ((float)refDim) * 0.5;
  int fontDim = (int)fontDimFloat;
  int pixRadius = refDim / 4;

  QString styleString;
  char sb[1400];

  //  This one control the appearance of the "un-dropped" control.
  snprintf(sb, sizeof(sb),
           "QComboBox { font-size: %dpx; font-weight: bold; min-height: %dpx; "
           "color: rgb(0,0,0); background-color: rgb(250,250,250); }",
           fontDim, refDim);
  styleString.append(sb);

  // This one controls the color and style of the drop list items
  snprintf(sb, sizeof(sb),
           "QComboBox QListView::item { color: rgb(0,0,0); background-color: "
           "rgb(95, 163, 237); }");
  styleString.append(sb);

  // This one controls the drop list font
  snprintf(sb, sizeof(sb),
           "QComboBox QAbstractItemView { font-size: %dpx; font-weight: bold;}",
           fontDim);
  styleString.append(sb);

  // This one is necessary to set min height of drop list items, otherwise they
  // are squished.
  snprintf(sb, sizeof(sb),
           "QComboBox QAbstractItemView::item {  min-height: %dpx; border: "
           "10px outset darkgray; border-radius: %dpx;  }",
           refDim, pixRadius);
  styleString.append(sb);

  // qDebug() << styleString;

  win->GetHandle()->setView(new QListView());  // Magic
  win->GetHandle()->setStyleSheet(styleString);
}

void setMenuStyleSheet(wxMenu *win, const wxFont &font) {
  if (!win) return;

  int points = font.GetPointSize();
  int fontPix = points / g_Platform->getFontPointsperPixel();

  // qDebug() << points << g_Platform->getFontPointsperPixel() << fontPix;

  QString styleString;
  char sb[1400];

  snprintf(sb, sizeof(sb), "QMenu { font: bold %dpx; }", fontPix);
  styleString.append(sb);

  snprintf(sb, sizeof(sb),
           "QMenu::separator { height: 4px; background: lightblue; "
           "margin-left: 10px; margin-right: 5px; }");
  styleString.append(sb);

  // qDebug() << styleString;

  win->GetHandle()->setStyleSheet(styleString);
}

QString getAdjustedDialogStyleSheet() { return qtStyleSheetDialog; }

QString getListBookStyleSheet() { return qtStyleSheetListBook; }

QString getScrollBarsStyleSheet() { return qtStyleSheetScrollbars; }

//      SVG Support
wxBitmap loadAndroidSVG(const wxString filename, unsigned int width,
                        unsigned int height) {
  wxCharBuffer abuf = filename.ToUTF8();
  if (abuf.data()) {  // OK conversion?
    std::string s(abuf.data());
    // qDebug() << "loadAndroidSVG" << s.c_str();
  } else {
    qDebug() << "loadAndroidSVG FAIL";
  }

  // Destination file location
  wxString save_file_dir =
      g_Platform->GetPrivateDataDir() + _T("/") + _T("icons");
  if (!wxDirExists(save_file_dir)) wxMkdir(save_file_dir);

  wxFileName fsvg(filename);
  wxFileName fn(save_file_dir + _T("/") + fsvg.GetFullName());
  fn.SetExt(_T("png"));

  /*
         //Caching does not work well, since we always build each icon twice.
      if(fn.FileExists()){
          wxBitmap bmp_test(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
          if(bmp_test.IsOk()){
              if((bmp_test.GetWidth() == (int)width) && (bmp_test.GetHeight() ==
     (int)height)) return bmp_test;
          }
      }
  */

  wxString val = callActivityMethod_s2s2i("buildSVGIcon", filename,
                                          fn.GetFullPath(), width, height);
  if (val == _T("OK")) {
    //        qDebug() << "OK";

    return wxBitmap(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  } else {
    return wxBitmap(width, height);
  }
}

void androidTestCPP() { callActivityMethod_vs("callFromCpp"); }

unsigned int androidColorPicker(unsigned int initialColor) {
  if (g_androidUtilHandler) {
    g_androidUtilHandler->m_eventTimer.Stop();
    g_androidUtilHandler->m_done = false;

    wxString val = callActivityMethod_is("doColorPickerDialog", initialColor);

    if (val == _T("OK")) {
      // qDebug() << "ResultOK, starting spin loop";
      g_androidUtilHandler->m_action = ACTION_COLORDIALOG_END;
      g_androidUtilHandler->m_eventTimer.Start(1000, wxTIMER_CONTINUOUS);

      //  Spin, waiting for result
      while (!g_androidUtilHandler->m_done) {
        wxMilliSleep(50);
        wxSafeYield(NULL, true);
      }

      // qDebug() << "out of spin loop";
      g_androidUtilHandler->m_action = ACTION_NONE;
      g_androidUtilHandler->m_eventTimer.Stop();

      wxString tresult = g_androidUtilHandler->GetStringResult();

      if (tresult.StartsWith(_T("cancel:"))) {
        // qDebug() << "Cancel1";
        return initialColor;
      } else if (tresult.StartsWith(_T("color:"))) {
        wxString color = tresult.AfterFirst(':');
        long a;
        color.ToLong(&a);
        unsigned int b = a;

        // char cc[30];
        // sprintf(cc, "%0X", b);
        // qDebug() << "OK " << cc;

        return b;
      }
    } else {
      qDebug() << "Result NOT OK";
    }
  }
  return 0;
}

bool AndroidSecureCopyFile(wxString in, wxString out) {
  bool bret = true;

  wxString result = callActivityMethod_s2s("SecureFileCopy", in, out);

  if (wxNOT_FOUND == result.Find(_T("OK"))) bret = false;

  return bret;
}

int doAndroidPersistState() {
  qDebug() << "doAndroidPersistState() starting...";
  wxLogMessage(_T("doAndroidPersistState() starting..."));

  // We save perspective before closing to restore position next time
  // Pane is not closed so the child is not notified (OnPaneClose)
  if (g_pauimgr) {
    if (g_pAISTargetList) {
      wxAuiPaneInfo &pane = g_pauimgr->GetPane(g_pAISTargetList);
      g_AisTargetList_perspective = g_pauimgr->SavePaneInfo(pane);
      g_pauimgr->DetachPane(g_pAISTargetList);

      pConfig->SetPath(_T ( "/AUI" ));
      pConfig->Write(_T ( "AUIPerspective" ), g_pauimgr->SavePerspective());
    }
  }

  //    Deactivate the PlugIns, allowing them to save state
  PluginLoader::getInstance()->DeactivateAllPlugIns();

  /*
   Automatically drop an anchorage waypoint, if enabled
   On following conditions:
   1.  In "Cruising" mode, meaning that speed has at some point exceeded 3.0
   kts.
   2.  Current speed is less than 0.5 kts.
   3.  Opencpn has been up at least 30 minutes
   4.  And, of course, opencpn is going down now.
   5.  And if there is no anchor watch set on "anchor..." icon mark           //
   pjotrc 2010.02.15
   */
  if (g_bAutoAnchorMark) {
    bool watching_anchor = false;  // pjotrc 2010.02.15
    if (pAnchorWatchPoint1)        // pjotrc 2010.02.15
      watching_anchor = (pAnchorWatchPoint1->GetIconName().StartsWith(
          _T("anchor")));    // pjotrc 2010.02.15
    if (pAnchorWatchPoint2)  // pjotrc 2010.02.15
      watching_anchor |= (pAnchorWatchPoint2->GetIconName().StartsWith(
          _T("anchor")));  // pjotrc 2010.02.15

    wxDateTime now = wxDateTime::Now();
    wxTimeSpan uptime = now.Subtract(g_start_time);

    if (!watching_anchor && (g_bCruising) && (gSog < 0.5) &&
        (uptime.IsLongerThan(wxTimeSpan(0, 30, 0, 0))))  // pjotrc 2010.02.15
    {
      //    First, delete any single anchorage waypoint closer than 0.25 NM from
      //    this point This will prevent clutter and database congestion....

      wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
      while (node) {
        RoutePoint *pr = node->GetData();
        if (pr->GetName().StartsWith(_T("Anchorage"))) {
          double a = gLat - pr->m_lat;
          double b = gLon - pr->m_lon;
          double l = sqrt((a * a) + (b * b));

          // caveat: this is accurate only on the Equator
          if ((l * 60. * 1852.) < (.25 * 1852.)) {
            pConfig->DeleteWayPoint(pr);
            pSelect->DeleteSelectablePoint(pr, SELTYPE_ROUTEPOINT);
            delete pr;
            break;
          }
        }

        node = node->GetNext();
      }

      wxString name = now.Format();
      name.Prepend(_("Anchorage created "));
      RoutePoint *pWP =
          new RoutePoint(gLat, gLon, _T("anchorage"), name, _T(""));
      pWP->m_bShowName = false;
      pWP->m_bIsolatedMark = true;

      pConfig->AddNewWayPoint(pWP, -1);  // use auto next num
    }
  }

  if (gFrame->GetPrimaryCanvas()->GetpCurrentStack()) {
    g_restore_stackindex =
        gFrame->GetPrimaryCanvas()->GetpCurrentStack()->CurrentStackEntry;
    g_restore_dbindex = gFrame->GetPrimaryCanvas()
                            ->GetpCurrentStack()
                            ->GetCurrentEntrydbIndex();
    if (gFrame->GetPrimaryCanvas() &&
        gFrame->GetPrimaryCanvas()->GetQuiltMode())
      g_restore_dbindex =
          gFrame->GetPrimaryCanvas()->GetQuiltReferenceChartIndex();
  }

  if (g_MainToolbar) {
    wxPoint tbp = g_MainToolbar->GetPosition();
    wxPoint tbp_incanvas = gFrame->GetPrimaryCanvas()->ScreenToClient(tbp);
    g_maintoolbar_x = tbp_incanvas.x;
    g_maintoolbar_y = tbp_incanvas.y;
    g_maintoolbar_orient = g_MainToolbar->GetOrient();
  }

  if (g_iENCToolbar) {
    wxPoint locn = g_iENCToolbar->GetPosition();
    wxPoint tbp_incanvas = gFrame->GetPrimaryCanvas()->ScreenToClient(locn);
    g_iENCToolbarPosY = tbp_incanvas.y;
    g_iENCToolbarPosX = tbp_incanvas.x;
  }

  pConfig->UpdateSettings();
  pConfig->UpdateNavObj();

  pConfig->m_pNavObjectChangesSet->reset();

  // Remove any leftover Routes and Waypoints from config file as they were
  // saved to navobj before
  pConfig->DeleteGroup(_T ( "/Routes" ));
  pConfig->DeleteGroup(_T ( "/Marks" ));
  pConfig->Flush();

  delete pConfig;  // All done
  pConfig = NULL;
  pBaseConfig = NULL;

  //    Unload the PlugIns
  //      Note that we are waiting until after the canvas is destroyed,
  //      since some PlugIns may have created children of canvas.
  //      Such a PlugIn must stay intact for the canvas dtor to call
  //      DestoryChildren()

  if (ChartData) ChartData->PurgeCachePlugins();

  PluginLoader::getInstance()->UnLoadAllPlugIns();
  if (g_pi_manager) {
    delete g_pi_manager;
    g_pi_manager = NULL;
  }

  wxLogMessage(_T("doAndroidPersistState() finished cleanly."));
  qDebug() << "doAndroidPersistState() finished cleanly.";

  wxLogMessage(_T("Closing logfile, Terminating App."));

  wxLog::FlushActive();
  g_Platform->CloseLogFile();

  return 0;
}

extern "C" {
JNIEXPORT int JNICALL
Java_org_opencpn_OCPNNativeLib_ScheduleCleanExit(JNIEnv *env, jobject obj) {
  qDebug() << "Java_org_opencpn_OCPNNativeLib_ScheduleCleanExit";
  wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
  evt.SetId(SCHEDULED_EVENT_CLEAN_EXIT);
  if (g_androidUtilHandler) {
    g_androidUtilHandler->AddPendingEvent(evt);
  }

  return 1;
}
}

void CheckMigrateCharts()
{
  qDebug() << "CheckMigrateCharts";
  if (g_Android_SDK_Version < 30)   // Only on Android/11 +
    return;

  // Force access to correct home directory, as a hint....
  pInit_Chart_Dir->Clear();

  // Scan the config file chart directory array.
  wxArrayString chartDirs = GetConfigChartDirectories(); //GetChartDirArrayString();
  wxArrayString migrateDirs;
  qDebug() << chartDirs.GetCount();

  for (unsigned int i=0; i < chartDirs.GetCount(); i++){
    qDebug() << chartDirs[i].mb_str();

    bool bOK = false;
    if ( chartDirs[i].StartsWith(g_androidGetFilesDirs0) )
      bOK = true;

    else if (!g_androidGetFilesDirs1.StartsWith("?")){
      if ( chartDirs[i].StartsWith(g_androidGetFilesDirs1) )
        bOK = true;
    }
    if (!bOK) {
      migrateDirs.Add(chartDirs[i]);
    }
  }

  if (!migrateDirs.GetCount())
    return;


  // Run the chart migration assistant
  g_migrateDialog = new MigrateAssistantDialog(gFrame, false);
  g_migrateDialog->SetSize( gFrame->GetSize());
  g_migrateDialog->Centre();
  g_migrateDialog->Raise();
  g_migrateDialog->ShowModal();


}

wxString androidGetDownloadDirectory()
{
  return g_androidDownloadDirectory;
}



wxString WrapText(wxWindow *win, const wxString& text, int widthMax)
{
    class HardBreakWrapper : public wxTextWrapper
    {
    public:
        HardBreakWrapper(wxWindow *win, const wxString& text, int widthMax)
        {
            Wrap(win, text, widthMax);
        }
        wxString const& GetWrapped() const { return m_wrapped; }
    protected:
        virtual void OnOutputLine(const wxString& line)
        {
            m_wrapped += line;
        }
        virtual void OnNewLine()
        {
            m_wrapped += '\n';
        }
    private:
        wxString m_wrapped;
    };
    HardBreakWrapper wrapper(win, text, widthMax);
    return wrapper.GetWrapped();
}

///////////////////////////////////////////////////////////////////////////////
/// Class MigrateAssistantDialog Implementation
///////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MigrateAssistantDialog, wxDialog)
EVT_BUTTON(ID_MIGRATE_CANCEL, MigrateAssistantDialog::OnMigrateCancelClick)
EVT_BUTTON(ID_MIGRATE_OK, MigrateAssistantDialog::OnMigrateOKClick)
EVT_BUTTON(ID_MIGRATE_START, MigrateAssistantDialog::OnMigrateClick)
EVT_BUTTON(ID_MIGRATE_CONTINUE, MigrateAssistantDialog::OnMigrate1Click)
EVT_TIMER(MIGRATION_STATUS_TIMER, MigrateAssistantDialog::onTimerEvent)
END_EVENT_TABLE()

MigrateAssistantDialog::MigrateAssistantDialog(wxWindow* parent, bool bskipScan,
                               wxWindowID id, const wxString& caption,
                               const wxPoint& pos, const wxSize& size,
                               long style)
    : wxDialog(parent, id, caption, pos, size,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  m_Status = "";
  m_permissionResult = "";
  m_bsdcard = false;
  m_radioSDCard = 0;
  m_bskipScan = bskipScan;

  m_statusTimer.SetOwner(this, MIGRATION_STATUS_TIMER);

  wxFont *qFont = OCPNGetFont(_("Dialog"), 10);
  SetFont(*qFont);

  CreateControls();
  GetSizer()->SetSizeHints(this);
  Centre();
}

MigrateAssistantDialog::~MigrateAssistantDialog(void) {

}


void MigrateAssistantDialog::CreateControls(void) {
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  wxStaticBox* mmsiBox =
      new wxStaticBox(this, wxID_ANY, _("OpenCPN for Android Migration Assistant"));

  wxStaticBoxSizer* infoSizer = new wxStaticBoxSizer(mmsiBox, wxVERTICAL);
  mainSizer->Add(infoSizer, 0, wxEXPAND | wxALL, 5);
  m_infoText = NULL;

  if (!m_bskipScan){
    // Scan the  chart directory array from the config file.
    wxArrayString chartDirs = GetConfigChartDirectories();

    for (unsigned int i=0; i < chartDirs.GetCount(); i++){
      bool bOK = false;
      if ( chartDirs[i].StartsWith(g_androidGetFilesDirs0) )
        bOK = true;

      else if (!g_androidGetFilesDirs1.StartsWith("?")){
        if ( chartDirs[i].StartsWith(g_androidGetFilesDirs1) )
          bOK = true;
      }
      if (!bOK) {
        m_migrateDirs.Add(chartDirs[i]);
      }
    }
  }

  if (m_migrateDirs.GetCount()){
    wxString infoText1(_("OpenCPN has detected chart folders in your configuration file that cannot be accessed on this version of Android"));

    wxString infoText1w = WrapText(this, infoText1, gFrame->GetSize().x * 95 / 100);

    m_infoText = new wxStaticText(this, wxID_STATIC, infoText1w);

    infoSizer->AddSpacer( 1 * GetCharWidth());
    infoSizer->Add(m_infoText, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 10);
    infoSizer->AddSpacer( 1 * GetCharWidth());

    wxString dirsMsg;

    for (unsigned int i=0; i < m_migrateDirs.GetCount(); i++){
      dirsMsg += wxString("     ");
      dirsMsg += m_migrateDirs[i];
      dirsMsg += wxString("\n");
    }
    //dirsMsg += wxString("\n");

    m_infoDirs = new wxStaticText(this, wxID_STATIC, dirsMsg);

    infoSizer->Add(m_infoDirs, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 10);

    wxString migrateMsg1 = _("OpenCPN can copy these chart folders to a suitable location, if desired.");
    migrateMsg1 += "\n\n";
    migrateMsg1 += _("To proceed with chart folder migration, choose the chart source folder, and follow the instructions given.");

    wxString migrateMsg1w = WrapText(this, migrateMsg1, gFrame->GetSize().x * 95/ 100);

    m_migrateStep1 = new wxStaticText(this, wxID_STATIC, migrateMsg1w);
    infoSizer->Add(m_migrateStep1, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 10);

  }
  else {

    wxString migrateMsg1 = _("Some chart folders may be inaccessible to OpenCPN on this version of Android. ");
    migrateMsg1 += _("OpenCPN can copy these chart folders to a suitable location, if desired.");
    migrateMsg1 += "\n\n";
    migrateMsg1 += _("To proceed with chart folder migration, choose the chart source folder, and follow the instructions given.");

    wxString migrateMsg1w = WrapText(this, migrateMsg1, gFrame->GetSize().x * 9 / 10);

    m_migrateStep1 = new wxStaticText(this, wxID_STATIC, migrateMsg1w);
    infoSizer->Add(m_migrateStep1, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 10);
  }

   mainSizer->AddSpacer( 1 * GetCharWidth());

  // Is SDCard available?
  if (!g_androidGetFilesDirs1.StartsWith("?")){

    wxStaticBoxSizer* sourceSizer = new wxStaticBoxSizer(
        new wxStaticBox(this, wxID_ANY, _("Migrate destination")), wxVERTICAL);
    mainSizer->Add(sourceSizer, 0, wxEXPAND | wxALL, 5);
    mainSizer->AddSpacer( 2 * GetCharWidth());

    m_radioInternal = new	wxRadioButton (this, wxID_ANY, _("OpenCPN Internal Storage"),wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    sourceSizer->Add( m_radioInternal, 0, /*wxEXPAND |*/ wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    m_radioSDCard = new	wxRadioButton (this, wxID_ANY, _("OpenCPN SDCard Storage"),wxDefaultPosition, wxDefaultSize);
    sourceSizer->Add( m_radioSDCard, 0, /*wxEXPAND |*/ wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    m_radioInternal->SetValue( true );
  }


  // control buttons
  m_migrateButton = new wxButton(this, ID_MIGRATE_START, _("Choose chart source folder."));
  mainSizer->Add(m_migrateButton, 0, wxEXPAND | wxALL, 5);

  //mainSizer->AddSpacer( 1 * GetCharWidth());

  statusSizer = new wxStaticBoxSizer(
        new wxStaticBox(this, wxID_ANY, _("Status")), wxVERTICAL);
    mainSizer->Add(statusSizer, 0, wxEXPAND | wxALL, 5);


  m_ipGauge = new InProgressIndicator(this, wxID_ANY, 100, wxDefaultPosition,
                                      wxSize(gFrame->GetSize().x * 8 / 10, gFrame->GetCharHeight() * 2));
  statusSizer->Add(m_ipGauge, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);

  mainSizer->AddSpacer( 1 * GetCharWidth());

  m_statusText = new wxStaticText(this, wxID_STATIC, m_Status);
  statusSizer->Add(m_statusText, 0, wxEXPAND | wxALL, 5);

  GetSizer()->Hide(statusSizer);

  wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
  mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 5);
  m_CancelButton = new wxButton(this, ID_MIGRATE_CANCEL, _("Cancel"));
  m_CancelButton->SetDefault();
  btnSizer->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(this, ID_MIGRATE_OK, _("OK"));
  btnSizer->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->Hide();

}


void MigrateAssistantDialog::OnMigrateCancelClick(wxCommandEvent& event) {

  m_statusTimer.Stop();
  callActivityMethod_vs("cancelMigration");

  EndModal(wxID_CANCEL);
}

void MigrateAssistantDialog::OnMigrateOKClick(wxCommandEvent& event) {
  EndModal(wxID_OK);
}

void MigrateAssistantDialog::OnMigrateClick(wxCommandEvent& event)
{

  wxString clickText1(_("On the next page, find and choose the root folder containing chart files to migrate\n\n\
Example: /storage/emulated/0/Charts\n\n"));
  clickText1 += _("This entire folder will be migrated.\n");
  clickText1 += _("Proceed?");

  if (wxID_OK == OCPNMessageBox(
        NULL, clickText1, _("OpenCPN for Android Migration Assistant"), wxOK | wxCANCEL )){

    if(m_infoText) m_infoText->Hide();
    m_migrateStep1->Hide();
    GetSizer()->Show(statusSizer);
    Layout();

    if (g_androidUtilHandler) {

      m_Status = _("Waiting for permission grant....");
      setStatus( m_Status );

      g_androidUtilHandler->m_eventTimer.Stop();
      g_androidUtilHandler->m_migratePermissionSetDone = false;

      wxString activityResult;
      activityResult = callActivityMethod_vs("migrateSetup");

      if (activityResult == _T("OK")) {
        // qDebug() << "Migrate Result1 OK, enabling timer wait";
        g_androidUtilHandler->m_action = ACTION_SAF_PERMISSION_END;
        g_androidUtilHandler->m_eventTimer.Start(1000, wxTIMER_CONTINUOUS);
      }
    }
  }
}

void MigrateAssistantDialog::OnMigrate1Click(wxCommandEvent& event) {

  // Construct the migration arguments

  //Destination is either internal, or SDCard, if available
  if( !m_bsdcard )
    m_migrateDestinationFolder = g_androidGetFilesDirs0 + "/Charts";
  else
    m_migrateDestinationFolder = g_androidGetFilesDirs1 + "/Charts";

  qDebug() << "m_migrateSourceFolder" << m_migrateSourceFolder.mb_str();
  qDebug() << "m_migrateDestinationFolder" << m_migrateDestinationFolder.mb_str();

  wxString activityResult;
  activityResult = callActivityMethod_s2s("migrateFolder", m_migrateSourceFolder, m_migrateDestinationFolder);

  m_Status = _("Migration started...");
  setStatus( m_Status );

  m_statusTimer.Start(500, wxTIMER_CONTINUOUS);
  m_ipGauge->Show();
  Layout();


}

void MigrateAssistantDialog::onPermissionGranted( wxString result ) {
  m_permissionResult = result;
  qDebug() << "onPermissionGranted " << result.mb_str();
  if(result.StartsWith("file")){
    m_migrateSourceFolder = result.Mid(5);

    m_Status = _("Permission granted to ");
    m_Status += m_migrateSourceFolder;
    setStatus( m_Status );

    // Carry on to the next step
    m_migrateButton->Hide();
    Layout();

    // Capture the destination
    if(m_radioSDCard)
      m_bsdcard = m_radioSDCard->GetValue();

    wxString clickText2(_("OpenCPN has obtained temporary permission to access the selected chart folders."));
    clickText2 += "\n\n";
    clickText2 += _("Chart migration is ready to proceed.");
    clickText2 += "\n\n";
    clickText2 += _("Source: ");
    clickText2 += m_migrateSourceFolder;
    clickText2 += "\n\n";
    if(!m_bsdcard)
      clickText2 += _("Destination: OpenCPN Internal Storage");
    else
      clickText2 += _("Destination: OpenCPN SDCard Storage");
    clickText2 += "\n\n";
    clickText2 += _("Migrate charts now?");



    if (wxID_OK == OCPNMessageBox(
        NULL, clickText2, _("OpenCPN for Android Migration Assistant"), wxOK | wxCANCEL )){

      wxCommandEvent evt(wxEVT_BUTTON);
      evt.SetId(ID_MIGRATE_CONTINUE);
      AddPendingEvent(evt);
    }
    else{
     m_Status = "";
     setStatus( m_Status );

     m_migrateButton->Show();
     Layout();
    }
  }
  else{
    m_Status = "";
    setStatus( m_Status );
  }

}

void MigrateAssistantDialog::onTimerEvent(wxTimerEvent &event)
{
    // Get and show the current status from Java upstream
    qDebug() << "Migration: onTimerEvent";

    m_Status = callActivityMethod_vs("getMigrateStatus");
    setStatus( m_Status );

    if (m_Status.StartsWith("Counting"))
      m_ipGauge->Pulse();

    if (m_Status.StartsWith("Migrating")){
      wxString prog = m_Status.Mid(10);
      //qDebug() << prog.mb_str();
      wxString np = prog.BeforeFirst('/');
      //qDebug() << np.mb_str();
      wxString np1 = prog.AfterFirst('/');
      wxString np2 = np1.BeforeFirst(';');
      //qDebug() << np2.mb_str();

      long i, n;
      np.ToLong(&i);
      np2.ToLong(&n);
      if (m_ipGauge->GetRange() != n)
        m_ipGauge->SetRange( n );
      m_ipGauge->SetValue( i );
    }




    // Finished?
    if (m_Status.Contains("Migration complete")){
      m_statusTimer.Stop();

      wxString clickText3(_("Chart migration is finished."));
      clickText3 += "\n\n";
      clickText3 += _("Migrated chart folders are now accessible to OpenCPN.");
      clickText3 += "\n";
      clickText3 += _("You may need to adjust your chart folders further, to accommodate individual chart groups");
      clickText3 += "\n\n";
      clickText3 += _("OpenCPN will now restart to apply changes.");

      if (wxID_OK == OCPNMessageBox(
        NULL, clickText3, _("OpenCPN for Android Migration Assistant"), wxOK )){

        FinishMigration();

      }
    }

}

wxArrayString GetConfigChartDirectories()
{
  wxArrayString rv;
  pConfig->SetPath(_T ( "/ChartDirectories" ));
  int iDirMax = pConfig->GetNumberOfEntries();
  if (iDirMax) {
    wxString str, val;
    long dummy;
    bool bCont = pConfig->GetFirstEntry(str, dummy);
    while (bCont) {
      pConfig->Read(str, &val);  // Get a Directory name
      rv.Add(val.BeforeFirst('^'));
      bCont = pConfig->GetNextEntry(str, dummy);

    }
  }

  return rv;
}


void MigrateAssistantDialog::FinishMigration()
{
    m_Status = _("Finishing migration");
    setStatus( m_Status );

    // Craft the migrated (destination) folder

    qDebug() << "m_migrateSourceFolder " << m_migrateSourceFolder.mb_str();
    qDebug() << "m_migrateDestinationFolder " << m_migrateDestinationFolder.mb_str();



    // Edit the config file, removing old inaccessible folders,
    // and adding migrated folders.

    wxArrayString finalArray;
    wxArrayString chartDirs = GetConfigChartDirectories(); //ChartData->GetChartDirArrayString();
    for (unsigned int i=0; i < chartDirs.GetCount(); i++){

        //qDebug() << "Checking: " << chartDirs[i].mb_str();

        // Leave the OK folders
        bool bOK = false;
        if ( chartDirs[i].StartsWith(g_androidGetFilesDirs0) )
          bOK = true;

        else if (!g_androidGetFilesDirs1.StartsWith("?")){
          if ( chartDirs[i].StartsWith(g_androidGetFilesDirs1) )
            bOK = true;
        }

        // Check inaccessible folders to see if they were (part of) the migration
        if (!bOK) {
            if(!chartDirs[i].StartsWith(m_migrateSourceFolder))    // not part of migration
              bOK = true;                                   // so, keep it.
                                                            // To be migrated on next round
        }

        if(bOK){
          //qDebug() << "Add: " << chartDirs[i].mb_str();
          finalArray.Add(chartDirs[i]);
        }

    }

    finalArray.Add(m_migrateDestinationFolder + "/MigratedCharts");

#if 0
    // Now manage the migrate folder
    // OCPN works faster if the chart dirs are at fine granularity
    // This is due to the expense of traversing a very deep directory tree.
    // If the migrated directory contains only subdirectories, and no files,
    //  then add the subdirs to the chart dir array.

    wxString migratedFolder = m_migrateDestinationFolder + "/MigratedCharts/";

    // If the migrate source is a shallow(single dir) copy,
    //  and it happens that the full path contains a folder that is already in the destination tree:
    //  Example:  source: /storage/xxxx-yyyy/Charts/ENC/R7
    //            destination" {internal}/files/Charts/MigratedCharts/Charts
    //   If destination already contains .../ENC
    //   then the migration would have merged the current contents.
    // In this case, we need to determine the actual migrated folder

    wxFileName fn(m_migrateSourceFolder);
    migratedFolder += fn.GetName();
    qDebug() << "migratedFolder " << migratedFolder.mb_str();

    wxDir migratedDir(migratedFolder + "/");
    if (migratedDir.HasFiles()){
      qDebug() << "Add A";
      finalArray.Add(migratedFolder);
    }
    else {
      qDebug() << "Add CSD";

      if (migratedDir.HasSubDirs())
      {
        qDebug() << "Add SD";
        wxArrayString children;
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (migratedFolder.c_str())) != NULL) {
          while ((ent = readdir (dir)) != NULL) {
            wxString sent(ent->d_name);
            qDebug() << "sent: " << sent.mb_str();
            if (!sent.StartsWith('.')){
              wxString dirToAdd = migratedFolder + "/" + sent;
              children.Add( wxString(dirToAdd));
            }
          }
          closedir (dir);
        }

        for (unsigned int j=0 ; j < children.GetCount() ; j++){
          qDebug() << "Child: " << children[j].mb_str();
          if(wxFileName::DirExists(children[j])){
            qDebug() << "ChildDir: " << children[j].mb_str();
            qDebug() << "Add B";
            finalArray.Add(children[j]);
          }
        }
      }
    }

#endif
    for (unsigned int j=0 ; j < finalArray.GetCount() ; j++){
      qDebug() << "finalEntry: " << finalArray[j].mb_str();
    }


    // Now delete and replace the chart directory list in the config file
    wxRemoveFile(ChartListFileName);

    pConfig->SetPath(_T ( "/ChartDirectories" ));
    pConfig->DeleteGroup(_T ( "/ChartDirectories" ));

    pConfig->SetPath(_T ( "/ChartDirectories" ));
    for (int iDir = 0; iDir < finalArray.GetCount(); iDir++) {
      wxString dirn = finalArray[iDir];
      dirn.Append(_T("^"));

      wxString str_buf;
      str_buf.Printf(_T ( "ChartDir%d" ), iDir + 1);
      pConfig->Write(str_buf, dirn);
    }
    pConfig->Flush();

    // Restart
    callActivityMethod_vs("restartOCPNAfterMigrate");

}

 BEGIN_EVENT_TABLE( InProgressIndicator, wxGauge )
 EVT_TIMER( 4356, InProgressIndicator::OnTimer )
 END_EVENT_TABLE()

 InProgressIndicator::InProgressIndicator()
 {
 }

 InProgressIndicator::InProgressIndicator(wxWindow* parent, wxWindowID id, int range,
                     const wxPoint& pos, const wxSize& size,
                     long style, const wxValidator& validator, const wxString& name)
{
    wxGauge::Create(parent, id, range, pos, size, style, validator, name);

    m_timer.SetOwner( this, 4356 );

    SetValue(0);
    m_bAlive = false;

}

InProgressIndicator::~InProgressIndicator()
{
    Stop();
}

void InProgressIndicator::OnTimer(wxTimerEvent &evt)
{
    if(m_bAlive)
        Pulse();
}


void InProgressIndicator::Start()
{
     m_bAlive = true;
     m_timer.Start( 50 );

}

void InProgressIndicator::Stop()
{
     m_bAlive = false;
     SetValue(0);
     m_timer.Stop();

}

