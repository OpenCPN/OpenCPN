/******************************************************************************
 *
 * Project:  OpenCP
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
 * $Log: thumbwin.h,v $
 * Revision 1.5  2010/04/27 01:45:32  bdbcat
 * Build 426
 *
 * Revision 1.4  2008/04/10 01:01:32  bdbcat
 * Cleanup
 *
 * Revision 1.3  2008/03/30 23:29:52  bdbcat
 * Cleanup/optimize
 *
 * Revision 1.2  2007/05/03 13:31:19  dsr
 * Major refactor for 1.2.0
 *
 * Revision 1.1.1.1  2006/08/21 05:52:11  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.2  2006/05/19 19:36:19  dsr
 * Cleanup
 *
 * Revision 1.1.1.1  2006/04/19 03:23:27  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.5  2006/04/19 00:59:54  dsr
 * Thumbwin sizing
 *
 * Revision 1.4  2006/03/16 03:28:12  dsr
 * Cleanup tabs
 *
 * Revision 1.3  2006/02/23 01:27:46  dsr
 * Cleanup
 *
 * Revision 1.2  2006/02/09 14:03:46  dsr
 * Define ThumbData and friends
 *
 * Revision 1.3  2006/02/10 03:19:06  dsr
 * *** empty log message ***
 *
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
      ThumbWin(wxFrame *frame);
      virtual ~ThumbWin();

      void Resize(void);
      void SetMaxSize(wxSize const &max_size);


      wxBitmap     *pThumbShowing;
      ChartBase    *pThumbChart;

private:
      void OnPaint(wxPaintEvent& event);

      wxSize      m_max_size;

DECLARE_EVENT_TABLE()
};

#endif
