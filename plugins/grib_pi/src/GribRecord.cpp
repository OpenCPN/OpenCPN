/**********************************************************************
zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

//#include "dychart.h"        // for some compile time fixups
//#include "cutil.h"
#include <stdlib.h>

//#include <QDateTime>

#include "GribRecord.h"

// interpolate two angles in range +- 180 or +-PI, with resulting angle in the same range
static double interp_angle(double a0, double a1, double d, double p)
{
    if(a0 - a1 > p) a0 -= 2*p;
    else if(a1 - a0 > p) a1 -= 2*p;
    double a = (1-d)*a0 + d*a1;
    if(a < ( p == 180. ? 0. : -p ) ) a += 2*p;
    return a;
}

//-------------------------------------------------------------------------------
void GribRecord::print()
{
	printf("%d: idCenter=%d idModel=%d idGrid=%d dataType=%d levelType=%d levelValue=%d hr=%f\n",
			id, idCenter, idModel, idGrid, dataType, levelType,levelValue,
			(curDate-refDate)/3600.0
			);
}

//-------------------------------------------------------------------------------
// Constructeur de recopie
//-------------------------------------------------------------------------------
GribRecord::GribRecord(const GribRecord &rec)
{
    *this = rec;
    IsDuplicated = true;
    // recopie les champs de bits
    if (rec.data != NULL) {
        int size = rec.Ni*rec.Nj;
        this->data = new double[size];
        for (int i=0; i<size; i++)
            this->data[i] = rec.data[i];
    }
    if (rec.BMSbits != NULL) {
        int size = rec.BMSsize;
        this->BMSbits = new zuchar[size];
        for (int i=0; i<size; i++)
            this->BMSbits[i] = rec.BMSbits[i];
    }
}

bool GribRecord::GetInterpolatedParameters
(const GribRecord &rec1, const GribRecord &rec2,
 double &La1, double &Lo1, double &La2, double &Lo2, double &Di, double &Dj,
 int &im1, int &jm1, int &im2, int &jm2,
 int &Ni, int &Nj, int &rec1offi, int &rec1offj, int &rec2offi, int &rec2offj )
{
    if(!rec1.isOk() || !rec2.isOk())
        return false;

    /* make sure Dj both have same sign */
    if(rec1.getDj() * rec2.getDj() <= 0)
        return false;

    Di = wxMax(rec1.getDi(), rec2.getDi());
    Dj = rec1.getDj() > 0 ?
        wxMax(rec1.getDj(), rec2.getDj()):
        wxMin(rec1.getDj(), rec2.getDj());

    /* get overlapping region */
    if(Dj > 0)
        La1 = wxMax(rec1.La1, rec2.La1), La2 = wxMin(rec1.La2, rec2.La2);
    else
        La1 = wxMin(rec1.La1, rec2.La1), La2 = wxMax(rec1.La2, rec2.La2);

    Lo1 = wxMax(rec1.Lo1, rec2.Lo1), Lo2 = wxMin(rec1.Lo2, rec2.Lo2);

    // align gribs on integer boundaries
    int i, j;
    // shut up compiler warning 'may be used uninitialized'
    // rec2.Dj / rec1.Dj > 0
    // XXX Is it true  for rec2.Di / rec1.Di ?
    double rec1offdi = 0, rec2offdi = 0;
    double rec1offdj = 0., rec2offdj = 0.;

    double iiters = rec2.Di / rec1.Di;
    if(iiters < 1) {
        iiters = 1/iiters;
        im1 = 1, im2 = iiters;
    } else
        im1 = iiters, im2 = 1;

    for(i=0; i<iiters; i++) {
        rec1offdi = (Lo1 - rec1.Lo1)/rec1.Di;
        rec2offdi = (Lo1 - rec2.Lo1)/rec2.Di;
        if(rec1offdi == floor(rec1offdi) && rec2offdi == floor(rec2offdi))
            break;

        Lo1 += wxMin(rec1.Di, rec2.Di);
    }
    if(i == iiters) // failed to align, would need spacial interpolation to work
        return false;

    double jiters = rec2.Dj / rec1.Dj;
    if(jiters < 1) {
        jiters = 1/jiters;
        jm1 = 1, jm2 = iiters;
    } else
        jm1 = jiters, jm2 = 1;

    for(j=0; j<jiters; j++) {
        rec1offdj = (La1 - rec1.La1)/rec1.Dj;
        rec2offdj = (La1 - rec2.La1)/rec2.Dj;
        if(rec1offdj == floor(rec1offdj) && rec2offdj == floor(rec2offdj))
            break;

        La1 += Dj < 0 ?
            wxMax(rec1.getDj(), rec2.getDj()):
            wxMin(rec1.getDj(), rec2.getDj());
    }
    if(j == jiters) // failed to align
        return false;

    /* no overlap */
    if(La1*Dj > La2*Dj || Lo1 > Lo2)
        return false;

    /* compute integer sizes for data array */
    Ni = (Lo2-Lo1)/Di + 1, Nj = (La2-La1)/Dj + 1;

    /* back-compute final La2 and Lo2 to fit this integer boundary */
    Lo2 = Lo1 + (Ni-1)*Di, La2 = La1 + (Nj-1)*Dj;

    rec1offi = rec1offdi, rec2offi = rec2offdi;
    rec1offj = rec1offdj, rec2offj = rec2offdj;

    if (!rec1.data || !rec2.data)
        return false;

    return true;
}

//-------------------------------------------------------------------------------
// Constructeur de interpolate
//-------------------------------------------------------------------------------
GribRecord * GribRecord::InterpolatedRecord(const GribRecord &rec1, const GribRecord &rec2, double d, bool dir)
{
    double La1, Lo1, La2, Lo2, Di, Dj;
    int im1, jm1, im2, jm2;
    int Ni, Nj, rec1offi, rec1offj, rec2offi, rec2offj;
    if(!GetInterpolatedParameters(rec1, rec2, La1, Lo1, La2, Lo2, Di, Dj,
                                  im1, jm1, im2, jm2,
                                  Ni, Nj, rec1offi, rec1offj, rec2offi, rec2offj))
        return NULL;

    // recopie les champs de bits
    int size = Ni*Nj;
    double *data = new double[size];

    zuchar *BMSbits = NULL;
    if (rec1.BMSbits != NULL && rec2.BMSbits != NULL)
        BMSbits = new zuchar[(Ni*Nj-1)/8+1]();

    for (int i=0; i<Ni; i++)
        for (int j=0; j<Nj; j++) {
            int in=j*Ni+i;
            int i1 = (j*jm1+rec1offj)*rec1.Ni + i*im1+rec1offi;
            int i2 = (j*jm2+rec2offj)*rec2.Ni + i*im2+rec2offi;
            double data1 = rec1.data[i1], data2 = rec2.data[i2];
            if(data1 == GRIB_NOTDEF || data2 == GRIB_NOTDEF)
                data[in] = GRIB_NOTDEF;
            else {
				if( !dir )
					data[in] = (1-d)*data1 + d*data2;
				else
					data[in] = interp_angle(data1, data2, d, 180.);
			}

            if(BMSbits) {
                int b1 = rec1.BMSbits[i1>>3] & 1<<(i1&7);
                int b2 = rec2.BMSbits[i2>>3] & 1<<(i2&7);
                if(b1 && b2)
                    BMSbits[in>>3] |= 1<<(in&7);
                else
                    BMSbits[in>>3] &= ~(1<<(in&7));
            }
        }

    /* should maybe update strCurDate ? */

    GribRecord *ret = new GribRecord;
    *ret = rec1;

    ret->Di = Di, ret->Dj = Dj;
    ret->Ni = Ni, ret->Nj = Nj;

    ret->La1 = La1, ret->La2 = La2;
    ret->Lo1 = Lo1, ret->Lo2 = Lo2;

    ret->data = data;
    ret->BMSbits = BMSbits;

    ret->latMin = wxMin(La1, La2), ret->latMax = wxMax(La1, La2);
    ret->lonMin = Lo1, ret->lonMax = Lo2;

    ret->m_bfilled = false;
    
    return ret;
}

/* for interpolation for x and y records, we must do them together because otherwise
   we end up with a vector interpolation which is not what we want.. instead we want
   to interpolate from the polar magnitude, and angles */
GribRecord *GribRecord::Interpolated2DRecord(GribRecord *&rety,
                                             const GribRecord &rec1x, const GribRecord &rec1y,
                                             const GribRecord &rec2x, const GribRecord &rec2y, double d)
{
    double La1, Lo1, La2, Lo2, Di, Dj;
    int im1, jm1, im2, jm2;
    int Ni, Nj, rec1offi, rec1offj, rec2offi, rec2offj;
    if(!GetInterpolatedParameters(rec1x, rec2x, La1, Lo1, La2, Lo2, Di, Dj,
                                  im1, jm1, im2, jm2,
                                  Ni, Nj, rec1offi, rec1offj, rec2offi, rec2offj))
        return NULL;


    if(!rec1y.data || !rec2y.data || !rec1y.isOk() || !rec2y.isOk() ||
       rec1x.Di != rec1y.Di ||rec1x.Dj != rec1y.Dj ||
       rec2x.Di != rec2y.Di ||rec2x.Dj != rec2y.Dj ||
       rec1x.Ni != rec1y.Ni ||rec1x.Nj != rec1y.Nj ||
       rec2x.Ni != rec2y.Ni ||rec2x.Nj != rec2y.Nj)
        // could also make sure lat and lon min/max are the same...
        return NULL;

    // recopie les champs de bits
    int size = Ni*Nj;
    double *datax = new double[size], *datay = new double[size];
    for (int i=0; i<Ni; i++) {
        for (int j=0; j<Nj; j++) {
            int in=j*Ni+i;
            int i1 = (j*jm1+rec1offj)*rec1x.Ni + i*im1+rec1offi;
            int i2 = (j*jm2+rec2offj)*rec2x.Ni + i*im2+rec2offi;
            double data1x = rec1x.data[i1], data1y = rec1y.data[i1];
            double data2x = rec2x.data[i2], data2y = rec2y.data[i2];
            if(data1x == GRIB_NOTDEF || data1y == GRIB_NOTDEF ||
               data2x == GRIB_NOTDEF || data2y == GRIB_NOTDEF) {
                datax[in] = GRIB_NOTDEF;
                datay[in] = GRIB_NOTDEF;
            } else {
                double data1m = sqrt(pow(data1x, 2) + pow(data1y, 2));
                double data2m = sqrt(pow(data2x, 2) + pow(data2y, 2));
                double datam = (1-d)*data1m + d*data2m;

                double data1a = atan2(data1y, data1x);
                double data2a = atan2(data2y, data2x);
                     if(data1a - data2a > M_PI) data1a -= 2*M_PI;
                else if(data2a - data1a > M_PI) data2a -= 2*M_PI;
                double dataa = (1-d)*data1a + d*data2a;

                datax[in] = datam*cos(dataa);
                datay[in] = datam*sin(dataa);
            }
        }
    }

    /* should maybe update strCurDate ? */

    GribRecord *ret = new GribRecord;

    *ret = rec1x;

    ret->Di = Di, ret->Dj = Dj;
    ret->Ni = Ni, ret->Nj = Nj;

    ret->La1 = La1, ret->La2 = La2;
    ret->Lo1 = Lo1, ret->Lo2 = Lo2;

    ret->data = datax;
    ret->BMSbits = NULL;
    ret->hasBMS = false; // I don't think wind or current ever use BMS correct?

    ret->latMin = wxMin(La1, La2), ret->latMax = wxMax(La1, La2);
    ret->lonMin = Lo1, ret->lonMax = Lo2;

    rety = new GribRecord;
    *rety = *ret;
    rety->dataType = rec1y.dataType;
    rety->data = datay;
    rety->BMSbits = NULL;
    rety->hasBMS = false;

    return ret;
}

GribRecord *GribRecord::MagnitudeRecord(const GribRecord &rec1, const GribRecord &rec2)
{
    GribRecord *rec = new GribRecord(rec1);

    /* generate a record which is the combined magnitude of two records */
    if (rec1.data && rec2.data && rec1.Ni == rec2.Ni && rec1.Nj == rec2.Nj) {
        int size = rec1.Ni*rec1.Nj;
        for (int i=0; i<size; i++)
            if(rec1.data[i] == GRIB_NOTDEF || rec2.data[i] == GRIB_NOTDEF)
                rec->data[i] = GRIB_NOTDEF;
            else
                rec->data[i] = sqrt(pow(rec1.data[i], 2) + pow(rec2.data[i], 2));
    } else
        rec->ok=false;

    if (rec1.BMSbits != NULL && rec2.BMSbits != NULL) {
        if(rec1.BMSsize == rec2.BMSsize) {
        int size = rec1.BMSsize;
        for (int i=0; i<size; i++)
            rec->BMSbits[i] = rec1.BMSbits[i] & rec2.BMSbits[i];
        } else
            rec->ok = false;
    }

    return rec;
}

void GribRecord::Substract(const GribRecord &rec, bool pos)
{
    // for now only substract records of same size
    if (rec.data == 0 || !rec.isOk())
        return;

    if (data == 0 || !isOk())
        return;
        
    if (Ni != rec.Ni || Nj != rec.Nj) 
        return;
        
    zuint size = Ni *Nj;
    for (zuint i=0; i<size; i++) {
        if (rec.data[i] == GRIB_NOTDEF)
           continue; 
        if (data[i] == GRIB_NOTDEF) {
            data[i] = -rec.data[i];
            if (BMSbits != 0) {
                if (BMSsize > i) {
                    BMSbits[i >>3] |= 1 << (i&7);
                }
            }
        }
        else
            data[i] -= rec.data[i];
        if (data[i] < 0. && pos) {
            // data type should be positive...
            data[i] = 0.;
        }
    } 
}


//------------------------------------------------------------------------------
void  GribRecord::setDataType(const zuchar t)
{
	dataType = t;
	dataKey = makeKey(dataType, levelType, levelValue);
}
//------------------------------------------------------------------------------
std::string GribRecord::makeKey(int dataType,int levelType,int levelValue)
{   // Make data type key  sample:'11-100-850'
//	char ktmp[32];
//	wxSnprintf((wxChar *)ktmp, 32, "%d-%d-%d", dataType, levelType, levelValue);
//	return std::string(ktmp);

	wxString k;
	k.Printf(_T("%d-%d-%d"), dataType, levelType, levelValue);
	return std::string(k.mb_str());

}
//-----------------------------------------
GribRecord::~GribRecord()
{
    if (data) {
        delete [] data;
        data = NULL;
    }
    if (BMSbits) {
        delete [] BMSbits;
        BMSbits = NULL;
    }

//if (dataType==GRB_TEMP) printf("record destroyed %s   %d\n", dataKey.mb_str(), (int)curDate/3600);
}

//-------------------------------------------------------------------------------
void  GribRecord::multiplyAllData(double k)
{
	for (zuint j=0; j<Nj; j++) {
		for (zuint i=0; i<Ni; i++)
		{
			if (isDefined(i,j)) {
				data[j*Ni+i] *= k;
			}
		}
	}
}

//----------------------------------------------
void  GribRecord::setRecordCurrentDate (time_t t)
{
	curDate = t;

    struct tm *date = gmtime(&t);

    zuint year   = date->tm_year+1900;
    zuint month  = date->tm_mon+1;
	zuint day    = date->tm_mday;
	zuint hour   = date->tm_hour;
	zuint minute = date->tm_min;
	sprintf(strCurDate, "%04d-%02d-%02d %02d:%02d", year,month,day,hour,minute);
}

//----------------------------------------------
time_t GribRecord::makeDate(
            zuint year,zuint month,zuint day,zuint hour,zuint min,zuint sec) {
    struct tm date;
    date.tm_sec  = 0  ;         /* seconds */
    date.tm_min  = 0;           /* minutes */
    date.tm_hour = 0;           /* hours */
    date.tm_mday = day;         /* day of the month */
    date.tm_mon  = month-1;     /* month */
    date.tm_year = year-1900;   /* year */
    date.tm_wday   = 0;         /* day of the week */
    date.tm_yday   = 0;         /* day in the year */
    date.tm_isdst  = 0;         /* daylight saving time */

	time_t   temps = -1;
    wxDateTime dt(date);
	temps = dt.GetTicks();
	temps += ((hour * 3600) + (min * 60 ) + sec);								//find datetime exactly as in the file
	wxDateTime dtt(temps);
	wxTimeSpan of = wxDateTime::Now() - (wxDateTime::Now().ToGMT() );			//transform to local time
	if(dtt.IsDST())																//correct dst offset applied 3 times  why ???
		of -= wxTimeSpan( 2, 0 );
	dtt += of;
	temps = dtt.GetTicks();

/*
	char sdate[64];
	sprintf(sdate, "%04d-%02d-%02d 00:00:00", year,month,day);
    QDateTime dt = QDateTime::fromString(sdate, "yyyy-MM-dd HH:mm:ss");
    dt.setTimeSpec(Qt::UTC);
	dt = dt.addSecs (hour*3600+min*60+sec);
	temps = dt.toTime_t();
*/
	return temps;
}


//===============================================================================================

double GribRecord::getInterpolatedValue(double px, double py, bool numericalInterpolation, bool dir) const
{
    if (!ok || Di==0 || Dj==0)
        return GRIB_NOTDEF;

    if (!isPointInMap(px,py)) {
        px += 360.0;               // tour du monde à droite ?
        if (!isPointInMap(px,py)) {
            px -= 2*360.0;              // tour du monde à gauche ?
            if (!isPointInMap(px,py)) {
                return GRIB_NOTDEF;
            }
        }
    }
    double pi, pj;     // coord. in grid unit
    pi = (px-Lo1)/Di;
    pj = (py-La1)/Dj;

    // 00 10      point is in a square
    // 01 11
    int i0 = (int) pi;  // point 00
    int j0 = (int) pj;

    unsigned int i1 = pi+1, j1 = pj+1;

    if(i1 >= Ni)
        i1 = i0;
    
    if(j1 >= Nj)
        j1 = j0;
    
    // distances to 00
    double dx = pi-i0;
    double dy = pj-j0;

    if (! numericalInterpolation)
    {
        if (dx >= 0.5)
            i0 = i1;
        if (dy >= 0.5)
            j0 = j1;

        return getValue(i0, j0);
    }

//     bool h00,h01,h10,h11;
//     int nbval = 0;     // how many values in grid ?
//     if ((h00=isDefined(i0, j0)))
//         nbval ++;
//     if ((h10=isDefined(i1, j0)))
//         nbval ++;
//     if ((h01=isDefined(i0, j1)))
//         nbval ++;
//     if ((h11=isDefined(i1, j1)))
//         nbval ++;

    int nbval = 0;     // how many values in grid ?
    if (getValue(i0, j0) != GRIB_NOTDEF)
        nbval ++;
    if (getValue(i1, j0) != GRIB_NOTDEF)
        nbval ++;
    if (getValue(i0, j1) != GRIB_NOTDEF)
        nbval ++;
    if (getValue(i1, j1) != GRIB_NOTDEF)
        nbval ++;
    
    
    
    
    
    if (nbval < 3)
        return GRIB_NOTDEF;

    dx = (3.0 - 2.0*dx)*dx*dx;   // pseudo hermite interpolation
    dy = (3.0 - 2.0*dy)*dy*dy;

    double xa, xb, xc, kx, ky;
    // Triangle :
    //   xa  xb
    //   xc
    // kx = distance(xa,x)
    // ky = distance(xa,y)
    if (nbval == 4)
    {
        double x00 = getValue(i0, j0);
        double x01 = getValue(i0, j1);
        double x10 = getValue(i1, j0);
        double x11 = getValue(i1, j1);
		if( !dir ) {
			double x1 = (1.0-dx)*x00 + dx*x10;
			double x2 = (1.0-dx)*x01 + dx*x11;
			return (1.0-dy)*x1 + dy*x2;
		} else {
			double x1 = interp_angle(x00, x01, dx, 180.);
			double x2 = interp_angle(x10, x11, dx, 180.);
			return interp_angle(x1, x2, dy, 180.);
		}
    }

	//interpolation with only three points is too hazardous for angles
	if( dir ) return GRIB_NOTDEF;

    // here nbval==3, check the corner without data
    if (getValue(i0, j0) == GRIB_NOTDEF) {
        //printf("! h00  %f %f\n", dx,dy);
        xa = getValue(i1, j1);   // A = point 11
        xb = getValue(i0, j1);   // B = point 01
        xc = getValue(i1, j0);   // C = point 10
        kx = 1-dx;
        ky = 1-dy;
    }
    else if (getValue(i0, j1) == GRIB_NOTDEF) {
        //printf("! h01  %f %f\n", dx,dy);
        xa = getValue(i1, j0);     // A = point 10
        xb = getValue(i1, j1);   // B = point 11
        xc = getValue(i0, j0);     // C = point 00
        kx = dy;
        ky = 1-dx;
    }
    else if (getValue(i1, j0) == GRIB_NOTDEF) {
        //printf("! h10  %f %f\n", dx,dy);
        xa = getValue(i0, j1);     // A = point 01
        xb = getValue(i0, j0);       // B = point 00
        xc = getValue(i1, j1);     // C = point 11
        kx = 1-dy;
        ky = dx;
    }
    else {
        //printf("! h11  %f %f\n", dx,dy);
        xa = getValue(i0, j0);  // A = point 00
        xb = getValue(i1, j0);  // B = point 10
        xc = getValue(i0, j1);  // C = point 01
        kx = dx;
        ky = dy;
    }

    double k = kx + ky;
    if (k<0 || k>1)
        return GRIB_NOTDEF;

    if (k == 0)
        return xa;

    // axes interpolation
    double vx = k*xb + (1-k)*xa;
    double vy = k*xc + (1-k)*xa;
    // diagonal interpolation
    double k2 = kx / k;
    return  k2*vx + (1-k2)*vy;
}

bool GribRecord::getInterpolatedValues(double &M, double &A,
                                       const GribRecord *GRX, const GribRecord *GRY,
                                       double px, double py, bool numericalInterpolation)
{
    if(!GRX || !GRY)
        return false;

    if (!GRX->ok || !GRY->ok || GRX->Di==0 || GRX->Dj==0)
        return false;

    if (!GRX->isPointInMap(px,py) || !GRY->isPointInMap(px,py)) {
        px += 360.0;               // tour du monde à droite ?
        if (!GRX->isPointInMap(px,py) || !GRY->isPointInMap(px,py)) {
            px -= 2*360.0;              // tour du monde à gauche ?
            if (!GRX->isPointInMap(px,py) || !GRY->isPointInMap(px,py)) {
                return false;
            }
        }
    }
    double pi, pj;     // coord. in grid unit
    pi = (px-GRX->Lo1)/GRX->Di;
    pj = (py-GRX->La1)/GRX->Dj;

    // 00 10      point is in a square
    // 01 11
    int i0 = (int) pi;  // point 00
    int j0 = (int) pj;

    unsigned int i1 = pi+1, j1 = pj+1;
    if(i1 >= GRX->Ni)
        i1 = i0;

    if(j1 >= GRX->Nj)
        j1 = j0;

    // distances to 00
    double dx = pi-i0;
    double dy = pj-j0;

    if (! numericalInterpolation)
    {
        double vx, vy;
        if (dx >= 0.5)
            i0 = i1;
        if (dy >= 0.5)
            j0 = j1;

        vx = GRX->getValue(i0, j0);
        vy = GRY->getValue(i0, j0);
        if (vx == GRIB_NOTDEF || vy == GRIB_NOTDEF)
            return false;

        M = sqrt(vx*vx + vy*vy);
        A = atan2(-vx, -vy) * 180 / M_PI;
        return true;
    }

//     bool h00,h01,h10,h11;
//     int nbval = 0;     // how many values in grid ?
//     if ((h00=GRX->isDefined(i0, j0) && GRX->isDefined(i0, j0)))
//         nbval ++;
//     if ((h10=GRX->isDefined(i1, j0) && GRY->isDefined(i1, j0)))
//         nbval ++;
//     if ((h01=GRX->isDefined(i0, j1) && GRY->isDefined(i0, j1)))
//         nbval ++;
//     if ((h11=GRX->isDefined(i1, j1) && GRY->isDefined(i1, j1)))
//         nbval ++;

     int nbval = 0;     // how many values in grid ?
     if (GRX->getValue(i0, j0) != GRIB_NOTDEF)
         nbval ++;
     if (GRY->getValue(i1, j0) != GRIB_NOTDEF)
         nbval ++;
     if (GRY->getValue(i0, j1) != GRIB_NOTDEF)
         nbval ++;
     if (GRY->getValue(i1, j1) != GRIB_NOTDEF)
         nbval ++;

    if (nbval < 3)
        return false;

    dx = (3.0 - 2.0*dx)*dx*dx;   // pseudo hermite interpolation
    dy = (3.0 - 2.0*dy)*dy*dy;

    // Triangle :
    //   xa  xb
    //   xc
    // kx = distance(xa,x)
    // ky = distance(xa,y)
    if (nbval == 4)
    {
        double x00x = GRX->getValue(i0, j0), x00y = GRY->getValue(i0, j0);
        double x00m = sqrt(x00x*x00x + x00y*x00y), x00a = atan2(x00x, x00y);

        double x01x = GRX->getValue(i0, j1), x01y = GRY->getValue(i0, j1);
        double x01m = sqrt(x01x*x01x + x01y*x01y), x01a = atan2(x01x, x01y);

        double x10x = GRX->getValue(i1, j0), x10y = GRY->getValue(i1, j0);
        double x10m = sqrt(x10x*x10x + x10y*x10y), x10a = atan2(x10x, x10y);

        double x11x = GRX->getValue(i1, j1), x11y = GRY->getValue(i1, j1);
        double x11m = sqrt(x11x*x11x + x11y*x11y), x11a = atan2(x11x, x11y);

        double x0m = (1-dx)*x00m + dx*x10m, x0a = interp_angle(x00a, x10a, dx, M_PI);

        double x1m = (1-dx)*x01m + dx*x11m, x1a = interp_angle(x01a, x11a, dx, M_PI);

        M = (1-dy)*x0m + dy*x1m;
        A = interp_angle(x0a, x1a, dy, M_PI);
        A *= 180 / M_PI; // degrees
        A += 180;

        return true;
    }

    return false; // TODO: make this work in the cases of only 3 points
#if 0
        double xa, xb, xc, kx, ky;
        // here nbval==3, check the corner without data
        if (!h00) {
            //printf("! h00  %f %f\n", dx,dy);
            xa = getValue(i1, j1);   // A = point 11
            xb = getValue(i0, j1);   // B = point 01
            xc = getValue(i1, j0);   // C = point 10
            kx = 1-dx;
            ky = 1-dy;
        }
        else if (!h01) {
            //printf("! h01  %f %f\n", dx,dy);
            xa = getValue(i1, j0);     // A = point 10
            xb = getValue(i1, j1);   // B = point 11
            xc = getValue(i0, j0);     // C = point 00
            kx = dy;
            ky = 1-dx;
        }
        else if (!h10) {
            //printf("! h10  %f %f\n", dx,dy);
            xa = getValue(i0, j1);     // A = point 01
            xb = getValue(i0, j0);       // B = point 00
            xc = getValue(i1, j1);     // C = point 11
            kx = 1-dy;
            ky = dx;
        }
        else {
            //printf("! h11  %f %f\n", dx,dy);
            xa = getValue(i0, j0);  // A = point 00
            xb = getValue(i1, j0);  // B = point 10
            xc = getValue(i0, j1);  // C = point 01
            kx = dx;
            ky = dy;
        }
    }
    double k = kx + ky;
    if (k<0 || k>1) {
        val = GRIB_NOTDEF;
    }
    else if (k == 0) {
        val = xa;
    }
    else {
        // axes interpolation
        double vx = k*xb + (1-k)*xa;
        double vy = k*xc + (1-k)*xa;
        // diagonal interpolation
        double k2 = kx / k;
        val =  k2*vx + (1-k2)*vy;
    }
    return val;
#endif
}
