#include "wx/wxprec.h"
#include "brgdist.h"

#ifndef WX_PRECOMP
  #include "wx/wx.h"

#endif //precompiled headers

#define DTOL 1e-12

#define HALFPI 1.5707963267948966
#define SPI 3.14159265359
#define TWOPI 6.2831853071795864769
#define ONEPI 3.14159265358979323846
#define MERI_TOL 1e-9

static double th1,costh1,sinth1,sina12,cosa12,M,N,c1,c2,D,P,s1;
static int merid, signS;

/* Input/Output from geodesic functions */
static double al12; /* Forward azimuth */
static double al21; /* Back azimuth */
static double geod_S; /* Distance */
static double phi1, lam1, phi2, lam2;

static int ellipse;
static double geod_f;
static double geod_a;
static double es, onef, f, f64, f2, f4;

//----------------------------------------------------------------
//
// DashboardInstrument_BrgDistImplementation
//
//----------------------------------------------------------------

DashboardInstrument_BrgDist::DashboardInstrument_BrgDist(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag1, int cap_flag2)
      :DashboardInstrument(pparent, id, title, cap_flag1 | cap_flag2)
{
      m_data1 = _T("---");
      m_data2 = _T("---");
      m_cap_flag1 = cap_flag1;
      m_cap_flag2 = cap_flag2;
}

wxSize DashboardInstrument_BrgDist::GetSize( int orient, wxSize hint )
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      dc.GetTextExtent(_T("000 00.0000 W"), &w, &m_DataHeight, 0, 0, g_pFontData);

      if( orient == wxHORIZONTAL ) {
          return wxSize( w+10, wxMax(hint.y, m_TitleHeight+m_DataHeight*2) );
      } else {
          return wxSize( wxMax(hint.x, w+10), m_TitleHeight+m_DataHeight*2 );
      }
}
void DashboardInstrument_BrgDist::Draw(wxGCDC* dc)
{
      wxColour cl;

      dc->SetFont(*g_pFontData);
      GetGlobalColor(_T("DASHF"), &cl);
      dc->SetTextForeground(cl);

      dc->DrawText(m_data1, 10, m_TitleHeight);
      dc->DrawText(m_data2, 10, m_TitleHeight+m_DataHeight);
}

void DashboardInstrument_BrgDist::SetData(int st, double data, wxString unit)
{
      if (st == m_cap_flag1)
      {
            m_data1 = toDEGNM(1, data);
      }
      else if (st == m_cap_flag2)
      {
            m_data2 = toDEGNM(2, data);
      }
      else return;
      Refresh(false);
}

/**************************************************************************/
/* Some assorted utilities */
/**************************************************************************/
wxString toDEGNM(int DEGNMflag, double dval)
{
    wxString s;
    if (DEGNMflag == 1)
    {
        s.Printf( _("%03d Deg"), (int) dval );
    }
    else s << FormatDistanceAdaptive( dval );
    return s;
}

wxString FormatDistanceAdaptive( double distance ) {
    wxString result;
    
    if( distance < 0.1 )
    {
        result << wxString::Format(_T("%1.0f "), distance*1852.0 ) << _T("meters");
        return result;
    }
    if( distance < 5.0 )
    {
        result << wxString::Format(_T("%2.2f "), distance ) << _T("NMi");
        return result;
    }
    if( distance < 10.0 )
    {
        result << wxString::Format(_T("%2.2f "), distance ) << _T("NMi");
        return result;
    }

    if( distance < 100.0 ) {
        result << wxString::Format(_T("%2.1f "), distance ) << _T("NMi");
        return result;
    }
    if( distance < 1000.0 )
    {
        result << wxString::Format(_T("%2.0f "), distance ) << _T("NMi");
        return result;
    }
    result << wxString::Format(_T("%2.0f "), distance ) << _T("NMi");
       return result;
}

double BrgDistCalc::adjlon (double lon) {
      if (fabs(lon) <= SPI) return( lon );
      lon += ONEPI; /* adjust to 0..2pi rad */
      lon -= TWOPI * floor(lon / TWOPI); /* remove integral # of 'revolutions'*/
      lon -= ONEPI; /* adjust back to -pi..pi rad */
      return( lon );
}

void BrgDistCalc::geod_inv() {
      double th1,th2,thm,dthm,dlamm,dlam,sindlamm,costhm,sinthm,cosdthm,
      sindthm,L,E,cosd,d,X,Y,T,sind,tandlammp,u,v,D,A,B;


            /* Stuff the WGS84 projection parameters as necessary
To avoid having to include <geodesic,h>
*/

      ellipse = 1;
      f = 1.0 / WGSinvf; /* WGS84 ellipsoid flattening parameter */
      geod_a = WGS84_semimajor_axis_meters;

      es = 2 * f - f * f;
      onef = sqrt(1. - es);
      geod_f = 1 - onef;
      f2 = geod_f/2;
      f4 = geod_f/4;
      f64 = geod_f*geod_f/64;


      if (ellipse) {
            th1 = atan(onef * tan(phi1));
            th2 = atan(onef * tan(phi2));
      } else {
            th1 = phi1;
            th2 = phi2;
      }
      thm = .5 * (th1 + th2);
      dthm = .5 * (th2 - th1);
      dlamm = .5 * ( dlam = adjlon(lam2 - lam1) );
      if (fabs(dlam) < DTOL && fabs(dthm) < DTOL) {
            al12 = al21 = geod_S = 0.;
            return;
      }
      sindlamm = sin(dlamm);
      costhm = cos(thm); sinthm = sin(thm);
      cosdthm = cos(dthm); sindthm = sin(dthm);
      L = sindthm * sindthm + (cosdthm * cosdthm - sinthm * sinthm)
                  * sindlamm * sindlamm;
      d = acos(cosd = 1 - L - L);
      if (ellipse) {
            E = cosd + cosd;
            sind = sin( d );
            Y = sinthm * cosdthm;
            Y *= (Y + Y) / (1. - L);
            T = sindthm * costhm;
            T *= (T + T) / L;
            X = Y + T;
            Y -= T;
            T = d / sind;
            D = 4. * T * T;
            A = D * E;
            B = D + D;
            geod_S = geod_a * sind * (T - f4 * (T * X - Y) +
                        f64 * (X * (A + (T - .5 * (A - E)) * X) -
                        Y * (B + E * Y) + D * X * Y));
            tandlammp = tan(.5 * (dlam - .25 * (Y + Y - E * (4. - X)) *
                        (f2 * T + f64 * (32. * T - (20. * T - A)
                        * X - (B + 4.) * Y)) * tan(dlam)));
      } else {
            geod_S = geod_a * d;
            tandlammp = tan(dlamm);
      }
      u = atan2(sindthm , (tandlammp * costhm));
      v = atan2(cosdthm , (tandlammp * sinthm));
      al12 = adjlon(TWOPI + v - u);
      al21 = adjlon(TWOPI - v - u);
}


double BrgDistCalc::DistGreatCircle(double slat, double slon, double dlat, double dlon)
{

      double d5;
      phi1 = slat * DEGREES;
      lam1 = slon * DEGREES;
      phi2 = dlat * DEGREES;
      lam2 = dlon * DEGREES;

      geod_inv();
      d5 = geod_S / 1852.0;

      return d5;
}


void BrgDistCalc::toSM_ECC(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
      double xlon, z, x1, s, y3, s0, y30, y4;

      double falsen;
      double test;
      double ypy;

      double f = 1.0 / WGSinvf; // WGS84 ellipsoid flattening parameter
      double e2 = 2 * f - f * f; // eccentricity^2 .006700
      double e = sqrt(e2);

      xlon = lon;


      /* Make sure lon and lon0 are same phase */

/* Took this out for 530 pacific....what else is broken??
if(lon * lon0 < 0.)
{
if(lon < 0.)
xlon += 360.;
else
xlon -= 360.;
}

if(fabs(xlon - lon0) > 180.)
{
if(xlon > lon0)
xlon -= 360.;
else
xlon += 360.;
}
*/
      z = WGS84_semimajor_axis_meters * mercator_k0;

      x1 = (xlon - lon0) * DEGREES * z;
      *x = x1;

// y =.5 ln( (1 + sin t) / (1 - sin t) )
      s = sin(lat * DEGREES);
      y3 = (.5 * log((1 + s) / (1 - s))) * z;

      s0 = sin(lat0 * DEGREES);
      y30 = (.5 * log((1 + s0) / (1 - s0))) * z;
      y4 = y3 - y30;
// *y = y4;

    //Add eccentricity terms

      falsen = z *log(tan(PI/4 + lat0 * DEGREES / 2)*pow((1. - e * s0)/(1. + e * s0), e/2.));
      test = z *log(tan(PI/4 + lat * DEGREES / 2)*pow((1. - e * s )/(1. + e * s ), e/2.));
      ypy = test - falsen;

      *y = ypy;
}


void BrgDistCalc::BearingDistanceMercator(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist)
{
      double east, north, brgt, C;
      double lon0x, lon1x, dlat;
      double mlat0;

      // Calculate bearing by conversion to SM (Mercator) coordinates, then simple trigonometry

      lon0x = lon0;
      lon1x = lon1;

      // Make lon points the same phase
      if((lon0x * lon1x) < 0.)
      {
            if(lon0x < 0.)
                  lon0x += 360.;
            else
                  lon1x += 360.;

            // Choose the shortest distance
            if(fabs(lon0x - lon1x) > 180.)
            {
                  if(lon0x > lon1x)
                        lon0x -= 360.;
                  else
                        lon1x -= 360.;
            }

            // Make always positive
            lon1x += 360.;
            lon0x += 360.;
      }

      // In the case of exactly east or west courses
      // we must make an adjustment if we want true Mercator distances

      // This idea comes from Thomas(Cagney)
      // We simply require the dlat to be (slightly) non-zero, and carry on.
      // MAS022210 for HamishB from 1e-4 && .001 to 1e-9 for better precision
      // on small latitude diffs
      mlat0 = lat0;
      if(fabs(lat1 - lat0) < 1e-9)
            mlat0 += 1e-9;

      toSM_ECC(lat1, lon1x, mlat0, lon0x, &east, &north);

      C = atan2(east, north);
      dlat = (lat1 - mlat0) * 60.; // in minutes

      // Classic formula, which fails for due east/west courses....

      if(dist)
      {
            if(cos(C))
                  *dist = (dlat /cos(C));
            else
                  *dist = DistGreatCircle(lat0, lon0, lat1, lon1);

      }

      // Calculate the bearing using the un-adjusted original latitudes and Mercator Sailing
      if(brg)
      {
            toSM_ECC(lat1, lon1x, lat0, lon0x, &east, &north);

            C = atan2(east, north);
            brgt = 180. + (C * 180. / PI);
            if (brgt < 0)
                  brgt += 360.;
            if (brgt > 360.)
                  brgt -= 360;

            *brg = brgt;
      }


      // Alternative formulary
      // From Roy Williams, "Geometry of Navigation", we have p = Dlo (Dlat/DMP) where p is the departure.
      // Then distance is then:D = sqrt(Dlat^2 + p^2)

/*
double dlo = (lon1x - lon0x) * 60.;
double departure = dlo * dlat / ((north/1852.));

if(dist)
*dist = sqrt((dlat*dlat) + (departure * departure));
*/
}