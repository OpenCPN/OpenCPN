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
 * GRIB Record Base Class Implementation.
 *
 * Defines the base class for GRIB weather data records. A GRIB record
 * represents a single parameter (like wind, pressure, temperature) for one
 * forecast time. This class provides common functionality shared between GRIB1
 * and GRIB2 formats.
 */
#ifndef GRIBRECORD_H
#define GRIBRECORD_H

#include <iostream>
#include <cmath>

#define DEBUG_INFO false
#define DEBUG_ERROR true
#define grib_debug(format, ...)             \
  {                                         \
    if (DEBUG_INFO) {                       \
      fprintf(stderr, format, __VA_ARGS__); \
      fprintf(stderr, "\n");                \
    }                                       \
  }
#define erreur(format, ...)                 \
  {                                         \
    if (DEBUG_ERROR) {                      \
      fprintf(stderr, "Grib ERROR: ");      \
      fprintf(stderr, format, __VA_ARGS__); \
      fprintf(stderr, "\n");                \
    }                                       \
  }

#define zuint unsigned int
#define zuchar unsigned char

#define GRIB_NOTDEF -999999999

//--------------------------------------------------------
// dataTypes    Cf function translateDataType()
//--------------------------------------------------------
#define GRB_PRESSURE 2   /* Pa     */
#define GRB_GEOPOT_HGT 7 /* gpm    */
#define GRB_TEMP 11      /* K      */
#define GRB_TPOT 13      /* K      */
#define GRB_TMAX 15      /* K      */
#define GRB_TMIN 16      /* K      */
#define GRB_DEWPOINT 17  /* K      */

#define GRB_WIND_DIR 31   /* Deg. Wind Direction */
#define GRB_WIND_SPEED 32 /* m/s  Wind Speed     */
#define GRB_WIND_VX 33    /* m/s U  */
#define GRB_WIND_VY 34    /* m/s V  */

#define GRB_CUR_DIR 47   /* Deg. Direction of current  */
#define GRB_CUR_SPEED 48 /* m/s Speed of current       */
#define GRB_UOGRD 49     /*"u-component of current", "m/s" */
#define GRB_VOGRD 50     /*"v-component of current", "m/s" */

#define GRB_HUMID_SPEC 51  /* kg/kg  */
#define GRB_HUMID_REL 52   /* %      */
#define GRB_PRECIP_RATE 59 /* l/m2/s */
#define GRB_PRECIP_TOT 61  /* l/m2   */
#define GRB_SNOW_DEPTH 66  /* m      */
#define GRB_CLOUD_TOT 71   /* %      */
#define GRB_HTSGW 100      /* m      */
#define GRB_WTMP 80        /* "Water Temperature", "K" */
#define GRB_COMP_REFL 212  /* dBZ */

#define GRB_WVDIR 101
#define GRB_WVHGT 102
#define GRB_WVPER 103
#define GRB_SWDIR 104
#define GRB_SWELL 105
#define GRB_SWPER 106
#define GRB_DIRPW 107
#define GRB_PERPW 108
#define GRB_DIRSW 109
#define GRB_PERSW 110
#define GRB_PER 209
#define GRB_DIR 210

#define GRB_CRAIN 140         /* "Categorical rain", "yes=1;no=0" */
#define GRB_FRZRAIN_CATEG 141 /* 1=yes 0=no */
#define GRB_SNOW_CATEG 143    /* 1=yes 0=no */
#define GRB_CAPE 157          /* J/kg   */

#define GRB_TSEC \
  171 /* "Seconds prior to initial reference time (defined in bytes 18-20)" */
#define GRB_WIND_GUST 180    /* m/s "wind gust */
#define GRB_WIND_GUST_VX 181 /* m/s */
#define GRB_WIND_GUST_VY 182 /* m/s */

#define GRB_USCT 190 /* Scatterometer estimated U Wind, NCEP Center 7  */
#define GRB_VSCT 191 /* Scatterometer estimated V Wind, NCEP Center 7  */

#define GRB_WIND_XY2D 250    /* private : GRB_WIND_VX+GRB_WIND_VX */
#define GRB_DIFF_TEMPDEW 251 /* private : GRB_TEMP-GRB_DEWPOINT */

//--------------------------------------------------------
// Levels types (altitude reference)
//--------------------------------------------------------
#define LV_GND_SURF 1
#define LV_ISOTHERM0 4
#define LV_ISOBARIC 100
#define LV_MSL 102
#define LV_ABOV_MSL 103
#define LV_ABOV_GND 105
#define LV_SIGMA 107
#define LV_ATMOS_ENT 10
#define LV_ATMOS_ALL 200
//---------------------------------------------------------
enum DataCenterModel {
  NOAA_GFS,
  NOAA_NCEP_WW3,
  NOAA_NCEP_SST,
  NOAA_RTOFS,
  FNMOC_WW3_GLB,
  FNMOC_WW3_MED,
  NORWAY_METNO,
  ECMWF_ERA5,
  KNMI_HIRLAM,
  KNMI_HARMONIE_AROME,
  OTHER_DATA_CENTER
};

//----------------------------------------------
class GribCode {
public:
  static zuint makeCode(zuchar dataType, zuchar levelType, zuint levelValue) {
    return ((levelValue & 0xFFFF) << 16) + ((levelType & 0xFF) << 8) + dataType;
  }
  static zuchar getDataType(zuint code) { return code & 0xFF; }
  static zuchar getLevelType(zuint code) { return (code >> 8) & 0xFF; }
  static zuint getLevelValue(zuint code) { return (code >> 16) & 0xFFFF; }
};

/**
 * Represents a meteorological data grid from a GRIB (Gridded Binary) file.
 *
 * A GribRecord contains a regular latitude/longitude grid of values
 * representing a single meteorological parameter (e.g., wind component,
 * pressure, temperature) at a specific:
 * - Time (forecast or analysis time)
 * - Vertical level (e.g., surface, pressure level, height above ground)
 * - Parameter type (defined by GRIB edition and originating center)
 *
 * Grid Structure:
 * - Origin point (La1, Lo1) and end point (La2, Lo2)
 * - Number of points in each direction (Ni, Nj)
 * - Grid spacing (Di, Dj)
 * - Data array of size Ni × Nj containing values at each grid point
 *
 * Features:
 * - Provides spatial interpolation for points between grid points.
 * - Handles vector fields (e.g., wind, currents) with special interpolation
 *   for magnitude and direction.
 * - Supports bitmap sections for irregular data coverage.
 * - Can be created from file data or generated through temporal/spatial
 *   interpolation.
 * - Derived quantities like wind speed from U/V components.
 * - Unit conversions and statistical calculations.
 *
 */
class GribRecord {
public:
  /** Copy constructor performs a deep copy of the GribRecord. */
  GribRecord(const GribRecord& rec);
  GribRecord() { m_bfilled = false; }

  virtual ~GribRecord();

  /**
   * Creates a new GribRecord by temporally interpolating between two time
   * points.
   *
   * Performs linear interpolation between two GribRecords that:
   * - Represent the same parameter (e.g., temperature, pressure).
   * - Have the same grid structure.
   * - Exist at different times.
   *
   * The interpolation is done value-by-value across the entire grid using:
   * - Linear interpolation for scalar values
   * - Angular interpolation for directional values (when dir=true)
   *   to properly handle the 0°/360° wrapping
   *
   * @param rec1 GribRecord at earlier time t1
   * @param rec2 GribRecord at later time t2
   * @param d Interpolation factor between [0,1] where:
   *          - 0 means use values from rec1
   *          - 1 means use values from rec2
   *          - 0.5 means halfway between rec1 and rec2
   * @param dir If true, treats values as angles in degrees and uses
   *           proper angular interpolation (e.g., for wind direction)
   *
   * @return New GribRecord containing the interpolated values, or NULL if:
   *         - Input records have different grid structures
   *         - Input records are invalid
   *         - Memory allocation fails
   *
   * @note For vector fields (e.g., wind, currents), use Interpolated2DRecord()
   *       instead to properly handle both components together
   */
  static GribRecord* InterpolatedRecord(const GribRecord& rec1,
                                        const GribRecord& rec2, double d,
                                        bool dir = false);

  /**
   * Creates temporally interpolated records for vector fields (wind, currents).
   *
   * Unlike scalar interpolation, vector fields require both X and Y components
   * to be interpolated together to preserve the vector characteristics. This
   * method:
   * 1. Converts X,Y components to magnitude and direction at each grid point/
   * 2. Interpolates magnitude linearly between times.
   * 3. Interpolates direction using proper angular interpolation.
   * 4. Converts back to X,Y components.
   *
   * @param rety [out] Pointer to store interpolated Y-component record
   * @param rec1x X-component record at earlier time t1 (e.g., wind u-component)
   * @param rec1y Y-component record at earlier time t1 (e.g., wind v-component)
   * @param rec2x X-component record at later time t2
   * @param rec2y Y-component record at later time t2
   * @param d Interpolation factor between [0,1] where:
   *          - 0 returns values from time t1
   *          - 1 returns values from time t2
   *
   * @return Pointer to interpolated X-component record, or NULL if:
   *         - Input records have mismatched grids
   *         - Any input record is invalid
   *         - Memory allocation fails
   */
  static GribRecord* Interpolated2DRecord(GribRecord*& rety,
                                          const GribRecord& rec1x,
                                          const GribRecord& rec1y,
                                          const GribRecord& rec2x,
                                          const GribRecord& rec2y, double d);

  static GribRecord* MagnitudeRecord(const GribRecord& rec1,
                                     const GribRecord& rec2);

  /**
   * Converts wind or current values from polar (direction/speed) to cartesian
   * (U/V) components.
   *
   * U component represents East/West velocity (positive = eastward)
   * V component represents North/South velocity (positive = northward)
   *
   * @param pDIR Direction record in meteorological degrees (0 = N, 90 = E).
   * @param pSPEED Speed record (units preserved).
   *
   * @note Modifies input records: pDIR becomes U component, pSPEED becomes V
   * component.
   */
  static void Polar2UV(GribRecord* pDIR, GribRecord* pSPEED);

  void multiplyAllData(double k);
  void Substract(const GribRecord& rec, bool positive = true);
  void Average(const GribRecord& rec);

  bool isOk() const { return ok; };
  bool isDataKnown() const { return knownData; };
  bool isEof() const { return eof; };
  bool isDuplicated() const { return IsDuplicated; };
  /**
   * Returns the type of meteorological parameter stored in this grid.
   *
   * Standard meteorological parameters are defined as constants, including:
   * - GRB_PRESSURE (2): Atmospheric pressure in Pa
   * - GRB_TEMP (11): Temperature in K
   * - GRB_WIND_VX (33): West-East wind component in m/s
   * - GRB_WIND_VY (34): South-North wind component in m/s
   * - GRB_HUMID_REL (52): Relative humidity in %
   * - GRB_PRECIP_TOT (61): Total precipitation in l/m2
   * - GRB_CLOUD_TOT (71): Total cloud cover in %
   *
   * @return Parameter type identifier as unsigned char
   *
   * @see The full list of parameter codes is defined at the top of GribRecord.h
   * @note Parameter definitions can vary between GRIB1 and GRIB2 formats
   */
  zuchar getDataType() const { return dataType; }
  void setDataType(const zuchar t);

  /**
   * Returns the type of vertical level for this grid's data.
   *
   * Level types define the vertical coordinate system, including:
   * - LV_GND_SURF (1): Ground or water surface
   * - LV_ISOBARIC (100): Constant pressure surface (e.g. 850 hPa)
   * - LV_MSL (102): Mean sea level
   * - LV_ABOV_GND (105): Height above ground
   * - LV_ABOV_MSL (103): Height above mean sea level
   *
   * @return Level type identifier as unsigned char
   *
   * @see levelValue() for the specific value within this level type
   */
  zuchar getLevelType() const { return levelType; }
  /**
   * Returns the numeric value associated with the level type.
   *
   * The meaning of this value depends on getLevelType():
   * - For LV_ISOBARIC: Pressure in hectoPascals (e.g., 850 for 850 hPa)
   * - For LV_ABOV_GND/LV_ABOV_MSL: Height in meters
   * - For LV_GND_SURF/LV_MSL: Value is typically 0
   *
   * @return Level value as unsigned int
   *
   * @see getLevelType() to determine how to interpret this value
   */
  zuint getLevelValue() const { return levelValue; }
  /**
   * Returns the numerical weather prediction model/center that produced this
   * data.
   *
   * Known data centers and models include:
   * - NOAA_GFS: NOAA Global Forecast System
   * - NOAA_NCEP_WW3: NOAA NCEP Wave Watch III model
   * - NOAA_RTOFS: NOAA Real-Time Ocean Forecast System
   * - FNMOC_WW3_GLB: US Navy FNMOC global Wave Watch III
   * - ECMWF_ERA5: European ECMWF ERA5 reanalysis
   * - KNMI_HIRLAM: Netherlands KNMI HIRLAM model
   * - OTHER_DATA_CENTER: Other/unknown sources
   *
   * @return Identifier for the data source model as defined in DataCenterModel
   * enum
   *
   * @note Different centers may use different parameters, units, grid
   * structures, and update frequencies
   */
  zuint getDataCenterModel() const { return dataCenterModel; }
  //-----------------------------------------

  /**
   * Returns the originating center ID as defined by WMO (World Meteorological
   * Organization).
   *
   * Standard center IDs include:
   * - 7: US National Weather Service (NWS)
   * - 34: Japanese Meteorological Agency (JMA)
   * - 58: European Centre for Medium-Range Weather Forecasts (ECMWF)
   * - 59: German Weather Service (DWD)
   * - 85: French Weather Service (Météo-France)
   *
   * @return Center identification code as defined in GRIB Table 0
   */
  zuchar getIdCenter() const { return idCenter; }
  /**
   * Returns the model/process ID within the originating center.
   *
   * Each center can run multiple forecast models or analysis processes.
   * This ID distinguishes between them (e.g., global vs. regional models).
   * The meaning depends on the center ID.
   *
   * @return Model/process identification number specific to the originating
   * center
   * @see getIdCenter()
   */
  zuchar getIdModel() const { return idModel; }
  /**
   * Returns the grid definition template number.
   *
   * Identifies how the grid coordinates are defined, including:
   * - Projection type (latitude/longitude, polar stereographic, etc.)
   * - Grid point layout and scanning mode
   * - Earth shape assumptions
   *
   * @return Grid definition identifier as defined in GRIB specifications
   */
  zuchar getIdGrid() const { return idGrid; }

  //-----------------------------------------
  std::string getKey() const { return dataKey; }
  static std::string makeKey(int dataType, int levelType, int levelValue);

  //-----------------------------------------
  /**
   * Returns the start of the period (P1) used for this record.
   *
   * The meaning depends on the time range indicator (getTimeRange()):
   * - For forecasts: Hours after reference time.
   * - For accumulations: Start of accumulation period.
   * - For averages: Start of averaging period.
   *
   * @return Period start indicator in units determined by time range.
   * @see getTimeRange()
   */
  int getPeriodP1() const { return periodP1; }
  /**
   * Returns the end of the period (P2) used for this record.
   *
   * The meaning depends on the time range indicator (getTimeRange()):
   * - For forecasts: Usually same as P1
   * - For accumulations: End of accumulation period
   * - For averages: End of averaging period
   *
   * @return Period end indicator in units determined by time range
   * @see getTimeRange()
   */
  int getPeriodP2() const { return periodP2; }
  /**
   * Returns the forecast period in seconds from reference time.
   *
   * This is the time offset from the model reference time (analysis time)
   * when this data is valid.
   *
   * @return Time offset in seconds from reference time.
   * @see getRecordRefDate()
   */
  zuint getPeriodSec() const { return periodsec; }
  /**
   * Returns the time range indicator that defines how P1 and P2 should be
   * interpreted.
   *
   * Common values include:
   * - 0: Product valid at reference time + P1
   * - 1: Product valid for reference time + P1 to reference time + P2
   * - 2: Product averaged from reference time + P1 to reference time + P2
   * - 4: Product accumulated from reference time + P1 to reference time + P2
   *
   * @return Time range indicator as defined in GRIB specifications
   */
  zuchar getTimeRange() const { return timeRange; }

  // Number of points in the grid
  /**
   * Returns the number of points in the longitude (i) direction of the grid.
   *
   * @return Number of grid points along longitude
   */
  int getNi() const { return Ni; }
  /**
   * Returns the number of points in the latitude (j) direction of the grid.
   *
   * @return Number of grid points along latitude
   */
  int getNj() const { return Nj; }
  /**
   * Returns the grid spacing in longitude (i) direction in degrees.
   *
   * @return Grid spacing in degrees longitude
   */
  double getDi() const { return Di; }
  /**
   * Returns the grid spacing in latitude (j) direction in degrees.
   *
   * @return Grid spacing in degrees latitude
   * @note Can be negative if grid runs from north to south
   */
  double getDj() const { return Dj; }

  /**
   * Returns the data value at a specific grid point.
   *
   * The grid is stored as a one-dimensional array in row-major order
   * (longitude varies fastest).
   *
   * @param i Index in longitude direction (0 to Ni-1)
   * @param j Index in latitude direction (0 to Nj-1)
   * @return Data value at grid point (i,j)
   * @note No bounds checking is performed
   */
  double getValue(int i, int j) const { return data[j * Ni + i]; }

  void setValue(zuint i, zuint j, double v) {
    if (i < Ni && j < Nj) data[j * Ni + i] = v;
  }

  /**
   * Get spatially interpolated value at exact lat/lon position.
   *
   * This method performs specialized vector interpolation for meteorological
   * vector fields like wind or ocean currents.
   *
   * @param px Longitude in degrees.
   * @param py Latitude in degrees.
   * @param numericalInterpolation Use bilinear interpolation if true.
   * @param dir Handle directional interpolation if true (e.g. for wind
   * direction).
   * @return Spatially interpolated value or GRIB_NOTDEF if outside grid.
   */
  double getInterpolatedValue(double px, double py,
                              bool numericalInterpolation = true,
                              bool dir = false) const;

  /**
   * Gets spatially interpolated wind or current vector values at a specific
   * latitude/longitude point.
   *
   * Takes X and Y component records and interpolates both magnitude and angle
   * using bilinear interpolation between grid points. It handles cases where
   * the requested point might cross the date line by adjusting the longitude if
   * needed.
   *
   * @param M [out] Vector magnitude at the interpolated point (preserves input
   * units). This is the speed or strength of the wind/current, typically
   * measured in meters per second (m/s).
   * @param A [out] Vector direction in meteorological degrees at the
   * interpolated point. This is the direction the wind/current is coming FROM,
   * measured in meteorological degrees (0 = North, 90 = East).
   * @param GRX X-component record of the vector field (u-component, West-East)
   * @param GRY Y-component record of the vector field (v-component,
   * South-North)
   * @param px [in] Longitude in degrees of the interpolation point.
   * @param py [in] Latitude in degrees of the interpolation point.
   * @param numericalInterpolation If true, uses bilinear interpolation; if
   * false, uses nearest neighbor interpolation.
   * @return true if interpolation was successful, false if the point is outside
   * map boundaries or if insufficient valid data points exist for interpolation
   *
   * @note The method expects the input components to follow meteorological
   * conventions where u is positive eastward and v is positive northward
   */
  static bool getInterpolatedValues(double& M, double& A, const GribRecord* GRX,
                                    const GribRecord* GRY, double px, double py,
                                    bool numericalInterpolation = true);

  /**
   * Converts grid index i to longitude in degrees.
   *
   * Uses the grid's origin (Lo1) and spacing (Di) to convert from
   * grid index to geographic coordinate.
   *
   * @param i Grid index in longitude direction (0 to Ni-1)
   * @return Longitude in degrees
   */
  inline double getX(int i) const { return Lo1 + i * Di; }
  /**
   * Converts grid index j to latitude in degrees.
   *
   * Uses the grid's origin (La1) and spacing (Dj) to convert from
   * grid index to geographic coordinate.
   *
   * @param j Grid index in latitude direction (0 to Nj-1)
   * @return Latitude in degrees
   * @note Returns decreasing latitudes when Dj is negative
   */
  inline double getY(int j) const { return La1 + j * Dj; }
  /**
   * Converts grid indices to longitude/latitude coordinates.
   *
   * Convenience method that performs both X and Y conversions.
   *
   * @param i Grid index in longitude direction
   * @param j Grid index in latitude direction
   * @param[out] x Pointer to store longitude in degrees
   * @param[out] y Pointer to store latitude in degrees
   */
  void getXY(int i, int j, double* x, double* y) const {
    *x = getX(i);
    *y = getY(j);
  };

  double getLatMin() const { return latMin; }
  double getLonMin() const { return lonMin; }
  double getLatMax() const { return latMax; }
  double getLonMax() const { return lonMax; }

  // Is there a value at a particular grid point ?
  inline bool hasValue(int i, int j) const;
  // Is there a value that is not GRIB_NOTDEF ?
  inline bool isDefined(int i, int j) const {
    return hasValue(i, j) && getValue(i, j) != GRIB_NOTDEF;
  }

  // Reference date Date (file creation date)
  time_t getRecordRefDate() const { return refDate; }
  const char* getStrRecordRefDate() const { return strRefDate; }

  // Date courante des prévisions
  time_t getRecordCurrentDate() const { return curDate; }
  const char* getStrRecordCurDate() const { return strCurDate; }
  void setRecordCurrentDate(time_t t);
  void print();
  bool isFilled() { return m_bfilled; }
  void setFilled(bool val = true) { m_bfilled = val; }

private:
  // Is a point within the extent of the grid?
  inline bool isPointInMap(double x, double y) const;
  inline bool isXInMap(double x) const;
  inline bool isYInMap(double y) const;

protected:
  // private:
  static bool GetInterpolatedParameters(const GribRecord& rec1,
                                        const GribRecord& rec2, double& La1,
                                        double& Lo1, double& La2, double& Lo2,
                                        double& Di, double& Dj, int& im1,
                                        int& jm1, int& im2, int& jm2, int& Ni,
                                        int& Nj, int& rec1offi, int& rec1offj,
                                        int& rec2offi, int& rec2offj);

  /**
   * Unique identifier for this record.
   *
   * Set during file reading for original records.
   * Monotonically increasing for records read from file.
   */
  int id;
  /**
   * Indicates record validity.
   *
   * A record may be invalid if source records have mismatched dimensions,
   * missing data arrays, failed interpolation, or the source records themselves
   * are invalid when creating derived records.
   */
  bool ok;
  /**
   * Indicates whether the data type in this record is recognized by the parser.
   * Used to skip unknown data types during processing.
   */
  bool knownData;
  /**
   * Differentiates wave-related parameters (height, direction, period) from
   * other meteorological data for specialized processing.
   */
  bool waveData;
  /**
   * Indicates if this record was created through copying rather than direct
   * reading.
   *
   * Tracks whether this record was copied to maintain data continuity. This
   * happens with wave data gaps or initial values for cumulative parameters.
   */
  bool IsDuplicated;
  /**
   * Signals when the end of the GRIB file has been reached during parsing.
   */
  bool eof;
  /**
   * Unique string identifier constructed from data type, level type, and level
   * value. Used for record lookup and comparison.
   */
  std::string dataKey;
  char strRefDate[32];
  char strCurDate[32];
  /**
   * Identifies the numerical weather model that produced this data.
   *
   * Standard WMO values from GRIB2 Table C-11 include:
   * - 7: US National Weather Service, National Centers for Environmental
   * Prediction (NCEP)
   * - 34: Japanese Meteorological Agency - Tokyo (JMA)
   * - 58: European Centre for Medium-Range Weather Forecasts (ECMWF)
   * - 59: DWD (German Weather Service)
   * - 85: French Weather Service (Meteo France)
   * - 251: Norwegian Meteorological Institute
   */
  int dataCenterModel;
  /**
   * Indicates whether the data array has been populated. Used to track
   * partial loading states during record construction.
   */
  bool m_bfilled;

  //---------------------------------------------
  // SECTION 0: THE INDICATOR SECTION (IS)
  //---------------------------------------------
  /**
   * GRIB edition number, indicating the version of the GRIB specification used.
   * Determines how subsequent sections should be parsed.
   */
  zuchar editionNumber;

  // SECTION 1: THE PRODUCT DEFINITION SECTION (PDS)
  /**
   * Originating center ID as defined by WMO common table C-1.
   * Identifies which meteorological center generated the forecast.
   */
  zuchar idCenter;
  /**
   * Model identifier within the originating center.
   * Distinguishes between different forecast models run by the same center.
   */
  zuchar idModel;
  /**
   * Grid identifier used by the originating center.
   * Specifies the coordinate system and projection of the data grid.
   */
  zuchar idGrid;
  /**
   * Parameter identifier as defined by GRIB tables.
   * Specifies what physical quantity is represented (wind, temperature, etc).
   */
  zuchar dataType;  // octet 9 = parameters and units
  /**
   * Vertical level type indicator.
   * Specifies the type of vertical coordinate (pressure level, height above
   * ground, etc).
   */
  zuchar levelType;
  /**
   * Numeric value associated with levelType.
   * For example, the specific pressure level in hectopascals.
   */
  zuint levelValue;

  /**
   * Indicates presence of a bitmap section.
   * When true, the data section uses a bitmap to indicate valid data points.
   */
  bool hasBMS;
  /**
   * Components of the reference time for this forecast.
   * Specifies when the forecast model was initialized.
   */
  zuint refyear, refmonth, refday, refhour, refminute;
  /**
   * Time range indicators for this forecast step.
   * Used to calculate the valid time period for this data.
   */
  zuint periodP1, periodP2;
  /**
   * Statistical processing indicator.
   * Describes how the data was processed over time (e.g., accumulation,
   * average).
   */
  zuchar timeRange;
  /**
   * Forecast period in seconds.
   * Time offset from the reference time.
   */
  zuint periodsec;
  /**
   * Unix timestamp of model initialization time.
   */
  time_t refDate;
  /**
   * Unix timestamp of when this forecast is valid.
   */
  time_t curDate;
  // SECTION 2: THE GRID DESCRIPTION SECTION (GDS)
  zuchar NV, PV;
  zuchar gridType;
  zuint Ni, Nj;
  double La1, Lo1;  ///< Grid origin coordinates
  double La2, Lo2;  ///< Grid end coordinates
  double latMin, lonMin, latMax, lonMax;
  double Di, Dj;
  zuchar resolFlags, scanFlags;
  bool hasDiDj;
  bool isEarthSpheric;
  bool isUeastVnorth;
  bool isScanIpositive;
  bool isScanJpositive;
  bool isAdjacentI;
  // SECTION 3: BIT MAP SECTION (BMS)
  zuint BMSsize;
  zuchar* BMSbits;
  // SECTION 4: BINARY DATA SECTION (BDS)
  double* data;
  // SECTION 5: END SECTION (ES)

  time_t makeDate(zuint year, zuint month, zuint day, zuint hour, zuint min,
                  zuint sec);

  //        void   print();
};

//==========================================================================
inline bool GribRecord::hasValue(int i, int j) const {
  // is data present in BMS ?
  if (!hasBMS) {
    return true;
  }
  int bit;
  if (isAdjacentI) {
    bit = j * Ni + i;
  } else {
    bit = i * Nj + j;
  }
  zuchar c = BMSbits[bit / 8];
  zuchar m = (zuchar)128 >> (bit % 8);
  return (m & c) != 0;
}

//-----------------------------------------------------------------
inline bool GribRecord::isPointInMap(double x, double y) const {
  return isXInMap(x) && isYInMap(y);
}
//-----------------------------------------------------------------
inline bool GribRecord::isXInMap(double x) const {
  if (Di > 0) {
    double maxLo = Lo2;
    if (Lo2 + Di >= 360) /* grib that covers the whole world */
      maxLo += Di;
    return x >= Lo1 && x <= maxLo;
  } else {
    double maxLo = Lo1;
    if (Lo2 + Di >= 360) /* grib that covers the whole world */
      maxLo += Di;
    return x >= Lo2 && x <= maxLo;
  }
}
//-----------------------------------------------------------------
inline bool GribRecord::isYInMap(double y) const {
  if (Dj < 0)
    return y <= La1 && y >= La2;
  else
    return y >= La1 && y <= La2;
}

#endif
