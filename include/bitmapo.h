/******************************************************************************
 * $Id: bitmapo.h,v 1.3 2010/04/27 01:44:36 bdbcat Exp $
 *
 * Project:  OpenCP
 * Purpose:  Optimized wxBitmap Object
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
 * $Log: bitmapo.h,v $
 * Revision 1.3  2010/04/27 01:44:36  bdbcat
 * Build 426
 *
 * Revision 1.2  2006/10/07 03:50:54  dsr
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2006/08/21 05:52:11  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.3  2006/03/16 03:28:12  dsr
 * Cleanup tabs
 *
 * Revision 1.2  2006/02/23 01:14:50  dsr
 * Cleanup
 *
 *
 *
 */

#ifndef _WX_BITMAPO_H_
#define _WX_BITMAPO_H_

void *x_malloc(size_t t);

#ifdef dyUSE_BITMAPO


#ifdef __WXMSW__
#include "wx/msw/gdiimage.h"
#endif

#include "wx/gdicmn.h"
#include "wx/palette.h"

#ifdef __WXX11__
#include "wx/x11/private.h"
#endif

class WXDLLEXPORT wxDC;
class WXDLLEXPORT wxControl;
class WXDLLEXPORT wxBitmap;
class WXDLLEXPORT wxBitmapHandler;
class WXDLLEXPORT wxIcon;
class WXDLLEXPORT wxMask;
class WXDLLEXPORT wxCursor;
class WXDLLEXPORT wxControl;
class WXDLLEXPORT wxImage;
class WXDLLEXPORT wxPalette;

// ----------------------------------------------------------------------------
// Bitmap data
//
// NB: this class is private, but declared here to make it possible inline
//     wxBitmap functions accessing it
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// wxBitmapo: a mono or colour bitmap
// ----------------------------------------------------------------------------

class WXDLLEXPORT wxBitmapo : public wxBitmap
{
public:
    // default ctor creates an invalid bitmap, you must Create() it later
    wxBitmapo(); //{ Init(); }
//    virtual ~wxBitmapo();





      // ctor
      // Create from Data
      wxBitmapo(void *pPix, int width, int height, int depth)
            { (void)CreateFromData(pPix, width, height, depth );}

      // ctor
      // Create from wxImage
      wxBitmapo(const wxImage& image, int depth)
            { CreateFromImage(image, depth );}



#ifdef __WXX11__
      // Create from XImage
      wxBitmapo(XImage *data_image, int width, int height, int depth, bool bShared = 0)
            {CreateFromXImage( data_image, width, height, depth, bShared );}


#endif




    // Implementation
public:


protected:
//    creates the bitmap from data, supposed to be called from ctor
      bool CreateFromData(void *pPix, int width, int height, int depth);

//    or from wximage
      bool CreateFromImage( const wxImage& image, int depth );


//    or from Ximage
#ifdef __WXX11__
      bool CreateFromXImage( XImage *data_image, int width, int height, int depth, bool bShared );
#endif

private:

    DECLARE_DYNAMIC_CLASS(wxBitmapo)
};


#endif      // dyUSE_BITMAPO

#endif  // _WX_BITMAPO_H_
