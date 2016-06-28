/////////////////////////////////////////////////////////////////////////////
// Name:        bbox.cpp
// Author:      Klaas Holwerda
// Created:     XX/XX/XX
// Copyright:   2000 (c) Klaas Holwerda
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "bbox.h"

wxBoundingBox::wxBoundingBox()
{
    m_minx = m_miny = m_maxx =  m_maxy = 0.0;
    m_validbbox = FALSE;
}


wxBoundingBox::wxBoundingBox(const wxBoundingBox &other)
{
    m_minx = other.m_minx;
    m_miny = other.m_miny;
    m_maxx = other.m_maxx;
    m_maxy = other.m_maxy;
    m_validbbox= other.m_validbbox;
}


wxBoundingBox::wxBoundingBox(const wxPoint2DDouble& a)
{
    m_minx = a.m_x;
    m_maxx = a.m_x;
    m_miny = a.m_y;
    m_maxy = a.m_y;
    m_validbbox = TRUE;
}

wxBoundingBox::wxBoundingBox(double xmin, double ymin, double xmax, double ymax)
{
    m_minx = xmin;
    m_miny = ymin;
    m_maxx = xmax;
    m_maxy = ymax;
    m_validbbox = TRUE;
}

wxBoundingBox::~wxBoundingBox()
{
}

// This function checks if two bboxes intersect
bool wxBoundingBox::And(wxBoundingBox *_bbox, double Marge)
{
    assert (m_validbbox == TRUE);
    assert (_bbox->GetValid());
    m_minx = wxMax(m_minx, _bbox->m_minx);
    m_maxx = wxMin(m_maxx, _bbox->m_maxx);
    m_miny = wxMax(m_miny, _bbox->m_miny);
    m_maxy = wxMin(m_maxy, _bbox->m_maxy);
    return (bool)
             (
             ((m_minx - Marge) < (m_maxx + Marge)) &&
             ((m_miny - Marge) < (m_maxy + Marge))
             );
}

// Shrink the boundingbox with the given marge
void wxBoundingBox::Shrink(const double Marge)
{
    assert (m_validbbox == TRUE);

    m_minx += Marge;
    m_maxx -= Marge;
    m_miny += Marge;
    m_maxy -= Marge;
}


// Expand the boundingbox with another boundingbox
void wxBoundingBox::Expand(const wxBoundingBox &other)
{
    if (!m_validbbox)
    {
        *this=other;
    }
    else
    {
        m_minx = wxMin(m_minx, other.m_minx);
        m_maxx = wxMax(m_maxx, other.m_maxx);
        m_miny = wxMin(m_miny, other.m_miny);
        m_maxy = wxMax(m_maxy, other.m_maxy);
    }
}


// Expand the boundingbox with a point
void wxBoundingBox::Expand(const wxPoint2DDouble& a_point)
{
    if (!m_validbbox)
    {
        m_minx = m_maxx = a_point.m_x;
        m_miny = m_maxy = a_point.m_y;
        m_validbbox=TRUE;
    }
    else
    {
        m_minx = wxMin(m_minx, a_point.m_x);
        m_maxx = wxMax(m_maxx, a_point.m_x);
        m_miny = wxMin(m_miny, a_point.m_y);
        m_maxy = wxMax(m_maxy, a_point.m_y);
    }
}

// Expand the boundingbox with a point
void wxBoundingBox::Expand(double x,double y)
{
    if (!m_validbbox)
    {
        m_minx = m_maxx = x;
        m_miny = m_maxy = y;
        m_validbbox=TRUE;
    }
    else
    {
        m_minx = wxMin(m_minx, x);
        m_maxx = wxMax(m_maxx, x);
        m_miny = wxMin(m_miny, y);
        m_maxy = wxMax(m_maxy, y);
    }
}


// Expand the boundingbox with two points
void wxBoundingBox::Expand(const wxPoint2DDouble& a, const wxPoint2DDouble& b)
{
    Expand(a);
    Expand(b);
}

// Enlarge the boundingbox with the given marge
void wxBoundingBox::EnLarge(const double marge)
{
    if (!m_validbbox)
    {
        m_minx = m_maxx = marge;
        m_miny = m_maxy = marge;
        m_validbbox=TRUE;
    }
    else
    {
        m_minx -= marge;
        m_maxx += marge;
        m_miny -= marge;
        m_maxy += marge;
    }
}

// Calculates if two boundingboxes intersect. If so, the function returns _ON.
// If they do not intersect, two scenario's are possible:
// other is outside this -> return _OUT
// other is inside this -> return _IN
OVERLAP wxBoundingBox::Intersect(const wxBoundingBox &other, double Marge) const
{
    assert (m_validbbox == TRUE);

    // other boundingbox must exist
    assert (&other);

    if (((m_minx - Marge) > (other.m_maxx + Marge)) ||
         ((m_maxx + Marge) < (other.m_minx - Marge)) ||
         ((m_maxy + Marge) < (other.m_miny - Marge)) ||
         ((m_miny - Marge) > (other.m_maxy + Marge)))
        return _OUT;

    // Check if other.bbox is inside this bbox
    if ((m_minx <= other.m_minx) &&
         (m_maxx >= other.m_maxx) &&
         (m_maxy >= other.m_maxy) &&
         (m_miny <= other.m_miny))
        return _IN;

    // Boundingboxes intersect
    return _ON;
}


// Checks if a line intersects the boundingbox
bool wxBoundingBox::LineIntersect(const wxPoint2DDouble& begin, const wxPoint2DDouble& end ) const
{
    assert (m_validbbox == TRUE);

    return (bool)
              !(((begin.m_y > m_maxy) && (end.m_y > m_maxy)) ||
                ((begin.m_y < m_miny) && (end.m_y < m_miny)) ||
                ((begin.m_x > m_maxx) && (end.m_x > m_maxx)) ||
                ((begin.m_x < m_minx) && (end.m_x < m_minx)));
}


// Is the given point in the boundingbox ??
bool wxBoundingBox::PointInBox(double x, double y, double Marge) const
{
    assert (m_validbbox == TRUE);

    if (  x >= (m_minx - Marge) && x <= (m_maxx + Marge) &&
            y >= (m_miny - Marge) && y <= (m_maxy + Marge) )
            return TRUE;
    return FALSE;
}


//
// Is the given point in the boundingbox ??
//
bool wxBoundingBox::PointInBox(const wxPoint2DDouble& a, double Marge) const
{
    assert (m_validbbox == TRUE);

    return PointInBox(a.m_x, a.m_y, Marge);
}


wxPoint2DDouble wxBoundingBox::GetMin() const
{
    assert (m_validbbox == TRUE);

    return wxPoint2DDouble(m_minx, m_miny);
}


wxPoint2DDouble wxBoundingBox::GetMax() const
{
    assert (m_validbbox == TRUE);

    return wxPoint2DDouble(m_maxx, m_maxy);
}

bool wxBoundingBox::GetValid() const
{
    return m_validbbox;
}

void wxBoundingBox::SetMin(double px, double py)
{
    m_minx = px;
    m_miny = py;
    if (!m_validbbox)
    {
        m_maxx = px;
        m_maxy = py;
        m_validbbox = TRUE;
    }
}

void wxBoundingBox::SetMax(double px, double py)
{
    m_maxx = px;
    m_maxy = py;
    if (!m_validbbox)
    {
        m_minx = px;
        m_miny = py;
        m_validbbox = TRUE;
    }
}

void wxBoundingBox::SetValid(bool value)
{
    m_validbbox = value;
}

// adds an offset to the boundingbox
// usage : a_boundingbox.Translate(a_point);
void wxBoundingBox::Translate(wxPoint2DDouble& offset)
{
    assert (m_validbbox == TRUE);

    m_minx += offset.m_x;
    m_maxx += offset.m_x;
    m_miny += offset.m_y;
    m_maxy += offset.m_y;
}


// clears the bounding box settings
void wxBoundingBox::Reset()
{
    m_minx = 0.0;
    m_maxx = 0.0;
    m_miny = 0.0;
    m_maxy = 0.0;
    m_validbbox = FALSE;
}


void wxBoundingBox::SetBoundingBox(const wxPoint2DDouble& a_point)
{
    m_minx = a_point.m_x;
    m_maxx = a_point.m_x;
    m_miny = a_point.m_y;
    m_maxy = a_point.m_y;

    m_validbbox = TRUE;
}


// Expands the boundingbox with the given point
// usage : a_boundingbox = a_boundingbox + pointer_to_an_offset;
wxBoundingBox& wxBoundingBox::operator+(wxBoundingBox &other)
{
    assert (m_validbbox == TRUE);
    assert (other.GetValid());

    Expand(other);
    return *this;
}


// makes a boundingbox same as the other
wxBoundingBox& wxBoundingBox::operator=( const wxBoundingBox &other)
{
    m_minx = other.m_minx;
    m_maxx = other.m_maxx;
    m_miny = other.m_miny;
    m_maxy = other.m_maxy;
    m_validbbox = other.m_validbbox;
    return *this;
}

void wxBoundingBox::MapBbox( const wxTransformMatrix& matrix)
{
    assert (m_validbbox == TRUE);

    double x1,y1,x2,y2,x3,y3,x4,y4;

    matrix.TransformPoint( m_minx, m_miny, x1, y1 );
    matrix.TransformPoint( m_minx, m_maxy, x2, y2 );
    matrix.TransformPoint( m_maxx, m_maxy, x3, y3 );
    matrix.TransformPoint( m_maxx, m_miny, x4, y4 );

    double xmin = wxMin(x1,x2);
    xmin = wxMin(xmin,x3);
    xmin = wxMin(xmin,x4);

    double xmax = wxMax( x1, x2);
    xmax = wxMax(xmax,x3);
    xmax = wxMax(xmax,x4);

    double ymin = wxMin(y1, y2);
    ymin = wxMin(ymin,y3);
    ymin = wxMin(ymin,y4);

    double ymax = wxMax(y1,y2);
    ymax = wxMax(ymax,y3);
    ymax = wxMax(ymax,y4);

    // Use these min and max values to set the new boundingbox
    m_minx = xmin;
    m_miny = ymin;
    m_maxx = xmax;
    m_maxy = ymax;
}

//----------------------------------------------------------------
//    LLBBox Implementation
//----------------------------------------------------------------

void LLBBox::Set(double minlat, double minlon, double maxlat, double maxlon)
{
#if 0
    // ensure average is from -180 to 180
    if(minlon + maxlon >= 360)
        minlon -= 360, maxlon -= 360;
    else
    if(minlon + maxlon <= -360)
        minlon += 360, maxlon += 360;
#endif
    
    m_minlat = minlat;
    m_minlon = minlon;
    m_maxlat = maxlat;
    m_maxlon = maxlon;
    m_valid = minlat <= maxlat && minlon <= maxlon;
}

void LLBBox::SetFromSegment(double lat1, double lon1, double lat2, double lon2)
{
    m_minlat = wxMin(lat1, lat2);
    m_maxlat = wxMax(lat1, lat2);

    double minlon[3], maxlon[3];
    double lon[2][3] = {{lon1}, {lon2}};
    for(int i=0; i<2; i++) {
        if(lon[i][0] < 0) {
            lon[i][1] = lon[i][0] + 360;
            lon[i][2] = lon[i][0];
        } else {
            lon[i][1] = lon[i][0];
            lon[i][2] = lon[i][0] - 360;
        }
    }
    
    double d[3];
    for(int k=0; k<3; k++) {
        minlon[k] = wxMin(lon[0][k], lon[1][k]);
        maxlon[k] = wxMax(lon[0][k], lon[1][k]);

        double a = maxlon[k] + minlon[k];
        // eliminate cases where the average longitude falls outside of -180 to 180
        if(a <= -360 || a >= 360)
            d[k] = 360;
        else
            d[k] = maxlon[k] - minlon[k];
    }

    double epsilon = 1e-2;  // because floating point rounding favor... d1, then d2 then d3
    d[1] += epsilon, d[2] += 2*epsilon;
    int mink = 0;
    for(int k=1; k<3; k++)
        if(d[k] < d[mink])
            mink = k;

    m_minlon = minlon[mink];
    m_maxlon = maxlon[mink];

    m_valid = TRUE;
}

void LLBBox::Expand(const LLBBox& other)
{
    if(!m_valid) {
        *this = other;
        return;
    }

    m_minlat = wxMin(m_minlat, other.m_minlat);
    m_maxlat = wxMax(m_maxlat, other.m_maxlat);

    double minlons[2][3] = {{m_minlon}, {other.m_minlon}};
    double maxlons[2][3] = {{m_maxlon}, {other.m_maxlon}};

    for(int i=0; i<2; i++) {
        if(minlons[i][0] < 0) {
            minlons[i][1] = minlons[i][0] + 360;
            maxlons[i][1] = maxlons[i][0] + 360;
        } else {
            minlons[i][1] = minlons[i][0];
            maxlons[i][1] = maxlons[i][0];
        }

        if(maxlons[i][0] > 0) {
            minlons[i][2] = minlons[i][0] - 360;
            maxlons[i][2] = maxlons[i][0] - 360;
        } else {
            minlons[i][2] = minlons[i][0];
            maxlons[i][2] = maxlons[i][0];
        }
    }

    double d[3];
    double minlon[3], maxlon[3];

    for(int k=0; k<3; k++) {
        minlon[k] = wxMin(minlons[0][k], minlons[1][k]);
        maxlon[k] = wxMax(maxlons[0][k], maxlons[1][k]);

        double a = maxlon[k] + minlon[k];
        // eliminate cases where the average longitude falls outside of -180 to 180
        if(a <= -360 || a >= 360)
            d[k] = 360;
        else
            d[k] = maxlon[k] - minlon[k];
    }

    double epsilon = 1e-2;  // because floating point rounding favor... d1, then d2 then d3
    d[1] += epsilon, d[2] += 2*epsilon;
    int mink = 0;
    for(int k=1; k<3; k++)
        if(d[k] < d[mink])
            mink = k;

    m_minlon = minlon[mink];
    m_maxlon = maxlon[mink];

}

bool LLBBox::Contains(double lat, double lon) const
{
    if(lat < m_minlat || lat > m_maxlat )
        return FALSE;

//    Box is centered in East lon, crossing IDL
    if(m_maxlon > 180.) {
        if( lon < m_maxlon - 360.)
            lon +=  360.;
    }
      //    Box is centered in Wlon, crossing IDL
    else if(m_minlon < -180.)
    {
        if(lon > m_minlon + 360.)
            lon -= 360.;
    }

    return lon >= m_minlon && lon <= m_maxlon;
}

bool LLBBox::ContainsMarge(double lat, double lon, double Marge) const
{
    if(lat < (m_minlat - Marge) || lat > (m_maxlat + Marge) )
        return FALSE;

//    Box is centered in East lon, crossing IDL
    if(m_maxlon > 180.) {
        if( lon < m_maxlon - 360.)
            lon +=  360.;
    }
      //    Box is centered in Wlon, crossing IDL
    else if(m_minlon < -180.)
    {
        if(lon > m_minlon + 360.)
            lon -= 360.;
    }

    return  lon >= (m_minlon - Marge) && lon <= (m_maxlon + Marge);
}

bool LLBBox::IntersectIn( const LLBBox &other ) const
{
    if( !GetValid() || !other.GetValid() )
        return false;

    if((m_maxlat <= other.m_maxlat) || (m_minlat >= other.m_minlat))
        return false;
    
    double minlon = m_minlon, maxlon = m_maxlon;
    if(m_maxlon < other.m_minlon)
        minlon += 360, maxlon += 360;
    else if(m_minlon > other.m_maxlon)
        minlon -= 360, maxlon -= 360;

    return (minlon > other.m_minlon) && (maxlon < other.m_maxlon);
}

bool LLBBox::IntersectOutGetBias( const LLBBox &other, double bias ) const
{
    // allow -180 to 180 or 0 to 360
    if( !GetValid() || !other.GetValid() )
        return true;
    
    if((m_maxlat < other.m_minlat) || (m_minlat > other.m_maxlat))
        return true;

    if(m_maxlon < other.m_minlon)
        bias = 360;
    else if(m_minlon > other.m_maxlon)
        bias = -360;
    else
        bias = 0;

    return (m_minlon + bias > other.m_maxlon) || (m_maxlon + bias < other.m_minlon);
}

#if 0 // use if needed...
OVERLAP LLBox::Intersect( const LLBBox &other) const
{
    if(IntersectOut(other))
        return _OUT;

    if(IntersectIn(other))
        return _IN;

    // Boundingboxes intersect
    return _ON;
}
#endif

void LLBBox::EnLarge(const double marge)
{
    if (!m_valid)
    {
        m_minlon = m_maxlon = marge;
        m_minlat = m_maxlat = marge;
        m_valid=TRUE;
    }
    else
    {
        m_minlon -= marge;
        m_maxlon += marge;
        m_minlat -= marge;
        m_maxlat += marge;
    }
}
