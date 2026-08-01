/**************************************************************************
 *   Copyright (C) 2026 Alec Leamas                                        *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Minimal ChartCanvas interfaces.
 */

#ifndef AbsTraCT_ChArt_canV_h
#define AbsTraCT_ChArt_canV_h

#include <wx/bitmap.h>
#include <wx/gdicmn.h>
#include <wx/window.h>

#include "model/route_point.h"

/** Minimal ChartCAnvas interface with very little dependencies. */
class AbstractChartCanvas : public wxWindow {
public:
  AbstractChartCanvas(wxWindow* parent, wxPoint point, wxSize size, int flags)
      : wxWindow(parent, wxID_ANY, point, size, flags) {}

  virtual ~AbstractChartCanvas() = default;

  virtual void ReloadVP(bool b_adjust = true) = 0;

  virtual float GetVPScale() = 0;
  virtual int GetENCDisplayCategory() = 0;
  virtual wxBitmap* GetScratchBitmap() const = 0;
  virtual void ResetGridFont() = 0;
  virtual void ResetGlGridFont() = 0;
  virtual void EnablePaint(bool b_enable) = 0;
  virtual bool CanAccelerateGlPanning() = 0;
  virtual void SetupGlCompression() = 0;
  virtual void TriggerDeferredFocus() = 0;
  virtual void Refresh(bool eraseBackground = true,
                       const wxRect* rect = nullptr) = 0;
  virtual void InvalidateRedo() = 0;
  virtual double GetScaleValue() = 0;

  virtual double GetCanvasRangeMeters() = 0;
  virtual void SetCanvasRangeMeters(double range) = 0;

  virtual double GetPrevRlat() const = 0;
  virtual void SetPrevRlat(double lat) = 0;

  virtual double GetPrevRlon() const = 0;
  virtual void SetPrevRlon(double lat) = 0;

  virtual RoutePoint* GetPrevMousePoint() const = 0;
  virtual void SetPrevMousePoint(RoutePoint* point) = 0;

  virtual int GetRouteState() const = 0;
  virtual void SetRouteState(int state) = 0;

  virtual RoutePoint* GetSelectedRoutePoint() const = 0;
  virtual void SetSelectedRoutePoint(RoutePoint* point) = 0;
};

#endif  /// AbsTraCT_ChArt_canV_h
