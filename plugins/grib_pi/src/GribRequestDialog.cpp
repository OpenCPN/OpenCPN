/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 ***************************************************************************
 *
 */

#include "wx/wx.h"

#include "email.h"

#include "GribRequestDialog.h"
#include "GribOverlayFactory.h"

#include "TexFont.h"

#define RESOLUTIONS 4

enum { SAILDOCS,ZYGRIB };                   //grib providers
enum { GFS,COAMPS,RTOFS };                  //forecast models

wxString toMailFormat ( int NEflag, int a )                 //convert position to mail necessary format
{
    char c = NEflag == 1 ? a < 0 ? 'S' : 'N' : a < 0 ? 'W' : 'E';
    wxString s;
    s.Printf ( _T ( "%01d%c" ), abs(a), c );
    return s;
}

extern int m_SavedZoneSelMode;
extern int m_ZoneSelMode;

//----------------------------------------------------------------------------------------------------------
//          GRIB Request Implementation
//----------------------------------------------------------------------------------------------------------
GribRequestSetting::GribRequestSetting(GRIBUICtrlBar &parent )
    : GribRequestSettingBase(&parent),
      m_parent(parent)
{
    m_Vp = 0;
    InitRequestConfig();
}

GribRequestSetting::~GribRequestSetting( )
{
    delete m_Vp;
}

void GribRequestSetting::InitRequestConfig()
{
    wxFileConfig *pConf = GetOCPNConfigObject();

    if(pConf) {
        pConf->SetPath ( _T( "/PlugIns/GRIB" ) );
        wxString l;
        int m;
        pConf->Read ( _T( "MailRequestConfig" ), &m_RequestConfigBase, _T( "000220XX........0" ) );
        pConf->Read ( _T( "MailSenderAddress" ), &l, _T("") );
        m_pSenderAddress->ChangeValue( l );
        pConf->Read ( _T( "MailRequestAddresses" ), &m_MailToAddresses, _T("query@saildocs.com;gribauto@zygrib.org") );
        pConf->Read ( _T( "ZyGribLogin" ), &l, _T("") );
        m_pLogin->ChangeValue( l );
        pConf->Read ( _T( "ZyGribCode" ), &l, _T("") );
        m_pCode->ChangeValue( l );
        pConf->Read ( _T( "SendMailMethod" ), &m_SendMethod, 0 );
        pConf->Read ( _T( "MovingGribSpeed" ), &m, 0 );
        m_sMovingSpeed->SetValue( m );
        pConf->Read ( _T( "MovingGribCourse" ), &m, 0 );
        m_sMovingCourse->SetValue( m );
        m_cManualZoneSel->SetValue( m_SavedZoneSelMode != AUTO_SELECTION );      //has been read in GriUbICtrlBar dialog implementation or updated previously
        m_cUseSavedZone->SetValue( m_SavedZoneSelMode == SAVED_SELECTION );
        fgZoneCoordinatesSizer->ShowItems( m_SavedZoneSelMode != AUTO_SELECTION );
        m_cUseSavedZone->Show( m_SavedZoneSelMode != AUTO_SELECTION );
        if( m_cManualZoneSel->GetValue() ) {
            pConf->Read ( _T( "RequestZoneMaxLat" ), &m, 0 );
            m_spMaxLat->SetValue( m );
            pConf->Read ( _T( "RequestZoneMinLat" ), &m, 0 );
            m_spMinLat->SetValue( m );
            pConf->Read ( _T( "RequestZoneMaxLon" ), &m, 0 );
            m_spMaxLon->SetValue( m );
            pConf->Read ( _T( "RequestZoneMinLon" ), &m, 0 );
            m_spMinLon->SetValue( m );

            SetCoordinatesText();
        }
    //if GriDataConfig has been corrupted , take the standard one to fix a crash
    if( m_RequestConfigBase.Len() != wxString (_T( "000220XX.............." ) ).Len() )
        m_RequestConfigBase = _T( "000220XX.............." );
    }
    //populate model, mail to, waves model choices
    wxString s1[] = {_T("GFS"),_T("COAMPS"),_T("RTOFS")};
    for( unsigned int i= 0;  i<(sizeof(s1) / sizeof(wxString));i++)
        m_pModel->Append( s1[i] );
    wxString s2[] = {_T("Saildocs"),_T("zyGrib")};
    for( unsigned int i= 0;  i<(sizeof(s2) / sizeof(wxString));i++)
        m_pMailTo->Append( s2[i] );
    wxString s3[] = {_T("WW3-GLOBAL"),_T("WW3-MEDIT")};
    for( unsigned int i= 0;  i<(sizeof(s3) / sizeof(wxString));i++)
        m_pWModel->Append( s3[i] );
    m_rButtonYes->SetLabel(_("Send"));
    m_rButtonApply->SetLabel(_("Save"));
    m_tResUnit->SetLabel(wxString::Format( _T("\u00B0")));
    m_sCourseUnit->SetLabel(wxString::Format( _T("\u00B0")));

    //Set wxSpinCtrl sizing
    int w,h;
    GetTextExtent( _T("-360"), &w, &h, 0, 0, OCPNGetFont(_("Dialog"), 10)); // optimal text control size
    w += 30;
    h += 4;
    m_sMovingSpeed->SetMinSize( wxSize(w, h) );
    m_sMovingCourse->SetMinSize( wxSize(w, h) );
    m_spMaxLat->SetMinSize( wxSize(w, h) );
    m_spMinLat->SetMinSize( wxSize(w, h) );
    m_spMaxLon->SetMinSize( wxSize(w, h) );
    m_spMinLon->SetMinSize( wxSize(w, h) );

    //add tooltips
    m_pSenderAddress->SetToolTip(_("Address used to send request eMail. (Mandatory for LINUX)"));
    m_pLogin->SetToolTip(_("This is your zyGrib's forum access Login"));
    m_pCode->SetToolTip(_("Get this Code in zyGrib's forum ( This is not your password! )"));
    m_sMovingSpeed->SetToolTip(_("Enter your forescasted Speed (in Knots)"));
    m_sMovingCourse->SetToolTip(_("Enter your forecasted Course"));

    long i,j,k;
    ( (wxString) m_RequestConfigBase.GetChar(0) ).ToLong( &i );             //MailTo
    m_pMailTo->SetSelection(i);
    ( (wxString) m_RequestConfigBase.GetChar(1) ).ToLong( &i );             //Model
    m_pModel->SetSelection(i);
    m_cMovingGribEnabled->SetValue(m_RequestConfigBase.GetChar(16) == 'X' );//Moving Grib
    ( (wxString) m_RequestConfigBase.GetChar(2) ).ToLong( &i );             //Resolution
    ( (wxString) m_RequestConfigBase.GetChar(3) ).ToLong( &j );             //interval
    ( (wxString) m_RequestConfigBase.GetChar(4) ).ToLong( &k, 16 );         //Time Range
    k--;                                         // range max = 2 to 16 stored in hexa from 1 to f

#ifdef __WXMSW__                                 //show / hide sender elemants as necessary
    m_pSenderSizer->ShowItems(false);
#else
    if(m_SendMethod == 0 )
        m_pSenderSizer->ShowItems(false);
    else
        m_pSenderSizer->ShowItems(true);                //possibility to use "sendmail" method with Linux
#endif

    m_tMouseEventTimer.Connect(wxEVT_TIMER, wxTimerEventHandler( GribRequestSetting::OnMouseEventTimer ), NULL, this);

    m_RenderZoneOverlay = 0;

    ApplyRequestConfig( i, j ,k);

    ( (wxString) m_RequestConfigBase.GetChar(5) ).ToLong( &j );             //Waves model
    m_pWModel->SetSelection( j );

    m_pWind->Enable( false );                                               //always selected if available
    m_pPress->Enable( false );

    DimeWindow( this );                                                     //aplly global colours scheme

    m_AllowSend = true;
    m_MailImage->SetValue( WriteMail() );
}

void GribRequestSetting::OnClose( wxCloseEvent& event )
{
    m_RenderZoneOverlay = 0;                                    //eventually stop graphical zone display
    RequestRefresh( m_parent.pParent );

    //allow to be back to old value if changes have not been saved
    m_ZoneSelMode = m_SavedZoneSelMode;
    m_parent.SetRequestBitmap( m_ZoneSelMode );                                           //set appopriate bitmap

    this->Hide();
}

void GribRequestSetting::SetRequestDialogSize()
{
    int y;
    /*first let's size the mail display space*/
    GetTextExtent( _T("abc"), NULL, &y, 0, 0, OCPNGetFont(_("Dialog"), 10) );
    m_MailImage->SetMinSize( wxSize( -1, ( (y * m_MailImage->GetNumberOfLines()) + 10 ) ) );

    /*then as default sizing do not work with wxScolledWindow let's compute it*/
    wxSize scroll = m_fgScrollSizer->Fit(m_sScrolledDialog);                                   // the area size to be scrolled

    int w = GetOCPNCanvasWindow()->GetClientSize().x;           // the display size
    int h = GetOCPNCanvasWindow()->GetClientSize().y;
    int dMargin = 80;                                      //set a margin
    h -= ( m_rButton->GetSize().GetY() + dMargin );         //height available for the scrolled window
    w -= dMargin;                                           //width available for the scrolled window
    m_sScrolledDialog->SetMinSize( wxSize( wxMin( w, scroll.GetWidth() ), h ) );		//set scrolled area size with margin

	Layout();
    Fit();
    Refresh();
}

void GribRequestSetting::SetVpSize(PlugIn_ViewPort *vp)
{
    double lonmax= vp->lon_max;
    double lonmin= vp->lon_min;
    if( ( fabs(vp->lat_max ) < 90. ) && ( fabs( lonmax ) < 360. ) ) {
        if( lonmax < -180. ) lonmax += 360.;
        if( lonmax > 180. ) lonmax -= 360.;
    }
    if( ( fabs( vp->lat_min ) < 90. ) && ( fabs( lonmin ) < 360. ) ) {
        if( lonmin < -180. ) lonmin += 360.;
        if( lonmin > 180. ) lonmin -= 360.;
    }

    m_spMaxLat->SetValue( (int) ceil( vp->lat_max) );
    m_spMinLon->SetValue( (int) floor(lonmin) );
    m_spMinLat->SetValue( (int) floor(vp->lat_min) );
    m_spMaxLon->SetValue( (int) ceil(lonmax) );

    SetCoordinatesText();
    m_MailImage->SetValue( WriteMail() );
}

bool GribRequestSetting::MouseEventHook( wxMouseEvent &event )
{
    if( m_ZoneSelMode == AUTO_SELECTION || m_ZoneSelMode == SAVED_SELECTION || m_ZoneSelMode == START_SELECTION ) return false;

    if( event.Moving()) return false;                           //maintain status bar and tracking dialog updated

    if( event.LeftDown() ) {
        m_parent.pParent->SetFocus();
        m_ZoneSelMode = DRAW_SELECTION;                         //restart a new drawing
        m_parent.SetRequestBitmap( m_ZoneSelMode );
        if( this->IsShown() ) this->Hide();                     //eventually hide diaog in case of mode change
        m_RenderZoneOverlay = 0;                                //eventually hide previous drawing
    }

    if( event.LeftUp () && m_RenderZoneOverlay == 2 ) {
        m_ZoneSelMode = COMPLETE_SELECTION;                     //ask to complete selection
        m_parent.SetRequestBitmap( m_ZoneSelMode );
        SetCoordinatesText();
        m_MailImage->SetValue( WriteMail() );
        m_RenderZoneOverlay = 1;
    }

    if( event.Dragging() ) {
        if( m_RenderZoneOverlay < 2 ) {
            m_StartPoint = event.GetPosition();                                    //starting selection point
            m_RenderZoneOverlay = 2;
        }
		m_IsMaxLong = m_StartPoint.x > event.GetPosition().x? true: false;         //find if startpoint is max longitude 
        GetCanvasLLPix( m_Vp, event.GetPosition(), &m_Lat, &m_Lon);                //extend selection
        if( !m_tMouseEventTimer.IsRunning() ) m_tMouseEventTimer.Start( 20, wxTIMER_ONE_SHOT );
    }
    return true;
}

void GribRequestSetting::OnMouseEventTimer( wxTimerEvent & event)
{
    //compute zone starting point lon/lat for zone drawing
    double lat,lon;
    GetCanvasLLPix( m_Vp, m_StartPoint, &lat, &lon);

    //compute rounded coordinates
    if( lat > m_Lat) {
        m_spMaxLat->SetValue( (int) ceil(lat) );
        m_spMinLat->SetValue( (int) floor(m_Lat) );
    }
    else {
        m_spMaxLat->SetValue( (int) ceil(m_Lat) );
        m_spMinLat->SetValue( (int) floor(lat) );
    }
	if(m_IsMaxLong) {
        m_spMaxLon->SetValue( (int) ceil(lon) );
        m_spMinLon->SetValue( (int) floor(m_Lon) );
    }
    else {
        m_spMaxLon->SetValue( (int) ceil(m_Lon) );
        m_spMinLon->SetValue( (int) floor(lon) );
    }

    RequestRefresh( m_parent.pParent );
}

void GribRequestSetting::SetCoordinatesText()
{
    m_stMaxLatNS->SetLabel( m_spMaxLat->GetValue() < 0 ? _("S")  : _("N") );
    m_stMinLonEW->SetLabel( m_spMinLon->GetValue() < 0 ? _("W")  : _("E") );
    m_stMaxLonEW->SetLabel( m_spMaxLon->GetValue() < 0 ? _("W")  : _("E") );
    m_stMinLatNS->SetLabel( m_spMinLat->GetValue() < 0 ? _("S")  : _("N") );
}

void GribRequestSetting::StopGraphicalZoneSelection()
{
    m_RenderZoneOverlay = 0;                                                //eventually stop graphical zone display

    RequestRefresh( m_parent.pParent );
}

void GribRequestSetting::OnVpChange(PlugIn_ViewPort *vp)
{
    delete m_Vp;
    m_Vp = new PlugIn_ViewPort(*vp);

    if(!m_AllowSend) return;
    if( m_cManualZoneSel->GetValue() ) return;

    SetVpSize(vp);
}

void GribRequestSetting::ApplyRequestConfig( unsigned rs, unsigned it, unsigned tr )
{
    //some useful  strings
    const wxString res[][RESOLUTIONS] = {
        {_T("0.25"), _T("0.5"), _T("1.0"), _T("2.0")},
        {_T("0.2"), _T("0.8"), _T("1.6"), wxEmptyString},
        {_T("0.05"), _T("0.25"), _T("1.0"), wxEmptyString}
    };

    IsZYGRIB = m_pMailTo->GetCurrentSelection() == ZYGRIB;
    if(IsZYGRIB) m_pModel->SetSelection(GFS);                       //Model is always GFS when Zygrib selected
    IsGFS = m_pModel->GetCurrentSelection() == GFS;
    bool IsRTOFS = m_pModel->GetCurrentSelection() == RTOFS;

    //populate resolution choice
    m_pResolution->Clear();
    for( int i = 0; i < RESOLUTIONS; i++ ) {
        if( res[m_pModel->GetCurrentSelection()][i] != wxEmptyString )
            m_pResolution->Append(res[m_pModel->GetCurrentSelection()][i]);
    }
     m_pResolution->SetSelection(rs);

    unsigned l;
     //populate time interval choice
    l = IsGFS ? 3 : IsRTOFS ? 12 : 6;
    m_pInterval->Clear();
    for( unsigned i=l; i<25; i*=2)
        m_pInterval->Append( wxString::Format(_T("%d"), i));
    m_pInterval->SetSelection(wxMin(it,m_pInterval->GetCount()-1));

    //populate time range choice
    l = IsZYGRIB ? 8 : IsGFS ? 16 : IsRTOFS ? 6 : 3;
    m_pTimeRange->Clear();
    for( unsigned i=2; i<l+1; i++)
        m_pTimeRange->Append( wxString::Format(_T("%d"), i));
    m_pTimeRange->SetSelection( wxMin(l-2, tr));

    m_pModel->Enable(!IsZYGRIB);
    m_pWind->SetValue( !IsRTOFS );
    m_pPress->SetValue( !IsRTOFS );
    m_pWaves->SetValue( m_RequestConfigBase.GetChar(8) == 'X' && IsGFS );
    m_pWaves->Enable( IsGFS && m_pTimeRange->GetCurrentSelection() < 7 );      //gfs & time range less than 8 days
    m_pRainfall->SetValue( m_RequestConfigBase.GetChar(9) == 'X' && IsGFS );
    m_pRainfall->Enable( IsGFS );
    m_pCloudCover->SetValue( m_RequestConfigBase.GetChar(10) == 'X' && IsGFS );
    m_pCloudCover->Enable( IsGFS );
    m_pAirTemp->SetValue( m_RequestConfigBase.GetChar(11) == 'X' && IsGFS );
    m_pAirTemp->Enable( IsGFS );
    m_pSeaTemp->SetValue( (m_RequestConfigBase.GetChar(12) == 'X' && (!IsZYGRIB && IsGFS)) || IsRTOFS );
    m_pSeaTemp->Enable( !IsZYGRIB && IsGFS );
    m_pWindGust->SetValue( m_RequestConfigBase.GetChar(14) == 'X' && IsGFS);
    m_pWindGust->Enable( IsGFS );
    m_pCAPE->SetValue( m_RequestConfigBase.GetChar(15) == 'X' && IsGFS );
    m_pCAPE->Enable( IsGFS );

    m_pAltitudeData->SetValue( IsGFS ? m_RequestConfigBase.GetChar(17) == 'X' : false );        //altitude data zigrib + saildocs only GFS
    m_pAltitudeData->Enable( IsGFS );
    m_p850hpa->SetValue( IsZYGRIB ? m_RequestConfigBase.GetChar(18) == 'X' : false );           //only zygrib
    m_p850hpa->Enable( IsZYGRIB );
    m_p700hpa->SetValue(  IsZYGRIB ? m_RequestConfigBase.GetChar(19) == 'X' : false );          //only zigrib
    m_p700hpa->Enable( IsZYGRIB );
    m_p500hpa->SetValue(  IsGFS ? m_RequestConfigBase.GetChar(20) == 'X' : false );             //zigrib + saildocs only GFS
    m_p300hpa->SetValue(  IsZYGRIB ? m_RequestConfigBase.GetChar(21) == 'X' : false  );         //only zigrib
    m_p300hpa->Enable( IsZYGRIB );

    m_pCurrent->SetValue( IsRTOFS );
    m_pCurrent->Enable( false );

    //show parameters only if necessary
    m_cMovingGribEnabled->Show(!IsZYGRIB);                                  //show/hide Moving settings
    m_fgMovingParams->ShowItems( m_cMovingGribEnabled->IsChecked() && m_cMovingGribEnabled->IsShown() );

    m_fgLog->ShowItems(IsZYGRIB);                                           //show/hide zigrib login

    m_pWModel->Show(IsZYGRIB && m_pWaves->IsChecked());                     //show/hide waves model

    m_fgAltitudeData->ShowItems(m_pAltitudeData->IsChecked());              //show/hide altitude params
}

void GribRequestSetting::OnTopChange(wxCommandEvent &event)
{

    ApplyRequestConfig( m_pResolution->GetCurrentSelection(), m_pInterval->GetCurrentSelection(), m_pTimeRange->GetCurrentSelection() );

    m_cMovingGribEnabled->Show( m_pMailTo->GetCurrentSelection() == SAILDOCS );

    if(m_AllowSend) m_MailImage->SetValue( WriteMail() );

    SetRequestDialogSize();
}

void GribRequestSetting::OnZoneSelectionModeChange( wxCommandEvent& event )
{
    StopGraphicalZoneSelection();                       //eventually stop graphical zone display

    if( !m_ZoneSelMode )
        SetVpSize( m_Vp );                              //recompute zone

    if( event.GetId() == MANSELECT ) {
        //set temporarily zone selection mode if manual selection set, put it directly in "drawing" position
        //else put it in "auto selection position
        m_ZoneSelMode = m_cManualZoneSel->GetValue() ? DRAW_SELECTION : AUTO_SELECTION;
        m_cUseSavedZone->SetValue( false );
    } else if(event.GetId() == SAVEDZONE ) {
        //set temporarily zone selection mode if saved selection set, put it directly in "no selection" position
        //else put it directly in "drawing" position
        m_ZoneSelMode = m_cUseSavedZone->GetValue()? SAVED_SELECTION : DRAW_SELECTION;
    }
    m_parent.SetRequestBitmap( m_ZoneSelMode );               //set appopriate bitmap
    fgZoneCoordinatesSizer->ShowItems( m_ZoneSelMode != AUTO_SELECTION ); //show coordinate if necessary
    m_cUseSavedZone->Show( m_ZoneSelMode != AUTO_SELECTION );
    if(m_AllowSend) m_MailImage->SetValue( WriteMail() );

    SetRequestDialogSize();
}

bool GribRequestSetting::DoRenderZoneOverlay()
{
    wxPoint p;
    GetCanvasPixLL( m_Vp, &p, m_Lat, m_Lon);

    int x = (m_StartPoint.x < p.x) ? m_StartPoint.x : p.x;
    int y = (m_StartPoint.y < p.y) ? m_StartPoint.y : p.y;

    int zw = fabs( (double ) p.x - m_StartPoint.x );
    int zh = fabs( (double ) p.y - m_StartPoint.y );

    wxPoint center;
    center.x = x + (zw / 2);
    center.y = y + (zh / 2);

    wxFont *font = OCPNGetFont(_("Dialog"), 10);
    wxColour pen_color, back_color;
    GetGlobalColor( _T ( "DASHR" ), &pen_color );
    GetGlobalColor( _T ( "YELO1" ), &back_color );

    int label_offsetx = 5, label_offsety = 1;

    double size;
    EstimateFileSize( &size );

    wxString label( _("Coord. ") );
    label.Append( toMailFormat(1, m_spMaxLat->GetValue()) + _T(" "));
    label.Append( toMailFormat(0, m_spMinLon->GetValue()) + _T(" "));
    label.Append( toMailFormat(1, m_spMinLat->GetValue()) + _T(" "));
    label.Append( toMailFormat(0, m_spMaxLon->GetValue()) + _T("\n"));
    label.Append( _T("Estim. Size ") ).Append((wxString::Format( _T("%1.2f " ) , size ) + _("MB") ) );

    if( m_pdc ) {
        wxPen pen(pen_color);
        pen.SetWidth(3);
        m_pdc->SetPen( pen );
        m_pdc->SetBrush( *wxTRANSPARENT_BRUSH);
        m_pdc->DrawRectangle(x, y, zw, zh);



        int w, h, sl;
#ifdef __WXMAC__
        wxScreenDC sdc;
        sdc.GetMultiLineTextExtent(label, &w, &h, &sl, font);
#else
        m_pdc->GetMultiLineTextExtent(label, &w, &h, &sl, font);
#endif
        w += 2*label_offsetx, h += 2*label_offsety;
        x = center.x - (w / 2);
        y = center.y - (h / 2);

        wxBitmap bm(w, h);
        wxMemoryDC mdc(bm);
        mdc.Clear();

        mdc.SetFont( *font );
        mdc.SetBrush(back_color);
        mdc.SetPen(*wxTRANSPARENT_PEN);
        mdc.SetTextForeground(wxColor( 0, 0, 0 ));
        mdc.DrawRectangle(0, 0, w, h);
        mdc.DrawLabel( label, wxRect( label_offsetx, label_offsety, w, h ) );

        wxImage im = bm.ConvertToImage();
        im.InitAlpha();
        w = im.GetWidth(), h = im.GetHeight();
        for( int j = 0; j < h; j++ )
			for( int i = 0; i < w; i++ )
				im.SetAlpha( i, j, 155 );

        m_pdc->DrawBitmap(im, x, y, true);

    } else {

#ifdef ocpnUSE_GL
    TexFont m_TexFontlabel;
    m_TexFontlabel.Build(*font);

    glColor3ub(pen_color.Red(), pen_color.Green(), pen_color.Blue() );

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_ENABLE_BIT |
                     GL_POLYGON_BIT | GL_HINT_BIT );

   glEnable( GL_LINE_SMOOTH );
   glEnable( GL_BLEND );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
   glLineWidth( 3.f );

   glBegin( GL_LINES );
   glVertex2d( x, y );
   glVertex2d( x+zw, y );
   glVertex2d( x+zw, y );
   glVertex2d( x+zw, y+zh );
   glVertex2d( x+zw, y+zh );
   glVertex2d( x, y+zh );
   glVertex2d( x, y+zh );
   glVertex2d( x, y );
   glEnd();

   int w, h;
   glColor4ub(back_color.Red(), back_color.Green(), back_color.Blue(), 155 );
   m_TexFontlabel.GetTextExtent(label, &w, &h );

    w += 2*label_offsetx, h += 2*label_offsety;
    x = center.x - (w / 2);
    y = center.y - (h / 2);

   /* draw text background */
   glBegin(GL_QUADS);
   glVertex2i(x,   y);
   glVertex2i(x+w, y);
   glVertex2i(x+w, y+h);
   glVertex2i(x,   y+h);
   glEnd();

   /* draw text */
   glColor3ub( 0, 0, 0 );

   glEnable(GL_TEXTURE_2D);
   m_TexFontlabel.RenderString(label, x + label_offsetx, y + label_offsety);
   glDisable(GL_TEXTURE_2D);

   glDisable( GL_BLEND );

#endif
    }
    return true;
}

bool GribRequestSetting::RenderGlZoneOverlay()
{
    if( m_RenderZoneOverlay == 0 ) return false;
    m_pdc = NULL;                  // inform lower layers that this is OpenGL render
    return DoRenderZoneOverlay();
}

bool GribRequestSetting::RenderZoneOverlay( wxDC &dc )
{
    if( m_RenderZoneOverlay == 0 ) return false;
    m_pdc = &dc;
    return DoRenderZoneOverlay();
}

void GribRequestSetting::OnMovingClick( wxCommandEvent& event )
{
    m_fgMovingParams->ShowItems( m_cMovingGribEnabled->IsChecked() && m_cMovingGribEnabled->IsShown() );

    if(m_AllowSend) m_MailImage->SetValue( WriteMail() );
    SetRequestDialogSize();

    this->Refresh();
}

void GribRequestSetting::OnCoordinatesChange( wxSpinEvent& event )
{
    SetCoordinatesText();

    StopGraphicalZoneSelection();                           //eventually stop graphical zone display

    if( !m_AllowSend ) return;

    m_MailImage->SetValue( WriteMail() );
}



void GribRequestSetting::OnAnyChange(wxCommandEvent &event)
{
    m_fgAltitudeData->ShowItems(m_pAltitudeData->IsChecked());

    m_pWModel->Show( IsZYGRIB && m_pWaves->IsChecked());

    if(m_AllowSend) m_MailImage->SetValue( WriteMail() );

    SetRequestDialogSize();
}

void GribRequestSetting::OnTimeRangeChange(wxCommandEvent &event)
{
    m_pWModel->Show( IsZYGRIB && m_pWaves->IsChecked());

    if( m_pModel->GetCurrentSelection() == 0 ) {               //gfs
        if( m_pTimeRange->GetCurrentSelection() > 6 ) {         //time range more than 8 days
            m_pWaves->SetValue(0);
            m_pWaves->Enable(false);
            OCPNMessageBox_PlugIn(this, _("You request a forecast for more than 8 days horizon.\nThis is conflicting with Wave data which will be removed from your request.\nDon't forget that beyond the first 8 days, the resolution will be only 2.5\u00B0x2.5\u00B0\nand the time intervall 12 hours."),
                _("Warning!") );
        } else
            m_pWaves->Enable(true);
    }

    if(m_AllowSend) m_MailImage->SetValue( WriteMail() );

    SetRequestDialogSize();
}

void GribRequestSetting::OnSaveMail( wxCommandEvent& event )
{
    m_RequestConfigBase.SetChar( 0, (char) ( m_pMailTo->GetCurrentSelection() + '0' ) );            //recipient
    m_cMovingGribEnabled->IsChecked() ? m_RequestConfigBase.SetChar( 16, 'X' )                      //moving grib
        : m_RequestConfigBase.SetChar( 16, '.' );

    if( !IsZYGRIB )
        m_RequestConfigBase.SetChar( 1, (char) ( m_pModel->GetCurrentSelection() + '0' ) );         //model

    if(m_pModel->GetCurrentSelection() != RTOFS)
        m_RequestConfigBase.SetChar( 2, (char) ( m_pResolution->GetCurrentSelection() + '0' ) );    //resolution

    m_RequestConfigBase.SetChar( 3, (char) ( m_pInterval->GetCurrentSelection() + '0' ) );

    wxString range;
    range.Printf(_T("%x"), m_pTimeRange->GetCurrentSelection() + 1 );                  //range max = 2 to 16 stored in hexa 1 to f
    m_RequestConfigBase.SetChar( 4, range.GetChar( 0 ) );

    if( IsZYGRIB && m_pWModel->IsShown() )
        m_RequestConfigBase.SetChar( 5, (char) ( m_pWModel->GetCurrentSelection() + '0' ) );        //waves model

    m_RequestConfigBase.SetChar( 6, 'X' );                                              //wind must be always selected as a default
    m_RequestConfigBase.SetChar( 7, 'X' );                                              //pressure must be always selected as a default

    if(m_pModel->GetCurrentSelection() != COAMPS) {
        m_pWindGust->IsChecked() ? m_RequestConfigBase.SetChar( 14, 'X' )                           //Gust
            : m_RequestConfigBase.SetChar( 14, '.' );
        m_pWaves->IsChecked() ? m_RequestConfigBase.SetChar( 8, 'X' )                               //waves
            : m_RequestConfigBase.SetChar( 8, '.' );
        m_pRainfall->IsChecked() ? m_RequestConfigBase.SetChar( 9, 'X' )                            //rainfall
            : m_RequestConfigBase.SetChar( 9, '.' );
        m_pCloudCover->IsChecked() ? m_RequestConfigBase.SetChar( 10, 'X' )                         //clouds
            : m_RequestConfigBase.SetChar( 10, '.' );
        m_pAirTemp->IsChecked() ? m_RequestConfigBase.SetChar( 11, 'X' )                            //air temp
            : m_RequestConfigBase.SetChar( 11, '.' );
        m_pSeaTemp->IsChecked() ? m_RequestConfigBase.SetChar( 12, 'X' )                            //sea temp
            : m_RequestConfigBase.SetChar( 12, '.' );
        m_pCAPE->IsChecked() ? m_RequestConfigBase.SetChar( 15, 'X' )                               //cape
            : m_RequestConfigBase.SetChar( 15, '.' );
    }
    if(m_pModel->GetCurrentSelection() != ZYGRIB && m_pModel->GetCurrentSelection() != COAMPS)      //current
        m_pCurrent->IsChecked() ? m_RequestConfigBase.SetChar( 13, 'X' )
            : m_RequestConfigBase.SetChar( 13, '.' );

    if( IsGFS ) {
        m_pAltitudeData->IsChecked() ?  m_RequestConfigBase.SetChar( 17, 'X' )                      //altitude data
        : m_RequestConfigBase.SetChar( 17, '.' );
        m_p500hpa->IsChecked() ?  m_RequestConfigBase.SetChar( 20, 'X' )
        : m_RequestConfigBase.SetChar( 20, '.' );
    }
    if( IsZYGRIB ) {
        m_p850hpa->IsChecked() ?  m_RequestConfigBase.SetChar( 18, 'X' )
            : m_RequestConfigBase.SetChar( 18, '.' );
        m_p700hpa->IsChecked() ?  m_RequestConfigBase.SetChar( 19, 'X' )
            : m_RequestConfigBase.SetChar( 19, '.' );
        m_p300hpa->IsChecked() ?  m_RequestConfigBase.SetChar( 21, 'X' )
            : m_RequestConfigBase.SetChar( 21, '.' );
    }

    wxFileConfig *pConf = GetOCPNConfigObject();
    if(pConf) {
        pConf->SetPath ( _T( "/PlugIns/GRIB" ) );

        pConf->Write ( _T ( "MailRequestConfig" ), m_RequestConfigBase );
        pConf->Write ( _T( "MailSenderAddress" ), m_pSenderAddress->GetValue() );
        pConf->Write ( _T( "MailRequestAddresses" ), m_MailToAddresses );
        pConf->Write ( _T( "ZyGribLogin" ), m_pLogin->GetValue() );
        pConf->Write ( _T( "ZyGribCode" ), m_pCode->GetValue() );
        pConf->Write ( _T( "SendMailMethod" ), m_SendMethod );
        pConf->Write ( _T( "MovingGribSpeed" ), m_sMovingSpeed->GetValue() );
        pConf->Write ( _T( "MovingGribCourse" ), m_sMovingCourse->GetValue() );

        m_SavedZoneSelMode = m_cUseSavedZone->GetValue()? SAVED_SELECTION: m_cManualZoneSel->GetValue()? START_SELECTION: AUTO_SELECTION;
        pConf->Write ( _T( "ManualRequestZoneSizing" ), m_SavedZoneSelMode );

        pConf->Write ( _T( "RequestZoneMaxLat" ), m_spMaxLat->GetValue() );
        pConf->Write ( _T( "RequestZoneMinLat" ), m_spMinLat->GetValue() );
        pConf->Write ( _T( "RequestZoneMaxLon" ), m_spMaxLon->GetValue() );
        pConf->Write ( _T( "RequestZoneMinLon" ), m_spMinLon->GetValue() );

    }

    wxCloseEvent evt;
    OnClose ( evt );
}

wxString GribRequestSetting::WriteMail()
{
    //define size limits for zyGrib
    int limit = IsZYGRIB ? 2 : 0;                                            //new limit  2 mb

    m_MailError_Nb = 0;
    //some useful strings
    const wxString s[] = { _T(","), _T(" ") };        //separators
    const wxString p[][11] = {{ _T("APCP"), _T("TCDC"), _T("AIRTMP"), _T("HTSGW,WVPER,WVDIR"),      //parameters Saildocs
        _T("SEATMP"), _T("GUST"), _T("CAPE"), wxEmptyString, wxEmptyString, _T("WIND500,HGT500"), wxEmptyString},
        {_T("PRECIP"), _T("CLOUD"), _T("TEMP"), _T("WVSIG WVWIND"), wxEmptyString, _T("GUST"),      //parameters zigrib
            _T("CAPE"), _T("A850"), _T("A700"), _T("A500"), _T("A300")} };

    wxString r_topmess,r_parameters,r_zone;
    //write the top part of the mail
    switch( m_pMailTo->GetCurrentSelection() ) {
    case SAILDOCS:                                                                         //Saildocs
        r_zone = toMailFormat(1, m_spMaxLat->GetValue() ) + _T(",") + toMailFormat(1, m_spMinLat->GetValue() ) + _T(",")
            + toMailFormat(2, m_spMinLon->GetValue() ) + _T(",") + toMailFormat(2, m_spMaxLon->GetValue() );
        r_topmess = wxT("send ");
        r_topmess.Append(m_pModel->GetStringSelection() + _T(":"));
        r_topmess.Append( r_zone  + _T("|"));
        r_topmess.Append(m_pResolution->GetStringSelection()).Append(_T(","))
            .Append(m_pResolution->GetStringSelection()).Append(_T("|"));
        double v;
        m_pInterval->GetStringSelection().ToDouble(&v);
        r_topmess.Append(wxString::Format(_T("0,%d,%d"), (int) v, (int) v*2));
        m_pTimeRange->GetStringSelection().ToDouble(&v);
        r_topmess.Append(wxString::Format(_T("..%d"), (int) v*24) + _T("|=\n"));
        break;
	case ZYGRIB:                                                                         //Zygrib
		double maxlon = (m_spMinLon->GetValue() > m_spMaxLon->GetValue() && m_spMaxLon->GetValue() < 0)?
			m_spMaxLon->GetValue() + 360 : m_spMaxLon->GetValue();
		r_zone = toMailFormat(1, m_spMinLat->GetValue() ) + toMailFormat(2, m_spMinLon->GetValue() ) + _T(" ")
			+ toMailFormat(1, m_spMaxLat->GetValue() ) + toMailFormat(2, maxlon );
        r_topmess = wxT("login : ");
        r_topmess.Append(m_pLogin->GetValue() + _T("\n"));
        r_topmess.Append(wxT("code :"));
        r_topmess.Append(m_pCode->GetValue() + _T("\n"));
        r_topmess.Append(wxT("area : "));
        r_topmess.append(r_zone + _T("\n"));
        r_topmess.Append(wxT("resol : "));
        r_topmess.append(m_pResolution->GetStringSelection() + _T("\n"));
        r_topmess.Append(wxT("days : "));
        r_topmess.append(m_pTimeRange->GetStringSelection() + _T("\n"));
        r_topmess.Append(wxT("hours : "));
        r_topmess.append(m_pInterval->GetStringSelection() + _T("\n"));
        if(m_pWaves->IsChecked()) {
            r_topmess.Append(wxT("waves : "));
            r_topmess.append(m_pWModel->GetStringSelection() + _T("\n"));
        }
        r_topmess.Append(wxT("meteo : "));
        r_topmess.append(m_pModel->GetStringSelection() + _T("\n"));
        if ( m_pLogin->GetValue().IsEmpty() || m_pCode->GetValue().IsEmpty() ) m_MailError_Nb = 6;
        break;
    }
    //write the parameters part of the mail
    switch( m_pModel->GetCurrentSelection() ) {
    case GFS:                                                                           //GFS
        r_parameters = wxT("WIND") + s[m_pMailTo->GetCurrentSelection()] + wxT("PRESS");    // the default minimum request parameters
        if( m_pRainfall->IsChecked() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][0] );
        if( m_pCloudCover->IsChecked() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][1] );
        if( m_pAirTemp->IsChecked() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][2] );
        if( m_pWaves->IsChecked() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][3] );
        if( m_pSeaTemp->IsChecked() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][4] );
        if( m_pWindGust->IsChecked() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][5] );
        if( m_pCAPE->IsChecked() )
            r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][6] );
        if(m_pAltitudeData->IsChecked() ){
            if( m_p850hpa->IsChecked() )
                r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][7] );
            if( m_p700hpa->IsChecked() )
                r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][8] );
            if( m_p500hpa->IsChecked() )
                r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][9] );
            if( m_p300hpa->IsChecked() )
                r_parameters.Append( s[m_pMailTo->GetCurrentSelection()] + p[m_pMailTo->GetCurrentSelection()][10] );
        }
        break;
    case COAMPS:                                                                           //COAMPS
        r_parameters = wxT("WIND,PRMSL");                                 //the default parameters for this model
        break;
    case RTOFS:                                                                           //RTOFS
        r_parameters = wxT("CUR,WTMP");                                   //the default parameters for this model
        break;
    }
    if( !IsZYGRIB && m_cMovingGribEnabled->IsChecked())            //moving grib
        r_parameters.Append(wxString::Format(_T("|%d,%d"),m_sMovingSpeed->GetValue(),m_sMovingCourse->GetValue()));

    // line lenth limitation
    int j = 0;
    char c =  m_pMailTo->GetCurrentSelection() == SAILDOCS ? ',' : ' ';
    for( size_t i = 0; i < r_parameters.Len(); i++ ) {
        if(r_parameters.GetChar( i ) == '|' ) j--;                        //do not split Saildocs "moving" values
        if(r_parameters.GetChar( i ) == c ) j++;
        if( j > 6 ) {                                                       //no more than 6 parameters on the same line
            r_parameters.insert( i + 1 , m_pMailTo->GetCurrentSelection() == SAILDOCS ? _T("=\n") : _T("\n"));
            break;
        }
    }

    double size;
    m_MailError_Nb += EstimateFileSize( &size );

    m_tFileSize->SetLabel(wxString::Format( _T("%1.2f " ) , size ) + _("MB") );

    if( IsZYGRIB ) {
        m_tLimit->SetLabel(wxString( _T("( ") ) + _("Max") + wxString::Format(_T(" %d "), limit) + _("MB") + _T(" )") );
        if(size > limit) m_MailError_Nb += 2;
    } else
        m_tLimit->SetLabel(wxEmptyString);

    return wxString( r_topmess + r_parameters );
}

int GribRequestSetting::EstimateFileSize( double *size )
{
    //too small zone ? ( mini 2 * resolutions )
    double reso,time,inter;
    m_pResolution->GetStringSelection().ToDouble(&reso);
    m_pTimeRange->GetStringSelection().ToDouble(&time);
    m_pInterval->GetStringSelection().ToDouble(&inter);

    double maxlon = m_spMaxLon->GetValue(), minlon = m_spMinLon->GetValue();
    double maxlat = m_spMaxLat->GetValue(), minlat = m_spMinLat->GetValue();
    if( maxlat - minlat < 0 )
        return 3;                               // maxlat must be > minlat
    double wlon = (maxlon > minlon ? 0 : 360) + maxlon - minlon;
    if (wlon > 180 || ( maxlat - minlat > 180 ))
        return 4;                           //ovoid too big area

    if ( fabs(wlon) < 2*reso || maxlat - minlat < 2*reso  )
        return 5;                           //ovoid too small area

    int npts = (int) (  ceil(((double)(maxlat - minlat )/reso))
                      * ceil(((double)(wlon )/reso)) );

    if(m_pModel->GetCurrentSelection() == COAMPS )                                           //limited area for COAMPS
        npts = wxMin(npts, (int) (  ceil(40.0/reso) * ceil(40.0/reso) ) );

    // Nombre de GribRecords
    int nbrec = (int) (time*24/inter)+1;
    int nbPress = (m_pPress->IsChecked()) ?  nbrec   : 0;
    int nbWind  = (m_pWind->IsChecked()) ?  2*nbrec : 0;
    int nbwave  = (m_pWaves->IsChecked()) ?  2*nbrec : 0;
    int nbRain  = (m_pRainfall->IsChecked()) ?  nbrec-1 : 0;
    int nbCloud = (m_pCloudCover->IsChecked()) ?  nbrec-1 : 0;
    int nbTemp  = (m_pAirTemp->IsChecked())    ?  nbrec   : 0;
    int nbSTemp  = (m_pSeaTemp->IsChecked())    ?  nbrec   : 0;
    int nbGUSTsfc  = (m_pWindGust->IsChecked()) ?  nbrec : 0;
    int nbCurrent  = (m_pCurrent->IsChecked()) ?  nbrec : 0;
    int nbCape  = (m_pCAPE->IsChecked()) ?  nbrec : 0;
    int nbAltitude  = IsZYGRIB ? 5 * nbrec : 3 * nbrec;             //five data types are included in each ZyGrib altitude request and only
                                                                    // three in sSaildocs's
    int head = 84;
    double estime = 0.0;
    int nbits;

    nbits = 13;
    estime += nbWind*(head+(nbits*npts)/8+2 );
    estime += nbCurrent*(head+(nbits*npts)/8+2 );

    nbits = 11;
    estime += nbTemp*(head+(nbits*npts)/8+2 );
    estime += nbSTemp*(head+(nbits*npts)/8+2 );

    nbits = 4;
    estime += nbRain*(head+(nbits*npts)/8+2 );

    nbits = 15;
    estime += nbPress*(head+(nbits*npts)/8+2 );

    nbits = 4;
    estime += nbCloud*(head+(nbits*npts)/8+2 );

    nbits = 7;
    estime += nbGUSTsfc*(head+(nbits*npts)/8+2 );

    nbits = 5;
    estime += nbCape*(head+(nbits*npts)/8+2 );

    nbits = 6;
    estime += nbwave*(head+(nbits*npts)/8+2 );

    if( m_pAltitudeData->IsChecked() ) {
        int nbalt = 0;
        if( m_p850hpa->IsChecked() ) nbalt ++;
        if( m_p700hpa->IsChecked() ) nbalt ++;
        if( m_p500hpa->IsChecked() ) nbalt ++;
        if( m_p300hpa->IsChecked() ) nbalt ++;

        nbits = 12;
        estime += nbAltitude*nbalt*(head+(nbits*npts)/8+2 );
    }


    if(size) *size = estime / (1024.*1024.);

    return 0;
}

void GribRequestSetting::OnSendMaiL( wxCommandEvent& event  )
{
    StopGraphicalZoneSelection();                    //eventually stop graphical zone display

    if(!m_AllowSend) {
        m_rButtonCancel->Show();
        m_rButtonApply->Show();
        m_rButtonYes->SetLabel(_("Send"));

        m_MailImage->SetForegroundColour(wxColor( 0, 0, 0 ));                   //permit to send a (new) message
        m_AllowSend = true;

        m_MailImage->SetValue( WriteMail() );
        SetRequestDialogSize();

        return;
    }

    const wxString error[] = { _T("\n\n"), _("Before sending an email to Zygrib you have to enter your Login and Code.\nPlease visit www.zygrib.org/ and follow instructions..."),
        _("Too big file! zyGrib limit is 2Mb!"), _("Error! Max Lat lower than Min Lat or Max Lon lower than Min Lon!"),
        _("Too large area! Each side must be less than 180\u00B0!"), _("Too small area for this resolution!") };

    ::wxBeginBusyCursor();

    m_MailImage->SetForegroundColour(wxColor( 255, 0, 0 ));
    m_AllowSend = false;

    if( m_MailError_Nb ) {
        if( m_MailError_Nb > 7 ) {
            m_MailImage->SetValue( error[1] + error[0] + error[m_MailError_Nb - 6] );
        } else {
            if( m_MailError_Nb == 6 ) m_MailError_Nb = 1;
            m_MailImage->SetValue( error[m_MailError_Nb] );
        }

        m_rButtonCancel->Hide();
        m_rButtonApply->Hide();
        m_rButtonYes->SetLabel(_("Continue..."));
        m_rButton->Layout();
        SetRequestDialogSize();

        ::wxEndBusyCursor();

        return;
    }

    wxMailMessage *message = new wxMailMessage(
    wxT("gribauto"),                                                                            //requested subject
    (m_pMailTo->GetCurrentSelection() == SAILDOCS) ? m_MailToAddresses.BeforeFirst(_T(';'))     //to request address
        : m_MailToAddresses.AfterFirst(_T(';')),
    WriteMail(),                                                                                 //message image
    m_pSenderAddress->GetValue()
    );
    wxEmail mail ;
    if(mail.Send( *message, m_SendMethod)) {
#ifdef __WXMSW__
        m_MailImage->SetValue(
            _("Your request is ready. An email is prepared in your email environment. \nYou have just to verify and send it...\nSave or Cancel to finish...or Continue...") );
#else
        if(m_SendMethod == 0 ) {
            m_MailImage->SetValue(
            _("Your request is ready. An email is prepared in your email environment. \nYou have just to verify and send it...\nSave or Cancel to finish...or Continue...") );
        } else {
        m_MailImage->SetValue(
            _("Your request was sent \n(if your system has an MTA configured and is able to send email).\nSave or Cancel to finish...or Continue...") );
        }
#endif
    } else {
        m_MailImage->SetValue(
            _("Request can't be sent. Please verify your email systeme parameters.\nYou should also have a look at your log file.\nSave or Cancel to finish..."));
        m_rButtonYes->Hide();
    }
    m_rButtonYes->SetLabel(_("Continue..."));
    m_rButton->Layout();
    SetRequestDialogSize();

    ::wxEndBusyCursor();
}
