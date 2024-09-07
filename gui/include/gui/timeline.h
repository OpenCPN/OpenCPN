/***************************************************************************
 *   Copyright (C) 2025 by OpenCPN development team                        *
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

#ifndef TIMELINE_H
#define TIMELINE_H

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/datetime.h>
#include <vector>
#include <memory>

#include "model/plugin_comm.h"

/**
 * A custom widget for displaying a timeline.
 *
 * A timeline can display various elements over a specified time range.
 * It includes controls for zooming, playing, and navigating through the
 * timeline.
 */
class Timeline : public wxPanel, public CoreMessageHandler {
public:
  /** Timer interval for timeline playback in milliseconds */
  static const int kPlayTimerIntervalMs = 100;

  /**
   * Construct a new Timeline object.
   *
   * @param parent The parent window
   * @param start The start date/time for the timeline
   * @param timelineDuration The duration of the timeline
   */
  Timeline(wxWindow* parent,
           const wxDateTime& timelineStartTs = wxDateTime::Now(),
           const wxTimeSpan& timelineDuration = wxTimeSpan::Days(16));

  ~Timeline();

  /**
   * Gets the currently selected timestamp.
   *
   * @return The currently selected timestamp
   */
  wxDateTime GetSelectedTimestamp() const { return m_selectedTimestamp; }

  /**
   * Sets the currently selected timestamp.
   *
   * @param timestamp The timestamp to set as selected
   */
  void SetSelectedTimestamp(const wxDateTime& timestamp);

  /**
   * Configures the timeline with comprehensive settings.
   *
   * @param selectedTime The currently focused time
   * @param duration Timeline duration (used if start/end times invalid)
   * @param selectedPosition Position of selected time (0.0=start, 1.0=end)
   */
  void ConfigureTimeline(const wxDateTime& selectedTime,
                         const wxTimeSpan& duration, double selectedPosition);

  /**
   * Returns true if the timeline is currently playing.
   */
  bool IsPlaying() const { return m_isPlaying; }

  /**
   * Enable or disable auto-hide functionality.
   * @param enable True to enable auto-hide, false to disable
   * @param hideDelay Delay in milliseconds before auto-hiding (default: 3000)
   */
  void SetAutoHide(bool enable, int hideDelay = 3000);

  /**
   * Show the timeline content. Used by both menu toggle and auto-hide.
   * @param autohide Whether this is triggered by auto-hide (true) or explicit
   * user action (false)
   */
  void ShowTimeline(bool autohide = true);

  /**
   * Hide the timeline content. Used by both menu toggle and auto-hide.
   * @param autohide Whether this is triggered by auto-hide (true) or explicit
   * user action (false)
   */
  void HideTimeline(bool autohide = true);

  /**
   * Reset the auto-hide timer (call when user interacts with timeline).
   */
  void ResetAutoHideTimer();

  /**
   * Returns true if the timeline content is currently visible (not hidden by
   * auto-hide). Note: This checks auto-hide state, not AUI pane visibility.
   */
  bool IsTimelineVisible() const { return m_isVisible; }

  /**
   * Set the full height for animation calculations.
   * Should be called after timeline is added to AUI manager.
   * @param height The full height in pixels when timeline is completely visible
   */
  void SetFullHeight(int height);

  bool HandlePluginMessage(const wxString& message_id,
                           const wxString& message_body) override;

private:
  // GUI components
  wxStaticText* m_timestampLabel;
  wxBitmapButton* m_playButton;  //< Button to play/pause the timeline.
  wxBitmapToggleButton*
      m_nowButton;  //< Button to set the focus time to the current
                    // computer time.
  wxBitmapButton*
      m_leftButton;  //< Button to move the focus time backward (left).
  wxBitmapButton*
      m_rightButton;  //< Button to move the focus time forward (right).
  wxBitmapButton* m_zoomInButton;   //< Button to zoom in the timeline.
  wxBitmapButton* m_zoomOutButton;  //< Button to zoom out the timeline.
  wxBitmapButton* m_closeButton;    //< Button to close/hide the timeline.
  wxPanel* m_timelinePanel;
  wxBoxSizer* m_ctrlPanel;

  /**
   * The start time of the timeline.
   * The user may adjust the start time by zooming in/out or moving the
   * timeline.
   */
  wxDateTime m_startTimestamp;
  /**
   * The duration of the timeline.
   * end time = m_startTimestamp + m_timelineDuration
   */
  wxTimeSpan m_timelineDuration;
  /**
   * The current focus time.
   * This is the selected time on the timeline, which may be adjusted by the
   * user by dragging the vertical line representing the focus time, or when
   * playing the timeline.
   */
  wxDateTime m_selectedTimestamp;

  /** Flag indicating if the timeline is currently playing. */
  bool m_isPlaying;
  /** Flag indicating if the user is currently dragging. */
  bool m_isDragging;
  /** Flag indicating if dragging the time indicator. */
  bool m_isDraggingIndicator;
  /** Flag indicating if dragging the timeline background. */
  bool m_isDraggingTimeline;
  /** Flag indicating if the timeline is tracking system time. */
  bool m_isTrackingNow;
  /** X-coordinate where drag started (logical pixels). */
  int m_dragStartX;
  /** Current drag offset for smooth visual feedback (logical pixels). */
  int m_dragOffsetX;
  /** Timestamp when drag started. */
  wxDateTime m_dragStartTimestamp;
  /** Timeline start when drag started. */
  wxDateTime m_dragStartTimelineStart;
  /** Timer to increment the focusTime when playing. */
  wxTimer* m_playTimer;
  /** Timer to update the vertical line representing the current system time. */
  wxTimer* m_systemTimeUpdateTimer;
  /** Timer to track system time when in tracking mode. */
  wxTimer* m_trackingTimer;

  /** Last wheel zoom time for rate limiting. */
  wxLongLong m_lastWheelZoomTime;

  // Auto-hide functionality
  /** Timer for auto-hide functionality. */
  wxTimer* m_autoHideTimer;
  /** Timer for smooth show/hide animation. */
  wxTimer* m_animationTimer;
  /** Flag indicating if timeline is currently visible (animation state). */
  bool m_isVisible;
  /** Flag indicating if timeline should be hidden automatically. */
  bool m_autoHideEnabled;
  /** Flag indicating if mouse is currently over the timeline panel. */
  bool m_mouseOverTimeline;
  /** Current animation progress (0.0 = hidden, 1.0 = visible). */
  double m_animationProgress;
  /** Duration for auto-hide delay in milliseconds. */
  int m_autoHideDelay;
  /** Full timeline height in logical pixels for animation calculations. */
  int m_fullHeight;

#ifdef HAVE_WX_GESTURE_EVENTS
  /** Stores the initial timeline duration when zoom gesture starts. */
  wxTimeSpan m_gestureStartDuration;
  /** Stores the initial selected timestamp when zoom gesture starts. */
  wxDateTime m_gestureStartTimestamp;
#endif

  // Event handlers
  /** Renders the timeline with time markers and indicators. */
  void OnPaint(wxPaintEvent& event);
  /** Refreshes timeline display when window dimensions change. */
  void OnSize(wxSizeEvent& event);
  /** Toggles timeline playback between play and pause states. */
  void OnPlayPause(wxCommandEvent& event);
  /** Decreases timeline duration to show more detail. */
  void OnZoomIn(wxCommandEvent& event);
  /** Increases timeline duration to show broader time range. */
  void OnZoomOut(wxCommandEvent& event);
  /** Toggles between tracking current system time and manual time selection. */
  void OnNowButton(wxCommandEvent& event);
  /** Moves selected time backward by one time unit. */
  void OnMoveBackward(wxCommandEvent& event);
  /** Moves selected time forward by one time unit. */
  void OnMoveForward(wxCommandEvent& event);
  /** Closes the timeline and notifies plugins of deactivation. */
  void OnClose(wxCommandEvent& event);
  /** Initiates drag operation for time indicator or timeline panning. */
  void OnMouseDown(wxMouseEvent& event);
  /** Updates time selection or timeline position during drag operations. */
  void OnMouseMove(wxMouseEvent& event);
  /** Commits drag operation and finalizes time or position changes. */
  void OnMouseUp(wxMouseEvent& event);
  /** Restores default cursor when mouse exits timeline area. */
  void OnMouseLeave(wxMouseEvent& event);
  /** Zooms timeline in or out based on wheel direction. */
  void OnMouseWheel(wxMouseEvent& event);
  /** Advances selected time during timeline playback. */
  void OnPlayTimer(wxTimerEvent& event);
  /** Updates display of current system time indicator. */
  void OnSystemTimeUpdateTimer(wxTimerEvent& event);
  /** Synchronizes selected time with system time when tracking is enabled. */
  void OnTrackingTimer(wxTimerEvent& event);
  /** Triggers timeline hiding after inactivity timeout. */
  void OnAutoHideTimer(wxTimerEvent& event);
  /** Handles smooth show/hide animation frame updates. */
  void OnAnimationTimer(wxTimerEvent& event);
  /** Shows timeline when mouse approaches and resets auto-hide timer. */
  void OnMouseEnter(wxMouseEvent& event);
  /** Resets auto-hide timer to prevent premature hiding during interaction. */
  void OnMouseMotion(wxMouseEvent& event);
#ifdef HAVE_WX_GESTURE_EVENTS
  /** Provides touch-based timeline zooming on supported platforms. */
  void OnZoomGesture(wxZoomGestureEvent& event);
#endif

  // Helper methods
  /** Orchestrates complete timeline rendering including time markers and
   * indicators. */
  void DrawTimeline(wxDC& dc);
  /** Renders major time divisions and labels for navigation reference. */
  void DrawTimeUnits(wxDC& dc, const wxString& timeUnit, int interval);
  /** Adds minor time subdivisions for improved precision and visual guidance.
   */
  void DrawTimeSubdivisions(wxDC& dc, const wxString& timeUnit);
  /**
   * Draws a vertical line representing the selected timestamp on the timeline.
   */
  void DrawTimeIndicator(wxDC& dc, const wxDateTime& timestamp,
                         const wxColour& color = wxColour(0, 0, 0));

  /** Refreshes timestamp label and triggers timeline repaint. */
  void UpdateTimelineDisplay();
  /** Determines appropriate time units and intervals based on current timeline
   * duration. */
  std::pair<wxString, int> GetTimeUnitAndInterval() const;
  /** Formats timestamps according to current time unit for display labels. */
  wxString FormatDate(const wxDateTime& date, const wxString& timeUnit,
                      int availableWidth = 0) const;
  /** Adjusts timeline start position to keep selected timestamp visible. */
  void EnsureTimestampVisible();
  /** Updates navigation button tooltips with current time increment values. */
  void UpdateNavigationTooltips();

  /** Calculates time increment per frame to complete timeline traversal in 30
   * seconds. */
  wxTimeSpan CalculateTimeIncrement() const;

  /** Positions selected time optimally for weather forecasting with minimal
   * lookback. */
  void CenterSelectedTimeOptimally();

  /** Determines if mouse click occurred on the time indicator for precise
   * dragging. */
  bool IsClickOnTimeIndicator(int mouseX) const;
  /** Calculates horizontal position of time indicator for rendering and hit
   * testing. */
  int GetTimeIndicatorPosition(const wxDateTime& timestamp) const;

  /** Broadcasts timestamp changes to all registered plugins and core handlers.
   */
  void NotifyPluginsTimeChanged();

  /** Stops timeline playback and restores UI to paused state. */
  void StopPlaying();

  /** Adjusts timeline duration while optionally maintaining selected time
   * position. */
  void PerformZoom(double zoomFactor, bool centerOnSelected = true);

  /** Creates properly scaled button icons from SVG files with consistent
   * padding. */
  wxBitmap LoadScaledSVG(const wxString& filename, int buttonWidth,
                         int buttonHeight);
};

// Global timeline functions for plugin communication
/**
 * Notifies all interested plugins when the selected time has changed in the
 * timeline widget.
 * @param selectedTime The newly selected timestamp in local time,
 *                     or wxInvalidDateTime if no time is selected
 * @param startTime    The start timestamp of the visible timeline range,
 *                     or wxInvalidDateTime if no range is set
 * @param endTime      The end timestamp of the visible timeline range,
 *                     or wxInvalidDateTime if no range is set
 * @note Only plugins using API v121 and later will receive this notification.
 */
void SendTimelineSelectedTimeToPlugins(const wxDateTime& selectedTime,
                                       const wxDateTime& startTime,
                                       const wxDateTime& endTime);

/**
 * Send timeline message to all plugins that support the GRIB timeline message.
 * This maintains compatibility with older plugins that expect GRIB_TIMELINE
 * messages.
 * @param time The selected timeline time, or wxInvalidDateTime for invalid time
 */
void SendPluginTimelineMessage(const wxDateTime& time);

#endif  // TIMELINE_H
