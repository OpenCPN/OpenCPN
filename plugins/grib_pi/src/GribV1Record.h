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
/**
 * \file
 * GRIB Version 1 Record Implementation.
 *
 * Implements record handling for the GRIB1 format, which is the original GRIB
 * specification widely used for weather data distribution. GRIB1 has a simpler
 * structure than GRIB2 but supports fewer parameters and options.
 */

#ifndef GRIBV1RECORD_H
#define GRIBV1RECORD_H

#include <iostream>
#include <cmath>

#include "zuFile.h"
#include "GribRecord.h"

//----------------------------------------------
class GribV1Record : public GribRecord {
public:
  GribV1Record(ZUFILE* file, int id_);
  GribV1Record(const GribRecord& rec);
  GribV1Record() {}

  ~GribV1Record();

protected:
private:
  zuint periodSeconds(zuchar unit, zuchar P1, zuchar P2, zuchar range);
  //-----------------------------------------
  void translateDataType();  // adapte les codes des différents centres météo
  //---------------------------------------------
  // SECTION 0: THE INDICATOR SECTION (IS)
  //---------------------------------------------
  zuint fileOffset0;
  zuint seekStart, totalSize;
  // zuchar editionNumber;
  bool b_len_add_8;

  // SECTION 1: THE PRODUCT DEFINITION SECTION (PDS)
  zuint fileOffset1;
  zuint sectionSize1;
  zuchar tableVersion;
  zuchar data1[28];
  bool hasGDS;
  // bool   hasBMS;
  double decimalFactorD;
  // SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
  zuint fileOffset2;
  zuint sectionSize2;
  // SECTION 3: BIT MAP SECTION (BMS)
  zuint fileOffset3;
  zuint sectionSize3;
  // zuchar *BMSbits;
  // SECTION 4: BINARY DATA SECTION (BDS)
  zuint fileOffset4;
  zuint sectionSize4;
  zuchar unusedBitsEndBDS;
  bool isGridData;  // not spherical harmonics
  bool isSimplePacking;
  bool isFloatValues;
  int scaleFactorE;
  double scaleFactorEpow2;
  double refValue;
  zuint nbBitsInPack;
  // SECTION 5: END SECTION (ES)

  //---------------------------------------------
  // Data Access
  //---------------------------------------------
  /**
   * Reads the Indicator Section containing GRIB header and total record size.
   * @param file Input file stream positioned at record start
   * @param b_skip_initial_GRIB 0=search for "GRIB", 1=first 'G' found, 2=full
   * "GRIB" read
   * @return true if section read successfully, false on error or EOF
   */
  bool readGribSection0_IS(ZUFILE* file, unsigned int b_skip_initial_GRIB);

  /**
   * Reads the Product Definition Section containing metadata about the data.
   * @param file Input file stream positioned at PDS start
   * @return true if section read successfully, false on error
   */
  bool readGribSection1_PDS(ZUFILE* file);

  /**
   * Reads the Grid Description Section defining the geographic grid structure.
   * @param file Input file stream positioned at GDS start
   * @return true if section read successfully, false on error
   */
  bool readGribSection2_GDS(ZUFILE* file);

  /**
   * Reads the optional Bit Map Section indicating valid/invalid grid points.
   * @param file Input file stream positioned at BMS start
   * @return true if section read successfully, false on error
   */
  bool readGribSection3_BMS(ZUFILE* file);

  /**
   * Reads the Binary Data Section containing packed meteorological values.
   * @param file Input file stream positioned at BDS start
   * @return true if section read successfully, false on error
   */
  bool readGribSection4_BDS(ZUFILE* file);

  /**
   * Reads the End Section marking the record termination with "7777".
   * @param file Input file stream positioned at ES start
   * @return true if section read successfully, false on error
   */
  bool readGribSection5_ES(ZUFILE* file);

  //---------------------------------------------
  // Utility functions
  //---------------------------------------------
  zuchar readChar(ZUFILE* file);
  int readSignedInt3(ZUFILE* file);
  int readSignedInt2(ZUFILE* file);
  zuint readInt2(ZUFILE* file);
  zuint readInt3(ZUFILE* file);
  double readFloat4(ZUFILE* file);

  zuint makeInt3(zuchar a, zuchar b, zuchar c);
  zuint makeInt2(zuchar b, zuchar c);

  //        void   print();
};

#endif
