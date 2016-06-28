/**************************************************************************
*
* Project:  OpenCPN
* Purpose:  RouteProperties Support
* Author:   David Register
*
***************************************************************************
*   Copyright (C) 2010 by David S. Register                               *
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
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
**************************************************************************/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/datetime.h>
#include <wx/clipbrd.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/stattext.h>
#include <wx/clrpicker.h>

#include "styles.h"
#include "routeprop.h"
#include "navutil.h"                // for Route
#include "georef.h"
#include "chart1.h"
#include "routeman.h"
#include "routemanagerdialog.h"
#include "routeprintout.h"
#include "chcanv.h"
#include "tcmgr.h"        // pjotrc 2011.03.02
#include "PositionParser.h"
#include "pluginmanager.h"
#include "OCPNPlatform.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#include <QtWidgets/QScroller>
#endif


extern double             gLat, gLon, gSog, gCog;
extern double             g_PlanSpeed;
extern wxDateTime         g_StartTime;
extern int                g_StartTimeTZ;
extern IDX_entry          *gpIDX;
extern TCMgr              *ptcmgr;
extern long               gStart_LMT_Offset;
extern MyConfig           *pConfig;
extern WayPointman        *pWayPointMan;
extern ChartCanvas        *cc1;
extern Select             *pSelect;
extern Routeman           *g_pRouteMan;
extern RouteManagerDialog *pRouteManagerDialog;
extern RouteProp          *pRoutePropDialog;
extern Track              *g_pActiveTrack;
extern RouteList          *pRouteList;
extern PlugInManager      *g_pi_manager;
extern bool                g_bShowMag;
extern wxColour            g_colourWaypointRangeRingsColour;

extern MyFrame            *gFrame;
extern OCPNPlatform       *g_Platform;

// Global print data, to remember settings during the session
extern wxPrintData               *g_printData;

// Global page setup data
extern wxPageSetupData*          g_pageSetupData;

extern float g_ChartScaleFactorExp;

/*!
* Helper stuff for calculating Route Plans
*/

#define    pi        (4.*atan(1.0))
#define    tpi        (2.*pi)
#define    twopi    (2.*pi)
#define    degs    (180./pi)
#define    rads    (pi/180.)

#define    MOTWILIGHT    1    // in some languages there may be a distinction between morning/evening
#define    SUNRISE        2
#define    DAY            3
#define    SUNSET        4
#define    EVTWILIGHT    5
#define    NIGHT        6

/* Next high tide, low tide, transition of the mark level, or some
combination.
Bit      Meaning
0       low tide
1       high tide
2       falling transition
3       rising transition
*/

#define    LW    1
#define    HW    2
#define    FALLING    4
#define    RISING    8

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

wxString GetDaylightString(int index)
{
    switch (index)
    {
        case 0:
            return      _T(" - ");
        case 1:
            return      _("MoTwilight");
        case 2:
            return      _("Sunrise");
        case 3:
            return      _("Daytime");
        case 4:
            return      _("Sunset");
        case 5:
            return      _("EvTwilight");
        case 6:
            return      _("Nighttime");

        default:
            return      _T("");
    }
}


static double sign( double x )
{
    if( x < 0. ) return -1.;
    else
        return 1.;
}

static double FNipart( double x )
{
    return ( sign( x ) * (int) ( fabs( x ) ) );
}

static double FNday( int y, int m, int d, int h )
{
    long fd = ( 367 * y - 7 * ( y + ( m + 9 ) / 12 ) / 4 + 275 * m / 9 + d );
    return ( (double) fd - 730531.5 + h / 24. );
}

static double FNrange( double x )
{
    double b = x / tpi;
    double a = tpi * ( b - FNipart( b ) );
    if( a < 0. ) a = tpi + a;
    return ( a );
}

double getDaylightEvent( double glat, double glong, int riset, double altitude, int y, int m,
        int d )
{
    double day = FNday( y, m, d, 0 );
    double days, correction;
    double utold = pi;
    double utnew = 0.;
    double sinalt = sin( altitude * rads ); // go for the sunrise/sunset altitude first
    double sinphi = sin( glat * rads );
    double cosphi = cos( glat * rads );
    double g = glong * rads;
    double t, L, G, ec, lambda, E, obl, delta, GHA, cosc;
    int limit = 12;
    while( ( fabs( utold - utnew ) > .001 ) ) {
        if( limit-- <= 0 ) return ( -1. );
        days = day + utnew / tpi;
        t = days / 36525.;
        //     get arguments of Sun's orbit
        L = FNrange( 4.8949504201433 + 628.331969753199 * t );
        G = FNrange( 6.2400408 + 628.3019501 * t );
        ec = .033423 * sin( G ) + .00034907 * sin( 2 * G );
        lambda = L + ec;
        E = -1. * ec + .0430398 * sin( 2 * lambda ) - .00092502 * sin( 4. * lambda );
        obl = .409093 - .0002269 * t;
        delta = asin( sin( obl ) * sin( lambda ) );
        GHA = utold - pi + E;
        cosc = ( sinalt - sinphi * sin( delta ) ) / ( cosphi * cos( delta ) );
        if( cosc > 1. ) correction = 0.;
        else
            if( cosc < -1. ) correction = pi;
            else
                correction = acos( cosc );
        double tmp = utnew;
        utnew = FNrange( utold - ( GHA + g + riset * correction ) );
        utold = tmp;
    }
    return ( utnew * degs / 15. );    // returns decimal hours UTC
}

static double getLMT( double ut, double lon )
{
    double t = ut + lon / 15.;
    if( t >= 0. ) if( t <= 24. ) return ( t );
    else
        return ( t - 24. );
    else
        return ( t + 24. );
}

int getDaylightStatus( double lat, double lon, wxDateTime utcDateTime )
{
    if( fabs( lat ) > 60. ) return ( 0 );
    int y = utcDateTime.GetYear();
    int m = utcDateTime.GetMonth() + 1;  // wxBug? months seem to run 0..11 ?
    int d = utcDateTime.GetDay();
    int h = utcDateTime.GetHour();
    int n = utcDateTime.GetMinute();
    int s = utcDateTime.GetSecond();
    if( y < 2000 || y > 2100 ) return ( 0 );

    double ut = (double) h + (double) n / 60. + (double) s / 3600.;
    double lt = getLMT( ut, lon );
    double rsalt = -0.833;
    double twalt = -12.;

    //wxString msg;

    if( lt <= 12. ) {
        double sunrise = getDaylightEvent( lat, lon, +1, rsalt, y, m, d );
        if( sunrise < 0. ) return ( 0 );
        else
            sunrise = getLMT( sunrise, lon );

        //            msg.Printf(_T("getDaylightEvent lat=%f lon=%f\n riset=%d rsalt=%f\n y=%d m=%d d=%d\n sun=%f lt=%f\n ut=%f\n"),
        // lat, lon, +1, rsalt, y, m, d, sunrise, lt, ut);
        //msg.Append(utcDateTime.Format());
        //            OCPNMessageDialog md1(gFrame, msg, _("Sunrise Message"), wxICON_ERROR );
        //            md1.ShowModal();

        if( fabs( lt - sunrise ) < 0.15 ) return ( SUNRISE );
        if( lt > sunrise ) return ( DAY );
        double twilight = getDaylightEvent( lat, lon, +1, twalt, y, m, d );
        if( twilight < 0. ) return ( 0 );
        else
            twilight = getLMT( twilight, lon );
        if( lt > twilight ) return ( MOTWILIGHT );
        else
            return ( NIGHT );
    } else {
        double sunset = getDaylightEvent( lat, lon, -1, rsalt, y, m, d );
        if( sunset < 0. ) return ( 0 );
        else
            sunset = getLMT( sunset, lon );
        if( fabs( lt - sunset ) < 0.15 ) return ( SUNSET );
        if( lt < sunset ) return ( DAY );
        double twilight = getDaylightEvent( lat, lon, -1, twalt, y, m, d );
        if( twilight < 0. ) return ( 0 );
        else
            twilight = getLMT( twilight, lon );
        if( lt < twilight ) return ( EVTWILIGHT );
        else
            return ( NIGHT );
    }
}

#define    UTCINPUT         0
#define    LTINPUT          1    // i.e. this PC local time
#define    LMTINPUT         2    // i.e. the remote location LMT time
#define    INPUT_FORMAT     1
#define    DISPLAY_FORMAT   2
#define    TIMESTAMP_FORMAT 3

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

WX_DECLARE_LIST(wxBitmap, BitmapList);
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(BitmapList);

WX_DECLARE_OBJARRAY(wxBitmap,      ArrayOfBitmaps);
#include <wx/arrimpl.cpp> 
WX_DEFINE_OBJARRAY(ArrayOfBitmaps);


class  OCPNIconCombo : public wxOwnerDrawnComboBox
{
public:
    
    OCPNIconCombo(wxWindow* parent, wxWindowID id, const wxString& value = _T(""),
                  const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                  int n = 0, const wxString choices[] = NULL,
                  long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = _T("OCPNIconCombo") );
    
    
    
    ~OCPNIconCombo ();
    
    void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
    wxCoord OnMeasureItem(size_t item) const;
    wxCoord OnMeasureItemWidth(size_t item) const;
    
    int Append(const wxString& item, wxBitmap bmp);
    void Clear( void );
    
private:
    int         itemHeight;
    ArrayOfBitmaps  bmpArray;
    
};


OCPNIconCombo::OCPNIconCombo (wxWindow* parent, wxWindowID id, const wxString& value,
                                  const wxPoint& pos, const wxSize& size, int n, const wxString choices[],
                                  long style, const wxValidator& validator, const wxString& name)
                        :wxOwnerDrawnComboBox(parent, id, value, pos, size, n, choices, style, validator, name)
{
    double fontHeight = GetFont().GetPointSize() / g_Platform->getFontPointsperPixel();
    itemHeight = (int)wxRound(fontHeight);
    
}

OCPNIconCombo::~OCPNIconCombo ()
{
}

void OCPNIconCombo::OnDrawItem( wxDC& dc,
                                       const wxRect& rect,
                                       int item,
                                       int flags ) const
{
    
    int offset_x = bmpArray.Item(item).GetWidth();
    int bmpHeight = bmpArray.Item(item).GetHeight();
    dc.DrawBitmap(bmpArray.Item(item), rect.x, rect.y + (rect.height - bmpHeight)/2, true);
    
    if ( flags & wxODCB_PAINTING_CONTROL )
    {
        wxString text = GetValue();
        int margin_x = 2;
        
#if wxCHECK_VERSION(2, 9, 0)
        if ( ShouldUseHintText() )
        {
            text = GetHint();
            wxColour col = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
            dc.SetTextForeground(col);
        }
        
        margin_x = GetMargins().x;
#endif

        dc.DrawText( text,
                     rect.x + margin_x + offset_x,
                     (rect.height-dc.GetCharHeight())/2 + rect.y );
    }
    else
    {
        dc.DrawText( GetVListBoxComboPopup()->GetString(item), rect.x + 2 + offset_x, (rect.height-dc.GetCharHeight())/2 + rect.y );
    }
}

wxCoord OCPNIconCombo::OnMeasureItem( size_t item ) const
{
    int bmpHeight = bmpArray.Item(item).GetHeight();
    
    return wxMax(itemHeight, bmpHeight);
}

wxCoord OCPNIconCombo::OnMeasureItemWidth( size_t item ) const
{
    return -1;
}

int OCPNIconCombo::Append(const wxString& item, wxBitmap bmp)
{
    bmpArray.Add(bmp);
    int idx = wxOwnerDrawnComboBox::Append(item);
    
    return idx;
}

void OCPNIconCombo::Clear( void )
{
    wxOwnerDrawnComboBox::Clear();
    bmpArray.Clear();
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
    EVT_RADIOBOX ( ID_TIMEZONESEL, RouteProp::OnTimeZoneSelected )
    EVT_BUTTON( ID_ROUTEPROP_CANCEL, RouteProp::OnRoutepropCancelClick )
    EVT_BUTTON( ID_ROUTEPROP_OK, RouteProp::OnRoutepropOkClick )
    EVT_LIST_ITEM_SELECTED( ID_LISTCTRL, RouteProp::OnRoutepropListClick )
    EVT_LIST_ITEM_SELECTED( ID_TRACKLISTCTRL, RouteProp::OnRoutepropListClick )
    EVT_BUTTON( ID_ROUTEPROP_SPLIT, RouteProp::OnRoutepropSplitClick )
    EVT_BUTTON( ID_ROUTEPROP_EXTEND, RouteProp::OnRoutepropExtendClick )
    EVT_BUTTON( ID_ROUTEPROP_PRINT, RouteProp::OnRoutepropPrintClick )
END_EVENT_TABLE()

/*!
 * RouteProp constructors
 */

bool RouteProp::instanceFlag = false;
RouteProp* RouteProp::single = NULL;
RouteProp* RouteProp::getInstance( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
{
    if(! instanceFlag)
    {
        single = new RouteProp( parent, id, title, pos, size, style);
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

RouteProp::RouteProp()
{
}

RouteProp::RouteProp( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
        const wxSize& size, long style )
{
    m_TotalDistCtl = NULL;
    m_wpList = NULL;
    m_nSelected = 0;
    m_pHead = NULL;
    m_pTail = NULL;
    m_pEnroutePoint = NULL;
    m_bStartNow = false;

    m_pRoute = 0;
    m_pEnroutePoint = NULL;
    m_bStartNow = false;
    long wstyle = style;
#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif

    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );
        
    m_bcompact = false;
    
#ifdef __OCPN__ANDROID__
    m_bcompact = true;
    CreateControlsCompact();
#else
    CreateControls();
#endif
    
    RecalculateSize();
}

void RouteProp::RecalculateSize( void )
{
    //  Make an estimate of the dialog size, without scrollbars showing
    
    wxSize esize;
    esize.x = GetCharWidth() * 110;
    esize.y = GetCharHeight() * 44;
    
    wxSize dsize = GetParent()->GetClientSize();
    esize.y = wxMin(esize.y, dsize.y - (2 * GetCharHeight()));
    esize.x = wxMin(esize.x, dsize.x - (1 * GetCharHeight()));
    SetClientSize(esize);
    
    wxSize fsize = GetSize();
    fsize.y = wxMin(fsize.y, dsize.y - (2 * GetCharHeight()));
    fsize.x = wxMin(fsize.x, dsize.x - (1 * GetCharHeight()));
    
    SetSize(fsize);

    if(m_bcompact){
        int sy = GetCharHeight() * m_wpList->GetItemCount();
        sy = wxMax(sy, 250);
        sy = wxMin(sy, 500);
        m_wpList->SetSize(wxSize(GetClientSize().x-40, sy) );
        
        if(m_wpList->GetItemCount())
            Layout();
    }
    
    Centre();
}


void RouteProp::OnRoutePropRightClick( wxListEvent &event )
{
    wxMenu menu;

    if( ! m_pRoute->m_bIsInLayer ) {
            
#ifdef __WXQT__    
        wxFont *pf = OCPNGetFont(_T("Menu"), 0);
            
        // add stuff
        wxMenuItem *editItem = new wxMenuItem(&menu, ID_RCLK_MENU_EDIT_WP, _("Waypoint Properties..."));
        editItem->SetFont(*pf);
        menu.Append(editItem);
        
        wxMenuItem *delItem = new wxMenuItem(&menu, ID_RCLK_MENU_DELETE, _("Remove Selected"));
        delItem->SetFont(*pf);
        menu.Append(delItem);
        
        
#else    
            
        wxMenuItem* editItem = menu.Append( ID_RCLK_MENU_EDIT_WP, _("&Waypoint Properties...") );
        
        wxMenuItem* delItem = menu.Append( ID_RCLK_MENU_DELETE, _("&Remove Selected") );
        
#endif
            
        editItem->Enable( m_wpList->GetSelectedItemCount() == 1 );
        delItem->Enable( m_wpList->GetSelectedItemCount() > 0 && m_wpList->GetItemCount() > 2 );
        
    }

    #ifndef __WXQT__    
    wxMenuItem* copyItem = menu.Append( ID_RCLK_MENU_COPY_TEXT, _("&Copy all as text") );
    #endif

    PopupMenu( &menu,  ::wxGetMousePosition() );
}

void RouteProp::OnRoutepropSplitClick( wxCommandEvent& event )
{
    m_SplitButton->Enable( false );

    if( m_pRoute->m_bIsInLayer ) return;

    if( ( m_nSelected > 1 ) && ( m_nSelected < m_pRoute->GetnPoints() ) ) {
        m_pHead = new Route();
        m_pTail = new Route();
        m_pHead->CloneRoute( m_pRoute, 1, m_nSelected, _("_A") );
        m_pTail->CloneRoute( m_pRoute, m_nSelected, m_pRoute->GetnPoints(), _("_B") );
        pRouteList->Append( m_pHead );
        pConfig->AddNewRoute( m_pHead );
        m_pHead->RebuildGUIDList();

        pRouteList->Append( m_pTail );
        pConfig->AddNewRoute( m_pTail );
        m_pTail->RebuildGUIDList();

        pConfig->DeleteConfigRoute( m_pRoute );

        pSelect->DeleteAllSelectableRoutePoints( m_pRoute );
        pSelect->DeleteAllSelectableRouteSegments( m_pRoute );
        g_pRouteMan->DeleteRoute( m_pRoute );
        pSelect->AddAllSelectableRouteSegments( m_pTail );
        pSelect->AddAllSelectableRoutePoints( m_pTail );
        pSelect->AddAllSelectableRouteSegments( m_pHead );
        pSelect->AddAllSelectableRoutePoints( m_pHead );
        
        SetRouteAndUpdate( m_pTail );
        UpdateProperties();

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateRouteListCtrl();
    }
}


// slot on pressed button "Print Route" with selection of the route properties to print
void RouteProp::OnRoutepropPrintClick( wxCommandEvent& event )
{
    RoutePrintSelection dlg( GetParent(), m_pRoute );
    dlg.ShowModal();
}

void RouteProp::OnRoutepropExtendClick( wxCommandEvent& event )
{
    m_ExtendButton->Enable( false );

    if( IsThisRouteExtendable() ) {
        int fm = m_pExtendRoute->GetIndexOf( m_pExtendPoint ) + 1;
        int to = m_pExtendRoute->GetnPoints();
        if( fm <= to ) {
            pSelect->DeleteAllSelectableRouteSegments( m_pRoute );
            m_pRoute->CloneRoute( m_pExtendRoute, fm, to, _("_plus") );
            pSelect->AddAllSelectableRouteSegments( m_pRoute );
            SetRouteAndUpdate( m_pRoute );
            UpdateProperties();
        }
    }
}

void RouteProp::OnRoutepropCopyTxtClick( wxCommandEvent& event )
{
    wxString tab("\t", wxConvUTF8);
    wxString eol("\n", wxConvUTF8);
    wxString csvString;

    csvString << this->GetTitle() << eol
            << _("Name") << tab << m_pRoute->m_RouteNameString << eol
            << _("Depart From") << tab << m_pRoute->m_RouteStartString << eol
            << _("Destination") << tab << m_pRoute->m_RouteEndString << eol
            << _("Total Distance") << tab << m_TotalDistCtl->GetValue() << eol
            << _("Speed (Kts)") << tab << m_PlanSpeedCtl->GetValue() << eol
            << _("Departure Time (m/d/y h:m)") << tab << m_StartTimeCtl->GetValue() << eol
            << _("Time Enroute") << tab << m_TimeEnrouteCtl->GetValue() << eol << eol;

    int noCols;
    int noRows;
    noCols = m_wpList->GetColumnCount();
    noRows = m_wpList->GetItemCount();
    wxListItem item;
    item.SetMask( wxLIST_MASK_TEXT );

    for( int i = 0; i < noCols; i++ ) {
        m_wpList->GetColumn( i, item );
        csvString << item.GetText() << tab;
    }
    csvString << eol;

    for( int j = 0; j < noRows; j++ ) {
        item.SetId( j );
        for( int i = 0; i < noCols; i++ ) {
            item.SetColumn( i );

            m_wpList->GetItem( item );

            csvString << item.GetText() << tab;
        }
        csvString << eol;
    }

    if( wxTheClipboard->Open() ) {
        wxTextDataObject* data = new wxTextDataObject;
        data->SetText( csvString );
        wxTheClipboard->SetData( data );
        wxTheClipboard->Close();
    }
}

bool RouteProp::IsThisRouteExtendable()
{
    m_pExtendRoute = NULL;
    m_pExtendPoint = NULL;
    if( m_pRoute->m_bRtIsActive || m_pRoute->m_bIsInLayer ) return false;

    RoutePoint *pLastPoint = m_pRoute->GetLastPoint();
    wxArrayPtrVoid *pEditRouteArray;

    pEditRouteArray = g_pRouteMan->GetRouteArrayContaining( pLastPoint );
    // remove invisible & own routes from choices
    int i;
    for( i = pEditRouteArray->GetCount(); i > 0; i-- ) {
        Route *p = (Route *) pEditRouteArray->Item( i - 1 );
        if( !p->IsVisible() || ( p->m_GUID == m_pRoute->m_GUID ) ) pEditRouteArray->RemoveAt(
            i - 1 );
    }
    if( pEditRouteArray->GetCount() == 1 ) m_pExtendPoint = pLastPoint;
    else
        if( pEditRouteArray->GetCount() == 0 ) {

            //int nearby_radius_meters = 8./cc1->GetCanvasScaleFactor(); // "nearby" means 8 pixels away
            int nearby_radius_meters = (int) ( 8. / cc1->GetCanvasTrueScale() );
            double rlat = pLastPoint->m_lat;
            double rlon = pLastPoint->m_lon;

            m_pExtendPoint = pWayPointMan->GetOtherNearbyWaypoint( rlat, rlon,
                                                                   nearby_radius_meters, pLastPoint->m_GUID );
            if( m_pExtendPoint ) {
                wxArrayPtrVoid *pCloseWPRouteArray = g_pRouteMan->GetRouteArrayContaining(
                    m_pExtendPoint );
                if( pCloseWPRouteArray ) {
                    pEditRouteArray = pCloseWPRouteArray;

                    // remove invisible & own routes from choices
                    for( i = pEditRouteArray->GetCount(); i > 0; i-- ) {
                        Route *p = (Route *) pEditRouteArray->Item( i - 1 );
                        if( !p->IsVisible() || ( p->m_GUID == m_pRoute->m_GUID ) ) pEditRouteArray->RemoveAt(
                            i - 1 );
                    }
                }
            }
        }

    if( pEditRouteArray->GetCount() == 1 ) {
        Route *p = (Route *) pEditRouteArray->Item( 0 );
        int fm = p->GetIndexOf( m_pExtendPoint ) + 1;
        int to = p->GetnPoints();
        if( fm <= to ) {
            m_pExtendRoute = p;
            delete pEditRouteArray;
            return true;
        }
    }
    delete pEditRouteArray;

    return false;
}


RouteProp::~RouteProp()
{
    delete m_TotalDistCtl;
    delete m_PlanSpeedCtl;
    delete m_TimeEnrouteCtl;

    delete m_RouteNameCtl;
    delete m_RouteStartCtl;
    delete m_RouteDestCtl;

    delete m_StartTimeCtl;

    delete m_wpList;

    instanceFlag = false;
}


/*!
 * Control creation for RouteProp
 */

void RouteProp::CreateControlsCompact()
{
     

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer1 );

    itemDialog1 = new wxScrolledWindow( this, wxID_ANY,
                                      wxDefaultPosition, wxSize(-1, -1), wxVSCROLL);
    itemDialog1->SetScrollRate(0, 1);
    

#ifdef __OCPN__ANDROID__
    //  Set Dialog Font by custom crafted Qt Stylesheet.
    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    
    wxString wqs = getFontQtStylesheet(qFont);
    wxCharBuffer sbuf = wqs.ToUTF8();
    QString qsb = QString(sbuf.data());
    
    QString qsbq = getQtStyleSheet();           // basic scrollbars, etc
    
    itemDialog1->GetHandle()->setStyleSheet( qsb + qsbq );      // Concatenated style sheets
    
#endif
    itemBoxSizer1->Add( itemDialog1, 1, wxEXPAND | wxALL, 0 );

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );


    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText4, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5 );

    m_RouteNameCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition,
            wxSize( 400, -1 ), 0 );
    itemBoxSizer2->Add( m_RouteNameCtl, 0,
                        wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM , 5 );

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Depart From"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText7, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );

    m_RouteStartCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, _T(""), wxDefaultPosition,
                                      wxSize( -1, -1 ), 0 );
    itemBoxSizer2->Add( m_RouteStartCtl, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Destination"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText8, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );


    m_RouteDestCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition,
            wxSize( -1, -1 ), 0 );
    itemBoxSizer2->Add( m_RouteDestCtl, 0,
                        wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    
    
    wxFlexGridSizer* itemFlexGridSizer6a = new wxFlexGridSizer( 4, 2, 0, 0 );
    itemFlexGridSizer6a->AddGrowableCol(1, 0);
    
    itemBoxSizer2->Add( itemFlexGridSizer6a, 0, wxEXPAND | wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC,
            _("Total Distance"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText11, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_TotalDistCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, _T(""), wxDefaultPosition,
                                     wxSize( -1, -1 ), wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_TotalDistCtl, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    
    
    
    m_PlanSpeedLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Plan Speed"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_PlanSpeedLabel, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_PlanSpeedCtl = new wxTextCtrl( itemDialog1, ID_PLANSPEEDCTL, _T(""), wxDefaultPosition,
                                     wxSize( 150, -1 ), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add( m_PlanSpeedCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    
    
    
    wxStaticText* itemStaticText12a = new wxStaticText( itemDialog1, wxID_STATIC, _("Time Enroute"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText12a, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_TimeEnrouteCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition,
                                       wxSize( -1, -1 ), wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_TimeEnrouteCtl, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    
    m_StartTimeLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Departure Time"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_StartTimeLabel, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_StartTimeCtl = new wxTextCtrl( itemDialog1, ID_STARTTIMECTL, _T(""), wxDefaultPosition,
            wxSize( -1, -1 ), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add( m_StartTimeCtl, 0,
            wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    wxString pDispTimeZone[] = { _("UTC"), _("Local @ PC"), _("LMT @ Location") };
    
    wxStaticText* itemStaticText12b = new wxStaticText( itemDialog1, wxID_STATIC, _("Times shown as"),
                                                                                    wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText12b, 0, wxEXPAND | wxALL, 5 );
    

    m_prb_tzUTC = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_UTC, _("UTC"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    itemBoxSizer2->Add( m_prb_tzUTC, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );
 
    m_prb_tzLocal = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_LOCAL, _("Local @ PC"));
    itemBoxSizer2->Add( m_prb_tzLocal, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );

    m_prb_tzLMT = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_LMT, _("LMT @ Location"));
    itemBoxSizer2->Add( m_prb_tzLMT, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );

    
    wxFlexGridSizer* itemFlexGridSizer6b = new wxFlexGridSizer( 3, 2, 0, 0 );
    itemBoxSizer2->Add( itemFlexGridSizer6b, 0, wxEXPAND | wxALIGN_LEFT | wxALL, 5 );
    
    m_staticText1 = new wxStaticText( itemDialog1, wxID_ANY, _("Color:"), wxDefaultPosition, wxDefaultSize,
            0 );
    itemFlexGridSizer6b->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxString m_chColorChoices[] = { _("Default color"), _("Black"), _("Dark Red"), _("Dark Green"),
            _("Dark Yellow"), _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
            _("Light Gray"), _("Dark Gray"), _("Red"), _("Green"), _("Yellow"), _("Blue"),
            _("Magenta"), _("Cyan"), _("White") };
    int m_chColorNChoices = sizeof( m_chColorChoices ) / sizeof(wxString);
    m_chColor = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(250, -1), m_chColorNChoices,
            m_chColorChoices, 0 );
    m_chColor->SetSelection( 0 );
    itemFlexGridSizer6b->Add( m_chColor, 0,  wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxStaticText *staticTextStyle = new wxStaticText( itemDialog1, wxID_ANY, _("Style:"), wxDefaultPosition, wxDefaultSize,
            0 );
    itemFlexGridSizer6b->Add( staticTextStyle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxString m_chStyleChoices[] = { _("Default"), _("Solid"), _("Dot"), _("Long dash"),
            _("Short dash"), _("Dot dash") };
    int m_chStyleNChoices = sizeof( m_chStyleChoices ) / sizeof(wxString);
    m_chStyle = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chStyleNChoices,
            m_chStyleChoices, 0 );
    m_chStyle->SetSelection( 0 );
    itemFlexGridSizer6b->Add( m_chStyle, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

#ifdef ocpnUSE_GLES // linestipple is emulated poorly
    staticTextStyle->Hide();
    m_chStyle->Hide();
#endif    
    
    
    m_staticText2 = new wxStaticText( itemDialog1, wxID_ANY, _("Width:"), wxDefaultPosition, wxDefaultSize,
            0 );
    itemFlexGridSizer6b->Add( m_staticText2, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxString m_chWidthChoices[] = { _("Default"), _("1 pixel"), _("2 pixels"), _("3 pixels"),
            _("4 pixels"), _("5 pixels"), _("6 pixels"), _("7 pixels"), _("8 pixels"),
            _("9 pixels"), _("10 pixels") };
    int m_chWidthNChoices = sizeof( m_chWidthChoices ) / sizeof(wxString);
    m_chWidth = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(150, -1), m_chWidthNChoices,
            m_chWidthChoices, 0 );
    m_chWidth->SetSelection( 0 );
    itemFlexGridSizer6b->Add( m_chWidth, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5 );


 
    wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox( itemDialog1, wxID_ANY, _("Waypoints") );
    m_pListSizer = new wxStaticBoxSizer( itemStaticBoxSizer14Static, wxVERTICAL );
    itemBoxSizer2->Add( m_pListSizer, 1, wxEXPAND | wxALL, 1 );
 
    
    wxScrolledWindow *itemlistWin = new wxScrolledWindow( itemDialog1, wxID_ANY,
                                                          wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    itemlistWin->SetScrollRate(2, 2);
    
    
    
    m_pListSizer->Add( itemlistWin, 0, wxEXPAND | wxALL, 6 );
    
    
    //      Create the list control
    m_wpList = new wxListCtrl( itemlistWin, ID_LISTCTRL, wxDefaultPosition, wxSize( 100, -1 ),
                               wxLC_REPORT | wxLC_HRULES | wxLC_VRULES );
    
    
#ifdef __OCPN__ANDROID__
        m_wpList->GetHandle()->setStyleSheet( getQtStyleSheet());
#endif    
        //  Buttons, etc...

/*        
        wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer( wxVERTICAL );
        itemBoxSizer1->Add( itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5 );
        
        
        int n_col = 3;
        
        wxFlexGridSizer* itemBoxSizerAux = new wxFlexGridSizer( 0, n_col, 0, 0 );
        itemBoxSizerAux->SetFlexibleDirection( wxBOTH );
        itemBoxSizerAux->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        
        
        
        itemBoxSizerBottom->Add( itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 5 );
        
#ifndef __OCPN__ANDROID__
        m_PrintButton = new wxButton( this, wxID_ANY, _("Print"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizerAux->Add( m_PrintButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );
        m_PrintButton->Enable( true );
#else
        m_PrintButton = NULL;
#endif
        
        m_SplitButton = new wxButton( this, wxID_ANY, _("Split"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizerAux->Add( m_SplitButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );
        m_SplitButton->Enable( false );
        
        m_ExtendButton = new wxButton( this, wxID_ANY, _("Extend"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizerAux->Add( m_ExtendButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );
        
        wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
        itemBoxSizerBottom->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5 );
        
        m_CancelButton = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer16->Add( m_CancelButton, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 5 );
        
        m_OKButton = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer16->Add( m_OKButton, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 5 );
        m_OKButton->SetDefault();
*/        
    
    wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer1->Add( itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5 );
    
    wxBoxSizer* itemBoxSizerAux = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizerBottom->Add( itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 3 );

#ifndef __OCPN__ANDROID__
    m_PrintButton = new wxButton( this, ID_ROUTEPROP_PRINT, _("Print Route"),
            wxDefaultPosition, wxDefaultSize, 0 );
     itemBoxSizerAux->Add( m_PrintButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3 );
    m_PrintButton->Enable( true );
#else
    m_PrintButton = NULL;
#endif
    
    m_ExtendButton = new wxButton( this, ID_ROUTEPROP_EXTEND, _("Extend Route"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizerAux->Add( m_ExtendButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3 );
    m_ExtendButton->Enable( false );

    m_SplitButton = new wxButton( this, ID_ROUTEPROP_SPLIT, _("Split Route"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizerAux->Add( m_SplitButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3 );
    m_SplitButton->Enable( false );

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizerBottom->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 3 );

    m_CancelButton = new wxButton( this, ID_ROUTEPROP_CANCEL, _("Cancel"), wxDefaultPosition,
            wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_CancelButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );

    m_OKButton = new wxButton( this, ID_ROUTEPROP_OK, _("OK"), wxDefaultPosition,
            wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_OKButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1);
    m_OKButton->SetDefault();



    //      To correct a bug in MSW commctl32, we need to catch column width drag events, and do a Refresh()
    //      Otherwise, the column heading disappear.....
    //      Does no harm for GTK builds, so no need for conditional
    Connect( wxEVT_COMMAND_LIST_COL_END_DRAG,
            (wxObjectEventFunction) (wxEventFunction) &RouteProp::OnEvtColDragEnd );


    
    
    int char_size = GetCharWidth();

    m_wpList->InsertColumn( 0, _("Leg"), wxLIST_FORMAT_LEFT, 10 );
    m_wpList->InsertColumn( 1, _("To Waypoint"), wxLIST_FORMAT_LEFT, char_size * 14 );
    m_wpList->InsertColumn( 2, _("Distance"), wxLIST_FORMAT_RIGHT, char_size * 9 );

    if(g_bShowMag)
        m_wpList->InsertColumn( 3, _("Bearing (M)"), wxLIST_FORMAT_LEFT, char_size * 10 );
    else
        m_wpList->InsertColumn( 3, _("Bearing"), wxLIST_FORMAT_LEFT, char_size * 10 );

    m_wpList->InsertColumn( 4, _("Latitude"), wxLIST_FORMAT_LEFT, char_size * 11 );
    m_wpList->InsertColumn( 5, _("Longitude"), wxLIST_FORMAT_LEFT, char_size * 11 );
    m_wpList->InsertColumn( 6, _("ETE/ETD"), wxLIST_FORMAT_LEFT, char_size * 15 );
    m_wpList->InsertColumn( 7, _("Speed"), wxLIST_FORMAT_CENTER, char_size * 9 );
    m_wpList->InsertColumn( 8, _("Next tide event"), wxLIST_FORMAT_LEFT, char_size * 11 );
    m_wpList->InsertColumn( 9, _("Description"), wxLIST_FORMAT_LEFT, char_size * 11 );
    if(g_bShowMag)
        m_wpList->InsertColumn( 10, _("Course (M)"), wxLIST_FORMAT_LEFT, char_size * 10 );
    else
        m_wpList->InsertColumn( 10, _("Course"), wxLIST_FORMAT_LEFT, char_size * 10 );

    
   
    //Set the maximum size of the entire  dialog
    int width, height;
    ::wxDisplaySize( &width, &height );
    SetSizeHints( -1, -1, width-100, height-100 );
    
    Connect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
            wxListEventHandler(RouteProp::OnRoutePropRightClick), NULL, this );
    Connect( wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(RouteProp::OnRoutePropMenuSelected), NULL, this );

    //  Fetch any config file values
    m_planspeed = g_PlanSpeed;

    if( g_StartTimeTZ == 0 )
        m_prb_tzUTC->SetValue( true);
    else if( g_StartTimeTZ == 1 )
        m_prb_tzLocal->SetValue( true);
    else if( g_StartTimeTZ == 2 )
        m_prb_tzLMT->SetValue( true);
    

    SetColorScheme( (ColorScheme) 0 );

    
}

void RouteProp::CreateControls()
{
    
    
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer1 );
    
    itemDialog1 = new wxScrolledWindow( this, wxID_ANY,
                                        wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    itemDialog1->SetScrollRate(2, 2);
    
    #ifdef __OCPN__ANDROID__
    //itemDialog1->GetHandle()->setStyleSheet( getQtStyleSheet());
    
    //  Set Dialog Font by custom crafted Qt Stylesheet.
    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    
    wxString wqs = getFontQtStylesheet(qFont);
    wxCharBuffer sbuf = wqs.ToUTF8();
    QString qsb = QString(sbuf.data());
    
    QString qsbq = getQtStyleSheet();           // basic scrollbars, etc
    
    itemDialog1->GetHandle()->setStyleSheet( qsb + qsbq );      // Concatenated style sheets
    
    #endif
    itemBoxSizer1->Add( itemDialog1, 2, wxEXPAND | wxALL, 0 );
    
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );
    
    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox( itemDialog1, wxID_ANY,
                                                              _("Properties") );
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer( itemStaticBoxSizer3Static,
                                                                  wxVERTICAL );
    itemBoxSizer2->Add( itemStaticBoxSizer3, 0, wxEXPAND | wxALL, 5 );
    
    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name"),
                                                      wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add( itemStaticText4, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5 );
    
    m_RouteNameCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition,
                                     wxSize( 710, -1 ), 0 );
    itemStaticBoxSizer3->Add( m_RouteNameCtl, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );
    
    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer( 2, 2, 0, 0 );
    itemStaticBoxSizer3->Add( itemFlexGridSizer6, 1, wxALIGN_LEFT | wxALL, 5 );
    
    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Depart From"),
                                                      wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add( itemStaticText7, 0,
                             wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );
    
    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Destination"),
                                                      wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add( itemStaticText8, 0,
                             wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );
    
    m_RouteStartCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, _T(""), wxDefaultPosition,
                                      wxSize( 300, -1 ), 0 );
    itemFlexGridSizer6->Add( m_RouteStartCtl, 0,
                             wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    m_RouteDestCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition,
                                     wxSize( 300, -1 ), 0 );
    itemFlexGridSizer6->Add( m_RouteDestCtl, 0,
                             wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    wxFlexGridSizer* itemFlexGridSizer6a = new wxFlexGridSizer( 2, 4, 0, 0 );
    itemStaticBoxSizer3->Add( itemFlexGridSizer6a, 1, wxALIGN_LEFT | wxALL, 5 );
    
    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC,
                                                       _("Total Distance"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText11, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
                              5 );
    
    m_PlanSpeedLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Plan Speed"),
                                         wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_PlanSpeedLabel, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
                              5 );
    
    wxStaticText* itemStaticText12a = new wxStaticText( itemDialog1, wxID_STATIC, _("Time Enroute"),
                                                        wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText12a, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
                              5 );
    
    m_StartTimeLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Departure Time (m/d/y h:m)"),
                                         wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_StartTimeLabel, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
                              5 );
    
    m_TotalDistCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, _T(""), wxDefaultPosition,
                                     wxDefaultSize, wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_TotalDistCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    m_PlanSpeedCtl = new wxTextCtrl( itemDialog1, ID_PLANSPEEDCTL, _T(""), wxDefaultPosition,
                                     wxSize( 100, -1 ), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add( m_PlanSpeedCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    m_TimeEnrouteCtl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition,
                                       wxSize( 200, -1 ), wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_TimeEnrouteCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    m_StartTimeCtl = new wxTextCtrl( itemDialog1, ID_STARTTIMECTL, _T(""), wxDefaultPosition,
                                     wxSize( 150, -1 ), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add( m_StartTimeCtl, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
    
    wxString pDispTimeZone[] = { _("UTC"), _("Local @ PC"), _("LMT @ Location") };
    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer( wxHORIZONTAL );
    
    wxStaticBox* itemStaticBoxTZ = new wxStaticBox( itemDialog1, wxID_ANY,  _("Times shown as") );
    wxStaticBoxSizer* itemStaticBoxSizerTZ = new wxStaticBoxSizer( itemStaticBoxTZ, wxHORIZONTAL );
    bSizer2->Add( itemStaticBoxSizerTZ, 0, wxEXPAND | wxALL, 5 );
    
    
    m_prb_tzUTC = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_UTC, _("UTC"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    itemStaticBoxSizerTZ->Add( m_prb_tzUTC, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );
    
    m_prb_tzLocal = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_LOCAL, _("Local @ PC"));
    itemStaticBoxSizerTZ->Add( m_prb_tzLocal, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );
    
    m_prb_tzLMT = new wxRadioButton(itemDialog1, ID_TIMEZONESEL_LMT, _("LMT @ Location"));
    itemStaticBoxSizerTZ->Add( m_prb_tzLMT, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );
    
    m_staticText1 = new wxStaticText( itemDialog1, wxID_ANY, _("Color:"), wxDefaultPosition, wxDefaultSize,
                                      0 );
    //m_staticText1->Wrap( -1 );
    bSizer2->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
    
    wxString m_chColorChoices[] = { _("Default color"), _("Black"), _("Dark Red"), _("Dark Green"),
    _("Dark Yellow"), _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
      _("Light Gray"), _("Dark Gray"), _("Red"), _("Green"), _("Yellow"), _("Blue"),
      _("Magenta"), _("Cyan"), _("White") };
      int m_chColorNChoices = sizeof( m_chColorChoices ) / sizeof(wxString);
      m_chColor = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chColorNChoices,
                                m_chColorChoices, 0 );
      m_chColor->SetSelection( 0 );
      bSizer2->Add( m_chColor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
      
      wxStaticText *staticTextStyle = new wxStaticText( itemDialog1, wxID_ANY, _("Style:"), wxDefaultPosition, wxDefaultSize,
        0 );
      bSizer2->Add( staticTextStyle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
      
      wxString m_chStyleChoices[] = { _("Default"), _("Solid"), _("Dot"), _("Long dash"),
      _("Short dash"), _("Dot dash") };
      int m_chStyleNChoices = sizeof( m_chStyleChoices ) / sizeof(wxString);
      m_chStyle = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chStyleNChoices,
                                m_chStyleChoices, 0 );
      m_chStyle->SetSelection( 0 );
      bSizer2->Add( m_chStyle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
      
      #ifdef ocpnUSE_GLES // linestipple is emulated poorly
      staticTextStyle->Hide();
      m_chStyle->Hide();
      #endif    
      
      
      m_staticText2 = new wxStaticText( itemDialog1, wxID_ANY, _("Width:"), wxDefaultPosition, wxDefaultSize,
        0 );
      //m_staticText2->Wrap( -1 );
      bSizer2->Add( m_staticText2, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
      
      wxString m_chWidthChoices[] = { _("Default"), _("1 pixel"), _("2 pixels"), _("3 pixels"),
      _("4 pixels"), _("5 pixels"), _("6 pixels"), _("7 pixels"), _("8 pixels"),
        _("9 pixels"), _("10 pixels") };
        int m_chWidthNChoices = sizeof( m_chWidthChoices ) / sizeof(wxString);
        m_chWidth = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chWidthNChoices,
                                  m_chWidthChoices, 0 );
        m_chWidth->SetSelection( 0 );
        bSizer2->Add( m_chWidth, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
        
        itemStaticBoxSizer3->Add( bSizer2, 1, wxEXPAND, 0 );
        
        wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox( this, wxID_ANY, _("Waypoints") );
        m_pListSizer = new wxStaticBoxSizer( itemStaticBoxSizer14Static, wxVERTICAL );
        itemBoxSizer1->Add( m_pListSizer, 2, wxEXPAND | wxALL, 1 );
        
        //      Create the list control
        m_wpList = new wxListCtrl( this, ID_LISTCTRL, wxDefaultPosition, wxSize( -1, -1 ),
                                   wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS );
        
        m_wpList->SetMinSize(wxSize(-1, 100) );
        m_pListSizer->Add( m_wpList, 1, wxEXPAND | wxALL, 6 );
        
        
        #ifdef __OCPN__ANDROID__
        m_wpList->GetHandle()->setStyleSheet( getQtStyleSheet());
        #endif    
        
        
        wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer( wxHORIZONTAL );
        itemBoxSizer1->Add( itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5 );
        
        wxBoxSizer* itemBoxSizerAux = new wxBoxSizer( wxHORIZONTAL );
        itemBoxSizerBottom->Add( itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 3 );
        
        m_PrintButton = new wxButton( this, ID_ROUTEPROP_PRINT, _("Print Route"),
          wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizerAux->Add( m_PrintButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3 );
        m_PrintButton->Enable( true );
        
        m_ExtendButton = new wxButton( this, ID_ROUTEPROP_EXTEND, _("Extend Route"),
        wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_ExtendButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3 );
      m_ExtendButton->Enable( false );
      
      m_SplitButton = new wxButton( this, ID_ROUTEPROP_SPLIT, _("Split Route"),
      wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_SplitButton, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 3 );
      m_SplitButton->Enable( false );
      
      wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
      itemBoxSizerBottom->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 3 );
      
      m_CancelButton = new wxButton( this, ID_ROUTEPROP_CANCEL, _("Cancel"), wxDefaultPosition,
      wxDefaultSize, 0 );
      itemBoxSizer16->Add( m_CancelButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
      
      m_OKButton = new wxButton( this, ID_ROUTEPROP_OK, _("OK"), wxDefaultPosition,
      wxDefaultSize, 0 );
      itemBoxSizer16->Add( m_OKButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1);
      m_OKButton->SetDefault();
      
      //      To correct a bug in MSW commctl32, we need to catch column width drag events, and do a Refresh()
      //      Otherwise, the column heading disappear.....
      //      Does no harm for GTK builds, so no need for conditional
      Connect( wxEVT_COMMAND_LIST_COL_END_DRAG,
               (wxObjectEventFunction) (wxEventFunction) &RouteProp::OnEvtColDragEnd );
      
      
      
      
      int char_size = GetCharWidth();
      
      m_wpList->InsertColumn( 0, _("Leg"), wxLIST_FORMAT_LEFT, char_size * 6 );
      m_wpList->InsertColumn( 1, _("To Waypoint"), wxLIST_FORMAT_LEFT, char_size * 14 );
      m_wpList->InsertColumn( 2, _("Distance"), wxLIST_FORMAT_RIGHT, char_size * 9 );
      
      if(g_bShowMag)
          m_wpList->InsertColumn( 3, _("Bearing (M)"), wxLIST_FORMAT_LEFT, char_size * 10 );
      else
          m_wpList->InsertColumn( 3, _("Bearing"), wxLIST_FORMAT_LEFT, char_size * 10 );
      
      m_wpList->InsertColumn( 4, _("Latitude"), wxLIST_FORMAT_LEFT, char_size * 11 );
      m_wpList->InsertColumn( 5, _("Longitude"), wxLIST_FORMAT_LEFT, char_size * 11 );
      m_wpList->InsertColumn( 6, _("ETE/ETD"), wxLIST_FORMAT_LEFT, char_size * 15 );
      m_wpList->InsertColumn( 7, _("Speed"), wxLIST_FORMAT_CENTER, char_size * 9 );
      m_wpList->InsertColumn( 8, _("Next tide event"), wxLIST_FORMAT_LEFT, char_size * 11 );
      m_wpList->InsertColumn( 9, _("Description"), wxLIST_FORMAT_LEFT, char_size * 11 );
      if(g_bShowMag)
          m_wpList->InsertColumn( 10, _("Course (M)"), wxLIST_FORMAT_LEFT, char_size * 10 );
      else
          m_wpList->InsertColumn( 10, _("Course"), wxLIST_FORMAT_LEFT, char_size * 10 );
      
      
      //Set the maximum size of the entire  dialog
      int width, height;
      ::wxDisplaySize( &width, &height );
      SetSizeHints( -1, -1, width-100, height-100 );
      
      Connect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
               wxListEventHandler(RouteProp::OnRoutePropRightClick), NULL, this );
      Connect( wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler(RouteProp::OnRoutePropMenuSelected), NULL, this );
      
      //  Fetch any config file values
      m_planspeed = g_PlanSpeed;
      
      if( g_StartTimeTZ == 0 )
          m_prb_tzUTC->SetValue( true);
      else if( g_StartTimeTZ == 1 )
          m_prb_tzLocal->SetValue( true);
      else if( g_StartTimeTZ == 2 )
          m_prb_tzLMT->SetValue( true);
      
      
      SetColorScheme( (ColorScheme) 0 );
      
      
      }
      
int RouteProp::GetTZSelection(void)
{
    if(m_prb_tzUTC && m_prb_tzUTC->GetValue())
        return 0;

    else if(m_prb_tzLocal && m_prb_tzLocal->GetValue())
        return 1;
    
    else if(m_prb_tzLMT && m_prb_tzLMT->GetValue())
        return 2;
    
    else
        return 0;
}
void RouteProp::OnRoutepropListClick( wxListEvent& event )
{
    long itemno = 0;
    m_nSelected = 0;

    //      We use different methods to determine the selected point,
    //      depending on whether this is a Route or a Track.
    int selected_no;
    const wxListItem &i = event.GetItem();
    i.GetText().ToLong( &itemno );
    selected_no = itemno;

    m_pRoute->ClearHighlights();

    wxRoutePointListNode *node = m_pRoute->pRoutePointList->GetFirst();
    while( node && itemno-- ) {
        node = node->GetNext();
    }
    if( node ) {
        RoutePoint *prp = node->GetData();
        if( prp ) {
            prp->m_bPtIsSelected = true;                // highlight the routepoint

            if( !m_pRoute->m_bIsInLayer && !m_pRoute->m_bRtIsActive ) {
                m_nSelected = selected_no + 1;
                m_SplitButton->Enable( true );
            }

            gFrame->JumpToPosition( prp->m_lat, prp->m_lon, cc1->GetVPScale() );

        }
    }
}


void RouteProp::OnRoutePropMenuSelected( wxCommandEvent& event )
{
    switch( event.GetId() ) {
        case ID_RCLK_MENU_COPY_TEXT: {
            OnRoutepropCopyTxtClick( event );
            break;
        }
        case ID_RCLK_MENU_DELETE: {
            int dlg_return = OCPNMessageBox( this, _("Are you sure you want to remove this waypoint?"),
                    _("OpenCPN Remove Waypoint"), (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );

            if( dlg_return == wxID_YES ) {
                long item = -1;
                item = m_wpList->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

                if( item == -1 ) break;

                RoutePoint *wp;
                wp = (RoutePoint *) m_wpList->GetItemData( item );

                cc1->RemovePointFromRoute( wp, m_pRoute );
            }
            break;
        }
        case ID_RCLK_MENU_EDIT_WP: {
            long item = -1;

            item = m_wpList->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

            if( item == -1 ) break;

            RoutePoint *wp = (RoutePoint *) m_wpList->GetItemData( item );
            if( !wp ) break;

            RouteManagerDialog::WptShowPropertiesDialog( wp, this );
            break;
        }
    }
}

void RouteProp::SetColorScheme( ColorScheme cs )
{
    DimeControl( this );
}

/*
 * Should we show tooltips?
 */

bool RouteProp::ShowToolTips()
{
    return TRUE;
}

void RouteProp::SetDialogTitle(const wxString & title)
{
    SetTitle(title);
}

void RouteProp::SetRouteAndUpdate( Route *pR, bool only_points )
{
    if( NULL == pR )
        return;

    //  Fetch any config file values
    if ( !only_points )
    {
        //      long LMT_Offset = 0;                    // offset in seconds from UTC for given location (-1 hr / 15 deg W)
        m_tz_selection = 1;

        if( pR == m_pRoute ) {
            gStart_LMT_Offset = 0;
            if( pR->m_PlannedDeparture.IsValid() )
                m_starttime = pR->m_PlannedDeparture;
            else
                m_starttime = g_StartTime;

        } else {
            g_StartTime = wxInvalidDateTime;
            g_StartTimeTZ = 1;
            if( pR->m_PlannedDeparture.IsValid() )
                m_starttime = pR->m_PlannedDeparture;
            else
                m_starttime = g_StartTime;
            if( pR->m_TimeDisplayFormat == RTE_TIME_DISP_UTC)
                m_tz_selection = 0;
            else if( pR->m_TimeDisplayFormat == RTE_TIME_DISP_LOCAL )
                m_tz_selection = 2;
            else
                m_tz_selection = g_StartTimeTZ;
            gStart_LMT_Offset = 0;
            m_pEnroutePoint = NULL;
            m_bStartNow = false;
            m_planspeed = pR->m_PlannedSpeed;
        }

        m_pRoute = pR;
        
        if( g_StartTimeTZ == 0 )
            m_prb_tzUTC->SetValue( true);
        else if( g_StartTimeTZ == 1 )
            m_prb_tzLocal->SetValue( true);
        else if( g_StartTimeTZ == 2 )
            m_prb_tzLMT->SetValue( true);
        

        if( m_pRoute ) {
            //    Calculate  LMT offset from the first point in the route
            if( m_pEnroutePoint && m_bStartNow ) gStart_LMT_Offset = long(
                    ( m_pEnroutePoint->m_lon ) * 3600. / 15. );
            else
                gStart_LMT_Offset = long(
                        ( m_pRoute->pRoutePointList->GetFirst()->GetData()->m_lon ) * 3600. / 15. );
        }

        // Reorganize dialog for route or track display
        if( m_pRoute ) {
            m_PlanSpeedLabel->SetLabel( _("Plan speed") );
            m_PlanSpeedCtl->SetEditable( true );
            m_ExtendButton->SetLabel( _("Extend Route") );
            m_SplitButton->SetLabel( _("Split Route") );

            //    Fill in some top pane properties from the Route member elements
            m_RouteNameCtl->SetValue( m_pRoute->m_RouteNameString );
            m_RouteStartCtl->SetValue( m_pRoute->m_RouteStartString );
            m_RouteDestCtl->SetValue( m_pRoute->m_RouteEndString );

            m_RouteNameCtl->SetFocus();
        } else {
            m_RouteNameCtl->Clear();
            m_RouteStartCtl->Clear();
            m_RouteDestCtl->Clear();
            m_PlanSpeedCtl->Clear();
            m_StartTimeCtl->Clear();
        }
    }
    //      if(m_pRoute)
    //            m_pRoute->UpdateSegmentDistances(m_planspeed);           // to interpret ETD properties

    m_wpList->DeleteAllItems();

#if 0
    // Select the proper list control, and add it to List sizer
    m_pListSizer->Clear();

    if( m_pRoute ) {
        m_wpList->Show();
        m_pListSizer->Add( m_wpList, 2, wxEXPAND | wxALL, 5 );
    }
//    GetSizer()->Fit( this );
    GetSizer()->Layout();
#endif


    InitializeList();

    UpdateProperties();

    if( m_pRoute )
        m_wpList->Show();

//    GetSizer()->Fit( this );
//    GetSizer()->Layout();

    RecalculateSize();
    Refresh( false );

}

void RouteProp::InitializeList()
{
    if( NULL == m_pRoute ) return;

    m_pRoute->UpdateSegmentDistances( m_planspeed );           // to fix ETD properties
    //  Iterate on Route Points, inserting blank fields starting with index 0
    wxRoutePointListNode *pnode = m_pRoute->pRoutePointList->GetFirst();
    int in = 0;
    while( pnode ) {
        m_wpList->InsertItem( in, _T(""), -1 );
        m_wpList->SetItemPtrData( in, (wxUIntPtr)pnode->GetData() );
        in++;
        if( pnode->GetData()->m_seg_etd.IsValid() ) {
            m_wpList->InsertItem( in, _T(""), -1 );
            in++;
        }
        pnode = pnode->GetNext();
    }

    //      Update the plan speed and route start time controls
    wxString s;
    s.Printf( _T("%5.2f"), m_planspeed );
    m_PlanSpeedCtl->SetValue( s );

    if( m_starttime.IsValid() ) {
        wxString s = ts2s( m_starttime, m_tz_selection, (int) gStart_LMT_Offset, INPUT_FORMAT );
        m_StartTimeCtl->SetValue( s );
    } else
        m_StartTimeCtl->Clear();
}

bool RouteProp::UpdateProperties()
{
    if( NULL == m_pRoute ) return false;

    ::wxBeginBusyCursor();

    m_TotalDistCtl->SetValue( _T("") );
    m_TimeEnrouteCtl->SetValue( _T("") );
    int tz_selection = GetTZSelection();
    long LMT_Offset = 0;         // offset in seconds from UTC for given location (-1 hr / 15 deg W)

    m_SplitButton->Enable( false );
    m_ExtendButton->Enable( false );

#if 0
    if( m_pRoute->m_bIsTrack ) {
        m_pRoute->UpdateSegmentDistances();           // get segment and total distance
        // but ignore leg speed calcs

        // Calculate AVG speed if we are showing a track and total time
        RoutePoint *last_point = m_pRoute->GetLastPoint();
        RoutePoint *first_point = m_pRoute->GetPoint( 1 );
        double total_seconds = 0.;

        if( last_point->GetCreateTime().IsValid() && first_point->GetCreateTime().IsValid() ) {
            total_seconds =
                    last_point->GetCreateTime().Subtract( first_point->GetCreateTime() ).GetSeconds().ToDouble();
            if( total_seconds != 0. ) {
                m_avgspeed = m_pRoute->m_route_length / total_seconds * 3600;
            } else {
                m_avgspeed = 0;
            }
            wxString s;
            s.Printf( _T("%5.2f"), toUsrSpeed( m_avgspeed ) );
            m_PlanSpeedCtl->SetValue( s );
        } else {
            wxString s( _T("--") );
            m_PlanSpeedCtl->SetValue( s );
        }

        //  Total length
        wxString slen;
        slen.Printf( wxT("%5.2f ") + getUsrDistanceUnit(), toUsrDistance( m_pRoute->m_route_length ) );

        m_TotalDistCtl->SetValue( slen );

        //  Time
        wxString time_form;
        wxTimeSpan time( 0, 0, (int) total_seconds, 0 );
        if(m_bcompact){
            if( total_seconds > 3600. * 24. )
                time_form = time.Format( _(" %D D  %H H  %M M") );
            else
                if( total_seconds > 0. )
                    time_form = time.Format( _(" %H H  %M M") );
                else
                    time_form = _T("--");
        }
        else{
            if( total_seconds > 3600. * 24. )
                time_form = time.Format( _(" %D Days  %H Hours  %M Minutes") );
            else
                if( total_seconds > 0. )
                    time_form = time.Format( _(" %H Hours  %M Minutes") );
                else
                    time_form = _T("--");
        }
        
        m_TimeEnrouteCtl->SetValue( time_form );

    } else        // Route
#endif        
    {
        double brg;
        double join_distance = 0.;
        RoutePoint *first_point = m_pRoute->GetPoint( 1 );
        if( first_point )
            DistanceBearingMercator( first_point->m_lat, first_point->m_lon, gLat, gLon, &brg, &join_distance );

        //    Update the "tides event" column header
        wxListItem column_info;
        if( m_wpList->GetColumn( 8, column_info ) ) {
            wxString c = _("Next tide event");
            if( gpIDX && m_starttime.IsValid() ) {
                c = _T("@~~");
                c.Append( wxString( gpIDX->IDX_station_name, wxConvUTF8 ) );
                int i = c.Find( ',' );
                if( i != wxNOT_FOUND ) c.Remove( i );

            }
            column_info.SetText( c );
            m_wpList->SetColumn( 8, column_info );
        }

        //Update the "ETE/Timestamp" column header

        if( m_wpList->GetColumn( 6, column_info ) ) {
            if( m_starttime.IsValid() ) column_info.SetText( _("ETA") );
            else
                column_info.SetText( _("ETE") );
            m_wpList->SetColumn( 6, column_info );
        }

        m_pRoute->UpdateSegmentDistances( m_planspeed );           // get segment and total distance
        double leg_speed = m_planspeed;
        wxTimeSpan stopover_time( 0 ); // time spent waiting for ETD
        wxTimeSpan joining_time( 0 );   // time spent before reaching first waypoint

        double total_seconds = 0.;

        if( m_pRoute ) {
            total_seconds = m_pRoute->m_route_time;
            if( m_bStartNow ) {
                if( m_pEnroutePoint ) gStart_LMT_Offset = long(
                        ( m_pEnroutePoint->m_lon ) * 3600. / 15. );
                else
                    gStart_LMT_Offset = long(
                            ( m_pRoute->pRoutePointList->GetFirst()->GetData()->m_lon ) * 3600.
                                    / 15. );
            }
        }

        if( m_bStartNow ) {
            joining_time = wxTimeSpan::Seconds((long) wxRound( ( join_distance * 3600. ) / m_planspeed ) );
            double join_seconds = joining_time.GetSeconds().ToDouble();
            total_seconds += join_seconds;
        }

        if( m_starttime.IsValid() ) {
            wxString s;
            if( m_bStartNow ) {
                wxDateTime d = wxDateTime::Now();
                if( tz_selection == 1 ) {
                    m_starttime = d.ToUTC();
                    s = _T(">");
                }
            }
            s += ts2s( m_starttime, tz_selection, (int) gStart_LMT_Offset, INPUT_FORMAT );
            m_StartTimeCtl->SetValue( s );
        } else
            m_StartTimeCtl->Clear();

        if( IsThisRouteExtendable() ) m_ExtendButton->Enable( true );

        //  Total length
        double total_length = m_pRoute->m_route_length;
        if( m_bStartNow ) {
            total_length += join_distance;
        }

        wxString slen;
        slen.Printf( wxT("%5.2f ") + getUsrDistanceUnit(), toUsrDistance( total_length ) );

        if( !m_pEnroutePoint ) m_TotalDistCtl->SetValue( slen );
        else
            m_TotalDistCtl->Clear();

        wxString time_form;
        wxString tide_form;

        //  Time
        wxTimeSpan time( 0, 0, (int) total_seconds, 0 );
        
        if(m_bcompact){
            if( total_seconds > 3600. * 24. )
                time_form = time.Format( _(" %D D  %H H  %M M") );
            else
                if( total_seconds > 0. )
                    time_form = time.Format( _(" %H H  %M M") );
                else
                    time_form = _T("--");
        }
        else{
            if( total_seconds > 3600. * 24. )
                time_form = time.Format( _(" %D Days  %H Hours  %M Minutes") );
            else
                if( total_seconds > 0. )
                    time_form = time.Format( _(" %H Hours  %M Minutes") );
                else
                    time_form = _T("--");
        }
        

        if( !m_pEnroutePoint )
            m_TimeEnrouteCtl->SetValue( time_form );
        else
            m_TimeEnrouteCtl->Clear();

        //  Iterate on Route Points
        wxRoutePointListNode *node = m_pRoute->pRoutePointList->GetFirst();

        int i = 0;
        double slat = gLat;
        double slon = gLon;
        double tdis = 0.;
        double tsec = 0.;    // total time in seconds

        int stopover_count = 0;
        bool arrival = true; // marks which pass over the wpt we do - 1. arrival 2. departure
        bool enroute = true; // for active route, skip all points up to the active point

        if( m_pRoute->m_bRtIsActive ) {
            if( m_pEnroutePoint && m_bStartNow )
                enroute = ( m_pRoute->GetPoint( 1 )->m_GUID == m_pEnroutePoint->m_GUID );
        }

        wxString nullify = _T("----");

        int i_prev_point = -1;
        RoutePoint *prev_route_point = NULL;

        while( node ) {
            RoutePoint *prp = node->GetData();
            long item_line_index = i + stopover_count;

            //  Leg
            wxString t;
            t.Printf( _T("%d"), i );
            if( i == 0 ) t = _T("---");
            if( arrival ) m_wpList->SetItem( item_line_index, 0, t );
/*
            if( arrival ){
                wxListItem item;
                item.SetId(i);
                item.SetColumn(0);
                item.SetText(t);
                m_wpList->SetItem(item);
            }
            
            m_wpList->SetColumnWidth( 0, wxLIST_AUTOSIZE );
            */
            
            //  Mark Name
            if( arrival ) m_wpList->SetItem( item_line_index, 1, prp->GetName() );
        // Store Dewcription
            if( arrival ) m_wpList->SetItem( item_line_index, 9, prp->GetDescription() );

            //  Distance
            //  Note that Distance/Bearing for Leg 000 is as from current position

            double brg, leg_dist;
            bool starting_point = false;

            starting_point = ( i == 0 ) && enroute;
            if( m_pEnroutePoint && !starting_point )
                starting_point = ( prp->m_GUID == m_pEnroutePoint->m_GUID );

            if( starting_point ) {
                slat = gLat;
                slon = gLon;
                if( gSog > 0.0 ) leg_speed = gSog; // should be VMG
                else
                    leg_speed = m_planspeed;
                if( m_bStartNow ) {
                    DistanceBearingMercator( prp->m_lat, prp->m_lon, slat, slon, &brg, &leg_dist );
                    joining_time = wxTimeSpan::Seconds( (long) wxRound( ( leg_dist * 3600. ) / leg_speed ) );
                }
                enroute = true;
            } else {
                if( prp->m_seg_vmg > 0. )
                    leg_speed = prp->m_seg_vmg;
                else
                    leg_speed = m_planspeed;
            }

            DistanceBearingMercator( prp->m_lat, prp->m_lon, slat, slon, &brg, &leg_dist );

        // calculation of course at current WayPoint.
        double course=10, tmp_leg_dist=23;
        wxRoutePointListNode *next_node = node->GetNext();
        RoutePoint * _next_prp = (next_node)? next_node->GetData(): NULL;
        if (_next_prp )
        {
        DistanceBearingMercator( _next_prp->m_lat, _next_prp->m_lon, prp->m_lat, prp->m_lon, &course, &tmp_leg_dist );
        }else
        {
          course = 0.0;
          tmp_leg_dist = 0.0;
        }

        prp->SetCourse(course); // save the course to the next waypoint for printing.
        // end of calculation


        t.Printf( _T("%6.2f ") + getUsrDistanceUnit(), toUsrDistance( leg_dist ) );
        if( arrival )
            m_wpList->SetItem( item_line_index, 2, t );
        if( !enroute )
            m_wpList->SetItem( item_line_index, 2, nullify );
        prp->SetDistance(leg_dist); // save the course to the next waypoint for printing.

            //  Bearing
        if( g_bShowMag ){
            double latAverage = (prp->m_lat + slat)/2;
            double lonAverage = (prp->m_lon + slon)/2;
            double varBrg = gFrame->GetTrueOrMag( brg, latAverage, lonAverage);
            
            t.Printf( _T("%03.0f Deg. M"), varBrg );
        }
        else
            t.Printf( _T("%03.0f Deg. T"), gFrame->GetTrueOrMag( brg ) );

        if( arrival )
            m_wpList->SetItem( item_line_index, 3, t );
        if( !enroute )
            m_wpList->SetItem( item_line_index, 3, nullify );

        // Course (bearing of next )
        if (_next_prp){
            if( g_bShowMag ){
                double next_lat = prp->m_lat;
                double next_lon = prp->m_lon;
                if (_next_prp ){
                    next_lat = _next_prp->m_lat;
                    next_lon = _next_prp->m_lon;
                }
                    
                double latAverage = (prp->m_lat + next_lat)/2;
                double lonAverage = (prp->m_lon + next_lon)/2;
                double varCourse = gFrame->GetTrueOrMag( course, latAverage, lonAverage);
                
                t.Printf( _T("%03.0f Deg. M"), varCourse );
            }
            else
                t.Printf( _T("%03.0f Deg. T"), gFrame->GetTrueOrMag( course ) );
            if( arrival )
                m_wpList->SetItem( item_line_index, 10, t );
        }
        else
            m_wpList->SetItem( item_line_index, 10, nullify );

            //  Lat/Lon
            wxString tlat = toSDMM( 1, prp->m_lat, false );  // low precision for routes
            if( arrival ) m_wpList->SetItem( item_line_index, 4, tlat );

            wxString tlon = toSDMM( 2, prp->m_lon, false );
            if( arrival ) m_wpList->SetItem( item_line_index, 5, tlon );


            tide_form = _T("");

            LMT_Offset = long( ( prp->m_lon ) * 3600. / 15. );

            // Time to each waypoint or creation date for tracks
            if( i == 0 && enroute ) {
                time_form.Printf( _("Start") );
                if( m_starttime.IsValid() ) {
                    wxDateTime act_starttime = m_starttime + joining_time;
                    time_form.Append( _T(": ") );

                    if( !arrival ) {
                        wxDateTime etd = prp->m_seg_etd;
                        if( etd.IsValid() && etd.IsLaterThan( m_starttime ) ) {
                            stopover_time += etd.Subtract( m_starttime );
                            act_starttime = prp->m_seg_etd;
                        }
                    }

                    int ds = getDaylightStatus( prp->m_lat, prp->m_lon, act_starttime );
                    wxString s = ts2s( act_starttime, tz_selection, (int) LMT_Offset,
                            DISPLAY_FORMAT );
                    time_form.Append( s );
                    time_form.Append( _T("   (") );
                    time_form.Append( GetDaylightString(ds) );
                    time_form.Append( _T(")") );

                    if( ptcmgr ) {
                        int jx = 0;
                        if( prp->GetName().Find( _T("@~~") ) != wxNOT_FOUND ) {
                            tide_form = prp->GetName().Mid( prp->GetName().Find( _T("@~~") ) + 3 );
                            jx = ptcmgr->GetStationIDXbyName( tide_form, prp->m_lat, prp->m_lon );
                        }
                        if( gpIDX || jx ) {
                            time_t tm = act_starttime.GetTicks();
                            tide_form = MakeTideInfo( jx, tm, tz_selection, LMT_Offset );
                        }
                    }
                }
                tdis = 0;
                tsec = 0.;
            } // end of route point 0
            else {
                if( arrival && enroute ) tdis += leg_dist;
                if( leg_speed ) {
                    if( arrival && enroute ) tsec += 3600 * leg_dist / leg_speed; // time in seconds to arrive here
                    wxTimeSpan time( 0, 0, (int) tsec, 0 );

                    if( m_starttime.IsValid() ) {

                        wxDateTime ueta = m_starttime;
                        ueta.Add( time + stopover_time + joining_time );

                        if( !arrival ) {
                            wxDateTime etd = prp->m_seg_etd;
                            if( etd.IsValid() && etd.IsLaterThan( ueta ) ) {
                                stopover_time += etd.Subtract( ueta );
                                ueta = prp->m_seg_etd;
                            }
                        }

                        int ds = getDaylightStatus( prp->m_lat, prp->m_lon, ueta );
                        time_form = ts2s( ueta, tz_selection, LMT_Offset, DISPLAY_FORMAT );
                        time_form.Append( _T("   (") );
                        time_form.Append( GetDaylightString(ds) );
                        time_form.Append( _T(")") );


                        if( ptcmgr ) {
                            int jx = 0;
                            if( prp->GetName().Find( _T("@~~") ) != wxNOT_FOUND ) {
                                tide_form = prp->GetName().Mid(
                                        prp->GetName().Find( _T("@~~") ) + 3 );
                                jx = ptcmgr->GetStationIDXbyName( tide_form, prp->m_lat, prp->m_lon );
                            }
                            if( gpIDX || jx ) {
                                time_t tm = ueta.GetTicks();
                                tide_form = MakeTideInfo( jx, tm, tz_selection, LMT_Offset );
                            }
                        }
                    } else {
                        if( tsec > 3600. * 24. ) time_form = time.Format( _T(" %D D  %H H  %M M") );
                        else
                            time_form = time.Format( _T(" %H H  %M M") );
                    }
                } // end if legspeed
            }  // end of repeatable route point

            if( enroute && ( arrival || m_starttime.IsValid() ) ) m_wpList->SetItem(
                    item_line_index, 6, time_form );
            else
                m_wpList->SetItem( item_line_index, 6, _T("----") );

            //Leg speed
            wxString s;
            s.Printf( _T("%5.2f"), toUsrSpeed( leg_speed ) );
            if( arrival ) m_wpList->SetItem( item_line_index, 7, s );

            if( enroute ) m_wpList->SetItem( item_line_index, 8, tide_form );
            else {
                m_wpList->SetItem( item_line_index, 7, nullify );
                m_wpList->SetItem( item_line_index, 8, nullify );
            }

            //  Save for iterating distance/bearing calculation
            slat = prp->m_lat;
            slon = prp->m_lon;

            // if stopover (ETD) found, loop for next output line for the same point
            //   with departure time & tide information

            if( arrival && ( prp->m_seg_etd.IsValid() ) ) {
                stopover_count++;
                arrival = false;
            } else {
                arrival = true;
                i++;
                node = node->GetNext();

                //    Record this point info for use as previous point in next iteration.
                i_prev_point = i - 1;
                prev_route_point = prp;
            }
        }
    }

    if( m_pRoute->m_Colour == wxEmptyString ) m_chColor->Select( 0 );
    else {
        for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
            if( m_pRoute->m_Colour == ::GpxxColorNames[i] ) {
                m_chColor->Select( i + 1 );
                break;
            }
        }
    }

    for( unsigned int i = 0; i < sizeof( ::StyleValues ) / sizeof(int); i++ ) {
        if( m_pRoute->m_style == ::StyleValues[i] ) {
            m_chStyle->Select( i );
            break;
        }
    }

    for( unsigned int i = 0; i < sizeof( ::WidthValues ) / sizeof(int); i++ ) {
        if( m_pRoute->m_width == ::WidthValues[i] ) {
            m_chWidth->Select( i );
            break;
        }
    }

    ::wxEndBusyCursor();

    m_wpList->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    
    return true;
}

wxString RouteProp::MakeTideInfo( int jx, time_t tm, int tz_selection, long LMT_Offset )
{
    int ev = 0;
    wxString tide_form;

    if( gpIDX ) {
        ev = ptcmgr->GetNextBigEvent( &tm,
                ptcmgr->GetStationIDXbyName( wxString( gpIDX->IDX_station_name, wxConvUTF8 ),
                        gpIDX->IDX_lat, gpIDX->IDX_lon ) );
    } else
        ev = ptcmgr->GetNextBigEvent( &tm, jx );

    wxDateTime dtm;
    dtm.Set( tm ).MakeUTC(); // apparently Set works as from LT
    if( ev == 1 ) tide_form.Printf( _T("LW: ") );
    if( ev == 2 ) tide_form.Printf( _T("HW: ") );
    tide_form.Append( ts2s( dtm, tz_selection, LMT_Offset, DISPLAY_FORMAT ) );
    if( !gpIDX ) {
        wxString locn( ptcmgr->GetIDX_entry( jx )->IDX_station_name, wxConvUTF8 );
        tide_form.Append( _T(" @~~") );
        tide_form.Append( locn );
    }
    return ( tide_form );
}

bool RouteProp::SaveChanges( void )
{

    //  Save the current planning speed
    g_PlanSpeed = m_planspeed;
    g_StartTime = m_starttime;    // both always UTC
    g_StartTimeTZ = GetTZSelection();
    m_StartTimeCtl->Clear();

    if( m_pRoute && !m_pRoute->m_bIsInLayer ) {
        //  Get User input Text Fields
        m_pRoute->m_RouteNameString = m_RouteNameCtl->GetValue();
        m_pRoute->m_RouteStartString = m_RouteStartCtl->GetValue();
        m_pRoute->m_RouteEndString = m_RouteDestCtl->GetValue();
        if( m_chColor->GetSelection() == 0 ) m_pRoute->m_Colour = wxEmptyString;
        else
            m_pRoute->m_Colour = ::GpxxColorNames[m_chColor->GetSelection() - 1];
        m_pRoute->m_style = (wxPenStyle)::StyleValues[m_chStyle->GetSelection()];
        m_pRoute->m_width = ::WidthValues[m_chWidth->GetSelection()];
        m_pRoute->m_PlannedDeparture = g_StartTime;
        m_pRoute->m_PlannedSpeed = m_planspeed;
        switch( g_StartTimeTZ ) {
            case 1 :
                m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_PC;
                break;
            case 2 :
                m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_LOCAL;
                break;
            default:
                m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_UTC;
        }

        pConfig->UpdateRoute( m_pRoute );
        pConfig->UpdateSettings();
    }

    if( m_pRoute && ( m_pRoute->IsActive() || ((Track*) m_pRoute)->IsRunning() ) )
    {
        wxJSONValue v;
        v[_T("Name")] =  m_pRoute->m_RouteNameString;
        v[_T("GUID")] =  m_pRoute->m_GUID;
        wxString msg_id( _T("OCPN_TRK_ACTIVATED") );
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
    }

    return true;
}

void RouteProp::OnPlanSpeedCtlUpdated( wxCommandEvent& event )
{
    //  Fetch the value, and see if it is a "reasonable" speed
    wxString spd = m_PlanSpeedCtl->GetValue();
    double s;
    spd.ToDouble( &s );
    if( ( 0.1 < s ) && ( s < 1000.0 ) ) {
        m_planspeed = fromUsrSpeed( s );

        UpdateProperties();
    }

    event.Skip();
}

void RouteProp::OnStartTimeCtlUpdated( wxCommandEvent& event )
{
    //  Fetch the value, and see if it is a "reasonable" time
    wxString stime = m_StartTimeCtl->GetValue();
    int tz_selection = GetTZSelection();

    wxDateTime d;
    if( stime.StartsWith( _T(">") ) ) {
        if( m_pRoute->m_bRtIsActive ) {
            m_pEnroutePoint = g_pRouteMan->GetpActivePoint();
        }
        m_bStartNow = true;
        d = wxDateTime::Now();
        if( tz_selection == 1 ) m_starttime = d.ToUTC();
        else
            m_starttime = wxInvalidDateTime; // can't get it to work otherwise
    } else {
        m_pEnroutePoint = NULL;
        m_bStartNow = false;
        if( !d.ParseDateTime( stime ) )        // only specific times accepted
        d = wxInvalidDateTime;

        m_starttime = d;

        if( m_starttime.IsValid() ) {
            if( tz_selection == 1 ) m_starttime = d.ToUTC();
            if( tz_selection == 2 ) {
                wxTimeSpan glmt( 0, 0, (int) gStart_LMT_Offset, 0 );
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

void RouteProp::OnRoutepropCancelClick( wxCommandEvent& event )
{
    //    Look in the route list to be sure the raoute is still available
    //    (May have been deleted by RouteMangerDialog...)

    bool b_found_route = false;
    wxRouteListNode *node = pRouteList->GetFirst();
    while( node ) {
        Route *proute = node->GetData();

        if( proute == m_pRoute ) {
            b_found_route = true;
            break;
        }
        node = node->GetNext();
    }

    if( b_found_route ) m_pRoute->ClearHighlights();

    m_bStartNow = false;

    #ifdef __WXGTK__ 
    gFrame->Raise();
    #endif
    Hide();
    cc1->Refresh( false );

    event.Skip();
}

void RouteProp::OnRoutepropOkClick( wxCommandEvent& event )
{
    //    Look in the route list to be sure the route is still available
    //    (May have been deleted by RouteManagerDialog...)

    bool b_found_route = false;
    wxRouteListNode *node = pRouteList->GetFirst();
    while( node ) {
        Route *proute = node->GetData();

        if( proute == m_pRoute ) {
            b_found_route = true;
            break;
        }
        node = node->GetNext();
    }

    if( b_found_route ) {
        SaveChanges();              // write changes to globals and update config
        m_pRoute->ClearHighlights();
    }

    m_pEnroutePoint = NULL;
    m_bStartNow = false;

    if( RouteManagerDialog::getInstanceFlag() && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateRouteListCtrl();

    #ifdef __WXGTK__ 
    gFrame->Raise();
    #endif
    Hide();
    cc1->InvalidateGL();
    cc1->Refresh( false );

    event.Skip();

}

void RouteProp::OnEvtColDragEnd( wxListEvent& event )
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
//------------------------------------------------------------------------------
//    LatLonTextCtrl Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(LatLonTextCtrl, wxWindow)
END_EVENT_TABLE()

// constructor
LatLonTextCtrl::LatLonTextCtrl( wxWindow* parent, wxWindowID id, const wxString& value,
        const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator,
        const wxString& name ) :
        wxTextCtrl( parent, id, value, pos, size, style, validator, name )
{
    m_pParentEventHandler = parent->GetEventHandler();
}

void LatLonTextCtrl::OnKillFocus( wxFocusEvent& event )
{
    //    Send an event to the Parent Dialog
    wxCommandEvent up_event( EVT_LLCHANGE, GetId() );
    up_event.SetEventObject( (wxObject *) this );
    m_pParentEventHandler->AddPendingEvent( up_event );
}

//-------------------------------------------------------------------------------
//
//    Mark Information Dialog Implementation
//
//-------------------------------------------------------------------------------
BEGIN_EVENT_TABLE( MarkInfoDef, wxDialog )
    EVT_CHECKBOX( ID_SHOWWAYPOINTRANGERINGS, MarkInfoDef::OnShowWaypointRangeRingSelect )
    EVT_CHOICE( ID_WAYPOINTRANGERINGS, MarkInfoDef::OnWaypointRangeRingSelect )
END_EVENT_TABLE()

MarkInfoDef::MarkInfoDef( wxWindow* parent, wxWindowID id, const wxString& title,
        const wxPoint& pos, const wxSize& size, long style )
{
    long wstyle = style;
#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif

    wxDialog::Create( parent, id, title, pos, size, wstyle );

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );
    int metric = GetCharHeight();
    
    #ifdef __OCPN__ANDROID__
    //  Set Dialog Font by custom crafted Qt Stylesheet.
    
    wxString wqs = getFontQtStylesheet(qFont);
    wxCharBuffer sbuf = wqs.ToUTF8();
    QString qsb = QString(sbuf.data());
    
    QString qsbq = getQtStyleSheet();           // basic scrollbars, etc
    
    this->GetHandle()->setStyleSheet( qsb + qsbq );      // Concatenated style sheets
    
    wxScreenDC sdc;
    if(sdc.IsOk())
        sdc.GetTextExtent(_T("W"), NULL, &metric, NULL, NULL, qFont);
    
    #endif
    
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer( wxVERTICAL );
    SetSizer( bSizer1 );
    bSizer1->SetSizeHints( this );   // set size hints to honour minimum size

    m_notebookProperties = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

    m_panelBasicProperties = new wxScrolledWindow( m_notebookProperties, wxID_ANY,
                                                   wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxTAB_TRAVERSAL);
    #ifdef __OCPN__ANDROID__
    m_panelBasicProperties->GetHandle()->setStyleSheet( getQtStyleSheet());
    #endif
    
    m_panelBasicProperties->SetScrollRate(0, 2);

    m_notebookProperties->AddPage( m_panelBasicProperties, _("Basic"), true );

//    wxBoxSizer* bSizerBasicProperties;
    bSizerBasicProperties = new wxBoxSizer( wxVERTICAL );
    m_panelBasicProperties->SetSizer( bSizerBasicProperties );

    wxStaticBoxSizer* sbSizerProperties;
    sbSizerProperties = new wxStaticBoxSizer(
            new wxStaticBox( m_panelBasicProperties, wxID_ANY, _("Properties") ), wxVERTICAL );

    wxBoxSizer* bSizerInnerProperties = new wxBoxSizer( wxHORIZONTAL );

//    m_bitmapIcon = new wxStaticBitmap( m_panelBasicProperties, wxID_ANY, wxNullBitmap,
//            wxDefaultPosition, wxDefaultSize, 0 );
//    bSizerInnerProperties->Add( m_bitmapIcon, 0, wxALL, 5 );
//    m_bitmapIcon->Show( false );
    
    wxBoxSizer* bSizerTextProperties;
    bSizerTextProperties = new wxBoxSizer( wxVERTICAL );

    m_staticTextLayer = new wxStaticText( m_panelBasicProperties, wxID_ANY,
            _("This waypoint is part of a layer and can't be edited"), wxDefaultPosition,
            wxDefaultSize, 0 );
    m_staticTextLayer->Enable( false );

    bSizerTextProperties->Add( m_staticTextLayer, 0, wxALL, 5 );

    wxBoxSizer* bSizerName;
    bSizerName = new wxBoxSizer( wxHORIZONTAL );

    m_staticTextName = new wxStaticText( m_panelBasicProperties, wxID_ANY, _("Name"),
            wxDefaultPosition, wxDefaultSize, 0 );
    //m_staticTextName->Wrap( -1 );
    bSizerName->Add( m_staticTextName, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

    wxBoxSizer* bSizerNameValue = new wxBoxSizer( wxVERTICAL );

    m_textName = new wxTextCtrl( m_panelBasicProperties, wxID_ANY, wxEmptyString, wxDefaultPosition,
            wxDefaultSize, 0 );
    bSizerNameValue->Add( m_textName, 0, wxALL | wxEXPAND, 5 );
    bSizerName->Add( bSizerNameValue, 1, wxEXPAND, 5 );
    bSizerTextProperties->Add( bSizerName, 0, wxEXPAND, 5 );

    m_checkBoxShowName = new wxCheckBox( m_panelBasicProperties, wxID_ANY, _("Show name"),
            wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    bSizerTextProperties->Add( m_checkBoxShowName, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

    ///
    wxBoxSizer* bSizer8 = new wxBoxSizer( wxHORIZONTAL );
    bSizerTextProperties->Add( bSizer8, 0, wxEXPAND, 5 );
    
    m_staticTextIcon = new wxStaticText( m_panelBasicProperties, wxID_ANY, _("Icon"),
            wxDefaultPosition, wxDefaultSize, 0 );
    bSizer8->Add( m_staticTextIcon, 0, wxALL, 5 );

    m_bcomboBoxIcon = new OCPNIconCombo( m_panelBasicProperties, wxID_ANY, _("Combo!"),
                                        wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
    
    m_bcomboBoxIcon->SetPopupMaxHeight(::wxGetDisplaySize().y / 2);
    
    //  Accomodate scaling of icon
    int min_size = metric * 2;
    min_size = wxMax( min_size, (32 *g_ChartScaleFactorExp) + 4 );
    m_bcomboBoxIcon->SetMinSize( wxSize(-1, min_size) );
    
    bSizer8->Add( m_bcomboBoxIcon, 1, wxALL, 5 );

    bSizerTextProperties->AddSpacer(5);
    
    wxFlexGridSizer *LLGrid = new wxFlexGridSizer( 0, 2, 1, 1 );
    LLGrid->AddGrowableCol( 1 );
    bSizerTextProperties->Add( LLGrid, 1, wxEXPAND, 0 );
    
    int w,h;
    GetTextExtent(_T("179 59.9999 W"), &w, &h);
    
    m_staticTextLatitude = new wxStaticText( m_panelBasicProperties, wxID_ANY, _("Latitude"));
    LLGrid->Add( m_staticTextLatitude, 0, wxALL | wxALIGN_LEFT, 0 );

    m_textLatitude = new wxTextCtrl( m_panelBasicProperties, wxID_ANY, wxEmptyString,
                                     wxDefaultPosition, wxSize(w + 20, -1), 0 );
    LLGrid->Add( m_textLatitude, 1, wxALL , 5 );

     m_staticTextLongitude = new wxStaticText( m_panelBasicProperties, wxID_ANY, _("Longitude"));
     LLGrid->Add( m_staticTextLongitude, 0, wxALL  | wxALIGN_LEFT, 0 );
 
     m_textLongitude = new wxTextCtrl( m_panelBasicProperties, wxID_ANY, wxEmptyString,
                                       wxDefaultPosition, wxSize(w + 20, -1), 0 );
     LLGrid->Add( m_textLongitude, 1, wxALL , 5 );

     
    wxBoxSizer* bSizerArrivalRadius;
    bSizerArrivalRadius = new wxBoxSizer( wxHORIZONTAL );
    m_staticTextArrivalRadius = new wxStaticText( m_panelBasicProperties, wxID_ANY, _("Arrival Radius"));
    bSizerArrivalRadius->Add( m_staticTextArrivalRadius, 0, wxALL, 0 );

    m_textArrivalRadius = new wxTextCtrl( m_panelBasicProperties, wxID_ANY, wxEmptyString);
    bSizerArrivalRadius->Add( m_textArrivalRadius, 0, wxALL | wxALIGN_RIGHT, 5 );
    bSizerTextProperties->Add( bSizerArrivalRadius, 0, wxEXPAND, 5 );

    //  Waypoints
    m_checkBoxShowWaypointRangeRings = new wxCheckBox( m_panelBasicProperties, ID_SHOWWAYPOINTRANGERINGS, _("Show Range Rings"),
            wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    bSizerTextProperties->Add( m_checkBoxShowWaypointRangeRings, 0, wxALL, 0 );

    waypointrrSelect = new wxFlexGridSizer( 1, 2, 1, 1 );
    waypointrrSelect->AddGrowableCol( 1 );
    bSizerTextProperties->Add( waypointrrSelect, 0, wxLEFT|wxRIGHT | wxEXPAND, 0 );

    wxStaticText *waypointrrTxt = new wxStaticText( m_panelBasicProperties, wxID_ANY, _("Range rings") );
    waypointrrSelect->Add( waypointrrTxt, 1, wxEXPAND | wxALL, 5 );
        
    wxString rrAlt[] = { _("None"), _T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9"), _T("10") };
    m_choiceWaypointRangeRingsNumber = new wxChoice( m_panelBasicProperties, ID_WAYPOINTRANGERINGS, wxDefaultPosition,
                                                     wxSize(250, -1), 11, rrAlt );
    waypointrrSelect->Add( m_choiceWaypointRangeRingsNumber, 0, wxALIGN_RIGHT | wxALL, 4 );

    waypointradarGrid = new wxFlexGridSizer( 0, 2, 1, 1 );
    waypointradarGrid->AddGrowableCol( 1 );
    bSizerTextProperties->Add( waypointradarGrid, 0, wxLEFT | wxRIGHT| wxEXPAND, 5 );

    wxStaticText* waypointdistanceText = new wxStaticText( m_panelBasicProperties, wxID_STATIC, _("Ring Spacing") );
    waypointradarGrid->Add( waypointdistanceText, 1, wxEXPAND | wxALL, 1 );

    m_textWaypointRangeRingsStep = new wxTextCtrl( m_panelBasicProperties, ID_TEXTCTRL, _T(""), wxDefaultPosition,
                                                   wxSize( 100, -1 ), 0 );
    waypointradarGrid->Add( m_textWaypointRangeRingsStep, 0, wxALIGN_RIGHT | wxALL, 4 );

    wxStaticText* waypointunitText = new wxStaticText( m_panelBasicProperties, wxID_STATIC, _("Distance Unit") );
    waypointradarGrid->Add( waypointunitText, 1, wxEXPAND | wxALL, 1 );

    wxString pDistUnitsStrings[] = { _T("Nautical Miles"), _T("Kilometers") };
    m_choiceWaypointRangeRingsUnits = new wxChoice( m_panelBasicProperties, wxID_ANY, wxDefaultPosition,
                                                    wxSize(250, -1), 2, pDistUnitsStrings );
    waypointradarGrid->Add( m_choiceWaypointRangeRingsUnits, 0, wxALIGN_RIGHT | wxALL, 4 );

        
    wxString cText = _("Ring Colour");
    wxSize cSize = wxSize(metric * 4, 40);
    
    wxStaticText* waypointrangeringsColour = new wxStaticText( m_panelBasicProperties, wxID_STATIC, cText );
    waypointradarGrid->Add( waypointrangeringsColour, 1, wxEXPAND | wxALL, 1 );
    
    
    wxString chColorChoices[] = { _("Default color"), _("Black"), _("Dark Red"), _("Dark Green"),
                                    _("Dark Yellow"), _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
                                    _("Light Gray"), _("Dark Gray"), _("Red"), _("Green"), _("Yellow"), _("Blue"),
                                    _("Magenta"), _("Cyan"), _("White") };
    int chColorNChoices = sizeof( chColorChoices ) / sizeof(wxString);
    m_chColor = new wxChoice( m_panelBasicProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              chColorNChoices,  chColorChoices, 0 );
    m_chColor->SetSelection( 0 );
    waypointradarGrid->Add( m_chColor, 0, wxALIGN_RIGHT | wxALL, 4);
    
    bSizerTextProperties->AddSpacer(15);
    
    m_staticTextDescription = new wxStaticText( m_panelBasicProperties, wxID_ANY, _("Description"),
            wxDefaultPosition, wxDefaultSize, 0 );
    bSizerTextProperties->Add( m_staticTextDescription, 0, wxALL, 5 );

    wxBoxSizer* bSizer14;
    bSizer14 = new wxBoxSizer( wxHORIZONTAL );

    m_textDescription = new wxTextCtrl( m_panelBasicProperties, wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    m_textDescription->SetMinSize( wxSize( -1, 60 ) );

    bSizer14->Add( m_textDescription, 1, wxALL | wxEXPAND, 5 );

    m_buttonExtDescription = new wxButton( m_panelBasicProperties, wxID_ANY, _("..."),
            wxDefaultPosition, wxSize( 20, -1 ), 0 );
    bSizer14->Add( m_buttonExtDescription, 0, wxALL | wxEXPAND, 5 );

    bSizerTextProperties->Add( bSizer14, 1, wxEXPAND, 5 );

    bSizerInnerProperties->Add( bSizerTextProperties, 1, wxEXPAND, 5 );

    sbSizerProperties->Add( bSizerInnerProperties, 1, wxEXPAND, 5 );

    bSizerBasicProperties->Add( sbSizerProperties, 3, wxALL | wxEXPAND, 5 );

    sbSizerLinks = new wxStaticBoxSizer( new wxStaticBox( m_panelBasicProperties, wxID_ANY, _("Links") ), wxVERTICAL );
    bSizerBasicProperties->Add( sbSizerLinks, 2, wxALL | wxEXPAND, 5 );
    
    m_scrolledWindowLinks = new wxScrolledWindow( m_panelBasicProperties, wxID_ANY,
            wxDefaultPosition, wxSize(-1, 60), wxHSCROLL | wxVSCROLL );
    m_scrolledWindowLinks->SetScrollRate( 2, 2 );
    sbSizerLinks->Add( m_scrolledWindowLinks, 0, wxEXPAND | wxALL, 5 );
    
    bSizerLinks = new wxBoxSizer( wxVERTICAL );
    m_scrolledWindowLinks->SetSizer( bSizerLinks );
    
    m_hyperlink17 = new wxHyperlinkCtrl( m_scrolledWindowLinks, wxID_ANY, _("wxFB Website"),
            wxT("file:///C:\\ProgramData\\opencpn\\opencpn.log"), wxDefaultPosition,
            wxDefaultSize, wxHL_DEFAULT_STYLE );

    m_menuLink = new wxMenu();
    wxMenuItem* m_menuItemDelete;
    m_menuItemDelete = new wxMenuItem( m_menuLink, wxID_ANY, wxString( _("Delete") ), wxEmptyString,
            wxITEM_NORMAL );
    m_menuLink->Append( m_menuItemDelete );

    wxMenuItem* m_menuItemEdit;
    m_menuItemEdit = new wxMenuItem( m_menuLink, wxID_ANY, wxString( _("Edit") ), wxEmptyString,
            wxITEM_NORMAL );
    m_menuLink->Append( m_menuItemEdit );

    wxMenuItem* m_menuItemAdd;
    m_menuItemAdd = new wxMenuItem( m_menuLink, wxID_ANY, wxString( _("Add new") ), wxEmptyString,
            wxITEM_NORMAL );
    m_menuLink->Append( m_menuItemAdd );

    m_hyperlink17->Connect( wxEVT_RIGHT_DOWN,
            wxMouseEventHandler( MarkInfoDef::m_hyperlink17OnContextMenu ), NULL, this );

    bSizerLinks->Add( m_hyperlink17, 0, wxALL, 5 );

    wxBoxSizer* bSizer9 = new wxBoxSizer( wxHORIZONTAL );

    m_buttonAddLink = new wxButton( m_panelBasicProperties, wxID_ANY, _("Add"), wxDefaultPosition,
            wxDefaultSize, wxBU_EXACTFIT );
    bSizer9->Add( m_buttonAddLink, 0, wxALL, 5 );

    m_toggleBtnEdit = new wxToggleButton( m_panelBasicProperties, wxID_ANY, _("Edit"),
            wxDefaultPosition, wxDefaultSize, 0 );
    bSizer9->Add( m_toggleBtnEdit, 0, wxALL, 5 );


    sbSizerLinks->Add( bSizer9, 0, wxEXPAND, 5 );

    m_staticTextEditEnabled = new wxStaticText( m_panelBasicProperties, wxID_ANY,
                                                _("Links are opened in the default browser."), wxDefaultPosition, wxDefaultSize,
                                                0 );
    sbSizerLinks->Add( m_staticTextEditEnabled, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
    

    m_panelDescription = new wxPanel( m_notebookProperties, wxID_ANY, wxDefaultPosition,
            wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* bSizer15;
    bSizer15 = new wxBoxSizer( wxVERTICAL );

    m_textCtrlExtDescription = new wxTextCtrl( m_panelDescription, wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    bSizer15->Add( m_textCtrlExtDescription, 1, wxALL | wxEXPAND, 5 );

    m_panelDescription->SetSizer( bSizer15 );
    m_notebookProperties->AddPage( m_panelDescription, _("Description"), false );
    m_panelExtendedProperties = new wxPanel( m_notebookProperties, wxID_ANY, wxDefaultPosition,
            wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* bSizerExtendedProperties;
    bSizerExtendedProperties = new wxBoxSizer( wxVERTICAL );

    m_checkBoxVisible = new wxCheckBox( m_panelExtendedProperties, wxID_ANY, _("Show on chart"),
            wxDefaultPosition, wxDefaultSize, 0 );
    bSizerExtendedProperties->Add( m_checkBoxVisible, 0, wxALL, 5 );

    wxBoxSizer* bSizerGuid;
    bSizerGuid = new wxBoxSizer( wxHORIZONTAL );

    m_staticTextGuid = new wxStaticText( m_panelExtendedProperties, wxID_ANY, _("GUID"),
            wxDefaultPosition, wxDefaultSize, 0 );
    bSizerGuid->Add( m_staticTextGuid, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

    m_textCtrlGuid = new wxTextCtrl( m_panelExtendedProperties, wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    bSizerGuid->Add( m_textCtrlGuid, 1, wxALL | wxEXPAND, 5 );

    bSizerExtendedProperties->Add( bSizerGuid, 0, wxEXPAND, 5 );

    m_staticTextGpx = new wxStaticText( m_panelExtendedProperties, wxID_ANY, _("GPX"),
            wxDefaultPosition, wxDefaultSize, 0 );
    m_staticTextGpx->Enable( false );

    bSizerExtendedProperties->Add( m_staticTextGpx, 0, wxALL, 5 );

    m_textCtrlGpx = new wxTextCtrl( m_panelExtendedProperties, wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY );
    m_textCtrlGpx->Enable( false );

    bSizerExtendedProperties->Add( m_textCtrlGpx, 1, wxALL | wxEXPAND, 5 );

    m_panelExtendedProperties->SetSizer( bSizerExtendedProperties );
    m_notebookProperties->AddPage( m_panelExtendedProperties, _("Extended"), false );

    bSizer1->Add( m_notebookProperties, 1, wxEXPAND | wxALL, 5 );

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
    bSizer1->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 3 );
    
    m_sdbSizerButtonsCancel = new wxButton( this, ID_MARKPROP_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_sdbSizerButtonsCancel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1 );
    m_sdbSizerButtonsOK = new wxButton( this, ID_MARKPROP_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add( m_sdbSizerButtonsOK, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 1);

    m_sdbSizerButtonsOK->SetDefault();

    Fit();

    SetMinSize(wxSize(-1, 600));
    RecalculateSize();
    
    // Connect Events
    m_textLatitude->Connect( wxEVT_COMMAND_TEXT_ENTER,
            wxCommandEventHandler( MarkInfoDef::OnPositionCtlUpdated ), NULL, this );
    m_textLongitude->Connect( wxEVT_COMMAND_TEXT_ENTER,
            wxCommandEventHandler( MarkInfoDef::OnPositionCtlUpdated ), NULL, this );

    m_textLatitude->Connect( wxEVT_CONTEXT_MENU,
            wxCommandEventHandler( MarkInfoImpl::OnRightClick ), NULL, this );
    m_textLongitude->Connect( wxEVT_CONTEXT_MENU,
            wxCommandEventHandler( MarkInfoImpl::OnRightClick ), NULL, this );
    m_textArrivalRadius->Connect( wxEVT_COMMAND_TEXT_ENTER,
            wxCommandEventHandler( MarkInfoDef::OnArrivalRadiusChange ), NULL, this );
    m_textWaypointRangeRingsStep->Connect( wxEVT_COMMAND_TEXT_ENTER,
            wxCommandEventHandler( MarkInfoDef::OnWaypointRangeRingsStepChange ), NULL, this );

    m_textDescription->Connect( wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler( MarkInfoDef::OnDescChangedBasic ), NULL, this );
    m_buttonExtDescription->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnExtDescriptionClick ), NULL, this );

    this->Connect( m_menuItemDelete->GetId(), wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnDeleteLink ) );
    this->Connect( m_menuItemEdit->GetId(), wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnEditLink ) );
    this->Connect( m_menuItemAdd->GetId(), wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnAddLink ) );
    this->Connect( ID_RCLK_MENU_COPY, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnCopyPasteLatLon ) );
    this->Connect( ID_RCLK_MENU_COPY_LL, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnCopyPasteLatLon ) );
    this->Connect( ID_RCLK_MENU_PASTE, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnCopyPasteLatLon ) );
    this->Connect( ID_RCLK_MENU_PASTE_LL, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnCopyPasteLatLon ) );

    m_buttonAddLink->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnAddLink ), NULL, this );
    m_toggleBtnEdit->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnEditLinkToggle ), NULL, this );
    m_textCtrlExtDescription->Connect( wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler( MarkInfoDef::OnDescChangedExt ), NULL, this );
    m_sdbSizerButtonsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnMarkInfoCancelClick ), NULL, this );
    m_sdbSizerButtonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnMarkInfoOKClick ), NULL, this );
}

void MarkInfoDef::RecalculateSize( void )
{
    
    Layout();

    // We change only Y size, unless X is too big for the parent client size....
    
    wxSize esize;

    esize.x = -1;
    esize.y = GetCharHeight() * 30;
    
    wxSize dsize = GetParent()->GetClientSize();
    esize.y = wxMin(esize.y, dsize.y - (2 * GetCharHeight()));
    esize.x = wxMin(esize.x, dsize.x - (1 * GetCharHeight()));
    SetSize(wxSize(esize.x, esize.y));
    
    wxSize fsize = GetSize();
    fsize.y = wxMin(fsize.y, dsize.y - (2 * GetCharHeight()));
    fsize.x = wxMin(fsize.x, dsize.x - (1 * GetCharHeight()));
    SetSize(wxSize(-1, fsize.y));
    
    m_defaultClientSize = GetClientSize();
    
}


void MarkInfoDef::OnShowWaypointRangeRingSelect( wxCommandEvent& event )
{
    if( m_checkBoxShowWaypointRangeRings->GetValue() == FALSE ) {
        waypointrrSelect->ShowItems( false );
        waypointradarGrid->ShowItems( false );
    } else {
        waypointrrSelect->ShowItems( true );
        if ( m_choiceWaypointRangeRingsNumber->GetSelection() != 0 ) 
            waypointradarGrid->ShowItems( true );
    }
    bSizerBasicProperties->Layout();
    m_panelBasicProperties->FitInside();
    Refresh();
    event.Skip();
}

void MarkInfoDef::OnWaypointRangeRingSelect( wxCommandEvent& event )
{
    if( m_choiceWaypointRangeRingsNumber->GetSelection() == 0 ) {
        waypointradarGrid->ShowItems( false );
    } else {
        waypointradarGrid->ShowItems( true );
    }
    bSizerBasicProperties->Layout();
    m_panelBasicProperties->FitInside();
    Refresh();
    event.Skip();
}

MarkInfoDef::~MarkInfoDef()
{
    // Disconnect Events
    m_textLatitude->Disconnect( wxEVT_COMMAND_TEXT_ENTER,
            wxCommandEventHandler( MarkInfoDef::OnPositionCtlUpdated ), NULL, this );
    m_textLongitude->Disconnect( wxEVT_COMMAND_TEXT_ENTER,
            wxCommandEventHandler( MarkInfoDef::OnPositionCtlUpdated ), NULL, this );
    m_textDescription->Disconnect( wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler( MarkInfoDef::OnDescChangedBasic ), NULL, this );
    m_textArrivalRadius->Disconnect( wxEVT_COMMAND_TEXT_ENTER,
            wxCommandEventHandler( MarkInfoDef::OnArrivalRadiusChange ), NULL, this );
    m_textWaypointRangeRingsStep->Disconnect( wxEVT_COMMAND_TEXT_ENTER,
            wxCommandEventHandler( MarkInfoDef::OnWaypointRangeRingsStepChange ), NULL, this );
    m_buttonExtDescription->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnExtDescriptionClick ), NULL, this );
    this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnDeleteLink ) );
    this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnEditLink ) );
    this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( MarkInfoDef::OnAddLink ) );
    m_buttonAddLink->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnAddLink ), NULL, this );
    m_toggleBtnEdit->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnEditLinkToggle ), NULL, this );
    m_textCtrlExtDescription->Disconnect( wxEVT_COMMAND_TEXT_UPDATED,
            wxCommandEventHandler( MarkInfoDef::OnDescChangedExt ), NULL, this );
    m_sdbSizerButtonsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnMarkInfoCancelClick ), NULL, this );
    m_sdbSizerButtonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( MarkInfoDef::OnMarkInfoOKClick ), NULL, this );

    delete m_menuLink;
    
#ifdef __OCPN__ANDROID__
    androidEnableBackButton( true );
#endif
    
}

bool MarkInfoImpl::instanceFlag = false;
MarkInfoImpl* MarkInfoImpl::single = NULL;

MarkInfoImpl *MarkInfoImpl::getInstance( wxWindow* parent, wxWindowID id,
                                  const wxString& title, const wxPoint& pos, const wxSize& size,
                                  long style)
{
    if(! instanceFlag)
    {
        single = new MarkInfoImpl( parent, id, title, pos, size,style);
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

MarkInfoImpl::MarkInfoImpl( wxWindow* parent, wxWindowID id, const wxString& title,
        const wxPoint& pos, const wxSize& size, long style ) :
        MarkInfoDef( parent, id, title, pos, size, style )
{
    m_pLinkProp = new LinkPropImpl( NULL );
    m_pMyLinkList = NULL;
    m_staticTextGpx->Show( false );
    m_textCtrlGpx->Show( false );
    SetColorScheme( (ColorScheme) 0 );
}

MarkInfoImpl::~MarkInfoImpl()
{
    m_bcomboBoxIcon->Clear();
    m_bcomboBoxIcon->Destroy();
    m_pLinkProp->Destroy();
    instanceFlag = false;
}

void MarkInfoImpl::InitialFocus( void )
{
    m_textName->SetFocus();
    m_textName->SetInsertionPointEnd();
#ifndef __OCPN__ANDROID__    
    m_panelBasicProperties->Scroll(0, 0);
#endif
}

void MarkInfoImpl::SetColorScheme( ColorScheme cs )
{
    DimeControl( this );

    DimeControl( m_pLinkProp );
}

bool MarkInfoImpl::UpdateProperties( bool positionOnly )
{
    if( m_pRoutePoint ) {

        m_textLatitude->SetValue( ::toSDMM( 1, m_pRoutePoint->m_lat ) );
        m_textLongitude->SetValue( ::toSDMM( 2, m_pRoutePoint->m_lon ) );
        m_lat_save = m_pRoutePoint->m_lat;
        m_lon_save = m_pRoutePoint->m_lon;

        if( positionOnly ) return true;

        //Layer or not?
        if( m_pRoutePoint->m_bIsInLayer ) {
            m_staticTextLayer->Enable();
            m_staticTextLayer->Show( true );
            m_textName->SetEditable( false );
            m_textDescription->SetEditable( false );
            m_textCtrlExtDescription->SetEditable( false );
            m_textLatitude->SetEditable( false );
            m_textLongitude->SetEditable( false );
            m_bcomboBoxIcon->Enable( false );
            m_buttonAddLink->Enable( false );
            m_toggleBtnEdit->Enable( false );
            m_toggleBtnEdit->SetValue( false );
            m_checkBoxShowName->Enable( false );
            m_checkBoxVisible->Enable( false );
            m_textArrivalRadius->SetEditable ( false );
            m_checkBoxShowWaypointRangeRings->Enable( false );
            m_choiceWaypointRangeRingsUnits->Enable( false );
            m_choiceWaypointRangeRingsNumber->Enable( false );
            m_textWaypointRangeRingsStep->SetEditable( false );
            m_chColor->Enable( false );
        } else {
            m_staticTextLayer->Enable( false );
            m_staticTextLayer->Show( false );
            m_textName->SetEditable( true );
            m_textDescription->SetEditable( true );
            m_textCtrlExtDescription->SetEditable( true );
            m_textLatitude->SetEditable( true );
            m_textLongitude->SetEditable( true );
            m_bcomboBoxIcon->Enable( true );
            m_buttonAddLink->Enable( true );
            m_toggleBtnEdit->Enable( true );
            m_checkBoxShowName->Enable( true );
            m_checkBoxVisible->Enable( true );
            m_textArrivalRadius->SetEditable ( true );
            m_checkBoxShowWaypointRangeRings->Enable( true );
            m_choiceWaypointRangeRingsUnits->Enable( true );
            m_choiceWaypointRangeRingsNumber->Enable( true );
            m_textWaypointRangeRingsStep->SetEditable( true );
            m_chColor->Enable( true );
        }
        m_textName->SetValue( m_pRoutePoint->GetName() );

        wxString s_ArrivalRadius;
        s_ArrivalRadius.Printf( _T("%.3f"), m_pRoutePoint->GetWaypointArrivalRadius() );
        m_textArrivalRadius->SetValue( s_ArrivalRadius );        
        
        m_textDescription->SetValue( m_pRoutePoint->m_MarkDescription );
        m_textCtrlExtDescription->SetValue( m_pRoutePoint->m_MarkDescription );
//        m_bitmapIcon->SetBitmap( *m_pRoutePoint->GetIconBitmap() );
        wxWindowList kids = m_scrolledWindowLinks->GetChildren();
        for( unsigned int i = 0; i < kids.GetCount(); i++ ) {
            wxWindowListNode *node = kids.Item( i );
            wxWindow *win = node->GetData();

            if( win->IsKindOf( CLASSINFO(wxHyperlinkCtrl) ) ) {
                ( (wxHyperlinkCtrl*) win )->Disconnect( wxEVT_COMMAND_HYPERLINK,
                        wxHyperlinkEventHandler( MarkInfoImpl::OnHyperLinkClick ) );
                ( (wxHyperlinkCtrl*) win )->Disconnect( wxEVT_RIGHT_DOWN,
                        wxMouseEventHandler( MarkInfoImpl::m_hyperlinkContextMenu ) );
                win->Destroy();
            }
        }
        m_checkBoxShowName->SetValue( m_pRoutePoint->m_bShowName );
        m_checkBoxVisible->SetValue( m_pRoutePoint->m_bIsVisible );
        m_textCtrlGuid->SetValue( m_pRoutePoint->m_GUID );
        m_checkBoxShowWaypointRangeRings->SetValue( m_pRoutePoint->GetShowWaypointRangeRings() );
        m_choiceWaypointRangeRingsNumber->SetSelection( m_pRoutePoint->GetWaypointRangeRingsNumber() );
        m_choiceWaypointRangeRingsUnits->SetSelection( m_pRoutePoint->GetWaypointRangeRingsStepUnits() );
        wxString buf;
        buf.Printf( _T("%.3f" ), m_pRoutePoint->GetWaypointRangeRingsStep() );
        m_textWaypointRangeRingsStep->SetValue( buf );
        
        wxColour col = m_pRoutePoint->m_wxcWaypointRangeRingsColour;
        wxString sColour = _("Default");
        
        for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
            if( col == ::GpxxColors[i] ) {
                sColour = ::GpxxColorNames[i];
                break;
            }
        }
        
        m_chColor->SetStringSelection(sColour);
        
        wxCommandEvent eDummy;
        OnShowWaypointRangeRingSelect( eDummy );
        

        int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
        HyperlinkList *hyperlinklist = m_pRoutePoint->m_HyperlinkList;
//            int len = 0;
        if( NbrOfLinks > 0 ) {
            wxHyperlinkListNode *linknode = hyperlinklist->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();
                wxString Link = link->Link;
                wxString Descr = link->DescrText;

                wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl( m_scrolledWindowLinks, wxID_ANY, Descr,
                        Link, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
                ctrl->Connect( wxEVT_COMMAND_HYPERLINK,
                        wxHyperlinkEventHandler( MarkInfoImpl::OnHyperLinkClick ), NULL, this );
                if( !m_pRoutePoint->m_bIsInLayer ) ctrl->Connect( wxEVT_RIGHT_DOWN,
                        wxMouseEventHandler( MarkInfoImpl::m_hyperlinkContextMenu ), NULL, this );

                bSizerLinks->Add( ctrl, 0, wxALL, 5 );

                linknode = linknode->GetNext();
            }
        }
        
        // Integrate all of the rebuilt hyperlink controls
        bSizerLinks->Layout();
        
        m_bcomboBoxIcon->Clear();
        //      Iterate on the Icon Descriptions, filling in the combo control
        bool fillCombo = m_bcomboBoxIcon->GetCount() == 0;
        wxImageList *icons = pWayPointMan->Getpmarkicon_image_list();

        int target = 16;
        if( fillCombo  && icons){
            for( int i = 0; i < pWayPointMan->GetNumIcons(); i++ ) {
                wxString *ps = pWayPointMan->GetIconDescription( i );
                wxBitmap bmp = icons->GetBitmap( i );

                if(g_ChartScaleFactorExp > 1.0){
                    target = bmp.GetHeight() * g_ChartScaleFactorExp;
                    wxImage img = bmp.ConvertToImage();
                    img.Rescale(target, target, wxIMAGE_QUALITY_HIGH);
                    bmp = wxBitmap(img);
                }
                
                m_bcomboBoxIcon->Append( *ps, bmp );
            }
        }
        
        // find the correct item in the combo box
        int iconToSelect = -1;
        for( int i = 0; i < pWayPointMan->GetNumIcons(); i++ ) {
            if( *pWayPointMan->GetIconKey( i ) == m_pRoutePoint->GetIconName() )
                iconToSelect = i;
        }

        //  not found, so add  it to the list, with a generic bitmap and using the name as description
        // n.b.  This should never happen...
        if( icons && -1 == iconToSelect){
            m_bcomboBoxIcon->Append( m_pRoutePoint->GetIconName(), icons->GetBitmap( 0 ) );
            iconToSelect = m_bcomboBoxIcon->GetCount() - 1;
        }
        
        
        m_bcomboBoxIcon->Select( iconToSelect );
        icons = NULL;
    }

    #ifdef __OCPN__ANDROID__
    androidEnableBackButton( false );
    #endif
    
    Fit();
    SetMinSize(wxSize(-1, 600));
    RecalculateSize();
    
    return true;
}

void MarkInfoImpl::SetRoutePoint( RoutePoint *pRP )
{
    m_pRoutePoint = pRP;
    if( m_pRoutePoint ) {
        m_lat_save = m_pRoutePoint->m_lat;
        m_lon_save = m_pRoutePoint->m_lon;
        m_IconName_save = m_pRoutePoint->GetIconName();
        m_bShowName_save = m_pRoutePoint->m_bShowName;
        m_bIsVisible_save = m_pRoutePoint->m_bIsVisible;
        if( m_pMyLinkList )
            delete m_pMyLinkList;
        m_pMyLinkList = new HyperlinkList();
        int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
//            int len = 0;
        if( NbrOfLinks > 0 ) {
            wxHyperlinkListNode *linknode = m_pRoutePoint->m_HyperlinkList->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();

                Hyperlink* h = new Hyperlink();
                h->DescrText = link->DescrText;
                h->Link = link->Link;
                h->LType = link->LType;

                m_pMyLinkList->Append( h );

                linknode = linknode->GetNext();
            }
        }
    }
}

void MarkInfoImpl::m_hyperlinkContextMenu( wxMouseEvent &event )
{
    m_pEditedLink = (wxHyperlinkCtrl*) event.GetEventObject();
    m_scrolledWindowLinks->PopupMenu( m_menuLink,
            m_pEditedLink->GetPosition().x + event.GetPosition().x,
            m_pEditedLink->GetPosition().y + event.GetPosition().y );
   
}

void MarkInfoImpl::OnDeleteLink( wxCommandEvent& event )
{
    wxHyperlinkListNode* nodeToDelete = NULL;
    wxString findurl = m_pEditedLink->GetURL();
    wxString findlabel = m_pEditedLink->GetLabel();
    
    wxWindowList kids = m_scrolledWindowLinks->GetChildren();
    for( unsigned int i = 0; i < kids.GetCount(); i++ ) {
        wxWindowListNode *node = kids.Item( i );
        wxWindow *win = node->GetData();
        win->Hide();    
    }
    
//    m_scrolledWindowLinks->DestroyChildren();
    int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
    HyperlinkList *hyperlinklist = m_pRoutePoint->m_HyperlinkList;
//      int len = 0;
    if( NbrOfLinks > 0 ) {
        wxHyperlinkListNode *linknode = hyperlinklist->GetFirst();
        while( linknode ) {
            Hyperlink *link = linknode->GetData();
            wxString Link = link->Link;
            wxString Descr = link->DescrText;
            if( Link == findurl
                    && ( Descr == findlabel || ( Link == findlabel && Descr == wxEmptyString ) ) ) nodeToDelete =
                    linknode;
            else {
                wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl( m_scrolledWindowLinks, wxID_ANY, Descr,
                        Link, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
                ctrl->Connect( wxEVT_COMMAND_HYPERLINK,
                        wxHyperlinkEventHandler( MarkInfoImpl::OnHyperLinkClick ), NULL, this );
                ctrl->Connect( wxEVT_RIGHT_DOWN,
                        wxMouseEventHandler( MarkInfoImpl::m_hyperlinkContextMenu ), NULL, this );

                bSizerLinks->Add( ctrl, 0, wxALL, 5 );
            }
            linknode = linknode->GetNext();
        }
    }
    if( nodeToDelete ) hyperlinklist->DeleteNode( nodeToDelete );
    m_scrolledWindowLinks->InvalidateBestSize();
    m_scrolledWindowLinks->Layout();
    sbSizerLinks->Layout();
    event.Skip();
}

void MarkInfoImpl::OnEditLink( wxCommandEvent& event )
{
    wxString findurl = m_pEditedLink->GetURL();
    wxString findlabel = m_pEditedLink->GetLabel();
    m_pLinkProp->m_textCtrlLinkDescription->SetValue( findlabel );
    m_pLinkProp->m_textCtrlLinkUrl->SetValue( findurl );
    
#ifdef __WXOSX__
    HideWithEffect(wxSHOW_EFFECT_BLEND );
#endif
    
    if( m_pLinkProp->ShowModal() == wxID_OK ) {
        int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
        HyperlinkList *hyperlinklist = m_pRoutePoint->m_HyperlinkList;
//            int len = 0;
        if( NbrOfLinks > 0 ) {
            wxHyperlinkListNode *linknode = hyperlinklist->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();
                wxString Link = link->Link;
                wxString Descr = link->DescrText;
                if( Link == findurl
                        && ( Descr == findlabel || ( Link == findlabel && Descr == wxEmptyString ) ) ) {
                    link->Link = m_pLinkProp->m_textCtrlLinkUrl->GetValue();
                    link->DescrText = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
                    wxHyperlinkCtrl* h =
                            (wxHyperlinkCtrl*) m_scrolledWindowLinks->FindWindowByLabel(
                                    findlabel );
                    if( h ) {
                        h->SetLabel( m_pLinkProp->m_textCtrlLinkDescription->GetValue() );
                        h->SetURL( m_pLinkProp->m_textCtrlLinkUrl->GetValue() );
                    }
                }
                linknode = linknode->GetNext();
            }
        }

        m_scrolledWindowLinks->InvalidateBestSize();
        m_scrolledWindowLinks->Layout();
        sbSizerLinks->Layout();
        event.Skip();
    }
    
#ifdef __WXOSX__
    ShowWithEffect(wxSHOW_EFFECT_BLEND );
#endif
    
    event.Skip();
}

void MarkInfoImpl::OnAddLink( wxCommandEvent& event )
{
    m_pLinkProp->m_textCtrlLinkDescription->SetValue( wxEmptyString );
    m_pLinkProp->m_textCtrlLinkUrl->SetValue( wxEmptyString );

#ifdef __WXOSX__
    HideWithEffect(wxSHOW_EFFECT_BLEND );
#endif
    
    if( m_pLinkProp->ShowModal() == wxID_OK ) {
        wxString desc = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
        if( desc == wxEmptyString ) desc = m_pLinkProp->m_textCtrlLinkUrl->GetValue();
        wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl( m_scrolledWindowLinks, wxID_ANY, desc,
                m_pLinkProp->m_textCtrlLinkUrl->GetValue(), wxDefaultPosition, wxDefaultSize,
                wxHL_DEFAULT_STYLE );
        ctrl->Connect( wxEVT_COMMAND_HYPERLINK,
                wxHyperlinkEventHandler( MarkInfoImpl::OnHyperLinkClick ), NULL, this );
        ctrl->Connect( wxEVT_RIGHT_DOWN,
                wxMouseEventHandler( MarkInfoImpl::m_hyperlinkContextMenu ), NULL, this );

        bSizerLinks->Add( ctrl, 0, wxALL, 5 );
        bSizerLinks->Fit( m_scrolledWindowLinks );

        Hyperlink* h = new Hyperlink();
        h->DescrText = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
        h->Link = m_pLinkProp->m_textCtrlLinkUrl->GetValue();
        h->LType = wxEmptyString;
        m_pRoutePoint->m_HyperlinkList->Append( h );
    }

#ifdef __WXOSX__
    ShowWithEffect(wxSHOW_EFFECT_BLEND );
#endif
    
    sbSizerLinks->Layout();

    event.Skip();
}

void MarkInfoImpl::OnEditLinkToggle( wxCommandEvent& event )
{
    if( m_toggleBtnEdit->GetValue() ) m_staticTextEditEnabled->SetLabel(
            _("Links are opened for editing.") );
    else
        m_staticTextEditEnabled->SetLabel( _("Links are opened in the default browser.") );
    event.Skip();
}

void MarkInfoImpl::OnDescChangedBasic( wxCommandEvent& event )
{
    if( m_panelBasicProperties->IsShownOnScreen() ) m_textCtrlExtDescription->ChangeValue(
            m_textDescription->GetValue() );
    event.Skip();
}

void MarkInfoImpl::OnDescChangedExt( wxCommandEvent& event )
{
    if( m_panelDescription->IsShownOnScreen() ) m_textDescription->ChangeValue(
            m_textCtrlExtDescription->GetValue() );
    event.Skip();
}

void MarkInfoImpl::OnExtDescriptionClick( wxCommandEvent& event )
{
    m_notebookProperties->SetSelection( 1 );
    event.Skip();
}

bool MarkInfoImpl::SaveChanges()
{
    if( m_pRoutePoint ) {
        if( m_pRoutePoint->m_bIsInLayer ) return true;

        // Get User input Text Fields
        m_pRoutePoint->SetName( m_textName->GetValue() );
        m_pRoutePoint->SetWaypointArrivalRadius( m_textArrivalRadius->GetValue() );
        m_pRoutePoint->SetShowWaypointRangeRings( m_checkBoxShowWaypointRangeRings->GetValue() );
        m_pRoutePoint->m_MarkDescription = m_textDescription->GetValue();
        m_pRoutePoint->SetVisible( m_checkBoxVisible->GetValue() );
        m_pRoutePoint->SetNameShown( m_checkBoxShowName->GetValue() );
        m_pRoutePoint->SetPosition( fromDMM( m_textLatitude->GetValue() ),
                fromDMM( m_textLongitude->GetValue() ) );
        wxString *icon_name = pWayPointMan->GetIconKey( m_bcomboBoxIcon->GetSelection() );
        if(icon_name && icon_name->Length())
            m_pRoutePoint->SetIconName( *icon_name );
        m_pRoutePoint->ReLoadIcon();

        // Here is some logic....
        // If the Markname is completely numeric, and is part of a route,
        // Then declare it to be of attribute m_bDynamicName = true
        // This is later used for re-numbering points on actions like
        // Insert Point, Delete Point, Append Point, etc

        if( m_pRoutePoint->m_bIsInRoute ) {
            bool b_name_is_numeric = true;
            for( unsigned int i = 0; i < m_pRoutePoint->GetName().Len(); i++ ) {
                if( wxChar( '0' ) > m_pRoutePoint->GetName()[i] ) b_name_is_numeric = false;
                if( wxChar( '9' ) < m_pRoutePoint->GetName()[i] ) b_name_is_numeric = false;
            }

            m_pRoutePoint->m_bDynamicName = b_name_is_numeric;
        } else
            m_pRoutePoint->m_bDynamicName = false;

        if( m_pRoutePoint->m_bIsInRoute ) {
            // Update the route segment selectables
            pSelect->UpdateSelectableRouteSegments( m_pRoutePoint );

            // Get an array of all routes using this point
            wxArrayPtrVoid *pEditRouteArray = g_pRouteMan->GetRouteArrayContaining( m_pRoutePoint );

            if( pEditRouteArray ) {
                for( unsigned int ir = 0; ir < pEditRouteArray->GetCount(); ir++ ) {
                    Route *pr = (Route *) pEditRouteArray->Item( ir );
                    pr->FinalizeForRendering();
                    pr->UpdateSegmentDistances();

                    pConfig->UpdateRoute( pr );
                }
                delete pEditRouteArray;
            }
        } else
            pConfig->UpdateWayPoint( m_pRoutePoint );

        // No general settings need be saved pConfig->UpdateSettings();
    }
    return true;
}

void MarkInfoImpl::OnMarkInfoOKClick( wxCommandEvent& event )
{
    if( m_pRoutePoint ) {
        m_pRoutePoint->m_iWaypointRangeRingsNumber = m_choiceWaypointRangeRingsNumber->GetSelection();
        m_pRoutePoint->m_fWaypointRangeRingsStep = atof( m_textWaypointRangeRingsStep->GetValue().mb_str() );
        m_pRoutePoint->m_iWaypointRangeRingsStepUnits = m_choiceWaypointRangeRingsUnits->GetSelection();
        
        wxString sColour;
        if( m_chColor->GetSelection() == 0 )
            sColour = _T("Red");
        else
            sColour = ::GpxxColorNames[m_chColor->GetSelection() - 1];

        wxColour col;
        for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
            if( sColour == ::GpxxColorNames[i] ) {
                col = ::GpxxColors[i];
                break;
            }
        }
        m_pRoutePoint->m_wxcWaypointRangeRingsColour = col;
        
        OnPositionCtlUpdated( event );
        SaveChanges(); // write changes to globals and update config
        cc1->RefreshRect( m_pRoutePoint->CurrentRect_in_DC.Inflate( 1000, 100 ), false );
    }

    #ifdef __WXGTK__ 
    gFrame->Raise();
    #endif
    
    Show( false );
    if( m_pMyLinkList ) {
        delete m_pMyLinkList;
        m_pMyLinkList = NULL;
    }

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateWptListCtrl();
        
    if( pRoutePropDialog && pRoutePropDialog->IsShown() )
        pRoutePropDialog->UpdateProperties();

//    SetClientSize(m_defaultClientSize);
    
    #ifdef __OCPN__ANDROID__
    androidEnableBackButton( true );
    #endif
    
    event.Skip();
}

void MarkInfoImpl::OnMarkInfoCancelClick( wxCommandEvent& event )
{
    if( m_pRoutePoint ) {
        m_pRoutePoint->SetVisible( m_bIsVisible_save );
        m_pRoutePoint->SetNameShown( m_bShowName_save );
        m_pRoutePoint->SetPosition( m_lat_save, m_lon_save );
        m_pRoutePoint->SetIconName( m_IconName_save );
        m_pRoutePoint->ReLoadIcon();

        m_pRoutePoint->m_HyperlinkList->Clear();

        int NbrOfLinks = m_pMyLinkList->GetCount();
        if( NbrOfLinks > 0 ) {
            wxHyperlinkListNode *linknode = m_pMyLinkList->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();
                Hyperlink* h = new Hyperlink();
                h->DescrText = link->DescrText;
                h->Link = link->Link;
                h->LType = link->LType;

                m_pRoutePoint->m_HyperlinkList->Append( h );

                linknode = linknode->GetNext();
            }
        }
    }

    #ifdef __WXGTK__ 
    gFrame->Raise();
    #endif
    
    Show( false );
    delete m_pMyLinkList;
    m_pMyLinkList = NULL;
//    SetClientSize(m_defaultClientSize);

    #ifdef __OCPN__ANDROID__
    androidEnableBackButton( true );
    #endif
    
    event.Skip();
}

void MarkInfoImpl::OnPositionCtlUpdated( wxCommandEvent& event )
{
    // Fetch the control values, convert to degrees
    double lat = fromDMM( m_textLatitude->GetValue() );
    double lon = fromDMM( m_textLongitude->GetValue() );

    if( !m_pRoutePoint->m_bIsInLayer ) {
        m_pRoutePoint->SetPosition( lat, lon );
        pSelect->ModifySelectablePoint( lat, lon, (void *) m_pRoutePoint, SELTYPE_ROUTEPOINT );
    }

    // Update the mark position dynamically
    cc1->Refresh();
}

void MarkInfoImpl::OnRightClick( wxCommandEvent& event )
{
    wxMenu* popup = new wxMenu();
    popup->Append( ID_RCLK_MENU_COPY, _("Copy") );
    popup->Append( ID_RCLK_MENU_COPY_LL, _("Copy lat/long") );
    popup->Append( ID_RCLK_MENU_PASTE, _("Paste") );
    popup->Append( ID_RCLK_MENU_PASTE_LL, _("Paste lat/long") );
    m_contextObject = event.GetEventObject();
    PopupMenu( popup );
    delete popup;
}

void MarkInfoDef::OnCopyPasteLatLon( wxCommandEvent& event )
{
    // Fetch the control values, convert to degrees
    double lat = fromDMM( m_textLatitude->GetValue() );
    double lon = fromDMM( m_textLongitude->GetValue() );

    wxString result;

    switch( event.GetId() ) {
        case ID_RCLK_MENU_PASTE: {
            if( wxTheClipboard->Open() ) {
                wxTextDataObject data;
                wxTheClipboard->GetData( data );
                result = data.GetText();
                ((wxTextCtrl*)m_contextObject)->SetValue( result );
                wxTheClipboard->Close();
            }
            return;
        }
        case ID_RCLK_MENU_PASTE_LL: {
            if( wxTheClipboard->Open() ) {
                wxTextDataObject data;
                wxTheClipboard->GetData( data );
                result = data.GetText();

                PositionParser pparse( result );

                if( pparse.IsOk() ) {
                    m_textLatitude->SetValue( pparse.GetLatitudeString() );
                    m_textLongitude->SetValue( pparse.GetLongitudeString() );
                }
                wxTheClipboard->Close();
            }
            return;
        }
        case ID_RCLK_MENU_COPY: {
            result = ((wxTextCtrl*)m_contextObject)->GetValue();
            break;
        }
        case ID_RCLK_MENU_COPY_LL: {
            result << toSDMM( 1, lat, true ) <<_T('\t');
            result << toSDMM( 2, lon, true );
            break;
        }
    }

    if( wxTheClipboard->Open() ) {
        wxTextDataObject* data = new wxTextDataObject;
        data->SetText( result );
        wxTheClipboard->SetData( data );
        wxTheClipboard->Close();
    }
}


void MarkInfoImpl::OnHyperLinkClick( wxHyperlinkEvent &event )
{
    if( m_toggleBtnEdit->GetValue() ) {
        m_pEditedLink = (wxHyperlinkCtrl*) event.GetEventObject();
        OnEditLink( event );
        event.Skip( false );
        return;
    }
    //    Windows has trouble handling local file URLs with embedded anchor points, e.g file://testfile.html#point1
    //    The trouble is with the wxLaunchDefaultBrowser with verb "open"
    //    Workaround is to probe the registry to get the default browser, and open directly
    //
    //    But, we will do this only if the URL contains the anchor point charater '#'
    //    What a hack......

#ifdef __WXMSW__

    wxString cc = event.GetURL();
    if( cc.Find( _T("#") ) != wxNOT_FOUND ) {
        wxRegKey RegKey( wxString( _T("HKEY_CLASSES_ROOT\\HTTP\\shell\\open\\command") ) );
        if( RegKey.Exists() ) {
            wxString command_line;
            RegKey.QueryValue( wxString( _T("") ), command_line );

            //  Remove "
            command_line.Replace( wxString( _T("\"") ), wxString( _T("") ) );

            //  Strip arguments
            int l = command_line.Find( _T(".exe") );
            if( wxNOT_FOUND == l ) l = command_line.Find( _T(".EXE") );

            if( wxNOT_FOUND != l ) {
                wxString cl = command_line.Mid( 0, l + 4 );
                cl += _T(" ");
                cc.Prepend( _T("\"") );
                cc.Append( _T("\"") );
                cl += cc;
                wxExecute( cl );        // Async, so Fire and Forget...
            }
        }
    } else
        event.Skip();
#else
    wxString url = event.GetURL();
    url.Replace(_T(" "), _T("%20") );
    ::wxLaunchDefaultBrowser(url);
//    event.Skip();
#endif
}

void MarkInfoImpl::ValidateMark( void )
{
    //    Look in the master list of Waypoints to see if the currently selected waypoint is still valid
    //    It may have been deleted as part of a route
    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

    bool b_found = false;
    while( node ) {
        RoutePoint *rp = node->GetData();
        if( m_pRoutePoint == rp ) {
            b_found = true;
            break;
        }

        node = node->GetNext();
    }

    if( !b_found ) m_pRoutePoint = NULL;
}
