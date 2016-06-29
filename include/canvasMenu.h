/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  CanvasMenuHandler
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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

#ifndef __CANVASMENU_H__
#define __CANVASMENU_H__

#include "bbox.h"

#include <wx/datetime.h>
#include <wx/treectrl.h>
#include "wx/dirctrl.h"
#include <wx/sound.h>
#include <wx/grid.h>
#include <wx/wxhtml.h>

#include "chart1.h"                 // for enum types
#include "ocpndc.h"
#include "undo.h"

#include "ocpCursor.h"
#include "S57QueryDialog.h"
#include "GoToPositionDialog.h"
#include "CM93DSlide.h"
#include "RolloverWin.h"
#include "AISTargetQueryDialog.h"
#include "timers.h"
#include "emboss_data.h"

//#include "s57chart.h"

class wxGLContext;
class GSHHSChart;
class IDX_entry;


//----------------------------------------------------------------------------
//    Forward Declarations
//----------------------------------------------------------------------------
      class Route;
      class TCWin;
      class RoutePoint;
      class SelectItem;
      class wxBoundingBox;
      class ocpnBitmap;
      class WVSChart;
      class MyFrame;
      class ChartBaseBSB;
      class ChartBase;
      class AIS_Target_Data;
      class S57ObjectTree;
      class S57ObjectDesc;
      class RolloverWin;
      class Quilt;
      class PixelCache;
      class ChInfoWin;
      class glChartCanvas;



//----------------------------------------------------------------------------
// CanvasMenuHandler
//----------------------------------------------------------------------------
class CanvasMenuHandler: public wxEvtHandler
{
public:
    CanvasMenuHandler(ChartCanvas *parentCanvas,
          Route *selectedRoute,
          Track *selectedTrack,
          RoutePoint *selectedPoint,
          int selectedAIS_MMSI,
          void *selectedTCIndex);
      
      ~CanvasMenuHandler();
 
      void CanvasPopupMenu( int x, int y, int seltype );
      void PopupMenuHandler( wxCommandEvent& event );
      
 private:
       
      int               popx, popy;
      ChartCanvas       *parent;
      Route             *m_pSelectedRoute;
      Track             *m_pSelectedTrack;
      RoutePoint        *m_pFoundRoutePoint;
      int               m_FoundAIS_MMSI;
      void *            m_pIDXCandidate;
      
      
};



#endif
