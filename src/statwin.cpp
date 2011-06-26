/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Status Window
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
 * $Log: statwin.cpp,v $
 * Revision 1.24  2010/06/13 21:05:49  bdbcat
 * 613a
 *
 * Revision 1.23  2010/05/23 23:15:24  bdbcat
 * Build 523a
 *
 * Revision 1.22  2010/05/15 04:03:22  bdbcat
 * Build 514
 *
 * Revision 1.21  2010/05/04 04:44:57  bdbcat
 * Build 504
 *
 * Revision 1.20  2010/05/04 01:33:52  bdbcat
 * Build 503
 *
 * Revision 1.19  2010/05/02 03:01:14  bdbcat
 * Build 501
 *
 * Revision 1.18  2010/04/27 01:43:46  bdbcat
 * Build 426
 *
 *
 *
 */



#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "dychart.h"

#include "statwin.h"
#include "chartdb.h"
#include "chart1.h"
#include "chartbase.h"

//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------
extern ChartDB          *ChartData;
//extern ChartStack       *pCurrentStack;

//------------------------------------------------------------------------------
//    StatWin Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(StatWin, wxWindow)
  EVT_PAINT(StatWin::OnPaint)
  EVT_SIZE(StatWin::OnSize)
  EVT_MOUSE_EVENTS(StatWin::MouseEvent)
END_EVENT_TABLE()

// ctor
StatWin::StatWin(wxFrame *frame):
 wxWindow(frame, wxID_ANY, wxPoint(20,20), wxSize(5,5), wxSIMPLE_BORDER)

{
      int x,y;
      GetClientSize(&x, &y);

      m_backBrush = wxBrush(GetGlobalColor(_T("UIBDR")), wxSOLID);

      SetBackgroundColour(GetGlobalColor(_T("UIBDR")));

      SetBackgroundStyle(wxBG_STYLE_CUSTOM);  // on WXMSW, this prevents flashing on color scheme change

      m_rows = 1;

 //   Create the Children

      pPiano = new PianoWin((wxFrame *)this);
      pPiano->SetSize(0, 0, x *6/10, y*1/m_rows);

#ifdef USE_WIFI_CLIENT
      pWiFi = new WiFiStatWin((wxFrame *)this);
      pWiFi->SetSize(x * 6/10, 0, x *4/10, y * 1/m_rows);
#endif

 }

StatWin::~StatWin()
{
      pPiano->Close();

#ifdef USE_WIFI_CLIENT
      pWiFi->Close();
#endif

}



void StatWin::OnPaint(wxPaintEvent& event)
{
      wxPaintDC dc(this);
      dc.SetBackground(m_backBrush);
      dc.Clear();
}


void StatWin::OnSize(wxSizeEvent& event)
{
      int width,height;
      GetClientSize(&width, &height);
      int x,y;
      GetPosition(&x, &y);

      if(width)
      {
            pPiano->SetSize(0,0, width *6/10, height*1/m_rows);
            pPiano->FormatKeys();
      }



#ifdef USE_WIFI_CLIENT
      if(width) pWiFi->SetSize(width * 6/10, 0, width *4/10, height*1/m_rows);
#endif

}

void StatWin::FormatStat(void)
{

      pPiano->FormatKeys();
}

void StatWin::MouseEvent(wxMouseEvent& event)
{
      int x,y;
      event.GetPosition(&x, &y);

}

int StatWin::GetFontHeight()
{
      wxClientDC dc(this);

      wxCoord w,h;
      GetTextExtent(_T("TEST"), &w, &h);

      return(h);
}

void StatWin::SetColorScheme(ColorScheme cs)
{

    m_backBrush = wxBrush(GetGlobalColor(_T("UIBDR")), wxSOLID);

    //  Also apply color scheme to all known children
    pPiano->SetColorScheme(cs);
#ifdef USE_WIFI_CLIENT
    pWiFi ->SetColorScheme(cs);
#endif

    Refresh();
}



//------------------------------------------------------------------------------
//          TextStat Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(TStatWin, wxWindow)
  EVT_PAINT(TStatWin::OnPaint)
  EVT_SIZE(TStatWin::OnSize)
END_EVENT_TABLE()

TStatWin::TStatWin(wxFrame *frame):
      wxWindow(frame, wxID_ANY,wxPoint(20,20), wxSize(5,5), wxSIMPLE_BORDER)
{
      SetBackgroundColour(GetGlobalColor(_T("UIBDR")));
      pText = new wxString();
      bTextSet = false;

}

TStatWin::~TStatWin(void)
{
      delete pText;
}


void TStatWin::OnSize(wxSizeEvent& event)
{
}

void TStatWin::OnPaint(wxPaintEvent& event)
{
      wxPaintDC dc(this);
      dc.DrawText(*pText, 0, 0);
}

void TStatWin::TextDraw(const wxString& text)
{
      *pText = text;
      bTextSet = true;
      Refresh(true);
}




//------------------------------------------------------------------------------
//          Piano Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(PianoWin, wxWindow)
  EVT_PAINT(PianoWin::OnPaint)
  EVT_SIZE(PianoWin::OnSize)
  EVT_MOUSE_EVENTS(PianoWin::MouseEvent)
END_EVENT_TABLE()

// Define a constructor
PianoWin::PianoWin(wxFrame *frame):
      wxWindow(frame, wxID_ANY, wxPoint(20,20), wxSize(5,5), wxSIMPLE_BORDER)
{
    m_index_last = -1;
    m_iactive = -1;

    m_hover_icon_last = -1;
    m_hover_last = -1;
    m_brounded = false;

    gparent = (MyFrame *)GetGrandParent();

    m_nRegions = 0;

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);  // on WXMSW, this prevents flashing on color scheme change

    m_pVizIconBmp = NULL;
    m_pInVizIconBmp = NULL;

 }

PianoWin::~PianoWin()
{
}



void PianoWin::OnSize(wxSizeEvent& event)
{
}

void PianoWin::SetColorScheme(ColorScheme cs)
{

      //    Recreate the local brushes

    m_backBrush = wxBrush(GetGlobalColor(_T("UIBDR")), wxSOLID);

    m_tBrush =    wxBrush(GetGlobalColor(_T("BLUE2")), wxSOLID);    // Raster Chart unselected
    m_slBrush =   wxBrush(GetGlobalColor(_T("BLUE1")), wxSOLID);    // and selected

    m_vBrush =    wxBrush(GetGlobalColor(_T("GREEN2")), wxSOLID);    // Vector Chart unselected
    m_svBrush =   wxBrush(GetGlobalColor(_T("GREEN1")), wxSOLID);    // and selected

    m_cBrush =    wxBrush(GetGlobalColor(_T("YELO2")), wxSOLID);     // CM93 Chart unselected
    m_scBrush =   wxBrush(GetGlobalColor(_T("YELO1")), wxSOLID);    // and selected

    m_uvBrush =   wxBrush(GetGlobalColor(_T("UINFD")), wxSOLID);    // and unavailable

}


void PianoWin::OnPaint(wxPaintEvent& event)
{
      int width, height;
      GetClientSize(&width, &height );
      wxPaintDC dc(this);

      dc.SetBackground(m_backBrush);
      dc.Clear();

//    Create the Piano Keys

      int nKeys = m_key_array.GetCount();

 //     assert(nKeys <= KEY_REGIONS_MAX);

      if(nKeys)
      {
            wxPen ppPen(GetGlobalColor(_T("CHBLK")), 1, wxSOLID);
            dc.SetPen(ppPen);

            dc.SetBrush(m_tBrush);

            for(int i=0 ; i<nKeys ; i++)
            {
                  int key_db_index = m_key_array.Item(i);

                  if(-1 == key_db_index)
                        continue;

                  if(ChartData->GetDBChartType(m_key_array.Item(i)) == CHART_TYPE_S57)
                  {
                        dc.SetBrush(m_vBrush);

                        for(unsigned int ino=0 ; ino < m_active_index_array.GetCount() ; ino++)
                        {
                              if(m_active_index_array.Item(ino) == key_db_index)        // chart is in the active list
                                    dc.SetBrush(m_svBrush);
                        }
                  }


                  else if(ChartData->GetDBChartType(m_key_array.Item(i)) == CHART_TYPE_CM93)
                  {
                        dc.SetBrush(m_cBrush);

                        for(unsigned int ino=0 ; ino < m_active_index_array.GetCount() ; ino++)
                        {
                              if(m_active_index_array.Item(ino) == key_db_index)        // chart is in the active list
                                    dc.SetBrush(m_scBrush);
                        }
                  }

                  else if(ChartData->GetDBChartType(m_key_array.Item(i)) == CHART_TYPE_CM93COMP)
                  {
                        dc.SetBrush(m_cBrush);

                        for(unsigned int ino=0 ; ino < m_active_index_array.GetCount() ; ino++)
                        {
                              if(m_active_index_array.Item(ino) == key_db_index)        // chart is in the active list
                                    dc.SetBrush(m_scBrush);
                        }
                  }

                  else
                  {
                        dc.SetBrush(m_tBrush);

                        for(unsigned int ino=0 ; ino < m_active_index_array.GetCount() ; ino++)
                        {
                              if(m_active_index_array.Item(ino) == key_db_index)        // chart is in the active list
                                    dc.SetBrush(m_slBrush);
                        }
                  }

                  // Check to see if this box appears in the sub_light array
                  // If so, add a crosshatch pattern to the brush
                  for(unsigned int ino=0 ; ino < m_sublite_index_array.GetCount() ; ino++)
                  {
                        if(m_sublite_index_array.Item(ino) == key_db_index)        // chart is in the sublite list
                        {
                              wxBrush ebrush(dc.GetBrush().GetColour(), wxCROSSDIAG_HATCH);
//                              dc.SetBrush(ebrush);
                        }
                  }

                  wxRect box = KeyRegion[i].GetBox();

                  if(m_brounded)
                        dc.DrawRoundedRectangle(box.x, box.y, box.width, box.height, 4);
                  else
                        dc.DrawRectangle(box);

                  for(unsigned int ino=0 ; ino < m_sublite_index_array.GetCount() ; ino++)
                  {
                        if(m_sublite_index_array.Item(ino) == key_db_index)        // chart is in the sublite list
                        {
                              dc.SetBrush(dc.GetBackground());
                              int w = 3;
                              dc.DrawRoundedRectangle(box.x + w, box.y + w, box.width - (2*w), box.height - (2 * w), 3);

                        }
                  }

                  //    Look in the current noshow array for this index
                        for(unsigned int ino=0 ; ino < m_noshow_index_array.GetCount() ; ino++)
                        {
                              if(m_noshow_index_array.Item(ino) == key_db_index)        // chart is in the noshow list
                              {
                                    if(m_pInVizIconBmp && m_pInVizIconBmp->IsOk())
                                          dc.DrawBitmap(*m_pInVizIconBmp, box.x + 4, box.y + 3, true);
                                    break;
                              }
                        }

                  //    Look in the current skew array for this index
                        for(unsigned int ino=0 ; ino < m_skew_index_array.GetCount() ; ino++)
                        {
                              if(m_skew_index_array.Item(ino) == key_db_index)        // chart is in the list
                              {
                                    if(m_pSkewIconBmp && m_pSkewIconBmp->IsOk())
                                          dc.DrawBitmap(*m_pSkewIconBmp, box.x + box.width - m_pSkewIconBmp->GetWidth(), box.y, true);
                                    break;
                              }
                        }

                  //    Look in the current tmerc array for this index
                        for(unsigned int ino=0 ; ino < m_tmerc_index_array.GetCount() ; ino++)
                        {
                              if(m_tmerc_index_array.Item(ino) == key_db_index)        // chart is in the list
                              {
                                    if(m_pTmercIconBmp && m_pTmercIconBmp->IsOk())
                                          dc.DrawBitmap(*m_pTmercIconBmp,box.x + box.width - m_pTmercIconBmp->GetWidth(), box.y, true);
                                    break;
                              }
                        }

                  //    Look in the current poly array for this index
                        for(unsigned int ino=0 ; ino < m_poly_index_array.GetCount() ; ino++)
                        {
                              if(m_poly_index_array.Item(ino) == key_db_index)        // chart is in the list
                              {
                                    if(m_pPolyIconBmp && m_pPolyIconBmp->IsOk())
                                          dc.DrawBitmap(*m_pPolyIconBmp,box.x + box.width - m_pPolyIconBmp->GetWidth(), box.y, true);
                                    break;
                              }
                        }

            }
      }
}

void PianoWin::SetKeyArray(ArrayOfInts array)
{
      m_key_array = array;
      FormatKeys();
}

void PianoWin::SetNoshowIndexArray(ArrayOfInts array)
{
      m_noshow_index_array = array;
}

void PianoWin::SetActiveKeyArray(ArrayOfInts array)
{
      m_active_index_array = array;
}

void PianoWin::SetSubliteIndexArray(ArrayOfInts array)
{
      m_sublite_index_array = array;
}

void PianoWin::SetSkewIndexArray(ArrayOfInts array)
{
      m_skew_index_array = array;
}

void PianoWin::SetTmercIndexArray(ArrayOfInts array)
{
      m_tmerc_index_array = array;
}

void PianoWin::SetPolyIndexArray(ArrayOfInts array)
{
      m_poly_index_array = array;
}



void PianoWin::FormatKeys(void)
{
      int width, height;
      GetClientSize(&width, &height );

      int nKeys = m_key_array.GetCount();

      if(nKeys)
      {
            int kw = width / nKeys;

//    Build the Key Regions

//            assert(nKeys <= KEY_REGIONS_MAX);

            for(int i=0 ; i<nKeys ; i++)
            {
                  wxRegion r((i * kw) +3, 2, kw-6, height-4);
                  KeyRegion[i] = r;
            }
      }
      m_nRegions = nKeys;

}
wxPoint PianoWin::GetKeyOrigin(int key_index)
{
      if((key_index >= 0) && (key_index <= (int)m_key_array.GetCount()-1))
      {
            wxRect box = KeyRegion[key_index].GetBox();
            return wxPoint(box.x, box.y);
      }
      else
            return wxPoint(-1, -1);
}

void PianoWin::MouseEvent(wxMouseEvent& event)
{

      int x,y;
      event.GetPosition(&x, &y);

//    Check the regions

      int sel_index = -1;
      int sel_dbindex = -1;

      for(int i=0 ; i<m_nRegions ; i++)
      {
            if(KeyRegion[i].Contains(x,y)  == wxInRegion)
            {
                  sel_index = i;
                  sel_dbindex = m_key_array.Item(i);
                  break;
            }
      }

      if(event.LeftDown())
      {

            if(-1 != sel_index)
            {
                gparent->HandlePianoClick(sel_index, sel_dbindex);
            }
      }

      else if(event.RightDown())
      {
            if(-1 != sel_index)
            {
                  gparent->HandlePianoRClick(x, y, sel_index, sel_dbindex);
            }
      }

      else
      {

            if(sel_index != m_hover_last)
            {
                  gparent->HandlePianoRollover(sel_index, sel_dbindex);
                  m_hover_last = sel_index;
            }

      }

      if(event.Leaving())
      {
            gparent->HandlePianoRollover(-1, -1);
            gparent->HandlePianoRolloverIcon(-1, -1);

            m_index_last = -1;
            m_hover_icon_last = -1;
            m_hover_last = -1;
      }

      /*
      Todo:
      Could do something like this to better encapsulate the pianowin
      Allows us to get rid of global statics...

      wxCommandEvent ev(MyPianoEvent);    // Private event
      ..set up event to specify action...SelectChart, SetChartThumbnail, etc
      ::PostEvent(pEventReceiver, ev);    // event receiver passed to ctor

      */

}

void PianoWin::ResetRollover(void)
{
      m_index_last = -1;
      m_hover_icon_last = -1;
      m_hover_last = -1;
}



#ifdef USE_WIFI_CLIENT
//------------------------------------------------------------------------------
//          WiFiStat Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WiFiStatWin, wxWindow)
        EVT_PAINT(WiFiStatWin::OnPaint)
        EVT_SIZE(WiFiStatWin::OnSize)
        END_EVENT_TABLE()

WiFiStatWin::WiFiStatWin(wxFrame *frame):
        wxWindow(frame, wxID_ANY,wxPoint(20,20), wxSize(5,5), wxSIMPLE_BORDER)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);  // on WXMSW, this prevents flashing on color scheme change

    SetColorScheme((ColorScheme)0);

    for(int ista = 0 ; ista < NSIGBARS ; ista++)
        m_quality[ista] = 0;

    m_bserverstat = true;
}

WiFiStatWin::~WiFiStatWin(void)
{
}


void WiFiStatWin::OnSize(wxSizeEvent& event)
{
}

void WiFiStatWin::SetColorScheme(ColorScheme cs)
{
    backBrush = wxBrush(GetGlobalColor(_T("UIBDR")), wxSOLID);

    qual_hiBrush =   wxBrush(GetGlobalColor(_T("CHYLW")), wxSOLID);    //Yellow
    secureBrush =    wxBrush(GetGlobalColor(_T("UINFO")), wxSOLID);    //Orange

    qual_hiNewBrush =   wxBrush(GetGlobalColor(_T("UGREN")), wxSOLID); //Bright Green
    secureNewBrush =    wxBrush(GetGlobalColor(_T("URED")), wxSOLID);  //Bright Red

}

void WiFiStatWin::OnPaint(wxPaintEvent& event)
{
    int width, height;
    GetClientSize(&width, &height );
    wxPaintDC dc(this);

    dc.SetBackground(backBrush);
    dc.Clear();

    int bar_total = width / NSIGBARS;

//    Create the Signal Strength Indicators
    dc.SetBrush(backBrush);
    wxPen ppPen(GetGlobalColor(_T("UBLCK")), 1, wxSOLID);
    dc.SetPen(ppPen);

    if(m_bserverstat)
    {
        for(int ista = 0 ; ista < NSIGBARS ; ista++)
        {
            if(0 != m_quality[ista])
            {
                int x = width - bar_total * (ista + 1);

                dc.SetBrush(backBrush);
                dc.DrawRectangle(x+2, 2, bar_total-4 , height-4);

                // Old stations get soft color bars
                if(m_age[ista])
                {
                    dc.SetBrush(qual_hiBrush);
                    if(m_secure[ista])
                        dc.SetBrush(secureBrush);
                }
                else
                {
                    dc.SetBrush(qual_hiNewBrush);
                    if(m_secure[ista])
                        dc.SetBrush(secureNewBrush);
                }

                DrawBars(dc, x+2, 2, bar_total-4 , height-4, m_quality[ista], 100);
            }
        }
    }
    else
    {
        wxPen yellowPen(GetGlobalColor(_T("CHYLW")), 1, wxSOLID);
        dc.SetPen(yellowPen);

        dc.DrawLine(1, 1, width-1, 1);
        dc.DrawLine(width-1, 1, width-1, height-1);
        dc.DrawLine(width-1, height-1, 1, height-1);
        dc.DrawLine(1, height-1, 1, 1);
    }
}


void WiFiStatWin::DrawBars(wxDC &dc, int x, int y, int box_width, int box_height, int val, int val_max)
{
    int xb = 0;
    //  Scale onto 0..50, so we can draw 5 bars = 50 points
    int aval = (val * 50) / val_max;

    int nBars = ((aval) / 10);

    int bar_w = box_width / 5;

    for(int i=0 ; i<nBars ; i++)
    {
        xb = x + (i * bar_w) + 2;
        dc.DrawRectangle(xb, y+2, bar_w - 2 , box_height-4);
    }

    // partial bar
    xb += bar_w;
    dc.DrawRectangle(xb, y+2, bar_w * (aval % 10) / 10, box_height-4);

}



void WiFiStatWin::TextDraw(const char *text)
{
    Refresh(true);
}

void WiFiStatWin::SetNumberStations(int n)
{
    m_nstations = n;

    Refresh(true);
}

void WiFiStatWin::SetStationQuality(int istation, int quality)
{
    m_quality[istation] = quality;
}
void WiFiStatWin::SetStationSecureFlag(int istation, int flag)
{
    m_secure[istation] = flag;
}
void WiFiStatWin::SetStationAge(int istation, int age)
{
    m_age[istation] = age;
}

#endif


