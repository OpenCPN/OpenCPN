/**************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  MarkProperties Support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
#include "config.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/datetime.h>
#include <wx/clipbrd.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/stattext.h>
#include <wx/clrpicker.h>
#include <wx/bmpbuttn.h>

#include "chcanv.h"
#include "gui_lib.h"
#include "MarkInfo.h"
#include "model/georef.h"
#include "model/navutil_base.h"
#include "model/own_ship.h"
#include "model/position_parser.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"
#include "navutil.h"  // for Route
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "pluginmanager.h"
#include "routemanagerdialog.h"
#include "routeprintout.h"
#include "RoutePropDlgImpl.h"
#include "styles.h"
#include "svg_utils.h"
#include "TCWin.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#include <QtWidgets/QScroller>
#endif

extern TCMgr* ptcmgr;
extern MyConfig* pConfig;
extern Routeman* g_pRouteMan;
extern RouteManagerDialog* pRouteManagerDialog;
extern RoutePropDlgImpl* pRoutePropDialog;
extern ocpnStyle::StyleManager* g_StyleManager;

extern MyFrame* gFrame;
extern OCPNPlatform* g_Platform;
extern wxString g_default_wp_icon;

// Global print data, to remember settings during the session

// Global page setup data

extern float g_MarkScaleFactorExp;

extern MarkInfoDlg* g_pMarkInfoDialog;

WX_DECLARE_LIST(wxBitmap, BitmapList);
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(BitmapList);

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfBitmaps);

#define EXTENDED_PROP_PAGE 2  // Index of the extended properties page

OCPNIconCombo::OCPNIconCombo(wxWindow* parent, wxWindowID id,
                             const wxString& value, const wxPoint& pos,
                             const wxSize& size, int n,
                             const wxString choices[], long style,
                             const wxValidator& validator, const wxString& name)
    : wxOwnerDrawnComboBox(parent, id, value, pos, size, n, choices, style,
                           validator, name) {
  double fontHeight =
      GetFont().GetPointSize() / g_Platform->getFontPointsperPixel();
  itemHeight = (int)wxRound(fontHeight);
}

OCPNIconCombo::~OCPNIconCombo() {}

void OCPNIconCombo::OnDrawItem(wxDC& dc, const wxRect& rect, int item,
                               int flags) const {
  int offset_x = bmpArray[item].GetWidth();
  int bmpHeight = bmpArray[item].GetHeight();
  dc.DrawBitmap(bmpArray[item], rect.x, rect.y + (rect.height - bmpHeight) / 2,
                true);

  if (flags & wxODCB_PAINTING_CONTROL) {
    wxString text = GetValue();
    int margin_x = 2;

#if wxCHECK_VERSION(2, 9, 0)
    if (ShouldUseHintText()) {
      text = GetHint();
      wxColour col = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
      dc.SetTextForeground(col);
    }

    margin_x = GetMargins().x;
#endif

    dc.DrawText(text, rect.x + margin_x + offset_x,
                (rect.height - dc.GetCharHeight()) / 2 + rect.y);
  } else {
    dc.DrawText(GetVListBoxComboPopup()->GetString(item), rect.x + 2 + offset_x,
                (rect.height - dc.GetCharHeight()) / 2 + rect.y);
  }
}

wxCoord OCPNIconCombo::OnMeasureItem(size_t item) const {
  int bmpHeight = bmpArray[item].GetHeight();

  return wxMax(itemHeight, bmpHeight);
}

wxCoord OCPNIconCombo::OnMeasureItemWidth(size_t item) const { return -1; }

int OCPNIconCombo::Append(const wxString& item, wxBitmap bmp) {
  bmpArray.Add(bmp);
  int idx = wxOwnerDrawnComboBox::Append(item);

  return idx;
}

void OCPNIconCombo::Clear(void) {
  wxOwnerDrawnComboBox::Clear();
  bmpArray.Clear();
}

//-------------------------------------------------------------------------------
//
//    Mark Properties Dialog Implementation
//
//-------------------------------------------------------------------------------
/*!
 * MarkProp type definition
 */

// DEFINE_EVENT_TYPE(EVT_LLCHANGE)           // events from LatLonTextCtrl
const wxEventType EVT_LLCHANGE = wxNewEventType();
//------------------------------------------------------------------------------
//    LatLonTextCtrl Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(LatLonTextCtrl, wxWindow)
END_EVENT_TABLE()

// constructor
LatLonTextCtrl::LatLonTextCtrl(wxWindow* parent, wxWindowID id,
                               const wxString& value, const wxPoint& pos,
                               const wxSize& size, long style,
                               const wxValidator& validator,
                               const wxString& name)
    : wxTextCtrl(parent, id, value, pos, size, style, validator, name) {
  m_pParentEventHandler = parent->GetEventHandler();
}

void LatLonTextCtrl::OnKillFocus(wxFocusEvent& event) {
  //    Send an event to the Parent Dialog
  wxCommandEvent up_event(EVT_LLCHANGE, GetId());
  up_event.SetEventObject((wxObject*)this);
  m_pParentEventHandler->AddPendingEvent(up_event);
}

//-------------------------------------------------------------------------------
//
//    Mark Information Dialog Implementation
//
//-------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(MarkInfoDlg, DIALOG_PARENT)
EVT_BUTTON(wxID_OK, MarkInfoDlg::OnMarkInfoOKClick)
EVT_BUTTON(wxID_CANCEL, MarkInfoDlg::OnMarkInfoCancelClick)
EVT_BUTTON(ID_BTN_DESC_BASIC, MarkInfoDlg::OnExtDescriptionClick)
EVT_BUTTON(ID_DEFAULT, MarkInfoDlg::DefautlBtnClicked)
EVT_BUTTON(ID_BTN_SHOW_TIDES, MarkInfoDlg::ShowTidesBtnClicked)
EVT_COMBOBOX(ID_BITMAPCOMBOCTRL, MarkInfoDlg::OnBitmapCombClick)
EVT_CHECKBOX(ID_SHOWNAMECHECKBOXBASIC,
             MarkInfoDlg::OnShowWaypointNameSelectBasic)
EVT_CHECKBOX(ID_SHOWNAMECHECKBOX_EXT, MarkInfoDlg::OnShowWaypointNameSelectExt)
EVT_CHECKBOX(ID_CHECKBOX_SCAMIN_VIS, MarkInfoDlg::OnSelectScaMinExt)
EVT_TEXT(ID_DESCR_CTR_DESC, MarkInfoDlg::OnDescChangedExt)
EVT_TEXT(ID_DESCR_CTR_BASIC, MarkInfoDlg::OnDescChangedBasic)
EVT_TEXT(ID_LATCTRL, MarkInfoDlg::OnPositionCtlUpdated)
EVT_TEXT(ID_LONCTRL, MarkInfoDlg::OnPositionCtlUpdated)
EVT_CHOICE(ID_WPT_RANGERINGS_NO, MarkInfoDlg::OnWptRangeRingsNoChange)
// the HTML listbox's events
EVT_HTML_LINK_CLICKED(wxID_ANY, MarkInfoDlg::OnHtmlLinkClicked)
EVT_CLOSE(MarkInfoDlg::OnClose)

// EVT_CHOICE( ID_WAYPOINTRANGERINGS, MarkInfoDef::OnWaypointRangeRingSelect )
END_EVENT_TABLE()

MarkInfoDlg::MarkInfoDlg(wxWindow* parent, wxWindowID id, const wxString& title,
                         const wxPoint& pos, const wxSize& size, long style) {
  DIALOG_PARENT::Create(parent, id, title, pos, size, style);

  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);
  int metric = GetCharHeight();

#ifdef __ANDROID__
  //  Set Dialog Font by custom crafted Qt Stylesheet.
  wxString wqs = getFontQtStylesheet(qFont);
  wxCharBuffer sbuf = wqs.ToUTF8();
  QString qsb = QString(sbuf.data());
  QString qsbq = getQtStyleSheet();              // basic scrollbars, etc
  this->GetHandle()->setStyleSheet(qsb + qsbq);  // Concatenated style sheets
  wxScreenDC sdc;
  if (sdc.IsOk()) sdc.GetTextExtent(_T("W"), NULL, &metric, NULL, NULL, qFont);
#endif
  Create();
  m_pMyLinkList = NULL;
  SetColorScheme((ColorScheme)0);
  m_pRoutePoint = NULL;
  m_SaveDefaultDlg = NULL;
  CenterOnScreen();

#ifdef __WXOSX__
  Connect(wxEVT_ACTIVATE, wxActivateEventHandler(MarkInfoDlg::OnActivate), NULL,
          this);
#endif
}

void MarkInfoDlg::OnActivate(wxActivateEvent& event) {
  auto pWin = dynamic_cast<DIALOG_PARENT*>(event.GetEventObject());
  long int style = pWin->GetWindowStyle();
  if (event.GetActive())
    pWin->SetWindowStyle(style | wxSTAY_ON_TOP);
  else
    pWin->SetWindowStyle(style ^ wxSTAY_ON_TOP);
}

void MarkInfoDlg::initialize_images(void) {
  wxString iconDir = g_Platform->GetSharedDataDir() + _T("uidata/MUI_flat/");
  _img_MUI_settings_svg = LoadSVG(iconDir + _T("MUI_settings.svg"),
                                  2 * GetCharHeight(), 2 * GetCharHeight());

  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
  wxBitmap tide = style->GetIcon(_T("tidesml"));
  wxImage tide1 = tide.ConvertToImage();
  wxImage tide1s = tide1.Scale(m_sizeMetric * 3 / 2, m_sizeMetric * 3 / 2,
                               wxIMAGE_QUALITY_HIGH);
  m_bmTide = wxBitmap(tide1s);

  return;
}

void MarkInfoDlg::Create() {
  wxFont* qFont = GetOCPNScaledFont(_("Dialog"));
  SetFont(*qFont);
  m_sizeMetric = GetCharHeight();

#ifdef __ANDROID__
  //  Set Dialog Font by custom crafted Qt Stylesheet.

  wxString wqs = getFontQtStylesheet(qFont);
  wxCharBuffer sbuf = wqs.ToUTF8();
  QString qsb = QString(sbuf.data());

  QString qsbq = getAdjustedDialogStyleSheet();  // basic scrollbars, etc

  this->GetHandle()->setStyleSheet(qsb + qsbq);  // Concatenated style sheets

  wxScreenDC sdc;
  if (sdc.IsOk())
    sdc.GetTextExtent(_T("W"), NULL, &m_sizeMetric, NULL, NULL, qFont);

#endif

  initialize_images();

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);
  SetSizer(bSizer1);
  bSizer1->SetSizeHints(this);  // set size hints to honour minimum size

  m_notebookProperties =
      new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

  m_panelBasicProperties = new wxScrolledWindow(
      m_notebookProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxHSCROLL | wxVSCROLL | wxTAB_TRAVERSAL);
#ifdef __ANDROID__
  m_panelBasicProperties->GetHandle()->setStyleSheet(
      getAdjustedDialogStyleSheet());
#endif

  m_panelBasicProperties->SetScrollRate(0, 2);

  m_notebookProperties->AddPage(m_panelBasicProperties, _("Basic"), true);

  bSizerBasicProperties = new wxBoxSizer(wxVERTICAL);
  m_panelBasicProperties->SetSizer(bSizerBasicProperties);

  wxStaticBoxSizer* sbSizerProperties;
  sbSizerProperties = new wxStaticBoxSizer(
      new wxStaticBox(m_panelBasicProperties, wxID_ANY, _("Properties")),
      wxVERTICAL);

  wxBoxSizer* bSizerInnerProperties = new wxBoxSizer(wxHORIZONTAL);

  //    m_bitmapIcon = new wxStaticBitmap( m_panelBasicProperties, wxID_ANY,
  //    wxNullBitmap,
  //            wxDefaultPosition, wxDefaultSize, 0 );
  //    bSizerInnerProperties->Add( m_bitmapIcon, 0, wxALL, 5 );
  //    m_bitmapIcon->Show( false );

  wxBoxSizer* bSizerTextProperties;
  bSizerTextProperties = new wxBoxSizer(wxVERTICAL);

  m_staticTextLayer = new wxStaticText(
      m_panelBasicProperties, wxID_ANY,
      _("This waypoint is part of a layer and can't be edited"),
      wxDefaultPosition, wxDefaultSize, 0);
  m_staticTextLayer->Enable(false);

  bSizerTextProperties->Add(m_staticTextLayer, 0, wxALL, 5);

  wxBoxSizer* bSizerName;
  bSizerName = new wxBoxSizer(wxHORIZONTAL);

  m_staticTextName =
      new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Name"),
                       wxDefaultPosition, wxDefaultSize, 0);
  // m_staticTextName->Wrap( -1 );
  bSizerName->Add(m_staticTextName, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  wxBoxSizer* bSizerNameValue = new wxBoxSizer(wxVERTICAL);

  m_checkBoxShowName =
      new wxCheckBox(m_panelBasicProperties, wxID_ANY, wxEmptyString,
                     wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_VERTICAL);
  bSizerName->Add(m_checkBoxShowName, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_textName = new wxTextCtrl(m_panelBasicProperties, wxID_ANY, wxEmptyString,
                              wxDefaultPosition, wxDefaultSize, 0);
  bSizerNameValue->Add(m_textName, 0, wxALL | wxEXPAND, 5);
  bSizerName->Add(bSizerNameValue, 1, wxEXPAND, 5);
  bSizerTextProperties->Add(bSizerName, 0, wxEXPAND, 5);

  ///
  wxBoxSizer* bSizer8 = new wxBoxSizer(wxHORIZONTAL);
  bSizerTextProperties->Add(bSizer8, 0, wxEXPAND, 5);

  m_staticTextIcon =
      new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Icon"),
                       wxDefaultPosition, wxDefaultSize, 0);
  bSizer8->Add(m_staticTextIcon, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_bcomboBoxIcon = new OCPNIconCombo(m_panelBasicProperties, wxID_ANY,
                                      _("Combo!"), wxDefaultPosition,
                                      wxDefaultSize, 0, NULL, wxCB_READONLY);

  m_bcomboBoxIcon->SetPopupMaxHeight(::wxGetDisplaySize().y / 2);

  //  Accomodate scaling of icon
  int min_size = m_sizeMetric * 2;
  min_size = wxMax(min_size, (32 * g_MarkScaleFactorExp) + 4);
  m_bcomboBoxIcon->SetMinSize(wxSize(-1, min_size));

  bSizer8->Add(m_bcomboBoxIcon, 1, wxALL, 5);

  bSizerTextProperties->AddSpacer(5);

  wxFlexGridSizer* LLGrid = new wxFlexGridSizer(0, 2, 1, 1);
  LLGrid->AddGrowableCol(1);
  bSizerTextProperties->Add(LLGrid, 0, wxEXPAND, 0);

  int w, h;
  GetTextExtent(_T("179 59.9999 W"), &w, &h);

  wxGridBagSizer* gridBagSizer = new wxGridBagSizer(5, 5);

  m_staticTextLatitude =
      new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Latitude"));
  gridBagSizer->Add(m_staticTextLatitude, wxGBPosition(0, 0), wxDefaultSpan,
                    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

  m_textLatitude =
      new wxTextCtrl(m_panelBasicProperties, wxID_ANY, wxEmptyString,
                     wxDefaultPosition, wxSize(w + 20, -1), 0);
  gridBagSizer->Add(m_textLatitude, wxGBPosition(0, 1), wxDefaultSpan,
                    wxALIGN_LEFT | wxEXPAND, 5);

  m_staticTextLongitude =
      new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Longitude"));
  gridBagSizer->Add(m_staticTextLongitude, wxGBPosition(0, 2), wxDefaultSpan,
                    wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

  m_textLongitude =
      new wxTextCtrl(m_panelBasicProperties, wxID_ANY, wxEmptyString,
                     wxDefaultPosition, wxSize(w + 20, -1), 0);
  gridBagSizer->Add(m_textLongitude, wxGBPosition(0, 3), wxDefaultSpan,
                    wxALIGN_LEFT | wxEXPAND, 5);

  bSizerTextProperties->Add(gridBagSizer, 0, wxEXPAND | wxALL, 5);

  m_staticTextDescription =
      new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Description"),
                       wxDefaultPosition, wxDefaultSize, 0);
  bSizerTextProperties->Add(m_staticTextDescription, 0, wxALL, 5);

  wxBoxSizer* bSizer14;
  bSizer14 = new wxBoxSizer(wxHORIZONTAL);

  m_textDescription = new wxTextCtrl(
      m_panelBasicProperties, ID_DESCR_CTR_BASIC, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
  m_textDescription->SetMinSize(wxSize(-1, 80));
  bSizer14->Add(m_textDescription, 1, wxALL | wxEXPAND, 5);

  m_buttonExtDescription =
      new wxButton(m_panelBasicProperties, ID_BTN_DESC_BASIC, _T("..."),
                   wxDefaultPosition, wxSize(GetCharHeight() * 15 / 10, -1), 0);
  bSizer14->Add(m_buttonExtDescription, 0, wxALL | wxEXPAND, 5);

  bSizerTextProperties->Add(bSizer14, 1, wxEXPAND, 5);

  bSizerInnerProperties->Add(bSizerTextProperties, 1, wxEXPAND, 5);

  sbSizerProperties->Add(bSizerInnerProperties, 1, wxEXPAND, 5);

  bSizerBasicProperties->Add(sbSizerProperties, 2, wxALL | wxEXPAND, 5);

#ifndef __ANDROID__  // wxSimpleHtmlListBox is broken on Android....
  wxStaticText* staticTextLinks =
      new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Links"),
                       wxDefaultPosition, wxDefaultSize, 0);
  bSizerTextProperties->Add(staticTextLinks, 0, wxALL, 5);

  wxBoxSizer* bSizer19 = new wxBoxSizer(wxHORIZONTAL);
  bSizerTextProperties->Add(bSizer19, 1, wxEXPAND, 5);

  m_htmlList = new wxSimpleHtmlListBox(m_panelBasicProperties, wxID_ANY,
                                       wxDefaultPosition, wxDefaultSize, 0);
  bSizer19->Add(m_htmlList, 1, wxALL | wxEXPAND, 5);
#else

  sbSizerLinks = new wxStaticBoxSizer(
      new wxStaticBox(m_panelBasicProperties, wxID_ANY, _("Links")),
      wxVERTICAL);
  bSizerBasicProperties->Add(sbSizerLinks, 1, wxALL | wxEXPAND, 5);

  m_scrolledWindowLinks =
      new wxScrolledWindow(m_panelBasicProperties, wxID_ANY, wxDefaultPosition,
                           wxSize(-1, 100), wxHSCROLL | wxVSCROLL);
  m_scrolledWindowLinks->SetMinSize(wxSize(-1, 80));
  m_scrolledWindowLinks->SetScrollRate(2, 2);
  sbSizerLinks->Add(m_scrolledWindowLinks, 1, wxEXPAND | wxALL, 5);

  bSizerLinks = new wxBoxSizer(wxVERTICAL);
  m_scrolledWindowLinks->SetSizer(bSizerLinks);

  m_menuLink = new wxMenu();
  wxMenuItem* m_menuItemDelete;
  m_menuItemDelete = new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Delete")),
                                    wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemDelete);

  wxMenuItem* m_menuItemEdit;
  m_menuItemEdit = new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Edit")),
                                  wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemEdit);

  wxMenuItem* m_menuItemAdd;
  m_menuItemAdd = new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Add new")),
                                 wxEmptyString, wxITEM_NORMAL);
  m_menuLink->Append(m_menuItemAdd);

  wxBoxSizer* bSizer9 = new wxBoxSizer(wxHORIZONTAL);

  m_buttonAddLink =
      new wxButton(m_panelBasicProperties, wxID_ANY, _("Add new"),
                   wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
  bSizer9->Add(m_buttonAddLink, 0, wxALL, 5);

  m_buttonAddLink->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(MarkInfoDlg::OnAddLink), NULL,
                           this);

  sbSizerLinks->Add(bSizer9, 0, wxEXPAND, 5);

#endif

  m_panelDescription =
      new wxPanel(m_notebookProperties, wxID_ANY, wxDefaultPosition,
                  wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer15;
  bSizer15 = new wxBoxSizer(wxVERTICAL);

  m_textCtrlExtDescription =
      new wxTextCtrl(m_panelDescription, ID_DESCR_CTR_DESC, wxEmptyString,
                     wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
  bSizer15->Add(m_textCtrlExtDescription, 1, wxALL | wxEXPAND, 5);

  m_panelDescription->SetSizer(bSizer15);
  m_notebookProperties->AddPage(m_panelDescription, _("Description"), false);

  /////////////////////////////////////////////////////// EXTENDED
  //////////////////////////////////////////////////////////

  m_panelExtendedProperties = new wxScrolledWindow(
      m_notebookProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxHSCROLL | wxVSCROLL | wxTAB_TRAVERSAL);
#ifdef __ANDROID__
  m_panelExtendedProperties->GetHandle()->setStyleSheet(
      getAdjustedDialogStyleSheet());
#endif

  m_panelExtendedProperties->SetScrollRate(0, 2);

  wxBoxSizer* fSizerExtProperties = new wxBoxSizer(wxVERTICAL);
  m_panelExtendedProperties->SetSizer(fSizerExtProperties);
  m_notebookProperties->AddPage(m_panelExtendedProperties, _("Extended"),
                                false);

  sbSizerExtProperties = new wxStaticBoxSizer(
      wxVERTICAL, m_panelExtendedProperties, _("Extended Properties"));
  wxFlexGridSizer* gbSizerInnerExtProperties = new wxFlexGridSizer(3, 0, 0);
  gbSizerInnerExtProperties->AddGrowableCol(2);
  gbSizerInnerExtProperties->SetFlexibleDirection(wxBOTH);
  gbSizerInnerExtProperties->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  m_checkBoxVisible = new wxCheckBox(sbSizerExtProperties->GetStaticBox(),
                                     ID_CHECKBOX_VIS_EXT, wxEmptyString);
  gbSizerInnerExtProperties->Add(m_checkBoxVisible);
  wxStaticText* m_staticTextVisible = new wxStaticText(
      sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Show on chart"));
  gbSizerInnerExtProperties->Add(m_staticTextVisible);
  gbSizerInnerExtProperties->Add(0, 0, 1, wxEXPAND, 0);

  m_checkBoxScaMin = new wxCheckBox(sbSizerExtProperties->GetStaticBox(),
                                    ID_CHECKBOX_SCAMIN_VIS, wxEmptyString);
  gbSizerInnerExtProperties->Add(m_checkBoxScaMin, 0, wxALIGN_CENTRE_VERTICAL,
                                 0);
  m_staticTextScaMin = new wxStaticText(sbSizerExtProperties->GetStaticBox(),
                                        wxID_ANY, _("Show at scale > 1 :"));
  gbSizerInnerExtProperties->Add(m_staticTextScaMin, 0, wxALIGN_CENTRE_VERTICAL,
                                 0);
  m_textScaMin = new wxTextCtrl(sbSizerExtProperties->GetStaticBox(), wxID_ANY);
  gbSizerInnerExtProperties->Add(m_textScaMin, 0, wxALL | wxEXPAND, 5);

  m_checkBoxShowNameExt =
      new wxCheckBox(sbSizerExtProperties->GetStaticBox(),
                     ID_SHOWNAMECHECKBOX_EXT, wxEmptyString);
  gbSizerInnerExtProperties->Add(m_checkBoxShowNameExt);
  m_staticTextShowNameExt = new wxStaticText(
      sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Show waypoint name"));
  gbSizerInnerExtProperties->Add(m_staticTextShowNameExt);
  gbSizerInnerExtProperties->Add(0, 0, 1, wxEXPAND, 0);

  sbRangeRingsExtProperties = new wxStaticBoxSizer(
      wxVERTICAL, sbSizerExtProperties->GetStaticBox(), _("Range rings"));
  wxFlexGridSizer* gbRRExtProperties = new wxFlexGridSizer(4, 0, 0);
  gbRRExtProperties->AddGrowableCol(0);
  gbRRExtProperties->AddGrowableCol(1);
  gbRRExtProperties->AddGrowableCol(3);
  m_staticTextRR1 = new wxStaticText(sbSizerExtProperties->GetStaticBox(),
                                     wxID_ANY, _("Number"));
  gbRRExtProperties->Add(m_staticTextRR1, 0, wxLEFT, 5);
  m_staticTextRR2 = new wxStaticText(sbSizerExtProperties->GetStaticBox(),
                                     wxID_ANY, _("Distance"));
  gbRRExtProperties->Add(m_staticTextRR2, 0, wxLEFT, 5);
  gbRRExtProperties->Add(0, 0, 1, wxEXPAND, 5);  // a spacer
  m_staticTextRR4 = new wxStaticText(sbSizerExtProperties->GetStaticBox(),
                                     wxID_ANY, _("Color"));
  gbRRExtProperties->Add(m_staticTextRR4, 0, wxLEFT, 5);

  wxString rrAlt[] = {_("None"), _T( "1" ), _T( "2" ), _T( "3" ),
                      _T( "4" ), _T( "5" ), _T( "6" ), _T( "7" ),
                      _T( "8" ), _T( "9" ), _T( "10" )};
  m_ChoiceWaypointRangeRingsNumber =
      new wxChoice(sbSizerExtProperties->GetStaticBox(), ID_WPT_RANGERINGS_NO,
                   wxDefaultPosition, wxDefaultSize, 11, rrAlt);

  gbRRExtProperties->Add(m_ChoiceWaypointRangeRingsNumber, 0, wxALL | wxEXPAND,
                         5);
  m_textWaypointRangeRingsStep =
      new wxTextCtrl(sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("0.05"),
                     wxDefaultPosition, wxDefaultSize, 0);
  gbRRExtProperties->Add(m_textWaypointRangeRingsStep, 0, wxALL | wxEXPAND, 5);

  wxString pDistUnitsStrings[] = {_("NMi"), _("km")};
  m_RangeRingUnits =
      new wxChoice(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                   wxDefaultPosition, wxDefaultSize, 2, pDistUnitsStrings);
  gbRRExtProperties->Add(m_RangeRingUnits, 0, wxALIGN_CENTRE_VERTICAL, 0);

  m_PickColor = new wxColourPickerCtrl(sbSizerExtProperties->GetStaticBox(),
                                       wxID_ANY, wxColour(0, 0, 0),
                                       wxDefaultPosition, wxDefaultSize, 0);
  gbRRExtProperties->Add(m_PickColor, 0, wxALL | wxEXPAND, 5);
  sbRangeRingsExtProperties->Add(
      gbRRExtProperties, 1,
      wxLEFT | wxTOP | wxEXPAND | wxALIGN_LEFT | wxALIGN_TOP, 5);

  sbSizerExtProperties->GetStaticBox()->Layout();

  wxFlexGridSizer* gbSizerInnerExtProperties2 = new wxFlexGridSizer(2, 0, 0);
  gbSizerInnerExtProperties2->AddGrowableCol(1);

  m_staticTextGuid =
      new wxStaticText(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                       _("GUID"), wxDefaultPosition, wxDefaultSize, 0);
  gbSizerInnerExtProperties2->Add(m_staticTextGuid, 0, wxALIGN_CENTRE_VERTICAL,
                                  0);
  m_textCtrlGuid = new wxTextCtrl(sbSizerExtProperties->GetStaticBox(),
                                  wxID_ANY, wxEmptyString, wxDefaultPosition,
                                  wxDefaultSize, wxTE_READONLY);
  m_textCtrlGuid->SetEditable(false);
  gbSizerInnerExtProperties2->Add(m_textCtrlGuid, 0, wxALL | wxEXPAND, 5);

  wxFlexGridSizer* gbSizerInnerExtProperties1 = new wxFlexGridSizer(3, 0, 0);
  gbSizerInnerExtProperties1->AddGrowableCol(1);

  m_staticTextTideStation =
      new wxStaticText(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                       _("Tide Station"), wxDefaultPosition, wxDefaultSize, 0);
  gbSizerInnerExtProperties1->Add(m_staticTextTideStation, 0,
                                  wxALIGN_CENTRE_VERTICAL, 5);

#ifdef __ANDROID__
  m_choiceTideChoices.Add(_T(" "));
  m_comboBoxTideStation =
      new wxChoice(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                   wxDefaultPosition, wxDefaultSize, m_choiceTideChoices);

  gbSizerInnerExtProperties1->Add(
      m_comboBoxTideStation, 0, wxALL | wxEXPAND | wxALIGN_CENTRE_VERTICAL, 5);

#else
  m_comboBoxTideStation = new wxComboBox(
      sbSizerExtProperties->GetStaticBox(), wxID_ANY, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
  gbSizerInnerExtProperties1->Add(
      m_comboBoxTideStation, 0, wxALL | wxEXPAND | wxALIGN_CENTRE_VERTICAL, 5);
#endif

  m_buttonShowTides = new wxBitmapButton(
      sbSizerExtProperties->GetStaticBox(), ID_BTN_SHOW_TIDES, m_bmTide,
      wxDefaultPosition, m_bmTide.GetSize(), 0);
  gbSizerInnerExtProperties1->Add(m_buttonShowTides, 0,
                                  wxALL | wxALIGN_CENTRE_VERTICAL, 5);

  m_staticTextArrivalRadius = new wxStaticText(
      sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Arrival Radius"));
  gbSizerInnerExtProperties1->Add(m_staticTextArrivalRadius, 0,
                                  wxALIGN_CENTRE_VERTICAL, 0);
  m_textArrivalRadius =
      new wxTextCtrl(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                     wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  gbSizerInnerExtProperties1->Add(m_textArrivalRadius, 0, wxALL | wxEXPAND, 5);
  m_staticTextArrivalUnits =
      new wxStaticText(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                       wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  gbSizerInnerExtProperties1->Add(m_staticTextArrivalUnits, 0,
                                  wxALIGN_CENTRE_VERTICAL, 0);

  m_staticTextPlSpeed =
      new wxStaticText(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                       _("Planned Speed"), wxDefaultPosition, wxDefaultSize, 0);
  gbSizerInnerExtProperties1->Add(m_staticTextPlSpeed, 0,
                                  wxALIGN_CENTRE_VERTICAL, 0);
  m_textCtrlPlSpeed =
      new wxTextCtrl(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                     wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
  gbSizerInnerExtProperties1->Add(m_textCtrlPlSpeed, 0, wxALL | wxEXPAND, 5);
  m_staticTextPlSpeedUnits =
      new wxStaticText(sbSizerExtProperties->GetStaticBox(), wxID_ANY,
                       getUsrSpeedUnit(), wxDefaultPosition, wxDefaultSize, 0);
  gbSizerInnerExtProperties1->Add(m_staticTextPlSpeedUnits, 0,
                                  wxALIGN_CENTRE_VERTICAL, 0);

  m_staticTextEta = new wxStaticText(sbSizerExtProperties->GetStaticBox(),
                                     wxID_ANY, _("ETD (UTC)"));
  gbSizerInnerExtProperties1->Add(m_staticTextEta, 0, wxALIGN_CENTRE_VERTICAL,
                                  0);
  wxBoxSizer* bsTimestamp = new wxBoxSizer(wxHORIZONTAL);
  m_cbEtaPresent = new wxCheckBox(sbSizerExtProperties->GetStaticBox(),
                                  wxID_ANY, wxEmptyString);
  bsTimestamp->Add(m_cbEtaPresent, 0, wxALL | wxEXPAND, 5);
  m_EtaDatePickerCtrl = new wxDatePickerCtrl(
      sbSizerExtProperties->GetStaticBox(), ID_ETA_DATEPICKERCTRL,
      wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT,
      wxDefaultValidator);
  bsTimestamp->Add(m_EtaDatePickerCtrl, 0, wxALL | wxEXPAND, 5);

#ifdef __WXGTK__
  m_EtaTimePickerCtrl =
      new TimeCtrl(sbSizerExtProperties->GetStaticBox(), ID_ETA_TIMEPICKERCTRL,
                   wxDefaultDateTime, wxDefaultPosition, wxDefaultSize);
#else
  m_EtaTimePickerCtrl = new wxTimePickerCtrl(
      sbSizerExtProperties->GetStaticBox(), ID_ETA_TIMEPICKERCTRL,
      wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT,
      wxDefaultValidator);
#endif

  bsTimestamp->Add(m_EtaTimePickerCtrl, 0, wxALL | wxEXPAND, 5);
  gbSizerInnerExtProperties1->Add(bsTimestamp, 0, wxEXPAND, 0);
  sbSizerExtProperties->Add(gbSizerInnerExtProperties, 0, wxALL | wxEXPAND, 5);
  sbSizerExtProperties->Add(sbRangeRingsExtProperties, 0, wxALL | wxEXPAND, 5);
  sbSizerExtProperties->Add(gbSizerInnerExtProperties2, 0, wxALL | wxEXPAND, 5);
  sbSizerExtProperties->Add(gbSizerInnerExtProperties1, 0, wxALL | wxEXPAND, 5);

  fSizerExtProperties->Add(sbSizerExtProperties, 1, wxALL | wxEXPAND);

  //-----------------
  bSizer1->Add(m_notebookProperties, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
  bSizer1->Add(btnSizer, 0, wxEXPAND, 0);

  DefaultsBtn =
      new wxBitmapButton(this, ID_DEFAULT, _img_MUI_settings_svg,
                         wxDefaultPosition, _img_MUI_settings_svg.GetSize(), 0);
  btnSizer->Add(DefaultsBtn, 0, wxALL | wxALIGN_LEFT | wxALIGN_BOTTOM, 5);
  btnSizer->Add(0, 0, 1, wxEXPAND);  // spacer

  m_sdbSizerButtons = new wxStdDialogButtonSizer();
  m_sdbSizerButtons->AddButton(new wxButton(this, wxID_OK));
  m_sdbSizerButtons->AddButton(new wxButton(this, wxID_CANCEL, _("Cancel")));
  m_sdbSizerButtons->Realize();
  btnSizer->Add(m_sdbSizerButtons, 0, wxALL, 5);

  // SetMinSize(wxSize(-1, 600));

  // Connect Events
  m_textLatitude->Connect(
      wxEVT_CONTEXT_MENU,
      wxCommandEventHandler(MarkInfoDlg::OnRightClickLatLon), NULL, this);
  m_textLongitude->Connect(
      wxEVT_CONTEXT_MENU,
      wxCommandEventHandler(MarkInfoDlg::OnRightClickLatLon), NULL, this);
#ifndef __ANDROID__  // wxSimpleHtmlListBox is broken on Android....
  m_htmlList->Connect(wxEVT_RIGHT_DOWN,
                      wxMouseEventHandler(MarkInfoDlg::m_htmlListContextMenu),
                      NULL, this);
#else
#endif
  m_notebookProperties->Connect(
      wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
      wxNotebookEventHandler(MarkInfoDlg::OnNotebookPageChanged), NULL, this);
  // m_EtaTimePickerCtrl->Connect( wxEVT_TIME_CHANGED, wxDateEventHandler(
  // MarkInfoDlg::OnTimeChanged ), NULL, this ); m_EtaDatePickerCtrl->Connect(
  // wxEVT_DATE_CHANGED, wxDateEventHandler( MarkInfoDlg::OnTimeChanged ), NULL,
  // this );
  m_comboBoxTideStation->Connect(
      wxEVT_COMMAND_COMBOBOX_SELECTED,
      wxCommandEventHandler(MarkInfoDlg::OnTideStationCombobox), NULL, this);
}

void MarkInfoDlg::OnClose(wxCloseEvent& event) {
  Hide();
  event.Veto();
  if (m_pRoutePoint) m_pRoutePoint->m_bRPIsBeingEdited = false;
}

#define TIDESTATION_BATCH_SIZE 10

void MarkInfoDlg::OnTideStationCombobox(wxCommandEvent& event) {
  int count = m_comboBoxTideStation->GetCount();
  int sel = m_comboBoxTideStation->GetSelection();
  if (sel == count - 1) {
    wxString n;
    int i = 0;
    for (auto ts : m_tss) {
      if (i == count + TIDESTATION_BATCH_SIZE) {
        break;
      }
      if (i > count) {
        n = wxString::FromUTF8(ts.second->IDX_station_name);
        m_comboBoxTideStation->Append(n);
      }
      i++;
    }
  }
}

void MarkInfoDlg::OnNotebookPageChanged(wxNotebookEvent& event) {
  if (event.GetSelection() == EXTENDED_PROP_PAGE) {
    if (m_lasttspos.IsSameAs(m_textLatitude->GetValue() +
                             m_textLongitude->GetValue())) {
      return;
    }
    m_lasttspos = m_textLatitude->GetValue() + m_textLongitude->GetValue();
    double lat = fromDMM(m_textLatitude->GetValue());
    double lon = fromDMM(m_textLongitude->GetValue());
    m_tss = ptcmgr->GetStationsForLL(lat, lon);
    wxString s = m_comboBoxTideStation->GetStringSelection();
    wxString n;
    int i = 0;
    m_comboBoxTideStation->Clear();
    m_comboBoxTideStation->Append(wxEmptyString);
    for (auto ts : m_tss) {
      if (i == TIDESTATION_BATCH_SIZE) {
        break;
      }
      i++;
      n = wxString::FromUTF8(ts.second->IDX_station_name);
      m_comboBoxTideStation->Append(n);
      if (s == n) {
        m_comboBoxTideStation->SetSelection(i);
      }
    }
    if (m_comboBoxTideStation->GetStringSelection() != s) {
      m_comboBoxTideStation->Insert(s, 1);
      m_comboBoxTideStation->SetSelection(1);
    }
  }
}

void MarkInfoDlg::RecalculateSize(void) {
#ifdef __ANDROID__

  Layout();

  wxSize dsize = GetParent()->GetClientSize();

  wxSize esize;

  esize.x = GetCharHeight() * 20;
  esize.y = GetCharHeight() * 40;
  // qDebug() << "esizeA" << esize.x << esize.y;

  esize.y = wxMin(esize.y, dsize.y - (2 * GetCharHeight()));
  esize.x = wxMin(esize.x, dsize.x - (1 * GetCharHeight()));
  SetSize(wxSize(esize.x, esize.y));
  // qDebug() << "esize" << esize.x << esize.y;

  wxSize fsize = GetSize();
  fsize.y = wxMin(fsize.y, dsize.y - (2 * GetCharHeight()));
  fsize.x = wxMin(fsize.x, dsize.x - (1 * GetCharHeight()));
  // qDebug() << "fsize" << fsize.x << fsize.y;

  //  And finally, not too tall...
  fsize.y = wxMin(fsize.y, (25 * GetCharHeight()));

  SetSize(wxSize(-1, fsize.y));

  m_defaultClientSize = GetClientSize();
  Center();
#else
  wxSize dsize = GetParent()->GetClientSize();
  SetSize(-1, wxMax(GetSize().y, dsize.y / 1.5));
#endif
}

MarkInfoDlg::~MarkInfoDlg() {
  // Disconnect Events
  m_textLatitude->Disconnect(
      wxEVT_CONTEXT_MENU,
      wxCommandEventHandler(MarkInfoDlg::OnRightClickLatLon), NULL, this);
  m_textLongitude->Disconnect(
      wxEVT_CONTEXT_MENU,
      wxCommandEventHandler(MarkInfoDlg::OnRightClickLatLon), NULL, this);
#ifndef __ANDROID__  // wxSimpleHtmlListBox is broken on Android....
  m_htmlList->Disconnect(
      wxEVT_RIGHT_DOWN, wxMouseEventHandler(MarkInfoDlg::m_htmlListContextMenu),
      NULL, this);
#else
#endif

  m_notebookProperties->Disconnect(
      wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
      wxNotebookEventHandler(MarkInfoDlg::OnNotebookPageChanged), NULL, this);
  m_EtaTimePickerCtrl->Disconnect(
      wxEVT_TIME_CHANGED, wxDateEventHandler(MarkInfoDlg::OnTimeChanged), NULL,
      this);
  m_EtaDatePickerCtrl->Disconnect(
      wxEVT_DATE_CHANGED, wxDateEventHandler(MarkInfoDlg::OnTimeChanged), NULL,
      this);

#ifdef __ANDROID__
  androidEnableBackButton(true);
#endif
}

void MarkInfoDlg::InitialFocus(void) {
  m_textName->SetFocus();
  m_textName->SetInsertionPointEnd();
}

void MarkInfoDlg::SetColorScheme(ColorScheme cs) { DimeControl(this); }

void MarkInfoDlg::ClearData() {
  m_pRoutePoint = NULL;
  UpdateProperties();
}

void MarkInfoDlg::SetRoutePoint(RoutePoint* pRP) {
  m_pRoutePoint = pRP;
  if (m_pRoutePoint) {
    m_lat_save = m_pRoutePoint->m_lat;
    m_lon_save = m_pRoutePoint->m_lon;
    m_IconName_save = m_pRoutePoint->GetIconName();
    m_bShowName_save = m_pRoutePoint->m_bShowName;
    m_bIsVisible_save = m_pRoutePoint->m_bIsVisible;
    m_Name_save = m_pRoutePoint->GetName();
    m_Description_save = m_pRoutePoint->m_MarkDescription;
    m_bUseScaMin_save = m_pRoutePoint->GetUseSca();
    m_iScaminVal_save = m_pRoutePoint->GetScaMin();

    if (m_pMyLinkList) delete m_pMyLinkList;
    m_pMyLinkList = new HyperlinkList();
    int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
    if (NbrOfLinks > 0) {
      wxHyperlinkListNode* linknode =
          m_pRoutePoint->m_HyperlinkList->GetFirst();
      while (linknode) {
        Hyperlink* link = linknode->GetData();

        Hyperlink* h = new Hyperlink();
        h->DescrText = link->DescrText;
        h->Link = link->Link;
        h->LType = link->LType;

        m_pMyLinkList->Append(h);

        linknode = linknode->GetNext();
      }
    }
  }
}

/*!
 * Attach route point name validator and bind to key event.
 */
void MarkInfoDlg::SetNameValidator(wxTextValidator* pValidator) {
  m_textName->SetValidator(*pValidator);
  m_textName->Bind(wxEVT_TEXT, &MarkInfoDlg::OnNameChanged, this);
}

void MarkInfoDlg::UpdateHtmlList() {
#ifndef __ANDROID__  // wxSimpleHtmlListBox is broken on Android....
  GetSimpleBox()->Clear();
  int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();

  if (NbrOfLinks > 0) {
    wxHyperlinkListNode* linknode = m_pRoutePoint->m_HyperlinkList->GetFirst();
    while (linknode) {
      Hyperlink* link = linknode->GetData();
      wxString s = wxString::Format(wxT("<a href='%s'>%s</a>"), link->Link,
                                    link->DescrText);
      GetSimpleBox()->AppendString(s);
      linknode = linknode->GetNext();
    }
  }
#else
  // Clear the list
  wxWindowList kids = m_scrolledWindowLinks->GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode* node = kids.Item(i);
    wxWindow* win = node->GetData();

    auto link_win = dynamic_cast<wxHyperlinkCtrl*>(win);
    if (link_win) {
      link_win->Disconnect(
          wxEVT_COMMAND_HYPERLINK,
          wxHyperlinkEventHandler(MarkInfoDlg::OnHyperLinkClick));
      link_win->Disconnect(
          wxEVT_RIGHT_DOWN,
          wxMouseEventHandler(MarkInfoDlg::m_htmlListContextMenu));
      win->Destroy();
    }
  }

  int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
  HyperlinkList* hyperlinklist = m_pRoutePoint->m_HyperlinkList;
  if (NbrOfLinks > 0) {
    wxHyperlinkListNode* linknode = hyperlinklist->GetFirst();
    while (linknode) {
      Hyperlink* link = linknode->GetData();
      wxString Link = link->Link;
      wxString Descr = link->DescrText;

      wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl(
          m_scrolledWindowLinks, wxID_ANY, Descr, Link, wxDefaultPosition,
          wxDefaultSize, wxNO_BORDER | wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT);
      ctrl->Connect(wxEVT_COMMAND_HYPERLINK,
                    wxHyperlinkEventHandler(MarkInfoDlg::OnHyperLinkClick),
                    NULL, this);
      if (!m_pRoutePoint->m_bIsInLayer)
        ctrl->Connect(wxEVT_RIGHT_DOWN,
                      wxMouseEventHandler(MarkInfoDlg::m_htmlListContextMenu),
                      NULL, this);

      bSizerLinks->Add(ctrl, 1, wxALL | wxEXPAND, 5);

      linknode = linknode->GetNext();
    }
  }

  // Integrate all of the rebuilt hyperlink controls
  m_scrolledWindowLinks->Layout();
#endif
}

void MarkInfoDlg::OnHyperLinkClick(wxHyperlinkEvent& event) {
  wxString url = event.GetURL();
  url.Replace(_T(" "), _T("%20"));
  if (g_Platform) g_Platform->platformLaunchDefaultBrowser(url);
}

void MarkInfoDlg::OnHtmlLinkClicked(wxHtmlLinkEvent& event) {
  //        Windows has trouble handling local file URLs with embedded anchor
  //        points, e.g file://testfile.html#point1 The trouble is with the
  //        wxLaunchDefaultBrowser with verb "open" Workaround is to probe the
  //        registry to get the default browser, and open directly
  //
  //        But, we will do this only if the URL contains the anchor point
  //        character '#' What a hack......

#ifdef __WXMSW__
  wxString cc = event.GetLinkInfo().GetHref().c_str();
  if (cc.Find(_T("#")) != wxNOT_FOUND) {
    wxRegKey RegKey(
        wxString(_T("HKEY_CLASSES_ROOT\\HTTP\\shell\\open\\command")));
    if (RegKey.Exists()) {
      wxString command_line;
      RegKey.QueryValue(wxString(_T("")), command_line);

      //  Remove "
      command_line.Replace(wxString(_T("\"")), wxString(_T("")));

      //  Strip arguments
      int l = command_line.Find(_T(".exe"));
      if (wxNOT_FOUND == l) l = command_line.Find(_T(".EXE"));

      if (wxNOT_FOUND != l) {
        wxString cl = command_line.Mid(0, l + 4);
        cl += _T(" ");
        cc.Prepend(_T("\""));
        cc.Append(_T("\""));
        cl += cc;
        wxExecute(cl);  // Async, so Fire and Forget...
      }
    }
  } else {
    wxString url = event.GetLinkInfo().GetHref().c_str();
    url.Replace(_T(" "), _T("%20"));
    ::wxLaunchDefaultBrowser(url);
    event.Skip();
  }
#else
  wxString url = event.GetLinkInfo().GetHref().c_str();
  url.Replace(_T(" "), _T("%20"));
  if (g_Platform) g_Platform->platformLaunchDefaultBrowser(url);

  event.Skip();
#endif
}

void MarkInfoDlg::OnDescChangedExt(wxCommandEvent& event) {
  if (m_panelDescription->IsShownOnScreen()) {
    m_textDescription->ChangeValue(m_textCtrlExtDescription->GetValue());
  }
  event.Skip();
}
void MarkInfoDlg::OnDescChangedBasic(wxCommandEvent& event) {
  if (m_panelBasicProperties->IsShownOnScreen()) {
    m_textCtrlExtDescription->ChangeValue(m_textDescription->GetValue());
  }
  event.Skip();
}

void MarkInfoDlg::OnExtDescriptionClick(wxCommandEvent& event) {
  long pos = m_textDescription->GetInsertionPoint();
  m_notebookProperties->SetSelection(1);
  m_textCtrlExtDescription->SetInsertionPoint(pos);
  event.Skip();
}

void MarkInfoDlg::OnShowWaypointNameSelectBasic(wxCommandEvent& event) {
  if (m_panelBasicProperties->IsShownOnScreen())
    m_checkBoxShowNameExt->SetValue(m_checkBoxShowName->GetValue());
  event.Skip();
}
void MarkInfoDlg::OnShowWaypointNameSelectExt(wxCommandEvent& event) {
  if (m_panelExtendedProperties->IsShownOnScreen())
    m_checkBoxShowName->SetValue(m_checkBoxShowNameExt->GetValue());
  event.Skip();
}

void MarkInfoDlg::OnWptRangeRingsNoChange(wxCommandEvent& event) {
  if (!m_pRoutePoint->m_bIsInLayer) {
    m_textWaypointRangeRingsStep->Enable(
        (bool)(m_ChoiceWaypointRangeRingsNumber->GetSelection() != 0));
    m_PickColor->Enable(
        (bool)(m_ChoiceWaypointRangeRingsNumber->GetSelection() != 0));
  }
}

void MarkInfoDlg::OnSelectScaMinExt(wxCommandEvent& event) {
  if (!m_pRoutePoint->m_bIsInLayer) {
    m_textScaMin->Enable(m_checkBoxScaMin->GetValue());
  }
}

void MarkInfoDlg::OnPositionCtlUpdated(wxCommandEvent& event) {
  // Fetch the control values, convert to degrees
  double lat = fromDMM(m_textLatitude->GetValue());
  double lon = fromDMM(m_textLongitude->GetValue());
  if (!m_pRoutePoint->m_bIsInLayer) {
    m_pRoutePoint->SetPosition(lat, lon);
    pSelect->ModifySelectablePoint(lat, lon, (void*)m_pRoutePoint,
                                   SELTYPE_ROUTEPOINT);
  }
  // Update the mark position dynamically
  gFrame->RefreshAllCanvas();
}

void MarkInfoDlg::m_htmlListContextMenu(wxMouseEvent& event) {
#ifndef __ANDROID__
  // SimpleHtmlList->HitTest doesn't seem to work under msWin, so we use a
  // custom made version
  wxPoint pos = event.GetPosition();
  i_htmlList_item = -1;
  for (int i = 0; i < (int)GetSimpleBox()->GetCount(); i++) {
    wxRect rect = GetSimpleBox()->GetItemRect(i);
    if (rect.Contains(pos)) {
      i_htmlList_item = i;
      break;
    }
  }

  wxMenu* popup = new wxMenu();
  if ((GetSimpleBox()->GetCount()) > 0 && (i_htmlList_item > -1) &&
      (i_htmlList_item < (int)GetSimpleBox()->GetCount())) {
    popup->Append(ID_RCLK_MENU_DELETE_LINK, _("Delete"));
    popup->Append(ID_RCLK_MENU_EDIT_LINK, _("Edit"));
  }
  popup->Append(ID_RCLK_MENU_ADD_LINK, _("Add New"));

  m_contextObject = event.GetEventObject();
  popup->Connect(
      wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(MarkInfoDlg::On_html_link_popupmenu_Click), NULL,
      this);
  PopupMenu(popup);
  delete popup;
#else

  m_pEditedLink = dynamic_cast<wxHyperlinkCtrl*>(event.GetEventObject());

  if (m_pEditedLink) {
    wxString url = m_pEditedLink->GetURL();
    wxString label = m_pEditedLink->GetLabel();
    i_htmlList_item = -1;
    HyperlinkList* hyperlinklist = m_pRoutePoint->m_HyperlinkList;
    if (hyperlinklist->GetCount() > 0) {
      int i = 0;
      wxHyperlinkListNode* linknode = hyperlinklist->GetFirst();
      while (linknode) {
        Hyperlink* link = linknode->GetData();
        if (link->DescrText == label) {
          i_htmlList_item = i;
          break;
        }

        linknode = linknode->GetNext();
        i++;
      }
    }

    wxFont sFont = GetOCPNGUIScaledFont(_T("Menu"));

    wxMenu* popup = new wxMenu();
    {
      wxMenuItem* menuItemDelete =
          new wxMenuItem(popup, ID_RCLK_MENU_DELETE_LINK, wxString(_("Delete")),
                         wxEmptyString, wxITEM_NORMAL);
#ifdef __WXQT__
      menuItemDelete->SetFont(sFont);
#endif
      popup->Append(menuItemDelete);

      wxMenuItem* menuItemEdit =
          new wxMenuItem(popup, ID_RCLK_MENU_EDIT_LINK, wxString(_("Edit")),
                         wxEmptyString, wxITEM_NORMAL);
#ifdef __WXQT__
      menuItemEdit->SetFont(sFont);
#endif
      popup->Append(menuItemEdit);
    }

    wxMenuItem* menuItemAdd =
        new wxMenuItem(popup, ID_RCLK_MENU_ADD_LINK, wxString(_("Add New")),
                       wxEmptyString, wxITEM_NORMAL);
#ifdef __WXQT__
    menuItemAdd->SetFont(sFont);
#endif
    popup->Append(menuItemAdd);

    m_contextObject = event.GetEventObject();
    popup->Connect(
        wxEVT_COMMAND_MENU_SELECTED,
        wxCommandEventHandler(MarkInfoDlg::On_html_link_popupmenu_Click), NULL,
        this);
    wxPoint p = m_scrolledWindowLinks->GetPosition();
    p.x += m_scrolledWindowLinks->GetSize().x / 2;
    PopupMenu(popup, p);
    delete popup;

    // m_scrolledWindowLinks->PopupMenu( m_menuLink,
    // m_pEditedLink->GetPosition().x /*+ event.GetPosition().x*/,
    // m_pEditedLink->GetPosition().y /*+ event.GetPosition().y*/ );
  }
/*
    wxPoint pos = event.GetPosition();
    i_htmlList_item = -1;
    for( int i=0; i <  (int)GetSimpleBox()->GetCount(); i++ )
    {
        wxRect rect = GetSimpleBox()->GetItemRect( i );
        if( rect.Contains( pos) ){
            i_htmlList_item = i;
            break;
        }
    }

 */
#endif
}

void MarkInfoDlg::OnAddLink(wxCommandEvent& event) {
  wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
  evt.SetId(ID_RCLK_MENU_ADD_LINK);

  On_html_link_popupmenu_Click(evt);
}

void MarkInfoDlg::On_html_link_popupmenu_Click(wxCommandEvent& event) {
  switch (event.GetId()) {
    case ID_RCLK_MENU_DELETE_LINK: {
      wxHyperlinkListNode* node =
          m_pRoutePoint->m_HyperlinkList->Item(i_htmlList_item);
      m_pRoutePoint->m_HyperlinkList->DeleteNode(node);
      UpdateHtmlList();
      break;
    }
    case ID_RCLK_MENU_EDIT_LINK: {
      Hyperlink* link =
          m_pRoutePoint->m_HyperlinkList->Item(i_htmlList_item)->GetData();
      LinkPropImpl* LinkPropDlg = new LinkPropImpl(this);
      LinkPropDlg->m_textCtrlLinkDescription->SetValue(link->DescrText);
      LinkPropDlg->m_textCtrlLinkUrl->SetValue(link->Link);
      DimeControl(LinkPropDlg);
      LinkPropDlg->ShowWindowModalThenDo([this, LinkPropDlg,
                                          link](int retcode) {
        if (retcode == wxID_OK) {
          link->DescrText = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
          link->Link = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
          m_pRoutePoint->m_HyperlinkList->Item(i_htmlList_item)->SetData(link);
          UpdateHtmlList();
        }
      });
      break;
    }
    case ID_RCLK_MENU_ADD_LINK: {
      LinkPropImpl* LinkPropDlg = new LinkPropImpl(this);
      LinkPropDlg->m_textCtrlLinkDescription->SetValue(wxEmptyString);
      LinkPropDlg->m_textCtrlLinkUrl->SetValue(wxEmptyString);
      DimeControl(LinkPropDlg);
      LinkPropDlg->ShowWindowModalThenDo([this, LinkPropDlg](int retcode) {
        if (retcode == wxID_OK) {
          Hyperlink* link = new Hyperlink;
          link->DescrText = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
          link->Link = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
          // Check if decent
          if (link->DescrText == wxEmptyString) {
            link->DescrText = link->Link;
          }
          if (link->Link == wxEmptyString) {
            delete link;
          } else {
            m_pRoutePoint->m_HyperlinkList->Append(link);
          }
          UpdateHtmlList();
        }
      });
      break;
    }
  }
  event.Skip();
}

void MarkInfoDlg::OnRightClickLatLon(wxCommandEvent& event) {
  wxMenu* popup = new wxMenu();
  popup->Append(ID_RCLK_MENU_COPY, _("Copy"));
  popup->Append(ID_RCLK_MENU_COPY_LL, _("Copy lat/long"));
  popup->Append(ID_RCLK_MENU_PASTE, _("Paste"));
  popup->Append(ID_RCLK_MENU_PASTE_LL, _("Paste lat/long"));
  m_contextObject = event.GetEventObject();
  popup->Connect(wxEVT_COMMAND_MENU_SELECTED,
                 wxCommandEventHandler(MarkInfoDlg::OnCopyPasteLatLon), NULL,
                 this);

  PopupMenu(popup);
  delete popup;
}

void MarkInfoDlg::OnCopyPasteLatLon(wxCommandEvent& event) {
  // Fetch the control values, convert to degrees
  double lat = fromDMM(m_textLatitude->GetValue());
  double lon = fromDMM(m_textLongitude->GetValue());

  wxString result;

  switch (event.GetId()) {
    case ID_RCLK_MENU_PASTE: {
      if (wxTheClipboard->Open()) {
        wxTextDataObject data;
        wxTheClipboard->GetData(data);
        result = data.GetText();
        ((wxTextCtrl*)m_contextObject)->SetValue(result);
        wxTheClipboard->Close();
      }
      return;
    }
    case ID_RCLK_MENU_PASTE_LL: {
      if (wxTheClipboard->Open()) {
        wxTextDataObject data;
        wxTheClipboard->GetData(data);
        result = data.GetText();

        PositionParser pparse(result);

        if (pparse.IsOk()) {
          m_textLatitude->SetValue(pparse.GetLatitudeString());
          m_textLongitude->SetValue(pparse.GetLongitudeString());
        }
        wxTheClipboard->Close();
      }
      return;
    }
    case ID_RCLK_MENU_COPY: {
      result = ((wxTextCtrl*)m_contextObject)->GetValue();
      break;
    }
    case ID_RCLK_MENU_COPY_LL: {
      result << toSDMM(1, lat, true) << _T('\t');
      result << toSDMM(2, lon, true);
      break;
    }
  }

  if (wxTheClipboard->Open()) {
    wxTextDataObject* data = new wxTextDataObject;
    data->SetText(result);
    wxTheClipboard->SetData(data);
    wxTheClipboard->Close();
  }
}

void MarkInfoDlg::DefautlBtnClicked(wxCommandEvent& event) {
  m_SaveDefaultDlg = new SaveDefaultsDialog(this);
  m_SaveDefaultDlg->Center();
  DimeControl(m_SaveDefaultDlg);
  int retcode = m_SaveDefaultDlg->ShowModal();

  {
    if (retcode == wxID_OK) {
      double value;
      if (m_SaveDefaultDlg->IconCB->GetValue()) {
        g_default_wp_icon =
            *pWayPointMan->GetIconKey(m_bcomboBoxIcon->GetSelection());
      }
      if (m_SaveDefaultDlg->RangRingsCB->GetValue()) {
        g_iWaypointRangeRingsNumber =
            m_ChoiceWaypointRangeRingsNumber->GetSelection();
        if (m_textWaypointRangeRingsStep->GetValue().ToDouble(&value))
          g_fWaypointRangeRingsStep = fromUsrDistance(value, -1);
        g_colourWaypointRangeRingsColour = m_PickColor->GetColour();
      }
      if (m_SaveDefaultDlg->ArrivalRCB->GetValue())
        if (m_textArrivalRadius->GetValue().ToDouble(&value))
          g_n_arrival_circle_radius = fromUsrDistance(value, -1);
      if (m_SaveDefaultDlg->ScaleCB->GetValue()) {
        g_iWpt_ScaMin = wxAtoi(m_textScaMin->GetValue());
        g_bUseWptScaMin = m_checkBoxScaMin->GetValue();
      }
      if (m_SaveDefaultDlg->NameCB->GetValue()) {
        g_bShowWptName = m_checkBoxShowName->GetValue();
      }
    }
    m_SaveDefaultDlg = NULL;
  }
}

void MarkInfoDlg::OnMarkInfoCancelClick(wxCommandEvent& event) {
  if (m_pRoutePoint) {
    m_pRoutePoint->SetVisible(m_bIsVisible_save);
    m_pRoutePoint->SetNameShown(m_bShowName_save);
    m_pRoutePoint->SetPosition(m_lat_save, m_lon_save);
    m_pRoutePoint->SetIconName(m_IconName_save);
    m_pRoutePoint->ReLoadIcon();
    m_pRoutePoint->SetName(m_Name_save);
    m_pRoutePoint->m_MarkDescription = m_Description_save;
    m_pRoutePoint->SetUseSca(m_bUseScaMin_save);
    m_pRoutePoint->SetScaMin(m_iScaminVal_save);

    m_pRoutePoint->m_HyperlinkList->Clear();

    int NbrOfLinks = m_pMyLinkList->GetCount();
    if (NbrOfLinks > 0) {
      wxHyperlinkListNode* linknode = m_pMyLinkList->GetFirst();
      while (linknode) {
        Hyperlink* link = linknode->GetData();
        Hyperlink* h = new Hyperlink();
        h->DescrText = link->DescrText;
        h->Link = link->Link;
        h->LType = link->LType;

        m_pRoutePoint->m_HyperlinkList->Append(h);

        linknode = linknode->GetNext();
      }
    }
  }

  m_lasttspos.Clear();

#ifdef __WXGTK__
  gFrame->Raise();
#endif

  Show(false);
  delete m_pMyLinkList;
  m_pMyLinkList = NULL;
  SetClientSize(m_defaultClientSize);

#ifdef __ANDROID__
  androidEnableBackButton(true);
#endif

  event.Skip();
}

void MarkInfoDlg::OnMarkInfoOKClick(wxCommandEvent& event) {
  if (m_pRoutePoint) {
    m_pRoutePoint->m_wxcWaypointRangeRingsColour = m_PickColor->GetColour();

    OnPositionCtlUpdated(event);
    SaveChanges();  // write changes to globals and update config
  }

#ifdef __WXGTK__
  gFrame->Raise();
#endif

  Show(false);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();

  if (pRoutePropDialog && pRoutePropDialog->IsShown())
    pRoutePropDialog->UpdatePoints();

  SetClientSize(m_defaultClientSize);

#ifdef __ANDROID__
  androidEnableBackButton(true);
#endif

  event.Skip();
}

bool MarkInfoDlg::UpdateProperties(bool positionOnly) {
  if (m_pRoutePoint) {
    m_textLatitude->SetValue(::toSDMM(1, m_pRoutePoint->m_lat));
    m_textLongitude->SetValue(::toSDMM(2, m_pRoutePoint->m_lon));
    m_lat_save = m_pRoutePoint->m_lat;
    m_lon_save = m_pRoutePoint->m_lon;
    m_textName->SetValue(m_pRoutePoint->GetName());
    m_textDescription->ChangeValue(m_pRoutePoint->m_MarkDescription);
    m_textCtrlExtDescription->ChangeValue(m_pRoutePoint->m_MarkDescription);
    m_checkBoxShowName->SetValue(m_pRoutePoint->m_bShowName);
    m_checkBoxShowNameExt->SetValue(m_pRoutePoint->m_bShowName);
    m_checkBoxVisible->SetValue(m_pRoutePoint->m_bIsVisible);
    m_checkBoxScaMin->SetValue(m_pRoutePoint->GetUseSca());
    m_textScaMin->SetValue(
        wxString::Format(wxT("%i"), (int)m_pRoutePoint->GetScaMin()));
    m_textCtrlGuid->SetValue(m_pRoutePoint->m_GUID);
    m_ChoiceWaypointRangeRingsNumber->SetSelection(
        m_pRoutePoint->GetWaypointRangeRingsNumber());
    wxString buf;
    buf.Printf(_T("%.3f"),
               toUsrDistance(m_pRoutePoint->GetWaypointRangeRingsStep(), -1));
    m_textWaypointRangeRingsStep->SetValue(buf);
    m_staticTextArrivalUnits->SetLabel(getUsrDistanceUnit());
    buf.Printf(_T("%.3f"),
               toUsrDistance(m_pRoutePoint->GetWaypointArrivalRadius(), -1));
    m_textArrivalRadius->SetValue(buf);

    int nUnits = m_pRoutePoint->GetWaypointRangeRingsStepUnits();
    m_RangeRingUnits->SetSelection(nUnits);

    wxColour col = m_pRoutePoint->m_wxcWaypointRangeRingsColour;
    m_PickColor->SetColour(col);

    if (m_comboBoxTideStation->GetStringSelection() !=
        m_pRoutePoint->m_TideStation) {
      m_comboBoxTideStation->Clear();
      m_comboBoxTideStation->Append(wxEmptyString);
      if (!m_pRoutePoint->m_TideStation.IsEmpty()) {
        m_comboBoxTideStation->Append(m_pRoutePoint->m_TideStation);
        m_comboBoxTideStation->SetSelection(1);
      }
    }

    if (m_pRoutePoint->GetPlannedSpeed() > .01) {
      m_textCtrlPlSpeed->SetValue(wxString::Format(
          "%.1f", toUsrSpeed(m_pRoutePoint->GetPlannedSpeed())));
    } else {
      m_textCtrlPlSpeed->SetValue(wxEmptyString);
    }

    wxDateTime etd;
    etd = m_pRoutePoint->GetManualETD();
    if (etd.IsValid()) {
      m_cbEtaPresent->SetValue(true);
      m_EtaDatePickerCtrl->SetValue(etd.GetDateOnly());
      m_EtaTimePickerCtrl->SetValue(etd);
    } else {
      m_cbEtaPresent->SetValue(false);
    }

    m_staticTextPlSpeed->Show(m_pRoutePoint->m_bIsInRoute);
    m_textCtrlPlSpeed->Show(m_pRoutePoint->m_bIsInRoute);
    m_staticTextEta->Show(m_pRoutePoint->m_bIsInRoute);
    m_EtaDatePickerCtrl->Show(m_pRoutePoint->m_bIsInRoute);
    m_EtaTimePickerCtrl->Show(m_pRoutePoint->m_bIsInRoute);
    m_cbEtaPresent->Show(m_pRoutePoint->m_bIsInRoute);
    m_staticTextPlSpeedUnits->Show(m_pRoutePoint->m_bIsInRoute);
    m_staticTextArrivalRadius->Show(m_pRoutePoint->m_bIsInRoute);
    m_staticTextArrivalUnits->Show(m_pRoutePoint->m_bIsInRoute);
    m_textArrivalRadius->Show(m_pRoutePoint->m_bIsInRoute);

    if (positionOnly) return true;

    // Layer or not?
    if (m_pRoutePoint->m_bIsInLayer) {
      m_staticTextLayer->Enable();
      m_staticTextLayer->Show(true);
      m_textName->SetEditable(false);
      m_textDescription->SetEditable(false);
      m_textCtrlExtDescription->SetEditable(false);
      m_textLatitude->SetEditable(false);
      m_textLongitude->SetEditable(false);
      m_bcomboBoxIcon->Enable(false);
      m_checkBoxShowName->Enable(false);
      m_checkBoxVisible->Enable(false);
      m_textArrivalRadius->SetEditable(false);
      m_checkBoxScaMin->Enable(false);
      m_textScaMin->SetEditable(false);
      m_checkBoxShowNameExt->Enable(false);
      m_ChoiceWaypointRangeRingsNumber->Enable(false);
      m_textWaypointRangeRingsStep->SetEditable(false);
      m_PickColor->Enable(false);
      DefaultsBtn->Enable(false);
      m_EtaDatePickerCtrl->Enable(false);
      m_EtaTimePickerCtrl->Enable(false);
      m_cbEtaPresent->Enable(false);
      m_notebookProperties->SetSelection(0);  // Show Basic page
      m_comboBoxTideStation->Enable(false);
    } else {
      m_staticTextLayer->Enable(false);
      m_staticTextLayer->Show(false);
      m_textName->SetEditable(true);
      m_textDescription->SetEditable(true);
      m_textCtrlExtDescription->SetEditable(true);
      m_textLatitude->SetEditable(true);
      m_textLongitude->SetEditable(true);
      m_bcomboBoxIcon->Enable(true);
      m_checkBoxShowName->Enable(true);
      m_checkBoxVisible->Enable(true);
      m_textArrivalRadius->SetEditable(true);
      m_checkBoxScaMin->Enable(true);
      m_textScaMin->SetEditable(true);
      m_checkBoxShowNameExt->Enable(true);
      m_ChoiceWaypointRangeRingsNumber->Enable(true);
      m_textWaypointRangeRingsStep->SetEditable(true);
      m_PickColor->Enable(true);
      DefaultsBtn->Enable(true);
      m_EtaDatePickerCtrl->Enable(true);
      m_EtaTimePickerCtrl->Enable(true);
      m_cbEtaPresent->Enable(true);
      m_notebookProperties->SetSelection(0);
      m_comboBoxTideStation->Enable(true);
    }

    // Fill the icon selector combo box
    m_bcomboBoxIcon->Clear();
    //      Iterate on the Icon Descriptions, filling in the combo control
    bool fillCombo = m_bcomboBoxIcon->GetCount() == 0;

    if (fillCombo) {
      for (int i = 0; i < pWayPointMan->GetNumIcons(); i++) {
        wxString* ps = pWayPointMan->GetIconDescription(i);
        wxBitmap bmp =
            pWayPointMan->GetIconBitmapForList(i, 2 * GetCharHeight());

        m_bcomboBoxIcon->Append(*ps, bmp);
      }
    }
    // find the correct item in the combo box
    int iconToSelect = -1;
    for (int i = 0; i < pWayPointMan->GetNumIcons(); i++) {
      if (*pWayPointMan->GetIconKey(i) == m_pRoutePoint->GetIconName()) {
        iconToSelect = i;
        m_bcomboBoxIcon->Select(iconToSelect);
        break;
      }
    }
    wxCommandEvent ev;
    OnShowWaypointNameSelectBasic(ev);
    OnWptRangeRingsNoChange(ev);
    OnSelectScaMinExt(ev);
    UpdateHtmlList();
  }

#ifdef __ANDROID__
  androidEnableBackButton(false);
#endif

  Fit();
  // SetMinSize(wxSize(-1, 600));
  RecalculateSize();

  return true;
}

/*!
 * Name changed event handler triggers validaton.
 */
void MarkInfoDlg::OnNameChanged(wxCommandEvent& event) {
  wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
  if (textCtrl) {
    if (!textCtrl->Validate()) {
      // Optional: Provide visual feedback or handle the error
      textCtrl->SetBackgroundColour(*wxRED);
      textCtrl->Refresh();
    } else {
      textCtrl->SetBackgroundColour(*wxWHITE);
      textCtrl->Refresh();
    }
  }
}

void MarkInfoDlg::OnBitmapCombClick(wxCommandEvent& event) {
  wxString* icon_name =
      pWayPointMan->GetIconKey(m_bcomboBoxIcon->GetSelection());
  if (icon_name && icon_name->Length()) m_pRoutePoint->SetIconName(*icon_name);
  m_pRoutePoint->ReLoadIcon();
  SaveChanges();
  // pConfig->UpdateWayPoint( m_pRoutePoint );
}

void MarkInfoDlg::ValidateMark(void) {
  //    Look in the master list of Waypoints to see if the currently selected
  //    waypoint is still valid It may have been deleted as part of a route
  wxRoutePointListNode* node = pWayPointMan->GetWaypointList()->GetFirst();

  bool b_found = false;
  while (node) {
    RoutePoint* rp = node->GetData();
    if (m_pRoutePoint == rp) {
      b_found = true;
      break;
    }
    node = node->GetNext();
  }
  if (!b_found) m_pRoutePoint = NULL;
}

bool MarkInfoDlg::SaveChanges() {
  if (m_pRoutePoint) {
    if (m_pRoutePoint->m_bIsInLayer) return true;

    // Get User input Text Fields
    m_pRoutePoint->SetName(m_textName->GetValue());
    m_pRoutePoint->SetWaypointArrivalRadius(m_textArrivalRadius->GetValue());
    m_pRoutePoint->SetScaMin(m_textScaMin->GetValue());
    m_pRoutePoint->SetUseSca(m_checkBoxScaMin->GetValue());
    m_pRoutePoint->m_MarkDescription = m_textDescription->GetValue();
    m_pRoutePoint->SetVisible(m_checkBoxVisible->GetValue());
    m_pRoutePoint->m_bShowName = m_checkBoxShowName->GetValue();
    m_pRoutePoint->SetPosition(fromDMM(m_textLatitude->GetValue()),
                               fromDMM(m_textLongitude->GetValue()));
    wxString* icon_name =
        pWayPointMan->GetIconKey(m_bcomboBoxIcon->GetSelection());
    if (icon_name && icon_name->Length())
      m_pRoutePoint->SetIconName(*icon_name);
    m_pRoutePoint->ReLoadIcon();
    m_pRoutePoint->SetShowWaypointRangeRings(
        (bool)(m_ChoiceWaypointRangeRingsNumber->GetSelection() != 0));
    m_pRoutePoint->SetWaypointRangeRingsNumber(
        m_ChoiceWaypointRangeRingsNumber->GetSelection());
    double value;
    if (m_textWaypointRangeRingsStep->GetValue().ToDouble(&value))
      m_pRoutePoint->SetWaypointRangeRingsStep(fromUsrDistance(value, -1));
    if (m_textArrivalRadius->GetValue().ToDouble(&value))
      m_pRoutePoint->SetWaypointArrivalRadius(fromUsrDistance(value, -1));

    if (m_RangeRingUnits->GetSelection() != wxNOT_FOUND)
      m_pRoutePoint->SetWaypointRangeRingsStepUnits(
          m_RangeRingUnits->GetSelection());

    m_pRoutePoint->m_TideStation = m_comboBoxTideStation->GetStringSelection();
    if (m_textCtrlPlSpeed->GetValue() == wxEmptyString) {
      m_pRoutePoint->SetPlannedSpeed(0.0);
    } else {
      double spd;
      if (m_textCtrlPlSpeed->GetValue().ToDouble(&spd)) {
        m_pRoutePoint->SetPlannedSpeed(fromUsrSpeed(spd));
      }
    }

    if (m_cbEtaPresent->GetValue()) {
      wxDateTime dt = m_EtaDatePickerCtrl->GetValue();
      dt.SetHour(m_EtaTimePickerCtrl->GetValue().GetHour());
      dt.SetMinute(m_EtaTimePickerCtrl->GetValue().GetMinute());
      dt.SetSecond(m_EtaTimePickerCtrl->GetValue().GetSecond());
      if (dt.IsValid()) {
        m_pRoutePoint->SetETD(dt.FormatISOCombined());
      }
    } else {
      m_pRoutePoint->SetETD(wxEmptyString);
    }

    if (m_pRoutePoint->m_bIsInRoute) {
      // Update the route segment selectables
      pSelect->UpdateSelectableRouteSegments(m_pRoutePoint);

      // Get an array of all routes using this point
      wxArrayPtrVoid* pEditRouteArray =
          g_pRouteMan->GetRouteArrayContaining(m_pRoutePoint);

      if (pEditRouteArray) {
        for (unsigned int ir = 0; ir < pEditRouteArray->GetCount(); ir++) {
          Route* pr = (Route*)pEditRouteArray->Item(ir);
          pr->FinalizeForRendering();
          pr->UpdateSegmentDistances();

          pConfig->UpdateRoute(pr);
        }
        delete pEditRouteArray;
      }
    } else
      pConfig->UpdateWayPoint(m_pRoutePoint);
    // No general settings need be saved pConfig->UpdateSettings();
  }
  return true;
}

SaveDefaultsDialog::SaveDefaultsDialog(MarkInfoDlg* parent)
    : wxDialog(parent, wxID_ANY, _("Save some defaults")) {
  //(*Initialize(SaveDefaultsDialog)
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
  wxStdDialogButtonSizer* StdDialogButtonSizer1;

  StaticText1 =
      new wxStaticText(this, wxID_ANY,
                       _("Check which properties of current waypoint\n should "
                         "be set as default for NEW waypoints."));
  bSizer1->Add(StaticText1, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

  wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer(2);

  wxString s =
      (g_pMarkInfoDialog->m_checkBoxShowName->GetValue() ? _("Do use")
                                                         : _("Don't use"));
  NameCB =
      new wxCheckBox(this, wxID_ANY, _("Show Waypoint Name"), wxDefaultPosition,
                     wxDefaultSize, 0, wxDefaultValidator);
  fgSizer1->Add(NameCB, 0, wxALL, 5);
  stName = new wxStaticText(this, wxID_ANY, _T("[") + s + _T("]"),
                            wxDefaultPosition, wxDefaultSize, 0);
  stName->Wrap(-1);
  fgSizer1->Add(stName, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

  s = g_pMarkInfoDialog->m_pRoutePoint->GetIconName();
  IconCB = new wxCheckBox(this, wxID_ANY, _("Icon"));
  fgSizer1->Add(IconCB, 0, wxALL, 5);
  stIcon = new wxStaticText(this, wxID_ANY, _T("[") + s + _T("]"),
                            wxDefaultPosition, wxDefaultSize, 0);
  stIcon->Wrap(-1);
  fgSizer1->Add(stIcon, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

  s = (g_pMarkInfoDialog->m_ChoiceWaypointRangeRingsNumber->GetSelection()
           ? _("Do use") +
                 wxString::Format(
                     _T(" (%i) "),
                     g_pMarkInfoDialog->m_ChoiceWaypointRangeRingsNumber
                         ->GetSelection())
           : _("Don't use"));
  RangRingsCB = new wxCheckBox(this, wxID_ANY, _("Range rings"));
  fgSizer1->Add(RangRingsCB, 0, wxALL, 5);
  stRR = new wxStaticText(this, wxID_ANY, _T("[") + s + _T("]"),
                          wxDefaultPosition, wxDefaultSize, 0);
  stRR->Wrap(-1);
  fgSizer1->Add(stRR, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

  s = (g_pMarkInfoDialog->m_textArrivalRadius->GetValue());
  ArrivalRCB = new wxCheckBox(this, wxID_ANY, _("Arrival radius"));
  fgSizer1->Add(ArrivalRCB, 0, wxALL, 5);
  stArrivalR = new wxStaticText(
      this, wxID_ANY,
      wxString::Format(_T("[%s %s]"), s.c_str(), getUsrDistanceUnit().c_str()),
      wxDefaultPosition, wxDefaultSize, 0);
  stArrivalR->Wrap(-1);
  fgSizer1->Add(stArrivalR, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL,
                5);

  s = (g_pMarkInfoDialog->m_checkBoxScaMin->GetValue()
           ? _("Show only if") + _T(" < ") +
                 g_pMarkInfoDialog->m_textScaMin->GetValue()
           : _("Show always"));
  ScaleCB = new wxCheckBox(this, wxID_ANY, _("Show only at scale"));
  fgSizer1->Add(ScaleCB, 0, wxALL, 5);
  stScale = new wxStaticText(this, wxID_ANY, _T("[") + s + _T("]"),
                             wxDefaultPosition, wxDefaultSize, 0);
  stScale->Wrap(-1);
  fgSizer1->Add(stScale, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

  bSizer1->Add(fgSizer1, 0, wxALL | wxEXPAND, 5);

  StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
  StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK));
  StdDialogButtonSizer1->AddButton(
      new wxButton(this, wxID_CANCEL, _("Cancel")));
  StdDialogButtonSizer1->Realize();
  bSizer1->Add(StdDialogButtonSizer1, 0, wxALL | wxEXPAND, 5);

  SetSizer(bSizer1);
  Fit();
  Layout();

#ifdef __ANDROID__
  SetSize(parent->GetSize());
#endif

  Center();
}

void MarkInfoDlg::ShowTidesBtnClicked(wxCommandEvent& event) {
  if (m_comboBoxTideStation->GetSelection() < 1) {
    return;
  }
  IDX_entry* pIDX = (IDX_entry*)ptcmgr->GetIDX_entry(
      ptcmgr->GetStationIDXbyName(m_comboBoxTideStation->GetStringSelection(),
                                  fromDMM(m_textLatitude->GetValue()),
                                  fromDMM(m_textLongitude->GetValue())));
  if (pIDX) {
    TCWin* pCwin = new TCWin(gFrame->GetPrimaryCanvas(), 0, 0, pIDX);
    pCwin->Show();
  } else {
    wxString msg(_("Tide Station not found"));
    msg += _T(":\n");
    msg += m_comboBoxTideStation->GetStringSelection();
    OCPNMessageBox(NULL, msg, _("OpenCPN Info"), wxOK | wxCENTER, 10);
  }
}
