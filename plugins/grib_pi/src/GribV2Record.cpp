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
/*
** File: unpackgrib2.c
**
** Author:  Bob Dattore
**          NCAR/DSS
**          dattore@ucar.edu
**          (303) 497-1825
**	latest
**          14 Aug 2015:
**             DRS Template 5.3 (complex packing and spatial differencing)

    copyright ?
*/

#define __STDC_LIMIT_MACROS 

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <stdlib.h>

#include "GribV2Record.h"

#ifdef JASPER
#include <jasper/jasper.h>
#endif

const double GRIB_MISSING_VALUE = GRIB_NOTDEF;

class GRIBStatproc {
public:
    int proc_code;
    int incr_type;
    int time_unit;
    int time_length;
    int incr_unit;
    int incr_length;
};

class GRIBMetadata {
public:
  GRIBMetadata() : bitmap(0), bms(0) {
    stat_proc.t = 0;
    lvl1_type = 0;
    lvl2_type = 0;
    lvl1 = 0.;
    lvl2 = 0.;
  };

  ~GRIBMetadata() {
    delete [] stat_proc.t;
    delete [] bitmap;
    delete [] bms;
  };

  int gds_templ_num;

  int earth_shape;
  unsigned char earth_sphere_scale_factor; // Scale factor of radius of spherical Earth
  int earth_sphere_scale_value;            // Scale value of radius of spherical Earth

  unsigned char earth_major_scale_factor; // Scale factor of major axis of oblate spheroid Earth
  int earth_major_scale_value;            // Scaled value of major axis of oblate spheroid Earth

  unsigned char earth_minor_scale_factor; // Scale factor of minor axis of oblate spheroid Earth
  int earth_minor_scale_value;            // Scaled value of minor axis of oblate spheroid Earth

  int nx,ny;
  double slat,slon,latin1,latin2,splat,splon;
  double latD;
  union {
    double elat;
    double lad;
  } lats;
  union {
    double elon;
    double lov;
  } lons;
  union {
    double loinc;
    double dxinc;
  } xinc;
  union {
    double lainc;
    double dyinc;
  } yinc;
  int rescomp,scan_mode,proj_flag;
  int pds_templ_num;
  int param_cat,param_num,gen_proc,time_unit,fcst_time;
  int ens_type,perturb_num,derived_fcst_code,nfcst_in_ensemble;
  int lvl1_type,lvl2_type;
  double lvl1,lvl2;
  struct {
    int eyr,emo,edy,etime;
    int num_ranges, nmiss;
    GRIBStatproc *t;
  } stat_proc;
  struct {
    int stat_proc,type,num_points;
  } spatial_proc;
  struct {
    int split_method,miss_val_mgmt;
    unsigned int num_groups;
    float primary_miss_sub,secondary_miss_sub;
    struct {
	int ref,pack_width;
    } width;
    struct {
	unsigned int ref,incr,last,pack_width;
    } length;
    struct {
	unsigned int order,order_vals_width;
    } spatial_diff;
  } complex_pack;
  int drs_templ_num;
  float R;
  int E,D,num_packed,pack_width,orig_val_type;
  int bms_ind;
  unsigned char *bitmap;
  ///
  zuchar	*bms;
  int		bmssize;
} ;

class  GRIB2Grid {
public:
  GRIB2Grid() : gridpoints(0) { };
  ~GRIB2Grid() { delete [] gridpoints; };
  
  double *gridpoints;
};

class  GRIBMessage {
public:
  GRIBMessage() : buffer(0) {};
  ~GRIBMessage() { 
      delete [] buffer;
  };
  unsigned char *buffer;
  int offset;  /* offset in bytes to next GRIB2 section */
  int total_len,disc,ed_num;
  int center_id,sub_center_id,table_ver,local_table_ver,ref_time_type;
  int yr,mo,dy,time;
  int prod_status,data_type;
  GRIBMetadata md;
  size_t num_grids;
  GRIB2Grid grids;
};


#ifdef JASPER
static int dec_jpeg2000(char *injpc,int bufsize,int *outfld)
/*$$$  SUBPROGRAM DOCUMENTATION BLOCK
*                .      .    .                                       .
* SUBPROGRAM:    dec_jpeg2000      Decodes JPEG2000 code stream
*   PRGMMR: Gilbert          ORG: W/NP11     DATE: 2002-12-02
*
* ABSTRACT: This Function decodes a JPEG2000 code stream specified in the
*   JPEG2000 Part-1 standard (i.e., ISO/IEC 15444-1) using JasPer 
*   Software version 1.500.4 (or 1.700.2) written by the University of British
*   Columbia and Image Power Inc, and others.
*   JasPer is available at http://www.ece.uvic.ca/~mdadams/jasper/.
*
* PROGRAM HISTORY LOG:
* 2002-12-02  Gilbert
*
* USAGE:     int dec_jpeg2000(char *injpc,int bufsize,int *outfld)
*
*   INPUT ARGUMENTS:
*      injpc - Input JPEG2000 code stream.
*    bufsize - Length (in bytes) of the input JPEG2000 code stream.
*
*   OUTPUT ARGUMENTS:
*     outfld - Output matrix of grayscale image values.
*
*   RETURN VALUES :
*          0 = Successful decode
*         -2 = no memory Error.
*         -3 = Error decode jpeg2000 code stream.
*         -5 = decoded image had multiple color components.
*              Only grayscale is expected.
*
* REMARKS:
*
*      Requires JasPer Software version 1.500.4 or 1.700.2
*
* ATTRIBUTES:
*   LANGUAGE: C
*   MACHINE:  IBM SP
*
*$$$*/

{
    int ier;
    int i,j,k;
    jas_image_t *image=0;
    jas_stream_t *jpcstream;
    jas_image_cmpt_t *pcmpt;
    char *opts=0;
    jas_matrix_t *data;

//    jas_init();

    ier=0;
//   
//     Create jas_stream_t containing input JPEG200 codestream in memory.
//       

    jpcstream=jas_stream_memopen(injpc,bufsize);
    if (jpcstream == nullptr) {
        printf(" dec_jpeg2000: no memory\n");
        return -2;
    }
//   
//     Decode JPEG200 codestream into jas_image_t structure.
//       
    image=jpc_decode(jpcstream,opts);
    if ( image == 0 ) {
       printf(" jpc_decode return = %d \n",ier);
       return -3;
    }
    
    pcmpt=image->cmpts_[0];

//   Expecting jpeg2000 image to be grayscale only.
//   No color components.
//
    if (image->numcmpts_ != 1 ) {
       printf("dec_jpeg2000: Found color image.  Grayscale expected.\n");
       return (-5);
    }

// 
//    Create a data matrix of grayscale image values decoded from
//    the jpeg2000 codestream.
//
    data=jas_matrix_create(jas_image_height(image), jas_image_width(image));
    jas_image_readcmpt(image,0,0,0,jas_image_width(image),
                       jas_image_height(image),data);
//
//    Copy data matrix to output integer array.
//
    k=0;
    for (i=0;i<pcmpt->height_;i++) 
      for (j=0;j<pcmpt->width_;j++) 
        outfld[k++]=data->rows_[i][j];
//
//     Clean up JasPer work structures.
//
    jas_matrix_destroy(data);
    ier=jas_stream_close(jpcstream);
    jas_image_destroy(image);

    return 0;

}
#endif

static unsigned int uint2(unsigned char const *p) {
    return (p[0] << 8) + p[1];
}

static unsigned int uint4(unsigned const char *p) {
    return ((p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]);
}

static int int2(unsigned const char *p) {
    int i;
    if ((p[0] & 0x80)) {
        i = -(((p[0] & 0x7f) << 8) + p[1]);
    }
    else {
        i = (p[0] << 8) + p[1];
    }
    return i;
}

static int int4(unsigned const char *p) {
    int i;
    if ((p[0] & 0x80)) {
        i = -(((p[0] & 0x7f) << 24) + (p[1] << 16) + (p[2] << 8) + p[3]);
    }
    else {
        i = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
    }
    return i;
}

static float ieee2flt(unsigned const char *ieee) {
    double fmant;
    int exp;

    if ((ieee[0] & 127) == 0 && ieee[1] == 0 && ieee[2] == 0 && ieee[3] == 0)
        return (float) 0.0;

    exp = ((ieee[0] & 127) << 1) + (ieee[1] >> 7);
    fmant = (double) ((int) ieee[3] + (int) (ieee[2] << 8) + (int) ((ieee[1] | 128) << 16));
    if (ieee[0] & 128) 
        fmant = -fmant;

    return (float) (ldexp(fmant, (int) (exp - 128 - 22)));
}

static inline void getBits(unsigned const char *buf, int *loc, size_t first, size_t nbBits)
{
    if (nbBits == 0) {
        // x >> 32 is undefined behavior, on x86 it returns x
        *loc = 0;
        return;
    }

    zuint oct = first / 8;
    zuint bit = first % 8;

    zuint val = (buf[oct]<<24) + (buf[oct+1]<<16) + (buf[oct+2]<<8) + (buf[oct+3]);
    val = val << bit;
    val = val >> (32-nbBits);
    *loc = val;
}

//-------------------------------------------------------------------------------
// Lecture depuis un fichier
//-------------------------------------------------------------------------------
static void unpackIDS(GRIBMessage *grib_msg)
{
  int length;
  int hh,mm,ss;
  size_t ofs = grib_msg->offset/8;
  unsigned char *b = grib_msg->buffer +ofs;

  length = uint4(b);   /* length of the IDS */

  grib_msg->center_id       = uint2(b +5); /* center ID */
  grib_msg->sub_center_id   = uint2(b +7); /* sub-center ID */
  grib_msg->table_ver       = b[9];        /* table version */
  grib_msg->local_table_ver = b[10];       /* local table version */
  grib_msg->ref_time_type   = b[11];       /* significance of reference time */
  grib_msg->yr              = uint2(b+12); /* year */
  grib_msg->mo              = b[14];       /* month */
  grib_msg->dy              = b[15];       /* day */
  hh                        = b[16];       /* hours */
  mm                        = b[17];       /* minutes */
  ss                        = b[18];       /* seconds */
  grib_msg->time=hh*10000+mm*100+ss;
  grib_msg->prod_status = b[19];  /* production status */
  grib_msg->data_type   = b[20]; /* type of data */
  grib_msg->offset += length*8;
}

static bool unpackLUS(GRIBMessage *grib_msg)
{
    return true;
}

static void parse_earth(GRIBMessage *grib_msg)
{
  size_t ofs = grib_msg->offset/8;
  unsigned char *b = grib_msg->buffer +ofs;

  grib_msg->md.earth_shape = b[14];         // shape of the earth

  grib_msg->md.earth_sphere_scale_factor = b[15];        // Scale factor of radius of spherical Earth
  grib_msg->md.earth_sphere_scale_value  = uint4(b +16); // Scale value of radius of spherical Earth

  grib_msg->md.earth_major_scale_factor = b[20];         // Scale factor of major axis of oblate spheroid Earth
  grib_msg->md.earth_major_scale_value  = uint4(b +21);  // Scaled value of major axis of oblate spheroid Earth

  grib_msg->md.earth_minor_scale_factor = b[25];         // Scale factor of minor axis of oblate spheroid Earth
  grib_msg->md.earth_minor_scale_value = uint4(b +26);   // Scaled value of minor axis of oblate spheroid Earth

}

static bool unpackGDS(GRIBMessage *grib_msg)
{
  int src,num_in_list;
  size_t ofs = grib_msg->offset/8;
  unsigned char *b = grib_msg->buffer +ofs;

  src = b[5]; /* source of grid definition */
  if (src != 0) {
    fprintf(stderr,"Don't recognize predetermined grid definitions");
    return false;
  }

  num_in_list = b[10];  /* quasi-regular grid indication */
  if (num_in_list > 0) {
    fprintf(stderr,"Unable to unpack quasi-regular grids");
    return false;
  }

  /* grid definition template number Table 3.1 */
  grib_msg->md.gds_templ_num = uint2(b +12);
  switch (grib_msg->md.gds_templ_num) {
    case 0:   /* Latitude/Longitude Also called Equidistant Cylindrical or Plate Caree */
    case 40:  /* Gaussian Latitude/Longitude  */
        parse_earth(grib_msg);

        grib_msg->md.nx          = uint4(b +30);  /* number of latitudes */
        grib_msg->md.ny          = uint4(b +34);  /* number of longitudes */

	grib_msg->md.slat = int4(b +46)/1000000.; /* latitude of first gridpoint */
	grib_msg->md.slon = int4(b +50)/1000000.; /* longitude of first gridpoint */

	grib_msg->md.rescomp = b[54];             /* resolution and component flags */

	grib_msg->md.lats.elat = int4(b +55)/1000000.; /* latitude of last gridpoint */
	grib_msg->md.lons.elon = int4(b +59)/1000000.; /* longitude of last gridpoint */

	grib_msg->md.xinc.loinc = uint4(b +63)/1000000.; /* longitude increment */

	if (grib_msg->md.gds_templ_num == 0)
	  grib_msg->md.yinc.lainc = uint4(b +67)/1000000.; /* latitude increment */

	grib_msg->md.scan_mode = b[71]; /* scanning mode flag */
	break;
    case 10: /* Mercator */
        parse_earth(grib_msg);

	grib_msg->md.nx = uint4(b +30); 	/* number of points along a parallel */
	grib_msg->md.ny = uint4(b +34); 	/* number of points along a meridian */

	grib_msg->md.slat = int4(b + 38)/1000000.; /* latitude of first gridpoint */
	grib_msg->md.slon = int4(b + 42)/1000000.; /* longitude of first gridpoint */

	grib_msg->md.rescomp = b[46]; /* resolution and component flags */
	grib_msg->md.latD    = int4(b +47)/1000000.; /* latitude at which the Mercator projection intersects the Earth 
                                                        (Latitude where Di and Dj are specified)   */

	grib_msg->md.lats.elat = int4(b +51)/1000000.; /* latitude of last gridpoint */
	grib_msg->md.lons.elon = int4(b +55)/1000000.; /* longitude of last gridpoint */

        grib_msg->md.scan_mode = b[59]; /* scanning mode flag */

	grib_msg->md.xinc.loinc = uint4(b +64)/1000.; /* longitude increment */
	grib_msg->md.yinc.lainc = uint4(b +68)/1000.; /* latitude increment */
        break;
    case 30: /* Lambert conformal grid */
        parse_earth(grib_msg);

	grib_msg->md.nx = uint4(b +30); /* number of points along a parallel */
	grib_msg->md.ny = uint4(b +34); /* number of points along a meridian */

	grib_msg->md.slat = int4(b + 38)/1000000.; /* latitude of first gridpoint */
	grib_msg->md.slon = int4(b + 42)/1000000.; /* longitude of first gridpoint */

	grib_msg->md.rescomp = b[46]; /* resolution and component flags */

	grib_msg->md.lats.lad = int4(b +47)/1000000.; /* LaD */
	grib_msg->md.lons.lov = int4(b +51)/1000000.; /* LoV */

	grib_msg->md.xinc.dxinc = int4(b +55)/1000.; /* x-direction increment */
	grib_msg->md.yinc.dyinc = int4(b +59)/1000.; /* y-direction increment */

	grib_msg->md.proj_flag = b[63]; /* projection center flag */
        grib_msg->md.scan_mode = b[64]; /* scanning mode flag */

	grib_msg->md.latin1 = int4(b +65)/1000000.; /* latin1 */
	grib_msg->md.latin2 = int4(b +69)/1000000.; /* latin2 */

	grib_msg->md.splat = int4(b +73)/1000000.; /* latitude of southern pole of projection */
	grib_msg->md.splon = int4(b +77)/1000000.; /* longitude of southern pole of projection */
	break;
    default:
	fprintf(stderr,"Grid template %d is not understood\n",grib_msg->md.gds_templ_num);
	return false;
  }
  return true;
}

static void unpack_stat_proc(GRIBMessage *grib_msg, unsigned const char *b)
{
  int hh,mm,ss;
  size_t n, off;

  grib_msg->md.stat_proc.eyr = uint2(b);
  grib_msg->md.stat_proc.emo = b[2];
  grib_msg->md.stat_proc.edy = b[3];
  hh = b[4];
  mm = b[5];
  ss = b[6];
  grib_msg->md.stat_proc.etime = hh*10000+mm*100+ss;

  grib_msg->md.stat_proc.num_ranges = b[7];        /* number of time range specifications */
  grib_msg->md.stat_proc.nmiss      = uint4(b +8); /* number of values missing from process */

  if (grib_msg->md.stat_proc.t != 0) {
      delete [] grib_msg->md.stat_proc.t;
  }
  grib_msg->md.stat_proc.t= new GRIBStatproc[grib_msg->md.stat_proc.num_ranges];
  off=12;
  for (n=0; n < (size_t)grib_msg->md.stat_proc.num_ranges; n++) {
      grib_msg->md.stat_proc.t[n].proc_code   = b[off];
      grib_msg->md.stat_proc.t[n].incr_type   = b[off +1];
      grib_msg->md.stat_proc.t[n].time_unit   = b[off +2];
      grib_msg->md.stat_proc.t[n].time_length = uint4(b + off +3);
      grib_msg->md.stat_proc.t[n].incr_unit   = b[off +7];
      grib_msg->md.stat_proc.t[n].incr_length = uint4(b +off +8);
      off += 12;
  }
}

// Section 4: Product Definition Section 
static bool unpackPDS(GRIBMessage *grib_msg)
{
  int num_coords,factor;
  size_t ofs = grib_msg->offset/8;
  unsigned char *b = grib_msg->buffer +ofs;

  num_coords = uint2(b +5);/* indication of hybrid coordinate system */
  if (num_coords > 0) {
    fprintf(stderr,"Unable to decode hybrid coordinates");
    return false;
  }

  grib_msg->md.pds_templ_num = uint2(b +7); /* product definition template number */
  grib_msg->md.stat_proc.num_ranges=0;
  switch (grib_msg->md.pds_templ_num) {
    case 0:
    case 1:
    case 2:
    case 8:   // Average, accumulation, extreme values
    case 11:
    case 12:
    case 15:
	grib_msg->md.ens_type=-1;
	grib_msg->md.derived_fcst_code=-1;
	grib_msg->md.spatial_proc.type=-1;
	grib_msg->md.param_cat = b[9];         /* parameter category */
	grib_msg->md.param_num = b[10];        /* parameter number */
	grib_msg->md.gen_proc  = b[11];        /* generating process */

	grib_msg->md.time_unit = b[17];        /* time range indicator*/
	grib_msg->md.fcst_time = uint4(b +18); /* forecast time */

	grib_msg->md.lvl1_type = b[22];        /* type of first level */
	factor = b[23];			       /* value of first level */
	grib_msg->md.lvl1 = int4(b +24)/pow(10.,(double)factor);

	grib_msg->md.lvl2_type = b[28];        /* type of second level */
	factor = b[29];                        /* value of second level */
	grib_msg->md.lvl2 = int4(b +30)/pow(10.,(double)factor);

	switch (grib_msg->md.pds_templ_num) {
	  case 1:
	  case 11:
	    grib_msg->md.ens_type    = b[34];
	    grib_msg->md.perturb_num = b[35];
	    grib_msg->md.nfcst_in_ensemble = b[36];

	    switch (grib_msg->md.pds_templ_num) {
		case 11:
		  unpack_stat_proc(grib_msg, b +37);
		  break;
	    }
	    break;
	  case 2:
	  case 12:
	    grib_msg->md.derived_fcst_code = b[34];
	    grib_msg->md.nfcst_in_ensemble = b[35];

	    switch (grib_msg->md.pds_templ_num) {
		case 12:
		  unpack_stat_proc(grib_msg, b +36);
		  break;
	    }
	    break;
	  case 8:
            unpack_stat_proc(grib_msg, b +34);
	    break;
	  case 15:
	    grib_msg->md.spatial_proc.stat_proc  = b[34];
	    grib_msg->md.spatial_proc.type       = b[35];
	    grib_msg->md.spatial_proc.num_points = b[36];
	    break;
	}
	break;
    default:
	fprintf(stderr,"Product Definition Template %d is not understood\n",grib_msg->md.pds_templ_num);
	return false;
  }
  return true;
}

//  Section 5: Data Representation Section 
static bool unpackDRS(GRIBMessage *grib_msg)
{
  size_t ofs = grib_msg->offset/8;
  unsigned char *b = grib_msg->buffer +ofs;

  grib_msg->md.num_packed = uint4(b +5);    /* number of packed values */
  grib_msg->md.drs_templ_num = uint2(b +9); /* data representation template number */

  switch (grib_msg->md.drs_templ_num) { // Table 5.0
    case 0:        // Grid Point Data - Simple Packing 
    case 2:        // Grid Point Data - Complex Packing
    case 3:        // Grid Point Data - Complex Packing and Spatial Differencing
#ifdef JASPER
    case 40:       // Grid Point Data - JPEG2000 Compression
    case 40000:
#endif
   /* cf http://www.wmo.int/pages/prog/www/WMOCodes/Guides/GRIB/GRIB2_062006.pdf p. 36*/
	grib_msg->md.R = ieee2flt(b+ 11);
	grib_msg->md.E = int2(b +15);
	grib_msg->md.D = int2(b +17);
	grib_msg->md.R /= pow(10.,grib_msg->md.D);

	grib_msg->md.pack_width = b[19];
	grib_msg->md.orig_val_type = b[20];

	if (grib_msg->md.drs_templ_num == 3 || grib_msg->md.drs_templ_num == 2) {
	  grib_msg->md.complex_pack.split_method = b[21];
	  grib_msg->md.complex_pack.miss_val_mgmt = b[22];
	  if (grib_msg->md.orig_val_type == 0) { // Table 5.1
	    grib_msg->md.complex_pack.primary_miss_sub   = ieee2flt(b+ 23);
	    grib_msg->md.complex_pack.secondary_miss_sub = ieee2flt(b+ 27);
	  }
	  else if (grib_msg->md.orig_val_type == 1) {
	    grib_msg->md.complex_pack.primary_miss_sub   = uint4(b +23);
	    grib_msg->md.complex_pack.secondary_miss_sub = uint4(b +27);
	  }
	  else {
	    fprintf(stderr,"Unable to decode missing value substitutes for original value type %d\n",grib_msg->md.orig_val_type);
	    return false;
	  }
	  grib_msg->md.complex_pack.num_groups = uint4(b +31);

	  grib_msg->md.complex_pack.width.ref        = b[35];
	  grib_msg->md.complex_pack.width.pack_width = b[36];

	  grib_msg->md.complex_pack.length.ref        = uint4(b +37);
	  grib_msg->md.complex_pack.length.incr       = b[41];
	  grib_msg->md.complex_pack.length.last       = uint4(b +42);
	  grib_msg->md.complex_pack.length.pack_width = b[46];
	}
	if (grib_msg->md.drs_templ_num == 3) {
	  grib_msg->md.complex_pack.spatial_diff.order = b[47];
	  grib_msg->md.complex_pack.spatial_diff.order_vals_width = b[48];
	}
	else {
	  grib_msg->md.complex_pack.spatial_diff.order = 0;
	  grib_msg->md.complex_pack.spatial_diff.order_vals_width = 0;
	}
	break;
    default:
	fprintf(stderr,"Data template %d is not understood\n",grib_msg->md.drs_templ_num);
	return false;
  }
  return true;
}

//  Section 6: Bit-Map Section 
static bool unpackBMS(GRIBMessage *grib_msg)
{
  int ind,len,n,bit;
  size_t ofs = grib_msg->offset/8;
  unsigned char *b = grib_msg->buffer +ofs;

  ind = b[5]; /* bit map indicator */
  switch (ind) {
    case 0: // A bit map applies to this product and is specified in this section.
	len = uint4(b);
	if (len < 7)
	    return false;
        len -=6;
	grib_msg->md.bmssize = len;
	len *= 8;
        delete [] grib_msg->md.bitmap;
        delete [] grib_msg->md.bms;
	grib_msg->md.bitmap = new unsigned char[len];
	grib_msg->md.bms = new zuchar[grib_msg->md.bmssize];
	memcpy (grib_msg->md.bms, b + 6, grib_msg->md.bmssize);
	for (n=0; n < len; n++) {
	  getBits(grib_msg->buffer, &bit, grib_msg->offset+48+n, 1);
	  grib_msg->md.bitmap[n]=bit;
	}
	break;
    case 254: // A bit map previously defined in the same GRIB2 message applies to this product.
	break;
    case 255:  // A bit map does not apply to this product.
        delete [] grib_msg->md.bitmap;
	grib_msg->md.bitmap=NULL;
        delete [] grib_msg->md.bms;
	grib_msg->md.bms=NULL;
	grib_msg->md.bmssize = 0;
	break;
    default:
	fprintf(stderr,"This code is not currently set up to deal with predefined bit-maps\n");
	return false;
  }
  return true;
}

// Section 7: Data Section
static bool unpackDS(GRIBMessage *grib_msg)
{
  int off,pval, l;
  unsigned int n, m;
  struct {
    int *ref_vals,*widths;
    int *lengths;
    int *first_vals = 0,sign,omin;
    long long miss_val,group_miss_val;
    int max_length;
  } groups;
  float lastgp,D=pow(10.,grib_msg->md.D),E=pow(2.,grib_msg->md.E);

  groups.omin = 0;
  groups.first_vals = nullptr;

  off= grib_msg->offset+40;
  switch (grib_msg->md.drs_templ_num) {
    case 0:
	grib_msg->grids.gridpoints = new double[grib_msg->md.ny *grib_msg->md.nx];
	for (l=0; l < grib_msg->md.ny*grib_msg->md.nx; l++) {
	  if (grib_msg->md.bitmap == NULL || grib_msg->md.bitmap[l] == 1) {
	    getBits(grib_msg->buffer,&pval,off,grib_msg->md.pack_width);
	    grib_msg->grids.gridpoints[l]=grib_msg->md.R+pval*E/D;
	    off+=grib_msg->md.pack_width;
	  }
	  else
	    grib_msg->grids.gridpoints[l]=GRIB_MISSING_VALUE;
	}
	break;
    case 3:
	if (grib_msg->md.complex_pack.num_groups > 0) {
          if (grib_msg->md.complex_pack.spatial_diff.order) {
	      groups.first_vals= new int[grib_msg->md.complex_pack.spatial_diff.order];
	      for (n=0; n < grib_msg->md.complex_pack.spatial_diff.order; ++n) {
	          getBits(grib_msg->buffer,&groups.first_vals[n],off,grib_msg->md.complex_pack.spatial_diff.order_vals_width*8);
	          off+=grib_msg->md.complex_pack.spatial_diff.order_vals_width*8;
              }
	  }
	  getBits(grib_msg->buffer,&groups.sign,off,1);
	  getBits(grib_msg->buffer,&groups.omin,off+1,grib_msg->md.complex_pack.spatial_diff.order_vals_width*8-1);
	  if (groups.sign == 1) {
	    groups.omin=-groups.omin;
	  }
	  off+=grib_msg->md.complex_pack.spatial_diff.order_vals_width*8;
	}
	// fall through
    case 2:
	grib_msg->grids.gridpoints=new double[grib_msg->md.ny*grib_msg->md.nx];
	if (grib_msg->md.complex_pack.num_groups == 0) {
	  for (l = 0; l < grib_msg->md.ny*grib_msg->md.nx; ++l) {
 	    grib_msg->grids.gridpoints[l] = GRIB_MISSING_VALUE;
	  }
	  break;
	}
        if (grib_msg->md.complex_pack.miss_val_mgmt > 0) {
	  groups.miss_val=pow(2.,grib_msg->md.pack_width)-1;
	}
	else {
	  groups.miss_val=GRIB_MISSING_VALUE;
	}

	groups.ref_vals = new int[grib_msg->md.complex_pack.num_groups];
	groups.widths   = new int[grib_msg->md.complex_pack.num_groups];
	groups.lengths  = new int[grib_msg->md.complex_pack.num_groups];

	for (n=0; n < grib_msg->md.complex_pack.num_groups; ++n) {
	  getBits(grib_msg->buffer,&groups.ref_vals[n],off,grib_msg->md.pack_width);
	  off+=grib_msg->md.pack_width;
	}
	off = (off + 7) & ~7; // byte boundary padding 

	for (n=0; n < grib_msg->md.complex_pack.num_groups; ++n) {
	  getBits(grib_msg->buffer,&groups.widths[n],off,grib_msg->md.complex_pack.width.pack_width);
          groups.widths[n] += grib_msg->md.complex_pack.width.ref;
	  off+=grib_msg->md.complex_pack.width.pack_width;
	}
	off = (off + 7) & ~7;

	for (n=0; n < grib_msg->md.complex_pack.num_groups; ++n) {
	  getBits(grib_msg->buffer,&groups.lengths[n],off,grib_msg->md.complex_pack.length.pack_width);
	  off+=grib_msg->md.complex_pack.length.pack_width;
	}
	off = (off + 7) & ~7;

	groups.max_length=0;
	for (n=0; n < grib_msg->md.complex_pack.num_groups-1; ++n) {
	  groups.lengths[n]=grib_msg->md.complex_pack.length.ref+groups.lengths[n]*grib_msg->md.complex_pack.length.incr;
	  if (groups.lengths[n] > groups.max_length) {
		groups.max_length=groups.lengths[n];
	  }
	}
	groups.lengths[n]=grib_msg->md.complex_pack.length.last;
	if (groups.lengths[n] > groups.max_length) {
	  groups.max_length=groups.lengths[n];
	}
	// unpack the field of differences
	for (n=0,l=0; n < grib_msg->md.complex_pack.num_groups; ++n) {
	  if (groups.widths[n] > 0) {
		if (grib_msg->md.complex_pack.miss_val_mgmt > 0) {
		  groups.group_miss_val=pow(2.,groups.widths[n])-1;
		}
		else {
		  groups.group_miss_val=GRIB_MISSING_VALUE;
		}
		for (int i = 0; i < groups.lengths[n]; ) {
		  if (grib_msg->md.bitmap != NULL && grib_msg->md.bitmap[l] == 0) {
                    grib_msg->grids.gridpoints[l]=GRIB_MISSING_VALUE;
                  }
                  else {
                    getBits(grib_msg->buffer,&pval,off,groups.widths[n]);
                    off+=groups.widths[n];
		      if (pval == groups.group_miss_val) {
                         grib_msg->grids.gridpoints[l]=GRIB_MISSING_VALUE;
		      }
		      else {
		          grib_msg->grids.gridpoints[l]=pval+groups.ref_vals[n]+groups.omin;
                    }
                    ++i;
		  }
		  ++l;
		}
	  }
	  else { // constant group XXX bitmap?
            for (int i=0; i < groups.lengths[n]; ) {
		  if (grib_msg->md.bitmap != NULL && grib_msg->md.bitmap[l] == 0) {
		    grib_msg->grids.gridpoints[l]=GRIB_MISSING_VALUE;
		  }
		  else {
		    if (groups.ref_vals[n] == groups.miss_val) {
                      grib_msg->grids.gridpoints[l]=GRIB_MISSING_VALUE;
                    }
                    else {
		        grib_msg->grids.gridpoints[l]=groups.ref_vals[n]+groups.omin;
                    }
		    ++i;
		  }
		  ++l;
            }
	  }
	}

	for (; l < grib_msg->md.nx*grib_msg->md.ny; ++l) {
	  grib_msg->grids.gridpoints[l]=GRIB_MISSING_VALUE;
	}

	if (grib_msg->md.drs_templ_num == 3) {
      	   if (groups.first_vals != nullptr) {
      	      for (n=grib_msg->md.complex_pack.spatial_diff.order-1; n > 0; --n) {
  	         lastgp=groups.first_vals[n]-groups.first_vals[n-1];
  	         for (l=0,m=0; l < grib_msg->md.nx*grib_msg->md.ny; ++l) {
  	            if (grib_msg->grids.gridpoints[l] != GRIB_MISSING_VALUE) {
                       if (m >= grib_msg->md.complex_pack.spatial_diff.order) {
                          grib_msg->grids.gridpoints[l]+=lastgp;
                          lastgp=grib_msg->grids.gridpoints[l];
                       }
                       ++m;
                    }
  	         }
              }
  	   }
  	   for (l=0,m=0,lastgp=0; l < grib_msg->md.nx*grib_msg->md.ny; ++l) {
  	     if (grib_msg->grids.gridpoints[l] != GRIB_MISSING_VALUE) {
  	   	if (m < grib_msg->md.complex_pack.spatial_diff.order) {
  	   	  grib_msg->grids.gridpoints[l]=grib_msg->md.R+groups.first_vals[m]*E/D;
  	   	  lastgp=grib_msg->md.R*D/E+groups.first_vals[m];
  	   	}
  	   	else {
  	   	  lastgp+=grib_msg->grids.gridpoints[l];
  	   	  grib_msg->grids.gridpoints[l]=lastgp*E/D;
  	   	}
  	   	++m;
  	     }
  	   }
  	   delete [] groups.first_vals;
	}
	else for (l=0; l < grib_msg->md.nx*grib_msg->md.ny; ++l) {
  	   if (grib_msg->grids.gridpoints[l] != GRIB_MISSING_VALUE) {
                grib_msg->grids.gridpoints[l]= grib_msg->md.R+grib_msg->grids.gridpoints[l]*E/D;
           }
	}
	delete [] groups.ref_vals;
	delete [] groups.widths;
	delete [] groups.lengths;
 	break;
#ifdef JASPER
    case 40:
    case 40000:
        int len, *jvals, cnt;
        int npoints;
	getBits(grib_msg->buffer,&len,grib_msg->offset,32);
	if (len < 5)
	    return false;
	len=len-5;
	npoints = grib_msg->md.ny*grib_msg->md.nx;
	jvals= new int[npoints];
	grib_msg->grids.gridpoints= new double[npoints];
	if (len > 0)
	  dec_jpeg2000((char *)&grib_msg->buffer[grib_msg->offset/8+5],len,jvals);
	cnt=0;
	for (l=0; l < npoints; l++) {
	  if (grib_msg->md.bitmap == NULL || grib_msg->md.bitmap[l] == 1) {
	    if (len == 0)
		jvals[cnt]=0;
	    grib_msg->grids.gridpoints[l]=grib_msg->md.R+jvals[cnt++]*E/D;
	  }
	  else
	    grib_msg->grids.gridpoints[l]=GRIB_MISSING_VALUE;
	}
	delete [] jvals;
	break;
#endif
    default:
        erreur("Unknown packing %d", grib_msg->md.drs_templ_num);
        break;
  }
  return true;
}

static zuchar GRBV2_TO_DATA(int productDiscipline, int dataCat, int dataNum)
{
    zuchar ret = 255;
    // printf("search %d %d %d\n", productDiscipline, dataCat,  dataNum);
    switch (productDiscipline) { // TABLE 4.2
    case 0:      // Meteorological products
        switch (dataCat) {
        case 0:  // Temperature
            switch (dataNum) {
            case 0: ret = GRB_TEMP; break; // DATA_TO_GRBV2[DATA_TEMP] = grb2DataType(0,0,0);
            case 2: ret= GRB_TPOT; break;  // DATA_TO_GRBV2[DATA_TEMP_POT] = grb2DataType(0,0,2);
            case 4: ret = GRB_TMAX; break; // DATA_TO_GRBV2[DATA_TMAX] = grb2DataType(0,0,4);
            case 5: ret = GRB_TMIN; break; // DATA_TO_GRBV2[DATA_TMIN] = grb2DataType(0,0,5);
            case 6: ret = GRB_DEWPOINT; break; //DATA_TO_GRBV2[DATA_DEWPOINT] = grb2DataType(0,0,6);
            }
            break;
        case 1: // dataCat Moisture
            switch (dataNum) {
            case 0: ret = GRB_HUMID_SPEC; break; //DATA_TO_GRBV2[DATA_HUMID_SPEC] = grb2DataType(0,1,0);
            case 1: ret = GRB_HUMID_REL; break; // DATA_TO_GRBV2[DATA_HUMID_REL] = grb2DataType(0,1,1);
            case 7: ret= GRB_PRECIP_RATE; break; // DATA_TO_GRBV2[DATA_PRECIP_RATE] = grb2DataType(0,1,7);
            case 49:							 // Total Water Precipitation (Meteo France Arome 0.01
            case 52:                             // Total precipitation rate kg m–2 s–1
            case 8: ret = GRB_PRECIP_TOT; break; // DATA_TO_GRBV2[DATA_PRECIP_TOT] = grb2DataType(0,1,8);
            case 11: ret = GRB_SNOW_DEPTH; break; // DATA_TO_GRBV2[DATA_SNOW_DEPTH] = grb2DataType(0,1,11);
            case 193: ret = GRB_FRZRAIN_CATEG; break; // DATA_TO_GRBV2[DATA_FRZRAIN_CATEG] = grb2DataType(0,1,193);
            case 195: ret = GRB_SNOW_CATEG; break; //DATA_TO_GRBV2[DATA_SNOW_CATEG] = grb2DataType(0,1,195);
            }
            break;
        case 2: // dataCat  Momentum 
            switch (dataNum) {
            case 0: ret = GRB_WIND_DIR; break;
            case 1: ret = GRB_WIND_SPEED; break;
            case 2: ret = GRB_WIND_VX; break; // DATA_TO_GRBV2[DATA_WIND_VX] = grb2DataType(0,2,2);
            case 3: ret = GRB_WIND_VY; break; // DATA_TO_GRBV2[DATA_WIND_VY] = grb2DataType(0,2,3);
            case 22: ret = GRB_WIND_GUST; break; // 
            }
            break;
        case 3: // dataCat mass
            switch (dataNum) {
            case 0: ret = GRB_PRESSURE; break; //DATA_TO_GRBV2[DATA_PRESSURE] = grb2DataType(0,3,0);
            case 1: ret = GRB_PRESSURE; break; // PRSMSL //DATA_TO_GRBV2[DATA_PRESSURE] = grb2DataType(0,3,0);
            case 5: ret = GRB_GEOPOT_HGT; break; // DATA_TO_GRBV2[DATA_GEOPOT_HGT]= grb2DataType(0,3,5);

            case 192: ret = GRB_PRESSURE; break; //DATA_TO_GRBV2[DATA_MSLET] = grb2DataType(0,3,192);
            }
            break;
        case 6: // dataCat
            switch (dataNum) {
            case 1: ret = GRB_CLOUD_TOT; break; // DATA_TO_GRBV2[DATA_CLOUD_TOT] = grb2DataType(0,6,1);
            }
            break;
        case 7:// dataCat
            switch (dataNum) {
            // case 7: ret =  GRB_?; break // DATA_TO_GRBV2[DATA_CIN] = grb2DataType(0,7,7);
            case 6: ret = GRB_CAPE; break; // DATA_TO_GRBV2[DATA_CAPE] = grb2DataType(0,7,6);
            }
            break;
        case 16: // Meteorological products, Forecast Radar Imagery category
            switch (dataNum) {
            case 196: ret =  GRB_COMP_REFL; break; // = grb2DataType(0,16, 196);
            }
            break;
        }
        break;
    case 10: // productDiscipline Oceanographic products 
        switch (dataCat) {
        case 0:         // waves
#if 0        
            switch (dataNum) {
            case 3: ret= GRB_WVHGT; break; //DATA_TO_GRBV2[DATA_WAVES_SIG_HGT_COMB] = grb2DataType(10,0,3);
            DATA_TO_GRBV2[DATA_WAVES_WND_DIR] = grb2DataType(10,0,4);
            DATA_TO_GRBV2[DATA_WAVES_WND_HGT] = grb2DataType(10,0,5);
            DATA_TO_GRBV2[DATA_WAVES_WND_PERIOD] = grb2DataType(10,0,6);
            DATA_TO_GRBV2[DATA_WAVES_SWL_DIR] = grb2DataType(10,0,7);
            DATA_TO_GRBV2[DATA_WAVES_SWL_HGT] = grb2DataType(10,0,8);
            DATA_TO_GRBV2[DATA_WAVES_SWL_PERIOD] = grb2DataType(10,0,9);
            DATA_TO_GRBV2[DATA_WAVES_PRIM_DIR] = grb2DataType(10,0,10);
            DATA_TO_GRBV2[DATA_WAVES_PRIM_PERIOD] = grb2DataType(10,0,11);
            DATA_TO_GRBV2[DATA_WAVES_SEC_DIR] = grb2DataType(10,0,12);
            DATA_TO_GRBV2[DATA_WAVES_SEC_PERIOD] = grb2DataType(10,0,13);
            }
#endif

            switch (dataNum) {
                case 3: ret= GRB_HTSGW; break; // Significant Height of Combined Wind Waves and Swell
                case 4: ret= GRB_WVDIR; break; // Direction of Wind Waves
                case 5: ret= GRB_WVHGT; break; // Significant Height of Wind Waves
                case 6: ret= GRB_WVPER; break; // Mean Period of Wind Waves
                case 14: ret= GRB_DIR; break;  // Direction of Combined Wind Waves and Swell
                case 15: ret= GRB_PER; break;  // Mean Period of Combined Wind Waves and Swell
            }
            break;

        case 1: // Currents 
            switch (dataNum) {
                case 0: ret = GRB_CUR_DIR; break;
                case 1: ret = GRB_CUR_SPEED; break;
                case 2: ret = GRB_UOGRD; break; // DATA_TO_GRBV2[DATA_CURRENT_VX] = grb2DataType(10,1,2);
                case 3: ret = GRB_VOGRD; break; // DATA_TO_GRBV2[DATA_CURRENT_VY] = grb2DataType(10,1,3);
            }
            break;
         case 3: // Surface Properties
            switch (dataNum) {
                case 0: ret = GRB_WTMP; break; // DATA_TO_GRBV2[DATA_CURRENT_VX] = grb2DataType(10,1,2);
            }
            break;
            
        }
        break;
    }
#if 1
    if (ret == 255) {
        erreur("unknown Discipline %d dataCat %d dataNum %d", productDiscipline,  dataCat, dataNum);
    }
#endif    
    return ret;    
}

/** Return UINT_MAX on errors. */
static int mapStatisticalEndTime(GRIBMessage *grid)
{
   // lovely md.fcst_time is in grid->md.time_unit but md.stat_proc.t[0].time_length is in grid->md.stat_proc.t[0].time_unit
   // not always the same.
  if (grid->md.time_unit == grid->md.stat_proc.t[0].time_unit) switch (grid->md.time_unit) { // table 4.4
    case 0:  // minute
	// return (grid->md.stat_proc.etime/100 % 100)-(grid->time/100 % 100);
    case 1:  // hour
         return grid->md.fcst_time +grid->md.stat_proc.t[0].time_length;
	 // return (grid->md.stat_proc.etime/10000- grid->time/10000);
    case 2:  // Day
	return (grid->md.stat_proc.edy -grid->dy);
    case 3:
	return (grid->md.stat_proc.emo -grid->mo);
    case 4:
	return (grid->md.stat_proc.eyr -grid->yr);
    default:
	fprintf(stderr,"Unable to map end time with units %d to GRIB1\n",grid->md.time_unit);
	return UINT_MAX;
  }

  if (grid->md.time_unit == 0 && grid->md.stat_proc.t[0].time_unit == 1) {
         // in minute + hourly increment
         return grid->md.fcst_time +grid->md.stat_proc.t[0].time_length *60;
  }

  if (grid->md.time_unit == 1 && grid->md.stat_proc.t[0].time_unit == 0 && (grid->md.stat_proc.t[0].time_unit  % 60) != 0 ) {
          // convert in hour
         return grid->md.fcst_time +grid->md.stat_proc.t[0].time_length /60;
  }

  fprintf(stderr, "Unable to map end time %d %d %d %d \n", grid->md.time_unit, grid->md.stat_proc.t[0].time_unit, grid->md.fcst_time, 
            grid->md.stat_proc.t[0].time_length);
  return UINT_MAX;
}

// map GRIB2 msg time to GRIB1 P1 and P2 in sec
static bool mapTimeRange(GRIBMessage *grid, zuint *p1, zuint *p2, zuchar *t_range,int *n_avg,int *n_missing, int center)
{
  switch (grid->md.pds_templ_num) {
    case 0:
    case 1:
    case 2:
    case 15:
	*t_range=0;
	*p1=grid->md.fcst_time;
	*p2=0;
	*n_avg=*n_missing=0;
	break;
    case 8:
    case 11:
    case 12:
	if (grid->md.stat_proc.num_ranges > 1) {
	  if (center == 7 && grid->md.stat_proc.num_ranges == 2) {
/* NCEP CFSR monthly grids */
	    *p2=grid->md.stat_proc.t[0].incr_length;
	    *p1=*p2 -grid->md.stat_proc.t[1].time_length;
	    *n_avg=grid->md.stat_proc.t[0].time_length;
	    switch (grid->md.stat_proc.t[0].proc_code) {
		case 193:
		  *t_range=113;
		  break;
		case 194:
		  *t_range=123;
		  break;
		case 195:
		  *t_range=128;
		  break;
		case 196:
		  *t_range=129;
		  break;
		case 197:
		  *t_range=130;
		  break;
		case 198:
		  *t_range=131;
		  break;
		case 199:
		  *t_range=132;
		  break;
		case 200:
		  *t_range=133;
		  break;
		case 201:
		  *t_range=134;
		  break;
		case 202:
		  *t_range=135;
		  break;
		case 203:
		  *t_range=136;
		  break;
		case 204:
		  *t_range=137;
		  break;
		case 205:
		  *t_range=138;
		  break;
		case 206:
		  *t_range=139;
		  break;
		case 207:
		  *t_range=140;
		  break;
		default:
		  fprintf(stderr,"Unable to map NCEP statistical process code %d to GRIB1\n",grid->md.stat_proc.t[0].proc_code);
		  return false;
	    }
	  }
	  else {
	    fprintf(stderr,"Unable to map multiple statistical processes to GRIB1\n");
	    return false;
	  }
	}
	else {
	  switch (grid->md.stat_proc.t[0].proc_code) {
	    case 0:
	    case 1:
	    case 4:
		switch (grid->md.stat_proc.t[0].proc_code) {
		  case 0: /* average */
		    *t_range=3;
		    break;
		  case 1: /* accumulation */
		    *t_range=4;
		    break;
		  case 4: /* difference */
		    *t_range=5;
		    break;
		}
		*p1=grid->md.fcst_time;
		*p2=mapStatisticalEndTime(grid);
                if (*p2 == UINT_MAX) {
                    return false;
                }
		if (grid->md.stat_proc.t[0].incr_length == 0)
		  *n_avg=0;
		else {
		  fprintf(stderr,"Unable to map discrete processing to GRIB1\n");
		  return false;
		}
		break;

	    case 2: // maximum
	    case 3: // minimum 
		*t_range=2;
		*p1=grid->md.fcst_time;
		*p2=mapStatisticalEndTime(grid);
                if (*p2 == UINT_MAX) {
                    return false;
                }
		if (grid->md.stat_proc.t[0].incr_length == 0)
		  *n_avg=0;
		else {
		  fprintf(stderr,"Unable to map discrete processing to GRIB1\n");
		  return false;
		}
		break;
	    default:
// patch for NCEP grids
		if (grid->md.stat_proc.t[0].proc_code == 255 && center == 7) {
 		  if (grid->disc == 0) {
		    if (grid->md.param_cat == 0) {
			switch (grid->md.param_num) {
			  case 4:
			  case 5:
			    *t_range=2;
			    *p1=grid->md.fcst_time;
			    *p2=mapStatisticalEndTime(grid);
                            if (*p2 == UINT_MAX) {
                                return false;
                            }
			    if (grid->md.stat_proc.t[0].incr_length == 0)
				*n_avg=0;
			    else {
				fprintf(stderr,"Unable to map discrete processing to GRIB1\n");
				return false;
			    }
			    break;
			}
		    }
		  }
		}
		else {
		  fprintf(stderr,"Unable to map statistical process %d to GRIB1\n",grid->md.stat_proc.t[0].proc_code);
		  return false;
		}
	  }
	}
	*n_missing=grid->md.stat_proc.nmiss;
	break;
    default:
	fprintf(stderr,"Unable to map time range for Product Definition Template %d into GRIB1\n",grid->md.pds_templ_num);
	return false;
  }
  return true;
}

//-------------------------------------------------------------------------------
// Adjust data type from different mete center
//-------------------------------------------------------------------------------
void  GribV2Record::translateDataType()
{
    this->knownData = true;
    dataCenterModel = OTHER_DATA_CENTER;
    //------------------------
    // NOAA GFS
    //------------------------
    if (dataType == GRB_PRECIP_RATE) {	// mm/s -> mm/h
        multiplyAllData( 3600.0 );
    }
    if ( idCenter==7 && idModel==2 )		// NOAA
    {
        dataCenterModel = NOAA_GFS;
        // altitude level (entire atmosphere vs entire atmosphere considered as 1 level)
        if (levelType == LV_ATMOS_ENT) {
            levelType = LV_ATMOS_ALL;
        }
        if (dataType == GRB_TEMP          //gfs Water surface Temperature
            && levelType == LV_GND_SURF
            && levelValue == 0) dataType = GRB_WTMP;
    }
    //------------------------
	//DNMI-NEurope.grb
    //------------------------
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
    else if (idCenter==84 && idModel <= 5 && idGrid==0)
    {
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
        switch (levelType) {
            case 100: // LV_ISOBARIC
                /* GRIB1 is in hectoPascal 
                   GRIB2 in Pascal, convert to GRIB1
                */
                levelValue = levelValue /100;
                break;
            case 103: levelType = LV_ABOV_GND;break;
            case 101: levelType = LV_MSL;break;
        }
        switch (getDataType()) {
            case GRB_WIND_GUST:
                levelType  = LV_GND_SURF;
                levelValue = 0;
                break;
            case GRB_UOGRD:
            case GRB_VOGRD:
                break;
            case GRB_WVHGT:
            case GRB_HTSGW:
            case GRB_WVDIR:
            case GRB_WVPER:
            case GRB_DIR:
            case GRB_PER:
                levelType  = LV_GND_SURF;
                levelValue = 0;
                break;
        }
    }
    // this->print();
}

// -------------------------------------
void GribV2Record::readDataSet(ZUFILE* file)
{
    bool skip = false;
    bool DS = false;
    int len, sec_num;

    data    = NULL;
    BMSbits = NULL;
    hasBMS = false;
    knownData = false;
    IsDuplicated = false;

    while (strncmp(&((char *)grib_msg->buffer)[grib_msg->offset/8],"7777",4) != 0) {
        DS = false;
        getBits(grib_msg->buffer, &len, grib_msg->offset, 32);
        getBits(grib_msg->buffer, &sec_num, grib_msg->offset +4*8, 8);
        switch (sec_num) {
	case 2: //  Section 2: Local Use Section
	     if (skip == true)  break;
             ok = unpackLUS(grib_msg);
             break;
	case 3: //  Section 3: Grid Definition Section
	     if (skip == true)  break;
	     ok = unpackGDS(grib_msg);
	     if (ok) {
	         Ni = grib_msg->md.nx;
	         Nj = grib_msg->md.ny;
	         La1 = grib_msg->md.slat;
	         Lo1 = grib_msg->md.slon;
	         La2 = grib_msg->md.lats.elat;
	         Lo2 = grib_msg->md.lons.elon;
	         Di = grib_msg->md.xinc.loinc;
	         Dj = grib_msg->md.yinc.lainc;
                 scanFlags = grib_msg->md.scan_mode;
                 isScanIpositive = (scanFlags&0x80) ==0;
                 isScanJpositive = (scanFlags&0x40) !=0;
                 isAdjacentI     = (scanFlags&0x20) ==0;
                 if (Lo1>=0 && Lo1<=180 && Lo2<0)
                     Lo2 += 360.0;    // cross the 180 deg meridien,beetwen alaska and russia
	                     
	         if (isScanIpositive) while ( Lo1> Lo2 ) {   // horizontal size > 360 °
	             Lo1 -= 360.0;
                 }
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
	     }
	     break;
	case 4: //  Section 4: Product Definition Section 
	     if (skip == true)  break;
	     ok = unpackPDS(grib_msg);
	     if (ok) {
	         // printf("template %d 0 meteo data cat %d data num %d\n", grib_msg->md.pds_templ_num, grib_msg->md.param_cat, grib_msg->md.param_num);
	         productTemplate = grib_msg->md.pds_templ_num;
	         dataCat = grib_msg->md.param_cat;
                 dataNum = grib_msg->md.param_num;
                 dataType= GRBV2_TO_DATA(productDiscipline,dataCat,dataNum);
                 if (dataType == 255) {
                     //printf("unused data type, skip\n");
                     skip = true;
                     break;
                 }
                                            
	         levelType = grib_msg->md.lvl1_type;
	         levelValue = grib_msg->md.lvl1;
	         if (grib_msg->md.lvl2_type == 8 && grib_msg->md.lvl1_type == 1) {
	             // cf table 4.5:  8 Nominal top of the atmosphere 
	             levelType = LV_ATMOS_ALL;
	             levelValue = 0.;
	         }
	         int n_avg, n_missing;
	          
	         if (!mapTimeRange(grib_msg, &periodP1 , &periodP2, &timeRange , &n_avg, &n_missing, idCenter)) {
	             skip = true;
	             break;
                 }
	         periodsec = periodSeconds(grib_msg->md.time_unit, periodP1, periodP2, timeRange);
	         setRecordCurrentDate(makeDate(refyear,refmonth,refday,refhour,refminute,periodsec));
	         //printf("%d %d %d %d %d %d \n", refyear,refmonth,refday,refhour,refminute,periodsec);
	         //printf("%d Periode %d P1=%d p2=%d %s\n", grib_msg->md.time_unit, periodsec, periodP1,periodP2, strCurDate);
	         
	     }
	     break;
	case 5: //  Section 5: Data Representation Section 
	     if (skip == true)  break;
	     ok = unpackDRS(grib_msg);
	     break;
	case 6: //  Section 6: Bit-Map Section 
	     if (skip == true)  break;
	     ok = unpackBMS(grib_msg);
	     if (ok) {
	        if (grib_msg->md.bmssize != 0) {
	             hasBMS = true;
	             BMSsize = grib_msg->md.bmssize;
	             BMSbits = new zuchar[grib_msg->md.bmssize];
	             memcpy (BMSbits, grib_msg->md.bms, grib_msg->md.bmssize);
                }
	     }
	     break;
	case 7:  // Section 7: Data Section
	     if (skip == false) {
    	         ok = unpackDS(grib_msg);
    	         if (ok) {
	             data = grib_msg->grids.gridpoints;
	             grib_msg->grids.gridpoints = 0;
                 }
	     }
	     if (grib_msg->num_grids != 1)
    	         DS = true;
	     break;
        }
        grib_msg->offset += len*8;
        if (ok == false || DS == true )
            break;
    }
    
    //ok = false;
if (false) {
//if (true) {
printf("==== GV2 %d\n", ok);
printf("Lo1=%f Lo2=%f    La1=%f La2=%f\n", Lo1,Lo2,La1,La2);
printf("Lo1=%f Lo2=%f    La1=%f La2=%f\n", grib_msg->md.slon, grib_msg->md.lons.elon, grib_msg->md.slat, grib_msg->md.lats.elat);
printf("Ni=%d Nj=%d\n", Ni,Nj);
printf("hasDiDj=%d Di,Dj=(%f %f)\n", hasDiDj, Di,Dj);
printf("isScanIpositive=%d isScanJpositive=%d isAdjacentI=%d\n",isScanIpositive,isScanJpositive,isAdjacentI );
printf("hasBMS=%d\n", hasBMS);
}
    if (ok) {
        if (!skip) 
        {
		translateDataType();
		setDataType(dataType);
        }
    }
    if (!ok || !DS || strncmp(&((char *)grib_msg->buffer)[grib_msg->offset/8],"7777",4) == 0) {
        delete grib_msg;
        grib_msg = 0;
    }
}

// -----------------
GribV2Record::GribV2Record(ZUFILE* file, int id_)
{
    id = id_;
    seekStart = zu_tell(file);           // moved to section 0 read
    data    = NULL;
    BMSsize = 0;
    BMSbits = NULL;
    hasBMS = false;
    eof     = false;
    knownData = false;
    IsDuplicated = false;
    long start = seekStart;

    grib_msg = new GRIBMessage();

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
    ok = readGribSection0_IS(file, b_haveReadGRIB ); // Section 0: Indicator Section
    
    int len, sec_num;    
    if (ok) {
        unpackIDS(grib_msg);  // Section 1: Identification Section
        int off;
        /* find out how many grids are in this message */
        off = grib_msg->offset /8;
        while (strncmp(&((char *)grib_msg->buffer)[off], "7777", 4) != 0) {
            len = uint4(grib_msg->buffer +off);
            sec_num = grib_msg->buffer[off+4];
            if (sec_num == 7)
                grib_msg->num_grids++;
            off += len;
        }
    }
    else {
        // seek back if V1
        (void)zu_seek(file, start, SEEK_SET);
        return;
    }   
    refyear  = grib_msg->yr;
    refmonth = grib_msg->mo;
    refday   = grib_msg->dy;
    refhour  = grib_msg->time /10000;
    refminute = (grib_msg->time/100) % 100;
    refDate = makeDate(refyear,refmonth,refday,refhour,refminute,0);
    sprintf(strRefDate, "%04d-%02d-%02d %02d:%02d", refyear,refmonth,refday,refhour,refminute);
    idCenter = grib_msg->center_id;
    idModel  = grib_msg->table_ver;
    idGrid   = 0; // FIXME data1[6];
    productDiscipline = grib_msg->disc;
    readDataSet(file);
}

// ---------------------------------------
bool GribV2Record::hasMoreDataSet() const
{
    return grib_msg && grib_msg->num_grids != 1?true:false;
}

// ---------------------------------------
GribV2Record *GribV2Record::GribV2NextDataSet(ZUFILE* file, int id_)
{
    GribV2Record *rec1 = new GribV2Record(*this);
    // XXX should have a shallow copy constructor 
    delete [] rec1->data;
    delete [] rec1->BMSbits;
    // new records take ownership
    this->grib_msg = 0;
    rec1->id = id_;
    rec1->readDataSet(file);
    return rec1;
}

//-------------------------------------------------------------------------------
// Constructeur de recopie
//-------------------------------------------------------------------------------
#pragma warning(disable: 4717)
GribV2Record::GribV2Record(const GribRecord &rec) : GribRecord(rec)
{
    *this = rec;
    #pragma warning(default: 4717)
}

GribV2Record::~GribV2Record()
{
    delete grib_msg;
}

//==============================================================
// Lecture des données
//==============================================================
//----------------------------------------------
// SECTION 0: THE INDICATOR SECTION (IS)
//----------------------------------------------
static bool unpackIS(ZUFILE* fp, GRIBMessage *grib_msg)
{
  unsigned char temp[16];
  int status;
  size_t num;

  if (grib_msg->buffer != NULL) {
    delete [] grib_msg->buffer;
    grib_msg->buffer=NULL;
  }
  grib_msg->num_grids = 0;

  if ( (status = zu_read(fp, &temp[4], 12)) != 12)
  {
    return false;
  }
  grib_msg->disc = temp[6];
  grib_msg->ed_num = temp[7];
  
  //  Bail out early if this is not GRIB2
  if(grib_msg->ed_num != 2)
      return false;
  
  getBits(temp,&grib_msg->total_len,96,32);
  // too small or overflow
  if ( grib_msg->total_len < 16 || grib_msg->total_len > (INT_MAX - 4))
      return false;

  grib_msg->md.nx = grib_msg->md.ny = 0;
  grib_msg->buffer = new unsigned char[grib_msg->total_len+4];
  memcpy(grib_msg->buffer,temp,16);
  num = grib_msg->total_len -16;

  status = zu_read(fp, &grib_msg->buffer[16], num);
  if (status != (int)num)
    return false;

  if (strncmp(&((char *)grib_msg->buffer)[grib_msg->total_len-4],"7777",4) != 0)
        fprintf(stderr,"Warning: no end section found\n");

  grib_msg->offset=128;
  return true;
}

bool GribV2Record::readGribSection0_IS(ZUFILE* file, bool b_skip_initial_GRIB) {
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
      if (strncmp(strgrib, "GRIB", 4) != 0)  {
            printf("readGribSection0_IS(): Unknown file header : %c%c%c%c\n", strgrib[0],strgrib[1],strgrib[2],strgrib[3]);
            ok = false;
            eof = true;
            return false;
      }
    }

    seekStart = zu_tell(file) - 4;
    // totalSize = readInt3(file);
    if (unpackIS(file , grib_msg) == false) {
        ok = false;
        eof = true;
        return false;
    }

    editionNumber = grib_msg->ed_num;
    if (editionNumber != 2)  {
        ok = false;
        eof = true;
        return false;
    }

    return true;
}

//==============================================================
// Fonctions utiles
//==============================================================
zuint GribV2Record::periodSeconds(zuchar unit,zuint P1,zuint P2,zuchar range) {
    zuint res, dur;

    switch (unit) {
        case 0: //      Minute
            res = 60; break;
        case 1: //      Hour
            res = 3600; break;
        case 2: //      Day
            res = 86400; break;
        case 10: //     3 hours
            res = 10800; break;
        case 11: //     6 hours
            res = 21600; break;
        case 12: //     12 hours
            res = 43200; break;
        case 13: // Second
            res = 1; break;
        case 3: //      Month
        case 4: //      Year
        case 5: //      Decade (10 years)
        case 6: //      Normal (30 years)
        case 7: //      Century (100 years)
        default:
            erreur("id=%d: unknown time unit in PDS b18=%d",id,unit);
            res = 0;
            ok = false;
    }
    grib_debug("id=%d: PDS unit %d (time range) b21=%d %d P1=%d P2=%d\n",id,unit, range,res,P1,P2);
    dur = 0;

    switch (range) {
        case 0:
            dur = (zuint)P1; break;
        case 1:
            dur = 0; break;

        case 2:
        case 3: // Average  (reference time + P1 to reference time + P2)
            // dur = ((zuint)P1+(zuint)P2)/2; break;     // TODO
            dur = (zuint)P2; break;

         case 4: // Accumulation  (reference time + P1 to reference time + P2)
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

