/******************************************************************************
 * $Id:  $
 *
 * Project:  OpenCPN
 * Purpose:  Class to draw targets on raderview
 * Author:   Johan van der Sman
 *
 ***************************************************************************
 *   Copyright (C) 2015 Johan van der Sman                                 *
 *   hannes@andcrew.nl                                                     *
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
 */

#ifndef _TARGET_H_
#define _TARGET_H_


#include "wx/wxprec.h"
#include "ocpn_plugin.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

//----------------------------------------------------------------------------------------------------------
//    Target Specification
//----------------------------------------------------------------------------------------------------------
class Target
{
public:
    Target( );
    ~Target();
    void SetCanvas(wxPoint &center, int radius, wxColor &cl);     // details of drawing area
    void SetNavDetails(double range, double mycog,                // details of navigation view
    bool ShowCogArrow, int CogArrowMinutes);
    void SetState(int mmsi, wxString name, double  dist,          // Target details
    			double brg, double cog, double sog,
                        int tclass, plugin_ais_alarm_type state,
                        int rot
                    );
    bool Render( wxDC& dc);                                       // render the target

private:
    plugin_ais_alarm_type     State;
    wxPoint                   CanvasCenter;
    int                       Radius;
    double                    Range;
    double                    Mycog;
    bool                      ShowArrow;
    int                       ArrowMinutes;
    int                       Mmsi;
    wxString                  Name;
    int                       Tclass;
    double                    Dist;
    double                    Brg;
    double                    Cog;
    double                    Sog;
    int                       Rot;
    wxPoint                   TargetCenter;
};

#endif
