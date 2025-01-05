///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!  //did that. despite warning.
///////////////////////////////////////////////////////////////////////////

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dataview.h>
#include <wx/datectrl.h>
#include <wx/datetime.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/menuitem.h>
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/timectrl.h>
#include <wx/window.h>

#include "gui_lib.h"
#include "model/nav_object_database.h"
#include "model/ocpn_types.h"
#include "routeman_gui.h"
#include "model/routeman.h"
#include "routemanagerdialog.h"
#include "RoutePropDlg.h"
#include "RoutePropDlgImpl.h"
#include "styles.h"

#if wxCHECK_VERSION(3, 1, 2)
#define CELL_EDITABLE wxDATAVIEW_CELL_EDITABLE
#else
#define CELL_EDITABLE wxDATAVIEW_CELL_INERT
#endif

///////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(RoutePropDlg, DIALOG_PARENT)
#ifdef __OCPN__ANDROID__
EVT_CHAR(RoutePropDlg::OnKeyChar)
#endif
END_EVENT_TABLE()

extern Routeman* g_pRouteMan;
extern RoutePropDlgImpl* pRoutePropDialog;
extern RouteManagerDialog* pRouteManagerDialog;

RoutePropDlgCtx RoutePropDlg::GetDlgCtx() {
  struct RoutePropDlgCtx ctx;
  ctx.set_route_and_update = [&](Route* r) {
    if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
      pRoutePropDialog->SetRouteAndUpdate(r, true);
    }
  };
  ctx.set_enroute_point = [&](Route* r, RoutePoint* rt) {
    if (pRoutePropDialog && pRoutePropDialog->IsShown()) {
      if (pRoutePropDialog->GetRoute() == r) {
        pRoutePropDialog->SetEnroutePoint(rt);
      }
    }
  };
  ctx.hide = [&](Route* r) {
    if (pRoutePropDialog && (pRoutePropDialog->IsShown()) &&
        (r == pRoutePropDialog->GetRoute())) {
      pRoutePropDialog->Hide();
    }
  };
  auto RouteMgrDlgUpdateListCtrl = [&]() {
    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateRouteListCtrl();
  };
  return ctx;
}

RoutePropDlg::RoutePropDlg(wxWindow* parent, wxWindowID id,
                           const wxString& title, const wxPoint& pos,
                           const wxSize& size, long style)
    : DIALOG_PARENT(parent, id, title, pos, size, style) {
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);

  this->SetSizeHints(wxSize(300, 300), wxDefaultSize);

  wxBoxSizer* bSizerMain;
  bSizerMain = new wxBoxSizer(wxVERTICAL);

  m_ntbRteProp =
      new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
  m_pnlBasic = new wxPanel(m_ntbRteProp, wxID_ANY, wxDefaultPosition,
                           wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizerData;
  bSizerData = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizerName;
  bSizerName = new wxBoxSizer(wxHORIZONTAL);

  m_stName = new wxStaticText(m_pnlBasic, wxID_ANY, _("Name"),
                              wxDefaultPosition, wxDefaultSize, 0);
  m_stName->Wrap(-1);
  bSizerName->Add(m_stName, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_tcName = new wxTextCtrl(m_pnlBasic, wxID_ANY, wxEmptyString,
                            wxDefaultPosition, wxDefaultSize, 0);
  bSizerName->Add(m_tcName, 1, wxALL, 5);

  bSizerData->Add(bSizerName, 0, wxEXPAND, 5);

  wxBoxSizer* bSizerFromTo;
  bSizerFromTo = new wxBoxSizer(wxHORIZONTAL);

  m_stFrom = new wxStaticText(m_pnlBasic, wxID_ANY, _("From"),
                              wxDefaultPosition, wxDefaultSize, 0);
  m_stFrom->Wrap(-1);
  bSizerFromTo->Add(m_stFrom, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_tcFrom = new wxTextCtrl(m_pnlBasic, wxID_ANY, wxEmptyString,
                            wxDefaultPosition, wxDefaultSize, 0);
  bSizerFromTo->Add(m_tcFrom, 1, wxALL, 5);

  m_stTo = new wxStaticText(m_pnlBasic, wxID_ANY, _("To"), wxDefaultPosition,
                            wxDefaultSize, 0);
  m_stTo->Wrap(-1);
  bSizerFromTo->Add(m_stTo, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_tcTo = new wxTextCtrl(m_pnlBasic, wxID_ANY, wxEmptyString,
                          wxDefaultPosition, wxDefaultSize, 0);
  bSizerFromTo->Add(m_tcTo, 1, wxALL, 5);

  bSizerData->Add(bSizerFromTo, 0, wxEXPAND, 5);

  wxWrapSizer* wSizerParams;
  wSizerParams = new wxWrapSizer(wxHORIZONTAL, wxEXTEND_LAST_ON_EACH_LINE |
                                                   wxREMOVE_LEADING_SPACES |
                                                   wxWRAPSIZER_DEFAULT_FLAGS);

  int maxFieldSize = GetCharWidth() * 10;

  wxBoxSizer* bSizerDistance;
  bSizerDistance = new wxBoxSizer(wxVERTICAL);

  m_stDistTotal = new wxStaticText(m_pnlBasic, wxID_ANY, _("Distance"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stDistTotal->Wrap(-1);
  bSizerDistance->Add(m_stDistTotal, 0, wxALL, 5);

  m_tcDistance =
      new wxTextCtrl(m_pnlBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_READONLY);
  m_tcDistance->SetMaxSize(wxSize(maxFieldSize, -1));
  m_tcDistance->SetMinSize(wxSize(maxFieldSize, -1));

  bSizerDistance->Add(m_tcDistance, 0, wxALL | wxEXPAND, 5);

  wSizerParams->Add(bSizerDistance, 1, wxEXPAND, 0);

  wxBoxSizer* bSizerSpeed;
  bSizerSpeed = new wxBoxSizer(wxVERTICAL);

  m_stPlanSpeed = new wxStaticText(m_pnlBasic, wxID_ANY, _("Plan speed"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stPlanSpeed->Wrap(-1);
  bSizerSpeed->Add(m_stPlanSpeed, 0, wxALL, 5);

  m_tcPlanSpeed =
      new wxTextCtrl(m_pnlBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_PROCESS_ENTER);
  m_tcPlanSpeed->SetMaxSize(wxSize(maxFieldSize, -1));
  m_tcPlanSpeed->SetMinSize(wxSize(maxFieldSize, -1));

  bSizerSpeed->Add(m_tcPlanSpeed, 0, wxALL, 5);

  wSizerParams->Add(bSizerSpeed, 1, wxEXPAND, 0);

  //
  wxBoxSizer* bSizerEnroute;
  bSizerEnroute = new wxBoxSizer(wxVERTICAL);

  m_stEnroute = new wxStaticText(m_pnlBasic, wxID_ANY, _("Enroute"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  m_stEnroute->Wrap(-1);
  bSizerEnroute->Add(m_stEnroute, 0, wxALL, 5);

  m_tcEnroute = new wxTextCtrl(m_pnlBasic, wxID_ANY, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
  m_tcEnroute->SetMaxSize(wxSize(maxFieldSize, -1));
  m_tcEnroute->SetMinSize(wxSize(maxFieldSize, -1));

  bSizerEnroute->Add(m_tcEnroute, 0, wxALL, 5);

  wSizerParams->Add(bSizerEnroute, 1, wxEXPAND, 0);

  //
  wxBoxSizer* bSizerDeparture;
  bSizerDeparture = new wxBoxSizer(wxVERTICAL);

  m_stDeparture = new wxStaticText(m_pnlBasic, wxID_ANY, _("Departure"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  m_stDeparture->Wrap(-1);
  bSizerDeparture->Add(m_stDeparture, 0, wxALL, 5);

  wxBoxSizer* bSizerDepartureTS;
  bSizerDepartureTS = new wxBoxSizer(wxHORIZONTAL);
  bSizerDeparture->Add(bSizerDepartureTS, 0, 0, 5);

  m_dpDepartureDate =
      new wxDatePickerCtrl(m_pnlBasic, wxID_ANY, wxDefaultDateTime,
                           wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT);
  bSizerDepartureTS->Add(m_dpDepartureDate, 0, wxALIGN_CENTER_VERTICAL | wxALL,
                         5);
#ifdef __ANDROID__
  int ys = GetCharHeight();
  m_dpDepartureDate->SetMinSize(wxSize(-1, ys * 15 / 10));
#endif

#ifdef __WXGTK__
  m_tpDepartureTime = new TimeCtrl(m_pnlBasic, wxID_ANY, wxDefaultDateTime,
                                   wxDefaultPosition, wxDefaultSize);
#else
  m_tpDepartureTime =
      new wxTimePickerCtrl(m_pnlBasic, wxID_ANY, wxDefaultDateTime,
                           wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT);
#endif
  bSizerDepartureTS->Add(m_tpDepartureTime, 0, wxALIGN_CENTER_VERTICAL | wxALL,
                         5);

  wSizerParams->Add(bSizerDeparture, 1, wxEXPAND, 0);

  wxBoxSizer* bSizerTime;
  bSizerTime = new wxBoxSizer(wxVERTICAL);

  m_stTimeZone = new wxStaticText(m_pnlBasic, wxID_ANY, _("Time"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  m_stTimeZone->Wrap(-1);
  bSizerTime->Add(m_stTimeZone, 0, wxALL, 5);

  wxString m_choiceTimezoneChoices[] = {_("UTC"), _("Local@PC"),
                                        _("LMT@Location")};
  int m_choiceTimezoneNChoices =
      sizeof(m_choiceTimezoneChoices) / sizeof(wxString);
  m_choiceTimezone =
      new wxChoice(m_pnlBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceTimezoneNChoices, m_choiceTimezoneChoices, 0);
  m_choiceTimezone->SetSelection(0);
  m_choiceTimezone->SetMaxSize(wxSize(GetCharWidth() * 12, -1));

  bSizerTime->Add(m_choiceTimezone, 0, wxALL, 5);

  wSizerParams->Add(bSizerTime, 1, wxEXPAND, 0);

  wxBoxSizer* bSizerColor;
  bSizerColor = new wxBoxSizer(wxVERTICAL);

  m_staticTextColor = new wxStaticText(m_pnlBasic, wxID_ANY, _("Color"),
                                       wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextColor->Wrap(-1);
  bSizerColor->Add(m_staticTextColor, 0, wxALL, 5);

  wxString m_choiceColorChoices[] = {
      _("Default color"), _("Black"),     _("Dark Red"),     _("Dark Green"),
      _("Dark Yellow"),   _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
      _("Light Gray"),    _("Dark Gray"), _("Red"),          _("Green"),
      _("Yellow"),        _("Blue"),      _("Magenta"),      _("Cyan"),
      _("White")};
  int m_choiceColorNChoices = sizeof(m_choiceColorChoices) / sizeof(wxString);
  m_choiceColor =
      new wxChoice(m_pnlBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceColorNChoices, m_choiceColorChoices, 0);
  m_choiceColor->SetSelection(0);
  bSizerColor->Add(m_choiceColor, 0, wxALL, 5);

  wSizerParams->Add(bSizerColor, 1, wxEXPAND, 5);

  wxBoxSizer* bSizerStyle;
  bSizerStyle = new wxBoxSizer(wxVERTICAL);

  m_stStyle = new wxStaticText(m_pnlBasic, wxID_ANY, _("Style"),
                               wxDefaultPosition, wxDefaultSize, 0);
  m_stStyle->Wrap(-1);
  bSizerStyle->Add(m_stStyle, 0, wxALL, 5);

  wxString m_choiceStyleChoices[] = {_("Default"), _("Solid"), _("Dot"),
                                     _("Long dash"), _("Short dash")};
  int m_choiceStyleNChoices = sizeof(m_choiceStyleChoices) / sizeof(wxString);
  m_choiceStyle =
      new wxChoice(m_pnlBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceStyleNChoices, m_choiceStyleChoices, 0);
  m_choiceStyle->SetSelection(0);
  bSizerStyle->Add(m_choiceStyle, 0, wxALL, 5);

  wSizerParams->Add(bSizerStyle, 1, wxEXPAND, 5);

  wxBoxSizer* bSizerWidth;
  bSizerWidth = new wxBoxSizer(wxVERTICAL);

  m_stWidth = new wxStaticText(m_pnlBasic, wxID_ANY, _("Width"),
                               wxDefaultPosition, wxDefaultSize, 0);
  m_stWidth->Wrap(-1);
  bSizerWidth->Add(m_stWidth, 0, wxALL, 5);

  wxString m_choiceWidthChoices[] = {
      _("Default"), _("1 px"), _("2px"),  _("3 px"), _("4 px"), _("5 px"),
      _("6 px"),    _("7 px"), _("8 px"), _("9 px"), _("10 px")};
  int m_choiceWidthNChoices = sizeof(m_choiceWidthChoices) / sizeof(wxString);
  m_choiceWidth =
      new wxChoice(m_pnlBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                   m_choiceWidthNChoices, m_choiceWidthChoices, 0);
  m_choiceWidth->SetSelection(0);
  bSizerWidth->Add(m_choiceWidth, 0, wxALL, 5);

  wSizerParams->Add(bSizerWidth, 1, wxEXPAND, 5);

  // wxDataViewListCtrl has some platform dependent variability
  // We account for this here...

  wxString toLabel = _("To waypoint");

#ifdef __WXQT__  // includes Android
  int columWidths[] = {
      wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE,
      wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE,
      wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE,
      wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE,
      wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE, wxCOL_WIDTH_AUTOSIZE};
  int colFlags = 0;
  toLabel = _("To WP");

#else
  int columWidths[] = {30,  80, 70,
                       60,   // Bearing
                       100,  // Distance Total
                       90,  90, 80,  120,
                       60,   // Speed
                       100,  // Next tide event
                       -1,  80, 120, -1};
  int colFlags = wxDATAVIEW_COL_RESIZABLE;

#endif

  bSizerData->Add(wSizerParams, 0, wxEXPAND, 0);

  m_dvlcWaypoints =
      new wxDataViewListCtrl(m_pnlBasic, wxID_ANY, wxDefaultPosition,
                             wxDefaultSize, wxDV_VERT_RULES | wxDV_HORIZ_RULES);
  m_dataViewListColumnLeg = m_dvlcWaypoints->AppendTextColumn(
      _("Leg"), wxDATAVIEW_CELL_INERT, columWidths[0],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnLeg->GetRenderer()->EnableEllipsize(wxELLIPSIZE_NONE);
  m_dataViewListColumnToWpt = m_dvlcWaypoints->AppendTextColumn(
      toLabel, wxDATAVIEW_CELL_INERT, columWidths[1],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnToWpt->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnDistance = m_dvlcWaypoints->AppendTextColumn(
      _("Distance"), wxDATAVIEW_CELL_INERT, columWidths[2],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnDistance->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnBearing = m_dvlcWaypoints->AppendTextColumn(
      _("Bearing"), wxDATAVIEW_CELL_INERT, columWidths[3],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnBearing->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnTotalDistance = m_dvlcWaypoints->AppendTextColumn(
      _("Total Distance"), wxDATAVIEW_CELL_INERT, columWidths[4],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnTotalDistance->GetRenderer()->EnableEllipsize(
      wxELLIPSIZE_END);
  m_dataViewListColumnLat = m_dvlcWaypoints->AppendTextColumn(
      _("Latitude"), wxDATAVIEW_CELL_INERT, columWidths[5],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnLat->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnLon = m_dvlcWaypoints->AppendTextColumn(
      _("Longitude"), wxDATAVIEW_CELL_INERT, columWidths[6],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnLon->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnETE = m_dvlcWaypoints->AppendTextColumn(
      _("ETE"), wxDATAVIEW_CELL_INERT, columWidths[7],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnETE->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnETA = m_dvlcWaypoints->AppendTextColumn(
      _("ETA"), wxDATAVIEW_CELL_INERT, columWidths[8],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnETA->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnSpeed = m_dvlcWaypoints->AppendTextColumn(
      _("Speed"), CELL_EDITABLE, columWidths[9],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnSpeed->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnNTE = m_dvlcWaypoints->AppendTextColumn(
      _("Next tide event"), wxDATAVIEW_CELL_INERT, columWidths[10],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnNTE->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnDesc = m_dvlcWaypoints->AppendTextColumn(
      _("Description"), wxDATAVIEW_CELL_INERT, columWidths[11],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnDesc->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnCourse = m_dvlcWaypoints->AppendTextColumn(
      _("Course"), wxDATAVIEW_CELL_INERT, columWidths[12],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnCourse->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnETD = m_dvlcWaypoints->AppendTextColumn(
      _("ETD"), CELL_EDITABLE, columWidths[13],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  m_dataViewListColumnETD->GetRenderer()->EnableEllipsize(wxELLIPSIZE_END);
  m_dataViewListColumnEmpty = m_dvlcWaypoints->AppendTextColumn(
      wxEmptyString, wxDATAVIEW_CELL_INERT, columWidths[14],
      static_cast<wxAlignment>(wxALIGN_LEFT), colFlags);
  bSizerData->Add(m_dvlcWaypoints, 1, wxALL | wxEXPAND, 5);

  m_pnlBasic->SetSizer(bSizerData);
  m_pnlBasic->Layout();
  bSizerData->Fit(m_pnlBasic);
  m_ntbRteProp->AddPage(m_pnlBasic, _("Basic"), false);
  m_pnlAdvanced = new wxPanel(m_ntbRteProp, wxID_ANY, wxDefaultPosition,
                              wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizerAdvanced;
  bSizerAdvanced = new wxBoxSizer(wxVERTICAL);

  m_stDescription = new wxStaticText(m_pnlAdvanced, wxID_ANY, _("Description"),
                                     wxDefaultPosition, wxDefaultSize, 0);
  m_stDescription->Wrap(-1);
  bSizerAdvanced->Add(m_stDescription, 0, wxALL, 5);

  m_tcDescription =
      new wxTextCtrl(m_pnlAdvanced, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_MULTILINE);
  bSizerAdvanced->Add(m_tcDescription, 1, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer* sbSizerLinks;
  sbSizerLinks = new wxStaticBoxSizer(
      new wxStaticBox(m_pnlAdvanced, wxID_ANY, _("Links")), wxVERTICAL);

  m_scrolledWindowLinks = new wxScrolledWindow(
      sbSizerLinks->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxHSCROLL | wxVSCROLL);
  m_scrolledWindowLinks->SetScrollRate(5, 5);
  wxBoxSizer* bSizerLinksWnd;
  bSizerLinksWnd = new wxBoxSizer(wxVERTICAL);

  bSizerLinks = new wxBoxSizer(wxVERTICAL);

  m_hyperlink1 =
      new wxHyperlinkCtrl(m_scrolledWindowLinks, wxID_ANY, _("wxFB Website"),
                          wxT("http://www.wxformbuilder.org"),
                          wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
  m_hyperlink1->Enable(false);
  m_hyperlink1->Hide();

  m_menuLink = new wxMenu();
  wxMenuItem* m_menuItemEdit;
  m_menuItemEdit = new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Edit")),
                                  wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemEdit);

  wxMenuItem* m_menuItemAdd;
  m_menuItemAdd = new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Add new")),
                                 wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemAdd);

  wxMenuItem* m_menuItemDelete;
  m_menuItemDelete = new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Delete")),
                                    wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemDelete);

  m_hyperlink1->Connect(
      wxEVT_RIGHT_DOWN,
      wxMouseEventHandler(RoutePropDlg::m_hyperlink1OnContextMenu), NULL, this);

  bSizerLinks->Add(m_hyperlink1, 0, wxALL, 5);

  bSizerLinksWnd->Add(bSizerLinks, 1, wxEXPAND, 5);

  wxBoxSizer* bSizerLinkBtns;
  bSizerLinkBtns = new wxBoxSizer(wxHORIZONTAL);

  m_btnAddLink = new wxButton(m_scrolledWindowLinks, wxID_ANY, _("Add"),
                              wxDefaultPosition, wxDefaultSize, 0);
  bSizerLinkBtns->Add(m_btnAddLink, 0, wxALL, 5);

  m_toggleBtnEdit =
      new wxToggleButton(m_scrolledWindowLinks, wxID_ANY, _("Edit"),
                         wxDefaultPosition, wxDefaultSize, 0);
  bSizerLinkBtns->Add(m_toggleBtnEdit, 0, wxALL, 5);

  m_stEditEnabled =
      new wxStaticText(m_scrolledWindowLinks, wxID_ANY,
                       _("Links are opened in the default browser."),
                       wxDefaultPosition, wxDefaultSize, 0);
  m_stEditEnabled->Wrap(-1);
  bSizerLinkBtns->Add(m_stEditEnabled, 0, wxALL, 5);

  bSizerLinksWnd->Add(bSizerLinkBtns, 0, wxEXPAND, 5);

  m_scrolledWindowLinks->SetSizer(bSizerLinksWnd);
  m_scrolledWindowLinks->Layout();
  bSizerLinksWnd->Fit(m_scrolledWindowLinks);
  m_menuLinks = new wxMenu();
  wxMenuItem* m_menuItemAddLink;
  m_menuItemAddLink =
      new wxMenuItem(m_menuLinks, wxID_ANY, wxString(_("Add new")),
                     wxEmptyString, wxITEM_NORMAL);
  m_menuLinks->Append(m_menuItemAddLink);

  m_scrolledWindowLinks->Connect(
      wxEVT_RIGHT_DOWN,
      wxMouseEventHandler(RoutePropDlg::m_scrolledWindowLinksOnContextMenu),
      NULL, this);

  sbSizerLinks->Add(m_scrolledWindowLinks, 1, wxEXPAND | wxALL, 5);

  bSizerAdvanced->Add(sbSizerLinks, 1, wxEXPAND, 5);

  m_pnlAdvanced->SetSizer(bSizerAdvanced);
  m_pnlAdvanced->Layout();
  bSizerAdvanced->Fit(m_pnlAdvanced);
  m_ntbRteProp->AddPage(m_pnlAdvanced, _("Advanced"), false);

  bSizerMain->Add(m_ntbRteProp, 1, wxEXPAND | wxALL, 5);

  wxWrapSizer* wSizerCustomBtns;
  wSizerCustomBtns = new wxWrapSizer(wxHORIZONTAL, wxWRAPSIZER_DEFAULT_FLAGS);

  m_btnPrint = new wxButton(this, wxID_ANY, _("Print"), wxDefaultPosition,
                            wxDefaultSize, 0);
  wSizerCustomBtns->Add(m_btnPrint, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

#ifdef __OCPN__ANDROID__
  m_btnPrint->Hide();
#endif

  m_btnExtend = new wxButton(this, wxID_ANY, _("Extend"), wxDefaultPosition,
                             wxDefaultSize, 0);
  wSizerCustomBtns->Add(m_btnExtend, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_btnSplit = new wxButton(this, wxID_ANY, _("Split"), wxDefaultPosition,
                            wxDefaultSize, 0);
  wSizerCustomBtns->Add(m_btnSplit, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wSizerCustomBtns->Add(0, 0, 1, wxEXPAND, 5);

  m_sdbSizerBtns = new wxStdDialogButtonSizer();
  m_sdbSizerBtnsOK = new wxButton(this, wxID_OK);
  m_sdbSizerBtns->AddButton(m_sdbSizerBtnsOK);
  m_sdbSizerBtnsCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
  m_sdbSizerBtns->AddButton(m_sdbSizerBtnsCancel);
  m_sdbSizerBtns->Realize();

  wSizerCustomBtns->Add(m_sdbSizerBtns, 1, wxEXPAND, 5);

  bSizerMain->Add(wSizerCustomBtns, 0, wxEXPAND, 0);

  this->SetSizer(bSizerMain);
  this->Layout();

  this->Centre(wxBOTH);

  // Connect Events
  this->Connect(wxEVT_CLOSE_WINDOW,
                wxCloseEventHandler(RoutePropDlg::RoutePropDlgOnClose));
  this->Connect(wxEVT_SIZE,
                wxSizeEventHandler(RoutePropDlg::RoutePropDlgOnSize));
  m_ntbRteProp->Connect(
      wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
      wxNotebookEventHandler(RoutePropDlg::RoutePropDlgOnNotebookPageChanged),
      NULL, this);
  m_tcPlanSpeed->Connect(
      wxEVT_KILL_FOCUS, wxFocusEventHandler(RoutePropDlg::PlanSpeedOnKillFocus),
      NULL, this);
  m_tcPlanSpeed->Connect(
      wxEVT_COMMAND_TEXT_ENTER,
      wxCommandEventHandler(RoutePropDlg::PlanSpeedOnTextEnter), NULL, this);
  m_tcPlanSpeed->Connect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(RoutePropDlg::PlanSpeedOnTextEnter), NULL, this);
  m_dpDepartureDate->Connect(
      wxEVT_DATE_CHANGED,
      wxDateEventHandler(RoutePropDlg::DepartureDateOnDateChanged), NULL, this);
  m_tpDepartureTime->Connect(
      wxEVT_TIME_CHANGED,
      wxDateEventHandler(RoutePropDlg::DepartureTimeOnTimeChanged), NULL, this);
  m_choiceTimezone->Connect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(RoutePropDlg::TimezoneOnChoice), NULL, this);
  m_dvlcWaypoints->Connect(
      wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU,
      wxDataViewEventHandler(
          RoutePropDlg::WaypointsOnDataViewListCtrlItemContextMenu),
      NULL, this);
  m_dvlcWaypoints->Connect(
      wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_DONE,
      wxDataViewEventHandler(
          RoutePropDlg::WaypointsOnDataViewListCtrlItemEditingDone),
      NULL, this);
  m_dvlcWaypoints->Connect(
      wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED,
      wxDataViewEventHandler(
          RoutePropDlg::WaypointsOnDataViewListCtrlItemValueChanged),
      NULL, this);
  m_dvlcWaypoints->Connect(
      wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED,
      wxDataViewEventHandler(
          RoutePropDlg::WaypointsOnDataViewListCtrlSelectionChanged),
      NULL, this);
  m_menuLink->Bind(wxEVT_COMMAND_MENU_SELECTED,
                   wxCommandEventHandler(RoutePropDlg::ItemEditOnMenuSelection),
                   this, m_menuItemEdit->GetId());
  m_menuLink->Bind(wxEVT_COMMAND_MENU_SELECTED,
                   wxCommandEventHandler(RoutePropDlg::ItemAddOnMenuSelection),
                   this, m_menuItemAdd->GetId());
  m_menuLink->Bind(
      wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(RoutePropDlg::ItemDeleteOnMenuSelection), this,
      m_menuItemDelete->GetId());
  m_btnAddLink->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::AddLinkOnButtonClick), NULL, this);
  m_toggleBtnEdit->Connect(
      wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::BtnEditOnToggleButton), NULL, this);
  m_menuLinks->Bind(wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler(RoutePropDlg::ItemAddOnMenuSelection),
                    this, m_menuItemAddLink->GetId());
  m_btnPrint->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                      wxCommandEventHandler(RoutePropDlg::PrintOnButtonClick),
                      NULL, this);
  m_btnExtend->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                       wxCommandEventHandler(RoutePropDlg::ExtendOnButtonClick),
                       NULL, this);
  m_btnSplit->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                      wxCommandEventHandler(RoutePropDlg::SplitOnButtonClick),
                      NULL, this);
  m_sdbSizerBtnsCancel->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::BtnsOnCancelButtonClick), NULL, this);
  m_sdbSizerBtnsOK->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::BtnsOnOKButtonClick), NULL, this);

  auto navobj = NavObjectChanges::getInstance();
  wxDEFINE_EVENT(EVT_ROUTEMAN_DEL_TRK, ObservedEvt);
  navobj_del_track_listener.Listen(navobj->evt_delete_track, this,
                                   EVT_ROUTEMAN_DEL_TRK);
  Bind(EVT_ROUTEMAN_DEL_TRK, [&](ObservedEvt& ev) {
    auto t = std::const_pointer_cast<Track>(UnpackEvtPointer<Track>(ev));
    RoutemanGui(*g_pRouteMan).DeleteTrack(t.get());
  });

  wxDEFINE_EVENT(EVT_ROUTEMAN_DEL_ROUTE, ObservedEvt);
  navobj_del_route_listener.Listen(navobj->evt_delete_route, this,
                                   EVT_ROUTEMAN_DEL_ROUTE);
  Bind(EVT_ROUTEMAN_DEL_ROUTE, [&](ObservedEvt& ev) {
    auto r = std::const_pointer_cast<Route>(UnpackEvtPointer<Route>(ev));
    g_pRouteMan->DeleteRoute(r.get(), navobj);
  });
}

RoutePropDlg::~RoutePropDlg() {
  // Disconnect Events
  this->Disconnect(wxEVT_CLOSE_WINDOW,
                   wxCloseEventHandler(RoutePropDlg::RoutePropDlgOnClose));
  this->Disconnect(wxEVT_SIZE,
                   wxSizeEventHandler(RoutePropDlg::RoutePropDlgOnSize));
  m_ntbRteProp->Disconnect(
      wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
      wxNotebookEventHandler(RoutePropDlg::RoutePropDlgOnNotebookPageChanged),
      NULL, this);
  m_tcPlanSpeed->Disconnect(
      wxEVT_KILL_FOCUS, wxFocusEventHandler(RoutePropDlg::PlanSpeedOnKillFocus),
      NULL, this);
  m_tcPlanSpeed->Disconnect(
      wxEVT_COMMAND_TEXT_ENTER,
      wxCommandEventHandler(RoutePropDlg::PlanSpeedOnTextEnter), NULL, this);
  m_tcPlanSpeed->Disconnect(
      wxEVT_COMMAND_TEXT_UPDATED,
      wxCommandEventHandler(RoutePropDlg::PlanSpeedOnTextEnter), NULL, this);
  m_dpDepartureDate->Disconnect(
      wxEVT_DATE_CHANGED,
      wxDateEventHandler(RoutePropDlg::DepartureDateOnDateChanged), NULL, this);
  m_tpDepartureTime->Disconnect(
      wxEVT_TIME_CHANGED,
      wxDateEventHandler(RoutePropDlg::DepartureTimeOnTimeChanged), NULL, this);
  m_choiceTimezone->Disconnect(
      wxEVT_COMMAND_CHOICE_SELECTED,
      wxCommandEventHandler(RoutePropDlg::TimezoneOnChoice), NULL, this);
  m_dvlcWaypoints->Disconnect(
      wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU,
      wxDataViewEventHandler(
          RoutePropDlg::WaypointsOnDataViewListCtrlItemContextMenu),
      NULL, this);
  m_dvlcWaypoints->Disconnect(
      wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_DONE,
      wxDataViewEventHandler(
          RoutePropDlg::WaypointsOnDataViewListCtrlItemEditingDone),
      NULL, this);
  m_dvlcWaypoints->Disconnect(
      wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED,
      wxDataViewEventHandler(
          RoutePropDlg::WaypointsOnDataViewListCtrlItemValueChanged),
      NULL, this);
  m_dvlcWaypoints->Disconnect(
      wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED,
      wxDataViewEventHandler(
          RoutePropDlg::WaypointsOnDataViewListCtrlSelectionChanged),
      NULL, this);
  m_btnAddLink->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::AddLinkOnButtonClick), NULL, this);
  m_toggleBtnEdit->Disconnect(
      wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::BtnEditOnToggleButton), NULL, this);
  m_btnPrint->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::PrintOnButtonClick), NULL, this);
  m_btnExtend->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::ExtendOnButtonClick), NULL, this);
  m_btnSplit->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::SplitOnButtonClick), NULL, this);
  m_sdbSizerBtnsCancel->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::BtnsOnCancelButtonClick), NULL, this);
  m_sdbSizerBtnsOK->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(RoutePropDlg::BtnsOnOKButtonClick), NULL, this);

  delete m_menuLink;
  delete m_menuLinks;
}

void RoutePropDlg::OnKeyChar(wxKeyEvent& event) {
  int key_char = event.GetKeyCode();
  int yyp = 4;
}
