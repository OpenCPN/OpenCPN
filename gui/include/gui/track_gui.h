/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 * Track and Trackpoint drawing stuff
 */

#ifndef TRACK_GUI_H_
#define TRACK_GUI_H_

#include <list>

#include "model/track.h"

#include "bbox.h"
#include "chcanv.h"
#include "ocpndc.h"
#include "viewport.h"

class TrackPointGui {
public:
  TrackPointGui(const TrackPoint &point) : m_point(point) {}
  void Draw(ChartCanvas *cc, ocpnDC &dc);

private:
  const TrackPoint &m_point;
};

class TrackGui {
public:
  TrackGui(Track &track) : m_track(track) {}
  void Draw(ChartCanvas *cc, ocpnDC &dc, ViewPort &VP, const LLBBox &box);

protected:
  void Segments(ChartCanvas *cc, std::list<std::list<wxPoint> > &pointlists,
                const LLBBox &box, double scale);

private:
  Track &m_track;
  void GetPointLists(ChartCanvas *cc,
                     std::list<std::list<wxPoint> > &pointlists, ViewPort &VP,
                     const LLBBox &box);
  void Finalize();
  void Assemble(ChartCanvas *cc, std::list<std::list<wxPoint> > &pointlists,
                const LLBBox &box, double scale, int &last, int level, int pos);
  void AddPointToList(ChartCanvas *cc,
                      std::list<std::list<wxPoint> > &pointlists, int n);
  void AddPointToLists(ChartCanvas *cc,
                       std::list<std::list<wxPoint> > &pointlists, int &last,
                       int n);
};

#endif  // _TRACK_GUI_H
