/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Display utilities
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#include <iostream>
#include "config.h"
#include "displays.h"
#include "model/logger.h"

// Platform-specific includes
#if __linux__
#ifdef OCPN_HAVE_X11
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <gdk/gdk.h>
#endif
#elif _WIN32
#include <Windows.h>
#include <ShellScalingApi.h>
#include <locale>
#include <codecvt>
#elif __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif

size_t g_num_monitors = 0;
size_t g_current_monitor = 0;
double g_current_monitor_dip_px_ratio = 1.0;
std::vector<OCPN_MonitorInfo> g_monitor_info;

#if _WIN32

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor,
                              LPRECT lprcMonitor, LPARAM dwData) {
  MONITORINFOEX monitorInfo;
  monitorInfo.cbSize = sizeof(MONITORINFOEX);
  if (GetMonitorInfo(hMonitor, &monitorInfo)) {
    UINT rawdpiX, rawdpiY;
    UINT effectivedpiX, effectivedpiY;
    /* In newer SDKs (Windows 8.1+) it is much simpler to get DPI for each
     * monitor as GetDpiForMonitor actually is exposed */
    HRESULT hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &effectivedpiX,
                                  &effectivedpiY);
    if (!SUCCEEDED(hr)) {
      WARNING_LOG << "GetDpiForMonitor MDT_EFFECTIVE_DPI failed, falling back "
                     "to 96 DPI";
      effectivedpiX = 96;
      effectivedpiY = 96;
    }
    hr = GetDpiForMonitor(hMonitor, MDT_RAW_DPI, &rawdpiX, &rawdpiY);
    if (!SUCCEEDED(hr) || rawdpiX == 0 || rawdpiY == 0) {
      WARNING_LOG << "GetDpiForMonitor MDT_RAW_DPI failed, falling back to "
                  << effectivedpiX
                  << " DPI";  // This happens in virtualized environments
      rawdpiX = effectivedpiX;
      rawdpiY = effectivedpiY;
    }
    DEBUG_LOG << "Raw DPI " << rawdpiX << "x" << rawdpiY;
    DEBUG_LOG << "Effective DPI " << effectivedpiX << "x" << effectivedpiY;
    DEVICE_SCALE_FACTOR scaleFactor;
    hr = GetScaleFactorForMonitor(hMonitor, &scaleFactor);
    if (!SUCCEEDED(hr) || scaleFactor == DEVICE_SCALE_FACTOR_INVALID) {
      WARNING_LOG << "GetScaleFactorForMonitor failed, falling back to 100";
      scaleFactor = SCALE_100_PERCENT;
    }

    auto width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
    auto height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
    auto mmx = width * 25.4 / rawdpiX;
    auto mmy = height * 25.4 / rawdpiY;
    std::wstring ws(monitorInfo.szDevice);
    DEBUG_LOG << "Display " << hMonitor << ":";
    DEBUG_LOG << "  Monitor Name: " << std::string(ws.begin(), ws.end());
    DEBUG_LOG << "  Resolution: " << width << "x" << height;
    DEBUG_LOG << "  Physical Size: " << mmx << " mm x " << mmy << " mm";
    DEBUG_LOG << "  Scale factor:" << scaleFactor;
    g_monitor_info.push_back(
        {std::string(ws.begin(), ws.end()), static_cast<size_t>(mmx),
         static_cast<size_t>(mmy), static_cast<size_t>(width),
         static_cast<size_t>(height), static_cast<size_t>(width),
         static_cast<size_t>(height), static_cast<size_t>(scaleFactor)});
  } else {
    DEBUG_LOG << "GetMonitorInfo failed";
  }
  return TRUE;
}
#endif

// Function to enumerate monitors and retrieve screen size
void EnumerateMonitors() {
  g_monitor_info.clear();
#if __linux__
#ifdef OCPN_HAVE_X11
  Display* display = XOpenDisplay(nullptr);
  if (!display) {
    std::cerr << "Error opening X display." << std::endl;
    return;
  }

  int screen = DefaultScreen(display);
  Window root = RootWindow(display, screen);

  XRRScreenResources* resources = XRRGetScreenResources(display, root);
  if (!resources) {
    ERROR_LOG << "Error getting screen resources.";
    XCloseDisplay(display);
    return;
  }

  GdkDisplay* gdk_display = gdk_display_get_default();
  int gdk_num_monitors = gdk_display_get_n_monitors(gdk_display);
  DEBUG_LOG << "GDK Monitors: " << gdk_num_monitors;
  size_t scale;
  for (int i = 0; i < resources->noutput; ++i) {
    XRROutputInfo* outputInfo =
        XRRGetOutputInfo(display, resources, resources->outputs[i]);
    XRRCrtcInfo* crtcInfo =
        XRRGetCrtcInfo(display, resources, resources->crtcs[i]);
    scale = 100;
    if (i < gdk_num_monitors) {
      GdkMonitor* monitor = gdk_display_get_monitor(gdk_display, i);
      scale = gdk_monitor_get_scale_factor(monitor) * 100;
    }
    if (outputInfo && crtcInfo) {
      // Physical size can be unknown and reported as zero (For example over
      // VNC, assume a "standard" DPI of 96 in that case to guess it)
      size_t mm_width = outputInfo->mm_width > 0
                            ? outputInfo->mm_width
                            : crtcInfo->width * 25.4 / 96.0;
      size_t mm_height = outputInfo->mm_height > 0
                             ? outputInfo->mm_height
                             : crtcInfo->height * 25.4 / 96.0;
      DEBUG_LOG << "Monitor " << i + 1 << ":";
      DEBUG_LOG << "  Name: " << outputInfo->name;
      DEBUG_LOG << "  Connection: "
                << (outputInfo->connection == RR_Connected
                        ? "Connected"
                        : "Disconnected/Unknown");
      DEBUG_LOG << "  Physical Size (mm): " << mm_width << " x " << mm_height;
      DEBUG_LOG << "  Resolution: " << crtcInfo->width << " x "
                << crtcInfo->height;
      DEBUG_LOG << "  Scale: " << scale;
      if (outputInfo->connection == RR_Connected && crtcInfo->width > 0 &&
          crtcInfo->height > 0) {
        g_monitor_info.push_back({outputInfo->name, mm_width, mm_height,
                                  crtcInfo->width, crtcInfo->height,
                                  crtcInfo->width, crtcInfo->height, scale});
      }
    }
    XRRFreeOutputInfo(outputInfo);
    XRRFreeCrtcInfo(crtcInfo);
  }

  XRRFreeScreenResources(resources);
  XCloseDisplay(display);
#endif
#elif _WIN32
  EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
  // Get the names of the monitors
  std::vector<DISPLAYCONFIG_PATH_INFO> paths;
  std::vector<DISPLAYCONFIG_MODE_INFO> modes;
  UINT32 flags = QDC_ONLY_ACTIVE_PATHS;
  LONG isError = ERROR_INSUFFICIENT_BUFFER;

  UINT32 pathCount, modeCount;
  isError = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

  if (!isError) {
    // Allocate the path and mode arrays
    paths.resize(pathCount);
    modes.resize(modeCount);

    // Get all active paths and their modes
    isError = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount,
                                 modes.data(), nullptr);

    // The function may have returned fewer paths/modes than estimated
    paths.resize(pathCount);
    modes.resize(modeCount);

    if (!isError) {
      // For each active path
      for (int i = 0; i < paths.size(); i++) {
        // Find the target (monitor) friendly name
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        isError = DisplayConfigGetDeviceInfo(&targetName.header);

        if (!isError) {
          if (targetName.flags.friendlyNameFromEdid) {
            std::wstring ws(targetName.monitorFriendlyDeviceName);
            std::wstring ws1(targetName.monitorDevicePath);
            DEBUG_LOG << "Monitor found: " << std::string(ws.begin(), ws.end())
                      << " - " << std::string(ws1.begin(), ws1.end());
            if (i < g_monitor_info.size()) {
              g_monitor_info[i].name = std::string(ws.begin(), ws.end());
            }
          }
        }
      }
    }
  }
#elif __APPLE__
  CGDirectDisplayID displayArray[32];
  uint32_t displayCount;
  CGGetOnlineDisplayList(32, displayArray, &displayCount);

  for (uint32_t i = 0; i < displayCount; ++i) {
    CGDirectDisplayID displayID = displayArray[i];
    CGSize displayPhysicalSize = CGDisplayScreenSize(displayID);
    int width = CGDisplayModeGetWidth(CGDisplayCopyDisplayMode(displayID));
    int height = CGDisplayModeGetHeight(CGDisplayCopyDisplayMode(displayID));
    int pixel_width =
        CGDisplayModeGetPixelWidth(CGDisplayCopyDisplayMode(displayID));
    int pixel_height =
        CGDisplayModeGetPixelHeight(CGDisplayCopyDisplayMode(displayID));
    DEBUG_LOG << "Display " << i + 1 << ":";
    DEBUG_LOG << "  Physical Size: " << displayPhysicalSize.width << "x"
              << displayPhysicalSize.height << " mm";
    DEBUG_LOG << "  Resolution: " << width << "x" << height << " pixels";
    DEBUG_LOG << "  Pixel resolution: " << pixel_width << "x" << pixel_height
              << " pixels";
    g_monitor_info.push_back(
        {std::to_string(i + 1), static_cast<size_t>(displayPhysicalSize.width),
         static_cast<size_t>(displayPhysicalSize.height),
         static_cast<size_t>(width), static_cast<size_t>(height),
         static_cast<size_t>(pixel_width), static_cast<size_t>(pixel_height),
         100});
  }
#endif
  if (g_monitor_info.size() == 0) {
    // This should never happen, but just in case...
    // If we didn't find any monitors at all, add some dummy default that makes
    // at least some sense (15.6 inch full HD) We might also use wxDisplaySize
    // and wxDisplaySizeMM here, but what the heck would they report?
    g_monitor_info.push_back(
        {"Dummy monitor", 340, 190, 1920, 1080, 1920, 1080, 100});
  }
  g_num_monitors = g_monitor_info.size();
  DEBUG_LOG << "Number of monitors: " << g_num_monitors;
  DEBUG_LOG << "Monitor info:";
  for (const auto& monitor : g_monitor_info) {
    DEBUG_LOG << "Monitor: " << monitor.name << " " << monitor.width_mm << "x"
              << monitor.height_mm << "mm " << monitor.width << "x"
              << monitor.height << "DIP " << monitor.width_px << "x"
              << monitor.height_px << "px " << monitor.scale << "%";
  }
}
