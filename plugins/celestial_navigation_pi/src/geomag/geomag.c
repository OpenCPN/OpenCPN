/* this program has been modified by Sean D'Epagnier
   to make it work like a library */

/****************************************************************************/
/*                                                                          */
/*     NGDC's Geomagnetic Field Modeling software for the IGRF and WMM      */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Disclaimer: This program has undergone limited testing. It is        */
/*     being distributed unoffically. The National Geophysical Data         */
/*     Center does not guarantee it's correctness.                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 7.0:                                                         */
/*     - input file format changed to                                       */
/*            -- accept new DGRF2005 coeffs with 0.01 nT precision          */
/*            -- make sure all values are separated by blanks               */
/*            -- swapped n and m: first is degree, second is order          */
/*     - new my_isnan function improves portablility                        */
/*     - corrected feet to km conversion factor                             */
/*     - fixed date conversion errors for yyyy,mm,dd format                 */
/*     - fixed lon/lat conversion errors for deg,min,sec format             */
/*     - simplified leap year identification                                */
/*     - changed comment: units of ddot and idot are arc-min/yr             */
/*     - added note that this program computes the secular variation as     */
/*            the 1-year difference, rather than the instantaneous change,  */
/*            which can be slightly different                               */
/*     - clarified that height is above ellipsoid, not above mean sea level */
/*            although the difference is negligible for magnetics           */
/*     - changed main(argv,argc) to usual definition main(argc,argv)        */
/*     - corrected rounding of angles close to 60 minutes                   */
/*     Thanks to all who provided bug reports and suggested fixes           */
/*                                                                          */
/*                                          Stefan Maus Jan-25-2010         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 6.1:                                                         */
/*     Included option to read coordinates from a file and output the       */
/*     results to a new file, repeating the input and adding columns        */
/*     for the output                                                       */
/*                                          Stefan Maus Jan-31-2008         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Version 6.0:                                                         */
/*     Bug fixes for the interpolation between models. Also added warnings  */
/*     for declination at low H and corrected behaviour at geogr. poles.    */
/*     Placed print-out commands into separate routines to facilitate       */
/*     fine-tuning of the tables                                            */
/*                                          Stefan Maus Aug-24-2004         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*      This program calculates the geomagnetic field values from           */
/*      a spherical harmonic model.  Inputs required by the user are:       */
/*      a spherical harmonic model data file, coordinate preference,        */
/*      altitude, date/range-step, latitude, and longitude.                 */
/*                                                                          */
/*         Spherical Harmonic                                               */
/*         Model Data File       :  Name of the data file containing the    */
/*                                  spherical harmonic coefficients of      */
/*                                  the chosen model.  The model and path   */
/*                                  must be less than PATH chars.           */
/*                                                                          */
/*         Coordinate Preference :  Geodetic (WGS84 latitude and altitude   */
/*                                  above ellipsoid (WGS84),                */
/*                                  or geocentric (spherical, altitude      */
/*                                  measured from the center of the Earth). */
/*                                                                          */
/*         Altitude              :  Altitude above ellipsoid (WGS84). The   */
/*                                  program asks for altitude above mean    */
/*                                  sea level, because the altitude above   */
/*                                  ellipsoid is not known to most users.   */
/*                                  The resulting error is very small and   */
/*                                  negligible for most practical purposes. */
/*                                  If geocentric coordinate preference is  */
/*                                  used, then the altitude must be in the  */
/*                                  range of 6370.20 km - 6971.20 km as     */
/*                                  measured from the center of the earth.  */
/*                                  Enter altitude in kilometers, meters,   */
/*                                  or feet                                 */
/*                                                                          */
/*         Date                  :  Date, in decimal years, for which to    */
/*                                  calculate the values of the magnetic    */
/*                                  field.  The date must be within the     */
/*                                  limits of the model chosen.             */
/*                                                                          */
/*         Latitude              :  Entered in decimal degrees in the       */
/*                                  form xxx.xxx.  Positive for northern    */
/*                                  hemisphere, negative for the southern   */
/*                                  hemisphere.                             */
/*                                                                          */
/*         Longitude             :  Entered in decimal degrees in the       */
/*                                  form xxx.xxx.  Positive for eastern     */
/*                                  hemisphere, negative for the western    */
/*                                  hemisphere.                             */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*      Subroutines called :  degrees_to_decimal,julday,getshc,interpsh,    */
/*                            extrapsh,shval3,dihf,safegets                 */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>            
#include <string.h>
#include <ctype.h>
#include <math.h> 

int my_isnan(double d)
{
  return (d != d);              /* IEEE: only NaN is not equal to itself */
}

#define NaN log(-1.0)
#define FT2KM (1.0/0.0003048)
#define PI 3.141592654
#define RAD2DEG (180.0/PI)

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define IEXT 0
#define FALSE 0
#define TRUE 1                  /* constants */
#define RECL 81

#define MAXINBUFF RECL+14

/** Max size of in buffer **/

#define MAXREAD MAXINBUFF-2
/** Max to read 2 less than total size (just to be safe) **/

#define MAXMOD 30
/** Max number of models in a file **/

#define PATH MAXREAD
/** Max path and filename length **/

#define EXT_COEFF1 (double)0
#define EXT_COEFF2 (double)0
#define EXT_COEFF3 (double)0

#define MAXDEG 13
#define MAXCOEFF (MAXDEG*(MAXDEG+2)+1) /* index starts with 1!, (from old Fortran?) */
double gh1[MAXCOEFF];
double gh2[MAXCOEFF];
double gha[MAXCOEFF];              /* Geomag global variables */
double ghb[MAXCOEFF];
double d=0,f=0,h=0,i=0;
double dtemp,ftemp,htemp,itemp;
double x=0,y=0,z=0;
double xtemp,ytemp,ztemp;

FILE *stream = NULL;                /* Pointer to specified model data file */


  /*  Subroutines used  */
  
  void print_dashed_line();
  void print_long_dashed_line(void);
  void print_header();
  void print_result(double date, double d, double i, double h, double x, double y, double z, double f);
  void print_header_sv();
  void print_result_sv(double date, double ddot, double idot, double hdot, double xdot, double ydot, double zdot, double fdot);
  void print_result_file(FILE *outf, double d, double i, double h, double x, double y, double z, double f,
                         double ddot, double idot, double hdot, double xdot, double ydot, double zdot, double fdot);
  double degrees_to_decimal();
  double julday();
  int   interpsh();
  int   extrapsh();
  int   shval3();
  int   dihf();
  int   safegets(char *buffer,int n);
  int getshc();

/****************************************************************************/
/*                                                                          */
/*                             Program Geomag                               */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*      This program, originally written in FORTRAN, was developed using    */
/*      subroutines written by                                              */
/*      A. Zunde                                                            */
/*      USGS, MS 964, Box 25046 Federal Center, Denver, Co.  80225          */
/*      and                                                                 */
/*      S.R.C. Malin & D.R. Barraclough                                     */
/*      Institute of Geological Sciences, United Kingdom.                   */
/*                                                                          */
/*      Translated                                                          */
/*      into C by    : Craig H. Shaffer                                     */
/*                     29 July, 1988                                        */
/*                                                                          */
/*      Rewritten by : David Owens                                          */
/*                     For Susan McLean                                     */
/*                                                                          */
/*      Maintained by: Stefan Maus                                          */
/*      Contact      : stefan.maus@noaa.gov                                 */
/*                     National Geophysical Data Center                     */
/*                     World Data Center-A for Solid Earth Geophysics       */
/*                     NOAA, E/GC1, 325 Broadway,                           */
/*                     Boulder, CO  80303                                   */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*      Some variables used in this program                                 */
/*                                                                          */
/*    Name         Type                    Usage                            */
/* ------------------------------------------------------------------------ */
/*                                                                          */
/*   a2,b2      Scalar Double          Squares of semi-major and semi-minor */
/*                                     axes of the reference spheroid used  */
/*                                     for transforming between geodetic or */
/*                                     geocentric coordinates.              */
/*                                                                          */
/*   minalt     Double array of MAXMOD Minimum height of model.             */
/*                                                                          */
/*   altmin     Double                 Minimum height of selected model.    */
/*                                                                          */
/*   altmax     Double array of MAXMOD Maximum height of model.             */
/*                                                                          */
/*   maxalt     Double                 Maximum height of selected model.    */
/*                                                                          */
/*   d          Scalar Double          Declination of the field from the    */
/*                                     geographic north (deg).              */
/*                                                                          */
/*   sdate      Scalar Double          start date inputted                  */
/*                                                                          */
/*   ddot       Scalar Double          annual rate of change of decl.       */
/*                                     (arc-min/yr)                         */
/*                                                                          */
/*   alt        Scalar Double          altitude above WGS84 Ellipsoid       */
/*                                                                          */
/*   epoch      Double array of MAXMOD epoch of model.                      */
/*                                                                          */
/*   ext        Scalar Double          Three 1st-degree external coeff.     */
/*                                                                          */
/*   latitude   Scalar Double          Latitude.                            */
/*                                                                          */
/*   longitude  Scalar Double          Longitude.                           */
/*                                                                          */
/*   gh1        Double array           Schmidt quasi-normal internal        */
/*                                     spherical harmonic coeff.            */
/*                                                                          */
/*   gh2        Double array           Schmidt quasi-normal internal        */
/*                                     spherical harmonic coeff.            */
/*                                                                          */
/*   gha        Double array           Coefficients of resulting model.     */
/*                                                                          */
/*   ghb        Double array           Coefficients of rate of change model.*/
/*                                                                          */
/*   i          Scalar Double          Inclination (deg).                   */
/*                                                                          */
/*   idot       Scalar Double          Rate of change of i (arc-min/yr).    */
/*                                                                          */
/*   igdgc      Integer                Flag for geodetic or geocentric      */
/*                                     coordinate choice.                   */
/*                                                                          */
/*   inbuff     Char a of MAXINBUF     Input buffer.                        */
/*                                                                          */
/*   irec_pos   Integer array of MAXMOD Record counter for header           */
/*                                                                          */
/*   stream  Integer                   File handles for an opened file.     */
/*                                                                          */
/*   fileline   Integer                Current line in file (for errors)    */
/*                                                                          */
/*   max1       Integer array of MAXMOD Main field coefficient.             */
/*                                                                          */
/*   max2       Integer array of MAXMOD Secular variation coefficient.      */
/*                                                                          */
/*   max3       Integer array of MAXMOD Acceleration coefficient.           */
/*                                                                          */
/*   mdfile     Character array of PATH  Model file name.                   */
/*                                                                          */
/*   minyr      Double                  Min year of all models              */
/*                                                                          */
/*   maxyr      Double                  Max year of all models              */
/*                                                                          */
/*   yrmax      Double array of MAXMOD  Max year of model.                  */
/*                                                                          */
/*   yrmin      Double array of MAXMOD  Min year of model.                  */
/*                                                                          */
/****************************************************************************/

  int   igdgc=3;
  int   modelI;             /* Which model (Index) */
  int   nmodel;             /* Number of models in file */

  long  irec_pos[MAXMOD];
  double minyr;
  double maxyr;
  double yrmin[MAXMOD];
  double yrmax[MAXMOD];
  double altmin[MAXMOD];
  double altmax[MAXMOD];

  int   max1[MAXMOD];
  int   max2[MAXMOD];
  int   max3[MAXMOD];

  char  model[MAXMOD][9];
  double epoch[MAXMOD];

int geomag_load(const char *mdfile)
{
  char  inbuff[MAXINBUFF];
  int   fileline;

  /* Initializations. */
  
  inbuff[MAXREAD+1]='\0';  /* Just to protect mem. */
  inbuff[MAXINBUFF-1]='\0';  /* Just to protect mem. */
  
  igdgc=1; /* hard coded to geodetic */
  
  stream=fopen(mdfile, "rt");

  if(stream == NULL) {
     printf("Could not open model file %s\n", mdfile);
     return -1;
  }
      
  rewind(stream);
          
  fileline = 0;                            /* First line will be 1 */
  modelI = -1;                             /* First model will be 0 */
  while (fgets(inbuff,MAXREAD,stream))     /* While not end of file
                                            * read to end of line or buffer */
  {
     fileline++;                           /* On new line */
              
     if (strlen(inbuff) != RECL)       /* IF incorrect record size */
     {
        printf("Corrupt record in file %s on line %d.\n", mdfile, fileline);
        fclose(stream);
        exit(5);
     }
              
     /* New statement Dec 1999 changed by wmd  required by year 2000 models */
     if (!strncmp(inbuff,"   ",3))         /* If 1st 3 chars are spaces */
     {
        modelI++;                           /* New model */
                  
        if (modelI > MAXMOD)                /* If too many headers */
        {
           printf("Too many models in file %s on line %d.", mdfile, fileline);
           fclose(stream);
           exit(6);
        }
                  
        irec_pos[modelI]=ftell(stream);
        /* Get fields from buffer into individual vars.  */
        sscanf(inbuff, "%s%lg%d%d%d%lg%lg%lg%lg", model[modelI], &epoch[modelI],
               &max1[modelI], &max2[modelI], &max3[modelI], &yrmin[modelI],
               &yrmax[modelI], &altmin[modelI], &altmax[modelI]);
                  
        /* Compute date range for all models */
        if (modelI == 0)                    /*If first model */
        {
           minyr=yrmin[0];
           maxyr=yrmax[0];
        } 
        else 
        {
           if (yrmin[modelI]<minyr)
           {
              minyr=yrmin[modelI];
           }
           if (yrmax[modelI]>maxyr){
              maxyr=yrmax[modelI];
           }
        } /* if modelI != 0 */
        
     } /* If 1st 3 chars are spaces */
     
  } /* While not end of model file */
  fclose(stream);
  
  nmodel = modelI + 1;

      /** This will compute everything needed for 1 point in time. **/
      for (modelI=0; modelI<nmodel; modelI++)
      if (max2[modelI] == 0) 
        {
          getshc(mdfile, 1, irec_pos[modelI], max1[modelI], 1);
          getshc(mdfile, 1, irec_pos[modelI+1], max1[modelI+1], 2);
        } 
      else 
        {
          getshc(mdfile, 1, irec_pos[modelI], max1[modelI], 1);
          getshc(mdfile, 0, irec_pos[modelI], max2[modelI], 2);
        }

      return 0;
}

/* modelfilename is the name of the file with coefficients to use (eg IGRF11.COF)
   latitude and longitude in degrees, alt in km
   day month and year may be given, or a decimal year

   results gives DIHXYZF and respective rate of change for each in nT */


int geomag_calc(double latitude, double longitude, double alt,
                int day, int month, double year,
                double results[14])
{
  /*  Variable declaration  */
  int   nmax;

  double minalt;
  double maxalt;
  double sdate=-1;
  double ddot;
  double fdot;
  double hdot;
  double idot;
  double xdot;
  double ydot;
  double zdot;
  
  if(month < 1 || month > 12) {
     printf("invalid month (%d) to geomag, must be from 1-12\n", month);
     return -1;
  }
          
  /* if date specified in command line then warn if past end of validity */
  sdate = julday(month,day,(int)floor(year)) + fmod(year, 1);

  if ((sdate>maxyr)&&(sdate<maxyr+1))
  {
     printf("\nWarning: The date %4.2f is out of range,\n", sdate);
     printf("         but still within one year of model expiration date.\n");
     printf("         An updated model file is available before 1.1.%4.0f\n",maxyr);
  }
          
      /* Pick model */
      for (modelI=0; modelI<nmodel; modelI++)
        if (sdate<yrmax[modelI]) break;
      if (modelI == nmodel) modelI--;           /* if beyond end of last model use last model */
      
      /* Get altitude min and max for selected model. */
      minalt=altmin[modelI];
      maxalt=altmax[modelI];
      
      /* Get Coordinate prefs */
      
      /* If needed modify ranges to reflect coords. */
      if (igdgc==2)
        {
          minalt+=6371.2;  /* Add radius to ranges. */
          maxalt+=6371.2;
        }
      
      if ((latitude<-90)||(latitude>90))
      {
         printf("\nThe latitude %3.2f is out of range", latitude);
      }
      
      if ((longitude<-180)||(longitude>180))
      {
         printf("\nThe longitude %3.2f is out of range", longitude);
      }

      if (max2[modelI] == 0) 
        {
          nmax = interpsh(sdate, yrmin[modelI], max1[modelI],
                          yrmin[modelI+1], max1[modelI+1], 3);
          nmax = interpsh(sdate+1, yrmin[modelI] , max1[modelI],
                          yrmin[modelI+1], max1[modelI+1],4);
        } 
      else 
        {
          nmax = extrapsh(sdate, epoch[modelI], max1[modelI], max2[modelI], 3);
          nmax = extrapsh(sdate+1, epoch[modelI], max1[modelI], max2[modelI], 4);
        }

      
      /* Do the first calculations */
      shval3(igdgc, latitude, longitude, alt, nmax, 3,
             IEXT, EXT_COEFF1, EXT_COEFF2, EXT_COEFF3);
      dihf(3);
      shval3(igdgc, latitude, longitude, alt, nmax, 4,
             IEXT, EXT_COEFF1, EXT_COEFF2, EXT_COEFF3);
      dihf(4);
      
      ddot = ((dtemp - d)*RAD2DEG);
      if (ddot > 180.0) ddot -= 360.0;
      if (ddot <= -180.0) ddot += 360.0;
      ddot *= 60.0;
      
      idot = ((itemp - i)*RAD2DEG)*60;
      d = d*(RAD2DEG);   i = i*(RAD2DEG);
      hdot = htemp - h;   xdot = xtemp - x;
      ydot = ytemp - y;   zdot = ztemp - z;
      fdot = ftemp - f;
      
      /* deal with geographic and magnetic poles */
      
      if (h < 100.0) /* at magnetic poles */
        {
          d = NaN;
          ddot = NaN;
          /* while rest is ok */
        }
      
      if (90.0-fabs(latitude) <= 0.001) /* at geographic poles */
        {
          x = NaN;
          y = NaN;
          d = NaN;
          xdot = NaN;
          ydot = NaN;
          ddot = NaN;
          /* while rest is ok */
        }
      
      results[0] = d;
      results[1] = i;
      results[2] = h;
      results[3] = x;
      results[4] = y;
      results[5] = z;
      results[6] = f;
      results[7] = ddot;
      results[8] = idot;
      results[9] = hdot;
      results[10] = xdot;
      results[11] = ydot;
      results[12] = zdot;
      results[13] = fdot;

  return 0;
}


/****************************************************************************/
/*                                                                          */
/*                       Subroutine safegets                                */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*  Gets characters from stdin untill it has reached n characters or \n,    */
/*     whichever comes first.  \n is converted to \0.                       */
/*                                                                          */
/*  Input: n - Integer number of chars                                      */
/*         *buffer - Character array ptr which can contain n+1 characters   */
/*                                                                          */
/*  Output: size - integer size of sting in buffer                          */
/*                                                                          */
/*  Note: All strings will be null terminated.                              */
/*                                                                          */
/*  By: David Owens                                                         */
/*      dio@ngdc.noaa.gov                                                   */
/****************************************************************************/

int safegets(char *buffer,int n){
  char *ptr;                    /** ptr used for finding '\n' **/
  
  buffer[0] = '\0';
  fgets(buffer,n,stdin);        /** Get n chars **/
  buffer[n+1]='\0';             /** Set last char to null **/
  ptr=strchr(buffer,'\n');      /** If string contains '\n' **/
  if (ptr!=NULL){                /** If string contains '\n' **/
    ptr[0]='\0';               /** Change char to '\0' **/
    if (buffer[0] == '\0') printf("\n ... no entry ...\n");
  }
  
  return strlen(buffer);        /** Return the length **/
}


/****************************************************************************/
/*                                                                          */
/*                       Subroutine degrees_to_decimal                      */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Converts degrees,minutes, seconds to decimal degrees.                */
/*                                                                          */
/*     Input:                                                               */
/*            degrees - Integer degrees                                     */
/*            minutes - Integer minutes                                     */
/*            seconds - Integer seconds                                     */
/*                                                                          */
/*     Output:                                                              */
/*            decimal - degrees in decimal degrees                          */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 12, 1988                                                */
/*                                                                          */
/****************************************************************************/

double degrees_to_decimal(int degrees,int minutes,int seconds)
{
  double deg;
  double min;
  double sec;
  double decimal;
  
  deg = degrees;
  min = minutes/60.0;
  sec = seconds/3600.0;
  
  decimal = fabs(sec) + fabs(min) + fabs(deg);
  
  if (deg < 0) {
    decimal = -decimal;
  } else if (deg == 0){
    if (min < 0){
      decimal = -decimal;
    } else if (min == 0){
      if (sec<0){
        decimal = -decimal;
      }
    }
  }
  
  return(decimal);
}

/****************************************************************************/
/*                                                                          */
/*                           Subroutine julday                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Computes the decimal day of year from month, day, year.              */
/*     Supplied by Daniel Bergstrom                                         */
/*                                                                          */
/* References:                                                              */
/*                                                                          */
/* 1. Nachum Dershowitz and Edward M. Reingold, Calendrical Calculations,   */
/*    Cambridge University Press, 3rd edition, ISBN 978-0-521-88540-9.      */
/*                                                                          */
/* 2. Claus Tøndering, Frequently Asked Questions about Calendars,          */
/*    Version 2.9, http://www.tondering.dk/claus/calendar.html              */
/*                                                                          */
/****************************************************************************/

double julday(month, day, year)
     int month;
     int day;
     int year;
{
  int days[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

  int leap_year = (((year % 4) == 0) &&
                   (((year % 100) != 0) || ((year % 400) == 0)));

  double day_in_year = (days[month - 1] + day + (month > 2 ? leap_year : 0));

  return ((double)year + (day_in_year / (365.0 + leap_year)));
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine getshc                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Reads spherical harmonic coefficients from the specified             */
/*     model into an array.                                                 */
/*                                                                          */
/*     Input:                                                               */
/*           stream     - Logical unit number                               */
/*           iflag      - Flag for SV equal to ) or not equal to 0          */
/*                        for designated read statements                    */
/*           strec      - Starting record number to read from model         */
/*           nmax_of_gh - Maximum degree and order of model                 */
/*                                                                          */
/*     Output:                                                              */
/*           gh1 or 2   - Schmidt quasi-normal internal spherical           */
/*                        harmonic coefficients                             */
/*                                                                          */
/*     FORTRAN                                                              */
/*           Bill Flanagan                                                  */
/*           NOAA CORPS, DESDIS, NGDC, 325 Broadway, Boulder CO.  80301     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 15, 1988                                                */
/*                                                                          */
/****************************************************************************/


int getshc(file, iflag, strec, nmax_of_gh, gh)
char file[PATH];
int iflag;
long int  strec;
int       nmax_of_gh;
int       gh;
{
  char  inbuff[MAXINBUFF];
  char irat[9];
  int ii,m,n,mm,nn;
  int ios;
  int line_num;
  double g,hh;
  double trash;
  
  stream = fopen(file, "rt");
  if (stream == NULL)
    {
      printf("\nError on opening file %s", file);
    }
  else
    {
      ii = 0;
      ios = 0;
      fseek(stream,strec,SEEK_SET);
      for ( nn = 1; nn <= nmax_of_gh; ++nn)
        {
          for (mm = 0; mm <= nn; ++mm)
            {
              if (iflag == 1)
                {
                  fgets(inbuff, MAXREAD, stream);
                  sscanf(inbuff, "%d%d%lg%lg%lg%lg%s%d",
                         &n, &m, &g, &hh, &trash, &trash, irat, &line_num);
                }
              else
                {
                  fgets(inbuff, MAXREAD, stream);
                  sscanf(inbuff, "%d%d%lg%lg%lg%lg%s%d",
                         &n, &m, &trash, &trash, &g, &hh, irat, &line_num);
                }
              if ((nn != n) || (mm != m))
                {
                  ios = -2;
                  fclose(stream);
                  return(ios);
                }
              ii = ii + 1;
              switch(gh)
                {
                case 1:  gh1[ii] = g;
                  break;
                case 2:  gh2[ii] = g;
                  break;
                default: printf("\nError in subroutine getshc");
                  break;
                }
              if (m != 0)
                {
                  ii = ii+ 1;
                  switch(gh)
                    {
                    case 1:  gh1[ii] = hh;
                      break;
                    case 2:  gh2[ii] = hh;
                      break;
                    default: printf("\nError in subroutine getshc");
                      break;
                    }
                }
            }
        }
    }
  fclose(stream);
  return(ios);
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine extrapsh                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Extrapolates linearly a spherical harmonic model with a              */
/*     rate-of-change model.                                                */
/*                                                                          */
/*     Input:                                                               */
/*           date     - date of resulting model (in decimal year)           */
/*           dte1     - date of base model                                  */
/*           nmax1    - maximum degree and order of base model              */
/*           gh1      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of base model                 */
/*           nmax2    - maximum degree and order of rate-of-change model    */
/*           gh2      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of rate-of-change model       */
/*                                                                          */
/*     Output:                                                              */
/*           gha or b - Schmidt quasi-normal internal spherical             */
/*                    harmonic coefficients                                 */
/*           nmax   - maximum degree and order of resulting model           */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 16, 1988                                                */
/*                                                                          */
/****************************************************************************/


int extrapsh(date, dte1, nmax1, nmax2, gh)
double date;
double dte1;
int   nmax1;
int   nmax2;
int   gh;
{
  int   nmax;
  int   k, l;
  int   ii;
  double factor;
  
  factor = date - dte1;
  if (nmax1 == nmax2)
    {
      k =  nmax1 * (nmax1 + 2);
      nmax = nmax1;
    }
  else
    {
      if (nmax1 > nmax2)
        {
          k = nmax2 * (nmax2 + 2);
          l = nmax1 * (nmax1 + 2);
          switch(gh)
            {
            case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  gha[ii] = gh1[ii];
                }
              break;
            case 4:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  ghb[ii] = gh1[ii];
                }
              break;
            default: printf("\nError in subroutine extrapsh");
              break;
            }
          nmax = nmax1;
        }
      else
        {
          k = nmax1 * (nmax1 + 2);
          l = nmax2 * (nmax2 + 2);
          switch(gh)
            {
            case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  gha[ii] = factor * gh2[ii];
                }
              break;
            case 4:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  ghb[ii] = factor * gh2[ii];
                }
              break;
            default: printf("\nError in subroutine extrapsh");
              break;
            }
          nmax = nmax2;
        }
    }
  switch(gh)
    {
    case 3:  for ( ii = 1; ii <= k; ++ii)
        {
          gha[ii] = gh1[ii] + factor * gh2[ii];
        }
      break;
    case 4:  for ( ii = 1; ii <= k; ++ii)
        {
          ghb[ii] = gh1[ii] + factor * gh2[ii];
        }
      break;
    default: printf("\nError in subroutine extrapsh");
      break;
    }
  return(nmax);
}

/****************************************************************************/
/*                                                                          */
/*                           Subroutine interpsh                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Interpolates linearly, in time, between two spherical harmonic       */
/*     models.                                                              */
/*                                                                          */
/*     Input:                                                               */
/*           date     - date of resulting model (in decimal year)           */
/*           dte1     - date of earlier model                               */
/*           nmax1    - maximum degree and order of earlier model           */
/*           gh1      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of earlier model              */
/*           dte2     - date of later model                                 */
/*           nmax2    - maximum degree and order of later model             */
/*           gh2      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of internal model             */
/*                                                                          */
/*     Output:                                                              */
/*           gha or b - coefficients of resulting model                     */
/*           nmax     - maximum degree and order of resulting model         */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 17, 1988                                                */
/*                                                                          */
/****************************************************************************/


int interpsh(date, dte1, nmax1, dte2, nmax2, gh)
     double date;
     double dte1;
     int   nmax1;
     double dte2;
     int   nmax2;
     int   gh;
{
  int   nmax;
  int   k, l;
  int   ii;
  double factor;
  
  factor = (date - dte1) / (dte2 - dte1);
  if (nmax1 == nmax2)
    {
      k =  nmax1 * (nmax1 + 2);
      nmax = nmax1;
    }
  else
    {
      if (nmax1 > nmax2)
        {
          k = nmax2 * (nmax2 + 2);
          l = nmax1 * (nmax1 + 2);
          switch(gh)
            {
            case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  gha[ii] = gh1[ii] + factor * (-gh1[ii]);
                }
              break;
            case 4:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  ghb[ii] = gh1[ii] + factor * (-gh1[ii]);
                }
              break;
            default: printf("\nError in subroutine extrapsh");
              break;
            }
          nmax = nmax1;
        }
      else
        {
          k = nmax1 * (nmax1 + 2);
          l = nmax2 * (nmax2 + 2);
          switch(gh)
            {
            case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  gha[ii] = factor * gh2[ii];
                }
              break;
            case 4:  for ( ii = k + 1; ii <= l; ++ii)
                {
                  ghb[ii] = factor * gh2[ii];
                }
              break;
            default: printf("\nError in subroutine extrapsh");
              break;
            }
          nmax = nmax2;
        }
    }
  switch(gh)
    {
    case 3:  for ( ii = 1; ii <= k; ++ii)
        {
          gha[ii] = gh1[ii] + factor * (gh2[ii] - gh1[ii]);
        }
      break;
    case 4:  for ( ii = 1; ii <= k; ++ii)
        {
          ghb[ii] = gh1[ii] + factor * (gh2[ii] - gh1[ii]);
        }
      break;
    default: printf("\nError in subroutine extrapsh");
      break;
    }
  return(nmax);
}





/****************************************************************************/
/*                                                                          */
/*                           Subroutine shval3                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Calculates field components from spherical harmonic (sh)             */
/*     models.                                                              */
/*                                                                          */
/*     Input:                                                               */
/*           igdgc     - indicates coordinate system used; set equal        */
/*                       to 1 if geodetic, 2 if geocentric                  */
/*           latitude  - north latitude, in degrees                         */
/*           longitude - east longitude, in degrees                         */
/*           elev      - WGS84 altitude above ellipsoid (igdgc=1), or       */
/*                       radial distance from earth's center (igdgc=2)      */
/*           a2,b2     - squares of semi-major and semi-minor axes of       */
/*                       the reference spheroid used for transforming       */
/*                       between geodetic and geocentric coordinates        */
/*                       or components                                      */
/*           nmax      - maximum degree and order of coefficients           */
/*           iext      - external coefficients flag (=0 if none)            */
/*           ext1,2,3  - the three 1st-degree external coefficients         */
/*                       (not used if iext = 0)                             */
/*                                                                          */
/*     Output:                                                              */
/*           x         - northward component                                */
/*           y         - eastward component                                 */
/*           z         - vertically-downward component                      */
/*                                                                          */
/*     based on subroutine 'igrf' by D. R. Barraclough and S. R. C. Malin,  */
/*     report no. 71/1, institute of geological sciences, U.K.              */
/*                                                                          */
/*     FORTRAN                                                              */
/*           Norman W. Peddie                                               */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 17, 1988                                                */
/*                                                                          */
/****************************************************************************/


int shval3(igdgc, flat, flon, elev, nmax, gh, iext, ext1, ext2, ext3)
     int   igdgc;
     double flat;
     double flon;
     double elev;
     int   nmax;
     int   gh;
     int   iext;
     double ext1;
     double ext2;
     double ext3;
{
  double earths_radius = 6371.2;
  double dtr = 0.01745329;
  double slat;
  double clat;
  double ratio;
  double aa, bb, cc, dd;
  double sd;
  double cd;
  double r;
  double a2;
  double b2;
  double rr;
  double fm,fn;
  double sl[14];
  double cl[14];
  double p[119];
  double q[119];
  int ii,j,k,l,m,n;
  int npq;
  int ios;
  double argument;
  double power;
  a2 = 40680631.59;            /* WGS84 */
  b2 = 40408299.98;            /* WGS84 */
  ios = 0;
  r = elev;
  argument = flat * dtr;
  slat = sin( argument );
  if ((90.0 - flat) < 0.001)
    {
      aa = 89.999;            /*  300 ft. from North pole  */
    }
  else
    {
      if ((90.0 + flat) < 0.001)
        {
          aa = -89.999;        /*  300 ft. from South pole  */
        }
      else
        {
          aa = flat;
        }
    }
  argument = aa * dtr;
  clat = cos( argument );
  argument = flon * dtr;
  sl[1] = sin( argument );
  cl[1] = cos( argument );
  switch(gh)
    {
    case 3:  x = 0;
      y = 0;
      z = 0;
      break;
    case 4:  xtemp = 0;
      ytemp = 0;
      ztemp = 0;
      break;
    default: printf("\nError in subroutine shval3");
      break;
    }
  sd = 0.0;
  cd = 1.0;
  l = 1;
  n = 0;
  m = 1;
  npq = (nmax * (nmax + 3)) / 2;
  if (igdgc == 1)
    {
      aa = a2 * clat * clat;
      bb = b2 * slat * slat;
      cc = aa + bb;
      argument = cc;
      dd = sqrt( argument );
      argument = elev * (elev + 2.0 * dd) + (a2 * aa + b2 * bb) / cc;
      r = sqrt( argument );
      cd = (elev + dd) / r;
      sd = (a2 - b2) / dd * slat * clat / r;
      aa = slat;
      slat = slat * cd - clat * sd;
      clat = clat * cd + aa * sd;
    }
  ratio = earths_radius / r;
  argument = 3.0;
  aa = sqrt( argument );
  p[1] = 2.0 * slat;
  p[2] = 2.0 * clat;
  p[3] = 4.5 * slat * slat - 1.5;
  p[4] = 3.0 * aa * clat * slat;
  q[1] = -clat;
  q[2] = slat;
  q[3] = -3.0 * clat * slat;
  q[4] = aa * (slat * slat - clat * clat);
  for ( k = 1; k <= npq; ++k)
    {
      if (n < m)
        {
          m = 0;
          n = n + 1;
          argument = ratio;
          power =  n + 2;
          rr = pow(argument,power);
          fn = n;
        }
      fm = m;
      if (k >= 5)
        {
          if (m == n)
            {
              argument = (1.0 - 0.5/fm);
              aa = sqrt( argument );
              j = k - n - 1;
              p[k] = (1.0 + 1.0/fm) * aa * clat * p[j];
              q[k] = aa * (clat * q[j] + slat/fm * p[j]);
              sl[m] = sl[m-1] * cl[1] + cl[m-1] * sl[1];
              cl[m] = cl[m-1] * cl[1] - sl[m-1] * sl[1];
            }
          else
            {
              argument = fn*fn - fm*fm;
              aa = sqrt( argument );
              argument = ((fn - 1.0)*(fn-1.0)) - (fm * fm);
              bb = sqrt( argument )/aa;
              cc = (2.0 * fn - 1.0)/aa;
              ii = k - n;
              j = k - 2 * n + 1;
              p[k] = (fn + 1.0) * (cc * slat/fn * p[ii] - bb/(fn - 1.0) * p[j]);
              q[k] = cc * (slat * q[ii] - clat/fn * p[ii]) - bb * q[j];
            }
        }
      switch(gh)
        {
        case 3:  aa = rr * gha[l];
          break;
        case 4:  aa = rr * ghb[l];
          break;
        default: printf("\nError in subroutine shval3");
          break;
        }
      if (m == 0)
        {
          switch(gh)
            {
            case 3:  x = x + aa * q[k];
              z = z - aa * p[k];
              break;
            case 4:  xtemp = xtemp + aa * q[k];
              ztemp = ztemp - aa * p[k];
              break;
            default: printf("\nError in subroutine shval3");
              break;
            }
          l = l + 1;
        }
      else
        {
          switch(gh)
            {
            case 3:  bb = rr * gha[l+1];
              cc = aa * cl[m] + bb * sl[m];
              x = x + cc * q[k];
              z = z - cc * p[k];
              if (clat > 0)
                {
                  y = y + (aa * sl[m] - bb * cl[m]) *
                    fm * p[k]/((fn + 1.0) * clat);
                }
              else
                {
                  y = y + (aa * sl[m] - bb * cl[m]) * q[k] * slat;
                }
              l = l + 2;
              break;
            case 4:  bb = rr * ghb[l+1];
              cc = aa * cl[m] + bb * sl[m];
              xtemp = xtemp + cc * q[k];
              ztemp = ztemp - cc * p[k];
              if (clat > 0)
                {
                  ytemp = ytemp + (aa * sl[m] - bb * cl[m]) *
                    fm * p[k]/((fn + 1.0) * clat);
                }
              else
                {
                  ytemp = ytemp + (aa * sl[m] - bb * cl[m]) *
                    q[k] * slat;
                }
              l = l + 2;
              break;
            default: printf("\nError in subroutine shval3");
              break;
            }
        }
      m = m + 1;
    }
  if (iext != 0)
    {
      aa = ext2 * cl[1] + ext3 * sl[1];
      switch(gh)
        {
        case 3:   x = x - ext1 * clat + aa * slat;
          y = y + ext2 * sl[1] - ext3 * cl[1];
          z = z + ext1 * slat + aa * clat;
          break;
        case 4:   xtemp = xtemp - ext1 * clat + aa * slat;
          ytemp = ytemp + ext2 * sl[1] - ext3 * cl[1];
          ztemp = ztemp + ext1 * slat + aa * clat;
          break;
        default:  printf("\nError in subroutine shval3");
          break;
        }
    }
  switch(gh)
    {
    case 3:   aa = x;
		x = x * cd + z * sd;
		z = z * cd - aa * sd;
		break;
    case 4:   aa = xtemp;
		xtemp = xtemp * cd + ztemp * sd;
		ztemp = ztemp * cd - aa * sd;
		break;
    default:  printf("\nError in subroutine shval3");
		break;
    }
  return(ios);
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine dihf                                */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Computes the geomagnetic d, i, h, and f from x, y, and z.            */
/*                                                                          */
/*     Input:                                                               */
/*           x  - northward component                                       */
/*           y  - eastward component                                        */
/*           z  - vertically-downward component                             */
/*                                                                          */
/*     Output:                                                              */
/*           d  - declination                                               */
/*           i  - inclination                                               */
/*           h  - horizontal intensity                                      */
/*           f  - total intensity                                           */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 22, 1988                                                */
/*                                                                          */
/****************************************************************************/

int dihf (gh)
     int gh;
{
  int ios;
  int j;
  double sn;
  double h2;
  double hpx;
  double argument, argument2;
  
  ios = gh;
  sn = 0.0001;
  
  switch(gh)
    {
    case 3:   for (j = 1; j <= 1; ++j)
        {
          h2 = x*x + y*y;
          argument = h2;
          h = sqrt(argument);       /* calculate horizontal intensity */
          argument = h2 + z*z;
          f = sqrt(argument);      /* calculate total intensity */
          if (f < sn)
            {
              d = NaN;        /* If d and i cannot be determined, */
              i = NaN;        /*       set equal to NaN         */
            }
          else
            {
              argument = z;
              argument2 = h;
              i = atan2(argument,argument2);
              if (h < sn)
                {
                  d = NaN;
                }
              else
                {
                  hpx = h + x;
                  if (hpx < sn)
                    {
                      d = PI;
                    }
                  else
                    {
                      argument = y;
                      argument2 = hpx;
                      d = 2.0 * atan2(argument,argument2);
                    }
                }
            }
        }
		break;
    case 4:   for (j = 1; j <= 1; ++j)
        {
          h2 = xtemp*xtemp + ytemp*ytemp;
          argument = h2;
          htemp = sqrt(argument);
          argument = h2 + ztemp*ztemp;
          ftemp = sqrt(argument);
          if (ftemp < sn)
            {
              dtemp = NaN;    /* If d and i cannot be determined, */
              itemp = NaN;    /*       set equal to 999.0         */
            }
          else
            {
              argument = ztemp;
              argument2 = htemp;
              itemp = atan2(argument,argument2);
              if (htemp < sn)
                {
                  dtemp = NaN;
                }
              else
                {
                  hpx = htemp + xtemp;
                  if (hpx < sn)
                    {
                      dtemp = PI;
                    }
                  else
                    {
                      argument = ytemp;
                      argument2 = hpx;
                      dtemp = 2.0 * atan2(argument,argument2);
                    }
                }
            }
        }
		break;
    default:  printf("\nError in subroutine dihf");
		break;
    }
  return(ios);
}
