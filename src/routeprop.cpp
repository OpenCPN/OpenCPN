/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  RouteProerties Support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/listimpl.cpp>    // toh, 2009.02.22

#include "wx/datetime.h"

#include "routeprop.h"
#include "navutil.h"                // for Route
#include "georef.h"
#include "chart1.h"
#include "routeman.h"
#include "routemanagerdialog.h"
#include "chcanv.h"
#include "tcmgr.h"		// pjotrc 2011.03.02



extern double           gLat, gLon, gSog, gCog;
extern double           g_PlanSpeed;
extern wxDateTime		g_StartTime;
extern int				g_StartTimeTZ;
extern IDX_entry		*gpIDX;
extern TCMgr			*ptcmgr;
extern long				gStart_LMT_Offset;
extern MyConfig         *pConfig;
extern WayPointman      *pWayPointMan;
extern ChartCanvas      *cc1;
extern Select           *pSelect;
extern Routeman         *g_pRouteMan;
extern RouteProp        *pRoutePropDialog;
extern RouteManagerDialog *pRouteManagerDialog;
extern Track     *g_pActiveTrack;
extern RouteList        *pRouteList;
extern MyFrame          *gFrame;

WX_DEFINE_LIST(HyperlinkCtrlList);        // toh, 2009.02.22

#define MAX_NUM_HYPERLINKS    6           // toh, 2009.02.23

/*!
 * Helper stuff for calculating Route Plans
 */

#define	pi	(4.*atan(1.0))
#define	tpi	(2.*pi)
#define	twopi	(2.*pi)
#define	degs	(180./pi)
#define	rads	(pi/180.)

#define	MOTWILIGHT	1	// in some languages there may be a distinction between morning/evening
#define	SUNRISE	2
#define	DAY		3
#define	SUNSET	4
#define	EVTWILIGHT	5
#define	NIGHT		6

char daylight_status[][20] = {
      "   ( - )",
      "   (Twilight)",
      "   (Sunrise)",
      "   (Daytime)",
      "   (Sunset)",
      "   (Twilight)",
	  "   (Nighttime)"
	};

/* Next high tide, low tide, transition of the mark level, or some
   combination.
       Bit      Meaning
        0       low tide
        1       high tide
        2       falling transition
        3       rising transition
*/

#define	LW	1
#define	HW	2
#define	FALLING	4
#define	RISING	8

char tide_status[][8] = {
	" LW ",
	" HW ",
	" ~~v ",
	" ~~^ "
	};

// Sunrise/twilight calculation for route properties.
// limitations: latitude below 60, year between 2000 and 2100
// riset is +1 for rise -1 for set
// adapted by author's permission from QBASIC source as published at
//     http://www.stargazing.net/kepler

static double sign(double x)
{
if (x < 0.) return -1.;
else return 1.;
}

static double FNipart (double x)
	{
		 return(sign(x)*(int)(fabs(x)));
	 }

static double FNday (int y, int m, int d, int h)
	{
		long fd = (367 * y - 7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d);
		return ( (double)fd - 730531.5 + h / 24.);
	}

static double FNrange (double x)
	{
		double b = x / tpi;
		double a = tpi * (b - FNipart(b));
		if (a < 0.) a = tpi + a;
		return(a);
	}

double getDaylightEvent (double glat, double glong, int riset, double altitude, int y, int  m, int d)
	{
		double day = FNday(y, m, d, 0);
		double days, correction;
		double utold = pi;
		double utnew = 0.;
		double sinalt = sin(altitude * rads); // go for the sunrise/sunset altitude first
		double sinphi = sin(glat * rads);
		double cosphi = cos(glat * rads);
		double g = glong * rads;
		double t, L, G, ec, lambda, E, obl, delta, GHA, cosc;
		int limit = 12;
		while ((fabs(utold - utnew) > .001)) {
			if (limit-- <= 0) return(-1.);
  			days = day + utnew / tpi;
  			t = days / 36525.;
  //     get arguments of Sun's orbit
  			L = FNrange(4.8949504201433 + 628.331969753199 * t);
  			G = FNrange(6.2400408 + 628.3019501 * t);
  			ec = .033423 * sin(G) + .00034907 * sin(2 * G);
  			lambda = L + ec;
  			E = -1. * ec + .0430398 * sin(2 * lambda) - .00092502 * sin(4. * lambda);
  			obl = .409093 - .0002269 * t;
  			delta = asin(sin(obl) * sin(lambda));
  			GHA = utold - pi + E;
  			cosc = (sinalt - sinphi * sin(delta)) / (cosphi * cos(delta));
  			if (cosc > 1.)
			  	correction = 0.;
			else if (cosc < -1.)
				correction = pi;
			else
			  	correction = acos(cosc);
			double tmp = utnew;
  			utnew = FNrange(utold - (GHA + g + riset * correction));
			utold =tmp;
		}
		return(utnew*degs/15.);	// returns decimal hours UTC
}

static double getLMT(double ut, double lon) {
	double t = ut + lon/15.;
	if (t >= 0.)
		if (t <= 24.) return (t); else return (t - 24.);
	else
		return (t + 24.);
	}

int getDaylightStatus(double lat, double lon, wxDateTime utcDateTime)
	{
		if (fabs(lat)>60.) return(0);
		int y = utcDateTime.GetYear();
		int m = utcDateTime.GetMonth()+1;  // wxBug? months seem to run 0..11 ?
		int d = utcDateTime.GetDay();
		int h = utcDateTime.GetHour();
		int n = utcDateTime.GetMinute();
		int s = utcDateTime.GetSecond();
		if ( y<2000 || y>2100) return (0);

		double ut = (double)h + (double)n/60. + (double)s/3600.;
		double lt = getLMT( ut, lon);
		double rsalt = -0.833;
		double twalt = -12.;

		//wxString msg;

		if (lt<=12.) {
			double sunrise = getDaylightEvent(lat, lon, +1, rsalt, y, m, d);
			if (sunrise < 0.) return(0); else sunrise = getLMT(sunrise, lon);

      //            msg.Printf(_T("getDaylightEvent lat=%f lon=%f\n riset=%d rsalt=%f\n y=%d m=%d d=%d\n sun=%f lt=%f\n ut=%f\n"),
					 // lat, lon, +1, rsalt, y, m, d, sunrise, lt, ut);
				  //msg.Append(utcDateTime.Format());
      //            OCPNMessageDialog md1(gFrame, msg, _("Sunrise Message"), wxICON_ERROR );
      //            md1.ShowModal();

			if (fabs(lt-sunrise)<0.15) return(SUNRISE);
			if (lt > sunrise) return(DAY);
			double twilight = getDaylightEvent(lat, lon, +1, twalt, y, m, d);
			if (twilight < 0.) return(0); else twilight = getLMT(twilight, lon);
			if (lt > twilight) return(MOTWILIGHT);
			else return(NIGHT);
		}
		else {
			double sunset = getDaylightEvent(lat, lon, -1, rsalt, y, m, d);
			if (sunset < 0.) return(0); else sunset = getLMT(sunset, lon);
			if (fabs(lt-sunset)<0.15) return(SUNSET);
			if (lt < sunset) return(DAY);
			double twilight = getDaylightEvent(lat, lon, -1, twalt, y, m, d);
			if (twilight < 0.) return(0); else twilight = getLMT(twilight, lon);
			if (lt < twilight) return(EVTWILIGHT);
			else return(NIGHT);
		}
}

#define     UTCINPUT    0
#define	LTINPUT     1	// i.e. this PC local time
#define	LMTINPUT    2	// i.e. the remote location LMT time
#define     INPUT_FORMAT      1
#define     DISPLAY_FORMAT    2
#define     TIMESTAMP_FORMAT  3

wxString ts2s(wxDateTime ts, int tz_selection, long LMT_offset, int format)
{
	wxString s = _T("");
	wxString f;
		if (format == INPUT_FORMAT) f = _T("%m/%d/%Y %H:%M");
            else if (format == TIMESTAMP_FORMAT) f = _T("%m/%d/%Y %H:%M:%S");
	      else f = _T(" %m/%d %H:%M");
		switch (tz_selection) {
			 case 0: s.Append(ts.Format(f));
                         if (format != INPUT_FORMAT) s.Append(_T(" UT"));
                         break;
			 case 1: s.Append(ts.FromUTC().Format(f)); break;
			 case 2:
				 wxTimeSpan lmt(0,0,(int)LMT_offset,0);
				 s.Append(ts.Add(lmt).Format(f));
                         if (format != INPUT_FORMAT) s.Append(_T(" LMT"));
		 }
	return(s);
}



/*!
 * RouteProp type definition
 */

IMPLEMENT_DYNAMIC_CLASS( RouteProp, wxDialog )

/*!
 * RouteProp event table definition
 */

BEGIN_EVENT_TABLE( RouteProp, wxDialog )


    EVT_TEXT( ID_PLANSPEEDCTL, RouteProp::OnPlanSpeedCtlUpdated )
    EVT_TEXT_ENTER( ID_STARTTIMECTL, RouteProp::OnStartTimeCtlUpdated )
	//EVT_RADIOBOX ( ID_TIMEZONESEL, RouteProp::OnStartTimeCtlUpdated )
	EVT_RADIOBOX ( ID_TIMEZONESEL, RouteProp::OnTimeZoneSelected )
    EVT_BUTTON( ID_ROUTEPROP_CANCEL, RouteProp::OnRoutepropCancelClick )
    EVT_BUTTON( ID_ROUTEPROP_OK, RouteProp::OnRoutepropOkClick )
    EVT_LIST_ITEM_SELECTED( ID_LISTCTRL, RouteProp::OnRoutepropListClick )
    EVT_BUTTON( ID_ROUTEPROP_SPLIT, RouteProp::OnRoutepropSplitClick )
    EVT_BUTTON( ID_ROUTEPROP_EXTEND, RouteProp::OnRoutepropExtendClick )

END_EVENT_TABLE()

/*!
 * RouteProp constructors
 */

RouteProp::RouteProp( )
{
}

RouteProp::RouteProp( wxWindow* parent, wxWindowID id,
                     const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
      m_TotalDistCtl = NULL;
      m_wpList = NULL;
      m_nSelected = 0;
      m_pHead = NULL;
      m_pTail = NULL;
      m_pEnroutePoint = NULL;
      m_bStartNow = false;

      m_pEnroutePoint = NULL;
      m_bStartNow = false;

/*
      wxScrollingDialog::Init();

      SetLayoutAdaptation(true);
      SetLayoutAdaptationLevel(2);

      long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxVSCROLL;
      wxScrollingDialog::Create( parent, id, caption, pos, size,wstyle );
*/
      long wstyle = style;
#ifdef __WXOSX__
      wstyle |= wxSTAY_ON_TOP;
#endif

      Create(parent, id, caption, pos, size, wstyle);
      GetSizer()->SetSizeHints(this);
      Centre();
}

void RouteProp::OnRoutepropSplitClick( wxCommandEvent& event )
{
      m_SplitButton->Enable(false);

      if (m_pRoute->m_bIsInLayer) return;

      if ((m_nSelected > 1) && (m_nSelected < m_pRoute->GetnPoints())) {
            if (m_pRoute->m_bIsTrack) {
                  m_pHead = new Track();
                  m_pTail = new Track();
                  m_pHead->CloneTrack(m_pRoute, 1, m_nSelected, _("_A"));
                  m_pTail->CloneTrack(m_pRoute, m_nSelected, m_pRoute->GetnPoints(), _("_B"));
            }
            else {
                  m_pHead = new Route();
                  m_pTail = new Route();
                  m_pHead->CloneRoute(m_pRoute, 1, m_nSelected, _("_A"));
                  m_pTail->CloneRoute(m_pRoute, m_nSelected, m_pRoute->GetnPoints(), _("_B"));
            }
            pRouteList->Append ( m_pHead );
            pConfig->AddNewRoute ( m_pHead, -1 );
            m_pHead->RebuildGUIDList();

            pRouteList->Append ( m_pTail );
            pConfig->AddNewRoute ( m_pTail, -1 );
            m_pTail->RebuildGUIDList();

            pConfig->DeleteConfigRoute ( m_pRoute );

            if (!m_pTail->m_bIsTrack) {
                  g_pRouteMan->DeleteRoute(m_pRoute);
                  pSelect->AddAllSelectableRouteSegments ( m_pTail );
                  pSelect->AddAllSelectableRoutePoints ( m_pTail );
                  pSelect->AddAllSelectableRouteSegments ( m_pHead );
                  pSelect->AddAllSelectableRoutePoints ( m_pHead );
            }
            else {
                  pSelect->DeleteAllSelectableTrackSegments(m_pRoute);
                  m_pRoute->ClearHighlights();
                  g_pRouteMan->DeleteTrack(m_pRoute);
                  pSelect->AddAllSelectableTrackSegments ( m_pTail );
                  pSelect->AddAllSelectableTrackSegments ( m_pHead );
            }

            SetRouteAndUpdate(m_pTail);
            UpdateProperties();

            if ( pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
                  if (!m_pTail->m_bIsTrack)
                        pRouteManagerDialog->UpdateRouteListCtrl();
                  else
                        pRouteManagerDialog->UpdateTrkListCtrl();
            }
      }
}

void RouteProp::OnRoutepropExtendClick( wxCommandEvent& event )
{
      m_ExtendButton->Enable(false);

      if (!m_pRoute->m_bIsTrack) {
            if ( IsThisRouteExtendable() ) {
                  int fm = m_pExtendRoute->GetIndexOf(m_pExtendPoint)+1;
                  int to = m_pExtendRoute->GetnPoints();
                  if (fm <= to) {
                        pSelect->DeleteAllSelectableRouteSegments ( m_pRoute );
                        m_pRoute->CloneRoute(m_pExtendRoute, fm, to, _("_plus"));
                        pSelect->AddAllSelectableRouteSegments ( m_pRoute );
                        SetRouteAndUpdate(m_pRoute);
                        UpdateProperties();
                  }
            }
      } // end route extend
      else {  // start track extend
            RoutePoint *pLastPoint = m_pRoute->GetPoint(1);

            if (IsThisTrackExtendable()) {
                        int begin = 1;
                        if (pLastPoint->m_CreateTime == m_pExtendPoint->m_CreateTime) begin = 2;
                        pSelect->DeleteAllSelectableTrackSegments(m_pExtendRoute);
                        m_pExtendRoute->CloneTrack(m_pRoute, begin, m_pRoute->GetnPoints(), _("_plus"));
                        pSelect->AddAllSelectableTrackSegments ( m_pExtendRoute );
                        pSelect->DeleteAllSelectableTrackSegments(m_pRoute);
                        m_pRoute->ClearHighlights();
                        g_pRouteMan->DeleteTrack(m_pRoute);

                        SetRouteAndUpdate(m_pExtendRoute);
                        UpdateProperties();

                        if ( pRouteManagerDialog && pRouteManagerDialog->IsShown())
                              pRouteManagerDialog->UpdateTrkListCtrl();
            }
      }
}

bool RouteProp::IsThisRouteExtendable()
{
      m_pExtendRoute = NULL;
      m_pExtendPoint = NULL;
      if (m_pRoute->m_bRtIsActive || m_pRoute->m_bIsInLayer) return false;

      if (!m_pRoute->m_bIsTrack) {
            RoutePoint *pLastPoint = m_pRoute->GetLastPoint();
            wxArrayPtrVoid *pEditRouteArray;

            pEditRouteArray = g_pRouteMan->GetRouteArrayContaining(pLastPoint);
            // remove invisible & own routes from choices
            int i;
            for ( i = pEditRouteArray->GetCount(); i > 0; i-- ) {
                  Route *p = (Route *)pEditRouteArray->Item(i-1);
                  if (!p->IsVisible() || (p->m_GUID == m_pRoute->m_GUID)) pEditRouteArray->RemoveAt(i-1);
            }
            if (pEditRouteArray->GetCount() == 1 )
                  m_pExtendPoint = pLastPoint;
            else
            if (pEditRouteArray->GetCount() == 0 ) {

                  //int nearby_radius_meters = 8./cc1->GetCanvasScaleFactor(); // "nearby" means 8 pixels away
                  int nearby_radius_meters = (int)(8./cc1->GetCanvasTrueScale());
                  double rlat = pLastPoint->m_lat;
                  double rlon = pLastPoint->m_lon;

                  m_pExtendPoint = pWayPointMan->GetOtherNearbyWaypoint(rlat, rlon, nearby_radius_meters, pLastPoint->m_GUID);
                  if ( m_pExtendPoint && !m_pExtendPoint->m_bIsInTrack )
                  {
                        wxArrayPtrVoid *pCloseWPRouteArray = g_pRouteMan->GetRouteArrayContaining(m_pExtendPoint);
                        if(pCloseWPRouteArray)
                        {
                              pEditRouteArray = pCloseWPRouteArray;

                        // remove invisible & own routes from choices
                              for ( i = pEditRouteArray->GetCount(); i > 0; i-- ) {
                                    Route *p = (Route *)pEditRouteArray->Item(i-1);
                                    if (!p->IsVisible() || (p->m_GUID == m_pRoute->m_GUID)) pEditRouteArray->RemoveAt(i-1);
                              }
                        }
                  }
            }

            if ( pEditRouteArray->GetCount() == 1 ) {
                  Route *p = (Route *)pEditRouteArray->Item(0);
                  int fm = p->GetIndexOf(m_pExtendPoint)+1;
                  int to = p->GetnPoints();
                  if (fm <= to) {
                        m_pExtendRoute = p;
                        delete pEditRouteArray;
                        return true;
                  }
            }
            delete pEditRouteArray;

      } // end route extend
      return false;
}

bool RouteProp::IsThisTrackExtendable()
{
            m_pExtendRoute = NULL;
            m_pExtendPoint = NULL;
            if(m_pRoute == g_pActiveTrack || m_pRoute->m_bIsInLayer) return false;

            RoutePoint *pLastPoint = m_pRoute->GetPoint(1);

            wxRouteListNode *route_node = pRouteList->GetFirst();
            while(route_node) {
                  Route *proute = route_node->GetData();
                  if (proute->m_bIsTrack && proute->IsVisible() && (proute->m_GUID != m_pRoute->m_GUID)) {
                        RoutePoint *track_node = proute->GetLastPoint();
                              if(track_node->m_CreateTime <= pLastPoint->m_CreateTime)
                                    if (!m_pExtendPoint || track_node->m_CreateTime > m_pExtendPoint->m_CreateTime) {
                                          m_pExtendPoint = track_node;
                                          m_pExtendRoute = proute;
                                    }
                  }
                  route_node = route_node->GetNext();                         // next route
            }
            if (m_pExtendRoute)
                  return (!m_pExtendRoute->m_bIsInLayer);
            else
                  return false;
}

void RouteProp::OnRoutepropListClick( wxListEvent& event )
{
    long itemno = 0;
      m_nSelected = 0;
    const wxListItem &i = event.GetItem();
    i.GetText().ToLong(&itemno);

    int selected_no = itemno;
    m_pRoute->ClearHighlights();

    wxRoutePointListNode *node = m_pRoute->pRoutePointList->GetFirst();
    while(node && itemno--)
    {
          node = node->GetNext();
    }
    if ( node )
    {
      RoutePoint *prp = node->GetData();
      if ( prp )
      {
          prp->m_bPtIsSelected = true;                // highlight the routepoint

          if (!(m_pRoute->m_bIsInLayer) && !(m_pRoute == g_pActiveTrack) && !(m_pRoute->m_bRtIsActive)) {
                m_nSelected = selected_no + 1;
                m_SplitButton->Enable(true);
          }

          gFrame->JumpToPosition(prp->m_lat, prp->m_lon, cc1->GetVPScale());

      }
    }
}

RouteProp::~RouteProp( )
{
    delete m_TotalDistCtl;
    delete m_PlanSpeedCtl;
    delete m_TimeEnrouteCtl;

    delete m_RouteNameCtl;
    delete m_RouteStartCtl;
    delete m_RouteDestCtl;

	delete m_StartTimeCtl;
//	delete pDispTz;

    delete m_wpList;
}

/*!
 * RouteProp creator
 */

bool RouteProp::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();

    return TRUE;
}

/*!
 * Control creation for RouteProp
 */

void RouteProp::CreateControls()
{
////@begin RouteProp content construction

    RouteProp* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Properties"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxEXPAND|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_RouteNameCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(710, -1), 0 );
    itemStaticBoxSizer3->Add(m_RouteNameCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 5);


    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(2, 2, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer6, 1, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Depart From"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Destination"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_RouteStartCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, _T(""), wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer6->Add(m_RouteStartCtl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_RouteDestCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxSize(300, -1), 0 );
    itemFlexGridSizer6->Add(m_RouteDestCtl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);


    wxFlexGridSizer* itemFlexGridSizer6a = new wxFlexGridSizer(2, 4, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer6a, 1, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Total Distance"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_PlanSpeedLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Plan Speed"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add(m_PlanSpeedLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText12a = new wxStaticText( itemDialog1, wxID_STATIC, _("Time Enroute"),wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add(itemStaticText12a, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_StartTimeLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Departure Time (m/d/y h:m)"),wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add(m_StartTimeLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_TotalDistCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemFlexGridSizer6a->Add(m_TotalDistCtl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_PlanSpeedCtl = new wxTextCtrl( itemDialog1, ID_PLANSPEEDCTL, _T(""), wxDefaultPosition, wxSize(100, -1), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add(m_PlanSpeedCtl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_TimeEnrouteCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition, wxSize(200, -1), wxTE_READONLY );
    itemFlexGridSizer6a->Add(m_TimeEnrouteCtl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_StartTimeCtl = new wxTextCtrl( itemDialog1, ID_STARTTIMECTL, _T(""), wxDefaultPosition, wxSize(150, -1), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add(m_StartTimeCtl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxString pDispTimeZone[] = {
        _("UTC"),
        _("Local @ PC"),
        _("LMT @ Location")
    };
    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer( wxHORIZONTAL );
    pDispTz = new wxRadioBox( itemDialog1, ID_TIMEZONESEL, _("Times shown as"), wxDefaultPosition, wxDefaultSize,
                               3, pDispTimeZone, 3, wxRA_SPECIFY_COLS );
    bSizer2->Add(pDispTz, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_staticText1 = new wxStaticText( this, wxID_ANY, _("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText1->Wrap( -1 );
    bSizer2->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxString m_chColorChoices[] = { _("Default color"), _("Black"), _("Dark Red"), _("Dark Green"), _("Dark Yellow"), _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"), _("Light Gray"), _("Dark Gray"), _("Red"), _("Green"), _("Yellow"), _("Blue"), _("Magenta"), _("Cyan"), _("White") };
    int m_chColorNChoices = sizeof( m_chColorChoices ) / sizeof( wxString );
    m_chColor = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chColorNChoices, m_chColorChoices, 0 );
    m_chColor->SetSelection( 0 );
    bSizer2->Add( m_chColor, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    itemStaticBoxSizer3->Add( bSizer2, 1, wxEXPAND, 0 );

    wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Waypoints"));
    wxStaticBoxSizer* itemStaticBoxSizer14 = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer14, 1, wxEXPAND|wxALL, 5);

    m_wpList = new wxListCtrl( itemDialog1, ID_LISTCTRL, wxDefaultPosition, wxSize(800, 200),
                               wxLC_REPORT|wxLC_HRULES|wxLC_VRULES|wxLC_EDIT_LABELS );
    itemStaticBoxSizer14->Add(m_wpList, 2, wxEXPAND|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT|wxALL, 5);

    m_ExtendButton = new wxButton( itemDialog1, ID_ROUTEPROP_EXTEND, _("Extend Route"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_ExtendButton, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_ExtendButton->Enable(false);

    m_SplitButton = new wxButton( itemDialog1, ID_ROUTEPROP_SPLIT, _("Split Route"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_SplitButton, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_SplitButton->Enable(false);

    m_CancelButton = new wxButton( itemDialog1, ID_ROUTEPROP_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OKButton = new wxButton( itemDialog1, ID_ROUTEPROP_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_OKButton->SetDefault();

    //      To correct a bug in MSW commctl32, we need to catch column width drag events, and do a Refresh()
    //      Otherwise, the column heading disappear.....
    //      Does no harm for GTK builds, so no need for conditional
    Connect(wxEVT_COMMAND_LIST_COL_END_DRAG, (wxObjectEventFunction)(wxEventFunction)&RouteProp::OnEvtColDragEnd);

    // note that under MSW for SetColumnWidth() to work we need to create the
    // items with images initially even if we specify dummy image id
//    wxListItem itemCol;
//    itemCol.SetImage(-1);

    m_wpList->InsertColumn( 0, _("Leg"), wxLIST_FORMAT_LEFT, 45 );

    m_wpList->InsertColumn( 1, _("To Waypoint"), wxLIST_FORMAT_LEFT, 120 );

    m_wpList->InsertColumn( 2, _("Distance"), wxLIST_FORMAT_RIGHT, 70 );

    m_wpList->InsertColumn( 3, _("Bearing"), wxLIST_FORMAT_LEFT, 70 );

    m_wpList->InsertColumn( 4, _("Latitude"), wxLIST_FORMAT_LEFT, 85 );

    m_wpList->InsertColumn( 5, _("Longitude"), wxLIST_FORMAT_LEFT, 90 );

    m_wpList->InsertColumn( 6, _("ETE/Timestamp"), wxLIST_FORMAT_LEFT, 135 );

    m_wpList->InsertColumn( 7, _("Speed (Kts)"), wxLIST_FORMAT_CENTER, 72 );

    m_wpList->InsertColumn( 8, _("Next tide event"), wxLIST_FORMAT_LEFT, 90 );

//  Fetch any config file values
    m_planspeed = g_PlanSpeed;

    pDispTz->SetSelection(g_StartTimeTZ);



    SetColorScheme((ColorScheme)0);

}


void RouteProp::SetColorScheme(ColorScheme cs)
{
      SetBackgroundColour(GetGlobalColor(_T("DILG1")));

      wxColour back_color =GetGlobalColor(_T("DILG2"));
      wxColour text_color = GetGlobalColor(_T("DILG3"));

      m_RouteNameCtl->SetBackgroundColour(back_color);
      m_RouteNameCtl->SetForegroundColour(text_color);

      m_RouteStartCtl->SetBackgroundColour(back_color);
      m_RouteStartCtl->SetForegroundColour(text_color);

      m_RouteDestCtl->SetBackgroundColour(back_color);
      m_RouteDestCtl->SetForegroundColour(text_color);

      m_TotalDistCtl->SetBackgroundColour(back_color);
      m_TotalDistCtl->SetForegroundColour(text_color);

      m_PlanSpeedCtl->SetBackgroundColour(back_color);
      m_PlanSpeedCtl->SetForegroundColour(text_color);

      m_TimeEnrouteCtl->SetBackgroundColour(back_color);
      m_TimeEnrouteCtl->SetForegroundColour(text_color);

      m_StartTimeCtl->SetBackgroundColour(back_color);
      m_StartTimeCtl->SetForegroundColour(text_color);

      m_wpList->SetBackgroundColour(back_color);
      m_wpList->SetForegroundColour(text_color);

      m_ExtendButton->SetBackgroundColour(back_color);
      m_ExtendButton->SetForegroundColour(text_color);

      m_SplitButton->SetBackgroundColour(back_color);
      m_SplitButton->SetForegroundColour(text_color);

      m_CancelButton->SetBackgroundColour(back_color);
      m_CancelButton->SetForegroundColour(text_color);

      m_OKButton->SetBackgroundColour(back_color);
      m_OKButton->SetForegroundColour(text_color);
}
/*!
 * Should we show tooltips?
 */

bool RouteProp::ShowToolTips()
{
    return TRUE;
}


void RouteProp::SetDialogTitle(wxString title)
{
      SetTitle(title);
}



void RouteProp::SetRouteAndUpdate(Route *pR)
{

      //  Fetch any config file values

//      long LMT_Offset = 0;                    // offset in seconds from UTC for given location (-1 hr / 15 deg W)
      int tz_selection = 1;

      if (pR == m_pRoute) {
	  //m_starttime = g_StartTime;
	  //tz_selection = g_StartTimeTZ;
	  gStart_LMT_Offset = 0;
      }
      else {
        g_StartTime = wxInvalidDateTime;
        g_StartTimeTZ = 1;
	  m_starttime = g_StartTime;
	  tz_selection = g_StartTimeTZ;
	  gStart_LMT_Offset = 0;
        m_pEnroutePoint = NULL;
        m_bStartNow = false;
      }

	pDispTz->SetSelection(tz_selection);

// Reorganize dialog for route or track display
      if(pR)
      {
            if(pR->m_bIsTrack)
            {
                  m_PlanSpeedLabel->SetLabel(_("Avg. speed (Kts)"));
                        m_PlanSpeedCtl->SetEditable(false);
            }
            else
            {
                  m_PlanSpeedLabel->SetLabel(_("Plan speed (Kts)"));
                  m_PlanSpeedCtl->SetEditable(true);
            }
      }
      m_pRoute = pR;
      m_wpList->DeleteAllItems();

//  Iterate on Route Points, inserting blank fields starting with index 0
      if(pR)
      {
            wxRoutePointListNode *pnode = m_pRoute->pRoutePointList->GetFirst();
            if (m_pEnroutePoint && m_bStartNow)
                  gStart_LMT_Offset = long ((m_pEnroutePoint->m_lon)*3600./15.);
            else
                  gStart_LMT_Offset = long ((m_pRoute->pRoutePointList->GetFirst()->GetData()->m_lon)*3600./15.);

            m_pRoute->UpdateSegmentDistances(m_planspeed);           // to interpret ETD properties


            int in=0;
            while(pnode)
            {
                  m_wpList->InsertItem(in, _T(""), 0);
                  in++;
                  if (pnode->GetData()->m_seg_etd.IsValid()) {
                        m_wpList->InsertItem(in, _T(""), 0);
                        in++;
                        }
                  pnode = pnode->GetNext();
            }

            //  Set User input Text Fields
            m_RouteNameCtl->SetValue(m_pRoute->m_RouteNameString);
            m_RouteStartCtl->SetValue(m_pRoute->m_RouteStartString);
            m_RouteDestCtl->SetValue(m_pRoute->m_RouteEndString);

            m_RouteNameCtl->SetFocus();

            //      Update the plan speed and route start time controls
            if(!m_pRoute->m_bIsTrack)
            {
                  wxString s;
                  s.Printf(_T("%5.2f"), m_planspeed);
                  m_PlanSpeedCtl->SetValue(s);

                  if (m_starttime.IsValid()) {
	                    wxString s = ts2s(m_starttime, tz_selection, (int)gStart_LMT_Offset, INPUT_FORMAT);
	                    m_StartTimeCtl->SetValue(s);
                    }
                  else
                  m_StartTimeCtl->Clear();
		}
      }
      else
      {
            m_RouteNameCtl->Clear();
            m_RouteStartCtl->Clear();
            m_RouteDestCtl->Clear();
            m_PlanSpeedCtl->Clear();
		m_StartTimeCtl->Clear();
      }
	  UpdateProperties();
}



bool RouteProp::UpdateProperties()
{
      ::wxBeginBusyCursor();

      m_TotalDistCtl->SetValue(_T(""));
      m_TimeEnrouteCtl->SetValue(_T(""));
      int tz_selection = pDispTz->GetSelection();
      long LMT_Offset = 0;              // offset in seconds from UTC for given location (-1 hr / 15 deg W)

      m_SplitButton->Enable(false);
      m_ExtendButton->Enable(false);

      if(m_pRoute)
      {
            if (m_pRoute->m_bIsTrack) {
                  m_ExtendButton->SetLabel(_("Extend Track"));
                  m_SplitButton->SetLabel(_("Split Track"));
            }
            else {
                  m_ExtendButton->SetLabel(_("Extend Route"));
                  m_SplitButton->SetLabel(_("Split Route"));
            }

            m_pRoute->UpdateSegmentDistances();           // get segment and total distance

            //    Update the "tides event" column header

            wxListItem column_info;
		if(m_wpList->GetColumn(8, column_info)) {
			wxString c = _("Next tide event");
			if (gpIDX && m_starttime.IsValid()) {
				  if(!m_pRoute->m_bIsTrack) {
						c = _T("@~~");
						c.Append(wxString::From8BitData(gpIDX->IDX_station_name));
						int i = c.Find(',');
						if (i != wxNOT_FOUND) c.Remove(i);
				 }
			}
			column_info.SetText(c);
			m_wpList->SetColumn(8, column_info);
		}

		//			Update the "ETE/Timestamp" column header

		if(m_wpList->GetColumn(6, column_info))
		{
			  if(m_pRoute->m_bIsTrack)
			  {
					column_info.SetText(_("Timestamp"));
					m_wpList->SetColumn(6, column_info);
			  }
			  else
			  {
					if (m_starttime.IsValid())
						column_info.SetText(_("ETA"));
					else
						column_info.SetText(_("ETE"));
					m_wpList->SetColumn(6, column_info);
			  }
		}

            m_pRoute->UpdateSegmentDistances(m_planspeed);           // get segment and total distance
            double leg_speed = m_planspeed;
            wxTimeSpan stopover_time (0); // time spent waiting for ETD
            wxTimeSpan joining_time (0);   // time spent before reaching first waypoint

            double total_seconds = 0.;

            // Calculate AVG speed if we are showing a track and total time
            if(m_pRoute->m_bIsTrack)
            {
                  total_seconds = m_pRoute->GetLastPoint()->m_CreateTime.Subtract(m_pRoute->GetPoint(1)->m_CreateTime).GetSeconds().ToDouble();
                  if(total_seconds != 0)
                  {
                        m_avgspeed = m_pRoute->m_route_length / total_seconds * 3600;
                  }
                  else
                  {
                        m_avgspeed = 0;
                  }
                  wxString s;
                  s.Printf(_T("%5.2f"), m_avgspeed);
                  m_PlanSpeedCtl->SetValue(s);

                  if (IsThisTrackExtendable()) m_ExtendButton->Enable(true);
            }
            else
            {
                  //if((0.1 < m_planspeed) && (m_planspeed < 1000.0))
                  //      total_seconds= 3600 * m_pRoute->m_route_length / m_planspeed;     // in seconds
                  total_seconds = m_pRoute->m_route_time;

                  if (m_pRoute && m_bStartNow) {
                        if (m_pEnroutePoint)
                              gStart_LMT_Offset = long ((m_pEnroutePoint->m_lon)*3600./15.);
                        else
                              gStart_LMT_Offset = long ((m_pRoute->pRoutePointList->GetFirst()->GetData()->m_lon)*3600./15.);
                  }

      		if (m_starttime.IsValid()) {
                        wxString s;
                        if (m_bStartNow) {
                              wxDateTime d = wxDateTime::Now();
                              if (tz_selection == 1) {
                                    m_starttime = d.ToUTC();
                                    s =_T(">");
                              }
                        }
                       s += ts2s(m_starttime, tz_selection, (int)gStart_LMT_Offset, INPUT_FORMAT);
                       m_StartTimeCtl->SetValue(s);
                  }
                  else
				m_StartTimeCtl->Clear();

                  if (IsThisRouteExtendable()) m_ExtendButton->Enable(true);
            }

      //  Total length
            wxString slen;
            slen.Printf(wxT("%5.2f"), m_pRoute->m_route_length);

            if (!m_pEnroutePoint)
                  m_TotalDistCtl->SetValue(slen);
            else
                  m_TotalDistCtl->Clear();

            wxString time_form;
		wxString tide_form;

      //  Time

            wxTimeSpan time(0,0, (int)total_seconds, 0);
            if(total_seconds > 3600. * 24.)
                  time_form = time.Format(_(" %D Days  %H Hours  %M Minutes"));
            else
                  time_form = time.Format(_(" %H Hours  %M Minutes"));

            if (!m_pEnroutePoint)
                  m_TimeEnrouteCtl->SetValue(time_form);
            else
                  m_TimeEnrouteCtl->Clear();

      //  Iterate on Route Points
            wxRoutePointListNode *node = m_pRoute->pRoutePointList->GetFirst();

            int i=0;
            double slat = gLat;
            double slon = gLon;
            double tdis = 0.;
            double tsec = 0.;    // total time in seconds

            int stopover_count = 0;
            bool arrival = true; // marks which pass over the wpt we do - 1. arrival 2. departure
            bool enroute = true; // for active route, skip all points up to the active point

            if (m_pRoute->m_bRtIsActive) {
                  if (m_pEnroutePoint && m_bStartNow)
                        enroute = (m_pRoute->GetPoint(1)->m_GUID == m_pEnroutePoint->m_GUID);
            }

            wxString nullify = _T("----");

            while(node)
            {
                  RoutePoint *prp = node->GetData();
                  long item_line_index = i + stopover_count;

      //  Leg
                  wxString t;
                  t.Printf(_T("%d"), i);
                  if(i == 0)
                        t = _T("---");
                  if (arrival) m_wpList->SetItem(item_line_index, 0, t);

      //  Mark Name
                  if (arrival) m_wpList->SetItem(item_line_index, 1, prp->GetName());

      //  Distance
      //  Note that Distance/Bearing for Leg 000 is as from current position

                  double brg, leg_dist;
                  bool starting_point = false;

                  starting_point = (i == 0) && enroute;
                  if (m_pEnroutePoint && !starting_point) starting_point = (prp->m_GUID == m_pEnroutePoint->m_GUID);

                  if( starting_point )
                  {
                        slat = gLat;
                        slon = gLon;
                        if (gSog > 0.0) leg_speed = gSog; // should be VMG
                        else leg_speed = m_planspeed;
                        if (m_bStartNow) {
                              DistanceBearingMercator(prp->m_lat, prp->m_lon, slat, slon, &brg, &leg_dist);
                              if (i == 0) joining_time = wxTimeSpan::Seconds((long)wxRound((leg_dist*3600.)/leg_speed));
                        }
                        enroute = true;
                  }
                  else {
                        if (prp->m_seg_vmg > 0.)
                              leg_speed = prp->m_seg_vmg;
                        else
                              leg_speed = m_planspeed;
                  }

                  DistanceBearingMercator(prp->m_lat, prp->m_lon, slat, slon, &brg, &leg_dist);

                  t.Printf(_T("%6.2f nm"),leg_dist);
                  if (arrival) m_wpList->SetItem(item_line_index, 2, t);
                  if (!enroute) m_wpList->SetItem(item_line_index, 2, nullify);

      //  Bearing
                  t.Printf(_T("%03.0f Deg. T"),brg);
                  if (arrival) m_wpList->SetItem(item_line_index, 3, t);
                  if (!enroute) m_wpList->SetItem(item_line_index, 3, nullify);


      //  Lat/Lon
                  wxString tlat = toSDMM(1, prp->m_lat, prp->m_bIsInTrack);  // low precision for routes
                  if (arrival) m_wpList->SetItem(item_line_index, 4, tlat);

                  wxString tlon = toSDMM(2, prp->m_lon, prp->m_bIsInTrack);
                  if (arrival) m_wpList->SetItem(item_line_index, 5, tlon);

                  tide_form = _T("");

                  LMT_Offset = long ((prp->m_lon)*3600./15.);

      // Time to each waypoint or creation date for tracks
                  if (!prp->m_bIsInTrack) {   //if it is a route point...    pjotrc 2010.02.11
                        if ( i == 0 && enroute)
                        {
                          time_form.Printf(_("Start"));
                          if (m_starttime.IsValid()) {
                              wxDateTime act_starttime = m_starttime + joining_time;
                              time_form.Append(_T(": "));

                              if (!arrival ) {
                                    wxDateTime etd = prp->m_seg_etd;
                                    if (etd.IsValid() && etd.IsLaterThan(m_starttime)) {
                                          stopover_time += etd.Subtract(m_starttime);
                                          act_starttime = prp->m_seg_etd;
                                          }
                                    }

				        int ds = getDaylightStatus( prp->m_lat, prp->m_lon, act_starttime);
				        wxString s = ts2s(act_starttime, tz_selection, (int)LMT_Offset, DISPLAY_FORMAT);
                                time_form.Append(s);
                                time_form.Append(wxString::From8BitData(&daylight_status[ds][0]));

                                if (ptcmgr) {
                                    int jx = 0;
                                    if (prp->GetName().Find(_T("@~~")) != wxNOT_FOUND) {
                                          tide_form = prp->GetName().Mid(prp->GetName().Find(_T("@~~"))+3);
	                                    jx = ptcmgr->GetStationIDXbyName(tide_form, prp->m_lat, prp->m_lon, ptcmgr);
	                              }
                                    if (gpIDX || jx) {
						      time_t tm = act_starttime.GetTicks();
                                          tide_form = MakeTideInfo( jx, tm, tz_selection, LMT_Offset);
                                    }
                                }
                          }
                        tdis = 0;
                        tsec = 0.;
                        } // end of route point 0
                        else
                        {
                              if (arrival && enroute) tdis += leg_dist;
                              if (leg_speed)
                              {
                              if (arrival && enroute) tsec += 3600 * leg_dist / leg_speed;         // time in seconds to arrive here
                              wxTimeSpan time(0,0, (int)tsec, 0);

						if (m_starttime.IsValid()) {

							wxDateTime ueta = m_starttime;
							ueta.Add(time + stopover_time + joining_time);

                                          if (!arrival) {
                                                wxDateTime etd = prp->m_seg_etd;
                                                if (etd.IsValid() && etd.IsLaterThan(ueta)) {
                                                      stopover_time += etd.Subtract(ueta);
                                                      ueta = prp->m_seg_etd;
                                                      }
                                                }

							int ds = getDaylightStatus( prp->m_lat, prp->m_lon, ueta);
							time_form = ts2s(ueta, tz_selection, LMT_Offset, DISPLAY_FORMAT);
							time_form.Append(wxString::From8BitData(&daylight_status[ds][0]));

							  if (ptcmgr) {
								  int jx = 0;
                                                  if (prp->GetName().Find(_T("@~~")) != wxNOT_FOUND) {
                                                        tide_form = prp->GetName().Mid(prp->GetName().Find(_T("@~~"))+3);
									jx = ptcmgr->GetStationIDXbyName(tide_form, prp->m_lat, prp->m_lon, ptcmgr);
								  }
								  if (gpIDX || jx) {
									  time_t tm = ueta.GetTicks();
									  tide_form = MakeTideInfo( jx, tm, tz_selection, LMT_Offset);
									  }
							  }
						}
						else {
							if(tsec > 3600. * 24.)
								  time_form = time.Format(_T(" %D D  %H H  %M M"));
							else
								  time_form = time.Format(_T(" %H H  %M M"));
							}
                              } // end if legspeed
                        }  // end of repeatable route point
                  } // end of route point
                  else
                  {          // it is a track point...
		            wxDateTime timestamp = prp->m_CreateTime;
		            time_form = ts2s(timestamp,tz_selection,LMT_Offset,TIMESTAMP_FORMAT);
                  }

                  if (enroute && (arrival || m_starttime.IsValid())) m_wpList->SetItem(item_line_index, 6, time_form);
                  else m_wpList->SetItem(item_line_index, 6, _T("----"));


                  //Leg speed
                  wxString s;
                  if(m_pRoute->m_bIsTrack)
                  {
                        if(i > 0)
                        {
                              double speed;
                              if(!m_pRoute->GetPoint(i+1)->m_CreateTime.IsEqualTo(m_pRoute->GetPoint(i)->m_CreateTime))
                              {
                                    speed = leg_dist / m_pRoute->GetPoint(i+1)->m_CreateTime.Subtract(m_pRoute->GetPoint(i)->m_CreateTime).GetSeconds().ToDouble() * 3600;
                              }
                              else
                              {
                                    speed = 0;
                              }
                              s.Printf(_T("%5.2f"), speed );
                        }
                        else
                        {
                              s = _("--");
                        }
                  }
                  else
                  {
                        s.Printf(_T("%5.2f"), leg_speed);
                  }
                  if (arrival) m_wpList->SetItem(item_line_index, 7, s);

                 if (enroute)
                       m_wpList->SetItem(item_line_index, 8, tide_form);
                 else {
                        m_wpList->SetItem(item_line_index, 7, nullify);
                        m_wpList->SetItem(item_line_index, 8, nullify);
                 }


      //  Save for iterating distance/bearing calculation
                  slat = prp->m_lat;
                  slon = prp->m_lon;

      // if stopover (ETD) found, loop for next output line for the same point
      //   with departure time & tide information

                  if (arrival && (prp->m_seg_etd.IsValid())) {
                              stopover_count++;
                              arrival = false;
                  }
                  else {
                        arrival = true;
                        i++;
                        node = node->GetNext();
                        }

            }

      #if 0
            m_wpList->SetColumnWidth( 0, wxLIST_AUTOSIZE/* 35 */);
            m_wpList->SetColumnWidth( 1,  wxLIST_AUTOSIZE/* 150*/ );
            m_wpList->SetColumnWidth( 2,  wxLIST_AUTOSIZE/* 80*/ );
            m_wpList->SetColumnWidth( 3,  wxLIST_AUTOSIZE/* 80*/ );
            m_wpList->SetColumnWidth( 4,  wxLIST_AUTOSIZE/* 95*/ );
            m_wpList->SetColumnWidth( 5,  wxLIST_AUTOSIZE/* 95*/ );
            m_wpList->SetColumnWidth( 6,  wxLIST_AUTOSIZE/* 120*/ );
      #endif
      }

      if(m_pRoute)
      {
            if ( m_pRoute->m_Colour == wxEmptyString )
                  m_chColor->Select(0);
            else
            {
                  for (unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof( wxString ); i++)
                  {
                        if ( m_pRoute->m_Colour == ::GpxxColorNames[i] )
                        {
                              m_chColor->Select( i + 1 );
                              break;
                        }
                  }
            }
      }
      ::wxEndBusyCursor();

      return true;
}

wxString RouteProp::MakeTideInfo(int jx, time_t tm, int tz_selection, long LMT_Offset)
	{
	  int ev = 0;
	  wxString tide_form;

        if (gpIDX)
        {
              ev = ptcmgr->GetNextBigEvent( &tm,
                        ptcmgr->GetStationIDXbyName(wxString(gpIDX->IDX_station_name, wxConvUTF8),
                        gpIDX->IDX_lat, gpIDX->IDX_lon, ptcmgr));
        }
        else ev = ptcmgr->GetNextBigEvent ( &tm, jx);

	  wxDateTime dtm;
	  dtm.Set(tm).MakeUTC(); // apparently Set works as from LT
	  if (ev==1) tide_form.Printf(_T("LW: "));
	  if (ev==2) tide_form.Printf(_T("HW: "));
	  tide_form.Append(ts2s(dtm, tz_selection, LMT_Offset, DISPLAY_FORMAT));
	  if (!gpIDX) {
		  wxString locn( ptcmgr->GetIDX_entry ( jx )->IDX_station_name, wxConvUTF8 );
		  tide_form.Append(_T(" @~~"));
		  tide_form.Append(locn);
			  }
	  return(tide_form);
	}


bool RouteProp::SaveChanges(void)
{

//  Save the current planning speed
    g_PlanSpeed = m_planspeed;
	g_StartTime = m_starttime;	// both always UTC
	g_StartTimeTZ = pDispTz->GetSelection();
	m_StartTimeCtl->Clear();

    if(m_pRoute && !m_pRoute->m_bIsInLayer)
    {
      //  Get User input Text Fields
      m_pRoute->m_RouteNameString = m_RouteNameCtl->GetValue();
      m_pRoute->m_RouteStartString = m_RouteStartCtl->GetValue();
      m_pRoute->m_RouteEndString = m_RouteDestCtl->GetValue();
      if (m_chColor->GetSelection() == 0)
            m_pRoute->m_Colour = wxEmptyString;
      else
            m_pRoute->m_Colour = ::GpxxColorNames[m_chColor->GetSelection() - 1];

      pConfig->UpdateRoute(m_pRoute);
      pConfig->UpdateSettings();
    }

    return true;
}


void RouteProp::OnPlanSpeedCtlUpdated( wxCommandEvent& event )
{
    //  Fetch the value, and see if it is a "reasonable" speed
    wxString spd = m_PlanSpeedCtl->GetValue();
    double s;
    spd.ToDouble(&s);
	if((0.1 < s) && (s < 1000.0) && !m_pRoute->m_bIsTrack)
    {
        m_planspeed = s;

        UpdateProperties();
    }

    event.Skip();
}

void RouteProp::OnStartTimeCtlUpdated( wxCommandEvent& event )
{
    //  Fetch the value, and see if it is a "reasonable" time
    wxString stime = m_StartTimeCtl->GetValue();
    int tz_selection = pDispTz->GetSelection();

    wxDateTime d;
	if (stime.StartsWith(_T(">"))){
            if (m_pRoute->m_bRtIsActive) {
                  m_pEnroutePoint = g_pRouteMan->GetpActivePoint();
            }
            m_bStartNow = true;
	      d = wxDateTime::Now();
		if (tz_selection == 1) m_starttime = d.ToUTC();
		else m_starttime = wxInvalidDateTime; // can't get it to work otherwise
	}
	else {
            m_pEnroutePoint = NULL;
            m_bStartNow = false;
		if (!d.ParseDateTime(stime))		// only specific times accepted
			d = wxInvalidDateTime;

		m_starttime = d;

		if (m_starttime.IsValid()) {
			if (tz_selection == 1) m_starttime = d.ToUTC();
			if (tz_selection == 2 ) {
				wxTimeSpan glmt(0,0,(int)gStart_LMT_Offset,0);
				m_starttime -= glmt;
				}
		}
	}

	UpdateProperties();

//    event.Skip();
}

void RouteProp::OnTimeZoneSelected( wxCommandEvent& event )
{
	UpdateProperties();

    event.Skip();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void RouteProp::OnRoutepropCancelClick( wxCommandEvent& event )
{
      //    Look in the route list to be sure the raoute is still available
      //    (May have been deleted by RouteMangerDialog...)

      bool  b_found_route = false;
      wxRouteListNode *node = pRouteList->GetFirst();
      while(node)
      {
            Route *proute = node->GetData();

            if(proute == m_pRoute)
            {
                  b_found_route = true;
                  break;
            }
            node = node->GetNext();
      }

      if(b_found_route)
            m_pRoute->ClearHighlights();

      Hide();
      cc1->Refresh(false);

      event.Skip();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void RouteProp::OnRoutepropOkClick( wxCommandEvent& event )
{
      //    Look in the route list to be sure the route is still available
      //    (May have been deleted by RouteManagerDialog...)

      bool  b_found_route = false;
      wxRouteListNode *node = pRouteList->GetFirst();
      while(node)
      {
            Route *proute = node->GetData();

            if(proute == m_pRoute)
            {
                  b_found_route = true;
                  break;
            }
                  node = node->GetNext();
      }

      if(b_found_route)
      {
            SaveChanges();              // write changes to globals and update config
            m_pRoute->ClearHighlights();
      }

      m_pEnroutePoint = NULL;
      m_bStartNow = false;

    Hide();
    cc1->Refresh(false);

    event.Skip();
}

void RouteProp::OnEvtColDragEnd(wxListEvent& event)
{
      m_wpList->Refresh();
}



//-------------------------------------------------------------------------------
//
//    Mark Properties Dialog Implementation
//
//-------------------------------------------------------------------------------
/*!
 * MarkProp type definition
 */

//DEFINE_EVENT_TYPE(EVT_LLCHANGE)           // events from LatLonTextCtrl
const wxEventType EVT_LLCHANGE = wxNewEventType();

IMPLEMENT_DYNAMIC_CLASS( MarkProp, wxDialog )

/*!
 * MarkProp event table definition
 */

BEGIN_EVENT_TABLE( MarkProp, wxDialog )

////@begin MarkProp event table entries

    EVT_BUTTON( ID_MARKPROP_CANCEL, MarkProp::OnMarkpropCancelClick )
    EVT_BUTTON( ID_MARKPROP_OK, MarkProp::OnMarkpropOkClick )
    EVT_LIST_ITEM_SELECTED( ID_ICONCTRL, MarkProp::OnIconListSelected)
    EVT_COMMAND(ID_LATCTRL, EVT_LLCHANGE, MarkProp::OnPositionCtlUpdated)
    EVT_COMMAND(ID_LONCTRL, EVT_LLCHANGE, MarkProp::OnPositionCtlUpdated)
    EVT_CHECKBOX( ID_SHOWNAMECHECKBOX1, MarkProp::OnShowNamecheckboxClick )

////@end RouteProp event table entries

END_EVENT_TABLE()

/*!
 * MarkProp constructors
 */

MarkProp::MarkProp( )
{
}

MarkProp::MarkProp(  wxWindow* parent, wxWindowID id,
                     const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
#if wxCHECK_VERSION(2, 9, 0)
#else
      wxScrollingDialog::Init();

      SetLayoutAdaptation(true);
#endif

      SetLayoutAdaptationLevel(2);

      long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxVSCROLL;
#ifdef __WXOSX__
      wstyle |= wxSTAY_ON_TOP;
#endif

#if wxCHECK_VERSION(2, 9, 0)
      wxDialog::Create( parent, id, caption, pos, size,wstyle );
#else
      wxScrollingDialog::Create( parent, id, caption, pos, size,wstyle );
#endif

      CreateControls();
      GetSizer()->SetSizeHints(this);
      Centre();
}

MarkProp::~MarkProp( )
{
    delete m_MarkNameCtl;
    delete m_MarkLatCtl;
    delete m_MarkLonCtl;
}



/*!
 * MarkProp creator
 */

bool MarkProp::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->SetSizeHints(this);
    Centre();

    return TRUE;
}

/*!
 * Control creation for MarkProp
 */

void MarkProp::CreateControls()
{
    MarkProp* itemDialog1 = this;
    int def_space = 4;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Properties"));

    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxEXPAND|wxALL, def_space);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Mark Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, def_space);

    m_MarkNameCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(-1, -1), 0 );
    itemStaticBoxSizer3->Add(m_MarkNameCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, def_space);

    m_ShowNameCheckbox = new wxCheckBox( itemDialog1, ID_SHOWNAMECHECKBOX1, _("Show Name"), wxDefaultPosition, wxSize(-1, -1), 0 );
    itemStaticBoxSizer3->Add(m_ShowNameCheckbox, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, def_space);

    wxStaticText* itemStaticText4b= new wxStaticText( itemDialog1, wxID_STATIC, _("Description"));
    itemStaticBoxSizer3->Add(itemStaticText4b, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, def_space);

    m_pDescTextCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRLDESC, _T(""),
                               wxDefaultPosition, wxSize(-1, -1),  wxTE_MULTILINE | wxTE_DONTWRAP
                             );
    m_pDescTextCtl->SetMinSize(wxSize(-1, 50));
    itemStaticBoxSizer3->Add(m_pDescTextCtl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, def_space);


    wxStaticText* itemStaticText4a= new wxStaticText( itemDialog1, wxID_STATIC, _("Mark Icon"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(itemStaticText4a, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, def_space);

    m_IconList = new wxListCtrl( itemDialog1, ID_ICONCTRL, wxDefaultPosition, wxSize(300, 150),
        wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES );
    itemStaticBoxSizer3->Add(m_IconList, 2, wxEXPAND|wxALL, def_space);


    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Position"));

    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxEXPAND|wxALL, def_space);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Latitude"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, def_space);

    m_MarkLatCtl = new LatLonTextCtrl( itemDialog1, ID_LATCTRL, _T(""), wxDefaultPosition, wxSize(180, -1), 0 );
    itemStaticBoxSizer4->Add(m_MarkLatCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, def_space);


    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Longitude"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add(itemStaticText6, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, def_space);

    m_MarkLonCtl = new LatLonTextCtrl( itemDialog1, ID_LONCTRL, _T(""), wxDefaultPosition, wxSize(180, -1), 0 );
    itemStaticBoxSizer4->Add(m_MarkLonCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, def_space);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT|wxALL, def_space);

    m_CancelButton = new wxButton( itemDialog1, ID_MARKPROP_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, def_space);

    m_OKButton = new wxButton( itemDialog1, ID_MARKPROP_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, def_space);
    m_OKButton->SetDefault();

    //  Fill in list control

    m_IconList->Hide();

    int client_x, client_y;
    m_IconList->GetClientSize(&client_x, &client_y);

    m_IconList->SetImageList(pWayPointMan->Getpmarkicon_image_list(), wxIMAGE_LIST_SMALL);

    wxListItem itemCol0;
    itemCol0.SetImage(-1);
    itemCol0.SetText(_("Icon"));

    m_IconList->InsertColumn(0, itemCol0);
    m_IconList->SetColumnWidth( 0, 40 );

    wxListItem itemCol;
    itemCol.SetText(_("Description"));
    itemCol.SetImage(-1);
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    m_IconList->InsertColumn(1, itemCol);
    m_IconList->SetColumnWidth( 1, client_x - 56 );


    //      Iterate on the Icon Descriptions, filling in the control

      for(int i = 0 ; i < pWayPointMan->GetNumIcons() ; i++)
      {
            wxString *ps = pWayPointMan->GetIconDescription(i);

            long item_index = m_IconList->InsertItem(i, _T(""), 0);
            m_IconList->SetItem(item_index, 1, *ps);

            m_IconList->SetItemImage(item_index,i);
      }

      m_IconList->Show();

      SetColorScheme((ColorScheme)0);
}


void MarkProp::SetColorScheme(ColorScheme cs)
{
      SetBackgroundColour(GetGlobalColor(_T("DILG1")));

      wxColour back_color =GetGlobalColor(_T("DILG2"));
      wxColour text_color = GetGlobalColor(_T("DILG3"));

      m_MarkNameCtl->SetBackgroundColour(back_color);
      m_MarkNameCtl->SetForegroundColour(text_color);

      m_pDescTextCtl->SetBackgroundColour(back_color);
      m_pDescTextCtl->SetForegroundColour(text_color);

      m_IconList->SetBackgroundColour(back_color);
      m_IconList->SetForegroundColour(text_color);

      m_MarkLatCtl->SetBackgroundColour(back_color);
      m_MarkLatCtl->SetForegroundColour(text_color);

      m_MarkLonCtl->SetBackgroundColour(back_color);
      m_MarkLonCtl->SetForegroundColour(text_color);

      m_CancelButton->SetBackgroundColour(back_color);
      m_CancelButton->SetForegroundColour(text_color);

      m_OKButton->SetBackgroundColour(back_color);
      m_OKButton->SetForegroundColour(text_color);

      m_IconList->SetImageList(pWayPointMan->Getpmarkicon_image_list(), wxIMAGE_LIST_SMALL);

}

bool MarkProp::ShowToolTips()
{
    return TRUE;
}

void MarkProp::SetDialogTitle(wxString title)
{
      SetTitle(title);
}



void MarkProp::SetRoutePoint(RoutePoint *pRP)
{
      m_pRoutePoint = pRP;

      if(m_pRoutePoint)
      {
            m_lat_save = m_pRoutePoint->m_lat;
            m_lon_save = m_pRoutePoint->m_lon;
            m_IconName_save = m_pRoutePoint->m_IconName;
            m_bShowName_save = m_pRoutePoint->m_bShowName;
      }
}


bool MarkProp::UpdateProperties()
{
      m_MarkLatCtl->SetValue(_T(""));
      m_MarkLonCtl->SetValue(_T(""));
      m_MarkNameCtl->SetValue(_T(""));
      m_pDescTextCtl->SetValue(_T(""));

      wxString t;
      if(m_pRoutePoint)
      {
//    Name
            m_MarkNameCtl->SetValue(m_pRoutePoint->GetName());
            m_ShowNameCheckbox->SetValue(m_pRoutePoint->m_bShowName);
            m_MarkNameCtl->SetInsertionPoint(0);
            m_MarkNameCtl->SetSelection(-1, -1);
            m_MarkNameCtl->SetFocus();

//    Description
            m_pDescTextCtl->SetValue(m_pRoutePoint->m_MarkDescription);
            m_pDescTextCtl->SetInsertionPoint(0);
            m_pDescTextCtl->SetSelection(-1, -1);

//  Lat/Lon
            wxString s = toSDMM(1, m_pRoutePoint->m_lat);
            m_MarkLatCtl->SetValue(s);

            s = toSDMM(2, m_pRoutePoint->m_lon);
            m_MarkLonCtl->SetValue(s);

//    Highlite the icon current selection
            m_current_icon_Index = pWayPointMan->GetIconIndex(m_pRoutePoint->m_pbmIcon);

            m_IconList->SetItemState(m_current_icon_Index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            m_IconList->EnsureVisible(m_current_icon_Index);
    }
    return true;
}

bool MarkProp::SaveChanges(void)
{
      if(m_pRoutePoint)
      {
            if (m_pRoutePoint->m_bIsInLayer) return true;

            //  Get User input Text Fields
            m_pRoutePoint->SetName(m_MarkNameCtl->GetValue());
            m_pRoutePoint->m_MarkDescription = m_pDescTextCtl->GetValue();

            //    Here is some logic....
            //    If the Markname is completely numeric, and is part of a route,
            //    Then declare it to be of attribute m_bDynamicName = true
            //    This is later used for re-numbering points on actions like
            //    Insert Point, Delete Point, Append Point, etc

            if(m_pRoutePoint->m_bIsInRoute)
            {
                  bool b_name_is_numeric = true;
                  for(unsigned int i=0 ; i<m_pRoutePoint->GetName().Len() ; i++)
                  {
                        if(wxChar('0') > m_pRoutePoint->GetName()[i])
                              b_name_is_numeric = false;
                        if(wxChar('9') < m_pRoutePoint->GetName()[i])
                              b_name_is_numeric = false;
                  }

                  m_pRoutePoint->m_bDynamicName = b_name_is_numeric;
            }
            else
                  m_pRoutePoint->m_bDynamicName = false;


            if(m_pRoutePoint->m_bIsInRoute)
            {
                  Route *pRoute = g_pRouteMan->FindRouteContainingWaypoint(m_pRoutePoint);
                  pConfig->UpdateRoute(pRoute);
            }
            else

                  pConfig->UpdateWayPoint(m_pRoutePoint);

//  No general settings need be saved            pConfig->UpdateSettings();
      }
    return true;
}


void MarkProp::OnMarkpropCancelClick( wxCommandEvent& event )
{
      if(m_pRoutePoint)
      {
            //    Restore saved values for lat/lon and icon
            m_pRoutePoint->m_lat = m_lat_save;
            m_pRoutePoint->m_lon = m_lon_save;
            m_pRoutePoint->m_IconName = m_IconName_save;
            m_pRoutePoint->m_bShowName = m_bShowName_save;

            m_pRoutePoint->m_pbmIcon = pWayPointMan->GetIconBitmap(m_IconName_save);

            pSelect->ModifySelectablePoint(m_lat_save, m_lon_save, (void *)m_pRoutePoint, SELTYPE_ROUTEPOINT);

            cc1->RefreshRect(m_pRoutePoint->CurrentRect_in_DC.Inflate(1000,100), false);
      }
      Show(false);
      event.Skip();
}


void MarkProp::OnMarkpropOkClick( wxCommandEvent& event )
{
      if(m_pRoutePoint)
      {
            SaveChanges();              // write changes to globals and update config

            cc1->RefreshRect(m_pRoutePoint->CurrentRect_in_DC.Inflate(1000,100), false);
      }

    Show(false);
    event.Skip();
}

void MarkProp::ValidateMark(void)
{
      //    Look in the master list of Waypoints to see if the currently selected waypoint is still valid
      //    It may have been deleted as part of a route
      wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

      bool b_found = false;
      while ( node )
      {
            RoutePoint *rp = node->GetData();
            if(m_pRoutePoint == rp)
            {
                  b_found = true;
                  break;
            }

            node = node->GetNext();
      }

      if(!b_found)
            m_pRoutePoint = NULL;
}


void MarkProp::OnIconListSelected( wxListEvent& event )
{
      int new_index = event.GetIndex();
      if(new_index != m_current_icon_Index)
      {
           m_current_icon_Index = new_index;

           if (m_pRoutePoint->m_bIsInLayer) return;

           m_pRoutePoint->m_IconName = *(pWayPointMan->GetIconKey(m_current_icon_Index));
           m_pRoutePoint->ReLoadIcon();

           // dynamically update the icon on the canvas
           cc1->RefreshRect(m_pRoutePoint->CurrentRect_in_DC, false);
      }
}

void MarkProp::OnShowNamecheckboxClick( wxCommandEvent& event )
{
     if (m_pRoutePoint->m_bIsInLayer) return;

      m_pRoutePoint->m_bShowName = m_ShowNameCheckbox->GetValue();

      // dynamically update the icon on the canvas
      cc1->RefreshRect(m_pRoutePoint->CurrentRect_in_DC, false);
}

void MarkProp::OnPositionCtlUpdated( wxCommandEvent& event )
{
      //    Fetch the control values, convert to degrees
      double lat = fromDMM(m_MarkLatCtl->GetValue());
      double lon = fromDMM(m_MarkLonCtl->GetValue());

      if (!m_pRoutePoint->m_bIsInLayer) {
            m_pRoutePoint->SetPosition(lat, lon);
            pSelect->ModifySelectablePoint(lat, lon, (void *)m_pRoutePoint, SELTYPE_ROUTEPOINT);
      }

      //    Update the mark position dynamically
      cc1->Refresh();
}


//------------------------------------------------------------------------------
//    LatLonTextCtrl Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(LatLonTextCtrl, wxWindow)

      EVT_KILL_FOCUS(LatLonTextCtrl::OnKillFocus)

END_EVENT_TABLE()

// constructor
LatLonTextCtrl::LatLonTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value,
            const wxPoint& pos, const wxSize& size,
            long style, const wxValidator& validator,
            const wxString& name):
wxTextCtrl(parent, id, value, pos, size, style, validator, name)
{
      m_pParentEventHandler = parent->GetEventHandler();
}


void LatLonTextCtrl::OnKillFocus(wxFocusEvent& event)
{
      //    Send an event to the Parent Dialog
      wxCommandEvent up_event( EVT_LLCHANGE,  GetId() );
      up_event.SetEventObject( (wxObject *)this );
      m_pParentEventHandler->AddPendingEvent(up_event);
}


//-------------------------------------------------------------------------------
//
//    Mark Information Dialog Implementation
//
//-------------------------------------------------------------------------------
/*!
 * MarkInfo type definition
 */

IMPLEMENT_DYNAMIC_CLASS( MarkInfo, wxDialog )

/*!
 * MarkInfo event table definition
 */

            BEGIN_EVENT_TABLE( MarkInfo, wxDialog )

////@begin MarkInfo event table entries

            EVT_BUTTON( ID_MARKINFO_CANCEL, MarkInfo::OnMarkinfoCancelClick )
            EVT_BUTTON( ID_MARKINFO_OK, MarkInfo::OnMarkinfoOkClick )

////@end MarkInfo event table entries

            END_EVENT_TABLE()

/*!
 * MarkInfo constructors
 */

            MarkInfo::MarkInfo( )
{
}

MarkInfo::MarkInfo(  wxWindow* parent, wxWindowID id,
                     const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
      Create(parent, id, caption, pos, size, style);
}

MarkInfo::~MarkInfo( )
{
      delete m_MarkNameCtl;
      delete m_MarkLatCtl;
      delete m_MarkLonCtl;

      m_HyperlinkCtrlList->DeleteContents(true);
      delete m_HyperlinkCtrlList;

}



/*!
 * MarkInfo creator
 */

bool MarkInfo::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
      SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
      wxDialog::Create( parent, id, caption, pos, size, style );

      CreateControls();
      GetSizer()->Fit(this);
      GetSizer()->SetSizeHints(this);
      Centre();

      return TRUE;
}

/*!
 * Control creation for MarkInfo
 */

void MarkInfo::CreateControls()
{
      MarkInfo* itemDialog1 = this;

      wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
      itemDialog1->SetSizer(itemBoxSizer2);

      wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Name"),wxDefaultPosition, wxSize(-1, -1));

      wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
      itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxEXPAND|wxALL|wxADJUST_MINSIZE, 5);

      m_MarkNameCtl = new wxStaticText( itemDialog1, wxID_STATIC, _T(""), wxDefaultPosition,wxDefaultSize, 0 );
      itemStaticBoxSizer3->Add(m_MarkNameCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxGROW, 5);

      wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Position"));

      wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
      itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxEXPAND|wxALL|wxADJUST_MINSIZE, 5);

      wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Latitude"), wxDefaultPosition, wxDefaultSize, 0 );
      itemStaticBoxSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

      m_MarkLatCtl = new wxStaticText( itemDialog1, wxID_STATIC, _T(""), wxDefaultPosition, wxSize(180, -1), 0 );
      itemStaticBoxSizer4->Add(m_MarkLatCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxGROW, 5);

      wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Longitude"), wxDefaultPosition, wxDefaultSize, 0 );
      itemStaticBoxSizer4->Add(itemStaticText6, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

      m_MarkLonCtl = new wxStaticText( itemDialog1, wxID_STATIC, _T(""), wxDefaultPosition, wxSize(180, -1), 0 );
      itemStaticBoxSizer4->Add(m_MarkLonCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM|wxGROW, 5);

      // Hyperlinks etc.
      wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Links"));

    // Controls for some Hyperlinks
      m_HyperlinkCtrlList = new HyperlinkCtrlList;
      wxStaticBoxSizer* itemStaticBoxSizer8 = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxVERTICAL);
      itemBoxSizer2->Add(itemStaticBoxSizer8, 1, wxEXPAND|wxALL|wxADJUST_MINSIZE, 5);
      for (int i=0;i<MAX_NUM_HYPERLINKS;i++)
      {
            wxHyperlinkCtrl *HyperlinkCtrl;
            HyperlinkCtrl = new wxHyperlinkCtrl(this,
                        wxID_ANY,
                        _T(" "),
                           _T(" "));
            itemStaticBoxSizer8->Add(HyperlinkCtrl, 1, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxEXPAND|wxFULL_REPAINT_ON_RESIZE, 5);
            m_HyperlinkCtrlList->Append(HyperlinkCtrl);
      }

      wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
      itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT|wxALL, 5);

      m_CancelButton = new wxButton( itemDialog1, ID_MARKINFO_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

      m_OKButton = new wxButton( itemDialog1, ID_MARKINFO_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
      m_OKButton->SetDefault();

      SetColorScheme((ColorScheme)0);
}


void MarkInfo::SetColorScheme(ColorScheme cs)
{
      SetBackgroundColour(GetGlobalColor(_T("DILG1")));

      wxColour back_color =GetGlobalColor(_T("DILG2"));
      wxColour text_color = GetGlobalColor(_T("DILG3"));

      m_MarkNameCtl->SetBackgroundColour(back_color);
      m_MarkNameCtl->SetForegroundColour(text_color);

      m_MarkLatCtl->SetBackgroundColour(back_color);
      m_MarkLatCtl->SetForegroundColour(text_color);

      m_MarkLonCtl->SetBackgroundColour(back_color);
      m_MarkLonCtl->SetForegroundColour(text_color);

      m_CancelButton->SetBackgroundColour(back_color);
      m_CancelButton->SetForegroundColour(text_color);

      m_OKButton->SetBackgroundColour(back_color);
      m_OKButton->SetForegroundColour(text_color);
}

bool MarkInfo::ShowToolTips()
{
      return TRUE;
}


void MarkInfo::SetRoutePoint(RoutePoint *pRP)
{
      m_pRoutePoint = pRP;

      m_lat_save = m_pRoutePoint->m_lat;
      m_lon_save = m_pRoutePoint->m_lon;
}


bool MarkInfo::UpdateProperties()
{
      wxString t;
      if(m_pRoutePoint)
      {
//    Name
            m_MarkNameCtl->SetLabel(m_pRoutePoint->GetName());

//  Lat/Lon
            char tc[50];

            todmm(1, m_pRoutePoint->m_lat, tc, 49);
            wxString strt(tc, wxConvUTF8);
            m_MarkLatCtl->SetLabel(strt);

            todmm(2, m_pRoutePoint->m_lon, tc, 49);
            wxString strn(tc, wxConvUTF8);
            m_MarkLonCtl->SetLabel(strn);

            int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
            HyperlinkList *hyperlinklist = m_pRoutePoint->m_HyperlinkList;
            int len = 0;
            if (NbrOfLinks > 0)
            {
                  wxHyperlinkListNode *linknode = hyperlinklist->GetFirst();
                  wxHyperlinkCtrlListNode *ctrlnode = m_HyperlinkCtrlList->GetFirst();

                  int i=0;
                           // toh, 2010.01.04, start
                          // Remove contents (if any) from hyperlink list
                  while (ctrlnode && i<MAX_NUM_HYPERLINKS)
                  {
                        wxHyperlinkCtrl *ctrl = ctrlnode->GetData();
                        ctrl->SetURL(_T(""));
                        ctrl->SetLabel(_T(""));
                        ctrlnode = ctrlnode->GetNext();
                        i++;
                  }

                  ctrlnode = m_HyperlinkCtrlList->GetFirst();

                  i=0;
                          // toh, 2010.01.04, end


                  while (linknode && ctrlnode && i<MAX_NUM_HYPERLINKS)
                  {
                        wxHyperlinkCtrl *ctrl = ctrlnode->GetData();
                        Hyperlink *link = linknode->GetData();
                        wxString Link = link->Link;
                        wxString Descr = link->DescrText;

                        ctrl->SetURL(Link);
                        ctrl->SetLabel(Descr);

                        int DescrLen = Descr.Length();
                        if (DescrLen > len)
                        {
                              len = DescrLen;
                              wxSize size = ctrl->GetBestSize();
                              ctrl->SetSize(size);
                              ctrl->Fit();
                              ctrl->SetMinSize(size);
                        }

                        linknode = linknode->GetNext();
                        ctrlnode = ctrlnode->GetNext();
                        i++;
                  }

                  if (i < 3)
                  {
                        while (ctrlnode && i<MAX_NUM_HYPERLINKS)
                        {
                              wxHyperlinkCtrl *ctrl = ctrlnode->GetData();
                              ctrl->SetURL(_T(""));
                              ctrl->SetLabel(_T(""));

                              ctrlnode = ctrlnode->GetNext();
                              i++;
                        }
                  }
            }

            GetSizer()->Fit(this);
            Centre();
      }
      return true;
}

bool MarkInfo::SaveChanges(void)
{
      return true;
}


void MarkInfo::OnMarkinfoCancelClick( wxCommandEvent& event )
{
      Show(false);
      event.Skip();
}


void MarkInfo::OnMarkinfoOkClick( wxCommandEvent& event )
{
      Show(false);
      event.Skip();
}










