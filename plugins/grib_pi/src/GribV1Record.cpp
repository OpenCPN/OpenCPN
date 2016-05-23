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

#include <stdlib.h>

#include "GribV1Record.h"

//-------------------------------------------------------------------------------
// Adjust data type from different mete center
//-------------------------------------------------------------------------------
void  GribV1Record::translateDataType()
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
		if (dataType == GRB_PRECIP_RATE) {	// mm/s -> mm/h
            multiplyAllData( 3600.0 );
        }
        if (dataType == GRB_TEMP                        //gfs Water surface Temperature
            && levelType == LV_GND_SURF
            && levelValue == 0) dataType = GRB_WTMP;

        // altitude level (entire atmosphere vs entire atmosphere considered as 1 level)
        if (levelType == LV_ATMOS_ENT) {
            levelType = LV_ATMOS_ALL;
        }
                                                                                
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
		if (dataType == GRB_PRECIP_RATE) {	// mm/s -> mm/h
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
            case GRB_UOGRD:
            case GRB_VOGRD:
                levelType  = LV_GND_SURF;
                levelValue = 0;
                break;
            case GRB_HTSGW:
            case GRB_WVDIR:
            case GRB_WVPER:
                levelType  = LV_GND_SURF;
                levelValue = 0;
                break;
        }
    }
    // this->print();
}


//-------------------------------------------------------------------------------
// Lecture depuis un fichier
//-------------------------------------------------------------------------------
GribV1Record::GribV1Record(ZUFILE* file, int id_)
{
    id = id_;
//   seekStart = zu_tell(file);           // moved to section 0 read
    data    = NULL;
    BMSbits = NULL;
    eof     = false;
    knownData = true;
    IsDuplicated = false;
    long start = zu_tell(file);

    //      Pre read 4 bytes to check for length adder needed for some GRIBS (like WRAMS and NAM)
    //but some Gribs has the "GRIB" header starting in second, third or fourth bytes. So for these cases
    //let's read its one by one. If 'G' is found in 1st byte or not found at all then
    //process as before, but if 'G' is not found in 1st byte but found in one of the next three bytes, stop reading
    //then the read can be continued from that position in the file in the section 0 read
    char strgrib[5];

    unsigned int b_haveReadGRIB = 0;				    // already read the "GRIB" of section 0 ?

	for (unsigned i = 0; i < 4; i++) {		            //read the four first bytes one by one
        if (zu_read(file, strgrib + i, 1) != 1) {       //detect end of file?
            ok = false;
            eof = true;
            return;
		} else {                            //search "GRIB" or at least "G"
			if (strgrib[0] != 'G') {		//if no 'G' found in the 1st byte
				if (strgrib[i] == 'G') {    //but found in the next 3 bytes
					b_haveReadGRIB = 1;		//stop reading.The 3 following bytes will be read in section 0 read starting at that position
					b_len_add_8 = false;
					break;
				} // end 'G' found in the next bytes
			} // end no 'G' found in 1st byte.
		}
	}// end reading four bytes

    if (b_haveReadGRIB == 0) {						//the four bytes have been read
		if (strncmp(strgrib, "GRIB", 4) != 0)
			b_len_add_8 = true;                 //"GRIB" header no valid so apply length adder. Further reading will happen
		else {
			b_haveReadGRIB = 2;					//"GRIB" header is valid so no further reading
			b_len_add_8 = false;
		}

        // Another special case, where zero padding is used between records.
        if((strgrib[0] == 0) &&
            (strgrib[1] == 0) &&
            (strgrib[2] == 0) &&
            (strgrib[3] == 0))
        {
            b_len_add_8 = false;
            b_haveReadGRIB = 0;
        }
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
    else {
        // XXX very slow with bzip2 file
        zu_seek(file, start, SEEK_SET);
    }
}

//-------------------------------------------------------------------------------
// Constructeur de recopie
//-------------------------------------------------------------------------------
GribV1Record::GribV1Record(const GribRecord &rec) : GribRecord(rec)
{
    *this = rec;
}

GribV1Record::~GribV1Record()
{
}

//----------------------------------------------
static zuint readPackedBits(zuchar *buf, zuint first, zuint nbBits)
{
    zuint oct = first / 8;
    zuint bit = first % 8;

    zuint val = (buf[oct]<<24) + (buf[oct+1]<<16) + (buf[oct+2]<<8) + (buf[oct+3]);
    val = val << bit;
    val = val >> (32-nbBits);
    return val;
}

//==============================================================
// Lecture des données
//==============================================================
//----------------------------------------------
// SECTION 0: THE INDICATOR SECTION (IS)
//----------------------------------------------
bool GribV1Record::readGribSection0_IS(ZUFILE* file, unsigned int b_skip_initial_GRIB) {
    char strgrib[4];
    fileOffset0 = zu_tell(file);

    if( b_skip_initial_GRIB == 0 )
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
    } else if(b_skip_initial_GRIB == 1)			//the first 'G' has been found previously
		strgrib[0] = 'G';

    if( b_skip_initial_GRIB == 0 || b_skip_initial_GRIB == 1 ) {    // contine to search the end of "GRIB" in the next three bytes
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
bool GribV1Record::readGribSection1_PDS(ZUFILE* file) {
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
bool GribV1Record::readGribSection2_GDS(ZUFILE* file) {
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
printf("==== GV1 \n");
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
bool GribV1Record::readGribSection3_BMS(ZUFILE* file) {
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
    BMSsize = sectionSize3-6;
    BMSbits = new zuchar[BMSsize];
    if (!BMSbits) {
        erreur("Record %d: out of memory",id);
        ok = false;
    }
    for (zuint i=0; i< BMSsize; i++) {
        BMSbits[i] = readChar(file);
    }
    return ok;
}

//----------------------------------------------
// SECTION 4: BINARY DATA SECTION (BDS)
//----------------------------------------------
bool GribV1Record::readGribSection4_BDS(ZUFILE* file) {
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
                    //printf(" %d %d %f ", i,j, data[ind]);
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
                    //printf(" %d %d %f ", i,j, data[ind]);
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
bool GribV1Record::readGribSection5_ES(ZUFILE* file) {
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
double GribV1Record::readFloat4(ZUFILE* file) {
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
zuchar GribV1Record::readChar(ZUFILE* file) {
    zuchar t;
    if (zu_read(file, &t, 1) != 1) {
        ok = false;
        eof = true;
        return 0;
    }
    return t;
}
//----------------------------------------------
int GribV1Record::readSignedInt3(ZUFILE* file) {
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
int GribV1Record::readSignedInt2(ZUFILE* file) {
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
zuint GribV1Record::readInt3(ZUFILE* file) {
    unsigned char t[3];
    if (zu_read(file, t, 3) != 3) {
        ok = false;
        eof = true;
        return 0;
    }
    return ((zuint)t[0]<<16)+((zuint)t[1]<<8)+(zuint)t[2];
}
//----------------------------------------------
zuint GribV1Record::readInt2(ZUFILE* file) {
    unsigned char t[2];
    if (zu_read(file, t, 2) != 2) {
        ok = false;
        eof = true;
        return 0;
    }
    return ((zuint)t[0]<<8)+(zuint)t[1];
}
//----------------------------------------------
zuint GribV1Record::makeInt3(zuchar a, zuchar b, zuchar c) {
    return ((zuint)a<<16)+((zuint)b<<8)+(zuint)c;
}
//----------------------------------------------
zuint GribV1Record::makeInt2(zuchar b, zuchar c) {
    return ((zuint)b<<8)+(zuint)c;
}
//----------------------------------------------
zuint GribV1Record::periodSeconds(zuchar unit,zuchar P1,zuchar P2,zuchar range) {
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
    grib_debug("id=%d: PDS unit %d (time range) b21=%d %d P1=%d P2=%d\n",id,unit, range,res,P1,P2);
    dur = 0;
    // (grib1/5.table)
    switch (range) {
        case 0:
            dur = (zuint)P1; break;
        case 1:
            dur = 0; break;

        case 2:
        case 3:  // Average  (reference time + P1 to reference time + P2)
            // dur = ((zuint)P1+(zuint)P2)/2; break;     // TODO
            dur = (zuint)P2; break;

         case 4: // Accumulation  (reference time + P1 to reference time + P2)
            dur = (zuint)P2; break;

        case 10: // P1 occupies octets 19 and 20; product valid at reference time + P1 
            dur = ((zuint)P1<<8) + (zuint)P2; break;
        default:
            erreur("id=%d: unknown time range in PDS b21=%d",id,range);
            dur = 0;
            ok = false;
    }
    return res*dur;
}


//===============================================================================================

