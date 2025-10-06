/***************************************************************************
 *   Copyright (C) 2025 by OpenCPN developer team                          *
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

#include "tooltip.h"
#include "chcanv.h"
#include "color_handler.h"
#include "font_mgr.h"
#include "ocpn_platform.h"
#include "ocpn_frame.h"
#include "navutil.h"

extern OCPNPlatform *g_Platform;
extern MyFrame *gFrame;
extern bool g_btouch;

// Define timer event ID
#define TOOLTIP_TIMER_ID 10002

//----------------------------------------------------------------------------
// Tooltip Implementation
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(Tooltip, wxFrame)
EVT_PAINT(Tooltip::OnPaint)
EVT_TIMER(TOOLTIP_TIMER_ID, Tooltip::OnTimer)
END_EVENT_TABLE()

Tooltip::Tooltip(wxWindow *parent, TooltipCallback on_destroy)
    : wxFrame(parent, wxID_ANY, "", wxPoint(0, 0), wxSize(1, 1),
              wxNO_BORDER | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR),
      m_showTimer(this, TOOLTIP_TIMER_ID),
      m_on_destroy(std::move(on_destroy)) {
  m_pbm = nullptr;
  m_hiviz = false;
  m_showPending = false;

  // Initialize colors using SetColorScheme to avoid duplicate code
  SetColorScheme(GLOBAL_COLOR_SCHEME_RGB);

  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  SetBackgroundColour(m_back_color);
  Hide();
}

Tooltip::~Tooltip() {
  m_on_destroy(this);
  m_showTimer.Stop();
  delete m_pbm;
}

bool Tooltip::Destroy() {
  m_on_destroy(this);
  return wxFrame::Destroy();
}

void Tooltip::SetString(const wxString &text) { m_string = text; }

void Tooltip::SetAbsolutePosition(const wxPoint &pt) {
  m_position = pt;
  m_requestedPosition = pt;
}

void Tooltip::SetHiviz(bool hiviz) { m_hiviz = hiviz; }

void Tooltip::SetColorScheme(ColorScheme cs) {
  m_cs = cs;
  m_back_color =
      GetGlobalColor("DILG0");  // Background gets progressively darker
  m_text_color = GetGlobalColor("DILG4");  // Text gets progressively lighter
}

void Tooltip::SetRelativePosition(const wxRect &windowRect, int offsetX,
                                  int offsetY) {
  // Calculate initial position to the right of the window
  wxPoint pos;
  pos.x = windowRect.x + windowRect.width + offsetX;
  pos.y = windowRect.y + offsetY;

  // Get tooltip size for boundary checking
  wxSize tooltipSize = GetRenderedSize();
  wxSize screenSize = wxGetDisplaySize();

  // Convert to screen coordinates if needed
  wxPoint screenPos = pos;
  if (GetParent() && GetParent() != wxTheApp->GetTopWindow()) {
    screenPos = GetParent()->ClientToScreen(pos);
  }

  // Adjust horizontal position if tooltip would go off screen
  if (screenPos.x + tooltipSize.x > screenSize.x) {
    // Try positioning to the left of the window instead
    pos.x = windowRect.x - tooltipSize.x - offsetX;
    if (GetParent() && GetParent() != wxTheApp->GetTopWindow()) {
      screenPos = GetParent()->ClientToScreen(pos);
    } else {
      screenPos = pos;
    }

    // If still off screen, clamp to screen edge
    if (screenPos.x < 0) {
      screenPos.x = offsetX;
    }
  }

  // Adjust vertical position if tooltip would go off screen
  if (screenPos.y + tooltipSize.y > screenSize.y) {
    // Try positioning above the window instead
    pos.y = windowRect.y - tooltipSize.y - offsetY;
    if (GetParent() && GetParent() != wxTheApp->GetTopWindow()) {
      screenPos = GetParent()->ClientToScreen(pos);
    } else {
      screenPos = pos;
    }

    // If still off screen, clamp to screen edge
    if (screenPos.y < 0) {
      screenPos.y = offsetY;
    }
  }

  // SetAbsolutePosition expects screen coordinates
  SetAbsolutePosition(screenPos);
}

wxSize Tooltip::GetRenderedSize() const {
  if (m_string.IsEmpty()) {
    return wxSize(0, 0);
  }

  wxScreenDC cdc;
  double scaler = g_Platform->GetDisplayDIPMult(const_cast<Tooltip *>(this));

  wxFont *plabelFont = FontMgr::Get().GetFont(_("ToolTips"));
  wxFont sFont = plabelFont->Scaled(1.0 / scaler);

  int w, h;
  cdc.GetMultiLineTextExtent(m_string, &w, &h, nullptr, &sFont);
  int sizeX = w + GetCharWidth() * 2;
  int sizeY = h + GetCharHeight() / 2;

  sizeX *= scaler;
  sizeY *= scaler;

  return wxSize(sizeX, sizeY);
}

void Tooltip::CreateBitmap() {
  if (m_string.IsEmpty()) return;

  wxScreenDC cdc;
  double scaler = g_Platform->GetDisplayDIPMult(this);

  wxFont *plabelFont = FontMgr::Get().GetFont(_("ToolTips"));
  wxFont sFont = plabelFont->Scaled(1.0 / scaler);

  int w, h;
  cdc.GetMultiLineTextExtent(m_string, &w, &h, nullptr, &sFont);

  m_size.x = w + GetCharWidth() * 2;
  m_size.y = h + GetCharHeight() / 2;

  m_size.x *= scaler;
  m_size.y *= scaler;

  wxMemoryDC mdc;

  delete m_pbm;
  m_pbm = new wxBitmap(m_size.x, m_size.y, -1);
  mdc.SelectObject(*m_pbm);

  wxPen pborder(m_text_color);
  wxBrush bback(m_back_color);
  mdc.SetPen(pborder);
  mdc.SetBrush(bback);

  // High visibility mode for night/dusk color schemes
  if (m_hiviz) {
    if ((m_cs == GLOBAL_COLOR_SCHEME_DUSK) ||
        (m_cs == GLOBAL_COLOR_SCHEME_NIGHT)) {
      wxBrush hv_back(wxColour(200, 200, 200));
      mdc.SetBrush(hv_back);
    }
  }

  mdc.DrawRectangle(0, 0, m_size.x, m_size.y);

  // Draw the text
  mdc.SetFont(sFont);
  mdc.SetTextForeground(m_text_color);
  mdc.SetTextBackground(m_back_color);

  int offx = GetCharWidth();
  int offy = GetCharHeight() / 4;
  offx *= scaler;
  offy *= scaler;
  mdc.DrawText(m_string, offx, offy);

  SetClientSize(m_size.x, m_size.y);
}

void Tooltip::SetBitmap() {
  CreateBitmap();
  CalculateOptimalPosition();
  SetSize(m_position.x, m_position.y, m_size.x, m_size.y);
}

void Tooltip::CalculateOptimalPosition() {
  if (!GetParent()) return;

  wxPoint screenPos = m_requestedPosition;

  // Adjust position to keep tooltip on screen
  wxSize tooltipSize = GetRenderedSize();
  wxSize screenSize = wxGetDisplaySize();

  if (screenPos.x + tooltipSize.x > screenSize.x) {
    screenPos.x = screenSize.x - tooltipSize.x - 10;
  }
  if (screenPos.y + tooltipSize.y > screenSize.y) {
    screenPos.y = screenSize.y - tooltipSize.y - 10;
  }

  if (screenPos.x < 0) screenPos.x = 10;
  if (screenPos.y < 0) screenPos.y = 10;

  m_position = screenPos;
}

void Tooltip::ShowTooltip(int delay_ms) {
  if (m_string.IsEmpty()) return;

  if (delay_ms > 0) {
    m_showPending = true;
    m_showTimer.Start(delay_ms, wxTIMER_ONE_SHOT);
  } else {
    SetBitmap();
    Show();
#ifndef __WXOSX__
    if (gFrame) gFrame->Raise();
#endif
  }
}

void Tooltip::HideTooltip() {
  m_showTimer.Stop();
  m_showPending = false;
  Hide();
}

void Tooltip::OnPaint(wxPaintEvent &event) {
  int width, height;
  GetClientSize(&width, &height);
  wxPaintDC dc(this);

  if (m_string.Len() && m_pbm) {
    wxMemoryDC mdc;
    mdc.SelectObject(*m_pbm);
    dc.Blit(0, 0, width, height, &mdc, 0, 0);
  }
}

void Tooltip::OnTimer(wxTimerEvent &event) {
  if (event.GetId() == TOOLTIP_TIMER_ID && m_showPending) {
    m_showPending = false;
    // Ensure we're still in a valid state before proceeding
    if (!IsBeingDeleted()) {
      SetBitmap();
      Show();
#ifndef __WXOSX__
      if (gFrame) gFrame->Raise();
#endif
    }
  }
}

//----------------------------------------------------------------------------
// TooltipManager Implementation
//----------------------------------------------------------------------------

TooltipManager *TooltipManager::s_instance = nullptr;

TooltipManager::TooltipManager()
    : m_currentTooltip(nullptr),
      m_currentParent(nullptr),
      m_colorScheme(GLOBAL_COLOR_SCHEME_RGB),
      m_enabled(true),
      m_showDelay(500),
      m_hideDelay(5000) {}

TooltipManager::~TooltipManager() { CleanupTooltip(); }

TooltipManager &TooltipManager::Get() {
  if (!s_instance) {
    s_instance = new TooltipManager();
  }
  return *s_instance;
}

void TooltipManager::ShowTooltipAtPosition(wxWindow *parent,
                                           const wxString &text,
                                           const wxPoint &position,
                                           bool hiviz) {
  if (!m_enabled || text.IsEmpty()) return;

  // Hide any existing tooltip
  HideTooltip();

  // Create or reuse tooltip
  m_currentTooltip = GetOrCreateTooltip(parent);
  m_currentParent = parent;

  // Configure tooltip
  m_currentTooltip->SetString(text);
  m_currentTooltip->SetAbsolutePosition(position);
  m_currentTooltip->SetHiviz(hiviz);
  m_currentTooltip->SetColorScheme(m_colorScheme);

  // Show with delay
  m_currentTooltip->ShowTooltip(m_showDelay);
}

void TooltipManager::ShowTooltipForWindow(wxWindow *window,
                                          const wxString &text, bool hiviz) {
  if (!window) return;

  // Hide any existing tooltip
  HideTooltip();

  // Create or reuse tooltip
  m_currentTooltip = GetOrCreateTooltip(window->GetParent());
  m_currentParent = window->GetParent();

  // Configure tooltip
  m_currentTooltip->SetString(text);
  m_currentTooltip->SetHiviz(hiviz);
  m_currentTooltip->SetColorScheme(m_colorScheme);

  // Use enhanced positioning with automatic boundary detection
  wxRect windowRect = window->GetRect();
  m_currentTooltip->SetRelativePosition(windowRect, 2, 0);

  // Show with delay
  m_currentTooltip->ShowTooltip(m_showDelay);
}

void TooltipManager::HideTooltip() {
  if (m_currentTooltip) {
    m_currentTooltip->HideTooltip();
  }
}

void TooltipManager::HideAllTooltips() {
  HideTooltip();
  CleanupTooltip();
}

void TooltipManager::SetColorScheme(ColorScheme cs) {
  m_colorScheme = cs;
  if (m_currentTooltip) {
    m_currentTooltip->SetColorScheme(cs);
  }
}

void TooltipManager::EnableTooltips(bool enable) {
  m_enabled = enable;
  if (!enable) {
    HideAllTooltips();
  }
}

bool TooltipManager::IsShown() const {
  return m_currentTooltip && m_currentTooltip->IsShown();
}

Tooltip *TooltipManager::GetOrCreateTooltip(wxWindow *parent) {
  if (m_currentTooltip && m_currentParent == parent) {
    return m_currentTooltip;
  }

  CleanupTooltip();

  m_currentTooltip = new Tooltip(parent, [&](const Tooltip *t) {
    if (t == m_currentTooltip) m_currentTooltip = nullptr;
  });
  m_currentParent = parent;

  return m_currentTooltip;
}
void TooltipManager::CleanupTooltip() {
  if (m_currentTooltip) {
    m_currentTooltip->HideTooltip();
    m_currentTooltip->Destroy();
    m_currentTooltip = nullptr;
    m_currentParent = nullptr;
  }
}
