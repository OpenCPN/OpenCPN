/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Symbols
 * Author:   Jesper Weissglas
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   bdbcat@yahoo.com                                                      *
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

#pragma once
#include <tinyxml.h>

#include "ocpn_types.h"

wxColour GetGlobalColor(wxString colorName);

enum StyleToolIconTypes
{
      TOOLICON_NORMAL,
      TOOLICON_TOGGLED,
      TOOLICON_DISABLED,
      TOOLICON_ACTIVE
};

wxBitmap MergeBitmaps( wxBitmap back, wxBitmap front, wxSize offset );
wxBitmap ConvertTo24Bit( wxColor bgColor, wxBitmap front );

namespace ocpnStyle {

WX_DECLARE_STRING_HASH_MAP( int, intHash );

class Tool {
public:
      wxString name;
      wxPoint iconLoc;
      wxPoint rolloverLoc;
      wxPoint disabledLoc;
      wxPoint activeLoc;
      wxBitmap icon;
      wxBitmap rollover;
      wxBitmap rolloverToggled;
      wxBitmap disabled;
      wxBitmap active;
      wxBitmap toggled;
      bool iconLoaded;
      bool rolloverLoaded;
      bool rolloverToggledLoaded;
      bool disabledLoaded;
      bool activeLoaded;
      bool toggledLoaded;
      wxSize customSize;

      void Unload(void) {
            iconLoaded= false;
            rolloverLoaded = false;
            rolloverToggledLoaded = false;
            disabledLoaded = false;
            activeLoaded =false;
            toggledLoaded =false;
      }

      Tool(void) { Unload(); }
};

class Icon {
public:
      wxString name;
      wxPoint iconLoc;
      wxSize size;
      wxBitmap icon;
      bool loaded;

      void Unload(void) {
            loaded = false;
      }

      Icon(void) { Unload(); }
};

class Style {

public:
      Style( void );
      ~Style( void );

      wxBitmap GetNormalBG();
      wxBitmap GetActiveBG();
      wxBitmap GetToggledBG();
      wxBitmap GetToolbarStart();
      wxBitmap GetToolbarEnd();
      bool HasBackground() { return hasBackground; }
      void HasBackground( bool b ) { hasBackground = b; }
      wxBitmap GetIcon( wxString name );
      wxBitmap GetToolIcon( wxString toolname, int iconType = TOOLICON_NORMAL, bool rollover = false );
      int GetTopMargin() { return toolMarginTop[currentOrientation]; }
      int GetRightMargin() { return toolMarginRight[currentOrientation]; }
      int GetBottomMargin() { return toolMarginBottom[currentOrientation]; }
      int GetLeftMargin() { return toolMarginLeft[currentOrientation]; }
      int GetToolbarCornerRadius();

      int GetToolSeparation() { return toolSeparation[currentOrientation]; }
      wxSize GetToolSize() { return toolSize[currentOrientation]; }
      wxSize GetToggledToolSize() { return toggledBGSize[currentOrientation]; }

      bool HasToolbarStart() { return toolbarStartLoc[currentOrientation] != wxPoint(0,0); }
      bool HasToolbarEnd() { return toolbarEndLoc[currentOrientation] != wxPoint(0,0); }
      void DrawToolbarLineStart( wxBitmap& bmp );
      void DrawToolbarLineEnd( wxBitmap& bmp );

      wxBitmap SetBitmapBrightness( wxBitmap& bitmap );

      void SetOrientation( long orient );
      void SetColorScheme( ColorScheme cs );
      void Unload();

      wxString name;
      wxString description;
      wxString graphicsFile;
      int toolMarginTop[2];
      int toolMarginRight[2];
      int toolMarginBottom[2];
      int toolMarginLeft[2];
      int toolSeparation[2];
      int cornerRadius[2];
      wxSize toolSize[2];
      wxSize toggledBGSize[2];
      wxPoint toggledBGlocation[2];
      wxPoint activeBGlocation[2];
      wxPoint normalBGlocation[2];
      wxSize verticalIconOffset;
      wxArrayPtrVoid tools;
      intHash toolIndex;
      wxArrayPtrVoid icons;
      intHash iconIndex;
      wxBitmap* graphics;

      wxColor consoleFontColor;
      wxPoint consoleTextBackgroundLoc;
      wxSize consoleTextBackgroundSize;
      wxPoint toolbarStartLoc[2];
      wxSize toolbarStartSize[2];
      wxPoint toolbarEndLoc[2];
      wxSize toolbarEndSize[2];
      wxBitmap consoleTextBackground;
      wxBitmap toolbarStart[2];
      wxBitmap toolbarEnd[2];

      bool marginsInvisible;

      int chartStatusIconWidth;
      bool chartStatusWindowTransparent;

      wxString myConfigFileDir;

private:
      int currentOrientation;
      ColorScheme colorscheme;
      bool hasBackground;
};

class StyleManager {
public:
      StyleManager(void);
      ~StyleManager(void);
      StyleManager( wxString& configDir );

      bool IsOK() { return isOK; }
      void Init( wxString fromPath );
      void SetStyle( wxString name );
      void SetStyleNextInvocation( wxString name ) { nextInvocationStyle = name; }
      wxString GetStyleNextInvocation() { return nextInvocationStyle; }
      Style* GetCurrentStyle();
      wxArrayPtrVoid GetArrayOfStyles() { return styles; };

private:
      bool isOK;
      wxArrayPtrVoid styles;
      Style* currentStyle;
      wxString nextInvocationStyle;
};

}
