//testing
#ifndef __WXBOUNDINGBOX_H__
#define __WXBOUNDINGBOX_H__


#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/matrix.h"
#include "wx/geometry.h"

enum OVERLAP {_IN,_ON,_OUT};

//Purpose   The wxBoundingBox class stores one wxBoundingBox.
//The wxBoundingBox is defined by two coordiates,
//a upperleft coordinate and a lowerright coordinate.
class wxBoundingBox
{
public:
    wxBoundingBox();
    wxBoundingBox(const wxBoundingBox&);
    wxBoundingBox(const wxPoint2DDouble&);
    wxBoundingBox(double xmin, double ymin, double xmax, double ymax);
    virtual ~wxBoundingBox();

    bool And(wxBoundingBox*, double Marge = 0);

    void EnLarge(const double Marge);
    void Shrink(const double Marge);

    void Expand(const wxPoint2DDouble& , const wxPoint2DDouble&);
    void Expand(const wxPoint2DDouble&);
    void Expand(double x,double y);
    void Expand(const wxBoundingBox& bbox);

    OVERLAP Intersect( const wxBoundingBox &, double Marge = 0) const;

    /* this routine is used very heavily, so this is a lightweight
       version for when we only care if the other box is out */
    virtual inline bool IntersectOut( const wxBoundingBox &other ) const {
        return (m_minx > other.m_maxx) || (m_maxx < other.m_minx) ||
               (m_maxy < other.m_miny) || (m_miny > other.m_maxy);
    }

    bool LineIntersect(const wxPoint2DDouble& begin, const wxPoint2DDouble& end ) const;
    bool PointInBox( const wxPoint2DDouble&, double Marge = 0) const;
    virtual bool PointInBox( double, double, double Marge = 0) const;

    void Reset();

    void Translate( wxPoint2DDouble& );
    void MapBbox( const wxTransformMatrix& matrix);

    double  GetWidth() const {return m_maxx-m_minx;};
    double  GetHeight() const {return m_maxy-m_miny;};
    bool    GetValid()  const;
    void    SetValid(bool);

    void    SetBoundingBox(const wxPoint2DDouble& a_point);

    void    SetMin(double, double);
    void    SetMax(double, double);
    inline  wxPoint2DDouble GetMin() const;
    inline  wxPoint2DDouble GetMax() const;
    inline  double GetMinX() const {return m_minx;};
    inline  double GetMinY() const {return m_miny;};
    inline  double GetMaxX() const {return m_maxx;};
    inline  double GetMaxY() const {return m_maxy;};

    wxBoundingBox&  operator+( wxBoundingBox& );
    wxBoundingBox&  operator=(  const wxBoundingBox& );

protected:
    //bounding box in world
    double        m_minx;
    double        m_miny;
    double        m_maxx;
    double        m_maxy;
    bool          m_validbbox;
};

//    A class derived from wxBoundingBox
//    that is assummed to be a geographic area, with coordinates
//    expressed in Lat/Lon.
//    This class understands the International Date Line (E/W Longitude)

class LLBBox
{
public:
    LLBBox() : m_valid(FALSE) {}
    void Set(double minlat, double minlon, double maxlat, double maxlon);
    void SetFromSegment(double lat1, double lon1, double lat2, double lon2);
    void Expand(const LLBBox& bbox);
    bool Contains(double Lat, double Lon) const;
    bool ContainsMarge(double Lat, double Lon, double Marge) const;
    bool GetValid() const { return m_valid; }
    void Invalidate() { m_valid = false; }

    bool IntersectIn( const LLBBox &other ) const;
    inline bool IntersectOut( const LLBBox &other ) const
    {
        // allow -180 to 180 or 0 to 360
        if( !GetValid() || !other.GetValid() )
            return true;

        if((m_maxlat < other.m_minlat) || (m_minlat > other.m_maxlat))
            return true;

        double minlon = m_minlon, maxlon = m_maxlon;
        if(m_maxlon < other.m_minlon)
            minlon += 360, maxlon += 360;
        else if(m_minlon > other.m_maxlon)
            minlon -= 360, maxlon -= 360;

        return (minlon > other.m_maxlon) || (maxlon < other.m_minlon);
    }
    bool IntersectOutGetBias( const LLBBox &other, double bias ) const;
    
    double GetMinLat() const {return m_minlat;};
    double GetMinLon() const {return m_minlon;};
    double GetMaxLat() const {return m_maxlat;};
    double GetMaxLon() const {return m_maxlon;};

    void EnLarge(const double Marge);
    double GetLonRange() const {return m_maxlon - m_minlon; }
    double GetLatRange() const {return m_maxlat - m_minlat;};

private:

    double        m_minlat;
    double        m_minlon;
    double        m_maxlat;
    double        m_maxlon;
    bool          m_valid;
};

#endif
