/*
    Garmin Jeeps - OpenCPN Interface Wrapper.

    Copyright (C) 2010 David S Register

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA

 */

#include "garmin_wrapper.h"

#include "gpsapp.h"
#include "garmin_gps.h"
#include "gpsserial.h"

#define GPS_DEBUG

extern char last_error[];

gpsdevh *my_gps_devh;


wxString GetLastGarminError(void)
{
      return wxString(GetDeviceLastError(),  wxConvUTF8);
}

/*
      Verify and force port closure after every called routine.
      This is necessary for aborts in Windows environment, since ports cannot be multiply opened.
*/

/*  Wrapped interface from higher level objects   */
int Garmin_GPS_Init( wxString &port_name)
{
      int ret;
#ifdef GPS_DEBUG0
//      if (getenv("OPENCPN_GPS_ERROR") != NULL)
	GPS_Enable_Error();
//      if (getenv("OPENCPN_GPS_WARNING") != NULL)
	GPS_Enable_Warning();
//      if (getenv("OPENCPN_GPS_USER") != NULL)
	GPS_Enable_User();
//      if (getenv("OPENCPN_GPS_DIAGNOSE") != NULL)
	GPS_Enable_Diagnose();
#endif
      char m[1];
      m[0] = '\0';

      GPS_Error(m);

      ret = GPS_Init(port_name.mb_str());
      VerifyPortClosed();

      return ret;
}

int Garmin_GPS_Open( wxString &port_name )
{
    return GPS_Init(port_name.mb_str());
}


int Garmin_GPS_PVT_On( wxString &port_name )
{
    return Garmin_Serial_GPS_PVT_On( port_name.mb_str() );
}

int Garmin_GPS_PVT_Off( wxString &port_name )
{
    return Garmin_Serial_GPS_PVT_Off( port_name.mb_str() );
}

int Garmin_GPS_GetPVT(void *pvt)
{
    return GPS_Serial_Command_Pvt_Get((GPS_PPvt_Data *)pvt );
    
}

void Garmin_GPS_ClosePortVerify(void)
{
    VerifyPortClosed();
}

wxString Garmin_GPS_GetSaveString()
{
      return wxString(gps_save_string,  wxConvUTF8);
}

void Garmin_GPS_PrepareWptData(GPS_PWay pway, RoutePoint *prp)
{
      pway->lat = prp->m_lat;
      pway->lon = prp->m_lon;
      pway->alt_is_unknown = 1;
      pway->alt = 0.0;
      strncpy(pway->ident, (prp->GetName().Truncate ( 6 )).mb_str(), 6);
}

int Garmin_GPS_SendWaypoints( wxString &port_name, RoutePointList *wplist)
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

            Garmin_GPS_PrepareWptData(pway, prp);
      }


      //    Transmit the list to the GPS receiver
      int xfer_result = GPS_Command_Send_Waypoint(port_name.mb_str(), ppway, nPoints, 0 /*int (*cb)(GPS_PWay *)*/);
      ret_val = xfer_result;

      //  Free all the memory
      for(int i=0 ; i < nPoints ; i++)
            GPS_Way_Del(&ppway[i]);

      free(ppway);

      VerifyPortClosed();
      return ret_val;
}

// This routine creates an array of waypoint structures for an A200 route
// transfer according to the "Garmin GPS Interface Specification" page 20.
// The returned array contains the following packets:
//
//    route header packet
//    waypoint packet
//    ...
//    waypoint packet
//
// The total number of elements in the array (route header packet and
// all waypoint packets) is returned in the "size" argument.

GPS_SWay **Garmin_GPS_Create_A200_Route(Route *pr, int route_number, int *size)
{
      RoutePointList *wplist = pr->pRoutePointList;
      int nPoints = wplist->GetCount();

      // Create the array of GPS_PWays
      // There will be one extra for the route header

      *size = nPoints + 1;

      GPS_SWay **ppway = (GPS_SWay **)malloc((*size) * sizeof(GPS_PWay));

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
      for(int i=1 ; i < *size ; i++)
      {
            GPS_PWay pway = ppway[i];
            wxRoutePointListNode *node = wplist->Item(i-1);
            RoutePoint *prp = node->GetData();

            Garmin_GPS_PrepareWptData(pway, prp);
      }

      return ppway;
}

// This routine creates an array of waypoint structures for an A201 route
// transfer according to the "Garmin GPS Interface Specification" page 21.
// The returned array contains the following packets:
//
//    route header packet
//    waypoint packet
//    link packet
//    waypoint packet
//    link packet
//    ...
//    waypoint packet
//
// The total number of elements in the array (route header packet, link
// packets and waypoint packets) is returned in the "size" argument.

GPS_SWay **Garmin_GPS_Create_A201_Route(Route *pr, int route_number, int *size)
{
      RoutePointList *wplist = pr->pRoutePointList;
      int nPoints = wplist->GetCount();

      // Create the array of GPS_PWays
      // There will be one for the route header, n for each way point
      //  and n-1 for each link

      *size = 1 + nPoints + (nPoints - 1);

      GPS_SWay **ppway = (GPS_SWay **)malloc((*size) * sizeof(GPS_PWay));

      //    and the GPS_Oways themselves
      for(int i=0 ; i < *size ; i++)
            ppway[i] = GPS_Way_New();


      //    Now fill in the useful elements

      //    Element 0 is a route record

      GPS_PWay pway = ppway[0];
      pway->isrte = true;
      pway->rte_num = route_number;
      strncpy(pway->rte_ident, (pr->m_RouteNameString.Truncate ( 255 )).mb_str(), 255);
      strncpy(pway->rte_cmnt, (pr->m_RouteNameString.Truncate ( 19 )).mb_str(), 19);


      //    Odd elements 1,3,5... are waypoints
      //    Even elements 2,4,6... are links
      for(int i=1 ; i < *size ; i++)
      {
	    if (i % 2 == 1) /* Odd */
	    {
	          GPS_PWay pway = ppway[i];
                  wxRoutePointListNode *node = wplist->Item((i-1)/2);
                  RoutePoint *prp = node->GetData();

                  Garmin_GPS_PrepareWptData(pway, prp);
	    }
	    else  /* Even */
	    {
	          /* Apparently, 0 filled links are OK */
	          GPS_PWay pway = ppway[i];
		  pway->islink = true;
		  pway->rte_link_class = 0;
		  memset(pway->rte_link_subclass, 0, sizeof(pway->rte_link_subclass));
		  memset(pway->rte_link_ident, 0 , sizeof(pway->rte_link_ident));
	    }
      }

      return ppway;
}

int Garmin_GPS_SendRoute( wxString &port_name, Route *pr, wxGauge *pProgress)
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

            if ( pProgress )
            {
                  pProgress->SetValue ( 60 );
                  pProgress->Refresh();
                  pProgress->Update();
            }

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
            bool bfound_empty = false;
            for(int i=1 ; i < 10 ; i++)
            {
                  if(brn[i] == false)
                  {
                        route_number = i;
                        bfound_empty = true;
                        break;
                  }
            }

            //  Ask the user if it is all right to overwrite
            if(!bfound_empty)
            {
                  int rv = OCPNMessageBox(NULL, _("Overwrite Garmin device route number 1?"),
                                          _("OpenCPN Message"), wxOK | wxCANCEL | wxICON_QUESTION);
                  if(rv != wxOK)
                        return 0;
            }

      }


      // Based on the route transfer protocol create the array of transfer packets
      GPS_SWay **ppway;
      int elements = 0;
      if (gps_route_transfer == pA201)
	ppway = Garmin_GPS_Create_A201_Route(pr, route_number, &elements);
      else
	ppway = Garmin_GPS_Create_A200_Route(pr, route_number, &elements);


      //    Transmit the Route to the GPS receiver
      int xfer_result = GPS_Command_Send_Route(port_name.mb_str(), ppway, elements);
      ret_val = xfer_result;

      //  Free all the memory
      for(int i=0 ; i < elements ; i++)
            GPS_Way_Del(&ppway[i]);

      free(ppway);


      if ( pProgress )
      {
            pProgress->SetValue ( 80 );
            pProgress->Refresh();
            pProgress->Update();
      }

      VerifyPortClosed();
      return ret_val;
}

/*
int Garmin_USB_On(void)
{
      int ret_val = GPS_Device_On("usb:", &my_gps_devh);

      return ret_val;
}

int Garmin_USB_Off(void)
{
      int ret_val = GPS_Device_Off(my_gps_devh);

      return ret_val;
}
*/
