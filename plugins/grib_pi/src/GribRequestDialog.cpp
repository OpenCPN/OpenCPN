/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 ***************************************************************************
 *
 */

#include "wx/wx.h"
#include <wx/utils.h>
#include <sstream>
#include "email.h"
#include "XyGribModelDef.h"

#include "pi_gl.h"

#include "GribRequestDialog.h"
#include "GribOverlayFactory.h"
#include <wx/wfstream.h>
#include "grib_pi.h"

#include <unordered_map>

#define RESOLUTIONS 4

enum { SAILDOCS, ZYGRIB };                       // grib providers
enum { GFS, COAMPS, RTOFS, HRRR, ICON, ECMWF };  // forecast models

wxString toMailFormat(int NEflag,
                      int a)  // convert position to mail necessary format
{
  char c = NEflag == 1 ? a < 0 ? 'S' : 'N' : a < 0 ? 'W' : 'E';
  wxString s;
  s.Printf(_T ( "%01d%c" ), abs(a), c);
  return s;
}

extern int m_SavedZoneSelMode;
extern int m_ZoneSelMode;

//----------------------------------------------------------------------------------------------------------
//          GRIB Request Implementation
//----------------------------------------------------------------------------------------------------------
GribRequestSetting::GribRequestSetting(GRIBUICtrlBar &parent)
    : GribRequestSettingBase(&parent), m_parent(parent) {
  m_Vp = 0;
  m_oDC = nullptr;

  m_displayScale = 1.0;
#if defined(__WXOSX__) || defined(__WXGTK3__)
  // Support scaled HDPI displays.
  m_displayScale = GetContentScaleFactor();
#endif

  InitRequestConfig();
  m_connected = false;
  m_downloading = false;
  m_bTransferSuccess = true;
  m_downloadType = GribDownloadType::NONE;

  auto bg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)
                .GetAsString(wxC2S_HTML_SYNTAX);
  auto fg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)
                .GetAsString(wxC2S_HTML_SYNTAX);
  m_htmlWinWorld->SetBorders(10);
  m_htmlWinWorld->SetPage(
      "<html><body bgcolor="
      "" +
      bg +
      ""
      "><font color="
      "" +
      fg +
      ""
      ">" +
      _("<h1>OpenCPN ECMWF forecast</h1>"
        "<p>Free service based on ECMWF Open Data published under the terms of "
        "Creative Commons CC-4.0-BY licence</p>"
        "<p>The IFS model GRIB files include information about surface "
        "temperature, "
        "atmospheric pressure, wind strength, wind direction, wave height and "
        "direction for the whole world on a 0.25 degree resolution "
        "grid with 3 hour "
        "step in the first 144 hours and 6 hour step up to 10 days.</p>"
        "The AIFS model contains data for wind, pressure and temperature on a "
        "0.25 degree grid with 6 hour step for up to 15 days"
        "<p>The data is updated twice a day as soon as the 00z and 12z model "
        "runs finish and the "
        "results are published by ECMWF, which usually means new forecast data "
        "is available shortly after 8AM and 8PM UTC.</p>"
        "<p>The grib downloaded covers the area of the primary chart "
        "canvas.</p>"
        "<p>The service is provided on best effort basis and comes with no "
        "guarantees. The server is hosted by a volunteer and the service is "
        "provided free of charge and without accepting any liability "
        "whatsoever for its continuous availability, or for any loss or damage "
        "arising from its use. If you find the service useful, please "
        "consider making a donation to the OpenCPN project.</p>"
        "<p>This service is based on data and products of the European Centre "
        "for Medium-Range Weather Forecasts (ECMWF).</p>"
        "<p>Source: www.ecmwf.int</p>"
        "<p>Disclaimer: ECMWF does not accept any liability whatsoever for any "
        "error or omission in the data, their availability, or for any loss or "
        "damage arising from their use.</p>"
        "</font></body></html>"));
  m_htmlInfoWin->SetBorders(10);
  m_htmlInfoWin->SetPage(
      "<html><body bgcolor="
      "" +
      bg +
      ""
      "><font color="
      "" +
      fg +
      ""
      ">" +
      _("<h1>Grib weather forecasts</h1>"
        "<p>Collection of local weather models from various sources available "
        "for download over the internet.</p>"
        "</font></body></html>"));
  ReadLocalCatalog();
  m_bLocal_source_selected = false;
  EnableDownloadButtons();

  m_selectedAtmModelIndex = 0;
  m_selectedWaveModelIndex = 0;
  InitializeXygribDialog();
}

GribRequestSetting::~GribRequestSetting() {
  if (m_downloading) {
    OCPN_cancelDownloadFileBackground(m_download_handle);
  }
  if (m_connected) {
    Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
  }
  delete m_Vp;
  if (m_oDC) {
    delete m_oDC;
  }
}

void GribRequestSetting::InitRequestConfig() {
  wxFileConfig *pConf = GetOCPNConfigObject();

  if (pConf) {
    pConf->SetPath(_T( "/PlugIns/GRIB" ));
    wxString l;
    int m;
    pConf->Read(_T( "MailRequestConfig" ), &m_RequestConfigBase,
                _T( "000220XX........0" ));
    pConf->Read(_T( "MailSenderAddress" ), &l, _T(""));
    m_pSenderAddress->ChangeValue(l);
    pConf->Read(_T( "MailRequestAddresses" ), &m_MailToAddresses,
                _T("query@saildocs.com;gribauto@zygrib.org"));
    pConf->Read(_T( "ZyGribLogin" ), &l, _T(""));
    m_pLogin->ChangeValue(l);
    pConf->Read(_T( "ZyGribCode" ), &l, _T(""));
    m_pCode->ChangeValue(l);
    pConf->Read(_T( "SendMailMethod" ), &m_SendMethod, 0);
    pConf->Read(_T( "MovingGribSpeed" ), &m, 0);
    m_sMovingSpeed->SetValue(m);
    pConf->Read(_T( "MovingGribCourse" ), &m, 0);
    m_sMovingCourse->SetValue(m);
    m_cManualZoneSel->SetValue(
        m_SavedZoneSelMode !=
        AUTO_SELECTION);  // has been read in GriUbICtrlBar dialog
                          // implementation or updated previously
    m_cUseSavedZone->SetValue(m_SavedZoneSelMode == SAVED_SELECTION);
    fgZoneCoordinatesSizer->ShowItems(m_SavedZoneSelMode != AUTO_SELECTION);
    m_cUseSavedZone->Show(m_SavedZoneSelMode != AUTO_SELECTION);
    if (m_cManualZoneSel->GetValue()) {
      pConf->Read(_T( "RequestZoneMaxLat" ), &m, 0);
      m_spMaxLat->SetValue(m);
      pConf->Read(_T( "RequestZoneMinLat" ), &m, 0);
      m_spMinLat->SetValue(m);
      pConf->Read(_T( "RequestZoneMaxLon" ), &m, 0);
      m_spMaxLon->SetValue(m);
      pConf->Read(_T( "RequestZoneMinLon" ), &m, 0);
      m_spMinLon->SetValue(m);

      SetCoordinatesText();
    }
    // if GriDataConfig has been corrupted , take the standard one to fix a
    // crash
    if (m_RequestConfigBase.Len() !=
        wxString(_T( "000220XX.............." )).Len())
      m_RequestConfigBase = _T( "000220XX.............." );
  }
  // populate model, mail to, waves model choices
  wxString s1[] = {_T("GFS"),  _T("COAMPS"), _T("RTOFS"),
                   _T("HRRR"), _T("ICON"),   _T("ECMWF")};
  for (unsigned int i = 0; i < (sizeof(s1) / sizeof(wxString)); i++)
    m_pModel->Append(s1[i]);
  wxString s2[] = {_T("Saildocs"), _T("zyGrib")};
  for (unsigned int i = 0; i < (sizeof(s2) / sizeof(wxString)); i++)
    m_pMailTo->Append(s2[i]);
  wxString s3[] = {_T("WW3-GLOBAL"), _T("WW3-MEDIT")};
  for (unsigned int i = 0; i < (sizeof(s3) / sizeof(wxString)); i++)
    m_pWModel->Append(s3[i]);
  m_rButtonYes->SetLabel(_("Send"));
  m_rButtonApply->SetLabel(_("Save"));
  m_tResUnit->SetLabel(wxString::Format(_T("\u00B0")));
  m_sCourseUnit->SetLabel(wxString::Format(_T("\u00B0")));

  // Set wxSpinCtrl sizing
  int w, h;
  GetTextExtent(_T("-360"), &w, &h, 0, 0,
                OCPNGetFont(_("Dialog"), 10));  // optimal text control size
  w += 30;
  h += 4;
  m_sMovingSpeed->SetMinSize(wxSize(w, h));
  m_sMovingCourse->SetMinSize(wxSize(w, h));
  m_spMaxLat->SetMinSize(wxSize(w, h));
  m_spMinLat->SetMinSize(wxSize(w, h));
  m_spMaxLon->SetMinSize(wxSize(w, h));
  m_spMinLon->SetMinSize(wxSize(w, h));

  // add tooltips
  m_pSenderAddress->SetToolTip(
      _("Address used to send request eMail. (Mandatory for LINUX)"));
  m_pLogin->SetToolTip(_("This is your zyGrib's forum access Login"));
  m_pCode->SetToolTip(
      _("Get this Code in zyGrib's forum ( This is not your password! )"));
  m_sMovingSpeed->SetToolTip(_("Enter your forescasted Speed (in Knots)"));
  m_sMovingCourse->SetToolTip(_("Enter your forecasted Course"));

  long i, j, k;
  ((wxString)m_RequestConfigBase.GetChar(0)).ToLong(&i);  // MailTo
  m_pMailTo->SetSelection(i);
  ((wxString)m_RequestConfigBase.GetChar(1)).ToLong(&i);  // Model
  m_pModel->SetSelection(i);
  m_cMovingGribEnabled->SetValue(m_RequestConfigBase.GetChar(16) ==
                                 'X');                        // Moving Grib
  ((wxString)m_RequestConfigBase.GetChar(2)).ToLong(&i);      // Resolution
  ((wxString)m_RequestConfigBase.GetChar(3)).ToLong(&j);      // interval
  ((wxString)m_RequestConfigBase.GetChar(4)).ToLong(&k, 16);  // Time Range
  k--;  // range max = 2 to 16 stored in hexa from 1 to f

#ifdef __WXMSW__  // show / hide sender elemants as necessary
  m_pSenderSizer->ShowItems(false);
#else
  if (m_SendMethod == 0)
    m_pSenderSizer->ShowItems(false);
  else
    m_pSenderSizer->ShowItems(
        true);  // possibility to use "sendmail" method with Linux
#endif

  m_tMouseEventTimer.Connect(
      wxEVT_TIMER, wxTimerEventHandler(GribRequestSetting::OnMouseEventTimer),
      NULL, this);

  m_RenderZoneOverlay = 0;

  ApplyRequestConfig(i, j, k);

  ((wxString)m_RequestConfigBase.GetChar(5)).ToLong(&j);  // Waves model
  m_pWModel->SetSelection(j);

  m_pWind->Enable(false);  // always selected if available
  m_pPress->Enable(false);

  DimeWindow(this);  // aplly global colours scheme

  m_AllowSend = true;
  m_MailImage->SetValue(WriteMail());
}

wxWindow *GetGRIBCanvas();
void GribRequestSetting::OnClose(wxCloseEvent &event) {
  if (m_downloading) {
    OCPN_cancelDownloadFileBackground(m_download_handle);
    m_downloading = false;
    m_download_handle = 0;
  }
  if (m_connected) {
    Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
  }
  m_RenderZoneOverlay = 0;  // eventually stop graphical zone display
  RequestRefresh(GetGRIBCanvas());

  // allow to be back to old value if changes have not been saved
  m_ZoneSelMode = m_SavedZoneSelMode;
  m_parent.SetRequestBitmap(m_ZoneSelMode);  // set appopriate bitmap
  m_parent.m_highlight_latmax = 0;
  m_parent.m_highlight_lonmax = 0;
  m_parent.m_highlight_latmin = 0;
  m_parent.m_highlight_lonmin = 0;
  this->Hide();
}

void GribRequestSetting::SetRequestDialogSize() {
  int y;
  /*first let's size the mail display space*/
  GetTextExtent(_T("abc"), NULL, &y, 0, 0, OCPNGetFont(_("Dialog"), 10));
  m_MailImage->SetMinSize(
      wxSize(-1, ((y * m_MailImage->GetNumberOfLines()) + 10)));

  /*then as default sizing do not work with wxScolledWindow let's compute it*/
  wxSize scroll =
      m_fgScrollSizer->Fit(m_sScrolledDialog);  // the area size to be scrolled

#ifdef __WXGTK__
  SetMinSize(wxSize(0, 0));
#endif

  wxWindow *frame = wxTheApp->GetTopWindow();

  int w = frame->GetClientSize().x;  // the display size
  int h = frame->GetClientSize().y;
  int dMargin = 80;  // set a margin
  h -= (m_rButton->GetSize().GetY() +
        dMargin);  // height available for the scrolled window
  w -= dMargin;    // width available for the scrolled window
  m_sScrolledDialog->SetMinSize(
      wxSize(wxMin(w, scroll.x),
             wxMin(h, scroll.y)));  // set scrolled area size with margin

  Layout();
  Fit();
#ifdef __WXGTK__
  wxSize sd = GetSize();
  if (sd.y == GetClientSize().y) sd.y += 30;
  SetSize(wxSize(sd.x, sd.y));
  SetMinSize(wxSize(sd.x, sd.y));
#endif
  Refresh();
}

void GribRequestSetting::SetVpSize(PlugIn_ViewPort *vp) {
  double lonmax = vp->lon_max;
  double lonmin = vp->lon_min;
  if ((fabs(vp->lat_max) < 90.) && (fabs(lonmax) < 360.)) {
    if (lonmax < -180.) lonmax += 360.;
    if (lonmax > 180.) lonmax -= 360.;
  }
  if ((fabs(vp->lat_min) < 90.) && (fabs(lonmin) < 360.)) {
    if (lonmin < -180.) lonmin += 360.;
    if (lonmin > 180.) lonmin -= 360.;
  }

  bool bnew_val = false;
  if (m_spMaxLat->GetValue() != (int)ceil(vp->lat_max)) bnew_val = true;
  if (m_spMinLon->GetValue() != (int)floor(lonmin)) bnew_val = true;
  if (m_spMinLat->GetValue() != (int)floor(vp->lat_min)) bnew_val = true;
  if (m_spMaxLon->GetValue() != (int)ceil(lonmax)) bnew_val = true;

  if (bnew_val) {
    m_spMaxLat->SetValue((int)ceil(vp->lat_max));
    m_spMinLon->SetValue((int)floor(lonmin));
    m_spMinLat->SetValue((int)floor(vp->lat_min));
    m_spMaxLon->SetValue((int)ceil(lonmax));

    SetCoordinatesText();
    m_MailImage->SetValue(WriteMail());
  }
}

bool GribRequestSetting::MouseEventHook(wxMouseEvent &event) {
  if (m_ZoneSelMode == AUTO_SELECTION || m_ZoneSelMode == SAVED_SELECTION ||
      m_ZoneSelMode == START_SELECTION)
    return false;

  if (event.Moving())
    return false;  // maintain status bar and tracking dialog updated

  int xm, ym;
  event.GetPosition(&xm, &ym);
  xm *= m_displayScale;
  ym *= m_displayScale;

  // This does not work, but something like it should
  //     wxObject *obj = event.GetEventObject();
  //     wxWindow *win = wxDynamicCast(obj, wxWindow);
  //     if( win && (win != PluginGetFocusCanvas()))
  //         return false;

  if (event.LeftDown()) {
    m_parent.pParent->SetFocus();
    m_ZoneSelMode = DRAW_SELECTION;  // restart a new drawing
    m_parent.SetRequestBitmap(m_ZoneSelMode);
    if (this->IsShown())
      this->Hide();           // eventually hide diaog in case of mode change
    m_RenderZoneOverlay = 0;  // eventually hide previous drawing
  }

  if (event.LeftUp() && m_RenderZoneOverlay == 2) {
    m_ZoneSelMode = COMPLETE_SELECTION;  // ask to complete selection
    m_parent.SetRequestBitmap(m_ZoneSelMode);
    SetCoordinatesText();
    m_MailImage->SetValue(WriteMail());
    m_RenderZoneOverlay = 1;
  }

  if (event.Dragging()) {
    if (m_RenderZoneOverlay < 2) {
      m_StartPoint =
          wxPoint(xm, ym);  // event.GetPosition();  // starting selection point
      m_RenderZoneOverlay = 2;
    }
    m_IsMaxLong = m_StartPoint.x > xm
                      ? true
                      : false;  // find if startpoint is max longitude
    GetCanvasLLPix(m_Vp, wxPoint(xm, ym) /*event.GetPosition()*/, &m_Lat,
                   &m_Lon);  // extend selection
    if (!m_tMouseEventTimer.IsRunning())
      m_tMouseEventTimer.Start(20, wxTIMER_ONE_SHOT);
  }
  return true;
}

void GribRequestSetting::OnMouseEventTimer(wxTimerEvent &event) {
  // compute zone starting point lon/lat for zone drawing
  double lat, lon;
  GetCanvasLLPix(m_Vp, m_StartPoint, &lat, &lon);

  // compute rounded coordinates
  if (lat > m_Lat) {
    m_spMaxLat->SetValue((int)ceil(lat));
    m_spMinLat->SetValue((int)floor(m_Lat));
  } else {
    m_spMaxLat->SetValue((int)ceil(m_Lat));
    m_spMinLat->SetValue((int)floor(lat));
  }
  if (m_IsMaxLong) {
    m_spMaxLon->SetValue((int)ceil(lon));
    m_spMinLon->SetValue((int)floor(m_Lon));
  } else {
    m_spMaxLon->SetValue((int)ceil(m_Lon));
    m_spMinLon->SetValue((int)floor(lon));
  }

  RequestRefresh(GetGRIBCanvas());
}

void GribRequestSetting::SetCoordinatesText() {
  m_stMaxLatNS->SetLabel(m_spMaxLat->GetValue() < 0 ? _("S") : _("N"));
  m_stMinLonEW->SetLabel(m_spMinLon->GetValue() < 0 ? _("W") : _("E"));
  m_stMaxLonEW->SetLabel(m_spMaxLon->GetValue() < 0 ? _("W") : _("E"));
  m_stMinLatNS->SetLabel(m_spMinLat->GetValue() < 0 ? _("S") : _("N"));
}

size_t LengthSelToHours(int sel) {
  switch (sel) {
    case 1:
      return 72;
    case 2:
      return 999;
    default:
      return 24;
  }
}

void GribRequestSetting::onDLEvent(OCPN_downloadEvent &ev) {
  // std::cout << "onDLEvent  " << ev.getDLEventCondition() << " "
  //           << ev.getDLEventStatus() << std::endl;
  switch (ev.getDLEventCondition()) {
    case OCPN_DL_EVENT_TYPE_END:
      m_bTransferSuccess =
          (ev.getDLEventStatus() == OCPN_DL_NO_ERROR) ? true : false;
      Disconnect(wxEVT_DOWNLOAD_EVENT,
                 (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::
                     onDLEvent);
      m_connected = false;
      m_downloading = false;
      m_download_handle = 0;
      wxYieldIfNeeded();
      break;

    case OCPN_DL_EVENT_TYPE_PROGRESS:
      if (ev.getTotal() != 0) {
        switch (m_downloadType) {
          case GribDownloadType::WORLD:
            m_staticTextInfo->SetLabelText(
                wxString::Format(_("Downloading... %li / %li"),
                                 ev.getTransferred(), ev.getTotal()));
            break;
          case GribDownloadType::LOCAL:
          case GribDownloadType::LOCAL_CATALOG:
            m_stLocalDownloadInfo->SetLabelText(
                wxString::Format(_("Downloading... %li / %li"),
                                 ev.getTransferred(), ev.getTotal()));
            break;
          case GribDownloadType::XYGRIB:
            // Update XyGrib progress gauge
            m_xygribPanel->m_progress_gauge->SetValue(
                100 * ev.getTransferred() / ev.getTotal());
            // Update status text to display information on file size
            m_xygribPanel->m_status_text->SetLabel(wxString::Format(
                "%s (%ld kB / %ld kB)",
                _("Downloading GRIB file").c_str().AsChar(),
                ev.getTransferred() / 1024, ev.getTotal() / 1024));
            break;
          default:
            break;
        }
      } else {
        if (ev.getTransferred() > 0) {
          switch (m_downloadType) {
            case GribDownloadType::WORLD:
              m_staticTextInfo->SetLabelText(wxString::Format(
                  _("Downloading... %li / ???"), ev.getTransferred()));
              break;
            case GribDownloadType::LOCAL:
            case GribDownloadType::LOCAL_CATALOG:
              m_stLocalDownloadInfo->SetLabelText(wxString::Format(
                  _("Downloading... %li / ???"), ev.getTransferred()));
              break;
            default:
              break;
          }
        }
      }
      wxYieldIfNeeded();
      break;
    default:
      break;
  }
}

void GribRequestSetting::OnWorldDownload(wxCommandEvent &event) {
  if (m_downloading) {
    OCPN_cancelDownloadFileBackground(m_download_handle);
    m_downloading = false;
    m_download_handle = 0;
    Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
    m_connected = false;
    m_btnDownloadWorld->SetLabelText(_("Download"));
    m_staticTextInfo->SetLabelText(_("Download canceled"));
    m_canceled = true;
    m_downloadType = GribDownloadType::NONE;
    EnableDownloadButtons();
    wxTheApp->ProcessPendingEvents();
    wxYieldIfNeeded();
    return;
  }
  m_canceled = false;
  m_downloading = true;
  m_downloadType = GribDownloadType::WORLD;
  EnableDownloadButtons();
  m_btnDownloadWorld->SetLabelText(_("Cancel"));
  m_staticTextInfo->SetLabelText(_("Preparing data on server..."));
  wxYieldIfNeeded();
  wxString model;
  switch (m_chECMWFResolution->GetSelection()) {
    case 0:
      model = "ecmwf0p25";
      break;
    case 1:
      model = "ecmwfaifs0p25";
      break;
    default:
      model = "ecmwf0p25";
      break;
  }
  std::ostringstream oss;
  oss << "https://grib.bosun.io/grib?";
  oss << "model=" << model;
  oss << "&latmin=" << m_Vp->lat_min;
  oss << "&latmax=" << m_Vp->lat_max;
  oss << "&lonmin=" << m_Vp->lon_min;
  oss << "&lonmax=" << m_Vp->lon_max;
  oss << "&length=" << LengthSelToHours(m_chForecastLength->GetSelection());
  wxString filename =
      wxString::Format("ocpn_%s_%li_%s.grb2", model.c_str(),
                       LengthSelToHours(m_chForecastLength->GetSelection()),
                       wxDateTime::Now().Format("%F-%H-%M"));
  wxString path = m_parent.GetGribDir();
  path.Append(wxFileName::GetPathSeparator());
  path.Append(filename);
  if (!m_connected) {
    m_connected = true;
    Connect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
  }
  auto res =
      OCPN_downloadFileBackground(oss.str(), path, this, &m_download_handle);
  while (m_downloading) {
    wxTheApp->ProcessPendingEvents();
    wxMilliSleep(10);
  }
  if (!m_canceled) {
    if (m_bTransferSuccess) {
      m_staticTextInfo->SetLabelText(
          wxString::Format(_("Download complete: %s"), path.c_str()));
      wxFileName fn(path);
      m_parent.m_grib_dir = fn.GetPath();
      m_parent.m_file_names.Clear();
      m_parent.m_file_names.Add(path);
      m_parent.OpenFile();
      if (m_parent.pPlugIn) {
        if (m_parent.pPlugIn->m_bZoomToCenterAtInit) m_parent.DoZoomToCenter();
      }
      m_parent.SetDialogsStyleSizePosition(true);
      Close();
    } else {
      m_staticTextInfo->SetLabelText(_("Download failed"));
    }
  }
  m_btnDownloadWorld->SetLabelText(_("Download"));
  m_downloadType = GribDownloadType::NONE;
  EnableDownloadButtons();
}

enum LocalSourceItem { SOURCE, AREA, GRIB };

enum LocalGribDownloadType { DIRECT, MANIFEST, WEBPAGE };

struct GribCatalogInfo : public wxTreeItemData {
  GribCatalogInfo(LocalSourceItem type, wxString name, wxString description,
                  wxString url, wxString filename,
                  LocalGribDownloadType download_type, double latmin,
                  double lonmin, double latmax, double lonmax)
      : type(type),
        name(name),
        description(description),
        url(url),
        filename(filename),
        download_type(download_type),
        latmin(latmin),
        lonmin(lonmin),
        latmax(latmax),
        lonmax(lonmax) {}
  LocalSourceItem type;
  wxString name;
  wxString description;
  wxString url;
  wxString filename;
  LocalGribDownloadType download_type;
  double latmin;
  double lonmin;
  double latmax;
  double lonmax;
};

void GribRequestSetting::FillTreeCtrl(wxJSONValue &data) {
  m_SourcesTreeCtrl1->DeleteAllItems();
  wxTreeItemId root =
      m_SourcesTreeCtrl1->AddRoot(_("Local high resolution forecasts"));
  if (data.HasMember("sources") && data["sources"].IsArray()) {
    for (int i = 0; i < data["sources"].Size(); i++) {
      wxJSONValue source = data["sources"][i];
      auto info = new GribCatalogInfo(
          LocalSourceItem::SOURCE, source["source"].AsString(),
          source["description"].AsString(), source["url"].AsString(),
          wxEmptyString, LocalGribDownloadType::WEBPAGE, 0, 0, 0, 0);
      wxTreeItemId src_id = m_SourcesTreeCtrl1->AppendItem(
          root, source["source"].AsString(), -1, -1, info);
      if (source.HasMember("areas") && source["areas"].IsArray()) {
        for (int j = 0; j < source[_T("areas")].Size(); j++) {
          wxJSONValue area = source[_T("areas")][j];
          auto info = new GribCatalogInfo(
              LocalSourceItem::AREA, area["name"].AsString(),
              source["description"].AsString(), source["url"].AsString(),
              wxEmptyString, LocalGribDownloadType::WEBPAGE,
              area["boundary"]["lat_min"].AsDouble(),
              area["boundary"]["lon_min"].AsDouble(),
              area["boundary"]["lat_max"].AsDouble(),
              area["boundary"]["lon_max"].AsDouble());
          m_SourcesTreeCtrl1->AppendItem(src_id, area["name"].AsString(), -1,
                                         -1, info);
          if (area.HasMember("gribs") && area["gribs"].IsArray()) {
            for (int k = 0; k < area["gribs"].Size(); k++) {
              wxJSONValue grib = area["gribs"][k];
              auto info = new GribCatalogInfo(
                  LocalSourceItem::GRIB, grib["name"].AsString(),
                  source["description"].AsString(),
                  grib.HasMember("url") ? grib["url"].AsString()
                                        : grib["cat_url"].AsString(),
                  grib.HasMember("filename") ? grib["filename"].AsString() : "",
                  grib.HasMember("url") ? LocalGribDownloadType::DIRECT
                                        : LocalGribDownloadType::MANIFEST,
                  area["boundary"]["lat_min"].AsDouble(),
                  area["boundary"]["lon_min"].AsDouble(),
                  area["boundary"]["lat_max"].AsDouble(),
                  area["boundary"]["lon_max"].AsDouble());
              m_SourcesTreeCtrl1->AppendItem(
                  m_SourcesTreeCtrl1->GetLastChild(src_id),
                  grib[_T("name")].AsString(), -1, -1, info);
            }
          }
        }
      }
      m_SourcesTreeCtrl1->CollapseAllChildren(src_id);
    }
  }
  m_SourcesTreeCtrl1->Expand(root);
}

void GribRequestSetting::ReadLocalCatalog() {
  wxJSONReader reader;
  wxFileInputStream str(m_parent.pPlugIn->m_local_sources_catalog);
  wxJSONValue root;
  reader.Parse(str, &root);
  FillTreeCtrl(root);
}

void GribRequestSetting::HighlightArea(double latmax, double lonmax,
                                       double latmin, double lonmin) {
  m_parent.m_highlight_latmax = latmax;
  m_parent.m_highlight_lonmax = lonmax;
  m_parent.m_highlight_latmin = latmin;
  m_parent.m_highlight_lonmin = lonmin;
}

void GribRequestSetting::OnLocalTreeSelChanged(wxTreeEvent &event) {
  wxTreeItemId item = m_SourcesTreeCtrl1->GetSelection();
  auto src = (GribCatalogInfo *)(m_SourcesTreeCtrl1->GetItemData(item));
  if (src) {
    if (src->type == LocalSourceItem::GRIB) {
      m_stLocalDownloadInfo->SetLabelText(_("Download grib..."));
      m_bLocal_source_selected = true;
      HighlightArea(src->latmax, src->lonmax, src->latmin, src->lonmin);
    } else {
      m_stLocalDownloadInfo->SetLabelText(_("Select grib..."));
      m_bLocal_source_selected = false;
      HighlightArea(src->latmax, src->lonmax, src->latmin, src->lonmin);
    }
  }
  EnableDownloadButtons();
}

void GribRequestSetting::OnUpdateLocalCatalog(wxCommandEvent &event) {
  if (m_downloading) {
    OCPN_cancelDownloadFileBackground(m_download_handle);
    m_downloading = false;
    m_download_handle = 0;
    Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
    m_connected = false;
    m_btnDownloadLocal->SetLabelText(_("Download"));
    m_stLocalDownloadInfo->SetLabelText(_("Download canceled"));
    m_canceled = true;
    m_downloadType = GribDownloadType::NONE;
    EnableDownloadButtons();
    wxTheApp->ProcessPendingEvents();
    wxYieldIfNeeded();
    return;
  }
  m_canceled = false;
  m_downloading = true;
  m_downloadType = GribDownloadType::LOCAL_CATALOG;
  EnableDownloadButtons();
  m_btnDownloadLocal->SetLabelText(_("Cancel"));
  m_staticTextInfo->SetLabelText(_("Downloading catalog update..."));
  wxYieldIfNeeded();
  if (!m_connected) {
    m_connected = true;
    Connect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
  }
  auto res = OCPN_downloadFileBackground(
      CATALOG_URL, m_parent.pPlugIn->m_local_sources_catalog + "new", this,
      &m_download_handle);
  while (m_downloading) {
    wxTheApp->ProcessPendingEvents();
    wxMilliSleep(10);
  }
  if (!m_canceled) {
    if (m_bTransferSuccess) {
      wxRenameFile(m_parent.pPlugIn->m_local_sources_catalog + "new",
                   m_parent.pPlugIn->m_local_sources_catalog, true);
      ReadLocalCatalog();
      m_stLocalDownloadInfo->SetLabelText(_("Catalog update complete."));
    } else {
      m_stLocalDownloadInfo->SetLabelText(_("Download failed"));
    }
  }
  m_btnDownloadLocal->SetLabelText(_("Download"));
  m_downloadType = GribDownloadType::NONE;
  EnableDownloadButtons();
}

void GribRequestSetting::OnDownloadLocal(wxCommandEvent &event) {
  if (m_downloading) {
    OCPN_cancelDownloadFileBackground(m_download_handle);
    m_downloading = false;
    m_download_handle = 0;
    Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
    m_connected = false;
    m_btnDownloadLocal->SetLabelText(_("Download"));
    m_stLocalDownloadInfo->SetLabelText(_("Download canceled"));
    m_canceled = true;
    m_downloadType = GribDownloadType::NONE;
    EnableDownloadButtons();
    wxTheApp->ProcessPendingEvents();
    wxYieldIfNeeded();
    return;
  }
  m_canceled = false;
  m_downloading = true;
  m_downloadType = GribDownloadType::LOCAL;
  EnableDownloadButtons();
  m_btnDownloadLocal->SetLabelText(_("Cancel"));
  m_staticTextInfo->SetLabelText(_("Downloading grib..."));
  wxYieldIfNeeded();
  auto src = (GribCatalogInfo *)(m_SourcesTreeCtrl1->GetItemData(
      m_SourcesTreeCtrl1->GetSelection()));
  if (!src || src->type != LocalSourceItem::GRIB || src->url.IsEmpty()) {
    m_downloading = false;
    m_stLocalDownloadInfo->SetLabelText(_("Download can't be started."));
    m_btnDownloadWorld->SetLabelText(_("Download"));
    m_downloadType = GribDownloadType::NONE;
    EnableDownloadButtons();
    return;
  }
  if (!m_connected) {
    m_connected = true;
    Connect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
  }
  wxString url = src->url;
  // If it is a grib that changes location and requires help of, download the
  // manifest first
  if (src->download_type == LocalGribDownloadType::MANIFEST) {
    wxString path = m_parent.GetGribDir();
    path.Append(wxFileName::GetPathSeparator());
    path.Append("grib_manifest.json");
    auto res = OCPN_downloadFileBackground(url, path, this, &m_download_handle);
    while (m_downloading) {
      wxTheApp->ProcessPendingEvents();
      wxMilliSleep(10);
    }
    bool success = true;
    if (!m_canceled) {
      if (!m_bTransferSuccess) {
        success = false;
        m_stLocalDownloadInfo->SetLabelText(_("Download failed"));
      }
    } else {
      success = false;
      m_stLocalDownloadInfo->SetLabelText(_("Download canceled"));
    }
    if (success) {
      wxJSONReader reader;
      wxFileInputStream str(path);
      wxJSONValue root;
      reader.Parse(str, &root);
      if (root.HasMember("url")) {
        wxString parsed = root["url"].AsString();
        if (parsed.StartsWith("http")) {
          url = parsed;
        } else {
          m_stLocalDownloadInfo->SetLabelText(_("Download failed"));
          success = false;
        }
      }
    }
    if (!success) {  // Something went wrong, clean up and do not continue to
                     // the actual download
      m_downloading = false;
      m_download_handle = 0;
      Disconnect(wxEVT_DOWNLOAD_EVENT,
                 (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::
                     onDLEvent);
      m_connected = false;
      m_btnDownloadLocal->SetLabelText(_("Download"));
      m_stLocalDownloadInfo->SetLabelText(_("Download failed"));
      m_canceled = true;
      m_downloadType = GribDownloadType::NONE;
      EnableDownloadButtons();
      wxTheApp->ProcessPendingEvents();
      wxYieldIfNeeded();
      return;
    }
  }
  // Download the actual grib
  m_downloading = true;
  wxYieldIfNeeded();
  if (!m_connected) {
    m_connected = true;
    Connect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
  }
  wxString filename;
  if (!src->filename.IsEmpty()) {
    filename = src->filename;
  } else {
    filename =
        url.AfterLast('/');  // Get last part of the URL and try to sanitize the
                             // filename somewhat if we call some API...
    filename.Replace("?", "_");
    filename.Replace("&", "_");
    if (!(filename.Contains(".grb2") || filename.Contains(".grib2") ||
          filename.Contains(".grb") || filename.Contains(".grib"))) {
      filename.Append(".grb");
    }
  }

  wxString path = m_parent.GetGribDir();
  path.Append(wxFileName::GetPathSeparator());
  path.Append(filename);
  auto res = OCPN_downloadFileBackground(url, path, this, &m_download_handle);
  while (m_downloading) {
    wxTheApp->ProcessPendingEvents();
    wxMilliSleep(10);
  }
  if (!m_canceled) {
    if (m_bTransferSuccess) {
      m_stLocalDownloadInfo->SetLabelText(_("Grib download complete."));
      m_stLocalDownloadInfo->SetLabelText(
          wxString::Format(_("Download complete: %s"), path.c_str()));
      wxFileName fn(path);
      m_parent.m_grib_dir = fn.GetPath();
      m_parent.m_file_names.Clear();
      m_parent.m_file_names.Add(
          path);  //("/home/nohal/Downloads/Cherbourg_4km_WRF_WAM_240228-12.grb.bz2");
      m_parent.OpenFile();
      if (m_parent.pPlugIn) {
        if (m_parent.pPlugIn->m_bZoomToCenterAtInit) m_parent.DoZoomToCenter();
      }
      m_parent.SetDialogsStyleSizePosition(true);
      Close();
    } else {
      m_stLocalDownloadInfo->SetLabelText(_("Download failed"));
    }
  }
  m_btnDownloadWorld->SetLabelText(_("Download"));
  m_downloadType = GribDownloadType::NONE;
  EnableDownloadButtons();
}

void GribRequestSetting::EnableDownloadButtons() {
  switch (m_downloadType) {
    case GribDownloadType::WORLD:
      m_btnDownloadWorld->Enable(true);
      m_btnDownloadLocal->Enable(false);
      m_buttonUpdateCatalog->Enable(false);
      break;
    case GribDownloadType::LOCAL:
    case GribDownloadType::LOCAL_CATALOG:
      m_btnDownloadWorld->Enable(false);
      m_btnDownloadLocal->Enable(m_bLocal_source_selected || m_downloading);
      m_buttonUpdateCatalog->Enable(false);
      break;
    case GribDownloadType::XYGRIB:
      m_xygribPanel->m_download_button->Enable(true);
      break;
    default:
      m_btnDownloadWorld->Enable(true);
      m_btnDownloadLocal->Enable(m_bLocal_source_selected || m_downloading);
      m_buttonUpdateCatalog->Enable(true);
      m_xygribPanel->m_download_button->Enable(true);
      break;
  }
}

void GribRequestSetting::StopGraphicalZoneSelection() {
  m_RenderZoneOverlay = 0;  // eventually stop graphical zone display

  RequestRefresh(GetGRIBCanvas());
}

void GribRequestSetting::OnVpChange(PlugIn_ViewPort *vp) {
  if (!vp || m_Vp == vp) return;

  delete m_Vp;
  m_Vp = new PlugIn_ViewPort(*vp);

  GetCanvasPixLL(m_Vp, &m_StartPoint, m_spMaxLat->GetValue(),
                 m_spMinLon->GetValue());

  if (!m_AllowSend) return;
  if (m_cManualZoneSel->GetValue()) return;

  SetVpSize(vp);
  // Viewport has changed : XyGrib estimated size must be recalculated
  UpdateGribSizeEstimate();
}

void GribRequestSetting::ApplyRequestConfig(unsigned rs, unsigned it,
                                            unsigned tr) {
  // some useful  strings
  const wxString res[][RESOLUTIONS] = {
      {_T("0.25"), _T("0.5"), _T("1.0"), _T("2.0")},
      {_T("0.2"), _T("0.8"), _T("1.6"), wxEmptyString},
      {_T("0.08"), _T("0.24"), _T("1.0"), wxEmptyString},         // RTOFS
      {_T("0.03"), _T("0.24"), _T("1.0"), wxEmptyString},         // HRRR
      {_T("0.0625"), _T("0.125"), wxEmptyString, wxEmptyString},  // ICON
      {_T("0.4"), _T("1.0"), _T("2.0"), wxEmptyString}            // ECMWF
  };

  IsZYGRIB = m_pMailTo->GetCurrentSelection() == ZYGRIB;
  if (IsZYGRIB)
    m_pModel->SetSelection(GFS);  // Model is always GFS when Zygrib selected
  IsGFS = m_pModel->GetCurrentSelection() == GFS;
  bool IsRTOFS = m_pModel->GetCurrentSelection() == RTOFS;
  bool IsHRRR = m_pModel->GetCurrentSelection() == HRRR;
  bool IsICON = m_pModel->GetCurrentSelection() == ICON;
  bool IsECMWF = m_pModel->GetCurrentSelection() == ECMWF;

  // populate resolution choice
  m_pResolution->Clear();
  if (m_pModel->GetCurrentSelection() >= 0) {
    for (int i = 0; i < RESOLUTIONS; i++) {
      if (res[m_pModel->GetCurrentSelection()][i] != wxEmptyString) {
        wxString s = res[m_pModel->GetCurrentSelection()][i];
        m_pResolution->Append(s);
      }
    }
  }
  m_pResolution->SetSelection(rs);

  unsigned l;
  // populate time interval choice
  l = (IsGFS || IsRTOFS || IsICON || IsECMWF) ? 3 : IsHRRR ? 1 : 6;

  unsigned m;
  m = IsHRRR ? 2 : 25;

  m_pInterval->Clear();
  for (unsigned i = l; i < m; i *= 2)
    m_pInterval->Append(wxString::Format(_T("%d"), i));
  m_pInterval->SetSelection(wxMin(it, m_pInterval->GetCount() - 1));

  // populate time range choice
  l = IsZYGRIB  ? 8
      : IsGFS   ? 16
      : IsRTOFS ? 6
      : IsECMWF ? 10
      : IsICON  ? 7
      : IsHRRR  ? 2
                : 3;
  m_pTimeRange->Clear();
  for (unsigned i = 2; i < l + 1; i++)
    m_pTimeRange->Append(wxString::Format(_T("%d"), i));
  m_pTimeRange->SetSelection(wxMin(l - 2, tr));

  m_pModel->Enable(!IsZYGRIB);
  m_pWind->SetValue(!IsRTOFS);
  m_pPress->SetValue(!IsRTOFS);
  m_pWaves->SetValue(m_RequestConfigBase.GetChar(8) == 'X' && IsGFS);
  m_pWaves->Enable(IsECMWF ||
                   (IsGFS && m_pTimeRange->GetCurrentSelection() < 7));
  // gfs & time range less than 8 days
  m_pRainfall->SetValue(m_RequestConfigBase.GetChar(9) == 'X' &&
                        (IsGFS || IsHRRR));
  m_pRainfall->Enable(IsGFS || IsHRRR);
  m_pCloudCover->SetValue(m_RequestConfigBase.GetChar(10) == 'X' && IsGFS);
  m_pCloudCover->Enable(IsGFS);
  m_pAirTemp->SetValue(m_RequestConfigBase.GetChar(11) == 'X' &&
                       (IsGFS || IsHRRR || IsICON || IsECMWF));
  m_pAirTemp->Enable(IsGFS || IsHRRR || IsICON || IsECMWF);
  m_pSeaTemp->SetValue(m_RequestConfigBase.GetChar(12) == 'X' &&
                       ((!IsZYGRIB && IsGFS) || IsRTOFS || IsHRRR || IsICON));
  m_pSeaTemp->Enable(!IsZYGRIB && (IsGFS || IsHRRR || IsICON));
  m_pWindGust->SetValue(m_RequestConfigBase.GetChar(14) == 'X' &&
                        (IsGFS || IsHRRR || IsICON));
  m_pWindGust->Enable(IsGFS || IsHRRR || IsICON);
  m_pCAPE->SetValue(m_RequestConfigBase.GetChar(15) == 'X' &&
                    (IsGFS || IsHRRR));
  m_pCAPE->Enable(IsGFS || IsHRRR);
  m_pReflectivity->Enable(IsGFS || IsHRRR);

  m_pAltitudeData->SetValue(
      (IsGFS || IsICON || IsECMWF)
          ? m_RequestConfigBase.GetChar(17) == 'X'
          : false);  // altitude data zigrib + saildocs GFS and ICON
  m_pAltitudeData->Enable(IsGFS || IsICON || IsECMWF);
  m_p850hpa->SetValue(IsZYGRIB ? m_RequestConfigBase.GetChar(18) == 'X'
                               : false);  // only zygrib
  m_p850hpa->Enable(IsZYGRIB);
  m_p700hpa->SetValue(IsZYGRIB ? m_RequestConfigBase.GetChar(19) == 'X'
                               : false);  // only zigrib
  m_p700hpa->Enable(IsZYGRIB);
  m_p500hpa->SetValue((IsGFS || IsICON || IsECMWF)
                          ? m_RequestConfigBase.GetChar(20) == 'X'
                          : false);  // zigrib + saildocs GFS ICON ECMWF
  m_p500hpa->Enable(IsGFS || IsICON || IsECMWF);
  m_p300hpa->SetValue(IsZYGRIB ? m_RequestConfigBase.GetChar(21) == 'X'
                               : false);  // only zigrib
  m_p300hpa->Enable(IsZYGRIB);

  m_pCurrent->SetValue(IsRTOFS);
  m_pCurrent->Enable(false);

  // show parameters only if necessary
  m_cMovingGribEnabled->Show(!IsZYGRIB);  // show/hide Moving settings
  m_fgMovingParams->ShowItems(m_cMovingGribEnabled->IsChecked() &&
                              m_cMovingGribEnabled->IsShown());

  m_fgLog->ShowItems(IsZYGRIB);  // show/hide zigrib login

  m_pWModel->Show(IsZYGRIB && m_pWaves->IsChecked());  // show/hide waves model

  m_fgAltitudeData->ShowItems(
      m_pAltitudeData->IsChecked());  // show/hide altitude params
}

void GribRequestSetting::OnTopChange(wxCommandEvent &event) {
  // deactivate momentary ZyGrib option
  if (m_pMailTo->GetCurrentSelection() == ZYGRIB) {
    m_pMailTo->SetSelection(0);
    int mes = OCPNMessageBox_PlugIn(
        this,
        _("Sorry...\nZyGrib momentary stopped providing this service...\nOnly "
          "Saildocs option is available"),
        _("Warning"), wxOK);
  }
  ApplyRequestConfig(m_pResolution->GetCurrentSelection(),
                     m_pInterval->GetCurrentSelection(),
                     m_pTimeRange->GetCurrentSelection());

  m_cMovingGribEnabled->Show(m_pMailTo->GetCurrentSelection() == SAILDOCS);

  if (m_AllowSend) m_MailImage->SetValue(WriteMail());

  SetRequestDialogSize();
}

void GribRequestSetting::OnZoneSelectionModeChange(wxCommandEvent &event) {
  StopGraphicalZoneSelection();  // eventually stop graphical zone display

  if (!m_ZoneSelMode) SetVpSize(m_Vp);  // recompute zone

  if (event.GetId() == MANSELECT) {
    // set temporarily zone selection mode if manual selection set, put it
    // directly in "drawing" position else put it in "auto selection position
    m_ZoneSelMode =
        m_cManualZoneSel->GetValue() ? DRAW_SELECTION : AUTO_SELECTION;
    m_cUseSavedZone->SetValue(false);
  } else if (event.GetId() == SAVEDZONE) {
    // set temporarily zone selection mode if saved selection set, put it
    // directly in "no selection" position else put it directly in "drawing"
    // position
    m_ZoneSelMode =
        m_cUseSavedZone->GetValue() ? SAVED_SELECTION : DRAW_SELECTION;
  }
  m_parent.SetRequestBitmap(m_ZoneSelMode);  // set appopriate bitmap
  fgZoneCoordinatesSizer->ShowItems(
      m_ZoneSelMode != AUTO_SELECTION);  // show coordinate if necessary
  m_cUseSavedZone->Show(m_ZoneSelMode != AUTO_SELECTION);
  if (m_AllowSend) m_MailImage->SetValue(WriteMail());

  SetRequestDialogSize();
}

bool GribRequestSetting::DoRenderZoneOverlay() {
  wxPoint p;
  GetCanvasPixLL(m_Vp, &p, m_Lat, m_Lon);

  int x = (m_StartPoint.x < p.x) ? m_StartPoint.x : p.x;
  int y = (m_StartPoint.y < p.y) ? m_StartPoint.y : p.y;

  int zw = fabs((double)p.x - m_StartPoint.x);
  int zh = fabs((double)p.y - m_StartPoint.y);

  wxPoint center;
  center.x = x + (zw / 2);
  center.y = y + (zh / 2);

  wxFont fo = *OCPNGetFont(_("Dialog"), 10);
  fo.SetPointSize(
      (fo.GetPointSize() * m_displayScale / OCPN_GetWinDIPScaleFactor()));
  wxFont *font = &fo;
  wxColour pen_color, back_color;
  GetGlobalColor(_T ( "DASHR" ), &pen_color);
  GetGlobalColor(_T ( "YELO1" ), &back_color);

  int label_offsetx = 5, label_offsety = 1;

  double size;
  EstimateFileSize(&size);

  wxString label(_("Coord. "));
  label.Append(toMailFormat(1, m_spMaxLat->GetValue()) + _T(" "));
  label.Append(toMailFormat(0, m_spMinLon->GetValue()) + _T(" "));
  label.Append(toMailFormat(1, m_spMinLat->GetValue()) + _T(" "));
  label.Append(toMailFormat(0, m_spMaxLon->GetValue()) + _T("\n"));
  label.Append(_T("Estim. Size "))
      .Append((wxString::Format(_T("%1.2f " ), size) + _("MB")));

  if (m_pdc) {
    wxPen pen(pen_color);
    pen.SetWidth(3);
    m_pdc->SetPen(pen);
    m_pdc->SetBrush(*wxTRANSPARENT_BRUSH);
    m_pdc->DrawRectangle(x, y, zw, zh);

    int w, h, sl;
#ifdef __WXMAC__
    wxScreenDC sdc;
    sdc.GetMultiLineTextExtent(label, &w, &h, &sl, font);
#else
    m_pdc->GetMultiLineTextExtent(label, &w, &h, &sl, font);
    w *= OCPN_GetWinDIPScaleFactor();
    h *= OCPN_GetWinDIPScaleFactor();
#endif
    w += 2 * label_offsetx, h += 2 * label_offsety;
    x = center.x - (w / 2);
    y = center.y - (h / 2);

    h *= m_displayScale;
    w *= m_displayScale;

    wxBitmap bm(w, h);
    wxMemoryDC mdc(bm);
    mdc.Clear();

    mdc.SetFont(*font);
    mdc.SetBrush(back_color);
    mdc.SetPen(*wxTRANSPARENT_PEN);
    mdc.SetTextForeground(wxColor(0, 0, 0));
    mdc.DrawRectangle(0, 0, w, h);
    mdc.DrawLabel(label, wxRect(label_offsetx, label_offsety, w, h));

    wxImage im = bm.ConvertToImage();
    im.InitAlpha();
    w = im.GetWidth(), h = im.GetHeight();
    for (int j = 0; j < h; j++)
      for (int i = 0; i < w; i++) im.SetAlpha(i, j, 155);

    m_pdc->DrawBitmap(im, x, y, true);

  } else {
#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2

    if (!m_oDC) {
      m_oDC = new pi_ocpnDC();
    }

    m_oDC->SetVP(m_Vp);
    m_oDC->SetPen(wxPen(pen_color, 3));

    wxPoint outline[5];
    outline[0] = wxPoint(x, y);
    outline[1] = wxPoint(x + zw, y);
    outline[2] = wxPoint(x + zw, y + zh);
    outline[3] = wxPoint(x, y + zh);
    outline[4] = wxPoint(x, y);
    m_oDC->DrawLines(5, outline);

    m_oDC->SetFont(*font);
    int w, h, ww, hw;
    m_oDC->GetTextExtent(label, &w, &h);
    h *= m_displayScale;
    w *= m_displayScale;

    m_oDC->GetTextExtent("W", &ww, &hw);

    int label_offsetx = ww, label_offsety = 1;
    int x = center.x - w / 2;
    int y = center.y - h / 2;

    w += 2 * label_offsetx, h += 2 * label_offsety;

    m_oDC->SetBrush(wxBrush(back_color));
    m_oDC->DrawRoundedRectangle(x, y, w, h, 0);

    /* draw bounding rectangle */
    m_oDC->SetPen(wxPen(wxColour(0, 0, 0), 1));

    outline[0] = wxPoint(x, y);
    outline[1] = wxPoint(x + w, y);
    outline[2] = wxPoint(x + w, y + h);
    outline[3] = wxPoint(x, y + h);
    outline[4] = wxPoint(x, y);
    m_oDC->DrawLines(5, outline);

    m_oDC->DrawText(label, x + label_offsetx, y + label_offsety);

#endif
#endif
  }
  return true;
}

bool GribRequestSetting::RenderGlZoneOverlay() {
  if (m_RenderZoneOverlay == 0) return false;
  m_pdc = NULL;  // inform lower layers that this is OpenGL render
  return DoRenderZoneOverlay();
}

bool GribRequestSetting::RenderZoneOverlay(wxDC &dc) {
  if (m_RenderZoneOverlay == 0) return false;
  m_pdc = &dc;
  return DoRenderZoneOverlay();
}

void GribRequestSetting::OnMovingClick(wxCommandEvent &event) {
  m_fgMovingParams->ShowItems(m_cMovingGribEnabled->IsChecked() &&
                              m_cMovingGribEnabled->IsShown());

  if (m_AllowSend) m_MailImage->SetValue(WriteMail());
  SetRequestDialogSize();

  this->Refresh();
}

void GribRequestSetting::OnCoordinatesChange(wxSpinEvent &event) {
  SetCoordinatesText();

  StopGraphicalZoneSelection();  // eventually stop graphical zone display

  if (!m_AllowSend) return;

  m_MailImage->SetValue(WriteMail());
}

void GribRequestSetting::OnAnyChange(wxCommandEvent &event) {
  m_fgAltitudeData->ShowItems(m_pAltitudeData->IsChecked());

  m_pWModel->Show(IsZYGRIB && m_pWaves->IsChecked());

  if (m_AllowSend) m_MailImage->SetValue(WriteMail());

  SetRequestDialogSize();
}

void GribRequestSetting::OnTimeRangeChange(wxCommandEvent &event) {
  m_pWModel->Show(IsZYGRIB && m_pWaves->IsChecked());

  if (m_pModel->GetCurrentSelection() == 0) {  // gfs
    if (m_pTimeRange->GetCurrentSelection() >
        6) {  // time range more than 8 days
      m_pWaves->SetValue(0);
      m_pWaves->Enable(false);
      OCPNMessageBox_PlugIn(
          this,
          _("You request a forecast for more than 8 days horizon.\nThis is "
            "conflicting with Wave data which will be removed from your "
            "request.\nDon't forget that beyond the first 8 days, the "
            "resolution will be only 2.5\u00B0x2.5\u00B0\nand the time "
            "intervall 12 hours."),
          _("Warning!"));
    } else
      m_pWaves->Enable(true);
  }

  if (m_AllowSend) m_MailImage->SetValue(WriteMail());

  SetRequestDialogSize();
}

void GribRequestSetting::OnSaveMail(wxCommandEvent &event) {
  bool IsCOAMPS = m_pModel->GetCurrentSelection() == COAMPS;
  bool IsRTOFS = m_pModel->GetCurrentSelection() == RTOFS;
  bool IsICON = m_pModel->GetCurrentSelection() == ICON;
  bool IsECMWF = m_pModel->GetCurrentSelection() == ECMWF;
  m_RequestConfigBase.SetChar(
      0, (char)(m_pMailTo->GetCurrentSelection() + '0'));  // recipient
  m_cMovingGribEnabled->IsChecked()
      ? m_RequestConfigBase.SetChar(16, 'X')  // moving grib
      : m_RequestConfigBase.SetChar(16, '.');

  if (!IsZYGRIB)
    m_RequestConfigBase.SetChar(
        1, (char)(m_pModel->GetCurrentSelection() + '0'));  // model
  if (!IsECMWF)
    m_RequestConfigBase.SetChar(
        2, (char)(m_pResolution->GetCurrentSelection() + '0'));  // resolution

  m_RequestConfigBase.SetChar(3,
                              (char)(m_pInterval->GetCurrentSelection() + '0'));

  wxString range;
  range.Printf(_T("%x"), m_pTimeRange->GetCurrentSelection() +
                             1);  // range max = 2 to 16 stored in hexa 1 to f
  m_RequestConfigBase.SetChar(4, range.GetChar(0));

  if (IsZYGRIB && m_pWModel->IsShown())
    m_RequestConfigBase.SetChar(
        5, (char)(m_pWModel->GetCurrentSelection() + '0'));  // waves model

  m_RequestConfigBase.SetChar(
      6, 'X');  // wind must be always selected as a default
  m_RequestConfigBase.SetChar(
      7, 'X');  // pressure must be always selected as a default

  if (!IsCOAMPS && !IsRTOFS) {
    m_pWindGust->IsChecked() ? m_RequestConfigBase.SetChar(14, 'X')  // Gust
                             : m_RequestConfigBase.SetChar(14, '.');
    m_pWaves->IsChecked() ? m_RequestConfigBase.SetChar(8, 'X')  // waves
                          : m_RequestConfigBase.SetChar(8, '.');
    m_pRainfall->IsChecked() ? m_RequestConfigBase.SetChar(9, 'X')  // rainfall
                             : m_RequestConfigBase.SetChar(9, '.');
    m_pCloudCover->IsChecked() ? m_RequestConfigBase.SetChar(10, 'X')  // clouds
                               : m_RequestConfigBase.SetChar(10, '.');
    m_pAirTemp->IsChecked() ? m_RequestConfigBase.SetChar(11, 'X')  // air temp
                            : m_RequestConfigBase.SetChar(11, '.');
    m_pSeaTemp->IsChecked() ? m_RequestConfigBase.SetChar(12, 'X')  // sea temp
                            : m_RequestConfigBase.SetChar(12, '.');
    m_pCAPE->IsChecked() ? m_RequestConfigBase.SetChar(15, 'X')  // cape
                         : m_RequestConfigBase.SetChar(15, '.');
  }
  if (IsRTOFS)  // current
    m_pCurrent->IsChecked() ? m_RequestConfigBase.SetChar(13, 'X')
                            : m_RequestConfigBase.SetChar(13, '.');

  if (IsGFS || IsICON || IsECMWF) {
    m_pAltitudeData->IsChecked()
        ? m_RequestConfigBase.SetChar(17, 'X')  // altitude data
        : m_RequestConfigBase.SetChar(17, '.');
    m_p500hpa->IsChecked() ? m_RequestConfigBase.SetChar(20, 'X')
                           : m_RequestConfigBase.SetChar(20, '.');
  }
  if (IsZYGRIB) {
    m_p850hpa->IsChecked() ? m_RequestConfigBase.SetChar(18, 'X')
                           : m_RequestConfigBase.SetChar(18, '.');
    m_p700hpa->IsChecked() ? m_RequestConfigBase.SetChar(19, 'X')
                           : m_RequestConfigBase.SetChar(19, '.');
    m_p300hpa->IsChecked() ? m_RequestConfigBase.SetChar(21, 'X')
                           : m_RequestConfigBase.SetChar(21, '.');
  }

  wxFileConfig *pConf = GetOCPNConfigObject();
  if (pConf) {
    pConf->SetPath(_T( "/PlugIns/GRIB" ));

    pConf->Write(_T ( "MailRequestConfig" ), m_RequestConfigBase);
    pConf->Write(_T( "MailSenderAddress" ), m_pSenderAddress->GetValue());
    pConf->Write(_T( "MailRequestAddresses" ), m_MailToAddresses);
    pConf->Write(_T( "ZyGribLogin" ), m_pLogin->GetValue());
    pConf->Write(_T( "ZyGribCode" ), m_pCode->GetValue());
    pConf->Write(_T( "SendMailMethod" ), m_SendMethod);
    pConf->Write(_T( "MovingGribSpeed" ), m_sMovingSpeed->GetValue());
    pConf->Write(_T( "MovingGribCourse" ), m_sMovingCourse->GetValue());

    m_SavedZoneSelMode = m_cUseSavedZone->GetValue()    ? SAVED_SELECTION
                         : m_cManualZoneSel->GetValue() ? START_SELECTION
                                                        : AUTO_SELECTION;
    pConf->Write(_T( "ManualRequestZoneSizing" ), m_SavedZoneSelMode);

    pConf->Write(_T( "RequestZoneMaxLat" ), m_spMaxLat->GetValue());
    pConf->Write(_T( "RequestZoneMinLat" ), m_spMinLat->GetValue());
    pConf->Write(_T( "RequestZoneMaxLon" ), m_spMaxLon->GetValue());
    pConf->Write(_T( "RequestZoneMinLon" ), m_spMinLon->GetValue());
  }

  wxCloseEvent evt;
  OnClose(evt);
}

wxString GribRequestSetting::WriteMail() {
  // define size limits for zyGrib
  int limit = IsZYGRIB ? 2 : 0;  // new limit  2 mb

  m_MailError_Nb = 0;
  // some useful strings
  const wxString s[] = {_T(","), _T(" ")};  // separators
  const wxString p[][11] = {
      // parameters GFS from Saildocs
      {_T("APCP"), _T("TCDC"), _T("AIRTMP"), _T("HTSGW,WVPER,WVDIR"),
       _T("SEATMP"), _T("GUST"), _T("CAPE"), wxEmptyString, wxEmptyString,
       _T("WIND500,HGT500"), wxEmptyString},
      {},  // COAMPS
      {},  // RTOFS
      {},  // HRRR = same parameters as GFS
      // parametres ICON
      {_T(""), _T(""), _T("AIRTMP"), _T(""), _T("SFCTMP"), _T("GUST"), _T(""),
       _T(""), _T(""), _T("WIND500,HGT500"), _T("")},
      // parametres ECMWF
      {_T(""), _T(""), _T("TEMP"), _T("WAVES"), _T(""), _T(""), _T(""), _T(""),
       _T(""), _T("WIND500,HGT500"), _T("")},
      // parameters GFS from zygrib
      {_T("PRECIP"), _T("CLOUD"), _T("TEMP"), _T("WVSIG WVWIND"), wxEmptyString,
       _T("GUST"), _T("CAPE"), _T("A850"), _T("A700"), _T("A500"), _T("A300")}};

  wxString r_topmess, r_parameters, r_zone;
  // write the top part of the mail
  switch (m_pMailTo->GetCurrentSelection()) {
    case SAILDOCS:  // Saildocs
      r_zone = toMailFormat(1, m_spMaxLat->GetValue()) + _T(",") +
               toMailFormat(1, m_spMinLat->GetValue()) + _T(",") +
               toMailFormat(2, m_spMinLon->GetValue()) + _T(",") +
               toMailFormat(2, m_spMaxLon->GetValue());
      r_topmess = wxT("send ");
      r_topmess.Append(m_pModel->GetStringSelection() + _T(":"));
      r_topmess.Append(r_zone + _T("|"));
      r_topmess.Append(m_pResolution->GetStringSelection())
          .Append(_T(","))
          .Append(m_pResolution->GetStringSelection())
          .Append(_T("|"));
      double v;
      m_pInterval->GetStringSelection().ToDouble(&v);
      r_topmess.Append(wxString::Format(_T("0,%d,%d"), (int)v, (int)v * 2));
      m_pTimeRange->GetStringSelection().ToDouble(&v);
      r_topmess.Append(wxString::Format(_T("..%d"), (int)v * 24) + _T("|=\n"));
      break;
    case ZYGRIB:  // Zygrib
      double maxlon = (m_spMinLon->GetValue() > m_spMaxLon->GetValue() &&
                       m_spMaxLon->GetValue() < 0)
                          ? m_spMaxLon->GetValue() + 360
                          : m_spMaxLon->GetValue();
      r_zone = toMailFormat(1, m_spMinLat->GetValue()) +
               toMailFormat(2, m_spMinLon->GetValue()) + _T(" ") +
               toMailFormat(1, m_spMaxLat->GetValue()) +
               toMailFormat(2, maxlon);
      r_topmess = wxT("login : ");
      r_topmess.Append(m_pLogin->GetValue() + _T("\n"));
      r_topmess.Append(wxT("code :"));
      r_topmess.Append(m_pCode->GetValue() + _T("\n"));
      r_topmess.Append(wxT("area : "));
      r_topmess.append(r_zone + _T("\n"));
      r_topmess.Append(wxT("resol : "));
      r_topmess.append(m_pResolution->GetStringSelection() + _T("\n"));
      r_topmess.Append(wxT("days : "));
      r_topmess.append(m_pTimeRange->GetStringSelection() + _T("\n"));
      r_topmess.Append(wxT("hours : "));
      r_topmess.append(m_pInterval->GetStringSelection() + _T("\n"));
      if (m_pWaves->IsChecked()) {
        r_topmess.Append(wxT("waves : "));
        r_topmess.append(m_pWModel->GetStringSelection() + _T("\n"));
      }
      r_topmess.Append(wxT("meteo : "));
      r_topmess.append(m_pModel->GetStringSelection() + _T("\n"));
      if (m_pLogin->GetValue().IsEmpty() || m_pCode->GetValue().IsEmpty())
        m_MailError_Nb = 6;
      break;
  }
  // write the parameters part of the mail
  int GFSZ = IsZYGRIB ? 6 : 0;
  switch (m_pModel->GetCurrentSelection()) {
    case GFS:  // GFS
      r_parameters = wxT("WIND") + s[m_pMailTo->GetCurrentSelection()] +
                     wxT("PRESS");  // the default minimum request parameters
      if (m_pRainfall->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                            p[GFS + GFSZ][0]);
      if (m_pCloudCover->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                            p[GFS + GFSZ][1]);
      if (m_pAirTemp->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                            p[GFS + GFSZ][2]);
      if (m_pWaves->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                            p[GFS + GFSZ][3]);
      if (m_pSeaTemp->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                            p[GFS + GFSZ][4]);
      if (m_pWindGust->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                            p[GFS + GFSZ][5]);
      if (m_pCAPE->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                            p[GFS + GFSZ][6]);
      if (m_pAltitudeData->IsChecked()) {
        if (m_p850hpa->IsChecked())
          r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                              p[GFS + GFSZ][7]);
        if (m_p700hpa->IsChecked())
          r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                              p[GFS + GFSZ][8]);
        if (m_p500hpa->IsChecked())
          r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                              p[GFS + GFSZ][9]);
        if (m_p300hpa->IsChecked())
          r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                              p[GFS + GFSZ][10]);
      }
      break;
    case COAMPS:                         // COAMPS
      r_parameters = wxT("WIND,PRMSL");  // the default parameters for this
                                         // model
      break;
    case RTOFS:                        // RTOFS
      r_parameters = wxT("CUR,WTMP");  // the default parameters for this model
      break;
    case HRRR:                           // HRRR
      r_parameters = wxT("WIND,PRMSL");  // the default parameters for this
                                         // model
      if (m_pRainfall->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[GFS][0]);
      if (m_pAirTemp->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[GFS][2]);
      if (m_pSeaTemp->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[GFS][4]);
      if (m_pWindGust->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[GFS][5]);
      if (m_pCAPE->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[GFS][6]);
      break;
    case ICON:
      r_parameters = wxT("WIND,PRMSL");  // the default parameters for this
                                         // model
      if (m_pAirTemp->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[ICON][2]);
      if (m_pSeaTemp->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[ICON][4]);
      if (m_pWindGust->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[ICON][5]);
      if (m_pAltitudeData->IsChecked()) {
        if (m_p500hpa->IsChecked())
          r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[ICON][9]);
      }
      break;
    case ECMWF:
      r_parameters = wxT("WIND,MSLP");  // the default parameters for this
      // model
      if (m_pAirTemp->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[ECMWF][2]);
      if (m_pAltitudeData->IsChecked()) {
        if (m_p500hpa->IsChecked())
          r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] +
                              p[ECMWF][9]);
      }
      if (m_pWaves->IsChecked())
        r_parameters.Append(s[m_pMailTo->GetCurrentSelection()] + p[ECMWF][3]);
      break;
  }
  if (!IsZYGRIB && m_cMovingGribEnabled->IsChecked())  // moving grib
    r_parameters.Append(wxString::Format(
        _T("|%d,%d"), m_sMovingSpeed->GetValue(), m_sMovingCourse->GetValue()));

  // line lenth limitation
  int j = 0;
  char c = m_pMailTo->GetCurrentSelection() == SAILDOCS ? ',' : ' ';
  for (size_t i = 0; i < r_parameters.Len(); i++) {
    if (r_parameters.GetChar(i) == '|')
      j--;  // do not split Saildocs "moving" values
    if (r_parameters.GetChar(i) == c) j++;
    if (j > 6) {  // no more than 6 parameters on the same line
      r_parameters.insert(i + 1, m_pMailTo->GetCurrentSelection() == SAILDOCS
                                     ? _T("=\n")
                                     : _T("\n"));
      break;
    }
  }

  double size;
  m_MailError_Nb += EstimateFileSize(&size);

  m_tFileSize->SetLabel(wxString::Format(_T("%1.2f " ), size) + _("MB"));

  if (IsZYGRIB) {
    m_tLimit->SetLabel(wxString(_T("( ")) + _("Max") +
                       wxString::Format(_T(" %d "), limit) + _("MB") +
                       _T(" )"));
    if (size > limit) m_MailError_Nb += 2;
  } else
    m_tLimit->SetLabel(wxEmptyString);

  return wxString(r_topmess + r_parameters);
}

int GribRequestSetting::EstimateFileSize(double *size) {
  if (!size) return 0;  // Wrong parameter
  *size = 0.;

  // too small zone ? ( mini 2 * resolutions )
  double reso, time, inter;
  m_pResolution->GetStringSelection().ToDouble(&reso);
  m_pTimeRange->GetStringSelection().ToDouble(&time);
  m_pInterval->GetStringSelection().ToDouble(&inter);

  double maxlon = m_spMaxLon->GetValue(), minlon = m_spMinLon->GetValue();
  double maxlat = m_spMaxLat->GetValue(), minlat = m_spMinLat->GetValue();
  if (maxlat - minlat < 0) return 3;  // maxlat must be > minlat
  double wlon = (maxlon > minlon ? 0 : 360) + maxlon - minlon;
  if (wlon > 180 || (maxlat - minlat > 180)) return 4;  // ovoid too big area

  if (fabs(wlon) < 2 * reso || maxlat - minlat < 2 * reso)
    return 5;  // ovoid too small area

  int npts = (int)(ceil(((double)(maxlat - minlat) / reso)) *
                   ceil(((double)(wlon) / reso)));

  if (m_pModel->GetCurrentSelection() == COAMPS)  // limited area for COAMPS
    npts = wxMin(npts, (int)(ceil(40.0 / reso) * ceil(40.0 / reso)));

  // Nombre de GribRecords
  int nbrec = (int)(time * 24 / inter) + 1;
  int nbPress = (m_pPress->IsChecked()) ? nbrec : 0;
  int nbWind = (m_pWind->IsChecked()) ? 2 * nbrec : 0;
  int nbwave = (m_pWaves->IsChecked()) ? 2 * nbrec : 0;
  int nbRain = (m_pRainfall->IsChecked()) ? nbrec - 1 : 0;
  int nbCloud = (m_pCloudCover->IsChecked()) ? nbrec - 1 : 0;
  int nbTemp = (m_pAirTemp->IsChecked()) ? nbrec : 0;
  int nbSTemp = (m_pSeaTemp->IsChecked()) ? nbrec : 0;
  int nbGUSTsfc = (m_pWindGust->IsChecked()) ? nbrec : 0;
  int nbCurrent = (m_pCurrent->IsChecked()) ? nbrec : 0;
  int nbCape = (m_pCAPE->IsChecked()) ? nbrec : 0;
  int nbAltitude =
      IsZYGRIB ? 5 * nbrec : 3 * nbrec;  // five data types are included in each
                                         // ZyGrib altitude request and only
                                         // three in sSaildocs's
  int head = 84;
  double estime = 0.0;
  int nbits;

  nbits = 13;
  estime += nbWind * (head + (nbits * npts) / 8 + 2);
  estime += nbCurrent * (head + (nbits * npts) / 8 + 2);

  nbits = 11;
  estime += nbTemp * (head + (nbits * npts) / 8 + 2);
  estime += nbSTemp * (head + (nbits * npts) / 8 + 2);

  nbits = 4;
  estime += nbRain * (head + (nbits * npts) / 8 + 2);

  nbits = 15;
  estime += nbPress * (head + (nbits * npts) / 8 + 2);

  nbits = 4;
  estime += nbCloud * (head + (nbits * npts) / 8 + 2);

  nbits = 7;
  estime += nbGUSTsfc * (head + (nbits * npts) / 8 + 2);

  nbits = 5;
  estime += nbCape * (head + (nbits * npts) / 8 + 2);

  nbits = 6;
  estime += nbwave * (head + (nbits * npts) / 8 + 2);

  if (m_pAltitudeData->IsChecked()) {
    int nbalt = 0;
    if (m_p850hpa->IsChecked()) nbalt++;
    if (m_p700hpa->IsChecked()) nbalt++;
    if (m_p500hpa->IsChecked()) nbalt++;
    if (m_p300hpa->IsChecked()) nbalt++;

    nbits = 12;
    estime += nbAltitude * nbalt * (head + (nbits * npts) / 8 + 2);
  }

  *size = estime / (1024. * 1024.);

  return 0;
}

const wxString EncodeURL(const wxString &uri) {
  static std::unordered_map<int, wxString> sEncodeMap = {
      {(int)'!', "%21"}, {(int)'#', "%23"},  {(int)'$', "%24"},
      {(int)'&', "%26"}, {(int)'\'', "%27"}, {(int)'(', "%28"},
      {(int)')', "%29"}, {(int)'*', "%2A"},  {(int)'+', "%2B"},
      {(int)',', "%2C"}, {(int)';', "%3B"},  {(int)'=', "%3D"},
      {(int)'?', "%3F"}, {(int)'@', "%40"},  {(int)'[', "%5B"},
      {(int)']', "%5D"}, {(int)' ', "%20"},  {(int)'|', "%7C"},
      {(int)':', "%3A"}, {(int)'\n', "%0A"}};

  wxString encoded;
  for (size_t i = 0; i < uri.length(); ++i) {
    wxChar ch = uri[i];
    std::unordered_map<int, wxString>::iterator iter = sEncodeMap.find((int)ch);
    if (iter != sEncodeMap.end()) {
      encoded << iter->second;
    } else {
      encoded << ch;
    }
  }
  return encoded;
}

void GribRequestSetting::OnSendMaiL(wxCommandEvent &event) {
  StopGraphicalZoneSelection();  // eventually stop graphical zone display

  if (!m_AllowSend) {
    m_rButtonCancel->Show();
    m_rButtonApply->Show();
    m_rButtonYes->SetLabel(_("Send"));

    m_MailImage->SetForegroundColour(
        wxColor(0, 0, 0));  // permit to send a (new) message
    m_AllowSend = true;

    m_MailImage->SetValue(WriteMail());
    SetRequestDialogSize();

    return;
  }

  const wxString error[] = {
      _T("\n\n"),
      _("Before sending an email to Zygrib you have to enter your Login and "
        "Code.\nPlease visit www.zygrib.org/ and follow instructions..."),
      _("Too big file! zyGrib limit is 2Mb!"),
      _("Error! Max Lat lower than Min Lat or Max Lon lower than Min Lon!"),
      _("Too large area! Each side must be less than 180\u00B0!"),
      _("Too small area for this resolution!")};

  ::wxBeginBusyCursor();

  m_MailImage->SetForegroundColour(wxColor(255, 0, 0));
  m_AllowSend = false;

  if (m_MailError_Nb) {
    if (m_MailError_Nb > 7) {
      m_MailImage->SetValue(error[1] + error[0] + error[m_MailError_Nb - 6]);
    } else {
      if (m_MailError_Nb == 6) m_MailError_Nb = 1;
      m_MailImage->SetValue(error[m_MailError_Nb]);
    }

    m_rButtonCancel->Hide();
    m_rButtonApply->Hide();
    m_rButtonYes->SetLabel(_("Continue..."));
    m_rButton->Layout();
    SetRequestDialogSize();

    ::wxEndBusyCursor();

    return;
  }

#ifdef __WXMAC__
  // macOS, at least Big Sur, requires the body to be URLEncoded, otherwise the
  // invocation of the mail application via sh/open in wxEmail fails due to
  // "invalid characters" in "filename" regardless of quotation used (which is
  // weird, but real)
  wxMailMessage *message = new wxMailMessage(
      (m_pMailTo->GetCurrentSelection() == SAILDOCS)
          ? _T("grib-request")
          : wxT("gribauto"),  // requested subject
      (m_pMailTo->GetCurrentSelection() == SAILDOCS)
          ? m_MailToAddresses.BeforeFirst(_T(';'))  // to request address
          : m_MailToAddresses.AfterFirst(_T(';')).BeforeFirst(_T(';')),
      EncodeURL(WriteMail()),  // message image
      m_pSenderAddress->GetValue());
#else
  wxMailMessage *message = new wxMailMessage(
      (m_pMailTo->GetCurrentSelection() == SAILDOCS)
          ? _T("grib-request")
          : wxT("gribauto"),  // requested subject
      (m_pMailTo->GetCurrentSelection() == SAILDOCS)
          ? m_MailToAddresses.BeforeFirst(_T(';'))  // to request address
          : m_MailToAddresses.AfterFirst(_T(';')).BeforeFirst(_T(';')),
      WriteMail(),  // message image
      m_pSenderAddress->GetValue());
#endif

  wxEmail mail;
  if (mail.Send(*message, m_SendMethod)) {
#ifdef __WXMSW__
    m_MailImage->SetValue(
        _("Your request is ready. An email is prepared in your email "
          "environment. \nYou have just to verify and send it...\nSave or "
          "Cancel to finish...or Continue..."));
#else
    if (m_SendMethod == 0) {
      m_MailImage->SetValue(
          _("Your request is ready. An email is prepared in your email "
            "environment. \nYou have just to verify and send it...\nSave or "
            "Cancel to finish...or Continue..."));
    } else {
      m_MailImage->SetValue(
          _("Your request was sent \n(if your system has an MTA configured and "
            "is able to send email).\nSave or Cancel to finish...or "
            "Continue..."));
    }
#endif
  } else {
    m_MailImage->SetValue(
        _("Request can't be sent. Please verify your email systeme "
          "parameters.\nYou should also have a look at your log file.\nSave or "
          "Cancel to finish..."));
    m_rButtonYes->Hide();
  }
  m_rButtonYes->SetLabel(_("Continue..."));
  m_rButton->Layout();
  SetRequestDialogSize();
  delete message;
  ::wxEndBusyCursor();
}

/// @brief Build XyGrib's GRIB URL from GUI selections
/// @return A string with the complete URL of the GRIB file
wxString GribRequestSetting::BuildXyGribUrl() {
  // Server's base address
  wxString urlStr =
      wxString::Format("http://grbsrv.opengribs.org/getmygribs2.php?");
  // Bounding box
  urlStr << wxString::Format("la1=%.0f", floor(m_Vp->lat_min));
  urlStr << wxString::Format("&la2=%.0f", ceil(m_Vp->lat_max));
  urlStr << wxString::Format("&lo1=%.0f", floor(m_Vp->lon_min));
  urlStr << wxString::Format("&lo2=%.0f", ceil(m_Vp->lon_max));

  // Atmospheric Model & resolution reference
  urlStr << wxString::Format(
      "&model=%s",
      xygribAtmModelList[m_selectedAtmModelIndex]
          ->reqName[m_xygribPanel->m_resolution_choice->GetSelection()]);
  // Interval
  urlStr << wxString::Format(
      "&intv=%d",
      xygribAtmModelList[m_selectedAtmModelIndex]
          ->interval[m_xygribPanel->m_interval_choice->GetSelection()]);
  // Length in days
  urlStr << wxString::Format(
      "&days=%d", m_xygribPanel->m_duration_choice->GetSelection() + 1);

  // Selected run
  // TODO : available runs depend on model
  wxString selStr = m_xygribPanel->m_run_choice->GetStringSelection();
  if (selStr.IsSameAs("18h", false)) {
    urlStr << "&cyc=18";
  } else if (selStr.IsSameAs("12h", false)) {
    urlStr << "&cyc=12";
  } else if (selStr.IsSameAs("6h", false)) {
    urlStr << "&cyc=06";
  } else if (selStr.IsSameAs("0h", false)) {
    urlStr << "&cyc=00";
  } else {
    urlStr << "&cyc=last";
  }

  // Atmospheric data fields
  urlStr << "&par=";
  if (m_xygribPanel->m_wind_cbox->IsEnabled() &&
      m_xygribPanel->m_wind_cbox->IsChecked())
    urlStr << "W;";
  if (m_xygribPanel->m_pressure_cbox->IsEnabled() &&
      m_xygribPanel->m_pressure_cbox->IsChecked()) {
    if (xygribAtmModelList[m_selectedAtmModelIndex]->altPressure) {
      urlStr << "p;";
    } else {
      urlStr << "P;";
    }
  }
  if (m_xygribPanel->m_precipitation_cbox->IsEnabled() &&
      m_xygribPanel->m_precipitation_cbox->IsChecked())
    urlStr << "R;";
  if (m_xygribPanel->m_cloudcover_cbox->IsEnabled() &&
      m_xygribPanel->m_cloudcover_cbox->IsChecked())
    urlStr << "C;";
  if (m_xygribPanel->m_temperature_cbox->IsEnabled() &&
      m_xygribPanel->m_temperature_cbox->IsChecked())
    urlStr << "T;";
  if (m_xygribPanel->m_cape_cbox->IsEnabled() &&
      m_xygribPanel->m_cape_cbox->IsChecked())
    urlStr << "c;";
  if (m_xygribPanel->m_reflectivity_cbox->IsEnabled() &&
      m_xygribPanel->m_reflectivity_cbox->IsChecked())
    urlStr << "r;";
  if (m_xygribPanel->m_gust_cbox->IsEnabled() &&
      m_xygribPanel->m_gust_cbox->IsChecked())
    urlStr << "G;";

  // Wave model a data fields
  if ((m_selectedWaveModelIndex >= 0) &&
      (xygribWaveModelList[m_selectedWaveModelIndex] != nullptr)) {
    wxString modelStr = wxString::Format(
        "&wmdl=%s", xygribWaveModelList[m_selectedWaveModelIndex]->reqName);
    wxString wParams = "";
    if (m_xygribPanel->m_waveheight_cbox->IsChecked()) {
      wParams << "s;";
    }
    if (m_xygribPanel->m_windwave_cbox->IsChecked()) {
      wParams << "h;d;p;";
    }
    if (wParams.length() > 0) {
      urlStr << wxString::Format("%s&wpar=%s", modelStr.c_str(),
                                 wParams.c_str());
    } else {
      urlStr << "&wmdl=none";
    }
  } else {
    urlStr << "&wmdl=none";
  }

  return urlStr;
}

/// @brief Return a resonably unique GRIB filename for a given configuration
/// @return Filename to be used for the current GRIB configuration
wxString GribRequestSetting::BuildGribFileName() {
  wxString selStr = m_xygribPanel->m_resolution_choice->GetStringSelection();
  selStr.Replace(".", "P");

  wxString fileName;
  if (m_selectedWaveModelIndex < 0) {
    fileName = wxString::Format(
        "XyGrib_%s_%s_%s.grb2", wxDateTime::Now().Format("%F-%H-%M"),
        m_xygribPanel->m_atmmodel_choice->GetStringSelection(), selStr);
  } else {
    fileName = wxString::Format(
        "XyGrib_%s_%s_%s_%s.grb2", wxDateTime::Now().Format("%F-%H-%M"),
        m_xygribPanel->m_atmmodel_choice->GetStringSelection(), selStr,
        m_xygribPanel->m_wavemodel_choice->GetStringSelection());
  }

  return fileName;
}

/// @brief Handle action of Download/Cancel button.
/// This function gathers the current GRIB configuration and handles the
/// downloading of the GRIB file. If a transfer is already ongoing, it cancels
/// it.
/// @param event Event data from the GUI loop
void GribRequestSetting::OnXyGribDownloadButton(wxCommandEvent &event) {
  // Check if we are already downloading a GRIB file
  if (m_downloading) {
    // Yes : it means that "Download" button has been changed to "Cancel"
    // button. So, let's cancel the on-going download
    OCPN_cancelDownloadFileBackground(m_download_handle);
    m_downloading = false;
    m_download_handle = 0;
    Disconnect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
    m_connected = false;
    m_xygribPanel->m_progress_gauge->SetValue(0);
    m_xygribPanel->m_download_button->SetLabelText(_("Download"));
    m_xygribPanel->m_status_text->SetLabelText(_("Download cancelled"));
    m_canceled = true;
    m_downloadType = GribDownloadType::NONE;
    EnableDownloadButtons();
    wxTheApp->ProcessPendingEvents();
    wxYieldIfNeeded();
    return;
  }
  // No : start a new download
  if (m_gribSizeEstimate > XYGRIB_MAX_DOWNLOADABLE_GRIB_SIZE_MB * 1024 * 1024) {
    m_xygribPanel->m_status_text->SetLabelText(
        wxString::Format(_("Can't download GRIB file bigger than %d MB"),
                         (int)XYGRIB_MAX_DOWNLOADABLE_GRIB_SIZE_MB));
    return;
  }

  // First we memorize the current configuration. This is the one which will be
  // saved in OpenCPN' config file at exit
  MemorizeXyGribConfiguration();
  m_canceled = false;
  m_downloading = true;
  m_downloadType = GribDownloadType::XYGRIB;
  EnableDownloadButtons();
  // Change "Download" button into "Cancel" button
  m_xygribPanel->m_download_button->SetLabelText(_("Cancel"));
  m_xygribPanel->m_status_text->SetLabelText(
      _("Preparing GRIB file on server..."));
  wxYieldIfNeeded();

  // Build the XyGrib DownloadURL from the current GUI configuration
  wxString requestUrl = BuildXyGribUrl();

  // First we download the temporary XML file that will tell us where to
  // download the file on the server Downloading the XML triggers the
  // construction of the GRIB file on the server.
  wxString filename = wxString::Format("ocpn_xygrib_%s.xml",
                                       wxDateTime::Now().Format("%F-%H-%M"));
  wxString path = m_parent.GetGribDir();
  path << wxFileName::GetPathSeparator();
  path << filename;
  if (!m_connected) {
    m_connected = true;
    Connect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
  }
  auto res = OCPN_downloadFileBackground(requestUrl.c_str(), path, this,
                                         &m_download_handle);

  // Wait for the file to be downloaded. Note that it can take some time because
  // server will not send it to us until the GRIB file is ready, or an error
  // occured.
  while (m_downloading) {
    wxTheApp->ProcessPendingEvents();
    wxMilliSleep(10);
  }

  if ((m_canceled) || (!m_bTransferSuccess)) {
    // Something went wrong : publish error on GUI
    m_xygribPanel->m_status_text->SetLabelText(_("Grib request failed"));
    m_xygribPanel->m_download_button->SetLabelText(_("Download"));
    m_downloadType = GribDownloadType::NONE;
    EnableDownloadButtons();
    wxRemoveFile(path);
    return;
  }

  // Read the XML file in a wxString
  wxFile xmlFile;
  wxString strXml;
  bool readOk = xmlFile.Open(path);
  if (readOk) {
    readOk &= xmlFile.ReadAll(&strXml);
    if (readOk) {
      xmlFile.Close();
    }
  }

  // Quick and dirty read of the XML file to check status (OK/NOK)
  wxString url;
  if (readOk && (((int)strXml.find("\"status\":true") == wxNOT_FOUND))) {
    // Status is NOK : stop download and report error to GUI
    wxString errorStr;
    int errPos = strXml.find("\"message\":\"");
    int errEnd = strXml.find("\"}");
    if ((errPos != wxNOT_FOUND) && (errEnd != wxNOT_FOUND)) {
      errPos += 11;
      errorStr = strXml.Mid(errPos, errEnd - errPos);
    } else {
      errorStr = "Unknown server error";
    }

    m_xygribPanel->m_status_text->SetLabelText(
        wxString::Format("%s (%s)", _("Server Error"), errorStr));
    m_xygribPanel->m_download_button->SetLabelText(_("Download"));
    m_downloadType = GribDownloadType::NONE;
    EnableDownloadButtons();
    wxRemoveFile(path);
    return;
  } else {
    // Extract GRIB URL from XML
    int urlPos = strXml.find("\"url\":\"http:");
    int urlEnd = strXml.find(".grb2\"");
    if ((urlPos != wxNOT_FOUND) && (urlEnd != wxNOT_FOUND)) {
      urlPos += 7;
      urlEnd += 5;
      url = strXml.Mid(urlPos, urlEnd - urlPos);
      url.Replace("\\/", "/");
    } else {
      readOk = false;
    }
  }

  wxRemoveFile(path);

  // Check if there was any error in the previous phase
  if (!readOk) {
    // Yes : stop download and report error to GUI
    m_xygribPanel->m_status_text->SetLabelText(
        _("Error parsing XML file from server"));
    m_xygribPanel->m_download_button->SetLabelText(_("Download"));
    m_downloadType = GribDownloadType::NONE;
    EnableDownloadButtons();
    return;
  }

  m_xygribPanel->m_status_text->SetLabelText(_("Downloading GRIB file"));

  // Build name of the GRIB file for disk storage
  path = m_parent.GetGribDir();
  path << wxFileName::GetPathSeparator();
  path << BuildGribFileName();

  // Start download
  m_canceled = false;
  m_downloading = true;
  if (!m_connected) {
    m_connected = true;
    Connect(
        wxEVT_DOWNLOAD_EVENT,
        (wxObjectEventFunction)(wxEventFunction)&GribRequestSetting::onDLEvent);
  }
  res =
      OCPN_downloadFileBackground(url.c_str(), path, this, &m_download_handle);
  // Wait for end of download. Note that the previously registered callback
  // "onDLEvent" will update the progress indicator on the GUI.
  while (m_downloading) {
    wxTheApp->ProcessPendingEvents();
    wxMilliSleep(10);
  }

  // Download finished : restore "Download" button
  m_xygribPanel->m_download_button->SetLabelText(_("Download"));

  if (!m_canceled) {
    if (m_bTransferSuccess) {
      // Transfer successfull : switch to GRIB display on chart
      m_xygribPanel->m_status_text->SetLabelText(_("Download complete"));
      wxFileName fn(path);
      m_parent.m_grib_dir = fn.GetPath();
      m_parent.m_file_names.Clear();
      m_parent.m_file_names.Add(path);
      m_parent.OpenFile();
      if (m_parent.pPlugIn) {
        if (m_parent.pPlugIn->m_bZoomToCenterAtInit) m_parent.DoZoomToCenter();
      }
      m_parent.SetDialogsStyleSizePosition(true);
      Close();
    } else {
      // Download failed : report error to GUI
      m_xygribPanel->m_status_text->SetLabelText(_("Download failed"));
    }
  }
  m_downloadType = GribDownloadType::NONE;
  EnableDownloadButtons();
}

/// @brief Handle action of changing atmospheric model selection.
/// This function update the GUI according to the selected atmospheric model
/// @param event Event data from the GUI loop
void GribRequestSetting::OnXyGribAtmModelChoice(wxCommandEvent &event) {
  AtmModelDef_t *selectedAtmModel = nullptr;

  // Find the model descriptor associated to the selected atmospheric model
  wxString atmModel = m_xygribPanel->m_atmmodel_choice->GetStringSelection();
  int modelIndex = 0;
  while ((selectedAtmModel = xygribAtmModelList[modelIndex]) != nullptr) {
    if (selectedAtmModel->name.IsSameAs(atmModel, true)) {
      break;
    }
    modelIndex++;
  }

  // If not found, use the first one. This is not supposed to happen.
  if (selectedAtmModel == nullptr) {
    selectedAtmModel = xygribAtmModelList[0];
    modelIndex = 0;
  }

  // Enable or disable parameters according to the model definition
  if (selectedAtmModel->wind)
    m_xygribPanel->m_wind_cbox->Enable();
  else
    m_xygribPanel->m_wind_cbox->Disable();

  if (selectedAtmModel->gust)
    m_xygribPanel->m_gust_cbox->Enable();
  else
    m_xygribPanel->m_gust_cbox->Disable();

  if (selectedAtmModel->pressure)
    m_xygribPanel->m_pressure_cbox->Enable();
  else
    m_xygribPanel->m_pressure_cbox->Disable();

  if (selectedAtmModel->temperature)
    m_xygribPanel->m_temperature_cbox->Enable();
  else
    m_xygribPanel->m_temperature_cbox->Disable();

  if (selectedAtmModel->cape)
    m_xygribPanel->m_cape_cbox->Enable();
  else
    m_xygribPanel->m_cape_cbox->Disable();

  if (selectedAtmModel->reflectivity)
    m_xygribPanel->m_reflectivity_cbox->Enable();
  else
    m_xygribPanel->m_reflectivity_cbox->Disable();

  if (selectedAtmModel->cloudCover)
    m_xygribPanel->m_cloudcover_cbox->Enable();
  else
    m_xygribPanel->m_cloudcover_cbox->Disable();

  if (selectedAtmModel->precipitation)
    m_xygribPanel->m_precipitation_cbox->Enable();
  else
    m_xygribPanel->m_precipitation_cbox->Disable();

  // Fill the resolution choice selection
  m_xygribPanel->m_resolution_choice->Clear();
  for (int i = 0; i < selectedAtmModel->nbRes; i++) {
    m_xygribPanel->m_resolution_choice->Insert(selectedAtmModel->resolution[i],
                                               i);
  }

  // Fill the duration choice selection
  m_xygribPanel->m_duration_choice->Clear();
  for (int i = 0; i < selectedAtmModel->duration; i++) {
    m_xygribPanel->m_duration_choice->Insert(wxString::Format("%d", i + 1), i);
  }

  // Fill the interval choice selection
  m_xygribPanel->m_interval_choice->Clear();
  for (int i = 0; i < selectedAtmModel->nbInterval; i++) {
    m_xygribPanel->m_interval_choice->Insert(
        wxString::Format("%dh", selectedAtmModel->interval[i]), i);
  }

  // Fill the run choice selection
  m_xygribPanel->m_run_choice->Clear();
  if (selectedAtmModel->runMask == XYGRIB_RUN_0_12) {
    m_xygribPanel->m_run_choice->Insert("0h", 0);
    m_xygribPanel->m_run_choice->Insert("12h", 1);
    m_xygribPanel->m_run_choice->Insert(_("Latest"), 2);
  } else {
    m_xygribPanel->m_run_choice->Insert("0h", 0);
    m_xygribPanel->m_run_choice->Insert("6h", 1);
    m_xygribPanel->m_run_choice->Insert("12h", 2);
    m_xygribPanel->m_run_choice->Insert("18h", 3);
    m_xygribPanel->m_run_choice->Insert(_("Latest"), 4);
  }

  if (modelIndex == m_parent.xyGribConfig.atmModelIndex) {
    ApplyXyGribConfiguration();
  } else {
    m_selectedAtmModelIndex = modelIndex;
    m_xygribPanel->m_resolution_choice->SetSelection(0);
    m_xygribPanel->m_duration_choice->SetSelection(
        m_xygribPanel->m_duration_choice->GetCount() - 1);
    m_xygribPanel->m_interval_choice->SetSelection(0);
    m_xygribPanel->m_run_choice->SetSelection(
        m_xygribPanel->m_run_choice->GetCount() - 1);
  }
  MemorizeXyGribConfiguration();
}

/// @brief Handle action of changing wave model selection.
/// This function update the GUI according to the selected wave model
/// @param event Event data from the GUI loop
void GribRequestSetting::OnXyGribWaveModelChoice(wxCommandEvent &event) {
  WaveModelDef_t *selectedModel = nullptr;

  // Find the model descriptor associated to the selected wave model
  wxString waveModel = m_xygribPanel->m_wavemodel_choice->GetStringSelection();
  int modelIndex = 0;
  while ((selectedModel = xygribWaveModelList[modelIndex]) != nullptr) {
    if (selectedModel->name.IsSameAs(waveModel, true)) {
      break;
    }
    modelIndex++;
  }

  // Model found in the table ?
  if (selectedModel == nullptr) {
    // If the model is not found in the table, disable wave model downloading
    m_selectedWaveModelIndex = -1;
    m_xygribPanel->m_waveheight_cbox->Disable();
    m_xygribPanel->m_windwave_cbox->Disable();
    MemorizeXyGribConfiguration();
    return;
  }

  m_selectedWaveModelIndex = modelIndex;

  // Else configure parameters according to model definition
  if (selectedModel->significantHeight) {
    m_xygribPanel->m_waveheight_cbox->Enable();
  } else {
    m_xygribPanel->m_waveheight_cbox->Disable();
  }

  if (selectedModel->windWaves) {
    m_xygribPanel->m_windwave_cbox->Enable();
  } else {
    m_xygribPanel->m_windwave_cbox->Disable();
  }
  MemorizeXyGribConfiguration();
}

/// @brief Handle action of changing a GRIB configuration parameter.
/// The function stores the new configuration and updates GRIB size estimate.
/// @param event Event data from the GUI loop
void GribRequestSetting::OnXyGribConfigChange(wxCommandEvent &event) {
  MemorizeXyGribConfiguration();
}

/// @brief Initialize GUI configuration with value read from configuration
void GribRequestSetting::InitializeXygribDialog() {
  AtmModelDef_t *selectedAtmModel = nullptr;
  WaveModelDef_t *selectedWaveModel = nullptr;

  // Fill selection of atmospheric models with the ones found in the model table
  m_xygribPanel->m_atmmodel_choice->Clear();
  int modelIndex = 0;
  while ((selectedAtmModel = xygribAtmModelList[modelIndex]) != nullptr) {
    m_xygribPanel->m_atmmodel_choice->Insert(selectedAtmModel->name,
                                             modelIndex);
    modelIndex++;
  }
  // Select the initial value from configuration
  m_selectedAtmModelIndex = m_parent.xyGribConfig.atmModelIndex;
  selectedAtmModel = xygribAtmModelList[m_selectedAtmModelIndex];

  // Fill selection of wave models with the ones found in the table
  m_xygribPanel->m_wavemodel_choice->Clear();
  modelIndex = 0;
  while ((selectedWaveModel = xygribWaveModelList[modelIndex]) != nullptr) {
    m_xygribPanel->m_wavemodel_choice->Insert(selectedWaveModel->name,
                                              modelIndex);
    modelIndex++;
  }
  // Add the "None" selection
  m_xygribPanel->m_wavemodel_choice->Insert("None", modelIndex);
  m_selectedWaveModelIndex = m_parent.xyGribConfig.waveModelIndex;
  selectedWaveModel = xygribWaveModelList[m_selectedWaveModelIndex];
  if (selectedWaveModel == nullptr) {
    m_selectedWaveModelIndex = -1;
  }

  // Fill the resolution choice selection
  m_xygribPanel->m_resolution_choice->Clear();
  for (int i = 0; i < selectedAtmModel->nbRes; i++) {
    m_xygribPanel->m_resolution_choice->Insert(selectedAtmModel->resolution[i],
                                               i);
  }

  // Fill the duration choice selection
  m_xygribPanel->m_duration_choice->Clear();
  for (int i = 0; i < selectedAtmModel->duration; i++) {
    m_xygribPanel->m_duration_choice->Insert(wxString::Format("%d", i + 1), i);
  }

  // Fill the interval choice selection
  m_xygribPanel->m_interval_choice->Clear();
  for (int i = 0; i < selectedAtmModel->nbInterval; i++) {
    m_xygribPanel->m_interval_choice->Insert(
        wxString::Format("%dh", selectedAtmModel->interval[i]), i);
  }

  // Fill the run choice selection
  m_xygribPanel->m_run_choice->Clear();
  if (selectedAtmModel->runMask == XYGRIB_RUN_0_12) {
    m_xygribPanel->m_run_choice->Insert("0h", 0);
    m_xygribPanel->m_run_choice->Insert("12h", 1);
    m_xygribPanel->m_run_choice->Insert(_("Latest"), 2);
  } else {
    m_xygribPanel->m_run_choice->Insert("0h", 0);
    m_xygribPanel->m_run_choice->Insert("6h", 1);
    m_xygribPanel->m_run_choice->Insert("12h", 2);
    m_xygribPanel->m_run_choice->Insert("18h", 3);
    m_xygribPanel->m_run_choice->Insert(_("Latest"), 4);
  }

  if (selectedAtmModel->wind)
    m_xygribPanel->m_wind_cbox->Enable();
  else
    m_xygribPanel->m_wind_cbox->Disable();

  if (selectedAtmModel->gust)
    m_xygribPanel->m_gust_cbox->Enable();
  else
    m_xygribPanel->m_gust_cbox->Disable();

  if (selectedAtmModel->pressure)
    m_xygribPanel->m_pressure_cbox->Enable();
  else
    m_xygribPanel->m_pressure_cbox->Disable();

  if (selectedAtmModel->temperature)
    m_xygribPanel->m_temperature_cbox->Enable();
  else
    m_xygribPanel->m_temperature_cbox->Disable();

  if (selectedAtmModel->cape)
    m_xygribPanel->m_cape_cbox->Enable();
  else
    m_xygribPanel->m_cape_cbox->Disable();

  if (selectedAtmModel->reflectivity)
    m_xygribPanel->m_reflectivity_cbox->Enable();
  else
    m_xygribPanel->m_reflectivity_cbox->Disable();

  if (selectedAtmModel->cloudCover)
    m_xygribPanel->m_cloudcover_cbox->Enable();
  else
    m_xygribPanel->m_cloudcover_cbox->Disable();

  if (selectedAtmModel->precipitation)
    m_xygribPanel->m_precipitation_cbox->Enable();
  else
    m_xygribPanel->m_precipitation_cbox->Disable();

  if ((selectedWaveModel != nullptr) && (selectedWaveModel->significantHeight))
    m_xygribPanel->m_waveheight_cbox->Enable();
  else
    m_xygribPanel->m_waveheight_cbox->Disable();

  if ((selectedWaveModel != nullptr) && (selectedWaveModel->windWaves))
    m_xygribPanel->m_windwave_cbox->Enable();
  else
    m_xygribPanel->m_windwave_cbox->Disable();

  ApplyXyGribConfiguration();
}

/// @brief Apply the configuration stored into the configuration structure to
/// the UI
void GribRequestSetting::ApplyXyGribConfiguration() {
  m_xygribPanel->m_atmmodel_choice->SetSelection(
      m_parent.xyGribConfig.atmModelIndex);
  m_xygribPanel->m_wavemodel_choice->SetSelection(
      m_parent.xyGribConfig.waveModelIndex);

  m_xygribPanel->m_wind_cbox->SetValue(m_parent.xyGribConfig.wind);
  m_xygribPanel->m_gust_cbox->SetValue(m_parent.xyGribConfig.gust);
  m_xygribPanel->m_pressure_cbox->SetValue(m_parent.xyGribConfig.pressure);
  m_xygribPanel->m_temperature_cbox->SetValue(
      m_parent.xyGribConfig.temperature);
  m_xygribPanel->m_cape_cbox->SetValue(m_parent.xyGribConfig.cape);
  m_xygribPanel->m_reflectivity_cbox->SetValue(
      m_parent.xyGribConfig.reflectivity);
  m_xygribPanel->m_cloudcover_cbox->SetValue(m_parent.xyGribConfig.cloudCover);
  m_xygribPanel->m_precipitation_cbox->SetValue(
      m_parent.xyGribConfig.precipitation);
  m_xygribPanel->m_waveheight_cbox->SetValue(m_parent.xyGribConfig.waveHeight);
  m_xygribPanel->m_windwave_cbox->SetValue(m_parent.xyGribConfig.windWaves);
  m_xygribPanel->m_resolution_choice->SetSelection(
      m_parent.xyGribConfig.resolutionIndex);
  m_xygribPanel->m_duration_choice->SetSelection(
      m_parent.xyGribConfig.durationIndex);
  m_xygribPanel->m_interval_choice->SetSelection(
      m_parent.xyGribConfig.intervalIndex);
  m_xygribPanel->m_run_choice->SetSelection(m_parent.xyGribConfig.runIndex);

  UpdateGribSizeEstimate();
}

/// @brief Apply the configuration stored into the configuration structure to
/// the UI
void GribRequestSetting::MemorizeXyGribConfiguration() {
  m_parent.xyGribConfig.atmModelIndex =
      m_xygribPanel->m_atmmodel_choice->GetSelection();
  m_parent.xyGribConfig.waveModelIndex =
      m_xygribPanel->m_wavemodel_choice->GetSelection();

  m_parent.xyGribConfig.wind = m_xygribPanel->m_wind_cbox->IsChecked();
  m_parent.xyGribConfig.gust = m_xygribPanel->m_gust_cbox->IsChecked();
  m_parent.xyGribConfig.pressure = m_xygribPanel->m_pressure_cbox->IsChecked();
  m_parent.xyGribConfig.temperature =
      m_xygribPanel->m_temperature_cbox->IsChecked();
  m_parent.xyGribConfig.cape = m_xygribPanel->m_cape_cbox->IsChecked();
  m_parent.xyGribConfig.reflectivity =
      m_xygribPanel->m_reflectivity_cbox->IsChecked();
  m_parent.xyGribConfig.cloudCover =
      m_xygribPanel->m_cloudcover_cbox->IsChecked();
  m_parent.xyGribConfig.precipitation =
      m_xygribPanel->m_precipitation_cbox->IsChecked();
  m_parent.xyGribConfig.waveHeight =
      m_xygribPanel->m_waveheight_cbox->IsChecked();
  m_parent.xyGribConfig.windWaves = m_xygribPanel->m_windwave_cbox->IsChecked();

  m_parent.xyGribConfig.resolutionIndex =
      m_xygribPanel->m_resolution_choice->GetSelection();
  m_parent.xyGribConfig.durationIndex =
      m_xygribPanel->m_duration_choice->GetSelection();
  m_parent.xyGribConfig.intervalIndex =
      m_xygribPanel->m_interval_choice->GetSelection();
  m_parent.xyGribConfig.runIndex = m_xygribPanel->m_run_choice->GetSelection();

  UpdateGribSizeEstimate();
}

/// @brief Estimates the size of the GRIB file to be downloaded and prints it
/// onto GUI Code taken from XyGrib application
/// (https://github.com/opengribs/XyGrib)
void GribRequestSetting::UpdateGribSizeEstimate() {
  double resolution;
  long days;
  long interval;

  if (!m_xygribPanel->m_resolution_choice->GetStringSelection().ToCDouble(
          &resolution)) {
    m_xygribPanel->m_sizeestimate_text->SetLabel("Unknown");
    return;
  }
  if (!m_xygribPanel->m_duration_choice->GetStringSelection().ToCLong(&days)) {
    m_xygribPanel->m_sizeestimate_text->SetLabel("Unknown");
    return;
  }
  wxString intvStr = m_xygribPanel->m_interval_choice->GetStringSelection();
  intvStr.Replace("h", "");
  if (!intvStr.ToCLong(&interval)) {
    m_xygribPanel->m_sizeestimate_text->SetLabel("Unknown");
    return;
  }

  if (m_Vp == nullptr) {
    m_xygribPanel->m_sizeestimate_text->SetLabel("Unknown");
    return;
  }

  double xmax = m_Vp->lon_max;
  double xmin = m_Vp->lon_min;
  double ymax = m_Vp->lat_max;
  double ymin = m_Vp->lat_min;

  int npts = (int)(ceil(fabs(xmax - xmin) / resolution) *
                   ceil(fabs(ymax - ymin) / resolution));

  // Number of GribRecords
  int nbrec = (int)days * 24 / interval + 1;

  int nbPress = (m_xygribPanel->m_pressure_cbox->IsChecked() &&
                 m_xygribPanel->m_pressure_cbox->IsEnabled())
                    ? nbrec
                    : 0;
  int nbWind = (m_xygribPanel->m_wind_cbox->IsChecked() &&
                m_xygribPanel->m_wind_cbox->IsEnabled())
                   ? 2 * nbrec
                   : 0;
  int nbRain = (m_xygribPanel->m_precipitation_cbox->IsChecked() &&
                m_xygribPanel->m_precipitation_cbox->IsEnabled())
                   ? nbrec - 1
                   : 0;
  int nbCloud = (m_xygribPanel->m_cloudcover_cbox->IsChecked() &&
                 m_xygribPanel->m_cloudcover_cbox->IsEnabled())
                    ? nbrec - 1
                    : 0;
  int nbTemp = (m_xygribPanel->m_temperature_cbox->IsChecked() &&
                m_xygribPanel->m_temperature_cbox->IsEnabled())
                   ? nbrec
                   : 0;

  int nbCAPEsfc = (m_xygribPanel->m_cape_cbox->IsChecked() &&
                   m_xygribPanel->m_cape_cbox->IsEnabled())
                      ? nbrec
                      : 0;
  int nbReflectivity = (m_xygribPanel->m_reflectivity_cbox->IsChecked() &&
                        m_xygribPanel->m_reflectivity_cbox->IsEnabled())
                           ? nbrec
                           : 0;
  int nbGUSTsfc = (m_xygribPanel->m_gust_cbox->IsChecked() &&
                   m_xygribPanel->m_gust_cbox->IsEnabled())
                      ? nbrec
                      : 0;

  int head = 179;
  int estimate = 0;
  int nbits;  // this can vary when c3 packing is used. Average numbers are used
              // here

  nbits = 12;
  estimate += nbWind * (head + (nbits * npts) / 8 + 2);

  nbits = 9;
  estimate += nbTemp * (head + (nbits * npts) / 8 + 2);

  //    estime += nbTempMin*(head+(nbits*npts)/8+2 );
  //    estime += nbTempMax*(head+(nbits*npts)/8+2 );

  nbits = 9;
  estimate += nbRain * (head + 24 + (nbits * npts) / 8 + 2);

  nbits = 14;
  estimate += nbPress * (head + (nbits * npts) / 8 + 2);

  nbits = 7;
  estimate += nbCloud * (head + 24 + (nbits * npts) / 8 + 2);

  nbits = 11;
  estimate += nbReflectivity * (head + (nbits * npts) / 8 + 2);

  nbits = 12;
  estimate += nbCAPEsfc * (head + (nbits * npts) / 8 + 2);

  nbits = 9;
  estimate += nbGUSTsfc * (head + (nbits * npts) / 8 + 2);
  //    estime += nbSUNSDsfc*(head+(nbits*npts)/8+2 );

  // keep a record of the atmosphere estimate. if 0 nothing was selected
  int atmEstimate = estimate;

  // and now the wave estimate
  // recalculate number of points based on which model is used
  if (m_xygribPanel->m_wavemodel_choice->GetStringSelection().IsSameAs(
          "WW3"))  // 0.5 deg
  {
    npts = (int)(ceil(fabs(xmax - xmin) / 0.5) * ceil(fabs(ymax - ymin) / 0.5));
    nbrec = (int)fmin(8, days) * 24 / interval + 1;
  } else if (m_xygribPanel->m_wavemodel_choice->GetStringSelection().IsSameAs(
                 "GWAM"))  // 0.25 deg
  {
    npts =
        (int)(ceil(fabs(xmax - xmin) / 0.25) * ceil(fabs(ymax - ymin) / 0.25));
    nbrec = (int)fmin(8, days) * 24 / interval + 1;
  } else if (m_xygribPanel->m_wavemodel_choice->GetStringSelection().IsSameAs(
                 "EWAM"))  // 0.1 x 0.05 deg
  {
    npts =
        (int)(ceil(fabs(xmax - xmin) / 0.05) * ceil(fabs(ymax - ymin) / 0.1));
    nbrec = (int)fmin(4, days) * 24 / interval + 1;
  } else {
    npts = 0;
    nbrec = 0;
  }

  if (m_xygribPanel->m_waveheight_cbox->IsChecked()) {
    nbits = 9;
    estimate += nbrec * (head + (nbits * npts) / 8 + 2);
  }

  if (m_xygribPanel->m_windwave_cbox->IsChecked()) {
    nbits = 15;
    estimate += nbrec * (head + (nbits * npts) / 8 + 2);
    nbits = 8;
    estimate += nbrec * (head + (nbits * npts) / 8 + 2);
    nbits = 10;
    estimate += nbrec * (head + (nbits * npts) / 8 + 2);
  }

  int wavEstimate = estimate - atmEstimate;
  // reduce wave estimate due to land overlapping by average of 40%
  wavEstimate = wavEstimate * 0.6;
  // now adjusted estimate is
  estimate = atmEstimate + wavEstimate;

  // adjust for packing ~ 65%
  estimate = estimate * 0.65;

  wxString warningStr = "";
  if (estimate / (1024 * 1024) > XYGRIB_MAX_DOWNLOADABLE_GRIB_SIZE_MB) {
    warningStr = wxString::Format("(Warning GRIB exceeds %d MB limit)", XYGRIB_MAX_DOWNLOADABLE_GRIB_SIZE_MB);
  }

  m_xygribPanel->m_sizeestimate_text->SetLabel(
      wxString::Format("%d kB %s", estimate / 1024, warningStr));

  m_gribSizeEstimate = estimate;
}