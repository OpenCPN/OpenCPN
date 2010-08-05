/*
    Garmin Jeeps - OpenCPN Interface Wrapper.

    Copyright (C) 2010 David S Register, bdbcat@yahoo.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

 */

#include "garmin_wrapper.h"

#include "gpsapp.h"
#include "garmin_gps.h"

/*  Wrapped interface from higher level objects   */
int Garmin_GPS_Init(ComPortManager *pPortMan, wxString &port_name)
{
      return GPS_Init(port_name.mb_str());
}

wxString Garmin_GPS_GetSaveString()
{
      return wxString(gps_save_string,  wxConvUTF8);
}

int Garmin_GPS_SendWaypoints(ComPortManager *pPortMan, wxString &port_name, RoutePointList *wplist)
{
      int ret_val = 0;

      int nPoints = wplist->GetCount();

      // Create the array of GPS_PWays

      GPS_SWay **ppway = (GPS_SWay **)malloc(nPoints * sizeof(GPS_PWay));

      //    and the GPS_Oways themselves
      for(int i=0 ; i < nPoints ; i++)
            ppway[i] = GPS_Way_New();


      //    Now fill in the useful elements
      for(int i=0 ; i < nPoints ; i++)
      {
            GPS_PWay pway = ppway[i];
            wxRoutePointListNode *node = wplist->Item(i);
            RoutePoint *prp = node->GetData();

            pway->lat = prp->m_lat;
            pway->lon = prp->m_lon;
            strncpy(pway->ident, (prp->m_MarkName.Truncate ( 6 )).mb_str(), 6);

      }

      //    Transmit the list to the GPS receiver
      int xfer_result = GPS_A100_Send(port_name.mb_str(), ppway, nPoints, 0 /*int (*cb)(GPS_PWay *)*/);
      ret_val = xfer_result;

      //  Free all the memory
      for(int i=0 ; i < nPoints ; i++)
            GPS_Way_Del(&ppway[i]);

      free(ppway);

      return ret_val;
}

int Garmin_GPS_SendRoute(ComPortManager *pPortMan, wxString &port_name, Route *pr)
{
      int ret_val = 0;

      int route_number = 1;

      //    If the device supports unique numbered waypoints,
      //    Then we must query the device to find an empty number
      if((gps_rte_hdr_type == pD200) || (gps_rte_hdr_type == pD201))
      {
      //    Retrieve <ALL> routes from the device
            GPS_PWay *pprouteway;
            int32 npacks = GPS_A200_Get(port_name.mb_str(), &pprouteway);
            if(npacks < 0)
                  return npacks;

            //  Iterate on the packets, finding the first route number from [0..9] that is not present

            //    An array of route numbers, set element to true as encountered
            bool brn[10];
            for(int i=0 ; i < 10 ; i++)
               brn[i] = false;

            for(int ip=0 ; ip < npacks ; ip++)
            {
                  GPS_PWay pway = pprouteway[ip];
                  if(pway->isrte)
                  {
                        if((pway->rte_num < 10))
                              brn[pway->rte_num] = true;
                  }
            }

            //    Find the first candidate within [1..9] that is unused
            for(int i=1 ; i < 10 ; i++)
            {
                  if(brn[i] == false)
                  {
                        route_number = i;
                        break;
                  }
            }
      }


      RoutePointList *wplist = pr->pRoutePointList;
      int nPoints = wplist->GetCount();

      // Create the array of GPS_PWays
      // There will be one extra for the route header

      GPS_SWay **ppway = (GPS_SWay **)malloc((nPoints+1) * sizeof(GPS_PWay));

      //    and the GPS_Oways themselves
      for(int i=0 ; i < nPoints+1 ; i++)
            ppway[i] = GPS_Way_New();


      //    Now fill in the useful elements

      //    Element 0 is a route record

      GPS_PWay pway = ppway[0];
      pway->isrte = true;
      pway->rte_num = route_number;
      strncpy(pway->rte_ident, (pr->m_RouteNameString.Truncate ( 255 )).mb_str(), 255);
      strncpy(pway->rte_cmnt, (pr->m_RouteNameString.Truncate ( 19 )).mb_str(), 19);


      //    Elements 1..n are waypoints
      for(int i=1 ; i < nPoints+1 ; i++)
      {
            GPS_PWay pway = ppway[i];
            wxRoutePointListNode *node = wplist->Item(i-1);
            RoutePoint *prp = node->GetData();

            pway->lat = prp->m_lat;
            pway->lon = prp->m_lon;
            strncpy(pway->ident, (prp->m_MarkName.Truncate ( 6 )).mb_str(), 6);
      }

      //    Transmit the Route to the GPS receiver
      int xfer_result = GPS_A200_Send(port_name.mb_str(), ppway, nPoints+1);
      ret_val = xfer_result;

      //  Free all the memory
      for(int i=0 ; i < nPoints ; i++)
            GPS_Way_Del(&ppway[i]);

      free(ppway);

      return ret_val;
}


