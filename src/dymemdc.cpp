/******************************************************************************
 * $Id: dymemdc.cpp,v 1.3 2010/04/27 01:40:44 bdbcat Exp $
 *
 * Project:  OpenCP
 * Purpose:  Enhanced wxMemoryDC
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
 * $Log: dymemdc.cpp,v $
 * Revision 1.3  2010/04/27 01:40:44  bdbcat
 * Build 426
 *
 * Revision 1.2  2007/05/03 13:23:55  dsr
 * Major refactor for 1.2.0
 *
 * Revision 1.1.1.1  2006/08/21 05:52:19  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.2  2006/08/04 11:42:01  dsr
 * no message
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.5  2006/04/19 00:44:09  dsr
 * *** empty log message ***
 *
 * Revision 1.4  2006/03/16 03:08:00  dsr
 * Cleanup tabs
 *
 * Revision 1.3  2006/03/13 05:06:03  dsr
 * Cleanup
 *
 * Revision 1.2  2006/02/23 01:42:47  dsr
 * Cleanup
 *
 *
 *
 */


// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------


// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/log.h"
#endif

#ifdef __WXMSW__
#include "wx/msw/private.h"
#include "wx/msw/dib.h"
#endif

#include "dychart.h"
//#include "dymemdc.h"

CPL_CVSID("$Id: dymemdc.cpp,v 1.3 2010/04/27 01:40:44 bdbcat Exp $");


// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(dyMemDC, wxMemoryDC)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// dyMemDC
// ----------------------------------------------------------------------------

dyMemDC::dyMemDC()
{
}


#ifdef dyUSE_DIBSECTION
void dyMemDC::SelectObject(wxDIB& dib)
{
    // select old bitmap out of the device context
    if ( m_oldBitmap )
    {
        ::SelectObject(GetHdc(), (HBITMAP) m_oldBitmap);
        if ( m_selectedBitmap.Ok() )
        {
#ifdef __WXDEBUG__
 //           m_selectedBitmap.SetSelectedInto(NULL);
#endif
            m_selectedBitmap = wxNullBitmap;
        }
    }

// check for whether the bitmap is already selected into a device context
//    wxASSERT_MSG( !bitmap.GetSelectedInto() ||
//                  (bitmap.GetSelectedInto() == this),
//                  wxT("Bitmap is selected in another wxMemoryDC, delete the first wxMemoryDC or use SelectObject(NULL)") );

/*
    m_selectedBitmap = bitmap;
    WXHBITMAP hBmp = m_selectedBitmap.GetHBITMAP();
    if ( !hBmp )
            return;                             // already selected
*/
      m_pselectedDIB = &dib;
      HBITMAP hDIB = m_pselectedDIB->GetHandle();
      if ( !hDIB)
            return;                             // already selected

#ifdef __WXDEBUG__
//    m_selectedBitmap.SetSelectedInto(this);
#endif

//    hBmp = (WXHBITMAP)::SelectObject(GetHdc(), (HBITMAP)hBmp);

      hDIB = (HBITMAP)::SelectObject(GetHdc(), hDIB);


      if ( !hDIB )
    {
        wxLogLastError(wxT("SelectObject(dyMemDC, DIB)"));

        wxFAIL_MSG(wxT("Couldn't select a DIB into dyMemDC"));
    }

    else if ( !m_oldBitmap )
    {
        m_oldBitmap = hDIB;
    }

}

#endif      //dyUSE_DIBSECTION
