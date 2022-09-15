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

#include "s52s57.h"
#include <tinyxml.h>
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
  std::vector<char *> attributeCodeArray;  // ArrayString of LUP Attributes
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
  int LoadRasterFileForColorTable(int tableNo, bool flush = false);
  int FindColorTable(const wxString &tableName);
  S52color *GetColor(const char *colorName, int fromTable);
  wxColor GetwxColor(const wxString &colorName, int fromTable);
  wxColor GetwxColor(const char *colorName, int fromTable);
  wxString HashKey(const char *symbolName);
  wxImage GetImage(const char *symbolName);
  unsigned int GetGLTextureRect(wxRect &rect, const char *symbolName);
  wxSize GLTextureSize();
  void SetColorTableIndex(int index);

  wxArrayPtrVoid m_colorTables;
  unsigned int rasterSymbolsTexture;
  wxSize rasterSymbolsTextureSize;
  wxBitmap rasterSymbols;
  int rasterSymbolsLoadedColorMapNumber;
  wxString configFileDirectory;
  int ColorTableIndex;
  symbolGraphicsHashMap m_symbolGraphicLocations;

private:
  void ProcessVectorTag(TiXmlElement *subNodes, SymbolSizeInfo_t &vectorSize);
  void ProcessColorTables(TiXmlElement *colortableodes);
  void ProcessLookups(TiXmlElement *lookupNodes);
  void ProcessLinestyles(TiXmlElement *linestyleNodes);
  void ProcessPatterns(TiXmlElement *patternNodes);
  void ProcessSymbols(TiXmlElement *symbolNodes);
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

  s52plib *plib;
};

#endif
