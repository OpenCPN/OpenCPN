/******************************************************************************
 * $Id: aa1chartnavhelper_pi.h,v 1.0 2011/02/26 01:54:37 nohal Exp $
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

#ifndef __AA1CHARTNAVHELPERPI_H__
#define __AA1CHARTNAVHELPERPI_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/fileconf.h>

#include "version.h"

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    14

#include "ocpn_plugin.h"

#include "Aa1ChartNavHelperUIDialog.h"
//#include "AA1_COF.h"

#include "jsonreader.h"
#include "jsonwriter.h"

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

#define AA1CHARTNAVHELPER_TOOL_POSITION    -1        // Request default positioning of toolbar tool
class aa1chartnavhelper_pi;
class Aa1ChartNavHelperUIDialog;
class Aa1ChartNavHelperPlotSettingsDialog;

class aa1chartnavhelper_pi : public opencpn_plugin_114
{
public:
    aa1chartnavhelper_pi(void *ppimgr);

//    The required PlugIn Methods
    int Init(void);
    bool DeInit(void);

    int GetAPIVersionMajor();
    int GetAPIVersionMinor();
    int GetPlugInVersionMajor();
    int GetPlugInVersionMinor();
    wxBitmap *GetPlugInBitmap();
    wxString GetCommonName();
    wxString GetShortDescription();
    wxString GetLongDescription();

    //The required override PlugIn Methods
    void SetCursorLatLon(double lat, double lon);
    void SetPositionFix(PlugIn_Position_Fix &pfix);

    void RenderOverlayBoth(wxDC *dc, PlugIn_ViewPort *vp);
    bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);

    //byAA10152017:
    bool plotPaperChartComponents(wxDC *dc, PlugIn_ViewPort *vp);
    
    bool m_moving_with_mouse_click;
    bool m_moving_with_mouse_click_just_finihed_moving;
    bool MouseEventHook( wxMouseEvent &event );
    
    void SetShowCompass(bool showcompas) { m_bShowCompass = showcompas; }

    //byAA10152017:
    
    int GetToolbarToolCount(void);
    void ShowPreferencesDialog( wxWindow* parent );
    void ShowPlotSettingsDialog( wxCommandEvent& event );

    void OnToolbarToolCallback(int id);

    // Optional plugin overrides
    void SetColorScheme(PI_ColorScheme cs);
    void SetPluginMessage(wxString &message_id, wxString &message_body);

    //Other public methods
    void SetAa1ChartNavHelperDialogX    (int x){ m_aa1chartnavhelper_dialog_x = x;};
    void SetAa1ChartNavHelperDialogY    (int x){ m_aa1chartnavhelper_dialog_y = x;}

    void OnAa1ChartNavHelperDialogClose();
    void ShowPlotSettings();

    //AA1 Declarations

    wxWindow       *m_parent_window;
    
    bool getShowCompass() { return m_bShowCompass; }
private:
    wxFileConfig     *m_pconfig;
    bool          LoadConfig(void);
    bool          SaveConfig(void);

    Aa1ChartNavHelperUIDialog    *m_pAa1ChartNavHelperDialog;

    int           m_aa1chartnavhelper_dialog_x, m_aa1chartnavhelper_dialog_y;
    int           m_display_width, m_display_height;
    bool          m_bShowLiveToolbarIcon;
    bool          m_bShowToolbarIcon;
    int           m_iOpacity;

    int           m_leftclick_tool_id;

    wxString        AngleToText(double angle);

    bool          m_bShowCompass;

    void          RearrangeWindow();
    void          SetIconType();
    wxString        m_aa1chartnavhelper_dir;

    //byAA10172017:
    int m_current_compass_rose_cx;
    int m_current_compass_rose_cy;
    int m_preffered_compass_rose_cx;
    int m_preffered_compass_rose_cy;
    void getCompassRoseCenter(wxDC *dc, PlugIn_ViewPort *vp, int& cx, int& cy);
    void DrawCompassRose(wxDC* dc, int cx, int cy, int radius, int startangle, bool showlabels);
    void DrawCompassRoseTicks(wxDC* dc, int cx, int cy, int radius, int startangle, bool showlabels,
                              double magVar, double magVarYdelta, bool markAt90);
    
    void DrawRulers(wxDC *dc, PlugIn_ViewPort *vp);
 
    bool          m_config_default_ShowCompass;
    int           m_year_for_MagVar;
    bool          m_receved_from_wmm__request_sent; //to allow handling of my requested response only (woks because of the synchronous execution of messaging)
    bool          m_useDefaultMagVar;
    double        m_config_default__magVar;
    double        m_config_default__magVarYdelta;

    double        m_receved_from_wmm__magVar;
    double        m_receved_from_wmm__magVarYdelta;

    void          getVariationForLatLon(double lat, double lon, double& magVar, double& magVarYdelta);
    //byAA10172017.
    void          SendVariationAt(double lat, double lon, int year, int month, int day);
    void          SendBoatVariation();
    void          SendCursorVariation();

//    wxFont        *pFontSmall;
//    double        m_scale;
      wxString      m_shareLocn;
};

class Aa1ChartNavHelperUIDialog : public Aa1ChartNavHelperUIDialogBase
{
public:
    Aa1ChartNavHelperUIDialog( aa1chartnavhelper_pi &_aa1chartnavhelper_pi, wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxString& title = _T("Chart Nav Helper"), const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxSize( 250,495 ),
                long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL )
    : Aa1ChartNavHelperUIDialogBase( parent, id, title, pos, size, style ), m_aa1chartnavhelper_pi(_aa1chartnavhelper_pi) {}
    
    void PlotSettings( wxCommandEvent& event );
    void EnableCompassChanged( wxCommandEvent& event );
    
    virtual bool Show(bool show = true)
    {
        m_cbEnableCompass->SetValue(m_aa1chartnavhelper_pi.getShowCompass());
        return Aa1ChartNavHelperUIDialogBase::Show(show);
    }
    
protected:
    aa1chartnavhelper_pi &m_aa1chartnavhelper_pi;
    

};

class Aa1ChartNavHelperPlotSettingsDialog : public Aa1ChartNavHelperPlotSettingsDialogBase
{
public:
    Aa1ChartNavHelperPlotSettingsDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Chart Nav Helper Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 375,180 ), long style = wxDEFAULT_DIALOG_STYLE ) : Aa1ChartNavHelperPlotSettingsDialogBase(parent, id, title, pos, size, style) {}
    
    void About( wxCommandEvent& event );
    void Save( wxCommandEvent& event ) { EndDialog(wxID_OK); }
    void Cancel( wxCommandEvent& event ) { EndDialog(wxID_CANCEL); }
};

//byAA10152017: ..similar as in dial.h/cpp
double rad2deg(double angle);
double deg2rad(double angle);
void DrawPolarCompassRose(wxDC* dc, int cx, int cy, int radius, int startangle, bool showlabels);
//byAA10152017.

#endif //__AA1CHARTNAVHELPERPI_H__
