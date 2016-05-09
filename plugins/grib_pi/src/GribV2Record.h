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
Elément de base d'un fichier GRIB V2
******************************************/

#ifndef GRIBV2RECORD_H
#define GRIBV2RECORD_H

#include <iostream>
#include <cmath>

#include "zuFile.h"
#include "GribRecord.h"

class  GRIBMessage;

//----------------------------------------------
class GribV2Record : public GribRecord
{
    public:
        GribV2Record(ZUFILE* file, int id_);
        GribV2Record(const GribRecord &rec);
        GribV2Record() { grib_msg = 0;}

        ~GribV2Record();

    private:
        zuint  periodSeconds(zuchar unit, zuint P1, zuchar P2, zuchar range);
        class  GRIBMessage *grib_msg;
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
       int productTemplate;
       int productDiscipline;
       int gridTemplateNum;
       int dataCat;
       int dataNum;
                                       

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
        bool readGribSection0_IS (ZUFILE* file, bool b_skip_initial_GRIB);

};


#endif



