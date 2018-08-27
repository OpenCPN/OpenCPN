/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Canvas Configuration
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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

#ifndef __CANVCONFIG_H__
#define __CANVCONFIG_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

class ChartCanvas;


//  Class to encapsulate persistant canvas configuration(s)
class canvasConfig
{
public:    
    canvasConfig();
    canvasConfig(int index);
    ~canvasConfig();
    
    int configIndex;
    ChartCanvas *canvas;
    double iLat, iLon, iScale, iRotation;
    int DBindex; 
    int GroupID;
    bool bFollow;
    bool bQuilt;
    wxString toolbarConfig;
    bool bShowTides;
    bool bShowCurrents;
    wxSize canvasSize;
    bool bShowToolbar;
    int toolbarOrientation;
    bool bShowGrid;
    bool bShowOutlines;
    bool bShowDepthUnits;
    
    // ENC options
    bool bShowENCText;
    int  nENCDisplayCategory;
    bool bShowENCDepths;
    bool bShowENCBuoyLabels;
    
};

WX_DECLARE_OBJARRAY(canvasConfig*, arrayofCanvasConfigPtr);

#endif
