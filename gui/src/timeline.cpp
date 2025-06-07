/***************************************************************************
 *   Copyright (C) 2024 by OpenCPN development team                        *
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

#include "timeline.h"
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include "styles.h"
#include "svg_utils.h"
#include "OCPNPlatform.h"
#include "model/idents.h"
#include "model/plugin_comm.h"

extern ocpnStyle::StyleManager* g_StyleManager;
extern OCPNPlatform* g_Platform;

Timeline::Timeline(wxWindow* parent, const wxDateTime& timelineStartTs,
                   const wxTimeSpan& timelineDuration)
    : wxPanel(parent),
      m_startTimestamp(timelineStartTs),
      m_timelineDuration(timelineDuration),
      m_selectedTimestamp(timelineStartTs),
      m_isDragging(false),
      m_isDraggingIndicator(false),
      m_isDraggingTimeline(false),
      m_dragStartX(0),
      m_dragOffsetX(0) {
  m_isPlaying = false;
  m_playTimer = new wxTimer(this, wxID_ANY);
  m_updateTimer = new wxTimer(this, wxID_ANY + 1);
  m_updateInterval = wxTimeSpan::Milliseconds(250);

  // Set background to match OpenCPN's UI theme
  SetBackgroundColour(wxColour(240, 240, 240));  // Light grey background

  // Create GUI components with better sizing
  m_timestampLabel =
      new wxStaticText(this, wxID_ANY, FormatTimestamp(m_selectedTimestamp));
  wxFont font = m_timestampLabel->GetFont();
  font.SetPointSize(10);
  font.SetWeight(wxFONTWEIGHT_BOLD);
  m_timestampLabel->SetFont(font);

  int buttonSize = 32;

  wxString iconDir = g_Platform->GetSharedDataDir() + "uidata/timeline/";

  m_playButton = new wxBitmapButton(
      this, wxID_ANY, LoadSVG(iconDir + "play.svg", buttonSize, buttonSize),
      wxDefaultPosition, wxSize(buttonSize, buttonSize));
  m_nowButton = new wxBitmapButton(
      this, wxID_ANY, LoadSVG(iconDir + "now.svg", buttonSize, buttonSize),
      wxDefaultPosition, wxSize(buttonSize, buttonSize));
  m_leftButton = new wxBitmapButton(
      this, wxID_ANY, LoadSVG(iconDir + "prev.svg", buttonSize, buttonSize),
      wxDefaultPosition, wxSize(buttonSize, buttonSize));
  m_rightButton = new wxBitmapButton(
      this, wxID_ANY, LoadSVG(iconDir + "next.svg", buttonSize, buttonSize),
      wxDefaultPosition, wxSize(buttonSize, buttonSize));
  m_zoomInButton = new wxBitmapButton(
      this, wxID_ANY, LoadSVG(iconDir + "zoom_in.svg", buttonSize, buttonSize),
      wxDefaultPosition, wxSize(buttonSize, buttonSize));
  m_zoomOutButton = new wxBitmapButton(
      this, wxID_ANY, LoadSVG(iconDir + "zoom_out.svg", buttonSize, buttonSize),
      wxDefaultPosition, wxSize(buttonSize, buttonSize));
  int closeButtonSize = 20;
  m_closeButton = new wxBitmapButton(
      this, wxID_ANY,
      LoadSVG(iconDir + "close.svg", closeButtonSize, closeButtonSize),
      wxDefaultPosition, wxSize(closeButtonSize, closeButtonSize));

  m_playButton->SetToolTip(_("Play/Stop timeline animation"));
  m_nowButton->SetToolTip(_("Jump to current time"));
  m_leftButton->SetToolTip(_("Move backward in time"));
  m_rightButton->SetToolTip(_("Move forward in time"));
  m_zoomInButton->SetToolTip(_("Zoom in timeline"));
  m_zoomOutButton->SetToolTip(_("Zoom out timeline"));
  m_closeButton->SetToolTip(_("Close timeline"));

  m_timelinePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
                                wxDefaultSize, wxBORDER_SUNKEN);
  m_timelinePanel->SetMinSize(wxSize(-1, 35));
  m_timelinePanel->SetMaxSize(wxSize(-1, 35));
  m_timelinePanel->SetBackgroundColour(
      wxColour(255, 255, 255));  // White timeline area

  // Bind events
  Bind(wxEVT_PAINT, &Timeline::OnPaint, this);
  Bind(wxEVT_SIZE, &Timeline::OnSize, this);

  // Bind timer events explicitly with timer IDs
  Bind(wxEVT_TIMER, &Timeline::OnPlayTimer, this, m_playTimer->GetId());
  Bind(wxEVT_TIMER, &Timeline::OnUpdateTimer, this, m_updateTimer->GetId());

  m_playButton->Bind(wxEVT_BUTTON, &Timeline::OnPlayPause, this);
  m_nowButton->Bind(wxEVT_BUTTON, &Timeline::OnNowButton, this);
  m_leftButton->Bind(wxEVT_BUTTON, &Timeline::OnMoveBackward, this);
  m_rightButton->Bind(wxEVT_BUTTON, &Timeline::OnMoveForward, this);
  m_zoomInButton->Bind(wxEVT_BUTTON, &Timeline::OnZoomIn, this);
  m_zoomOutButton->Bind(wxEVT_BUTTON, &Timeline::OnZoomOut, this);
  m_closeButton->Bind(wxEVT_BUTTON, &Timeline::OnClose, this);

  m_timelinePanel->Bind(wxEVT_PAINT, &Timeline::OnPaint, this);
  m_timelinePanel->Bind(wxEVT_LEFT_DOWN, &Timeline::OnMouseDown, this);
  m_timelinePanel->Bind(wxEVT_MOTION, &Timeline::OnMouseMove, this);
  m_timelinePanel->Bind(wxEVT_LEFT_UP, &Timeline::OnMouseUp, this);
  m_timelinePanel->Bind(wxEVT_LEAVE_WINDOW, &Timeline::OnMouseLeave, this);

  // Layout with proper spacing
  m_ctrlPanel = new wxBoxSizer(wxHORIZONTAL);
  m_ctrlPanel->Add(m_playButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
  m_ctrlPanel->Add(m_timelinePanel, 1, wxEXPAND | wxLEFT | wxRIGHT, 4);
  m_ctrlPanel->Add(m_zoomOutButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
  m_ctrlPanel->Add(m_zoomInButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
  m_ctrlPanel->Add(m_nowButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);
  m_ctrlPanel->Add(m_leftButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);
  m_ctrlPanel->Add(m_rightButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 2);

  // Create a horizontal sizer for the top row (timestamp label + close button)
  wxBoxSizer* topRowSizer = new wxBoxSizer(wxHORIZONTAL);
  topRowSizer->Add(m_timestampLabel, 1,
                   wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL);
  topRowSizer->Add(m_closeButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 8);

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(topRowSizer, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 3);
  mainSizer->Add(m_ctrlPanel, 0, wxEXPAND | wxALL, 4);

  SetSizer(mainSizer);

  // If selected timestamp is current time, position optimally for weather
  // forecasting
  wxDateTime now = wxDateTime::Now();
  if (abs((m_selectedTimestamp - now).GetSeconds().ToLong()) < 60) {
    // Selected time is within 1 minute of current time, so position optimally
    CenterCurrentTimeOptimally();
  }

  m_updateTimer->Start(60000);  // Update every minute
}

Timeline::~Timeline() {
  delete m_playTimer;
  delete m_updateTimer;
}

void Timeline::SetTimelineElements(
    const std::vector<TimelineElement>& elements) {
  m_timelineElements = elements;
  m_timelinePanel->Refresh();
}

void Timeline::OnPaint(wxPaintEvent& event) {
  wxAutoBufferedPaintDC dc(m_timelinePanel);
  dc.Clear();
  DrawTimeline(dc);
}

void Timeline::OnSize(wxSizeEvent& event) {
  m_timelinePanel->Refresh();
  event.Skip();
}

void Timeline::DrawTimeline(wxDC& dc) {
  wxSize size = m_timelinePanel->GetSize();
  int width = size.GetWidth();
  int height = size.GetHeight();

  // Draw time units
  wxString timeUnit;
  int interval;
  std::tie(timeUnit, interval) = GetTimeUnitAndInterval();
  DrawTimeUnits(dc, timeUnit, interval);

  // Draw timeline elements
  DrawTimelineElements(dc);

  DrawTimeIndicator(dc, m_selectedTimestamp);

  /** Draw "now" indicator if it's within the visible range. */
  wxDateTime now = wxDateTime::Now();
  if (m_startTimestamp <= now && now <= m_startTimestamp + m_timelineDuration) {
    DrawTimeIndicator(dc, now,
                      wxColour(80, 140, 255));  // More visible blue for "now"
  }
}

void Timeline::DrawTimeUnits(wxDC& dc, const wxString& timeUnit, int interval) {
  wxSize size = m_timelinePanel->GetSize();
  int width = size.GetWidth();
  int height = size.GetHeight();

  // Calculate the effective timeline range for drawing
  // During drag, extend the range to cover the visual offset area
  wxDateTime effectiveStart = m_startTimestamp;
  wxDateTime effectiveEnd = m_startTimestamp + m_timelineDuration;

  if (m_isDraggingTimeline) {
    // Direction-aware range extension based on drag direction
    double offsetFraction = static_cast<double>(m_dragOffsetX) / width;
    wxTimeSpan offsetTime = wxTimeSpan::Seconds(
        m_timelineDuration.GetSeconds().ToLong() * abs(offsetFraction));

    if (m_dragOffsetX > 0) {
      // Right drag (positive offset): extend left side to show content sliding
      // in from left
      effectiveStart = m_startTimestamp - offsetTime;
      effectiveEnd = m_startTimestamp + m_timelineDuration;
    } else if (m_dragOffsetX < 0) {
      // Left drag (negative offset): extend right side to show content sliding
      // in from right
      effectiveStart = m_startTimestamp;
      effectiveEnd = m_startTimestamp + m_timelineDuration + offsetTime;
    }
    // If m_dragOffsetX == 0, use original range (no extension needed)
  }

  wxDateTime current = effectiveStart;

  // Always align to proper time boundaries for clean display
  // This ensures times show as 07:00, 08:00, etc. instead of 07:23, 08:23
  if (timeUnit == "hour") {
    // Align to hour boundary
    current = wxDateTime(current.GetDay(), current.GetMonth(),
                         current.GetYear(), current.GetHour(), 0, 0, 0);
  } else if (timeUnit == "day") {
    // Align to day boundary
    current = wxDateTime(current.GetDay(), current.GetMonth(),
                         current.GetYear(), 0, 0, 0, 0);
  } else if (timeUnit == "month") {
    // Align to month boundary
    current = wxDateTime(1, current.GetMonth(), current.GetYear(), 0, 0, 0, 0);
  } else if (timeUnit == "year") {
    // Align to year boundary
    current = wxDateTime(1, wxDateTime::Jan, current.GetYear(), 0, 0, 0, 0);
  }

  // During dragging, we need to extend content generation to ensure smooth
  // panning in both directions
  if (m_isDraggingTimeline) {
    if (m_dragOffsetX > 0) {
      // Right drag: step back one more interval to ensure complete coverage
      if (timeUnit == "hour") {
        current -= wxTimeSpan::Hours(1);
      } else if (timeUnit == "day") {
        current -= wxDateSpan::Days(1);
      } else if (timeUnit == "month") {
        current -= wxDateSpan::Months(1);
      } else if (timeUnit == "year") {
        current -= wxDateSpan::Years(1);
      }
    } else if (m_dragOffsetX < 0) {
      // Left drag: extend end range to ensure complete coverage
      // This ensures smooth left dragging by pre-generating content
      if (timeUnit == "hour") {
        effectiveEnd += wxTimeSpan::Hours(1);
      } else if (timeUnit == "day") {
        effectiveEnd += wxDateSpan::Days(1);
      } else if (timeUnit == "month") {
        effectiveEnd += wxDateSpan::Months(1);
      } else if (timeUnit == "year") {
        effectiveEnd += wxDateSpan::Years(1);
      }
    }
  }

  // Draw major time divisions
  while (current <= effectiveEnd) {
    double fraction = (current - m_startTimestamp).GetSeconds().ToDouble() /
                      m_timelineDuration.GetSeconds().ToDouble();
    int x = static_cast<int>(fraction * width);

    // Apply drag offset for smooth panning during timeline drag
    if (m_isDraggingTimeline) {
      x += m_dragOffsetX;
    }

    // Only draw if the line would be visible in the panel
    if (x >= -50 && x <= width + 50) {  // Small margin for smooth transitions
      // Draw major vertical line to mark start/end of time period
      dc.SetPen(wxPen(wxColour(160, 160, 160),
                      2));  // Darker grey for major divisions
      dc.DrawLine(x, 18, x, height);

      wxString label = FormatDate(current, timeUnit);
      wxSize textSize = dc.GetTextExtent(label);

      dc.SetTextForeground(wxColour(100, 100, 100));
      dc.DrawText(label, x - textSize.GetWidth() / 2, 0);
    }

    if (timeUnit == "hour")
      current += wxTimeSpan::Hours(interval);
    else if (timeUnit == "day")
      current += wxDateSpan::Days(interval);
    else if (timeUnit == "month")
      current += wxDateSpan::Months(interval);
    else if (timeUnit == "year")
      current += wxDateSpan::Years(interval);
  }

  // Draw minor subdivisions for better granularity
  DrawTimeSubdivisions(dc, timeUnit, interval);
}

void Timeline::DrawTimeSubdivisions(wxDC& dc, const wxString& timeUnit,
                                    int interval) {
  wxSize size = m_timelinePanel->GetSize();
  int width = size.GetWidth();
  int height = size.GetHeight();

  // Calculate the effective timeline range for drawing
  wxDateTime effectiveStart = m_startTimestamp;
  wxDateTime effectiveEnd = m_startTimestamp + m_timelineDuration;

  if (m_isDraggingTimeline) {
    // Direction-aware range extension based on drag direction
    double offsetFraction = static_cast<double>(m_dragOffsetX) / width;
    wxTimeSpan offsetTime = wxTimeSpan::Seconds(
        m_timelineDuration.GetSeconds().ToLong() * abs(offsetFraction));

    if (m_dragOffsetX > 0) {
      // Right drag (positive offset): extend left side to show content sliding
      // in from left
      effectiveStart = m_startTimestamp - offsetTime;
      effectiveEnd = m_startTimestamp + m_timelineDuration;
    } else if (m_dragOffsetX < 0) {
      // Left drag (negative offset): extend right side to show content sliding
      // in from right
      effectiveStart = m_startTimestamp;
      effectiveEnd = m_startTimestamp + m_timelineDuration + offsetTime;
    }
    // If m_dragOffsetX == 0, use original range (no extension needed)
  }

  // Determine subdivision interval based on main time unit
  int subDivisions = 1;
  wxTimeSpan subDivisionInterval;

  if (timeUnit == "hour") {
    subDivisions = 4;  // 15-minute subdivisions
    subDivisionInterval = wxTimeSpan::Minutes(15);
  } else if (timeUnit == "day") {
    subDivisions = 4;  // 6-hour subdivisions
    subDivisionInterval = wxTimeSpan::Hours(6);
  } else if (timeUnit == "month") {
    subDivisions = 4;  // Weekly subdivisions (approximately)
    subDivisionInterval = wxTimeSpan::Days(7);
  } else {
    // For years and other units, don't add subdivisions to avoid clutter
    return;
  }

  // Start from the subdivision boundary that's closest to but before
  // effectiveStart
  wxDateTime subDivisionStart = effectiveStart;

  if (timeUnit == "hour") {
    // Align to 15-minute boundary
    int minutes = subDivisionStart.GetMinute();
    int alignedMinutes = (minutes / 15) * 15;
    subDivisionStart =
        wxDateTime(subDivisionStart.GetDay(), subDivisionStart.GetMonth(),
                   subDivisionStart.GetYear(), subDivisionStart.GetHour(),
                   alignedMinutes, 0, 0);
    // Extend range for smooth dragging in both directions
    if (m_isDraggingTimeline) {
      if (m_dragOffsetX > 0) {
        subDivisionStart -= subDivisionInterval;
      } else if (m_dragOffsetX < 0) {
        effectiveEnd += subDivisionInterval;
      }
    }
  } else if (timeUnit == "day") {
    // Align to 6-hour boundary (0, 6, 12, 18)
    int hours = subDivisionStart.GetHour();
    int alignedHours = (hours / 6) * 6;
    subDivisionStart =
        wxDateTime(subDivisionStart.GetDay(), subDivisionStart.GetMonth(),
                   subDivisionStart.GetYear(), alignedHours, 0, 0, 0);
    // Extend range for smooth dragging in both directions
    if (m_isDraggingTimeline) {
      if (m_dragOffsetX > 0) {
        subDivisionStart -= subDivisionInterval;
      } else if (m_dragOffsetX < 0) {
        effectiveEnd += subDivisionInterval;
      }
    }
  } else if (timeUnit == "month") {
    // Align to weekly boundary (Monday)
    wxDateTime::WeekDay weekDay = subDivisionStart.GetWeekDay();
    int daysFromMonday = (weekDay == wxDateTime::Sun) ? 6 : weekDay - 1;
    subDivisionStart -= wxDateSpan::Days(daysFromMonday);
    subDivisionStart =
        wxDateTime(subDivisionStart.GetDay(), subDivisionStart.GetMonth(),
                   subDivisionStart.GetYear(), 0, 0, 0, 0);
    // Extend range for smooth dragging in both directions
    if (m_isDraggingTimeline) {
      if (m_dragOffsetX > 0) {
        subDivisionStart -= wxDateSpan::Days(7);
      } else if (m_dragOffsetX < 0) {
        effectiveEnd += wxDateSpan::Days(7);
      }
    }
  }

  // Draw subdivisions
  wxDateTime current = subDivisionStart;
  while (current <= effectiveEnd) {
    // Skip subdivision lines that coincide with major division lines
    bool isOnMajorDivision = false;

    if (timeUnit == "hour") {
      // Major divisions are on the hour, so skip subdivisions at minute 0
      isOnMajorDivision = (current.GetMinute() == 0);
    } else if (timeUnit == "day") {
      // Major divisions are at midnight, so skip subdivisions at hour 0
      isOnMajorDivision = (current.GetHour() == 0);
    } else if (timeUnit == "month") {
      // Major divisions are at month start, so skip subdivisions on day 1
      isOnMajorDivision = (current.GetDay() == 1);
    }

    if (!isOnMajorDivision && current >= effectiveStart &&
        current <= effectiveEnd) {
      double fraction = (current - m_startTimestamp).GetSeconds().ToDouble() /
                        m_timelineDuration.GetSeconds().ToDouble();
      int x = static_cast<int>(fraction * width);

      // Apply drag offset for smooth panning during timeline drag
      if (m_isDraggingTimeline) {
        x += m_dragOffsetX;
      }

      // Only draw if the line would be visible in the panel
      if (x >= -50 && x <= width + 50) {
        // Draw minor subdivision line
        dc.SetPen(wxPen(wxColour(200, 200, 200),
                        1));  // Very light grey for minor divisions
        dc.DrawLine(x, height - 10, x, height);  // Short lines at bottom
      }
    }

    // Move to next subdivision
    if (timeUnit == "hour") {
      current += subDivisionInterval;
    } else if (timeUnit == "day") {
      current += subDivisionInterval;
    } else if (timeUnit == "month") {
      current += wxDateSpan::Days(7);  // Add exactly 7 days
    }
  }
}

void Timeline::DrawTimelineElements(wxDC& dc) {
  wxSize size = m_timelinePanel->GetSize();
  int width = size.GetWidth();
  int height = size.GetHeight();

  int elementHeight = 8;
  int elementPadding = 2;
  int startY = 20;  // Start below the time unit labels

  for (size_t i = 0; i < m_timelineElements.size(); ++i) {
    int elementY = startY + i * (elementHeight + elementPadding);

    for (const auto& part : m_timelineElements[i].parts) {
      double startFraction =
          (part.start - m_startTimestamp).GetSeconds().ToDouble() /
          m_timelineDuration.GetSeconds().ToDouble();
      double endFraction = (part.start + part.duration - m_startTimestamp)
                               .GetSeconds()
                               .ToDouble() /
                           m_timelineDuration.GetSeconds().ToDouble();

      int startX = static_cast<int>(startFraction * width);
      int endX = static_cast<int>(endFraction * width);

      // Apply drag offset for smooth panning during timeline drag
      if (m_isDraggingTimeline) {
        startX += m_dragOffsetX;
        endX += m_dragOffsetX;
      }

      // Only draw if the element would be visible in the panel (with margin)
      if (endX >= -50 && startX <= width + 50) {
        dc.SetPen(wxPen(part.color, elementHeight));
        dc.DrawLine(startX, elementY + elementHeight / 2, endX,
                    elementY + elementHeight / 2);
      }
    }
  }
}

void Timeline::DrawTimeIndicator(wxDC& dc, const wxDateTime& timestamp,
                                 const wxColour& color) {
  wxSize size = m_timelinePanel->GetSize();
  int width = size.GetWidth();
  int height = size.GetHeight();

  double fraction = (timestamp - m_startTimestamp).GetSeconds().ToDouble() /
                    m_timelineDuration.GetSeconds().ToDouble();
  int x = static_cast<int>(fraction * width);

  // Apply drag offset for smooth panning during timeline drag
  // Note: Only apply to non-selected time indicators during timeline drag
  if (m_isDraggingTimeline && timestamp != m_selectedTimestamp) {
    x += m_dragOffsetX;
  }

  // Only draw if the indicator would be visible in the panel (with margin)
  if (x >= -50 && x <= width + 50) {
    // Check if this is the "now" indicator (blue color means it's the current
    // time)
    bool isNowIndicator =
        (color == wxColour(0, 0, 200) || color == wxColour(50, 150, 255) ||
         color == wxColour(80, 140, 255));

    if (isNowIndicator) {
      // Draw dotted line for "now" indicator with more visible blue
      wxPen dottedPen(wxColour(80, 140, 255),
                      3);  // Thicker line and more saturated blue
      dottedPen.SetStyle(wxPENSTYLE_SHORT_DASH);  // Use short dash instead of
                                                  // dot for better visibility
      dc.SetPen(dottedPen);
      dc.DrawLine(x, 0, x, height);
    } else {
      // Draw solid line for selected time indicator
      dc.SetPen(wxPen(color, 2));
      dc.DrawLine(x, 0, x, height);

      // Draw inverted triangles (tips facing each other) for selected time
      dc.SetBrush(wxBrush(color));
      dc.SetPen(wxPen(color, 1));  // Set pen for triangle outline

      // Top triangle - tip pointing down toward center
      wxPoint topTriangle[3];
      topTriangle[0] = wxPoint(x, 6);      // Tip pointing down
      topTriangle[1] = wxPoint(x - 4, 0);  // Left corner at top edge
      topTriangle[2] = wxPoint(x + 4, 0);  // Right corner at top edge
      dc.DrawPolygon(3, topTriangle);

      // Bottom triangle - tip pointing up toward center (positioned well within
      // bounds)
      wxPoint bottomTriangle[3];
      bottomTriangle[0] = wxPoint(
          x, height -
                 10);  // Tip pointing up, 10px from bottom to ensure visibility
      bottomTriangle[1] =
          wxPoint(x - 4, height - 4);  // Left corner 4px from bottom
      bottomTriangle[2] =
          wxPoint(x + 4, height - 4);  // Right corner 4px from bottom
      dc.DrawPolygon(3, bottomTriangle);
    }
  }  // End visibility check
}

void Timeline::OnPlayPause(wxCommandEvent& event) {
  m_isPlaying = !m_isPlaying;

  if (m_isPlaying) {
    // Before starting, ensure the selected timestamp is positioned for optimal
    // playback If we're at or near the end, reset to the beginning
    if (m_selectedTimestamp >=
        m_startTimestamp + m_timelineDuration - CalculateTimeIncrement()) {
      m_selectedTimestamp = m_startTimestamp;
      EnsureTimestampVisible();
      UpdateTimelineDisplay();

      // Notify plugins of time change
      NotifyPluginsTimeChanged();
    }

    // Change button to stop icon
    wxString iconDir = g_Platform->GetSharedDataDir() + "uidata/timeline/";
    m_playButton->SetBitmapLabel(LoadSVG(iconDir + "stop.svg", 32, 32));
    m_playButton->SetToolTip(_("Stop timeline animation"));

    // Start the play timer with 250ms intervals
    m_playTimer->Start(250);
  } else {
    // Stop the timer
    m_playTimer->Stop();

    // Change button back to play icon
    wxString iconDir = g_Platform->GetSharedDataDir() + "uidata/timeline/";
    m_playButton->SetBitmapLabel(LoadSVG(iconDir + "play.svg", 32, 32));
    m_playButton->SetToolTip(_("Play timeline animation"));
  }
}

void Timeline::OnPlayTimer(wxTimerEvent& event) {
  // Calculate and apply time increment
  wxTimeSpan increment = CalculateTimeIncrement();
  m_selectedTimestamp += increment;

  // Update the display
  UpdateTimelineDisplay();

  // Notify plugins of time change
  NotifyPluginsTimeChanged();

  // Ensure the selected timestamp remains visible
  EnsureTimestampVisible();

  // Stop playing when we reach the end of the timeline
  if (m_selectedTimestamp >= m_startTimestamp + m_timelineDuration) {
    m_playTimer->Stop();
    m_isPlaying = false;

    // Reset the play button icon
    wxString iconDir = g_Platform->GetSharedDataDir() + "uidata/timeline/";
    m_playButton->SetBitmapLabel(LoadSVG(iconDir + "play.svg", 32, 32));
    m_playButton->SetToolTip(_("Play timeline animation"));
  }
}

wxTimeSpan Timeline::CalculateTimeIncrement() const {
  // Target: complete timeline traversal in 30 seconds
  wxTimeSpan targetTraversalTime = wxTimeSpan::Seconds(30);

  // Calculate how many timer ticks we'll have in 30 seconds
  // Timer fires every 250ms, so 30 seconds = 120 ticks
  double traversalTicks =
      targetTraversalTime.GetMilliseconds().ToDouble() / 250.0;

  // Calculate increment per tick
  wxLongLong totalMs = m_timelineDuration.GetMilliseconds();
  wxLongLong incrementMs = totalMs / static_cast<long long>(traversalTicks);

  // Ensure we have a minimum increment to avoid zero movement
  if (incrementMs.ToLong() <= 0) {
    incrementMs = 1000;  // Default to 1 second if calculation fails
  }

  return wxTimeSpan::Milliseconds(incrementMs);
}

void Timeline::OnZoomIn(wxCommandEvent& event) {
  if (m_timelineDuration > wxTimeSpan::Hours(4)) {
    m_timelineDuration = wxTimeSpan::Milliseconds(
        m_timelineDuration.GetMilliseconds().GetValue() / 2);
    UpdateTimelineDisplay();
  }
}

void Timeline::OnZoomOut(wxCommandEvent& event) {
  if (m_timelineDuration < wxTimeSpan::Days(365 * 4)) {
    m_timelineDuration = wxTimeSpan::Milliseconds(
        m_timelineDuration.GetMilliseconds().GetValue() * 2);
    UpdateTimelineDisplay();
  }
}

void Timeline::OnNowButton(wxCommandEvent& event) {
  m_selectedTimestamp = wxDateTime::Now();
  CenterCurrentTimeOptimally();
  UpdateTimelineDisplay();

  // Notify plugins of time change
  NotifyPluginsTimeChanged();
}

void Timeline::CenterCurrentTimeOptimally() {
  // Position current time at 1/4 from the left for optimal weather viewing
  // This provides some look-back context while maximizing look-ahead time

  // Calculate how much time should be to the left of current time (25% of
  // duration)
  wxTimeSpan lookBackTime =
      wxTimeSpan::Seconds(m_timelineDuration.GetSeconds().ToLong() / 4);

  // Set the timeline start to position current time at the 1/4 mark
  m_startTimestamp = m_selectedTimestamp - lookBackTime;
}

void Timeline::OnUpdateTimer(wxTimerEvent& event) {
  // Update the display to show current time indicator
  m_timelinePanel->Refresh();
}

void Timeline::OnMoveBackward(wxCommandEvent& event) {
  wxString timeUnit;
  int interval;
  std::tie(timeUnit, interval) = GetTimeUnitAndInterval();

  if (timeUnit == "hour")
    m_selectedTimestamp -= wxTimeSpan::Hours(interval);
  else if (timeUnit == "day")
    m_selectedTimestamp -= wxDateSpan::Days(interval);
  else if (timeUnit == "month")
    m_selectedTimestamp -= wxDateSpan::Months(interval);
  else if (timeUnit == "year")
    m_selectedTimestamp -= wxDateSpan::Years(interval);

  EnsureTimestampVisible();
  UpdateTimelineDisplay();

  // Notify plugins of time change
  NotifyPluginsTimeChanged();
}

void Timeline::OnMoveForward(wxCommandEvent& event) {
  wxString timeUnit;
  int interval;
  std::tie(timeUnit, interval) = GetTimeUnitAndInterval();

  if (timeUnit == "hour")
    m_selectedTimestamp += wxTimeSpan::Hours(interval);
  else if (timeUnit == "day")
    m_selectedTimestamp += wxDateSpan::Days(interval);
  else if (timeUnit == "month")
    m_selectedTimestamp += wxDateSpan::Months(interval);
  else if (timeUnit == "year")
    m_selectedTimestamp += wxDateSpan::Years(interval);

  EnsureTimestampVisible();
  UpdateTimelineDisplay();

  // Notify plugins of time change
  NotifyPluginsTimeChanged();
}

void Timeline::OnClose(wxCommandEvent& event) {
  // Notify plugins that timeline is closing (no selected time)
  SendTimelineSelectedTimeToPlugins(wxInvalidDateTime);

  // Post a timeline toggle event to the parent frame to hide the timeline
  wxCommandEvent closeEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MENU_UI_TIMELINE);
  closeEvent.SetEventObject(this);
  GetParent()->GetEventHandler()->ProcessEvent(closeEvent);
}

void Timeline::OnMouseDown(wxMouseEvent& event) {
  if (m_isPlaying) {
    wxCommandEvent dummyEvent;
    OnPlayPause(dummyEvent);
  }

  m_isDragging = true;
  m_dragStartX = event.GetX();
  m_dragOffsetX = 0;  // Reset drag offset
  m_dragStartTimestamp = m_selectedTimestamp;
  m_dragStartTimelineStart = m_startTimestamp;

  // Determine what the user clicked on
  if (IsClickOnTimeIndicator(event.GetX())) {
    m_isDraggingIndicator = true;
    m_isDraggingTimeline = false;
    // Change cursor to indicate we're dragging the time indicator
    m_timelinePanel->SetCursor(wxCursor(wxCURSOR_SIZEWE));
  } else {
    m_isDraggingIndicator = false;
    m_isDraggingTimeline = true;
    // Change cursor to indicate we're panning the timeline
    m_timelinePanel->SetCursor(wxCursor(wxCURSOR_HAND));
  }
}

void Timeline::OnMouseMove(wxMouseEvent& event) {
  if (!m_isDragging) {
    // Update cursor based on what's under the mouse when not dragging
    if (IsClickOnTimeIndicator(event.GetX())) {
      m_timelinePanel->SetCursor(wxCursor(wxCURSOR_SIZEWE));
    } else {
      m_timelinePanel->SetCursor(wxCursor(wxCURSOR_HAND));
    }
    return;
  }

  // Use consistent coordinate system - all in logical pixels
  int currentX = event.GetX();       // Logical pixels from wxMouseEvent
  int dx = currentX - m_dragStartX;  // Logical pixel difference
  wxSize panelSize = m_timelinePanel->GetSize();  // Logical pixel size
  int panelWidth = panelSize.GetWidth();          // Logical pixels

  if (m_isDraggingIndicator) {
    // Dragging the time indicator: change the selected timestamp
    double fraction = static_cast<double>(dx) / panelWidth;
    wxTimeSpan delta = wxTimeSpan::Seconds(
        m_timelineDuration.GetSeconds().ToLong() * fraction);
    m_selectedTimestamp = m_dragStartTimestamp + delta;
    EnsureTimestampVisible();
    UpdateTimelineDisplay();

    // Notify plugins of time change
    NotifyPluginsTimeChanged();
  } else if (m_isDraggingTimeline) {
    // Dragging the timeline background: store raw pixel offset
    m_dragOffsetX = dx;

    // Update the timestamp label to show the time at the indicator position
    double fraction = static_cast<double>(-dx) / panelWidth;
    wxTimeSpan delta = wxTimeSpan::Seconds(
        m_timelineDuration.GetSeconds().ToLong() * fraction);
    wxDateTime newSelectedTime = m_dragStartTimestamp + delta;
    m_timestampLabel->SetLabel(FormatTimestamp(newSelectedTime));

    // Refresh the timeline panel to show the visual offset
    m_timelinePanel->Refresh();
  }
}

void Timeline::OnMouseUp(wxMouseEvent& event) {
  if (m_isDraggingTimeline && m_dragOffsetX != 0) {
    // Commit the timeline drag operation using consistent calculation
    wxSize panelSize = m_timelinePanel->GetSize();
    int panelWidth = panelSize.GetWidth();

    // Use the final mouse position for exact calculation
    int finalDx = event.GetX() - m_dragStartX;
    double fraction = static_cast<double>(-finalDx) / panelWidth;
    wxTimeSpan delta = wxTimeSpan::Seconds(
        m_timelineDuration.GetSeconds().ToLong() * fraction);

    // Update the actual timeline data to exactly match what was visually shown
    m_startTimestamp = m_dragStartTimelineStart + delta;
    m_selectedTimestamp = m_dragStartTimestamp + delta;

    // Reset the drag offset BEFORE the final refresh to avoid double-offset
    m_dragOffsetX = 0;

    // Notify plugins of time change
    NotifyPluginsTimeChanged();
  }

  m_isDragging = false;
  m_isDraggingIndicator = false;
  m_isDraggingTimeline = false;

  // Reset cursor to default
  m_timelinePanel->SetCursor(wxCursor(wxCURSOR_DEFAULT));

  // Final refresh to show the committed state (no visual offset anymore)
  UpdateTimelineDisplay();
}

void Timeline::OnMouseLeave(wxMouseEvent& event) {
  if (!m_isDragging) {
    // Reset cursor to default when mouse leaves the timeline panel
    m_timelinePanel->SetCursor(wxCursor(wxCURSOR_DEFAULT));
  }
}

void Timeline::UpdateTimelineDisplay() {
  m_timestampLabel->SetLabel(FormatTimestamp(m_selectedTimestamp));
  m_timelinePanel->Refresh();
}

void Timeline::EnsureTimestampVisible() {
  if (m_selectedTimestamp < m_startTimestamp)
    m_startTimestamp = m_selectedTimestamp;
  else if (m_selectedTimestamp > m_startTimestamp + m_timelineDuration)
    m_startTimestamp = m_selectedTimestamp - m_timelineDuration;
}

wxString Timeline::FormatTimestamp(const wxDateTime& timestamp) const {
  return timestamp.Format("%Y-%m-%d %H:%M %Z");
}

std::pair<wxString, int> Timeline::GetTimeUnitAndInterval() const {
  if (m_timelineDuration <= wxTimeSpan::Days(1))
    return std::make_pair("hour", 1);
  else if (m_timelineDuration <= wxTimeSpan::Days(2))
    return std::make_pair("hour", 4);
  else if (m_timelineDuration <= wxTimeSpan::Days(30))
    return std::make_pair("day", 1);
  else if (m_timelineDuration <= wxTimeSpan::Days(60))
    return std::make_pair("day", 7);
  else if (m_timelineDuration <= wxTimeSpan::Days(365))
    return std::make_pair("month", 1);
  else
    return std::make_pair("year", 1);
}

wxString Timeline::FormatDate(const wxDateTime& date, const wxString& timeUnit,
                              int availableWidth) const {
  if (timeUnit == "hour")
    return date.Format("%H:%M");
  else if (timeUnit == "day")
    return date.Format("%d %b");
  else if (timeUnit == "month")
    return date.Format("%b %Y");
  else  // year
    return date.Format("%Y");
}

bool Timeline::IsClickOnTimeIndicator(int mouseX) const {
  int indicatorX = GetTimeIndicatorPosition(m_selectedTimestamp);
  // Allow a 6-pixel tolerance on each side of the indicator line
  return (mouseX >= indicatorX - 6 && mouseX <= indicatorX + 6);
}

int Timeline::GetTimeIndicatorPosition(const wxDateTime& timestamp) const {
  wxSize size = m_timelinePanel->GetSize();
  int width = size.GetWidth();

  double fraction = (timestamp - m_startTimestamp).GetSeconds().ToDouble() /
                    m_timelineDuration.GetSeconds().ToDouble();
  return static_cast<int>(fraction * width);
}

void Timeline::SetSelectedTimestamp(const wxDateTime& timestamp) {
  m_selectedTimestamp = timestamp;
  EnsureTimestampVisible();
  UpdateTimelineDisplay();
  NotifyPluginsTimeChanged();
}

void Timeline::ConfigureTimeline(const wxDateTime& selectedTime,
                                 const wxTimeSpan& duration,
                                 double selectedPosition) {
  // Set the selected time
  m_selectedTimestamp = selectedTime;

  // Calculate start time based on duration and position
  wxTimeSpan beforeSelected = wxTimeSpan::Seconds(static_cast<long long>(
      duration.GetSeconds().ToLong() * selectedPosition));
  m_startTimestamp = selectedTime - beforeSelected;
  m_timelineDuration = duration;

  // Ensure the timeline duration is reasonable
  if (m_timelineDuration.GetSeconds() <= 0) {
    m_timelineDuration = wxTimeSpan::Days(7);  // Default fallback
  }

  // Update the display
  UpdateTimelineDisplay();
  NotifyPluginsTimeChanged();
}

void Timeline::NotifyPluginsTimeChanged() {
  // Notify plugins that support the timeline API
  SendTimelineSelectedTimeToPlugins(m_selectedTimestamp);
}
