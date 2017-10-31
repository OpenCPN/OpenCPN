/******************************************************************************
 * $Id: aa1chartnavhelperchartnavhelper_pi.cpp,v 1.0 2011/02/26 01:54:37 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  AA1ChartNavHelper Plugin
 * Author:   Andras Andras
 *
 ***************************************************************************
 *   Copyright (C) 2017 by Andras Andras   *
 *   $EMAIL$   *
 *                                                 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                         *
 *                                                 *
 *   This program is distributed in the hope that it will be useful,     *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
 *   GNU General Public License for more details.                  *
 *                                                 *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                           *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#ifndef __OCPN__ANDROID__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include "qopengl.h"                  // this gives us the qt runtime gles2.h
#include "GL/gl_private.h"
#endif

//byAA10152017:
#include <wx/dcgraph.h>         // supplemental, for Mac
//byAA10152017.

void AA1ChartNavHelperLogMessage1(wxString s) { wxLogMessage(_T("AA1: ") + s); }
extern "C" void AA1ChartNavHelperLogMessage(const char *s) { AA1ChartNavHelperLogMessage1(wxString::FromAscii(s)); }

#include "aa1chartnavhelper_pi.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new aa1chartnavhelper_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

//the user defined variation .. would need static linking oops
//extern double g_UserVar;

//---------------------------------------------------------------------------------------------------------
//
//    AA1 PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"

void Aa1ChartNavHelperUIDialog::PlotSettings( wxCommandEvent& event )
{
    m_aa1chartnavhelper_pi.ShowPlotSettings();
}

void Aa1ChartNavHelperUIDialog::EnableCompassChanged( wxCommandEvent& event )
{
    m_aa1chartnavhelper_pi.SetShowCompass(m_cbEnableCompass->GetValue());
    RequestRefresh( m_aa1chartnavhelper_pi.m_parent_window );
}

void Aa1ChartNavHelperPlotSettingsDialog::About( wxCommandEvent& event )
{
    wxString msg0(
        _("\n\
Created(/Assembled) by Andras A. - Chart Nav Helper, \n\
This plugin adds a compass rose and rulers on the viewed map - \n\
The compass rose and ruler will appear on the printed maps also, and by this it \n\
will make the printed maps useful in manual navigation -\n\n\
It is based on code from The World Magnetic Model Plugin which was written by Pavel Kalian \
and extended by Sean D'Epagnier, and some other OpenCPN code segments from various parts of the code base.\n\n\
This plugin uses the WMM plugin to get information about variation at the center of the compass rose.  \n\
It has a configuration dialog where the year at which the variation is expected to be calculated can be set.\n\
The variation can be set to a fixed value (unfortunately the user provided variation value was not accessible by this plugin)\n\n\
NOTES: The intent for building this plugin was to help the training and practice of navigation skills at USPS.\n\
(The compass rose can be moved by the mouse by dragging it at the center of the compass rose) "));

    wxMessageDialog dlg( this, msg0, _("Chart Nav Helper Plugin"), wxOK );

    dlg.ShowModal();
}

//---------------------------------------------------------------------------------------------------------
//
//        PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

aa1chartnavhelper_pi::aa1chartnavhelper_pi(void *ppimgr)
    : opencpn_plugin_114(ppimgr),
    //m_bShowPlot(false),
    m_bShowCompass(false)
    //m_bComputingPlot(false)
{
    // Create the PlugIn icons
    initialize_images();
}

int aa1chartnavhelper_pi::Init(void)
{
    AddLocaleCatalog( _T("opencpn-aa1chartnavhelper_pi") );

    //byAA10182017:
    m_useDefaultMagVar = false;
    m_year_for_MagVar = 2002;
    m_config_default__magVarYdelta = -15.25; //USPS training material default (equivalent with: 15.15 W)
    //byAA10182017.
    
    // Set some default private member parameters
    m_aa1chartnavhelper_dialog_x = 0;
    m_aa1chartnavhelper_dialog_y = 0;

    m_preffered_compass_rose_cx = 0;
    m_preffered_compass_rose_cy = 0;

    m_moving_with_mouse_click_just_finihed_moving = false;
    m_moving_with_mouse_click = false;

    ::wxDisplaySize(&m_display_width, &m_display_height);

    //    Get a pointer to the opencpn display canvas, to use as a parent for the POI Manager dialog
    m_parent_window = GetOCPNCanvasWindow();

    //    Get a pointer to the opencpn configuration object
    m_pconfig = GetOCPNConfigObject();

    //    And load the configuration items
    LoadConfig();

    SetShowCompass(m_config_default_ShowCompass);
    
    // = new wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    m_shareLocn =*GetpSharedDataLocation() +
    _T("plugins") + wxFileName::GetPathSeparator() +
    _T("aa1chartnavhelper_pi") + wxFileName::GetPathSeparator() +
    _T("data") + wxFileName::GetPathSeparator();
    
    //    AA1 initialization
    /* Memory allocation */

    int ret_flag =  (WANTS_OVERLAY_CALLBACK |
    WANTS_OPENGL_OVERLAY_CALLBACK |
    WANTS_CURSOR_LATLON     |
    WANTS_TOOLBAR_CALLBACK  |
    WANTS_NMEA_EVENTS       |
    WANTS_PREFERENCES       |
    WANTS_CONFIG            |
    WANTS_PLUGIN_MESSAGING  |
    WANTS_MOUSE_EVENTS
    );
    
    if(m_bShowToolbarIcon){
        //    This PlugIn needs a toolbar icon, so request its insertion
        m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_AA1ChartNavHelper, _img_AA1ChartNavHelper, wxITEM_NORMAL,
                                            _("Chart Nav Helper"), _T(""), NULL, AA1CHARTNAVHELPER_TOOL_POSITION, 0, this);
        
        SetIconType();          // SVGs allowed if not showing live icon
        
        ret_flag |= INSTALLS_TOOLBAR_TOOL;
    }

    m_pAa1ChartNavHelperDialog = NULL;

    return ret_flag;
}

bool aa1chartnavhelper_pi::DeInit(void)
{
    //    Record the dialog position
     if (NULL != m_pAa1ChartNavHelperDialog)
     {
         wxPoint p = m_pAa1ChartNavHelperDialog->GetPosition();
         SetAa1ChartNavHelperDialogX(p.x);
         SetAa1ChartNavHelperDialogY(p.y);
 
         m_pAa1ChartNavHelperDialog->Close();
         delete m_pAa1ChartNavHelperDialog;
         m_pAa1ChartNavHelperDialog = NULL;
     }
    SaveConfig();
    RemovePlugInTool(m_leftclick_tool_id);

    //delete pFontSmall;
    return true;
}

int aa1chartnavhelper_pi::GetAPIVersionMajor()
{
    return MY_API_VERSION_MAJOR;
}

int aa1chartnavhelper_pi::GetAPIVersionMinor()
{
    return MY_API_VERSION_MINOR;
}

int aa1chartnavhelper_pi::GetPlugInVersionMajor()
{
    return PLUGIN_VERSION_MAJOR;
}

int aa1chartnavhelper_pi::GetPlugInVersionMinor()
{
    return PLUGIN_VERSION_MINOR;
}

wxBitmap *aa1chartnavhelper_pi::GetPlugInBitmap()
{
    return _img_AA1ChartNavHelper_pi;
}

wxString aa1chartnavhelper_pi::GetCommonName()
{
    return _("Chart Nav Helper");
}


wxString aa1chartnavhelper_pi::GetShortDescription()
{
    return _("Chart Nav Helper PlugIn for OpenCPN");
}

wxString aa1chartnavhelper_pi::GetLongDescription()
{
    return _("Chart Nav Helper PlugIn for OpenCPN\n\
adds to the screen (and printouts) traditional navigation chart features \n\
like a compass rose and rulers at the side of the map.\n\
Using these features a printed map can help in manual navigation.\n\n\
NOTES: The intent for building this plugin was to help the training and practice \n\
of navigation and piloting skills at USPS.\n\n\
USAGE: The compass rose can be moved to a convenient location by using the mouse to dragging the compass rose.\n\
The Compass rose can be grabbed at/around the center of the compass rose.");
}


int aa1chartnavhelper_pi::GetToolbarToolCount(void)
{
    return 1;
}

void aa1chartnavhelper_pi::SetColorScheme(PI_ColorScheme cs)
{
    if (NULL == m_pAa1ChartNavHelperDialog)
        return;
    DimeWindow(m_pAa1ChartNavHelperDialog);
}

void aa1chartnavhelper_pi::SetIconType()
{
    if(m_bShowLiveToolbarIcon){
        SetToolbarToolBitmaps(m_leftclick_tool_id, _img_AA1ChartNavHelper, _img_AA1ChartNavHelper);
        SetToolbarToolBitmapsSVG(m_leftclick_tool_id, _T(""), _T(""), _T(""));
    }
    else{
        SetToolbarToolBitmaps(m_leftclick_tool_id, _img_AA1ChartNavHelper, _img_AA1ChartNavHelper);
        SetToolbarToolBitmapsSVG(m_leftclick_tool_id, _T(""), _T(""), _T(""));

        /*
        wxString normalIcon = m_shareLocn + _T("aa1chartnavhelper_pi.svg");
        wxString toggledIcon = m_shareLocn + _T("aa1chartnavhelper_pi.svg");
        wxString rolloverIcon = m_shareLocn + _T("aa1chartnavhelper_pi.svg");
        
        SetToolbarToolBitmapsSVG(m_leftclick_tool_id, normalIcon, rolloverIcon, toggledIcon);
         */
    }
    
}


void aa1chartnavhelper_pi::RearrangeWindow()
{
    if (NULL == m_pAa1ChartNavHelperDialog)
        return;
    
    SetColorScheme(PI_ColorScheme());

    m_pAa1ChartNavHelperDialog->Fit();

#ifdef __WXMSW__
    //UGLY!!!!!!! On Windows XP the transparent window is not refreshed properly in OpenGL mode at least on the Atom powered netbooks, so we have to disable transparency.
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;
    bool gl = true;
    if(pConf)
    {
        pConf->SetPath(_T("/Settings"));
        pConf->Read(_T("OpenGL"), &gl, false);
        pConf = NULL;
    }
    if (!(gl && wxPlatformInfo::Get().GetOSMajorVersion() == 5 && wxPlatformInfo::Get().GetOSMinorVersion() == 1))
#endif
        if (m_pAa1ChartNavHelperDialog->CanSetTransparent())
            m_pAa1ChartNavHelperDialog->SetTransparent(m_iOpacity);
}

void aa1chartnavhelper_pi::OnToolbarToolCallback(int id)
{
    if(NULL == m_pAa1ChartNavHelperDialog)
    {
        m_pAa1ChartNavHelperDialog = new Aa1ChartNavHelperUIDialog(*this, m_parent_window);
        wxFont *pFont = OCPNGetFont(_T("Dialog"), 0);
        m_pAa1ChartNavHelperDialog->SetFont(*pFont);
        
        m_pAa1ChartNavHelperDialog->Move(wxPoint(m_aa1chartnavhelper_dialog_x, m_aa1chartnavhelper_dialog_y));
    }

    RearrangeWindow();

    m_pAa1ChartNavHelperDialog->Show(!m_pAa1ChartNavHelperDialog->IsShown());
    m_pAa1ChartNavHelperDialog->Layout();     // Some platforms need a re-Layout at this point (gtk, at least)
    if (m_pAa1ChartNavHelperDialog->IsShown())
        SendPluginMessage(_T("AA1CHARTNAVHELPER_WINDOW_SHOWN"), wxEmptyString);
    else
        SendPluginMessage(_T("AA1CHARTNAVHELPER_WINDOW_HIDDEN"), wxEmptyString);

    wxPoint p = m_pAa1ChartNavHelperDialog->GetPosition();
    m_pAa1ChartNavHelperDialog->Move(0,0);      // workaround for gtk autocentre dialog behavior
    m_pAa1ChartNavHelperDialog->Move(p);
    
#ifdef __OCPN__ANDROID__
    m_pAa1ChartNavHelperDialog->CentreOnScreen();
    m_pAa1ChartNavHelperDialog->Move(-1,0);
#endif    
    
}

void aa1chartnavhelper_pi::RenderOverlayBoth(wxDC *dc, PlugIn_ViewPort *vp)
{
    if (m_bShowCompass)
    {
		//byAA10152017:
		plotPaperChartComponents(dc, vp);
		//byAA10152017.
    }
}

//byAA10152017:
#define ANGLE_OFFSET 90

double rad2deg(double angle)
{
    return angle*180.0/M_PI;
}
double deg2rad(double angle)
{
    return angle/180.0*M_PI;
}

/**This method is not used at this version. It is drawing a Compuss Star with N, NE, E, ... direction pointers only
   we do not have the OpenGL version of this method, that may come later if this Compass Rose/Star code will remain in the code.
 */
void aa1chartnavhelper_pi::DrawCompassRose(wxDC* dc, int cx, int cy, int radius, int startangle, bool showlabels)
{
    if (dc == NULL)
    {
        //at thsi veriosn this compass rose is not beeing used... we do not yet have an OpenGL implementation for this.
    }

    wxPoint pt, points[3];
    wxString Value;
    int width, height;
    wxString CompassArray[] = {_("N"),_("NE"),_("E"),_("SE"),_("S"),_("SW"),_("W"),_("NW"),_("N")};
    
    
    wxFont aFont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    dc->SetFont(aFont);
    
    wxColour cl;
    wxPen* pen;
    GetGlobalColor(_T("UINFM"), &cl);
    pen = wxThePenList->FindOrCreatePen( cl, 1, wxPENSTYLE_SOLID );
    wxBrush* b2 = wxTheBrushList->FindOrCreateBrush( cl );

    dc->SetPen(*pen);
    dc->SetTextForeground(cl);
    
    GetGlobalColor(_T("DASHL"), &cl);
    wxBrush* b1 = wxTheBrushList->FindOrCreateBrush( cl );
    
    dc->SetBrush(*b2);
    
    int offset = 0;
    for(double tmpangle = startangle - ANGLE_OFFSET;
        tmpangle < startangle + 360 - ANGLE_OFFSET; tmpangle+=90)
    {
        if (showlabels)
        {
            Value = CompassArray[offset];
            dc->GetTextExtent(Value, &width, &height, 0, 0, &aFont);
            double x = width/2;
            long double anglefortext = tmpangle - rad2deg(asin((x/radius)));
            pt.x = cx + radius * cos(deg2rad(anglefortext));
            pt.y = cy + radius * sin(deg2rad(anglefortext));
            dc->DrawRotatedText(Value, pt.x, pt.y, -90 - tmpangle);
            Value = CompassArray[offset+1];
            dc->GetTextExtent(Value, &width, &height, 0, 0, &aFont);
            x = width/2;
            anglefortext = tmpangle - rad2deg(asin((x/radius))) + 45;
            pt.x = cx + radius * cos(deg2rad(anglefortext));
            pt.y = cy + radius * sin(deg2rad(anglefortext));
            dc->DrawRotatedText(Value, pt.x, pt.y, -135 - tmpangle);
        }
        points[0].x = cx;
        points[0].y = cy;
        points[1].x = cx + radius * 0.15 * cos(deg2rad(tmpangle));
        points[1].y = cy + radius * 0.15 * sin(deg2rad(tmpangle));
        points[2].x = cx + radius * 0.6 * cos(deg2rad(tmpangle+45));
        points[2].y = cy + radius * 0.6 * sin(deg2rad(tmpangle+45));
        dc->DrawPolygon(3, points, 0, 0);
        points[1].x = cx + radius * 0.15 * cos(deg2rad(tmpangle+90));
        points[1].y = cy + radius * 0.15 * sin(deg2rad(tmpangle+90));
        dc->SetBrush(*b1);
        dc->DrawPolygon(3, points, 0, 0);
        points[2].x = cx + radius * 0.8 * cos(deg2rad(tmpangle));
        points[2].y = cy + radius * 0.8 * sin(deg2rad(tmpangle));
        points[1].x = cx + radius * 0.15 * cos(deg2rad(tmpangle+45));
        points[1].y = cy + radius * 0.15 * sin(deg2rad(tmpangle+45));
        dc->DrawPolygon(3, points, 0, 0);
        points[2].x = cx + radius * 0.8 * cos(deg2rad(tmpangle+90));
        points[2].y = cy + radius * 0.8 * sin(deg2rad(tmpangle+90));
        dc->SetBrush(*b2);
        dc->DrawPolygon(3, points, 0, 0);
        offset += 2;
    }
    
    dc->SetPen(*pen);
    dc->SetTextForeground(cl);
    dc->SetBrush(*wxTRANSPARENT_BRUSH);
    dc->DrawCircle(cx, cy, radius);
}

const int SMALL_TICK_LEN = 4;
const int MED_TICK_LEN = 6;
const int BIG_TICK_LEN = 10;

// utility method to hide differences between OpenGL and DC drawing
void mySetPen(wxDC* dc, wxPen* pen)
{
    if (dc)
    {
        dc->SetPen(*pen);
    }
    else
    {
        glLineWidth(pen->GetWidth());
    }
}

// utility method to hide differences between OpenGL and DC drawing
void mySetTextForground(wxDC* dc, wxColour& cl)
{
    if (dc)
    {
        dc->SetTextForeground(cl);
    }
    else
    {
        glColor4ub(cl.Red(), cl.Green(), cl.Blue(), cl.Alpha());
    }
}

// utility method to hide differences between OpenGL and DC drawing
void mySetFont(wxDC* dc, wxFont& aFont, TexFont& aTexFont)
{
    if (dc)
    {
        dc->SetFont(aFont);
    }
    else
    {
        aTexFont.Build(aFont);
    }
}

// utility method to hide differences between OpenGL and DC drawing
void mySetBrush(wxDC* dc, const wxBrush& b2)
{
    if (dc)
    {
        dc->SetBrush(b2);
    }
    else
    {
        //???
    }
}

// utility method to hide differences between OpenGL and DC drawing
void myDrawLine(wxDC* dc, wxPoint& pt_tick_s, wxPoint& pt_tick_e)
{
    if(dc)
        dc->DrawLine(pt_tick_s.x, pt_tick_s.y, pt_tick_e.x, pt_tick_e.y);
    else {
        
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

        glBegin(GL_LINES);
        glVertex2i(pt_tick_s.x, pt_tick_s.y);
        glVertex2i(pt_tick_e.x, pt_tick_e.y);
        glEnd();
    }
}

// utility method to hide differences between OpenGL and DC drawing
void myDrawLine(wxDC* dc, int x1, int y1, int x2, int y2)
{
    wxPoint p_s = wxPoint(x1, y1);
    wxPoint p_e = wxPoint(x2, y2);
    myDrawLine(dc, p_s, p_e);
}

// utility method to hide differences between OpenGL and DC drawing
void myDrawCircle(wxDC* dc, int cx, int cy, int radius)
{
    if(dc)
    {
        dc->DrawCircle(cx, cy, radius);
    }
    else
    {
        int num_segments = 360;
        glBegin(GL_LINE_LOOP);
        for(int ii = 0; ii < num_segments; ii++)
        {
            float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle
            
            float x = radius * cosf(theta);//calculate the x component
            float y = radius * sinf(theta);//calculate the y component
            
            glVertex2f(x + cx, y + cy);//output vertex
        }
        glEnd();
    }
    
}

// utility method to hide differences between OpenGL and DC drawing
void myDrawText(wxDC* dc, const wxString& text, int cx, int cy, bool centered, wxFont& aFont, TexFont& aTexFont)
{
    if(dc) {
        int w=0;
        int h=0;
        if (centered)
        {
            dc->GetTextExtent( text, &w, &h);
        }
        dc->DrawText(text, cx - w/2, cy - h/2);
    } else {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        
        int w = 0;
        int h = 0;
        if (centered)
        {
            aTexFont.GetTextExtent( text, &w, &h);
        }
        
        glEnable(GL_TEXTURE_2D);
        aTexFont.RenderString(text, cx - w/2, cy - h/2);
        glDisable(GL_TEXTURE_2D);
        
        glDisable(GL_BLEND);
    }
}

// utility method to hide differences between OpenGL and DC drawing
/**this is a sompler text trawing rutine, to abstract the text drawing details of the DC and OpenGL
   this routine is used when writing the numbers around the compass rose, AND this routin is also used by the
   DC drawing od the text on the compass rose.
 */
void myDrawTextAtAngleRelToCenter(wxDC* dc,
                                   const wxString& text,
                                   int cx, int cy, double VarDispPosFacotr,
                                   int l_angle, int radius, int startangle, double magVar,
                                   wxFont& aFont, TexFont& aTexFont)
{
    wxPoint pt;
    int width, height;
    if(dc)
    {
        dc->GetTextExtent(text, &width, &height, 0, 0, &aFont);
        double x = width/2;
        long double anglefortext = l_angle + startangle - rad2deg(asin((x/(radius*VarDispPosFacotr))));
        pt.x = cx + (radius * VarDispPosFacotr) * cos(deg2rad(anglefortext + magVar));
        pt.y = cy + (radius * VarDispPosFacotr) * sin(deg2rad(anglefortext + magVar));
        dc->DrawRotatedText(text, pt.x, pt.y, -90 - l_angle - startangle - magVar);
    }
    else
    {
        glPushMatrix();

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        
        aTexFont.GetTextExtent( text, &width, &height);

        double x = width/2;
        long double anglefortext = l_angle + startangle - rad2deg(asin((x/(radius*VarDispPosFacotr))));
        pt.x = cx + (radius * VarDispPosFacotr) * cos(deg2rad(anglefortext + magVar));
        pt.y = cy + (radius * VarDispPosFacotr) * sin(deg2rad(anglefortext + magVar));

        glTranslatef(pt.x /*- x*/, pt.y /*- height/2*/, 0);
        glRotated(anglefortext + magVar + 90, 0.0, 0.0, 1.0);

        glEnable(GL_TEXTURE_2D);
        //aTexFont.RenderString(msg, r.x - w/2, r.y - h/2);
        aTexFont.RenderString(text, 0, 0);//pt.x - x, pt.y - height/2);
        glDisable(GL_TEXTURE_2D);
        
        glDisable(GL_BLEND);
        
        glPopMatrix();
    }
}

// utility method to hide differences between OpenGL and DC drawing
/** this is a method which .. with difficulty .. draws the text on the compass rose
 there are 3-lines of text, the last line is a static text "MAGNETIC"
 The difference between the DC and OpenGL drawing of the texts is that in DC drowing the "MAGNETIC" word is
 drawn upside down, while in OpenGL mode it is drawn right side up... this remains a signature of the map.
 In printed maps will be possible to tell what rendering was used nased on this minor clue.
 
 This method uses the simpler text drawung methoud for the DC drawing but it uses a different implementation
 for the OpenGL case.
 */
void drawInfoText(wxDC* dc,
                  int cx, int cy,
                  int l_angle, int lr_angle, int radius, int startangle, double magVar,
                  wxFont& aFont, TexFont& aTexFont,
                  const wxString& line1, const wxString& line2, const wxString& line3)
{
    if(dc)
    {
         if ((lr_angle == 0) || (lr_angle == 360) )
         {
             {
                 double VarDispPosFacotr = 9.0/10.0;
                 myDrawTextAtAngleRelToCenter(dc, line1, cx, cy, VarDispPosFacotr,
                 l_angle, radius, startangle, magVar,
                 aFont, aTexFont);
             }
             {
                 double VarDispPosFacotr = 9.0/10.0;
                 myDrawTextAtAngleRelToCenter(dc, line2, cx, cy, VarDispPosFacotr,
                 l_angle, radius, startangle, magVar,
                 aFont, aTexFont);
             }
         }
         if (lr_angle == 180)
         {
             {
                 double VarDispPosFacotr = 0.8;
                 myDrawTextAtAngleRelToCenter(dc, line3, cx, cy, VarDispPosFacotr,
                 l_angle, radius, startangle, magVar,
                 aFont, aTexFont);
             }
         }
    }
    else
    {
        if ((lr_angle == 0) || (lr_angle == 360) )
        {
            int width1, height1;
            int width2, height2;
            int width3, height3;

            glPushMatrix();
            
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            
            aTexFont.GetTextExtent( line1, &width1, &height1);
            aTexFont.GetTextExtent( line2, &width2, &height2);
            aTexFont.GetTextExtent( line3, &width3, &height3);

            double line1_y_delta = -radius +25;
            double line2_y_delta = line1_y_delta + height1 + 5;
            double line3_y_delta = -line1_y_delta -25;

            glTranslatef(cx, cy, 0);

            glRotated(-1 * (-90 - l_angle - startangle - magVar), 0.0, 0.0, 1.0);
            
            glEnable(GL_TEXTURE_2D);
            
            aTexFont.RenderString(line1, -width1/2 , line1_y_delta);
            aTexFont.RenderString(line2, -width2/2 , line2_y_delta);
            aTexFont.RenderString(line3, -width3/2 , line3_y_delta);

            glDisable(GL_TEXTURE_2D);
            
            glDisable(GL_BLEND);
            
            glPopMatrix();
        }
    }
}

/** this is the drawing method for the compus rose.. it is actually called twice to draw two circles with tick marks
 the first is the True copuss rose, the second is the MAcnetic one which is rotated by the Mag variations.
 */
void aa1chartnavhelper_pi::DrawCompassRoseTicks(wxDC* dc, int cx, int cy, int radius, int startangle, bool showlabels,
                          double magVar, double magVarYdelta, bool markAt90)
{
    wxPoint pt, pt_tick_s, pt_tick_e;
    wxString Value;
    //int width, height;
    int tick_len = SMALL_TICK_LEN;
    
    TexFont aTexFont;
    wxFont aFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    mySetFont(dc, aFont, aTexFont);
    
    wxColour cl;
    wxPen* pen;
    GetGlobalColor(_T("UINFM"), &cl);
    pen = wxThePenList->FindOrCreatePen( cl, 1, wxPENSTYLE_SOLID );

    mySetPen(dc, pen);
    mySetTextForground(dc, cl);
    mySetBrush(dc, *wxTRANSPARENT_BRUSH);
    
    for(int l_angle = - ANGLE_OFFSET;
        l_angle < 360 - ANGLE_OFFSET; l_angle+=1)
    {
        //the real angle is lr_angle ... the l_angle is assuming the way drowing coords are on Y axis
        int lr_angle = l_angle + ANGLE_OFFSET;

        bool showlabels_i = false;
        if ((lr_angle % 10) == 0)
        {
            tick_len = BIG_TICK_LEN;
        }
        else
        {
            if ((lr_angle % 5) == 0)
            {
                tick_len = MED_TICK_LEN;
            }
            else
            {
                tick_len = SMALL_TICK_LEN;
            }
        }
        
        if ((lr_angle % 30) == 0)
        {
            if (showlabels)
            {
                showlabels_i = true;
            }
        }
        
        int extra_tick_s = 0;
        int extra_tick_e = 0;
        if (markAt90)
        {
            if ((lr_angle == 0) || (lr_angle == 90) || (lr_angle == 180) || (lr_angle == 270) || (lr_angle == 360))
            {
                extra_tick_s = 0;
                extra_tick_e = 10;
            }
            if ((lr_angle == 0) || (lr_angle == 360) || (lr_angle == 180) )
            {
                wxString line1 = wxString::Format("as of %d/1/1", m_year_for_MagVar);
                wxString marVarS = AngleToText(magVar);
                wxString line2 = wxString::Format("Var: %s AnualChg: %.2f",marVarS,magVarYdelta);
                wxString line3 = wxString::Format("MAGNETIC");
                
                drawInfoText(dc, cx, cy,
                             l_angle, lr_angle, radius, startangle, magVar,
                             aFont, aTexFont,
                             line1, line2, line3);
            }
        }
        
        if (showlabels_i)
        {
            double VarDispPosFacotr = 1.0;
            Value = wxString::Format("%d",lr_angle);
            
            myDrawTextAtAngleRelToCenter(dc, Value, cx, cy, VarDispPosFacotr,
                                         l_angle, radius, startangle, magVar,
                                         aFont, aTexFont);
            
            /*
            dc->GetTextExtent(Value, &width, &height, 0, 0, &aFont);
            double x = width/2;
            long double anglefortext = l_angle + startangle - rad2deg(asin((x/radius)));
            pt.x = cx + radius * cos(deg2rad(anglefortext + magVar));
            pt.y = cy + radius * sin(deg2rad(anglefortext + magVar));
            dc->DrawRotatedText(Value, pt.x, pt.y, -90 - l_angle - startangle - magVar);
            */
        }
        
        //draw ticks
        long double anglefortick = l_angle + startangle;
        pt_tick_s.x = cx + (radius + extra_tick_s) * cos(deg2rad(anglefortick + magVar));
        pt_tick_s.y = cy + (radius + extra_tick_s) * sin(deg2rad(anglefortick + magVar));
        
        pt_tick_e.x = cx + (radius + tick_len + extra_tick_e) * cos(deg2rad(anglefortick + magVar));
        pt_tick_e.y = cy + (radius + tick_len + extra_tick_e) * sin(deg2rad(anglefortick + magVar));
        
        mySetBrush(dc, *wxTRANSPARENT_BRUSH);
        myDrawLine(dc, pt_tick_s, pt_tick_e);
    }
    //draw center cross
    if (markAt90)
    {
        int cross_len = 10;
        
        long double angleforcross = 0;
        pt_tick_s.x = cx + (cross_len) * cos(deg2rad(angleforcross + magVar));
        pt_tick_s.y = cy + (cross_len) * sin(deg2rad(angleforcross + magVar));
        
        pt_tick_e.x = cx + (-cross_len) * cos(deg2rad(angleforcross + magVar));
        pt_tick_e.y = cy + (-cross_len) * sin(deg2rad(angleforcross + magVar));
        
        mySetBrush(dc, *wxTRANSPARENT_BRUSH);
        myDrawLine(dc, pt_tick_s, pt_tick_e);

        angleforcross = 90;
        pt_tick_s.x = cx + (cross_len) * cos(deg2rad(angleforcross + magVar));
        pt_tick_s.y = cy + (cross_len) * sin(deg2rad(angleforcross + magVar));
        
        pt_tick_e.x = cx + (-cross_len) * cos(deg2rad(angleforcross + magVar));
        pt_tick_e.y = cy + (-cross_len) * sin(deg2rad(angleforcross + magVar));
        
        mySetBrush(dc, *wxTRANSPARENT_BRUSH);
        myDrawLine(dc, pt_tick_s, pt_tick_e);

        //extra lines
        int small_radius = radius/2;
        int large_radius = radius-20;
        
        for (int l_angleforextraline = 0 - ANGLE_OFFSET;
             l_angleforextraline < 360 - ANGLE_OFFSET;
             l_angleforextraline +=90)
        {
            int lr_angleforextraline = l_angleforextraline + ANGLE_OFFSET;
            pt_tick_s.x = cx + (small_radius) * cos(deg2rad(l_angleforextraline + magVar));
            pt_tick_s.y = cy + (small_radius) * sin(deg2rad(l_angleforextraline + magVar));
            
            int l_large_radius = large_radius;
            if ((lr_angleforextraline == 0) || (lr_angleforextraline == 180))
            {
                l_large_radius = small_radius + radius/8;
            }
            
            pt_tick_e.x = cx + (l_large_radius) * cos(deg2rad(l_angleforextraline + magVar));
            pt_tick_e.y = cy + (l_large_radius) * sin(deg2rad(l_angleforextraline + magVar));
        
            mySetBrush(dc, *wxTRANSPARENT_BRUSH);
            myDrawLine(dc, pt_tick_s, pt_tick_e);
        }
    }
    
    mySetBrush(dc, *wxTRANSPARENT_BRUSH);
    myDrawCircle(dc, cx, cy, radius);
}

void aa1chartnavhelper_pi::getCompassRoseCenter(wxDC *dc, PlugIn_ViewPort *vp, int& cx, int& cy)
{
    if(dc) {
        // DC drawing
    } else {
        // OpenGL drawing
    }
    
    if ((m_preffered_compass_rose_cx != 0) && (m_preffered_compass_rose_cy != 0))
    {
    	if (m_moving_with_mouse_click || m_moving_with_mouse_click_just_finihed_moving)
    	{
            int move_boundary = 100;
    		if (m_preffered_compass_rose_cx < move_boundary)
    		{
    			m_preffered_compass_rose_cx = move_boundary;
    		}
    		if (m_preffered_compass_rose_cx > (vp->rv_rect.width - move_boundary))
    		{
    			m_preffered_compass_rose_cx = (vp->rv_rect.width - move_boundary);
    		}
    		if (m_preffered_compass_rose_cy < move_boundary)
    		{
    			m_preffered_compass_rose_cy = move_boundary;
    		}
    		if (m_preffered_compass_rose_cy > (vp->rv_rect.height - move_boundary))
    		{
    			m_preffered_compass_rose_cy = (vp->rv_rect.height - move_boundary);
    		}

    		m_moving_with_mouse_click_just_finihed_moving = false;
    	}

        cx = m_preffered_compass_rose_cx;
        cy = m_preffered_compass_rose_cy;
    }
    else
    {
        //figure out the coordinates of the center of the left upper quarter of the viewport
        wxPoint r1, r2;
        GetCanvasPixLL(vp, &r1, vp->lat_max, vp->lon_max);
        GetCanvasPixLL(vp, &r2, vp->clat, vp->clon);
        
        cx = (r1.x - r2.x) / 2;
        cy = (r2.y - r1.y) / 2 + 30+40; //+xx is to push the rose below the toolbar
    }
    
    m_current_compass_rose_cx = cx;
    m_current_compass_rose_cy = cy;
}

bool aa1chartnavhelper_pi::plotPaperChartComponents(wxDC *dc, PlugIn_ViewPort *vp)
{
    int cx, cy;
    getCompassRoseCenter(dc, vp, cx, cy);
    
    double angle = - vp->rotation;
    
    //get variation at the center of the rose ... TODO: get rid of hardcoded year: 2017/1/1
    double magVar;
    double magVarYdelta;
    
    //calculate the Lat/lon for the new graphic center coordonate of the cmpas rose --> so that we can get precise magVar.
    double c_lat;
    double c_lon;
    wxPoint c_p;
    c_p.x = cx;
    c_p.y = cy;
    GetCanvasLLPix(vp, c_p, &c_lat, &c_lon);    
    getVariationForLatLon(c_lat, c_lon, magVar, magVarYdelta);
    
    //DrawCompassRose(dc, cx,cy,80,angle+magVar, true);
    DrawCompassRoseTicks(dc, cx,cy,160+40,angle, true, 0, 0, false);
    DrawCompassRoseTicks(dc, cx,cy,125+40,angle, true, magVar, magVarYdelta, true);
    
    //draw the rulers
    DrawRulers(dc, vp);
    
    return true;
}

#define RULER_SMALLEST_STEP 0.001
#define RULER_WIDTH_SMALL 5 //should be 2 or 4
#define RULER_WIDTH_MICRO 2 //should be 2 or 4
#define RULER_DELTA_FROM_BOTTOM 22
#define RULER_LEBEL_HIGHT 11
#define RESOLUTION_LIMIT_FOR_MICRO_RULER 100 //if more then this many minor ruler lines are then dont draw micro rulers

/* @ChartCanvas::CalcGridSpacing
 **
 ** Calculate the major and minor spacing between the lat/lon grid
 **
 ** @param [r] WindowDegrees [float] displayed number of lat or lan in the window
 ** @param [w] MajorSpacing [float &] Major distance between grid lines
 ** @param [w] MinorSpacing [float &] Minor distance between grid lines
 ** @return [void]
 */
void CalcGridSpacing( float view_scale_ppm, float& MajorSpacing, float&MinorSpacing, float&MicroSpacing )
{
    // table for calculating the distance between the grids
    // [0] view_scale ppm
    // [1] spacing between major grid lines in degrees
    // [2] spacing between minor grid lines in degrees
    const float lltab[][3] =
    {   {  0.0f,    90.0f,           30.0f    },
        { .000001f, 45.0f,           15.0f    },
        { .0002f,   30.0f,           10.0f    },
        { .0003f,   10.0f,            2.0f    },
        { .0008f,    5.0f,            1.0f    },
        { .001f,     2.0f,           30.0f  / 60.0f },
        { .003f,     1.0f,           20.0f  / 60.0f },
        { .006f,     0.5f,           10.0f  / 60.0f },
        { .03f,     15.0f  / 60.0f,   5.0f  / 60.0f },
        { .01f,     10.0f  / 60.0f,   2.0f  / 60.0f },
        { .06f,      5.0f  / 60.0f,   1.0f  / 60.0f },
        { .1f,       2.0f  / 60.0f,   1.0f  / 60.0f },
        { .4f,       1.0f  / 60.0f,   0.5f  / 60.0f },// we do not want "xx yy,zz" values, "xx yy,z" is adequat for manual charting
        //{ .4f,       1.0f  / 60.0f,   1.0f  / 60.0f },
        { .6f,       0.5f  / 60.0f,   0.1f  / 60.0f },// we do not want "xx yy,zz" values, "xx yy,z" is adequat for manual charting
        //{ .6f,       1.0f  / 60.0f,   1.0f  / 60.0f },
        { 1.0f,      0.2f  / 60.0f,   0.1f  / 60.0f },// we do not want "xx yy,zz" values, "xx yy,z" is adequat for manual charting
        //{ 1.0f,      1.0f  / 60.0f,   1.0f  / 60.0f },
        //{ 1e10f,     0.1f  / 60.0f,   0.05f / 60.0f } // we do not want "xx yy,zz" values, "xx yy,z" is adequat for manual charting
        { 1e10f,     0.1f  / 60.0f,   0.1f  / 60.0f }
    };
    
    unsigned int tabi;
    for( tabi = 0; tabi < (sizeof lltab) / (sizeof *lltab); tabi++ )
        if( view_scale_ppm < lltab[tabi][0] )
            break;
    MajorSpacing = lltab[tabi][1]; // major latitude distance
    MinorSpacing = lltab[tabi][2]; // minor latitude distance
    //MinorSpacing = MajorSpacing / 10;
    MicroSpacing = MinorSpacing / 5;
    return;
}
/* @ChartCanvas::CalcGridText *************************************
 **
 ** Calculates text to display at the major grid lines
 **
 ** @param [r] latlon [float] latitude or longitude of grid line
 ** @param [r] spacing [float] distance between two major grid lines
 ** @param [r] bPostfix [bool] true for latitudes, false for longitudes
 **
 ** @return
 */

wxString CalcGridText( float latlon, float spacing, bool bPostfix )
{
    int deg = (int) fabs( latlon ); // degrees
    float min = fabs( ( fabs( latlon ) - deg ) * 60.0 ); // Minutes
    char postfix;
    
    // calculate postfix letter (NSEW)
    if( latlon > 0.0 ) {
        if( bPostfix ) {
            postfix = 'N';
        } else {
            postfix = 'E';
        }
    } else if( latlon < 0.0 ) {
        if( bPostfix ) {
            postfix = 'S';
        } else {
            postfix = 'W';
        }
    } else {
        postfix = ' '; // no postfix for equator and greenwich
    }
    // calculate text, display minutes only if spacing is smaller than one degree
    
    wxString ret;
    if( spacing >= 1.0 ) {
        ret.Printf( _T("%3d%c %c"), deg, 0x00b0, postfix );
    } else if( spacing >= ( 1.0 / 60.0 ) ) {
        ret.Printf( _T("%3d%c%02.0f %c"), deg, 0x00b0, min, postfix );
    } else {
        //ret.Printf( _T("%3d%c%02.2f %c"), deg, 0x00b0, min, postfix );
        ret.Printf( _T("%3d%c%02.1f %c"), deg, 0x00b0, min, postfix );
    }
    
    return ret;
}

/* @ChartCanvas::GridDraw *****************************************
 **
 ** Draws major and minor Lat/Lon Grid on the chart
 ** - distance between Grid-lm ines are calculated automatic
 ** - major grid lines will be across the whole chart window
 ** - minor grid lines will be 10 pixel at each edge of the chart window.
 **
 ** @param [w] dc [wxDC&] the wx drawing context
 **
 ** @return [void]
 ************************************************************************/
void aa1chartnavhelper_pi::DrawRulers(wxDC *dc, PlugIn_ViewPort *vp)
{
    //if ( fabs( vp->rotation ) < 1e-5 )
    //    return;
    TexFont aTexFont;

    double nlat, elon, slat, wlon;
    float lat, lon;
    float dlat, dlon;
    float gridlatMajor, gridlatMinor, gridlatMicro, gridlonMajor, gridlonMinor, gridlonMicro;
    wxCoord w, h;

    wxColour cl;
    GetGlobalColor(_T("DASHR"), &cl);

    wxPen GridPen( cl, 1, wxPENSTYLE_SOLID );
    mySetPen(dc, &GridPen);
    mySetTextForground(dc, cl);
    mySetBrush(dc, *wxTRANSPARENT_BRUSH);
    
    wxFont aFont( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL );
    mySetFont(dc, aFont, aTexFont);
    
    wxPoint minC, maxC;
    GetCanvasPixLL(vp, &maxC, vp->lat_max, vp->lon_max);
    GetCanvasPixLL(vp, &minC, vp->lat_min, vp->lon_min);
    
    w = (maxC.x - minC.x);
    h = (minC.y - maxC.y);
    
    GetCanvasLLPix(vp, wxPoint(0, 0), &nlat, &wlon ); // get lat/lon of upper left point of the window
    GetCanvasLLPix(vp, wxPoint(w, h), &slat, &elon ); // get lat/lon of lower right point of the window
    dlat = nlat - slat; // calculate how many degrees of latitude are shown in the window
    dlon = elon - wlon; // calculate how many degrees of longitude are shown in the window
    if( dlon < 0.0 ) // concider datum border at 180 degrees longitude
    {
        dlon = dlon + 360.0;
    }
    // calculate distance between latitude grid lines
    CalcGridSpacing( vp->view_scale_ppm, gridlatMajor, gridlatMinor, gridlatMicro );
    
    // calculate position of first major latitude grid line
    lat = ceil( slat / gridlatMajor ) * gridlatMajor;
    
    // Draw Major latitude grid lines and text
    while( lat < nlat ) {
        wxPoint r;
        wxString st = CalcGridText( lat, gridlatMajor, true ); // get text for grid line
        GetCanvasPixLL(vp, &r, lat, ( elon + wlon ) / 2);
        //GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
        myDrawLine(dc, 0, r.y, RULER_WIDTH_SMALL*2, r.y);                             // draw grid line
        myDrawLine(dc, w, r.y, w - RULER_WIDTH_SMALL*2, r.y);                             // draw grid line
        //dc->DrawText( st, 0, r.y ); // draw text
        myDrawText(dc, st, 0, r.y, false, aFont, aTexFont);
        lat = lat + gridlatMajor;
        
        if( fabs( lat - wxRound( lat ) ) < 1e-5 ) lat = wxRound( lat );
    }
    
    // calculate position of first minor latitude grid line
    lat = ceil( slat / gridlatMinor ) * gridlatMinor;
    int minor_count = 0;
    
    // Draw minor latitude grid lines
    while( lat < nlat ) {
        wxPoint r;
        wxString st = CalcGridText( lat, gridlatMinor, true ); // get text for grid line
        GetCanvasPixLL(vp, &r, lat, ( elon + wlon ) / 2);
        //GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
        myDrawLine(dc, 0, r.y, RULER_WIDTH_SMALL, r.y );
        myDrawLine(dc, w - RULER_WIDTH_SMALL, r.y, w, r.y );
        //dc->DrawText( st, 0, r.y ); // draw text
        myDrawText(dc, st, 0, r.y, false, aFont, aTexFont);
        lat = lat + gridlatMinor;
        
        minor_count++;
    }
    
    bool draw_micro = false;
    if (minor_count < RESOLUTION_LIMIT_FOR_MICRO_RULER)
    {
        //for now we will not draw the micro grid ... it is not needed for USPS training.. next version may have a config to contorl this.
        //draw_micro = true;
    }
    
    if (draw_micro)
    {
        // calculate position of first minor latitude grid line
        lat = ceil( slat / gridlatMicro ) * gridlatMicro;
        
        // Draw minor latitude grid lines
        while( lat < nlat ) {
            wxPoint r;
            GetCanvasPixLL(vp, &r, lat, ( elon + wlon ) / 2);
            //GetCanvasPointPix( lat, ( elon + wlon ) / 2, &r );
            myDrawLine(dc, 0, r.y, RULER_WIDTH_MICRO, r.y );
            myDrawLine(dc, w - RULER_WIDTH_MICRO, r.y, w, r.y );
            lat = lat + gridlatMicro;
        }
    }
    
    // calculate distance between grid lines
    CalcGridSpacing( vp->view_scale_ppm, gridlonMajor, gridlonMinor, gridlonMicro );
    
    // calculate position of first major latitude grid line
    lon = ceil( wlon / gridlonMajor ) * gridlonMajor;
    
    // draw major longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMajor ); i <= itermax; i++ ) {
        wxPoint r;
        wxString st = CalcGridText( lon, gridlonMajor, false );
        GetCanvasPixLL(vp, &r,( nlat + slat ) / 2, lon);
        //GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
        r.y = r.y - RULER_DELTA_FROM_BOTTOM;
        myDrawLine(dc, r.x, 0, r.x, 0 -1*(- RULER_DELTA_FROM_BOTTOM - RULER_WIDTH_SMALL*2));
        myDrawLine(dc, r.x, h - RULER_DELTA_FROM_BOTTOM - RULER_WIDTH_SMALL*2, r.x, h - RULER_DELTA_FROM_BOTTOM );
        //dc->DrawText( st, r.x, h - RULER_DELTA_FROM_BOTTOM - RULER_LEBEL_HIGHT);
        myDrawText(dc, st, r.x, h - RULER_DELTA_FROM_BOTTOM - RULER_LEBEL_HIGHT, false, aFont, aTexFont);

        lon = lon + gridlonMajor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }
        
        if( fabs( lon - wxRound( lon ) ) < 1e-5 ) lon = wxRound( lon );
        
    }
    
    // calculate position of first minor longitude grid line
    lon = ceil( wlon / gridlonMinor ) * gridlonMinor;
    // draw minor longitude grid lines
    for( int i = 0, itermax = (int) ( dlon / gridlonMinor ); i <= itermax; i++ ) {
        wxPoint r;
        wxString st = CalcGridText( lon, gridlonMinor, false );
        GetCanvasPixLL(vp, &r,( nlat + slat ) / 2, lon);
        //GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
        r.y = r.y - RULER_DELTA_FROM_BOTTOM;
        myDrawLine(dc, r.x, 0, r.x, RULER_WIDTH_SMALL);
        myDrawLine(dc, r.x, h - RULER_DELTA_FROM_BOTTOM - RULER_WIDTH_SMALL, r.x, h - RULER_DELTA_FROM_BOTTOM );
        //dc->DrawText( st, r.x, h - RULER_DELTA_FROM_BOTTOM - RULER_LEBEL_HIGHT);
        myDrawText(dc, st, r.x, h - RULER_DELTA_FROM_BOTTOM - RULER_LEBEL_HIGHT, false, aFont, aTexFont);

        lon = lon + gridlonMinor;
        if( lon > 180.0 ) {
            lon = lon - 360.0;
        }
    }

    if (draw_micro)
    {
        // calculate position of first minor longitude grid line
        lon = ceil( wlon / gridlonMicro ) * gridlonMicro;
        // draw minor longitude grid lines
        for( int i = 0, itermax = (int) ( dlon / gridlonMicro ); i <= itermax; i++ ) {
            wxPoint r;
            GetCanvasPixLL(vp, &r,( nlat + slat ) / 2, lon);
            //GetCanvasPointPix( ( nlat + slat ) / 2, lon, &r );
            r.y = r.y - RULER_DELTA_FROM_BOTTOM;
            myDrawLine(dc, r.x, 0, r.x, RULER_WIDTH_MICRO);
            myDrawLine(dc, r.x, h - RULER_DELTA_FROM_BOTTOM - RULER_WIDTH_MICRO, r.x, h - RULER_DELTA_FROM_BOTTOM );
            lon = lon + gridlonMicro;
            if( lon > 180.0 ) {
                lon = lon - 360.0;
            }
        }
    }

}

//byAA10152017.

bool aa1chartnavhelper_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    RenderOverlayBoth(&dc, vp);
    return true;
}
bool aa1chartnavhelper_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    //return true;
    
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_HINT_BIT );
    
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    
    RenderOverlayBoth(0, vp);
    
    glPopAttrib();
    
    return true;
}
void aa1chartnavhelper_pi::SetCursorLatLon(double lat, double lon)
{
}

void aa1chartnavhelper_pi::SetPositionFix(PlugIn_Position_Fix &pfix)
{
}

//Demo implementation of response mechanism
void aa1chartnavhelper_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
    if (message_id == _T("WMM_VARIATION")) //this is the message we received as a response from the WMM plugin
    {
        //only process messages if we are expecting results.
        if (m_receved_from_wmm__request_sent)
        {
            wxJSONReader r;
            wxJSONValue v;
            r.Parse(message_body, &v);
            m_receved_from_wmm__magVar = v[_T("Decl")].AsDouble();
            m_receved_from_wmm__magVarYdelta = v[_T("Decldot")].AsDouble();
            
            //mark the message "already received"
            m_receved_from_wmm__request_sent = false;
        }
    }
}

//byAA10172017:
void aa1chartnavhelper_pi::getVariationForLatLon(double lat, double lon, double& magVar, double& magVarYdelta)
{
    bool use_WMM = ! m_useDefaultMagVar; // we will dynamically "sense" that the WMM is missing... and not yet able to get the UserDefined Variance (g_UserVar)
    
    //to make the code aver of the plugin manager, we could capture the pointer in the cpnstructor ... BUT
    //  but then we would becoam dependent on the current version of the manager... so the plugin would become version specific
    //  which would require a recompilateion every time the main OpenCPN changes... TOO BAD ... better just use config file...
    
    //bool b_haveWMM = g_pi_manager && g_pi_manager->IsPlugInAvailable(_T("WMM"));
    
    if (use_WMM)
    {
        wxJSONValue v;
        v[_T("Lat")] = lat;
        v[_T("Lon")] = lon;
        v[_T("Year")] = m_year_for_MagVar;
        v[_T("Month")] = 0; // zero based
        v[_T("Day")] = 1;
        
        wxJSONWriter w;
        wxString out;
        w.Write(v, out);

        m_receved_from_wmm__request_sent = true;
        //forget old values!
        m_receved_from_wmm__magVar = 0;
        m_receved_from_wmm__magVarYdelta = 0;
        
        SendPluginMessage(wxString(_T("WMM_VARIATION_REQUEST")), out);
        //the WMM plugin will respond by calling the SendPluginMessage message with _T("WMM_VARIATION) message id
        // all this will happen synchronously, so at the next line here we are already after the
        //  aa1chartnavhelper_pi::SetPluginMessage(wxString &message_id, wxString &message_body) .. was called
        // here we talek the values which we received from the wmm_pi::SetPluginMessage(wxString &message_id, wxString &message_body) call (callback)
        
        // we set the m_receved_from_wmm__request_sent to fasle when the _T("WMM_VARIATION) response message is received.
        if (!m_receved_from_wmm__request_sent)
        {
            magVar = m_receved_from_wmm__magVar;
            magVarYdelta = m_receved_from_wmm__magVarYdelta;
        }
        else // if we did not get a response for the _T("WMM_VARIATION_REQUEST") message then the WMM plugin may be disabled, and so use the user defined variation
        {
            magVar = m_config_default__magVar; //g_UserVar;
            magVarYdelta = m_config_default__magVarYdelta; // wasically we dont know this value without the WMM plugin
        }
    }
    else
    {
        magVar = m_config_default__magVar; //g_UserVar;
        magVarYdelta = m_config_default__magVarYdelta; // basically we dont know this value without the WMM plugin
    }
}

bool aa1chartnavhelper_pi::MouseEventHook( wxMouseEvent &event )
{
    bool retval = false;
    if(m_bShowCompass)
    {

        if( event.Moving())
        {
            //return false; //maintain status bar and tracking dialog updated
        }
        else
        {
            if( event.LeftDown() )
            {
                //1) get event coord (lon, lat)
                wxPoint pos = event.GetPosition();
                //2) convert ot pixel coords
                //3) compar to current pixel coord
                //4) if current and mouse pix coords are withing X pixel squer then start moving
                if ((abs(m_current_compass_rose_cx - pos.x) < 10)
                    &&
                    (abs(m_current_compass_rose_cy - pos.y) < 10))
                {
                    m_moving_with_mouse_click = true;
                
                    //steel this click
                    retval = true;
                }
            }
            
            if( event.LeftUp () && m_moving_with_mouse_click)
            {
                if (m_moving_with_mouse_click)
                {
                    m_moving_with_mouse_click = false;
                    m_moving_with_mouse_click_just_finihed_moving = true; //do one last update.. mainly for jirking final moce out of boundary.. this will correct it
                    //steel this click
                    retval = true;
                }
            }
            
            if( event.Dragging() )
            {
                if (m_moving_with_mouse_click)
                {
                    //1) get event coord (lon, lat)
                    wxPoint pos = event.GetPosition();
                    //2) set as next compas rose coords
                    //3) trigger redrow
                    m_preffered_compass_rose_cx = pos.x;
                    m_preffered_compass_rose_cy = pos.y;
                    
                    //steel this click
                    retval = true;

                    //request paint...
                    RequestRefresh(m_parent_window);
                }
            }
        }
    }
    
    return retval;
}
//byAA10172017.


wxString aa1chartnavhelper_pi::AngleToText(double angle)
{
    int deg = (int)fabs(angle);
    int min = (fabs(angle) - deg) * 60;
    if (angle < 0)
        return wxString::Format(_T("%u\u00B0%u' W"), deg, min);
    else
        return wxString::Format(_T("%u\u00B0%u' E"), deg, min);
}

bool aa1chartnavhelper_pi::LoadConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

    if(pConf)
    {
        pConf->SetPath ( _T( "/Settings/AA1ChartNavHelper" ) );

        //byAA10182017:
        pConf->Read ( _T( "ShowCompass" ),  &m_config_default_ShowCompass, false);
        
        pConf->Read ( _T( "MagVarYear" ),  &m_year_for_MagVar, 2017 );

        pConf->Read ( _T( "MagVarForceUseDefaultMagVar" ),  &m_useDefaultMagVar, false);
        pConf->Read ( _T( "MagVarIfNoWMM" ),  &m_config_default__magVar, -15.25 );
        pConf->Read ( _T( "MagVarYdeltaIfNoWMM" ),  &m_config_default__magVarYdelta, 0.01 );
        
        pConf->Read ( _T( "PrefferedCompassRoseCx"), &m_preffered_compass_rose_cx, 0);
        pConf->Read ( _T( "PrefferedCompassRoseCy"), &m_preffered_compass_rose_cy, 0);

        //byAA10182017.

        pConf->Read ( _T( "ShowLiveIcon" ),  &m_bShowLiveToolbarIcon, 1 );
        pConf->Read ( _T( "ShowIcon" ),  &m_bShowToolbarIcon, 1 );
        pConf->Read ( _T( "Opacity" ),  &m_iOpacity, 255 );

        m_aa1chartnavhelper_dialog_x =  pConf->Read ( _T ( "DialogPosX" ), 20L );
        m_aa1chartnavhelper_dialog_y =  pConf->Read ( _T ( "DialogPosY" ), 20L );

        if((m_aa1chartnavhelper_dialog_x < 0) || (m_aa1chartnavhelper_dialog_x > m_display_width))
            m_aa1chartnavhelper_dialog_x = 5;
        if((m_aa1chartnavhelper_dialog_y < 0) || (m_aa1chartnavhelper_dialog_y > m_display_height))
            m_aa1chartnavhelper_dialog_y = 5;

        pConf->SetPath ( _T( "/Settings/AA1ChartNavHelper/Data" ) );

        pConf->SetPath ( _T ( "/Directories" ) );
        wxString s =wxFileName::GetPathSeparator();
        wxString def = *GetpSharedDataLocation() + _T("plugins")
            + s + _T("aa1chartnavhelper_pi") + s + _T("data") + s;
        //pConf->Read ( _T ( "AA1ChartNavHelperDataLocation" ), &m_aa1chartnavhelper_dir, def);
        m_aa1chartnavhelper_dir = def;
        return true;
    }
    else
        return false;
}

bool aa1chartnavhelper_pi::SaveConfig(void)
{
    wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

    if(pConf)
    {
        pConf->SetPath ( _T ( "/Settings/AA1ChartNavHelper" ) );

        //byAA10182017:
        pConf->Write ( _T( "ShowCompass" ),  m_bShowCompass ); //m_config_default_ShowCompass to be loaded back to
        
        pConf->Write ( _T( "MagVarYear" ),  m_year_for_MagVar);

        pConf->Write ( _T( "MagVarForceUseDefaultMagVar" ),  m_useDefaultMagVar);
        pConf->Write ( _T( "MagVarIfNoWMM" ),  m_config_default__magVar);
        pConf->Write ( _T( "MagVarYdeltaIfNoWMM" ),  m_config_default__magVarYdelta);
        
        pConf->Write ( _T( "PrefferedCompassRoseCx"), m_preffered_compass_rose_cx);
        pConf->Write ( _T( "PrefferedCompassRoseCy"), m_preffered_compass_rose_cy);
        
        //byAA10182017.

        pConf->Write ( _T ( "ShowLiveIcon" ), m_bShowLiveToolbarIcon );
        pConf->Write ( _T ( "ShowIcon" ), m_bShowToolbarIcon );
        pConf->Write ( _T ( "Opacity" ), m_iOpacity );

        pConf->Write ( _T ( "DialogPosX" ),   m_aa1chartnavhelper_dialog_x );
        pConf->Write ( _T ( "DialogPosY" ),   m_aa1chartnavhelper_dialog_y );

        pConf->SetPath ( _T( "/Settings/AA1ChartNavHelper/Data" ) );

        pConf->SetPath ( _T ( "/Directories" ) );
        pConf->Write ( _T ( "AA1ChartNavHelperDataLocation" ), m_aa1chartnavhelper_dir );

        return true;
    }
    else
        return false;
}

void aa1chartnavhelper_pi::ShowPreferencesDialog( wxWindow* parent )
{
    Aa1ChartNavHelperPrefsDialog *dialog = new Aa1ChartNavHelperPrefsDialog( parent, wxID_ANY, _("Chart Nav Helper Preferences"), wxPoint( m_aa1chartnavhelper_dialog_x, m_aa1chartnavhelper_dialog_y), wxDefaultSize, wxDEFAULT_DIALOG_STYLE );
    dialog->Fit();
    wxColour cl;
    GetGlobalColor(_T("DILG1"), &cl);
    dialog->SetBackgroundColour(cl);

    dialog->m_cbShowToolbarIcon->SetValue(m_bShowToolbarIcon);
    dialog->m_cbShowLiveToolbarIcon->SetValue(m_bShowLiveToolbarIcon);
    dialog->m_sOpacity->SetValue(m_iOpacity);

    if(dialog->ShowModal() == wxID_OK)
    {
        m_bShowToolbarIcon = dialog->m_cbShowToolbarIcon->GetValue();
        m_bShowLiveToolbarIcon = dialog->m_cbShowLiveToolbarIcon->GetValue();
        m_iOpacity = dialog->m_sOpacity->GetValue();

        RearrangeWindow();
        SetIconType();

        SaveConfig();
    }
    delete dialog;
}

void aa1chartnavhelper_pi::ShowPlotSettings()
{
    Aa1ChartNavHelperPlotSettingsDialog *dialog = new Aa1ChartNavHelperPlotSettingsDialog( m_parent_window );
    wxFont *pFont = OCPNGetFont(_T("Dialog"), 0);
    dialog->SetFont(*pFont);
    
    dialog->Fit();
    wxColour cl;
    GetGlobalColor(_T("DILG1"), &cl);
    dialog->SetBackgroundColour(cl);

    dialog->m_scCtrlValYearSpacing->SetValue(m_year_for_MagVar);

    dialog->m_checkBoxUseDefaultVar->SetValue(m_useDefaultMagVar);
    dialog->m_textDefaultVar->SetValue(wxString::Format("%.2f", m_config_default__magVar));
    dialog->m_textDefaultVarYearDelta->SetValue(wxString::Format("%.2f", m_config_default__magVarYdelta));

    if(dialog->ShowModal() == wxID_OK)
    {
        m_useDefaultMagVar = dialog->m_checkBoxUseDefaultVar->GetValue();
        m_year_for_MagVar = dialog->m_scCtrlValYearSpacing->GetValue();
        
        wxString testVal = dialog->m_textDefaultVar->GetValue();
        if(!testVal.ToDouble(&m_config_default__magVar))
        {
            //format error don't save value...
        }
        testVal = dialog->m_textDefaultVarYearDelta->GetValue();
        if(!testVal.ToDouble(&m_config_default__magVarYdelta))
        {
          //format error don't save value...
        }
        
        //m_bCachedPlotOk = false;

        RequestRefresh( m_parent_window );
        RearrangeWindow();

        SaveConfig();
    }
    delete dialog;
}



