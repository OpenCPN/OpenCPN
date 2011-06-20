/******************************************************************************
 * $Id: bitmapo.cpp,v 1.4 2010/04/27 01:40:44 bdbcat Exp $
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
 * $Log: bitmapo.cpp,v $
 * Revision 1.4  2010/04/27 01:40:44  bdbcat
 * Build 426
 *
 * Revision 1.3  2008/03/30 21:39:04  bdbcat
 * Update for Mac OSX/Unicode
 *
 * Revision 1.2  2006/10/07 03:50:27  dsr
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2006/08/21 05:52:19  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.3  2006/08/04 11:42:01  dsr
 * no message
 *
 * Revision 1.2  2006/07/28 20:29:12  dsr
 * Clean up MIT_SHM code
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.4  2006/04/19 00:34:06  dsr
 * *** empty log message ***
 *
 * Revision 1.3  2006/03/16 03:07:59  dsr
 * Cleanup tabs
 *
 * Revision 1.2  2006/02/23 01:30:30  dsr
 * Cleanup
 *
 *
 *
 */


//      Original comment header for xshm test code
//      imported from xshm.c
/* xshm.c */

/*
 * Example of how to use the X Shared Memory extension: MIT_SHM.
 * This code was lifted from my Mesa library.  It hasn't been tested
 * in this form but should be close enough for you to get it working.
 * Beware that this extension isn't available on all systems.  Your
 * application code should use #ifdef's around this code so it can be
 * omitted on systems that don't have it, then fallback to using a regular
 * XImage.
 *
 * Brian Paul  Sep, 20, 1995  brianp@ssec.wisc.edu
 */




// ============================================================================
// declarations
// ============================================================================

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
      #include "wx/wx.h"
#endif


#include "dychart.h"


#ifdef dyUSE_BITMAPO

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------


#include "bitmapo.h"


#ifndef WX_PRECOMP
    #include <stdio.h>

    #include "wx/list.h"
    #include "wx/utils.h"
    #include "wx/app.h"
    #include "wx/palette.h"
    #include "wx/dcmemory.h"
    #include "wx/bitmap.h"
    #include "wx/icon.h"
#endif

#ifdef __WXMSW__
      #include "wx/msw/private.h"
      #include "wx/log.h"
      #include "wx/msw/dib.h"
#endif

#include "wx/bitmap.h"
#include "wx/icon.h"
#include "wx/log.h"
#include "wx/image.h"
#include "wx/app.h"




#include "wx/math.h"



// missing from mingw32 header
#ifndef CLR_INVALID
    #define CLR_INVALID ((COLORREF)-1)
#endif // no CLR_INVALID



#ifdef __WXX11__
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

CPL_CVSID("$Id: bitmapo.cpp,v 1.4 2010/04/27 01:40:44 bdbcat Exp $");

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxBitmapo, wxGDIObject)


//class wxBitmapRefData;

// ============================================================================
// implementation
// ============================================================================

wxBitmapo::wxBitmapo()
{
}

//wxBitmapo::~wxBitmapo()
//{
//}

//          The included methods are very different for MSW and X11
//          See the Code


#ifdef __WXX11__
//    This is the X11 Version


class mywxBitmapRefData: public wxObjectRefData
{
public:
    mywxBitmapRefData();
    ~mywxBitmapRefData();

    WXPixmap        m_pixmap;
    WXPixmap        m_bitmap;
    WXDisplay      *m_display;
    wxMask         *m_mask;
    int             m_width;
    int             m_height;
    int             m_bpp;
    wxPalette      *m_palette;
};

mywxBitmapRefData::mywxBitmapRefData()
{
    m_pixmap = NULL;
    m_bitmap = NULL;
    m_display = NULL;
    m_mask = (wxMask *) NULL;
    m_width = 0;
    m_height = 0;
    m_bpp = 0;
    m_palette = (wxPalette *) NULL;
}

mywxBitmapRefData::~mywxBitmapRefData()
{
    if (m_pixmap) XFreePixmap( (Display*) m_display, (Pixmap) m_pixmap );
    if (m_bitmap) XFreePixmap( (Display*) m_display, (Pixmap) m_bitmap );
    if (m_mask) delete m_mask;
    if (m_palette) delete m_palette;
}


//#define M_BMPDATA ((wxBitmapRefData *)m_refData)


/*
 * Error handling.
 */
/*
static int ErrorFlag = 0;


static int HandleXError( Display *dpy, XErrorEvent *event )
{
      ErrorFlag = 1;
      return 0;
}
*/

// ----------------------------------------------------------------------------
// Create from XImage
// ----------------------------------------------------------------------------

bool wxBitmapo::CreateFromXImage( XImage *data_image, int width, int height, int depth, bool bShared )
{



//    Do some basic setup in the parent  wxBitmap class
      Create(width, height, -1);

      Display *xdisplay = (Display *)GetDisplay();

/*
      int ignore;
      bool bMIT_SHM;
      if (XQueryExtension( xdisplay, "MIT-SHM", &ignore, &ignore, &ignore ))
            bMIT_SHM = true;
      else
            bMIT_SHM = false;
*/

//    int xscreen = DefaultScreen( xdisplay );
//    Window xroot = RootWindow( xdisplay, xscreen );
//    Visual* xvisual = DefaultVisual( xdisplay, xscreen );

//    int bpp = wxTheApp->GetVisualInfo(xdisplay)->m_visualDepth;


// Blit picture

// ugly hack
// I don't have the definition of wxBitmapRefData available,
//  as it is inside x11/bitmap.cpp.
// So, I clone it here as mywxBitmapRefData, hoping that the
//  compiler will not move the member elements around
        Pixmap mypixmap = (Pixmap )(((mywxBitmapRefData *)GetRefData())->m_pixmap);

        GC gc = XCreateGC( xdisplay, mypixmap, 0, NULL );

        if(bShared)
              XShmPutImage( xdisplay, mypixmap, gc, data_image, 0, 0, 0, 0, width, height, False );
        else
              XPutImage( xdisplay, mypixmap, gc, data_image, 0, 0, 0, 0, width, height );

        XFreeGC( xdisplay, gc );

        return true;
}

static int ErrorFlag;
// ----------------------------------------------------------------------------
// Create from Data
// ----------------------------------------------------------------------------

bool wxBitmapo::CreateFromData( void *pPix, int width, int height, int depth )
{

    XImage *img;
//    Do some basic setup in the parent wxBitmap class
    Create(width, height, -1);

    Display *xdisplay = (Display *)GetDisplay();

    int xscreen = DefaultScreen( xdisplay );
    Visual* xvisual = DefaultVisual( xdisplay, xscreen );

    int bpp = wxTheApp->GetVisualInfo(xdisplay)->m_visualDepth;




#ifdef ocpUSE_MITSHM

    bool buse_mit = false;
    XShmSegmentInfo shminfo;

//      Check to see if the basic extension is supported
    int ignore;
    bool bMIT_SHM = XQueryExtension( xdisplay, "MIT-SHM", &ignore, &ignore, &ignore );


    if(bMIT_SHM)
    {

        img = XShmCreateImage( xdisplay, xvisual, bpp,
                             ZPixmap, NULL, &shminfo,
                             width, height );
        if (img == NULL)
        {
          wxLogError("XShmCreateImage failed!");
          goto after_check;
        }

//    Identify and allocate the shared memory buffer
        shminfo.shmid = shmget( IPC_PRIVATE, img->bytes_per_line * img->height, IPC_CREAT|0777 );
        if (shminfo.shmid < 0)
        {
          XDestroyImage( img );
          img = NULL;
          wxLogMessage( _T("alloc_back_buffer: Shared memory error (shmget), disabling." ));
          goto after_check;
        }

        shminfo.shmaddr = img->data
              = (char*)shmat( shminfo.shmid, 0, 0 );
        if (shminfo.shmaddr == (char *) -1)
        {
          XDestroyImage( img );
          img = NULL;
          wxLogMessage(_T("shmat failed"));
          goto after_check;
        }

//    Make some further checks
        shminfo.readOnly = False;
        ErrorFlag = 0;

//Todo      XSetErrorHandler( HandleXError );
// This may trigger the X protocol error we're ready to catch:
        XShmAttach( xdisplay, &shminfo );
        XSync( xdisplay, False );

        if (ErrorFlag)
        {
// we are on a remote display, this error is normal, don't print it
          XFlush( xdisplay );
          ErrorFlag = 0;
          XDestroyImage( img );
          shmdt( shminfo.shmaddr );
          shmctl( shminfo.shmid, IPC_RMID, 0 );
//            return NULL;
        }

        shmctl( shminfo.shmid, IPC_RMID, 0 ); /* nobody else needs it */

        buse_mit = true;                // passed all tests

    }
after_check:
          // if bMIT_SHM
#endif

    if(NULL == img)
    {
        img = XCreateImage( xdisplay, xvisual, bpp, ZPixmap, 0, 0, width, height, 32, 0 );
        img->data = (char*) x_malloc( img->bytes_per_line * img->height );

        if (img->data == NULL)
        {
            XDestroyImage( img );
            img = NULL;
            wxLogError( wxT("Bitmapo:Cannot malloc for data image.") );
            return false;
        }
    }


//    Faster render

      if((pPix != NULL ) && (NULL != img))
      {
          unsigned char* data = (unsigned char *)pPix;
          if(depth == 32)                          // special fast case
          {
              for (int y = 0; y < height; y++)
              {
                  char *pd = img->data + (y * img->bytes_per_line);
                  unsigned char *ps = data + (y * width * 4);
                  memcpy(pd, ps, width*4);
              }
          }

          else
          {
              int *pi = (int *)img->data;
              int index = 0;
              for (int y = 0; y < height; y++)
              {
                  for (int x = 0; x < width; x++)
                  {
                      int ri = *(int *)(&data[index]);
                      index++;
                      index++;
                      index++;

                      *pi = ri;
                      pi++;

                  }
              }
          }
      }

        // Blit picture

// ugly hack
// I don't have the definition of wxBitmapRefData available,
//  as it is inside x11/bitmap.cpp.
// So, I clone it here as mywxBitmapRefData, hoping that the
//  compiler will not move the member elements around
      Pixmap mypixmap = (Pixmap )(((mywxBitmapRefData *)GetRefData())->m_pixmap);
      GC gc = XCreateGC( xdisplay, mypixmap, 0, NULL );

#ifdef ocpUSE_MITSHM
      if(buse_mit)
      {
        XShmPutImage( xdisplay, mypixmap, gc, img, 0, 0, 0, 0, width, height, False );

        XShmDetach( xdisplay, &shminfo );
        XDestroyImage( img );
        shmdt( shminfo.shmaddr );
      }
      else
      {
          XPutImage( xdisplay, mypixmap, gc, img, 0, 0, 0, 0, width, height );
          XDestroyImage( img );
      }
#else

      XPutImage( xdisplay, mypixmap, gc, img, 0, 0, 0, 0, width, height );
      XDestroyImage( img );
#endif


      XFreeGC( xdisplay, gc );

     return TRUE;
}







// ----------------------------------------------------------------------------
// Create from Image
//  Simplified and optimized from original wxWidgets wxbitmap() class
// ----------------------------------------------------------------------------

bool wxBitmapo::CreateFromImage( const wxImage& image, int depth )
{

//    Do some basic setup
      Create(image.GetWidth(), image.GetHeight(), depth);


//    UnRef();

//    wxCHECK_MSG( image.Ok(), FALSE, wxT("invalid image") )
//    wxCHECK_MSG( depth == -1, FALSE, wxT("invalid bitmap depth") )

//    m_refData = new wxBitmapRefData();

//    M_BMPDATA->m_display = wxGlobalDisplay();

//    Display *xdisplay = (Display*) M_BMPDATA->m_display;
      Display *xdisplay = (Display *)GetDisplay();

      int xscreen = DefaultScreen( xdisplay );
      Window xroot = RootWindow( xdisplay, xscreen );
      Visual* xvisual = DefaultVisual( xdisplay, xscreen );

//    int bpp = wxTheApp->GetVisualInfo(M_BMPDATA->m_display)->m_visualDepth;
      int bpp = wxTheApp->GetVisualInfo(xdisplay)->m_visualDepth;

      int width = image.GetWidth();
      int height = image.GetHeight();
//    M_BMPDATA->m_width = width;
//    M_BMPDATA->m_height = height;

      if (depth != 1) depth = bpp;
//    M_BMPDATA->m_bpp = depth;

      if (depth == 1)
      {
            wxFAIL_MSG( wxT("mono images later") );
      }
      else
      {
        // Create image

            XImage *data_image = XCreateImage( xdisplay, xvisual, bpp, ZPixmap, 0, 0, width, height, 32, 0 );
            data_image->data = (char*) x_malloc( data_image->bytes_per_line * data_image->height );

            if (data_image->data == NULL)
            {
                wxLogError( wxT("Bitmapo:Cannot malloc for data image.") );
                return FALSE;
            }

 // Done in Create()
//          M_BMPDATA->m_pixmap = (WXPixmap) XCreatePixmap( xdisplay, xroot, width, height, depth );

        // Create mask

            XImage *mask_image = (XImage*) NULL;
            if (image.HasMask())
            {
                  mask_image = XCreateImage( xdisplay, xvisual, 1, ZPixmap, 0, 0, width, height, 32, 0 );
                  mask_image->data = (char*) x_malloc( mask_image->bytes_per_line * mask_image->height );

                  if (mask_image->data == NULL)
                  {
                wxLogError( wxT("Bitmapo:Cannot malloc for data image.") );
                return FALSE;
                  }

                  wxMask *mask = new wxMask();
                  mask->SetDisplay( xdisplay );
                  mask->SetBitmap( (WXPixmap) XCreatePixmap( xdisplay, xroot, width, height, 1 ) );

                  SetMask( mask );
            }

            if (bpp < 8) bpp = 8;

        // Render

//    Faster render

            unsigned char* data = image.GetData();
            int *pi = (int *)data_image->data;

            int index = 0;
            for (int y = 0; y < height; y++)
            {
                  for (int x = 0; x < width; x++)
                  {
//                      int r = data[index];
                        int ri = *(int *)(&data[index]);
                        index++;
                        index++;
                        index++;


                        *pi = ri;
                        pi++;

                  }
            }

        // Blit picture

// ugly hack
// I don't have the definition of wxBitmapRefData available,
//  as it is inside x11/bitmap.cpp.
// So, I clone it here as mywxBitmapRefData, hoping that the
//  compiler will not move the member elements around
            Pixmap mypixmap = (Pixmap )(((mywxBitmapRefData *)GetRefData())->m_pixmap);
            GC gc = XCreateGC( xdisplay, mypixmap, 0, NULL );
            XPutImage( xdisplay, mypixmap, gc, data_image, 0, 0, 0, 0, width, height );

            XDestroyImage( data_image );
            XFreeGC( xdisplay, gc );

        // Blit mask

            if (image.HasMask())
            {
                  GC gc = XCreateGC( xdisplay, (Pixmap) GetMask()->GetBitmap(), 0, NULL );
                  XPutImage( xdisplay, (Pixmap) GetMask()->GetBitmap(), gc, mask_image, 0, 0, 0, 0, width, height );

                  XDestroyImage( mask_image );
                  XFreeGC( xdisplay, gc );
            }
      }

      return TRUE;
}

#endif            //__WXX11__


#ifdef __WXMSW__

// ----------------------------------------------------------------------------
// Create from Data
// ----------------------------------------------------------------------------
bool wxBitmapo::CreateFromData( void *pPix, int width, int height, int depth )
{
      m_refData = CreateData();                 // found in wxBitmap
//    int width = image.GetWidth();
//    int height0 = image.GetHeight();

      int height0 = height;
    int sizeLimit = 1280*1024 *3;

    int bmpHeight = height0;
//    int height = bmpHeight;

    // calc the number of bytes per scanline and padding
    int bytePerLine = width*3;
    int sizeDWORD = sizeof( DWORD );
    int lineBoundary = bytePerLine % sizeDWORD;
    int padding = 0;
    if( lineBoundary > 0 )
    {
        padding = sizeDWORD - lineBoundary;
        bytePerLine += padding;
    }

    // set bitmap parameters
    SetWidth( width );
    SetHeight( bmpHeight );
    if (depth == -1) depth = wxDisplayDepth();
    SetDepth( depth );


    // create a DIB header
    int headersize = sizeof(BITMAPINFOHEADER);
    BITMAPINFO *lpDIBh = (BITMAPINFO *) malloc( headersize );
    wxCHECK_MSG( lpDIBh, FALSE, wxT("could not allocate memory for DIB header") );
    // Fill in the DIB header
    lpDIBh->bmiHeader.biSize = headersize;
    lpDIBh->bmiHeader.biWidth = (DWORD)width;
    lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
    lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
    //   the general formula for biSizeImage:
    //      ( ( ( ((DWORD)width*24) +31 ) & ~31 ) >> 3 ) * height;
    lpDIBh->bmiHeader.biPlanes = 1;
    lpDIBh->bmiHeader.biBitCount = 24;
    lpDIBh->bmiHeader.biCompression = BI_RGB;
    lpDIBh->bmiHeader.biClrUsed = 0;
    // These seem not really needed for our purpose here.
    lpDIBh->bmiHeader.biClrImportant = 0;
    lpDIBh->bmiHeader.biXPelsPerMeter = 0;
    lpDIBh->bmiHeader.biYPelsPerMeter = 0;
    // memory for DIB data
    unsigned char *lpBits;
    lpBits = (unsigned char *)malloc( lpDIBh->bmiHeader.biSizeImage );
    if( !lpBits )
    {
        wxFAIL_MSG( wxT("could not allocate memory for DIB") );
        free( lpDIBh );
        return FALSE;
    }

    // create and set the device-dependent bitmap
    HDC hdc = ::GetDC(NULL);
    HDC memdc = ::CreateCompatibleDC( hdc );
    HBITMAP hbitmap;
//    hbitmap = ::CreateCompatibleBitmap( hdc, width, bmpHeight );

//    if(hbitmap == NULL)
//          int cop =0;

//    ::SelectObject( memdc, hbitmap);


// copy image data into DIB data
    unsigned char *data = (unsigned char *)pPix;
    int i, j;
    int origin = 0;
    unsigned char *ptdata = data;
    unsigned char *ptbits;

    ptbits = lpBits;

    if(pPix)
      {
            for( j=0; j<height; j++ )
            {

                  memcpy(ptbits, ptdata, width * 3);
                  ptbits += width * 3;
                  ptdata += width * 3;

                  for( i=0; i< padding; i++ )   *(ptbits++) = 0;
            }
      }

      else
      {
            for( j=0; j<height; j++ )
            {

                  memset(ptbits, 0,  width * 3);
                  ptbits += width * 3;

                  for( i=0; i< padding; i++ )   *(ptbits++) = 0;
            }
      }




hbitmap = CreateDIBitmap( hdc, &(lpDIBh->bmiHeader), CBM_INIT, lpBits, lpDIBh, DIB_RGB_COLORS );
// The above line is equivalent to the following two lines.
//    hbitmap = ::CreateCompatibleBitmap( hdc, width, height );
//    ::SetDIBits( hdc, hbitmap, 0, height, lpBits, lpDIBh, DIB_RGB_COLORS);
    // or the following lines
//    hbitmap = ::CreateCompatibleBitmap( hdc, width, height );
//    HDC memdc = ::CreateCompatibleDC( hdc );
//    ::SelectObject( memdc, hbitmap);
//    ::SetDIBitsToDevice( memdc, 0, 0, width, height,
//              0, 0, 0, height, (void *)lpBits, lpDIBh, DIB_RGB_COLORS);
//    ::SelectObject( memdc, 0 );
//    ::DeleteDC( memdc );
    SetHBITMAP( (WXHBITMAP) hbitmap );


    // free allocated resources
    ::DeleteDC( memdc );
    ::ReleaseDC(NULL, hdc);
    free(lpDIBh);
    free(lpBits);

    return TRUE;
}






// ----------------------------------------------------------------------------
// wxImage from conversion
// ----------------------------------------------------------------------------


bool wxBitmapo::CreateFromImage( const wxImage& image, int depth )
{
    wxCHECK_MSG( image.Ok(), FALSE, wxT("invalid image") )

      m_refData = CreateData();                 // found in wxBitmap

    // sizeLimit is the MS upper limit for the DIB size
    int sizeLimit = 1280*1024 *3;

    // width and height of the device-dependent bitmap
    int width = image.GetWidth();
    int bmpHeight = image.GetHeight();

    // calc the number of bytes per scanline and padding
    int bytePerLine = width*3;
    int sizeDWORD = sizeof( DWORD );
    int lineBoundary = bytePerLine % sizeDWORD;
    int padding = 0;
    if( lineBoundary > 0 )
    {
        padding = sizeDWORD - lineBoundary;
        bytePerLine += padding;
    }
    // calc the number of DIBs and heights of DIBs
    int numDIB = 1;
    int hRemain = 0;
    int height = sizeLimit/bytePerLine;
    if( height >= bmpHeight )
        height = bmpHeight;
    else
    {
        numDIB =  bmpHeight / height;
        hRemain = bmpHeight % height;
        if( hRemain >0 )  numDIB++;
    }

    // set bitmap parameters
    wxCHECK_MSG( image.Ok(), FALSE, wxT("invalid image") );
    SetWidth( width );
    SetHeight( bmpHeight );
    if (depth == -1) depth = wxDisplayDepth();
    SetDepth( depth );

#if wxUSE_PALETTE
    // Copy the palette from the source image
    SetPalette(image.GetPalette());
#endif // wxUSE_PALETTE

    // create a DIB header
    int headersize = sizeof(BITMAPINFOHEADER);
    BITMAPINFO *lpDIBh = (BITMAPINFO *) malloc( headersize );
    wxCHECK_MSG( lpDIBh, FALSE, wxT("could not allocate memory for DIB header") );
    // Fill in the DIB header
    lpDIBh->bmiHeader.biSize = headersize;
    lpDIBh->bmiHeader.biWidth = (DWORD)width;
    lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
    lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
    //   the general formula for biSizeImage:
    //      ( ( ( ((DWORD)width*24) +31 ) & ~31 ) >> 3 ) * height;
    lpDIBh->bmiHeader.biPlanes = 1;
    lpDIBh->bmiHeader.biBitCount = 24;
    lpDIBh->bmiHeader.biCompression = BI_RGB;
    lpDIBh->bmiHeader.biClrUsed = 0;
    // These seem not really needed for our purpose here.
    lpDIBh->bmiHeader.biClrImportant = 0;
    lpDIBh->bmiHeader.biXPelsPerMeter = 0;
    lpDIBh->bmiHeader.biYPelsPerMeter = 0;
    // memory for DIB data
    unsigned char *lpBits;
    lpBits = (unsigned char *)malloc( lpDIBh->bmiHeader.biSizeImage );
    if( !lpBits )
    {
        wxFAIL_MSG( wxT("could not allocate memory for DIB") );
        free( lpDIBh );
        return FALSE;
    }

    // create and set the device-dependent bitmap
    HDC hdc = ::GetDC(NULL);
    HDC memdc = ::CreateCompatibleDC( hdc );
    HBITMAP hbitmap;
//    hbitmap = ::CreateCompatibleBitmap( hdc, width, bmpHeight );

//    if(hbitmap == NULL)
//          int cop =0;

//    ::SelectObject( memdc, hbitmap);

#if wxUSE_PALETTE
    HPALETTE hOldPalette = 0;
    if (image.GetPalette().Ok())
    {
        hOldPalette = ::SelectPalette(memdc, (HPALETTE) image.GetPalette().GetHPALETTE(), FALSE);
        ::RealizePalette(memdc);
    }
#endif // wxUSE_PALETTE

    // copy image data into DIB data and then into DDB (in a loop)
    unsigned char *data = image.GetData();
    int i, j, n;
    int origin = 0;
    unsigned char *ptdata = data;
    unsigned char *ptbits;

    for( n=0; n<numDIB; n++ )
    {
        if( numDIB > 1 && n == numDIB-1 && hRemain > 0 )
        {
            // redefine height and size of the (possibly) last smaller DIB
            // memory is not reallocated
            height = hRemain;
            lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
            lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
        }
        ptbits = lpBits;

        for( j=0; j<height; j++ )
        {

            memcpy(ptbits, ptdata, width * 3);
                  ptbits += width * 3;
                  ptdata += width * 3;

/*
                  for( i=0; i<width; i++ )
            {
                *(ptbits++) = *(ptdata+2);
                *(ptbits++) = *(ptdata+1);
                *(ptbits++) = *(ptdata  );
                ptdata += 3;
            }
*/
            for( i=0; i< padding; i++ )   *(ptbits++) = 0;
        }
//        ::StretchDIBits( memdc, 0, origin, width, height,
//            0, 0, width, height, lpBits, lpDIBh, DIB_RGB_COLORS, SRCCOPY);
        origin += height;
        // if numDIB = 1,  lines below can also be used
    hbitmap = CreateDIBitmap( hdc, &(lpDIBh->bmiHeader), CBM_INIT, lpBits, lpDIBh, DIB_RGB_COLORS );
      if(hbitmap == NULL)
            int cop =0;
        // The above line is equivalent to the following two lines.
//    hbitmap = ::CreateCompatibleBitmap( hdc, width, height );
//    ::SetDIBits( hdc, hbitmap, 0, height, lpBits, lpDIBh, DIB_RGB_COLORS);
        // or the following lines
//    hbitmap = ::CreateCompatibleBitmap( hdc, width, height );
//    HDC memdc = ::CreateCompatibleDC( hdc );
//    ::SelectObject( memdc, hbitmap);
//    ::SetDIBitsToDevice( memdc, 0, 0, width, height,
//              0, 0, 0, height, (void *)lpBits, lpDIBh, DIB_RGB_COLORS);
//    ::SelectObject( memdc, 0 );
//    ::DeleteDC( memdc );
    }
    SetHBITMAP( (WXHBITMAP) hbitmap );

      if(!this->Ok())
         int cop = this->Ok();

#if wxUSE_PALETTE
    if (hOldPalette)
        SelectPalette(memdc, hOldPalette, FALSE);
#endif // wxUSE_PALETTE

    // similarly, created an mono-bitmap for the possible mask
    if( image.HasMask() )
    {
        hbitmap = ::CreateBitmap( (WORD)width, (WORD)bmpHeight, 1, 1, NULL );
        HGDIOBJ hbmpOld = ::SelectObject( memdc, hbitmap);
        if( numDIB == 1 )   height = bmpHeight;
        else                height = sizeLimit/bytePerLine;
        lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
        lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
        origin = 0;
        unsigned char r = image.GetMaskRed();
        unsigned char g = image.GetMaskGreen();
        unsigned char b = image.GetMaskBlue();
        unsigned char zero = 0, one = 255;
        ptdata = data;
        for( n=0; n<numDIB; n++ )
        {
            if( numDIB > 1 && n == numDIB - 1 && hRemain > 0 )
            {
                // redefine height and size of the (possibly) last smaller DIB
                // memory is not reallocated
                height = hRemain;
                lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
                lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
            }
            ptbits = lpBits;
            for( int j=0; j<height; j++ )
            {
                for(i=0; i<width; i++ )
                {
                    // was causing a code gen bug in cw : if( ( cr !=r) || (cg!=g) || (cb!=b) )
                    unsigned char cr = (*(ptdata++)) ;
                    unsigned char cg = (*(ptdata++)) ;
                    unsigned char cb = (*(ptdata++)) ;

                    if( ( cr !=r) || (cg!=g) || (cb!=b) )
                    {
                        *(ptbits++) = one;
                        *(ptbits++) = one;
                        *(ptbits++) = one;
                    }
                    else
                    {
                        *(ptbits++) = zero;
                        *(ptbits++) = zero;
                        *(ptbits++) = zero;
                    }
                }
                for( i=0; i< padding; i++ )   *(ptbits++) = zero;
            }
            ::StretchDIBits( memdc, 0, origin, width, height,\
                0, 0, width, height, lpBits, lpDIBh, DIB_RGB_COLORS, SRCCOPY);
            origin += height;
        }
        // create a wxMask object
        wxMask *mask = new wxMask();
        mask->SetMaskBitmap( (WXHBITMAP) hbitmap );
        SetMask( mask );
        // It will be deleted when the wxBitmap object is deleted (as of 01/1999)
        /* The following can also be used but is slow to run
        wxColour colour( GetMaskRed(), GetMaskGreen(), GetMaskBlue());
        wxMask *mask = new wxMask( *this, colour );
        SetMask( mask );
        */

        ::SelectObject( memdc, hbmpOld );
    }

    // free allocated resources
    ::DeleteDC( memdc );
    ::ReleaseDC(NULL, hdc);
    free(lpDIBh);
    free(lpBits);

#if WXWIN_COMPATIBILITY_2
    // check the wxBitmap object
    GetBitmapData()->SetOk();
#endif // WXWIN_COMPATIBILITY_2

    return TRUE;
}



#endif            // __WXMSW__

#endif            //dyUSE_BITMAPO
