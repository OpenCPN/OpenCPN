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

  // Member variables to control the timeline (play, zoom, drag)
  bool m_isPlaying;   //< Flag indicating if the timeline is currently playing.
  bool m_isDragging;  //< Flag indicating if the user is currently dragging.
  bool m_isDraggingIndicator;  //< Flag indicating if dragging the time
                               // indicator.
  bool m_isDraggingTimeline;   //< Flag indicating if dragging the timeline
                               // background.
  int m_dragStartX;   //< X-coordinate where drag started (logical pixels).
  int m_dragOffsetX;  //< Current drag offset for smooth visual feedback
                      //(logical pixels).
  wxDateTime m_dragStartTimestamp;      //< Timestamp when drag started.
  wxDateTime m_dragStartTimelineStart;  //< Timeline start when drag started.
  wxTimer* m_playTimer;    //< Timer to increment the focusTime when playing.
  wxTimer* m_updateTimer;  //< Timer to update the vertical line representing
                           // the current computer time.
  wxTimeSpan m_updateInterval;  //< Interval for updating the timeline display.

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
  void OnPlayTimer(wxTimerEvent& event);
  void OnUpdateTimer(wxTimerEvent& event);

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
  wxString FormatTimestamp(const wxDateTime& timestamp) const;
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
   *
   * NOTE: All coordinate calculations in this class use LOGICAL PIXELS.
   * Logical pixels are device-independent pixels returned by wxWidgets
   * mouse events and sizing functions. On high-DPI displays, logical
   * pixels may not correspond 1:1 with physical pixels, but wxWidgets
   * handles the scaling automatically.
   */
  bool IsClickOnTimeIndicator(int mouseX) const;
  int GetTimeIndicatorPosition(const wxDateTime& timestamp) const;

  /**
   * Notifies plugins when the selected timestamp changes.
   * This is called whenever m_selectedTimestamp is modified.
   */
  void NotifyPluginsTimeChanged();
};

#endif  // TIMELINE_H
