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
// Adjust data type from different mete center
//-------------------------------------------------------------------------------
void  GribRecord::translateDataType()
{
	this->knownData = true;
	//------------------------
	// NOAA GFS
	//------------------------
	if (   idCenter==7
		&& (idModel==96 || idModel==81)		// NOAA
		&& (idGrid==4 || idGrid==255))		// Saildocs
	{
        dataCenterModel = NOAA_GFS;
		if (dataType == GRB_PRECIP_TOT) {	// mm/period -> mm/h
			if (periodP2 > periodP1)
				multiplyAllData( 1.0/(periodP2-periodP1) );
		}
		if (dataType == GRB_PRECIP_RATE) {	// mm/s -> mm/h
			if (periodP2 > periodP1)
				multiplyAllData( 3600.0 );
        }
        if (dataType == GRB_TEMP                        //gfs Water surface Temperature
            && levelType == LV_GND_SURF
            && levelValue == 0) dataType = GRB_WTMP;

	}
    //------------------------
	//DNMI-NEurope.grb
	//------------------------
    else if ( (idCenter==88 && idModel==255 && idGrid==255)
		|| (idCenter==88 && idModel==230 && idGrid==255)
		|| (idCenter==88 && idModel==200 && idGrid==255)
		|| (idCenter==88 && idModel==67 && idGrid==255) )
    {
        if( dataType==GRB_TEMP && levelType==LV_GND_SURF && levelValue==0 ) {       //air temperature at groud level
            levelType = LV_ABOV_GND; levelValue = 2;
        }
		dataCenterModel = NORWAY_METNO;
    }
	//------------------------
	// WRF NMM grib.meteorologic.net
	//------------------------
	else if (idCenter==7 && idModel==89 && idGrid==255)
    {
        // dataCenterModel ??
		if (dataType == GRB_PRECIP_TOT) {	// mm/period -> mm/h
			if (periodP2 > periodP1)
				multiplyAllData( 1.0/(periodP2-periodP1) );
		}
		if (dataType == GRB_PRECIP_RATE) {	// mm/s -> mm/h
			if (periodP2 > periodP1)
				multiplyAllData( 3600.0 );
		}


	}
    else if ( idCenter==7 && idModel==88 && idGrid==255 ) {  // saildocs
		dataCenterModel = NOAA_NCEP_WW3;
	}
    //----------------------------
    //NOAA RTOFS
    //--------------------------------
    else if(idCenter==7 && idModel==45 && idGrid==255) {
        dataCenterModel = NOAA_RTOFS;
    }
    //----------------------------------------------
    // NCEP sea surface temperature
    //----------------------------------------------
    else if ((idCenter==7 && idModel==44 && idGrid==173)
        || (idCenter==7 && idModel==44 && idGrid==235))
    {
        dataCenterModel = NOAA_NCEP_SST;
    }
    //----------------------------------------------
    // FNMOC WW3 mediterranean sea
    //----------------------------------------------
    else if (idCenter==58 && idModel==111 && idGrid==179)
    {
        dataCenterModel = FNMOC_WW3_MED;
    }
    //----------------------------------------------
    // FNMOC WW3
    //----------------------------------------------
    else if (idCenter==58 && idModel==110 && idGrid==240)
    {
        dataCenterModel = FNMOC_WW3_GLB;
    }
	//------------------------
	// Meteorem (Scannav)
	//------------------------
	else if (idCenter==59 && idModel==78 && idGrid==255)
	{
        //dataCenterModel = ??
		if ( (getDataType()==GRB_WIND_VX || getDataType()==GRB_WIND_VY)
			&& getLevelType()==LV_MSL
			&& getLevelValue()==0)
		{
			levelType  = LV_ABOV_GND;
			levelValue = 10;
		}
		if ( getDataType()==GRB_PRECIP_TOT
			&& getLevelType()==LV_MSL
			&& getLevelValue()==0)
		{
			levelType  = LV_GND_SURF;
			levelValue = 0;
		}
	}
	//------------------------
	// Unknown center
	//------------------------
	else
	{
        dataCenterModel = OTHER_DATA_CENTER;
//		printf("Uncorrected GribRecord: ");
//		this->print();
//		this->knownData = false;

	}
    //translate significant wave height and dir
    if (this->knownData) {
        switch (getDataType()) {
            case GRB_HTSGW:
            case GRB_WVDIR:
            case GRB_WVPER:
                levelType  = LV_GND_SURF;
                levelValue = 0;
                break;
        }
    }
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
// Lecture depuis un fichier
//-------------------------------------------------------------------------------
GribRecord::GribRecord(ZUFILE* file, int id_)
{
    id = id_;
//   seekStart = zu_tell(file);           // moved to section 0 read
    data    = NULL;
    BMSbits = NULL;
    eof     = false;
    knownData = true;
    IsDuplicated = false;

    //      Pre read 4 bytes to check for length adder needed for some GRIBS (like WRAMS and NAM)
    char strgrib[5];
    if (zu_read(file, strgrib, 4) != 4)
    {
            ok = false;
            eof = true;
            return;
    }

    bool b_haveReadGRIB = false;         // already read the "GRIB" of section 0 ??

    if (strncmp(strgrib, "GRIB", 4) != 0)
            b_len_add_8 = true;
    else
    {
            b_len_add_8 = false;
            b_haveReadGRIB = true;
    }

    // Another special case, where zero padding is used between records.
    if((strgrib[0] == 0) &&
        (strgrib[1] == 0) &&
        (strgrib[2] == 0) &&
        (strgrib[3] == 0))
    {
          b_len_add_8 = false;
          b_haveReadGRIB = false;
    }




    ok = readGribSection0_IS(file, b_haveReadGRIB );
    if (ok) {
        ok = readGribSection1_PDS(file);
        zu_seek(file, fileOffset1+sectionSize1, SEEK_SET);
    }
    if (ok) {
        ok = readGribSection2_GDS(file);
        zu_seek(file, fileOffset2+sectionSize2, SEEK_SET);
    }
    if (ok) {
        ok = readGribSection3_BMS(file);
        zu_seek(file, fileOffset3+sectionSize3, SEEK_SET);
    }
    if (ok) {
        ok = readGribSection4_BDS(file);
        zu_seek(file, fileOffset4+sectionSize4, SEEK_SET);
    }
    if (ok) {
        ok = readGribSection5_ES(file);
    }
    if (ok) {
          zu_seek(file, seekStart+totalSize + (b_len_add_8 ? 8 : 0), SEEK_SET);
    }

    if (ok) {
		translateDataType();
		setDataType(dataType);
	}
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
        int size = rec.sectionSize3-6;
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
    double rec1offdi, rec2offdi;
    double rec1offdj, rec2offdj;

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
        if(rec1.sectionSize3 == rec2.sectionSize3) {
        int size = rec1.sectionSize3-6;
        for (int i=0; i<size; i++)
            rec->BMSbits[i] = rec1.BMSbits[i] & rec2.BMSbits[i];
        } else
            rec->ok = false;
    }

    return rec;
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

//==============================================================
// Lecture des données
//==============================================================
//----------------------------------------------
// SECTION 0: THE INDICATOR SECTION (IS)
//----------------------------------------------
bool GribRecord::readGribSection0_IS(ZUFILE* file, bool b_skip_initial_GRIB) {
    char strgrib[4];
    fileOffset0 = zu_tell(file);

    if(!b_skip_initial_GRIB)
    {
            // Cherche le 1er 'G'
            while ( (zu_read(file, strgrib, 1) == 1)
                              &&  (strgrib[0] != 'G') )
            { }

      if (strgrib[0] != 'G') {
            ok = false;
            eof = true;
            return false;
      }
      if (zu_read(file, strgrib+1, 3) != 3) {
            ok = false;
            eof = true;
            return false;
      }
      /*    if (zu_read(file, strgrib, 4) != 4) {
            ok = false;
            eof = true;
            return false;
      }*/
      if (strncmp(strgrib, "GRIB", 4) != 0)  {
//            erreur("readGribSection0_IS(): Unknown file header : %c%c%c%c",
//                        strgrib[0],strgrib[1],strgrib[2],strgrib[3]);
            ok = false;
            eof = true;
            return false;
      }
    }

    seekStart = zu_tell(file) - 4;
    totalSize = readInt3(file);

    editionNumber = readChar(file);
    if (editionNumber != 1)  {
        ok = false;
        eof = true;
        return false;
    }

    return true;
}
//----------------------------------------------
// SECTION 1: THE PRODUCT DEFINITION SECTION (PDS)
//----------------------------------------------
bool GribRecord::readGribSection1_PDS(ZUFILE* file) {
    fileOffset1 = zu_tell(file);
    if (zu_read(file, data1, 28) != 28) {
        ok=false;
        eof = true;
        return false;
    }
    sectionSize1 = makeInt3(data1[0],data1[1],data1[2]);
    tableVersion = data1[3];
    idCenter = data1[4];
    idModel  = data1[5];
    idGrid   = data1[6];
    hasGDS = (data1[7]&128)!=0;
    hasBMS = (data1[7]&64)!=0;

    dataType = data1[8];	 // octet 9 = parameters and units
	levelType = data1[9];
	levelValue = makeInt2(data1[10],data1[11]);

    refyear   = (data1[24]-1)*100+data1[12];
    refmonth  = data1[13];
    refday    = data1[14];
    refhour   = data1[15];
    refminute = data1[16];

    refDate = makeDate(refyear,refmonth,refday,refhour,refminute,0);
	sprintf(strRefDate, "%04d-%02d-%02d %02d:%02d", refyear,refmonth,refday,refhour,refminute);

    periodP1  = data1[18];
    periodP2  = data1[19];
    timeRange = data1[20];
    periodsec = periodSeconds(data1[17],data1[18],data1[19],timeRange);
    curDate = makeDate(refyear,refmonth,refday,refhour,refminute,periodsec);

//if (dataType == GRB_PRECIP_TOT) printf("P1=%d p2=%d\n", periodP1,periodP2);

    int decim;
    decim = (int)(((((zuint)data1[26]&0x7F)<<8)+(zuint)data1[27])&0x7FFF);
    if (data1[26]&0x80)
        decim *= -1;
    decimalFactorD = pow(10.0, decim);

    // Controls
    if (! hasGDS) {
        erreur("Record %d: GDS not found",id);
        ok = false;
    }
    if (decimalFactorD == 0) {
        erreur("Record %d: decimalFactorD null",id);
        ok = false;
    }
    return ok;
}
//----------------------------------------------
// SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
//----------------------------------------------
bool GribRecord::readGribSection2_GDS(ZUFILE* file) {
    if (! hasGDS)
        return 0;
    fileOffset2 = zu_tell(file);
    sectionSize2 = readInt3(file);  // byte 1-2-3
    NV = readChar(file);			// byte 4
    PV = readChar(file); 			// byte 5
    gridType = readChar(file); 		// byte 6

    if (gridType != 0
    		// && gridType != 4
		) {
        erreur("Record %d: unknown grid type GDS(6) : %d",id,gridType);
        ok = false;
    }

    Ni  = readInt2(file);				// byte 7-8
    Nj  = readInt2(file);				// byte 9-10
    La1 = readSignedInt3(file)/1000.0;	// byte 11-12-13
    Lo1 = readSignedInt3(file)/1000.0;	// byte 14-15-16
    resolFlags = readChar(file);		// byte 17
    La2 = readSignedInt3(file)/1000.0;	// byte 18-19-20
    Lo2 = readSignedInt3(file)/1000.0;	// byte 21-22-23

    if (Lo1>=0 && Lo1<=180 && Lo2<0) {
        Lo2 += 360.0;    // cross the 180 deg meridien,beetwen alaska and russia
    }

    Di  = readSignedInt2(file)/1000.0;	// byte 24-25
    Dj  = readSignedInt2(file)/1000.0;	// byte 26-27

    while ( Lo1> Lo2   &&  Di >0) {   // horizontal size > 360 °
        Lo1 -= 360.0;
    }
    hasDiDj = (resolFlags&0x80) !=0;
    isEarthSpheric = (resolFlags&0x40) ==0;
    isUeastVnorth =  (resolFlags&0x08) ==0;

    scanFlags = readChar(file);			// byte 28
    isScanIpositive = (scanFlags&0x80) ==0;
    isScanJpositive = (scanFlags&0x40) !=0;
    isAdjacentI     = (scanFlags&0x20) ==0;

   	if (Lo2 > Lo1) {
	    lonMin = Lo1;
	    lonMax = Lo2;
	}
  	else {
	    lonMin = Lo2;
	    lonMax = Lo1;
	}
   	if (La2 > La1) {
	    latMin = La1;
	    latMax = La2;
	}
  	else {
	    latMin = La2;
	    latMax = La1;
	}
	if (Ni<=1 || Nj<=1) {
		erreur("Record %d: Ni=%d Nj=%d",id,Ni,Nj);
		ok = false;
	}
	else {
		Di = (Lo2-Lo1) / (Ni-1);
		Dj = (La2-La1) / (Nj-1);
	}

if (false) {
printf("====\n");
printf("Lo1=%f Lo2=%f    La1=%f La2=%f\n", Lo1,Lo2,La1,La2);
printf("Ni=%d Nj=%d\n", Ni,Nj);
printf("hasDiDj=%d Di,Dj=(%f %f)\n", hasDiDj, Di,Dj);
printf("hasBMS=%d\n", hasBMS);
printf("isScanIpositive=%d isScanJpositive=%d isAdjacentI=%d\n",
                        isScanIpositive,isScanJpositive,isAdjacentI );
}
    return ok;
}
//----------------------------------------------
// SECTION 3: BIT MAP SECTION (BMS)
//----------------------------------------------
bool GribRecord::readGribSection3_BMS(ZUFILE* file) {
    fileOffset3 = zu_tell(file);
    if (! hasBMS) {
        sectionSize3 = 0;
        return ok;
    }
    sectionSize3 = readInt3(file);
    (void) readChar(file);
    int bitMapFollows = readInt2(file);

    if (bitMapFollows != 0) {
        return ok;
    }
    BMSbits = new zuchar[sectionSize3-6];
    if (!BMSbits) {
        erreur("Record %d: out of memory",id);
        ok = false;
    }
    for (zuint i=0; i<sectionSize3-6; i++) {
        BMSbits[i] = readChar(file);
    }
    return ok;
}
//----------------------------------------------
// SECTION 4: BINARY DATA SECTION (BDS)
//----------------------------------------------
bool GribRecord::readGribSection4_BDS(ZUFILE* file) {
    fileOffset4  = zu_tell(file);
    sectionSize4 = readInt3(file);  // byte 1-2-3

    zuchar flags  = readChar(file);			// byte 4
    scaleFactorE = readSignedInt2(file);	// byte 5-6
    refValue     = readFloat4(file);		// byte 7-8-9-10
    nbBitsInPack = readChar(file);			// byte 11
    scaleFactorEpow2 = pow(2.,scaleFactorE);
    unusedBitsEndBDS = flags & 0x0F;
    isGridData      = (flags&0x80) ==0;
    isSimplePacking = (flags&0x80) ==0;
    isFloatValues   = (flags&0x80) ==0;

//printf("BDS type=%3d - bits=%02d - level %3d - %d\n", dataType, nbBitsInPack, levelType,levelValue);

    if (! isGridData) {
        erreur("Record %d: need grid data",id);
        ok = false;
    }
    if (! isSimplePacking) {
        erreur("Record %d: need simple packing",id);
        ok = false;
    }
    if (! isFloatValues) {
        erreur("Record %d: need double values",id);
        ok = false;
    }

    if (!ok) {
        return ok;
    }

    // Allocate memory for the data
    data = new double[Ni*Nj];
    if (!data) {
        erreur("Record %d: out of memory",id);
        ok = false;
    }

    zuint  startbit  = 0;
    int  datasize = sectionSize4-11;
    zuchar *buf = new zuchar[datasize+4]();  // +4 pour simplifier les décalages ds readPackedBits
    if (!buf) {
        erreur("Record %d: out of memory",id);
        ok = false;
    }
    if (zu_read(file, buf, datasize) != datasize) {
        erreur("Record %d: data read error",id);
        ok = false;
        eof = true;
    }
    if (!ok) {
        return ok;
    }

    // Read data in the order given by isAdjacentI
    zuint i, j, x;
    int ind;
    if (isAdjacentI) {
        for (j=0; j<Nj; j++) {
            for (i=0; i<Ni; i++) {
                if (!hasDiDj && !isScanJpositive) {
                    ind = (Nj-1 -j)*Ni+i;
                }
                else {
                    ind = j*Ni+i;
                }
                if (hasValue(i,j)) {
                    x = readPackedBits(buf, startbit, nbBitsInPack);
                    data[ind] = (refValue + x*scaleFactorEpow2)/decimalFactorD;
                    startbit += nbBitsInPack;
                }
                else {
                    data[ind] = GRIB_NOTDEF;
                }
            }
        }
    }
    else {
        for (i=0; i<Ni; i++) {
            for (j=0; j<Nj; j++) {
                if (!hasDiDj && !isScanJpositive) {
                    ind = (Nj-1 -j)*Ni+i;
                }
                else {
                    ind = j*Ni+i;
                }
                if (hasValue(i,j)) {
                    x = readPackedBits(buf, startbit, nbBitsInPack);
                    startbit += nbBitsInPack;
                    data[ind] = (refValue + x*scaleFactorEpow2)/decimalFactorD;
                }
                else {
                    data[ind] = GRIB_NOTDEF;
                }
            }
        }
    }

    if (buf) {
        delete [] buf;
        buf = NULL;
    }
    return ok;
}



//----------------------------------------------
// SECTION 5: END SECTION (ES)
//----------------------------------------------
bool GribRecord::readGribSection5_ES(ZUFILE* file) {
    char str[4];
    if (zu_read(file, str, 4) != 4) {
        ok = false;
        eof = true;
        return false;
    }
    if (strncmp(str, "7777", 4) != 0)  {
        erreur("Final 7777 not read: %c%c%c%c",str[0],str[1],str[2],str[3]);
        ok = false;
        return false;
    }
    return ok;
}






//==============================================================
// Fonctions utiles
//==============================================================
double GribRecord::readFloat4(ZUFILE* file) {
    unsigned char t[4];
    if (zu_read(file, t, 4) != 4) {
        ok = false;
        eof = true;
        return 0;
    }

    double val;
    int A = (zuint)t[0]&0x7F;
    int B = ((zuint)t[1]<<16)+((zuint)t[2]<<8)+(zuint)t[3];

    val = pow(2.,-24)*B*pow(16.,A-64);
    if (t[0]&0x80)
        return -val;
    else
        return val;
}
//----------------------------------------------
zuchar GribRecord::readChar(ZUFILE* file) {
    zuchar t;
    if (zu_read(file, &t, 1) != 1) {
        ok = false;
        eof = true;
        return 0;
    }
    return t;
}
//----------------------------------------------
int GribRecord::readSignedInt3(ZUFILE* file) {
    unsigned char t[3];
    if (zu_read(file, t, 3) != 3) {
        ok = false;
        eof = true;
        return 0;
    }
    int val = (((zuint)t[0]&0x7F)<<16)+((zuint)t[1]<<8)+(zuint)t[2];
    if (t[0]&0x80)
        return -val;
    else
        return val;
}
//----------------------------------------------
int GribRecord::readSignedInt2(ZUFILE* file) {
    unsigned char t[2];
    if (zu_read(file, t, 2) != 2) {
        ok = false;
        eof = true;
        return 0;
    }
    int val = (((zuint)t[0]&0x7F)<<8)+(zuint)t[1];
    if (t[0]&0x80)
        return -val;
    else
        return val;
}
//----------------------------------------------
zuint GribRecord::readInt3(ZUFILE* file) {
    unsigned char t[3];
    if (zu_read(file, t, 3) != 3) {
        ok = false;
        eof = true;
        return 0;
    }
    return ((zuint)t[0]<<16)+((zuint)t[1]<<8)+(zuint)t[2];
}
//----------------------------------------------
zuint GribRecord::readInt2(ZUFILE* file) {
    unsigned char t[2];
    if (zu_read(file, t, 2) != 2) {
        ok = false;
        eof = true;
        return 0;
    }
    return ((zuint)t[0]<<8)+(zuint)t[1];
}
//----------------------------------------------
zuint GribRecord::makeInt3(zuchar a, zuchar b, zuchar c) {
    return ((zuint)a<<16)+((zuint)b<<8)+(zuint)c;
}
//----------------------------------------------
zuint GribRecord::makeInt2(zuchar b, zuchar c) {
    return ((zuint)b<<8)+(zuint)c;
}
//----------------------------------------------
zuint GribRecord::readPackedBits(zuchar *buf, zuint first, zuint nbBits)
{
    zuint oct = first / 8;
    zuint bit = first % 8;

    zuint val = (buf[oct]<<24) + (buf[oct+1]<<16) + (buf[oct+2]<<8) + (buf[oct+3]);
    val = val << bit;
    val = val >> (32-nbBits);
    return val;
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
//----------------------------------------------
zuint GribRecord::periodSeconds(zuchar unit,zuchar P1,zuchar P2,zuchar range) {
    zuint res, dur;
    switch (unit) {
        case 0: //	Minute
            res = 60; break;
        case 1: //	Hour
            res = 3600; break;
        case 2: //	Day
            res = 86400; break;
        case 10: //	3 hours
            res = 10800; break;
        case 11: //	6 hours
            res = 21600; break;
        case 12: //	12 hours
            res = 43200; break;
        case 254: // Second
            res = 1; break;
        case 3: //	Month
        case 4: //	Year
        case 5: //	Decade (10 years)
        case 6: //	Normal (30 years)
        case 7: //	Century (100 years)
        default:
            erreur("id=%d: unknown time unit in PDS b18=%d",id,unit);
            res = 0;
            ok = false;
    }
    debug("id=%d: PDS (time range) b21=%d P1=%d P2=%d",id,range,P1,P2);
    dur = 0;
    switch (range) {
        case 0:
            dur = (zuint)P1; break;
        case 1:
            dur = 0; break;
        case 2:
        case 3:
            // dur = ((zuint)P1+(zuint)P2)/2; break;     // TODO
            dur = (zuint)P2; break;
         case 4:
            dur = (zuint)P2; break;
        case 10:
            dur = ((zuint)P1<<8) + (zuint)P2; break;
        default:
            erreur("id=%d: unknown time range in PDS b21=%d",id,range);
            dur = 0;
            ok = false;
    }
    return res*dur;
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
        i1 -= Ni;

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

    bool h00,h01,h10,h11;
    int nbval = 0;     // how many values in grid ?
    if ((h00=isDefined(i0, j0)))
        nbval ++;
    if ((h10=isDefined(i1, j0)))
        nbval ++;
    if ((h01=isDefined(i0, j1)))
        nbval ++;
    if ((h11=isDefined(i1, j1)))
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
        i1 -= GRX->Ni;

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

        M = sqrt(vx*vx + vy*vy);
        A = atan2(-vx, -vy) * 180 / M_PI;
        return true;
    }

    bool h00,h01,h10,h11;
    int nbval = 0;     // how many values in grid ?
    if ((h00=GRX->isDefined(i0, j0) && GRX->isDefined(i0, j0)))
        nbval ++;
    if ((h10=GRX->isDefined(i1, j0) && GRY->isDefined(i1, j0)))
        nbval ++;
    if ((h01=GRX->isDefined(i0, j1) && GRY->isDefined(i0, j1)))
        nbval ++;
    if ((h11=GRX->isDefined(i1, j1) && GRY->isDefined(i1, j1)))
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
