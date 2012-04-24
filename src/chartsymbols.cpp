/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Symbols
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   bdbcat@yahoo.com                                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/filename.h>
#include <stdlib.h>

#include "chartsymbols.h"

//--------------------------------------------------------------------------------------
// The below data is global since there will ever only be one ChartSymbols instance,
// and some methods+data of class S52plib are needed inside ChartSymbol, and s2plib
// needs some methods from ChartSymbol. So s52plib only calls static methods in
// order to resolve circular include file dependencies.

wxArrayPtrVoid* colorTables;
wxBitmap rasterSymbols;
int	rasterSymbolsLoadedColorMapNumber;
wxString configFileDirectory;

WX_DECLARE_STRING_HASH_MAP( wxRect, symbolGraphicsHashMap );

symbolGraphicsHashMap* symbolGraphicLocations;
//--------------------------------------------------------------------------------------


ChartSymbols::ChartSymbols( void ) {
	colorTables = new wxArrayPtrVoid;
	symbolGraphicLocations = new symbolGraphicsHashMap;
	rasterSymbolsLoadedColorMapNumber = -1;
}

ChartSymbols::~ChartSymbols( void ) {}

#define GET_INT_PROPERTY_VALUE( node, name, target )  \
	propVal = node->GetPropVal( _(name), _("0")); \
	propVal.ToLong( &numVal, 0 ); \
	target = numVal;

void ChartSymbols::ProcessColorTables( wxXmlNode* colortableNodes ) {

	wxXmlNode* tableNode = colortableNodes->GetChildren();

	while(tableNode) {
		colTable *colortable = new colTable;
		colortable->tableName = new wxString( tableNode->GetPropVal( _("name"), _("<void>")));

		wxXmlNode* colorNode = tableNode->GetChildren();

		while( colorNode ) {
			S52color color;
			wxString propVal;
			long numVal;

			if( colorNode->GetName() == _("graphics-file") ) {
				colortable->rasterFileName = colorNode->GetPropVal( _("name"), _("<void>") );
				goto next;
			}
			else {
				GET_INT_PROPERTY_VALUE( colorNode, "r", color.R )
				GET_INT_PROPERTY_VALUE( colorNode, "g", color.G )
				GET_INT_PROPERTY_VALUE( colorNode, "b", color.B )

				wxString key = colorNode->GetPropVal( _("name"), _("<N/A>"));
				strncpy( color.colName, key.char_str(), 5 );
				color.colName[5] = 0;

				colortable->colors[key] = color;

				wxColour wxcolor ( color.R, color.G, color.B );
				colortable->wxColors[key] = wxcolor;
			}

			next: colorNode = colorNode->GetNext();
		}

		colorTables->Add( (void *) colortable );
		tableNode = tableNode->GetNext();
	}
}


void ChartSymbols::ProcessLookups( wxXmlNode* lookupNodes ) {
	wxXmlNode* lookupNode = lookupNodes->GetChildren();
	Lookup lookup;
	wxString propVal;
	long numVal;

	while(lookupNode) {

		GET_INT_PROPERTY_VALUE( lookupNode, "id", lookup.id )
		GET_INT_PROPERTY_VALUE( lookupNode, "RCID", lookup.RCID )
		lookup.name = lookupNode->GetPropVal( _("name"), _("<void>"));
		lookup.attributeCodeArray = NULL;

		wxXmlNode* subNodes = lookupNode->GetChildren();

		while( subNodes ) {
			if( subNodes->GetName() == _("type")) {
				wxString typeStr = subNodes->GetNodeContent();
				if( typeStr == _("Area") ) lookup.type = AREAS_T;
				else if( typeStr == _("Line") ) lookup.type = LINES_T;
				else lookup.type = POINT_T;
				goto nextNode;
			}
			if( subNodes->GetName() == _("disp-prio")) {
				wxString typeStr = subNodes->GetNodeContent();
				if( typeStr == _("Group 1") ) lookup.displayPrio = PRIO_GROUP1;
				else if( typeStr == _("Area 1") ) lookup.displayPrio = PRIO_AREA_1;
				else if( typeStr == _("Area 2") ) lookup.displayPrio = PRIO_AREA_2;
				else if( typeStr == _("Point Symbol") ) lookup.displayPrio = PRIO_SYMB_POINT;
				else if( typeStr == _("Line Symbol") ) lookup.displayPrio = PRIO_SYMB_LINE;
				else if( typeStr == _("Area Symbol") ) lookup.displayPrio = PRIO_SYMB_AREA;
				else if( typeStr == _("Routing") ) lookup.displayPrio = PRIO_ROUTEING;
				else if( typeStr == _("Hazards") ) lookup.displayPrio = PRIO_HAZARDS;
				else lookup.displayPrio = PRIO_MARINERS;
				goto nextNode;
			}
			if( subNodes->GetName() == _("radar-prio")) {
				wxString typeStr = subNodes->GetNodeContent();
				if( typeStr == _("On Top") ) lookup.radarPrio = RAD_OVER;
				else lookup.radarPrio = RAD_SUPP;
				goto nextNode;
			}
			if( subNodes->GetName() == _("table-name")) {
				wxString typeStr = subNodes->GetNodeContent();
				if( typeStr == _("Simplified") ) lookup.tableName = SIMPLIFIED;
				else if( typeStr == _("Lines") ) lookup.tableName = LINES;
				else if( typeStr == _("Plain") ) lookup.tableName = PLAIN_BOUNDARIES;
				else if( typeStr == _("Symbolized") ) lookup.tableName = SYMBOLIZED_BOUNDARIES;
				else lookup.tableName = PAPER_CHART;
				goto nextNode;
			}
			if( subNodes->GetName() == _("display-cat")) {
				wxString typeStr = subNodes->GetNodeContent();
				if( typeStr == _("Displaybase") ) lookup.displayCat = DISPLAYBASE;
				else if( typeStr == _("Standard") ) lookup.displayCat = STANDARD;
				else if( typeStr == _("Other") ) lookup.displayCat = OTHER;
				else if( typeStr == _("Mariners") ) lookup.displayCat = MARINERS_STANDARD;
				else lookup.displayCat = OTHER;
				goto nextNode;
			}
			if( subNodes->GetName() == _("comment")) {
				wxString comment = subNodes->GetNodeContent();
				long value;
				comment.ToLong( &value, 0 );
				lookup.comment = value;
				goto nextNode;
			}
			if( subNodes->GetName() == _("instruction")) {
				lookup.instruction = subNodes->GetNodeContent();
				lookup.instruction.Append( '\037' );
				goto nextNode;
			}
			if( subNodes->GetName() == _("attrib-code")) {
				if( ! lookup.attributeCodeArray ) lookup.attributeCodeArray = new wxArrayString();
				lookup.attributeCodeArray->Add( subNodes->GetNodeContent() );
				goto nextNode;
			}

			nextNode: subNodes = subNodes->GetNext();
		}

		BuildLookup( lookup );
		lookupNode = lookupNode->GetNext();
	}
}


void ChartSymbols::BuildLookup( Lookup &lookup ) {

	LUPrec *LUP = (LUPrec*) calloc( 1, sizeof(LUPrec) );
	plib->pAlloc->Add( LUP );

	LUP->RCID = lookup.RCID;
	LUP->nSequence = lookup.id;
	LUP->DISC = lookup.displayCat;
	LUP->FTYP = lookup.type;
	LUP->DPRI = lookup.displayPrio;
	LUP->RPRI = lookup.radarPrio;
	LUP->TNAM = lookup.tableName;
	strcpy( LUP->OBCL, lookup.name.mb_str() );

	LUP->ATTCArray = lookup.attributeCodeArray;

	LUP->INST = new wxString(lookup.instruction);
	LUP->LUCM = lookup.comment;

	// Add LUP to array
	// Search the LUPArray to see if there is already a LUP with this RCID
	// If found, replace it with the new LUP
	// This provides a facility for updating the LUP tables after loading a basic set

	unsigned int index = 0;
	wxArrayOfLUPrec *pLUPARRAYtyped = plib->SelectLUPARRAY( LUP->TNAM );

	while( index < pLUPARRAYtyped->GetCount() ) {
		LUPrec *pLUPCandidate = pLUPARRAYtyped->Item( index );
		if( LUP->RCID == pLUPCandidate->RCID ) {
			plib->DestroyLUP( pLUPCandidate ); // empties the LUP
			pLUPARRAYtyped->Remove( pLUPCandidate );
			break;
		}
		index++;
	}

	pLUPARRAYtyped->Add( LUP );
}


void ChartSymbols::ProcessVectorTag( wxXmlNode* vectorNode, SymbolSizeInfo_t &vectorSize ) {
	wxString propVal;
	long numVal;
	GET_INT_PROPERTY_VALUE( vectorNode, "width", vectorSize.size.x )
	GET_INT_PROPERTY_VALUE( vectorNode, "height", vectorSize.size.y )
	wxXmlNode* vectorNodes = vectorNode->GetChildren();
	while( vectorNodes ) {
		if( vectorNodes->GetName() == _("distance")) {
			GET_INT_PROPERTY_VALUE( vectorNodes, "min", vectorSize.minDistance )
			GET_INT_PROPERTY_VALUE( vectorNodes, "max", vectorSize.maxDistance )
			goto nextVector;
		}
		if( vectorNodes->GetName() == _("origin")) {
			GET_INT_PROPERTY_VALUE( vectorNodes, "x", vectorSize.origin.x )
			GET_INT_PROPERTY_VALUE( vectorNodes, "y", vectorSize.origin.y )
			goto nextVector;
		}
		if( vectorNodes->GetName() == _("pivot")) {
			GET_INT_PROPERTY_VALUE( vectorNodes, "x", vectorSize.pivot.x )
			GET_INT_PROPERTY_VALUE( vectorNodes, "y", vectorSize.pivot.y )
			goto nextVector;
		}
		nextVector: vectorNodes = vectorNodes->GetNext();
	}
}

void ChartSymbols::ProcessLinestyles( wxXmlNode* linestyleNodes ) {
	LineStyle lineStyle;
	wxString propVal;
	long numVal;
	wxXmlNode* lineStyleNode = linestyleNodes->GetChildren();

	while(lineStyleNode) {

		GET_INT_PROPERTY_VALUE( lineStyleNode, "RCID", lineStyle.RCID )

		wxXmlNode* subNodes = lineStyleNode->GetChildren();

		while( subNodes ) {
			if( subNodes->GetName() == _("description")) {
				lineStyle.description = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("name")) {
				lineStyle.name = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("color-ref")) {
				lineStyle.colorRef = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("HPGL")) {
				lineStyle.HPGL = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("vector")) {
				ProcessVectorTag( subNodes, lineStyle.vectorSize );
			}
			nextNode: subNodes = subNodes->GetNext();
		}

		BuildLineStyle( lineStyle );
		lineStyleNode = lineStyleNode->GetNext();
	}
}


void ChartSymbols::BuildLineStyle( LineStyle &lineStyle ) {
	Rule *lnstmp = NULL;
	Rule *lnst = (Rule*) calloc( 1, sizeof(Rule) );
	plib->pAlloc->Add( lnst );

	lnst->exposition.LXPO = new wxString( lineStyle.description );
	lnst->RCID = lineStyle.RCID;
	strcpy( lnst->name.PANM, lineStyle.name.mb_str() );
	lnst->bitmap.PBTM = NULL;

	lnst->vector.LVCT = (char *) calloc( lineStyle.HPGL.Len() + 1, 1 );
	strncpy( lnst->vector.LVCT, lineStyle.HPGL.mb_str(), lineStyle.HPGL.Len() );

	lnst->colRef.LCRF = (char *) calloc( lineStyle.colorRef.Len() + 1, 1 );
	strncpy( lnst->colRef.LCRF, lineStyle.colorRef.mb_str(), lineStyle.colorRef.Len() );

	lnst->pos.line.minDist.PAMI = lineStyle.vectorSize.minDistance;
	lnst->pos.line.maxDist.PAMA = lineStyle.vectorSize.maxDistance;

	lnst->pos.line.pivot_x.PACL = lineStyle.vectorSize.pivot.x;
	lnst->pos.line.pivot_y.PARW = lineStyle.vectorSize.pivot.y;

	lnst->pos.line.bnbox_w.PAHL = lineStyle.vectorSize.size.x;
	lnst->pos.line.bnbox_h.PAVL = lineStyle.vectorSize.size.y;

	lnst->pos.line.bnbox_x.SBXC = lineStyle.vectorSize.origin.x;
	lnst->pos.line.bnbox_y.SBXR = lineStyle.vectorSize.origin.y;

	lnstmp = ( *plib->_line_sym )[lineStyle.name];

	if( NULL == lnstmp ) ( *plib->_line_sym )[lineStyle.name] = lnst;
	else if( lnst->name.LINM != lnstmp->name.LINM ) ( *plib->_line_sym )[lineStyle.name] = lnst;
}


void ChartSymbols::ProcessPatterns( wxXmlNode* patternNodes ) {
	wxXmlNode* patternNode = patternNodes->GetChildren();
	Pattern pattern;
	wxString propVal;
	long numVal;

	while(patternNode) {

		GET_INT_PROPERTY_VALUE( patternNode, "RCID", pattern.RCID )

		pattern.hasVector = false;
		pattern.hasBitmap = false;
		pattern.preferBitmap = true;

		wxXmlNode* subNodes = patternNode->GetChildren();

		while( subNodes ) {
			if( subNodes->GetName() == _("description")) {
				pattern.description = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("name")) {
				pattern.name = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("filltype")) {
				pattern.fillType = *subNodes->GetNodeContent().c_str();
				goto nextNode;
			}
			if( subNodes->GetName() == _("spacing")) {
				pattern.spacing = *subNodes->GetNodeContent().c_str();
				goto nextNode;
			}
			if( subNodes->GetName() == _("color-ref")) {
				pattern.colorRef = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("definition")) {
				if( subNodes->GetNodeContent() == _("V") ) pattern.hasVector = true;
				goto nextNode;
			}
			if( subNodes->GetName() == _("prefer-bitmap")) {
				if( subNodes->GetNodeContent().Lower() == _("no") ) pattern.preferBitmap = false;
				if( subNodes->GetNodeContent().Lower() == _("false") ) pattern.preferBitmap = false;
				goto nextNode;
			}
			if( subNodes->GetName() == _("bitmap")) {
				GET_INT_PROPERTY_VALUE( subNodes, "width", pattern.bitmapSize.size.x )
				GET_INT_PROPERTY_VALUE( subNodes, "height", pattern.bitmapSize.size.y )
				pattern.hasBitmap = true;
				wxXmlNode* bitmapNodes = subNodes->GetChildren();
				while( bitmapNodes ) {
					if( bitmapNodes->GetName() == _("distance")) {
						GET_INT_PROPERTY_VALUE( bitmapNodes, "min", pattern.bitmapSize.minDistance )
						GET_INT_PROPERTY_VALUE( bitmapNodes, "max", pattern.bitmapSize.maxDistance )
						goto nextBitmap;
					}
					if( bitmapNodes->GetName() == _("origin")) {
						GET_INT_PROPERTY_VALUE( bitmapNodes, "x", pattern.bitmapSize.origin.x )
						GET_INT_PROPERTY_VALUE( bitmapNodes, "y", pattern.bitmapSize.origin.y )
						goto nextBitmap;
					}
					if( bitmapNodes->GetName() == _("pivot")) {
						GET_INT_PROPERTY_VALUE( bitmapNodes, "x", pattern.bitmapSize.pivot.x )
						GET_INT_PROPERTY_VALUE( bitmapNodes, "y", pattern.bitmapSize.pivot.y )
						goto nextBitmap;
					}
					if( bitmapNodes->GetName() == _("graphics-location")) {
						GET_INT_PROPERTY_VALUE( bitmapNodes, "x", pattern.bitmapSize.graphics.x )
						GET_INT_PROPERTY_VALUE( bitmapNodes, "y", pattern.bitmapSize.graphics.y )
					}
					nextBitmap: bitmapNodes = bitmapNodes->GetNext();
				}
				goto nextNode;
			}
			if( subNodes->GetName() == _("HPGL")) {
				pattern.hasVector = true;
				pattern.HPGL = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("vector")) {
				ProcessVectorTag( subNodes, pattern.vectorSize );
			}
			nextNode: subNodes = subNodes->GetNext();
		}

		BuildPattern( pattern );

		patternNode = patternNode->GetNext();
	}
}


void ChartSymbols::BuildPattern( Pattern &pattern ) {
	Rule *pattmp = NULL;

	Rule *patt = (Rule*) calloc( 1, sizeof(Rule) );
	plib->pAlloc->Add( patt );

	patt->RCID = pattern.RCID;
	patt->exposition.PXPO = new wxString( pattern.description );
	strcpy( patt->name.PANM, pattern.name.mb_str() );
	patt->bitmap.PBTM = NULL;
	patt->fillType.PATP = pattern.fillType;
	patt->spacing.PASP = pattern.spacing;

	patt->vector.PVCT = (char *) calloc( pattern.HPGL.Len() + 1, 1 );
	strncpy( patt->vector.PVCT, pattern.HPGL.mb_str(), pattern.HPGL.Len() );

	patt->colRef.PCRF = (char *) calloc( pattern.colorRef.Len() + 1, 1 );
	strncpy( patt->colRef.PCRF, pattern.colorRef.mb_str(), pattern.colorRef.Len() );

	SymbolSizeInfo_t patternSize;

	if( pattern.hasVector && !(pattern.preferBitmap && pattern.hasBitmap) ) {
		patt->definition.PADF = 'V';
		patternSize = pattern.vectorSize;
	}
	else {
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

	wxRect graphicsLocation( pattern.bitmapSize.graphics, pattern.bitmapSize.size );
	(*symbolGraphicLocations)[pattern.name] = graphicsLocation;

	// check if key already there

	pattmp = ( *plib->_patt_sym )[pattern.name];

	if( NULL == pattmp ) {
		( *plib->_patt_sym )[pattern.name] = patt; // insert in hash table
	}
	else // already something here with same key...
	{ // if the pattern names are not identical
		if( patt->name.PANM != pattmp->name.PANM ) {
			( *plib->_patt_sym )[pattern.name] = patt; // replace the pattern
			plib->DestroyPatternRuleNode( pattmp ); // remember to free to replaced node
			// the node itself is destroyed as part of pAlloc
		}
	}
}




void ChartSymbols::ProcessSymbols( wxXmlNode* symbolNodes ) {
	wxXmlNode* symbolNode = symbolNodes->GetChildren();
	ChartSymbol symbol;
	wxString propVal;
	long numVal;

	while(symbolNode) {

		GET_INT_PROPERTY_VALUE( symbolNode, "RCID", symbol.RCID )

		symbol.hasVector = false;
		symbol.hasBitmap = false;
		symbol.preferBitmap = true;

		wxXmlNode* subNodes = symbolNode->GetChildren();

		while( subNodes ) {
			if( subNodes->GetName() == _("description")) {
				symbol.description = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("name")) {
				symbol.name = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("color-ref")) {
				symbol.colorRef = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("definition")) {
				if( subNodes->GetNodeContent() == _("V") ) symbol.hasVector = true;
				goto nextNode;
			}
			if( subNodes->GetName() == _("HPGL")) {
				symbol.HPGL = subNodes->GetNodeContent();
				goto nextNode;
			}
			if( subNodes->GetName() == _("prefer-bitmap")) {
				if( subNodes->GetNodeContent().Lower() == _("no") ) symbol.preferBitmap = false;
				if( subNodes->GetNodeContent().Lower() == _("false") ) symbol.preferBitmap = false;
				goto nextNode;
			}
			if( subNodes->GetName() == _("bitmap")) {
				GET_INT_PROPERTY_VALUE( subNodes, "width", symbol.bitmapSize.size.x )
				GET_INT_PROPERTY_VALUE( subNodes, "height", symbol.bitmapSize.size.y )
				symbol.hasBitmap = true;
				wxXmlNode* bitmapNodes = subNodes->GetChildren();
				while( bitmapNodes ) {
					if( bitmapNodes->GetName() == _("distance")) {
						GET_INT_PROPERTY_VALUE( bitmapNodes, "min", symbol.bitmapSize.minDistance )
						GET_INT_PROPERTY_VALUE( bitmapNodes, "max", symbol.bitmapSize.maxDistance )
						goto nextBitmap;
					}
					if( bitmapNodes->GetName() == _("origin")) {
						GET_INT_PROPERTY_VALUE( bitmapNodes, "x", symbol.bitmapSize.origin.x )
						GET_INT_PROPERTY_VALUE( bitmapNodes, "y", symbol.bitmapSize.origin.y )
						goto nextBitmap;
					}
					if( bitmapNodes->GetName() == _("pivot")) {
						GET_INT_PROPERTY_VALUE( bitmapNodes, "x", symbol.bitmapSize.pivot.x )
						GET_INT_PROPERTY_VALUE( bitmapNodes, "y", symbol.bitmapSize.pivot.y )
						goto nextBitmap;
					}
					if( bitmapNodes->GetName() == _("graphics-location")) {
						GET_INT_PROPERTY_VALUE( bitmapNodes, "x", symbol.bitmapSize.graphics.x )
						GET_INT_PROPERTY_VALUE( bitmapNodes, "y", symbol.bitmapSize.graphics.y )
					}
					nextBitmap: bitmapNodes = bitmapNodes->GetNext();
				}
				goto nextNode;
			}
			if( subNodes->GetName() == _("vector")) {
				GET_INT_PROPERTY_VALUE( subNodes, "width", symbol.vectorSize.size.x )
				GET_INT_PROPERTY_VALUE( subNodes, "height", symbol.vectorSize.size.y )
				symbol.hasVector = true;
				wxXmlNode* vectorNodes = subNodes->GetChildren();
				while( vectorNodes ) {
					if( vectorNodes->GetName() == _("distance")) {
						GET_INT_PROPERTY_VALUE( vectorNodes, "min", symbol.vectorSize.minDistance )
						GET_INT_PROPERTY_VALUE( vectorNodes, "max", symbol.vectorSize.maxDistance )
						goto nextVector;
					}
					if( vectorNodes->GetName() == _("origin")) {
						GET_INT_PROPERTY_VALUE( vectorNodes, "x", symbol.vectorSize.origin.x )
						GET_INT_PROPERTY_VALUE( vectorNodes, "y", symbol.vectorSize.origin.y )
						goto nextVector;
					}
					if( vectorNodes->GetName() == _("pivot")) {
						GET_INT_PROPERTY_VALUE( vectorNodes, "x", symbol.vectorSize.pivot.x )
						GET_INT_PROPERTY_VALUE( vectorNodes, "y", symbol.vectorSize.pivot.y )
						goto nextVector;
					}
					if( vectorNodes->GetName() == _("HPGL")) {
						symbol.HPGL = vectorNodes->GetNodeContent();
					}
					nextVector: vectorNodes = vectorNodes->GetNext();
				}
			}
			nextNode: subNodes = subNodes->GetNext();
		}

		BuildSymbol( symbol );

		symbolNode = symbolNode->GetNext();
	}
}


void ChartSymbols::BuildSymbol( ChartSymbol& symbol ) {

	Rule *symb = (Rule*) calloc( 1, sizeof(Rule) );
	plib->pAlloc->Add( symb );
//	Rule *symbtmp = NULL;

	wxString SVCT;
	wxString SCRF;

	symb->RCID = symbol.RCID;
	strncpy( symb->name.SYNM, symbol.name.char_str(), 8 );

	symb->exposition.SXPO = &(symbol.description);

    symb->vector.SVCT = ( char * ) calloc ( symbol.HPGL.Len()+1, 1 );
    strncpy ( symb->vector.SVCT, symbol.HPGL.mb_str(), symbol.HPGL.Len() );

    symb->colRef.SCRF = ( char * ) calloc ( symbol.colorRef.Len()+1, 1 );
    strncpy ( symb->colRef.SCRF, symbol.colorRef.mb_str(), symbol.colorRef.Len() );

	symb->bitmap.SBTM = NULL;

	SymbolSizeInfo_t symbolSize;

	if( symbol.hasVector && ! (symbol.preferBitmap && symbol.hasBitmap) ) {
		symb->definition.SYDF = 'V';
		symbolSize = symbol.vectorSize;
	}
	else {
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

	wxRect graphicsLocation( symbol.bitmapSize.graphics, symbol.bitmapSize.size );
	(*symbolGraphicLocations)[symbol.name] = graphicsLocation;

	(*plib->_symb_sym)[symbol.name] = symb;
}


bool ChartSymbols::LoadConfigFile( s52plib* plibArg, wxString s52ilePath ) {
	wxXmlDocument doc;

	plib = plibArg;

	// Expect to find library data XML file in same folder as other S52 data.
	// Files in CWD takes precedence.

	wxString name, extension;
	wxString xmlFileName = _("chartsymbols.xml");

	wxFileName::SplitPath( s52ilePath, &configFileDirectory, &name, &extension );
	wxString fullFilePath = configFileDirectory + wxFileName::GetPathSeparator() + xmlFileName;

	if( ! wxFileName::FileExists(fullFilePath) ) {
            wxString msg( _("ChartSymbols ConfigFile not found: "));
            msg += fullFilePath;
		wxLogMessage( msg );

            if( wxFileName::FileExists( xmlFileName ) ) {
                  fullFilePath = xmlFileName;
                  configFileDirectory = _T(".");
            }
            else
                  return false;
	}

	if( ! doc.Load( fullFilePath ) ) {
            wxString msg( _("    ChartSymbols ConfigFile Failed to load "));
            msg += fullFilePath;
            wxLogMessage( msg );
		return false;
	}

      wxString msg( _("ChartSymbols loaded from "));
      msg += fullFilePath;
	wxLogMessage( msg );

	if( doc.GetRoot()->GetName() != _("chartsymbols") ) {
		wxLogMessage( _("    ChartSymbols::LoadConfigFile(): Expected XML Root <chartsymbols> not found.") );
		return false;
	}

	wxXmlNode *child = doc.GetRoot()->GetChildren();
	while( child ) {
		if ( child->GetName() == _("color-tables") ) ProcessColorTables( child );
		if ( child->GetName() == _("lookups") ) ProcessLookups( child );
		if ( child->GetName() == _("line-styles") ) ProcessLinestyles( child );
		if ( child->GetName() == _("patterns") ) ProcessPatterns( child );
		if ( child->GetName() == _("symbols") ) ProcessSymbols( child );

		child = child->GetNext();
	}

	return true;
}

int ChartSymbols::LoadRasterFileForColorTable( int tableNo ) {

	if( tableNo == rasterSymbolsLoadedColorMapNumber ) return true;

	colTable* coltab = (colTable *) colorTables->Item( tableNo );

	wxString filename = configFileDirectory + wxFileName::GetPathSeparator() + coltab->rasterFileName;

	wxImage rasterFileImg;
	if( rasterFileImg.LoadFile( filename, wxBITMAP_TYPE_PNG ) ) {
		rasterSymbols = wxBitmap( rasterFileImg, -1/*32*/ );
		rasterSymbolsLoadedColorMapNumber = tableNo;
		return true;
	}

      wxString msg( _("ChartSymbols...Failed to load raster symbols file "));
      msg += filename;
	wxLogMessage( msg );
	return false;
}

// Convinience method for old s52plib code.
wxArrayPtrVoid* ChartSymbols::GetColorTables() {
	return colorTables;
}


S52color* ChartSymbols::GetColor( const char *colorName, int fromTable ) {
	colTable *colortable;
	wxString key( colorName, wxConvUTF8, 5 );
	colortable = (colTable *) colorTables->Item( fromTable );
	return &(colortable->colors[key]);
}


wxColor ChartSymbols::GetwxColor( const wxString &colorName, int fromTable ) {
	colTable *colortable;
	colortable = (colTable *) colorTables->Item( fromTable );
	wxColor c = colortable->wxColors[colorName];
	return c;
}


wxColor ChartSymbols::GetwxColor( const char *colorName, int fromTable ) {
	wxString key( colorName, wxConvUTF8, 5 );
	return GetwxColor( key, fromTable );
}


int ChartSymbols::FindColorTable( wxString& tableName ) {
	for( unsigned int i = 0; i < colorTables->GetCount(); i++ ) {
		colTable *ct = (colTable *) colorTables->Item( i );
		if( tableName.IsSameAs( *ct->tableName ) ) {
			return i;
		}
	}
	return 0;
}


wxString ChartSymbols::HashKey( const char* symbolName ) {
	char key[9];
    key[8]=0;
    strncpy ( key, symbolName, 8 );
    return wxString( key,wxConvUTF8 );
}


wxImage ChartSymbols::GetImage( const char* symbolName ) {
	wxRect bmArea = (*symbolGraphicLocations)[HashKey(symbolName)];
	wxBitmap bitmap = rasterSymbols.GetSubBitmap( bmArea );
	return bitmap.ConvertToImage();
}
