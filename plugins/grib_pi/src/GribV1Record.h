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


/******************************************
Elément de base d'un fichier GRIB V1
******************************************/

#ifndef GRIBV1RECORD_H
#define GRIBV1RECORD_H

#include <iostream>
#include <cmath>

#include "zuFile.h"
#include "GribRecord.h"


//----------------------------------------------
class GribV1Record : public GribRecord
{
    public:
        GribV1Record(ZUFILE* file, int id_);
        GribV1Record(const GribRecord &rec);
        GribV1Record() {}

        ~GribV1Record();

    protected:

    private:
        zuint  periodSeconds(zuchar unit, zuchar P1, zuchar P2, zuchar range);
        //-----------------------------------------
        void    translateDataType();  // adapte les codes des différents centres météo
        //---------------------------------------------
        // SECTION 0: THE INDICATOR SECTION (IS)
        //---------------------------------------------
        zuint  fileOffset0;
        zuint  seekStart, totalSize;
        // zuchar editionNumber;
        bool   b_len_add_8;

        // SECTION 1: THE PRODUCT DEFINITION SECTION (PDS)
        zuint  fileOffset1;
        zuint  sectionSize1;
        zuchar tableVersion;
        zuchar data1[28];
        bool   hasGDS;
        // bool   hasBMS;
        double  decimalFactorD;
        // SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
        zuint  fileOffset2;
        zuint  sectionSize2;
        // SECTION 3: BIT MAP SECTION (BMS)
        zuint  fileOffset3;
        zuint  sectionSize3;
        // zuchar *BMSbits;
        // SECTION 4: BINARY DATA SECTION (BDS)
        zuint  fileOffset4;
        zuint  sectionSize4;
        zuchar unusedBitsEndBDS;
        bool  isGridData;          // not spherical harmonics
        bool  isSimplePacking;
        bool  isFloatValues;
        int   scaleFactorE;
        double scaleFactorEpow2;
        double refValue;
        zuint  nbBitsInPack;
        // SECTION 5: END SECTION (ES)

        //---------------------------------------------
        // Data Access
        //---------------------------------------------
        bool readGribSection0_IS (ZUFILE* file, unsigned int b_skip_initial_GRIB);
        bool readGribSection1_PDS(ZUFILE* file);
        bool readGribSection2_GDS(ZUFILE* file);
        bool readGribSection3_BMS(ZUFILE* file);
        bool readGribSection4_BDS(ZUFILE* file);
        bool readGribSection5_ES (ZUFILE* file);

        //---------------------------------------------
        // Utility functions
        //---------------------------------------------
        zuchar readChar(ZUFILE* file);
        int    readSignedInt3(ZUFILE* file);
        int    readSignedInt2(ZUFILE* file);
        zuint  readInt2(ZUFILE* file);
        zuint  readInt3(ZUFILE* file);
        double readFloat4(ZUFILE* file);

        zuint  makeInt3(zuchar a, zuchar b, zuchar c);
        zuint  makeInt2(zuchar b, zuchar c);

//        void   print();
};


#endif



