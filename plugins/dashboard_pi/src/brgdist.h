#ifndef _BRGDIST_H_
#define _BRGDIST_H_

#include "instrument.h"
#include <math.h>
#include <typeinfo>
#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif
#define DEGREES (PI/180.0)
#define RADIAN (180.0/PI)
static const double WGS84_semimajor_axis_meters = 6378137.0; // WGS84 semimajor axis
static const double mercator_k0 = 0.9996;
static const double WGSinvf = 298.257223563; /* WGS84 1/f */

class DashboardInstrument_BrgDist;
wxString toDEGNM(int DEGNMflag, double dval);
wxString FormatDistanceAdaptive( double distance );

class BrgDistCalc
{
public:
    BrgDistCalc( ){}
    ~BrgDistCalc(){}
    
    void BearingDistanceMercator(double dcurlat, double dcurlon, double dglat, double dglon, double *brg, double *dist);
private:
    double DistGreatCircle(double slat, double slon, double dlat, double dlon);
    double adjlon (double lon);
    
    void toSM_ECC(double lat, double lon, double lat0, double lon0, double *x, double *y);
    void geod_inv();
};

class DashboardInstrument_BrgDist : public DashboardInstrument
{
public:
      DashboardInstrument_BrgDist(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag1=OCPN_DBP_STC_LAT, int cap_flag2=OCPN_DBP_STC_LON);
      ~DashboardInstrument_BrgDist(){}

      wxSize GetSize( int orient, wxSize hint );
      void SetData(int st, double data, wxString unit);

protected:
      wxString m_data1;
      wxString m_data2;
      int m_cap_flag1;
      int m_cap_flag2;
      int m_DataHeight;

      void Draw(wxGCDC* dc);
};
#endif