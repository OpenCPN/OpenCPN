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
//#include "config.h"

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/filename.h>
#include <stdlib.h>

#if defined(__OCPN__ANDROID__)
 #include <qopengl.h>
 #include <GL/gl_private.h>  // this is a cut-down version of gl.h
 #include <GLES2/gl2.h>
#elif defined(__MSVC__)
 #include "glew.h"
#elif defined(__WXOSX__)
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 typedef void (*  _GLUfuncptr)();
 #define GL_COMPRESSED_RGB_FXT1_3DFX       0x86B0
#elif defined(__WXQT__) || defined(__WXGTK__)
 #include <GL/glew.h>
 #include <GL/glu.h>
#endif

#include "chartsymbols.h"
#ifdef ocpnUSE_GL
#include <wx/glcanvas.h>
#endif

#include "s52plib.h"

//--------------------------------------------------------------------------------------

ChartSymbols::ChartSymbols(void) {}

ChartSymbols::~ChartSymbols(void) {}

void ChartSymbols::InitializeTables(void) {
  rasterSymbolsLoadedColorMapNumber = -1;
  ColorTableIndex = 0;
  rasterSymbolsTexture = 0;
}

void ChartSymbols::DeleteGlobals(void) {
  m_symbolGraphicLocations.clear();

  for (unsigned int i = 0; i < m_colorTables.GetCount(); i++) {
    colTable *ct = (colTable *)m_colorTables.Item(i);
    delete ct->tableName;
    ct->colors.clear();
    ct->wxColors.clear();
    delete ct;
  }

  m_colorTables.Clear();
}

void ChartSymbols::ProcessColorTables(pugi::xml_node &node) {
  for (pugi::xml_node child = node.first_child(); child != 0;
       child = child.next_sibling()) {
    const char *pcn = child.name();

    if (!strcmp(pcn, "color-table")) {
      colTable *colortable = new colTable;
      colortable->tableName =
          new wxString(child.first_attribute().value(), wxConvUTF8);

      pugi::xml_node colorNode = child.first_child();
      while (colorNode) {
        if (!strcmp(colorNode.name(), "graphics-file")) {
          colortable->rasterFileName =
              wxString(colorNode.first_attribute().value(), wxConvUTF8);
        }

        if (!strcmp(colorNode.name(), "color")) {
          wxString key;
          S52color color;

          for (pugi::xml_attribute attr = colorNode.first_attribute(); attr;
               attr = attr.next_attribute()) {
            const char *pca = attr.name();
            if (!strcmp(pca, "name")) {
              strncpy(color.colName, attr.value(), 5);
              color.colName[5] = 0;
              key = wxString(attr.value(), wxConvUTF8);

            } else if (!strcmp(pca, "r")) {
              color.R = attr.as_int();
            } else if (!strcmp(pca, "g")) {
              color.G = attr.as_int();
            } else if (!strcmp(pca, "b")) {
              color.B = attr.as_int();
            }
          }

          colortable->colors[key] = color;
          wxColour wxcolor(color.R, color.G, color.B);
          colortable->wxColors[key] = wxcolor;
        }

        colorNode = colorNode.next_sibling();
      }

      m_colorTables.Add((void *)colortable);
    }
  }
}

#define TGET_INT_PROPERTY_VALUE(node, name, target)      \
  propVal = wxString(node->Attribute(name), wxConvUTF8); \
  propVal.ToLong(&numVal, 0);                            \
  target = numVal;
void ChartSymbols::ProcessLookups(pugi::xml_node &node) {
  Lookup lookup;

  for (pugi::xml_node child = node.first_child(); child != 0;
       child = child.next_sibling()) {
    const char *pcn = child.name();

    if (!strcmp(pcn, "lookup")) {
      for (pugi::xml_attribute attr = child.first_attribute(); attr;
           attr = attr.next_attribute()) {
        const char *pca = attr.name();
        if (!strcmp(pca, "name")) {
          lookup.name = wxString(attr.value(), wxConvUTF8);
        } else if (!strcmp(pca, "RCID")) {
          lookup.RCID = attr.as_int();
        } else if (!strcmp(pca, "id")) {
          lookup.id = attr.as_int();
        }
      }
    }

    pugi::xml_node lookupNode = child.first_child();
    while (lookupNode) {
      const char *nodeText = lookupNode.first_child().value();

      if (!strcmp(lookupNode.name(), "type")) {
        if (!strcmp(nodeText, "Area"))
          lookup.type = AREAS_T;
        else if (!strcmp(nodeText, "Line"))
          lookup.type = LINES_T;
        else
          lookup.type = POINT_T;
      } else if (!strcmp(lookupNode.name(), "disp-prio")) {
        if (!strcmp(nodeText, "Group 1"))
          lookup.displayPrio = PRIO_GROUP1;
        else if (!strcmp(nodeText, "Area 1"))
          lookup.displayPrio = PRIO_AREA_1;
        else if (!strcmp(nodeText, "Area 2"))
          lookup.displayPrio = PRIO_AREA_2;
        else if (!strcmp(nodeText, "Point Symbol"))
          lookup.displayPrio = PRIO_SYMB_POINT;
        else if (!strcmp(nodeText, "Line Symbol"))
          lookup.displayPrio = PRIO_SYMB_LINE;
        else if (!strcmp(nodeText, "Area Symbol"))
          lookup.displayPrio = PRIO_SYMB_AREA;
        else if (!strcmp(nodeText, "Routing"))
          lookup.displayPrio = PRIO_ROUTEING;
        else if (!strcmp(nodeText, "Hazards"))
          lookup.displayPrio = PRIO_HAZARDS;
        else if (!strcmp(nodeText, "Mariners"))
          lookup.displayPrio = PRIO_MARINERS;
        else
          lookup.displayPrio = PRIO_NODATA;

      }

      else if (!strcmp(lookupNode.name(), "radar-prio")) {
        if (!strcmp(nodeText, "On Top"))
          lookup.radarPrio = RAD_OVER;
        else
          lookup.radarPrio = RAD_SUPP;
      }

      else if (!strcmp(lookupNode.name(), "table-name")) {
        if (!strcmp(nodeText, "Simplified"))
          lookup.tableName = SIMPLIFIED;
        else if (!strcmp(nodeText, "Lines"))
          lookup.tableName = LINES;
        else if (!strcmp(nodeText, "Plain"))
          lookup.tableName = PLAIN_BOUNDARIES;
        else if (!strcmp(nodeText, "Symbolized"))
          lookup.tableName = SYMBOLIZED_BOUNDARIES;
        else
          lookup.tableName = PAPER_CHART;
      }

      else if (!strcmp(lookupNode.name(), "display-cat")) {
        if (!strcmp(nodeText, "Displaybase"))
          lookup.displayCat = DISPLAYBASE;
        else if (!strcmp(nodeText, "Standard"))
          lookup.displayCat = STANDARD;
        else if (!strcmp(nodeText, "Other"))
          lookup.displayCat = OTHER;
        else if (!strcmp(nodeText, "Mariners"))
          lookup.displayCat = MARINERS_STANDARD;
        else
          lookup.displayCat = OTHER;
      }

      else if (!strcmp(lookupNode.name(), "comment")) {
        lookup.comment = lookupNode.first_child().text().as_int();
      }

      else if (!strcmp(lookupNode.name(), "instruction")) {
        wxString inst(nodeText, wxConvUTF8);
        lookup.instruction = inst;
        lookup.instruction.Append('\037');

      }

      else if (!strcmp(lookupNode.name(), "attrib-code")) {
        int nc = strlen(nodeText);
        if (nc >= 6) {  //  ignore spurious short fields
          std::string attVal = std::string(nodeText);
          if(attVal.length() == 6)
            attVal += ' ';
          lookup.attributeCodeArray.push_back(attVal);
        }
      }

      lookupNode = lookupNode.next_sibling();
    }

    BuildLookup(lookup);
    lookup.attributeCodeArray.clear();
  }
}

void ChartSymbols::ProcessVectorTag(pugi::xml_node &vectorNode,
                                    SymbolSizeInfo_t &vectorSize) {
  vectorSize.size.x = vectorNode.attribute("width").as_int();
  vectorSize.size.y = vectorNode.attribute("height").as_int();

  for (pugi::xml_node child = vectorNode.first_child(); child != 0;
       child = child.next_sibling()) {
    const char *nodeType = child.name();

    if (!strcmp(nodeType, "distance")) {
      vectorSize.minDistance = child.attribute("min").as_int();
      vectorSize.maxDistance = child.attribute("max").as_int();
    }

    else if (!strcmp(nodeType, "origin")) {
      vectorSize.origin.x = child.attribute("x").as_int();
      vectorSize.origin.y = child.attribute("y").as_int();
    }

    else if (!strcmp(nodeType, "pivot")) {
      vectorSize.pivot.x = child.attribute("x").as_int();
      vectorSize.pivot.y = child.attribute("y").as_int();
    }
  }
}

void ChartSymbols::ProcessLinestyles(pugi::xml_node &node) {
  LineStyle lineStyle;

  for (pugi::xml_node child = node.first_child(); child != 0;
       child = child.next_sibling()) {
    lineStyle.RCID = child.attribute("RCID").as_int();

    pugi::xml_node lineNode = child.first_child();
    while (lineNode) {
      const char *nodeText = lineNode.first_child().value();
      const char *nodeType = lineNode.name();

      if (!strcmp(nodeType, "description"))
        lineStyle.description = nodeText;
      else if (!strcmp(nodeType, "name"))
        lineStyle.name = nodeText;
      else if (!strcmp(nodeType, "color-ref"))
        lineStyle.colorRef = nodeText;
      else if (!strcmp(nodeType, "HPGL"))
        lineStyle.HPGL = nodeText;
      else if (!strcmp(nodeType, "vector"))
        ProcessVectorTag(lineNode, lineStyle.vectorSize);

      lineNode = lineNode.next_sibling();
    }
    BuildLineStyle(lineStyle);
  }
}

void ChartSymbols::ProcessPatterns(pugi::xml_node &node) {
  OCPNPattern pattern;

  for (pugi::xml_node child = node.first_child(); child != 0;
       child = child.next_sibling()) {
    pattern.RCID = child.attribute("RCID").as_int();

    pattern.hasVector = false;
    pattern.hasBitmap = false;
    pattern.preferBitmap = true;

    pugi::xml_node pattNode = child.first_child();
    while (pattNode) {
      const char *nodeText = pattNode.first_child().value();
      const char *nodeType = pattNode.name();

      if (!strcmp(nodeType, "description"))
        pattern.description = nodeText;
      else if (!strcmp(nodeType, "name"))
        pattern.name = nodeText;
      else if (!strcmp(nodeType, "filltype"))
        pattern.fillType = nodeText[0];
      else if (!strcmp(nodeType, "spacing"))
        pattern.spacing = nodeText[0];
      else if (!strcmp(nodeType, "definition"))
        pattern.hasVector = !strcmp(nodeText, "V");
      else if (!strcmp(nodeType, "color-ref"))
        pattern.colorRef = nodeText;
      else if (!strcmp(nodeType, "HPGL")) {
        pattern.HPGL = nodeText;
        pattern.hasVector = true;
      }

      else if (!strcmp(nodeType, "prefer-bitmap")) {
        if (!strcmp(nodeText, "no"))
          pattern.preferBitmap = false;
        else if (!strcmp(nodeText, "false"))
          pattern.preferBitmap = false;
      }

      else if (!strcmp(nodeType, "bitmap")) {
        pattern.bitmapSize.size.x = pattNode.attribute("width").as_int();
        pattern.bitmapSize.size.y = pattNode.attribute("height").as_int();

        for (pugi::xml_node child = pattNode.first_child(); child != 0;
             child = child.next_sibling()) {
          const char *nodeType = child.name();

          if (!strcmp(nodeType, "distance")) {
            pattern.bitmapSize.minDistance = child.attribute("min").as_int();
            pattern.bitmapSize.maxDistance = child.attribute("max").as_int();
          } else if (!strcmp(nodeType, "origin")) {
            pattern.bitmapSize.origin.x = child.attribute("x").as_int();
            pattern.bitmapSize.origin.y = child.attribute("y").as_int();
          } else if (!strcmp(nodeType, "pivot")) {
            pattern.bitmapSize.pivot.x = child.attribute("x").as_int();
            pattern.bitmapSize.pivot.y = child.attribute("y").as_int();
          } else if (!strcmp(nodeType, "graphics-location")) {
            pattern.bitmapSize.graphics.x = child.attribute("x").as_int();
            pattern.bitmapSize.graphics.y = child.attribute("y").as_int();
          }
        }
      }

      else if (!strcmp(nodeType, "vector"))
        ProcessVectorTag(pattNode, pattern.vectorSize);

      pattNode = pattNode.next_sibling();
    }

    BuildPattern(pattern);
  }
}

void ChartSymbols::ProcessSymbols(pugi::xml_node &node) {
  ChartSymbol symbol;

  for (pugi::xml_node child = node.first_child(); child != 0;
       child = child.next_sibling()) {
    symbol.RCID = child.attribute("RCID").as_int();

    symbol.hasVector = false;
    symbol.hasBitmap = false;
    symbol.preferBitmap = true;

    pugi::xml_node symbolNode = child.first_child();
    while (symbolNode) {
      const char *nodeText = symbolNode.first_child().value();
      const char *nodeType = symbolNode.name();

      if (!strcmp(nodeType, "description"))
        symbol.description = nodeText;
      else if (!strcmp(nodeType, "name"))
        symbol.name = nodeText;
      else if (!strcmp(nodeType, "definition"))
        symbol.hasVector = !strcmp(nodeText, "V");
      else if (!strcmp(nodeType, "color-ref"))
        symbol.colorRef = nodeText;

      else if (!strcmp(nodeType, "prefer-bitmap")) {
        if (!strcmp(nodeText, "no"))
          symbol.preferBitmap = false;
        else if (!strcmp(nodeText, "false"))
          symbol.preferBitmap = false;
      }

      else if (!strcmp(nodeType, "bitmap")) {
        symbol.bitmapSize.size.x = symbolNode.attribute("width").as_int();
        symbol.bitmapSize.size.y = symbolNode.attribute("height").as_int();
        symbol.hasBitmap = true;

        for (pugi::xml_node child = symbolNode.first_child(); child != 0;
             child = child.next_sibling()) {
          const char *nodeType = child.name();

          if (!strcmp(nodeType, "distance")) {
            symbol.bitmapSize.minDistance = child.attribute("min").as_int();
            symbol.bitmapSize.maxDistance = child.attribute("max").as_int();
          } else if (!strcmp(nodeType, "origin")) {
            symbol.bitmapSize.origin.x = child.attribute("x").as_int();
            symbol.bitmapSize.origin.y = child.attribute("y").as_int();
          } else if (!strcmp(nodeType, "pivot")) {
            symbol.bitmapSize.pivot.x = child.attribute("x").as_int();
            symbol.bitmapSize.pivot.y = child.attribute("y").as_int();
          } else if (!strcmp(nodeType, "graphics-location")) {
            symbol.bitmapSize.graphics.x = child.attribute("x").as_int();
            symbol.bitmapSize.graphics.y = child.attribute("y").as_int();
          }
        }
      }

      else if (!strcmp(nodeType, "vector")) {
        symbol.vectorSize.size.x = symbolNode.attribute("width").as_int();
        symbol.vectorSize.size.y = symbolNode.attribute("height").as_int();
        symbol.hasVector = true;

        for (pugi::xml_node child = symbolNode.first_child(); child != 0;
             child = child.next_sibling()) {
          const char *nodeType = child.name();

          if (!strcmp(nodeType, "distance")) {
            symbol.vectorSize.minDistance = child.attribute("min").as_int();
            symbol.vectorSize.maxDistance = child.attribute("max").as_int();
          } else if (!strcmp(nodeType, "origin")) {
            symbol.vectorSize.origin.x = child.attribute("x").as_int();
            symbol.vectorSize.origin.y = child.attribute("y").as_int();
          } else if (!strcmp(nodeType, "pivot")) {
            symbol.vectorSize.pivot.x = child.attribute("x").as_int();
            symbol.vectorSize.pivot.y = child.attribute("y").as_int();
          } else if (!strcmp(nodeType, "HPGL")) {
            symbol.HPGL = wxString(child.first_child().value(), wxConvUTF8);
          }
        }
      }

      symbolNode = symbolNode.next_sibling();
    }

    BuildSymbol(symbol);
  }
}


void ChartSymbols::BuildLookup(Lookup &lookup) {
  auto LUP = new LUPrec();

  LUP->RCID = lookup.RCID;
  LUP->nSequence = lookup.id;
  LUP->DISC = lookup.displayCat;
  LUP->FTYP = lookup.type;
  LUP->DPRI = lookup.displayPrio;
  LUP->RPRI = lookup.radarPrio;
  LUP->TNAM = lookup.tableName;
  LUP->OBCL[6] = 0;
  memcpy(LUP->OBCL, lookup.name.mb_str(), 7);

  LUP->ATTArray = lookup.attributeCodeArray;

  LUP->INST = lookup.instruction;
  LUP->LUCM = lookup.comment;

  // Add LUP to array
  // Search the LUPArray to see if there is already a LUP with this RCID
  // If found, replace it with the new LUP
  // This provides a facility for updating the LUP tables after loading a basic
  // set

  unsigned int index = 0;
  wxArrayOfLUPrec *pLUPARRAYtyped = plib->SelectLUPARRAY(LUP->TNAM);

  while (index < pLUPARRAYtyped->GetCount()) {
    auto pLUPCandidate = pLUPARRAYtyped->Item(index);
    if (LUP->RCID == pLUPCandidate->RCID) {
      pLUPARRAYtyped->RemoveAt(index);
      plib->DestroyLUP(pLUPCandidate);  // empties the LUP
      break;
    }
    index++;
  }
  pLUPARRAYtyped->Add(LUP);
}

void ChartSymbols::BuildLineStyle(LineStyle &lineStyle) {
  Rule *lnstmp = NULL;
  Rule *lnst = (Rule *)calloc(1, sizeof(Rule));
  plib->pAlloc->Add(lnst);

  lnst->RCID = lineStyle.RCID;
  memcpy(lnst->name.PANM, lineStyle.name.mb_str(), 8);
  lnst->bitmap.PBTM = NULL;

  lnst->vector.LVCT = (char *)malloc(lineStyle.HPGL.Len() + 1);
  strcpy(lnst->vector.LVCT, lineStyle.HPGL.mb_str());

  lnst->colRef.LCRF = (char *)malloc(lineStyle.colorRef.Len() + 1);
  strcpy(lnst->colRef.LCRF, lineStyle.colorRef.mb_str());

  lnst->pos.line.minDist.PAMI = lineStyle.vectorSize.minDistance;
  lnst->pos.line.maxDist.PAMA = lineStyle.vectorSize.maxDistance;

  lnst->pos.line.pivot_x.PACL = lineStyle.vectorSize.pivot.x;
  lnst->pos.line.pivot_y.PARW = lineStyle.vectorSize.pivot.y;

  lnst->pos.line.bnbox_w.PAHL = lineStyle.vectorSize.size.x;
  lnst->pos.line.bnbox_h.PAVL = lineStyle.vectorSize.size.y;

  lnst->pos.line.bnbox_x.SBXC = lineStyle.vectorSize.origin.x;
  lnst->pos.line.bnbox_y.SBXR = lineStyle.vectorSize.origin.y;

  lnstmp = (*plib->_line_sym)[lineStyle.name];

  if (NULL == lnstmp)
    (*plib->_line_sym)[lineStyle.name] = lnst;
  else if (strncmp(lnst->name.LINM, lnstmp->name.LINM, 8))
    (*plib->_line_sym)[lineStyle.name] = lnst;
}


void ChartSymbols::BuildPattern(OCPNPattern &pattern) {
  Rule *pattmp = NULL;

  Rule *patt = (Rule *)calloc(1, sizeof(Rule));
  plib->pAlloc->Add(patt);

  patt->RCID = pattern.RCID;
  patt->exposition.PXPO = new wxString(pattern.description);
  memcpy(patt->name.PANM, pattern.name.mb_str(), 8);
  patt->bitmap.PBTM = NULL;
  patt->fillType.PATP = pattern.fillType;
  patt->spacing.PASP = pattern.spacing;

  patt->vector.PVCT = (char *)malloc(pattern.HPGL.Len() + 1);
  strcpy(patt->vector.PVCT, pattern.HPGL.mb_str());

  patt->colRef.PCRF = (char *)malloc(pattern.colorRef.Len() + 1);
  strcpy(patt->colRef.PCRF, pattern.colorRef.mb_str());

  SymbolSizeInfo_t patternSize;

  if (pattern.hasVector && !(pattern.preferBitmap && pattern.hasBitmap)) {
    patt->definition.PADF = 'V';
    patternSize = pattern.vectorSize;
  } else {
    patt->definition.PADF = 'R';
    patternSize = pattern.bitmapSize;
  }

  patt->pos.patt.minDist.PAMI = patternSize.minDistance;
  patt->pos.patt.maxDist.PAMA = patternSize.maxDistance;

  patt->pos.patt.pivot_x.PACL = patternSize.pivot.x;
  patt->pos.patt.pivot_y.PARW = patternSize.pivot.y;

  patt->pos.patt.bnbox_w.PAHL = patternSize.size.x;
  patt->pos.patt.bnbox_h.PAVL = patternSize.size.y;

  patt->pos.patt.bnbox_x.SBXC = patternSize.origin.x;
  patt->pos.patt.bnbox_y.SBXR = patternSize.origin.y;

  wxRect graphicsLocation(pattern.bitmapSize.graphics, pattern.bitmapSize.size);
  m_symbolGraphicLocations[pattern.name] = graphicsLocation;

  // check if key already there

  pattmp = (*plib->_patt_sym)[pattern.name];

  if (NULL == pattmp) {
    (*plib->_patt_sym)[pattern.name] = patt;  // insert in hash table
  } else  // already something here with same key...
  {       // if the pattern names are not identical
    if (strncmp(patt->name.PANM, pattmp->name.PANM, 8)) {
      (*plib->_patt_sym)[pattern.name] = patt;  // replace the pattern
      plib->DestroyPatternRuleNode(
          pattmp);  // remember to free to replaced node
                    // the node itself is destroyed as part of pAlloc
    }
  }
}


void ChartSymbols::BuildSymbol(ChartSymbol &symbol) {
  Rule *symb = (Rule *)calloc(1, sizeof(Rule));
  plib->pAlloc->Add(symb);

  wxString SVCT;
  wxString SCRF;

  symb->RCID = symbol.RCID;
  memcpy(symb->name.SYNM, symbol.name.char_str(), 8);

  symb->exposition.SXPO = new wxString(symbol.description);

  symb->vector.SVCT = (char *)malloc(symbol.HPGL.Len() + 1);
  strcpy(symb->vector.SVCT, symbol.HPGL.mb_str());

  symb->colRef.SCRF = (char *)malloc(symbol.colorRef.Len() + 1);
  strcpy(symb->colRef.SCRF, symbol.colorRef.mb_str());

  symb->bitmap.SBTM = NULL;

  SymbolSizeInfo_t symbolSize;

  if (symbol.hasVector && !(symbol.preferBitmap && symbol.hasBitmap)) {
    symb->definition.SYDF = 'V';
    symbolSize = symbol.vectorSize;
  } else {
    symb->definition.SYDF = 'R';
    symbolSize = symbol.bitmapSize;
  }

  symb->pos.symb.minDist.PAMI = symbolSize.minDistance;
  symb->pos.symb.maxDist.PAMA = symbolSize.maxDistance;

  symb->pos.symb.pivot_x.SYCL = symbolSize.pivot.x;
  symb->pos.symb.pivot_y.SYRW = symbolSize.pivot.y;

  symb->pos.symb.bnbox_w.SYHL = symbolSize.size.x;
  symb->pos.symb.bnbox_h.SYVL = symbolSize.size.y;

  symb->pos.symb.bnbox_x.SBXC = symbolSize.origin.x;
  symb->pos.symb.bnbox_y.SBXR = symbolSize.origin.y;

  wxRect graphicsLocation(symbol.bitmapSize.graphics, symbol.bitmapSize.size);
  m_symbolGraphicLocations[symbol.name] = graphicsLocation;

  // Already something here with same key? Then free its strings, otherwise they
  // leak.
  Rule *symbtmp = (*plib->_symb_sym)[symbol.name];
  if (symbtmp) {
    free(symbtmp->colRef.SCRF);
    free(symbtmp->vector.SVCT);
    delete symbtmp->exposition.SXPO;
  }

  (*plib->_symb_sym)[symbol.name] = symb;
}

bool ChartSymbols::LoadConfigFile(s52plib *plibArg,
                                  const wxString &s52ilePath) {
  plib = plibArg;

  // Expect to find library data XML file in same folder as other S52 data.
  // Files in CWD takes precedence.

  wxString name, extension;
  wxString xmlFileName = _T("chartsymbols.xml");

  wxFileName::SplitPath(s52ilePath, &configFileDirectory, &name, &extension);
  wxString fullFilePath =
      configFileDirectory + wxFileName::GetPathSeparator() + xmlFileName;

  if (wxFileName::FileExists(xmlFileName)) {
    fullFilePath = xmlFileName;
    configFileDirectory = _T(".");
  }

  if (!wxFileName::FileExists(fullFilePath)) {
    wxString msg(_T("ChartSymbols ConfigFile not found: "));
    msg += fullFilePath;
    wxLogMessage(msg);
    return false;
  }

  if (m_symbolsDoc.load_file(fullFilePath.fn_str())) {
    wxString msg(_T("ChartSymbols loaded from "));
    msg += fullFilePath;
    wxLogMessage(msg);

    pugi::xml_node elements = m_symbolsDoc.child("chartsymbols");

    for (pugi::xml_node element = elements.first_child(); element;
         element = element.next_sibling()) {
      if (!strcmp(element.name(), "color-tables"))
        ProcessColorTables(element);
      else if (!strcmp(element.name(), "lookups"))
        ProcessLookups(element);
      else if (!strcmp(element.name(), "line-styles"))
        ProcessLinestyles(element);
      else if (!strcmp(element.name(), "patterns"))
        ProcessPatterns(element);
      else if (!strcmp(element.name(), "symbols"))
        ProcessSymbols(element);
    }
    m_symbolsDoc.reset();  // purge the document to recover memory;
  }


  return true;
}
void ChartSymbols::SetColorTableIndex(int index, bool flush,
                                      const ChartCtx& ctx) {
  ColorTableIndex = index;
  LoadRasterFileForColorTable(ColorTableIndex, flush, ctx);
}

int ChartSymbols::LoadRasterFileForColorTable(int tableNo, bool flush,
                                              const ChartCtx& ctx) {
  if (tableNo == rasterSymbolsLoadedColorMapNumber && !flush) {
    if (ctx.m_use_opengl) {
      if (rasterSymbolsTexture) return true;
#ifdef ocpnUSE_GL
      if (!ctx.m_texture_rectangle_format && rasterSymbols.IsOk())
        return true;
#endif
    }
    if (rasterSymbols.IsOk()) return true;
  }

  colTable *coltab = (colTable *)m_colorTables.Item(tableNo);

  wxString filename = configFileDirectory + wxFileName::GetPathSeparator() +
                      coltab->rasterFileName;

  wxImage rasterFileImg;
  if (rasterFileImg.LoadFile(filename, wxBITMAP_TYPE_PNG)) {
#ifdef ocpnUSE_GL
    /* for opengl mode, load the symbols into a texture */
    if (ctx.m_use_opengl && ctx.m_texture_rectangle_format) {
      int w = rasterFileImg.GetWidth();
      int h = rasterFileImg.GetHeight();

      //    Get the glRGBA format data from the wxImage
      unsigned char *d = rasterFileImg.GetData();
      unsigned char *a = rasterFileImg.GetAlpha();

      /* combine rgb with alpha */
      unsigned char *e = (unsigned char *)malloc(w * h * 4);
      if (d && a) {
        for (int y = 0; y < h; y++)
          for (int x = 0; x < w; x++) {
            int off = (y * w + x);

            e[off * 4 + 0] = d[off * 3 + 0];
            e[off * 4 + 1] = d[off * 3 + 1];
            e[off * 4 + 2] = d[off * 3 + 2];
            e[off * 4 + 3] = a[off];
          }
      }
      if (!rasterSymbolsTexture) glGenTextures(1, &rasterSymbolsTexture);

      glBindTexture(ctx.m_texture_rectangle_format, rasterSymbolsTexture);

      /* unfortunately this texture looks terrible with compression */
      GLuint format = GL_RGBA;
      glTexImage2D(ctx.m_texture_rectangle_format, 0, format, w, h, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, e);

      //             glTexParameteri( g_texture_rectangle_format,
      //             GL_TEXTURE_MAG_FILTER, GL_NEAREST ); glTexParameteri(
      //             g_texture_rectangle_format, GL_TEXTURE_MIN_FILTER,
      //             GL_NEAREST );

      glTexParameteri(ctx.m_texture_rectangle_format, GL_TEXTURE_WRAP_S,
                      GL_CLAMP_TO_EDGE);
      glTexParameteri(ctx.m_texture_rectangle_format, GL_TEXTURE_WRAP_T,
                      GL_CLAMP_TO_EDGE);

      glTexParameteri(ctx.m_texture_rectangle_format, GL_TEXTURE_MAG_FILTER,
                      GL_NEAREST);  // No mipmapping
      glTexParameteri(ctx.m_texture_rectangle_format, GL_TEXTURE_MIN_FILTER,
                      GL_NEAREST);

      rasterSymbolsTextureSize = wxSize(w, h);

      glDisable(GL_TEXTURE_2D);

      free(e);
    }
#endif
    { rasterSymbols = wxBitmap(rasterFileImg, -1 /*32*/); }

    rasterSymbolsLoadedColorMapNumber = tableNo;
    return true;
  }

  wxString msg(_T("ChartSymbols...Failed to load raster symbols file "));
  msg += filename;
  wxLogMessage(msg);
  return false;
}


S52color *ChartSymbols::GetColor(const char *colorName, int fromTable) {
  colTable *colortable;
  wxString key(colorName, wxConvUTF8, 5);
  colortable = (colTable *)m_colorTables.Item(fromTable);
  return &(colortable->colors[key]);
}

wxColor ChartSymbols::GetwxColor(const wxString &colorName, int fromTable) {
  colTable *colortable;
  colortable = (colTable *)m_colorTables.Item(fromTable);
  wxColor c = colortable->wxColors[colorName];
  return c;
}

wxColor ChartSymbols::GetwxColor(const char *colorName, int fromTable) {
  wxString key(colorName, wxConvUTF8, 5);
  return GetwxColor(key, fromTable);
}

int ChartSymbols::FindColorTable(const wxString &tableName) {
  for (unsigned int i = 0; i < m_colorTables.GetCount(); i++) {
    colTable *ct = (colTable *)m_colorTables.Item(i);
    if (tableName.IsSameAs(*ct->tableName)) {
      return i;
    }
  }
  return 0;
}

bool ChartSymbols::UpdateTableColor( std::string table_name, std::string color_name, wxColor& new_color){
  int table_index = FindColorTable( wxString(table_name.c_str()));

  colTable *colortable = (colTable *)m_colorTables.Item(table_index);
  wxString key(color_name.c_str(), wxConvUTF8, 5);

  colortable->wxColors[key] = new_color;
  if (colortable->colors.find(key) != colortable->colors.end()) {
    S52color *color = &colortable->colors[key];
    color->R = new_color.Red();
    color->G = new_color.Green();
    color->B = new_color.Blue();
    return true;
  }
  else
    return false;
}


wxString ChartSymbols::HashKey(const char *symbolName) {
  char key[9];
  key[8] = 0;
  strncpy(key, symbolName, 8);
  return wxString(key, wxConvUTF8);
}

wxImage ChartSymbols::GetImage(const char *symbolName) {
  wxRect bmArea = m_symbolGraphicLocations[HashKey(symbolName)];
  if (rasterSymbols.IsOk()) {
    wxBitmap bitmap = rasterSymbols.GetSubBitmap(bmArea);
    return bitmap.ConvertToImage();
  } else
    return wxImage(1, 1);
}

unsigned int ChartSymbols::GetGLTextureRect(wxRect &rect,
                                            const char *symbolName) {
  rect = m_symbolGraphicLocations[HashKey(symbolName)];
  return rasterSymbolsTexture;
}

wxSize ChartSymbols::GLTextureSize() { return rasterSymbolsTextureSize; }
