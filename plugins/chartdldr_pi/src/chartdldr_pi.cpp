/******************************************************************************
 * $Id: chartdldr_pi.cpp,v 1.0 2011/02/26 01:54:37 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Chart downloader Plugin
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Pavel Kalian                                    *
 *   $EMAIL$                                                               *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/msgdlg.h>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "chartdldr_pi.h"
#include "chartdldr_bulk_orchestrate.h"
#include "chartdldr_bulk_schedule.h"
#include "chartdldr_panel.h"
#include "chartdldr_panel_impl.h"
#include "chartdldr_prefs.h"
#include "wxWTranslateCatalog.h"
#include <wx/stdpaths.h>
#include <wx/url.h>
#include <wx/progdlg.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/filesys.h>
#include <memory>
#include <wx/regex.h>
#include <wx/debug.h>
#include <wx/timer.h>
#include <wx/utils.h>

#ifdef __ANDROID__
#define _LIBCPP_HAS_NO_OFF_T_FUNCTIONS
#endif

#include <fstream>

#ifdef __ANDROID__
#include "androidSupport.h"
#include "android_jvm.h"
#include <jni.h>
#endif

#ifdef __WXMAC__
#define CATALOGS_NAME_WIDTH 300
#define CATALOGS_DATE_WIDTH 120
#define CATALOGS_PATH_WIDTH 100
#define CHARTS_NAME_WIDTH 300
#define CHARTS_STATUS_WIDTH 100
#define CHARTS_DATE_WIDTH 120
#else
#ifdef __ANDROID__

#define CATALOGS_NAME_WIDTH 350
#define CATALOGS_DATE_WIDTH 500
#define CATALOGS_PATH_WIDTH 1000
#define CHARTS_NAME_WIDTH 520
#define CHARTS_STATUS_WIDTH 150
#define CHARTS_DATE_WIDTH 200

#else

#define CATALOGS_NAME_WIDTH 200
#define CATALOGS_DATE_WIDTH 130
#define CATALOGS_PATH_WIDTH 250
#define CHARTS_NAME_WIDTH 320
#define CHARTS_STATUS_WIDTH 150
#define CHARTS_DATE_WIDTH 130

#endif
#endif  // __WXMAC__

#ifdef __ANDROID__
#include <QtAndroidExtras/QAndroidJniObject>
#include "qdebug.h"

#endif

bool getDisplayMetrics();

#define CHART_DIR "Charts"

int g_Android_SDK_Version;

bool IsDLDirWritable(wxFileName fn) {
#ifndef __ANDROID__
  return fn.IsDirWritable();
#else
  if (g_Android_SDK_Version >= 30) {  // scoped storage?
    // Use a simple test here
    return (fn.GetFullPath().Contains("org.opencpn.opencpn"));  // fast test
  } else
    return fn.IsDirWritable();

#endif
}

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin *create_pi(void *ppimgr) {
  return new chartdldr_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin *p) { delete p; }

double g_androidDPmm;
chartdldr_pi *g_pi;

//---------------------------------------------------------------------------------------------------------
//
//    ChartDldr PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

chartdldr_pi::chartdldr_pi(void *ppimgr)
    : opencpn_plugin_113(ppimgr), m_scheduler(this) {
  // Create the PlugIn icons
  initialize_images();

  m_parent_window = NULL;
  m_pChartSource = NULL;
  m_pconfig = NULL;
  m_preselect_new = false;
  m_preselect_updated = false;
  m_allow_bulk_update = false;
  m_pOptionsPage = NULL;
  m_selected_source = -1;
  m_dldrpanel = NULL;
  m_panel_host = NULL;
  m_schartdldr_sources = wxEmptyString;

  g_pi = this;
}

int chartdldr_pi::Init(void) {
  AddLocaleCatalog(PLUGIN_CATALOG_NAME);

  //    Get a pointer to the opencpn display canvas, to use as a parent for the
  //    POI Manager dialog
  m_parent_window = GetOCPNCanvasWindow();

  m_scheduler.Attach(m_parent_window);

  //    Get a pointer to the opencpn configuration object
  m_pconfig = GetOCPNConfigObject();
  m_pOptionsPage = NULL;

  m_pChartSource = NULL;

#ifdef __ANDROID__
  androidGetSDKVersion();
#endif

  //    And load the configuration items
  LoadConfig();

  getDisplayMetrics();

  wxStringTokenizer st(m_schartdldr_sources, _T("|"), wxTOKEN_DEFAULT);
  while (st.HasMoreTokens()) {
    wxString s1 = st.GetNextToken();
    wxString s2 = st.GetNextToken();
    wxString s3 = st.GetNextToken();
    if (!s2.IsEmpty())  // scrub empty sources.
      m_ChartSources.push_back(std::make_unique<ChartSource>(s1, s2, s3));
  }

  EnsureDownloaderPanel();
  m_scheduler.Restart();

  return (WANTS_PREFERENCES | WANTS_CONFIG | INSTALLS_TOOLBOX_PAGE);
}

bool chartdldr_pi::DeInit(void) {
  wxLogMessage(_T("chartdldr_pi: DeInit"));

  if (m_dldrpanel) {
    m_dldrpanel->Bulk().CancelScheduledRun();
  }
  m_scheduler.Detach();

  m_ChartSources.clear();
  ChartDldrDestroyDownloaderUI(this);
  if (m_pOptionsPage) {
    if (DeleteOptionsPage(m_pOptionsPage)) m_pOptionsPage = NULL;
  }
  return true;
}

int chartdldr_pi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int chartdldr_pi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

int chartdldr_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int chartdldr_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap *chartdldr_pi::GetPlugInBitmap() { return _img_chartdldr_pi; }

wxString chartdldr_pi::GetCommonName() { return _("ChartDownloader"); }

wxString chartdldr_pi::GetShortDescription() {
  return _("Chart Downloader PlugIn for OpenCPN");
}

wxString chartdldr_pi::GetLongDescription() {
  return _(
      "Chart Downloader PlugIn for OpenCPN\n\
Manages chart downloads and updates from sources supporting\n\
NOAA Chart Catalog format");
}

void chartdldr_pi::OnSetupOptions(void) {
  m_pOptionsPage =
      AddOptionsPage(PI_OPTIONS_PARENT_CHARTS, _("Chart Downloader"));
  if (!m_pOptionsPage) {
    wxLogMessage(
        _T("Error: chartdldr_pi::OnSetupOptions AddOptionsPage failed!"));
    return;
  }

  if (!EnsureDownloaderPanel()) {
    wxLogMessage(_T("Error: chartdldr_pi::OnSetupOptions panel init failed!"));
    return;
  }

  if (!ChartDldrPanelOnOptionsPage(this)) {
    AttachDownloaderPanelToOptions(m_pOptionsPage);
  }

  m_dldrpanel->SetBulkUpdate(m_allow_bulk_update);
  m_dldrpanel->Show();
  m_dldrpanel->FitInside();
}

void chartdldr_pi::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel) {
  if (!m_dldrpanel) {
    SaveConfig();
    return;
  }

  m_dldrpanel->CancelDownload();
#ifndef __ANDROID__
  OCPN_cancelDownloadFileBackground(
      0);  // Stop the thread, is something like this needed on Android as well?
#endif
  m_selected_source = m_dldrpanel->GetSelectedCatalog();
  SaveConfig();
}

bool chartdldr_pi::LoadConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (pConf) {
    pConf->SetPath(_T ( "/Settings/ChartDnldr" ));
    pConf->Read(_T ( "ChartSources" ), &m_schartdldr_sources, wxEmptyString);
    pConf->Read(_T ( "Source" ), &m_selected_source, -1);

    wxFileName fn(GetWritableDocumentsDir(), wxEmptyString);
    fn.AppendDir(_T(CHART_DIR));

    pConf->Read(_T ( "BaseChartDir" ), &m_base_chart_dir, fn.GetPath());
    wxLogMessage(_T ( "chartdldr_pi:m_base_chart_dir: " ) + m_base_chart_dir);

    // Check to see if the directory is writeable, esp. on App updates.
    wxFileName testFN(m_base_chart_dir);
    if (!IsDLDirWritable(testFN)) {
      wxLogMessage(
          "Cannot write to m_base_chart_dir, override to "
          "GetWritableDocumentsDir()");
      m_base_chart_dir = fn.GetPath();
      wxLogMessage(_T ( "chartdldr_pi: Corrected: " ) + m_base_chart_dir);
    }

    pConf->Read(_T ( "PreselectNew" ), &m_preselect_new, true);
    pConf->Read(_T ( "PreselectUpdated" ), &m_preselect_updated, true);
    pConf->Read(_T ( "AllowBulkUpdate" ), &m_allow_bulk_update, false);
    m_schedule.Load(pConf);
    return true;
  } else
    return false;
}

bool chartdldr_pi::SaveConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  m_schartdldr_sources.Clear();

  for (size_t i = 0; i < m_ChartSources.size(); i++) {
    std::unique_ptr<ChartSource> &cs = m_ChartSources.at(i);
    m_schartdldr_sources.Append(
        wxString::Format(_T("%s|%s|%s|"), cs->GetName().c_str(),
                         cs->GetUrl().c_str(), cs->GetDir().c_str()));
  }

  if (pConf) {
    pConf->SetPath(_T ( "/Settings/ChartDnldr" ));
    pConf->Write(_T ( "ChartSources" ), m_schartdldr_sources);
    pConf->Write(_T ( "Source" ), m_selected_source);
    pConf->Write(_T ( "BaseChartDir" ), m_base_chart_dir);
    pConf->Write(_T ( "PreselectNew" ), m_preselect_new);
    pConf->Write(_T ( "PreselectUpdated" ), m_preselect_updated);
    pConf->Write(_T ( "AllowBulkUpdate" ), m_allow_bulk_update);
    m_schedule.Save(pConf);

    return true;
  } else
    return false;
}

void SetBackColor(wxWindow *ctrl, wxColour col) {
  static int depth = 0;  // recursion count
  if (depth == 0) {      // only for the window root, not for every child

    ctrl->SetBackgroundColour(col);
  }

  wxWindowList kids = ctrl->GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode *node = kids.Item(i);
    wxWindow *win = node->GetData();

    if (dynamic_cast<wxListBox *>(win))
      dynamic_cast<wxListBox *>(win)->SetBackgroundColour(col);

    else if (dynamic_cast<wxTextCtrl *>(win))
      dynamic_cast<wxTextCtrl *>(win)->SetBackgroundColour(col);

    //        else if( win->IsKindOf( CLASSINFO(wxStaticText) ) )
    //            ( (wxStaticText*) win )->SetForegroundColour( uitext );

    else if (dynamic_cast<wxChoice *>(win))
      dynamic_cast<wxChoice *>(win)->SetBackgroundColour(col);

    else if (dynamic_cast<wxComboBox *>(win))
      dynamic_cast<wxComboBox *>(win)->SetBackgroundColour(col);

    else if (dynamic_cast<wxRadioButton *>(win))
      dynamic_cast<wxRadioButton *>(win)->SetBackgroundColour(col);

    else if (dynamic_cast<wxScrolledWindow *>(win)) {
      dynamic_cast<wxScrolledWindow *>(win)->SetBackgroundColour(col);
    }

    else if (dynamic_cast<wxButton *>(win)) {
      dynamic_cast<wxButton *>(win)->SetBackgroundColour(col);
    }

    else {
      ;
    }

    if (win->GetChildren().GetCount() > 0) {
      depth++;
      wxWindow *w = win;
      SetBackColor(w, col);
      depth--;
    }
  }
}

void chartdldr_pi::ShowPreferencesDialog(wxWindow *parent) {
  ChartDldrPrefsDlgImpl *dialog = new ChartDldrPrefsDlgImpl(parent, this);

  wxFont fo = GetOCPNGUIScaledFont_PlugIn(_("Dialog"));
  dialog->SetFont(fo);

#ifdef __ANDROID__
  if (m_parent_window) {
    int xmax = m_parent_window->GetSize().GetWidth();
    int ymax = m_parent_window->GetParent()
                   ->GetSize()
                   .GetHeight();  // This would be the Options dialog itself
    dialog->SetSize(xmax, ymax);
    dialog->Layout();

    dialog->Move(0, 0);
  }

  wxColour cl = wxColour(214, 218, 222);
  SetBackColor(dialog, cl);
#endif

  dialog->SetPath(m_base_chart_dir);
  dialog->SetPreferences(m_preselect_new, m_preselect_updated,
                         m_allow_bulk_update);
  dialog->SetSchedulePreferences(m_schedule);

  dialog->ShowModal();
  dialog->Destroy();
}

void chartdldr_pi::UpdatePrefs(ChartDldrPrefsDlgImpl *dialog) {
  m_base_chart_dir = dialog->GetPath();
  dialog->GetPreferences(m_preselect_new, m_preselect_updated,
                         m_allow_bulk_update);
  ChartDldrScheduleConfig schedule = m_schedule;
  if (dialog->GetSchedulePreferences(schedule)) {
    m_schedule = schedule;
  }
  SaveConfig();
  if (m_dldrpanel) m_dldrpanel->SetBulkUpdate(m_allow_bulk_update);
  m_scheduler.Restart();
}

void chartdldr_pi::ApplyChartDatabaseUpdate() {
  wxArrayString dirs = GetChartDBDirArrayString();
  if (dirs.GetCount() > 0) {
    UpdateChartDBInplace(dirs, false, false);
  }
}

bool chartdldr_pi::EnsureDownloaderPanel() {
  if (m_dldrpanel) {
    return true;
  }
  if (!m_parent_window) {
    return false;
  }

  if (!m_panel_host) {
    m_panel_host = new wxPanel(m_parent_window, wxID_ANY, wxDefaultPosition,
                               wxDefaultSize, wxNO_BORDER);
    m_panel_host->Hide();
  }

  m_dldrpanel =
      new ChartDldrPanelImpl(this, m_panel_host, wxID_ANY, wxDefaultPosition,
                             wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
  m_dldrpanel->Hide();

  wxBoxSizer *host_sizer = new wxBoxSizer(wxVERTICAL);
  m_panel_host->SetSizer(host_sizer);
  host_sizer->Add(m_dldrpanel, 1, wxEXPAND);
  m_dldrpanel->SetBulkUpdate(m_allow_bulk_update);
  return true;
}

void chartdldr_pi::AttachDownloaderPanelToOptions(wxScrolledWindow *page) {
  ChartDldrAttachDownloaderPanelToOptions(this, page);
}

bool chartdldr_pi::RequestBulkUpdate(ChartDldrBulkRunKind kind) {
  return ChartDldrRequestBulkUpdate(this, kind);
}

bool chartdldr_pi::ConfirmInteractiveBulkUpdate(wxWindow *parent) const {
  wxString message;
  if (m_preselect_new && m_preselect_updated) {
    message =
        _("You have chosen to update all chart catalogs.\nThen download "
          "all new and updated charts.\nThis may take a long time.");
  } else if (m_preselect_new) {
    message =
        _("You have chosen to update all chart catalogs.\nThen download "
          "only new (but not updated) charts.\nThis may take a long "
          "time.");
  } else if (m_preselect_updated) {
    message =
        _("You have chosen to update all chart catalogs.\nThen download "
          "only updated (but not new) charts.\nThis may take a long "
          "time.");
  } else {
    return true;
  }

  wxMessageDialog mess(parent, message, _("Chart Downloader"), wxOK | wxCANCEL);
  return mess.ShowModal() != wxID_CANCEL;
}

void chartdldr_pi::StartAsyncBulkUpdate() {
  if (m_dldrpanel) {
    m_dldrpanel->Bulk().StartScheduledRun(this);
  }
  m_scheduler.EnsureIdleWatch(true);
}

void chartdldr_pi::OnScheduledBulkRunFinished() {
  m_scheduler.EnsureIdleWatch(false);
}

ChartDldrBulkRequestInput chartdldr_pi::MakeBulkRequestInput() const {
  ChartDldrBulkRequestInput input;
  input.allow_bulk_update = m_allow_bulk_update;
  input.has_chart_sources = HasChartSources();
  input.bulk_run_active = m_dldrpanel && m_dldrpanel->Bulk().IsRunActive();
  return input;
}

bool getDisplayMetrics() {
#ifdef __ANDROID__

  g_androidDPmm = 4.0;  // nominal default

  //  Get a reference to the running native activity
  QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
      "org/qtproject/qt5/android/QtNative", "activity",
      "()Landroid/app/Activity;");

  if (!activity.isValid()) {
    return false;
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

  //  Return string may have commas instead of periods, if using Euro locale
  //  We just fix it here...
  return_string.Replace(_T(","), _T("."));

  //     wxLogMessage(_T("Metrics:") + return_string);
  //     wxSize screen_size = ::wxGetDisplaySize();
  //     wxString msg;
  //     msg.Printf(_T("wxGetDisplaySize(): %d %d"), screen_size.x,
  //     screen_size.y); wxLogMessage(msg);

  double density = 1.0;
  wxStringTokenizer tk(return_string, _T(";"));
  if (tk.HasMoreTokens()) {
    wxString token = tk.GetNextToken();  // xdpi
    token = tk.GetNextToken();           // density

    long b = ::wxGetDisplaySize().y;
    token.ToDouble(&density);

    token = tk.GetNextToken();  // ldpi

    token = tk.GetNextToken();  // width
    token = tk.GetNextToken();  // height - statusBarHeight
    token = tk.GetNextToken();  // width
    token = tk.GetNextToken();  // height
    token = tk.GetNextToken();  // dm.widthPixels
    token = tk.GetNextToken();  // dm.heightPixels

    token = tk.GetNextToken();  // actionBarHeight
    long abh;
    token.ToLong(&abh);
    //        g_ActionBarHeight = wxMax(abh, 50);

    //        qDebug() << "g_ActionBarHeight" << abh << g_ActionBarHeight;
  }

  double ldpi = 160. * density;

  //    double maxDim = wxMax(::wxGetDisplaySize().x, ::wxGetDisplaySize().y);
  //    ret = (maxDim / ldpi) * 25.4;

  //    msg.Printf(_T("Android Auto Display Size (mm, est.): %g"), ret);
  //    wxLogMessage(msg);

  //  Save some items as global statics for convenience
  g_androidDPmm = ldpi / 25.4;
  //    g_androidDensity = density;

  // qDebug() << "PI Metrics" << g_androidDPmm << density;
  return true;
#else

  return true;
#endif
}

