/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2010, Anders Lund <anders@alweb.dk>
 */

#include "config.h"

#include "routemanagerdialog.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/progdlg.h>
#include <wx/clipbrd.h>
#include <wx/statline.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include "model/ais_decoder.h"
#include "model/config_vars.h"
#include "model/georef.h"
#include "model/mdns_cache.h"
#include "model/mdns_query.h"
#include "model/navutil_base.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/track.h"

#include "chartbase.h"
#include "chcanv.h"
#include "dychart.h"
#include "Layer.h"
#include "MarkInfo.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "route_validators.h"
#include "routeman_gui.h"
#include "route_point_gui.h"
#include "RoutePropDlgImpl.h"
#include "SendToGpsDlg.h"
#include "SendToPeerDlg.h"
#include "styles.h"
#include "svg_utils.h"
#include "TrackPropDlg.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#define DIALOG_MARGIN 10

enum { rmVISIBLE = 0, rmROUTENAME, rmROUTEDESC };  // RMColumns;
enum { colTRKVISIBLE = 0, colTRKNAME, colTRKLENGTH, colTRKDATE };
enum { colLAYVISIBLE = 0, colLAYNAME, colLAYITEMS, colLAYPERSIST };
enum { colWPTICON = 0, colWPTSCALE, colWPTNAME, colWPTDIST };

// GLOBALS :0
extern RouteList *pRouteList;
extern std::vector<Track *> g_TrackList;
extern LayerList *pLayerList;
extern wxString GetLayerName(int id);
extern RoutePropDlgImpl *pRoutePropDialog;
extern TrackPropDlg *pTrackPropDialog;
extern Routeman *g_pRouteMan;
extern MyConfig *pConfig;
extern ActiveTrack *g_pActiveTrack;
extern MarkInfoDlg *g_pMarkInfoDialog;
extern MyFrame *gFrame;
extern bool g_bShowLayers;
extern wxString g_default_wp_icon;
extern OCPNPlatform *g_Platform;

// Helper for conditional file name separator
void appendOSDirSlash(wxString *pString);

static int SortRouteTrack(const int order, const wxString &it1,
                          const wxString &it2) {
  if (order & 1) return it2.CmpNoCase(it1);

  return it1.CmpNoCase(it2);
}

// sort callback. Sort by route name.
static int sort_route_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortRoutesOnName(wxIntPtr item1, wxIntPtr item2,
                                       wxIntPtr list)
#else
int wxCALLBACK SortRoutesOnName(long item1, long item2, long list)
#endif
{
  return SortRouteTrack(sort_route_name_dir, ((Route *)item1)->GetName(),
                        ((Route *)item2)->GetName());
}

// sort callback. Sort by route Destination.
static int sort_route_to_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortRoutesOnTo(wxIntPtr item1, wxIntPtr item2,
                                     wxIntPtr list)
#else
int wxCALLBACK SortRoutesOnTo(long item1, long item2, long list)
#endif
{
  return SortRouteTrack(sort_route_to_dir, ((Route *)item1)->GetTo(),
                        ((Route *)item2)->GetTo());
}

// sort callback. Sort by track name.
static int sort_track_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortTracksOnName(wxIntPtr item1, wxIntPtr item2,
                                       wxIntPtr list)
#else
int wxCALLBACK SortTracksOnName(long item1, long item2, long list)
#endif
{
  return SortRouteTrack(sort_track_name_dir, ((Track *)item1)->GetName(),
                        ((Track *)item2)->GetName());
}

static int SortDouble(const int order, const double &it1, const double &it2) {
  double l1;
  double l2;

  if (order & 1) {
    l1 = it2;
    l2 = it1;
  } else {
    l1 = it1;
    l2 = it2;
  }

  if (l1 == l2) return 0;
  if (l2 < l1) return 1;
  return -1;
}

// sort callback. Sort by track length.
static int sort_track_len_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortTracksOnDistance(wxIntPtr item1, wxIntPtr item2,
                                           wxIntPtr list)
#else
int wxCALLBACK SortTracksOnDistance(long item1, long item2, long list)
#endif
{
  return SortDouble(sort_track_len_dir, ((Track *)item1)->Length(),
                    ((Track *)item2)->Length());
}

// sort callback. Sort by track start date.
static int sort_track_date_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortTracksOnDate(wxIntPtr item1, wxIntPtr item2,
                                       wxIntPtr list)
#else
int wxCALLBACK SortTracksOnDate(long item1, long item2, long list)
#endif
{
  return SortRouteTrack(sort_track_date_dir, ((Track *)item1)->GetDate(),
                        ((Track *)item2)->GetDate());
}

static int sort_wp_key;
static int sort_track_key;

// sort callback. Sort by wpt name.
static int sort_wp_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortWaypointsOnName(wxIntPtr item1, wxIntPtr item2,
                                          wxIntPtr list)
#else
int wxCALLBACK SortWaypointsOnName(long item1, long item2, long list)
#endif

{
  RoutePoint *pRP1 = (RoutePoint *)item1;
  RoutePoint *pRP2 = (RoutePoint *)item2;

  if (pRP1 && pRP2) {
    if (sort_wp_name_dir & 1)
      return pRP2->GetName().CmpNoCase(pRP1->GetName());
    else
      return pRP1->GetName().CmpNoCase(pRP2->GetName());
  } else
    return 0;
}

// sort callback. Sort by wpt distance.
static int sort_wp_len_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortWaypointsOnDistance(wxIntPtr item1, wxIntPtr item2,
                                              wxIntPtr list)
#else
int wxCALLBACK SortWaypointsOnDistance(long item1, long item2, long list)
#endif
{
  double dst1, dst2;
  DistanceBearingMercator(((RoutePoint *)item1)->m_lat,
                          ((RoutePoint *)item1)->m_lon, gLat, gLon, NULL,
                          &dst1);
  DistanceBearingMercator(((RoutePoint *)item2)->m_lat,
                          ((RoutePoint *)item2)->m_lon, gLat, gLon, NULL,
                          &dst2);
  return SortDouble(sort_wp_len_dir, dst1, dst2);
}

// sort callback. Sort by layer name.
static int sort_layer_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortLayersOnName(wxIntPtr item1, wxIntPtr item2,
                                       wxIntPtr list)
#else
int wxCALLBACK SortLayersOnName(long item1, long item2, long list)
#endif
{
  return SortRouteTrack(sort_layer_name_dir, ((Layer *)item1)->m_LayerName,
                        ((Layer *)item2)->m_LayerName);
}

// sort callback. Sort by layer size.
static int sort_layer_len_dir;
#if wxCHECK_VERSION(2, 9, 0)
static int wxCALLBACK SortLayersOnSize(wxIntPtr item1, wxIntPtr item2,
                                       wxIntPtr list)
#else
int wxCALLBACK SortLayersOnSize(long item1, long item2, long list)
#endif
{
  return SortDouble(sort_layer_len_dir, ((Layer *)item1)->m_NoOfItems,
                    ((Layer *)item2)->m_NoOfItems);
}

// event table. Mostly empty, because I find it much easier to see what is
// connected to what using Connect() where possible, so that it is visible in
// the code.
BEGIN_EVENT_TABLE(RouteManagerDialog, wxFrame)
EVT_NOTEBOOK_PAGE_CHANGED(
    wxID_ANY,
    RouteManagerDialog::OnTabSwitch)  // This should work under Windows :-(
EVT_CLOSE(RouteManagerDialog::OnClose)
EVT_COMMAND(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, RouteManagerDialog::OnOK)
EVT_CHAR_HOOK(RouteManagerDialog::OnKey)
END_EVENT_TABLE()

void RouteManagerDialog::OnKey(wxKeyEvent &ke) {
  if (ke.GetKeyCode() == WXK_ESCAPE)
    Close(true);
  else
    ke.Skip();
}

void RouteManagerDialog::OnTabSwitch(wxNotebookEvent &event) {
  if (!m_pNotebook) return;
  int current_page = m_pNotebook->GetSelection();
  if (current_page == 3) {
    if (btnImport) btnImport->Enable(false);
    if (btnExport) btnExport->Enable(false);
    if (btnExportViz) btnExportViz->Enable(false);
  } else {
    if (btnImport) btnImport->Enable(true);
    if (btnExport) btnExport->Enable(true);
    if (btnExportViz) btnExportViz->Enable(true);
  }
  event.Skip();  // remove if using event table... why?
}

// implementation

bool RouteManagerDialog::instanceFlag = false;
RouteManagerDialog *RouteManagerDialog::single = NULL;

RouteManagerDialog *RouteManagerDialog::getInstance(wxWindow *parent) {
  if (!instanceFlag) {
    single = new RouteManagerDialog(parent);
    instanceFlag = true;
    return single;
  } else {
    return single;
  }
}

RouteManagerDialog::RouteManagerDialog(wxWindow *parent) {
  long style =
      wxDEFAULT_FRAME_STYLE | wxRESIZE_BORDER | wxFRAME_FLOAT_ON_PARENT;

  wxFrame::Create(parent, -1, wxString(_("Route & Mark Manager")),
                  wxDefaultPosition, wxDefaultSize, style);

  wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  m_lastWptItem = -1;
  m_lastTrkItem = -1;
  m_lastRteItem = -1;
  sort_wp_key = SORT_ON_NAME;
  sort_track_key = SORT_ON_NAME;

  btnImport = NULL;
  btnExport = NULL;
  btnExportViz = NULL;

  Create();
  routes_update_listener.Init(g_pRouteMan->on_routes_update,
                              [&](wxCommandEvent) { UpdateRouteListCtrl(); });
}

void RouteManagerDialog::Create() {
  //  Get a text height metric for reference
  int char_width, char_height;
  GetTextExtent(_T("W"), &char_width, &char_height);
  m_charWidth = char_width;

  wxBoxSizer *itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer1);

  m_pNotebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition,
                               wxSize(-1, -1), wxNB_TOP);
  itemBoxSizer1->Add(m_pNotebook, 1, wxALL | wxEXPAND, 5);

  //  Create "Routes" panel
  m_pPanelRte = new wxPanel(m_pNotebook, wxID_ANY, wxDefaultPosition,
                            wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

  wxBoxSizer *sbsRoutes = new wxBoxSizer(wxHORIZONTAL);
  m_pPanelRte->SetSizer(sbsRoutes);
  m_pNotebook->AddPage(m_pPanelRte, _("Routes"));

  sort_wp_len_dir = 1;
  sort_wp_name_dir = 0;
  sort_track_len_dir = 1;
  sort_route_to_dir = 0;
  sort_track_name_dir = 0;
  sort_route_name_dir = 0;
  sort_layer_name_dir = 0;
  sort_layer_len_dir = 1;

  m_listIconSize = 2 * GetCharHeight();

  // Setup GUI
  wxBoxSizer *bSizerRteContents;
  bSizerRteContents = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer *fgSizerFilterRte;
  fgSizerFilterRte = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizerFilterRte->AddGrowableCol(1);
  fgSizerFilterRte->SetFlexibleDirection(wxBOTH);
  fgSizerFilterRte->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stFilterRte = new wxStaticText(m_pPanelRte, wxID_ANY, _("Filter"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stFilterRte->Wrap(-1);
  fgSizerFilterRte->Add(m_stFilterRte, 0, wxALL, 5);

  m_tFilterRte = new wxTextCtrl(m_pPanelRte, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize, 0);
  fgSizerFilterRte->Add(m_tFilterRte, 1, wxALL | wxEXPAND, 5);

  bSizerRteContents->Add(fgSizerFilterRte, 0, wxEXPAND, 5);
  m_tFilterRte->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(RouteManagerDialog::OnFilterChanged), NULL, this);

  m_cbShowAllRte = new wxCheckBox(m_pPanelRte, wxID_ANY, _("Show all routes"));
  bSizerRteContents->Add(m_cbShowAllRte, 0, wxEXPAND | wxLEFT, 5);
  m_cbShowAllRte->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnShowAllRteCBClicked), NULL,
      this);

  m_pRouteListCtrl =
      new wxListCtrl(m_pPanelRte, -1, wxDefaultPosition, wxSize(-1, -1),
                     wxLC_REPORT | wxLC_SORT_ASCENDING | wxLC_HRULES |
                         wxBORDER_SUNKEN /*|wxLC_VRULES*/);
#ifdef __ANDROID__
  m_pRouteListCtrl->GetHandle()->setStyleSheet(getAdjustedDialogStyleSheet());
#endif

  m_pRouteListCtrl->Connect(
      wxEVT_COMMAND_LIST_ITEM_SELECTED,
      wxListEventHandler(RouteManagerDialog::OnRteSelected), NULL, this);
  m_pRouteListCtrl->Connect(
      wxEVT_COMMAND_LIST_ITEM_DESELECTED,
      wxListEventHandler(RouteManagerDialog::OnRteSelected), NULL, this);
  m_pRouteListCtrl->Connect(
      wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
      wxListEventHandler(RouteManagerDialog::OnRteDefaultAction), NULL, this);
  m_pRouteListCtrl->Connect(
      wxEVT_LEFT_DOWN,
      wxMouseEventHandler(RouteManagerDialog::OnRteToggleVisibility), NULL,
      this);
  m_pRouteListCtrl->Connect(
      wxEVT_COMMAND_LIST_COL_CLICK,
      wxListEventHandler(RouteManagerDialog::OnRteColumnClicked), NULL, this);
  bSizerRteContents->Add(m_pRouteListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN);
  sbsRoutes->Add(bSizerRteContents, 1, wxEXPAND, 5);

  // Columns: visibility ctrl, name
  // note that under MSW for SetColumnWidth() to work we need to create the
  // items with images initially even if we specify dummy image id
  m_pRouteListCtrl->InsertColumn(rmVISIBLE, _("Show"), wxLIST_FORMAT_LEFT,
                                 10 /*4 * char_width*/);
  m_pRouteListCtrl->InsertColumn(rmROUTENAME, _("Route Name"),
                                 wxLIST_FORMAT_LEFT, 15 * char_width);
  m_pRouteListCtrl->InsertColumn(rmROUTEDESC, _("From <-> To"),
                                 wxLIST_FORMAT_LEFT, 10 * char_width);

  // Buttons: Delete, Properties...
  wxBoxSizer *bsRouteButtons = new wxBoxSizer(wxVERTICAL);
  sbsRoutes->Add(bsRouteButtons, 0, wxEXPAND);

  wxScrolledWindow *winr = new wxScrolledWindow(
      m_pPanelRte, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxNO_BORDER | wxTAB_TRAVERSAL | wxVSCROLL);
  winr->SetScrollRate(0, 5);

  bsRouteButtons->Add(winr, 1, wxALL | wxEXPAND, DIALOG_MARGIN);

  wxBoxSizer *bsRouteButtonsInner = new wxBoxSizer(wxVERTICAL);
  winr->SetSizer(bsRouteButtonsInner);

  btnRteProperties = new wxButton(winr, -1, _("&Properties") + _T("..."));
  bsRouteButtonsInner->Add(btnRteProperties, 0, wxALL | wxEXPAND,
                           DIALOG_MARGIN);
  btnRteProperties->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRtePropertiesClick), NULL,
      this);

  btnRteActivate = new wxButton(winr, -1, _("&Activate"));
  bsRouteButtonsInner->Add(btnRteActivate, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnRteActivate->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteActivateClick), NULL,
      this);
  btnRteActivate->Connect(
      wxEVT_LEFT_DOWN,
      wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this);

  btnRteZoomto = new wxButton(winr, -1, _("&Center View"));
  bsRouteButtonsInner->Add(btnRteZoomto, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnRteZoomto->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteZoomtoClick), NULL, this);
  btnRteZoomto->Connect(
      wxEVT_LEFT_DOWN,
      wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this);

  btnRteReverse = new wxButton(winr, -1, _("&Reverse"));
  bsRouteButtonsInner->Add(btnRteReverse, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnRteReverse->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteReverseClick), NULL, this);

  btnRteDelete = new wxButton(winr, -1, _("&Delete"));
  bsRouteButtonsInner->Add(btnRteDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnRteDelete->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteDeleteClick), NULL, this);

  wxString reseq_label(_("&Resequence Waypoints"));
  wxString export_label(_("&Export selected..."));
  wxString send_to_gps_label(_("&Send to GPS..."));
  wxString send_to_peer_label(_("Send to &Peer..."));

#ifdef __ANDROID__
  reseq_label = wxString(_("Resequence"));
  export_label = wxString(_("Export"));
  send_to_gps_label = wxString(_("Send to GPS"));
  send_to_peer_label = wxString(_("Send to Peer"));
#endif

  btnRteExport = new wxButton(winr, -1, export_label);
  bsRouteButtonsInner->Add(btnRteExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnRteExport->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteExportClick), NULL, this);

  btnRteResequence = new wxButton(winr, -1, reseq_label);
  bsRouteButtonsInner->Add(btnRteResequence, 0, wxALL | wxEXPAND,
                           DIALOG_MARGIN);
  btnRteResequence->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteResequenceClick), NULL,
      this);

  btnRteSendToPeer = new wxButton(winr, -1, send_to_peer_label);
  bsRouteButtonsInner->Add(btnRteSendToPeer, 0, wxALL | wxEXPAND,
                           DIALOG_MARGIN);
  btnRteSendToPeer->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteSendToPeerClick), NULL,
      this);

  btnRteSendToGPS = new wxButton(winr, -1, send_to_gps_label);
  bsRouteButtonsInner->Add(btnRteSendToGPS, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnRteSendToGPS->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteSendToGPSClick), NULL,
      this);

  bsRouteButtonsInner->AddSpacer(10);

  btnRteDeleteAll = new wxButton(winr, -1, _("&Delete All"));
  bsRouteButtonsInner->Add(btnRteDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnRteDeleteAll->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnRteDeleteAllClick), NULL,
      this);

  //  Create "Tracks" panel
  m_pPanelTrk = new wxPanel(m_pNotebook, wxID_ANY, wxDefaultPosition,
                            wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
  wxBoxSizer *sbsTracks = new wxBoxSizer(wxHORIZONTAL);
  m_pPanelTrk->SetSizer(sbsTracks);
  m_pNotebook->AddPage(m_pPanelTrk, _("Tracks"));

  wxBoxSizer *bSizerTrkContents;
  bSizerTrkContents = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer *fgSizerFilterTrk;
  fgSizerFilterTrk = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizerFilterTrk->AddGrowableCol(1);
  fgSizerFilterTrk->SetFlexibleDirection(wxBOTH);
  fgSizerFilterTrk->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stFilterTrk = new wxStaticText(m_pPanelTrk, wxID_ANY, _("Filter"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stFilterTrk->Wrap(-1);
  fgSizerFilterTrk->Add(m_stFilterTrk, 0, wxALL, 5);

  m_tFilterTrk = new wxTextCtrl(m_pPanelTrk, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize, 0);
  fgSizerFilterTrk->Add(m_tFilterTrk, 1, wxALL | wxEXPAND, 5);

  bSizerTrkContents->Add(fgSizerFilterTrk, 0, wxEXPAND, 5);
  m_tFilterTrk->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(RouteManagerDialog::OnFilterChanged), NULL, this);

  m_cbShowAllTrk = new wxCheckBox(m_pPanelTrk, wxID_ANY, _("Show all tracks"));
  bSizerTrkContents->Add(m_cbShowAllTrk, 0, wxEXPAND | wxLEFT, 5);
  m_cbShowAllTrk->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnShowAllTrkCBClicked), NULL,
      this);

  m_pTrkListCtrl =
      new wxListCtrl(m_pPanelTrk, -1, wxDefaultPosition, wxDefaultSize,
                     wxLC_REPORT | wxLC_SORT_ASCENDING | wxLC_HRULES |
                         wxBORDER_SUNKEN /*|wxLC_VRULES*/);

#ifdef __ANDROID__
  m_pTrkListCtrl->GetHandle()->setStyleSheet(getAdjustedDialogStyleSheet());
#endif

  m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
                          wxListEventHandler(RouteManagerDialog::OnTrkSelected),
                          NULL, this);
  m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
                          wxListEventHandler(RouteManagerDialog::OnTrkSelected),
                          NULL, this);
  m_pTrkListCtrl->Connect(
      wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
      wxListEventHandler(RouteManagerDialog::OnTrkDefaultAction), NULL, this);
  m_pTrkListCtrl->Connect(
      wxEVT_LEFT_DOWN,
      wxMouseEventHandler(RouteManagerDialog::OnTrkToggleVisibility), NULL,
      this);
  m_pTrkListCtrl->Connect(
      wxEVT_COMMAND_LIST_COL_CLICK,
      wxListEventHandler(RouteManagerDialog::OnTrkColumnClicked), NULL, this);
  m_pTrkListCtrl->Connect(
      wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
      wxListEventHandler(RouteManagerDialog::OnTrkRightClick), NULL, this);
  this->Connect(wxEVT_COMMAND_MENU_SELECTED,
                wxCommandEventHandler(RouteManagerDialog::OnTrkMenuSelected),
                NULL, this);

  bSizerTrkContents->Add(m_pTrkListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN);
  sbsTracks->Add(bSizerTrkContents, 1, wxEXPAND, 5);

  m_pTrkListCtrl->InsertColumn(colTRKVISIBLE, _("Show"), wxLIST_FORMAT_LEFT,
                               4 * char_width);
  m_pTrkListCtrl->InsertColumn(colTRKNAME, _("Track Name"), wxLIST_FORMAT_LEFT,
                               20 * char_width);
  m_pTrkListCtrl->InsertColumn(colTRKDATE, _("Start Date"), wxLIST_FORMAT_LEFT,
                               20 * char_width);
  m_pTrkListCtrl->InsertColumn(colTRKLENGTH, _("Length"), wxLIST_FORMAT_LEFT,
                               5 * char_width);

  wxBoxSizer *bsTrkButtons = new wxBoxSizer(wxVERTICAL);
  sbsTracks->Add(bsTrkButtons, 0, wxEXPAND);

  wxScrolledWindow *wint = new wxScrolledWindow(
      m_pPanelTrk, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxNO_BORDER | wxTAB_TRAVERSAL | wxVSCROLL);
  wint->SetScrollRate(0, 5);

  bsTrkButtons->Add(wint, 1, wxALL | wxEXPAND, DIALOG_MARGIN);

  wxBoxSizer *bsTrkButtonsInner = new wxBoxSizer(wxVERTICAL);
  wint->SetSizer(bsTrkButtonsInner);

  btnTrkNew = new wxButton(wint, -1, _("&Start Track"));
  bsTrkButtonsInner->Add(btnTrkNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnTrkNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(RouteManagerDialog::OnTrkNewClick),
                     NULL, this);

  btnTrkProperties = new wxButton(wint, -1, _("&Properties"));
  bsTrkButtonsInner->Add(btnTrkProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnTrkProperties->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnTrkPropertiesClick), NULL,
      this);

  btnTrkDelete = new wxButton(wint, -1, _("&Delete"));
  bsTrkButtonsInner->Add(btnTrkDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnTrkDelete->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteClick), NULL, this);

  btnTrkExport = new wxButton(wint, -1, _("&Export selected..."));
  bsTrkButtonsInner->Add(btnTrkExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnTrkExport->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnTrkExportClick), NULL, this);

  btnTrkRouteFromTrack = new wxButton(wint, -1, _("Route from Track"));
  bsTrkButtonsInner->Add(btnTrkRouteFromTrack, 0, wxALL | wxEXPAND,
                         DIALOG_MARGIN);
  btnTrkRouteFromTrack->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnTrkRouteFromTrackClick), NULL,
      this);

  btnTrkSendToPeer = new wxButton(wint, -1, _("Send to &Peer"));
  bsTrkButtonsInner->Add(btnTrkSendToPeer, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnTrkSendToPeer->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnTrkSendToPeerClick), NULL,
      this);

  bsTrkButtonsInner->AddSpacer(10);

  btnTrkDeleteAll = new wxButton(wint, -1, _("&Delete All"));
  bsTrkButtonsInner->Add(btnTrkDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnTrkDeleteAll->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteAllClick), NULL,
      this);

  //  Create "Marks" panel
  m_pPanelWpt = new wxPanel(m_pNotebook, wxID_ANY, wxDefaultPosition,
                            wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
  wxBoxSizer *sbsWpts = new wxBoxSizer(wxHORIZONTAL);
  m_pPanelWpt->SetSizer(sbsWpts);
  m_pNotebook->AddPage(m_pPanelWpt, _("Marks"));

  wxBoxSizer *bSizerWptContents;
  bSizerWptContents = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer *fgSizerFilterWpt;
  fgSizerFilterWpt = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizerFilterWpt->AddGrowableCol(1);
  fgSizerFilterWpt->SetFlexibleDirection(wxBOTH);
  fgSizerFilterWpt->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stFilterWpt = new wxStaticText(m_pPanelWpt, wxID_ANY, _("Filter"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stFilterWpt->Wrap(-1);
  fgSizerFilterWpt->Add(m_stFilterWpt, 0, wxALL, 5);

  m_tFilterWpt = new wxTextCtrl(m_pPanelWpt, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize, 0);
  fgSizerFilterWpt->Add(m_tFilterWpt, 1, wxALL | wxEXPAND, 5);

  bSizerWptContents->Add(fgSizerFilterWpt, 0, wxEXPAND, 5);
  m_tFilterWpt->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(RouteManagerDialog::OnFilterChanged), NULL, this);

  m_cbShowAllWP = new wxCheckBox(m_pPanelWpt, wxID_ANY, _("Show all marks"));
  bSizerWptContents->Add(m_cbShowAllWP, 0, wxEXPAND | wxLEFT, 5);
  m_cbShowAllWP->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnShowAllWpCBClicked), NULL,
      this);

  m_pWptListCtrl =
      new wxListCtrl(m_pPanelWpt, -1, wxDefaultPosition, wxDefaultSize,
                     wxLC_REPORT | wxLC_SORT_ASCENDING | wxLC_HRULES |
                         wxBORDER_SUNKEN /*|wxLC_VRULES*/);
#ifdef __ANDROID__
  m_pWptListCtrl->GetHandle()->setStyleSheet(getAdjustedDialogStyleSheet());
#endif

  m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
                          wxListEventHandler(RouteManagerDialog::OnWptSelected),
                          NULL, this);
  m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
                          wxListEventHandler(RouteManagerDialog::OnWptSelected),
                          NULL, this);
  m_pWptListCtrl->Connect(
      wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
      wxListEventHandler(RouteManagerDialog::OnWptDefaultAction), NULL, this);
  m_pWptListCtrl->Connect(
      wxEVT_LEFT_DOWN,
      wxMouseEventHandler(RouteManagerDialog::OnWptToggleVisibility), NULL,
      this);
  m_pWptListCtrl->Connect(
      wxEVT_COMMAND_LIST_COL_CLICK,
      wxListEventHandler(RouteManagerDialog::OnWptColumnClicked), NULL, this);
  bSizerWptContents->Add(m_pWptListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN);
  sbsWpts->Add(bSizerWptContents, 1, wxEXPAND, 5);

  m_pWptListCtrl->InsertColumn(colWPTICON, _("Icon"), wxLIST_FORMAT_LEFT,
                               4 * char_width);
  m_pWptListCtrl->InsertColumn(colWPTSCALE, _("Scale"), wxLIST_FORMAT_LEFT,
                               8 * char_width);
  m_pWptListCtrl->InsertColumn(colWPTNAME, _("Mark Name"), wxLIST_FORMAT_LEFT,
                               15 * char_width);
  m_pWptListCtrl->InsertColumn(colWPTDIST, _("Distance from own ship"),
                               wxLIST_FORMAT_LEFT, 14 * char_width);

  wxBoxSizer *bsWptButtons = new wxBoxSizer(wxVERTICAL);
  sbsWpts->Add(bsWptButtons, 0, wxEXPAND);

  wxScrolledWindow *winw = new wxScrolledWindow(
      m_pPanelWpt, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxNO_BORDER | wxTAB_TRAVERSAL | wxVSCROLL);
  winw->SetScrollRate(0, 5);

  bsWptButtons->Add(winw, 1, wxALL | wxEXPAND, DIALOG_MARGIN);

  wxBoxSizer *bsWptButtonsInner = new wxBoxSizer(wxVERTICAL);
  winw->SetSizer(bsWptButtonsInner);

  btnWptNew = new wxButton(winw, -1, _("&New"));
  bsWptButtonsInner->Add(btnWptNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(RouteManagerDialog::OnWptNewClick),
                     NULL, this);

  btnWptProperties = new wxButton(winw, -1, _("&Properties"));
  bsWptButtonsInner->Add(btnWptProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptProperties->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnWptPropertiesClick), NULL,
      this);

  btnWptZoomto = new wxButton(winw, -1, _("&Center View"));
  bsWptButtonsInner->Add(btnWptZoomto, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptZoomto->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnWptZoomtoClick), NULL, this);

  btnWptDelete = new wxButton(winw, -1, _("&Delete"));
  bsWptButtonsInner->Add(btnWptDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptDelete->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnWptDeleteClick), NULL, this);

  btnWptGoTo = new wxButton(winw, -1, _("&Go To"));
  bsWptButtonsInner->Add(btnWptGoTo, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptGoTo->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                      wxCommandEventHandler(RouteManagerDialog::OnWptGoToClick),
                      NULL, this);

  btnWptExport = new wxButton(winw, -1, _("&Export selected..."));
  bsWptButtonsInner->Add(btnWptExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptExport->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnWptExportClick), NULL, this);

  btnWptSendToGPS = new wxButton(winw, -1, _("&Send to GPS"));
  bsWptButtonsInner->Add(btnWptSendToGPS, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptSendToGPS->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnWptSendToGPSClick), NULL,
      this);

  btnWptSendToPeer = new wxButton(winw, -1, _("Send to &Peer"));
  bsWptButtonsInner->Add(btnWptSendToPeer, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptSendToPeer->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnWptSendToPeerClick), NULL,
      this);

  bsWptButtonsInner->AddSpacer(10);

  btnWptDeleteAll = new wxButton(winw, -1, _("Delete All"));
  bsWptButtonsInner->Add(btnWptDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnWptDeleteAll->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnWptDeleteAllClick), NULL,
      this);

  wxBoxSizer *itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer1->Add(itemBoxSizer5, 0, wxALL | wxEXPAND);

  wxBoxSizer *itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer5->Add(itemBoxSizer6, 1, wxALL | wxEXPAND);

  btnImport = new wxButton(this, -1, _("I&mport GPX..."));
  itemBoxSizer6->Add(btnImport, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN);
  btnImport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(RouteManagerDialog::OnImportClick),
                     NULL, this);
  /*
   *    btnExport = new wxButton( this, -1, _("E&xport All...") );
   *    itemBoxSizer6->Add( btnExport, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN );
   *    btnExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
   *            wxCommandEventHandler(RouteManagerDialog::OnExportClick), NULL,
   * this );
   */
  btnExportViz = new wxButton(this, -1, _("Export All Visible..."));
  itemBoxSizer6->Add(btnExportViz, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN);
  btnExportViz->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnExportVizClick), NULL, this);

  // Dialog OK button
  wxSize sz = ::wxGetDisplaySize();
  if (sz.y < sz.x) {                           // landscape
    itemBoxSizer6->Add(0, 0, 1, wxEXPAND, 5);  // Spacer
    itemBoxSizer6->Add(new wxButton(this, wxID_OK), 0, wxALL, DIALOG_MARGIN);
  } else {
    wxStaticLine *staticLine121 = new wxStaticLine(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    itemBoxSizer1->Add(staticLine121, 0, wxALL | wxEXPAND, DIALOG_MARGIN);

    wxBoxSizer *itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer7, 0, wxEXPAND);

    wxBoxSizer *itemBoxSizer7a = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer7a, 1, wxEXPAND);

    itemBoxSizer7a->AddStretchSpacer();
    itemBoxSizer7a->Add(new wxButton(this, wxID_OK), 0, wxRIGHT | wxALIGN_RIGHT,
                        DIALOG_MARGIN * 4);
  }

  //  Create "Layers" panel
  m_pPanelLay = new wxPanel(m_pNotebook, wxID_ANY, wxDefaultPosition,
                            wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
  wxBoxSizer *sbsLayers = new wxBoxSizer(wxHORIZONTAL);
  m_pPanelLay->SetSizer(sbsLayers);
  m_pNotebook->AddPage(m_pPanelLay, _("Layers"));

  wxBoxSizer *bSizerLayContents;
  bSizerLayContents = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer *fgSizerFilterLay;
  fgSizerFilterLay = new wxFlexGridSizer(0, 2, 0, 0);
  fgSizerFilterLay->AddGrowableCol(1);
  fgSizerFilterLay->SetFlexibleDirection(wxBOTH);
  fgSizerFilterLay->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_stFilterLay = new wxStaticText(m_pPanelLay, wxID_ANY, _("Filter"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stFilterLay->Wrap(-1);
  fgSizerFilterLay->Add(m_stFilterLay, 0, wxALL, 5);

  m_tFilterLay = new wxTextCtrl(m_pPanelLay, wxID_ANY, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize, 0);
  fgSizerFilterLay->Add(m_tFilterLay, 1, wxALL | wxEXPAND, 5);

  bSizerLayContents->Add(fgSizerFilterLay, 0, wxEXPAND, 5);
  m_tFilterLay->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(RouteManagerDialog::OnFilterChanged), NULL, this);

  m_cbShowAllLay = new wxCheckBox(m_pPanelLay, wxID_ANY, _("Show all layers"));
  bSizerLayContents->Add(m_cbShowAllLay, 0, wxEXPAND | wxLEFT, 5);
  m_cbShowAllLay->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnShowAllLayCBClicked), NULL,
      this);

  m_pLayListCtrl =
      new wxListCtrl(m_pPanelLay, -1, wxDefaultPosition, wxDefaultSize,
                     wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING |
                         wxLC_HRULES | wxBORDER_SUNKEN /*|wxLC_VRULES*/);
#ifdef __ANDROID__
  m_pLayListCtrl->GetHandle()->setStyleSheet(getAdjustedDialogStyleSheet());
#endif

  m_pLayListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
                          wxListEventHandler(RouteManagerDialog::OnLaySelected),
                          NULL, this);
  m_pLayListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
                          wxListEventHandler(RouteManagerDialog::OnLaySelected),
                          NULL, this);
  m_pLayListCtrl->Connect(
      wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
      wxListEventHandler(RouteManagerDialog::OnLayDefaultAction), NULL, this);
  m_pLayListCtrl->Connect(
      wxEVT_LEFT_DOWN,
      wxMouseEventHandler(RouteManagerDialog::OnLayToggleVisibility), NULL,
      this);
  m_pLayListCtrl->Connect(
      wxEVT_COMMAND_LIST_COL_CLICK,
      wxListEventHandler(RouteManagerDialog::OnLayColumnClicked), NULL, this);
  bSizerLayContents->Add(m_pLayListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN);
  sbsLayers->Add(bSizerLayContents, 1, wxEXPAND, 5);

  m_pLayListCtrl->InsertColumn(colLAYVISIBLE, _T(""), wxLIST_FORMAT_LEFT,
                               4 * char_width);
  m_pLayListCtrl->InsertColumn(colLAYNAME, _("Layer Name"), wxLIST_FORMAT_LEFT,
                               14 * char_width);
  m_pLayListCtrl->InsertColumn(colLAYITEMS, _("No. of items"),
                               wxLIST_FORMAT_LEFT, 10 * char_width);
  m_pLayListCtrl->InsertColumn(colLAYPERSIST, _("Layer type"),
                               wxLIST_FORMAT_LEFT, 10 * char_width);

  wxBoxSizer *bsLayButtons = new wxBoxSizer(wxVERTICAL);
  sbsLayers->Add(bsLayButtons, 0, wxEXPAND);

  wxScrolledWindow *winl = new wxScrolledWindow(
      m_pPanelLay, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxNO_BORDER | wxTAB_TRAVERSAL | wxVSCROLL);
  winl->SetScrollRate(0, 5);

  bsLayButtons->Add(winl, 1, wxALL | wxEXPAND, DIALOG_MARGIN);

  wxBoxSizer *bsLayButtonsInner = new wxBoxSizer(wxVERTICAL);
  winl->SetSizer(bsLayButtonsInner);

  btnLayNew = new wxButton(winl, -1, _("Create Temporary layer"));
  bsLayButtonsInner->Add(btnLayNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnLayNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                     wxCommandEventHandler(RouteManagerDialog::OnLayNewClick),
                     NULL, this);

  btnPerLayNew = new wxButton(winl, -1, _("Create Persistent layer"));
  bsLayButtonsInner->Add(btnPerLayNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnPerLayNew->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnPerLayNewClick), NULL, this);

  btnLayDelete = new wxButton(winl, -1, _("&Delete"));
  bsLayButtonsInner->Add(btnLayDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  btnLayDelete->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnLayDeleteClick), NULL, this);

  cbLayToggleChart = new wxCheckBox(winl, -1, _("Show on chart"));
  bsLayButtonsInner->Add(cbLayToggleChart, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  cbLayToggleChart->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnLayToggleChartClick), NULL,
      this);

  cbLayToggleNames =
      new wxCheckBox(winl, -1, _("Show WPT names"), wxDefaultPosition,
                     wxDefaultSize, wxCHK_3STATE);

  bsLayButtonsInner->Add(cbLayToggleNames, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
  cbLayToggleNames->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnLayToggleNamesClick), NULL,
      this);

  cbLayToggleListing = new wxCheckBox(winl, -1, _("List contents"));
  bsLayButtonsInner->Add(cbLayToggleListing, 0, wxALL | wxEXPAND,
                         DIALOG_MARGIN);
  cbLayToggleListing->Connect(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      wxCommandEventHandler(RouteManagerDialog::OnLayToggleListingClick), NULL,
      this);

  RecalculateSize();

  int imageRefSize = m_charWidth * 2;  // g_Platform->GetDisplayDPmm() * 4;
  wxImageList *imglist = new wxImageList(imageRefSize, imageRefSize, true, 1);

  // Load eye icons
  wxString UserIconPath = g_Platform->GetSharedDataDir() + _T("uidata") +
                          wxFileName::GetPathSeparator();
  wxImage iconSVG =
      LoadSVG(UserIconPath + _T("eye.svg"), imageRefSize, imageRefSize)
          .ConvertToImage();
  if (iconSVG.IsOk()) {
    iconSVG.Resize(wxSize(imageRefSize, imageRefSize),
                   wxPoint(0, 0));  // Avoid wxImageList size asserts
    imglist->Add(wxBitmap(iconSVG));
  }

  iconSVG = LoadSVG(UserIconPath + _T("eyex.svg"), imageRefSize, imageRefSize)
                .ConvertToImage();
  if (iconSVG.IsOk()) {
    iconSVG.Resize(wxSize(imageRefSize, imageRefSize), wxPoint(0, 0));
    imglist->Add(wxBitmap(iconSVG));
  }

  iconSVG =
      LoadSVG(UserIconPath + _T("eyeGray.svg"), imageRefSize, imageRefSize)
          .ConvertToImage();
  if (iconSVG.IsOk()) {
    iconSVG.Resize(wxSize(imageRefSize, imageRefSize), wxPoint(0, 0));
    imglist->Add(wxBitmap(iconSVG));
  }

  m_pRouteListCtrl->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

#ifdef __ANDROID__
  m_pRouteListCtrl->GetHandle()->setIconSize(QSize(imageRefSize, imageRefSize));
#endif

  // Assign will handle destroy, Set will not. It's OK, that's what we want
  m_pTrkListCtrl->SetImageList(imglist, wxIMAGE_LIST_SMALL);
  m_pWptListCtrl->SetImageList(
      pWayPointMan->Getpmarkicon_image_list(m_listIconSize),
      wxIMAGE_LIST_SMALL);
  m_pLayListCtrl->SetImageList(imglist, wxIMAGE_LIST_SMALL);

  SetColorScheme();

  UpdateLists();

  // This should work under Linux :-(
  // m_pNotebook->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
  // wxNotebookEventHandler(RouteManagerDialog::OnTabSwitch), NULL, this);

  m_bNeedConfigFlush = false;
}

RouteManagerDialog::~RouteManagerDialog() {
  delete m_pRouteListCtrl;
  delete m_pTrkListCtrl;
  delete m_pWptListCtrl;
  delete m_pLayListCtrl;

  delete btnRteDelete;
  delete btnRteResequence;
  delete btnRteExport;
  delete btnRteZoomto;
  delete btnRteProperties;
  delete btnRteActivate;
  delete btnRteReverse;
  delete btnRteSendToGPS;
  delete btnRteDeleteAll;
  delete btnTrkNew;
  delete btnTrkProperties;
  delete btnTrkDelete;
  delete btnTrkExport;
  delete btnTrkRouteFromTrack;
  delete btnTrkDeleteAll;
  delete btnWptNew;
  delete btnWptProperties;
  delete btnWptZoomto;
  delete btnWptDelete;
  delete btnWptGoTo;
  delete btnWptExport;
  delete btnWptSendToGPS;
  delete btnWptDeleteAll;
  delete btnLayNew;
  // delete btnLayProperties;
  delete cbLayToggleChart;
  delete cbLayToggleListing;
  delete cbLayToggleNames;
  delete btnLayDelete;
  delete btnImport;
  delete btnExport;
  delete btnExportViz;
  btnImport = NULL;
  btnExport = NULL;
  btnExportViz = NULL;

  delete m_pNotebook;
  instanceFlag = false;
}

void RouteManagerDialog::RecalculateSize() {
  //  All of this dialog layout is expandable, so we need to set a specific size
  //  target for the onscreen display. The size will then be adjusted so that it
  //  fits within the parent's client area, with some padding

  //  Get a text height metric for reference
  int char_width, char_height;
  GetTextExtent(_T("W"), &char_width, &char_height);

  wxSize sz;
  sz.x = 60 * char_width;
  sz.y = 30 * char_height;

  wxSize dsize = GetParent()->GetClientSize();
  sz.y = wxMin(sz.y, dsize.y);
  sz.x = wxMin(sz.x, dsize.x);
  SetClientSize(sz);

  wxSize fsize = GetSize();
  fsize.y = wxMin(fsize.y, dsize.y);
  fsize.x = wxMin(fsize.x, dsize.x);
  SetSize(fsize);

  CentreOnParent();
}

void RouteManagerDialog::OnClose(wxCloseEvent &event) {
#ifdef __WXGTK__
  gFrame->Raise();
#endif
  Hide();
  //    pRouteManagerDialog = NULL;
}

void RouteManagerDialog::OnOK(wxCommandEvent &event) {
#ifdef __WXGTK__
  gFrame->Raise();
#endif
  Hide();
}

void RouteManagerDialog::SetColorScheme() { DimeControl(this); }

void RouteManagerDialog::OnShowAllRteCBClicked(wxCommandEvent &event) {
  bool viz = m_cbShowAllRte->GetValue();
  long item = -1;
  for (;;) {
    item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                         wxLIST_STATE_DONTCARE);
    if (item == -1) break;

    Route *pR = (Route *)m_pRouteListCtrl->GetItemData(item);

    pR->SetVisible(viz, viz);
    pR->SetSharedWPViz(viz);

    m_pRouteListCtrl->SetItemImage(item, !viz);  // visible

    pConfig->UpdateRoute(pR);
  }

  UpdateWptListCtrlViz();

  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::OnShowAllWpCBClicked(wxCommandEvent &event) {
  bool viz = m_cbShowAllWP->GetValue();
  long item = -1;
  for (;;) {
    item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_DONTCARE);
    if (item == -1) break;

    RoutePoint *pRP = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

    if (!pRP->IsSharedInVisibleRoute()) {
      pRP->SetVisible(viz);
    } else
      pRP->SetVisible(true);

    m_pWptListCtrl->SetItemImage(item, RoutePointGui(*pRP).GetIconImageIndex());
    pConfig->UpdateWayPoint(pRP);
  }

  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::OnShowAllTrkCBClicked(wxCommandEvent &event) {
  bool viz = m_cbShowAllTrk->GetValue();
  long item = -1;
  for (;;) {
    item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_DONTCARE);
    if (item == -1) break;

    Track *track = (Track *)m_pTrkListCtrl->GetItemData(item);

    track->SetVisible(viz);
    m_pTrkListCtrl->SetItemImage(item, track->IsVisible() ? 0 : 1);
  }

  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::OnShowAllLayCBClicked(wxCommandEvent &event) {
  bool viz = m_cbShowAllLay->GetValue();
  long item = -1;
  for (;;) {
    item = m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_DONTCARE);
    if (item == -1) break;

    Layer *layer = (Layer *)m_pLayListCtrl->GetItemData(item);

    layer->SetVisibleOnChart(viz);
    m_pLayListCtrl->SetItemImage(item, layer->IsVisibleOnChart() ? 0 : 1);

    ToggleLayerContentsOnChart(layer);
  }

  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::UpdateRouteListCtrl() {
  // if an item was selected, make it selected again if it still exist
  long item = -1;
  item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
  wxUIntPtr selected_id = wxUIntPtr(0);
  if (item != -1) selected_id = m_pRouteListCtrl->GetItemData(item);

  // Delete existing items
  m_pRouteListCtrl->DeleteAllItems();

  // then add routes to the listctrl
  RouteList::iterator it;
  int index = 0;
  int list_index = 0;
  bool bpartialViz = false;

  for (it = (*pRouteList).begin(); it != (*pRouteList).end(); ++it, ++index) {
    if (!(*it)->IsListed()) continue;

    if (!(*it)->GetName().Upper().Contains(m_tFilterRte->GetValue().Upper())) {
      continue;
    }

    wxListItem li;
    li.SetId(list_index);
    li.SetImage((*it)->IsVisible() ? 0 : 1);
    li.SetData(*it);
    li.SetText(_T(""));
    li.SetAlign(wxLIST_FORMAT_LEFT);

    if ((*it)->m_bRtIsActive) {
      wxFont font = *wxNORMAL_FONT;
      font.SetWeight(wxFONTWEIGHT_BOLD);
      li.SetFont(font);
    }

    long idx = m_pRouteListCtrl->InsertItem(li);

    wxString name = (*it)->m_RouteNameString;
    if (name.IsEmpty()) name = _("(Unnamed Route)");
    m_pRouteListCtrl->SetItem(idx, rmROUTENAME, name);

    wxString startend = (*it)->m_RouteStartString;
    if (!(*it)->m_RouteEndString.IsEmpty())
      startend.append(_(" - ") + (*it)->m_RouteEndString);
    m_pRouteListCtrl->SetItem(idx, rmROUTEDESC, startend);

    wxListItem lic;
    lic.SetId(list_index);
    lic.SetColumn(1);
    lic.SetAlign(wxLIST_FORMAT_LEFT);
    m_pRouteListCtrl->SetItem(lic);

    lic.SetColumn(2);
    lic.SetAlign(wxLIST_FORMAT_LEFT);
    m_pRouteListCtrl->SetItem(lic);

    // Keep track if any are invisible
    if (!(*it)->IsVisible()) bpartialViz = true;

    list_index++;
  }

  m_pRouteListCtrl->SortItems(SortRoutesOnName, (wxIntPtr)NULL);

  m_pRouteListCtrl->SetColumnWidth(0, 4 * m_charWidth);

  // restore selection if possible
  // NOTE this will select a different item, if one is deleted
  // (the next route will get that index).
  if (selected_id != wxUIntPtr(0)) {
    item = m_pRouteListCtrl->FindItem(-1, selected_id);
    m_pRouteListCtrl->SetItemState(
        item, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
        wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
  }

  if ((m_lastRteItem >= 0) && (m_pRouteListCtrl->GetItemCount()))
    m_pRouteListCtrl->EnsureVisible(m_lastRteItem);
  UpdateRteButtons();

  //  If any route is invisible, then "show all" cb must be clear.
  m_cbShowAllRte->SetValue(!bpartialViz);
}

void RouteManagerDialog::UpdateRteButtons() {
  // enable/disable buttons
  long selected_index_index =
      m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  bool enable1 = m_pRouteListCtrl->GetSelectedItemCount() == 1;
  bool enablemultiple = m_pRouteListCtrl->GetSelectedItemCount() >= 1;

  m_lastRteItem = selected_index_index;

  btnRteDelete->Enable(m_pRouteListCtrl->GetSelectedItemCount() > 0);
  btnRteZoomto->Enable(enable1);
  btnRteProperties->Enable(enable1);
  btnRteReverse->Enable(enable1);
  btnRteExport->Enable(enablemultiple);
  btnRteResequence->Enable(enable1);
  btnRteSendToGPS->Enable(enable1);
  btnRteDeleteAll->Enable(m_pRouteListCtrl->GetItemCount() > 0);
  btnRteSendToPeer->Enable(enablemultiple);

  // set activate button text
  Route *route = NULL;
  if (enable1)
    route = (Route *)m_pRouteListCtrl->GetItemData(selected_index_index);

  if (!g_pRouteMan->IsAnyRouteActive()) {
    btnRteActivate->Enable(enable1);
    if (enable1) btnRteActivate->SetLabel(_("Activate"));

  } else {
    if (enable1) {
      if (route && route->m_bRtIsActive) {
        btnRteActivate->Enable(enable1);
        btnRteActivate->SetLabel(_("Deactivate"));
      } else
        btnRteActivate->Enable(false);
    } else
      btnRteActivate->Enable(false);
  }
}

void RouteManagerDialog::MakeAllRoutesInvisible() {
  RouteList::iterator it;
  long index = 0;
  for (it = (*pRouteList).begin(); it != (*pRouteList).end(); ++it, ++index) {
    if ((*it)->IsVisible()) {  // avoid config updating as much as possible!
      (*it)->SetVisible(false);
      m_pRouteListCtrl->SetItemImage(m_pRouteListCtrl->FindItem(-1, index),
                                     1);  // Likely not same order :0
      pConfig->UpdateRoute(*it);          // auch, flushes config to disk. FIXME
    }
  }
}

void RouteManagerDialog::ZoomtoRoute(Route *route) {
  // Calculate bbox center
  LLBBox RBBox = route->GetBBox();
  double clat = (RBBox.GetMinLat() + RBBox.GetMaxLat()) / 2;
  double clon = (RBBox.GetMinLon() + RBBox.GetMaxLon()) / 2;

  // Calculate ppm
  double rw, rh, ppm;  // route width, height, final ppm scale to use
  int ww, wh;          // chart window width, height
  // route bbox width in nm
  DistanceBearingMercator(RBBox.GetMinLat(), RBBox.GetMinLon(),
                          RBBox.GetMinLat(), RBBox.GetMaxLon(), NULL, &rw);
  // route bbox height in nm
  DistanceBearingMercator(RBBox.GetMinLat(), RBBox.GetMinLon(),
                          RBBox.GetMaxLat(), RBBox.GetMinLon(), NULL, &rh);

  if (gFrame->GetFocusCanvas()) {
    gFrame->GetFocusCanvas()->GetSize(&ww, &wh);
    ppm = wxMin(ww / (rw * 1852), wh / (rh * 1852)) * (100 - fabs(clat)) / 90;
    ppm = wxMin(ppm, 1.0);

    gFrame->JumpToPosition(gFrame->GetFocusCanvas(), clat, clon, ppm);
  }

  m_bNeedConfigFlush = true;
}

// BEGIN Event handlers
void RouteManagerDialog::OnRteDeleteClick(wxCommandEvent &event) {
  RouteList list;

  int answer = OCPNMessageBox(
      this, _("Are you sure you want to delete the selected object(s)"),
      wxString(_("OpenCPN Alert")), wxYES_NO);
  if (answer != wxID_YES) return;

  bool busy = false;
  if (m_pRouteListCtrl->GetSelectedItemCount()) {
    ::wxBeginBusyCursor();
    gFrame->CancelAllMouseRoute();
    m_bNeedConfigFlush = true;
    busy = true;
  }

  long item = -1;
  for (;;) {
    item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                         wxLIST_STATE_SELECTED);
    if (item == -1) break;

    Route *proute_to_delete = (Route *)m_pRouteListCtrl->GetItemData(item);

    if (proute_to_delete) list.Append(proute_to_delete);
  }

  if (busy) {
    for (unsigned int i = 0; i < list.GetCount(); i++) {
      Route *route = list.Item(i)->GetData();
      if (route) {
        pConfig->DeleteConfigRoute(route);
        g_pRouteMan->DeleteRoute(route, NavObjectChanges::getInstance());
      }
    }

    m_lastRteItem = -1;
    UpdateRouteListCtrl();
    UpdateTrkListCtrl();

    gFrame->InvalidateAllCanvasUndo();
    gFrame->RefreshAllCanvas();
    ::wxEndBusyCursor();
  }
}

void RouteManagerDialog::OnRteDeleteAllClick(wxCommandEvent &event) {
  int dialog_ret =
      OCPNMessageBox(this, _("Are you sure you want to delete <ALL> routes?"),
                     wxString(_("OpenCPN Alert")), wxYES_NO);

  if (dialog_ret == wxID_YES) {
    if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();

    gFrame->CancelAllMouseRoute();

    g_pRouteMan->DeleteAllRoutes(NavObjectChanges::getInstance());
    // TODO Seth
    //            m_pSelectedRoute = NULL;
    //            m_pFoundRoutePoint = NULL;
    //            m_pFoundRoutePointSecond = NULL;

    m_lastRteItem = -1;
    UpdateRouteListCtrl();

    //    Also need to update the track list control, since routes and tracks
    //    share a common global list (pRouteList)
    UpdateTrkListCtrl();

    if (pRoutePropDialog) pRoutePropDialog->Hide();
    gFrame->InvalidateAllCanvasUndo();
    gFrame->RefreshAllCanvas();

    m_bNeedConfigFlush = true;
  }
}

void RouteManagerDialog::OnRtePropertiesClick(wxCommandEvent &event) {
  // Show routeproperties dialog for selected route
  long item = -1;
  item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Route *route = (Route *)m_pRouteListCtrl->GetItemData(item);

  if (!route) return;

  pRoutePropDialog = RoutePropDlgImpl::getInstance(GetParent());

  pRoutePropDialog->SetRouteAndUpdate(route);

  if (!pRoutePropDialog->IsShown()) pRoutePropDialog->Show();

  pRoutePropDialog->Raise();

  m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteZoomtoClick(wxCommandEvent &event) {
  // Zoom into the bounding box of the selected route
  long item = -1;
  item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
  if (item == -1) return;

  // optionally make this route exclusively visible
  if (m_bCtrlDown) MakeAllRoutesInvisible();

  Route *route = (Route *)m_pRouteListCtrl->GetItemData(item);

  if (!route) return;

  // Ensure route is visible
  if (!route->IsVisible()) {
    route->SetVisible(true);
    m_pRouteListCtrl->SetItemImage(item, route->IsVisible() ? 0 : 1);
    pConfig->UpdateRoute(route);
  }

  ZoomtoRoute(route);
}

void RouteManagerDialog::OnRteReverseClick(wxCommandEvent &event) {
  // Reverse selected route
  long item = -1;
  item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Route *route = (Route *)m_pRouteListCtrl->GetItemData(item);

  if (!route) return;
  if (route->m_bIsInLayer) return;

  int ask_return = OCPNMessageBox(this, g_pRouteMan->GetRouteReverseMessage(),
                                  _("Rename Waypoints?"), wxYES_NO | wxCANCEL);
  if (ask_return != wxID_CANCEL) {
    bool rename = (ask_return == wxID_YES);

    pSelect->DeleteAllSelectableRouteSegments(route);
    route->Reverse(rename);
    pSelect->AddAllSelectableRouteSegments(route);

    // update column 2 - create a UpdateRouteItem(index) instead?
    wxString startend = route->m_RouteStartString;
    if (!route->m_RouteEndString.IsEmpty())
      startend.append(_(" - ") + route->m_RouteEndString);
    m_pRouteListCtrl->SetItem(item, 2, startend);

    pConfig->UpdateRoute(route);
    gFrame->RefreshAllCanvas();
  }

  m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteExportClick(wxCommandEvent &event) {
  RouteList list;

  wxString suggested_name = _T("routes");

  long item = -1;
  for (;;) {
    item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                         wxLIST_STATE_SELECTED);
    if (item == -1) break;

    Route *proute_to_export = (Route *)m_pRouteListCtrl->GetItemData(item);

    if (proute_to_export) {
      list.Append(proute_to_export);
      if (proute_to_export->m_RouteNameString != wxEmptyString)
        suggested_name = proute_to_export->m_RouteNameString;
    }
  }

  ExportGPXRoutes(this, &list, suggested_name);
}

void RouteManagerDialog::OnRteResequenceClick(wxCommandEvent &event) {
  long item = -1;
  item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Route *route = (Route *)m_pRouteListCtrl->GetItemData(item);

  if (!route) return;
  if (route->m_bIsInLayer) return;
  route->RenameRoutePoints();
}

void RouteManagerDialog::OnRteSendToPeerClick(wxCommandEvent &event) {
  std::vector<Route *> list;
  long item = -1;
  for (;;) {
    item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                         wxLIST_STATE_SELECTED);
    if (item == -1) break;

    Route *proute = (Route *)m_pRouteListCtrl->GetItemData(item);

    if (proute) {
      list.push_back(proute);
    }
  }
  if (!list.empty()) {
    SendToPeerDlg dlg;
    for (auto r : list) {
      dlg.SetRoute(r);
    }

    // Perform initial scan, if necessary

    // Check for stale cache...
    MdnsCache::GetInstance().Validate();
    if (MdnsCache::GetInstance().GetCache().empty()) dlg.SetScanOnCreate(true);

    dlg.SetScanTime(5);  // seconds
    dlg.Create(NULL, -1, _("Send Route(s) to OpenCPN Peer") + _T( "..." ),
               _T(""));
    dlg.ShowModal();
  }
}

void RouteManagerDialog::OnWptSendToPeerClick(wxCommandEvent &event) {
  std::vector<RoutePoint *> list;
  long item = -1;
  for (;;) {
    item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    if (item == -1) break;

    RoutePoint *proutep = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

    if (proutep) {
      list.push_back(proutep);
    }
  }
  if (!list.empty()) {
    SendToPeerDlg dlg;
    for (auto r : list) {
      dlg.SetWaypoint(r);
    }

    // Perform initial scan, if necessary

    // Check for stale cache...
    MdnsCache::GetInstance().Validate();
    if (MdnsCache::GetInstance().GetCache().empty()) dlg.SetScanOnCreate(true);

    dlg.SetScanTime(5);  // seconds
    dlg.Create(NULL, -1, _("Send Waypoint(s) to OpenCPN Peer") + _T( "..." ),
               _T(""));
    dlg.ShowModal();
  }
}

void RouteManagerDialog::OnTrkSendToPeerClick(wxCommandEvent &event) {
  std::vector<Track *> list;
  long item = -1;
  for (;;) {
    item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    if (item == -1) break;

    Track *ptrk = (Track *)m_pTrkListCtrl->GetItemData(item);

    if (ptrk) {
      list.push_back(ptrk);
    }
  }
  if (!list.empty()) {
    SendToPeerDlg dlg;
    for (auto r : list) {
      dlg.SetTrack(r);
    }

    // Perform initial scan, if necessary

    // Check for stale cache...
    MdnsCache::GetInstance().Validate();
    if (MdnsCache::GetInstance().GetCache().empty()) dlg.SetScanOnCreate(true);

    dlg.SetScanTime(5);  // seconds
    dlg.Create(NULL, -1, _("Send Track(s) to OpenCPN Peer") + _T( "..." ),
               _T(""));
    dlg.ShowModal();
  }
}

void RouteManagerDialog::OnRteActivateClick(wxCommandEvent &event) {
  // Activate the selected route, unless it already is
  long item = -1;
  item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
  if (item == -1) return;

  if (m_bCtrlDown) MakeAllRoutesInvisible();

  Route *route = (Route *)m_pRouteListCtrl->GetItemData(item);

  if (!route) return;

  if (!route->m_bRtIsActive) {
    if (!route->IsVisible()) {
      route->SetVisible(true);
      m_pRouteListCtrl->SetItemImage(item, 0, 0);
    }

    ZoomtoRoute(route);

    RoutePoint *best_point =
        g_pRouteMan->FindBestActivatePoint(route, gLat, gLon, gCog, gSog);
    g_pRouteMan->ActivateRoute(route, best_point);
    //            g_pRouteMan->ActivateRoute(route);
  } else
    g_pRouteMan->DeactivateRoute();

  UpdateRouteListCtrl();

  pConfig->UpdateRoute(route);

  gFrame->RefreshAllCanvas();

  //      btnRteActivate->SetLabel(route->m_bRtIsActive ? _("Deactivate") :
  //      _("Activate"));

  m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteToggleVisibility(wxMouseEvent &event) {
  wxPoint pos = event.GetPosition();
  int flags = 0;
  long clicked_index = m_pRouteListCtrl->HitTest(pos, flags);

  //    Clicking Visibility column?
  if (clicked_index > -1 &&
      event.GetX() < m_pRouteListCtrl->GetColumnWidth(rmVISIBLE)) {
    // Process the clicked item
    Route *route = (Route *)m_pRouteListCtrl->GetItemData(clicked_index);

    route->SetVisible(!route->IsVisible());

    m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : 1);

    ::wxBeginBusyCursor();

    pConfig->UpdateRoute(route);
    gFrame->RefreshAllCanvas();

    //   We need to update the waypoint list control since the visibility of
    //   shared waypoints might have changed.
    if (g_pRouteMan->DoesRouteContainSharedPoints(route))
      UpdateWptListCtrlViz();

    // Manage "show all" checkbox
    bool viz = true;
    long item = -1;
    for (;;) {
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                           wxLIST_STATE_DONTCARE);
      if (item == -1) break;

      Route *pR = (Route *)m_pRouteListCtrl->GetItemData(item);

      if (!pR->IsVisible()) {
        viz = false;
        break;
      }
    }
    m_cbShowAllRte->SetValue(viz);

    ::wxEndBusyCursor();
  }

  // Allow wx to process...
  event.Skip();
}

void RouteManagerDialog::OnRteBtnLeftDown(wxMouseEvent &event) {
  m_bCtrlDown = event.ControlDown();
  event.Skip();
}

void RouteManagerDialog::OnRteSelected(wxListEvent &event) {
  long clicked_index = event.m_itemIndex;
  // Process the clicked item
  Route *route = (Route *)m_pRouteListCtrl->GetItemData(clicked_index);
  //    route->SetVisible(!route->IsVisible());
  m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : 1);
  //    pConfig->UpdateRoute(route);

  gFrame->RefreshAllCanvas();

  UpdateRteButtons();
}

void RouteManagerDialog::OnRteColumnClicked(wxListEvent &event) {
  if (event.m_col == 1) {
    sort_route_name_dir++;

    m_pRouteListCtrl->SortItems(SortRoutesOnName, (wxIntPtr)NULL);
  } else if (event.m_col == 2) {
    sort_route_to_dir++;
    m_pRouteListCtrl->SortItems(SortRoutesOnTo, (wxIntPtr)NULL);
  }
}

void RouteManagerDialog::OnRteSendToGPSClick(wxCommandEvent &event) {
  long item = -1;
  item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Route *route = (Route *)m_pRouteListCtrl->GetItemData(item);

  if (!route) return;

  SendToGpsDlg *pdlg = new SendToGpsDlg();
  pdlg->SetRoute(route);

  wxFont fo = GetOCPNGUIScaledFont(_T("Dialog"));
  pdlg->SetFont(fo);

  wxString source;
  pdlg->Create(NULL, -1, _("Send to GPS") + _T( "..." ), source);

#ifdef __WXOSX__
  HideWithEffect(wxSHOW_EFFECT_BLEND);
#endif

  pdlg->ShowModal();

#ifdef __WXOSX__
  ShowWithEffect(wxSHOW_EFFECT_BLEND);
#endif

  pdlg->Destroy();
}

void RouteManagerDialog::OnRteDefaultAction(wxListEvent &event) {
  wxCommandEvent evt;
  OnRtePropertiesClick(evt);
}

void RouteManagerDialog::OnTrkDefaultAction(wxListEvent &event) {
  wxCommandEvent evt;
  OnTrkPropertiesClick(evt);
}

void RouteManagerDialog::OnTrkRightClick(wxListEvent &event) {
  wxMenu menu;
  wxMenuItem *mergeItem = menu.Append(TRACK_MERGE, _("&Merge Selected Tracks"));
  mergeItem->Enable(m_pTrkListCtrl->GetSelectedItemCount() > 1);
  wxMenuItem *cleanItem = menu.Append(TRACK_CLEAN, _("Reduce Data..."));
  cleanItem->Enable(m_pTrkListCtrl->GetSelectedItemCount() == 1);
  wxMenuItem *copyItem = menu.Append(TRACK_COPY_TEXT, _("&Copy as text"));
  copyItem->Enable(m_pTrkListCtrl->GetSelectedItemCount() > 0);
  PopupMenu(&menu);
}

static bool CompareTracks(Track *track1, Track *track2) {
  TrackPoint *start1 = track1->GetPoint(0);
  TrackPoint *start2 = track2->GetPoint(0);
  return start1->GetCreateTime() < start2->GetCreateTime();
}

void RouteManagerDialog::OnTrkMenuSelected(wxCommandEvent &event) {
  int item = -1;

  switch (event.GetId()) {
    case TRACK_CLEAN: {
      item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                         wxLIST_STATE_SELECTED);
      if (item == -1) break;
      Track *track = (Track *)m_pTrkListCtrl->GetItemData(item);
      if (track->IsRunning()) {
        wxBell();
        break;
      }

      const wxString choices[] = {_T("5.0"), _T("10.0"), _T("20.0"), _T("50.0"),
                                  _T("100.0")};

      wxSingleChoiceDialog precisionDlg(this,
                                        _("Select the maximum error allowed "
                                          "(in meters)\nafter data reduction:"),
                                        _("Reduce Data Precision"), 5, choices);
#ifdef __WXOSX__
      precisionDlg.ShowWindowModal();
      while (precisionDlg.IsShown()) {
        wxMilliSleep(10);
        wxYield();
      }
      int result = precisionDlg.GetReturnCode();
#else
      int result = precisionDlg.ShowModal();
#endif
      if (result == wxID_CANCEL) break;
      double precision = 5.0;
      switch (precisionDlg.GetSelection()) {
        case 0:
          precision = 5.0;
          break;
        case 1:
          precision = 10.0;
          break;
        case 2:
          precision = 20.0;
          break;
        case 3:
          precision = 50.0;
          break;
        case 4:
          precision = 100.0;
          break;
      }

      int pointsBefore = track->GetnPoints();

      int reduction = track->Simplify(precision);
      gFrame->Refresh(false);

      reduction = 100 * reduction / pointsBefore;
      wxString msg = wxString::Format(
          _("The amount of data used by the track\n was reduced by %d%%."),
          reduction);
      OCPNMessageBox(this, msg, _("OpenCPN info"), wxICON_INFORMATION | wxOK);

      UpdateTrkListCtrl();
      UpdateRouteListCtrl();

      break;
    }

    case TRACK_COPY_TEXT: {
      wxString csvString;
      while (1) {
        item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                           wxLIST_STATE_SELECTED);
        if (item == -1) break;
        Track *track = (Track *)m_pTrkListCtrl->GetItemData(item);
        csvString << track->GetName() << _T("\t")
                  << wxString::Format(_T("%.1f"), track->Length()) << _T("\t")
                  << _T("\n");
      }

      if (wxTheClipboard->Open()) {
        wxTextDataObject *data = new wxTextDataObject;
        data->SetText(csvString);
        wxTheClipboard->SetData(data);
        wxTheClipboard->Close();
      }

      break;
    }

    case TRACK_MERGE: {
      Track *targetTrack = NULL;
      TrackPoint *tPoint;
      TrackPoint *newPoint;
      TrackPoint *lastPoint;
      std::vector<Track *> mergeList;
      std::vector<Track *> deleteList;
      bool runningSkipped = false;

      ::wxBeginBusyCursor();

      while (1) {
        item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                           wxLIST_STATE_SELECTED);
        if (item == -1) break;
        Track *track = (Track *)m_pTrkListCtrl->GetItemData(item);
        mergeList.push_back(track);
      }

      std::sort(mergeList.begin(), mergeList.end(), CompareTracks);

      targetTrack = mergeList[0];
      lastPoint = targetTrack->GetLastPoint();

      for (auto const &mergeTrack : mergeList) {
        if (mergeTrack == *mergeList.begin()) continue;

        if (mergeTrack->IsRunning()) {
          runningSkipped = true;
          continue;
        }

        for (int i = 0; i < mergeTrack->GetnPoints(); i++) {
          tPoint = mergeTrack->GetPoint(i);
          newPoint = new TrackPoint(tPoint->m_lat, tPoint->m_lon,
                                    tPoint->GetCreateTime());

          targetTrack->AddPoint(newPoint);

          pSelect->AddSelectableTrackSegment(lastPoint->m_lat, lastPoint->m_lon,
                                             newPoint->m_lat, newPoint->m_lon,
                                             lastPoint, newPoint, targetTrack);

          lastPoint = newPoint;
        }
        deleteList.push_back(mergeTrack);
      }

      for (auto const &deleteTrack : deleteList) {
        g_pAIS->DeletePersistentTrack(deleteTrack);
        pConfig->DeleteConfigTrack(deleteTrack);
        RoutemanGui(*g_pRouteMan).DeleteTrack(deleteTrack);
      }

      mergeList.clear();
      deleteList.clear();

      ::wxEndBusyCursor();

      UpdateTrkListCtrl();
      UpdateRouteListCtrl();
      gFrame->RefreshAllCanvas();

      if (runningSkipped) {
        wxMessageDialog skipWarning(
            this,
            _("The currently running Track was not merged.\nYou can merge it "
              "later when it is completed."),
            _T("Warning"), wxCANCEL | wxICON_WARNING);
        skipWarning.ShowModal();
      }

      break;
    }
  }
}

void RouteManagerDialog::UpdateTrkListCtrl() {
  // if an item was selected, make it selected again if it still exist
  long item = -1;
  item =
      m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

  wxUIntPtr selected_id = wxUIntPtr(0);
  if (item != -1) selected_id = m_pTrkListCtrl->GetItemData(item);

  // Delete existing items
  m_pTrkListCtrl->DeleteAllItems();

  // then add tracks to the listctrl
  std::vector<Track *>::iterator it;
  int index = 0;
  int list_index = 0;
  bool bpartialViz = false;

  for (Track *trk : g_TrackList) {
    if (!trk->IsVisible()) bpartialViz = true;

    if (!trk->IsListed()) continue;

    if (!trk->GetName(true).Upper().Contains(
            m_tFilterTrk->GetValue().Upper())) {
      continue;
    }

    wxListItem li;
    li.SetId(list_index);
    li.SetImage(trk->IsVisible() ? 0 : 1);
    li.SetData(trk);
    li.SetText(_T(""));

    if (g_pActiveTrack == trk) {
      wxFont font = *wxNORMAL_FONT;
      font.SetWeight(wxFONTWEIGHT_BOLD);
      li.SetFont(font);
    }
    long idx = m_pTrkListCtrl->InsertItem(li);

    m_pTrkListCtrl->SetItem(idx, colTRKNAME, trk->GetName(true));
    m_pTrkListCtrl->SetItem(idx, colTRKDATE, trk->GetDate(true));

    wxString len;
    len.Printf(wxT("%5.2f"), trk->Length());
    m_pTrkListCtrl->SetItem(idx, colTRKLENGTH, len);

    wxListItem lic;
    lic.SetId(list_index);
    lic.SetColumn(1);
    lic.SetAlign(wxLIST_FORMAT_LEFT);
    m_pTrkListCtrl->SetItem(lic);

    lic.SetColumn(2);
    lic.SetAlign(wxLIST_FORMAT_LEFT);
    m_pTrkListCtrl->SetItem(lic);

    list_index++;
  }

  switch (sort_track_key) {
    case SORT_ON_DISTANCE:
      m_pTrkListCtrl->SortItems(SortTracksOnDistance, (wxIntPtr)NULL);
      break;
    case SORT_ON_DATE:
      m_pTrkListCtrl->SortItems(SortTracksOnDate, (wxIntPtr)NULL);
      break;
    case SORT_ON_NAME:
    default:
      m_pTrkListCtrl->SortItems(SortTracksOnName, (wxIntPtr)NULL);
      break;
  }

  m_pTrkListCtrl->SetColumnWidth(0, 4 * m_charWidth);

  // restore selection if possible
  // NOTE this will select a different item, if one is deleted
  // (the next route will get that index).
  if (selected_id != wxUIntPtr(0)) {
    item = m_pTrkListCtrl->FindItem(-1, selected_id);
    m_pTrkListCtrl->SetItemState(item,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
  }

  if ((m_lastTrkItem >= 0) && (m_pTrkListCtrl->GetItemCount()))
    m_pTrkListCtrl->EnsureVisible(m_lastTrkItem);

  m_cbShowAllTrk->SetValue(!bpartialViz);
  UpdateTrkButtons();
}

void RouteManagerDialog::OnTrkSelected(wxListEvent &event) {
  UpdateTrkButtons();
}

void RouteManagerDialog::OnTrkColumnClicked(wxListEvent &event) {
  if (event.m_col == 1) {
    sort_track_key = SORT_ON_NAME;
    sort_track_name_dir++;
    m_pTrkListCtrl->SortItems(SortTracksOnName, (wxIntPtr)NULL);
  } else if (event.m_col == 2) {
    sort_track_key = SORT_ON_DISTANCE;
    sort_track_len_dir++;
    m_pTrkListCtrl->SortItems(SortTracksOnDistance, (wxIntPtr)NULL);
  } else if (event.m_col == 3) {
    sort_track_key = SORT_ON_DATE;
    sort_track_date_dir++;
    m_pTrkListCtrl->SortItems(SortTracksOnDate, (wxIntPtr)NULL);
  }
}

void RouteManagerDialog::UpdateTrkButtons() {
  long item = -1;
  item =
      m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  int items = m_pTrkListCtrl->GetSelectedItemCount();

  m_lastTrkItem = item;

  btnTrkProperties->Enable(items == 1);
  btnTrkDelete->Enable(items >= 1);
  btnTrkExport->Enable(items >= 1);
  btnTrkRouteFromTrack->Enable(items == 1);
  btnTrkDeleteAll->Enable(m_pTrkListCtrl->GetItemCount() > 0);
  btnTrkSendToPeer->Enable(items >= 1);
}

void RouteManagerDialog::OnTrkToggleVisibility(wxMouseEvent &event) {
  wxPoint pos = event.GetPosition();
  int flags = 0;
  long clicked_index = m_pTrkListCtrl->HitTest(pos, flags);

  //    Clicking Visibility column?
  if (clicked_index > -1 &&
      event.GetX() < m_pTrkListCtrl->GetColumnWidth(colTRKVISIBLE)) {
    // Process the clicked item
    Track *track = (Track *)m_pTrkListCtrl->GetItemData(clicked_index);
    if (track) {
      track->SetVisible(!track->IsVisible());
      m_pTrkListCtrl->SetItemImage(clicked_index, track->IsVisible() ? 0 : 1);
    }

    // Manage "show all" checkbox
    bool viz = true;
    long item = -1;
    for (;;) {
      item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                         wxLIST_STATE_DONTCARE);
      if (item == -1) break;

      Track *track = (Track *)m_pTrkListCtrl->GetItemData(item);

      if (!track->IsVisible()) {
        viz = false;
        break;
      }
    }
    m_cbShowAllTrk->SetValue(viz);

    gFrame->RefreshAllCanvas();
  }

  // Allow wx to process...
  event.Skip();
}

void RouteManagerDialog::OnTrkNewClick(wxCommandEvent &event) {
  gFrame->TrackOff();
  if (pConfig && pConfig->IsChangesFileDirty()) {
    pConfig->UpdateNavObj(true);
  }

  gFrame->TrackOn();

  UpdateTrkListCtrl();
}

void RouteManagerDialog::OnTrkPropertiesClick(wxCommandEvent &event) {
  // Show trackproperties dialog for selected track
  long item = -1;
  item =
      m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Track *track = (Track *)m_pTrkListCtrl->GetItemData(item);

  if (!track) return;

  pTrackPropDialog = TrackPropDlg::getInstance(GetParent());
  pTrackPropDialog->SetTrackAndUpdate(track);

  if (!pTrackPropDialog->IsShown()) pTrackPropDialog->Show();
  UpdateTrkListCtrl();

  m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnTrkDeleteClick(wxCommandEvent &event) {
  std::vector<Track *> list;

  int answer = OCPNMessageBox(
      this, _("Are you sure you want to delete the selected object(s)"),
      wxString(_("OpenCPN Alert")), wxYES_NO);
  if (answer != wxID_YES) return;

  bool busy = false;
  if (m_pTrkListCtrl->GetSelectedItemCount()) {
    ::wxBeginBusyCursor();
    m_bNeedConfigFlush = true;
    busy = true;
  }

  long item = -1;
  for (;;) {
    item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    if (item == -1) break;

    Track *ptrack_to_delete = (Track *)m_pTrkListCtrl->GetItemData(item);

    if (ptrack_to_delete) list.push_back(ptrack_to_delete);
  }

  if (busy) {
    for (unsigned int i = 0; i < list.size(); i++) {
      Track *track = list.at(i);
      if (track) {
        g_pAIS->DeletePersistentTrack(track);
        pConfig->DeleteConfigTrack(track);
        RoutemanGui(*g_pRouteMan).DeleteTrack(track);
      }
    }

    m_lastTrkItem = -1;
    //        UpdateRouteListCtrl();
    UpdateTrkListCtrl();

    if (pConfig && pConfig->IsChangesFileDirty()) {
      pConfig->UpdateNavObj(true);
    }

    gFrame->InvalidateAllCanvasUndo();
    gFrame->RefreshAllCanvas();
    ::wxEndBusyCursor();
  }
}

void RouteManagerDialog::OnTrkExportClick(wxCommandEvent &event) {
  std::vector<Track *> list;
  wxString suggested_name = _T("tracks");

  long item = -1;
  for (;;) {
    item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    if (item == -1) break;

    Track *ptrack_to_export = (Track *)m_pTrkListCtrl->GetItemData(item);

    if (ptrack_to_export) {
      list.push_back(ptrack_to_export);
      if (ptrack_to_export->GetName() != wxEmptyString)
        suggested_name = ptrack_to_export->GetName();
    }
  }

  ExportGPXTracks(this, &list, suggested_name);
}

void RouteManagerDialog::TrackToRoute(Track *track) {
  if (!track) return;
  if (track->m_bIsInLayer) return;

  wxGenericProgressDialog pprog(_("OpenCPN Converting Track to Route...."),
                                _("Processing Waypoints..."), 101, NULL,
                                wxPD_AUTO_HIDE | wxPD_SMOOTH |
                                    wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME |
                                    wxPD_REMAINING_TIME);

  ::wxBeginBusyCursor();

  Route *route = track->RouteFromTrack(&pprog);

  pRouteList->Append(route);

  pprog.Update(101, _("Done."));

  gFrame->RefreshAllCanvas();

  ::wxEndBusyCursor();
}

void RouteManagerDialog::OnTrkRouteFromTrackClick(wxCommandEvent &event) {
  long item = -1;
  item =
      m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Track *track = (Track *)m_pTrkListCtrl->GetItemData(item);

  TrackToRoute(track);

  UpdateRouteListCtrl();
}

void RouteManagerDialog::OnTrkDeleteAllClick(wxCommandEvent &event) {
  int dialog_ret =
      OCPNMessageBox(this, _("Are you sure you want to delete <ALL> tracks?"),
                     wxString(_("OpenCPN Alert")), wxYES_NO);

  if (dialog_ret == wxID_YES) {
    RoutemanGui(*g_pRouteMan).DeleteAllTracks();
  }

  m_lastTrkItem = -1;
  m_lastRteItem = -1;

  UpdateTrkListCtrl();

  //    Also need to update the route list control, since routes and tracks
  //    share a common global list (pRouteList)
  UpdateRouteListCtrl();

  if (pRoutePropDialog) pRoutePropDialog->Hide();

  gFrame->RefreshAllCanvas();

  m_bNeedConfigFlush = true;
}

void RouteManagerDialog::UpdateWptListCtrl(RoutePoint *rp_select,
                                           bool b_retain_sort) {
  wxIntPtr selected_id = wxUIntPtr(0);
  long item = -1;

  if (NULL == rp_select) {
    // if an item was selected, make it selected again if it still exists
    item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);

    if (item != -1) selected_id = m_pWptListCtrl->GetItemData(item);
  }

  //  Freshen the image list
  m_pWptListCtrl->SetImageList(
      pWayPointMan->Getpmarkicon_image_list(m_listIconSize),
      wxIMAGE_LIST_SMALL);

  m_pWptListCtrl->DeleteAllItems();

  wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

  int index = 0;
  bool b_anyHidden = false;
  while (node) {
    RoutePoint *rp = node->GetData();
    if (rp && rp->IsListed()) {
      if (rp->m_bIsInRoute && !rp->IsShared()) {
        node = node->GetNext();
        continue;
      }

      if (!rp->GetName().Upper().Contains(m_tFilterWpt->GetValue().Upper())) {
        node = node->GetNext();
        continue;
      }

      wxListItem li;
      li.SetId(index);
      li.SetImage(RoutePointGui(*rp).GetIconImageIndex());
      li.SetData(rp);
      li.SetText(_T(""));
      long idx = m_pWptListCtrl->InsertItem(li);

      wxString scamin = wxString::Format(_T("%i"), (int)rp->GetScaMin());
      if (!rp->GetUseSca()) scamin = _("Always");
      if (g_bOverruleScaMin) scamin = _("Overruled");
      m_pWptListCtrl->SetItem(idx, colWPTSCALE, scamin);

      wxString name = rp->GetName();
      if (name.IsEmpty()) name = _("(Unnamed Waypoint)");
      m_pWptListCtrl->SetItem(idx, colWPTNAME, name);

      double dst;
      DistanceBearingMercator(rp->m_lat, rp->m_lon, gLat, gLon, NULL, &dst);
      wxString dist;
      dist.Printf(_T("%5.2f ") + getUsrDistanceUnit(), toUsrDistance(dst));
      m_pWptListCtrl->SetItem(idx, colWPTDIST, dist);

      if (rp == rp_select) selected_id = (wxIntPtr)rp_select;

      wxListItem lic;
      lic.SetId(index);
      lic.SetColumn(1);
      lic.SetAlign(wxLIST_FORMAT_LEFT);
      m_pWptListCtrl->SetItem(lic);

      lic.SetColumn(2);
      lic.SetAlign(wxLIST_FORMAT_LEFT);
      m_pWptListCtrl->SetItem(lic);

      if (!rp->IsVisible()) b_anyHidden = true;

      index++;
    }

    node = node->GetNext();
  }

  if (!b_retain_sort) {
    m_pWptListCtrl->SortItems(SortWaypointsOnName,
                              reinterpret_cast<wxIntPtr>(m_pWptListCtrl));
    sort_wp_key = SORT_ON_NAME;
  } else {
    switch (sort_wp_key) {
      case SORT_ON_NAME:
        m_pWptListCtrl->SortItems(SortWaypointsOnName,
                                  reinterpret_cast<wxIntPtr>(m_pWptListCtrl));
        break;
      case SORT_ON_DISTANCE:
        m_pWptListCtrl->SortItems(SortWaypointsOnDistance,
                                  reinterpret_cast<wxIntPtr>(m_pWptListCtrl));
        break;
    }
  }

  if (selected_id != wxUIntPtr(0)) {
    item = m_pWptListCtrl->FindItem(-1, selected_id);
    m_pWptListCtrl->SetItemState(item,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
  }

  if ((m_lastWptItem >= 0) && (m_pWptListCtrl->GetItemCount()))
    m_pWptListCtrl->EnsureVisible(m_lastWptItem);

  if (pWayPointMan->Getpmarkicon_image_list(m_listIconSize)->GetImageCount()) {
    int iwidth, iheight;
    pWayPointMan->Getpmarkicon_image_list(m_listIconSize)
        ->GetSize(0, iwidth, iheight);

    m_pWptListCtrl->SetColumnWidth(0, wxMax(iwidth + 4, 4 * m_charWidth));
  }

  UpdateWptButtons();

  m_cbShowAllWP->SetValue(!b_anyHidden);
}

void RouteManagerDialog::UpdateWptListCtrlViz() {
  long item = -1;
  for (;;) {
    item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_DONTCARE);
    if (item == -1) break;

    RoutePoint *pRP = (RoutePoint *)m_pWptListCtrl->GetItemData(item);
    int imageIndex = RoutePointGui(*pRP).GetIconImageIndex();

    m_pWptListCtrl->SetItemImage(item, imageIndex);
  }
}

void RouteManagerDialog::OnWptDefaultAction(wxListEvent &event) {
  wxCommandEvent evt;
  OnWptPropertiesClick(evt);
}

void RouteManagerDialog::OnWptSelected(wxListEvent &event) {
  UpdateWptButtons();
}

void RouteManagerDialog::OnWptColumnClicked(wxListEvent &event) {
  if (event.m_col == NAME_COLUMN) {
    sort_wp_name_dir++;
    m_pWptListCtrl->SortItems(SortWaypointsOnName,
                              reinterpret_cast<wxIntPtr>(m_pWptListCtrl));
    sort_wp_key = SORT_ON_NAME;
  } else {
    if (event.m_col == DISTANCE_COLUMN) {
      sort_wp_len_dir++;
      m_pWptListCtrl->SortItems(SortWaypointsOnDistance,
                                reinterpret_cast<wxIntPtr>(m_pWptListCtrl));
      sort_wp_key = SORT_ON_DISTANCE;
    }
  }
  UpdateWptListCtrl();
}

void RouteManagerDialog::UpdateWptButtons() {
  long item = -1;
  item =
      m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  bool enable1 = (m_pWptListCtrl->GetSelectedItemCount() == 1);
  bool enablemultiple = (m_pWptListCtrl->GetSelectedItemCount() >= 1);

  if (enable1)
    m_lastWptItem = item;
  else
    m_lastWptItem = -1;

  //  Check selection to see if it is in a layer
  //  If so, disable the "delete" button
  bool b_delete_enable = true;
  item = -1;
  for (;;) {
    item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    if (item == -1) break;

    RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

    if (wp && wp->m_bIsInLayer) {
      b_delete_enable = false;
      break;
    }
  }

  btnWptProperties->Enable(enablemultiple);
  btnWptZoomto->Enable(enable1);
  btnWptDeleteAll->Enable(m_pWptListCtrl->GetItemCount() > 0);
  btnWptDelete->Enable(b_delete_enable && enablemultiple);
  btnWptGoTo->Enable(enable1);
  btnWptExport->Enable(enablemultiple);
  btnWptSendToGPS->Enable(enable1);
  btnWptSendToPeer->Enable(enablemultiple);
}

void RouteManagerDialog::OnWptToggleVisibility(wxMouseEvent &event) {
  wxPoint pos = event.GetPosition();
  int flags = 0;
  long clicked_index = m_pWptListCtrl->HitTest(pos, flags);

  //    Clicking Visibility column?
  if (clicked_index > -1 &&
      event.GetX() < m_pWptListCtrl->GetColumnWidth(colTRKVISIBLE)) {
    // Process the clicked item
    RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(clicked_index);

    if (!wp->IsSharedInVisibleRoute()) {
      wp->SetVisible(!wp->IsVisible());
      m_pWptListCtrl->SetItemImage(clicked_index,
                                   RoutePointGui(*wp).GetIconImageIndex());

      pConfig->UpdateWayPoint(wp);
    }

    // Manage "show all" checkbox
    bool viz = true;
    long item = -1;
    for (;;) {
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                         wxLIST_STATE_DONTCARE);
      if (item == -1) break;

      RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

      if (!wp->IsVisible()) {
        viz = false;
        break;
      }
    }
    m_cbShowAllWP->SetValue(viz);

    gFrame->RefreshAllCanvas();
  } else  //  clicked on ScaMin column??
    if (clicked_index > -1 &&
        event.GetX() > m_pWptListCtrl->GetColumnWidth(colTRKVISIBLE) &&
        event.GetX() < (m_pWptListCtrl->GetColumnWidth(colTRKVISIBLE) +
                        m_pWptListCtrl->GetColumnWidth(colWPTSCALE)) &&
        !g_bOverruleScaMin) {
      RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(clicked_index);
      wp->SetUseSca(!wp->GetUseSca());
      pConfig->UpdateWayPoint(wp);
      gFrame->RefreshAllCanvas();
      wxString scamin = wxString::Format(_T("%i"), (int)wp->GetScaMin());
      if (!wp->GetUseSca()) scamin = _("Always");
      m_pWptListCtrl->SetItem(clicked_index, colWPTSCALE, scamin);
    }

  // Allow wx to process...
  event.Skip();
}

void RouteManagerDialog::OnWptNewClick(wxCommandEvent &event) {
  RoutePoint *pWP = new RoutePoint(gLat, gLon, g_default_wp_icon, wxEmptyString,
                                   wxEmptyString);
  pWP->m_bIsolatedMark = true;  // This is an isolated mark
  pSelect->AddSelectableRoutePoint(gLat, gLon, pWP);
  pConfig->AddNewWayPoint(pWP, -1);  // use auto next num
  gFrame->RefreshAllCanvas();

  // g_pMarkInfoDialog = MarkInfoImpl::getInstance( GetParent() );
  // There is on global instance of the MarkProp Dialog
  if (!g_pMarkInfoDialog) g_pMarkInfoDialog = new MarkInfoDlg(GetParent());

  WptShowPropertiesDialog(std::vector<RoutePoint *>{pWP}, GetParent());
}

void RouteManagerDialog::OnWptPropertiesClick(wxCommandEvent &event) {
  std::vector<RoutePoint *> wptlist;
  long item = wxNOT_FOUND;
  item =
      m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  while (item != wxNOT_FOUND) {
    auto wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);
    if (wp) {
      wptlist.push_back(wp);
    }
    item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
  }

  if (wptlist.size() == 0) return;

  WptShowPropertiesDialog(wptlist, GetParent());

  UpdateWptListCtrl();
  m_bNeedConfigFlush = true;
}

void RouteManagerDialog::WptShowPropertiesDialog(
    std::vector<RoutePoint *> wptlist, wxWindow *parent) {
  if (!g_pMarkInfoDialog)  // There is one global instance of the MarkProp
                           // Dialog
    g_pMarkInfoDialog = new MarkInfoDlg(parent);

  RoutePointNameValidator *pRPNameValidator = new RoutePointNameValidator();
  g_pMarkInfoDialog->SetNameValidator(pRPNameValidator);
  g_pMarkInfoDialog->SetRoutePoints(wptlist);
  g_pMarkInfoDialog->UpdateProperties();

  wxString base_title = _("Mark Properties");
  if (wptlist[0]->m_bIsInRoute) base_title = _("Waypoint Properties");

  if (wptlist[0]->m_bIsInLayer) {
    wxString caption(wxString::Format(_T("%s, %s: %s"), base_title, _("Layer"),
                                      GetLayerName(wptlist[0]->m_LayerID)));
    g_pMarkInfoDialog->SetDialogTitle(caption);
  } else {
    if (wptlist.size() > 1)
      g_pMarkInfoDialog->SetDialogTitle(
          base_title + wxString::Format(_(" (%lu points)"), wptlist.size()));
    else
      g_pMarkInfoDialog->SetDialogTitle(base_title);
  }

  if (!g_pMarkInfoDialog->IsShown()) g_pMarkInfoDialog->Show();
  g_pMarkInfoDialog->Raise();
}

void RouteManagerDialog::OnWptZoomtoClick(wxCommandEvent &event) {
  long item = -1;
  item =
      m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

  if (!wp) return;

  if (gFrame->GetFocusCanvas()) {
    gFrame->JumpToPosition(gFrame->GetFocusCanvas(), wp->m_lat, wp->m_lon,
                           gFrame->GetFocusCanvas()->GetVPScale());
  }
}

void RouteManagerDialog::OnWptDeleteClick(wxCommandEvent &event) {
  RoutePointList list;

  int answer = OCPNMessageBox(
      this, _("Are you sure you want to delete the selected object(s)"),
      wxString(_("OpenCPN Alert")), wxYES_NO);
  if (answer != wxID_YES) return;

  bool busy = false;
  if (m_pWptListCtrl->GetSelectedItemCount()) {
    ::wxBeginBusyCursor();
    m_bNeedConfigFlush = true;
    busy = true;
  }

  long item = -1;
  long item_last_selected = -1;
  for (;;) {
    item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    if (item == -1) break;

    item_last_selected = item;
    RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

    if (wp && !wp->m_bIsInLayer) list.Append(wp);
  }

  if (busy) {
    for (unsigned int i = 0; i < list.GetCount(); i++) {
      RoutePoint *wp = list.Item(i)->GetData();
      if (wp) {
        if (wp->m_bIsInRoute) {
          if (wxID_YES ==
              OCPNMessageBox(this,
                             _("The waypoint you want to delete is used in a "
                               "route, do you really want to delete it?"),
                             _("OpenCPN Alert"), wxYES_NO))
            pWayPointMan->DestroyWaypoint(wp);
        } else
          pWayPointMan->DestroyWaypoint(wp);
      }
    }

    long item_next =
        m_pWptListCtrl->GetNextItem(item_last_selected);  // next in list
    RoutePoint *wp_next = NULL;
    if (item_next > -1)
      wp_next = (RoutePoint *)m_pWptListCtrl->GetItemData(item_next);

    m_lastWptItem = item_next;

    UpdateRouteListCtrl();
    UpdateTrkListCtrl();
    UpdateWptListCtrl(wp_next, true);

    if (g_pMarkInfoDialog) {
      g_pMarkInfoDialog->ClearData();
    }

    gFrame->InvalidateAllCanvasUndo();
    gFrame->RefreshAllCanvas();
    ::wxEndBusyCursor();
  }
}

void RouteManagerDialog::OnWptGoToClick(wxCommandEvent &event) {
  long item = -1;
  item =
      m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

  if (!wp) return;

  RoutePoint *pWP_src = new RoutePoint(gLat, gLon, g_default_wp_icon,
                                       wxEmptyString, wxEmptyString);
  pSelect->AddSelectableRoutePoint(gLat, gLon, pWP_src);

  Route *temp_route = new Route();
  pRouteList->Append(temp_route);

  temp_route->AddPoint(pWP_src);
  temp_route->AddPoint(wp);

  pSelect->AddSelectableRouteSegment(gLat, gLon, wp->m_lat, wp->m_lon, pWP_src,
                                     wp, temp_route);

  wxString name = wp->GetName();
  if (name.IsEmpty()) name = _("(Unnamed Waypoint)");
  wxString rteName = _("Go to ");
  rteName.Append(name);
  temp_route->m_RouteNameString = rteName;
  temp_route->m_RouteStartString = _("Here");

  temp_route->m_RouteEndString = name;
  temp_route->m_bDeleteOnArrival = true;

  if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();

  g_pRouteMan->ActivateRoute(temp_route, wp);

  UpdateRouteListCtrl();
}

void RouteManagerDialog::OnWptExportClick(wxCommandEvent &event) {
  RoutePointList list;

  wxString suggested_name = _T("waypoints");

  long item = -1;
  for (;;) {
    item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    if (item == -1) break;

    RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

    if (wp && !wp->m_bIsInLayer) {
      list.Append(wp);
      if (wp->GetName() != wxEmptyString) suggested_name = wp->GetName();
    }
  }

  ExportGPXWaypoints(this, &list, suggested_name);
}

void RouteManagerDialog::OnWptSendToGPSClick(wxCommandEvent &event) {
  long item = -1;
  item =
      m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

  if (!wp) return;

  SendToGpsDlg *pdlg = new SendToGpsDlg();
  pdlg->SetWaypoint(wp);

  wxString source;
  pdlg->Create(NULL, -1, _("Send to GPS") + _T( "..." ), source);

#ifdef __WXOSX__
  HideWithEffect(wxSHOW_EFFECT_BLEND);
#endif
  pdlg->ShowModal();
#ifdef __WXOSX__
  ShowWithEffect(wxSHOW_EFFECT_BLEND);
#endif

  delete pdlg;
}

void RouteManagerDialog::OnWptDeleteAllClick(wxCommandEvent &event) {
  wxString prompt;
  int buttons, type;
  if (!pWayPointMan->SharedWptsExist()) {
    prompt = _("Are you sure you want to delete <ALL> waypoints?");
    buttons = wxYES_NO;
    type = 1;
  } else {
    prompt =
        _("There are some waypoints used in routes or anchor alarms.\n Do you "
          "want to delete them as well?\n This will change the routes and "
          "disable the anchor alarms.\n Answering No keeps the waypoints used "
          "in routes or alarms.");
    buttons = wxYES_NO | wxCANCEL;
    type = 2;
  }
  int answer =
      OCPNMessageBox(this, prompt, wxString(_("OpenCPN Alert")), buttons);
  if (answer == wxID_YES) pWayPointMan->DeleteAllWaypoints(true);
  if (answer == wxID_NO && type == 2)
    pWayPointMan->DeleteAllWaypoints(false);  // only delete unused waypoints

  if (g_pMarkInfoDialog) {
    g_pMarkInfoDialog->ClearData();
  }

  m_lastWptItem = -1;
  UpdateRouteListCtrl();
  UpdateWptListCtrl();
  gFrame->InvalidateAllCanvasUndo();
  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::OnLaySelected(wxListEvent &event) {
  UpdateLayButtons();
}

void RouteManagerDialog::OnLayColumnClicked(wxListEvent &event) {
  if (event.m_col == 1) {
    sort_layer_name_dir++;
    m_pLayListCtrl->SortItems(SortLayersOnName, (wxIntPtr)NULL);
  } else if (event.m_col == 2) {
    sort_layer_len_dir++;
    m_pLayListCtrl->SortItems(SortLayersOnSize, (wxIntPtr)NULL);
  }
}

void RouteManagerDialog::UpdateLayButtons() {
  long item = -1;
  item =
      m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  bool enable = (item != -1);

  // btnLayProperties->Enable(false);
  btnLayDelete->Enable(enable);
  cbLayToggleChart->Enable(enable);
  cbLayToggleListing->Enable(enable);
  cbLayToggleNames->Enable(enable);

  if (item >= 0) {
    cbLayToggleChart->SetValue(
        ((Layer *)m_pLayListCtrl->GetItemData(item))->IsVisibleOnChart());

    cbLayToggleNames->Set3StateValue(
        ((Layer *)m_pLayListCtrl->GetItemData(item))->HasVisibleNames());

    cbLayToggleListing->SetValue(
        ((Layer *)m_pLayListCtrl->GetItemData(item))->IsVisibleOnListing());

  } else {
    cbLayToggleChart->SetValue(true);
    cbLayToggleNames->Set3StateValue(wxCHK_UNDETERMINED);
    cbLayToggleListing->SetValue(true);
  }
}

void RouteManagerDialog::OnLayToggleVisibility(wxMouseEvent &event) {
  wxPoint pos = event.GetPosition();
  int flags = 0;
  long clicked_index = m_pLayListCtrl->HitTest(pos, flags);

  //    Clicking Visibility column?
  if (clicked_index > -1 &&
      event.GetX() < m_pLayListCtrl->GetColumnWidth(colLAYVISIBLE)) {
    // Process the clicked item
    Layer *layer = (Layer *)m_pLayListCtrl->GetItemData(clicked_index);

    layer->SetVisibleOnChart(!layer->IsVisibleOnChart());
    m_pLayListCtrl->SetItemImage(clicked_index,
                                 layer->IsVisibleOnChart() ? 0 : 1);

    // Manage "show all" checkbox
    bool viz = true;
    long item = -1;
    for (;;) {
      item = m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                         wxLIST_STATE_DONTCARE);
      if (item == -1) break;

      Layer *layer = (Layer *)m_pLayListCtrl->GetItemData(item);

      if (!layer->IsVisibleOnChart()) {
        viz = false;
        break;
      }
    }
    m_cbShowAllLay->SetValue(viz);

    ToggleLayerContentsOnChart(layer);
  }

  // Allow wx to process...
  event.Skip();
}

void RouteManagerDialog::UpdateLists() {
  UpdateRouteListCtrl();
  UpdateTrkListCtrl();
  UpdateWptListCtrl();
  UpdateLayListCtrl();
}

void RouteManagerDialog::OnLayNewClick(wxCommandEvent &event) {
  AddNewLayer(false);  // Temporary layer
}

void RouteManagerDialog::OnPerLayNewClick(wxCommandEvent &event) {
  AddNewLayer(true);  // Persistent layer
}

void RouteManagerDialog::AddNewLayer(bool isPersistent) {
  bool show_flag = g_bShowLayers;
  g_bShowLayers = true;

  UI_ImportGPX(this, true, _T(""), true, isPersistent);

  g_bShowLayers = show_flag;
  UpdateLists();
  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::OnLayPropertiesClick(wxCommandEvent &event) {
  // Show layer properties dialog for selected layer - todo
  long item = -1;
  item =
      m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;
}

void RouteManagerDialog::OnLayDeleteClick(wxCommandEvent &event) {
  long item = -1;
  item =
      m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Layer *layer = (Layer *)m_pLayListCtrl->GetItemData(item);

  if (!layer) return;
  // Check if this file is a persistent layer.
  // If added in this session the listctrl file path is origin dir and not yet
  // /layers
  bool ispers = false;
  wxString destf, f, name, ext;
  f = layer->m_LayerFileName;
  wxFileName::SplitPath(f, NULL, NULL, &name, &ext);
  destf = g_Platform->GetPrivateDataDir();
  appendOSDirSlash(&destf);
  destf.Append(_T("layers"));
  appendOSDirSlash(&destf);
  destf << name << _T(".") << ext;

  wxString prompt = _(
      "Are you sure you want to delete this layer and <ALL> of its contents?");
  if (wxFileExists(destf)) {
    prompt.Append(_T("\n"));
    prompt.Append(
        _("The file will also be deleted from OpenCPN's layer directory."));
    prompt.Append(_T("\n (") + destf + _T(")" ));
    ispers = true;
  }
  int answer =
      OCPNMessageBox(this, prompt, wxString(_("OpenCPN Alert")), wxYES_NO);
  if (answer == wxID_NO) return;

  // Delete a persistent layer file if present
  if (ispers) {
    wxString remMSG;
    if (wxRemoveFile(destf))
      remMSG.sprintf(_T("Layer file: %s is deleted"), destf);
    else
      remMSG.sprintf(_T("Error deleting Layer file: %s"), destf);

    wxLogMessage(remMSG);
  }

  // Process Tracks and Routes in this layer
  wxRouteListNode *node1 = pRouteList->GetFirst();
  while (node1) {
    Route *pRoute = node1->GetData();
    wxRouteListNode *next_node = node1->GetNext();
    if (pRoute->m_bIsInLayer && (pRoute->m_LayerID == layer->m_LayerID)) {
      pRoute->m_bIsInLayer = false;
      pRoute->m_LayerID = 0;
      g_pRouteMan->DeleteRoute(pRoute, NavObjectChanges::getInstance());
    }
    node1 = next_node;
  }

  for (Track *pTrack : g_TrackList) {
    if (pTrack->m_bIsInLayer && (pTrack->m_LayerID == layer->m_LayerID)) {
      pTrack->m_bIsInLayer = false;
      pTrack->m_LayerID = 0;
      RoutemanGui(*g_pRouteMan).DeleteTrack(pTrack);
    }
  }

  // Process waypoints in this layer
  wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
  wxRoutePointListNode *node3;

  while (node) {
    node3 = node->GetNext();
    RoutePoint *rp = node->GetData();
    if (rp && (rp->m_LayerID == layer->m_LayerID)) {
      rp->m_bIsInLayer = false;
      rp->m_LayerID = 0;
      pWayPointMan->DestroyWaypoint(
          rp, false);  // no need to update the change set on layer ops
    }

    node = node3;
    node3 = NULL;
  }

  if (g_pMarkInfoDialog) {
    g_pMarkInfoDialog->ClearData();
  }

  pLayerList->DeleteObject(layer);

  UpdateLists();

  gFrame->RefreshAllCanvas();

  m_bNeedConfigFlush = false;
}

void RouteManagerDialog::OnLayToggleChartClick(wxCommandEvent &event) {
  // Toggle  visibility on chart for selected layer
  long item = -1;
  item =
      m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Layer *layer = (Layer *)m_pLayListCtrl->GetItemData(item);

  if (!layer) return;

  layer->SetVisibleOnChart(!layer->IsVisibleOnChart());
  m_pLayListCtrl->SetItemImage(item, layer->IsVisibleOnChart() ? 0 : 1);

  ToggleLayerContentsOnChart(layer);
}

void RouteManagerDialog::ToggleLayerContentsOnChart(Layer *layer) {
  // Process Tracks and Routes in this layer
  wxRouteListNode *node1 = pRouteList->GetFirst();
  while (node1) {
    Route *pRoute = node1->GetData();
    if (pRoute->m_bIsInLayer && (pRoute->m_LayerID == layer->m_LayerID)) {
      pRoute->SetVisible(layer->IsVisibleOnChart());
    }
    node1 = node1->GetNext();
  }

  for (Track *pTrack : g_TrackList) {
    if (pTrack->m_bIsInLayer && (pTrack->m_LayerID == layer->m_LayerID))
      pTrack->SetVisible(layer->IsVisibleOnChart());
  }

  // Process waypoints in this layer
  wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

  while (node) {
    RoutePoint *rp = node->GetData();
    if (rp && (rp->m_LayerID == layer->m_LayerID)) {
      rp->SetVisible(layer->IsVisibleOnChart());
    }

    node = node->GetNext();
  }
  UpdateLists();

  UpdateLayButtons();

  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::OnLayToggleNamesClick(wxCommandEvent &event) {
  // Toggle WPT names visibility on chart for selected layer
  long item = -1;
  item =
      m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Layer *layer = (Layer *)m_pLayListCtrl->GetItemData(item);

  if (!layer) return;

  layer->SetVisibleNames(cbLayToggleNames->Get3StateValue());

  ToggleLayerContentsNames(layer);
}

void RouteManagerDialog::ToggleLayerContentsNames(Layer *layer) {
  // Process Tracks and Routes in this layer
  wxRouteListNode *node1 = pRouteList->GetFirst();
  while (node1) {
    Route *pRoute = node1->GetData();
    if (pRoute->m_bIsInLayer && (pRoute->m_LayerID == layer->m_LayerID)) {
      wxRoutePointListNode *node = pRoute->pRoutePointList->GetFirst();
      RoutePoint *prp1 = node->GetData();
      while (node) {
        if (layer->HasVisibleNames() == wxCHK_UNDETERMINED) {
          prp1->m_bShowName = prp1->m_bShowNameData;
        } else {
          prp1->m_bShowName = (layer->HasVisibleNames() == wxCHK_CHECKED);
        }
        node = node->GetNext();
      }
    }
    node1 = node1->GetNext();
  }

  // Process waypoints in this layer
  wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

  while (node) {
    RoutePoint *rp = node->GetData();
    if (rp && (rp->m_LayerID == layer->m_LayerID)) {
      rp->SetNameShown(layer->HasVisibleNames() == wxCHK_CHECKED ||
                       (rp->m_bShowNameData &&
                        layer->HasVisibleNames() == wxCHK_UNDETERMINED));
    }

    node = node->GetNext();
  }

  UpdateLayButtons();

  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::OnLayToggleListingClick(wxCommandEvent &event) {
  // Toggle  visibility on listing for selected layer
  long item = -1;
  item =
      m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (item == -1) return;

  Layer *layer = (Layer *)m_pLayListCtrl->GetItemData(item);

  if (!layer) return;

  layer->SetVisibleOnListing(!layer->IsVisibleOnListing());

  ToggleLayerContentsOnListing(layer);
}

void RouteManagerDialog::ToggleLayerContentsOnListing(Layer *layer) {
  ::wxBeginBusyCursor();

  // Process Tracks and Routes in this layer
  wxRouteListNode *node1 = pRouteList->GetFirst();
  while (node1) {
    Route *pRoute = node1->GetData();
    if (pRoute->m_bIsInLayer && (pRoute->m_LayerID == layer->m_LayerID)) {
      pRoute->SetListed(layer->IsVisibleOnListing());
    }
    node1 = node1->GetNext();
  }

  for (Track *pTrack : g_TrackList) {
    if (pTrack->m_bIsInLayer && (pTrack->m_LayerID == layer->m_LayerID))
      pTrack->SetListed(layer->IsVisibleOnListing());
  }

  // Process waypoints in this layer
  //  n.b.  If the waypoint belongs to a track, and is not shared, then do not
  //  list it. This is a performance optimization, allowing large track support.

  wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

  while (node) {
    RoutePoint *rp = node->GetData();
    if (rp && rp->m_bIsolatedMark && (rp->m_LayerID == layer->m_LayerID)) {
      rp->SetListed(layer->IsVisibleOnListing());
    }

    node = node->GetNext();
  }

  UpdateLists();

  ::wxEndBusyCursor();

  gFrame->RefreshAllCanvas();
}

void RouteManagerDialog::OnLayDefaultAction(wxListEvent &event) {
  wxCommandEvent evt;
  OnLayPropertiesClick(evt);
}

void RouteManagerDialog::UpdateLayListCtrl() {
  // if an item was selected, make it selected again if it still exist
  long item = -1;
  item =
      m_pLayListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

  wxUIntPtr selected_id = wxUIntPtr(0);
  if (item != -1) selected_id = m_pLayListCtrl->GetItemData(item);

  // Delete existing items
  m_pLayListCtrl->DeleteAllItems();

  // then add routes to the listctrl
  LayerList::iterator it;
  int index = 0;
  bool b_anyHidden = false;
  for (it = (*pLayerList).begin(); it != (*pLayerList).end(); ++it, ++index) {
    Layer *lay = (Layer *)(*it);

    if (!lay->m_LayerName.Upper().Contains(m_tFilterLay->GetValue().Upper())) {
      continue;
    }

    wxListItem li;
    li.SetId(index);
    li.SetImage(lay->IsVisibleOnChart() ? 0 : 1);
    li.SetData(lay);
    li.SetText(_T(""));

    long idx = m_pLayListCtrl->InsertItem(li);

    wxString name = lay->m_LayerName;
    if (name.IsEmpty()) {
      // RoutePoint *rp = trk->GetPoint(1);
      // if (rp)
      //      name = rp->m_CreateTime.FormatISODate() + _T(" ") +
      //      rp->m_CreateTime.FormatISOTime();   //name =
      //      rp->m_CreateTime.Format();
      // else
      name = _("(Unnamed Layer)");
    }
    m_pLayListCtrl->SetItem(idx, colLAYNAME, name);

    wxString len;
    len.Printf(wxT("%d"), (int)lay->m_NoOfItems);
    m_pLayListCtrl->SetItem(idx, colLAYITEMS, len);
    m_pLayListCtrl->SetItem(idx, colLAYPERSIST, lay->m_LayerType);

    wxListItem lic;
    lic.SetId(index);
    lic.SetColumn(1);
    lic.SetAlign(wxLIST_FORMAT_LEFT);
    m_pLayListCtrl->SetItem(lic);

    lic.SetColumn(2);
    lic.SetAlign(wxLIST_FORMAT_LEFT);
    m_pLayListCtrl->SetItem(lic);

    if (!lay->IsVisibleOnChart()) b_anyHidden = true;
  }

  m_pLayListCtrl->SortItems(SortLayersOnName,
                            reinterpret_cast<wxIntPtr>(m_pLayListCtrl));
  m_pLayListCtrl->SetColumnWidth(0, 4 * m_charWidth);

  // restore selection if possible
  // NOTE this will select a different item, if one is deleted
  // (the next route will get that index).
  if (selected_id != wxUIntPtr(0)) {
    item = m_pLayListCtrl->FindItem(-1, selected_id);
    m_pLayListCtrl->SetItemState(item,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
  }
  UpdateLayButtons();

  m_cbShowAllLay->SetValue(!b_anyHidden);
}

void RouteManagerDialog::OnImportClick(wxCommandEvent &event) {
  // Import routes
  // FIXME there is no way to instruct this function about what to import.
  // Suggest to add that!

  UI_ImportGPX(this);

  UpdateLists();

  gFrame->RefreshAllCanvas();
}
void RouteManagerDialog::OnExportClick(wxCommandEvent &event) {
  ExportGPX(this);
}

void RouteManagerDialog::OnExportVizClick(wxCommandEvent &event) {
  ExportGPX(this, true, true);  // only visible objects, layers included
}

void RouteManagerDialog::OnFilterChanged(wxCommandEvent &event) {
  if (event.GetEventObject() == m_tFilterWpt) {
    UpdateWptListCtrl(NULL, true);
  } else if (event.GetEventObject() == m_tFilterRte) {
    UpdateRouteListCtrl();
  } else if (event.GetEventObject() == m_tFilterTrk) {
    UpdateTrkListCtrl();
  } else if (event.GetEventObject() == m_tFilterLay) {
    UpdateLayListCtrl();
  }
}

// END Event handlers
