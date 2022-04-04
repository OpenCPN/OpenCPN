/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  MUI Control Bar
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 *
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <wx/statline.h>

#include "dychart.h"

#include "chcanv.h"
#include "MUIBar.h"
#include "OCPNPlatform.h"
#include "CanvasOptions.h"
#include "styles.h"
#include "navutil.h"
#include "svg_utils.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------

extern OCPNPlatform* g_Platform;
extern ChartCanvas* g_focusCanvas;
extern ocpnStyle::StyleManager* g_StyleManager;
extern bool g_bShowMuiZoomButtons;

double getValue(int animationType, double t);

//  Helper classes

#define ID_SCALE_CANCEL 8301
#define ID_SCALE_OK 8302
#define ID_SCALECTRL 8303

class SetScaleDialog : public wxDialog {
  DECLARE_EVENT_TABLE()

public:
  /// Constructors
  SetScaleDialog();
  SetScaleDialog(wxWindow* parent, wxWindowID id = SYMBOL_GOTOPOS_IDNAME,
                 const wxString& caption = _("Set scale"),
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxDEFAULT_DIALOG_STYLE);

  ~SetScaleDialog();

  /// Creation
  bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
              const wxString& caption = _("Set scale"),
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = wxDEFAULT_DIALOG_STYLE);

  void SetColorScheme(ColorScheme cs);

  void CreateControls();

  void OnSetScaleCancelClick(wxCommandEvent& event);
  void OnSetScaleOKClick(wxCommandEvent& event);

  /// Should we show tooltips?

  wxTextCtrl* m_ScaleCtl;
  wxButton* m_CancelButton;
  wxButton* m_OKButton;
};

BEGIN_EVENT_TABLE(SetScaleDialog, wxDialog)
EVT_BUTTON(ID_GOTOPOS_CANCEL, SetScaleDialog::OnSetScaleCancelClick)
EVT_BUTTON(ID_GOTOPOS_OK, SetScaleDialog::OnSetScaleOKClick)
END_EVENT_TABLE()

/*!
 * SetScaleDialog constructors
 */

SetScaleDialog::SetScaleDialog() {}

SetScaleDialog::SetScaleDialog(wxWindow* parent, wxWindowID id,
                               const wxString& caption, const wxPoint& pos,
                               const wxSize& size, long style) {
  long wstyle =
      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxFRAME_FLOAT_ON_PARENT;

  Create(parent, id, caption, pos, size, wstyle);
}

SetScaleDialog::~SetScaleDialog() {}

/*!
 * SetScaleDialog creator
 */

bool SetScaleDialog::Create(wxWindow* parent, wxWindowID id,
                            const wxString& caption, const wxPoint& pos,
                            const wxSize& size, long style) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls();
  GetSizer()->SetSizeHints(this);
  Centre();

  return TRUE;
}

/*!
 * Control creation for GoToPositionDialog
 */

void SetScaleDialog::CreateControls() {
  SetScaleDialog* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemDialog1->SetSizer(itemBoxSizer2);

  wxStaticBox* itemStaticBoxSizer4Static =
      new wxStaticBox(itemDialog1, wxID_ANY, _("Chart Scale"));

  wxStaticBoxSizer* itemStaticBoxSizer4 =
      new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
  itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxEXPAND | wxALL, 5);

  wxStaticText* itemStaticText5 = new wxStaticText(
      itemDialog1, wxID_STATIC, _T(""), wxDefaultPosition, wxDefaultSize, 0);
  itemStaticBoxSizer4->Add(itemStaticText5, 0,
                           wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  m_ScaleCtl = new wxTextCtrl(itemDialog1, ID_SCALECTRL, _T(""),
                              wxDefaultPosition, wxSize(180, -1), 0);
  itemStaticBoxSizer4->Add(
      m_ScaleCtl, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_GOTOPOS_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_GOTOPOS_OK, _("OK"),
                            wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();

  SetColorScheme((ColorScheme)0);
}

void SetScaleDialog::SetColorScheme(ColorScheme cs) { DimeControl(this); }

void SetScaleDialog::OnSetScaleCancelClick(wxCommandEvent& event) {
  Close();
  event.Skip();
}

void SetScaleDialog::OnSetScaleOKClick(wxCommandEvent& event) {
  SetReturnCode(wxID_OK);
  EndModal(wxID_OK);
  return;
}

//------------------------------------------------------------------------------
//    MUIButton Static Storage
//------------------------------------------------------------------------------

class MUIButton : public wxWindow {
  DECLARE_DYNAMIC_CLASS(MUIButton)
  DECLARE_EVENT_TABLE()

  wxSize DoGetBestSize() const;

public:
  MUIButton();
  MUIButton(wxWindow* parent, wxWindowID id = wxID_ANY,
            float scale_factor = 1.0,
            const wxString& bitmapState0 = wxEmptyString,
            const wxString& bitmapState1 = wxEmptyString,
            const wxString& bitmapState2 = wxEmptyString,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = wxNO_BORDER);

  bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
              float scale_factor = 1.0,
              const wxString& bitmapState0 = wxEmptyString,
              const wxString& bitmapState1 = wxEmptyString,
              const wxString& bitmapState2 = wxEmptyString,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize, long style = wxNO_BORDER);

  ~MUIButton();

  void Init();
  void CreateControls();

  void SetState(int state);

  void SetColorScheme(ColorScheme cs);
  void OnSize(wxSizeEvent& event);
  void OnPaint(wxPaintEvent& event);
  void OnLeftDown(wxMouseEvent& event);
  void OnLeftUp(wxMouseEvent& event);

  wxBitmap GetBitmapResource(const wxString& name);

  wxIcon GetIconResource(const wxString& name);

  /// Should we show tooltips?
  static bool ShowToolTips();

private:
  wxString m_bitmapFileState0;
  wxString m_bitmapFileState1;
  wxString m_bitmapFileState2;
  wxBitmap m_bitmap;
  wxBitmap m_bitmapState0;
  wxBitmap m_bitmapState1;
  wxBitmap m_bitmapState2;

  int mState;
  float m_scaleFactor;
  wxSize m_styleToolSize;
  ColorScheme m_cs;
};

IMPLEMENT_DYNAMIC_CLASS(MUIButton, wxWindow)

BEGIN_EVENT_TABLE(MUIButton, wxWindow)

EVT_SIZE(MUIButton::OnSize)
EVT_PAINT(MUIButton::OnPaint)
EVT_LEFT_DOWN(MUIButton::OnLeftDown)
EVT_LEFT_UP(MUIButton::OnLeftUp)

END_EVENT_TABLE()

MUIButton::MUIButton() { Init(); }

MUIButton::MUIButton(wxWindow* parent, wxWindowID id, float scale_factor,
                     const wxString& bitmap, const wxString& bitmapState1,
                     const wxString& bitmapState2, const wxPoint& pos,
                     const wxSize& size, long style) {
  Init();
  Create(parent, id, scale_factor, bitmap, bitmapState1, bitmapState2, pos,
         size, style);
}

bool MUIButton::Create(wxWindow* parent, wxWindowID id, float scale_factor,
                       const wxString& bitmap, const wxString& bitmapState1,
                       const wxString& bitmapState2, const wxPoint& pos,
                       const wxSize& size, long style) {
  wxWindow::Create(parent, id, pos, size, style);
  m_bitmapFileState0 = bitmap;
  m_bitmapFileState1 = bitmapState1;
  m_bitmapFileState2 = bitmapState2;

  m_scaleFactor = scale_factor;

  m_styleToolSize = g_StyleManager->GetCurrentStyle()->GetToolSize();

  //  Arbitrarily boost the MUIButton default size above the style defined size.
  //  No good reason.....
  m_styleToolSize = wxSize(m_styleToolSize.x * 1.25, m_styleToolSize.y * 1.25);

  CreateControls();
  return true;
}

MUIButton::~MUIButton() {}

void MUIButton::Init() {
  mState = 0;
  m_cs = (ColorScheme)-1;  // GLOBAL_COLOR_SCHEME_RGB;
}

void MUIButton::CreateControls() {
  this->SetForegroundColour(wxColour(255, 255, 255));

  wxColour backColor = GetGlobalColor(_T("GREY3"));
  SetBackgroundColour(backColor);

  this->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                       wxFONTWEIGHT_BOLD, false, wxT("Tahoma")));
}

void MUIButton::SetColorScheme(ColorScheme cs) {
  if (m_cs != cs) {
    wxColour backColor = GetGlobalColor(_T("GREY3"));
    SetBackgroundColour(backColor);

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    wxBitmap bmp = LoadSVG(m_bitmapFileState0, GetSize().x, GetSize().y);
    m_bitmapState0 = style->SetBitmapBrightness(bmp, cs);

    bmp = LoadSVG(m_bitmapFileState1, GetSize().x, GetSize().y);
    if (bmp.IsOk())
      m_bitmapState1 = style->SetBitmapBrightness(bmp, cs);
    else
      m_bitmapState1 = m_bitmapState0;

    bmp = LoadSVG(m_bitmapFileState2, GetSize().x, GetSize().y);
    if (bmp.IsOk())
      m_bitmapState2 = style->SetBitmapBrightness(bmp, cs);
    else
      m_bitmapState2 = m_bitmapState0;

    switch (mState) {
      case 0:
      default:
        m_bitmap = m_bitmapState0;
        break;

      case 1:
        m_bitmap = m_bitmapState1;
        break;

      case 2:
        m_bitmap = m_bitmapState2;
        break;
    }

    m_cs = cs;
  }
}

bool MUIButton::ShowToolTips() { return true; }

void MUIButton::SetState(int state) {
  switch (state) {
    case 0:
    default:
      m_bitmap = m_bitmapState0;
      break;

    case 1:
      m_bitmap = m_bitmapState1;
      break;

    case 2:
      m_bitmap = m_bitmapState2;
      break;
  }

  mState = state;

  Refresh();
}

void MUIButton::OnSize(wxSizeEvent& event) {
  if (m_bitmap.IsOk()) {
    if (event.GetSize() == m_bitmap.GetSize()) return;
  }

  if (!m_bitmapFileState0.IsEmpty())
    m_bitmapState0 =
        LoadSVG(m_bitmapFileState0, event.GetSize().x, event.GetSize().y);

  if (!m_bitmapFileState1.IsEmpty())
    m_bitmapState1 =
        LoadSVG(m_bitmapFileState1, event.GetSize().x, event.GetSize().y);
  if (!m_bitmapState1.IsOk() || m_bitmapFileState1.IsEmpty())
    m_bitmapState1 = m_bitmapState0;

  if (!m_bitmapFileState2.IsEmpty())
    m_bitmapState2 =
        LoadSVG(m_bitmapFileState2, event.GetSize().x, event.GetSize().y);
  if (!m_bitmapState2.IsOk() || m_bitmapFileState2.IsEmpty())
    m_bitmapState2 = m_bitmapState0;

  switch (mState) {
    case 0:
    default:
      m_bitmap = m_bitmapState0;
      break;

    case 1:
      m_bitmap = m_bitmapState1;
      break;

    case 2:
      m_bitmap = m_bitmapState2;
      break;
  }
}

wxBitmap MUIButton::GetBitmapResource(const wxString& name) {
  // Bitmap retrieval
  wxUnusedVar(name);
  return wxNullBitmap;
}

wxIcon MUIButton::GetIconResource(const wxString& name) {
  // Icon retrieval
  wxUnusedVar(name);
  return wxNullIcon;
}

wxSize MUIButton::DoGetBestSize() const {
  //     wxSize labelSize = wxDefaultSize;
  //     GetTextExtent(m_Label, &labelSize.x, &labelSize.y);
  //     return wxSize(wxMax(40, labelSize.x + 20), wxMax(20, labelSize.y +
  //     10));
  return wxSize(m_styleToolSize.x * m_scaleFactor,
                m_styleToolSize.y * m_scaleFactor);
}

void MUIButton::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(this);

  if (m_bitmap.IsOk()) {
    dc.DrawBitmap(m_bitmap, 0, 0, true);
  }

#if 0
    wxBufferedPaintDC dc(this);

    wxRect clientRect = GetClientRect();
    wxRect gradientRect = clientRect;
    gradientRect.SetHeight(gradientRect.GetHeight()/2 + ((GetCapture() == this) ? 1 : 0));
    if(GetCapture() != this)
    {
        dc.GradientFillLinear(gradientRect,
                              m_GradientTopStartColour, m_GradientTopEndColour, wxSOUTH);
    }
    else
    {
        dc.SetPen(wxPen(m_PressedColourTop));
        dc.SetBrush(wxBrush(m_PressedColourTop));
        dc.DrawRectangle(gradientRect);
    }

    gradientRect.Offset(0, gradientRect.GetHeight());

    if(GetCapture() != this)
    {
        dc.GradientFillLinear(gradientRect,
                              m_GradientBottomStartColour, m_GradientBottomEndColour, wxSOUTH);
    }
    else
    {
        dc.SetPen(wxPen(m_PressedColourBottom));
        dc.SetBrush(wxBrush(m_PressedColourBottom));
        dc.DrawRectangle(gradientRect);
    }
    dc.SetPen(wxPen(GetBackgroundColour()));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, clientRect.GetWidth(), clientRect.GetHeight());
    dc.SetFont(GetFont());
    dc.SetTextForeground(GetForegroundColour());
    if(GetCapture() == this)
    {
        clientRect.Offset(1, 1);
    }
    dc.DrawLabel(m_Label, clientRect, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
#endif
}

#if 1
void MUIButton::OnLeftDown(wxMouseEvent& event) { event.Skip(); }

void MUIButton::OnLeftUp(wxMouseEvent& event) {
  if (GetClientRect().Contains(event.GetPosition())) {
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, GetId());
    GetParent()->GetEventHandler()->AddPendingEvent(evt);
  }
  event.Skip();
}
#endif

#define CANVAS_OPTIONS_ANIMATION_TIMER_1 800
#define CANVAS_OPTIONS_TIMER 801

//------------------------------------------------------------------------------
//          MUIBar Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(MUIBar, wxFrame)
EVT_TIMER(CANVAS_OPTIONS_ANIMATION_TIMER_1,
          MUIBar::onCanvasOptionsAnimationTimerEvent)
// EVT_PAINT ( MUIBar::OnPaint )
EVT_SIZE(MUIBar::OnSize)
EVT_MENU(-1, MUIBar::OnToolLeftClick)
EVT_TIMER(CANVAS_OPTIONS_TIMER, MUIBar::CaptureCanvasOptionsBitmapChain)

END_EVENT_TABLE()

// Define a constructor
MUIBar::MUIBar() {}

MUIBar::MUIBar(ChartCanvas* parent, int orientation, float size_factor,
               wxWindowID id, const wxPoint& pos, const wxSize& size,
               long style, const wxString& name) {
  m_parentCanvas = parent;
  m_orientation = orientation;

  // SetBackgroundStyle( wxBG_STYLE_TRANSPARENT );
  // wxWindow::Create(parent, id, pos, size, style, name);
  // long mstyle = wxSIMPLE_BORDER;
  long mstyle = wxNO_BORDER | wxFRAME_NO_TASKBAR | wxFRAME_SHAPED |
                wxFRAME_FLOAT_ON_PARENT | wxFRAME_TOOL_WINDOW;

  m_scaleFactor = size_factor;
  m_cs = (ColorScheme)-1;

  wxFrame::Create(parent, id, _T(""), pos, size, mstyle, name);
  Init();
  CreateControls();
  // Show();
}

MUIBar::~MUIBar() {
  if (m_canvasOptions) {
    m_canvasOptions->Destroy();
    m_canvasOptions = 0;
  }
  if (m_scaleTextBox)
    m_scaleTextBox->Unbind(wxEVT_LEFT_DOWN, &MUIBar::OnScaleSelected, this);
}

void MUIBar::Init() {
  m_zinButton = NULL;
  m_zoutButton = NULL;
  m_followButton = NULL;
  m_menuButton = NULL;

  m_canvasOptions = NULL;
  m_canvasOptionsAnimationTimer.SetOwner(this,
                                         CANVAS_OPTIONS_ANIMATION_TIMER_1);
  m_backcolorString = _T("GREY3");
  m_scaleTextBox = NULL;
  m_capture_size_y = 0;

  m_COTopOffset = 60;  //  TODO should be below GPS/Compass

  CanvasOptionTimer.SetOwner(this, CANVAS_OPTIONS_TIMER);
  m_coAnimateByBitmaps = false;
  m_bEffects = true;
#ifdef __OCPN__ANDROID__
  m_bEffects = false;
#endif
}

void MUIBar::SetColorScheme(ColorScheme cs) {
  if (m_cs != cs) {
    wxColour backColor = GetGlobalColor(m_backcolorString);
    SetBackgroundColour(backColor);

    if (m_zinButton) m_zinButton->SetColorScheme(cs);
    if (m_zoutButton) m_zoutButton->SetColorScheme(cs);
    if (m_followButton) m_followButton->SetColorScheme(cs);
    if (m_menuButton) m_menuButton->SetColorScheme(cs);

    if (m_scaleTextBox) {
      wxColour textbackColor = GetGlobalColor(_T("GREY1"));
      m_scaleTextBox->SetForegroundColour(textbackColor);
    }
    Refresh();
    m_cs = cs;
  }
}

void MUIBar::OnScaleSelected(wxMouseEvent& event) {
  ChartCanvas* pcc = wxDynamicCast(m_parent, ChartCanvas);
  if (!pcc) return;

  SetScaleDialog dlg(pcc);
  dlg.Centre();
  dlg.ShowModal();
  if (dlg.GetReturnCode() == wxID_OK) {
    wxString newScale = dlg.m_ScaleCtl->GetValue();
    if (newScale.Contains(':')) newScale = newScale.AfterFirst(':');
    double dScale;
    if (newScale.ToDouble(&dScale)) {
      // Try to constrain the scale to something reasonable
      dScale = wxMin(dScale, 3e6);
      dScale = wxMax(dScale, 1000);
      double displayScaleNow = pcc->GetScaleValue();
      double factor = displayScaleNow / dScale;
      pcc->DoZoomCanvas(factor, false);

      // Run the calculation again, to reduce roundoff error in large scale
      // jumps.
      displayScaleNow = pcc->GetScaleValue();
      factor = displayScaleNow / dScale;
      pcc->DoZoomCanvas(factor, false);
    }
  }
}

void MUIBar::SetCanvasENCAvailable(bool avail) {
  m_CanvasENCAvail = avail;
  if (m_canvasOptions) m_canvasOptions->SetENCAvailable(avail);
}

void MUIBar::CreateControls() {
  // SetBackgroundStyle( wxBG_STYLE_TRANSPARENT );

  wxColour backColor = GetGlobalColor(m_backcolorString);
  SetBackgroundColour(backColor);
  wxBoxSizer* topSizer;

  wxString iconDir = g_Platform->GetSharedDataDir() + _T("uidata/MUI_flat/");

  if (m_orientation == wxHORIZONTAL) {
    topSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topSizer);

    wxBoxSizer* barSizer = new wxBoxSizer(wxHORIZONTAL);
    topSizer->Add(barSizer, 0, wxEXPAND);

    // Buttons

    if (g_bShowMuiZoomButtons) {
      m_zinButton = new MUIButton(this, ID_ZOOMIN, m_scaleFactor,
                                  iconDir + _T("MUI_zoom-in.svg"));
      barSizer->Add(m_zinButton, 0, wxSHAPED);

      m_zoutButton = new MUIButton(this, ID_ZOOMOUT, m_scaleFactor,
                                   iconDir + _T("MUI_zoom-out.svg"));
      barSizer->Add(m_zoutButton, 0, wxSHAPED);

      barSizer->AddSpacer(2);
    }

#ifndef __OCPN__ANDROID__
    //  Scale
    m_scaleTextBox = new wxStaticText(this, wxID_ANY, _T("1:400000"));
    wxColour textbackColor = GetGlobalColor(_T("GREY1"));
    m_scaleTextBox->SetForegroundColour(textbackColor);
    barSizer->Add(m_scaleTextBox, 0, wxALIGN_CENTER_VERTICAL);
    m_scaleTextBox->Bind(wxEVT_LEFT_DOWN, &MUIBar::OnScaleSelected, this);

    barSizer->AddSpacer(5);

    m_followButton = new MUIButton(this, ID_FOLLOW, m_scaleFactor,
                                   iconDir + _T("MUI_follow.svg"),
                                   iconDir + _T("MUI_follow_active.svg"),
                                   iconDir + _T("MUI_follow_ahead.svg"));
    barSizer->Add(m_followButton, 0, wxSHAPED);

    barSizer->AddSpacer(2);
#endif
    m_menuButton = new MUIButton(this, ID_MUI_MENU, m_scaleFactor,
                                 iconDir + _T("MUI_menu.svg"));
    barSizer->Add(m_menuButton, 0, wxSHAPED);
  } else {
    topSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topSizer);

    wxBoxSizer* barSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(barSizer, 0, wxEXPAND);

    // Buttons
    if (g_bShowMuiZoomButtons) {
      m_zinButton = new MUIButton(this, ID_ZOOMIN, m_scaleFactor,
                                  iconDir + _T("MUI_zoom-in.svg"));
      barSizer->Add(m_zinButton, 1, wxSHAPED);

      m_zoutButton = new MUIButton(this, ID_ZOOMOUT, m_scaleFactor,
                                   iconDir + _T("MUI_zoom-out.svg"));
      barSizer->Add(m_zoutButton, 1, wxSHAPED);

      barSizer->AddSpacer(5);
    }

#ifndef __OCPN__ANDROID__
    m_followButton = new MUIButton(this, ID_FOLLOW, m_scaleFactor,
                                   iconDir + _T("MUI_follow.svg"),
                                   iconDir + _T("MUI_follow_active.svg"),
                                   iconDir + _T("MUI_follow_ahead.svg"));
    barSizer->Add(m_followButton, 1, wxSHAPED);

    barSizer->AddSpacer(5);
#endif

    m_menuButton = new MUIButton(this, ID_MUI_MENU, m_scaleFactor,
                                 iconDir + _T("MUI_menu.svg"));
    barSizer->Add(m_menuButton, 1, wxALIGN_RIGHT | wxSHAPED);
  }

  // topSizer->SetSizeHints( this );
  topSizer->Fit(this);
}

void MUIBar::SetBestPosition(void) {
#if 0  // for wxWindow
    int x = (m_parent->GetClientSize().x - GetSize().x) / 2;
    if(x > 0){
        int bottomOffset = 0;

        ChartCanvas *pcc = wxDynamicCast(m_parent, ChartCanvas);
        bottomOffset += pcc->GetPianoHeight();

        int y = m_parent->GetClientSize().y - GetSize().y - bottomOffset;
        SetSize(x, y, -1, -1, wxSIZE_USE_EXISTING);
    }

#else  // for wxDialog
  int x = (m_parent->GetClientSize().x - (GetSize().x * 1.00));

#ifndef __WXGTK__  // Adjust for wxNO_BORDER canvas window style
  x -= 2;
#endif

  // if(x > 0)
  {
    int bottomOffset = 2;

    ChartCanvas* pcc = wxDynamicCast(m_parent, ChartCanvas);
    //         bottomOffset += pcc->GetPianoHeight();

    int y = m_parent->GetClientSize().y - GetSize().y - bottomOffset;

    wxPoint m_position = wxPoint(x, y);
    wxPoint screenPos = pcc->ClientToScreen(m_position);

    //  GTK sometimes has trouble with ClientToScreen() if executed in the
    //  context of an event handler The position of the window is calculated
    //  incorrectly if a deferred Move() has not been processed yet. So work
    //  around this here...

#ifdef __WXGTK__
    wxPoint pp = m_parent->GetPosition();
    wxPoint ppg = m_parent->GetParent()->GetScreenPosition();
    wxPoint screen_pos_fix = ppg + pp + m_position;
    screenPos.x = screen_pos_fix.x;
#endif

    Move(screenPos);

    if (m_canvasOptions) {
      m_canvasOptions->Destroy();
      m_canvasOptions = 0;
    }
    Show();
  }
#endif
}

void MUIBar::OnSize(wxSizeEvent& event) {
  // int buttonSize = event.GetSize().y / 2;
  Layout();

#if !defined(__WXMAC__) && !defined(__OCPN__ANDROID__)
  if (1) {
    wxBitmap m_MaskBmp = wxBitmap(GetSize().x, GetSize().y);
    wxMemoryDC sdc(m_MaskBmp);
    sdc.SetBackground(*wxWHITE_BRUSH);
    sdc.Clear();
    sdc.SetBrush(*wxBLACK_BRUSH);
    sdc.SetPen(*wxBLACK_PEN);
    sdc.DrawRoundedRectangle(0, 0, m_MaskBmp.GetWidth(), m_MaskBmp.GetHeight(),
                             5);
    sdc.SelectObject(wxNullBitmap);
    SetShape(wxRegion(m_MaskBmp, *wxWHITE, 0));
  }
#endif
}

void MUIBar::UpdateDynamicValues() {
  if (!m_scaleTextBox) return;

  wxString scaleString;
  int scale = m_parentCanvas->GetScaleValue();
  if (scale < 1e6)
    scaleString.Printf(_T("1:%d"), scale);
  else
    scaleString.Printf(_T("1:%4.1f MM"), scale / 1e6);

  if (m_scaleTextBox) m_scaleTextBox->SetLabel(scaleString);
}

void MUIBar::SetFollowButtonState(int state) {
  if (m_followButton) m_followButton->SetState(state);
}

void MUIBar::OnToolLeftClick(wxCommandEvent& event) {
  //  Handle the MUIButton clicks here
  ChartCanvas* pcc = wxDynamicCast(m_parent, ChartCanvas);

  switch (event.GetId()) {
    case ID_ZOOMIN:
    case ID_ZOOMOUT: {
      wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, event.GetId());
      GetParent()->GetEventHandler()->AddPendingEvent(evt);

      if (g_focusCanvas) g_focusCanvas->TriggerDeferredFocus();
      break;
    }

    case ID_FOLLOW: {
      wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, event.GetId());
      GetParent()->GetEventHandler()->AddPendingEvent(evt);

      if (g_focusCanvas) g_focusCanvas->TriggerDeferredFocus();
      break;
    }

    case ID_MUI_MENU: {
      if (!m_canvasOptions) {
        m_canvasOptions = new CanvasOptions(m_parent);

        // calculate best size for CanvasOptions dialog

        wxPoint parentClientUpperRight =
            m_parent->ClientToScreen(wxPoint(m_parent->GetSize().x, 0));
        wxRect rmui = m_parentCanvas->GetMUIBarRect();
        int size_y = rmui.y - (parentClientUpperRight.y + m_COTopOffset);
        size_y -= GetCharHeight();
        size_y = wxMax(size_y, 100);  // ensure always big enough to see

        m_canvasOptions->SetSize(wxSize(-1, size_y));
        m_canvasOptionsFullSize = m_canvasOptions->GetSize();
        m_canvasOptionsFullSize.x +=
            m_canvasOptions->GetCharWidth();  // Allow for scroll bar, since
                                              // sizer won't do it.

        if (1)
          m_currentCOPos = m_parent->ClientToScreen(
              wxPoint(m_parent->GetSize().x, m_COTopOffset));
        else
          m_currentCOPos = wxPoint(m_parent->GetSize().x, 20);

        m_canvasOptions->Move(m_currentCOPos);
        m_canvasOptions->Hide();
      }

      m_canvasOptions->SetENCAvailable(m_CanvasENCAvail);

      if (m_canvasOptions->IsShown())
        PushCanvasOptions();  // hide it
      else {
        // Grab the backing bitmap, if available

        if (m_coAnimateByBitmaps && m_capture_size_y) {
          int overShoot_x = m_canvasOptions->GetSize().x * 2 / 10;  // 20%
          m_backingPoint =
              wxPoint(m_capturePoint.x - overShoot_x, m_capturePoint.y);

          m_backingBitmap = wxBitmap(m_canvasOptionsFullSize.x + overShoot_x,
                                     m_capture_size_y, -1);
          wxMemoryDC mdcb;
          mdcb.SelectObject(m_backingBitmap);
          wxScreenDC sdc;
          mdcb.Blit(0, 0, m_canvasOptionsFullSize.x + overShoot_x,
                    m_capture_size_y, &sdc, m_capturePoint.x - overShoot_x,
                    m_capturePoint.y, wxCOPY);
          mdcb.SelectObject(wxNullBitmap);
        }
        PullCanvasOptions();
      }

      break;
    }

    default:
      break;
  }
  // event.Skip();
}

void MUIBar::CaptureCanvasOptionsBitmap() {
  m_coSequence = 0;
  CanvasOptionTimer.Start(100, wxTIMER_ONE_SHOT);
}

void MUIBar::CaptureCanvasOptionsBitmapChain(wxTimerEvent& event) {
  if (m_coSequence == 0) {
    if (!m_canvasOptions) m_canvasOptions = new CanvasOptions(m_parent);

    wxPoint parentClientUpperRight =
        m_parent->ClientToScreen(wxPoint(m_parent->GetSize().x, 0));
    wxRect rmui = m_parentCanvas->GetMUIBarRect();
    int size_y = rmui.y - (parentClientUpperRight.y + m_COTopOffset);
    size_y -= GetCharHeight();
    size_y = wxMax(size_y, 100);  // ensure always big enough to see
    m_capture_size_y = size_y;

    m_canvasOptions->SetSize(wxSize(-1, size_y));

    m_capturePoint =
        m_parent->ClientToScreen(wxPoint(m_parent->GetSize().x, m_COTopOffset));
    m_canvasOptions->Move(m_capturePoint);
    m_canvasOptions->Show();

    m_coSequence++;
    CanvasOptionTimer.Start(1, wxTIMER_ONE_SHOT);
  }

  else if (m_coSequence == 1) {
    m_capturePoint = m_parent->ClientToScreen(wxPoint(
        m_parent->GetSize().x - m_canvasOptionsFullSize.x, m_COTopOffset));
    m_canvasOptions->Move(m_capturePoint);

    m_coSequence++;
    CanvasOptionTimer.Start(1, wxTIMER_ONE_SHOT);
  }

  else if (m_coSequence == 2) {
    m_animateBitmap =
        wxBitmap(m_canvasOptions->GetSize().x, m_capture_size_y, -1);
    wxMemoryDC mdc(m_animateBitmap);

    wxScreenDC sdc;

    mdc.Blit(0, 0, m_canvasOptions->GetSize().x, m_capture_size_y, &sdc,
             m_capturePoint.x, m_capturePoint.y, wxCOPY);
    mdc.SelectObject(wxNullBitmap);

    // delete m_canvasOptions;
    // m_canvasOptions = NULL;
  }
}

void MUIBar::OnEraseBackground(wxEraseEvent& event) {}

void MUIBar::OnPaint(wxPaintEvent& event) {
  return;

  int width, height;
  GetClientSize(&width, &height);
  wxPaintDC dc(this);

  //    dc.SetBackgroundMode(wxTRANSPARENT);
  //    dc.SetBackground(*wxTRANSPARENT_BRUSH);

  //     dc.SetPen( *wxTRANSPARENT_PEN );
  //     dc.SetBrush( *wxTRANSPARENT_BRUSH );
  //     dc.DrawRectangle( 0, 0, width, height);

  wxColour backColor = GetGlobalColor(m_backcolorString);

  dc.SetBrush(wxBrush(backColor /*wxColour(200, 200, 200)*/));
  dc.SetPen(wxPen(backColor));
  dc.DrawRoundedRectangle(0, 0, width - 10, height - 10, 8);
}

void MUIBar::ResetCanvasOptions() {
  delete m_canvasOptions;
  m_canvasOptions = NULL;
}

void MUIBar::PullCanvasOptions() {
  //  Target position
  int cox = m_parent->GetSize().x - m_canvasOptionsFullSize.x;
  int coy = m_COTopOffset;
  m_targetCOPos = m_parent->ClientToScreen(wxPoint(cox, coy));

  if (!m_bEffects) {
    m_canvasOptions->Move(m_targetCOPos);
    m_canvasOptions->Show();
    return;
  }

  //  Capture the animation bitmap, if required..

  if (m_coAnimateByBitmaps && !m_animateBitmap.IsOk()) {
    m_canvasOptions->Move(m_targetCOPos);
    m_canvasOptions->Show();
    CaptureCanvasOptionsBitmap();
    return;
  }

  //  Setup animation parameters
  //  Start Position
  m_startCOPos = m_canvasOptions->GetPosition();

  //  Present Position
  m_currentCOPos = m_startCOPos;

  m_animationType = CO_ANIMATION_CUBIC_REVERSE;  // CO_ANIMATION_CUBIC_BACK_IN;
  m_animateSteps = 10;
  m_animationTotalTime = 200;  // msec

  m_pushPull = CO_PULL;
  ChartCanvas* pcc = wxDynamicCast(m_parent, ChartCanvas);
  pcc->m_b_paint_enable = false;

  // Start the animation....
  m_animateStep = 0;
  m_canvasOptionsAnimationTimer.Start(10, true);
  m_canvasOptions->Move(m_targetCOPos);
  m_canvasOptions->Hide();
}

void MUIBar::PushCanvasOptions() {
  if (!m_bEffects) {
    m_canvasOptions->Hide();
    return;
  }

  //  Setup animation parameters

  //  Target position
  int cox = m_parent->GetSize().x;
  int coy = 20;

  if (1)
    m_targetCOPos = m_parent->ClientToScreen(wxPoint(cox, coy));
  else
    m_targetCOPos = wxPoint(cox, coy);

  //  Start Position
  m_startCOPos = m_canvasOptions->GetPosition();

  //  Present Position
  m_currentCOPos = m_startCOPos;

  //  Animation type
  m_animationType = CO_ANIMATION_LINEAR;
  m_animateSteps = 5;
  m_animationTotalTime = 100;  // msec
  m_pushPull = CO_PUSH;
  ChartCanvas* pcc = wxDynamicCast(m_parent, ChartCanvas);

  // Start the animation....
  m_animateStep = 0;
  m_canvasOptionsAnimationTimer.Start(10, true);
  m_canvasOptions->Show();
}

void MUIBar::onCanvasOptionsAnimationTimerEvent(wxTimerEvent& event) {
  double progress = m_animateStep / (double)m_animateSteps;
  double valueX = getValue(m_animationType, progress);

  double dx = (m_targetCOPos.x - m_startCOPos.x) * valueX;

  wxPoint newPos = wxPoint(m_startCOPos.x + dx, m_currentCOPos.y);

  int size_x;
  if (m_pushPull == CO_PULL)
    size_x = abs(dx);
  else
    size_x = (m_targetCOPos.x - m_startCOPos.x) - abs(dx);

  if (!m_coAnimateByBitmaps) {
    m_canvasOptions->SetSize(newPos.x, newPos.y, size_x, wxDefaultCoord,
                             wxSIZE_USE_EXISTING);
    // m_canvasOptions->GetSizer()->Layout();
    m_canvasOptions->Show();
  } else {
    m_canvasOptions->Hide();
    wxScreenDC sdc;

    if (1 /*m_pushPull == CO_PULL*/) {
      //  restore Backing bitmap, to cover any overshoot
      if (m_backingBitmap.IsOk()) {
        wxMemoryDC mdc_back(m_backingBitmap);
        sdc.Blit(m_backingPoint.x, m_backingPoint.y,
                 m_backingBitmap.GetWidth() - size_x,
                 m_backingBitmap.GetHeight(), &mdc_back, 0, 0, wxCOPY);
      }
    }

    wxMemoryDC mdc(m_animateBitmap);
    sdc.Blit(newPos.x, newPos.y, size_x, m_animateBitmap.GetHeight(), &mdc, 0,
             0, wxCOPY);
    mdc.SelectObject(wxNullBitmap);
  }

  m_currentCOPos = newPos;

  double dt = m_animationTotalTime / m_animateSteps;

  if (m_animateStep++ < m_animateSteps + 1) {
    m_canvasOptionsAnimationTimer.Start(dt, true);
  } else {
    m_currentCOPos = m_targetCOPos;
    m_canvasOptions->Show(m_pushPull == CO_PULL);

    ChartCanvas* pcc = wxDynamicCast(m_parent, ChartCanvas);
    if (pcc) {
      pcc->m_b_paint_enable = true;

      if (m_pushPull == CO_PUSH) {
        delete m_canvasOptions;
        m_canvasOptions = NULL;
      }
#ifdef __WXOSX__
      if (m_pushPull == CO_PUSH) pcc->TriggerDeferredFocus();
#else
      pcc->TriggerDeferredFocus();
#endif

      pcc->Refresh();
    }
  }
}

//   Animation support

double bounceMaker(double t, double c, double a) {
  if (t == 1.0) return c;
  if (t < (4 / 11.0)) {
    return c * (7.5625 * t * t);
  } else if (t < (8 / 11.0)) {
    t -= (6 / 11.0);
    return -a * (1. - (7.5625 * t * t + .75)) + c;
  } else if (t < (10 / 11.0)) {
    t -= (9 / 11.0);
    return -a * (1. - (7.5625 * t * t + .9375)) + c;
  } else {
    t -= (21 / 22.0);
    return -a * (1. - (7.5625 * t * t + .984375)) + c;
  }
}

double getValue(int animationType, double t) {
  double value = 0;
  double s = 1;
  double tp;
  switch (animationType) {
    case CO_ANIMATION_LINEAR:
    default:
      value = t;
      break;
    case CO_ANIMATION_CUBIC:
      tp = t - 1.0;
      value = tp * tp * tp + 1;
      // value = t*t*t;
      break;
    case CO_ANIMATION_CUBIC_REVERSE:
      tp = t - 1.0;
      value = tp * tp * tp + 1;
      break;
    case CO_ANIMATION_CUBIC_BOUNCE_IN:
      value = bounceMaker(t, 1, s);
      break;

    case CO_ANIMATION_CUBIC_BACK_IN:
      tp = t - 1.0;
      value = tp * tp * ((s + 1) * tp + s) + 1;
      break;
  }

  return value;
}
