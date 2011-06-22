/******************************************************************************
 * $Id: dymemdc.h,v 1.3 2010/04/27 01:44:56 bdbcat Exp $
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
 * $Log: dymemdc.h,v $
 * Revision 1.3  2010/04/27 01:44:56  bdbcat
 * Build 426
 *
 * Revision 1.2  2007/05/03 13:31:19  dsr
 * Major refactor for 1.2.0
 *
 * Revision 1.1.1.1  2006/08/21 05:52:11  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.1.1.1  2006/04/19 03:23:27  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.3  2006/03/16 03:28:12  dsr
 * Cleanup tabs
 *
 * Revision 1.2  2006/02/23 01:21:51  dsr
 * Cleanup
 *
 *
 *
 */


#ifndef _DY_DCMEM_H_
#define _DY_DCMEM_H_

#include "wx/dcclient.h"
#include "wx/dcmemory.h"

#ifdef __WXMSW__
      #include "wx/msw/dib.h"
#endif

class WXDLLEXPORT dyMemDC : public wxMemoryDC
{
public:
      dyMemDC();

//      void SelectObject(const wxBitmap& bitmap){wxMemoryDC::SelectObject(bitmap);}

      //    Satisfy wxX11 2.8.0
      void SelectObject(wxBitmap& bitmap){wxMemoryDC::SelectObject(bitmap);}

//    Add a method to select a DIB section directly into the DC
#ifdef dyUSE_DIBSECTION
      void SelectObject(wxDIB& dib);
#endif

protected:

private:
#ifdef dyUSE_DIBSECTION
      wxDIB *m_pselectedDIB;
#endif

    DECLARE_DYNAMIC_CLASS_NO_COPY(dyMemDC)
};

#endif
    // _DY_DCMEM_H_
