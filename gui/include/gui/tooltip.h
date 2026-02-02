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

#ifndef _TOOLTIP_H__
#define _TOOLTIP_H__

#include <functional>

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/bitmap.h>
#include "color_types.h"

class Tooltip;  // forward
using TooltipCallback = std::function<void(const Tooltip *)>;

/**
 * Tooltip with color scheme support and high-visibility mode.
 *
 * This class is typically not instantiated directly by developers.
 * Use TooltipManager::Get().ShowTooltip()
 */
class Tooltip : public wxFrame {
public:
  Tooltip(wxWindow *parent, TooltipCallback on_destroy);
  ~Tooltip();

  /** Set the tooltip text to display */
  void SetString(const wxString &text);
  /**
   * Position tooltip relative to a window rectangle with automatic screen
   * boundary detection. The window rectangle should be in the same coordinate
   * system as the tooltip's parent.
   * The tooltip position is automatically adjusted to stay within screen
   * bounds.
   *
   * @param windowRect Rectangle of the window in parent's client coordinates
   * (physical pixels)
   * @param offsetX Horizontal offset from window's right edge in physical
   * pixels
   * @param offsetY Vertical offset from window's top edge in physical pixels
   */
  void SetRelativePosition(const wxRect &windowRect, int offsetX = 2,
                           int offsetY = 0);
  /** Enable/disable high visibility mode */
  void SetHiviz(bool hiviz);
  /** Set the color scheme for tooltip appearance */
  void SetColorScheme(ColorScheme cs);

  /**
   * Set the tooltip position in absolute screen coordinates (physical pixels).
   * Position (0,0) represents the top-left corner of the primary display.
   * All coordinates are in physical pixels, which may differ from logical
   * pixels on high-DPI displays.
   */
  void SetAbsolutePosition(const wxPoint &pt);

  /** Create the tooltip bitmap. */
  void SetBitmap();
  /** Get the rendered size of the tooltip. */
  wxSize GetRenderedSize() const;

  /**
   * Show the tooltip with optional delay in milliseconds.
   * The tooltip is positioned according to the previously set position
   * (via SetAbsolutePosition or SetRelativePosition).
   *
   * @param delay_ms Delay in milliseconds before showing the tooltip (0 =
   * immediate)
   */
  void ShowTooltip(int delay_ms = 0);
  /** Hide the tooltip immediately */
  void HideTooltip();

  bool Destroy() override;

  // Event handlers
  void OnPaint(wxPaintEvent &event);
  void OnTimer(wxTimerEvent &event);

private:
  void CreateBitmap();
  void CalculateOptimalPosition();

  wxString m_string;
  wxSize m_size;
  /** Final tooltip position in absolute screen coordinates (physical pixels)
   * after boundary adjustments. */
  wxPoint m_position;
  /** Requested tooltip position before boundary detection and
   * adjustments. */
  wxPoint m_requestedPosition;
  wxBitmap *m_pbm;
  wxColour m_back_color;
  wxColour m_text_color;
  ColorScheme m_cs;
  bool m_hiviz;

  wxTimer m_showTimer;
  bool m_showPending;
  TooltipCallback m_on_destroy;

  DECLARE_EVENT_TABLE()
};

/**
 * Coordinates tooltip display across OpenCPN components.
 * Ensures only one tooltip is shown at a time.
 *
 * Developer usage:
 *
 * 1. Standard UI widgets (buttons, checkboxes, dropdowns, etc.):
 *    Keep using: widget->SetToolTip("Help text");
 *    These work automatically with existing wxWidgets tooltip system.
 *
 * 2. Chart canvas and custom-drawn widgets:
 *    Use: TooltipManager::Get().ShowTooltipAtPosition(parent, "Text",
 * screenPosition); Required for widgets that don't use standard wxWidgets
 * painting, such as ChartCanvas where charts are rendered directly to device
 * context.
 *    - screenPosition: Absolute screen coordinates (physical pixels)
 *    - Use wxGetMousePosition() to show tooltip at current mouse cursor
 */
class TooltipManager {
public:
  /** Get the singleton instance */
  static TooltipManager &Get();

  /**
   * Show tooltip for a window using automatic positioning relative to the
   * window. The tooltip is positioned automatically next to the window
   * (typically to the right) with automatic screen boundary detection. This is
   * the recommended method for most use cases.
   *
   * @param window Window to show tooltip for (position calculated
   * automatically)
   * @param text Text to display in the tooltip
   * @param hiviz Enable high visibility mode for better contrast
   */
  void ShowTooltipForWindow(wxWindow *window, const wxString &text,
                            bool hiviz = false);
  /**
   * Show tooltip at specified position in absolute screen coordinates (physical
   * pixels). Position (0,0) represents the top-left corner of the primary
   * display. All coordinates are in physical pixels, which may differ from
   * logical pixels on high-DPI displays. Use this when you need precise control
   * over tooltip placement.
   *
   * @param parent Parent window for the tooltip
   * @param text Text to display in the tooltip
   * @param position Absolute screen position in physical pixels where tooltip
   * should appear
   * @param hiviz Enable high visibility mode for better contrast
   */
  void ShowTooltipAtPosition(wxWindow *parent, const wxString &text,
                             const wxPoint &position, bool hiviz = false);

  /** Hide the current tooltip */
  void HideTooltip();
  /** Hide all tooltips */
  void HideAllTooltips();
  /** Set color scheme for all tooltips */
  void SetColorScheme(ColorScheme cs);
  /** Enable or disable tooltip system */
  void EnableTooltips(bool enable);
  /** Check if tooltips are enabled */
  bool AreTooltipsEnabled() const { return m_enabled; }

  /** Check if a tooltip is currently shown */
  bool IsShown() const;

  /** Set delay before showing tooltips */
  void SetShowDelay(int ms) { m_showDelay = ms; }
  /** Set delay before hiding tooltips */
  void SetHideDelay(int ms) { m_hideDelay = ms; }

private:
  TooltipManager();
  ~TooltipManager();

  Tooltip *GetOrCreateTooltip(wxWindow *parent);
  void CleanupTooltip();

  Tooltip *m_currentTooltip;
  wxWindow *m_currentParent;
  ColorScheme m_colorScheme;
  bool m_enabled;
  int m_showDelay;
  int m_hideDelay;

  // Singleton instance
  static TooltipManager *s_instance;
};

#endif  // _TOOLTIP_H__
