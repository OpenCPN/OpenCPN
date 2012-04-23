#pragma once

#include "s52plib.h"
#include <wx/xml/xml.h>


class Lookup {
public:
	int	RCID;
	int id;
	wxString name;
	Object_t       type;             // 'A' Area, 'L' Line, 'P' Point
	DisPrio        displayPrio;             // Display Priority
	RadPrio        radarPrio;             // 'O' or 'S', Radar Priority
	LUPname        tableName;             // FTYP:  areas, points, lines
	wxArrayString *attributeCodeArray;        // ArrayString of LUP Attributes
	wxString       instruction;            // Instruction Field (rules)
	DisCat         displayCat;             // Display Categorie: D/S/O, DisplayBase, Standard, Other
	int            comment;             // Look-Up Comment (PLib3.x put 'groupes' here,
};


typedef struct _SymbolSizeInfo {
	wxSize size;
	wxPoint origin;
	wxPoint pivot;
	wxPoint graphics;
	int minDistance;
	int maxDistance;
} SymbolSizeInfo_t;


class Pattern {
public:
	int	RCID;
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
	int	RCID;
	wxString name;
	wxString description;
	wxString colorRef;
	SymbolSizeInfo_t vectorSize;
	wxBitmap bitmap;
	wxString HPGL;
};


class ChartSymbol
{
public:
	wxString name;
	int	RCID;
	bool hasVector;
	bool hasBitmap;
	bool preferBitmap;
	wxString description;
	wxString colorRef;
	SymbolSizeInfo_t bitmapSize;
	SymbolSizeInfo_t vectorSize;
	wxString HPGL;
};


class ChartSymbols
{
public:
	ChartSymbols(void);
	~ChartSymbols(void);
	bool LoadConfigFile( s52plib* plibArg, wxString path );

	static int LoadRasterFileForColorTable( int tableNo );
	static wxArrayPtrVoid * GetColorTables();
	static int FindColorTable( wxString& tableName );
	static S52color* GetColor( const char *colorName, int fromTable );
	static wxColor GetwxColor( const wxString &colorName, int fromTable );
	static wxColor GetwxColor( const char *colorName, int fromTable );
	static wxString HashKey( const char* symbolName );
	static wxImage GetImage( const char* symbolName );


private:
	void ProcessVectorTag( wxXmlNode* subNodes, SymbolSizeInfo_t &vectorSize );
	void ProcessColorTables( wxXmlNode* colortableodes );
	void ProcessLookups( wxXmlNode* lookupNodes );
	void ProcessLinestyles( wxXmlNode* linestyleNodes );
	void ProcessPatterns( wxXmlNode* patternNodes );
	void ProcessSymbols( wxXmlNode* symbolNodes );
	void BuildLineStyle( LineStyle &lineStyle );
	void BuildLookup( Lookup &lookup );
	void BuildPattern( Pattern &pattern );
	void BuildSymbol( ChartSymbol &symol );

	s52plib* plib;
};

