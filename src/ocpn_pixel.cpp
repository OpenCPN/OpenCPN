/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Optimized wxBitmap Object
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
// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
      #include "wx/wx.h"
#endif


#include "dychart.h"
#include "ocpn_pixel.h"

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
#include "wx/gdicmn.h"
#include "wx/palette.h"


// missing from mingw32 header
#ifndef CLR_INVALID
    #define CLR_INVALID ((COLORREF)-1)
#endif // no CLR_INVALID


#ifdef ocpnUSE_ocpnBitmap
#ifdef __WXX11__
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif
#endif



#ifdef  __WXX11__

#ifdef ocpUSE_MITSHM
/*
 * MIT-SHM Test Error handling.
 */
static int MITErrorFlag = 0;
static int HandleXError( Display *dpy, XErrorEvent *event )
{
    MITErrorFlag = 1;
    return 0;
}
#endif



//---------------------------------------------------------------------------------------------------------
//              Private Memory Management
//---------------------------------------------------------------------------------------------------------

static void *x_malloc(size_t t)
{
    void *pr = malloc( t );

    //      malloc fails
    if( NULL == pr ) {
        wxLogMessage( _T("x_malloc...malloc fails with request of %d bytes."), t );

        // Cat the /proc/meminfo file

        char *p;
        char buf[2000];
        int len;

        int fd = open( "/proc/meminfo", O_RDONLY );

        if( fd == -1 ) exit( 1 );

        len = read( fd, buf, sizeof( buf ) - 1 );
        if( len <= 0 ) {
            close( fd );
            exit( 1 );
        }
        close( fd );
        buf[len] = 0;

        p = buf;
        while( *p ) {
//                        printf("%c", *p++);
        }

        exit( 0 );
        return NULL;                            // for MSVC
    }

    else {
        if( t > malloc_max ) {
            malloc_max = t;
//                      wxLogMessage(_T("New malloc_max: %d", malloc_max));
        }

        return pr;                                      // good return
    }

}


//----------------------------------------------------------------------
//      ocpnXImage Implementation
//----------------------------------------------------------------------

ocpnXImage::ocpnXImage(int width, int height)
{

    m_width = width;
    m_height = height;
    buse_mit = false;
    m_img = NULL;

    xdisplay = (Display *)wxGlobalDisplay();
    xscreen = DefaultScreen( xdisplay );
    xvisual = DefaultVisual( xdisplay, xscreen );
    int bpp = wxTheApp->GetVisualInfo(xdisplay)->m_visualDepth;

#ifdef ocpUSE_MITSHM

//      Check to see if the basic extension is supported
    int ignore;
    bool bMIT_SHM = XQueryExtension( xdisplay, "MIT-SHM", &ignore, &ignore, &ignore );


    if(bMIT_SHM)
    {
        m_img = XShmCreateImage( xdisplay, xvisual, bpp,
                                 ZPixmap, NULL, &shminfo,
                                 width, height );
        if (m_img == NULL)
        {
              wxLogError(_T("XShmCreateImage failed!"));
            goto after_check;
        }

//    Identify and allocate the shared memory buffer
        shminfo.shmid = shmget( IPC_PRIVATE, m_img->bytes_per_line * m_img->height, IPC_CREAT|0777 );
        if (shminfo.shmid < 0)
        {
            XDestroyImage( m_img );
            m_img = NULL;
            wxLogMessage( _T("alloc_back_buffer: Shared memory error (shmget), disabling.") );
            goto after_check;
        }

        shminfo.shmaddr = m_img->data  = (char*)shmat( shminfo.shmid, 0, 0 );

        if (shminfo.shmaddr == (char *) -1)
        {
            XDestroyImage( m_img );
            m_img = NULL;
            wxLogMessage(_T("shmat failed"));
            goto after_check;
        }

//    Make some further checks
        shminfo.readOnly = False;
        MITErrorFlag = 0;

        XSetErrorHandler( HandleXError );
// This may trigger the X protocol error we're ready to catch:
        XShmAttach( xdisplay, &shminfo );
        XSync( xdisplay, False );

        if (MITErrorFlag)
        {
// we are on a remote display, this error is normal, don't print it
            XFlush( xdisplay );
            MITErrorFlag = 0;
            XDestroyImage( m_img );
            m_img = NULL;
            shmdt( shminfo.shmaddr );
            shmctl( shminfo.shmid, IPC_RMID, 0 );
            goto after_check;
        }

        shmctl( shminfo.shmid, IPC_RMID, 0 ); /* nobody else needs it */

        buse_mit = true;                // passed all tests
    }

after_check:
          // if bMIT_SHM
#endif

    if(NULL == m_img)
    {
        m_img = XCreateImage( xdisplay, xvisual, bpp, ZPixmap, 0, 0, width, height, 32, 0 );
        m_img->data = (char*) x_malloc( m_img->bytes_per_line * m_img->height );

        if (m_img->data == NULL)
        {
            XDestroyImage( m_img );
            m_img = NULL;
            wxLogError( wxT("ocpn_Bitmap:Cannot malloc for data image.") );
        }
    }

}

ocpnXImage::~ocpnXImage()
{
#ifdef ocpUSE_MITSHM
    if(buse_mit)
    {
        XShmDetach( xdisplay, &shminfo );
        XDestroyImage( m_img );
        shmdt( shminfo.shmaddr );
    }
    else
    {
        XDestroyImage( m_img );
    }
#else
    XDestroyImage( m_img );
#endif
}



bool ocpnXImage::PutImage(Pixmap pixmap, GC gc)
{
#ifdef ocpUSE_MITSHM
    if(buse_mit)
        XShmPutImage( xdisplay, pixmap, gc, m_img, 0, 0, 0, 0, m_width, m_height, False );
    else
        XPutImage( xdisplay, pixmap, gc, m_img, 0, 0, 0, 0, m_width, m_height );

#else
    XPutImage( xdisplay, pixmap, gc, m_img, 0, 0, 0, 0, m_width, m_height );
#endif

    return true;
}


#endif      //  __WXX11__




/*  Class : PixelCache
    Why a specific class for what is, in effect, a simple unsigned char[] ?
    Answer: Allow performance optimization for specific platforms,
            such as MSW dibSections, and X11 Pixmaps
*/

// ============================================================================
// PixelCache Implementation
// ============================================================================
PixelCache::PixelCache(int width, int height, int depth)
{
    m_width = width;
    m_height = height;
    m_depth = depth;
    m_pbm = NULL;
    m_rgbo = RGB;                        // default value;
    pData = NULL;

    bytes_per_pixel = BPP / 8;
    line_pitch_bytes = bytes_per_pixel * width;


#ifdef ocpnUSE_ocpnBitmap
      m_rgbo = BGR;
#endif

#ifdef __PIX_CACHE_PIXBUF__
      m_rgbo = RGB;
#endif

#ifdef __PIX_CACHE_DIBSECTION__
      m_pDS = new wxDIB(width, -height, BPP);
      pData = m_pDS->GetData();
      //        For DIBsections, each scan line is DWORD aligned, padded on the right
      line_pitch_bytes = (((m_width * 24) + 31) & ~31) >> 3;
      
#endif


#ifdef __PIX_CACHE_WXIMAGE__
      m_pimage = new wxImage(m_width, m_height, (bool)FALSE);
      pData = m_pimage->GetData();
#endif

#ifdef __PIX_CACHE_X11IMAGE__
      m_pocpnXI = new ocpnXImage(width, height);
      pData = (unsigned char *)m_pocpnXI->m_img->data;
#endif            //__PIX_CACHE_X11IMAGE__

#ifdef __PIX_CACHE_PIXBUF__
//      m_pbm = new ocpnBitmap((unsigned char *)NULL, m_width, m_height, m_depth);

///      m_pbm = new ocpnBitmap();
///      m_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
///                                           1,
///                                           8, m_width, m_height);

///      m_pixbuf = m_pbm->GetPixbuf();
///      pData = gdk_pixbuf_get_pixels(m_pixbuf);
///      m_pbm->SetPixbuf(m_pixbuf, 32);

      pData = (unsigned char *)malloc(m_width * m_height * 4);
///      memset(pData, 255, m_width * m_height * 4);       // set alpha channel to 1
#endif
}

PixelCache::~PixelCache()
{
#ifdef __PIX_CACHE_WXIMAGE__
      delete m_pimage;
      delete m_pbm;
#endif


#ifdef __PIX_CACHE_DIBSECTION__
      delete m_pDS;
#endif

#ifdef __PIX_CACHE_X11IMAGE__
      delete m_pbm;
      delete m_pocpnXI;
#endif

#ifdef __PIX_CACHE_PIXBUF__
      free(pData);
      delete m_pbm;
#endif

}

void PixelCache::Update(void)
{
#ifdef __PIX_CACHE_WXIMAGE__
    delete m_pbm;                       // kill the old one
    m_pbm = NULL;
#endif
}


void PixelCache::SelectIntoDC(wxMemoryDC &dc)
{

#ifdef __PIX_CACHE_DIBSECTION__
      ocpnMemDC *pmdc = dynamic_cast<ocpnMemDC*>(&dc);
      pmdc->SelectObject (*m_pDS);

#endif      //__PIX_CACHE_DIBSECTION__


#ifdef __PIX_CACHE_WXIMAGE__
//    delete m_pbm;                       // kill the old one

      //    Convert image to bitmap
#ifdef ocpnUSE_ocpnBitmap
      if(!m_pbm)
            m_pbm = new ocpnBitmap(*m_pimage, m_depth);
#else
      if(!m_pbm)
            m_pbm = new wxBitmap (*m_pimage, -1);
#endif

      if(m_pbm)
            dc.SelectObject(*m_pbm);
#endif            // __PIX_CACHE_WXIMAGE__


#ifdef __PIX_CACHE_X11IMAGE__
      if(!m_pbm)
        m_pbm = new ocpnBitmap(m_pocpnXI, m_width, m_height, m_depth);
      dc.SelectObject(*m_pbm);
#endif            //__PIX_CACHE_X11IMAGE__

#ifdef __PIX_CACHE_PIXBUF__
      if(!m_pbm)
            m_pbm = new ocpnBitmap(pData, m_width, m_height, m_depth);
      if(m_pbm)
      {
          dc.SelectObject(*m_pbm);
      }
#endif          //__PIX_CACHE_PIXBUF__


}


unsigned char *PixelCache::GetpData(void) const
{
    return pData;
}



#ifdef ocpnUSE_ocpnBitmap
//-----------------------------------------------------------------------------
/*
    Class: ocpnBitmap

    Derived from wxBitmap

    Why?....
    wxWidgets does a very correct, but sometimes slow job of bitmap creation
    and copying. ocpnBitmap is an optimization of wxBitmap for specific
    platforms and color formats.

    ocpn_Bitmap is optimized specifically for Windows and X11 Linux/Unix systems,
    taking advantage of some known underlying data structures and formats.

    There is (currently) no optimization for for other platforms,
    such as GTK or MAC

    The included methods are very different for MSW and X11
          See the Code

    */

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------
#define M_BMPDATA wx_static_cast(wxBitmapRefData*, m_refData)

IMPLEMENT_DYNAMIC_CLASS(ocpnBitmap, wxBitmap/*wxGDIObject*/)


//class wxBitmapRefData;

// ============================================================================
// implementation
// ============================================================================

ocpnBitmap::ocpnBitmap()
{
}

//ocpnBitmap::~ocpnBitmap()
//{
//}

/// This is the GDK version
#ifdef  __WXGTK__
#ifdef opcnUSE_GTK_OPTIMIZE

// ----------------------------------------------------------------------------
// Create from Data
// ----------------------------------------------------------------------------

bool ocpnBitmap::CreateFromData( void *pPix, int width, int height, int depth )
{
    /*
    XImage *img = NULL;

//    Do some basic setup in the parent wxBitmap class
    Create(width, height, -1);

    Display *xdisplay = (Display *)GetDisplay();

    ocpnXImage *pXI = new ocpnXImage(width, height);
    img = pXI->m_img;

//    Faster render from a 24 or 32 bit pixel buffer

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

    Pixmap mypixmap = ((Pixmap )GetPixmap());
    GC gc = XCreateGC( xdisplay, mypixmap, 0, NULL );

    pXI->PutImage(mypixmap, gc);

    delete pXI;

    XFreeGC( xdisplay, gc );
    */

    /////////////////////
    Create(width, height, 32);

//    GdkPixbuf* pixbuf = wx_static_cast(wxBitmapRefData*, m_refData))->m_pixbuf;
 //   GdkPixbuf* pixbuf = M_BMPDATA->m_pixbuf;

    // Copy the data:
    if(NULL != pPix)
    {
        GdkPixbuf* pixbuf = GetPixbuf();

        if (!pixbuf)
            return false;

        unsigned char* in = (unsigned char *)pPix;
        unsigned char *out = gdk_pixbuf_get_pixels(pixbuf);

        int rowpad = gdk_pixbuf_get_rowstride(pixbuf) - 4 * width;

        for (int y = 0; y < height; y++, out += rowpad)
        {
///            memcpy(out, in, width * 4);
///            in  += 4 * width;
///            out += 4 * width;


            for (int x = 0; x < width; x++, out += 4, in += 4)
            {
                out[0] = in[0];
                out[1] = in[1];
                out[2] = in[2];
                out[3] = in[3];
            }

        }
    }
    return true;

}

/*
bool wxBitmap::CreateFromImageAsPixbuf(const wxImage& image)
{
    wxASSERT(image.HasAlpha());

    int width = image.GetWidth();
    int height = image.GetHeight();

    Create(width, height, 32);
    GdkPixbuf* pixbuf = M_BMPDATA->m_pixbuf;
    if (!pixbuf)
        return false;

    // Copy the data:
    const unsigned char* in = image.GetData();
    unsigned char *out = gdk_pixbuf_get_pixels(pixbuf);
    unsigned char *alpha = image.GetAlpha();

    int rowpad = gdk_pixbuf_get_rowstride(pixbuf) - 4 * width;

    for (int y = 0; y < height; y++, out += rowpad)
    {
        for (int x = 0; x < width; x++, alpha++, out += 4, in += 3)
        {
            out[0] = in[0];
            out[1] = in[1];
            out[2] = in[2];
            out[3] = *alpha;
        }
    }

    return true;
}
*/
#endif //opcnUSE_GTK_OPTIMIZE
#endif



#ifdef __WXX11__
//    This is the X11 Version

// ----------------------------------------------------------------------------
// Create from ocpnXImage
// ----------------------------------------------------------------------------

bool ocpnBitmap::CreateFromocpnXImage( ocpnXImage *poXI, int width, int height, int depth )
{
//    Do some basic setup in the parent  wxBitmap class
    Create(width, height, -1);

    Display *xdisplay = (Display *)GetDisplay();

    XImage *data_image = poXI->m_img;
    bool bShared = poXI->buse_mit;

// Blit picture

    Pixmap mypixmap = ((Pixmap )GetPixmap());

    GC gc = XCreateGC( xdisplay, mypixmap, 0, NULL );

    if(bShared)
            XShmPutImage( xdisplay, mypixmap, gc, data_image, 0, 0, 0, 0, width, height, False );
    else
            XPutImage( xdisplay, mypixmap, gc, data_image, 0, 0, 0, 0, width, height );

    XFreeGC( xdisplay, gc );

    return true;
}

// ----------------------------------------------------------------------------
// Create from Data
// ----------------------------------------------------------------------------

bool ocpnBitmap::CreateFromData( void *pPix, int width, int height, int depth )
{

    XImage *img = NULL;

//    Do some basic setup in the parent wxBitmap class
    Create(width, height, -1);

    Display *xdisplay = (Display *)GetDisplay();

    ocpnXImage *pXI = new ocpnXImage(width, height);
    img = pXI->m_img;

//    Faster render from a 24 or 32 bit pixel buffer

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

    Pixmap mypixmap = ((Pixmap )GetPixmap());
    GC gc = XCreateGC( xdisplay, mypixmap, 0, NULL );

    pXI->PutImage(mypixmap, gc);

    delete pXI;

    XFreeGC( xdisplay, gc );

    return TRUE;
}

#endif            //__WXX11__


#ifdef __WXMSW__

// ----------------------------------------------------------------------------
// Create from Data
// ----------------------------------------------------------------------------
bool ocpnBitmap::CreateFromData( void *pPix, int width, int height, int depth )
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


bool ocpnBitmap::CreateFromImage( const wxImage& image, int depth )
{
//    wxCHECK_MSG( image.Ok(), FALSE, wxT("invalid image") )

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
//      if(hbitmap == NULL)
//            int cop =0;

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

//      if(!this->Ok())
//         int cop = this->Ok();

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

#endif            //ocpnUSE_ocpnBitmap



// ============================================================================
// ocpnMemDC implementation
// ============================================================================

IMPLEMENT_DYNAMIC_CLASS(ocpnMemDC, wxMemoryDC)


ocpnMemDC::ocpnMemDC()
{
}


#ifdef ocpnUSE_DIBSECTION
void ocpnMemDC::SelectObject(wxDIB& dib)
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
          wxLogLastError(wxT("SelectObject(ocpnMemDC, DIB)"));

          wxFAIL_MSG(wxT("Couldn't select a DIB into ocpnMemDC"));
      }

      else if ( !m_oldBitmap )
      {
          m_oldBitmap = hDIB;
      }

}

#endif      //ocpnUSE_DIBSECTION




/*
 * Rotation code by Carlos Moreno
 * Adapted to static and modified for improved performance by dsr
 */

static const double wxROTATE_EPSILON = 1e-10;

// Auxiliary function to rotate a point (x,y) with respect to point p0
// make it inline and use a straight return to facilitate optimization
// also, the function receives the sine and cosine of the angle to avoid
// repeating the time-consuming calls to these functions -- sin/cos can
// be computed and stored in the calling function.


static inline wxRealPoint
            wxRotatePoint(const wxRealPoint& p, double cos_angle, double sin_angle,
                          const wxRealPoint& p0)
{
      return wxRealPoint(p0.x + (p.x - p0.x) * cos_angle - (p.y - p0.y) * sin_angle,
                         p0.y + (p.y - p0.y) * cos_angle + (p.x - p0.x) * sin_angle);
}

static inline wxRealPoint
            wxRotatePoint(double x, double y, double cos_angle, double sin_angle,
                          const wxRealPoint & p0)
{
      return wxRotatePoint (wxRealPoint(x,y), cos_angle, sin_angle, p0);
}


wxImage Image_Rotate(wxImage &base_image, double angle, const wxPoint & centre_of_rotation, bool interpolating, wxPoint * offset_after_rotation)
{
      int i;
      angle = -angle;     // screen coordinates are a mirror image of "real" coordinates

      bool has_alpha = base_image.HasAlpha();

      const int w = base_image.GetWidth(),
                             h = base_image.GetHeight();

    // Create pointer-based array to accelerate access to wxImage's data
                             unsigned char ** data = new unsigned char * [h];
                             data[0] = base_image.GetData();
                             for (i = 1; i < h; i++)
                                   data[i] = data[i - 1] + (3 * w);

    // Same for alpha channel
                             unsigned char ** alpha = NULL;
                             if (has_alpha)
                             {
                                   alpha = new unsigned char * [h];
                                   alpha[0] = base_image.GetAlpha();
                                   for (i = 1; i < h; i++)
                                         alpha[i] = alpha[i - 1] + w;
                             }

    // precompute coefficients for rotation formula
    // (sine and cosine of the angle)
                             const double cos_angle = cos(angle);
                             const double sin_angle = sin(angle);

    // Create new Image to store the result
    // First, find rectangle that covers the rotated image;  to do that,
    // rotate the four corners

                             const wxRealPoint p0(centre_of_rotation.x, centre_of_rotation.y);

                             wxRealPoint p1 = wxRotatePoint (0, 0, cos_angle, sin_angle, p0);
                             wxRealPoint p2 = wxRotatePoint (0, h, cos_angle, sin_angle, p0);
                             wxRealPoint p3 = wxRotatePoint (w, 0, cos_angle, sin_angle, p0);
                             wxRealPoint p4 = wxRotatePoint (w, h, cos_angle, sin_angle, p0);

                             int x1a = (int) floor (wxMin (wxMin(p1.x, p2.x), wxMin(p3.x, p4.x)));
                             int y1a = (int) floor (wxMin (wxMin(p1.y, p2.y), wxMin(p3.y, p4.y)));
                             int x2a = (int) ceil (wxMax (wxMax(p1.x, p2.x), wxMax(p3.x, p4.x)));
                             int y2a = (int) ceil (wxMax (wxMax(p1.y, p2.y), wxMax(p3.y, p4.y)));

    // Create rotated image
                             wxImage rotated (x2a - x1a + 1, y2a - y1a + 1, false);
    // With alpha channel
                             if (has_alpha)
                                   rotated.SetAlpha();

                             if (offset_after_rotation != NULL)
                             {
                                   *offset_after_rotation = wxPoint (x1a, y1a);
                             }

    // GRG: The rotated (destination) image is always accessed
    //      sequentially, so there is no need for a pointer-based
    //      array here (and in fact it would be slower).
                             //
                             unsigned char * dst = rotated.GetData();

                             unsigned char * alpha_dst = NULL;
                             if (has_alpha)
                                   alpha_dst = rotated.GetAlpha();

    // GRG: if the original image has a mask, use its RGB values
    //      as the blank pixel, else, fall back to default (black).
                             //
                             unsigned char blank_r = 0;
                             unsigned char blank_g = 0;
                             unsigned char blank_b = 0;

                             if (base_image.HasMask())
                             {
                                   blank_r = base_image.GetMaskRed();
                                   blank_g = base_image.GetMaskGreen();
                                   blank_b = base_image.GetMaskBlue();
                                   rotated.SetMaskColour( blank_r, blank_g, blank_b );
                             }

    // Now, for each point of the rotated image, find where it came from, by
    // performing an inverse rotation (a rotation of -angle) and getting the
    // pixel at those coordinates

                             const int rH = rotated.GetHeight();
                             const int rW = rotated.GetWidth();

    // GRG: I've taken the (interpolating) test out of the loops, so that
    //      it is done only once, instead of repeating it for each pixel.

                             if (interpolating)
                             {
                                   for (int y = 0; y < rH; y++)
                                   {
                                         for (int x = 0; x < rW; x++)
                                         {
                                               wxRealPoint src = wxRotatePoint (x + x1a, y + y1a, cos_angle, -sin_angle, p0);

                                               if (-0.25 < src.x && src.x < w - 0.75 &&
                                                    -0.25 < src.y && src.y < h - 0.75)
                                               {
                    // interpolate using the 4 enclosing grid-points.  Those
                    // points can be obtained using floor and ceiling of the
                    // exact coordinates of the point
                                                     int x1, y1, x2, y2;

                                                     if (0 < src.x && src.x < w - 1)
                                                     {
                                                           x1 = wxRound(floor(src.x));
                                                           x2 = wxRound(ceil(src.x));
                                                     }
                                                     else    // else means that x is near one of the borders (0 or width-1)
                                                     {
                                                           x1 = x2 = wxRound (src.x);
                                                     }

                                                     if (0 < src.y && src.y < h - 1)
                                                     {
                                                           y1 = wxRound(floor(src.y));
                                                           y2 = wxRound(ceil(src.y));
                                                     }
                                                     else
                                                     {
                                                           y1 = y2 = wxRound (src.y);
                                                     }

                    // get four points and the distances (square of the distance,
                    // for efficiency reasons) for the interpolation formula

                    // GRG: Do not calculate the points until they are
                    //      really needed -- this way we can calculate
                    //      just one, instead of four, if d1, d2, d3
                    //      or d4 are < wxROTATE_EPSILON

                                                     const double d1 = (src.x - x1) * (src.x - x1) + (src.y - y1) * (src.y - y1);
                                                     const double d2 = (src.x - x2) * (src.x - x2) + (src.y - y1) * (src.y - y1);
                                                     const double d3 = (src.x - x2) * (src.x - x2) + (src.y - y2) * (src.y - y2);
                                                     const double d4 = (src.x - x1) * (src.x - x1) + (src.y - y2) * (src.y - y2);

                    // Now interpolate as a weighted average of the four surrounding
                    // points, where the weights are the distances to each of those points

                    // If the point is exactly at one point of the grid of the source
                    // image, then don't interpolate -- just assign the pixel

                    // d1,d2,d3,d4 are positive -- no need for abs()
                                                     if (d1 < wxROTATE_EPSILON)
                                                     {
                                                           unsigned char *p = data[y1] + (3 * x1);
                                                           *(dst++) = *(p++);
                                                           *(dst++) = *(p++);
                                                           *(dst++) = *p;

                                                           if (has_alpha)
                                                                 *(alpha_dst++) = *(alpha[y1] + x1);
                                                     }
                                                     else if (d2 < wxROTATE_EPSILON)
                                                     {
                                                           unsigned char *p = data[y1] + (3 * x2);
                                                           *(dst++) = *(p++);
                                                           *(dst++) = *(p++);
                                                           *(dst++) = *p;

                                                           if (has_alpha)
                                                                 *(alpha_dst++) = *(alpha[y1] + x2);
                                                     }
                                                     else if (d3 < wxROTATE_EPSILON)
                                                     {
                                                           unsigned char *p = data[y2] + (3 * x2);
                                                           *(dst++) = *(p++);
                                                           *(dst++) = *(p++);
                                                           *(dst++) = *p;

                                                           if (has_alpha)
                                                                 *(alpha_dst++) = *(alpha[y2] + x2);
                                                     }
                                                     else if (d4 < wxROTATE_EPSILON)
                                                     {
                                                           unsigned char *p = data[y2] + (3 * x1);
                                                           *(dst++) = *(p++);
                                                           *(dst++) = *(p++);
                                                           *(dst++) = *p;

                                                           if (has_alpha)
                                                                 *(alpha_dst++) = *(alpha[y2] + x1);
                                                     }
                                                     else
                                                     {
                        // weights for the weighted average are proportional to the inverse of the distance
                                                           unsigned char *v1 = data[y1] + (3 * x1);
                                                           unsigned char *v2 = data[y1] + (3 * x2);
                                                           unsigned char *v3 = data[y2] + (3 * x2);
                                                           unsigned char *v4 = data[y2] + (3 * x1);

                                                           const double w1 = 1/d1, w2 = 1/d2, w3 = 1/d3, w4 = 1/d4;

                        // GRG: Unrolled.

                                                           *(dst++) = (unsigned char)
                                                                       ( (w1 * *(v1++) + w2 * *(v2++) +
                                                                       w3 * *(v3++) + w4 * *(v4++)) /
                                                                       (w1 + w2 + w3 + w4) );
                                                           *(dst++) = (unsigned char)
                                                                       ( (w1 * *(v1++) + w2 * *(v2++) +
                                                                       w3 * *(v3++) + w4 * *(v4++)) /
                                                                       (w1 + w2 + w3 + w4) );
                                                           *(dst++) = (unsigned char)
                                                                       ( (w1 * *v1 + w2 * *v2 +
                                                                       w3 * *v3 + w4 * *v4) /
                                                                       (w1 + w2 + w3 + w4) );

                                                           if (has_alpha)
                                                           {
                                                                 v1 = alpha[y1] + (x1);
                                                                 v2 = alpha[y1] + (x2);
                                                                 v3 = alpha[y2] + (x2);
                                                                 v4 = alpha[y2] + (x1);

                                                                 *(alpha_dst++) = (unsigned char)
                                                                             ( (w1 * *v1 + w2 * *v2 +
                                                                             w3 * *v3 + w4 * *v4) /
                                                                             (w1 + w2 + w3 + w4) );
                                                           }
                                                     }
                                               }
                                               else
                                               {
                                                     *(dst++) = blank_r;
                                                     *(dst++) = blank_g;
                                                     *(dst++) = blank_b;

                                                     if (has_alpha)
                                                           *(alpha_dst++) = 0;
                                               }
                                         }
                                   }
                             }
                             else    // not interpolating
                             {
                                   double x0 = p0.x;
                                   double y0 = p0.y;
                                   double x1b = x1a - p0.x;
                                   double y1b = y1a - p0.y;
                                   double msa = -sin_angle;

                                   for (int y = 0; y < rH; y++)
                                   {
                                         for (int x = 0; x < rW; x++)
                                         {
//                                               wxRealPoint src = wxRotatePoint (x + x1a, y + y1a, cos_angle, -sin_angle, p0);

//                                               double sx = p0.x + (x + x1a - p0.x) * cos_angle - (y + y1a - p0.y) * -sin_angle;
//                                               double sy=  p0.y + (y + y1a - p0.y) * cos_angle + (x + x1a - p0.x) * -sin_angle;

                                               double sx = x0 + (x + x1b) * cos_angle - (y + y1b) * msa;
                                               double sy=  y0 + (y + y1b) * cos_angle + (x + x1b) * msa;

                                               const int xs = (int)sx;
                                               const int ys = (int)sy;


//                                               return wxRealPoint(p0.x + (p.x - p0.x) * cos_angle - (p.y - p0.y) * sin_angle,
//                                                           p0.y + (p.y - p0.y) * cos_angle + (p.x - p0.x) * sin_angle);

//                                               const int xs = /*wxRound*/ (src.x);      // wxRound rounds to the
//                                               const int ys = /*wxRound*/ (src.y);      // closest integer

                                               if (0 <= xs && xs < w && 0 <= ys && ys < h)
                                               {
                                                     unsigned char *p = data[ys] + (3 * xs);
                                                     *(dst++) = *(p++);
                                                     *(dst++) = *(p++);
                                                     *(dst++) = *p;



                                                     if (has_alpha)
                                                           *(alpha_dst++) = *(alpha[ys] + (xs));
                                               }
                                               else
                                               {
                                                     *(dst++) = blank_r;
                                                     *(dst++) = blank_g;
                                                     *(dst++) = blank_b;

                                                     if (has_alpha)
                                                           *(alpha_dst++) = 255;
                                               }

                                         }
                                   }
                             }

                             delete [] data;

                             if (has_alpha)
                                   delete [] alpha;

                             return rotated;
}




