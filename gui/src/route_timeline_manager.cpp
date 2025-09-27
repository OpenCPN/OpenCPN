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

#include "route_timeline_manager.h"
#include "chcanv.h"

extern arrayofCanvasPtr g_canvasArray;

RouteTimelineManager& RouteTimelineManager::GetInstance() {
  static RouteTimelineManager instance;
  return instance;
}

void RouteTimelineManager::Initialize() {
  if (m_initialized) {
    return;  // Already initialized
  }

  // Register to receive timeline notifications
  RegisterCoreTimelineHandler(this);

  m_initialized = true;
}

void RouteTimelineManager::Cleanup() {
  if (!m_initialized) {
    return;  // Not initialized
  }

  // Unregister from all core notifications
  UnregisterCoreHandler(this);

  m_initialized = false;
  m_currentTime = wxInvalidDateTime;
}

void RouteTimelineManager::SetTimelineRenderingEnabled(bool enabled) {
  if (m_enabled != enabled) {
    m_enabled = enabled;

    // Refresh all canvases to show/hide timeline positions
    RefreshAllCanvases();
  }
}

void RouteTimelineManager::OnTimelineSelectedTimeChanged(
    const wxDateTime& selectedTime, const wxDateTime& startTime,
    const wxDateTime& endTime) {
  if (!m_initialized) {
    return;
  }

  wxDateTime oldTime = m_currentTime;
  m_currentTime = selectedTime;

  // Only refresh if timeline rendering is enabled and time actually changed
  if (m_enabled && oldTime != m_currentTime) {
    RefreshAllCanvases();
  }
}

void RouteTimelineManager::RefreshAllCanvases() {
  // Safety check: ensure g_canvasArray is valid
  if (g_canvasArray.GetCount() == 0) {
    return;  // No canvases to refresh
  }

  // Refresh all chart canvases to show updated route positions
  for (unsigned int i = 0; i < g_canvasArray.GetCount(); ++i) {
    ChartCanvas* canvas = g_canvasArray.Item(i);
    if (canvas && canvas->IsShown()) {
      canvas->Refresh(false);
    }
  }
}

// Convenience function for global access
RouteTimelineManager& GetRouteTimelineManager() {
  return RouteTimelineManager::GetInstance();
}
