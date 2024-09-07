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
#include <wx/datetime.h>
#include <vector>
#include <memory>

/**
 * Represents an element on the timeline.
 */
struct TimelineElement {
  /**
   * Represents a part of a timeline element.
   */
  struct Part {
    wxDateTime start;     ///< Start time of the part
    wxTimeSpan duration;  ///< Duration of the part
    wxColour color;       ///< Color of the part
    wxString label;       ///< Label for the part
  };

  std::vector<Part> parts;  ///< Vector of parts composing the timeline element
};

/**
 * A custom widget for displaying a timeline.
 *
 * A timeline can display various elements over a specified time range.
 * It includes controls for zooming, playing, and navigating through the
 * timeline.
 */
class Timeline : public wxPanel {
public:
  /**
   * Construct a new Timeline object
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
   * Sets the timeline elements to be displayed
   *
   * @param elements Vector of TimelineElement objects to display
   */
  void SetTimelineElements(const std::vector<TimelineElement>& elements);

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

private:
  // GUI components
  wxStaticText* m_timestampLabel;
  wxBitmapButton* m_playButton;  //< Button to play/pause the timeline.
  wxBitmapButton* m_nowButton;   //< Button to set the focus time to the current
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
  std::vector<TimelineElement> m_timelineElements;

  /** Flag indicating if the timeline is currently playing. */
  bool m_isPlaying;
  /** Flag indicating if the user is currently dragging. */
  bool m_isDragging;
  /** Flag indicating if dragging the time indicator. */
  bool m_isDraggingIndicator;
  /** Flag indicating if dragging the timeline background. */
  bool m_isDraggingTimeline;
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
  /** Timer to update the vertical line representing the current computer time.
   */
  wxTimer* m_updateTimer;
  /** Interval for updating the timeline display. */
  wxTimeSpan m_updateInterval;

  /** Last wheel zoom time for rate limiting. */
  wxLongLong m_lastWheelZoomTime;

#ifdef HAVE_WX_GESTURE_EVENTS
  /** Stores the initial timeline duration when zoom gesture starts. */
  wxTimeSpan m_gestureStartDuration;
  /** Stores the initial selected timestamp when zoom gesture starts. */
  wxDateTime m_gestureStartTimestamp;
#endif

  // Event handlers
  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnPlayPause(wxCommandEvent& event);
  void OnZoomIn(wxCommandEvent& event);
  void OnZoomOut(wxCommandEvent& event);
  void OnNowButton(wxCommandEvent& event);
  void OnMoveBackward(wxCommandEvent& event);
  void OnMoveForward(wxCommandEvent& event);
  void OnClose(wxCommandEvent& event);
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);
  void OnMouseLeave(wxMouseEvent& event);
  void OnMouseWheel(wxMouseEvent& event);
  void OnPlayTimer(wxTimerEvent& event);
  void OnUpdateTimer(wxTimerEvent& event);
#ifdef HAVE_WX_GESTURE_EVENTS
  void OnZoomGesture(wxZoomGestureEvent& event);
#endif

  // Helper methods
  void DrawTimeline(wxDC& dc);
  void DrawTimeUnits(wxDC& dc, const wxString& timeUnit, int interval);
  void DrawTimeSubdivisions(wxDC& dc, const wxString& timeUnit, int interval);
  void DrawTimelineElements(wxDC& dc);
  /**
   * Draws a vertical line representing the specified timestamp on the timeline.
   */
  void DrawTimeIndicator(wxDC& dc, const wxDateTime& timestamp,
                         const wxColour& color = wxColour(0, 0, 0));

  void UpdateTimelineDisplay();
  std::pair<wxString, int> GetTimeUnitAndInterval() const;
  wxString FormatDate(const wxDateTime& date, const wxString& timeUnit,
                      int availableWidth = 0) const;
  void EnsureTimestampVisible();

  /**
   * Returns the duration of the timeline increment for each "play" timer tick.
   * The goal is to traverse the entire timeline in about 30 seconds when
   * playing.
   */
  wxTimeSpan CalculateTimeIncrement() const;

  /**
   * Centers the current time optimally for weather forecast viewing.
   * Positions current time at 1/4 from the left, providing look-back context
   * and maximizing look-ahead time for forecasts.
   */
  void CenterCurrentTimeOptimally();

  /**
   * Helper methods for improved drag behavior
   * All coordinate calculations in this class use logical pixels.
   */
  bool IsClickOnTimeIndicator(int mouseX) const;
  int GetTimeIndicatorPosition(const wxDateTime& timestamp) const;

  /**
   * Notifies plugins when the selected timestamp changes.
   * This is called whenever m_selectedTimestamp is modified.
   */
  void NotifyPluginsTimeChanged();

  /**
   * Stop timeline playback and reset UI state.
   */
  void StopPlaying();

  /**
   * Performs zoom operation on the timeline.
   * @param zoomFactor The zoom factor (>1 for zoom in, <1 for zoom out)
   * @param centerOnSelected Whether to keep the selected timestamp centered
   */
  void PerformZoom(double zoomFactor, bool centerOnSelected = true);

  /**
   * Loads an SVG icon with proper scaling to fit within button bounds.
   * Ensures icons are not truncated and have appropriate padding.
   * @param filename Path to the SVG file
   * @param buttonWidth Width of the target button
   * @param buttonHeight Height of the target button
   * @return Properly scaled and centered bitmap
   */
  wxBitmap LoadScaledSVG(const wxString& filename, int buttonWidth,
                         int buttonHeight);
};

#endif  // TIMELINE_H
