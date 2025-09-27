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
#include <wx/jsonval.h>
#include <wx/jsonwriter.h>
#include <wx/aui/aui.h>
#include <vector>
#include "styles.h"
#include "model/svg_utils.h"
#include "ocpn_platform.h"
#include "model/datetime.h"
#include "model/idents.h"
#include "model/plugin_comm.h"
#include "model/plugin_loader.h"
#include "ocpn_frame.h"
#include "chcanv.h"
#include "navutil.h"

extern ocpnStyle::StyleManager* g_StyleManager;
extern OCPNPlatform* g_Platform;
extern MyFrame* gFrame;

Timeline::Timeline(wxWindow* parent, const wxDateTime& timelineStartTs,
                   const wxTimeSpan& timelineDuration)
    : wxPanel(parent),
      m_startTimestamp(timelineStartTs),
      m_timelineDuration(timelineDuration),
      m_selectedTimestamp(timelineStartTs),
      m_isDragging(false),
      m_isDraggingIndicator(false),
      m_isDraggingTimeline(false),
      m_isTrackingNow(false),
      m_dragStartX(0),
      m_dragOffsetX(0) {
  m_isPlaying = false;
  m_playTimer = new wxTimer(this, wxID_ANY);
  m_systemTimeUpdateTimer = new wxTimer(this, wxID_ANY + 1);
  m_trackingTimer = new wxTimer(this, wxID_ANY + 2);
  m_autoHideTimer = new wxTimer(this, wxID_ANY + 3);
  m_animationTimer = new wxTimer(this, wxID_ANY + 4);
  m_lastWheelZoomTime = 0;

  // Initialize auto-hide functionality
  m_isVisible = true;
  m_autoHideEnabled = true;
  m_mouseOverTimeline = false;
  m_animationProgress = 1.0;
  m_autoHideDelay = 10000;  // Auto-hide delay
  m_fullHeight = 60;        // Default height in logical pixels

  RegisterCoreMessageHandler("GRIB_TIMELINE_REQUEST", this);

#ifdef HAVE_WX_GESTURE_EVENTS
  m_gestureStartDuration = wxTimeSpan();
  m_gestureStartTimestamp = wxInvalidDateTime;
#endif

  SetBackgroundColour(wxColour(240, 240, 240));  // Light grey

  // Create GUI components with better sizing
  m_timestampLabel = new wxStaticText(
      this, wxID_ANY, ocpn::toUsrDateTimeFormat(m_selectedTimestamp));
  wxFont font = m_timestampLabel->GetFont();
  font.SetPointSize(10);
  font.SetWeight(wxFONTWEIGHT_BOLD);
  m_timestampLabel->SetFont(font);
  m_timestampLabel->SetForegroundColour(GetGlobalColor("UBLCK"));

  int buttonSize = 32;
  int closeButtonSize = 20;

  wxString iconDir = g_Platform->GetSharedDataDir() + "uidata/timeline/";

  m_playButton = new wxBitmapButton(
      this, wxID_ANY, LoadSVG(iconDir + "play.svg", buttonSize, buttonSize),
      wxDefaultPosition, wxSize(buttonSize, buttonSize));
  m_nowButton = new wxBitmapToggleButton(
      this, wxID_ANY, LoadSVG(iconDir + "now.svg", buttonSize, buttonSize),
      wxDefaultPosition, wxSize(buttonSize, buttonSize));
  // Set the pressed state bitmap
  m_nowButton->SetBitmapPressed(
      LoadSVG(iconDir + "now_tracking.svg", buttonSize, buttonSize));
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
  m_closeButton = new wxBitmapButton(
      this, wxID_ANY,
      LoadSVG(iconDir + "close.svg", closeButtonSize, closeButtonSize),
      wxDefaultPosition, wxSize(closeButtonSize, closeButtonSize));

  m_playButton->SetToolTip(_("Play/Stop timeline animation"));
  m_nowButton->SetToolTip(_("Track and follow system time"));
  m_zoomInButton->SetToolTip(_("Zoom in timeline"));
  m_zoomOutButton->SetToolTip(_("Zoom out timeline"));
  m_closeButton->SetToolTip(_("Close timeline"));

  m_timelinePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
                                wxDefaultSize, wxBORDER_SUNKEN);
  m_timelinePanel->SetMinSize(wxSize(-1, 35));
  m_timelinePanel->SetMaxSize(wxSize(-1, 35));
  m_timelinePanel->SetBackgroundColour(wxColour(255, 255, 255));

  // Bind events
  Bind(wxEVT_SIZE, &Timeline::OnSize, this);

  // Bind timer events explicitly with timer IDs
  Bind(wxEVT_TIMER, &Timeline::OnPlayTimer, this, m_playTimer->GetId());
  Bind(wxEVT_TIMER, &Timeline::OnSystemTimeUpdateTimer, this,
       m_systemTimeUpdateTimer->GetId());
  Bind(wxEVT_TIMER, &Timeline::OnTrackingTimer, this, m_trackingTimer->GetId());
  Bind(wxEVT_TIMER, &Timeline::OnAutoHideTimer, this, m_autoHideTimer->GetId());
  Bind(wxEVT_TIMER, &Timeline::OnAnimationTimer, this,
       m_animationTimer->GetId());

  m_playButton->Bind(wxEVT_BUTTON, &Timeline::OnPlayPause, this);
  m_nowButton->Bind(wxEVT_TOGGLEBUTTON, &Timeline::OnNowButton, this);
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
  m_timelinePanel->Bind(wxEVT_MOUSEWHEEL, &Timeline::OnMouseWheel, this);

  // Bind mouse events for auto-hide reset
  Bind(wxEVT_ENTER_WINDOW, &Timeline::OnMouseEnter, this);
  Bind(wxEVT_MOTION, &Timeline::OnMouseMotion, this);
  Bind(wxEVT_LEAVE_WINDOW, &Timeline::OnMouseLeave, this);
  m_timelinePanel->Bind(wxEVT_ENTER_WINDOW, &Timeline::OnMouseEnter, this);
  m_timelinePanel->Bind(wxEVT_MOTION, &Timeline::OnMouseMotion, this);
  m_timelinePanel->Bind(wxEVT_LEAVE_WINDOW, &Timeline::OnMouseLeave, this);

#ifdef HAVE_WX_GESTURE_EVENTS
  // Enable zoom gesture support
  if (!m_timelinePanel->EnableTouchEvents(wxTOUCH_ZOOM_GESTURE)) {
    wxLogDebug("Failed to enable zoom gesture events for timeline");
  }
  m_timelinePanel->Bind(wxEVT_GESTURE_ZOOM, &Timeline::OnZoomGesture, this);
#endif

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

  // If selected timestamp is system time, position optimally for weather
  // forecasting.
  wxDateTime now = wxDateTime::Now();
  if (abs((m_selectedTimestamp - now).GetSeconds().ToLong()) < 60) {
    // Selected time is within 1 minute of system time, so position optimally.
    CenterSelectedTimeOptimally();
  }

  // Set initial navigation button tooltips with current time increments.
  UpdateNavigationTooltips();

  m_systemTimeUpdateTimer->Start(60000);  // Update every minute
}

Timeline::~Timeline() {
  delete m_playTimer;
  delete m_systemTimeUpdateTimer;
  delete m_trackingTimer;
  delete m_autoHideTimer;
  delete m_animationTimer;
}

void Timeline::OnPaint(wxPaintEvent& event) {
  wxPaintDC dc(m_timelinePanel);
  dc.Clear();

  // Only draw timeline content if we're supposed to be visible
  // This makes the timeline appear hidden even when AUI pane has 1px height
  if (m_isVisible && m_animationProgress > 0.0) {
    DrawTimeline(dc);
  }
}

void Timeline::OnSize(wxSizeEvent& event) {
  wxSize newSize = m_timelinePanel->GetSize();
  int newWidth = newSize.GetWidth();

  if (newWidth > 0) {
    // Calculate current position ratio of selected timestamp
    double selectedRatio = 0.5;  // Default to center
    if (m_timelineDuration.GetSeconds() > 0) {
      selectedRatio =
          (m_selectedTimestamp - m_startTimestamp).GetSeconds().ToDouble() /
          m_timelineDuration.GetSeconds().ToDouble();
      selectedRatio =
          std::max(0.0, std::min(1.0, selectedRatio));  // Clamp to [0,1]
    }

    // Calculate new timeline duration based on optimal spacing
    auto timeUnit = GetTimeUnitAndInterval();
    double dpiScale =
        gFrame ? gFrame->GetPrimaryCanvas()->GetContentScaleFactor() : 1.0;
    int targetSpacing = static_cast<int>(110 * dpiScale);
    int divisionCount = std::max(2, newWidth / targetSpacing);

    // Calculate time per division
    double secondsPerDivision;
    if (timeUnit.first == "hour") {
      secondsPerDivision = 3600.0 * timeUnit.second;
    } else if (timeUnit.first == "day") {
      secondsPerDivision = 86400.0 * timeUnit.second;
    } else if (timeUnit.first == "month") {
      secondsPerDivision = 2592000.0 * timeUnit.second;   // 30 days approx
    } else {                                              // year
      secondsPerDivision = 31536000.0 * timeUnit.second;  // 365 days
    }

    // Set new timeline duration
    wxTimeSpan newDuration = wxTimeSpan::Seconds(
        static_cast<long long>(secondsPerDivision * divisionCount));
    m_timelineDuration = newDuration;

    // Adjust start time to maintain selected timestamp ratio
    wxTimeSpan offsetFromStart = wxTimeSpan::Seconds(static_cast<long long>(
        newDuration.GetSeconds().ToLong() * selectedRatio));
    m_startTimestamp = m_selectedTimestamp - offsetFromStart;
  }

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

  // Calculate the effective timeline range for drawing.
  // During drag, extend the range to cover the visual offset area.
  wxDateTime effectiveStart = m_startTimestamp;
  wxDateTime effectiveEnd = m_startTimestamp + m_timelineDuration;

  if (m_isDraggingTimeline) {
    // Direction-aware range extension based on drag direction.
    double offsetFraction = static_cast<double>(m_dragOffsetX) / width;
    wxTimeSpan offsetTime = wxTimeSpan::Seconds(
        m_timelineDuration.GetSeconds().ToLong() * abs(offsetFraction));

    if (m_dragOffsetX > 0) {
      // Right drag (positive offset): extend left side to show content sliding
      // in from left.
      effectiveStart = m_startTimestamp - offsetTime;
      effectiveEnd = m_startTimestamp + m_timelineDuration;
    } else if (m_dragOffsetX < 0) {
      // Left drag (negative offset): extend right side to show content sliding
      // in from right.
      effectiveStart = m_startTimestamp;
      effectiveEnd = m_startTimestamp + m_timelineDuration + offsetTime;
    }
    // If m_dragOffsetX == 0, use original range (no extension needed)
  }

  wxDateTime current = effectiveStart;

  // Always align to proper time boundaries for clean display.
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
  // panning in both directions.
  if (m_isDraggingTimeline) {
    if (m_dragOffsetX > 0) {
      // Right drag: step back one more interval to ensure complete coverage.
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
      // Left drag: extend end range to ensure complete coverage.
      // This ensures smooth left dragging by pre-generating content.
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

  // Keep track of label positions to avoid overlaps.
  std::vector<std::pair<int, int>>
      labelPositions;  // Store start and end positions of drawn labels

  // Draw major time divisions
  while (current <= effectiveEnd) {
    wxLongLong currentMs = (current - m_startTimestamp).GetValue();
    wxLongLong durationMs = m_timelineDuration.GetValue();

    double fraction = static_cast<double>(currentMs.ToLong()) /
                      static_cast<double>(durationMs.ToLong());
    int x = static_cast<int>(fraction * width +
                             0.5);  // Add 0.5 for proper rounding

    // Apply drag offset for smooth panning during timeline drag.
    if (m_isDraggingTimeline) {
      x += m_dragOffsetX;
    }

    // Only draw if the line would be visible in the panel.
    if (x >= -50 && x <= width + 50) {  // Small margin for smooth transitions
      // Always draw major vertical line to mark start/end of time period
      dc.SetPen(wxPen(wxColour(160, 160, 160),
                      2));  // Darker grey for major divisions
      dc.DrawLine(x, 18, x, height);

      // Draw label only if it's at the correct interval boundary and won't
      // overlap
      bool shouldDrawLabel = false;

      // Check if this tick is at an interval boundary
      if (timeUnit == "hour") {
        shouldDrawLabel =
            (current.GetHour() % interval == 0 && current.GetMinute() == 0);
      } else if (timeUnit == "day") {
        shouldDrawLabel =
            ((current.GetDay() - 1) % interval == 0 && current.GetHour() == 0);
      } else if (timeUnit == "month") {
        shouldDrawLabel = (current.GetDay() == 1);
      } else if (timeUnit == "year") {
        shouldDrawLabel =
            (current.GetMonth() == wxDateTime::Jan && current.GetDay() == 1);
      }

      if (shouldDrawLabel) {
        wxString label = FormatDate(current, timeUnit);
        wxSize textSize = dc.GetTextExtent(label);

        // Check if we have enough space for this label
        bool canDrawLabel = true;
        int labelWidth = textSize.GetWidth();
        int labelStart = x - labelWidth / 2;
        int labelEnd = x + labelWidth / 2;

        // Check for overlap with existing labels
        for (const auto& pos : labelPositions) {
          if (!(labelEnd < pos.first - 10 ||
                labelStart > pos.second + 10)) {  // 10 pixel minimum gap
            canDrawLabel = false;
            break;
          }
        }

        if (canDrawLabel) {
          dc.SetTextForeground(wxColour(100, 100, 100));
          dc.DrawText(label, labelStart, 0);
          labelPositions.push_back({labelStart, labelEnd});
        }
      }
    }

    if (timeUnit == "hour")
      current += wxTimeSpan::Hours(1);
    else if (timeUnit == "day")
      current += wxDateSpan::Days(1);
    else if (timeUnit == "month")
      current += wxDateSpan::Months(1);
    else if (timeUnit == "year")
      current += wxDateSpan::Years(1);
  }

  // Draw minor subdivisions for better granularity
  DrawTimeSubdivisions(dc, timeUnit);
}

void Timeline::DrawTimeSubdivisions(wxDC& dc, const wxString& timeUnit) {
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

  if (timeUnit == "hour") {
    subDivisions = 4;  // 15-minute subdivisions
  } else if (timeUnit == "day") {
    subDivisions = 4;  // 6-hour subdivisions
  } else if (timeUnit == "year") {
    subDivisions = 12;  // Monthly subdivisions
  } else {
    // For months and other units, don't add subdivisions to avoid uneven
    // subdivisions
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
        subDivisionStart -= wxTimeSpan::Minutes(15);
      } else if (m_dragOffsetX < 0) {
        effectiveEnd += wxTimeSpan::Minutes(15);
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
        subDivisionStart -= wxTimeSpan::Hours(6);
      } else if (m_dragOffsetX < 0) {
        effectiveEnd += wxTimeSpan::Hours(6);
      }
    }
  } else if (timeUnit == "year") {
    // Align to month boundary
    subDivisionStart = wxDateTime(1, effectiveStart.GetMonth(),
                                  effectiveStart.GetYear(), 0, 0, 0, 0);

    // Find the first month boundary at or after effectiveStart
    while (subDivisionStart < effectiveStart) {
      subDivisionStart += wxDateSpan::Months(1);
    }

    // Extend range for smooth dragging in both directions
    if (m_isDraggingTimeline) {
      if (m_dragOffsetX > 0) {
        subDivisionStart -= wxDateSpan::Months(1);
      } else if (m_dragOffsetX < 0) {
        effectiveEnd += wxDateSpan::Months(1);
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
    } else if (timeUnit == "year") {
      // Major divisions are at year start, so skip subdivisions on Jan 1
      isOnMajorDivision =
          (current.GetMonth() == wxDateTime::Jan && current.GetDay() == 1);
    }

    if (!isOnMajorDivision && current >= effectiveStart &&
        current <= effectiveEnd) {
      // Use higher precision calculation to avoid floating point errors
      wxLongLong currentMs = (current - m_startTimestamp).GetValue();
      wxLongLong durationMs = m_timelineDuration.GetValue();

      double fraction = static_cast<double>(currentMs.ToLong()) /
                        static_cast<double>(durationMs.ToLong());
      int x = static_cast<int>(fraction * width +
                               0.5);  // Add 0.5 for proper rounding

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
      current += wxTimeSpan::Minutes(15);
    } else if (timeUnit == "day") {
      current += wxTimeSpan::Hours(6);
    } else if (timeUnit == "year") {
      current += wxDateSpan::Months(
          1);  // Add exactly 1 month for proper monthly subdivisions
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

  // Handle drag behavior for different drag types
  if (m_isDraggingIndicator && timestamp == m_selectedTimestamp) {
    // When dragging the selected time indicator, calculate its visual position
    // based on where it should appear given the current timeline state
    double currentFraction =
        (timestamp - m_startTimestamp).GetSeconds().ToDouble() /
        m_timelineDuration.GetSeconds().ToDouble();
    x = static_cast<int>(currentFraction * width);

    // The position is already calculated correctly based on the updated
    // timestamp and timeline position from OnMouseMove, so no additional offset
    // needed
  } else if (m_isDraggingTimeline && timestamp == m_selectedTimestamp) {
    // Timeline dragging logic (existing code)
    int proposedX = x + m_dragOffsetX;
    const int edgePadding = 20;

    if (proposedX < edgePadding) {
      x = edgePadding;
    } else if (proposedX > width - edgePadding) {
      x = width - edgePadding;
    } else {
      x = proposedX;
    }
  } else if (m_isDraggingTimeline && timestamp != m_selectedTimestamp) {
    // For non-selected time indicators (like "now"), apply normal drag offset
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

      // Triangle dimensions
      const int triangleWidth = 5;
      const int triangleHeight = triangleWidth + 2;

      // Top triangle - tip pointing down toward center
      wxPoint topTriangle[3];
      topTriangle[0] = wxPoint(x, triangleHeight);  // Tip pointing down
      topTriangle[1] =
          wxPoint(x - triangleWidth, 0);  // Left corner at top edge
      topTriangle[2] =
          wxPoint(x + triangleWidth, 0);  // Right corner at top edge
      dc.DrawPolygon(3, topTriangle);

      // Bottom triangle - tip pointing up toward center (positioned well within
      // bounds)
      wxPoint bottomTriangle[3];
      bottomTriangle[0] =
          wxPoint(x, height - (triangleHeight + 5));  // Tip pointing up
      bottomTriangle[1] = wxPoint(x - triangleWidth,
                                  height - 5);  // Left corner
      bottomTriangle[2] = wxPoint(x + triangleWidth,
                                  height - 5);  // Right corner
      dc.DrawPolygon(3, bottomTriangle);
    }
  }
}

void Timeline::OnPlayPause(wxCommandEvent& event) {
  ResetAutoHideTimer();  // Reset auto-hide when user interacts

  m_isPlaying = !m_isPlaying;

  if (m_isPlaying) {
    // Stop tracking if currently tracking
    if (m_isTrackingNow) {
      m_isTrackingNow = false;
      m_trackingTimer->Stop();

      // Update button to show normal state
      m_nowButton->SetValue(false);
      m_nowButton->SetToolTip(_("Track and follow system time"));
    }

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
    m_playButton->SetBitmapLabel(LoadScaledSVG(iconDir + "stop.svg", 32, 32));
    m_playButton->SetToolTip(_("Stop timeline animation"));

    m_playTimer->Start(kPlayTimerIntervalMs);
  } else {
    // Stop the timer
    StopPlaying();
  }
}

void Timeline::OnPlayTimer(wxTimerEvent& event) {
  // Calculate and apply time increment
  wxTimeSpan increment = CalculateTimeIncrement();
  wxDateTime nextTimestamp = m_selectedTimestamp + increment;

  // Check if we've reached the end before advancing.
  // Stop one increment before the absolute end to ensure there is always
  // some padding on the right side of the vertical bar.
  if (nextTimestamp >=
      (m_startTimestamp + m_timelineDuration - 5 * increment)) {
    // Stop playing - we're at the end.
    StopPlaying();
    return;
  }

  // Normal advancement
  m_selectedTimestamp = nextTimestamp;

  UpdateTimelineDisplay();

  // Notify plugins of time change.
  NotifyPluginsTimeChanged();

  // Ensure the selected timestamp remains visible.
  EnsureTimestampVisible();
}

wxTimeSpan Timeline::CalculateTimeIncrement() const {
  // Target: complete timeline traversal in 60 seconds.
  wxTimeSpan targetTraversalTime = wxTimeSpan::Seconds(60);

  double traversalTicks =
      targetTraversalTime.GetMilliseconds().ToDouble() / kPlayTimerIntervalMs;

  // Calculate increment per tick.
  wxLongLong totalMs = m_timelineDuration.GetMilliseconds();
  wxLongLong incrementMs = totalMs / static_cast<long long>(traversalTicks);

  // Ensure we have a minimum increment to avoid zero movement.
  if (incrementMs.ToLong() <= 0) {
    incrementMs = 1000;  // Default to 1 second if calculation fails.
  }

  return wxTimeSpan::Milliseconds(incrementMs);
}

void Timeline::OnZoomIn(wxCommandEvent& event) {
  if (m_timelineDuration > wxTimeSpan::Hours(4)) {
    m_timelineDuration = wxTimeSpan::Milliseconds(
        m_timelineDuration.GetMilliseconds().GetValue() / 2);
    UpdateTimelineDisplay();
    UpdateNavigationTooltips();
  }
}

void Timeline::OnZoomOut(wxCommandEvent& event) {
  if (m_timelineDuration < wxTimeSpan::Days(365 * 4)) {
    m_timelineDuration = wxTimeSpan::Milliseconds(
        m_timelineDuration.GetMilliseconds().GetValue() * 2);
    UpdateTimelineDisplay();
    UpdateNavigationTooltips();
  }
}

void Timeline::OnNowButton(wxCommandEvent& event) {
  ResetAutoHideTimer();  // Reset auto-hide when user interacts

  // Stop playing if currently playing.
  if (m_isPlaying) {
    StopPlaying();
  }

  // Toggle tracking mode.
  m_isTrackingNow = !m_isTrackingNow;

  if (m_isTrackingNow) {
    // Start tracking system time.
    m_selectedTimestamp = wxDateTime::Now();
    CenterSelectedTimeOptimally();
    UpdateTimelineDisplay();

    // Update button to show pressed/toggled state.
    m_nowButton->SetValue(true);
    m_nowButton->SetToolTip(
        _("Stop tracking system time (currently following system time)"));

    // Start the tracking timer - synchronize every minute
    m_trackingTimer->Start(60000);  // 60 seconds

    // Notify plugins of time change.
    NotifyPluginsTimeChanged();
  } else {
    // Stop tracking system time.
    m_trackingTimer->Stop();

    // Update button to show normal state.
    m_nowButton->SetValue(false);
    m_nowButton->SetToolTip(_("Track and follow system time"));
  }
}

void Timeline::CenterSelectedTimeOptimally() {
  // Position selected time at 1/20th ratio for optimal weather viewing.
  // Provide some look-back context while maximizing look-ahead time.

  // Calculate how much time should be to the left of selected time.
  wxTimeSpan lookBackTime =
      wxTimeSpan::Seconds(m_timelineDuration.GetSeconds().ToLong() / 20);

  // Set the timeline start to position selected time.
  m_startTimestamp = m_selectedTimestamp - lookBackTime;
}

void Timeline::OnSystemTimeUpdateTimer(wxTimerEvent& event) {
  m_timelinePanel->Refresh();
}

void Timeline::OnTrackingTimer(wxTimerEvent& event) {
  if (m_isTrackingNow) {
    wxDateTime now = wxDateTime::Now();

    // Only update if the time has actually changed (to avoid unnecessary
    // notifications)
    if (m_selectedTimestamp != now) {
      m_selectedTimestamp = now;

      // Keep selected time optimally positioned
      CenterSelectedTimeOptimally();
      UpdateTimelineDisplay();

      // Notify plugins of time change
      NotifyPluginsTimeChanged();
    }
  }
}

void Timeline::OnAutoHideTimer(wxTimerEvent& event) {
  // Only hide if auto-hide is enabled, timeline is visible, and mouse is not
  // over timeline
  if (m_autoHideEnabled && m_isVisible && !m_mouseOverTimeline) {
    HideTimeline(true);
  } else if (m_mouseOverTimeline) {
    // If mouse is over timeline, restart the timer to check again later
    m_autoHideTimer->Start(m_autoHideDelay, true);
  }
}

void Timeline::OnAnimationTimer(wxTimerEvent& event) {
  bool animationComplete = false;

  if (!m_isVisible) {
    // Hiding animation
    m_animationProgress -= 0.10;  // 10% per frame
    if (m_animationProgress <= 0.0) {
      m_animationProgress = 0.0;
      animationComplete = true;
    }
  } else {
    // Showing animation - fast for immediate access
    m_animationProgress += 0.2;  // 20% per frame
    if (m_animationProgress >= 1.0) {
      m_animationProgress = 1.0;
      animationComplete = true;
    }
  }

  // Update the timeline height based on animation progress
  int currentHeight = (int)(m_fullHeight * m_animationProgress);
  // Allow height to go to 0 for complete hiding
  if (currentHeight < 0) currentHeight = 0;

  // Update AUI pane size if we're managed by AUI
  wxWindow* parent = GetParent();
  if (parent && parent->IsKindOf(wxCLASSINFO(wxFrame))) {
    wxFrame* frame = static_cast<wxFrame*>(parent);
    wxAuiManager* auiMgr = wxAuiManager::GetManager(frame);
    if (auiMgr) {
      wxAuiPaneInfo& pane = auiMgr->GetPane(this);
      if (pane.IsOk()) {
        if (currentHeight == 0) {
          // For auto-hide, completely hide the pane
          // Mouse proximity detection in ChartCanvas will handle showing it
          pane.Show(false);
        } else {
          // Show pane during animation
          pane.Show(true);
          pane.MinSize(wxSize(-1, currentHeight));
          pane.MaxSize(wxSize(-1, currentHeight));
          pane.BestSize(wxSize(-1, currentHeight));
        }
        auiMgr->Update();
      }
    }
  }

  // Trigger repaint to update visual content
  Refresh();

  if (animationComplete) {
    m_animationTimer->Stop();
  }
}
void Timeline::OnMouseEnter(wxMouseEvent& event) {
  m_mouseOverTimeline = true;
  ResetAutoHideTimer();
  if (!m_isVisible) {
    ShowTimeline(true);
  }
  event.Skip();
}

void Timeline::OnMouseMotion(wxMouseEvent& event) {
  m_mouseOverTimeline = true;
  ResetAutoHideTimer();
  event.Skip();
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
  // Notify core handlers that timeline is closing (no selected time)
  SendTimelineSelectedTimeToCoreHandlers(wxInvalidDateTime, wxInvalidDateTime,
                                         wxInvalidDateTime);

  // Notify plugins that timeline is closing (no selected time)
  SendTimelineSelectedTimeToPlugins(wxInvalidDateTime, wxInvalidDateTime,
                                    wxInvalidDateTime);

  // Post a timeline toggle event to the parent frame to hide the timeline
  wxCommandEvent closeEvent(wxEVT_COMMAND_MENU_SELECTED, ID_MENU_UI_TIMELINE);
  closeEvent.SetEventObject(this);
  GetParent()->GetEventHandler()->ProcessEvent(closeEvent);
}

void Timeline::OnMouseDown(wxMouseEvent& event) {
  ResetAutoHideTimer();  // Reset auto-hide when user interacts

  // Stop any active modes when user starts interacting
  if (m_isPlaying) {
    wxCommandEvent dummyEvent;
    OnPlayPause(dummyEvent);
  }

  if (m_isTrackingNow) {
    // Stop tracking when user manually interacts with timeline
    m_isTrackingNow = false;
    m_trackingTimer->Stop();

    // Update button to show normal state
    m_nowButton->SetValue(false);
    m_nowButton->SetToolTip(_("Track and follow system time"));
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
    // Dragging the time indicator: implement smooth timeline scrolling at edges

    // Calculate where the indicator would be positioned
    int proposedIndicatorX = m_dragStartX + dx;

    // Define edge zones (20 pixels from edges) where timeline should scroll
    const int edgeScrollZone = 20;

    if (proposedIndicatorX < edgeScrollZone) {
      // Indicator would go off left edge - keep it at edge and scroll timeline
      // right
      double scrollFraction =
          static_cast<double>(edgeScrollZone - proposedIndicatorX) / panelWidth;
      wxTimeSpan scrollAmount = wxTimeSpan::Seconds(
          m_timelineDuration.GetSeconds().ToLong() * scrollFraction);

      // Move timeline start to the right (showing earlier times)
      m_startTimestamp = m_dragStartTimelineStart - scrollAmount;

      // Calculate selected timestamp based on edge position
      double edgeFraction = static_cast<double>(edgeScrollZone) / panelWidth;
      wxTimeSpan offsetFromStart = wxTimeSpan::Seconds(
          m_timelineDuration.GetSeconds().ToLong() * edgeFraction);
      m_selectedTimestamp = m_startTimestamp + offsetFromStart;

    } else if (proposedIndicatorX > panelWidth - edgeScrollZone) {
      // Indicator would go off right edge - keep it at edge and scroll timeline
      // left
      double scrollFraction =
          static_cast<double>(proposedIndicatorX -
                              (panelWidth - edgeScrollZone)) /
          panelWidth;
      wxTimeSpan scrollAmount = wxTimeSpan::Seconds(
          m_timelineDuration.GetSeconds().ToLong() * scrollFraction);

      // Move timeline start to the left (showing later times)
      m_startTimestamp = m_dragStartTimelineStart + scrollAmount;

      // Calculate selected timestamp based on edge position
      double edgeFraction =
          static_cast<double>(panelWidth - edgeScrollZone) / panelWidth;
      wxTimeSpan offsetFromStart = wxTimeSpan::Seconds(
          m_timelineDuration.GetSeconds().ToLong() * edgeFraction);
      m_selectedTimestamp = m_startTimestamp + offsetFromStart;

    } else {
      // Normal case - indicator stays within bounds, no timeline scrolling
      // needed
      double fraction = static_cast<double>(dx) / panelWidth;
      wxTimeSpan delta = wxTimeSpan::Seconds(
          m_timelineDuration.GetSeconds().ToLong() * fraction);
      m_selectedTimestamp = m_dragStartTimestamp + delta;

      // Keep the original timeline position for smooth transition back
      m_startTimestamp = m_dragStartTimelineStart;
    }

    UpdateTimelineDisplay();
    // Notify plugins of time change
    NotifyPluginsTimeChanged();
  } else if (m_isDraggingTimeline) {
    // Dragging the timeline background: implement intelligent dragging behavior
    m_dragOffsetX = dx;

    // Calculate the new timeline start position based on drag
    double fraction = static_cast<double>(-dx) / panelWidth;
    wxTimeSpan delta = wxTimeSpan::Seconds(
        m_timelineDuration.GetSeconds().ToLong() * fraction);
    wxDateTime newTimelineStart = m_dragStartTimelineStart + delta;

    // Calculate where the selected time indicator would be positioned
    double selectedFraction =
        (m_dragStartTimestamp - newTimelineStart).GetSeconds().ToDouble() /
        m_timelineDuration.GetSeconds().ToDouble();
    int selectedIndicatorX = static_cast<int>(selectedFraction * panelWidth);

    // Define padding zone (20 pixels from edges)
    const int edgePadding = 20;

    // Check if the selected time indicator would be outside the visible area
    // with padding
    if (selectedIndicatorX < edgePadding ||
        selectedIndicatorX > panelWidth - edgePadding) {
      // Selected time would go outside visible area - keep it at the edge and
      // adjust selected time
      if (selectedIndicatorX < edgePadding) {
        // Indicator would go off left edge - keep it at left padding position
        selectedFraction = static_cast<double>(edgePadding) / panelWidth;
      } else {
        // Indicator would go off right edge - keep it at right padding position
        selectedFraction =
            static_cast<double>(panelWidth - edgePadding) / panelWidth;
      }

      // Calculate the new selected timestamp that keeps the indicator at the
      // edge
      wxTimeSpan offsetFromStart = wxTimeSpan::Seconds(
          m_timelineDuration.GetSeconds().ToLong() * selectedFraction);
      wxDateTime newSelectedTime = newTimelineStart + offsetFromStart;

      // Update the actual selected timestamp and notify plugins immediately
      if (m_selectedTimestamp != newSelectedTime) {
        m_selectedTimestamp = newSelectedTime;
        // Notify plugins of time change during drag
        NotifyPluginsTimeChanged();
      }

      // Update the timestamp label to show this new selected time
      m_timestampLabel->SetLabel(ocpn::toUsrDateTimeFormat(newSelectedTime));
    } else {
      // Selected time indicator remains visible - keep original selected time
      // The visual indicator will move with the timeline drag

      // Restore original timestamp if it was temporarily changed
      if (m_selectedTimestamp != m_dragStartTimestamp) {
        m_selectedTimestamp = m_dragStartTimestamp;
        // Notify plugins of the restoration
        NotifyPluginsTimeChanged();
      }

      m_timestampLabel->SetLabel(
          ocpn::toUsrDateTimeFormat(m_dragStartTimestamp));
    }

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

    // Update the timeline start position
    m_startTimestamp = m_dragStartTimelineStart + delta;

    // The selected timestamp has already been updated during OnMouseMove,
    // so we don't need to recalculate it here. Just reset the drag offset.
    m_dragOffsetX = 0;

    // No need for additional plugin notification here since OnMouseMove
    // has been handling that in real-time
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
  m_mouseOverTimeline = false;
  if (!m_isDragging) {
    // Reset cursor to default when mouse leaves the timeline panel
    m_timelinePanel->SetCursor(wxCursor(wxCURSOR_DEFAULT));
  }
  // Don't reset the auto-hide timer immediately on leave - give the user
  // a brief moment to move back into the timeline area
  event.Skip();
}

void Timeline::UpdateTimelineDisplay() {
  m_timestampLabel->SetLabel(ocpn::toUsrDateTimeFormat(m_selectedTimestamp));
  m_timelinePanel->Refresh();
}

void Timeline::EnsureTimestampVisible() {
  // Check if selected timestamp is outside the visible range
  if (m_selectedTimestamp < m_startTimestamp) {
    // Selected time is before the visible range - position it at 1/4 from left
    // edge
    wxTimeSpan lookAheadTime =
        wxTimeSpan::Seconds(m_timelineDuration.GetSeconds().ToLong() * 3 / 4);
    m_startTimestamp =
        m_selectedTimestamp -
        wxTimeSpan::Seconds(m_timelineDuration.GetSeconds().ToLong() / 4);
  } else if (m_selectedTimestamp > m_startTimestamp + m_timelineDuration) {
    // Selected time is after the visible range - position it at 3/4 from left
    // edge
    wxTimeSpan lookBackTime =
        wxTimeSpan::Seconds(m_timelineDuration.GetSeconds().ToLong() * 3 / 4);
    m_startTimestamp = m_selectedTimestamp - lookBackTime;
  }
  // If timestamp is already visible, don't change anything
}

std::pair<wxString, int> Timeline::GetTimeUnitAndInterval() const {
  wxSize size = m_timelinePanel->GetSize();
  int width = size.GetWidth();

  if (width <= 0) {
    return std::make_pair("hour", 1);
  }

  // Target spacing between major divisions (adjusted for DPI)
  double dpiScale =
      gFrame ? gFrame->GetPrimaryCanvas()->GetContentScaleFactor() : 1.0;
  int targetSpacing = static_cast<int>(110 * dpiScale);

  // Calculate how many divisions should fit
  int divisionCount = std::max(2, width / targetSpacing);

  // Calculate seconds per division
  double totalSeconds = m_timelineDuration.GetSeconds().ToDouble();
  double secondsPerDivision = totalSeconds / divisionCount;

  // Select the best "nice" time interval
  struct TimeInterval {
    double seconds;
    wxString unit;
    int interval;
  };

  std::vector<TimeInterval> intervals = {
      {3600, "hour", 1},      // 1 hour
      {7200, "hour", 2},      // 2 hours
      {14400, "hour", 4},     // 4 hours
      {21600, "hour", 6},     // 6 hours
      {43200, "hour", 12},    // 12 hours
      {86400, "day", 1},      // 1 day
      {172800, "day", 2},     // 2 days
      {604800, "day", 7},     // 7 days
      {2592000, "month", 1},  // 1 month
      {7776000, "month", 3},  // 3 months
      {31536000, "year", 1}   // 1 year
  };

  // Find the interval closest to our target
  double bestDiff = std::numeric_limits<double>::max();
  std::pair<wxString, int> bestInterval("hour", 1);

  for (const auto& interval : intervals) {
    double diff = std::abs(interval.seconds - secondsPerDivision);
    if (diff < bestDiff) {
      bestDiff = diff;
      bestInterval = std::make_pair(interval.unit, interval.interval);
    }
  }

  return bestInterval;
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

void Timeline::UpdateNavigationTooltips() {
  wxString timeUnit;
  int interval;
  std::tie(timeUnit, interval) = GetTimeUnitAndInterval();

  wxString incrementText;
  if (timeUnit == "hour") {
    if (interval == 1) {
      incrementText = _("1 hour");
    } else {
      incrementText = wxString::Format(_("%d hours"), interval);
    }
  } else if (timeUnit == "day") {
    if (interval == 1) {
      incrementText = _("1 day");
    } else {
      incrementText = wxString::Format(_("%d days"), interval);
    }
  } else if (timeUnit == "month") {
    if (interval == 1) {
      incrementText = _("1 month");
    } else {
      incrementText = wxString::Format(_("%d months"), interval);
    }
  } else if (timeUnit == "year") {
    if (interval == 1) {
      incrementText = _("1 year");
    } else {
      incrementText = wxString::Format(_("%d years"), interval);
    }
  }

  m_leftButton->SetToolTip(
      wxString::Format(_("Move backward in time (%s)"), incrementText));
  m_rightButton->SetToolTip(
      wxString::Format(_("Move forward in time (%s)"), incrementText));
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

  // Reset auto-hide timer when user changes timestamp
  ResetAutoHideTimer();
}

void Timeline::SetAutoHide(bool enable, int hideDelay) {
  m_autoHideEnabled = enable;
  m_autoHideDelay = hideDelay;

  if (enable) {
    m_autoHideTimer->Start(hideDelay, true);
  } else {
    m_autoHideTimer->Stop();
    if (!m_isVisible) {
      ShowTimeline(true);
    }
  }
}

void Timeline::ShowTimeline(bool autohide) {
  if (m_isVisible) return;

  m_isVisible = true;

  // Always ensure AUI pane is shown first, especially for auto-hide recovery
  wxWindow* parent = GetParent();
  if (parent && parent->IsKindOf(wxCLASSINFO(wxFrame))) {
    wxFrame* frame = static_cast<wxFrame*>(parent);
    wxAuiManager* auiMgr = wxAuiManager::GetManager(frame);
    if (auiMgr) {
      wxAuiPaneInfo& pane = auiMgr->GetPane(this);
      if (pane.IsOk() && !pane.IsShown()) {
        pane.Show(true);
        auiMgr->Update();
      }
    }
  }

  if (autohide) {
    // Smooth animation for auto-hide show
    if (!m_animationTimer->IsRunning()) {
      m_animationTimer->Start(30);
    }
  } else {
    // Immediate show for explicit user action (menu or keyboard shortcut).
    m_animationProgress = 1.0;

    // Update AUI pane size immediately
    if (parent && parent->IsKindOf(wxCLASSINFO(wxFrame))) {
      wxFrame* frame = static_cast<wxFrame*>(parent);
      wxAuiManager* auiMgr = wxAuiManager::GetManager(frame);
      if (auiMgr) {
        wxAuiPaneInfo& pane = auiMgr->GetPane(this);
        if (pane.IsOk()) {
          pane.Show();  // Ensure pane is shown
          pane.MinSize(wxSize(-1, m_fullHeight));
          pane.MaxSize(wxSize(-1, m_fullHeight));
          pane.BestSize(wxSize(-1, m_fullHeight));
          auiMgr->Update();
        }
      }
    }
    // Trigger a repaint to show the content
    Refresh();
  }

  // Start auto-hide timer when timeline becomes visible (for both autohide and
  // explicit show).
  if (m_autoHideEnabled) {
    m_autoHideTimer->Start(m_autoHideDelay, true);
  }
}

void Timeline::HideTimeline(bool autohide) {
  if (!m_isVisible) return;

  m_isVisible = false;

  // Stop playing only if this is an explicit hide action (menu/keyboard
  // shortcut). Don't stop playing for auto-hide functionality.
  if (!autohide && m_isPlaying) {
    StopPlaying();
  }

  // Stop auto-hide timer when timeline is being hidden
  m_autoHideTimer->Stop();

  if (autohide) {
    // Smooth animation for auto-hide
    if (!m_animationTimer->IsRunning()) {
      m_animationTimer->Start(
          60);  // 60ms intervals for slower hiding animation
    }
  } else {
    // Immediate hide for explicit user action - also stop animation timer.
    m_animationTimer->Stop();
    m_animationProgress = 0.0;

    // Update AUI pane size immediately and hide it
    wxWindow* parent = GetParent();
    if (parent && parent->IsKindOf(wxCLASSINFO(wxFrame))) {
      wxFrame* frame = static_cast<wxFrame*>(parent);
      wxAuiManager* auiMgr = wxAuiManager::GetManager(frame);
      if (auiMgr) {
        wxAuiPaneInfo& pane = auiMgr->GetPane(this);
        if (pane.IsOk()) {
          pane.Show(
              false);  // Hide the pane completely for explicit user action.
          auiMgr->Update();
        }
      }
    }
  }
}

void Timeline::ResetAutoHideTimer() {
  if (m_autoHideEnabled && m_isVisible) {
    m_autoHideTimer->Stop();
    m_autoHideTimer->Start(m_autoHideDelay, true);
  }
}

void Timeline::SetFullHeight(int height) {
  m_fullHeight = height;

  // Start auto-hide timer now that we have proper dimensions
  if (m_autoHideEnabled) {
    m_autoHideTimer->Start(m_autoHideDelay, true);
  }
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
  UpdateNavigationTooltips();
  NotifyPluginsTimeChanged();
}

void Timeline::NotifyPluginsTimeChanged() {
  // Calculate timeline range.
  wxDateTime startTime = m_startTimestamp;
  wxDateTime endTime = m_startTimestamp + m_timelineDuration;

  // Notify core handlers.
  SendTimelineSelectedTimeToCoreHandlers(m_selectedTimestamp, startTime,
                                         endTime);

  // Notify plugins that support the timeline API.
  SendTimelineSelectedTimeToPlugins(m_selectedTimestamp, startTime, endTime);
}

void Timeline::StopPlaying() {
  // Stop the timer.
  m_playTimer->Stop();
  m_isPlaying = false;

  // Reset the play button icon.
  wxString iconDir = g_Platform->GetSharedDataDir() + "uidata/timeline/";
  m_playButton->SetBitmapLabel(LoadScaledSVG(iconDir + "play.svg", 32, 32));
  m_playButton->SetToolTip(_("Play timeline animation"));
}

void Timeline::PerformZoom(double zoomFactor, bool centerOnSelected) {
  // If timeline is playing, stop it during zoom operation.
  if (m_isPlaying) {
    wxCommandEvent dummyEvent;
    OnPlayPause(dummyEvent);
  }

  // Calculate new duration based on zoom factor.
  // Zoom in (factor > 1) should decrease duration
  // Zoom out (factor < 1) should increase duration
  wxTimeSpan newDuration = wxTimeSpan::Milliseconds(static_cast<long long>(
      m_timelineDuration.GetMilliseconds().ToDouble() / zoomFactor));

  // Apply reasonable limits to prevent extreme zoom levels
  wxTimeSpan minDuration = wxTimeSpan::Hours(1);       // Minimum 1 hour
  wxTimeSpan maxDuration = wxTimeSpan::Days(365 * 4);  // Maximum 4 years

  if (newDuration < minDuration) {
    newDuration = minDuration;
  } else if (newDuration > maxDuration) {
    newDuration = maxDuration;
  }

  // Only update if the duration actually changed
  if (newDuration != m_timelineDuration) {
    m_timelineDuration = newDuration;

    if (centerOnSelected) {
      // Keep the selected timestamp centered during zoom
      // Calculate how much time should be to the left and right of selected
      // time
      wxTimeSpan halfDuration = wxTimeSpan::Milliseconds(
          m_timelineDuration.GetMilliseconds().ToLong() / 2);

      // Update the start timestamp to keep selected time centered
      m_startTimestamp = m_selectedTimestamp - halfDuration;
    } else {
      // Ensure the selected timestamp remains visible
      EnsureTimestampVisible();
    }

    // Update the display to reflect the new zoom level
    UpdateTimelineDisplay();
    UpdateNavigationTooltips();
  }
}

void Timeline::OnMouseWheel(wxMouseEvent& event) {
  // On macOS, trackpad zoom gestures come through as wheel events
  // Check if this is a zoom gesture (typically with CMD key modifier on macOS)
  int wheelRotation = event.GetWheelRotation();

  if (wheelRotation != 0) {
    // Add rate limiting to prevent overly sensitive zooming
    wxLongLong currentTime = wxGetLocalTimeMillis();
    if (currentTime - m_lastWheelZoomTime < 50) {
      // Limit to 20 updates per second
      return;
    }
    m_lastWheelZoomTime = currentTime;

    // Determine zoom direction and factor
    double zoomFactor;
    if (wheelRotation > 0) {
      // Wheel up/zoom out - increase timeline duration to show more time
      zoomFactor = 0.95;
    } else {
      // Wheel down/zoom in - decrease timeline duration to show less time
      zoomFactor = 1.05;
    }

    // On macOS, check for command key to distinguish between scroll and zoom
    // If no command key, this might be a regular scroll - let's still treat as
    // zoom since the timeline doesn't have vertical scrolling
    PerformZoom(zoomFactor, true);
  }

  // Don't call event.Skip() to prevent further processing
}

wxBitmap Timeline::LoadScaledSVG(const wxString& filename, int buttonWidth,
                                 int buttonHeight) {
  // Leave some padding so the icon doesn't touch the button edges
  int iconSize =
      std::min(buttonWidth, buttonHeight) - 4;  // 2px padding on each side

  wxBitmap bitmap = LoadSVG(filename, iconSize, iconSize);

  // If the SVG didn't load properly or is the wrong size, try again
  if (!bitmap.IsOk() || bitmap.GetWidth() != iconSize ||
      bitmap.GetHeight() != iconSize) {
    bitmap = LoadSVG(filename, iconSize, iconSize);
    if (!bitmap.IsOk()) {
      // Create a simple fallback
      bitmap = wxBitmap(iconSize, iconSize);
      wxMemoryDC dc(bitmap);
      dc.SetBackground(wxBrush(wxColour(220, 220, 220)));
      dc.Clear();
      dc.SelectObject(wxNullBitmap);
    }
  }

  // Center the icon in the button if needed
  if (iconSize < buttonWidth || iconSize < buttonHeight) {
    wxBitmap centeredBitmap(buttonWidth, buttonHeight);
    wxMemoryDC dc(centeredBitmap);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();
    int x = (buttonWidth - iconSize) / 2;
    int y = (buttonHeight - iconSize) / 2;
    dc.DrawBitmap(bitmap, x, y);
    dc.SelectObject(wxNullBitmap);
    return centeredBitmap;
  }

  return bitmap;
}

bool Timeline::HandlePluginMessage(const wxString& message_id,
                                   const wxString& message_body) {
  if (message_id == "GRIB_TIMELINE_REQUEST") {
    // Get current timeline selection or current time as fallback
    wxDateTime timelineTime = GetSelectedTimestamp();
    if (!timelineTime.IsValid()) {
      timelineTime = wxDateTime::Now();
    }

    // Use the existing function to send the timeline message
    SendPluginTimelineMessage(timelineTime);

    return true;  // Message consumed
  }
  return false;  // Not handled by timeline
}

#ifdef HAVE_WX_GESTURE_EVENTS
void Timeline::OnZoomGesture(wxZoomGestureEvent& event) {
  // Skip spurious end zoom events
  if (event.IsGestureEnd()) {
    return;
  }

  double zoomFactor = event.GetZoomFactor();

  // Store initial state when gesture starts
  if (event.IsGestureStart()) {
    m_gestureStartDuration = m_timelineDuration;
    m_gestureStartTimestamp = m_selectedTimestamp;
  }

  // Calculate cumulative zoom factor from gesture start
  double cumulativeZoomFactor =
      m_gestureStartDuration.GetMilliseconds().ToDouble() /
      m_timelineDuration.GetMilliseconds().ToDouble() * zoomFactor;

  // Use the common zoom function
  PerformZoom(cumulativeZoomFactor, true);
}
#endif

// Global timeline functions for plugin communication
void SendTimelineSelectedTimeToPlugins(const wxDateTime& selectedTime,
                                       const wxDateTime& startTime,
                                       const wxDateTime& endTime) {
  auto plugin_array = PluginLoader::GetInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); i++) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (pic->m_enabled && pic->m_init_state) {
      switch (pic->m_api_version) {
        case 121: {
          auto* ppi = dynamic_cast<opencpn_plugin_121*>(pic->m_pplugin);
          if (ppi)
            ppi->OnTimelineSelectedTimeChanged(selectedTime, startTime,
                                               endTime);
          break;
        }
        default:
          break;
      }
    }
  }

  // Backwards compatibility - send GRIB_TIMELINE message
  SendPluginTimelineMessage(selectedTime);
}

void SendPluginTimelineMessage(const wxDateTime& time) {
  wxJSONValue v;
  if (time.IsValid()) {
    v[_T("Day")] = time.GetDay();
    v[_T("Month")] = time.GetMonth();
    v[_T("Year")] = time.GetYear();
    v[_T("Hour")] = time.GetHour();
    v[_T("Minute")] = time.GetMinute();
    v[_T("Second")] = time.GetSecond();
  } else {
    v[_T("Day")] = -1;
    v[_T("Month")] = -1;
    v[_T("Year")] = -1;
    v[_T("Hour")] = -1;
    v[_T("Minute")] = -1;
    v[_T("Second")] = -1;
  }

  wxJSONWriter w;
  wxString out;
  w.Write(v, out);

  // Send to plugins using the existing function that also posts OCPN_MsgEvent
  SendPluginMessage(wxString(_T("GRIB_TIMELINE")), out);
}
