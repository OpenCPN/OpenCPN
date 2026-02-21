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


#ifndef _CHARTSYMBOLS_H_
#define _CHARTSYMBOLS_H_

#ifdef ocpnUSE_GL
#ifdef __OCPN_USE_GLEW__
 #ifndef __OCPN__ANDROID__
  #if defined(_WIN32)
    #include "glew.h"
  #elif defined(__WXQT__) || defined(__WXGTK__)
   #include <GL/glew.h>
  #endif
 #endif
#endif
#endif

#if defined(__OCPN__ANDROID__)
 //#include <GLES2/gl2.h>
 #include <qopengl.h>
 #include <GL/gl_private.h>  // this is a cut-down version of gl.h
 #include <GLES2/gl2.h>
#elif defined(_WIN32)
 #define GL_GLEXT_PROTOTYPES
 #include <GL/gl.h>
 #include <GL/glu.h>
 //typedef void (__stdcall * _GLUfuncptr)(void);
#elif defined(__WXOSX__)
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 typedef void (*  _GLUfuncptr)();
 #define GL_COMPRESSED_RGB_FXT1_3DFX       0x86B0
#elif defined(__WXQT__) || defined(__WXGTK__)
 #define GL_GLEXT_PROTOTYPES
 #include <GL/glu.h>
 #include <GL/gl.h>
 #include <GL/glx.h>
#endif



#include "s52s57.h"
#include "pugixml.hpp"
class s52plib;

WX_DECLARE_STRING_HASH_MAP(wxRect, symbolGraphicsHashMap);

class Lookup {
public:
  int RCID;
  int id;
  wxString name;
  Object_t type;                           // 'A' Area, 'L' Line, 'P' Point
  DisPrio displayPrio;                     // Display Priority
  RadPrio radarPrio;                       // 'O' or 'S', Radar Priority
  LUPname tableName;                       // FTYP:  areas, points, lines
  std::vector<std::string> attributeCodeArray;  // ArrayString of LUP Attributes
  wxString instruction;                    // Instruction Field (rules)
  DisCat displayCat;  // Display Categorie: D/S/O, DisplayBase, Standard, Other
  int comment;        // Look-Up Comment (PLib3.x put 'groupes' here,
};

typedef struct _SymbolSizeInfo {
  wxSize size;
  wxPoint origin;
  wxPoint pivot;
  wxPoint graphics;
  int minDistance;
  int maxDistance;
} SymbolSizeInfo_t;

class OCPNPattern {
public:
  int RCID;
  wxString name;
  wxString description;
  wxString colorRef;
  bool hasVector;
  bool hasBitmap;
  bool preferBitmap;
  char fillType;
  char spacing;
  SymbolSizeInfo_t bitmapSize;
  SymbolSizeInfo_t vectorSize;
  wxBitmap bitmap;
  wxString HPGL;
};

class LineStyle {
public:
  int RCID;
  wxString name;
  wxString description;
  wxString colorRef;
  SymbolSizeInfo_t vectorSize;
  wxBitmap bitmap;
  wxString HPGL;
};

/** Generic method argument wrapping differences using opengl or not */
struct ChartCtx {
#ifdef ocpnUSE_GL
  const  bool m_use_opengl;
  const GLenum m_texture_rectangle_format;
  ChartCtx(bool use_opengl, GLenum rect_format)
    : m_use_opengl(use_opengl), m_texture_rectangle_format(rect_format) {}
#else
  const  bool m_use_opengl;
  ChartCtx(bool use_opengl) : m_use_opengl(use_opengl) {}
#endif
};



class ChartSymbol {
public:
  wxString name;
  int RCID;
  bool hasVector;
  bool hasBitmap;
  bool preferBitmap;
  wxString description;
  wxString colorRef;
  SymbolSizeInfo_t bitmapSize;
  SymbolSizeInfo_t vectorSize;
  wxString HPGL;
};

class ChartSymbols {
public:
  ChartSymbols(void);
  ~ChartSymbols(void);
  bool LoadConfigFile(s52plib *plibArg, const wxString &path);

  void InitializeTables(void);
  void DeleteGlobals(void);
  int FindColorTable(const wxString &tableName);
  S52color *GetColor(const char *colorName, int fromTable);
  wxColor GetwxColor(const wxString &colorName, int fromTable);
  wxColor GetwxColor(const char *colorName, int fromTable);
  wxString HashKey(const char *symbolName);
  wxImage GetImage(const char *symbolName);
  unsigned int GetGLTextureRect(wxRect &rect, const char *symbolName);
  wxSize GLTextureSize();
  void SetTextureFormat( int format){ m_texture_rectangle_format = format; }
  int LoadRasterFileForColorTable(int table_nr, bool flush,
                                  const ChartCtx& ctx);
  void SetColorTableIndex(int index, bool flush, const ChartCtx& ctx);

  wxArrayPtrVoid m_colorTables;
  unsigned int rasterSymbolsTexture;
  wxSize rasterSymbolsTextureSize;
  wxBitmap rasterSymbols;
  int rasterSymbolsLoadedColorMapNumber;
  wxString configFileDirectory;
  int ColorTableIndex;
  symbolGraphicsHashMap m_symbolGraphicLocations;
  bool UpdateTableColor( std::string table_name, std::string color_name, wxColor& new_color);

private:
  void BuildLineStyle(LineStyle &lineStyle);
  void BuildLookup(Lookup &lookup);
  void BuildPattern(OCPNPattern &pattern);
  void BuildSymbol(ChartSymbol &symol);

  void ProcessColorTables(pugi::xml_node &node);
  void ProcessLookups(pugi::xml_node &node);
  void ProcessLinestyles(pugi::xml_node &node);
  void ProcessPatterns(pugi::xml_node &node);
  void ProcessSymbols(pugi::xml_node &node);
  void ProcessVectorTag(pugi::xml_node &vectorNode,
                        SymbolSizeInfo_t &vectorSize);

  pugi::xml_document m_symbolsDoc;
  GLenum m_texture_rectangle_format;

  s52plib *plib;
};

#endif
