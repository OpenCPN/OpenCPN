/***************************************************************************
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
 **************************************************************************/

#pragma once
#include <tinyxml.h>

#include "ocpn_types.h"

enum StyleToolIconTypes
{
      TOOLICON_NORMAL,
      TOOLICON_TOGGLED,
      TOOLICON_DISABLED,
      TOOLICON_ACTIVE
};

void bmdump(wxBitmap bm, wxString name);
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

      Tool(void) {
          Unload();
      }
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
      bool HasBackground() const { return hasBackground; }
      void HasBackground( bool b ) { hasBackground = b; }
      wxBitmap GetIcon(const wxString & name);
      wxBitmap GetToolIcon(const wxString & toolname, int iconType = TOOLICON_NORMAL, bool rollover = false );
      wxBitmap BuildPluginIcon( const wxBitmap* bm, int iconType );

      int GetTopMargin() const { return toolMarginTop[currentOrientation]; }
      int GetRightMargin() const { return toolMarginRight[currentOrientation]; }
      int GetBottomMargin() const { return toolMarginBottom[currentOrientation]; }
      int GetLeftMargin() const { return toolMarginLeft[currentOrientation]; }
      int GetToolbarCornerRadius();

      int GetCompassTopMargin() const { return compassMarginTop; }
      int GetCompassRightMargin() const { return compassMarginRight; }
      int GetCompassBottomMargin() const { return compassMarginBottom; }
      int GetCompassLeftMargin() const { return compassMarginLeft; }
      int GetCompassCornerRadius() const { return compasscornerRadius; }
      int GetCompassXOffset() const { return compassXoffset; }
      int GetCompassYOffset() const { return compassYoffset; }

      int GetToolSeparation() const { return toolSeparation[currentOrientation]; }
      wxSize GetToolSize() const { return toolSize[currentOrientation]; }
      wxSize GetToggledToolSize() const { return toggledBGSize[currentOrientation]; }

      bool HasToolbarStart() const { return toolbarStartLoc[currentOrientation] != wxPoint(0,0); }
      bool HasToolbarEnd() const { return toolbarEndLoc[currentOrientation] != wxPoint(0,0); }
      void DrawToolbarLineStart( wxBitmap& bmp );
      void DrawToolbarLineEnd( wxBitmap& bmp );

      wxBitmap SetBitmapBrightness( wxBitmap& bitmap );
      wxBitmap SetBitmapBrightnessAbs( wxBitmap& bitmap, double level );
      
      void SetOrientation( long orient );
      int GetOrientation();
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
      int compassMarginTop;
      int compassMarginRight;
      int compassMarginBottom;
      int compassMarginLeft;
      int compasscornerRadius;
      int compassXoffset;
      int compassYoffset;

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

      wxString embossFont;
      int embossHeight;

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
      StyleManager(const wxString & configDir);

      bool IsOK() const { return isOK; }
      void Init(const wxString & fromPath);
      void SetStyle(wxString name);
      void SetStyleNextInvocation(const wxString & name) { nextInvocationStyle = name; }
      const wxString & GetStyleNextInvocation() const { return nextInvocationStyle; }
      Style* GetCurrentStyle();
      wxArrayPtrVoid GetArrayOfStyles() { return styles; };

private:
      bool isOK;
      wxArrayPtrVoid styles;
      Style* currentStyle;
      wxString nextInvocationStyle;
};

}
