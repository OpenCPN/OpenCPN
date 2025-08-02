/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Console Canvas
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */

#ifndef __concanv_H__
#define __concanv_H__

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#include "ocpn_frame.h"  //FIXME (dave)  Only needed for colorscheme stuff

#define SPEED_VMG 0
#define SPEED_SOG 1

#define ID_LEGROUTE 1000
#define SECONDS_PER_DAY 86400

// Class declarations
class Routeman;

/**
 * Course Deviation Indicator display. Provides a visual representation
 * of the vessel's course deviation from the planned route.
 */
class CDI : public wxWindow {
public:
  CDI(wxWindow *parent, wxWindowID id, long style, const wxString &name);

  void OnPaint(wxPaintEvent &event);
  void SetColorScheme(ColorScheme cs);
  void MouseEvent(wxMouseEvent &event);

  wxBrush *m_pbackBrush;
  wxBrush *m_proadBrush;
  wxPen *m_proadPen;

  DECLARE_EVENT_TABLE()
};

/**
 * Annunciator Text display. Displays labeled values for various
 * navigation data points in the console.
 */
class AnnunText : public wxWindow {
public:
  AnnunText(wxWindow *parent, wxWindowID id, const wxString &LegendElement,
            const wxString &ValueElement);

  ~AnnunText();

  void SetALabel(const wxString &l);
  void SetAValue(const wxString &v);
  void OnPaint(wxPaintEvent &event);
  void RefreshFonts(void);
  void SetLegendElement(const wxString &element);
  void SetValueElement(const wxString &element);
  void SetColorScheme(ColorScheme cs);
  void MouseEvent(wxMouseEvent &event);

private:
  void CalculateMinSize(void);

  wxBrush m_backBrush;
  wxColour m_default_text_color;

  wxString m_label;
  wxString m_value;
  wxFont *m_plabelFont;
  wxFont *m_pvalueFont;

  wxString m_LegendTextElement;
  wxString m_ValueTextElement;
  wxColour m_legend_color;
  wxColour m_val_color;

  DECLARE_EVENT_TABLE()
};

/**
 * Primary navigation console display for route and vessel tracking.
 *
 * Manages a dynamic interface that presents real-time navigation metrics
 * during active route tracking. Provides detailed route information and
 * interaction capabilities for maritime navigation.
 *
 * Key Responsibilities:
 * - Display current leg and total route navigation data
 * - Render route-related information like XTE, bearing, range, TTG
 * - Support user interactions for route display modes
 * - Manage color scheme and font rendering
 *
 * Navigation Display Modes:
 * - Single Leg Mode: Focuses on current route segment
 * - Total Route Mode: Displays cumulative route statistics
 * - Speed Calculation: Supports VMG and SOG calculations
 *
 * Interaction Features:
 * - Context menu for route and display configuration
 * - Toggleable route total/leg display
 * - Dynamic font and color scheme adaptation
 */
class ConsoleCanvasWin : public wxWindow {
public:
  ConsoleCanvasWin(wxWindow *parent);
  ~ConsoleCanvasWin();
  /**
   * Updates route-related data displays.
   *
   * Calculates and refreshes navigation metrics based on current
   * route state, vessel position, and selected display mode.
   */
  void UpdateRouteData();
  /**
   * Recomputes and applies new fonts to console elements.
   *
   * Ensures consistent font rendering across different platforms
   * and display configurations. Triggers layout recalculation.
   */
  void ShowWithFreshFonts(void);
  void UpdateFonts(void);
  void SetColorScheme(ColorScheme cs);
  void LegRoute();
  void OnContextMenu(wxContextMenuEvent &event);
  void OnContextMenuSelection(wxCommandEvent &event);
  void RefreshConsoleData(void);
  void ToggleShowHighway();

  /**
   * Toggles between route total and current leg display modes.
   *
   * Switches speed calculation method and route information
   * presentation between:
   * - Current leg metrics
   * - Total route statistics
   */
  void ToggleRouteTotalDisplay();

  wxWindow *m_pParent;
  wxStaticText *pThisLegText;
  wxBoxSizer *m_pitemBoxSizerLeg;

  AnnunText *pXTE;
  AnnunText *pBRG;
  AnnunText *pRNG;
  AnnunText *pTTG;
  AnnunText *pVMG;
  CDI *pCDI;

  wxFont *pThisLegFont;
  bool m_bNeedClear;
  wxBrush *pbackBrush;

private:
  void OnPaint(wxPaintEvent &event);
  void OnShow(wxShowEvent &event);
  char m_speedUsed;

  DECLARE_EVENT_TABLE()
};

class ConsoleCanvasFrame : public wxFrame {
public:
  ConsoleCanvasFrame(wxWindow *parent);
  ~ConsoleCanvasFrame();
  /**
   * Updates route-related data displays.
   *
   * Calculates and refreshes navigation metrics based on current
   * route state, vessel position, and selected display mode.
   */
  void UpdateRouteData();
  /**
   * Recomputes and applies new fonts to console elements.
   *
   * Ensures consistent font rendering across different platforms
   * and display configurations. Triggers layout recalculation.
   */
  void ShowWithFreshFonts(void);
  void UpdateFonts(void);
  void SetColorScheme(ColorScheme cs);
  void LegRoute();
  void OnContextMenu(wxContextMenuEvent &event);
  void OnContextMenuSelection(wxCommandEvent &event);
  void RefreshConsoleData(void);
  void ToggleShowHighway();

  /**
   * Toggles between route total and current leg display modes.
   *
   * Switches speed calculation method and route information
   * presentation between:
   * - Current leg metrics
   * - Total route statistics
   */
  void ToggleRouteTotalDisplay();

  wxWindow *m_pParent;
  wxStaticText *pThisLegText;
  wxBoxSizer *m_pitemBoxSizerLeg;

  AnnunText *pXTE;
  AnnunText *pBRG;
  AnnunText *pRNG;
  AnnunText *pTTG;
  AnnunText *pVMG;
  CDI *pCDI;

  wxFont *pThisLegFont;
  bool m_bNeedClear;
  wxBrush *pbackBrush;

private:
  void OnPaint(wxPaintEvent &event);
  void OnShow(wxShowEvent &event);
  char m_speedUsed;

  DECLARE_EVENT_TABLE()
};

class APConsole {
public:
  APConsole(wxWindow *parent);
  virtual ~APConsole();

  void SetColorScheme(ColorScheme cs);
  bool IsShown();
  void UpdateFonts(void);
  void RefreshConsoleData(void);
  void Raise();
  void ShowWithFreshFonts(void);
  void Show(bool bshow = true);
  CDI *GetCDI();
  wxSize GetSize();
  void ToggleShowHighway();
  void Move(wxPoint p);

private:
  ConsoleCanvasWin *m_con_win;
  ConsoleCanvasFrame *m_con_frame;
};

#endif
