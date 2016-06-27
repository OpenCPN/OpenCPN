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
**
    copyright ?
*/
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

class GRIBMetadata {
public:
  GRIBMetadata() : bitmap(0), bms(0) {
    stat_proc.proc_code = 0;
    stat_proc.incr_type = 0; 
    stat_proc.time_unit = 0; 
    stat_proc.time_length = 0;
    stat_proc.incr_unit = 0;
    stat_proc.incr_length = 0;
    lvl1_type = 0;
    lvl2_type = 0;
    lvl1 = 0.;
    lvl2 = 0.;
  };

  ~GRIBMetadata() {
    delete [] stat_proc.proc_code;
    delete [] stat_proc.incr_type;
    delete [] stat_proc.time_unit;
    delete [] stat_proc.time_length;
    delete [] stat_proc.incr_unit;
    delete [] stat_proc.incr_length;
    delete [] bitmap;
    delete [] bms;
  };

  int gds_templ_num;
  int earth_shape;
  int nx,ny;
  double slat,slon,latin1,latin2,splat,splon;
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
    int *proc_code,*incr_type,*time_unit,*time_length,*incr_unit,*incr_length;
  } stat_proc;
  struct {
    int stat_proc,type,num_points;
  } spatial_proc;
  int drs_templ_num;
  float R;
  int E,D,num_packed,pack_width;
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
  GRIBMessage() : buffer(0), grids(0) {};
  ~GRIBMessage() { 
      delete [] grids; 
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
  GRIB2Grid *grids;
};


#ifdef JASPER
int dec_jpeg2000(char *injpc,int bufsize,int *outfld)
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

static inline void getBits(unsigned char *buf, int *loc, size_t first, size_t nbBits)
{
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

/* length of the IDS */
  getBits(grib_msg->buffer,&length,grib_msg->offset,32);
/* center ID */
  getBits(grib_msg->buffer,&grib_msg->center_id,grib_msg->offset + 5*8,16);
/* sub-center ID */
  getBits(grib_msg->buffer,&grib_msg->sub_center_id,grib_msg->offset +7*8,16);
/* table version */
  getBits(grib_msg->buffer,&grib_msg->table_ver,grib_msg->offset+72,8);
/* local table version */
  getBits(grib_msg->buffer,&grib_msg->local_table_ver,grib_msg->offset+80,8);
/* significance of reference time */
  getBits(grib_msg->buffer,&grib_msg->ref_time_type,grib_msg->offset+88,8);
/* year */
  getBits(grib_msg->buffer,&grib_msg->yr,grib_msg->offset+96,16);
/* month */
  getBits(grib_msg->buffer,&grib_msg->mo,grib_msg->offset+112,8);
/* day */
  getBits(grib_msg->buffer,&grib_msg->dy,grib_msg->offset+120,8);
/* hours */
  getBits(grib_msg->buffer,&hh,grib_msg->offset+128,8);
/* minutes */
  getBits(grib_msg->buffer,&mm,grib_msg->offset+136,8);
/* seconds */
  getBits(grib_msg->buffer,&ss,grib_msg->offset+144,8);
  grib_msg->time=hh*10000+mm*100+ss;
/* production status */
  getBits(grib_msg->buffer,&grib_msg->prod_status,grib_msg->offset+152,8);
/* type of data */
  getBits(grib_msg->buffer,&grib_msg->data_type,grib_msg->offset+160,8);
  grib_msg->offset+=length*8;
}

static bool unpackLUS(GRIBMessage *grib_msg)
{
    return true;
}

static bool unpackGDS(GRIBMessage *grib_msg)
{
  int src,num_in_list;
  int sign,value;

/* source of grid definition */
  getBits(grib_msg->buffer,&src,grib_msg->offset+40,8);
  if (src != 0) {
    fprintf(stderr,"Don't recognize predetermined grid definitions");
    return false;
  }
/* quasi-regular grid indication */
  getBits(grib_msg->buffer,&num_in_list,grib_msg->offset+80,8);
  if (num_in_list > 0) {
    fprintf(stderr,"Unable to unpack quasi-regular grids");
    return false;
  }
/* grid definition template number */
  getBits(grib_msg->buffer,&grib_msg->md.gds_templ_num,grib_msg->offset+96,16);
  switch (grib_msg->md.gds_templ_num) {
/* Latitude/longitude grid */
    case 0:
    case 40:
/* shape of the earth */
	getBits(grib_msg->buffer,&grib_msg->md.earth_shape,grib_msg->offset+112,8);
/* number of latitudes */
	getBits(grib_msg->buffer,&grib_msg->md.nx,grib_msg->offset+240,32);
/* number of longitudes */
	getBits(grib_msg->buffer,&grib_msg->md.ny,grib_msg->offset+272,32);
/* latitude of first gridpoint */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+368,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+369,31);
	grib_msg->md.slat=value/1000000.;
	if (sign == 1)
	  grib_msg->md.slat=-grib_msg->md.slat;
/* longitude of first gridpoint */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+400,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+401,31);
	grib_msg->md.slon=value/1000000.;
	if (sign == 1)
	  grib_msg->md.slon=-grib_msg->md.slon;
/* resolution and component flags */
	getBits(grib_msg->buffer,&grib_msg->md.rescomp,grib_msg->offset+432,8);
/* latitude of last gridpoint */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+440,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+441,31);
	grib_msg->md.lats.elat=value/1000000.;
	if (sign == 1)
	  grib_msg->md.lats.elat=-grib_msg->md.lats.elat;
/* longitude of last gridpoint */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+472,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+473,31);
	grib_msg->md.lons.elon=value/1000000.;
	if (sign == 1)
	  grib_msg->md.lons.elon=-grib_msg->md.lons.elon;
/* longitude increment */
	getBits(grib_msg->buffer,&value,grib_msg->offset+504,32);
	grib_msg->md.xinc.loinc=value/1000000.;
/* latitude increment */
	getBits(grib_msg->buffer,&value,grib_msg->offset+536,32);
	if (grib_msg->md.gds_templ_num == 0)
	  grib_msg->md.yinc.lainc=value/1000000.;
/* scanning mode flag */
	getBits(grib_msg->buffer,&grib_msg->md.scan_mode,grib_msg->offset+568,8);
	break;
/* Lambert conformal grid */
    case 30:
	getBits(grib_msg->buffer,&grib_msg->md.earth_shape,grib_msg->offset+112,8);
/* number of points along a parallel */
	getBits(grib_msg->buffer,&grib_msg->md.nx,grib_msg->offset+240,32);
/* number of points along a meridian */
	getBits(grib_msg->buffer,&grib_msg->md.ny,grib_msg->offset+272,32);
/* latitude of first gridpoint */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+304,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+305,31);
	grib_msg->md.slat=value/1000000.;
	if (sign == 1)
	  grib_msg->md.slat=-grib_msg->md.slat;
/* longitude of first gridpoint */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+336,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+337,31);
	grib_msg->md.slon=value/1000000.;
	if (sign == 1)
	  grib_msg->md.slon=-grib_msg->md.slon;
/* resolution and component flags */
	getBits(grib_msg->buffer,&grib_msg->md.rescomp,grib_msg->offset+368,8);
/* LaD */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+376,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+377,31);
	grib_msg->md.lats.lad=value/1000000.;
	if (sign == 1)
	  grib_msg->md.lats.lad=-grib_msg->md.lats.lad;
/* LoV */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+408,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+409,31);
	grib_msg->md.lons.lov=value/1000000.;
	if (sign == 1)
	  grib_msg->md.lons.lov=-grib_msg->md.lons.lov;
/* x-direction increment */
	getBits(grib_msg->buffer,&value,grib_msg->offset+440,32);
	grib_msg->md.xinc.dxinc=value/1000.;
/* y-direction increment */
	getBits(grib_msg->buffer,&value,grib_msg->offset+472,32);
	grib_msg->md.yinc.dyinc=value/1000.;
/* projection center flag */
	getBits(grib_msg->buffer,&grib_msg->md.proj_flag,grib_msg->offset+504,8);
/* scanning mode flag */
	getBits(grib_msg->buffer,&grib_msg->md.scan_mode,grib_msg->offset+512,8);
/* latin1 */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+520,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+521,31);
	grib_msg->md.latin1=value/1000000.;
	if (sign == 1)
	  grib_msg->md.latin1=-grib_msg->md.latin1;
/* latin2 */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+552,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+553,31);
	grib_msg->md.latin2=value/1000000.;
	if (sign == 1)
	  grib_msg->md.latin2=-grib_msg->md.latin2;
/* latitude of southern pole of projection */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+584,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+585,31);
	grib_msg->md.splat=value/1000000.;
	if (sign == 1)
	  grib_msg->md.splat=-grib_msg->md.splat;
/* longitude of southern pole of projection */
	getBits(grib_msg->buffer,&sign,grib_msg->offset+616,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+617,31);
	grib_msg->md.splon=value/1000000.;
	if (sign == 1)
	  grib_msg->md.splon=-grib_msg->md.splon;
	break;
    default:
	fprintf(stderr,"Grid template %d is not understood\n",grib_msg->md.gds_templ_num);
	return false;
  }
  return true;
}

static bool unpackPDS(GRIBMessage *grib_msg)
{
  int num_coords,factor,sign,value;
  int hh,mm,ss;
  size_t n,off,start;

/* indication of hybrid coordinate system */
  getBits(grib_msg->buffer,&num_coords,grib_msg->offset+40,16);
  if (num_coords > 0) {
    fprintf(stderr,"Unable to decode hybrid coordinates");
    return false;
  }
/* product definition template number */
  getBits(grib_msg->buffer,&grib_msg->md.pds_templ_num,grib_msg->offset+56,16);
  grib_msg->md.stat_proc.num_ranges=0;
  switch (grib_msg->md.pds_templ_num) {
    case 0:
    case 1:
    case 2:
    case 8:
    case 11:
    case 12:
    case 15:
	grib_msg->md.ens_type=-1;
	grib_msg->md.derived_fcst_code=-1;
	grib_msg->md.spatial_proc.type=-1;
/* parameter category */
	getBits(grib_msg->buffer,&grib_msg->md.param_cat,grib_msg->offset +72,8);
/* parameter number */
	getBits(grib_msg->buffer,&grib_msg->md.param_num,grib_msg->offset +(8 *10),8);
/* generating process */
	getBits(grib_msg->buffer,&grib_msg->md.gen_proc,grib_msg->offset +88,8);
/* time range indicator*/
	getBits(grib_msg->buffer,&grib_msg->md.time_unit,grib_msg->offset +136,8);
/* forecast time */
	getBits(grib_msg->buffer,&grib_msg->md.fcst_time,grib_msg->offset+144,32);
/* type of first level */
	getBits(grib_msg->buffer,&grib_msg->md.lvl1_type,grib_msg->offset+176,8);
/* value of first level */
	getBits(grib_msg->buffer,&factor,grib_msg->offset+184,8);
	getBits(grib_msg->buffer,&sign,grib_msg->offset+192,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+193,31);
	if (sign == 1)
	  value=-value;
	grib_msg->md.lvl1=(double)value/pow(10.,(double)factor);
/* type of second level */
	getBits(grib_msg->buffer,&grib_msg->md.lvl2_type,grib_msg->offset+224,8);
/* value of second level */
	getBits(grib_msg->buffer,&factor,grib_msg->offset+232,8);
	getBits(grib_msg->buffer,&sign,grib_msg->offset+240,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+241,31);
	if (sign == 1)
	  value=-value;
	grib_msg->md.lvl2=(double)value/pow(10.,(double)factor);
	start=272;
	switch (grib_msg->md.pds_templ_num) {
	  case 1:
	  case 11:
	    getBits(grib_msg->buffer,&grib_msg->md.ens_type,grib_msg->offset+272,8);
	    getBits(grib_msg->buffer,&grib_msg->md.perturb_num,grib_msg->offset+280,8);
	    getBits(grib_msg->buffer,&grib_msg->md.nfcst_in_ensemble,grib_msg->offset+288,8);
	    switch (grib_msg->md.pds_templ_num) {
		case 11:
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.eyr,grib_msg->offset+296,16);
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.emo,grib_msg->offset+312,8);
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.edy,grib_msg->offset+320,8);
		  getBits(grib_msg->buffer,&hh,grib_msg->offset+328,8);
		  getBits(grib_msg->buffer,&mm,grib_msg->offset+336,8);
		  getBits(grib_msg->buffer,&ss,grib_msg->offset+344,8);
		  grib_msg->md.stat_proc.etime=hh*10000+mm*100+ss;
/* number of time range specifications */
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.num_ranges,grib_msg->offset+352,8);
/* number of values missing from process */
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.nmiss,grib_msg->offset+360,32);
		  if (grib_msg->md.stat_proc.proc_code != NULL) {
		    delete [] grib_msg->md.stat_proc.proc_code;
		    delete [] grib_msg->md.stat_proc.incr_type;
		    delete [] grib_msg->md.stat_proc.time_unit;
		    delete [] grib_msg->md.stat_proc.time_length;
		    delete [] grib_msg->md.stat_proc.incr_unit;
		    delete [] grib_msg->md.stat_proc.incr_length;
		    grib_msg->md.stat_proc.proc_code=NULL;
		  }
		  grib_msg->md.stat_proc.proc_code= new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.incr_type = new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.time_unit= new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.time_length= new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.incr_unit= new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.incr_length= new int[grib_msg->md.stat_proc.num_ranges];
		  off=392;
		  for (n=0; n < (size_t)grib_msg->md.stat_proc.num_ranges; n++) {
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.proc_code[n],grib_msg->offset+off,8);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_type[n],grib_msg->offset+off+8,8);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.time_unit[n],grib_msg->offset+off+16,8);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.time_length[n],grib_msg->offset+off+24,32);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_unit[n],grib_msg->offset+off+56,8);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_length[n],grib_msg->offset+off+64,32);
		    off+=96;
		  }
		  break;
	    }
	    break;
	  case 2:
	  case 12:
	    getBits(grib_msg->buffer,&grib_msg->md.derived_fcst_code,grib_msg->offset+272,8);
	    getBits(grib_msg->buffer,&grib_msg->md.nfcst_in_ensemble,grib_msg->offset+280,8);
	    switch (grib_msg->md.pds_templ_num) {
		case 12:
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.eyr,grib_msg->offset+288,16);
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.emo,grib_msg->offset+304,8);
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.edy,grib_msg->offset+312,8);
		  getBits(grib_msg->buffer,&hh,grib_msg->offset+320,8);
		  getBits(grib_msg->buffer,&mm,grib_msg->offset+328,8);
		  getBits(grib_msg->buffer,&ss,grib_msg->offset+336,8);
		  grib_msg->md.stat_proc.etime=hh*10000+mm*100+ss;
/* number of time range specifications */
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.num_ranges,grib_msg->offset+344,8);
/* number of values missing from process */
		  getBits(grib_msg->buffer,&grib_msg->md.stat_proc.nmiss,grib_msg->offset+352,32);
		  if (grib_msg->md.stat_proc.proc_code != 0) {
		    delete [] grib_msg->md.stat_proc.proc_code;
		    delete [] grib_msg->md.stat_proc.incr_type;
		    delete [] grib_msg->md.stat_proc.time_unit;
		    delete [] grib_msg->md.stat_proc.time_length;
		    delete [] grib_msg->md.stat_proc.incr_unit;
		    delete [] grib_msg->md.stat_proc.incr_length;
		    grib_msg->md.stat_proc.proc_code=NULL;
		  }
		  grib_msg->md.stat_proc.proc_code = new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.incr_type = new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.time_unit = new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.time_length = new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.incr_unit = new int[grib_msg->md.stat_proc.num_ranges];
		  grib_msg->md.stat_proc.incr_length= new int[grib_msg->md.stat_proc.num_ranges];
		  off=384;
		  for (n=0; n < (size_t)grib_msg->md.stat_proc.num_ranges; n++) {
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.proc_code[n],grib_msg->offset+off,8);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_type[n],grib_msg->offset+off+8,8);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.time_unit[n],grib_msg->offset+off+16,8);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.time_length[n],grib_msg->offset+off+24,32);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_unit[n],grib_msg->offset+off+56,8);
		    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_length[n],grib_msg->offset+off+64,32);
		    off+=96;
		  }
		  break;
	    }
	    break;
	  case 8:
	    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.eyr,grib_msg->offset+272,16);
	    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.emo,grib_msg->offset+288,8);
	    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.edy,grib_msg->offset+296,8);
	    getBits(grib_msg->buffer,&hh,grib_msg->offset+304,8);
	    getBits(grib_msg->buffer,&mm,grib_msg->offset+312,8);
	    getBits(grib_msg->buffer,&ss,grib_msg->offset+320,8);
	    grib_msg->md.stat_proc.etime=hh*10000+mm*100+ss;
/* number of time range specifications */
	    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.num_ranges,grib_msg->offset+328,8);
/* number of values missing from process */
	    getBits(grib_msg->buffer,&grib_msg->md.stat_proc.nmiss,grib_msg->offset+336,32);
	    if (grib_msg->md.stat_proc.proc_code != NULL) {
		delete [] grib_msg->md.stat_proc.proc_code;
		delete [] grib_msg->md.stat_proc.incr_type;
		delete [] grib_msg->md.stat_proc.time_unit;
		delete [] grib_msg->md.stat_proc.time_length;
		delete [] grib_msg->md.stat_proc.incr_unit;
		delete [] grib_msg->md.stat_proc.incr_length;
		grib_msg->md.stat_proc.proc_code=NULL;
	    }
            grib_msg->md.stat_proc.proc_code = new int[grib_msg->md.stat_proc.num_ranges];
            grib_msg->md.stat_proc.incr_type = new int[grib_msg->md.stat_proc.num_ranges];
            grib_msg->md.stat_proc.time_unit = new int[grib_msg->md.stat_proc.num_ranges];
            grib_msg->md.stat_proc.time_length = new int[grib_msg->md.stat_proc.num_ranges];
            grib_msg->md.stat_proc.incr_unit = new int[grib_msg->md.stat_proc.num_ranges];
            grib_msg->md.stat_proc.incr_length= new int[grib_msg->md.stat_proc.num_ranges];
	    off=368;
	    for (n=0; n  < (size_t)grib_msg->md.stat_proc.num_ranges; n++) {
		getBits(grib_msg->buffer,&grib_msg->md.stat_proc.proc_code[n],grib_msg->offset+off,8);
		getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_type[n],grib_msg->offset+off+8,8);
		getBits(grib_msg->buffer,&grib_msg->md.stat_proc.time_unit[n],grib_msg->offset+off+16,8);
		getBits(grib_msg->buffer,&grib_msg->md.stat_proc.time_length[n],grib_msg->offset+off+24,32);
		getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_unit[n],grib_msg->offset+off+56,8);
		getBits(grib_msg->buffer,&grib_msg->md.stat_proc.incr_length[n],grib_msg->offset+off+64,32);
		off+=96;
	    }
	    break;
	  case 15:
	    getBits(grib_msg->buffer,&grib_msg->md.spatial_proc.stat_proc,grib_msg->offset+272,8);
	    getBits(grib_msg->buffer,&grib_msg->md.spatial_proc.type,grib_msg->offset+280,8);
	    getBits(grib_msg->buffer,&grib_msg->md.spatial_proc.num_points,grib_msg->offset+288,8);
	    break;
	}
	break;
    default:
	fprintf(stderr,"Product Definition Template %d is not understood\n",grib_msg->md.pds_templ_num);
	return false;
  }
  return true;
}

static bool unpackDRS(GRIBMessage *grib_msg)
{
  int sign,value;

/* number of packed values */
  getBits(grib_msg->buffer,&grib_msg->md.num_packed,grib_msg->offset+40,32);
/* data representation template number */
  getBits(grib_msg->buffer,&grib_msg->md.drs_templ_num,grib_msg->offset+72,16);
  switch (grib_msg->md.drs_templ_num) {
    case 0:
#ifdef JASPER
    case 40:
    case 40000:
#endif
   /* cf http://www.wmo.int/pages/prog/www/WMOCodes/Guides/GRIB/GRIB2_062006.pdf p. 36*/
	getBits(grib_msg->buffer,(int *)&grib_msg->md.R,grib_msg->offset+88,32);
	getBits(grib_msg->buffer,&sign,grib_msg->offset+120,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+121,15);
	if (sign == 1)
	  value=-value;
	grib_msg->md.E=value;
	getBits(grib_msg->buffer,&sign,grib_msg->offset+136,1);
	getBits(grib_msg->buffer,&value,grib_msg->offset+137,15);
	if (sign == 1)
	  value=-value;
	grib_msg->md.D=value;
	grib_msg->md.R/=pow(10.,grib_msg->md.D);
	getBits(grib_msg->buffer,&grib_msg->md.pack_width,grib_msg->offset+152,8);
	break;
    default:
	fprintf(stderr,"Data template %d is not understood\n",grib_msg->md.drs_templ_num);
	return false;
  }
  return true;
}

static bool unpackBMS(GRIBMessage *grib_msg)
{
  int ind,len,n,bit;

/* bit map indicator */
  getBits(grib_msg->buffer,&ind,grib_msg->offset+40,8);
  grib_msg->md.bmssize = 0;
  switch (ind) {
    case 0:
	getBits(grib_msg->buffer,&len,grib_msg->offset,32);
	grib_msg->md.bmssize = len -6;
	len=(len-6)*8;
	grib_msg->md.bitmap = new unsigned char[len];
	grib_msg->md.bms = new zuchar[grib_msg->md.bmssize];
	memcpy (grib_msg->md.bms, grib_msg->buffer +grib_msg->offset + 6, grib_msg->md.bmssize);
	for (n=0; n < len; n++) {
	  getBits(grib_msg->buffer, &bit, grib_msg->offset+48+n, 1);
	  grib_msg->md.bitmap[n]=bit;
	}
	break;
    case 254:
	break;
    case 255:
	grib_msg->md.bitmap=NULL;
	break;
    default:
	fprintf(stderr,"This code is not currently set up to deal with predefined bit-maps\n");
	return false;
  }
  return true;
}

static bool unpackDS(GRIBMessage *grib_msg,int grid_num)
{
  int off,n,pval; 

  off=grib_msg->offset+40;
  switch (grib_msg->md.drs_templ_num) {
    case 0:
	(grib_msg->grids[grid_num]).gridpoints= new double[grib_msg->md.ny *grib_msg->md.nx];
	for (n=0; n < grib_msg->md.ny*grib_msg->md.nx; n++) {
	  if (grib_msg->md.bitmap == NULL || grib_msg->md.bitmap[n] == 1) {
	    getBits(grib_msg->buffer,&pval,off,grib_msg->md.pack_width);
	    grib_msg->grids[grid_num].gridpoints[n]=grib_msg->md.R+pval*pow(2.,grib_msg->md.E)/pow(10.,grib_msg->md.D);
	    off+=grib_msg->md.pack_width;
	  }
	  else
	    grib_msg->grids[grid_num].gridpoints[n]=GRIB_MISSING_VALUE;
	}
	break;
#ifdef JASPER
    case 40:
    case 40000:
        int len, *jvals, cnt;
	getBits(grib_msg->buffer,&len,grib_msg->offset,32);
	len=len-5;
	jvals=(int *)malloc(grib_msg->md.ny*grib_msg->md.nx*sizeof(int));
	(grib_msg->grids[grid_num]).gridpoints= new double[grib_msg->md.ny *grib_msg->md.nx];
	if (len > 0)
	  dec_jpeg2000((char *)&grib_msg->buffer[grib_msg->offset/8+5],len,jvals);
	cnt=0;
	for (n=0; n < grib_msg->md.ny*grib_msg->md.nx; n++) {
	  if (grib_msg->md.bitmap == NULL || grib_msg->md.bitmap[n] == 1) {
	    if (len == 0)
		jvals[cnt]=0;
	    grib_msg->grids[grid_num].gridpoints[n]=grib_msg->md.R+jvals[cnt++]*pow(2.,grib_msg->md.E)/pow(10.,grib_msg->md.D);
	  }
	  else
	    grib_msg->grids[grid_num].gridpoints[n]=GRIB_MISSING_VALUE;
	}
	free(jvals);
	break;
#endif
  }
  return true;
}

static zuchar GRBV2_TO_DATA(int productDiscipline, int dataCat, int dataNum)
{
    zuchar ret = 255;
    // printf("search %d %d %d\n", productDiscipline, dataCat,  dataNum);
    switch (productDiscipline) {
    case 0:
        switch (dataCat) {
        case 0:
            switch (dataNum) {
            case 0: ret = GRB_TEMP; break; // DATA_TO_GRBV2[DATA_TEMP] = grb2DataType(0,0,0);
            case 2: ret= GRB_TPOT; break;  // DATA_TO_GRBV2[DATA_TEMP_POT] = grb2DataType(0,0,2);
            case 4: ret = GRB_TMAX; break; // DATA_TO_GRBV2[DATA_TMAX] = grb2DataType(0,0,4);
            case 5: ret = GRB_TMIN; break; // DATA_TO_GRBV2[DATA_TMIN] = grb2DataType(0,0,5);
            case 6: ret = GRB_DEWPOINT; break; //DATA_TO_GRBV2[DATA_DEWPOINT] = grb2DataType(0,0,6);
            }
            break;
        case 1: // dataCat
            switch (dataNum) {
            case 0: ret = GRB_HUMID_SPEC; break; //DATA_TO_GRBV2[DATA_HUMID_SPEC] = grb2DataType(0,1,0);
            case 1: ret = GRB_HUMID_REL; break; // DATA_TO_GRBV2[DATA_HUMID_REL] = grb2DataType(0,1,1);
            case 7: ret= GRB_PRECIP_RATE; break; // DATA_TO_GRBV2[DATA_PRECIP_RATE] = grb2DataType(0,1,7);
            case 52:                             // Total precipitation rate kg m–2 s–1
            case 8: ret = GRB_PRECIP_TOT; break; // DATA_TO_GRBV2[DATA_PRECIP_TOT] = grb2DataType(0,1,8);
            case 11: ret = GRB_SNOW_DEPTH; break; // DATA_TO_GRBV2[DATA_SNOW_DEPTH] = grb2DataType(0,1,11);
            case 193: ret = GRB_FRZRAIN_CATEG; break; // DATA_TO_GRBV2[DATA_FRZRAIN_CATEG] = grb2DataType(0,1,193);
            case 195: ret = GRB_SNOW_CATEG; break; //DATA_TO_GRBV2[DATA_SNOW_CATEG] = grb2DataType(0,1,195);
            }
            break;
        case 2: // dataCat
            switch (dataNum) {
            case 2: ret = GRB_WIND_VX; break; // DATA_TO_GRBV2[DATA_WIND_VX] = grb2DataType(0,2,2);
            case 3: ret = GRB_WIND_VY; break; // DATA_TO_GRBV2[DATA_WIND_VY] = grb2DataType(0,2,3);
            case 22: ret = GRB_WIND_GUST; break; // 
            }
            break;
        case 3: // dataCat
            switch (dataNum) {
            case 0: ret = GRB_PRESSURE; break; //DATA_TO_GRBV2[DATA_PRESSURE] = grb2DataType(0,3,0);
            case 1: ret = GRB_PRESSURE; break; // PRSMSL //DATA_TO_GRBV2[DATA_PRESSURE] = grb2DataType(0,3,0);
            case 5: ret = GRB_GEOPOT_HGT; break; // DATA_TO_GRBV2[DATA_GEOPOT_HGT]= grb2DataType(0,3,5);
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
        }
        break;
    case 10: // productDiscipline
        switch (dataCat) {
        // waves
        case 0:
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
                case 3: ret= GRB_HTSGW; break;
                case 5: ret= GRB_WVHGT; break;
                case 4: ret= GRB_WVDIR; break;
            }
            break;

        case 1:
            switch (dataNum) {
                case 2: ret = GRB_UOGRD; break; // DATA_TO_GRBV2[DATA_CURRENT_VX] = grb2DataType(10,1,2);
                case 3: ret = GRB_VOGRD; break; // DATA_TO_GRBV2[DATA_CURRENT_VY] = grb2DataType(10,1,3);
            }
            break;
         case 3:
            switch (dataNum) {
                case 0: ret = GRB_WTMP; break; // DATA_TO_GRBV2[DATA_CURRENT_VX] = grb2DataType(10,1,2);
            }
            break;
            
        }
        break;
    }
#if 1
    if (ret == 255) {
        printf("unknown %d %d %d\n", productDiscipline,  dataCat,dataNum);
    }
#endif    
    return ret;    
}

static int mapStatisticalEndTime(GRIBMessage *grid)
{
  switch (grid->md.time_unit) {
    case 0:
	return (grid->md.stat_proc.etime/100 % 100)-(grid->time/100 % 100);
    case 1:
         return grid->md.fcst_time +grid->md.stat_proc.time_length[0];
	 // return (grid->md.stat_proc.etime/10000- grid->time/10000);
    case 2:
	return (grid->md.stat_proc.edy -grid->dy);
    case 3:
	return (grid->md.stat_proc.emo -grid->mo);
    case 4:
	return (grid->md.stat_proc.eyr -grid->yr);
    default:
	fprintf(stderr,"Unable to map end time with units %d to GRIB1\n",grid->md.time_unit);
	exit(1);
  }
}

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
	    *p2=grid->md.stat_proc.incr_length[0];
	    *p1=*p2 -grid->md.stat_proc.time_length[1];
	    *n_avg=grid->md.stat_proc.time_length[0];
	    switch (grid->md.stat_proc.proc_code[0]) {
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
		  fprintf(stderr,"Unable to map NCEP statistical process code %d to GRIB1\n",grid->md.stat_proc.proc_code[0]);
		  return false;
	    }
	  }
	  else {
	    fprintf(stderr,"Unable to map multiple statistical processes to GRIB1\n");
	    return false;
	  }
	}
	else {
	  switch (grid->md.stat_proc.proc_code[0]) {
	    case 0:
	    case 1:
	    case 4:
		switch (grid->md.stat_proc.proc_code[0]) {
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
		if (grid->md.stat_proc.incr_length[0] == 0)
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
		if (grid->md.stat_proc.incr_length[0] == 0)
		  *n_avg=0;
		else {
		  fprintf(stderr,"Unable to map discrete processing to GRIB1\n");
		  return false;
		}
		break;
	    default:
// patch for NCEP grids
		if (grid->md.stat_proc.proc_code[0] == 255 && center == 7) {
 		  if (grid->disc == 0) {
		    if (grid->md.param_cat == 0) {
			switch (grid->md.param_num) {
			  case 4:
			  case 5:
			    *t_range=2;
			    *p1=grid->md.fcst_time;
			    *p2=mapStatisticalEndTime(grid);
			    if (grid->md.stat_proc.incr_length[0] == 0)
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
		  fprintf(stderr,"Unable to map statistical process %d to GRIB1\n",grid->md.stat_proc.proc_code[0]);
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
  if (*p2 < 0)
      return false;
  return true;
}

//-------------------------------------------------------------------------------
// Adjust data type from different mete center
//-------------------------------------------------------------------------------
void  GribV2Record::translateDataType()
{
    this->knownData = true;
    //------------------------
    // NOAA GFS
    //------------------------
    if (dataType == GRB_PRECIP_RATE) {	// mm/s -> mm/h
        multiplyAllData( 3600.0 );
    }
    else if (   idCenter==7 && idModel==2 )		// NOAA
    {
        dataCenterModel = NOAA_GFS;
        // altitude level (entire atmosphere vs entire atmosphere considered as 1 level)
        if (levelType == LV_ATMOS_ENT) {
            levelType = LV_ATMOS_ALL;
        }
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
                levelType  = LV_GND_SURF;
                levelValue = 0;
                break;
        }
    }
    // this->print();
}

GribV2Record::GribV2Record(ZUFILE* file, int id_)
{
    id = id_;
    seekStart = zu_tell(file);           // moved to section 0 read
    data    = NULL;
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
    ok = readGribSection0_IS(file, b_haveReadGRIB );
    
    int n, len, sec_num;    
    if (ok) {
        unpackIDS(grib_msg);
        int off;
        /* find out how many grids are in this message */
        off = grib_msg->offset;
        while (strncmp(&((char *)grib_msg->buffer)[off/8], "7777", 4) != 0) {
            getBits(grib_msg->buffer, &len, off, 32);
            getBits(grib_msg->buffer, &sec_num, off+32, 8);
            if (sec_num == 7)
                grib_msg->num_grids++;
            off += len*8;
        }
        if (grib_msg->num_grids != 1) {
            ok = false;
            return;
        }
    }
    else {
        // seek back if V1
        zu_seek(file, start, SEEK_SET);
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

    grib_msg->grids = new GRIB2Grid [grib_msg->num_grids];
    n = 0;
    bool skip = false;
    while (strncmp(&((char *)grib_msg->buffer)[grib_msg->offset/8],"7777",4) != 0) {
        getBits(grib_msg->buffer, &len, grib_msg->offset, 32);
        getBits(grib_msg->buffer, &sec_num, grib_msg->offset +4*8, 8);
        if (skip == false) switch (sec_num) {
	case 2:
             ok = unpackLUS(grib_msg);
             break;
	case 3:
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
                 if (Lo1>=0 && Lo1<=180 && Lo2<0)
                     Lo2 += 360.0;    // cross the 180 deg meridien,beetwen alaska and russia
	                     
	         while ( Lo1> Lo2   &&  Di >0) {   // horizontal size > 360 °
	             Lo1 -= 360.0;
                 }
                 scanFlags = grib_msg->md.scan_mode;
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
	     }
	     break;
	case 4:
	     ok = unpackPDS(grib_msg);
	     if (ok) {
	         //printf("template %d 0 meteo\n", grib_msg->md.pds_templ_num);
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
	case 5:
	     ok = unpackDRS(grib_msg);
	     break;
	case 6:
	     ok = unpackBMS(grib_msg);
	     if (ok) {
	        if (grib_msg->md.bmssize != 0) {
	             hasBMS = true;
	             BMSsize = grib_msg->md.bmssize;
	             BMSbits = grib_msg->md.bms; 
	             grib_msg->md.bms = 0; 
                }
	     }
	     break;
	case 7:
	     ok = unpackDS(grib_msg,n);
	     if (ok) {
	         data = grib_msg->grids[n].gridpoints;
	         grib_msg->grids[n].gridpoints = 0;
	     }
	     n++;
	     break;
        }
        grib_msg->offset += len*8;
        if (ok == false)
            break;
    }

    //ok = false;
if (false) {
printf("==== GV2 %d\n", ok);
printf("Lo1=%f Lo2=%f    La1=%f La2=%f\n", Lo1,Lo2,La1,La2);
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
    delete grib_msg;
    grib_msg = 0;
}

//-------------------------------------------------------------------------------
// Constructeur de recopie
//-------------------------------------------------------------------------------
GribV2Record::GribV2Record(const GribRecord &rec) : GribRecord(rec)
{
    *this = rec;
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
  size_t n,num;

  if (grib_msg->buffer != NULL) {
    delete [] grib_msg->buffer;
    grib_msg->buffer=NULL;
  }
  else {
    grib_msg->grids=NULL;
  }
  if (grib_msg->grids != NULL) {
    for (n = 0; n < grib_msg->num_grids; n++) {
	delete grib_msg->grids[n].gridpoints;
    }
    delete [] grib_msg->grids;
    grib_msg->grids=NULL;
  }
  grib_msg->num_grids = 0;

  if ( (status = zu_read(fp, &temp[4], 12)) != 12)
  {
    return false;
  }
  getBits(temp,&grib_msg->disc,48,8);
  getBits(temp,&grib_msg->ed_num,56,8);
  
  //  Bail out early if this is not GRIB2
  if(grib_msg->ed_num != 2)
      return false;
  
  getBits(temp,&grib_msg->total_len,96,32);

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
zuint GribV2Record::periodSeconds(zuchar unit,zuint P1,zuchar P2,zuchar range) {
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

