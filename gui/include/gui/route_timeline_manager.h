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

#ifndef _ROUTE_TIMELINE_MANAGER_H
#define _ROUTE_TIMELINE_MANAGER_H

#include <wx/datetime.h>
#include <wx/colour.h>

#include "model/plugin_comm.h"

class ChartCanvas;

/**
 * Manages timeline-based route position visualization.
 *
 * This singleton class handles notifications from the timeline widget and
 * coordinates the rendering of expected boat positions on routes at the
 * selected time. It implements the CoreMessageHandler interface to receive
 * timeline events directly without going through the plugin messaging system.
 */
class RouteTimelineManager : public CoreMessageHandler {
public:
  /**
   * Get the singleton instance of the route timeline manager.
   *
   * @return Reference to the singleton instance
   */
  static RouteTimelineManager& GetInstance();

  /**
   * Initialize the timeline manager and register for timeline events.
   * Should be called during application startup.
   */
  void Initialize();

  /**
   * Cleanup and unregister from timeline events.
   * Should be called during application shutdown.
   */
  void Cleanup();

  /**
   * Check if timeline position rendering is currently enabled.
   *
   * @return true if timeline positions should be rendered
   */
  bool IsTimelineRenderingEnabled() const { return m_enabled; }

  /**
   * Enable or disable timeline position rendering.
   *
   * @param enabled true to enable rendering, false to disable
   */
  void SetTimelineRenderingEnabled(bool enabled);

  /**
   * Get the current timeline timestamp for route position calculations.
   *
   * @return Current timeline timestamp, or wxInvalidDateTime if no time
   * selected
   */
  wxDateTime GetCurrentTimelineTime() const { return m_currentTime; }

  // CoreMessageHandler interface
  void OnTimelineSelectedTimeChanged(const wxDateTime& selectedTime,
                                     const wxDateTime& startTime,
                                     const wxDateTime& endTime) override;

private:
  RouteTimelineManager() = default;
  ~RouteTimelineManager() = default;

  // Prevent copying
  RouteTimelineManager(const RouteTimelineManager&) = delete;
  RouteTimelineManager& operator=(const RouteTimelineManager&) = delete;

  /**
   * Trigger refresh of all chart canvases to show updated route positions.
   */
  void RefreshAllCanvases();

  bool m_enabled = true;       ///< Whether timeline rendering is enabled
  bool m_initialized = false;  ///< Whether manager has been initialized
  wxDateTime m_currentTime = wxInvalidDateTime;  ///< Current timeline time
};

/**
 * Get the global route timeline manager instance.
 * Convenience function for accessing the singleton.
 *
 * @return Reference to the singleton instance
 */
RouteTimelineManager& GetRouteTimelineManager();

#endif  // _ROUTE_TIMELINE_MANAGER_H
