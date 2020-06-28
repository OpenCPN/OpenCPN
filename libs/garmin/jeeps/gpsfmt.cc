/********************************************************************
** @source JEEPS output functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA  02110-1301, USA.
********************************************************************/
#include "gps.h"
#include <cstdio>
#include <ctime>


static void GPS_Fmt_Print_Way100(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way101(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way102(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way103(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way104(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way105(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way106(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way107(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way108(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way109(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way150(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way151(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way152(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way154(GPS_PWay way, FILE* outf);
static void GPS_Fmt_Print_Way155(GPS_PWay way, FILE* outf);

static void GPS_Fmt_Print_Track301(GPS_PTrack* trk, int32 n, FILE* outf);
static void GPS_Fmt_Print_D300(GPS_PTrack trk, FILE* outf);
static void GPS_Fmt_Print_D301(GPS_PTrack trk, FILE* outf);

static int32 GPS_Fmt_Print_Route201(GPS_PWay* way, int32 n, FILE* outf);


const char* gps_marine_sym[]= {
  "Anchor","Bell","Diamond-grn","Diamond_red","Dive1","Dive2","Dollar",
  "Fish","Fuel","Horn","House","Knife","Light","Mug","Skull",
  "Square_grn","Square_red","Wbuoy","Wpt_dot","Wreck","Null","Mob",

  "Buoy_amber","Buoy_blck","Buoy_blue","Buoy_grn","Buoy_grn_red",
  "Buoy_grn_wht","Buoy_orng","Buoy_red","Buoy_red_grn","Buoy_red_wht",
  "Buoy_violet","Buoy_wht","Buoy_wht_grn","Buoy_wht_red","Dot","Rbcn",

  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","","","","","","","","","",
  "","",

  "Boat_ramp","Camp","Toilets","Showers","Drinking_wtr","Phone",
  "1st_aid","Info","Parking","Park","Picnic","Scenic","Skiing",
  "Swimming","Dam","Controlled","Danger","Restricted","Null_2","Ball",
  "Car","Deer","Shpng_trolley","Lodging","Mine","Trail_head",
  "Lorry_stop","User_exit","Flag","Circle-x"
};



const char* gps_land_sym[]= {
  "Is_hwy","Us_hwy","St_hwy","Mi_mrkr","Trcbck","Golf","Sml_cty",
  "Med_cty","Lrg_cty","Freeway","Ntl_hwy","Cap_cty","Amuse_pk",
  "Bowling","Car_rental","Car_repair","Fastfood","Fitness","Film",
  "Museum","Chemist","Pizza","Post_ofc","Rv_park","School",
  "Stadium","Shop","Zoo","Petrol_plus","Theatre","Ramp_int",
  "St_int","","","Weigh_stn","Toll_booth","Elev_pt","Ex_no_srvc",
  "Geo_place_mm","Geo_place_wtr","Geo_place_lnd","Bridge","Building",
  "Cemetery","Church","Civil_loc","Crossing","Hist_town","River_Embankment",
  "Military_loc","Oil_field","Tunnel","Beach","Forest","Summit",
  "Lrg_ramp_int","Lrg_exit_no_srvc","Official_badge","Gambling",
  "Snow_ski","Ice_ski","Tow_truck","Border"
};


const char* gps_aviation_sym[]= {
  "Airport","Int","Ndb","Vor","Heliport","Private","Soft_fld",
  "Tall_tower","Short_tower","Glider","Ultralight","Parachute",
  "Vortac","Vordme","Faf","Lom","Map","Tacan","Seaplane"
};


const char* gps_16_sym[]= {
  "Dot","House","Fuel","Car","Fish","Boat","Anchor","Wreck",
  "Exit","Skull","Flag","Camp","Circle-x","Deer","1st_aid","Back_track"
};





/* @func GPS_Fmt_Print_Time ********************************************
**
** Output Date/time
**
** @param [r] Time [time_t] unix-style time
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

void GPS_Fmt_Print_Time(time_t Time, FILE* outf)
{
  (void) fprintf(outf,"%s",ctime(&Time));
  fflush(outf);

  return;
}



/* @func GPS_Fmt_Print_Position ********************************************
**
** Output position
**
** @param [r] lat [double] latitude  (deg)
** @param [r] lon [double] longitude (deg)
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

void GPS_Fmt_Print_Position(double lat, double lon, FILE* outf)
{
  (void) fprintf(outf,"Latitude: %f   Longitude %f\n",lat,lon);
  fflush(outf);

  return;
}



/* @func GPS_Fmt_Print_Pvt ********************************************
**
** Output pvt
**
** @param [r] pvt [GPS_PPvt_Data] pvt
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

void GPS_Fmt_Print_Pvt(GPS_PPvt_Data pvt, FILE* outf)
{

  (void) fprintf(outf,"Fix: ");
  switch (pvt->fix) {
  case 0:
    (void) fprintf(outf,"UNUSABLE\n\n");
    break;
  case 1:
    (void) fprintf(outf,"INVALID \n\n");
    break;
  case 2:
    (void) fprintf(outf,"2D      \n\n");
    break;
  case 3:
    (void) fprintf(outf,"3D      \n\n");
    break;
  case 4:
    (void) fprintf(outf,"2D-diff \n\n");
    break;
  case 5:
    (void) fprintf(outf,"2D-diff \n\n");
    break;
  default:
    (void) fprintf(stderr,"PVT: Unsupported Fix type\n");
    break;
  }

  (void) fprintf(outf,"Altitude (WGS 84): %-20f \n",pvt->alt);
  (void) fprintf(outf,"EPE:               %-20f \n",pvt->epe);
  (void) fprintf(outf,"EPE (hor only):    %-20f \n",pvt->eph);
  (void) fprintf(outf,"EPE (ver only):    %-20f \n",pvt->epv);
  (void) fprintf(outf,"Time of week:      %-20d \n",(int)pvt->tow);
  (void) fprintf(outf,"Latitude:          %-20f \n",pvt->lat);
  (void) fprintf(outf,"Longitude:         %-20f \n",pvt->lon);
  (void) fprintf(outf,"East velocity:     %-20f \n",pvt->east);
  (void) fprintf(outf,"North velocity:    %-20f \n",pvt->north);
  (void) fprintf(outf,"Upward velocity    %-20f \n",pvt->up);
  (void) fprintf(outf,"Height above MSL:  %-20f \n",pvt->msl_hght+pvt->alt);
  (void) fprintf(outf,"Leap seconds:      %-20d \n",pvt->leap_scnds);
  (void) fprintf(outf,"Week number days:  %-20d \n",(int)pvt->wn_days);

  fflush(outf);

  return;
}



/* @func GPS_Fmt_Print_Almanac ********************************************
**
** Output almanac
**
** @param [r] alm [GPS_PAlmanac *] almanac array
** @param [r] n [int32] number of almanac entries
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

void GPS_Fmt_Print_Almanac(GPS_PAlmanac* alm, int32 n, FILE* outf)
{
  int32 i;
  int32 t;
  int32 s;

  /* Type 0 models require all 32 satellites to be sent */
  t=32;
  s=0;
  if (n && alm[0]->svid!=0xff) {
    s=1;
    t=n;
  }
  (void) fprintf(outf,"Almanac %d %d\n",(int)t,(int)s);


  for (i=0; i<n; ++i) {
    if (alm[i]->wn<0) {
      continue;
    }

    if (alm[i]->svid == 0xff) {
      alm[i]->svid = i;
    }
    (void) fprintf(outf,"#\n#\n");
    (void) fprintf(outf,"\tID:                           %d\n",
                   alm[i]->svid+1);
    (void) fprintf(outf,"\tWeek number:                  %d\n",
                   alm[i]->wn);
    (void) fprintf(outf,"\tAlmanac Data Reference Time:  %f\n",
                   alm[i]->toa);
    (void) fprintf(outf,"\tClock Correction Coeff (s):   %f\n",
                   alm[i]->af0);
    (void) fprintf(outf,"\tClock Correction Coeff (s/s): %f\n",
                   alm[i]->af1);
    (void) fprintf(outf,"\tEccentricity:                 %f\n",
                   alm[i]->e);
    (void) fprintf(outf,"\tSqrt of semi-major axis:      %f\n",
                   alm[i]->sqrta);
    (void) fprintf(outf,"\tMean Anomaly at Ref. Time:    %f\n",
                   alm[i]->m0);
    (void) fprintf(outf,"\tArgument of perigee:          %f\n",
                   alm[i]->w);
    (void) fprintf(outf,"\tRight ascension:              %f\n",
                   alm[i]->omg0);
    (void) fprintf(outf,"\tRate of right ascension:      %f\n",
                   alm[i]->odot);
    (void) fprintf(outf,"\tInclination angle:            %f\n",
                   alm[i]->i);
    (void) fprintf(outf,"\tHealth:                       %d\n",
                   alm[i]->hlth);
  }


  fflush(outf);

  return;
}



/* @func GPS_Fmt_Print_Track ********************************************
**
** Output track log
**
** @param [r] trk [GPS_PTrack *] track array
** @param [r] n [int32] number of track entries
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

void GPS_Fmt_Print_Track(GPS_PTrack* trk, int32 n, FILE* outf)
{
  int32 i;


  switch (gps_trk_transfer) {
  case pA300:
    break;
  case pA301:
    GPS_Fmt_Print_Track301(trk,n,outf);
    return;
  default:
    GPS_Error("GPS_Fmt_Print_Track: Unknown protocol");
    return;
  }


  (void) fprintf(outf,"Track log 300 %d\n#\n",(int)gps_trk_type);
  (void) fprintf(outf,"Start\n#\n");

  for (i=0; i<n; ++i) {
    if (trk[i]->tnew) {
      (void) fprintf(outf,"#\nNew track\n#\n");
    }

    switch (gps_trk_type) {
    case pD300:
      GPS_Fmt_Print_D300(trk[i],outf);
      break;
    case pD301:
      GPS_Fmt_Print_D301(trk[i],outf);
      break;
    default:
      break;
    }
  }

  (void) fprintf(outf,"End\n#\n");
  fflush(outf);

  return;
}



/* @funcstatic GPS_Fmt_Print_Track301 ***********************************
**
** Output track log
**
** @param [r] trk [GPS_PTrack *] track array
** @param [r] n [int32] number of track entries
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Track301(GPS_PTrack* trk, int32 n, FILE* outf)
{
  int32 i;

  if (!n) {
    return;
  }

  (void) fprintf(outf,"Track log 301 %d\n#\n",(int)gps_trk_type);
  (void) fprintf(outf,"Start\n#\n");

  for (i=0; i<n; ++i) {
    if (trk[i]->ishdr) {
      (void) fprintf(outf,"Header\n");
      (void) fprintf(outf,"\tIdent:       %s\n",trk[i]->trk_ident);
      (void) fprintf(outf,"\tDisplay:     %d\n",(int)trk[i]->dspl);
      (void) fprintf(outf,"\tColour:      %d\n#\n",
                     (int)trk[i]->colour);
      continue;
    }

    if (trk[i]->tnew) {
      (void) fprintf(outf,"#\nNew track\n#\n");
    }

    switch (gps_trk_type) {
    case pD300:
      GPS_Fmt_Print_D300(trk[i],outf);
      break;
    case pD301:
      GPS_Fmt_Print_D301(trk[i],outf);
      break;
    default:
      break;
    }
  }

  (void) fprintf(outf,"End\n#\n");
  fflush(outf);

  return;
}


/* @funcstatic GPS_Fmt_Print_D300 ****************************************
**
** Output track log
**
** @param [r] trk [GPS_PTrack *] track array
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_D300(GPS_PTrack trk, FILE* outf)
{
  (void) fprintf(outf,"\tLatitude:    %f\n",trk->lat);
  (void) fprintf(outf,"\tLongitude:   %f\n",trk->lon);
  if (trk->Time) {
    (void) fprintf(outf,"\tTime:        %s\n",ctime(&trk->Time));
  } else {
    (void) fprintf(outf,"\tTime:        Computer\n\n");
  }

  return;
}



/* @funcstatic GPS_Fmt_Print_D301 ****************************************
**
** Output track log
**
** @param [r] trk [GPS_PTrack *] track array
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_D301(GPS_PTrack trk, FILE* outf)
{
  (void) fprintf(outf,"\tLatitude:    %f\n",trk->lat);
  (void) fprintf(outf,"\tLongitude:   %f\n",trk->lon);
  if (trk->Time) {
    (void) fprintf(outf,"\tTime:        %s",ctime(&trk->Time));
  } else {
    (void) fprintf(outf,"\tTime:        Computer\n");
  }
  (void) fprintf(outf,"\tAltitude:    %f\n",trk->alt);
  (void) fprintf(outf,"\tDepth:       %f\n\n",trk->dpth);

  return;
}




/* @func GPS_Fmt_Print_Waypoint *****************************************
**
** Output waypoints
**
** @param [r] way [GPS_PWay *] waypoint array
** @param [r] n [int32] number of waypoint entries
** @param [w] outf [FILE *] output stream
**
** @return [int32] success
************************************************************************/

int32 GPS_Fmt_Print_Waypoint(GPS_PWay* way, int32 n, FILE* outf)
{
  int32 i;


  if (!n) {
    return 1;
  }

  (void) fprintf(outf,"Waypoints Type: %d\n#\nStart\n#\n",
                 (int)way[0]->prot);


  for (i=0; i<n; ++i) {
    switch (way[i]->prot) {
    case 100:
      GPS_Fmt_Print_Way100(way[i],outf);
      break;
    case 101:
      GPS_Fmt_Print_Way101(way[i],outf);
      break;
    case 102:
      GPS_Fmt_Print_Way102(way[i],outf);
      break;
    case 103:
      GPS_Fmt_Print_Way103(way[i],outf);
      break;
    case 104:
      GPS_Fmt_Print_Way104(way[i],outf);
      break;
    case 105:
      GPS_Fmt_Print_Way105(way[i],outf);
      break;
    case 106:
      GPS_Fmt_Print_Way106(way[i],outf);
      break;
    case 107:
      GPS_Fmt_Print_Way107(way[i],outf);
      break;
    case 108:
      GPS_Fmt_Print_Way108(way[i],outf);
      break;
    case 109:
      GPS_Fmt_Print_Way109(way[i],outf);
      break;
    case 150:
      GPS_Fmt_Print_Way150(way[i],outf);
      break;
    case 151:
      GPS_Fmt_Print_Way151(way[i],outf);
      break;
    case 152:
      GPS_Fmt_Print_Way152(way[i],outf);
      break;
    case 154:
      GPS_Fmt_Print_Way154(way[i],outf);
      break;
    case 155:
      GPS_Fmt_Print_Way155(way[i],outf);
      break;
    default:
      GPS_Error("Print_Waypoint: Unknown waypoint protocol");
      return PROTOCOL_ERROR;
    }
    (void) fprintf(outf,"#\n");
  }
  (void) fprintf(outf,"End\n#\n");

  return 1;
}



/* @func GPS_Fmt_Print_Proximity *****************************************
**
** Output proximity
**
** @param [r] way [GPS_PWay *] waypoint array
** @param [r] n [int32] number of waypoint entries
** @param [w] outf [FILE *] output stream
**
** @return [int32] success
************************************************************************/

int32 GPS_Fmt_Print_Proximity(GPS_PWay* way, int32 n, FILE* outf)
{
  int32 i;


  if (!n) {
    return 1;
  }

  (void) fprintf(outf,"Waypoints Type: %d\n#\nStart\n#\n",
                 (int)way[0]->prot);


  for (i=0; i<n; ++i) {
    switch (way[i]->prot) {
    case 400:
      GPS_Fmt_Print_Way100(way[i],outf);
      break;
    case 101:
      GPS_Fmt_Print_Way101(way[i],outf);
      break;
    case 102:
      GPS_Fmt_Print_Way102(way[i],outf);
      break;
    case 403:
      GPS_Fmt_Print_Way103(way[i],outf);
      break;
    case 104:
      GPS_Fmt_Print_Way104(way[i],outf);
      break;
    case 105:
      GPS_Fmt_Print_Way105(way[i],outf);
      break;
    case 106:
      GPS_Fmt_Print_Way106(way[i],outf);
      break;
    case 107:
      GPS_Fmt_Print_Way107(way[i],outf);
      break;
    case 108:
      GPS_Fmt_Print_Way108(way[i],outf);
      break;
    case 109:
      GPS_Fmt_Print_Way109(way[i],outf);
      break;
    case 450:
      GPS_Fmt_Print_Way150(way[i],outf);
      (void) fprintf(outf,"\tPindex:           %d\n",(int)way[i]->idx);
      break;
    case 151:
      GPS_Fmt_Print_Way151(way[i],outf);
      break;
    case 152:
      GPS_Fmt_Print_Way152(way[i],outf);
      break;
    case 154:
      GPS_Fmt_Print_Way154(way[i],outf);
      break;
    case 155:
      GPS_Fmt_Print_Way155(way[i],outf);
      break;
    default:
      GPS_Error("Print_Proximity: Unknown proximity protocol");
      return PROTOCOL_ERROR;
    }
    (void) fprintf(outf,"\tDistance:         %f\n",way[i]->dst);
    (void) fprintf(outf,"#\n");
  }
  (void) fprintf(outf,"End\n#\n");

  return 1;
}




/* @funcstatic GPS_Fmt_Print_Way100 *************************************
**
** Output waypoint D100
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way100(GPS_PWay way, FILE* outf)
{

  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);

  return;
}


/* @funcstatic GPS_Fmt_Print_Way101 ************************************
**
** Output waypoint D101
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way101(GPS_PWay way, FILE* outf)
{

  if (way->smbl > 176) {
    way->smbl=36;
  }

  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tSymbol:           %d [%s]\n",(int)way->smbl,
                 gps_marine_sym[way->smbl]);

  return;
}


/* @funcstatic GPS_Fmt_Print_Way102 *************************************
**
** Output waypoint D102
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way102(GPS_PWay way, FILE* outf)
{
  const char** p;
  int32  x;

  if (way->smbl < 8192) {
    p = gps_marine_sym;
    x = 0;
  } else if (way->smbl < 16384) {
    p = gps_land_sym;
    x = 8192;
  } else {
    p = gps_aviation_sym;
    x = 16384;
  }


  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tSymbol:           %d [%s]\n",(int)way->smbl,
                 p[way->smbl-x]);

  return;
}


/* @funcstatic GPS_Fmt_Print_Way103 ************************************
**
** Output waypoint D103
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way103(GPS_PWay way, FILE* outf)
{
  static const char* dspl[]= {
    "SW","S","SC"
  };

  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 gps_16_sym[way->smbl]);
  (void) fprintf(outf,"\tDisplay:          %-6d [%s]\n",(int)way->dspl,
                 dspl[way->dspl]);

  return;
}


/* @funcstatic GPS_Fmt_Print_Way104 *************************************
**
** Output waypoint D104
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way104(GPS_PWay way, FILE* outf)
{
  static const char* dspl[]= {
    "S","S","","SW","","SC"
  };
  const char** p;
  int32  x;

  if (way->smbl < 8192) {
    p = gps_marine_sym;
    x = 0;
  } else if (way->smbl < 16384) {
    p = gps_land_sym;
    x = 8192;
  } else {
    p = gps_aviation_sym;
    x = 16384;
  }

  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 p[way->smbl-x]);
  (void) fprintf(outf,"\tDisplay:          %-6d [%s]\n",(int)way->dspl,
                 dspl[way->dspl]);

  return;
}


/* @funcstatic GPS_Fmt_Print_Way105 ************************************
**
** Output waypoint D105
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way105(GPS_PWay way, FILE* outf)
{
  const char** p;
  int32  x;

  if (way->smbl < 8192) {
    p = gps_marine_sym;
    x = 0;
  } else if (way->smbl < 16384) {
    p = gps_land_sym;
    x = 8192;
  } else {
    p = gps_aviation_sym;
    x = 16384;
  }

  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 p[way->smbl-x]);
  (void) fprintf(outf,"\tWpt_ident         %s\n",way->wpt_ident);

  return;
}


/* @funcstatic GPS_Fmt_Print_Way106 *************************************
**
** Output waypoint D106
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way106(GPS_PWay way, FILE* outf)
{
  const char** p;
  int32  x;

  if (way->smbl < 8192) {
    p = gps_marine_sym;
    x = 0;
  } else if (way->smbl < 16384) {
    p = gps_land_sym;
    x = 8192;
  } else {
    p = gps_aviation_sym;
    x = 16384;
  }

  if (!way->wpt_class) {
    (void) fprintf(outf,"\tClass:            %d [User]\n",way->wpt_class);
    (void) fprintf(outf,"\tSubclass:         %d [%-13.13s]\n",
                   way->wpt_class,way->subclass);
    (void) fprintf(outf,"\tSubclass:\n");
  } else {
    (void) fprintf(outf,"\tClass:            %d [Non-user]\n",
                   way->wpt_class);
    (void) fprintf(outf,"\tSubclass:         %d [%13.13s]\n",
                   way->wpt_class,
                   way->subclass);
  }
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 p[way->smbl-x]);
  (void) fprintf(outf,"\tWpt_ident         %s\n",way->wpt_ident);
  (void) fprintf(outf,"\tLnk_ident         %s\n",way->lnk_ident);

  return;
}


/* @funcstatic GPS_Fmt_Print_Way107 ************************************
**
** Output waypoint D107
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way107(GPS_PWay way, FILE* outf)
{
  static const char* dspl[]= {
    "SW","S","SC"
  };
  static const char* col[]= {
    "Default","Red","Green","Blue"
  };


  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 gps_16_sym[way->smbl]);
  (void) fprintf(outf,"\tDisplay:          %-6d [%s]\n",(int)way->dspl,
                 dspl[way->dspl]);
  (void) fprintf(outf,"\tColour:           %-6d [%s]\n",(int)way->colour,
                 col[way->colour]);

  return;
}



/* @funcstatic GPS_Fmt_Print_Way108 ************************************
**
** Output waypoint D108
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way108(GPS_PWay way, FILE* outf)
{
  const char** p;
  int32  x;

  static const char* dspl[]= {
    "SW","S","SC"
  };

  static const char* col[]= {
    "Black","Dark_Red","Dark_Green","Dark_Yellow","Dark_Blue",
    "Dark_Magenta","Dark_Cyan","Light_Grey","Dark_Grey","Red","Green",
    "Yellow","Blue","Magenta","Cyan","White"
  };


  if (way->smbl < 8192) {
    p = gps_marine_sym;
    x = 0;
  } else if (way->smbl < 16384) {
    p = gps_land_sym;
    x = 8192;
  } else {
    p = gps_aviation_sym;
    x = 16384;
  }

  (void) fprintf(outf,"\tIdent:            %s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  if (way->colour==0xff) {
    (void) fprintf(outf,"\tColour:           255    [Default]\n");
  } else
    (void) fprintf(outf,"\tColour:           %-6d [%s]\n",(int)way->colour,
                   col[way->colour]);
  (void) fprintf(outf,"\tDisplay:          %-6d [%s]\n",(int)way->dspl,
                 dspl[way->dspl]);
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 p[way->smbl-x]);
  (void) fprintf(outf,"\tAltitude:         %d\n",(int)way->alt);
  (void) fprintf(outf,"\tDepth:            %f\n",way->dpth);
  (void) fprintf(outf,"\tState:            %-2.2s\n",way->state);
  (void) fprintf(outf,"\tCountry:          %-2.2s\n",way->cc);
  (void) fprintf(outf,"\tClass:            %d\n",way->wpt_class);
  if (way->wpt_class>=0x80 && way->wpt_class<=0x85) {
    (void) fprintf(outf,"\tSubclass:         %18.18s\n",way->subclass);
  }
  if (!way->wpt_class) {
    (void) fprintf(outf,"\tComment:          %s\n",way->cmnt);
  }
  if (way->wpt_class>=0x40 && way->wpt_class<=0x46) {
    (void) fprintf(outf,"\tFacility:         %s\n",way->facility);
    (void) fprintf(outf,"\tCity:             %s\n",way->city);
  }
  if (way->wpt_class==0x83) {
    (void) fprintf(outf,"\tAddress:          %s\n",way->addr);
  }
  if (way->wpt_class==0x82) {
    (void) fprintf(outf,"\tCross Road:       %s\n",way->cross_road);
  }


  return;
}

/* @funcstatic GPS_Fmt_Print_Way109 ************************************
**
** Output waypoint D109
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way109(GPS_PWay way, FILE* outf)
{
  const char** p;
  int32  x;

  static const char* dspl[]= {
    "SW","S","SC"
  };

  static const char* col[]= {
    "Black","Dark_Red","Dark_Green","Dark_Yellow","Dark_Blue",
    "Dark_Magenta","Dark_Cyan","Light_Grey","Dark_Grey","Red","Green",
    "Yellow","Blue","Magenta","Cyan","White"
  };


  if (way->smbl < 8192) {
    p = gps_marine_sym;
    x = 0;
  } else if (way->smbl < 16384) {
    p = gps_land_sym;
    x = 8192;
  } else {
    p = gps_aviation_sym;
    x = 16384;
  }

  (void) fprintf(outf,"\tIdent:            %s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  if (way->colour==0xff) {
    (void) fprintf(outf,"\tColour:           255    [Default]\n");
  } else
    (void) fprintf(outf,"\tColour:           %-6d [%s]\n",(int)way->colour,
                   col[way->colour]);
#if 0
  /* avoid bounds violation in D109.   Probably masking a bug elswhere...*/
  (void) fprintf(outf,"\tDisplay:          %-6d [%s]\n",(int)way->dspl,
                 dspl[way->dspl]);
#endif
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 p[way->smbl-x]);
  (void) fprintf(outf,"\tAltitude:         %d\n",(int)way->alt);
  (void) fprintf(outf,"\tDepth:            %f\n",way->dpth);
  (void) fprintf(outf,"\tState:            %-2.2s\n",way->state);
  (void) fprintf(outf,"\tCountry:          %-2.2s\n",way->cc);
  (void) fprintf(outf,"\tClass:            %d\n",way->wpt_class);
  if (way->wpt_class>=0x80 && way->wpt_class<=0x85) {
    (void) fprintf(outf,"\tSubclass:         %18.18s\n",way->subclass);
  }
  if (!way->wpt_class) {
    (void) fprintf(outf,"\tComment:          %s\n",way->cmnt);
  }
  if (way->wpt_class>=0x40 && way->wpt_class<=0x46) {
    (void) fprintf(outf,"\tFacility:         %s\n",way->facility);
    (void) fprintf(outf,"\tCity:             %s\n",way->city);
  }
  if (way->wpt_class==0x83) {
    (void) fprintf(outf,"\tAddress:          %s\n",way->addr);
  }
  if (way->wpt_class==0x82) {
    (void) fprintf(outf,"\tCross Road:       %s\n",way->cross_road);
  }


  return;
}

/* @funcstatic GPS_Fmt_Print_Way150 *************************************
**
** Output waypoint D150
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way150(GPS_PWay way, FILE* outf)
{

  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tClass:            %d\n",way->wpt_class);
  if (way->wpt_class!=4) {
    (void) fprintf(outf,"\tCountry:          %-2.2s\n",way->cc);
    (void) fprintf(outf,"\tCity:             %-24.24s\n",way->city);
    (void) fprintf(outf,"\tState:            %-2.2s\n",way->state);
    (void) fprintf(outf,"\tName:             %-30.30s\n",way->name);
  }
  if (!way->wpt_class) {
    (void) fprintf(outf,"\tAltitude:         %d\n",(int)way->alt);
  }

  return;
}


/* @funcstatic GPS_Fmt_Print_Way151 ************************************
**
** Output waypoint D151
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way151(GPS_PWay way, FILE* outf)
{

  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tClass:            %d\n",way->wpt_class);
  if (way->wpt_class!=2) {
    (void) fprintf(outf,"\tCountry:          %-2.2s\n",way->cc);
    (void) fprintf(outf,"\tCity:             %-24.24s\n",way->city);
    (void) fprintf(outf,"\tState:            %-2.2s\n",way->state);
    (void) fprintf(outf,"\tName:             %-30.30s\n",way->name);
  }
  if (!way->wpt_class) {
    (void) fprintf(outf,"\tAltitude:         %d\n",(int)way->alt);
  }

  return;
}



/* @funcstatic GPS_Fmt_Print_Way152 ************************************
**
** Output waypoint D152
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way152(GPS_PWay way, FILE* outf)
{

  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tClass:            %d\n",way->wpt_class);
  if (way->wpt_class!=4) {
    (void) fprintf(outf,"\tCountry:          %-2.2s\n",way->cc);
    (void) fprintf(outf,"\tCity:             %-24.24s\n",way->city);
    (void) fprintf(outf,"\tState:            %-2.2s\n",way->state);
    (void) fprintf(outf,"\tName:             %-30.30s\n",way->name);
  }
  if (!way->wpt_class) {
    (void) fprintf(outf,"\tAltitude:         %d\n",(int)way->alt);
  }

  return;
}


/* @funcstatic GPS_Fmt_Print_Way154 ************************************
**
** Output waypoint D154
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way154(GPS_PWay way, FILE* outf)
{
  const char** p;
  int32  x;

  if (way->smbl < 8192) {
    p = gps_marine_sym;
    x = 0;
  } else if (way->smbl < 16384) {
    p = gps_land_sym;
    x = 8192;
  } else {
    p = gps_aviation_sym;
    x = 16384;
  }


  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 p[way->smbl-x]);
  (void) fprintf(outf,"\tClass:            %d\n",way->wpt_class);
  if (way->wpt_class!=4 && way->wpt_class!=8) {
    (void) fprintf(outf,"\tCountry:          %-2.2s\n",way->cc);
    (void) fprintf(outf,"\tCity:             %-24.24s\n",way->city);
    (void) fprintf(outf,"\tState:            %-2.2s\n",way->state);
    (void) fprintf(outf,"\tName:             %-30.30s\n",way->name);
  }
  if (!way->wpt_class) {
    (void) fprintf(outf,"\tAltitude:         %d\n",(int)way->alt);
  }

  return;
}


/* @funcstatic GPS_Fmt_Print_Way155 *************************************
**
** Output waypoint D155
**
** @param [r] way [GPS_PWay] waypoint
** @param [w] outf [FILE *] output stream
**
** @return [void]
************************************************************************/

static void GPS_Fmt_Print_Way155(GPS_PWay way, FILE* outf)
{
  static const char* dspl[]= {
    "","S","","SW","","SC"
  };

  const char** p;
  int32  x;

  if (way->smbl < 8192) {
    p = gps_marine_sym;
    x = 0;
  } else if (way->smbl < 16384) {
    p = gps_land_sym;
    x = 8192;
  } else {
    p = gps_aviation_sym;
    x = 16384;
  }


  (void) fprintf(outf,"\tIdent:            %-6.6s\n",way->ident);
  (void) fprintf(outf,"\tLatitude:         %f\n",way->lat);
  (void) fprintf(outf,"\tLongitude:        %f\n",way->lon);
  (void) fprintf(outf,"\tComment:          %-40.40s\n",way->cmnt);
  (void) fprintf(outf,"\tSymbol:           %-6d [%s]\n",(int)way->smbl,
                 p[way->smbl-x]);
  (void) fprintf(outf,"\tDisplay:          %-6d [%s]\n",(int)way->dspl,
                 dspl[way->dspl]);
  (void) fprintf(outf,"\tClass:            %d\n",way->wpt_class);
  if (way->wpt_class!=4 && way->wpt_class!=8) {
    (void) fprintf(outf,"\tCountry:          %-2.2s\n",way->cc);
    (void) fprintf(outf,"\tCity:             %-24.24s\n",way->city);
    (void) fprintf(outf,"\tState:            %-2.2s\n",way->state);
    (void) fprintf(outf,"\tName:             %-30.30s\n",way->name);
  }
  if (!way->wpt_class) {
    (void) fprintf(outf,"\tAltitude:         %d\n",(int)way->alt);
  }

  return;
}



/* @func GPS_Fmt_Print_Route *****************************************
**
** Output route(s)
**
** @param [r] way [GPS_PWay *] waypoint array
** @param [r] n [int32] number of waypoint entries
** @param [w] outf [FILE *] output stream
**
** @return [int32] success
************************************************************************/

int32 GPS_Fmt_Print_Route(GPS_PWay* way, int32 n, FILE* outf)
{
  int32 i;
  int32 first;

  if (!n) {
    return 1;
  }


  switch (gps_route_transfer) {
  case pA200:
    break;
  case pA201:
    return GPS_Fmt_Print_Route201(way,n,outf);
  default:
    GPS_Error("GPS_Fmt_Print_Route: Unknown protocol");
    return PROTOCOL_ERROR;
  }


  (void) fprintf(outf,"Route log 200 %d\n#\n",(int)gps_rte_type);
  (void) fprintf(outf,"Start\n#\n");



  first = 1;

  for (i=0; i<n; ++i) {
    if (way[i]->isrte) {
      if (!first) {
        (void) fprintf(outf,"End\n#\n");
      }
      (void) fprintf(outf,"Route Type: %d ",(int)way[i]->rte_prot);
      first=0;

      switch (way[i]->rte_prot) {
      case 200:
        (void) fprintf(outf,"Number: %d",way[i]->rte_num);
        break;
      case 201:
        (void) fprintf(outf,"Number: %d Comment: %-20.20s",
                       way[i]->rte_num,way[i]->rte_cmnt);
        break;
      case 202:
        (void) fprintf(outf,"Comment: %s",way[i]->rte_ident);
        break;
      default:
        GPS_Error("Print_Route: Unknown route protocol");
        return PROTOCOL_ERROR;
      }
      (void) fprintf(outf,"\n#\n");
      (void) fprintf(outf,"Waypoints Type: %d\n#\nStart\n#\n"
                     ,(int)way[i]->prot);
      continue;
    }

    switch (way[i]->prot) {
    case 100:
      GPS_Fmt_Print_Way100(way[i],outf);
      break;
    case 101:
      GPS_Fmt_Print_Way101(way[i],outf);
      break;
    case 102:
      GPS_Fmt_Print_Way102(way[i],outf);
      break;
    case 103:
      GPS_Fmt_Print_Way103(way[i],outf);
      break;
    case 104:
      GPS_Fmt_Print_Way104(way[i],outf);
      break;
    case 105:
      GPS_Fmt_Print_Way105(way[i],outf);
      break;
    case 106:
      GPS_Fmt_Print_Way106(way[i],outf);
      break;
    case 107:
      GPS_Fmt_Print_Way107(way[i],outf);
      break;
    case 108:
      GPS_Fmt_Print_Way108(way[i],outf);
      break;
    case 109:
      GPS_Fmt_Print_Way109(way[i],outf);
      break;
    case 150:
      GPS_Fmt_Print_Way150(way[i],outf);
      break;
    case 151:
      GPS_Fmt_Print_Way151(way[i],outf);
      break;
    case 152:
      GPS_Fmt_Print_Way152(way[i],outf);
      break;
    case 154:
      GPS_Fmt_Print_Way154(way[i],outf);
      break;
    case 155:
      GPS_Fmt_Print_Way155(way[i],outf);
      break;
    default:
      GPS_Error("Print_Route: Unknown waypoint protocol");
      return PROTOCOL_ERROR;
    }
    (void) fprintf(outf,"#\n");
  }
  (void) fprintf(outf,"End\n#\n");

  return 1;
}



/* @funcstatic GPS_Fmt_Print_Route201 ***********************************
**
** Output route(s)
**
** @param [r] way [GPS_PWay *] waypoint array
** @param [r] n [int32] number of waypoint entries
** @param [w] outf [FILE *] output stream
**
** @return [int32] success
************************************************************************/

static int32 GPS_Fmt_Print_Route201(GPS_PWay* way, int32 n, FILE* outf)
{
  int32 i;
  int32 first;

  if (!n) {
    return 1;
  }


  (void) fprintf(outf,"Route log 201 %d\n#\n",(int)gps_rte_link_type);
  (void) fprintf(outf,"Start\n#\n");


  first = 1;

  for (i=0; i<n; ++i) {
    if (way[i]->isrte) {
      if (!first) {
        (void) fprintf(outf,"End\n#\n");
      }
      (void) fprintf(outf,"Route Type: %d ",(int)way[i]->rte_prot);
      first=0;

      switch (way[i]->rte_prot) {
      case 200:
        (void) fprintf(outf,"Number: %d",way[i]->rte_num);
        break;
      case 201:
        (void) fprintf(outf,"Number: %d Comment: %-20.20s",
                       way[i]->rte_num,way[i]->rte_cmnt);
        break;
      case 202:
        (void) fprintf(outf,"Comment: %s",way[i]->rte_ident);
        break;
      default:
        GPS_Error("Print_Route: Unknown route protocol");
        return PROTOCOL_ERROR;
      }
      (void) fprintf(outf,"\n#\n");
      (void) fprintf(outf,"Waypoints Type: %d\n#\n"
                     ,(int)way[i]->prot);
      continue;
    }


    if (way[i]->islink) {
      (void) fprintf(outf,"\tLink Class:       %d\n",
                     (int)way[i]->rte_link_class);
      if (!(way[i]->rte_link_class==3 || way[i]->rte_link_class==0xff))
        (void) fprintf(outf,"\tLink Subclass:    %-18.18s\n",
                       way[i]->rte_link_subclass);
      (void) fprintf(outf,"\tLink Ident:       %s\n#\n",
                     way[i]->rte_link_ident);
      continue;
    }


    switch (way[i]->prot) {
    case 100:
      GPS_Fmt_Print_Way100(way[i],outf);
      break;
    case 101:
      GPS_Fmt_Print_Way101(way[i],outf);
      break;
    case 102:
      GPS_Fmt_Print_Way102(way[i],outf);
      break;
    case 103:
      GPS_Fmt_Print_Way103(way[i],outf);
      break;
    case 104:
      GPS_Fmt_Print_Way104(way[i],outf);
      break;
    case 105:
      GPS_Fmt_Print_Way105(way[i],outf);
      break;
    case 106:
      GPS_Fmt_Print_Way106(way[i],outf);
      break;
    case 107:
      GPS_Fmt_Print_Way107(way[i],outf);
      break;
    case 108:
      GPS_Fmt_Print_Way108(way[i],outf);
      break;
    case 109:
      GPS_Fmt_Print_Way109(way[i],outf);
      break;
    case 150:
      GPS_Fmt_Print_Way150(way[i],outf);
      break;
    case 151:
      GPS_Fmt_Print_Way151(way[i],outf);
      break;
    case 152:
      GPS_Fmt_Print_Way152(way[i],outf);
      break;
    case 154:
      GPS_Fmt_Print_Way154(way[i],outf);
      break;
    case 155:
      GPS_Fmt_Print_Way155(way[i],outf);
      break;
    default:
      GPS_Error("Print_Route: Unknown waypoint protocol");
      return PROTOCOL_ERROR;
    }
    (void) fprintf(outf,"#\n");
  }
  (void) fprintf(outf,"End\n");

  return 1;
}
