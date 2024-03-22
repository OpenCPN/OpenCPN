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

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/statline.h>

#include "chcanv.h"
#include "MUIBar.h"
#include "OCPNPlatform.h"
#include "CanvasOptions.h"
#include "DetailSlider.h"
#include "GoToPositionDialog.h"
#include "styles.h"
#include "navutil.h"
#include "svg_utils.h"
#include "model/idents.h"
#include "color_handler.h"
#include "navutil.h"
#include "pluginmanager.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

#ifdef ocpnUSE_GL
extern GLenum g_texture_rectangle_format;
#endif

//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------

extern OCPNPlatform* g_Platform;
extern ChartCanvas* g_focusCanvas;
extern ocpnStyle::StyleManager* g_StyleManager;
extern bool g_bShowMuiZoomButtons;
extern bool g_bopengl;

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
//    MUIButton
//------------------------------------------------------------------------------

class MUIButton  {

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
  int GetState(){ return mState; }

  void SetColorScheme(ColorScheme cs);
  void OnSize(wxSizeEvent& event);
  void OnLeftDown(wxMouseEvent& event);
  void OnLeftUp(wxMouseEvent& event);

  wxBitmap GetBitmapResource(const wxString& name);

  wxIcon GetIconResource(const wxString& name);
  wxBitmap GetButtonBitmap(){ return m_bitmap; }

  /// Should we show tooltips?
  static bool ShowToolTips();
  wxSize m_size;
  wxPoint m_position;

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
  m_bitmapFileState0 = bitmap;
  m_bitmapFileState1 = bitmapState1;
  m_bitmapFileState2 = bitmapState2;

  m_scaleFactor = scale_factor;

  m_styleToolSize = g_StyleManager->GetCurrentStyle()->GetToolSize();

  //  Arbitrarily boost the MUIButton default size above the style defined size.
  //  No good reason.....
  m_styleToolSize = wxSize(m_styleToolSize.x * 1.25, m_styleToolSize.y * 1.25);

  m_size = wxSize(m_styleToolSize.x * m_scaleFactor,
                m_styleToolSize.y * m_scaleFactor);

  CreateControls();
  return true;
}

MUIButton::~MUIButton() {}

void MUIButton::Init() {
  mState = 0;
  m_cs = (ColorScheme)-1;  // GLOBAL_COLOR_SCHEME_RGB;
}

void MUIButton::CreateControls() {
//  this->SetForegroundColour(wxColour(255, 255, 255));

//  wxColour backColor = GetGlobalColor(_T("GREY3"));
//  SetBackgroundColour(backColor);

//  this->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
//                       wxFONTWEIGHT_NORMAL, false, wxT("Tahoma")));
}

void MUIButton::SetColorScheme(ColorScheme cs) {
#if 1
  if (m_cs != cs) {
    wxColour backColor = GetGlobalColor(_T("GREY3"));
    //SetBackgroundColour(backColor);

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    wxBitmap bmp = LoadSVG(m_bitmapFileState0, m_size.x, m_size.y);
    m_bitmapState0 = style->SetBitmapBrightness(bmp, cs);

    bmp = LoadSVG(m_bitmapFileState1, m_size.x, m_size.y);
    if (bmp.IsOk())
      m_bitmapState1 = style->SetBitmapBrightness(bmp, cs);
    else
      m_bitmapState1 = m_bitmapState0;

    bmp = LoadSVG(m_bitmapFileState2, m_size.x, m_size.y);
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
#endif
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


wxSize MUIButton::DoGetBestSize() const {
  //     wxSize labelSize = wxDefaultSize;
  //     GetTextExtent(m_Label, &labelSize.x, &labelSize.y);
  //     return wxSize(wxMax(40, labelSize.x + 20), wxMax(20, labelSize.y +
  //     10));
  return wxSize(m_styleToolSize.x * m_scaleFactor,
                m_styleToolSize.y * m_scaleFactor);
}



//------------------------------------------------------------------------------
//    MUITextButton
//------------------------------------------------------------------------------

class MUITextButton {

public:
  MUITextButton();
  MUITextButton(wxWindow* parent, wxWindowID id = wxID_ANY,
                float scale_factor = 1.0,
                const wxString& text = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = wxNO_BORDER);

  bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
              float scale_factor = 1.0,
              const wxString& text = wxEmptyString,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize, long style = wxNO_BORDER);

  ~MUITextButton();

  void Init();
  void CreateControls();

  void SetState(int state);

  void SetColorScheme(ColorScheme cs);
  void OnSize(wxSizeEvent& event);
  void OnLeftDown(wxMouseEvent& event);
  void OnLeftUp(wxMouseEvent& event);
  void SetText( const wxString &text);

  wxBitmap GetButtonBitmap(){ return m_bitmap; }
  wxSize m_size;
  wxPoint m_position;

private:
  void BuildBitmap();

  wxString m_text;
  wxBitmap m_bitmap;

  int mState;
  float m_scaleFactor;
  wxSize m_styleToolSize;
  ColorScheme m_cs;
  wxFont m_font;
};


MUITextButton::MUITextButton() { Init(); }

MUITextButton::MUITextButton(wxWindow* parent, wxWindowID id, float scale_factor,
                             const wxString& text,
                             const wxPoint& pos,
                             const wxSize& size, long style) {
  Init();


  Create(parent, id, scale_factor, text, pos, size, style);
}

bool MUITextButton::Create(wxWindow* parent, wxWindowID id, float scale_factor,
                           const wxString& text,
                           const wxPoint& pos,
                           const wxSize& size, long style) {
  m_text = text;

  m_scaleFactor = scale_factor;

  m_styleToolSize = g_StyleManager->GetCurrentStyle()->GetToolSize();

  //  Arbitrarily boost the MUITextButton default size above the style defined size.
  //  No good reason.....
  m_styleToolSize = wxSize(m_styleToolSize.x * 1.25, m_styleToolSize.y * 1.25);

  // Really contorted logic to work around wxFont problems with Windows scaled displays,
  // And the apparent failure of wxFont::Scale()
  // Sorry...
  int font_test_size = 12;
  double target_size = 0.4;  // Referenced to height of m_styleToolSize
  wxFont *t_font = wxTheFontList->FindOrCreateFont(
      font_test_size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

  int w, h;
  wxScreenDC sdc;
  sdc.GetTextExtent("M", &w, &h, NULL, NULL, t_font);

  double fraction = ((double)h) / (m_styleToolSize.y);
  double new_font_size = font_test_size * (target_size /fraction) / OCPN_GetWinDIPScaleFactor();
  new_font_size /= OCPN_GetWinDIPScaleFactor();
  new_font_size *= m_scaleFactor;

  m_font = *wxTheFontList->FindOrCreateFont(new_font_size, wxFONTFAMILY_MODERN,
                                            wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
#ifndef __WXMSW__
  // Twek the font size on those platforms that correctly support wxFont::Scaled()
  sdc.GetTextExtent("M", &w, &h, NULL, NULL, &m_font);
  m_font = m_font.Scaled( 1.5 * target_size * (m_styleToolSize.y * m_scaleFactor) / h);
#endif

  wxCoord descent, exlead, gw, gh;
  sdc.GetTextExtent(m_text, &gw, &gh, &descent, &exlead, &m_font);

  int min_width = gw * 1.2;
  min_width *= OCPN_GetWinDIPScaleFactor();

  m_size = wxSize(min_width, (m_styleToolSize.y * m_scaleFactor) - 1);


  CreateControls();
  return true;
}

MUITextButton::~MUITextButton() {}

void MUITextButton::Init() {
  mState = 0;
  m_cs = (ColorScheme)-1;  // GLOBAL_COLOR_SCHEME_RGB;
}

void MUITextButton::CreateControls() {
  //wxColour backColor = GetGlobalColor(_T("GREY3"));
  //SetBackgroundColour(backColor);
}

void MUITextButton::SetText( const wxString &text){
  if (!m_text.IsSameAs(text)){
    m_text = text;
    BuildBitmap();
  }
}

void MUITextButton::SetColorScheme(ColorScheme cs) {
  if (m_cs != cs) {
    //wxColour backColor = GetGlobalColor(_T("GREY3"));
    //SetBackgroundColour(backColor);

    m_cs = cs;
  }
  BuildBitmap();
}


void MUITextButton::SetState(int state) {
  mState = state;
}

void MUITextButton::OnSize(wxSizeEvent& event) {
  BuildBitmap();
  return;
}


void MUITextButton::BuildBitmap(){
  int width = m_size.x;
  int height = m_size.y;

  //Make the bitmap
  wxMemoryDC mdc;
  wxBitmap bm(width, height);
  mdc.SelectObject(bm);
  wxColour backColor = *wxBLACK; //GetGlobalColor(_T("GREY3"));
  mdc.SetBackground(backColor);
  mdc.Clear();

  wxCoord descent, exlead, gw, gh;
  mdc.SetFont(m_font);
  mdc.GetTextExtent(m_text, &gw, &gh, &descent, &exlead);

  mdc.SetTextForeground(GetGlobalColor("CHWHT"));
  mdc.DrawText(m_text, (width-gw)/2, (height-gh)/2);

  mdc.SelectObject(wxNullBitmap);
  m_bitmap = bm;
}


#define CANVAS_OPTIONS_ANIMATION_TIMER_1 800
#define CANVAS_OPTIONS_TIMER 801

//------------------------------------------------------------------------------
//          MUIBar Implementation
//------------------------------------------------------------------------------

// Define a constructor
MUIBar::MUIBar() {}

MUIBar::MUIBar(ChartCanvas* parent, int orientation, float size_factor,
               wxWindowID id, const wxPoint& pos, const wxSize& size,
               long style, const wxString& name) {
  m_parentCanvas = parent;
  m_orientation = orientation;

  m_scaleFactor = size_factor;
  m_cs = (ColorScheme)-1;
//  wxColour backColor = wxColor(*wxBLACK); //GetGlobalColor(m_backcolorString);
//  SetBackgroundColour(backColor);

  Init();
  CreateControls();
}

MUIBar::~MUIBar() {
  if (m_canvasOptions) {
    m_canvasOptions->Destroy();
    m_canvasOptions = 0;
  }
  delete m_zinButton;
  delete m_zoutButton;
  delete m_followButton;
  delete m_menuButton;
  delete m_scaleButton;
  InvalidateBitmap();
}

void MUIBar::Init() {
  m_zinButton = NULL;
  m_zoutButton = NULL;
  m_followButton = NULL;
  m_menuButton = NULL;
  m_scaleButton = NULL;

  m_canvasOptions = NULL;
  m_canvasOptionsAnimationTimer.SetOwner(this,
                                         CANVAS_OPTIONS_ANIMATION_TIMER_1);
  m_backcolorString = _T("GREY3");
  m_capture_size_y = 0;

  m_COTopOffset = 60;  //  TODO should be below GPS/Compass

  CanvasOptionTimer.SetOwner(this, CANVAS_OPTIONS_TIMER);
  m_coAnimateByBitmaps = false;
  m_bEffects = false; //true;
#ifdef __OCPN__ANDROID__
  m_bEffects = false;
#endif
  m_texture = 0;
  m_end_margin = m_parentCanvas->GetCharWidth()/2;
  m_scale = 0;
}

void MUIBar::SetColorScheme(ColorScheme cs) {
  if (m_cs != cs) {

    if (m_zinButton) m_zinButton->SetColorScheme(cs);
    if (m_zoutButton) m_zoutButton->SetColorScheme(cs);
    if (m_followButton) m_followButton->SetColorScheme(cs);
    if (m_menuButton) m_menuButton->SetColorScheme(cs);

    if (m_scaleButton) m_scaleButton->SetColorScheme(cs);

    m_cs = cs;
    InvalidateBitmap();
  }
}
void MUIBar::InvalidateBitmap() {
  m_bitmap = wxNullBitmap;

#ifdef ocpnUSE_GL
  if(g_bopengl) {
    glDeleteTextures(1, &m_texture);
    m_texture = 0;
  }
#endif
}

bool MUIBar::MouseEvent(wxMouseEvent &event) {
  int x, y;
  event.GetPosition(&x, &y);

  //    Check the regions
  wxRect r = wxRect(m_screenPos, m_size);
  if (r.Contains(x,y)) {
    // Check buttons
    if (event.LeftDown()) {
      if (g_bShowMuiZoomButtons) {
        wxRect rzin(m_zinButton->m_position.x, m_zinButton->m_position.y,
                    m_zinButton->m_size.x, m_zinButton->m_size.y);
        rzin.Offset(m_screenPos);
        if (rzin.Contains(x, y)) {
          wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_ZOOMIN);
          m_parentCanvas->GetEventHandler()->AddPendingEvent(evt);
          if (g_focusCanvas) g_focusCanvas->TriggerDeferredFocus();
          return true;
        }

        wxRect rzout(m_zoutButton->m_position.x, m_zoutButton->m_position.y,
                     m_zoutButton->m_size.x, m_zoutButton->m_size.y);
        rzout.Offset(m_screenPos);
        if (rzout.Contains(x, y)) {
          wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_ZOOMOUT);
          m_parentCanvas->GetEventHandler()->AddPendingEvent(evt);
          if (g_focusCanvas) g_focusCanvas->TriggerDeferredFocus();
        }
      }

      wxRect rfollow(m_followButton->m_position.x, m_followButton->m_position.y,
                    m_followButton->m_size.x, m_followButton->m_size.y);
      rfollow.Offset(m_screenPos);
      if (rfollow.Contains(x, y)) {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_FOLLOW);
        m_parentCanvas->GetEventHandler()->AddPendingEvent(evt);
        if (g_focusCanvas) g_focusCanvas->TriggerDeferredFocus();
      }

      wxRect rmenu(m_menuButton->m_position.x, m_menuButton->m_position.y,
                    m_menuButton->m_size.x, m_menuButton->m_size.y);
      rmenu.Offset(m_screenPos);
      if (rmenu.Contains(x, y)) {
        HandleMenuClick();
        if (g_focusCanvas) g_focusCanvas->TriggerDeferredFocus();
      }
    }
    else if (event.LeftUp()) {
      if (m_scaleButton) {
        wxRect rscale(m_scaleButton->m_position.x, m_scaleButton->m_position.y,
                      m_scaleButton->m_size.x, m_scaleButton->m_size.y);
        rscale.Offset(m_screenPos);
        if (rscale.Contains(x, y)) {
          OnScaleSelected(event);
        }
      }
    }
    return true;
  }
  return false;
}

void MUIBar::OnScaleSelected(wxMouseEvent& event) {
  ChartCanvas* pcc = wxDynamicCast(m_parentCanvas, ChartCanvas);
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
  wxString iconDir = g_Platform->GetSharedDataDir() + _T("uidata/MUI_flat/");

  if (m_orientation == wxHORIZONTAL) {
    // Buttons

    int xoff = 0;
    if (g_bShowMuiZoomButtons) {
      m_zinButton = new MUIButton(m_parentCanvas, ID_ZOOMIN, m_scaleFactor,
                                  iconDir + _T("MUI_zoom-in.svg"));
      m_zinButton->m_position = wxPoint(xoff,0);
      xoff += m_zinButton->m_size.x;

      m_zoutButton = new MUIButton(m_parentCanvas, ID_ZOOMOUT, m_scaleFactor,
                                   iconDir + _T("MUI_zoom-out.svg"));
      m_zoutButton->m_position = wxPoint(xoff,0);
      xoff += m_zoutButton->m_size.x;

    }

#ifndef __OCPN__ANDROID__
    //  Scale

    m_scaleButton = new MUITextButton(m_parentCanvas, wxID_ANY, m_scaleFactor,
                                      "1:400000");
    m_scaleButton->m_position = wxPoint(xoff,0);
    xoff += m_scaleButton->m_size.x;

    m_followButton = new MUIButton(m_parentCanvas, ID_FOLLOW, m_scaleFactor,
                                   iconDir + _T("MUI_follow.svg"),
                                   iconDir + _T("MUI_follow_active.svg"),
                                   iconDir + _T("MUI_follow_ahead.svg"));
    m_followButton->m_position = wxPoint(xoff,0);
    xoff += m_followButton->m_size.x;
#endif

    m_menuButton = new MUIButton(m_parentCanvas, ID_MUI_MENU, m_scaleFactor,
                                 iconDir + _T("MUI_menu.svg"));
    m_menuButton->m_position = wxPoint(xoff,0);
    xoff += m_menuButton->m_size.x;
    m_size.x = xoff;
    m_size.y = m_zinButton->m_size.y;

  } else {

    int yoff = 0;

    // Buttons
    if (g_bShowMuiZoomButtons) {
      m_zinButton = new MUIButton(m_parentCanvas, ID_ZOOMIN, m_scaleFactor,
                                  iconDir + _T("MUI_zoom-in.svg"));
      m_zinButton->m_position = wxPoint(0, yoff);
      yoff += m_zinButton->m_size.y;

      m_zoutButton = new MUIButton(m_parentCanvas, ID_ZOOMOUT, m_scaleFactor,
                                   iconDir + _T("MUI_zoom-out.svg"));
      m_zoutButton->m_position = wxPoint(0,yoff);
      yoff += m_zoutButton->m_size.y;

    }

#ifndef __OCPN__ANDROID__
    m_followButton = new MUIButton(m_parentCanvas, ID_FOLLOW, m_scaleFactor,
                                   iconDir + _T("MUI_follow.svg"),
                                   iconDir + _T("MUI_follow_active.svg"),
                                   iconDir + _T("MUI_follow_ahead.svg"));
    m_followButton->m_position = wxPoint(0,yoff);
    yoff += m_followButton->m_size.y;
#endif

    m_menuButton = new MUIButton(m_parentCanvas, ID_MUI_MENU, m_scaleFactor,
                                 iconDir + _T("MUI_menu.svg"));
    m_menuButton->m_position = wxPoint(0,yoff);
    yoff += m_menuButton->m_size.y;

    m_size.y = yoff;
    m_size.x = m_zinButton->m_size.x;

  }

}

void MUIBar::SetBestPosition(void) {
  int x = (m_parentCanvas->GetClientSize().x - (m_size.x + (m_end_margin) * 2.00));

  int bottomOffset = 6;


  int y = m_parentCanvas->GetClientSize().y - m_size.y - bottomOffset;
  //if ( g_bopengl){
  //  y -= m_parentCanvas->GetClientSize().y % 1;
 // }

  wxPoint position = wxPoint(x, y);
  m_screenPos = position;

  if (m_canvasOptions) {
    m_canvasOptions->Destroy();
    m_canvasOptions = 0;
  }
}


void MUIBar::UpdateDynamicValues() {
  if (!m_scaleButton) return;

  wxString scaleString;
  int scale = m_parentCanvas->GetScaleValue();

  if (scale != m_scale)
    InvalidateBitmap();
  m_scale = scale;

  if (scale < 1e6)
    scaleString.Printf(_T("1:%d"), scale);
  else
    scaleString.Printf(_T("1:%4.1f MM"), scale / 1e6);

  if (m_scaleButton) m_scaleButton->SetText(scaleString);
}

void MUIBar::SetFollowButtonState(int state) {
  if (m_followButton && m_followButton->GetState() != state) {
    m_followButton->SetState(state);
    InvalidateBitmap();
  }
}


void MUIBar::HandleMenuClick(){
  if (!m_canvasOptions) {
    m_canvasOptions = new CanvasOptions(m_parentCanvas);

    // calculate best size for CanvasOptions dialog

    wxPoint parentClientUpperRight =
        m_parentCanvas->ClientToScreen(wxPoint(m_parentCanvas->GetSize().x, 0));
    wxPoint muibar_top = m_parentCanvas->ClientToScreen(m_screenPos);
    int size_y = muibar_top.y - (parentClientUpperRight.y + m_COTopOffset);
    size_y -= m_parentCanvas->GetCharHeight();
    size_y = wxMax(size_y, 100);  // ensure always big enough to see

    m_canvasOptions->SetSize(wxSize(-1, size_y));
    m_canvasOptionsFullSize = m_canvasOptions->GetSize();
    m_canvasOptionsFullSize.x +=
        m_canvasOptions->GetCharWidth();  // Allow for scroll bar, since
                                          // sizer won't do it.

    m_currentCOPos = m_parentCanvas->ClientToScreen(
        wxPoint(m_parentCanvas->GetSize().x, m_COTopOffset));

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

}

void MUIBar::CaptureCanvasOptionsBitmap() {
  m_coSequence = 0;
  CanvasOptionTimer.Start(100, wxTIMER_ONE_SHOT);
}

void MUIBar::CaptureCanvasOptionsBitmapChain(wxTimerEvent& event) {
  if (m_coSequence == 0) {
    if (!m_canvasOptions) m_canvasOptions = new CanvasOptions(m_parentCanvas);

    wxPoint parentClientUpperRight =
        m_parentCanvas->ClientToScreen(wxPoint(m_parentCanvas->GetSize().x, 0));
    wxRect rmui = m_parentCanvas->GetMUIBarRect();
    int size_y = rmui.y - (parentClientUpperRight.y + m_COTopOffset);
    size_y -= m_parentCanvas->GetCharHeight();
    size_y = wxMax(size_y, 100);  // ensure always big enough to see
    m_capture_size_y = size_y;

    m_canvasOptions->SetSize(wxSize(-1, size_y));

    m_capturePoint =
        m_parentCanvas->ClientToScreen(wxPoint(m_parentCanvas->GetSize().x, m_COTopOffset));
    m_canvasOptions->Move(m_capturePoint);
    m_canvasOptions->Show();

    m_coSequence++;
    CanvasOptionTimer.Start(1, wxTIMER_ONE_SHOT);
  }

  else if (m_coSequence == 1) {
    m_capturePoint = m_parentCanvas->ClientToScreen(wxPoint(
        m_parentCanvas->GetSize().x - m_canvasOptionsFullSize.x, m_COTopOffset));
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


wxBitmap &MUIBar::CreateBitmap(double displayScale) {
  if (m_bitmap.IsOk())
    return m_bitmap;

  //Make the bitmap
  int width = m_size.x;
  int height = m_size.y;

  wxMemoryDC mdc;
  wxBitmap bm(width, height);
  mdc.SelectObject(bm);
  mdc.SetBackground(*wxBLACK_BRUSH);
  mdc.Clear();

  wxBitmap bmd;
  if (g_bShowMuiZoomButtons) {
    wxBitmap bmd = m_zinButton->GetButtonBitmap();
    if (bmd.IsOk())
      mdc.DrawBitmap(bmd,
                   m_zinButton->m_position.x,
                   m_zinButton->m_position.y,
                   false);

    bmd = m_zoutButton->GetButtonBitmap();
    if (bmd.IsOk())
      mdc.DrawBitmap(bmd,
                   m_zoutButton->m_position.x,
                   m_zoutButton->m_position.y,
                   false);
  }

  if (m_scaleButton) {
    bmd = m_scaleButton->GetButtonBitmap();
    if (bmd.IsOk())
      mdc.DrawBitmap(bmd,
                     m_scaleButton->m_position.x,
                     m_scaleButton->m_position.y,
                     false);
  }

  if (m_followButton) {
    bmd = m_followButton->GetButtonBitmap();
    if (bmd.IsOk())
      mdc.DrawBitmap(bmd,
                     m_followButton->m_position.x, m_followButton->m_position.y,
                     false);
  }

  if (m_menuButton) {
    bmd = m_menuButton->GetButtonBitmap();
    if (bmd.IsOk())
      mdc.DrawBitmap(bmd,
                     m_menuButton->m_position.x, m_menuButton->m_position.y,
                     false);
  }

  mdc.SelectObject(wxNullBitmap);

  m_bitmap = bm;
  return m_bitmap;
}

void MUIBar::DrawGL(ocpnDC &gldc, double displayScale) {
#ifdef ocpnUSE_GL

  wxColour backColor = *wxBLACK;
  gldc.SetBrush(wxBrush(backColor));
  gldc.SetPen(wxPen(backColor));

  wxRect r = wxRect(m_screenPos, m_size);
  if (m_orientation == wxHORIZONTAL)
    gldc.DrawRoundedRectangle((r.x - m_end_margin/2)*displayScale,
                                    (r.y-1)*displayScale,
                                    (r.width + m_end_margin)*displayScale,
                                    (r.height+2)*displayScale,
                                    (m_end_margin * 1)*displayScale);
  else
    gldc.DrawRoundedRectangle((r.x-1)*displayScale,
                              (r.y- m_end_margin/2)*displayScale,
                              (r.width + 2)*displayScale,
                              (r.height + 2 * m_end_margin)*displayScale,
                              (m_end_margin * 1.5)*displayScale);

  int width = m_size.x;
  int height = m_size.y;

  CreateBitmap(displayScale);

  // Make a GL texture
  if (!m_texture) {
    glGenTextures(1, &m_texture);

    glBindTexture(g_texture_rectangle_format, m_texture);
    glTexParameterf(g_texture_rectangle_format, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);
    glTexParameteri(g_texture_rectangle_format, GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameteri(g_texture_rectangle_format, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(g_texture_rectangle_format, GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
  } else {
    glBindTexture(g_texture_rectangle_format, m_texture);
  }


  // fill texture data
  wxImage image = m_bitmap.ConvertToImage();

  unsigned char *d = image.GetData();
  unsigned char *e = new unsigned char[4 * width * height];
  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      int i = y * width + x;
      memcpy(e + 4 * i, d + 3 * i, 3);
      e[4 * i + 3] = 255; //d[3*i + 2] == 255 ? 0:255; //255 - d[3 * i + 2];
    }
  glTexImage2D(g_texture_rectangle_format, 0, GL_RGBA, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, e);
  delete[] e;
  glDisable(g_texture_rectangle_format);
  glDisable(GL_BLEND);

  // Render the texture
  if (m_texture) {
    glEnable(g_texture_rectangle_format);
    glBindTexture(g_texture_rectangle_format, m_texture);
    glEnable(GL_BLEND);

    int x0 = m_screenPos.x, x1 = x0 + width;
    int y0 = m_screenPos.y - 0, y1 = y0 + height;
    x0 *= displayScale;  x1 *= displayScale;
    y0 *= displayScale;  y1 *= displayScale;

    float tx, ty;
    if (GL_TEXTURE_RECTANGLE_ARB == g_texture_rectangle_format)
      tx = width, ty = height;
    else
      tx = ty = 1;

    float coords[8];
    float uv[8];

    // normal uv
    uv[0] = 0;
    uv[1] = 0;
    uv[2] = tx;
    uv[3] = 0;
    uv[4] = tx;
    uv[5] = ty;
    uv[6] = 0;
    uv[7] = ty;

    // pixels
    coords[0] = x0;
    coords[1] = y0;
    coords[2] = x1;
    coords[3] = y0;
    coords[4] = x1;
    coords[5] = y1;
    coords[6] = x0;
    coords[7] = y1;

    m_parentCanvas->GetglCanvas()->RenderTextures(gldc, coords, uv, 4,
                                                  m_parentCanvas->GetpVP());

    glDisable(g_texture_rectangle_format);
    glBindTexture(g_texture_rectangle_format, 0);
    glDisable(GL_BLEND);
  }
#endif

    return;
}

void MUIBar::DrawDC(ocpnDC &dc, double displayScale) {
    CreateBitmap(1.0);
    dc.DrawBitmap(m_bitmap, m_screenPos.x, m_screenPos.y, false);
}

void MUIBar::ResetCanvasOptions() {
  delete m_canvasOptions;
  m_canvasOptions = NULL;
}

void MUIBar::PullCanvasOptions() {
  //  Target position
  int cox = m_parentCanvas->GetSize().x - m_canvasOptionsFullSize.x;
  int coy = m_COTopOffset;
  m_targetCOPos = m_parentCanvas->ClientToScreen(wxPoint(cox, coy));

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
  ChartCanvas* pcc = wxDynamicCast(m_parentCanvas, ChartCanvas);
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
  int cox = m_parentCanvas->GetSize().x;
  int coy = 20;

  if (1)
    m_targetCOPos = m_parentCanvas->ClientToScreen(wxPoint(cox, coy));
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
  ChartCanvas* pcc = wxDynamicCast(m_parentCanvas, ChartCanvas);

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

    ChartCanvas* pcc = wxDynamicCast(m_parentCanvas, ChartCanvas);
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
