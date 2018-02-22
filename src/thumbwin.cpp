/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Thumbnail Object
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
 * $Log: thumbwin.cpp,v $
 * Revision 1.7  2010/04/27 01:44:02  bdbcat
 * Build 426
 *
 * Revision 1.6  2009/11/18 01:26:13  bdbcat
 * 1.3.5 Beta 1117
 *
 * Revision 1.5  2008/08/26 13:46:25  bdbcat
 * Better color scheme support
 *
 * Revision 1.4  2008/03/30 22:29:22  bdbcat
 * Cleanup
 *
 * Revision 1.3  2007/05/03 13:23:56  dsr
 * Major refactor for 1.2.0
 *
 * Revision 1.2  2006/09/21 01:37:37  dsr
 * Major refactor/cleanup
 *
 * Revision 1.1.1.1  2006/08/21 05:52:19  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.3  2006/08/04 11:42:03  dsr
 * no message
 *
 * Revision 1.2  2006/05/19 19:29:58  dsr
 * New Thumbchart logic
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.5  2006/04/19 00:53:01  dsr
 * Do not allow Thumbwin.y to be larger than parent frame.y
 *
 * Revision 1.4  2006/03/16 03:08:25  dsr
 * Cleanup tabs
 *
 * Revision 1.3  2006/02/23 01:52:15  dsr
 * Cleanup
 *
 *
 *
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "dychart.h"

#include "thumbwin.h"
#include "chart1.h"
#include "chartdb.h"
#include "chcanv.h"


//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------
extern ChartDB          *ChartData;
extern ChartStack       *pCurrentStack;

extern MyFrame          *gFrame;
extern ChartBase        *Current_Ch;
extern ChartCanvas      *cc1;


//------------------------------------------------------------------------------
//    Thumbwin Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ThumbWin, wxWindow)
  EVT_PAINT(ThumbWin::OnPaint)
END_EVENT_TABLE()

// Define a constructor

ThumbWin::ThumbWin(wxFrame *frame):
                   wxWindow(frame, wxID_ANY,    wxPoint(20,20), wxSize(5,5), wxSIMPLE_BORDER)
{
      pThumbChart = NULL;
      m_max_size.x = m_max_size.y = 100;
      Show(false);
}


ThumbWin::~ThumbWin()
{
}


void ThumbWin::Resize(void)
{
    if(pThumbChart)
    {
      if(pThumbChart->GetThumbData()->pDIBThumb)
      {
            int newheight = __min(m_max_size.y, pThumbChart->GetThumbData()->pDIBThumb->GetHeight());
            int newwidth  = __min(m_max_size.x, pThumbChart->GetThumbData()->pDIBThumb->GetWidth());
            SetSize(0,0,newwidth,newheight );
      }
    }

}

void ThumbWin::SetMaxSize(wxSize const &max_size)
{
      m_max_size = max_size;
}


void ThumbWin::OnPaint(wxPaintEvent& event)
{

    wxPaintDC dc(this);

    if(pThumbChart)
    {
      if(pThumbChart->GetThumbData())
      {
            if(pThumbChart->GetThumbData()->pDIBThumb)
                  dc.DrawBitmap(*(pThumbChart->GetThumbData()->pDIBThumb), 0, 0, false);

            wxPen ppPen(GetGlobalColor(_T("CHBLK")), 1, wxSOLID);
            dc.SetPen(ppPen);
            wxBrush yBrush(GetGlobalColor(_T("CHYLW")), wxSOLID);
            dc.SetBrush(yBrush);
            dc.DrawCircle(pThumbChart->GetThumbData()->ShipX, pThumbChart->GetThumbData()->ShipY, 6);
      }
    }

}



