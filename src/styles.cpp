/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Symbols
 * Author:   Jesper Weissglas
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
 **************************************************************************/

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/filename.h>
#include <wx/dir.h>
#include <stdlib.h>
#include "OCPNPlatform.h"

#include "styles.h"
#include "chart1.h"
#include "wx28compat.h"

extern OCPNPlatform     *g_Platform;

using namespace ocpnStyle;

void bmdump(wxBitmap bm, wxString name)
{
    wxImage img = bm.ConvertToImage();
    img.SaveFile( name << _T(".png"), wxBITMAP_TYPE_PNG );
}

// This function can be used to create custom bitmap blending for all platforms
// where 32 bit bitmap ops are broken. Can hopefully be removed for wxWidgets 3.0...

wxBitmap MergeBitmaps( wxBitmap back, wxBitmap front, wxSize offset )
{
    wxBitmap merged( back.GetWidth(), back.GetHeight(), back.GetDepth() );

    //  If the front bitmap has no alpha channel, then merging will accomplish nothing
    //  So, simply return the bitmap intact
    wxImage im_front = front.ConvertToImage();
    if(!im_front.HasAlpha())
        return front;
#if !wxCHECK_VERSION(2,9,4)

    // Manual alpha blending for broken wxWidgets alpha bitmap support, pervasive in wx2.8.
    merged.UseAlpha();
    back.UseAlpha();
    front.UseAlpha();
    
//    wxImage im_front = front.ConvertToImage();
    wxImage im_back = back.ConvertToImage();
    wxImage im_result = back.ConvertToImage();// Only way to make result have alpha channel in wxW 2.8.

    unsigned char *presult = im_result.GetData();
    unsigned char *pback = im_back.GetData();
    unsigned char *pfront = im_front.GetData();

    unsigned char *afront = NULL;
    if( im_front.HasAlpha() )
    afront = im_front.GetAlpha();

    unsigned char *aback = NULL;
    if( im_back.HasAlpha() )
    aback = im_back.GetAlpha();

    unsigned char *aresult = NULL;
    if( im_result.HasAlpha() )
    aresult = im_result.GetAlpha();

    // Do alpha blending, associative version of "over" operator.
    if(presult && pback && pfront){ 
        for( int i = 0; i < back.GetHeight(); i++ ) {
            for( int j = 0; j < back.GetWidth(); j++ ) {

                int fX = j - offset.x;
                int fY = i - offset.y;

                bool inFront = true;
                if( fX < 0 || fY < 0 ) inFront = false;
                if( fX >= front.GetWidth() ) inFront = false;
                if( fY >= front.GetHeight() ) inFront = false;

                if( inFront ) {
                    double alphaF = (double) ( *afront++ ) / 256.0;
                    double alphaB = (double) ( *aback++ ) / 256.0;
                    double alphaRes = alphaF + alphaB * ( 1.0 - alphaF );
                    unsigned char a = alphaRes * 256;
                    *aresult++ = a;
                    unsigned char r = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
                    *presult++ = r;
                    unsigned char g = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
                    *presult++ = g;
                    unsigned char b = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
                    *presult++ = b;
                } else {
                    *aresult++ = *aback++;
                    *presult++ = *pback++;
                    *presult++ = *pback++;
                    *presult++ = *pback++;
                }
            }
        }
    }
    merged = wxBitmap( im_result );

#else
    wxMemoryDC mdc( merged );
    mdc.Clear();
    mdc.DrawBitmap( back, 0, 0, true );
    mdc.DrawBitmap( front, offset.x, offset.y, true );
    mdc.SelectObject( wxNullBitmap );
#endif

    return merged;
}

// The purpouse of ConvertTo24Bit is to take an icon with 32 bit depth and alpha
// channel and put it in a 24 bit deep bitmap with no alpha, that can be safely
// drawn in the crappy wxWindows implementations.

wxBitmap ConvertTo24Bit( wxColor bgColor, wxBitmap front ) {
    if( front.GetDepth() == 24 ) return front;

    wxBitmap result( front.GetWidth(), front.GetHeight(), 24 );
#if !wxCHECK_VERSION(2,9,4)
    front.UseAlpha();
#endif

    wxImage im_front = front.ConvertToImage();
    wxImage im_result = result.ConvertToImage();

    unsigned char *presult = im_result.GetData();
    unsigned char *pfront = im_front.GetData();

    if(!presult || !pfront)
        return wxNullBitmap;
    
    unsigned char *afront = NULL;
    if( im_front.HasAlpha() )
    afront = im_front.GetAlpha();

    for( int i = 0; i < result.GetWidth(); i++ ) {
        for( int j = 0; j < result.GetHeight(); j++ ) {

            double alphaF = 1.0;
            if(afront)
                alphaF = (double) ( *afront++ ) / 256.0;
            unsigned char r = *pfront++ * alphaF + bgColor.Red() * ( 1.0 - alphaF );
            *presult++ = r;
            unsigned char g = *pfront++ * alphaF + bgColor.Green() * ( 1.0 - alphaF );
            *presult++ = g;
            unsigned char b = *pfront++ * alphaF + bgColor.Blue() * ( 1.0 - alphaF );
            *presult++ = b;
        }
    }

    result = wxBitmap( im_result );
    return result;
}

// Tools and Icons perform on-demand loading and dimming of bitmaps.
// Changing color scheme invalidatres all loaded bitmaps.

wxBitmap Style::GetIcon(const wxString & name)
{
    if( iconIndex.find( name ) == iconIndex.end() ) {
        wxString msg( _T("The requested icon was not found in the style: ") );
        msg += name;
        wxLogMessage( msg );
        return wxBitmap( GetToolSize().x, GetToolSize().y ); // Prevents crashing.
    }

    int index = iconIndex[name]; // FIXME: this operation is not const but should be, use 'find'

    Icon* icon = (Icon*) icons.Item( index );

    if( icon->loaded ) return icon->icon;
    if( icon->size.x == 0 ) icon->size = toolSize[currentOrientation];
    wxRect location( icon->iconLoc, icon->size );
    wxBitmap bm = graphics->GetSubBitmap( location );
    icon->icon = SetBitmapBrightness( bm );
    icon->loaded = true;
    return icon->icon;
}

wxBitmap Style::GetToolIcon(const wxString & toolname, int iconType, bool rollover )
{

    if( toolIndex.find( toolname ) == toolIndex.end() ) {
//  This will produce a flood of log messages for some PlugIns, notably WMM_PI, and GRADAR_PI
//        wxString msg( _T("The requested tool was not found in the style: ") );
//        msg += toolname;
//        wxLogMessage( msg );
        return wxBitmap( GetToolSize().x, GetToolSize().y, 1 );
    }

    int index = toolIndex[toolname];

    Tool* tool = (Tool*) tools.Item( index );

    switch( iconType ){
        case TOOLICON_NORMAL: {
            if( tool->iconLoaded && !rollover ) return tool->icon;
            if( tool->rolloverLoaded && rollover ) return tool->rollover;

            wxSize size = tool->customSize;
            if( size.x == 0 ) size = toolSize[currentOrientation];
            wxRect location( tool->iconLoc, size );

            //  If rollover icon does not exist, use the defult icon
            if( rollover ) {
                if( (tool->rolloverLoc.x != 0) || (tool->rolloverLoc.y != 0) )
                    location = wxRect( tool->rolloverLoc, size );
            }

            if( currentOrientation ) {
                location.x -= verticalIconOffset.x;
                location.y -= verticalIconOffset.y;
            }

            wxBitmap bm = graphics->GetSubBitmap( location );
            if( hasBackground ) {
                bm = MergeBitmaps( GetNormalBG(), bm, wxSize( 0, 0 ) );
            } else {
                wxBitmap bg( GetToolSize().x, GetToolSize().y );
                wxMemoryDC mdc( bg );
                mdc.SetBackground( wxBrush( GetGlobalColor( _T("GREY2") ), wxBRUSHSTYLE_SOLID ) );
                mdc.Clear();
                mdc.SelectObject( wxNullBitmap );
                bm = MergeBitmaps( bg, bm, wxSize( 0, 0 ) );
            }
            if( rollover ) {
                tool->rollover = SetBitmapBrightness( bm );
                tool->rolloverLoaded = true;
                return tool->rollover;
            } else {
                if( toolname == _T("mob_btn") ) {
                    double dimLevel = 1.0;
                    if(colorscheme ==  GLOBAL_COLOR_SCHEME_DUSK)
                        dimLevel = 0.5;
                    else if(colorscheme ==  GLOBAL_COLOR_SCHEME_NIGHT)
                        dimLevel = 0.5;
                    tool->icon = SetBitmapBrightnessAbs( bm, dimLevel );
                }
                else {
                    tool->icon = SetBitmapBrightness( bm );
                }
                
                tool->iconLoaded = true;
                return tool->icon;
            }
        }
        case TOOLICON_TOGGLED: {
            if( tool->toggledLoaded && !rollover ) return tool->toggled;
            if( tool->rolloverToggledLoaded && rollover ) return tool->rolloverToggled;

            wxSize size = tool->customSize;
            if( size.x == 0 ) size = toolSize[currentOrientation];
            wxRect location( tool->iconLoc, size );
            if( rollover ) location = wxRect( tool->rolloverLoc, size );
            wxSize offset( 0, 0 );
            if( GetToolSize() != GetToggledToolSize() ) {
                offset = GetToggledToolSize() - GetToolSize();
                offset /= 2;
            }
            if( currentOrientation ) {
                location.x -= verticalIconOffset.x;
                location.y -= verticalIconOffset.y;
            }
            wxBitmap bm = MergeBitmaps( GetToggledBG(), graphics->GetSubBitmap( location ),
                    offset );
            if( rollover ) {
                tool->rolloverToggled = SetBitmapBrightness( bm );
                tool->rolloverToggledLoaded = true;
                return tool->rolloverToggled;
            } else {
                tool->toggled = SetBitmapBrightness( bm );
                tool->toggledLoaded = true;
                return tool->toggled;
            }
        }
        case TOOLICON_DISABLED: {
            if( tool->disabledLoaded ) return tool->disabled;
            wxSize size = tool->customSize;
            if( size.x == 0 ) size = toolSize[currentOrientation];
            wxRect location( tool->disabledLoc, size );
            wxBitmap bm = graphics->GetSubBitmap( location );
            if( currentOrientation ) {
                location.x -= verticalIconOffset.x;
                location.y -= verticalIconOffset.y;
            }
            if( hasBackground ) {
                bm = MergeBitmaps( GetNormalBG(), bm, wxSize( 0, 0 ) );
            }
            tool->disabled = SetBitmapBrightness( bm );
            tool->disabledLoaded = true;
            return tool->disabled;
        }
    }
    wxString msg( _T("A requested icon type for this tool was not found in the style: ") );
    msg += toolname;
    wxLogMessage( msg );
    return wxBitmap( GetToolSize().x, GetToolSize().y ); // Prevents crashing.
}

wxBitmap Style::BuildPluginIcon( const wxBitmap* bm, int iconType )
{
	if( ! bm || ! bm->IsOk() ) return wxNullBitmap;

    wxBitmap iconbm;

    switch( iconType ){
        case TOOLICON_NORMAL: {
            if( hasBackground ) {
                wxBitmap bg = GetNormalBG();
                bg = SetBitmapBrightness( bg );
                wxSize offset = wxSize( bg.GetWidth() - bm->GetWidth(), bg.GetHeight() - bm->GetHeight() );
                offset /= 2;
                iconbm = MergeBitmaps( bg, *bm, offset );
            } else {
                wxBitmap bg( GetToolSize().x, GetToolSize().y );
                wxMemoryDC mdc( bg );
                wxSize offset = GetToolSize() - wxSize( bm->GetWidth(), bm->GetHeight() );
                offset /= 2;
                mdc.SetBackground( wxBrush( GetGlobalColor( _T("GREY2") ), wxBRUSHSTYLE_SOLID ) );
                mdc.Clear();
                mdc.SelectObject( wxNullBitmap );
                iconbm = MergeBitmaps( bg, *bm, offset );
            }
            break;
        }
        case TOOLICON_TOGGLED: {
            iconbm = MergeBitmaps( GetToggledBG(), *bm, wxSize( 0, 0 ) );
            break;
        }
    }
    return iconbm;
}

wxBitmap Style::SetBitmapBrightness( wxBitmap& bitmap )
{
    double dimLevel;
    switch( colorscheme ){
        case GLOBAL_COLOR_SCHEME_DUSK: {
            dimLevel = 0.5;
            break;
        }
        case GLOBAL_COLOR_SCHEME_NIGHT: {
            dimLevel = 0.125;
            break;
        }
        default: {
            return bitmap;
        }
    }
    
    return SetBitmapBrightnessAbs(bitmap, dimLevel);
}

wxBitmap Style::SetBitmapBrightnessAbs( wxBitmap& bitmap, double level )
{
    wxImage image = bitmap.ConvertToImage();

    int gimg_width = image.GetWidth();
    int gimg_height = image.GetHeight();

    for( int iy = 0; iy < gimg_height; iy++ ) {
        for( int ix = 0; ix < gimg_width; ix++ ) {
            if( !image.IsTransparent( ix, iy, 30 ) ) {
                wxImage::RGBValue rgb( image.GetRed( ix, iy ), image.GetGreen( ix, iy ),
                        image.GetBlue( ix, iy ) );
                wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                hsv.value = hsv.value * level;
                wxImage::RGBValue nrgb = wxImage::HSVtoRGB( hsv );
                image.SetRGB( ix, iy, nrgb.red, nrgb.green, nrgb.blue );
            }
        }
    }
    return wxBitmap( image );
}

wxBitmap Style::GetNormalBG()
{
    wxSize size = toolSize[currentOrientation];
    return graphics->GetSubBitmap(
            wxRect( normalBGlocation[currentOrientation].x, normalBGlocation[currentOrientation].y,
                    size.x, size.y ) );
}

wxBitmap Style::GetActiveBG()
{
    return graphics->GetSubBitmap(
            wxRect( activeBGlocation[currentOrientation].x, activeBGlocation[currentOrientation].y,
                    toolSize[currentOrientation].x, toolSize[currentOrientation].y ) );
}

wxBitmap Style::GetToggledBG()
{
    wxSize size = toolSize[currentOrientation];
    if( toggledBGSize[currentOrientation].x ) {
        size = toggledBGSize[currentOrientation];
    }
    return graphics->GetSubBitmap( wxRect( toggledBGlocation[currentOrientation], size ) );
}

wxBitmap Style::GetToolbarStart()
{
    wxSize size = toolbarStartSize[currentOrientation];
    if( toolbarStartSize[currentOrientation].x == 0 ) {
        size = toolbarStartSize[currentOrientation];
    }
    return graphics->GetSubBitmap( wxRect( toolbarStartLoc[currentOrientation], size ) );
}

wxBitmap Style::GetToolbarEnd()
{
    wxSize size = toolbarEndSize[currentOrientation];
    if( toolbarEndSize[currentOrientation].x == 0 ) {
        size = toolbarEndSize[currentOrientation];
    }
    return graphics->GetSubBitmap( wxRect( toolbarEndLoc[currentOrientation], size ) );
}

int Style::GetToolbarCornerRadius()
{
    return cornerRadius[currentOrientation];
}

void Style::DrawToolbarLineStart( wxBitmap& bmp )
{
    if( !HasToolbarStart() ) return;
    wxMemoryDC dc( bmp );
    dc.DrawBitmap( GetToolbarStart(), 0, 0, true );
    dc.SelectObject( wxNullBitmap );
}

void Style::DrawToolbarLineEnd( wxBitmap& bmp )
{
    if( !HasToolbarStart() ) return;
    wxMemoryDC dc( bmp );
    if( currentOrientation ) {
        dc.DrawBitmap( GetToolbarEnd(), 0, bmp.GetHeight() - GetToolbarEnd().GetHeight(), true );
    } else {
        dc.DrawBitmap( GetToolbarEnd(), bmp.GetWidth() - GetToolbarEnd().GetWidth(), 0, true );
    }
    dc.SelectObject( wxNullBitmap );
}

void Style::SetOrientation( long orient )
{
    int newOrient = 0;
    if( orient == wxTB_VERTICAL ) newOrient = 1;
    if( newOrient == currentOrientation ) return;
    currentOrientation = newOrient;
    Unload();
}

int Style::GetOrientation()
{
    return currentOrientation;
}

void Style::SetColorScheme( ColorScheme cs )
{
    colorscheme = cs;
    Unload();
    
    if( (consoleTextBackgroundSize.x) && (consoleTextBackgroundSize.y)) {
        wxBitmap bm = graphics->GetSubBitmap(
            wxRect( consoleTextBackgroundLoc, consoleTextBackgroundSize ) );

    // The background bitmap in the icons file may be too small, so will grow it arbitrailly
        wxImage image = bm.ConvertToImage();
        image.Rescale( consoleTextBackgroundSize.GetX() * 2, consoleTextBackgroundSize.GetY() * 2 , wxIMAGE_QUALITY_NORMAL );
        wxBitmap bn( image );
        consoleTextBackground = SetBitmapBrightness( bn );
    }
}

void Style::Unload()
{
    for( unsigned int i = 0; i < tools.Count(); i++ ) {
        Tool* tool = (Tool*) tools.Item( i );
        tool->Unload();
    }

    for( unsigned int i = 0; i < icons.Count(); i++ ) {
        Icon* icon = (Icon*) icons.Item( i );
        icon->Unload();
    }
}

Style::Style( void )
{
    graphics = NULL;
    currentOrientation = 0;
    colorscheme = GLOBAL_COLOR_SCHEME_DAY;
    marginsInvisible = false;
    hasBackground = false;
    chartStatusIconWidth = 0;
    chartStatusWindowTransparent = false;
    embossHeight = 40;
    embossFont = wxEmptyString;

    //  Set compass window style defauilts
    compassMarginTop = 4;
    compassMarginRight = 0;
    compassMarginBottom = 4;
    compassMarginLeft = 4;
    compasscornerRadius = 3;
    compassXoffset = 0;
    compassYoffset = 0;

    for( int i = 0; i < 2; i++ ) {
        toolbarStartLoc[i] = wxPoint( 0, 0 );
        toolbarEndLoc[i] = wxPoint( 0, 0 );
        cornerRadius[i] = 0;
    }
}

Style::~Style( void )
{
    for( unsigned int i = 0; i < tools.Count(); i++ ) {
        delete (Tool*) ( tools.Item( i ) );
    }
    tools.Clear();

    for( unsigned int i = 0; i < icons.Count(); i++ ) {
        delete (Icon*) ( icons.Item( i ) );
    }
    icons.Clear();

    if( graphics ) delete graphics;

    toolIndex.clear();
    iconIndex.clear();
}

StyleManager::StyleManager(void)
{
    isOK = false;
    currentStyle = NULL;
    Init( g_Platform->GetSharedDataDir() + _T("uidata") + wxFileName::GetPathSeparator() );
    Init( g_Platform->GetHomeDir() );
    Init( g_Platform->GetHomeDir() + _T(".opencpn") + wxFileName::GetPathSeparator() );
    SetStyle( _T("") );
}

StyleManager::StyleManager(const wxString & configDir)
{
    isOK = false;
    currentStyle = NULL;
    Init( configDir );
    SetStyle( _T("") );
}

StyleManager::~StyleManager(void)
{
    for( unsigned int i = 0; i < styles.Count(); i++ ) {
        delete (Style*) ( styles.Item( i ) );
    }
    styles.Clear();
}

void StyleManager::Init(const wxString & fromPath)
{
    TiXmlDocument doc;

    if( !wxDir::Exists( fromPath ) ) {
        wxString msg = _T("No styles found at: ");
        msg << fromPath;
        wxLogMessage( msg );
        return;
    }

    wxDir dir( fromPath );
    if( !dir.IsOpened() ) return;

    wxString filename;

    // We allow any number of styles to load from files called style<something>.xml

    bool more = dir.GetFirst( &filename, _T("style*.xml"), wxDIR_FILES );

    if( !more ) {
        wxString msg = _T("No styles found at: ");
        msg << fromPath;
        wxLogMessage( msg );
        return;
    }

    bool firstFile = true;
    while( more ) {
        wxString name, extension;

        if( !firstFile ) more = dir.GetNext( &filename );
        if( !more ) break;
        firstFile = false;

        wxString fullFilePath = fromPath + filename;

        if( !doc.LoadFile( (const char*) fullFilePath.mb_str() ) ) {
            wxString msg( _T("Attempt to load styles from this file failed: ") );
            msg += fullFilePath;
            wxLogMessage( msg );
            continue;
        }

        wxString msg( _T("Styles loading from ") );
        msg += fullFilePath;
        wxLogMessage( msg );

        TiXmlHandle hRoot( doc.RootElement() );

        wxString root = wxString( doc.RootElement()->Value(), wxConvUTF8 );
        if( root != _T("styles" ) ) {
            wxLogMessage( _T("    StyleManager: Expected XML Root <styles> not found.") );
            continue;
        }

        TiXmlElement* styleElem = hRoot.FirstChild().Element();

        for( ; styleElem; styleElem = styleElem->NextSiblingElement() ) {

            if( wxString( styleElem->Value(), wxConvUTF8 ) == _T("style") ) {

                Style* style = new Style();
                styles.Add( style );

                style->name = wxString( styleElem->Attribute( "name" ), wxConvUTF8 );
                style->myConfigFileDir = fromPath;

                TiXmlElement* subNode = styleElem->FirstChild()->ToElement();

                for( ; subNode; subNode = subNode->NextSiblingElement() ) {
                    wxString nodeType( subNode->Value(), wxConvUTF8 );

                    if( nodeType == _T("description") ) {
                        style->description = wxString( subNode->GetText(), wxConvUTF8 );
                        continue;
                    }
                    if( nodeType == _T("chart-status-icon") ) {
                        int w = 0;
                        subNode->QueryIntAttribute( "width", &w );
                        style->chartStatusIconWidth = w;
                        continue;
                    }
                    if( nodeType == _T("chart-status-window") ) {
                        style->chartStatusWindowTransparent = wxString(
                                subNode->Attribute( "transparent" ), wxConvUTF8 ).Lower().IsSameAs(
                                _T("true") );
                        continue;
                    }
                    if( nodeType == _T("embossed-indicators") ) {
                        style->embossFont = wxString( subNode->Attribute( "font" ), wxConvUTF8 );
                        subNode->QueryIntAttribute( "size", &(style->embossHeight) );
                        continue;
                    }
                    if( nodeType == _T("graphics-file") ) {
                        style->graphicsFile = wxString( subNode->Attribute( "name" ), wxConvUTF8 );
                        isOK = true; // If we got this far we are at least partially OK...
                        continue;
                    }
                    if( nodeType == _T("active-route") ) {
                        TiXmlHandle handle( subNode );
                        TiXmlElement* tag = handle.Child( "font-color", 0 ).ToElement();
                        if( tag ) {
                            int r, g, b;
                            tag->QueryIntAttribute( "r", &r );
                            tag->QueryIntAttribute( "g", &g );
                            tag->QueryIntAttribute( "b", &b );
                            style->consoleFontColor = wxColour( r, g, b );
                        }
                        tag = handle.Child( "text-background-location", 0 ).ToElement();
                        if( tag ) {
                            int x, y, w, h;
                            tag->QueryIntAttribute( "x", &x );
                            tag->QueryIntAttribute( "y", &y );
                            tag->QueryIntAttribute( "width", &w );
                            tag->QueryIntAttribute( "height", &h );
                            style->consoleTextBackgroundLoc = wxPoint( x, y );
                            style->consoleTextBackgroundSize = wxSize( w, h );
                        }
                        continue;
                    }
                    if( nodeType == _T("icons") ) {
                        TiXmlElement* iconNode = subNode->FirstChild()->ToElement();

                        for( ; iconNode; iconNode = iconNode->NextSiblingElement() ) {
                            wxString nodeType( iconNode->Value(), wxConvUTF8 );
                            if( nodeType == _T("icon") ) {
                                Icon* icon = new Icon();
                                style->icons.Add( icon );
                                icon->name = wxString( iconNode->Attribute( "name" ), wxConvUTF8 );
                                style->iconIndex[icon->name] = style->icons.Count() - 1;
                                TiXmlHandle handle( iconNode );
                                TiXmlElement* tag = handle.Child( "icon-location", 0 ).ToElement();
                                if( tag ) {
                                    int x, y;
                                    tag->QueryIntAttribute( "x", &x );
                                    tag->QueryIntAttribute( "y", &y );
                                    icon->iconLoc = wxPoint( x, y );
                                }
                                tag = handle.Child( "size", 0 ).ToElement();
                                if( tag ) {
                                    int x, y;
                                    tag->QueryIntAttribute( "x", &x );
                                    tag->QueryIntAttribute( "y", &y );
                                    icon->size = wxSize( x, y );
                                }
                            }
                        }
                    }
                    if( nodeType == _T("tools") ) {
                        TiXmlElement* toolNode = subNode->FirstChild()->ToElement();

                        for( ; toolNode; toolNode = toolNode->NextSiblingElement() ) {
                            wxString nodeType( toolNode->Value(), wxConvUTF8 );

                            if( nodeType == _T("horizontal") || nodeType == _T("vertical") ) {
                                int orientation = 0;
                                if( nodeType == _T("vertical") ) orientation = 1;

                                TiXmlElement* attrNode = toolNode->FirstChild()->ToElement();
                                for( ; attrNode; attrNode = attrNode->NextSiblingElement() ) {
                                    wxString nodeType( attrNode->Value(), wxConvUTF8 );
                                    if( nodeType == _T("separation") ) {
                                        attrNode->QueryIntAttribute( "distance",
                                                &style->toolSeparation[orientation] );
                                        continue;
                                    }
                                    if( nodeType == _T("margin") ) {
                                        attrNode->QueryIntAttribute( "top",
                                                &style->toolMarginTop[orientation] );
                                        attrNode->QueryIntAttribute( "right",
                                                &style->toolMarginRight[orientation] );
                                        attrNode->QueryIntAttribute( "bottom",
                                                &style->toolMarginBottom[orientation] );
                                        attrNode->QueryIntAttribute( "left",
                                                &style->toolMarginLeft[orientation] );
                                        wxString invis = wxString(
                                                attrNode->Attribute( "invisible" ), wxConvUTF8 );
                                        style->marginsInvisible = ( invis.Lower() == _T("true") );
                                        continue;;
                                    }
                                    if( nodeType == _T("toggled-location") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->toggledBGlocation[orientation] = wxPoint( x, y );
                                        x = 0;
                                        y = 0;
                                        attrNode->QueryIntAttribute( "width", &x );
                                        attrNode->QueryIntAttribute( "height", &y );
                                        style->toggledBGSize[orientation] = wxSize( x, y );
                                        continue;
                                    }
                                    if( nodeType == _T("toolbar-start") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->toolbarStartLoc[orientation] = wxPoint( x, y );
                                        x = 0;
                                        y = 0;
                                        attrNode->QueryIntAttribute( "width", &x );
                                        attrNode->QueryIntAttribute( "height", &y );
                                        style->toolbarStartSize[orientation] = wxSize( x, y );
                                        continue;
                                    }
                                    if( nodeType == _T("toolbar-end") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->toolbarEndLoc[orientation] = wxPoint( x, y );
                                        x = 0;
                                        y = 0;
                                        attrNode->QueryIntAttribute( "width", &x );
                                        attrNode->QueryIntAttribute( "height", &y );
                                        style->toolbarEndSize[orientation] = wxSize( x, y );
                                        continue;
                                    }
                                    if( nodeType == _T("toolbar-corners") ) {
                                        int r;
                                        attrNode->QueryIntAttribute( "radius", &r );
                                        style->cornerRadius[orientation] = r;
                                        continue;
                                    }
                                    if( nodeType == _T("background-location") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->normalBGlocation[orientation] = wxPoint( x, y );
                                        style->HasBackground( true );
                                        continue;
                                    }
                                    if( nodeType == _T("active-location") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->activeBGlocation[orientation] = wxPoint( x, y );
                                        continue;
                                    }
                                    if( nodeType == _T("size") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->toolSize[orientation] = wxSize( x, y );
                                        continue;
                                    }
                                    if( nodeType == _T("icon-offset") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->verticalIconOffset = wxSize( x, y );
                                        continue;
                                    }
                                }
                                continue;
                            }
                            if( nodeType == _T("compass") ) {

                                TiXmlElement* attrNode = toolNode->FirstChild()->ToElement();
                                for( ; attrNode; attrNode = attrNode->NextSiblingElement() ) {
                                    wxString nodeType( attrNode->Value(), wxConvUTF8 );
                                    if( nodeType == _T("margin") ) {
                                        attrNode->QueryIntAttribute( "top",
                                                                     &style->compassMarginTop );
                                        attrNode->QueryIntAttribute( "right",
                                                                     &style->compassMarginRight );
                                        attrNode->QueryIntAttribute( "bottom",
                                                                     &style->compassMarginBottom );
                                        attrNode->QueryIntAttribute( "left",
                                                                     &style->compassMarginLeft );
                                        continue;
                                    }
                                    if( nodeType == _T("compass-corners") ) {
                                            int r;
                                            attrNode->QueryIntAttribute( "radius", &r );
                                            style->compasscornerRadius = r;
                                            continue;
                                        }
                                    if( nodeType == _T("offset") ) {
                                        attrNode->QueryIntAttribute( "x",
                                                                     &style->compassXoffset );
                                        attrNode->QueryIntAttribute( "y",
                                                                     &style->compassYoffset );
                                        continue;
                                    }
                                }
                             }

                             if( nodeType == _T("tool") ) {
                                Tool* tool = new Tool();
                                style->tools.Add( tool );
                                tool->name = wxString( toolNode->Attribute( "name" ), wxConvUTF8 );
                                style->toolIndex[tool->name] = style->tools.Count() - 1;
                                TiXmlHandle toolHandle( toolNode );
                                TiXmlElement* toolTag =
                                        toolHandle.Child( "icon-location", 0 ).ToElement();
                                if( toolTag ) {
                                    int x, y;
                                    toolTag->QueryIntAttribute( "x", &x );
                                    toolTag->QueryIntAttribute( "y", &y );
                                    tool->iconLoc = wxPoint( x, y );
                                }
                                toolTag = toolHandle.Child( "rollover-location", 0 ).ToElement();
                                if( toolTag ) {
                                    int x, y;
                                    toolTag->QueryIntAttribute( "x", &x );
                                    toolTag->QueryIntAttribute( "y", &y );
                                    tool->rolloverLoc = wxPoint( x, y );
                                }
                                toolTag = toolHandle.Child( "disabled-location", 0 ).ToElement();
                                if( toolTag ) {
                                    int x, y;
                                    toolTag->QueryIntAttribute( "x", &x );
                                    toolTag->QueryIntAttribute( "y", &y );
                                    tool->disabledLoc = wxPoint( x, y );
                                }
                                toolTag = toolHandle.Child( "size", 0 ).ToElement();
                                if( toolTag ) {
                                    int x, y;
                                    toolTag->QueryIntAttribute( "x", &x );
                                    toolTag->QueryIntAttribute( "y", &y );
                                    tool->customSize = wxSize( x, y );
                                }
                                continue;
                            }
                        }
                        continue;
                    }
                }
            }
        }
    }
}

void StyleManager::SetStyle(wxString name)
{
    Style* style = NULL;
    bool ok = true;
    if( currentStyle ) currentStyle->Unload();
    else ok = false;

    bool selectFirst = false;

    if( name.Length() == 0 ) selectFirst = true;

    for( unsigned int i = 0; i < styles.Count(); i++ ) {
        style = (Style*) ( styles.Item( i ) );
        if( style->name == name || selectFirst ) {
            if( style->graphics ) {
                currentStyle = style;
                ok = true;
                break;
            }

            wxString fullFilePath = style->myConfigFileDir + wxFileName::GetPathSeparator()
                    + style->graphicsFile;

            if( !wxFileName::FileExists( fullFilePath ) ) {
                wxString msg( _T("Styles Graphics File not found: ") );
                msg += fullFilePath;
                wxLogMessage( msg );
                ok = false;
                if( selectFirst ) continue;
                break;
            }

            wxImage img; // Only image does PNG LoadFile properly on GTK.

            if( !img.LoadFile( fullFilePath, wxBITMAP_TYPE_PNG ) ) {
                wxString msg( _T("Styles Graphics File failed to load: ") );
                msg += fullFilePath;
                wxLogMessage( msg );
                ok = false;
                break;
            }
            style->graphics = new wxBitmap( img );
            currentStyle = style;
            ok = true;
            break;
        }
    }

    if( !ok ) {
        wxString msg( _T("The requested style was not found: ") );
        msg += name;
        wxLogMessage( msg );
        return;
    }

    if(style) {
        if( (style->consoleTextBackgroundSize.x) && (style->consoleTextBackgroundSize.y)) {
            style->consoleTextBackground = style->graphics->GetSubBitmap(
            wxRect( style->consoleTextBackgroundLoc, style->consoleTextBackgroundSize ) );
        }
    }

    if(style)
        nextInvocationStyle = style->name;
    
    return;
}

Style* StyleManager::GetCurrentStyle()
{
    return currentStyle;
}
