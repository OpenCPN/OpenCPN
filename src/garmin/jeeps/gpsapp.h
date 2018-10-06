/********************************************************************
** @source JEEPS application and data functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @modified Copyright (C) 2004, 2005, 2006 Robert Lipe
** @modified Copyright (C) 2007 Achim Schumacher
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
** Boston, MA 02110-1301,  USA.
********************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gpsapp_h
#define gpsapp_h


#include "garmin_gps.h"

void VerifyPortClosed();

int32  GPS_Init(const char *port);

int32  GPS_A100_Get(const char *port, GPS_PWay **way, int (*cb)(int ct, GPS_PWay *));
int32 GPS_A101_Get(const char *port);
int32  GPS_A100_Send(const char *port, GPS_PWay *way, int32 n, int (*cb)(GPS_PWay *));

int32  GPS_A200_Get(const char *port, GPS_PWay **way);
int32  GPS_A201_Get(const char *port, GPS_PWay **way);
int32  GPS_A200_Send(const char *port, GPS_PWay *way, int32 n);
int32  GPS_A201_Send(const char *port, GPS_PWay *way, int32 n);

int32  GPS_A300_Get(const char *port, GPS_PTrack **trk, pcb_fn cb);
int32  GPS_A301_Get(const char *port, GPS_PTrack **trk, pcb_fn cb, int protoid);
int32  GPS_A300_Send(const char *port, GPS_PTrack *trk, int32 n);
int32  GPS_A301_Send(const char *port, GPS_PTrack *trk, int32 n, int protoid,
                    gpsdevh *fd);

int32  GPS_D300_Get(GPS_PTrack *trk, int32 entries, gpsdevh *h);
void   GPS_D300b_Get(GPS_PTrack *trk, UC *data);
void   GPS_D301b_Get(GPS_PTrack *trk, UC *data);
void   GPS_D302b_Get(GPS_PTrack *trk, UC *data);
void   GPS_D303b_Get(GPS_PTrack *trk, UC *data); /*D304*/
void   GPS_D310_Get(GPS_PTrack *trk, UC *s);
void   GPS_D311_Get(GPS_PTrack *trk, UC *s);
void   GPS_D300_Send(UC *data, GPS_PTrack trk, int32 *len);
void   GPS_D301_Send(UC *data, GPS_PTrack trk, int32 *len, int type);
void   GPS_D303_Send(UC *data, GPS_PTrack trk, int32 *len, int protoid);
void   GPS_D310_Send(UC *data, GPS_PTrack trk, int32 *len);
void   GPS_D311_Send(UC *data, GPS_PTrack trk, int32 *len);

int32  GPS_A400_Get(const char *port, GPS_PWay **way);
int32  GPS_A400_Send(const char *port, GPS_PWay *way, int32 n);

int32  GPS_A500_Get(const char *port, GPS_PAlmanac **alm);
int32  GPS_A500_Send(const char *port, GPS_PAlmanac *alm, int32 n);

time_t GPS_A600_Get(const char *port);
time_t GPS_D600_Get(GPS_PPacket packet);
int32  GPS_A600_Send(const char *port, time_t Time);
void   GPS_D600_Send(GPS_PPacket *packet, time_t Time);

int32  GPS_A700_Get(const char *port, double *lat, double *lon);
int32  GPS_A700_Send(const char *port, double lat, double lon);
void   GPS_D700_Get(GPS_PPacket packet, double *lat, double *lon);
void   GPS_D700_Send(GPS_PPacket *packet, double lat, double lon);

int32  GPS_A800_On(const char *port, gpsdevh **fd);
int32  GPS_A800_Off(const char *port, gpsdevh **fd);
int32  GPS_A800_Get(gpsdevh **fd, GPS_PPvt_Data *packet);
void   GPS_D800_Get(GPS_PPacket packet, GPS_PPvt_Data *pvt);

int32  GPS_A906_Get(const char *port, GPS_PLap **lap, pcb_fn cb);
void   GPS_D1011b_Get(GPS_PLap *Lap,UC *data); /*D906 D1001 D1015*/

int32  GPS_A1006_Get(const char *port, GPS_PCourse **crs, pcb_fn cb);
int32  GPS_A1006_Send(const char *port, GPS_PCourse *crs, int32 n_crs,
                      gpsdevh *fd);
void   GPS_D1006_Get(GPS_PCourse *crs, UC *p);
void   GPS_D1006_Send(UC *data, GPS_PCourse crs, int32 *len);

int32  GPS_A1007_Get(const char *port, GPS_PCourse_Lap **clp, pcb_fn cb);
int32  GPS_A1007_Send(const char *port, GPS_PCourse_Lap *clp, int32 n_clp,
                      gpsdevh *fd);
void   GPS_D1007_Get(GPS_PCourse_Lap *clp, UC *p);
void   GPS_D1007_Send(UC *data, GPS_PCourse_Lap clp, int32 *len);

int32  GPS_A1008_Get(const char *port, GPS_PCourse_Point **cpt, pcb_fn cb);
int32  GPS_A1008_Send(const char *port, GPS_PCourse_Point *cpt, int32 n_cpt,
                      gpsdevh *fd);
void   GPS_D1012_Get(GPS_PCourse_Point *cpt, UC *p);
void   GPS_D1012_Send(UC *data, GPS_PCourse_Point cpt, int32 *len);

int32  GPS_A1009_Get(const char *port, GPS_PCourse_Limits limits);
void   GPS_D1013_Get(GPS_PCourse_Limits limits, UC *p);

/* Unhandled documented protocols, as of:
  Garmin Device Interface Specification, May 19, 2006, Drawing Number: 001-00063-00 Rev. C
A650  FlightBook Transfer Protocol
A1000  Run Transfer Protocol
	Capability A1000: D1009
		D1000 D1010
A1002  Workout Transfer Protocol
	Capability A1002: D1008
		D1002
	Capability A1003: D1003
A1004  Fitness User Profile Transfer Protocol
	Capability A1004: D1004
A1005  Workout Limits Transfer Protocol
	Capability A1005: D1005
*/
/* Unimplemted and Undocumented, as listed from the following device/sw:
	GF305 3.70

Capability A601: D601
Capability A801: D801

Capability A902:
Capability A903:
Capability A907: D907 D908 D909 D910
Capability A918: D918
Capability A1013: D1014
*/

const char * Get_Pkt_Type(US p, US d0, const char **xinfo);

void GPS_Prepare_Track_For_Device(GPS_PTrack **trk, int32 *n);

#endif

#ifdef __cplusplus
}
#endif
