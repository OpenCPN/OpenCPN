/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 */

#include "ocpCursor.h"

#ifdef __WXX11__

#include "wx/x11/private.h"

class ocpCursorRefData: public wxObjectRefData
{
public:

    ocpCursorRefData();
    ~ocpCursorRefData();

    WXCursor m_cursor;
    WXDisplay *m_display;
};

ocpCursorRefData::ocpCursorRefData()
{
    m_cursor = NULL;
    m_display = NULL;
}

ocpCursorRefData::~ocpCursorRefData()
{
    if ( m_cursor )
        XFreeCursor ( ( Display* ) m_display, ( Cursor ) m_cursor );
}

//-----------------------------------------------------------------------------

#define M_CURSORDATA ((ocpCursorRefData *)m_refData)



ocpCursor::ocpCursor ( const wxString& cursorName, long type,
                       int hotSpotX, int hotSpotY ) : wxCursor ( wxCURSOR_CROSS )
{
    wxImage cImage;

    if ( !cImage.CanRead ( cursorName ) )
        ::wxInitAllImageHandlers();

    cImage.LoadFile ( cursorName );

    int width = cImage.GetWidth();
    int height = cImage.GetHeight();

//    m_refData = new wxCursorRefData();

    //    Get some X parameters
    int xscreen = DefaultScreen ( ( Display* ) wxGlobalDisplay() );
    Window xroot = RootWindow ( ( Display* ) wxGlobalDisplay(), xscreen );
    Visual* xvisual = DefaultVisual ( ( Display* ) wxGlobalDisplay(), xscreen );

    M_CURSORDATA->m_display = wxGlobalDisplay();
    wxASSERT_MSG ( M_CURSORDATA->m_display, wxT ( "No display" ) );

    //    Make a pixmap
    Pixmap cpixmap = XCreatePixmap ( ( Display* ) wxGlobalDisplay(),
                                     xroot, width, height, 1 );

    //    Make an Ximage
    XImage *data_image = XCreateImage ( ( Display* ) wxGlobalDisplay(), xvisual,
                                        1, ZPixmap, 0, 0, width, height, 32, 0 );
    data_image->data = ( char* ) malloc ( data_image->bytes_per_line * data_image->height );

    int index = 0;
    int pixel = 0;
    unsigned char* data = cImage.GetData();

    // Create mask

    Pixmap cmask;
    unsigned char mr, mg, mb;

    if ( cImage.HasMask() )
    {
        XImage *mask_image = XCreateImage ( ( Display* ) wxGlobalDisplay(), xvisual,
                                            1, ZPixmap, 0, 0, width, height, 32, 0 );
        mask_image->data = ( char* ) malloc ( mask_image->bytes_per_line * mask_image->height );

        cImage.GetOrFindMaskColour ( &mr, &mg, &mb );

        int rit = ( mr << 16 ) + ( mg << 8 ) + mb;
        for ( int y = 0; y < height; y++ )
        {
            for ( int x = 0; x < width; x++ )
            {
                int ri = ( int ) data[index++];
                ri += data[index++] << 8;
                ri += data[index++] << 16;

                /*
                 int ri = *(int *)(&data[index]);
                 ri &= 0x00ffffff;
                 index++;
                 index++;
                 index++;
                 */
                pixel = 1;
                if ( ri == rit )        // if data is mask value, mask pixel gets 0
                    pixel = 0;

                XPutPixel ( mask_image, x, y, pixel );

            }
        }

        cmask = XCreatePixmap ( ( Display* ) wxGlobalDisplay(), xroot, width, height, 1 );

        GC gc = XCreateGC ( ( Display* ) wxGlobalDisplay(), cmask, 0, NULL );
        XPutImage ( ( Display* ) wxGlobalDisplay(), cmask, gc, mask_image,
                    0, 0, 0, 0, width, height );

        XDestroyImage ( mask_image );
        XFreeGC ( ( Display* ) wxGlobalDisplay(), gc );

    }

    //    Render the wxImage cImage onto the Ximage
    //    Simple black/white cursors only, please

    index = 0;

    for ( int y = 0; y < height; y++ )
    {
        for ( int x = 0; x < width; x++ )
        {
            int ri = ( int ) data[index++];
            ri += data[index++] << 8;
            ri += data[index++] << 16;

            /*
             int ri = *(int *)(&data[index]);
             ri &= 0x00ffffff;
             index++;
             index++;
             index++;
             */

            pixel = 0;
            if ( ri )
                pixel = 1;

            XPutPixel ( data_image, x, y, pixel );

        }
    }

    //    Put the Ximage into the pixmap

    GC gc = XCreateGC ( ( Display* ) wxGlobalDisplay(), cpixmap, 0, NULL );
    XPutImage ( ( Display* ) wxGlobalDisplay(), cpixmap, gc, data_image,
                0, 0, 0, 0, width, height );

    //    Free the Ximage stuff
    XDestroyImage ( data_image );
    XFreeGC ( ( Display* ) wxGlobalDisplay(), gc );

    //    Make a X cursor from the pixmap

    XColor fg, bg;
    fg.red = fg.blue = fg.green = 0xffff;
    bg.red = bg.blue = bg.green = 0;

    M_CURSORDATA->m_cursor = ( WXCursor ) XCreatePixmapCursor ( ( Display* ) wxGlobalDisplay(), cpixmap, cmask,
                             &fg, &bg, hotSpotX, hotSpotY );

}

ocpCursor::ocpCursor ( const char **xpm_data, long type,
                       int hotSpotX, int hotSpotY ) : wxCursor ( wxCURSOR_CROSS )
{
    wxImage cImage ( xpm_data );

    int width = cImage.GetWidth();
    int height = cImage.GetHeight();

//    m_refData = new wxCursorRefData();

    //    Get some X parameters
    int xscreen = DefaultScreen ( ( Display* ) wxGlobalDisplay() );
    Window xroot = RootWindow ( ( Display* ) wxGlobalDisplay(), xscreen );
    Visual* xvisual = DefaultVisual ( ( Display* ) wxGlobalDisplay(), xscreen );

    M_CURSORDATA->m_display = wxGlobalDisplay();
    wxASSERT_MSG ( M_CURSORDATA->m_display, wxT ( "No display" ) );

    //    Make a pixmap
    Pixmap cpixmap = XCreatePixmap ( ( Display* ) wxGlobalDisplay(),
                                     xroot, width, height, 1 );

    //    Make an Ximage
    XImage *data_image = XCreateImage ( ( Display* ) wxGlobalDisplay(), xvisual,
                                        1, ZPixmap, 0, 0, width, height, 32, 0 );
    data_image->data = ( char* ) malloc ( data_image->bytes_per_line * data_image->height );

    int index = 0;
    int pixel = 0;
    unsigned char* data = cImage.GetData();

    // Create mask

    Pixmap cmask;
    unsigned char mr, mg, mb;

    if ( cImage.HasMask() )
    {
        XImage *mask_image = XCreateImage ( ( Display* ) wxGlobalDisplay(), xvisual,
                                            1, ZPixmap, 0, 0, width, height, 32, 0 );
        mask_image->data = ( char* ) malloc ( mask_image->bytes_per_line * mask_image->height );

        cImage.GetOrFindMaskColour ( &mr, &mg, &mb );

        int rit = ( mr << 16 ) + ( mg << 8 ) + mb;
        for ( int y = 0; y < height; y++ )
        {
            for ( int x = 0; x < width; x++ )
            {
                int ri = ( int ) data[index++];
                ri += data[index++] << 8;
                ri += data[index++] << 16;

                /*
                 int ri = *(int *)(&data[index]);
                 ri &= 0x00ffffff;
                 index++;
                 index++;
                 index++;
                 */
                pixel = 1;
                if ( ri == rit )        // if data is mask value, mask pixel gets 0
                    pixel = 0;

                XPutPixel ( mask_image, x, y, pixel );

            }
        }

        cmask = XCreatePixmap ( ( Display* ) wxGlobalDisplay(), xroot, width, height, 1 );

        GC gc = XCreateGC ( ( Display* ) wxGlobalDisplay(), cmask, 0, NULL );
        XPutImage ( ( Display* ) wxGlobalDisplay(), cmask, gc, mask_image,
                    0, 0, 0, 0, width, height );

        XDestroyImage ( mask_image );
        XFreeGC ( ( Display* ) wxGlobalDisplay(), gc );

    }

    //    Render the wxImage cImage onto the Ximage
    //    Simple black/white cursors only, please

    index = 0;

    for ( int y = 0; y < height; y++ )
    {
        for ( int x = 0; x < width; x++ )
        {
            int ri = ( int ) data[index++];
            ri += data[index++] << 8;
            ri += data[index++] << 16;

            /*
             int ri = *(int *)(&data[index]);
             ri &= 0x00ffffff;
             index++;
             index++;
             index++;
             */

            pixel = 0;
            if ( ri )
                pixel = 1;

            XPutPixel ( data_image, x, y, pixel );

        }
    }

    //    Put the Ximage into the pixmap

    GC gc = XCreateGC ( ( Display* ) wxGlobalDisplay(), cpixmap, 0, NULL );
    XPutImage ( ( Display* ) wxGlobalDisplay(), cpixmap, gc, data_image,
                0, 0, 0, 0, width, height );

    //    Free the Ximage stuff
    XDestroyImage ( data_image );
    XFreeGC ( ( Display* ) wxGlobalDisplay(), gc );

    //    Make a X cursor from the pixmap

    XColor fg, bg;
    fg.red = fg.blue = fg.green = 0xffff;
    bg.red = bg.blue = bg.green = 0;

    M_CURSORDATA->m_cursor = ( WXCursor ) XCreatePixmapCursor ( ( Display* ) wxGlobalDisplay(), cpixmap, cmask,
                             &fg, &bg, hotSpotX, hotSpotY );

}

#endif      // __WXX11__


//      We derive a class from wxCursor to create ocpCursor
//      Specifically to fix a bug in wxImage-wxBitmap conversions

#ifdef __WXMSW__

ocpCursor::ocpCursor( const wxString& cursorName, long type, int hotSpotX, int hotSpotY ) :
    wxCursor( wxCURSOR_ARROW )

{
    wxImage cImage;

    if( !cImage.CanRead( cursorName ) ) ::wxInitAllImageHandlers();

    cImage.LoadFile( cursorName );

//      wxMSW Bug???
//      On Windows XP, conversion from wxImage to wxBitmap fails at the ::CreateDIBitmap() call
//      unless a "compatible" dc is provided.  Why??
//      As a workaround, just make a simple wxDC for temporary use

    wxBitmap tbmp( cImage.GetWidth(), cImage.GetHeight(), -1 );
    wxMemoryDC dwxdc;
    dwxdc.SelectObject( tbmp );

//        HCURSOR hcursor = wxBitmapToHCURSOR ( wxBitmap ( cImage, ( wxDC & ) dwxdc ),
//                                              hotSpotX, hotSpotY );
    HCURSOR hcursor = NULL;

    if( !hcursor ) {
        wxLogWarning( _T( "Failed to create ocpCursor." ) );
        return;
    }

//  Replace the HANDLE created in the base class constructor
//  Probably leaks....
    GetGDIImageData()->m_handle = hcursor;
}

ocpCursor::ocpCursor( const char **xpm_data, long type, int hotSpotX, int hotSpotY ) :
    wxCursor( wxCURSOR_ARROW )

{
    wxImage cImage( xpm_data );

//      wxMSW Bug???
//      On Windows XP, conversion from wxImage to wxBitmap fails at the ::CreateDIBitmap() call
//      unless a "compatible" dc is provided.  Why??
//      As a workaround, just make a simple wxDC for temporary use

    wxBitmap tbmp( cImage.GetWidth(), cImage.GetHeight(), -1 );
    wxMemoryDC dwxdc;
    dwxdc.SelectObject( tbmp );

//        HCURSOR hcursor = wxBitmapToHCURSOR ( wxBitmap ( cImage, ( wxDC & ) dwxdc ),
//                                              hotSpotX, hotSpotY );

    HCURSOR hcursor = NULL;

    if( !hcursor ) {
        wxLogWarning( _T( "Failed to create ocpCursor." ) );
        return;
    }

//  Replace the HANDLE created in the base class constructor
//  Probably leaks....
    GetGDIImageData()->m_handle = hcursor;
}
#endif   // __MSW


#ifdef __WXOSX__  // begin rms
ocpCursor::ocpCursor ( const wxString& cursorName, long type,
                       int hotSpotX, int hotSpotY ) : wxCursor ( wxCURSOR_ARROW )

{
    wxImage cImage;

    if ( !cImage.CanRead ( cursorName ) )
        ::wxInitAllImageHandlers();

    cImage.LoadFile ( cursorName );

//      wxMSW Bug???
//      On Windows XP, conversion from wxImage to wxBitmap fails at the ::CreateDIBitmap() call
//      unless a "compatible" dc is provided.  Why??
//      As a workaround, just make a simple wxDC for temporary use

    wxBitmap tbmp ( cImage.GetWidth(),cImage.GetHeight(),-1 );
}

//----------------------------------------------------------------------------------------------
//      A new constructor taking a static char ** of XPM data and assign as a cursor
//----------------------------------------------------------------------------------------------

ocpCursor::ocpCursor ( const char **xpm_data, long type,
                       int hotSpotX, int hotSpotY ) : wxCursor ( wxCURSOR_ARROW )

{
    wxImage cImage ( xpm_data );

    wxBitmap tbmp ( cImage.GetWidth(),cImage.GetHeight(),-1 );
}

#endif   // __WXOSX__ end rms

