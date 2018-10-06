/*    WMM Subroutine library was tested in the following environments
 *
 *    1. Red Hat Linux  with GCC Compiler
 *    2. MS Windows XP with CodeGear C++ compiler
 *    3. Sun Solaris with GCC Compiler
 *
 *
 * MODIFICATIONS
 *
 *    Date                 Version
 *    ----                 -----------
 *    Jul 15, 2009         0.1
 *    Nov 15, 2009         0.2
      Nov 23, 2009	   0.3
	Jan 28, 2010	   1.0
	
*/

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif

#define WMMMODELFILE "WMM2010ISO.COF.20101221"
#define READONLYMODE "r"
#define MAXLINELENGTH (1024)
#define NOOFPARAMS (15)
#define MAX_MODELS (30)
#define NOOFCOEFFICIENTS (7)

//#define _HEADER_LIMIT_EXCEEDED (-2)
#define _DEGREE_NOT_FOUND (-2)
#define CALCULATE_NUMTERMS(N)    (N * ( N + 1 ) / 2 + N)

#ifndef WMMHEADER_H
#define WMMHEADER_H

#ifndef M_PI
#define M_PI    ((2)*(acos(0.0)))
#endif

#define RAD2DEG(rad)    ((rad)*(180.0L/M_PI))
#define DEG2RAD(deg)    ((deg)*(M_PI/180.0L))
#define ATanH(x)	    (0.5 * log((1 + x) / (1 - x)))


#ifndef TRUE
#define TRUE            ((int)1)
#endif

#ifndef FALSE
#define FALSE           ((int)0)
#endif

#define WMM_MAX_MODEL_DEGREES	12
#define WMM_MAX_SECULAR_VARIATION_MODEL_DEGREES 12

#define WMM_PS_MIN_LAT_DEGREE  -55 /* Minimum Latitude for  Polar Stereographic projection in degrees   */
#define WMM_PS_MAX_LAT_DEGREE  55  /* Maximum Latitude for Polar Stereographic projection in degrees     */
#define WMM_UTM_MIN_LAT_DEGREE -80.5  /* Minimum Latitude for UTM projection in degrees   */
#define WMM_UTM_MAX_LAT_DEGREE  84.5  /* Maximum Latitude for UTM projection in degrees     */

#define WMM_GEO_POLE_TOLERANCE  1e-5
#define WMM_USE_GEOID	1    /* 1 Geoid - Ellipsoid difference should be corrected, 0 otherwise */

/*
Data types and prototype declaration for
World Magnetic Model (WMM) subroutines.

July 28, 2009

manoj.c.nair@noaa.gov*/


typedef struct {
			double EditionDate;
			double epoch;       //Base time of Geomagnetic model epoch (yrs)
			char  ModelName[20];
			double *Main_Field_Coeff_G;         // C - Gauss coefficients of main geomagnetic model (nT)
			double *Main_Field_Coeff_H;         // C - Gauss coefficients of main geomagnetic model (nT)
			double *Secular_Var_Coeff_G; // CD - Gauss coefficients of secular geomagnetic model (nT/yr)
			double *Secular_Var_Coeff_H; // CD - Gauss coefficients of secular geomagnetic model (nT/yr)
			int nMax; // Maximum degree of spherical harmonic model
			int nMaxSecVar;//Maxumum degree of spherical harmonic secular model
			int SecularVariationUsed; //Whether or not the magnetic secular variation vector will be needed by program
			} WMMtype_MagneticModel;

typedef struct {
			double a; /*semi-major axis of the ellipsoid*/
			double b; /*semi-minor axis of the ellipsoid*/
			double fla; /* flattening */
			double epssq; /*first eccentricity squared */
			double eps; /* first eccentricity */
			double re;/* mean radius of  ellipsoid*/
			} WMMtype_Ellipsoid;

typedef struct {
			double lambda;// longitude
			double phi; // geodetic latitude
			double HeightAboveEllipsoid; // height above the ellipsoid (HaE)
			double HeightAboveGeoid;  /* (height above the EGM96 geoid model ) */
			int UseGeoid;
			} WMMtype_CoordGeodetic;

typedef struct {
			double lambda;/* longitude*/
			double phig;/* geocentric latitude*/
			double r;  /* distance from the center of the ellipsoid*/
			} WMMtype_CoordSpherical;


typedef struct {
			int	Year;
			int	Month;
			int	Day;
			double DecimalYear;     /* decimal years */
			} WMMtype_Date;

typedef struct {
			int WMM_Mercator;
			int	WMM_LambertConformalConic;
			int	WMM_PolarStereographic;
			int	WMM_TransverseMercator;
			} WMMtype_MapProjectionCode ; /*is an integer, with these possible values:*/


typedef struct {
			double *Pcup;  /* Legendre Function */
			double *dPcup; /* Derivative of Lagendre fn */
				} WMMtype_LegendreFunction;

typedef struct {
			double Bx;    /* North */
			double By;	  /* East */
			double Bz;    /* Down */
			} WMMtype_MagneticResults;

typedef struct {

			double RelativeRadiusPower[WMM_MAX_MODEL_DEGREES+1];  /* [earth_reference_radius_km / sph. radius ]^n  */
			double cos_mlambda[WMM_MAX_MODEL_DEGREES+1]; /*cp(m)  - cosine of (m*spherical coord. longitude)*/
			double sin_mlambda[WMM_MAX_MODEL_DEGREES+1]; /* sp(m)  - sine of (m*spherical coord. longitude) */
			}   WMMtype_SphericalHarmonicVariables;

typedef struct {
			double Decl; 	/* 1. Angle between the magnetic field vector and true north, positive east*/
			double Incl; 	/*2. Angle between the magnetic field vector and the horizontal plane, positive down*/
			double F; 		/*3. Magnetic Field Strength*/
			double H; 		/*4. Horizontal Magnetic Field Strength*/
			double X; 		/*5. Northern component of the magnetic field vector*/
			double Y; 		/*6. Eastern component of the magnetic field vector*/
			double Z; 		/*7. Downward component of the magnetic field vector*/
			double GV; 		/*8. The Grid Variation*/
			double Decldot; /*9. Yearly Rate of change in declination*/
			double Incldot; /*10. Yearly Rate of change in inclination*/
			double Fdot; 	/*11. Yearly rate of change in Magnetic field strength*/
			double Hdot; 	/*12. Yearly rate of change in horizontal field strength*/
			double Xdot; 	/*13. Yearly rate of change in the northern component*/
			double Ydot; 	/*14. Yearly rate of change in the eastern component*/
			double Zdot; 	/*15. Yearly rate of change in the downward component*/
			double GVdot;	/*16. Yearly rate of chnage in grid variation*/
			} WMMtype_GeoMagneticElements;
typedef struct {
			int NumbGeoidCols ;   /* 360 degrees of longitude at 15 minute spacing */
			int NumbGeoidRows ;   /* 180 degrees of latitude  at 15 minute spacing */
			int NumbHeaderItems ;    /* min, max lat, min, max long, lat, long spacing*/
			int	ScaleFactor;    /* 4 grid cells per degree at 15 minute spacing  */
			float *GeoidHeightBuffer;
			int NumbGeoidElevs;
			int  Geoid_Initialized ;  /* indicates successful initialization */
			int UseGeoid; /*Is the Geoid being used?*/
			} WMMtype_Geoid;
typedef struct {
			char Longitude[40];
			char Latitude[40];
			} WMMtype_CoordGeodeticStr;

typedef struct {
			double Easting; /* (X) in meters*/
			double Northing;/* (Y) in meters */
			int Zone;/*UTM Zone*/
			char HemiSphere ;
			double CentralMeridian;
			double ConvergenceOfMeridians;
			double PointScale;
			}WMMtype_UTMParameters;

  enum PARAMS
    {
        ISO,
        MODELNAME,
        PUBLISHER,
        RELEASEDATE,
        DATACUTOFF,
        MODELSTARTYEAR,
        MODELENDYEAR,
        EPOCH,
        INTERNALSTATICDEGREE,
        INTERNALSECVARDEGREE,
        EXTERNALSTATICDEGREE,
        EXTERNALSECVARDEGREE,
        GEOMAGREFRAD,
        NORMALIZATION,
        SPATBASFUNC,
    };
    enum COEFFICIENTS
    {
        IE,
        N,
        M,
        GNM,
        HNM,
        DGNM,
        DHNM,
    };

    enum YYYYMMDD
    {
        YEAR,
        MONTH,
        DAY,
    };

/*Prototypes */


	WMMtype_LegendreFunction *WMM_AllocateLegendreFunctionMemory(int NumTerms);

	WMMtype_MagneticModel *WMM_AllocateModelMemory(int NumTerms);

	int WMM_AssociatedLegendreFunction(	WMMtype_CoordSpherical CoordSpherical, int nMax, WMMtype_LegendreFunction *LegendreFunction);

	int WMM_CalculateGeoMagneticElements(WMMtype_MagneticResults *MagneticResultsGeo, WMMtype_GeoMagneticElements *GeoMagneticElements);

	int WMM_CalculateGridVariation(WMMtype_CoordGeodetic location, WMMtype_GeoMagneticElements *elements);

	int WMM_CalculateSecularVariation(WMMtype_MagneticResults MagneticVariation, WMMtype_GeoMagneticElements *MagneticElements);

	int WMM_CheckGeographicPole(WMMtype_CoordGeodetic *CoordGeodetic);

	int WMM_ComputeSphericalHarmonicVariables(	WMMtype_Ellipsoid  Ellip,
							WMMtype_CoordSpherical  CoordSpherical,
							int nMax,
							WMMtype_SphericalHarmonicVariables * SphVariables);

	int WMM_DateToYear(WMMtype_Date *Calendar_Date, char *Error);

	void WMM_DegreeToDMSstring (double DegreesOfArc, int UnitDepth, char *DMSstring);

	void WMM_DMSstringToDegree (char *DMSstring, double *DegreesOfArc);

	void WMM_Error (int control);

	int WMM_FreeMemory(WMMtype_MagneticModel *MagneticModel, WMMtype_MagneticModel *TimedMagneticModel, WMMtype_LegendreFunction *LegendreFunction);

	int WMM_FreeLegendreMemory(WMMtype_LegendreFunction *LegendreFunction);

	int WMM_FreeMagneticModelMemory(WMMtype_MagneticModel *MagneticModel);

	int WMM_GeodeticToSpherical(WMMtype_Ellipsoid Ellip, WMMtype_CoordGeodetic CoordGeodetic, WMMtype_CoordSpherical *CoordSpherical);

	int WMM_Geomag(WMMtype_Ellipsoid Ellip,
					WMMtype_CoordSpherical CoordSpherical,
					WMMtype_CoordGeodetic CoordGeodetic,
					WMMtype_MagneticModel *TimedMagneticModel,
					WMMtype_GeoMagneticElements  *GeoMagneticElements);


	char WMM_GeomagIntroduction(WMMtype_MagneticModel *MagneticModel);
	char EMM_GeomagIntroduction(WMMtype_MagneticModel *MagneticModel);

	int WMM_GetUserGrid(WMMtype_CoordGeodetic *minimum, 
						WMMtype_CoordGeodetic *maximum, 
						double *step_size, 
						double *a_step_size, 
						double *step_time, 
						WMMtype_Date *StartDate, 
						WMMtype_Date *EndDate, 
						int *ElementOption, 
						int *PrintOption, 
						char *OutputFile, 
						WMMtype_Geoid *Geoid);

	int WMM_GetUserInput(WMMtype_MagneticModel *MagneticModel,
							WMMtype_Geoid *Geoid,
							WMMtype_CoordGeodetic *CoordGeodetic,
							WMMtype_Date *MagneticDate);

	int WMM_Grid(WMMtype_CoordGeodetic minimum,
						WMMtype_CoordGeodetic maximum,
						double step_size,
						double altitude_step_size,
						double time_step,
						WMMtype_MagneticModel *MagneticModel,
						WMMtype_Geoid *geoid,
						WMMtype_Ellipsoid Ellip,
						WMMtype_Date StartDate,
						WMMtype_Date EndDate,
						int ElementOption,
						int PrintOption,
						char *OutputFile);

	int WMM_PcupLow( double *Pcup, double *dPcup, double x, int nMax);

	int WMM_PcupHigh( double *Pcup, double *dPcup, double x, int nMax);


	void WMM_PrintUserData(WMMtype_GeoMagneticElements GeomagElements,
								WMMtype_CoordGeodetic SpaceInput,
								WMMtype_Date TimeInput,
								WMMtype_MagneticModel *MagneticModel,
								WMMtype_Geoid *Geoid);

	int WMM_readMagneticModel(char *filename, WMMtype_MagneticModel *MagneticModel);

	int WMM_readMagneticModel_Large(char *filename, char *filenameSV, WMMtype_MagneticModel *MagneticModel);

	int WMM_RotateMagneticVector(WMMtype_CoordSpherical ,
								 WMMtype_CoordGeodetic CoordGeodetic,
								 WMMtype_MagneticResults MagneticResultsSph,
								 WMMtype_MagneticResults *MagneticResultsGeo);

	int WMM_SetDefaults(WMMtype_Ellipsoid *Ellip, WMMtype_MagneticModel *MagneticModel, WMMtype_Geoid *Geoid);

	int WMM_SecVarSummation(WMMtype_LegendreFunction *LegendreFunction,
							WMMtype_MagneticModel *MagneticModel,
							WMMtype_SphericalHarmonicVariables SphVariables,
							WMMtype_CoordSpherical CoordSpherical,
							WMMtype_MagneticResults *MagneticResults);

	int WMM_SecVarSummationSpecial(WMMtype_MagneticModel *MagneticModel,
								WMMtype_SphericalHarmonicVariables SphVariables,
								WMMtype_CoordSpherical CoordSpherical,
								WMMtype_MagneticResults *MagneticResults);

	int WMM_Summation(	WMMtype_LegendreFunction *LegendreFunction,
						WMMtype_MagneticModel *MagneticModel,
						WMMtype_SphericalHarmonicVariables SphVariables,
						WMMtype_CoordSpherical CoordSpherical,
						WMMtype_MagneticResults *MagneticResults);

	int WMM_SummationSpecial(WMMtype_MagneticModel *MagneticModel,
						WMMtype_SphericalHarmonicVariables SphVariables,
						WMMtype_CoordSpherical CoordSpherical,
						WMMtype_MagneticResults *MagneticResults);

	int WMM_TimelyModifyMagneticModel(WMMtype_Date UserDate, WMMtype_MagneticModel *MagneticModel,  WMMtype_MagneticModel *TimedMagneticModel);

	int WMM_ValidateDMSstringlat (char *input, char *Error);

	int WMM_ValidateDMSstringlong (char *input, char *Error);

	int WMM_Warnings(int control, double value, WMMtype_MagneticModel *MagneticModel);
	int WMM_GetTransverseMercator(WMMtype_CoordGeodetic CoordGeodetic, WMMtype_UTMParameters *UTMParameters);

	int  WMM_GetUTMParameters (  double Latitude,
							  double Longitude,
							  int   *Zone,
							  char   *Hemisphere,
							  double *CentralMeridian);

  	void WMM_TMfwd4(double Eps, double Epssq, double K0R4, double K0R4oa,
		 double Acoeff[], double Lam0, double K0, double falseE,
		 double falseN, int XYonly, double Lambda, double Phi,
		 double *X, double *Y, double *pscale, double *CoM);

/*Prototypes for Geoid Functions*/

	int WMM_InitializeGeoid (WMMtype_Geoid *Geoid);
/*
 * The function Initialize_Geoid reads geoid separation data from a file in
 * the current directory and builds the geoid separation table from it.  If an
 * error occurs, the error code is returned, otherwise GEOID_NO_ERROR is
 * returned.
 */

  int WMM_ConvertGeoidToEllipsoidHeight (WMMtype_CoordGeodetic *CoordGeodetic,  WMMtype_Geoid *Geoid);
/*
 * The function Convert_Geoid_To_Ellipsoid_Height converts the specified WGS84
 * geoid height at the specified geodetic coordinates to the equivalent
 * ellipsoid height, using the EGM96 gravity model.
 *
 *    Latitude            : Geodetic latitude in radians           (input)
 *    Longitude           : Geodetic longitude in radians          (input)
 *    Geoid_Height        : Geoid height, in meters                (input)
 *    Ellipsoid_Height    : Ellipsoid height, in meters.           (output)
 *
 */

 int WMM_GetGeoidHeight (double Latitude, double Longitude, 	double *DeltaHeight, WMMtype_Geoid *Geoid);
/*
 * The private function Get_Geoid_Height returns the height of the
 * WGS84 geiod above or below the WGS84 ellipsoid,
 * at the specified geodetic coordinates,
 * using a grid of height adjustments from the EGM96 gravity model.
 *
 *    Latitude            : Geodetic latitude in radians           (input)
 *    Longitude           : Geodetic longitude in radians          (input)
 *    DeltaHeight         : Height Adjustment, in meters.          (output)
 *
 */


int WMM_swab_type();
float WMM_FloatSwap( float f );
char *trim(char *str);
void assignheadervalues(WMMtype_MagneticModel *model, char values[][MAXLINELENGTH]);
int WMM_readMagneticModel_ISO(char *filename, WMMtype_MagneticModel *magneticmodels[],int array_size);

extern char geoiddatapath[1024];


#endif /*WMMHEADER_H*/
