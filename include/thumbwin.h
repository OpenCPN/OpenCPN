/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Thumbnail Object
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
 ***************************************************************************
 *
 */


//

#ifndef __thumbwin_H__
#define __thumbwin_H__


// Include wxWindows' headers

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

//#include "ocpn_pixel.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Fwd declarations
//----------------------------------------------------------------------------

class ChartBase;

//----------------------------------------------------------------------------
// ThumbWin
//----------------------------------------------------------------------------
class ThumbWin: public wxWindow
{
public:
      ThumbWin();
      ThumbWin(wxWindow *parent);
      virtual ~ThumbWin();

      void Resize(void);
      void SetMaxSize(wxSize const &max_size);
      const wxBitmap &GetBitmap(void);


      wxBitmap     m_bitmap;
      ChartBase    *pThumbChart;

private:
      void OnPaint(wxPaintEvent& event);

      wxSize      m_max_size;

DECLARE_EVENT_TABLE()
};

#endif
