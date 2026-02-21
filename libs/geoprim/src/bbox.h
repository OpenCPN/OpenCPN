/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Latitude and Longitude Bounding Box
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2015 by Sean D'Epagnier                                 *
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
 ***************************************************************************
 */

#ifndef __BBOX_H__
#define __BBOX_H__

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/geometry.h>

enum OVERLAP { _IN, _ON, _OUT };

// Purpose   The BoundingBox class stores one BoundingBox.
// The BoundingBox is defined by two coordiates,
// a upperleft coordinate and a lowerright coordinate.
class BoundingBox {
public:
  BoundingBox();
  BoundingBox(const BoundingBox&);
  BoundingBox(const wxPoint2DDouble&);
  BoundingBox(double xmin, double ymin, double xmax, double ymax);
  virtual ~BoundingBox();

  bool And(BoundingBox*, double Marge = 0);

  void EnLarge(const double Marge);
  void Shrink(const double Marge);

  void Expand(const wxPoint2DDouble&, const wxPoint2DDouble&);
  void Expand(const wxPoint2DDouble&);
  void Expand(double x, double y);
  void Expand(const BoundingBox& bbox);

  OVERLAP Intersect(const BoundingBox&, double Marge = 0) const;

  /* this routine is used very heavily, so this is a lightweight
     version for when we only care if the other box is out */
  virtual inline bool IntersectOut(const BoundingBox& other) const {
    return (m_minx > other.m_maxx) || (m_maxx < other.m_minx) ||
           (m_maxy < other.m_miny) || (m_miny > other.m_maxy);
  }

  bool LineIntersect(const wxPoint2DDouble& begin,
                     const wxPoint2DDouble& end) const;
  bool PointInBox(const wxPoint2DDouble&, double Marge = 0) const;
  virtual bool PointInBox(double, double, double Marge = 0) const;

  void Reset();

  void Translate(wxPoint2DDouble&);

  double GetWidth() const { return m_maxx - m_minx; };
  double GetHeight() const { return m_maxy - m_miny; };
  bool GetValid() const;
  void SetValid(bool);

  void SetBoundingBox(const wxPoint2DDouble& a_point);

  void SetMin(double, double);
  void SetMax(double, double);
  inline wxPoint2DDouble GetMin() const;
  inline wxPoint2DDouble GetMax() const;
  inline double GetMinX() const { return m_minx; };
  inline double GetMinY() const { return m_miny; };
  inline double GetMaxX() const { return m_maxx; };
  inline double GetMaxY() const { return m_maxy; };

  BoundingBox& operator+(BoundingBox&);
  BoundingBox& operator=(const BoundingBox&);

protected:
  // bounding box in world
  double m_minx;
  double m_miny;
  double m_maxx;
  double m_maxy;
  bool m_validbbox;
};

/**
 * A class that represents a geographic area with coordinates expressed in
 * Lat/Lon. This class understands the International Date Line (E/W Longitude)
 * and handles calculations across it properly.
 */
class LLBBox {
public:
  LLBBox() : m_valid(FALSE) {}
  /**
   * Sets the boundaries of the bounding box with the given coordinates.
   * The box is considered valid only if minlat <= maxlat and minlon <= maxlon.
   *
   * @param minlat Minimum latitude (southern boundary)
   * @param minlon Minimum longitude (western boundary)
   * @param maxlat Maximum latitude (northern boundary)
   * @param maxlon Maximum longitude (eastern boundary)
   */
  void Set(double minlat, double minlon, double maxlat, double maxlon);
  /**
   * Creates a bounding box from a line segment defined by two geographic
   * points. This method automatically determines the most efficient
   * representation of the longitude range, handling cases that cross the
   * International Date Line by choosing the representation with the smallest
   * longitude span.
   *
   * @param lat1 Latitude of the first point
   * @param lon1 Longitude of the first point
   * @param lat2 Latitude of the second point
   * @param lon2 Longitude of the second point
   */
  void SetFromSegment(double lat1, double lon1, double lat2, double lon2);
  /**
   * Expands this bounding box to include the area of another bounding box.
   * If this bounding box is invalid, it becomes a copy of the provided box.
   * This method properly handles expansion across the International Date Line
   * by evaluating different longitude representations to find the one with the
   * smallest span.
   *
   * @param bbox The bounding box to include in this one
   */
  void Expand(const LLBBox& bbox);
  bool Contains(double Lat, double Lon) const;
  /**
   * Determines if a geographic point is contained within the bounding box,
   * with an optional margin added to the box boundaries. This method handles
   * points near the International Date Line correctly.
   *
   * @param Lat Latitude of the point to test
   * @param Lon Longitude of the point to test
   * @param Marge Margin to add to the box boundaries (in degrees)
   * @return true if the point is inside the expanded box, false otherwise
   */
  bool ContainsMarge(double Lat, double Lon, double Marge) const;
  bool GetValid() const { return m_valid; }
  void Invalidate() { m_valid = false; }
  /**
   * Tests if another bounding box is completely contained within this box.
   * Returns false if either box is invalid. This method handles boxes that
   * cross the International Date Line.
   *
   * @param other The bounding box to test for containment
   * @return true if other is completely inside this box, false otherwise
   */
  bool IntersectIn(const LLBBox& other) const;

  /**
   * Tests if another bounding box is completely outside this box.
   * It is optimized for determining non-intersection across the International
   * Date Line. Uses a small epsilon value (1e-6 degrees) for floating-point
   * comparison tolerance. epsilon is about 300 mm at the equator.
   *
   * @param other The bounding box to test for non-intersection
   * @return TRUE if boxes do not intersect, FALSE otherwise
   */
  inline bool IntersectOut(const LLBBox& other) const {
    // allow -180 to 180 or 0 to 360
    if (!GetValid() || !other.GetValid()) return true;

    if ((m_maxlat + 1e-6 < other.m_minlat) ||
        (m_minlat - 1e-6 > other.m_maxlat))
      return true;

    double minlon = m_minlon, maxlon = m_maxlon;
    if (m_maxlon < other.m_minlon)
      minlon += 360, maxlon += 360;
    else if (m_minlon > other.m_maxlon)
      minlon -= 360, maxlon -= 360;

    return (minlon - 1e-6 > other.m_maxlon) || (maxlon + 1e-6 < other.m_minlon);
  }
  bool IntersectOutGetBias(const LLBBox& other, double bias) const;

  double GetMinLat() const { return m_minlat; };
  double GetMinLon() const { return m_minlon; };
  double GetMaxLat() const { return m_maxlat; };
  double GetMaxLon() const { return m_maxlon; };

  void EnLarge(const double Marge);
  double GetLonRange() const { return m_maxlon - m_minlon; }
  double GetLatRange() const { return m_maxlat - m_minlat; };

private:
  double m_minlat;
  double m_minlon;
  double m_maxlat;
  double m_maxlon;
  bool m_valid;
};

#endif  // _WXBOUNDINGBOX_H__
