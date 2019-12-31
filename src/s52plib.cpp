/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S52 Presentation Library
 * Authors:   David Register
 *            Jesper Weissglas
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

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <math.h>
#include <stdlib.h>

#include "config.h"

#include "georef.h"
#include "viewport.h"

#include "s52plib.h"
#include "mygeom.h"
#include "cutil.h"
#include "s52utils.h"
#include "chartsymbols.h"
#include "TexFont.h"
#include "ocpn_plugin.h"
#include "gdal/cpl_csv.h"

#include "s57chart.h"
#include <wx/image.h>
#include <wx/tokenzr.h>
#include <wx/fileconf.h>

#ifndef PROJECTION_MERCATOR
    #define PROJECTION_MERCATOR 1
#endif


#ifdef USE_ANDROID_GLES2
#include "linmath.h"
#endif

#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#endif

extern float g_GLMinCartographicLineWidth;
extern float g_GLMinSymbolLineWidth;
extern double  g_overzoom_emphasis_base;
extern bool    g_oz_vector_scale;
extern float g_ChartScaleFactorExp;
extern int g_chart_zoom_modifier_vector;

float g_scaminScale;

extern PFNGLGENBUFFERSPROC                 s_glGenBuffers;
extern PFNGLBINDBUFFERPROC                 s_glBindBuffer;
extern PFNGLBUFFERDATAPROC                 s_glBufferData;
extern PFNGLDELETEBUFFERSPROC              s_glDeleteBuffers;

#ifndef USE_ANDROID_GLES2
#define glGenBuffers(a,b) (s_glGenBuffers)(a,b);
#define glBindBuffer(a,b) (s_glBindBuffer)(a,b);
#define glBufferData(a,b,c,d) (s_glBufferData)(a,b,c,d);
#define glDeleteBuffers(a,b) (s_glDeleteBuffers)(a,b);
#endif

void DrawAALine( wxDC *pDC, int x0, int y0, int x1, int y1, wxColour clrLine, int dash, int space );
extern bool GetDoubleAttr( S57Obj *obj, const char *AttrName, double &val );
void PLIBDrawGLThickLine( float x1, float y1, float x2, float y2, wxPen pen, bool b_hiqual );

void LoadS57Config();
bool loadS52Shaders();

#ifdef ocpnUSE_GL
typedef struct {
    TexFont cache;
    wxFont  *key;
} TexFontCache;

#define TXF_CACHE 8
static TexFontCache s_txf[TXF_CACHE];
#endif

#ifdef USE_ANDROID_GLES2

GLint S52color_tri_shader_program;
GLint S52texture_2D_shader_program;
GLint S52circle_filled_shader_program;
GLint S52ring_shader_program;
GLint S52Dash_shader_program;
GLint S52AP_shader_program;

#endif

//    Implement all lists
#include <wx/listimpl.cpp>
WX_DEFINE_LIST( TextObjList );


//    Implement all arrays
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfNoshow);

//  S52_TextC Implementation
S52_TextC::S52_TextC()
{ 
    pcol = NULL;
    pFont = NULL;
    texobj = 0;
    bnat = false;
    bspecial_char = false;
}

S52_TextC::~S52_TextC()
{
    if(texobj){
        glDeleteTextures(1, (GLuint *)(&this->texobj) );
    }
}

//      In GDAL-1.2.0, CSVGetField is not exported.......
//      So, make my own simplified copy
/************************************************************************/
/*                           MyCSVGetField()                            */
/*                                                                      */
/************************************************************************/

const char *MyPLIBCSVGetField( const char * pszFilename, const char * pszKeyFieldName,
                           const char * pszKeyFieldValue, CSVCompareCriteria eCriteria, const char * pszTargetField )

{
    char **papszRecord;
    int iTargetField;
    
    /* -------------------------------------------------------------------- */
    /*      Find the correct record.                                        */
    /* -------------------------------------------------------------------- */
    papszRecord = CSVScanFileByName( pszFilename, pszKeyFieldName, pszKeyFieldValue, eCriteria );
    
    if( papszRecord == NULL ) return "";
    
    /* -------------------------------------------------------------------- */
    /*      Figure out which field we want out of this.                     */
    /* -------------------------------------------------------------------- */
    iTargetField = CSVGetFileFieldId( pszFilename, pszTargetField );
    if( iTargetField < 0 ) return "";
    
    if( iTargetField >= CSLCount( papszRecord ) ) return "";
    
    return ( papszRecord[iTargetField] );
}

wxString GetS57AttributeDecode( wxString& att, int ival )
{
    wxString ret_val = _T("");
    
    wxString s57data_dir = *GetpSharedDataLocation();
    s57data_dir += _T("s57data");
    
    if( !s57data_dir.Len() )
        return ret_val;

//  Get the attribute code from the acronym
    const char *att_code;
    
    wxString file = s57data_dir;
    file.Append( _T("/s57attributes.csv") );
    
    if( !wxFileName::FileExists( file ) ) {
        wxString msg( _T("   Could not open ") );
        msg.Append( file );
        wxLogMessage( msg );
        
        return ret_val;
    }
    
    att_code = MyPLIBCSVGetField( file.mb_str(), "Acronym",                  // match field
                              att.mb_str(),               // match value
                              CC_ExactString, "Code" );             // return field
    
    // Now, get a nice description from s57expectedinput.csv
    //  This will have to be a 2-d search, using ID field and Code field
    
    // Ingest, and get a pointer to the ingested table for "Expected Input" file
    wxString ei_file = s57data_dir;
    ei_file.Append( _T("/s57expectedinput.csv") );
    
    if( !wxFileName::FileExists( ei_file ) ) {
        wxString msg( _T("   Could not open ") );
        msg.Append( ei_file );
        wxLogMessage( msg );
        
        return ret_val;
    }
    
    CSVTable *psTable = CSVAccess( ei_file.mb_str() );
    CSVIngest( ei_file.mb_str() );
    
    char **papszFields = NULL;
    int bSelected = FALSE;
    
    /* -------------------------------------------------------------------- */
    /*      Scan from in-core lines.                                        */
    /* -------------------------------------------------------------------- */
    int iline = 0;
    while( !bSelected && iline + 1 < psTable->nLineCount ) {
        iline++;
        papszFields = CSVSplitLine( psTable->papszLines[iline] );
        
        if( !strcmp( papszFields[0], att_code ) ) {
            if( atoi( papszFields[1] ) == ival ) {
                ret_val = wxString( papszFields[2], wxConvUTF8 );
                bSelected = TRUE;
            }
        }
        
        CSLDestroy( papszFields );
    }
    
    return ret_val;
    
}


//-----------------------------------------------------------------------------
//      Comparison Function for LUPArray sorting
//      Note Global Scope
//-----------------------------------------------------------------------------
#ifndef _COMPARE_LUP_DEFN_
#define _COMPARE_LUP_DEFN_

int CompareLUPObjects( LUPrec *item1, LUPrec *item2 )
{
    // sort the items by their name...
    int ir = strcmp( item1->OBCL, item2->OBCL );
    
    if( ir != 0 )
        return ir;
    int c1 = item1->ATTArray.size();
    int c2 = item2->ATTArray.size();
    
    if( c1 != c2 )
        return c2 - c1;
    return item1->nSequence - item2->nSequence;
}

#endif














//-----------------------------------------------------------------------------
//      LUPArrayContainer implementation
//-----------------------------------------------------------------------------
LUPArrayContainer::LUPArrayContainer()
{
    //   Build the initially empty sorted arrays of LUP Records, per LUP type.
    //   Sorted on object name, e.g. ACHARE.  Why sorted?  Helps in the S52_LUPLookup method....
    LUPArray = new wxArrayOfLUPrec( CompareLUPObjects );
}

LUPArrayContainer::~LUPArrayContainer()
{
    if( LUPArray ) {
        for( unsigned int il = 0; il < LUPArray->GetCount(); il++ )
            s52plib::DestroyLUP( LUPArray->Item( il ) );
        
        LUPArray->Clear();
        delete LUPArray;
    }
    
    LUPArrayIndexHash::iterator it;
    for( it = IndexHash.begin(); it != IndexHash.end(); ++it ){
        free( it->second );
    }
}

LUPHashIndex *LUPArrayContainer::GetArrayIndexHelper( const char *objectName )
{
    // Look for the key
    wxString key(objectName, wxConvUTF8);
    LUPArrayIndexHash::iterator it = IndexHash.find( key );
    
    if( it == IndexHash.end() ){             
        //      Key not found, needs to be added
        LUPHashIndex *pindex = (LUPHashIndex *)malloc(sizeof(LUPHashIndex));
        pindex->n_start = -1;
        pindex->count = 0;
        IndexHash[key] = pindex;
        
        //      Find the first matching entry in the LUP Array
        int index = 0;
        int index_max = LUPArray->GetCount();
        int first_match = 0;
        int ocnt = 0;
        LUPrec *LUPCandidate;
        
        //        This technique of extracting proper LUPs depends on the fact that
        //        the LUPs have been sorted in their array, by OBCL.
        //        Thus, all the LUPS with the same OBCL will be grouped together
        
        while( !first_match && ( index < index_max ) ) {
            LUPCandidate = LUPArray->Item( index );
            if( !strcmp( objectName, LUPCandidate->OBCL ) ) {
                pindex->n_start = index;
                first_match = 1;
                ocnt++;
                index++;
                break;
            }
            index++;
        }
        
        while( first_match && ( index < index_max ) ) {
            LUPCandidate = LUPArray->Item( index );
            if( !strcmp( objectName, LUPCandidate->OBCL ) ) {
                ocnt++;
            } else {
                break;
            }
            
            index++;
        }
        
        pindex->count = ocnt;
        
        return pindex;
    }
    else
        return it->second;              // return a pointer to the found record
        
    
}


//-----------------------------------------------------------------------------
//      s52plib implementation
//-----------------------------------------------------------------------------
s52plib::s52plib( const wxString& PLib, bool b_forceLegacy )
{
    m_plib_file = PLib;

    pOBJLArray = new wxArrayPtrVoid;

    condSymbolLUPArray = NULL; // Dynamic Conditional Symbology

    _symb_sym = NULL;

    m_txf_ready = false;
    m_txf = NULL;

    ChartSymbols::InitializeGlobals();

    m_bOK = !( S52_load_Plib( PLib, b_forceLegacy ) == 0 );

    m_bShowS57Text = false;
    m_bShowS57ImportantTextOnly = false;
    m_colortable_index = 0;

    _symb_symR = NULL;
    bUseRasterSym = false;

    //      Sensible defaults
    m_nSymbolStyle = PAPER_CHART;
    m_nBoundaryStyle = PLAIN_BOUNDARIES;
    m_nDisplayCategory = OTHER;
    m_nDepthUnitDisplay = 1; // metres
    
    UpdateMarinerParams();

    ledge = new int[2000];
    redge = new int[2000];

    //    Defaults
    m_VersionMajor = 3;
    m_VersionMinor = 2;

    canvas_pix_per_mm = 3.;
    m_rv_scale_factor = 1.0;
    
    //        Set up some default flags
    m_bDeClutterText = false;
    m_bShowAtonText = true;
    m_bShowNationalTexts = false;

    m_bShowSoundg = true;
    m_bShowLdisText = true;
    m_bExtendLightSectors = true;

    // Set a few initial states
    AddObjNoshow( "M_QUAL" );
    m_lightsOff = false;
    m_anchorOn = true;
    m_qualityOfDataOn = false;

    GenerateStateHash();

    HPGL = new RenderFromHPGL( this );
    
#ifdef USE_ANDROID_GLES2
    loadS52Shaders();
#endif
    //workBuf = NULL;;
    //workBufSize = 0;;

    //  Set defaults for OCPN version, may be overridden later
    m_coreVersionMajor = 4;
    m_coreVersionMinor = 6;
    m_coreVersionPatch = 0;
    
    m_myConfig = PI_GetPLIBStateHash();

    // GL Options/capabilities
    m_useStencil = false;
    m_useStencilAP = false;
    m_useScissors = false;
    m_useFBO = false;
    m_useVBO = false;
    m_TextureFormat = -1;

    m_display_size_mm = 300;
    SetGLPolygonSmoothing( true );
    SetGLLineSmoothing( true );
    
}

s52plib::~s52plib()
{
    delete areaPlain_LAC;
    delete line_LAC ;
    delete areaSymbol_LAC;
    delete pointSimple_LAC;
    delete pointPaper_LAC;
    
    S52_flush_Plib();

   
//      Free the OBJL Array Elements
    for( unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++ )
        free( pOBJLArray->Item( iPtr ) );

    delete pOBJLArray;

    delete[] ledge;
    delete[] redge;

    ChartSymbols::DeleteGlobals();

    delete HPGL;
}

void s52plib::SetOCPNVersion(int major, int minor, int patch)
{
    m_coreVersionMajor = major;
    m_coreVersionMinor = minor;
    m_coreVersionPatch = patch;
}

void s52plib::SetGLOptions(bool b_useStencil,
                  bool b_useStencilAP,
                  bool b_useScissors,
                  bool b_useFBO,
                  bool b_useVBO,
                  int  nTextureFormat)
{
    // Set GL Options/capabilities
    m_useStencil = b_useStencil;
    m_useStencilAP = b_useStencilAP;
    m_useScissors = b_useScissors;
    m_useFBO = b_useFBO;
    m_useVBO = b_useVBO;
    m_TextureFormat = nTextureFormat;
}

void s52plib::SetPPMM( float ppmm )
{ 
    canvas_pix_per_mm = ppmm;
    
    // We need a supplemental scale factor for HPGL vector symbol rendering.
    //  This will cause raster and vector symbols to be rendered harmoniously
    
    //  We do this by making an arbitrary measurement and declaration:
    // We declare that the nominal size of a "flare" light rendered as HPGL vector should be roughly twice the
    // size of a simplified lateral bouy rendered as raster.
    
    // Referring to the chartsymbols.xml file, we find that the dimension of a flare light is 810 units, 
    // and a raster BOYLAT is 16 pix.
    
    m_rv_scale_factor = 2.0 * (1600. / (810 * ppmm));
    
    // Estimate the display size
    
    int ww, hh;
    ::wxDisplaySize( &ww, &hh);
    m_display_size_mm = wxMax(ww, hh) / GetPPMM();        // accurate enough for internal use
    

}    

//      Various static helper methods

void s52plib::DestroyLUP( LUPrec *pLUP )
{
    Rules *top = pLUP->ruleList;
    DestroyRulesChain(top);
    
    for(unsigned int i = 0 ; i < pLUP->ATTArray.size() ; i++)
        free (pLUP->ATTArray[i]);
    
    delete pLUP->INST;
}

void s52plib::DestroyRulesChain( Rules *top )
{
    while( top != NULL ) {
        Rules *Rtmp = top->next;
        
        free( top->INST0 ); // free the Instruction string head

        if( top->b_private_razRule ) // need to free razRule?
        {
            Rule *pR = top->razRule;
            delete pR->exposition.LXPO;
            
            free( pR->vector.LVCT );
            
            delete pR->bitmap.SBTM;
            
            free( pR->colRef.SCRF );
            
            ClearRulesCache( pR );
            
            free( pR );
        }
        
        free( top );
        top = Rtmp;
    }
    
}



DisCat s52plib::findLUPDisCat(const char *objectName, LUPname TNAM)
{
    LUPArrayContainer *plac = SelectLUPArrayContainer( TNAM );
    
    wxArrayOfLUPrec *LUPArray = SelectLUPARRAY( TNAM  );
    
    
    //      Find the first matching entry in the LUP Array
    int index = 0;
    int index_max = LUPArray->GetCount();
    LUPrec *LUPCandidate;
    
    
    while(  index < index_max  ) {
        LUPCandidate = LUPArray->Item( index );
        if( !strcmp( objectName, LUPCandidate->OBCL ) ) {
            return LUPCandidate->DISC;
        }
        index++;
    }
    
    return (DisCat)(-1);
}
    





bool s52plib::GetAnchorOn()
{
    //  Investigate and report the logical condition that "Anchoring Condition" is shown
    
    int old_vis =  0;
    OBJLElement *pOLE = NULL;
        
    if(  MARINERS_STANDARD == GetDisplayCategory()){
        old_vis = m_anchorOn;
    }
    else if(OTHER == GetDisplayCategory())
        old_vis = true;

    //other cat  
    //const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL", "SBDARE" };

    old_vis &= !IsObjNoshow("SBDARE");

    return (old_vis != 0);
}

bool s52plib::GetQualityOfData()
{
    //  Investigate and report the logical condition that "Quality of Data Condition" is shown
    
    int old_vis =  0;
    OBJLElement *pOLE = NULL;
        
    if(  MARINERS_STANDARD == GetDisplayCategory()){
            for( unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++ ) {
                OBJLElement *pOLE = (OBJLElement *) ( pOBJLArray->Item( iPtr ) );
                if( !strncmp( pOLE->OBJLName, "M_QUAL", 6 ) ) {
                    old_vis = pOLE->nViz;
                    break;
                }
            }
    }
    else if(OTHER == GetDisplayCategory())
        old_vis = true;

    old_vis &= !IsObjNoshow("M_QUAL");

    return (old_vis != 0);
}
        

void s52plib::SetGLRendererString(const wxString &renderer)
{
}

/*
 Update the S52 Conditional Symbology Parameter Set to reflect the
 current state of the library member options.
 */

void s52plib::UpdateMarinerParams( void )
{

    //      Symbol Style
    if( SIMPLIFIED == m_nSymbolStyle ) S52_setMarinerParam( S52_MAR_SYMPLIFIED_PNT, 1.0 );
    else
        S52_setMarinerParam( S52_MAR_SYMPLIFIED_PNT, 0.0 );

    //      Boundary Style
    if( SYMBOLIZED_BOUNDARIES == m_nBoundaryStyle ) S52_setMarinerParam( S52_MAR_SYMBOLIZED_BND,
            1.0 );
    else
        S52_setMarinerParam( S52_MAR_SYMBOLIZED_BND, 0.0 );

}

void s52plib::GenerateStateHash()
{
    unsigned char state_buffer[512];  // Needs to be at least this big...
    memset(state_buffer, 0, sizeof(state_buffer));
    
    int time = ::wxGetUTCTime();
    memcpy(state_buffer, &time, sizeof(int));
    
    size_t offset = sizeof(int);           // skipping the time int, first element
    
    for(int i=0 ; i < S52_MAR_NUM ; i++){
        if( (offset + sizeof(double)) < sizeof(state_buffer)){
            double t = S52_getMarinerParam((S52_MAR_param_t) i);
            memcpy( &state_buffer[offset], &t, sizeof(double));
	    offset += sizeof(double);
        }
    }
    
    for(unsigned int i=0 ; i < m_noshow_array.GetCount() ; i++){
        if( (offset + 6) < sizeof(state_buffer)){
            memcpy(&state_buffer[offset], m_noshow_array[i].obj, 6) ;
            offset += 6;
        }
    }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
        { memcpy(&state_buffer[offset], &m_bShowSoundg, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
        { memcpy(&state_buffer[offset], &m_bShowS57Text, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
        { memcpy(&state_buffer[offset], &m_bShowS57ImportantTextOnly, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
        { memcpy(&state_buffer[offset], &m_bDeClutterText, sizeof(bool)); offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
        { memcpy(&state_buffer[offset], &m_bShowNationalTexts, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
        { memcpy(&state_buffer[offset], &m_bShowAtonText, sizeof(bool));  offset += sizeof(bool); }

    if(offset + sizeof(bool) < sizeof(state_buffer))
        { memcpy(&state_buffer[offset], &m_bShowLdisText, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
        { memcpy(&state_buffer[offset], &m_bExtendLightSectors, sizeof(bool));  offset += sizeof(bool); }

    m_state_hash = crc32buf(state_buffer, offset );
    
}

wxArrayOfLUPrec* s52plib::SelectLUPARRAY( LUPname TNAM  )
{
    switch( TNAM ){
        case SIMPLIFIED:
            return pointSimple_LAC->GetLUPArray();
        case PAPER_CHART:
            return pointPaper_LAC->GetLUPArray();
        case LINES:
            return line_LAC->GetLUPArray();
        case PLAIN_BOUNDARIES:
            return areaPlain_LAC->GetLUPArray();
        case SYMBOLIZED_BOUNDARIES:
            return areaSymbol_LAC->GetLUPArray();
        default:
            return NULL;
    }
}

LUPArrayContainer *s52plib::SelectLUPArrayContainer( LUPname TNAM )
{
    switch( TNAM ){
        case SIMPLIFIED:
            return pointSimple_LAC;
        case PAPER_CHART:
            return pointPaper_LAC;
        case LINES:
            return line_LAC;
        case PLAIN_BOUNDARIES:
            return areaPlain_LAC;
        case SYMBOLIZED_BOUNDARIES:
            return areaSymbol_LAC;
        default:
            return NULL;
    }
}


extern Cond condTable[];

LUPrec *s52plib::FindBestLUP( wxArrayOfLUPrec *LUPArray, unsigned int startIndex, unsigned int count, S57Obj *pObj, bool bStrict )
{
    //  Check the parameters
    if( 0 == count )
        return NULL;
    if( startIndex >= LUPArray->GetCount() )
        return NULL;
    
    // setup default return to the first LUP that matches Feature name.
    LUPrec *LUP = LUPArray->Item( startIndex );

    int nATTMatch = 0;
    int countATT = 0;
    bool bmatch_found = false;

    if( pObj->att_array == NULL )
        goto check_LUP;       // object has no attributes to compare, so return "best" LUP
        
    for( unsigned int i = 0; i < count; ++i ) {
        LUPrec *LUPCandidate = LUPArray->Item( startIndex + i );
        
        if( !LUPCandidate->ATTArray.size() )
            continue;        // this LUP has no attributes coded

        countATT = 0;
        char *currATT = pObj->att_array;
        int attIdx = 0;

        for( unsigned int iLUPAtt = 0; iLUPAtt < LUPCandidate->ATTArray.size(); iLUPAtt++ ) {

            // Get the LUP attribute name
            char *slatc = LUPCandidate->ATTArray[iLUPAtt];

            if( slatc && (strlen(slatc) < 6) )
                goto next_LUP_Attr;         // LUP attribute value not UTF8 convertible (never seen in PLIB 3.x)

            if( slatc ){
                char *slatv = slatc + 6;
                while( attIdx < pObj->n_attr ) {
                    if( 0 == strncmp( slatc, currATT, 6 ) ) {
                        //OK we have an attribute name match
                        
                        
                        bool attValMatch = false;
                        
                        // special case (i)
                        if( !strncmp( slatv, " ", 1 ) ) {        // any object value will match wild card (S52 para 8.3.3.4)
                            ++countATT;
                            goto next_LUP_Attr;
                        }
                        
                        // special case (ii)
                        //TODO  Find an ENC with "UNKNOWN" DRVAL1 or DRVAL2 and debug this code
                        if( !strncmp( slatv, "?", 1) ){          // if LUP attribute value is "undefined"

                        //  Match if the object does NOT contain this attribute
                            goto next_LUP_Attr;
                        }
                        
                        
                        //checking against object attribute value
                        S57attVal *v = ( pObj->attVal->Item( attIdx ) );
                        
                        switch( v->valType ){
                            case OGR_INT: // S57 attribute type 'E' enumerated, 'I' integer
                            {
                                int LUP_att_val = atoi( slatv );
                                if( LUP_att_val == *(int*) ( v->value ) )
                                    attValMatch = true;
                                break;
                            }
                            
                            case OGR_INT_LST: // S57 attribute type 'L' list: comma separated integer
                            {
                                int a;
                                char ss[41];
                                strncpy( ss, slatv, 39 );
                                ss[40] = '\0';
                                char *s = &ss[0];
                                
                                int *b = (int*) v->value;
                                sscanf( s, "%d", &a );
                                
                                while( *s != '\0' ) {
                                    if( a == *b ) {
                                        sscanf( ++s, "%d", &a );
                                        b++;
                                        attValMatch = true;
                                        
                                    } else
                                        attValMatch = false;
                                }
                                break;
                            }
                            case OGR_REAL: // S57 attribute type'F' float
                            {
                                double obj_val = *(double*) ( v->value );
                                float att_val = atof( slatv );
                                if( fabs( obj_val - att_val ) < 1e-6 )
                                    if( obj_val == att_val  )
                                        attValMatch = true;
                                break;
                            }
                            
                            case OGR_STR: // S57 attribute type'A' code string, 'S' free text
                            {
                                //    Strings must be exact match
                                //    n.b. OGR_STR is used for S-57 attribute type 'L', comma-separated list
                                
                                //wxString cs( (char *) v->value, wxConvUTF8 ); // Attribute from object
                                //if( LATTC.Mid( 6 ) == cs )
                                if( !strcmp((char *) v->value, slatv))
                                    attValMatch = true;
                                break;
                            }
                            
                            default:
                                break;
                        } //switch
                        
                        // value match
                        if( attValMatch )
                            ++countATT;
                                                
                        goto next_LUP_Attr;
                    } // if attribute name match
                    
                    //  Advance to the next S57obj attribute
                    currATT += 6;
                    ++attIdx;
                    
                } //while
            } //if
            
next_LUP_Attr:
         
            currATT = pObj->att_array; // restart the object attribute list
            attIdx = 0;
        } // for iLUPAtt
        
        //      Create a "match score", defined as fraction of candidate LUP attributes
        //      actually matched by feature.
        //      Used later for resolving "ties"
        
        int nattr_matching_on_candidate = countATT;
        int nattrs_on_candidate = LUPCandidate->ATTArray.size();
        double candidate_score = ( 1. * nattr_matching_on_candidate )
        / ( 1. * nattrs_on_candidate );
        
        //       According to S52 specs, match must be perfect,
        //         and the first 100% match is selected
        if( candidate_score == 1.0 ) {
            LUP = LUPCandidate;
            bmatch_found = true;
            break; // selects the first 100% match
        }
        
    } //for loop
    

check_LUP:
//  In strict mode, we require at least one attribute to match exactly
    
    if( bStrict ) {
        if( nATTMatch == 0 ) // nothing matched
            LUP = NULL;
    } else {
        //      If no match found, return the first LUP in the list which has no attributes
        if( !bmatch_found ) {
            for( unsigned int j = 0; j < count; ++j ) {
                LUPrec *LUPtmp = NULL;
                
                LUPtmp = LUPArray->Item( startIndex + j );
                if( !LUPtmp->ATTArray.size() ) {
                    return LUPtmp;
                }
            }
        }
    }
    
    return LUP;
}




// scan foward stop on ; or end-of-record
#define SCANFWRD        while( !(*str == ';' || *str == '\037')) ++str;

#define INSTRUCTION(s,t)        if(0==strncmp(s,str,2)){\
                              str+=3;\
                              r->ruleType = t;\
                              r->INSTstr  = str;

Rules *s52plib::StringToRules( const wxString& str_in )
{
    wxCharBuffer buffer=str_in.ToUTF8();
    if(!buffer.data())
        return NULL;

    size_t len = strlen( buffer.data() );
    char *str0 = (char *) calloc( len + 1, 1 );
    memcpy( str0, buffer.data(), len );
    char *str = str0;

    Rules *top;
    Rules *last;
    char strk[20];

    //    Allocate and pre-clear the Rules structure
    Rules *r = (Rules*) calloc( 1, sizeof(Rules) );
    top = r;
    last = top;

    r->INST0 = str0; // save the head for later free

    while( *str != '\0' ) {
        if( r->ruleType ) // in the loop, r has been used
        {
            r = (Rules*) calloc( 1, sizeof(Rules) );
            last->next = r;
            last = r;
        }

        // parse Symbology instruction in string

        // Special Case for Circular Arc,  (opencpn private)
        // Allocate a Rule structure to be used to hold a cached bitmap of the created symbol
        INSTRUCTION ( "CA",RUL_ARC_2C )
            r->razRule = (Rule*) calloc( 1, sizeof(Rule) );
            r->b_private_razRule = true; // mark this raxRule to be free'd later
            SCANFWRD
        }

        // Special Case for MultPoint Soundings
        INSTRUCTION ( "MP",RUL_MUL_SG )
            SCANFWRD
        }

// SHOWTEXT
        INSTRUCTION ( "TX",RUL_TXT_TX )
            SCANFWRD
        }

        INSTRUCTION ( "TE",RUL_TXT_TE )
            SCANFWRD
        }

// SHOWPOINT

        if( 0 == strncmp( "SY", str, 2 ) ) {
            str += 3;
            r->ruleType = RUL_SYM_PT;
            r->INSTstr = str;

            strncpy( strk, str, 8 );
            strk[8] = 0;
            wxString key( strk, wxConvUTF8 );

            r->razRule = ( *_symb_sym )[key];

            if( r->razRule == NULL ) r->razRule = ( *_symb_sym )[_T ( "QUESMRK1" )];

            SCANFWRD
        }

// SHOWLINE
        INSTRUCTION ( "LS",RUL_SIM_LN )
            SCANFWRD
        }

        INSTRUCTION ( "LC",RUL_COM_LN )
            strncpy( strk, str, 8 );
            strk[8] = 0;
            wxString key( strk, wxConvUTF8 );

            r->razRule = ( *_line_sym )[key];

            if( r->razRule == NULL ) r->razRule = ( *_symb_sym )[_T ( "QUESMRK1" )];
            SCANFWRD
        }

        // SHOWAREA
        INSTRUCTION ( "AC",RUL_ARE_CO )
            SCANFWRD
        }

        INSTRUCTION ( "AP",RUL_ARE_PA )
            strncpy( strk, str, 8 );
            strk[8] = 0;
            wxString key( strk, wxConvUTF8 );

            r->razRule = ( *_patt_sym )[key];
            if( r->razRule == NULL ) r->razRule = ( *_patt_sym )[_T ( "QUESMRK1V" )];
            SCANFWRD
        }

        // CALLSYMPROC

        if( 0 == strncmp( "CS", str, 2 ) ) {
            str += 3;
            r->ruleType = RUL_CND_SY;
            r->INSTstr = str;

//      INSTRUCTION("CS",RUL_CND_SY)
            char stt[9];
            strncpy( stt, str, 8 );
            stt[8] = 0;
            wxString index( stt, wxConvUTF8 );
            r->razRule = ( *_cond_sym )[index];
            if( r->razRule == NULL ) r->razRule = ( *_cond_sym )[_T ( "QUESMRK1" )];
            SCANFWRD
        }

        ++str;
    }

    //  If it should happen that no rule is built, delete the initially allocated rule
    if( 0 == top->ruleType ) {
        if( top->INST0 ) free( top->INST0 );

        free( top );

        top = NULL;
    }

    //   Traverse the entire rule set tree, pruning after first unallocated (dead) rule
    r = top;
    while( r ) {
        if( 0 == r->ruleType ) {
            free( r );
            last->next = NULL;
            break;
        }

        last = r;
        Rules *n = r->next;
        r = n;
    }

    //   Traverse the entire rule set tree, adding sequence numbers
    r = top;
    int i = 0;
    while( r ) {
        r->n_sequence = i++;

        r = r->next;
    }

    return top;
}

int s52plib::_LUP2rules( LUPrec *LUP, S57Obj *pObj )
{
    if( NULL == LUP ) return -1;
    // check if already parsed
    if( LUP->ruleList != NULL ) {
        return 0;
    }

    if( LUP->INST != NULL ) {
        Rules *top = StringToRules( *LUP->INST );
        LUP->ruleList = top;

        return 1;
    } else
        return 0;
}


int s52plib::S52_load_Plib( const wxString& PLib, bool b_forceLegacy )
{

    pAlloc = new wxArrayPtrVoid;

    //   Create the Rule Lookup Hash Tables
    _line_sym = new RuleHash; // line
    _patt_sym = new RuleHash; // pattern
    _symb_sym = new RuleHash; // symbol
    _cond_sym = new RuleHash; // conditional

    line_LAC = new LUPArrayContainer;
    areaPlain_LAC= new LUPArrayContainer;
    areaSymbol_LAC= new LUPArrayContainer;
    pointSimple_LAC= new LUPArrayContainer;
    pointPaper_LAC= new LUPArrayContainer;
    
    condSymbolLUPArray = new wxArrayOfLUPrec( CompareLUPObjects ); // dynamic Cond Sym LUPs

    m_unused_color.R = 2;
    m_unused_color.G = 2;
    m_unused_color.B = 2;
    m_unused_wxColor.Set( 2,2,2 );

#if 0
    // First, honor the user attempt for force Lecagy mode.
    // Next, try to load symbols using the newer XML/PNG format.
    // If this fails, try Legacy S52RAZDS.RLE file.

    if( b_forceLegacy ) {
        RazdsParser parser;
        useLegacyRaster = true;
        if( parser.LoadFile( this, PLib ) ) {
            wxString msg( _T("Loaded legacy PLIB data: ") );
            msg += PLib;
            wxLogMessage( msg );
        } else
            return 0;
    } else {
        ChartSymbols chartSymbols;
        useLegacyRaster = false;
        if( !chartSymbols.LoadConfigFile( this, PLib ) ) {
            RazdsParser parser;
            useLegacyRaster = true;
            if( parser.LoadFile( this, PLib ) ) {
                wxString msg( _T("Loaded legacy PLIB data: ") );
                msg += PLib;
                wxLogMessage( msg );
            } else
                return 0;
        }
    }
#endif
    ChartSymbols chartSymbols;
    useLegacyRaster = false;
    if( !chartSymbols.LoadConfigFile( this, PLib ) ) {
        wxString msg( _T("Could not load XML PLib symbol file: ") );
        msg += PLib;
        wxLogMessage( msg );
 
        return 0;
    }

    //   Initialize the _cond_sym Hash Table from the jump table found in S52CNSY.CPP
    //   Hash Table indices are the literal CS Strings, e.g. "RESARE02"
    //   Hash Results Values are the Rule *, i.e. the CS procedure entry point

    for( int i = 0; condTable[i].condInst != NULL; ++i ) {
        wxString index( condTable[i].name, wxConvUTF8 );
        ( *_cond_sym )[index] = (Rule *) ( condTable[i].condInst );
    }

    wxString s57data_dir = *GetpSharedDataLocation();
    s57data_dir += _T("s57data");
    
    wxString oc_file( s57data_dir );
    oc_file.Append( _T("/s57objectclasses.csv") );

    PreloadOBJLFromCSV( oc_file );

    return 1;
}

void s52plib::ClearRulesCache( Rule *pR ) //  Clear out any existing cached symbology
{
    switch( pR->parm0 ){
        case ID_wxBitmap: {
            wxBitmap *pbm = (wxBitmap *) ( pR->pixelPtr );
            delete pbm;
            pR->pixelPtr = NULL;
            pR->parm0 = ID_EMPTY;
            break;
        }
        case ID_RGBA: {
            unsigned char *p = (unsigned char *) ( pR->pixelPtr );
            free( p );
            pR->pixelPtr = NULL;
            pR->parm0 = ID_EMPTY;
            break;
        }
        case ID_GL_PATT_SPEC: {
            render_canvas_parms *pp = (render_canvas_parms *) ( pR->pixelPtr );
            free( pp->pix_buff );
#ifdef ocpnUSE_GL
            if(pp->OGL_tex_name) glDeleteTextures( 1, (GLuint *) &pp->OGL_tex_name );
#endif
            delete pp;
            pR->pixelPtr = NULL;
            pR->parm0 = ID_EMPTY;
            break;
        }
        case ID_RGB_PATT_SPEC: {
            render_canvas_parms *pp = (render_canvas_parms *) ( pR->pixelPtr );
            free( pp->pix_buff );
            delete pp;
            pR->pixelPtr = NULL;
            pR->parm0 = ID_EMPTY;
            break;
        }
        case ID_EMPTY:
            break;
        default:
            assert(false);
            break;
    }
}

void s52plib::DestroyPatternRuleNode( Rule *pR )
{
    DestroyRuleNode(pR);
}

void s52plib::DestroyRuleNode( Rule *pR )
{
    if( !pR )
        return;

    delete pR->exposition.LXPO;

    free( pR->vector.LVCT );

    delete pR->bitmap.SBTM;

    free( pR->colRef.SCRF );

    ClearRulesCache( pR ); //  Clear out any existing cached symbology
}

void s52plib::DestroyRules( RuleHash *rh )
{
    RuleHash::iterator it;
    Rule *pR;

    for( it = ( *rh ).begin(); it != ( *rh ).end(); ++it ) {
        pR = it->second;
        DestroyRuleNode( pR );
    }

    rh->clear();
    delete rh;
}

void s52plib::FlushSymbolCaches( void )
{
    if( !useLegacyRaster ) ChartSymbols::LoadRasterFileForColorTable( m_colortable_index, true );

    RuleHash *rh = _symb_sym;

    if( !rh ) return;

    RuleHash::iterator it;
    Rule *pR;

    for( it = ( *rh ).begin(); it != ( *rh ).end(); ++it ) {
        pR = it->second;
        if( pR ) ClearRulesCache( pR );
    }

    //    Flush any pattern definitions
    rh = _patt_sym;

    if( !rh ) return;

    for( it = ( *rh ).begin(); it != ( *rh ).end(); ++it ) {
        pR = it->second;
        if( pR ) ClearRulesCache( pR );
    }

    //    OpenGL Hashmaps
    CARC_Hash::iterator ita;
    for( ita = m_CARC_hashmap.begin(); ita != m_CARC_hashmap.end(); ++ita ) {
        CARC_Buffer buffer = ita->second;
        delete [] buffer.data;
    }
    m_CARC_hashmap.clear();

#ifdef ocpnUSE_GL    
#ifndef USE_ANDROID_GLES2
    CARC_DL_Hash::iterator itd;
    for( itd = m_CARC_DL_hashmap.begin(); itd != m_CARC_DL_hashmap.end(); ++itd ) {
        GLuint list = itd->second;
        glDeleteLists( list, 1 );
    }
    m_CARC_DL_hashmap.clear();
#endif
#endif

    
    // Flush all texFonts
    TexFont *f_cache = 0;
    unsigned int i;
    for (i = 0; i < TXF_CACHE; i++)
    {
        if (s_txf[i].key != 0) {
            f_cache = &s_txf[i].cache;
            f_cache->Delete();
            s_txf[i].key = 0;
        }
    }
    
}

void s52plib::DestroyPattRules( RuleHash *rh )
{
    DestroyRules(rh);
}


void s52plib::DestroyLUPArray( wxArrayOfLUPrec *pLUPArray )
{
    if( pLUPArray ) {
        for( unsigned int il = 0; il < pLUPArray->GetCount(); il++ )
            DestroyLUP( pLUPArray->Item( il ) );

        pLUPArray->Clear();

        delete pLUPArray;
    }
}

void s52plib::ClearCNSYLUPArray( void )
{
    if( condSymbolLUPArray ) {
        for( unsigned int i = 0; i < condSymbolLUPArray->GetCount(); i++ )
            DestroyLUP( condSymbolLUPArray->Item( i ) );

        condSymbolLUPArray->Clear();
    }
}

bool s52plib::S52_flush_Plib()
{
    if(!m_bOK)
        return false;

#ifdef ocpnUSE_GL
    //    OpenGL Hashmaps
    CARC_Hash::iterator ita;
    for( ita = m_CARC_hashmap.begin(); ita != m_CARC_hashmap.end(); ++ita ) {
        CARC_Buffer buffer = ita->second;
        delete [] buffer.data;
    }
    m_CARC_hashmap.clear();

#ifndef USE_ANDROID_GLES2
    CARC_DL_Hash::iterator itd;
    for( itd = m_CARC_DL_hashmap.begin(); itd != m_CARC_DL_hashmap.end(); ++itd ) {
        GLuint list = itd->second;
        glDeleteLists( list, 1 );
    }
    m_CARC_DL_hashmap.clear();
#endif
    
#endif
    
    DestroyLUPArray( condSymbolLUPArray );

//      Destroy Rules
    DestroyRules( _line_sym );
    DestroyPattRules( _patt_sym );
    DestroyRules( _symb_sym );

    if( _symb_symR ) DestroyRules( _symb_symR );

//      Special case for CS
    _cond_sym->clear();
    delete ( _cond_sym );

    for( unsigned int ipa = 0; ipa < pAlloc->GetCount(); ipa++ ) {
        void *t = pAlloc->Item( ipa );
        free( t );
    }

    pAlloc->clear();
    delete pAlloc;

    return TRUE;
}

LUPrec *s52plib::S52_LUPLookup( LUPname LUP_Name, const char * objectName, S57Obj *pObj, bool bStrict )
{
    LUPrec *LUP = NULL;

    LUPArrayContainer *plac = SelectLUPArrayContainer( LUP_Name );
    
    LUPHashIndex *hip = plac->GetArrayIndexHelper( objectName );
    int nLUPs = hip->count;
    int nStartIndex = hip->n_start;
    
    LUP = FindBestLUP( plac->GetLUPArray(), nStartIndex, nLUPs, pObj, bStrict );
    
    return LUP;
}


void  s52plib::SetPLIBColorScheme( ColorScheme cs )
{
    wxString SchemeName;
    switch( cs ){
        case GLOBAL_COLOR_SCHEME_DAY:
            SchemeName = _T("DAY");
            break;
        case GLOBAL_COLOR_SCHEME_DUSK:
            SchemeName = _T("DUSK");
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            SchemeName = _T("NIGHT");
            break;
        default:
            SchemeName = _T("DAY");
            break;
    }
    
    SetPLIBColorScheme( SchemeName );
}
    


void s52plib::SetPLIBColorScheme( wxString scheme )
{
    wxString str_find;
    str_find = scheme;
    m_colortable_index = 0; // default is the first color in the table

// Of course, it also depends on the plib version...
// plib version 3.2 calls "DAY" colr as "DAY_BRIGHT"

    if( ( GetMajorVersion() == 3 ) && ( GetMinorVersion() == 2 ) ) {
        if( scheme.IsSameAs( _T ( "DAY" ) ) ) str_find = _T ( "DAY_BRIGHT" );
    }
    m_colortable_index = ChartSymbols::FindColorTable( scheme );

//    if( !useLegacyRaster ) ChartSymbols::LoadRasterFileForColorTable( m_colortable_index );

    if( !useLegacyRaster ) ChartSymbols::SetColorTableIndex( m_colortable_index );
    
    m_ColorScheme = scheme;
}

S52color* s52plib::getColor( const char *colorName )
{
    S52color* c;
    c = ChartSymbols::GetColor( colorName, m_colortable_index );
    return c;
}

wxColour s52plib::getwxColour( const wxString &colorName )
{
    wxColor c;
    c = ChartSymbols::GetwxColor( colorName, m_colortable_index );
    return c;
}

//----------------------------------------------------------------------------------
//
//              Object Rendering Module
//
//----------------------------------------------------------------------------------

//-----------------------------
//
// S52 TEXT COMMAND WORD PARSER
//
//-----------------------------
#define APOS   '\047'
#define MAXL       512

char *s52plib::_getParamVal( ObjRazRules *rzRules, char *str, char *buf, int bsz )
// Symbology Command Word Parameter Value Parser.
//
//      str: psz to Attribute of interest
//
//      Results:Put in 'buf' one of:
//  1- LUP constant value,
//  2- ENC value,
//  3- LUP default value.
// Return pointer to the next field in the string (delim is ','), NULL to abort
{
    wxString value;
    int defval = 0; // default value
    int len = 0;
    char *ret_ptr = str;
    char *tmp = buf;

    if(!buf)
        return NULL;

    buf[0] = 0;
    // parse constant parameter with concatenation operator "'"
    if( str != NULL ) {
        if( *ret_ptr == APOS ) {
            ret_ptr++;
            while( *ret_ptr != APOS &&  *ret_ptr != '\0' && len < ( bsz - 1 )) {
                ++len;
                *tmp++ = *ret_ptr++;
            }
            *tmp = '\0';
            ret_ptr++; // skip "'"
            ret_ptr++; // skip ","

            return ret_ptr;
        }

        while( *ret_ptr != ',' && *ret_ptr != ')' && *ret_ptr != '\0' && len < ( bsz - 1 ) ) {
            *tmp++ = *ret_ptr++;
            ++len;
        }
        *tmp = '\0';

        ret_ptr++; // skip ',' or ')'
    }
    if( len < 6 )
        return ret_ptr;

    // chop string if default value present
    if( len > 6 && *( buf + 6 ) == '=' ) {
        *( buf + 6 ) = '\0';
        defval = 1;
    }

    value = rzRules->obj->GetAttrValueAsString( buf );
    wxCharBuffer buffer=value.ToUTF8();
    if(!buffer.data())
        return ret_ptr;

    if( value.IsEmpty() ) {
        if( defval )
            _getParamVal( rzRules, buf + 7, buf, bsz - 7 ); // default value --recursion
        else {
            return NULL; // abort
        }
    } else {

        //    Special case for conversion of some vertical (height) attributes to feet
        if( ( !strncmp( buf, "VERCLR", 6 ) ) || ( !strncmp( buf, "VERCCL", 6 ) ) || ( !strncmp( buf, "VERCOP", 6 ) ) ) {
            switch( m_nDepthUnitDisplay ){
                case 0: // feet
                case 2: // fathoms
                    double ft_val;
                    value.ToDouble( &ft_val );
                    ft_val = ft_val * 3 * 39.37 / 36; // feet
                    value.Printf( _T("%4.1f"), ft_val );
                    break;
                default:
                    break;
            }
        }

        // special case when ENC returns an index for particular attribute types
        if( !strncmp( buf, "NATSUR", 6 ) ) {
            wxString natsur_att( _T ( "NATSUR" ) );
            wxString result;
            wxString svalue = value;
            wxStringTokenizer tkz( svalue, _T ( "," ) );

            int icomma = 0;
            while( tkz.HasMoreTokens() ) {
                if( icomma )
                    result += _T ( "," );

                wxString token = tkz.GetNextToken();
                long i;
                if( token.ToLong(&i) ){
                    wxString nat;
                    if( !m_natsur_hash[i].IsEmpty() )            // entry available?
                        nat = m_natsur_hash[i];
                    else {
                        nat = GetS57AttributeDecode( natsur_att, (int)i );
                        m_natsur_hash[i] = nat;            // cache the entry
                    }
                        
                    if( !nat.IsEmpty() )
                        result += nat; // value from ENC
                    else
                        result += _T ( "unk" );
                }
                else
                    result += _T ( "unk" );

                icomma++;
            }

            value = result;
        }

        wxCharBuffer buffer=value.ToUTF8();
        if(buffer.data()){
            unsigned int len = wxMin(strlen(buffer.data()), (unsigned int)bsz-1);
            memcpy( buf, buffer.data(), len );
            buf[len] = 0;
        }
        else
            *buf = 0;
    }

    return ret_ptr;
}

char *s52plib::_parseTEXT( ObjRazRules *rzRules, S52_TextC *text, char *str0 )
{
    char buf[MAXL]; // output string

    char *str = str0;
    if( text ) {
        memset(buf, 0, 4);
        str = _getParamVal( rzRules, str, &text->hjust, MAXL ); // HJUST
        str = _getParamVal( rzRules, str, &text->vjust, MAXL ); // VJUST
        str = _getParamVal( rzRules, str, &text->space, MAXL ); // SPACE

        // CHARS
        str = _getParamVal( rzRules, str, buf, MAXL );
        text->style = buf[0];
        text->weight = buf[1];
        text->width = buf[2];
        text->bsize = atoi( buf + 3 );

        str = _getParamVal( rzRules, str, buf, MAXL );
        text->xoffs = atoi( buf );
        str = _getParamVal( rzRules, str, buf, MAXL );
        text->yoffs = atoi( buf );
        str = _getParamVal( rzRules, str, buf, MAXL );
        text->pcol = getColor( buf );
        str = _getParamVal( rzRules, str, buf, MAXL );
        text->dis = atoi( buf ); // Text Group, used for "Important" text detection
    }
    return str;
}

S52_TextC *s52plib::S52_PL_parseTX( ObjRazRules *rzRules, Rules *rules, char *cmd )
{
    S52_TextC *text = NULL;
    char *str = NULL;
    char val[MAXL]; // value of arg
    char strnobjnm[7] = { "NOBJNM" };
    char valn[MAXL]; // value of arg

    valn[0] = 0;
    str = (char*) rules->INSTstr;

    if( m_bShowNationalTexts && NULL != strstr( str, "OBJNAM" ) ) // in case user wants the national text shown and the rule contains OBJNAM, try to get the value
    {
        _getParamVal( rzRules, strnobjnm, valn, MAXL );
        if( !strcmp( strnobjnm, valn ) )
            valn[0] = '\0'; //NOBJNM is not defined
        else
            valn[MAXL - 1] = '\0'; // make sure the string terminates
    }

    str = _getParamVal( rzRules, str, val, MAXL ); // get ATTRIB list

    if( NULL == str ) return 0; // abort this command word if mandatory param absent

    val[MAXL - 1] = '\0'; // make sure the string terminates

    text = new S52_TextC;
    str = _parseTEXT( rzRules, text, str );
    if( NULL != text )
    {
        if ( valn[0] != '\0' ) {
            text->frmtd = wxString( valn, wxConvUTF8 );
            text->bnat = true;
        }
        else {
            text->frmtd = wxString( val, wxConvUTF8 );
            text->bnat = false;
        }
    }

    //  We check to see if the formatted text has any "special" characters
    wxCharBuffer buf = text->frmtd.ToUTF8();
    
    unsigned int n = text->frmtd.Length();
    for(unsigned int i=0 ; i < n ; ++i){
        unsigned char c =buf.data()[i];
        if(c > 127){
            text->bspecial_char = true;
            break;
        }
    }
    
    return text;
}

S52_TextC *s52plib::S52_PL_parseTE( ObjRazRules *rzRules, Rules *rules, char *cmd )
// same as S52_PL_parseTX put parse 'C' format first
{
    char arg[MAXL]; // ATTRIB list
    char fmt[MAXL]; // FORMAT
    char buf[MAXL]; // output string
    char *b = buf;
    char *parg = arg;
    char *pf = fmt;
    S52_TextC *text = NULL;

    char *str = (char*) rules->INSTstr;

    if( str && *str ) {
        str = _getParamVal( rzRules, str, fmt, MAXL ); // get FORMAT

        str = _getParamVal( rzRules, str, arg, MAXL ); // get ATTRIB list
        if( NULL == str ) return 0; // abort this command word if mandatory param absent

        //*b = *pf;
        while( *pf != '\0' ) {

            // begin a convertion specification
            if( *pf == '%' ) {
                char val[MAXL]; // value of arg
                char tmp[MAXL] = { '\0' }; // temporary format string
                char *t = tmp;
                int cc = 0; // 1 == Conversion Character found
                //*t = *pf;

                // get value for this attribute
                parg = _getParamVal( rzRules, parg, val, MAXL );
                if( NULL == parg ) return 0; // abort

                if( 0 == strcmp( val, "2147483641" ) ) return 0;

                *t = *pf; // stuff the '%'

                // scan for end at convertion character
                do {
                    *++t = *++pf; // fill conver spec

                    switch( *pf ){
                        case 'c':
                        case 's':
                            b += sprintf( b, tmp, val );
                            cc = 1;
                            break;
                        case 'f':
                            b += sprintf( b, tmp, atof( val ) );
                            cc = 1;
                            break;
                        case 'd':
                        case 'i':
                            b += sprintf( b, tmp, atoi( val ) );
                            cc = 1;
                            break;
                    }
                } while( !cc );
                pf++; // skip conv. char

            } else
                *b++ = *pf++;
        }

        *b = 0;
        text = new S52_TextC;
        str = _parseTEXT( rzRules, text, str );
        if( NULL != text ) text->frmtd = wxString( buf, wxConvUTF8 );
        
        //  We check to see if the formatted text has any "special" characters
        wxCharBuffer buf = text->frmtd.ToUTF8();
        
        unsigned int n = text->frmtd.Length();
        for(unsigned int i=0 ; i < n ; ++i){
            unsigned char c =buf.data()[i];
            if(c > 127){
                text->bspecial_char = true;
                break;
            }
        }
        
    }

    return text;
}

static void rotate(wxRect *r, ViewPort const &vp)
{
    float cx = vp.pix_width/2.;
    float cy = vp.pix_height/2.;
    float c = cosf(vp.rotation );
    float s = sinf(vp.rotation );
    float x = r->GetX() -cx;
    float y = r->GetY() -cy;
    r->SetX( x*c - y*s +cx);
    r->SetY( x*s + y*c +cy);
}

bool s52plib::RenderText( wxDC *pdc, S52_TextC *ptext, int x, int y, wxRect *pRectDrawn,
                          S57Obj *pobj, bool bCheckOverlap, ViewPort *vp )
{
    #ifdef DrawText
    #undef DrawText
    #define FIXIT
    #endif
    bool bdraw = true;
    
    wxFont *scaled_font = ptext->pFont;
    wxCoord w_scaled = 0;
    wxCoord h_scaled = 0;
    wxCoord descent = 0;
    wxCoord exlead = 0;
    
    double sfactor = vp->ref_scale/vp->chart_scale;
    double scale_factor = wxMax((sfactor - g_overzoom_emphasis_base)  / 4., 1.);
    
    if(!g_oz_vector_scale || !vp->b_quilt)
        scale_factor = 1.0;
    
    //  Place an upper bound on the scaled text size
        scale_factor = wxMin(scale_factor, 4);
        
        if( !pdc ) // OpenGL
    {
#ifdef ocpnUSE_GL
        
        bool b_force_no_texture = false;
        if(scale_factor > 1.){
            b_force_no_texture = true;
            
            int old_size = ptext->pFont->GetPointSize();
            int new_size = old_size * scale_factor;
            scaled_font = FindOrCreateFont_PlugIn( new_size, ptext->pFont->GetFamily(),
                                                           ptext->pFont->GetStyle(), ptext->pFont->GetWeight(), false,
                                                           ptext->pFont->GetFaceName() );
            wxScreenDC sdc;
            sdc.GetTextExtent( ptext->frmtd, &w_scaled, &h_scaled, &descent, &exlead, scaled_font ); // measure the text
            
            
            // Has font size changed?  If so, clear the cached bitmap, and rebuild it
            if( (h_scaled - descent) != ptext->rendered_char_height){
                glDeleteTextures(1, (GLuint *)&ptext->texobj);
                ptext->texobj = 0;
            }
            
            // We cannot get the font ascent value to remove the interline spacing from the font "height".
            // So we have to estimate based on conventional Arial metrics
            ptext->rendered_char_height = (h_scaled - descent) * 8 / 10;
            
        }
        // We render string with "special" characters the old, hard way, since we don't necessarily have the glyphs in our font, 
        // or if we do we would need a hashmap to cache and extract them
        // And we also do this if the text is to be scaled up artificially.
        if( (ptext->bspecial_char) || b_force_no_texture) {       
            if( !ptext->texobj ) // is texture ready?
            {
                wxScreenDC sdc;
                
                if(scale_factor <= 1.){
                    sdc.GetTextExtent( ptext->frmtd, &w_scaled, &h_scaled, &descent, &exlead, scaled_font ); // measure the text

                    // We cannot get the font ascent value to remove the interline spacing from the font "height".
                    // So we have to estimate based on conventional Arial metrics
                    ptext->rendered_char_height = (h_scaled - descent) * 8 / 10;
                }
                
                ptext->text_width = w_scaled;
                ptext->text_height = h_scaled;
                
                /* make power of 2 */
                int tex_w, tex_h;
                for(tex_w = 1; tex_w < ptext->text_width; tex_w *= 2);
                for(tex_h = 1; tex_h < ptext->text_height; tex_h *= 2);
           
           wxMemoryDC mdc;
           wxBitmap bmp( tex_w, tex_h );
           mdc.SelectObject( bmp );
           mdc.SetFont( *( scaled_font ) );
           
           if( mdc.IsOk() ) {
               //  Render the text as white on black, so that underlying anti-aliasing of
               //  wxDC text rendering can be extracted and converted to alpha-channel values.
               
               mdc.SetBackground( wxBrush( wxColour( 0, 0, 0 ) ) );
               mdc.SetBackgroundMode( wxTRANSPARENT );
               
               mdc.SetTextForeground( wxColour( 255, 255, 255 ) );
               
               mdc.Clear();
               
               mdc.DrawText( ptext->frmtd, 0, 0 );
               mdc.SelectObject( wxNullBitmap );
               
               wxImage image = bmp.ConvertToImage();
               int ws = image.GetWidth(), hs = image.GetHeight();
               
               ptext->RGBA_width = ws;
               ptext->RGBA_height = hs;
               unsigned char *pRGBA = (unsigned char *) malloc( 4 * ws * hs );
               
               unsigned char *d = image.GetData();
               unsigned char *pdest = pRGBA;
               S52color *ccolor = ptext->pcol;
               
               if(d){
                   for( int y = 0; y < hs; y++ )
                       for( int x = 0; x < ws; x++ ) {
                           unsigned char r, g, b;
                           int off = ( y * ws + x );
                           
                           r = d[off * 3 + 0];
                           g = d[off * 3 + 1];
                           b = d[off * 3 + 2];
                           
                           pdest[off * 4 + 0] = ccolor->R;
                           pdest[off * 4 + 1] = ccolor->G;
                           pdest[off * 4 + 2] = ccolor->B;
                           
                           int alpha = ( r + g + b ) / 3;
                           pdest[off * 4 + 3] = (unsigned char) ( alpha & 0xff );
                       }
               }
               
               
               int draw_width = ptext->RGBA_width;
               int draw_height = ptext->RGBA_height;
               
               glEnable( GL_TEXTURE_2D );
               
               GLuint texobj;
               glGenTextures( 1, &texobj );
               
               glBindTexture( GL_TEXTURE_2D, texobj );
               
               glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
               glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
               glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST/*GL_LINEAR*/ );
               glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
               
               glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, draw_width, draw_height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, pRGBA );
               
               free( pRGBA );
               
               ptext->texobj = texobj;
               
           } // mdc OK
            } // Building texobj
            
            //    Render the texture
            if( ptext->texobj ) {
                int yadjust = 0;
                int xadjust = 0;
                
                //  Adjust the y position to account for the convention that S52 text is drawn
                //  with the lower left corner at the specified point, instead of the wx convention
                //  using upper left corner.
                //  Also, allow for full text height in the bitmap/texture, not the estimated "rendered" height.
                
                yadjust =  -ptext->rendered_char_height * 10 / 8;
                
                //  Add in the offsets, specified in units of nominal font height
                yadjust += ptext->yoffs * ( ptext->rendered_char_height );
                //  X offset specified in units of average char width
                xadjust += ptext->xoffs * ptext->avgCharWidth;
                
                // adjust for text justification
                int w = ptext->text_width; 
                switch ( ptext->hjust){
                    case '1':               // centered
                    xadjust -= w/2;
                    break;
                    case '2':               // right
                     xadjust -= w;
                     break;
                    case '3':               // left (default)
                    default:
                        break;
                }
                
                switch ( ptext->vjust){
                    case '3':               // top
                    yadjust += ptext->rendered_char_height;
                    break;
                    case '2':               // centered
                     yadjust += ptext->rendered_char_height/2;
                     break;
                    case '1':               // bottom (default)
                    default:
                        break;
                }
                
                int xp = x;
                int yp = y;

                if(fabs(vp->rotation) > 0.01){
                    float c = cosf(-vp->rotation );
                    float s = sinf(-vp->rotation );
                    float x = xadjust;
                    float y = yadjust;
                    xp += x*c - y*s;
                    yp += x*s + y*c;
                    
                }
                else{
                    xp+= xadjust;
                    yp+= yadjust;
                }
                
                
                pRectDrawn->SetX( xp );
                pRectDrawn->SetY( yp );
                pRectDrawn->SetWidth( ptext->text_width );
                pRectDrawn->SetHeight( ptext->text_height );
                
                if( bCheckOverlap ) {
                    if( CheckTextRectList( *pRectDrawn, ptext ) )
                        bdraw = false;
                }
                
                if( bdraw ) {
extern GLenum       g_texture_rectangle_format;
#ifndef USE_ANDROID_GLES2
                    
                    int draw_width = ptext->text_width;
                    int draw_height = ptext->text_height;
                    
                    
                    glEnable( GL_BLEND );
                    glEnable( GL_TEXTURE_2D );
                    
                    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
                    
                    glBindTexture( GL_TEXTURE_2D, ptext->texobj );
                    
                    glPushMatrix();
                    glTranslatef(xp, yp, 0);
                    
                    /* undo previous rotation to make text level */
                    glRotatef(vp->rotation*180/PI, 0, 0, -1);
                    
                    
                    float tx1 = 0, tx2 = draw_width;
                    float ty1 = 0, ty2 = draw_height;
                    
                    if(g_texture_rectangle_format == GL_TEXTURE_2D) {
                        
                        tx1 /= ptext->RGBA_width, tx2 /= ptext->RGBA_width;
                        ty1 /= ptext->RGBA_height, ty2 /= ptext->RGBA_height;
                    }
                    
                    
                    glBegin( GL_QUADS );
                    
                    glTexCoord2f( tx1, ty1 );  glVertex2i( 0, 0 );
                    glTexCoord2f( tx2, ty1 );  glVertex2i( draw_width, 0 );
                    glTexCoord2f( tx2, ty2 );  glVertex2i( draw_width, draw_height );
                    glTexCoord2f( tx1, ty2 );  glVertex2i( 0, draw_height );
                    
                    glEnd();
                    
                    glPopMatrix();
                    
                    glDisable( g_texture_rectangle_format );
                    glDisable( GL_BLEND );
#else
                    glEnable( GL_BLEND );
                    glEnable( GL_TEXTURE_2D );

                    float uv[8];
                    float coords[8];
                    
                    // Note swizzle of points to allow TRIANGLE_STRIP drawing
                    //normal uv
                    uv[0] = 0; uv[1] = 0; uv[2] = 1; uv[3] = 0;
                    uv[6] = 1; uv[7] = 1; uv[4] = 0; uv[5] = 1;
                    
                    //w *= scale_factor;
                    //h *= scale_factor;
                    
                    // pixels
                    coords[0] = 0; coords[1] = 0; coords[2] = ptext->RGBA_width; coords[3] = 0;
                    coords[6] = ptext->RGBA_width; coords[7] = ptext->RGBA_height; coords[4] = 0; coords[5] = ptext->RGBA_height;
                    
                    glUseProgram( S52texture_2D_shader_program );
                    
                    // Get pointers to the attributes in the program.
                    GLint mPosAttrib = glGetAttribLocation( S52texture_2D_shader_program, "position" );
                    GLint mUvAttrib  = glGetAttribLocation( S52texture_2D_shader_program, "aUV" );
                    
                    // Select the active texture unit.
                    glActiveTexture( GL_TEXTURE0 );
                    
                    // Bind our texture to the texturing target.
                    glBindTexture( GL_TEXTURE_2D, ptext->texobj );
                    
                    // Set up the texture sampler to texture unit 0
                    GLint texUni = glGetUniformLocation( S52texture_2D_shader_program, "uTex" );
                    glUniform1i( texUni, 0 );
                    
                    // Disable VBO's (vertex buffer objects) for attributes.
                    glBindBuffer( GL_ARRAY_BUFFER, 0 );
                    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
                    
                    // Set the attribute mPosAttrib with the vertices in the screen coordinates...
                    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
                    // ... and enable it.
                    glEnableVertexAttribArray( mPosAttrib );
                    
                    // Set the attribute mUvAttrib with the vertices in the GL coordinates...
                    glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uv );
                    // ... and enable it.
                    glEnableVertexAttribArray( mUvAttrib );
                    
                    // Rotate
                    mat4x4 I, Q;
                    mat4x4_identity(I);
                    
                    mat4x4_translate_in_place(I, x, y, 0);
                    mat4x4_rotate_Z(Q, I, -vp->rotation);
                    mat4x4_translate_in_place(Q, xadjust, yadjust, 0);
                    
                    
                    GLint matloc = glGetUniformLocation(S52texture_2D_shader_program,"TransformMatrix");
                    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);
                    
                    // Perform the actual drawing.
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    
                    // Restore the per-object transform to Identity Matrix
                    mat4x4 IM;
                    mat4x4_identity(IM);
                    GLint matlocf = glGetUniformLocation(S52texture_2D_shader_program,"TransformMatrix");
                    glUniformMatrix4fv( matlocf, 1, GL_FALSE, (const GLfloat*)IM);
                    
                    glDisable( GL_TEXTURE_2D );
                    glDisable( GL_BLEND );
#endif
                    
                    
                } // bdraw
                
            }
            
            bdraw = true;
        }
        
        else {                                          // render using cached texture glyphs
            // rebuild font if needed
            TexFont *f_cache = 0;
            unsigned int i;
            for (i = 0; i < TXF_CACHE; i++)
            {
                if (s_txf[i].key == ptext->pFont) {
                    f_cache = &s_txf[i].cache;
                    break;
                }
                if (s_txf[i].key == 0) {
                    break;
                }
            }
            if (i == TXF_CACHE) {
                i = rand() & (TXF_CACHE -1);
            }
            if (f_cache == 0) {
                s_txf[i].key = ptext->pFont;
                f_cache = &s_txf[i].cache;
                f_cache->Build(*ptext->pFont);
            }

            int w, h;
            f_cache->GetTextExtent(ptext->frmtd, &w, &h);
            
            // We don't store descent/ascent info for font texture cache
            // So we have to estimate based on conventional Arial metrics
            ptext->rendered_char_height = h * 65 / 100;
            
            //  Adjust the y position to account for the convention that S52 text is drawn
            //  with the lower left corner at the specified point, instead of the wx convention
            //  using upper right corner
            int yadjust = 0;
            int xadjust = 0;
            
            yadjust =  -ptext->rendered_char_height;
            
            
            //  Add in the offsets, specified in units of nominal font height
            yadjust += ptext->yoffs * ( ptext->rendered_char_height );
            //  X offset specified in units of average char width
            xadjust += ptext->xoffs * ptext->avgCharWidth;
            
            // adjust for text justification
            switch ( ptext->hjust){
                case '1':               // centered
                    xadjust -= w/2;
                    break;
                case '2':               // right
                     xadjust -= w;
                     break;
                case '3':               // left (default)
                default:
                    break;
            }
            
            switch ( ptext->vjust){
                case '3':               // top
                    yadjust += ptext->rendered_char_height;
                    break;
                case '2':               // centered
                     yadjust += ptext->rendered_char_height/2;
                     break;
                case '1':               // bottom (default)
                default:
                    break;
            }

            int xp = x;
            int yp = y;
            
            if(fabs(vp->rotation) > 0.01){
                float c = cosf(-vp->rotation );
                float s = sinf(-vp->rotation );
                float x = xadjust;
                float y = yadjust;
                xadjust =  x*c - y*s;
                yadjust =  x*s + y*c;
            }
          
            xp+= xadjust;
            yp+= yadjust;
            
            pRectDrawn->SetX( xp );
            pRectDrawn->SetY( yp );
            pRectDrawn->SetWidth( w );
            pRectDrawn->SetHeight( h );
            
            if( bCheckOverlap ) {
                if(fabs( vp->rotation ) > .01){
                    rotate(pRectDrawn, *vp );
                }
                if( CheckTextRectList( *pRectDrawn, ptext ) ) bdraw = false;
            }
            
            if( bdraw ) {
#ifndef USE_ANDROID_GLES2
                wxColour wcolor = GetFontColour_PlugIn(_("ChartTexts"));
                if( wcolor == *wxBLACK )
                    glColor3ub( ptext->pcol->R, ptext->pcol->G, ptext->pcol->B );
                else
                    glColor3ub( wcolor.Red(), wcolor.Green(), wcolor.Blue() );
                
                glEnable( GL_BLEND );
                glEnable( GL_TEXTURE_2D );
                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
                
                glPushMatrix();
                glTranslatef(xp, yp, 0);
                
                /* undo previous rotation to make text level */
                glRotatef(vp->rotation*180/PI, 0, 0, -1);
                
                f_cache->RenderString(ptext->frmtd);
                glPopMatrix();
                
                glDisable( GL_TEXTURE_2D );
                glDisable( GL_BLEND );
#else
                glEnable( GL_BLEND );
                glEnable( GL_TEXTURE_2D );

                wxColour wcolor = GetFontColour_PlugIn(_("ChartTexts"));
                f_cache->SetColor( wcolor );
                
                /* undo previous rotation to make text level */
                //glRotatef(vp->rotation*180/PI, 0, 0, -1);

                f_cache->RenderString(ptext->frmtd, xp, yp);

                glDisable( GL_TEXTURE_2D );
                glDisable( GL_BLEND );
#endif
            }
        }
        
        #endif
    } else {                // Not OpenGL
            wxFont oldfont = pdc->GetFont(); // save current font
            
            
            if(scale_factor > 1){
                wxFont *pf = ptext->pFont;
                int old_size = pf->GetPointSize();
                int new_size = old_size * scale_factor;
                wxFont *scaled_font = FindOrCreateFont_PlugIn( new_size, pf->GetFamily(),
                                                                       pf->GetStyle(), pf->GetWeight(), false,
                                                                       pf->GetFaceName() );
                pdc->SetFont( *scaled_font);
            }
            else{
                pdc->SetFont( *( ptext->pFont ) );
            }
            
            wxCoord w, h, descent, exlead;
            pdc->GetTextExtent( ptext->frmtd, &w, &h, &descent, &exlead ); // measure the text
            
            // We cannot get the font ascent value to remove the interline spacing.
            // So we have to estimate based on conventional Arial metrics
            int rendered_text_height = (h - descent) * 8 / 10;
            
            //  Adjust the y position to account for the convention that S52 text is drawn
            //  with the lower left corner at the specified point, instead of the wx convention
            //  using upper right corner
            int yadjust = 0;
            int xadjust = 0;
            
            yadjust =  - ( rendered_text_height );
            
            //  Add in the offsets, specified in units of nominal font height
            yadjust += ptext->yoffs * ( rendered_text_height );
            
            //  X offset specified in units of average char width
            xadjust += ptext->xoffs * ptext->avgCharWidth;
            
            // adjust for text justification
            switch ( ptext->hjust){
                case '1':               // centered
                    xadjust -= w/2;
                    break;
                case '2':               // right
                     xadjust -= w;
                     break;
                case '3':               // left (default)
                default:
                    break;
            }
            
            switch ( ptext->vjust){
                case '3':               // top
                    yadjust += rendered_text_height;
                    break;
                case '2':               // centered
                     yadjust += rendered_text_height/2;
                     break;
                case '1':               // bottom (default)
                default:
                    break;
            }
            
            int xp = x;
            int yp = y;
            
            
            if(fabs(vp->rotation) > 0.01){
                float cx = vp->pix_width/2.;
                float cy = vp->pix_height/2.;
                float c = cosf(vp->rotation );
                float s = sinf(vp->rotation );
                float x = xp -cx;
                float y = yp -cy;
                xp =  x*c - y*s +cx + vp->rv_rect.x;
                yp =  x*s + y*c +cy + vp->rv_rect.y;
                
            }
            
            xp+= xadjust;
            yp+= yadjust;
            
            pRectDrawn->SetX( xp );
            pRectDrawn->SetY( yp );
            pRectDrawn->SetWidth( w );
            pRectDrawn->SetHeight( h );
            
            if( bCheckOverlap ) {
                if( CheckTextRectList( *pRectDrawn, ptext ) )
                    bdraw = false;
            }
            
            if( bdraw ) {
                wxColour wcolor = GetFontColour_PlugIn(_("ChartTexts"));
                
                // If the user has not changed the color from BLACK, then use the color specified in the S52 LUP
                if( wcolor == *wxBLACK )
                    wcolor = wxColour( ptext->pcol->R, ptext->pcol->G, ptext->pcol->B );
                pdc->SetTextForeground( wcolor );
                
                pdc->DrawText( ptext->frmtd, xp, yp );
            }
            
            pdc->SetFont( oldfont ); // restore last font
            
    }
    return bdraw;
    
    
    #ifdef FIXIT
    #undef FIXIT
    #define DrawText DrawTextA
    #endif
    
}



//    Return true if test_rect overlaps any rect in the current text rectangle list, except itself
bool s52plib::CheckTextRectList( const wxRect &test_rect, S52_TextC *ptext )
{
    //    Iterate over the current object list, looking at rText

    for( TextObjList::Node *node = m_textObjList.GetFirst(); node; node = node->GetNext() ) {
        wxRect *pcurrent_rect = &( node->GetData()->rText );

        if( pcurrent_rect->Intersects( test_rect ) ) {
            if( node->GetData() != ptext )
                return true;

        }
    }
    return false;
}

bool s52plib::TextRenderCheck( ObjRazRules *rzRules )
{
    if( !m_bShowS57Text ) return false;

    if(rzRules->obj->bIsAton ){

       if( !strncmp( rzRules->obj->FeatureName, "LIGHTS", 6 ) ) {
           if( !m_bShowLdisText )
               return false;
       }
       else{
           if( !m_bShowAtonText )
               return false;
       }
    }
        
        

    // Declutter LIGHTS descriptions
    if( ( rzRules->obj->bIsAton ) && ( !strncmp( rzRules->obj->FeatureName, "LIGHTS", 6 ) ) ){
        if( lastLightLat == rzRules->obj->m_lat && lastLightLon == rzRules->obj->m_lon ){
            return false;       // only render text for the first object at this lat/lon
        }
        else{
            lastLightLat = rzRules->obj->m_lat;
            lastLightLon = rzRules->obj->m_lon;
        }
    }
    
    //    An optimization for CM93 charts.
    //    Don't show the text associated with some objects, since CM93 database includes _texto objects aplenty
    if( ( (int)rzRules->obj->auxParm3 == (int)PI_CHART_TYPE_CM93 )
        || ( (int)rzRules->obj->auxParm3 == (int)PI_CHART_TYPE_CM93COMP ) ) {
        if( !strncmp( rzRules->obj->FeatureName, "BUAARE", 6 ) )
            return false;
        else if( !strncmp( rzRules->obj->FeatureName, "SEAARE", 6 ) )
            return false;
        else if( !strncmp( rzRules->obj->FeatureName, "LNDRGN", 6 ) )
            return false;
        else if( !strncmp( rzRules->obj->FeatureName, "LNDARE", 6 ) )
            return false;
        }

    return true;
}

int s52plib::RenderT_All( ObjRazRules *rzRules, Rules *rules, ViewPort *vp, bool bTX )
{
    if( !TextRenderCheck( rzRules ) ) return 0;

    S52_TextC *text = NULL;
    bool b_free_text = false;

    //  The first Ftext object is cached in the S57Obj.
    //  If not present, create it on demand
    if( !rzRules->obj->bFText_Added ) {
        if( bTX ) text = S52_PL_parseTX( rzRules, rules, NULL );
        else
            text = S52_PL_parseTE( rzRules, rules, NULL );

        if( text ) {
            rzRules->obj->bFText_Added = true;
            rzRules->obj->FText = text;
            rzRules->obj->FText->rul_seq_creator = rules->n_sequence;
        }
    }

    //    S57Obj already contains a cached text object
    //    If it was created by this Rule earlier, then render it
    //    Otherwise, create a new text object, render it, and delete when done
    //    This will be slower, obviously, but happens infrequently enough?
    else {
        if( rules->n_sequence == rzRules->obj->FText->rul_seq_creator ) text = rzRules->obj->FText;
        else {
            if( bTX ) text = S52_PL_parseTX( rzRules, rules, NULL );
            else
                text = S52_PL_parseTE( rzRules, rules, NULL );

            b_free_text = true;
        }

    }

    if( text ) {
        if( m_bShowS57ImportantTextOnly && ( text->dis >= 20 ) ) {
            if( b_free_text ) delete text;
            return 0;
        }

        //    Establish a font
        if( !text->pFont ) {
            
            // Process the font specifications from the LUP symbolizatio rule
            int spec_weight = text->weight - 0x30;
            wxFontWeight fontweight;
            if( spec_weight < 5 )
                fontweight = wxFONTWEIGHT_LIGHT;
            else{
                if( spec_weight == 5 )
                    fontweight = wxFONTWEIGHT_NORMAL;
                else
                    fontweight = wxFONTWEIGHT_BOLD;
            }
             
            wxFont *specFont = FindOrCreateFont_PlugIn( text->bsize, wxFONTFAMILY_SWISS,  wxFONTSTYLE_NORMAL, fontweight );
            
            //Get the width of a single average character in the spec font
            wxScreenDC dc;
            dc.SetFont(*specFont);
            int width;
            dc.GetTextExtent(_T("X"), &width, NULL, NULL, NULL, specFont);
            text->avgCharWidth = width; 
            
            //    If we have loaded a legacy S52 compliant PLIB,
            //    then we should use the formal font selection as required by S52 specifications.
            //    Otherwise, we have our own plan...
            
            if( useLegacyRaster ) {
                text->pFont = specFont;
            } else {
                int spec_weight = text->weight - 0x30;
                wxFontWeight fontweight;
                if( spec_weight < 5 ) fontweight = wxFONTWEIGHT_LIGHT;
                else
                    if( spec_weight == 5 ) fontweight = wxFONTWEIGHT_NORMAL;
                    else
                        fontweight = wxFONTWEIGHT_BOLD;

                wxFont sys_font = *wxNORMAL_FONT;
                int default_size = sys_font.GetPointSize();

#ifdef __WXOSX__
                default_size += 1;     // default to 1pt larger than system UI font
#else
                default_size += 2;     // default to 2pt larger than system UI font
#endif
                
                wxFont *templateFont = GetOCPNScaledFont_PlugIn(_("ChartTexts"), default_size );
                
                // NOAA ENC fles requests font size up to 20 points, which looks very
                // disproportioned. Let's scale those sizes down to more reasonable values.
                int fontSize = text->bsize;

                if( fontSize > 18 ) fontSize -= 8;
                else
                    if( fontSize > 13 ) fontSize -= 3;

                // Now factor in the users selected font size.
                fontSize += templateFont->GetPointSize() - 10;
                
                // In no case should font size be less than 10, since it becomes unreadable
                fontSize = wxMax(10, fontSize);

                text->pFont = FindOrCreateFont_PlugIn( fontSize, wxFONTFAMILY_SWISS,
                        templateFont->GetStyle(), fontweight, false, templateFont->GetFaceName() );
            }
        }

        //  Render text at declared x/y of object
        wxPoint r;
        GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

        wxRect rect;
        bool bwas_drawn = RenderText( m_pdc, text, r.x, r.y, &rect, rzRules->obj, m_bDeClutterText, vp );

        //  If this is an un-cached text render, it probably means that a single object has two or more
        //  text renders in its rule set.  RDOCAL is one example.  There are others
        //  We need to cache only the first text structure, but should update the render rectangle
        //  to reflect all texts rendered for this object,  in order to process the declutter logic.
        bool b_dupok = false;
        if( b_free_text ) {
            delete text;
        
            if(!bwas_drawn){
                return 1;
            }
            else{                               // object was drawn
                text = rzRules->obj->FText;
                
                wxRect r0 = text->rText;
                r0 = r0.Union(rect);
                text->rText = r0;
            
                b_dupok = true;                 // OK to add a duplicate text structure to the declutter list, just for this case.
            }
        }
        else
            text->rText = rect;
        
        
        //      If this text was actually drawn, add a pointer to its rect to the de-clutter list if it doesn't already exist
        if( m_bDeClutterText ) {
            if( bwas_drawn ) {
                bool b_found = false;
                for( TextObjList::Node *node = m_textObjList.GetFirst(); node; node =  node->GetNext() ) {
                    S52_TextC *oc = node->GetData();

                    if( oc == text ) {
                        if(!b_dupok)
                            b_found = true;
                        break;
                    }
                }
                if( !b_found )
                    m_textObjList.Append( text );
            }
        }

        //  Update the object Bounding box
        //  so that subsequent drawing operations will redraw the item fully
        //  and so that cursor hit testing includes both the text and the object

//            if ( rzRules->obj->Primitive_type == GEO_POINT )
        {
            double latmin, lonmin, latmax, lonmax, extent = 0;

            GetPixPointSingleNoRotate( rect.GetX(), rect.GetY() + rect.GetHeight(), &latmin, &lonmin, vp );
            GetPixPointSingleNoRotate( rect.GetX() + rect.GetWidth(), rect.GetY(), &latmax, &lonmax, vp );
            LLBBox bbtext;
            bbtext.Set( latmin, lonmin, latmax, lonmax );

            rzRules->obj->BBObj.Expand( bbtext );
        }
    }

    return 1;
}

// Text
int s52plib::RenderTX( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    return RenderT_All( rzRules, rules, vp, true );
}

// Text formatted
int s52plib::RenderTE( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    return RenderT_All( rzRules, rules, vp, false );
}

bool s52plib::RenderHPGL( ObjRazRules *rzRules, Rule *prule, wxPoint &r, ViewPort *vp, float rot_angle )
{
    float fsf = 100 / canvas_pix_per_mm;


    float xscale = 1.0;
    //  Set the onscreen size of the symbol
    //  Compensate for various display resolutions
    //  Develop empirically, making a flare light about 6 mm long
    double pix_factor = GetPPMM() / 6.0;
    xscale *= pix_factor;
    
    
    if( (!strncmp(rzRules->obj->FeatureName, "TSSLPT", 6))
        || (!strncmp(rzRules->obj->FeatureName, "DWRTPT", 6))
        || (!strncmp(rzRules->obj->FeatureName, "TWRTPT", 6))
        || (!strncmp(rzRules->obj->FeatureName, "RCTLPT", 6))
    ){
        // assume the symbol length 
        float sym_length = 30;
        float scaled_length = sym_length / vp->view_scale_ppm;
        
        double fac1 = scaled_length / fsf;
        
        
        float target_length = 1852;
        
        xscale = target_length / scaled_length;
        xscale = wxMin(xscale, 1.0);
        xscale = wxMax(.4, xscale);
        
        //printf("scaled length: %g   xscale: %g\n", scaled_length, xscale);
        
        
        fsf *= xscale;
    }
    
    xscale *= g_ChartScaleFactorExp;
    
    //  Special case for GEO_AREA objects with centred symbols
    if( rzRules->obj->Primitive_type == GEO_AREA ) {
        wxPoint r;
        GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );
        
        double latdraw, londraw;                // position of the drawn symbol with pivot applied
        GetPixPointSingleNoRotate( r.x + ((prule->pos.symb.pivot_x.SYCL - prule->pos.symb.bnbox_x.SBXC) / fsf),
                                   r.y + ((prule->pos.symb.pivot_y.SYRW - prule->pos.symb.bnbox_y.SBXR) / fsf),
                                   &latdraw, &londraw, vp );
        
        if( !rzRules->obj->BBObj.Contains( latdraw, londraw ) ) // Symbol reference point is outside base area object
             return 1;
    }
    
    double render_angle = rot_angle;
    
    //  Very special case for ATON flare lights at 135 degrees, the standard render angle.
    //  We don't want them to rotate with the viewport.
    if(rzRules->obj->bIsAton && (!strncmp(rzRules->obj->FeatureName, "LIGHTS", 6))  && (fabs(rot_angle - 135.0) < 1.) ){
        render_angle -= vp->rotation * 180./PI;
        
        //  And, due to popular request, we make the flare lights a little bit smaller than S52 specifications
        xscale = xscale * 6. / 7.;
    }
    
    int width = prule->pos.symb.bnbox_x.SBXC + prule->pos.symb.bnbox_w.SYHL;
    width *= 4; // Grow the drawing bitmap to allow for rotation of symbols with highly offset pivot points
    width = (int) ( width / fsf );

    int height = prule->pos.symb.bnbox_y.SBXR + prule->pos.symb.bnbox_h.SYVL;
    height *= 4;
    height = (int) ( height / fsf );

    int origin_x = prule->pos.symb.bnbox_x.SBXC;
    int origin_y = prule->pos.symb.bnbox_y.SBXR;
    wxPoint origin(origin_x, origin_y);
    
    int pivot_x = prule->pos.symb.pivot_x.SYCL;
    int pivot_y = prule->pos.symb.pivot_y.SYRW;
    wxPoint pivot( pivot_x, pivot_y );
    
    char *str = prule->vector.LVCT;
    char *col = prule->colRef.LCRF;
    wxPoint r0( (int) ( pivot_x / fsf ), (int) ( pivot_y / fsf ) );

    HPGL->SetVP(vp);
    
    if( !m_pdc ) { // OpenGL Mode, do a direct render
        HPGL->SetTargetOpenGl();
        HPGL->Render( str, col, r, pivot, origin, xscale, render_angle, true );

        //  Update the object Bounding box
        //  so that subsequent drawing operations will redraw the item fully

        int r_width = prule->pos.symb.bnbox_w.SYHL;
        r_width = (int) ( r_width / fsf );
        int r_height = prule->pos.symb.bnbox_h.SYVL;
        r_height = (int) ( r_height / fsf );
        int maxDim = wxMax(r_height, r_width);
        
        double latmin, lonmin, latmax, lonmax;
        GetPixPointSingleNoRotate( r.x - maxDim, r.y + maxDim, &latmin, &lonmin, vp );
        GetPixPointSingleNoRotate( r.x + maxDim, r.y - maxDim, &latmax,  &lonmax, vp );
        LLBBox symbox;
        symbox.Set( latmin, lonmin, latmax, lonmax );
        
        rzRules->obj->BBObj.Expand( symbox );
        
    } else {

#if (( defined(__WXGTK__) || defined(__WXMAC__) ) && !wxCHECK_VERSION(2,9,4))
        wxBitmap *pbm = new wxBitmap( width, height );
#else
        wxBitmap *pbm = new wxBitmap( width, height, 32 );
# if !wxCHECK_VERSION(2,9,4)
        pbm->UseAlpha();
# endif
#endif
        wxMemoryDC mdc( *pbm );
        if(!mdc.IsOk()){
            wxString msg;
            msg.Printf(_T("RenderHPGL: width %d  height %d"), width, height);
            wxLogMessage(msg);
            return false;
        }
        
#if wxUSE_GRAPHICS_CONTEXT
        wxGCDC gdc( mdc );
        HPGL->SetTargetGCDC( &gdc );
#else
        wxMemoryDC &gdc( mdc );
        HPGL->SetTargetDC( &gdc );
#endif
        HPGL->Render( str, col, r0, pivot, origin, xscale, (double) rot_angle, true );

        int bm_width = ( gdc.MaxX() - gdc.MinX() ) + 4;
        int bm_height = ( gdc.MaxY() - gdc.MinY() ) + 4;
        int bm_orgx = wxMax ( 0, gdc.MinX()-2 );
        int bm_orgy = wxMax ( 0, gdc.MinY()-2 );
        int screenOriginX = r.x + ( bm_orgx - (int) ( pivot_x / fsf ) );
        int screenOriginY = r.y + ( bm_orgy - (int) ( pivot_y / fsf ) );

        //      Pre-clip the sub-bitmap to avoid assert errors
        if( ( bm_height + bm_orgy ) > height ) bm_height = height - bm_orgy;
        if( ( bm_width + bm_orgx ) > width ) bm_width = width - bm_orgx;

        mdc.SelectObject( wxNullBitmap );

        //  Grab a copy of the existing screen DC rectangle
        wxBitmap targetBm( bm_width, bm_height, 24 );
        wxMemoryDC targetDc( targetBm );
        if(!targetDc.IsOk())
            return false;
        targetDc.Blit( 0, 0, bm_width, bm_height, m_pdc, screenOriginX, screenOriginY );
        

#if wxUSE_GRAPHICS_CONTEXT /*&& (( defined(__WXGTK__) || defined(__WXMAC__) ) && !wxCHECK_VERSION(2,9,4))*/
        //  Re-render onto the screen-grab copy, since wxDC::DrawBitmap() for alpha channel bitmaps is broken somehow in wxGCDC
        wxGCDC targetGcdc( targetDc );
        r0 -= wxPoint( bm_orgx, bm_orgy );
        HPGL->SetTargetGCDC( &targetGcdc );
        HPGL->Render( str, col, r0, pivot,origin, xscale, (double) rot_angle, true );
#else
        //  We can use the bitmap already rendered
        //  Get smallest containing bitmap
        wxBitmap *sbm = new wxBitmap( pbm->GetSubBitmap( wxRect( bm_orgx, bm_orgy, bm_width, bm_height ) ) );
    
        //  render the symbol graphics onto the screen-grab copy, with transparency...
        targetDc.DrawBitmap( *sbm, 0, 0 );
        delete sbm;
#endif
        
        //  Render the final bitmap onto the screen DC
        m_pdc->Blit( screenOriginX, screenOriginY, bm_width, bm_height, &targetDc, 0, 0 );

        // Clean up
        delete pbm;
        targetDc.SelectObject( wxNullBitmap );

        //  Update the object Bounding box
        //  so that subsequent drawing operations will redraw the item fully

        double latmin, lonmin, latmax, lonmax;
        GetPixPointSingleNoRotate( r.x + prule->parm2, r.y + prule->parm3 + bm_height, &latmin, &lonmin, vp );
        GetPixPointSingleNoRotate( r.x + prule->parm2 + bm_width, r.y + prule->parm3, &latmax,  &lonmax, vp );
        LLBBox symbox;
        symbox.Set( latmin, lonmin, latmax, lonmax );

        rzRules->obj->BBObj.Expand( symbox );
    }

    return true;
}

//-----------------------------------------------------------------------------------------
//      Instantiate a Symbol or Pattern stored as XBM ascii in a rule
//      Producing a wxImage
//-----------------------------------------------------------------------------------------
wxImage s52plib::RuleXBMToImage( Rule *prule )
{
    //      Decode the color definitions
    wxArrayPtrVoid *pColorArray = new wxArrayPtrVoid;

    int i = 0;
    char *cstr = prule->colRef.SCRF;

    char colname[6];
    int nl = strlen( cstr );

    while( i < nl ) {
        i++;

        strncpy( colname, &cstr[i], 5 );
        colname[5] = 0;
        S52color *pColor = getColor( colname );

        pColorArray->Add( (void *) pColor );

        i += 5;
    }

    //      Get geometry
    int width = prule->pos.line.bnbox_w.SYHL;
    int height = prule->pos.line.bnbox_h.SYVL;

    wxString gstr( *prule->bitmap.SBTM ); // the bit array

    wxImage Image( width, height );

    for( int iy = 0; iy < height; iy++ ) {
        wxString thisrow = gstr( iy * width, width ); // extract a row

        for( int ix = 0; ix < width; ix++ ) {
            int cref = (int) ( thisrow[ix] - 'A' ); // make an index
            if( cref >= 0 ) {
                S52color *pthisbitcolor = (S52color *) ( pColorArray->Item( cref ) );
                Image.SetRGB( ix, iy, pthisbitcolor->R, pthisbitcolor->G, pthisbitcolor->B );
            } else {
                Image.SetRGB( ix, iy, m_unused_color.R, m_unused_color.G, m_unused_color.B );
            }

        }
    }

    pColorArray->Clear();
    delete pColorArray;
    return Image;
}

//
//      Render Raster Symbol
//      Symbol is instantiated as a bitmap the first time it is needed
//      and re-built on color scheme change
//
bool s52plib::RenderRasterSymbol( ObjRazRules *rzRules, Rule *prule, wxPoint &r, ViewPort *vp,
                                  float rot_angle )
{
    double scale_factor = 1.0;
 
    scale_factor *=  g_ChartScaleFactorExp;
    scale_factor *= g_scaminScale;
    
    if(m_display_size_mm < 200){                //about 8 inches, implying some sort of smaller mobile device
        //  Set the onscreen size of the symbol
        //  Compensate for various display resolutions
        //  Develop empirically, making a buoy about 4 mm tall
        double boyHeight = 21. / GetPPMM();           // from raster symbol definitions, boylat is xx pix high
        
        double targetHeight0 = 4.0;  
        
        // But we want to scale the size for smaller displays
        double displaySize = m_display_size_mm;
        displaySize = wxMax(displaySize, 100);
        
        float targetHeight = wxMin(targetHeight0, displaySize / 30);
        
        double pix_factor = targetHeight / boyHeight;
        
        //qDebug() << "scaleing" << m_display_size_mm  << targetHeight0 << targetHeight << GetPPMM() << boyHeight << pix_factor;
        
        // for Hubert, and my moto 
        //scaleing 93.98 93 4 3.33333 12.7312 1.64949 2.02082
        // My nvidia tab
        //scaleing 144.78 144 4 4 12.6667 1.65789 2.4127
        // judgement: all OK
        
        
        scale_factor *= pix_factor;
    }
    
    if(g_oz_vector_scale && vp->b_quilt){
        double sfactor = vp->ref_scale/vp->chart_scale;
        scale_factor = wxMax((sfactor - g_overzoom_emphasis_base)  / 4., scale_factor);
        scale_factor = wxMin(scale_factor, 20);
    }

    // a few special cases here
    if( !strncmp(rzRules->obj->FeatureName, "notmrk", 6 )
        || !strncmp(rzRules->obj->FeatureName, "NOTMRK", 6)
        || !strncmp(prule->name.SYNM, "ADDMRK", 6)    
        )
    {
        // get the symbol size
        wxRect trect;
        ChartSymbols::GetGLTextureRect( trect, prule->name.SYNM );
        
        int scale_dim = wxMax(trect.width, trect.height);
        
        double scaled_size = scale_dim / vp->view_scale_ppm;
        
        double target_size = 100;               // roughly, meters maximum scaled size for these inland signs
        
        double xscale = target_size / scaled_size;
        xscale = wxMin(xscale, 1.0);
        xscale = wxMax(.2, xscale);
        
        scale_factor *= xscale;
    }
    
    int pivot_x = prule->pos.line.pivot_x.SYCL;
    int pivot_y = prule->pos.line.pivot_y.SYRW;

    pivot_x *= scale_factor;
    pivot_y *= scale_factor;
    
    // For opengl, hopefully the symbols are loaded in a texture
    unsigned int texture = 0;
    wxRect texrect;
    if(!m_pdc) {
      texture = ChartSymbols::GetGLTextureRect(texrect, prule->name.SYNM);
      if(texture) {
          prule->parm2 = texrect.width * scale_factor;
          prule->parm3 = texrect.height * scale_factor;
      }
    }
    
    if( m_pdc || !texture ) {

        //    Check to see if any cached data is valid
        bool b_dump_cache = false;
        if( prule->pixelPtr ) {
            
            // Detect switches between DC and GL modes, flush if switch occurs
            if( m_pdc ) {
                if( prule->parm0 != ID_wxBitmap ) b_dump_cache = true;
            } else {
                if( prule->parm0 != ID_RGBA ) b_dump_cache = true;
            }
        }

        // If the requested scaled symbol size is not the same as is currently cached, 
        // we have to dump the cache
        wxRect trect;
        ChartSymbols::GetGLTextureRect( trect, prule->name.SYNM );
        if(prule->parm2 != trect.width * scale_factor)
            b_dump_cache = true;
        
        wxBitmap *pbm = NULL;
        wxImage Image;

        //Instantiate the symbol if necessary
        if( ( prule->pixelPtr == NULL ) || ( prule->parm1 != m_colortable_index ) || b_dump_cache ) {
            Image = useLegacyRaster ?
                RuleXBMToImage( prule ) : ChartSymbols::GetImage( prule->name.SYNM );

            // delete any old private data
            ClearRulesCache( prule );

            // always display something, TMARDEF1 as width of 2
            int w0 = wxMax(1, Image.GetWidth() * scale_factor);
            int h0 = wxMax(1, Image.GetHeight() * scale_factor);
            Image.Rescale(w0 , h0 , wxIMAGE_QUALITY_HIGH);
            
            int w = Image.GetWidth();
            int h = Image.GetHeight();

            if( !m_pdc )          // opengl, not using textures
            {
                //    Get the glRGBA format data from the wxImage
                unsigned char *d = Image.GetData();
                unsigned char *a = Image.GetAlpha();
                
                Image.SetMaskColour( m_unused_wxColor.Red(), m_unused_wxColor.Green(),
                                     m_unused_wxColor.Blue() );
                unsigned char mr, mg, mb;
                if( !a && !Image.GetOrFindMaskColour( &mr, &mg, &mb ) )
                    printf( "trying to use mask to draw a bitmap without alpha or mask\n" );
                
                unsigned char *e = (unsigned char *) malloc( w * h * 4 );
                // XXX FIXME a or e ?
                if( d && a){
                    for( int y = 0; y < h; y++ ) {
                        for( int x = 0; x < w; x++ ) {
                            unsigned char r, g, b;
                            int off = ( y * w + x );
                            r = d[off * 3 + 0];
                            g = d[off * 3 + 1];
                            b = d[off * 3 + 2];
                            
                            e[off * 4 + 0] = r;
                            e[off * 4 + 1] = g;
                            e[off * 4 + 2] = b;
                            
                            e[off * 4 + 3] =
                                a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                        }
                    }
                }
                
                //      Save the bitmap ptr and aux parms in the rule
                prule->pixelPtr = e;
                prule->parm0 = ID_RGBA;
                prule->parm1 = m_colortable_index;
                prule->parm2 = w;
                prule->parm3 = h;
            } else {
                //      Make the masked Bitmap
                if( useLegacyRaster ) {
                    pbm = new wxBitmap( Image );
                    wxMask *pmask = new wxMask( *pbm, m_unused_wxColor );
                    pbm->SetMask( pmask );
                }
                
                bool b_has_trans = false;
#if (defined(__WXGTK__) || defined(__WXMAC__))
                
                //    Blitting of wxBitmap with transparency in wxGTK is broken....
                //    We can do it the hard way, by manually alpha blending the
                //    symbol with a clip taken from the current screen DC contents.
                
                //    Inspect the symbol image, to see if it actually has alpha transparency
                if(Image.HasAlpha())
                {
                    unsigned char *a = Image.GetAlpha();
                    for(int i = 0; i < Image.GetHeight(); i++, a++)
                    {
                        for(int j = 0; j < Image.GetWidth(); j++)
                        {
                            if((*a) && (*a != 255)) {
                                b_has_trans = true;
                                break;
                            }
                        }
                        if(b_has_trans)
                            break;
                    }
                }
#ifdef __WXMAC__
                b_has_trans = true;
#endif
                
                //    If the symbol image has no transparency, then a standard wxDC:Blit() will work
                if(!b_has_trans) {
                    pbm = new wxBitmap( Image, -1 );
                    wxMask *pmask = new wxMask( *pbm, m_unused_wxColor );
                    pbm->SetMask( pmask );
                }
                
#else
                if( !useLegacyRaster ) {
                    pbm = new wxBitmap( Image, 32 );                // windows
                    wxMask *pmask = new wxMask( *pbm, m_unused_wxColor );
                    pbm->SetMask( pmask );
                }
#endif
                
                //      Save the bitmap ptr and aux parms in the rule
                prule->pixelPtr = pbm;
                prule->parm0 = ID_wxBitmap;
                prule->parm1 = m_colortable_index;
                prule->parm2 = w;
                prule->parm3 = h;
                
            }
        }               // instantiation
    }

    //        Get the bounding box for the to-be-drawn symbol
    int b_width, b_height;
    b_width = prule->parm2;
    b_height = prule->parm3;

    LLBBox symbox;
    double latmin, lonmin, latmax, lonmax;

    if( !m_pdc && fabs( vp->rotation ) > .01)          // opengl
    {
        float cx = vp->pix_width/2.;
        float cy = vp->pix_height/2.;
        float c = cosf(vp->rotation );
        float s = sinf(vp->rotation );
        float x = r.x - pivot_x -cx;
        float y = r.y - pivot_y + b_height -cy;
        GetPixPointSingle( x*c - y*s +cx, x*s + y*c +cy, &latmin, &lonmin, vp );

        x = r.x - pivot_x + b_width -cx;
        y = r.y - pivot_y -cy;
        GetPixPointSingle( x*c - y*s +cx, x*s + y*c +cy, &latmax, &lonmax, vp );
    } else {
        GetPixPointSingle( r.x - pivot_x, r.y - pivot_y + b_height, &latmin, &lonmin, vp );
        GetPixPointSingle( r.x - pivot_x + b_width, r.y - pivot_y, &latmax, &lonmax, vp );
    }
    symbox.Set( latmin, lonmin, latmax, lonmax );

    //  Special case for GEO_AREA objects with centred symbols
     if( rzRules->obj->Primitive_type == GEO_AREA ) {
         if( !rzRules->obj->BBObj.IntersectIn( symbox ) ) // Symbol is wholly outside base object
             return true;
     }

    //      Now render the symbol

    if( !m_pdc )          // opengl
    {
#ifdef ocpnUSE_GL
        glEnable( GL_BLEND );
        
        if(texture) {
            extern GLenum       g_texture_rectangle_format;

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture);

            int w = texrect.width, h = texrect.height;
            
            float tx1 = texrect.x, ty1 = texrect.y;
            float tx2 = tx1 + w, ty2 = ty1 + h;

#ifndef USE_ANDROID_GLES2
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
            if(g_texture_rectangle_format == GL_TEXTURE_2D) {
                wxSize size = ChartSymbols::GLTextureSize();
                tx1 /= size.x, tx2 /= size.x;
                ty1 /= size.y, ty2 /= size.y;
            }
            
            {
                glPushMatrix();

                glTranslatef(r.x, r.y, 0);
                glRotatef(vp->rotation * 180/PI, 0, 0, -1);
                glTranslatef(-pivot_x, -pivot_y, 0);
                glScalef(scale_factor, scale_factor, 1);
                
                glBegin(GL_QUADS);
                    glTexCoord2f(tx1, ty1);    glVertex2i( 0, 0);
                    glTexCoord2f(tx2, ty1);    glVertex2i( w, 0);
                    glTexCoord2f(tx2, ty2);    glVertex2i( w, h);
                    glTexCoord2f(tx1, ty2);    glVertex2i( 0, h);
                glEnd();
                
                glPopMatrix();
            }
#else

                
            if(g_texture_rectangle_format == GL_TEXTURE_2D) {
                    
                // Normalize the sybmol texture coordinates against the next higher POT size
                wxSize size = ChartSymbols::GLTextureSize();
#if 0
                                int i=1;
                                while(i < size.x) i <<= 1;
                                int rb_x = i;
                    
                                i=1;
                                while(i < size.y) i <<= 1;
                                int rb_y = i;
                    
#else                
                int rb_x = size.x;
                int rb_y = size.y;
#endif
                //qDebug() << "texnorm" << rb_x << rb_y;
                tx1 /= rb_x , tx2 /= rb_x ;
                ty1 /= rb_y , ty2 /= rb_y ;
                }
                
            float uv[8];
            float coords[8];

            // Note swizzle of points to allow TRIANGLE_STRIP drawing
            //normal uv
            uv[0] = tx1; uv[1] = ty1; uv[2] = tx2; uv[3] = ty1;
            uv[6] = tx2; uv[7] = ty2; uv[4] = tx1; uv[5] = ty2;

            w *= scale_factor;
            h *= scale_factor;

            // pixels
            coords[0] = 0; coords[1] = 0; coords[2] = w; coords[3] = 0;
            coords[6] = w; coords[7] = h; coords[4] = 0; coords[5] = h;

            glUseProgram( S52texture_2D_shader_program );

            // Get pointers to the attributes in the program.
            GLint mPosAttrib = glGetAttribLocation( S52texture_2D_shader_program, "position" );
            GLint mUvAttrib  = glGetAttribLocation( S52texture_2D_shader_program, "aUV" );

            // Select the active texture unit.
            glActiveTexture( GL_TEXTURE0 );

            // Bind our texture to the texturing target.
            glBindTexture( GL_TEXTURE_2D, texture );

            // Set up the texture sampler to texture unit 0
            GLint texUni = glGetUniformLocation( S52texture_2D_shader_program, "uTex" );
            glUniform1i( texUni, 0 );

            // Disable VBO's (vertex buffer objects) for attributes.
            glBindBuffer( GL_ARRAY_BUFFER, 0 );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

            // Set the attribute mPosAttrib with the vertices in the screen coordinates...
            glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
            // ... and enable it.
            glEnableVertexAttribArray( mPosAttrib );

            // Set the attribute mUvAttrib with the vertices in the GL coordinates...
            glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uv );
            // ... and enable it.
            glEnableVertexAttribArray( mUvAttrib );

            // Rotate
            mat4x4 I, Q;
            mat4x4_identity(I);

            mat4x4_translate_in_place(I, r.x, r.y, 0);
            mat4x4_rotate_Z(Q, I, -vp->rotation);
            mat4x4_translate_in_place(Q, -pivot_x, -pivot_y, 0);


            GLint matloc = glGetUniformLocation(S52texture_2D_shader_program,"TransformMatrix");
            glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);

            // Perform the actual drawing.
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // Restore the per-object transform to Identity Matrix
            mat4x4 IM;
            mat4x4_identity(IM);
            GLint matlocf = glGetUniformLocation(S52texture_2D_shader_program,"TransformMatrix");
            glUniformMatrix4fv( matlocf, 1, GL_FALSE, (const GLfloat*)IM);


#endif          // GLES2
            glDisable(g_texture_rectangle_format);
        } else { /* this is only for legacy mode, or systems without NPOT textures */
            float cr = cosf( vp->rotation );
            float sr = sinf( vp->rotation );
            float ddx = pivot_x * cr + pivot_y * sr;
            float ddy = pivot_y * cr - pivot_x * sr;
#ifndef USE_ANDROID_GLES2
            glColor4f( 1, 1, 1, 1 );

            //  Since draw pixels is so slow, lets not draw anything we don't have to
            wxRect sym_rect(r.x - ddx, r.y - ddy, b_width, b_height);
            if(vp->rv_rect.Intersects(sym_rect) ) {
                
                glPushAttrib( GL_SCISSOR_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                
                glDisable( GL_SCISSOR_TEST );
                glDisable( GL_STENCIL_TEST );
                glDisable( GL_DEPTH_TEST );
                
                glRasterPos2f( r.x - ddx, r.y - ddy );
                glPixelZoom( 1, -1 );
                glDrawPixels( b_width, b_height, GL_RGBA, GL_UNSIGNED_BYTE, prule->pixelPtr );
                glPixelZoom( 1, 1 );

                glPopAttrib();
            }
#endif
        }

        glDisable( GL_BLEND );
#endif
    } else {

        if( !( prule->pixelPtr ) )                // This symbol requires manual alpha blending
        {
            //    Don't bother if the symbol is off the true screen,
            //    as for instance when an area-centered symbol is called for.
            if( ( r.x - pivot_x  < vp->pix_width ) && ( r.y - pivot_y < vp->pix_height ) ) {
                
                // Get the current screen contents to a wxImage
                wxBitmap b1( b_width, b_height, -1 );
                wxMemoryDC mdc1( b1 );
                mdc1.Blit( 0, 0, b_width, b_height, m_pdc, r.x - pivot_x, r.y - pivot_y, wxCOPY );
                wxImage im_back = b1.ConvertToImage();

                //    Get the scaled symbol as a wxImage
                wxImage im_sym = ChartSymbols::GetImage( prule->name.SYNM );
                im_sym.Rescale(b_width, b_height, wxIMAGE_QUALITY_HIGH);

                wxImage im_result( b_width, b_height );
                unsigned char *pdest = im_result.GetData();
                unsigned char *pback = im_back.GetData();
                unsigned char *psym = im_sym.GetData();

                unsigned char *asym = NULL;
                if( im_sym.HasAlpha() )
                    asym = im_sym.GetAlpha();

                //    Do alpha blending, the hard way

                if(pdest && psym && pback){
                    for( int i = 0; i < b_height; i++ ) {
                        for( int j = 0; j < b_width; j++ ) {
                            double alpha = 1.0;
                            if(asym)
                                alpha = ( *asym++ ) / 256.0;
                            unsigned char r = ( *psym++ * alpha ) + ( *pback++ * ( 1.0 - alpha ) );
                            *pdest++ = r;
                            unsigned char g = ( *psym++ * alpha ) + ( *pback++ * ( 1.0 - alpha ) );
                            *pdest++ = g;
                            unsigned char b = ( *psym++ * alpha ) + ( *pback++ * ( 1.0 - alpha ) );
                            *pdest++ = b;
                        }
                    }
                }

                wxBitmap result( im_result );
                wxMemoryDC result_dc( result );

                m_pdc->Blit( r.x - pivot_x, r.y - pivot_y, b_width, b_height, &result_dc, 0, 0,  wxCOPY, false );

                result_dc.SelectObject( wxNullBitmap );
                mdc1.SelectObject( wxNullBitmap );
            }
        } else {
            //      Get the symbol bitmap into a memory dc
            wxBitmap &bmp = (wxBitmap &) ( *( (wxBitmap *) ( prule->pixelPtr ) ) );
            wxMemoryDC mdc(bmp);

            //      Blit it into the target dc, with mask
            m_pdc->Blit( r.x - pivot_x, r.y - pivot_y, bmp.GetWidth(), bmp.GetHeight(), &mdc, 0, 0, wxCOPY, true );

            mdc.SelectObject( wxNullBitmap );

        }
// Debug
//if(m_pdc){
//m_pdc->SetPen(wxPen(*wxGREEN, 1));
//m_pdc->SetBrush(wxBrush(*wxGREEN, wxTRANSPARENT));
//m_pdc->DrawRectangle(r.x - pivot_x, r.y - pivot_y, b_width, b_height);
//}

    }

    //  Update the object Bounding box
    //  so that subsequent drawing operations will redraw the item fully
    //  We expand the object's BBox to account for objects rendered by multiple symbols, such as SOUNGD.
    //  so that expansions are cumulative.
    if( rzRules->obj->Primitive_type == GEO_POINT )
        rzRules->obj->BBObj.Expand( symbox );

    //  Dump the cache for next time
    if(g_oz_vector_scale && (scale_factor > 1.0))
        ClearRulesCache( prule );
    
    return true;
}

// SYmbol
int s52plib::RenderSY( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    float angle = 0;
    double orient;

    if( rules->razRule != NULL ) {
        if( rules->INSTstr[8] == ',' ) // supplementary parameter assumed to be angle, seen in LIGHTSXX
                {
            char sangle[10];
            int cp = 0;
            while( rules->INSTstr[cp + 9] && ( rules->INSTstr[cp + 9] != ')' ) ) {
                sangle[cp] = rules->INSTstr[cp + 9];
                cp++;
            }
            sangle[cp] = 0;
            int angle_i = atoi( sangle );
            angle = angle_i;
        }

        if( GetDoubleAttr( rzRules->obj, "ORIENT", orient ) ) // overriding any LIGHTSXX angle, probably TSSLPT
                {
            angle = orient;
            if( strncmp( rzRules->obj->FeatureName, "LIGHTS", 6 ) == 0 ) {
                angle += 180;
                if( angle > 360 ) angle -= 360;
            }
        }

        //  Render symbol at object's x/y
        wxPoint r, r1;
        GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

         //  Render a raster or vector symbol, as specified by LUP rules
        if( rules->razRule->definition.SYDF == 'V' ){
            RenderHPGL( rzRules, rules->razRule, r, vp, angle );
        }
        else{
            if( rules->razRule->definition.SYDF == 'R' )
                RenderRasterSymbol( rzRules, rules->razRule, r, vp, angle );
        }
    }

    return 0;

}

// Line Simple Style, OpenGL
int s52plib::RenderGLLS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    // for now don't use vbo model in non-mercator
    if(vp->m_projection_type != PROJECTION_MERCATOR)
        return RenderLS(rzRules, rules, vp);

    if( !m_benableGLLS )                        // root chart cannot support VBO model, for whatever reason
        return RenderLS(rzRules, rules, vp);

#ifndef USE_ANDROID_GLES2    
    double scale_factor = vp->ref_scale/vp->chart_scale;
    if(scale_factor > 10.0)
        return RenderLS(rzRules, rules, vp);
#endif     

    if( !rzRules->obj->m_chart_context->chart )
        return RenderLS(rzRules, rules, vp);    // this is where S63 PlugIn gets caught
    
    if(( vp->GetBBox().GetMaxLon() >= 180.) || (vp->GetBBox().GetMinLon() <= -180.))
        return RenderLS(rzRules, rules, vp);    // cm03 has trouble at IDL        
    
    bool b_useVBO = false;
    float *vertex_buffer = 0;
    
    if(rzRules->obj->auxParm2 > 0)             // Has VBO been defined and uploaded? 
        b_useVBO = true;

    if( !b_useVBO ){
#if 0        
        if( rzRules->obj->m_chart_context->chart ){
            vertex_buffer = rzRules->obj->m_chart_context->chart->GetLineVertexBuffer(); 
        }
        else {
            vertex_buffer = rzRules->obj->m_chart_context->vertex_buffer; 
        }
        
        
        if(!vertex_buffer)
            return RenderLS(rzRules, rules, vp);    // this is where cm93 gets caught
#else
        vertex_buffer = rzRules->obj->m_chart_context->vertex_buffer; 
            
#endif            
    }

    
#ifdef ocpnUSE_GL

    char *str = (char*) rules->INSTstr;

#ifdef USE_ANDROID_GLES2
    if( (!strncmp( str, "DASH", 4 ) ) || ( !strncmp( str, "DOTT", 4 ) ) )
        return RenderLS_Dash_GLSL(rzRules, rules, vp);
#endif


    LLBBox BBView = vp->GetBBox();

    //  Allow a little slop in calculating whether a segment
    //  is within the requested Viewport
    double margin = BBView.GetLonRange() * .05;
    BBView.EnLarge( margin );

    //  Try to determine if the feature needs to be drawn in the most efficient way
    //  We need to look at priority and visibility of each segment
    int bdraw = 0;
    
    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    
    line_segment_element *ls_list = rzRules->obj->m_ls_list;
    
    S52color *c = getColor( str + 7 ); // Colour
    int w = atoi( str + 5 ); // Width
    
#ifndef ocpnUSE_GLES // linestipple is emulated poorly
    glColor3ub( c->R, c->G, c->B );
#endif
    
    //    Set drawing width
    float lineWidth = w;
    
//    float parms[2];
//     if( w > 1 ) {
//         if( w > parms[1] )
//             lineWidth = wxMax(g_GLMinCartographicLineWidth, parms[1]);
//         else
//             lineWidth = wxMax(g_GLMinCartographicLineWidth, w);
//     } else
        lineWidth = wxMax(g_GLMinCartographicLineWidth, 1);

    // Manage super high density displays
    float target_w_mm = 0.5 * w;
    if(GetPPMM() > 7){               // arbitrary
        target_w_mm = ((float)w) / 6.0;  // Target width in mm
                                               //  The value "w" comes from S52 library CNSY procedures, in "nominal" pixels
                                               // the value "6" comes from semi-standard LCD display densities
                                               // or something like 0.18 mm pitch, or 6 pix per mm.
        lineWidth =  wxMax(g_GLMinCartographicLineWidth, target_w_mm * GetPPMM());
    }

    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );
    
#ifdef __OCPN__ANDROID__
//     if( w > 1 )
//         lineWidth = wxMin(lineWidth, parms[1]);
    glLineWidth(lineWidth);
    
#else    
    glLineWidth(lineWidth);
    if(lineWidth > 4.0 && m_GLLineSmoothing){
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
    }
#endif
    
#ifndef ocpnUSE_GLES // linestipple is emulated poorly
    if( !strncmp( str, "DASH", 4 ) ) {
        glLineStipple( 1, 0x3F3F );
        glEnable( GL_LINE_STIPPLE );
    }
    else if( !strncmp( str, "DOTT", 4 ) ) {
        glLineStipple( 1, 0x3333 );
        glEnable( GL_LINE_STIPPLE );
    }
    else
        glDisable( GL_LINE_STIPPLE );
#endif    
        
    GLuint textureDot = -1;


#ifndef USE_ANDROID_GLES2
    glColor3ub( c->R, c->G, c->B );

    glPushMatrix();
    
    // Set up the OpenGL transform matrix for this object
    //  Transform from Simple Mercator (relative to chart reference point) to screen coordinates.
    
    //  First, the VP transform
    glTranslatef( vp->pix_width / 2, vp->pix_height/2, 0 );
    glScalef( vp->view_scale_ppm, -vp->view_scale_ppm, 0 );
    glTranslatef( -rzRules->sm_transform_parms->easting_vp_center, -rzRules->sm_transform_parms->northing_vp_center, 0 );
    
    //  Next, the per-object transform
    if( rzRules->obj->m_chart_context->chart ){
        glTranslatef( rzRules->obj->x_origin, rzRules->obj->y_origin, 0);
        glScalef( rzRules->obj->x_rate, rzRules->obj->y_rate, 0 );
    }

    glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array

#endif

    //   Has line segment PBO been allocated for this chart?
    if(b_useVBO){
        glBindBuffer(GL_ARRAY_BUFFER, rzRules->obj->auxParm2);
    }

    
#ifdef USE_ANDROID_GLES2
    glUseProgram(S52color_tri_shader_program);

    // Disable VBO's (vertex buffer objects) for attributes.
    if(!b_useVBO)
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");

    float angle = 0;

    // We cannot use the prepared shader uniforms, as we can (and should for performance) do the per-object transforms 
    // all at once in the shader matrix. 
    // But we also must restore the prepared matrix for later rendering of other object classes.
    
    // Build Transform matrix
    //  First, the VP transform
    mat4x4 I, Q;
    mat4x4_identity(I);

    // Scale per object
    I[0][0] *= rzRules->obj->x_rate;
    I[1][1] *= rzRules->obj->y_rate;

    // Translate per object
    I[3][0] = -(rzRules->sm_transform_parms->easting_vp_center - rzRules->obj->x_origin) * vp->view_scale_ppm ;
    I[3][1] = -(rzRules->sm_transform_parms->northing_vp_center - rzRules->obj->y_origin) * -vp->view_scale_ppm ;

    // Scale for screen
    I[0][0] *= vp->view_scale_ppm;
    I[1][1] *= -vp->view_scale_ppm;

    //Rotate
    mat4x4_rotate_Z(Q, I, angle);

    // Translate for screen
    Q[3][0] += vp->pix_width / 2;
    Q[3][1] += vp->pix_height / 2;

    GLint matloc = glGetUniformLocation(S52color_tri_shader_program,"TransformMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);

    float colorv[4];
    colorv[0] = c->R / float(256);
    colorv[1] = c->G / float(256);
    colorv[2] = c->B / float(256);
    colorv[3] = 1.0;

    GLint colloc = glGetUniformLocation(S52color_tri_shader_program,"color");
    glUniform4fv(colloc, 1, colorv);

    if(!b_useVBO){
        unsigned char *buffer = (unsigned char *)vertex_buffer;
        float *bufBase = (float *)buffer;
        glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), bufBase);
        glEnableVertexAttribArray(pos);
    }

#endif
  
    // from above ls_list is the first drawable segment
    while( ls_list){
        
        if( ls_list->priority == priority_current  )   
        {
            size_t seg_vbo_offset = 0;
            size_t point_count = 0;
            
            //  Check visibility of the segment
            bool b_drawit = false;
            if( (ls_list->ls_type == TYPE_EE) || (ls_list->ls_type == TYPE_EE_REV) ){
//                 if((BBView.GetMinLat() < ls_list->pedge->edgeBBox.GetMaxLat() && BBView.GetMaxLat() > ls_list->pedge->edgeBBox.GetMinLat()) &&
//                     ((BBView.GetMinLon() <= ls_list->pedge->edgeBBox.GetMaxLon() && BBView.GetMaxLon() >= ls_list->pedge->edgeBBox.GetMinLon()) ||
//                     (BBView.GetMaxLon() >=  180 && BBView.GetMaxLon() - 360 > ls_list->pedge->edgeBBox.GetMinLon()) ||
//                     (BBView.GetMinLon() <= -180 && BBView.GetMinLon() + 360 < ls_list->pedge->edgeBBox.GetMaxLon())))
                {
                    // render the segment
                        b_drawit = true;
                        seg_vbo_offset = ls_list->pedge->vbo_offset;
                        point_count = ls_list->pedge->nCount;
                 }
                    
            }
            else{
//                 if((BBView.GetMinLat() < ls_list->pcs->cs_lat_avg && BBView.GetMaxLat() > ls_list->pcs->cs_lat_avg) &&
//                     ((BBView.GetMinLon() <= ls_list->pcs->cs_lon_avg && BBView.GetMaxLon() >= ls_list->pcs->cs_lon_avg) ||
//                     (BBView.GetMaxLon() >=  180 && BBView.GetMaxLon() - 360 > ls_list->pcs->cs_lon_avg) ||
//                     (BBView.GetMinLon() <= -180 && BBView.GetMinLon() + 360 < ls_list->pcs->cs_lon_avg)))
                {
                    // render the segment
                        b_drawit = true;
                        seg_vbo_offset = ls_list->pcs->vbo_offset;
                        point_count = 2;
                 }
            }
            
            
            
            
            
            if( b_drawit) {
                // render the segment
                
#ifndef USE_ANDROID_GLES2
                if(b_useVBO){
                    glVertexPointer(2, GL_FLOAT, 2 * sizeof(float), (GLvoid *)(seg_vbo_offset));
                    glDrawArrays(GL_LINE_STRIP, 0, point_count);
                }
                else{
                    glVertexPointer(2, GL_FLOAT, 2 * sizeof(float), (unsigned char *)vertex_buffer + seg_vbo_offset);
                    glDrawArrays(GL_LINE_STRIP, 0, point_count);
                }
#else

//                unsigned char *buffer = (unsigned char *)vertex_buffer + seg_vbo_offset;
//                float *bufBase = (float *)buffer;
//                glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), bufBase);
//                glEnableVertexAttribArray(pos);

                if(b_useVBO){
                    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (GLvoid *)(seg_vbo_offset));
                    glEnableVertexAttribArray(pos);
                    glDrawArrays(GL_LINE_STRIP, 0, point_count);
                }
                else{
#if 1 
                    unsigned char *buffer = (unsigned char *)vertex_buffer;
                    buffer += seg_vbo_offset;
                    float *bufBase = (float *)buffer;
                    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), bufBase);
                    glEnableVertexAttribArray(pos);
                    glDrawArrays(GL_LINE_STRIP, 0, point_count);

#else
                    unsigned char *buffer = (unsigned char *)vertex_buffer;
                    float *bufBase = (float *)buffer;
                    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), bufBase);
                    glEnableVertexAttribArray(pos);
                    glDrawArrays(GL_LINE_STRIP, seg_vbo_offset/(2 * sizeof(float)), point_count);
#endif                    
                }

#endif
            }
        }
        ls_list = ls_list->next;
    }
    
     if(b_useVBO) 
         glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
    
#ifndef USE_ANDROID_GLES2
    glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
    glPopMatrix();
#else
    // Restore shader TransForm Matrix to identity.
    mat4x4 IM;
    mat4x4_identity(IM);
    GLint matlocf = glGetUniformLocation(S52color_tri_shader_program,"TransformMatrix");
    glUniformMatrix4fv( matlocf, 1, GL_FALSE, (const GLfloat*)IM);
#endif


    glDisable( GL_LINE_STIPPLE );
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );
    
#endif                  // OpenGL
    
    return 1;
}
    

// Line Simple Style
int s52plib::RenderLS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    // catch legacy PlugIns (e.g.s63_pi)
    if( rzRules->obj->m_n_lsindex  && !rzRules->obj->m_ls_list) 
        return RenderLSLegacy(rzRules, rules, vp);

    // catch improperly coded edge arrays, usually seen on cm93
    if( !rzRules->obj->m_n_lsindex  && !rzRules->obj->m_ls_list) 
        return 0;
    
    S52color *c;
    int w;

    char *str = (char*) rules->INSTstr;
    c = getColor( str + 7 ); // Colour
    wxColour color( c->R, c->G, c->B );
    w = atoi( str + 5 ); // Width

    double scale_factor = vp->ref_scale/vp->chart_scale;
    double scaled_line_width = wxMax((scale_factor - g_overzoom_emphasis_base), 1);
    bool b_wide_line = g_oz_vector_scale && vp->b_quilt && (scale_factor > g_overzoom_emphasis_base);
    
    wxPen wide_pen(*wxBLACK_PEN);
    wxDash dashw[2];
    dashw[0] = 3;
    dashw[1] = 1; 
    
    if( b_wide_line)
    {
        int w = wxMax(scaled_line_width, 2);            // looks better
        w = wxMin(w, 50);                               // upper bound
        wide_pen.SetWidth( w );
        wide_pen.SetColour(color);
        
        if( !strncmp( str, "DOTT", 4 ) ) {
            dashw[0] = 1;
            wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
            wide_pen.SetDashes( 2, dashw );
        }        
        else if( !strncmp( str, "DASH", 4 ) ){
            wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
            if( m_pdc){ //DC mode
                dashw[0] = 1;
                dashw[1] = 2;
            }
                
            wide_pen.SetDashes( 2, dashw );
        }
    }
 
    wxPen thispen(color, w, wxPENSTYLE_SOLID);
    wxDash dash1[2];
    
    if( m_pdc) //DC mode
    {
        if( !strncmp( str, "DOTT", 4 ) ) {
            thispen.SetStyle(wxPENSTYLE_USER_DASH);
            dash1[0] = 1;
            dash1[1] = 2; 
            thispen.SetDashes( 2, dash1 );
        }        
        else if( !strncmp( str, "DASH", 4 ) ){
            thispen.SetStyle(wxPENSTYLE_SHORT_DASH);
        }
         
        if(b_wide_line)
            m_pdc->SetPen( wide_pen );
        else
            m_pdc->SetPen( thispen );
        
    }
#ifdef ocpnUSE_GL
    else // OpenGL mode
    {
#ifndef ocpnUSE_GLES
        glColor3ub( c->R, c->G, c->B );
#endif
        glDisable( GL_LINE_SMOOTH );
        
        //    Set drawing width
        if( w > 1 ) {
            GLint parms[2];
            glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
            if( w > parms[1] )
                glLineWidth( wxMax(g_GLMinCartographicLineWidth, parms[1]) );
            else
                glLineWidth( wxMax(g_GLMinCartographicLineWidth, w) );
        } else
            glLineWidth( wxMax(g_GLMinCartographicLineWidth, 1) );
        
#ifndef ocpnUSE_GLES // linestipple is emulated poorly
            if( !strncmp( str, "DASH", 4 ) ) {
                glLineStipple( 1, 0x3F3F );
                glEnable( GL_LINE_STIPPLE );
            }
            else if( !strncmp( str, "DOTT", 4 ) ) {
                glLineStipple( 1, 0x3333 );
                glEnable( GL_LINE_STIPPLE );
            }
            else
                glDisable( GL_LINE_STIPPLE );
#endif

#ifndef __OCPN__ANDROID__
            if(w >= 2 && m_GLLineSmoothing){    
                glEnable( GL_LINE_SMOOTH );
                glEnable( GL_BLEND );
            }
#endif
    }
#endif
    

    //    Get a true pixel clipping/bounding box from the vp
    wxPoint pbb = vp->GetPixFromLL( vp->clat, vp->clon );
    int xmin_ = pbb.x - (vp->rv_rect.width / 2) - (4 * scaled_line_width);
    int xmax_ = xmin_ + vp->rv_rect.width + (8 * scaled_line_width);
    int ymin_ = pbb.y - (vp->rv_rect.height / 2) - (4 * scaled_line_width) ;
    int ymax_ = ymin_ + vp->rv_rect.height + (8 * scaled_line_width);

    int x0, y0, x1, y1;

    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    
    if( rzRules->obj->m_ls_list )
    {
        float *ppt;

        unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
        line_segment_element *ls = rzRules->obj->m_ls_list;

#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2
        if( !m_pdc && !b_wide_line)
            glBegin( GL_LINES );
#endif
#endif
        while(ls){
            if( ls->priority == priority_current  ) {  

                int nPoints;
            // fetch the first point
                if( (ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV) ){
                    ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
                    nPoints = ls->pedge->nCount;
                }
                else{
                    ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
                    nPoints = 2;
                }

                wxPoint l;
                GetPointPixSingle( rzRules, ppt[1], ppt[0], &l, vp );
                ppt += 2;
            
                for(int ip=0 ; ip < nPoints - 1 ; ip++){
                    wxPoint r;
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &r, vp );
                            //        Draw the edge as point-to-point
                    x0 = l.x, y0 = l.y;
                    x1 = r.x, y1 = r.y;
                            
                        // Do not draw null segments
                    if( ( x0 != x1 ) || ( y0 != y1 ) ){
                    
                        if(m_pdc){
                        
                            if( cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, xmin_, xmax_,
                                    ymin_, ymax_ ) != Invisible )
                                    m_pdc->DrawLine( x0, y0, x1, y1 );
                        }
#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2
                        else {
                            // simplified faster test, let opengl do the rest
                            if((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                                (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {
                                if(!b_wide_line) {
                                    glVertex2i( x0, y0 );
                                    glVertex2i( x1, y1 );
                                } else
                                    PLIBDrawGLThickLine( x0, y0, x1, y1, wide_pen, true );
                            }
                        }
#endif      
#endif
                    }
                        
                    l = r;
                    ppt += 2;
                }            
            }
            
            ls = ls->next;
        }
#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2
    if(!m_pdc && !b_wide_line)
            glEnd();
#endif              
#endif
    }
    
#ifdef ocpnUSE_GL
    if( !m_pdc ){
        glDisable( GL_LINE_STIPPLE );
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_BLEND );
    }
#endif                
    return 1;
}

// Line Simple Style
int s52plib::RenderLSLegacy( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    if( !rzRules->obj->m_chart_context->chart )
        return RenderLSPlugIn( rzRules, rules, vp );

    // Must be cm93
    S52color *c;
    int w;

    char *str = (char*) rules->INSTstr;
    c = getColor( str + 7 ); // Colour
    wxColour color( c->R, c->G, c->B );
    w = atoi( str + 5 ); // Width

    double scale_factor = vp->ref_scale/vp->chart_scale;
    double scaled_line_width = wxMax((scale_factor - g_overzoom_emphasis_base), 1);
    bool b_wide_line = g_oz_vector_scale && vp->b_quilt && (scale_factor > g_overzoom_emphasis_base);
    
    wxPen wide_pen(*wxBLACK_PEN);
    wxDash dashw[2];
    dashw[0] = 3;
    dashw[1] = 1; 
    
    if( b_wide_line)
    {
        int w = wxMax(scaled_line_width, 2);            // looks better
        w = wxMin(w, 50);                               // upper bound
        wide_pen.SetWidth( w );
        wide_pen.SetColour(color);
        
        if( !strncmp( str, "DOTT", 4 ) ) {
            dashw[0] = 1;
            wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
            wide_pen.SetDashes( 2, dashw );
        }        
        else if( !strncmp( str, "DASH", 4 ) ){
            wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
            if( m_pdc){ //DC mode
                dashw[0] = 1;
                dashw[1] = 2;
            }
                
            wide_pen.SetDashes( 2, dashw );
        }
    }
 
    wxPen thispen(color, w, wxPENSTYLE_SOLID);
    wxDash dash1[2];
    
    if( m_pdc) //DC mode
    {
        if( !strncmp( str, "DOTT", 4 ) ) {
            thispen.SetStyle(wxPENSTYLE_USER_DASH);
            dash1[0] = 1;
            dash1[1] = 2; 
            thispen.SetDashes( 2, dash1 );
        }        
        else if( !strncmp( str, "DASH", 4 ) ){
            thispen.SetStyle(wxPENSTYLE_SHORT_DASH);
        }
         
        if(b_wide_line)
            m_pdc->SetPen( wide_pen );
        else
            m_pdc->SetPen( thispen );
        
    }

#ifdef ocpnUSE_GL
    else // OpenGL mode
    {
#ifndef __OCPN__ANDROID__
        glColor3ub( c->R, c->G, c->B );
#endif
        glDisable( GL_LINE_SMOOTH );
        
        //    Set drawing width
        if( w > 1 ) {
            GLint parms[2];
            glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
            if( w > parms[1] )
                glLineWidth( wxMax(g_GLMinCartographicLineWidth, parms[1]) );
            else
                glLineWidth( wxMax(g_GLMinCartographicLineWidth, w) );
        } else
            glLineWidth( wxMax(g_GLMinCartographicLineWidth, 1) );

#ifndef ocpnUSE_GLES // linestipple is emulated poorly
        if( !strncmp( str, "DASH", 4 ) ) {
            glLineStipple( 1, 0x3F3F );
            glEnable( GL_LINE_STIPPLE );
        }
        else if( !strncmp( str, "DOTT", 4 ) ) {
            glLineStipple( 1, 0x3333 );
            glEnable( GL_LINE_STIPPLE );
        }
        else
            glDisable( GL_LINE_STIPPLE );
#endif

#ifndef __OCPN__ANDROID__
        if(w >= 2 && m_GLLineSmoothing){
            glEnable( GL_LINE_SMOOTH );
            glEnable( GL_BLEND );
        }
#endif
    }
#endif

    //    Get a true pixel clipping/bounding box from the vp
    wxPoint pbb = vp->GetPixFromLL( vp->clat, vp->clon );
    int xmin_ = pbb.x - (vp->rv_rect.width / 2) - (4 * scaled_line_width);
    int xmax_ = xmin_ + vp->rv_rect.width + (8 * scaled_line_width);
    int ymin_ = pbb.y - (vp->rv_rect.height / 2) - (4 * scaled_line_width) ;
    int ymax_ = ymin_ + vp->rv_rect.height + (8 * scaled_line_width);

    int x0, y0, x1, y1;

    
    if( rzRules->obj->m_n_lsindex ) {
        VE_Hash *ve_hash; 
        VC_Hash *vc_hash; 
        ve_hash = (VE_Hash *)rzRules->obj->m_chart_context->m_pve_hash;             // This is cm93 
        vc_hash = (VC_Hash *)rzRules->obj->m_chart_context->m_pvc_hash; 


        //  Get the current display priority
        //  Default comes from the LUP, unless overridden
        int priority_current = rzRules->LUP->DPRI - '0';
        if(rzRules->obj->m_DPRI >= 0)
            priority_current = rzRules->obj->m_DPRI;
        
        int *index_run;
        float *ppt;

        VC_Element *pnode;

#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2
        if(!b_wide_line)
            glBegin( GL_LINES );
#endif

#ifdef USE_ANDROID_GLES2
            glUseProgram(S52color_tri_shader_program);

            float fBuf[4];
            GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");
            glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), fBuf);
            glEnableVertexAttribArray(pos);

            float colorv[4];
            colorv[0] = c->R / float(256);
            colorv[1] = c->G / float(256);
            colorv[2] = c->B / float(256);
            colorv[3] = 1.0;

            GLint colloc = glGetUniformLocation(S52color_tri_shader_program,"color");
            glUniform4fv(colloc, 1, colorv);
#endif

#endif
        for( int iseg = 0; iseg < rzRules->obj->m_n_lsindex; iseg++ ) {
            int seg_index = iseg * 3;
            index_run = &rzRules->obj->m_lsindex_array[seg_index];

            //  Get first connected node
            unsigned int inode = *index_run++;

            //  Get the edge
            unsigned int enode = *index_run++;
            VE_Element *pedge = 0;
            if(enode)
                pedge = (*ve_hash)[enode];

            //  Get last connected node
            unsigned int jnode = *index_run++;

            int nls;
            if(pedge) {
                //  Here we decide to draw or not based on the highest priority seen for this segment
                //  That is, if this segment is going to be drawn at a higher priority later, then "continue", and don't draw it here.
            
                // This logic is not perfectly right for one case:
                // If the segment has only two end connected nodes, and no intermediate edge,
                // then we have no good way to evaluate the priority.
                // This is due to the fact that priority is only precalculated for edge segments, not connector nodes.
                // Only thing to do is take the conservative approach and draw the segment, in this case.
                if( pedge->nCount && pedge->max_priority != priority_current )
                    continue;
                nls = pedge->nCount + 1;
            } else
                nls = 1;

            wxPoint l;
            bool lastvalid = false;
            for( int ipc = 0; ipc < nls + 1; ipc++ ) {
                ppt = 0;
                if( ipc == 0 ) {
                    if( inode ) {
                        pnode = (*vc_hash)[inode];
                        if( pnode )
                            ppt = pnode->pPoint;
                    }
                } else if(ipc == nls) {
                    if( ( jnode ) ) {
                        pnode = (*vc_hash)[jnode];
                        if( pnode )
                            ppt = pnode->pPoint;
                    }
                } else if(pedge)
                    ppt = pedge->pPoints + 2*(ipc-1);
                
                if(ppt) {
                    wxPoint r;
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &r, vp );

                    if(r.x != INVALID_COORD) {
                        if(lastvalid) {
                            //        Draw the edge as point-to-point
                            x0 = l.x, y0 = l.y;
                            x1 = r.x, y1 = r.y;

                            // Do not draw null segments
                            if( ( x0 == x1 ) && ( y0 == y1 ) ) continue;

                            if( m_pdc ) {
                                if( cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, xmin_, xmax_,
                                                                  ymin_, ymax_ ) != Invisible )
                                    m_pdc->DrawLine( x0, y0, x1, y1 );
                            }
#ifdef ocpnUSE_GL
                            else {

#ifdef USE_ANDROID_GLES2

                                fBuf[0] = x0;
                                fBuf[1] = y0;
                                fBuf[2] = x1;
                                fBuf[3] = y1;

                                glDrawArrays(GL_LINES, 0, 2);

#else
                                // simplified faster test, let opengl do the rest
                                if((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                                   (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {
                                    if(!b_wide_line) {
                                        glVertex2i( x0, y0 );
                                        glVertex2i( x1, y1 );
                                    } else
                                        PLIBDrawGLThickLine( x0, y0, x1, y1, wide_pen, true );
                                }
#endif
                            }
#endif      
                        }

                        l = r;
                        lastvalid = true;
                    } else
                        lastvalid = false;
                } else
                    lastvalid = false;
            }
        }
#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2
        if(!b_wide_line)
            glEnd();
#endif              
#endif
    }
#ifdef ocpnUSE_GL
    if( !m_pdc ){
        glDisable( GL_LINE_STIPPLE );
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_BLEND );
    }
#endif                
    return 1;
}

class PI_connector_segment              // This was extracted verbatim from S63_pi private definition
{
public:
    void *start;
    void *end;
    SegmentType type;
    int vbo_offset;
    int max_priority;
};

int s52plib::RenderLSPlugIn( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifndef USE_ANDROID_GLES2
    S52color *c;
    int w;
    
    char *str = (char*) rules->INSTstr;
    c = getColor( str + 7 ); // Colour
    wxColour color( c->R, c->G, c->B );
    w = atoi( str + 5 ); // Width
    
    double scale_factor = vp->ref_scale/vp->chart_scale;
    double scaled_line_width = wxMax((scale_factor - g_overzoom_emphasis_base), 1);
    bool b_wide_line = g_oz_vector_scale && vp->b_quilt && (scale_factor > g_overzoom_emphasis_base);
    
    wxPen wide_pen(*wxBLACK_PEN);
    wxDash dashw[2];
    dashw[0] = 3;
    dashw[1] = 1; 
    
    if( b_wide_line)
    {
        int w = wxMax(scaled_line_width, 2);            // looks better
        w = wxMin(w, 50);                               // upper bound
        wide_pen.SetWidth( w );
        wide_pen.SetColour(color);
        
        if( !strncmp( str, "DOTT", 4 ) ) {
            dashw[0] = 1;
            wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
            wide_pen.SetDashes( 2, dashw );
        }        
        else if( !strncmp( str, "DASH", 4 ) ){
            wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
            if( m_pdc){ //DC mode
                dashw[0] = 1;
                dashw[1] = 2;
            }
            
            wide_pen.SetDashes( 2, dashw );
        }
    }
    
    wxPen thispen(color, w, wxPENSTYLE_SOLID);
    wxDash dash1[2];
    
    if( m_pdc) //DC mode
    {
        if( !strncmp( str, "DOTT", 4 ) ) {
            thispen.SetStyle(wxPENSTYLE_USER_DASH);
            dash1[0] = 1;
            dash1[1] = 2; 
            thispen.SetDashes( 2, dash1 );
        }        
        else if( !strncmp( str, "DASH", 4 ) ){
            thispen.SetStyle(wxPENSTYLE_SHORT_DASH);
        }
        
        if(b_wide_line)
            m_pdc->SetPen( wide_pen );
        else
            m_pdc->SetPen( thispen );
        
    }
    
    #ifdef ocpnUSE_GL
    else // OpenGL mode
    {
        glColor3ub( c->R, c->G, c->B );
        
        //    Set drawing width
        if( w > 1 ) {
            GLint parms[2];
            glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
            if( w > parms[1] )
                glLineWidth( wxMax(g_GLMinCartographicLineWidth, parms[1]) );
            else
                glLineWidth( wxMax(g_GLMinCartographicLineWidth, w) );
        } else
            glLineWidth( wxMax(g_GLMinCartographicLineWidth, 1) );
        
        #ifndef ocpnUSE_GLES // linestipple is emulated poorly
            if( !strncmp( str, "DASH", 4 ) ) {
                glLineStipple( 1, 0x3F3F );
                glEnable( GL_LINE_STIPPLE );
            }
            else if( !strncmp( str, "DOTT", 4 ) ) {
                glLineStipple( 1, 0x3333 );
                glEnable( GL_LINE_STIPPLE );
            }
            else
                glDisable( GL_LINE_STIPPLE );
            #endif
                
    }
    #endif
    
    
    //    Get a true pixel clipping/bounding box from the vp
    wxPoint pbb = vp->GetPixFromLL( vp->clat, vp->clon );
    int xmin_ = pbb.x - (vp->rv_rect.width / 2) - (4 * scaled_line_width);
    int xmax_ = xmin_ + vp->rv_rect.width + (8 * scaled_line_width);
    int ymin_ = pbb.y - (vp->rv_rect.height / 2) - (4 * scaled_line_width) ;
    int ymax_ = ymin_ + vp->rv_rect.height + (8 * scaled_line_width);
    
    int x0, y0, x1, y1;
    
    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    
    if( rzRules->obj->m_ls_list_legacy )
    {
                 float *ppt;
                
                VE_Element *pedge;
                
                
                PI_connector_segment *pcs;
               
                unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
                PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;
                
                #ifdef ocpnUSE_GL
                if(!b_wide_line && !m_pdc)
                    glBegin( GL_LINES );
                #endif
                    
                    while(ls){
                        if( ls->priority == priority_current  ) {  
                            
                            int nPoints;
                            // fetch the first point
                            if(ls->type == TYPE_EE){
                                pedge = (VE_Element *)ls->private0;
                                ppt = (float *)(vbo_point + pedge->vbo_offset);
                                nPoints = pedge->nCount;
                            }
                            else{
                                pcs = (PI_connector_segment *)ls->private0;
                                ppt = (float *)(vbo_point + pcs->vbo_offset);
                                nPoints = 2;
                            }
                            
                            wxPoint l;
                            GetPointPixSingle( rzRules, ppt[1], ppt[0], &l, vp );
                            ppt += 2;
                            
                            for(int ip=0 ; ip < nPoints - 1 ; ip++){
                                wxPoint r;
                                GetPointPixSingle( rzRules, ppt[1], ppt[0], &r, vp );
                                //        Draw the edge as point-to-point
                                x0 = l.x, y0 = l.y;
                                x1 = r.x, y1 = r.y;
                                
                                // Do not draw null segments
                                if( ( x0 != x1 ) || ( y0 != y1 ) ){
                                    
                                    if(m_pdc){
                                        
                                        if( cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, xmin_, xmax_,
                                            ymin_, ymax_ ) != Invisible )
                                            m_pdc->DrawLine( x0, y0, x1, y1 );
                                    }
                                    #ifdef ocpnUSE_GL
                                    else {
                                        // simplified faster test, let opengl do the rest
                                        if((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                                            (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {
                                            if(!b_wide_line) {
                                                glVertex2i( x0, y0 );
                                                glVertex2i( x1, y1 );
                                            } else
                                                PLIBDrawGLThickLine( x0, y0, x1, y1, wide_pen, true );
                                            }
                                    }
                                    #endif      
                                    
                                }
                                
                                l = r;
                                ppt += 2;
                            }            
                        }
                        
                        ls = ls->next;
                    }
                    #ifdef ocpnUSE_GL
                    if(!b_wide_line && !m_pdc)
                        glEnd();
                    #endif              
    }
    
    #ifdef ocpnUSE_GL
    if( !m_pdc )
        glDisable( GL_LINE_STIPPLE );
    #endif                

#endif
        return 1;
}

// Line Simple Style, Dashed, using GLSL
int s52plib::RenderLS_Dash_GLSL( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifdef USE_ANDROID_GLES2
#if 0
        GLuint format = GL_RGBA;
        GLuint internalformat = format;
        int stride = 4;

        int tex_w = 32;
        int dash = 16;
        unsigned char bufDot[ 2 * tex_w * stride];
        for(int i=0 ; i < tex_w ; i++){
            bufDot[i * stride] = 255;
            bufDot[i * stride +1] = 0;
            bufDot[i * stride +2] = 0;
            if(i < dash)
                bufDot[i * stride + 3] = 255;
            else
                bufDot[i * stride + 3] = 0;

 //           bufDot[i * stride + 3] = 255;

        }

//         for(int i=0 ; i < sizeof(bufDot) ; i++){
//             bufDot[i] = 255;
//         }

 /*       for(int i=0 ; i < tex_w ; i++){
            bufDot[i * stride] = 255;
            bufDot[i * stride +1] = 0;
            bufDot[i * stride +2] = 0;
            if(i < dash)
                bufDot[i * stride + 3] = 255;
            else
                bufDot[i * stride + 3] = 0;
        }
 */
        GLuint textureDot = -1;

        glGenTextures( 1, &textureDot );
        glBindTexture( GL_TEXTURE_2D, textureDot );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST/*GL_LINEAR*/ );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glTexImage2D( GL_TEXTURE_2D, 0, internalformat, tex_w, 1, 0,  format, GL_UNSIGNED_BYTE, bufDot );

        glEnable( GL_TEXTURE_2D );

        glEnable( GL_BLEND );
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

#endif

//    ocpnDC *odc = new ocpnDC(*cc1->GetglCanvas());

    S52color *c;

    char *str = (char*) rules->INSTstr;
    c = getColor( str + 7 ); // Colour
    wxColour color( c->R, c->G, c->B );
    int w = atoi( str + 5 ); // Width

    double scale_factor = vp->ref_scale/vp->chart_scale;
    double scaled_line_width = wxMax((scale_factor - g_overzoom_emphasis_base), 1);

    wxPen thispen(color, w, wxPENSTYLE_SOLID);

    wxDash dash1[2];

    if( !strncmp( str, "DOTT", 4 ) ) {
        thispen.SetStyle(wxPENSTYLE_USER_DASH);
        dash1[0] = 1;
        dash1[1] = 2;
        thispen.SetDashes( 2, dash1 );
    }
    else if( !strncmp( str, "DASH", 4 ) ){
        thispen.SetStyle(wxPENSTYLE_USER_DASH);
        dash1[0] = 2;
        dash1[1] = 4;
        thispen.SetDashes( 2, dash1 );
    }

//    odc->SetPen( thispen );

        //    Set GL drawing width
/*
        if( w > 1 ) {
            GLint parms[2];
            glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
            if( w > parms[1] )
                glLineWidth( wxMax(g_GLMinCartographicLineWidth, parms[1]) );
            else
                glLineWidth( wxMax(g_GLMinCartographicLineWidth, w) );
        } else
            glLineWidth( wxMax(g_GLMinCartographicLineWidth, 1) );

        if(w >= 2){
            glEnable( GL_LINE_SMOOTH );
            glEnable( GL_BLEND );
        }
*/

   //    Set drawing width

   float lineWidth = w;
   GLint parms[2];
   glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
   GLint parmsa[2];
   glGetIntegerv( GL_SMOOTH_LINE_WIDTH_RANGE, &parmsa[0] );
   GLint parmsb[2];
   glGetIntegerv( GL_SMOOTH_LINE_WIDTH_GRANULARITY, &parmsb[0] );

   if( w > 1 ) {
       if( w > parms[1] )
           lineWidth = wxMax(g_GLMinCartographicLineWidth, parms[1]);
       else
           lineWidth = wxMax(g_GLMinCartographicLineWidth, w);
   } else
       lineWidth = wxMax(g_GLMinCartographicLineWidth, 1);

   // Manage super high density displays
       float target_w_mm = 0.5 * w;;
       if(GetPPMM() > 7){               // arbitrary, leaves average desktop/laptop display untweaked...
            target_w_mm = ((float)w) / 6.0;  // Target width in mm
        //  The value "w" comes from S52 library CNSY procedures, in "nominal" pixels
        // the value "6" comes from semi-standard LCD display densities
        // or something like 0.18 mm pitch, or 6 pix per mm.
            lineWidth =  wxMax(g_GLMinCartographicLineWidth, target_w_mm * GetPPMM() );
       }

       glDisable( GL_LINE_SMOOTH );
       glEnable( GL_BLEND );                    // for shader

#ifdef __OCPN__ANDROID__
       lineWidth = wxMin(lineWidth, parms[1]);
       glLineWidth(lineWidth);

#else
       glLineWidth(lineWidth);
       if(lineWidth > 4.0){
           glEnable( GL_LINE_SMOOTH );
       }
#endif

    //    Get a true pixel clipping/bounding box from the vp
    wxPoint pbb = vp->GetPixFromLL( vp->clat, vp->clon );
    int xmin_ = pbb.x - (vp->rv_rect.width / 2) - (4 * scaled_line_width);
    int xmax_ = xmin_ + vp->rv_rect.width + (8 * scaled_line_width);
    int ymin_ = pbb.y - (vp->rv_rect.height / 2) - (4 * scaled_line_width) ;
    int ymax_ = ymin_ + vp->rv_rect.height + (8 * scaled_line_width);

    int x0, y0, x1, y1;

    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;

    glUseProgram( S52Dash_shader_program );

    // Get pointers to the attributes in the program.
    GLint mPosAttrib = glGetAttribLocation( S52Dash_shader_program, "position" );

    GLint startPos  = glGetUniformLocation( S52Dash_shader_program, "startPos" );
    GLint texWidth  = glGetUniformLocation( S52Dash_shader_program, "texWidth" );

    float colorv[4];
    colorv[0] = color.Red() / float(256);
    colorv[1] = color.Green() / float(256);
    colorv[2] = color.Blue() / float(256);
    colorv[3] = 1.0; //transparency;

    GLint colloc = glGetUniformLocation(S52Dash_shader_program,"color");
    glUniform4fv(colloc, 1, colorv);

    // Select the active texture unit.
    glActiveTexture( GL_TEXTURE0 );

    // Bind our texture to the texturing target.
    //                            glBindTexture( GL_TEXTURE_2D, textureDot );

    // Set up the texture sampler to texture unit 0
    GLint texUni = glGetUniformLocation( S52Dash_shader_program, "uTex" );
    glUniform1i( texUni, 0 );

    float width = 32; //tex_w;
    glUniform1f(texWidth, width);

    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );


    // Rotate
    mat4x4 I, Q;
    mat4x4_identity(I);

    //mat4x4_rotate_Z(Q, I, vp->rotation);

    // Translate
    //             Q[3][0] = r.x-pivot_x;
    //             Q[3][1] = r.y-pivot_y;


    //mat4x4 X;
    //mat4x4_mul(X, (float (*)[4])vp->vp_transform, Q);

    GLint matloc = glGetUniformLocation(S52Dash_shader_program,"TransformMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)I);

    if( rzRules->obj->m_ls_list )
    {
        float *ppt;

        unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->chart->GetLineVertexBuffer();;
        line_segment_element *ls = rzRules->obj->m_ls_list;

            while(ls){
                if( ls->priority == priority_current  ) {

                    int nPoints;
                    // fetch the first point
                    if( (ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV) ){
                        ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
                        nPoints = ls->pedge->nCount;
                    }
                    else{
                        ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
                        nPoints = 2;
                    }

                    wxPoint l;
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &l, vp );
                    ppt += 2;

                    for(int ip=0 ; ip < nPoints - 1 ; ip++){
                        wxPoint r;
                        GetPointPixSingle( rzRules, ppt[1], ppt[0], &r, vp );
                        //        Draw the edge as point-to-point
                        x0 = l.x, y0 = l.y;
                        x1 = r.x, y1 = r.y;

                        // Do not draw null segments
                        if( ( x0 != x1 ) || ( y0 != y1 ) ){

#if 0
                                if( cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, xmin_, xmax_,
                                    ymin_, ymax_ ) != Invisible )
                                    odc->DrawLine( x0, y0, x1, y1, true );
#else



                            // segment must be at least on-screen....
                            if((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                                (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {

                                //  And intersecting the current clip rectangle
                                int xa = x0;
                                int xw = x1-x0;
                                if(xw < 0){
                                    xa = x1;
                                    xw = -xw;
                                }
                                int ya = y0;
                                int yh = y1-y0;
                                if(yh < 0){
                                    ya = y1;
                                    yh = -yh;
                                }

                                wxRect rseg(xa, ya, xw, yh);
                                rseg.Inflate(1);        // avoid zero width/height
                                if(rseg.Intersects(m_last_clip_rect))
                                {
                                    float coords[4];

                                    coords[0] = x0; coords[1] = y0; coords[2] = x1; coords[3] = y1;

                                    float start[2];
                                    start[0] = x0; start[1] = GetOCPNCanvasWindow()->GetSize().y - y0;
                                    glUniform2fv(startPos, 1, start);


                                    // Set the attribute mPosAttrib with the vertices in the screen coordinates...
                                    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
                                    glEnableVertexAttribArray( mPosAttrib );

                                    // Perform the actual drawing.
                                    glDrawArrays(GL_LINES, 0, 2);
                                }
                            }

#endif

                        }

                        l = r;
                        ppt += 2;
                    }
                }

                ls = ls->next;
            }
    }

//    delete odc;
#endif

        return 1;
}

// Line Complex
int s52plib::RenderLC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    return 0;
    //     if(rzRules->obj->Index != 7574)
    //         return 0;
    
    // catch cm93 and legacy PlugIns (e.g.s63_pi)
    if( rzRules->obj->m_n_lsindex  && !rzRules->obj->m_ls_list) 
        return RenderLCLegacy(rzRules, rules, vp);
    
    wxPoint r;
    
    
    int isym_len = rules->razRule->pos.line.bnbox_w.SYHL + (rules->razRule->pos.line.bnbox_x.LBXC - rules->razRule->pos.line.pivot_x.LICL);
    float sym_len = isym_len * canvas_pix_per_mm / 100;
    float sym_factor = 1.0; ///1.50;                        // gives nicer effect
    
    //      Create a color for drawing adjustments outside of HPGL renderer
    char *tcolptr = rules->razRule->colRef.LCRF;
    S52color *c = getColor( tcolptr + 1 ); // +1 skips "n" in HPGL SPn format
    int w = 1; // arbitrary width
    wxColour color( c->R, c->G, c->B );
    double LOD = 2.0 / vp->view_scale_ppm;              // empirical value, by experiment
    LOD = 0; //wxMin(LOD, 10.0);
    
    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    
    if( rzRules->obj->m_n_lsindex ) {
        
        
        // Calculate the size of a work buffer
        int max_points = 0;
        if( rzRules->obj->m_n_edge_max_points > 0 ) 
            max_points = rzRules->obj->m_n_edge_max_points;
        else{
            line_segment_element *lsa = rzRules->obj->m_ls_list;
            
            while(lsa){
                
                if( (lsa->ls_type == TYPE_EE) || (lsa->ls_type == TYPE_EE_REV) )
                    max_points += lsa->pedge->nCount;
                else
                    max_points += 2;
                
                lsa = lsa->next;
            }
        }
        
        
        //  Allocate some storage for converted points
        wxPoint *ptp = (wxPoint *) malloc( ( max_points ) * sizeof(wxPoint) ); 
        double *pdp = (double *)malloc( 2 * ( max_points ) * sizeof(double) ); 
        
        unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer; //chart->GetLineVertexBuffer();
        line_segment_element *ls = rzRules->obj->m_ls_list;
        
        unsigned int index = 0;
        unsigned int idouble = 0;
        int nls = 0;
        wxPoint lp;
        float *ppt;
        
        int direction = 1;
        int ndraw = 0;
        while(ls){
            if( ls->priority == priority_current  ) {  
 
                
                //transcribe the segment in the proper order into the output buffer
                int nPoints;
                int idir = 1;
                // fetch the first point
                if( (ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV) ){
                    ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
                    nPoints = ls->pedge->nCount;
                    if(ls->ls_type == TYPE_EE_REV)
                        idir = -1;
                    
                }
                else{
                    ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
                    nPoints = 2;
                }
                
                
                int vbo_index = 0;
                int vbo_inc = 2;
                if(idir == -1){
                    vbo_index = (nPoints-1) * 2;
                    vbo_inc = -2;
                }
                for(int ip=0 ; ip < nPoints ; ip++){
                    wxPoint r;
                    GetPointPixSingle( rzRules, ppt[vbo_index + 1], ppt[vbo_index], &r, vp );
                    if( (r.x != lp.x) || (r.y != lp.y) ){
                        ptp[index++] = r;
                        pdp[idouble++] = ppt[vbo_index];
                        pdp[idouble++] = ppt[vbo_index + 1];
                        
                        nls++;
                    }
                    else{               // sKipping point
                    }
                    
                    lp = r;
                    vbo_index += vbo_inc;
                }            
                
            }  // priority
            
            // inspect the next segment to see if it can be connected, or if the chain breaks
            int idir = 1;
            if(ls->next){
                
                int nPoints_next;
                line_segment_element *lsn = ls->next;
                // fetch the first point
                if( (lsn->ls_type == TYPE_EE) || (lsn->ls_type == TYPE_EE_REV) ){
                    ppt = (float *)(vbo_point + lsn->pedge->vbo_offset);
                    nPoints_next = lsn->pedge->nCount;
                    if(lsn->ls_type == TYPE_EE_REV)
                        idir = -1;
                    
                }
                else{
                    ppt = (float *)(vbo_point + lsn->pcs->vbo_offset);
                    nPoints_next = 2;
                }
                
                wxPoint ptest;
                if(idir == 1)
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &ptest, vp );

                else{
                // fetch the last point
                    int index_last_next = (nPoints_next-1) * 2;
                    GetPointPixSingle( rzRules, ppt[index_last_next +1], ppt[index_last_next], &ptest, vp );
                }
                
                // try to match the correct point in this segment with the last point in the previous segment

                if(lp != ptest)         // not connectable?
                {
                    
                    if(nls){
                        wxPoint2DDouble *pReduced = 0;
                        int nPointReduced = reduceLOD( LOD, nls, pdp, &pReduced);
                    
                        wxPoint *ptestp = (wxPoint *) malloc( ( max_points ) * sizeof(wxPoint) ); 
                        GetPointPixArray( rzRules, pReduced, ptestp, nPointReduced, vp );
                        free(pReduced);
                    
                        draw_lc_poly( m_pdc, color, w, ptestp, nPointReduced, sym_len, sym_factor, rules->razRule, vp );
                        free(ptestp);
                    
                        ndraw++;
                    }
                    
                    nls = 0;
                    index = 0;
                    idouble = 0;
                    lp = wxPoint(0,0);
                    direction = 1;
                }
                
                
            }
            else{
                // no more segments, so render what is available
                if(nls){
                    wxPoint2DDouble *pReduced = 0;
                    int nPointReduced = reduceLOD( LOD, nls, pdp, &pReduced);
                    
                    wxPoint *ptestp = (wxPoint *) malloc( ( max_points ) * sizeof(wxPoint) ); 
                    GetPointPixArray( rzRules, pReduced, ptestp, nPointReduced, vp );
                    free(pReduced);
                    
                    draw_lc_poly( m_pdc, color, w, ptestp, nPointReduced, sym_len, sym_factor, rules->razRule, vp );
                    free( ptestp );
                }
            }
            
            ls = ls->next;
        }
        
        
        free( ptp );
        free(pdp);
    }
    
    return 1;
}


int s52plib::reduceLOD( double LOD_meters, int nPoints, double *source, wxPoint2DDouble **dest)
{
    //      Reduce the LOD of this linestring
    std::vector<int> index_keep;
    if(nPoints > 5 && (LOD_meters > .01)){
	index_keep.push_back(0);
	index_keep.push_back(nPoints-1);
	index_keep.push_back(nPoints-2);
        
        DouglasPeucker(source, 1, nPoints-2, LOD_meters, &index_keep);
        
    }
    else {
	index_keep.resize(nPoints);
	// Consider using std::iota here when there is C++11 support.
        for(int i = 0 ; i < nPoints ; i++)
            index_keep[i] = i;
    }
    
    wxPoint2DDouble *pReduced = (wxPoint2DDouble *)malloc( ( index_keep.size() ) * sizeof(wxPoint2DDouble) ); 
    *dest = pReduced;
    
    double *ppr = source;  
    int ir = 0;
    for(int ip = 0 ; ip < nPoints ; ip++)
    {
        double x = *ppr++;
        double y = *ppr++;
        
        for(unsigned int j=0 ; j < index_keep.size() ; j++){
            if(index_keep[j] == ip){
                pReduced[ir++] = wxPoint2DDouble(x, y);
                break;
            }
        }
    }
    
    return index_keep.size();
}
    

// Line Complex
int s52plib::RenderLCLegacy( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    if( !rzRules->obj->m_chart_context->chart )
        return RenderLCPlugIn( rzRules, rules, vp );
    
    //  Must be cm93
        
    wxPoint r;

    int isym_len = rules->razRule->pos.line.bnbox_w.SYHL;
    float sym_len = isym_len * canvas_pix_per_mm / 100;
    float sym_factor = 1.0; ///1.50;                        // gives nicer effect

//      Create a color for drawing adjustments outside of HPGL renderer
    char *tcolptr = rules->razRule->colRef.LCRF;
    S52color *c = getColor( tcolptr + 1 ); // +1 skips "n" in HPGL SPn format
    int w = 1; // arbitrary width
    wxColour color( c->R, c->G, c->B );

    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;

    if( rzRules->obj->m_n_lsindex ) {
        
        VE_Hash *ve_hash = (VE_Hash *)rzRules->obj->m_chart_context->m_pve_hash; 
        VC_Hash *vc_hash = (VC_Hash *)rzRules->obj->m_chart_context->m_pvc_hash; 
        
        VE_Element *ptedge = (*ve_hash)[0];
        
        unsigned int nls_max;
        if( rzRules->obj->m_n_edge_max_points > 0 ) // size has been precalculated on SENC load
            nls_max = rzRules->obj->m_n_edge_max_points;
        else {
            //  Calculate max malloc size required
            nls_max = 0;
            int *index_run_x = rzRules->obj->m_lsindex_array;
            for( int imseg = 0; imseg < rzRules->obj->m_n_lsindex; imseg++ ) {
                index_run_x++; //Skip cNode
                //  Get the edge
                unsigned int enode = *index_run_x;
                if( enode ){
                    VE_Element *pedge = (*ve_hash)[enode];
                    if(pedge){
                        if( pedge->nCount > nls_max )
                            nls_max = pedge->nCount;
                    }
                }
                index_run_x += 2;
            }
            rzRules->obj->m_n_edge_max_points = nls_max; // Got it, cache for next time
        }

        //  Allocate some storage for converted points
        wxPoint *ptp = (wxPoint *) malloc( ( nls_max + 2 ) * sizeof(wxPoint) ); // + 2 allows for end nodes

        int *index_run;
        float *ppt;
        double easting, northing;
        wxPoint pra( 0, 0 );
        VC_Element *pnode;

        for( int iseg = 0; iseg < rzRules->obj->m_n_lsindex; iseg++ ) {
            int seg_index = iseg * 3;
            index_run = &rzRules->obj->m_lsindex_array[seg_index];

            //  Get first connected node
            unsigned int inode = *index_run++;
            if( inode ) {
                pnode = (*vc_hash)[inode];
                if( pnode ) {
                    ppt = pnode->pPoint;
                    easting = *ppt++;
                    northing = *ppt;
                    GetPointPixSingle( rzRules, (float) northing, (float) easting, &pra, vp );
                }
                ptp[0] = pra; // insert beginning node
            }

            //  Get the edge
            unsigned int enode = *index_run++;
            VE_Element *pedge = 0;
            if(enode)
                pedge = (*ve_hash)[enode];

            int nls = 0;
            if(pedge){
            //  Here we decide to draw or not based on the highest priority seen for this segment
            //  That is, if this segment is going to be drawn at a higher priority later, then don't draw it here.
            
            // This logic is not perfectly right for one case:
            // If the segment has only two end connected nodes, and no intermediate edge,
            // then we have no good way to evaluate the priority.
            // This is due to the fact that priority is only precalculated for edge segments, not connector nodes.
            // Only thing to do is take the conservative approach and draw the segment, in this case.
            if( pedge->nCount ){
                if( pedge->max_priority != priority_current ) continue;
            }
            
                if( pedge->max_priority != priority_current ) continue;

                nls = pedge->nCount;

                ppt = pedge->pPoints;
                for( int ip = 0; ip < nls; ip++ ) {
                    easting = *ppt++;
                    northing = *ppt++;
                    GetPointPixSingle( rzRules, (float) northing, (float) easting, &ptp[ip + 1], vp );
                }
            }

            //  Get last connected node
            unsigned int jnode = *index_run++;
            if( jnode ) {
                pnode = (*vc_hash)[jnode];
                if( pnode ) {
                    ppt = pnode->pPoint;
                    easting = *ppt++;
                    northing = *ppt;
                    GetPointPixSingle( rzRules, (float) northing, (float) easting, &pra, vp );
                }
                ptp[nls + 1] = pra; // insert ending node
            }

            if( ( inode ) && ( jnode ) ){
                draw_lc_poly( m_pdc, color, w, ptp, nls + 2, sym_len, sym_factor, rules->razRule, vp );
            }
            else if(nls){
                draw_lc_poly( m_pdc, color, w, &ptp[1], nls, sym_len, sym_factor, rules->razRule, vp );
            }

        }
        free( ptp );
    }

    else
        if( rzRules->obj->pPolyTessGeo ) {
            if( !rzRules->obj->pPolyTessGeo->IsOk() ){ // perform deferred tesselation
                rzRules->obj->pPolyTessGeo->BuildDeferredTess();
            }

            PolyTriGroup *pptg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
            float *ppolygeo = pptg->pgroup_geom;
            if(ppolygeo){
                int ctr_offset = 0;
                for( int ic = 0; ic < pptg->nContours; ic++ ) {

                    int npt = pptg->pn_vertex[ic];
                    wxPoint *ptp = (wxPoint *) malloc( ( npt + 1 ) * sizeof(wxPoint) );
                    wxPoint *pr = ptp;
                    for( int ip = 0; ip < npt; ip++ ) {
                        float plon = ppolygeo[( 2 * ip ) + ctr_offset];
                        float plat = ppolygeo[( 2 * ip ) + ctr_offset + 1];

                        GetPointPixSingle( rzRules, plat, plon, pr, vp );
                        pr++;
                    }
                    float plon = ppolygeo[ctr_offset]; // close the polyline
                    float plat = ppolygeo[ctr_offset + 1];
                    GetPointPixSingle( rzRules, plat, plon, pr, vp );

                    draw_lc_poly( m_pdc, color, w, ptp, npt + 1, sym_len, sym_factor, rules->razRule,
                            vp );

                    free( ptp );

                    ctr_offset += npt * 2;
                }
            }
        }

    return 1;
}

int s52plib::RenderLCPlugIn( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    wxPoint r;
    
    int isym_len = rules->razRule->pos.line.bnbox_w.SYHL;
    float sym_len = isym_len * canvas_pix_per_mm / 100;
    float sym_factor = 1.0; ///1.50;                        // gives nicer effect
    
    //      Create a color for drawing adjustments outside of HPGL renderer
    char *tcolptr = rules->razRule->colRef.LCRF;
    S52color *c = getColor( tcolptr + 1 ); // +1 skips "n" in HPGL SPn format
    int w = 1; // arbitrary width
    wxColour color( c->R, c->G, c->B );
    
    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    
 
    //  Calculate max malloc size required
    unsigned int nls_max = 0;
        
    if( rzRules->obj->m_ls_list_legacy )
    {
        float *ppt;
        
        VE_Element *pedge;
        PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;
        
            
        while(ls){
            int nPoints;
                    // fetch the first point
            if(ls->type == TYPE_EE){
                pedge = (VE_Element *)ls->private0;
                nPoints = pedge->nCount;
             }
             else{
                nPoints = 2;
             }
                
             nls_max = wxMax(nls_max, nPoints);
                
            ls = ls->next;
        }
    }


    //  Allocate some storage for converted points
//    wxPoint *ptp = (wxPoint *) malloc( ( nls_max + 2 ) * sizeof(wxPoint) ); // + 2 allows for end nodes


    if( rzRules->obj->m_ls_list_legacy )
    {
        float *ppt;

        VE_Element *pedge;



        //  Allocate some storage for converted points
        wxPoint *ptp = (wxPoint *) malloc( ( nls_max + 2 ) * sizeof(wxPoint) ); // + 2 allows for end nodes
    
        PI_connector_segment *pcs;
        
        unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
        PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;
        
        while(ls){
                if( ls->priority == priority_current  ) {  
                    
                    int nPoints;
                    // fetch the first point
                    if(ls->type == TYPE_EE){
                        pedge = (VE_Element *)ls->private0;
                        ppt = (float *)(vbo_point + pedge->vbo_offset);
                        nPoints = pedge->nCount;
                    }
                    else{
                        pcs = (PI_connector_segment *)ls->private0;
                        ppt = (float *)(vbo_point + pcs->vbo_offset);
                        nPoints = 2;
                    }
                    
                    for(int ip=0 ; ip < nPoints ; ip++){
                        wxPoint r;
                        GetPointPixSingle( rzRules, ppt[1], ppt[0], &ptp[ip], vp );
                        
                        ppt += 2;
                    }
                    
                    if(nPoints)
                        draw_lc_poly( m_pdc, color, w, &ptp[0], nPoints, sym_len, sym_factor, rules->razRule, vp );
                }
                
                ls = ls->next;
        }
        
        free(ptp);
    }
    
    return 1;
}

//      Render Line Complex Polyline

void s52plib::draw_lc_poly( wxDC *pdc, wxColor &color, int width, wxPoint *ptp, int npt,
        float sym_len, float sym_factor, Rule *draw_rule, ViewPort *vp )
{
    if(npt < 2)
        return;
    
    wxPoint r;

    //  We calculate the winding direction of the poly
    //  in order to know which side to draw symbol on
    double dfSum = 0.0;
    
    for( int iseg = 0; iseg < npt - 1; iseg++ ) {
        dfSum += ptp[iseg].x * ptp[iseg+1].y - ptp[iseg].y * ptp[iseg+1].x;
    }
    dfSum += ptp[npt-1].x * ptp[0].y - ptp[npt-1].y * ptp[0].x;
    
    bool cw = dfSum < 0.;
    
    //    Get a true pixel clipping/bounding box from the vp
    wxPoint pbb = vp->GetPixFromLL( vp->clat, vp->clon );
    int xmin_ = pbb.x - vp->rv_rect.width / 2;
    int xmax_ = xmin_ + vp->rv_rect.width;
    int ymin_ = pbb.y - vp->rv_rect.height / 2;
    int ymax_ = ymin_ + vp->rv_rect.height;

    int x0, y0, x1, y1;

    if( pdc ) {
        wxPen *pthispen = wxThePenList->FindOrCreatePen( color, width, wxPENSTYLE_SOLID );
        m_pdc->SetPen( *pthispen );

        int start_seg = 0;
        int end_seg = npt - 1;
        int inc = 1;
        
        if( cw ){
            start_seg = npt - 1;
            end_seg = 0;
            inc = -1;
        }
        
        float dx, dy, seg_len, theta;
        
        bool done = false;
        int iseg = start_seg;
        while( !done ){
            
            // Do not bother with segments that are invisible

            x0 = ptp[iseg].x;
            y0 = ptp[iseg].y;
            x1 = ptp[iseg + inc].x;
            y1 = ptp[iseg + inc].y;

            ClipResult res = cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, xmin_, xmax_, ymin_,
                    ymax_ );

            if( res == Invisible )
                goto next_seg_dc;

            dx = ptp[iseg + inc].x - ptp[iseg].x;
            dy = ptp[iseg + inc].y - ptp[iseg].y;
            seg_len = sqrt( dx * dx + dy * dy );
            
            if( seg_len >= 1.0 ) {
                if( seg_len <= sym_len * sym_factor ) {
                    int xst1 = ptp[iseg].x;
                    int yst1 = ptp[iseg].y;
                    float xst2, yst2;
                    if( seg_len >= sym_len ) {
                        xst2 = xst1 + ( sym_len * dx / seg_len );
                        yst2 = yst1 + ( sym_len * dy / seg_len );
                    } else {
                        xst2 = ptp[iseg + inc].x;
                        yst2 = ptp[iseg + inc].y;
                    }
                    
                    pdc->DrawLine( xst1, yst1, (wxCoord) floor( xst2 ), (wxCoord) floor( yst2 ) );
                }

                else {
                    float s = 0;
                    float xs = ptp[iseg].x;
                    float ys = ptp[iseg].y;

                    while( s + ( sym_len * sym_factor ) < seg_len ) {
                        r.x = (int) xs;
                        r.y = (int) ys;
                        char *str = draw_rule->vector.LVCT;
                        char *col = draw_rule->colRef.LCRF;
                        wxPoint pivot( draw_rule->pos.line.pivot_x.LICL,
                                draw_rule->pos.line.pivot_y.LIRW );

                        HPGL->SetTargetDC( pdc );
                        theta = atan2f( dy, dx );
                        HPGL->SetVP(vp);
                        HPGL->Render( str, col, r, pivot, pivot, 1.0, theta * 180. / PI, false );

                        xs += sym_len * dx / seg_len * sym_factor;
                        ys += sym_len * dy / seg_len * sym_factor;
                        s += sym_len * sym_factor;
                    }

                    pdc->DrawLine( (int) xs, (int) ys, ptp[iseg + inc].x, ptp[iseg + inc].y );
                }
            }
next_seg_dc:            
            iseg += inc;
            if(iseg == end_seg)
                done = true;
            
        } // while
    } // if pdc
    
#ifdef ocpnUSE_GL
    else // opengl
    {
        //    Set up the color
#ifndef USE_ANDROID_GLES2
        glColor4ub( color.Red(), color.Green(), color.Blue(), color.Alpha() );
#endif
        
        // Adjust line width up a bit, to improve render quality for GL_BLEND/GL_LINE_SMOOTH
        float awidth = wxMax(g_GLMinCartographicLineWidth, (float)width * 0.7);
        awidth = wxMax(awidth, 1.5);
        glLineWidth( awidth );
        
        int start_seg = 0;
        int end_seg = npt - 1;
        int inc = 1;
        
        if( cw ){
            start_seg = npt - 1;
            end_seg = 0;
            inc = -1;
        }
        
        float dx, dy, seg_len, theta;
        
        bool done = false;
        int iseg = start_seg;
        while( !done ){
           // Do not bother with segments that are invisible

            x0 = ptp[iseg].x;
            y0 = ptp[iseg].y;
            x1 = ptp[iseg + inc].x;
            y1 = ptp[iseg + inc].y;

            ClipResult res = cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, xmin_, xmax_, ymin_,
                    ymax_ );

            if( res == Invisible )
                goto next_seg;

            dx = ptp[iseg + inc].x - ptp[iseg].x;
            dy = ptp[iseg + inc].y - ptp[iseg].y;
            seg_len = sqrt( dx * dx + dy * dy );
            
            if( seg_len >= 1.0 ) {
                if( seg_len <= sym_len * sym_factor ) {
                    int xst1 = ptp[iseg].x;
                    int yst1 = ptp[iseg].y;
                    float xst2, yst2;

                    if( seg_len >= sym_len ) {
                        xst2 = xst1 + ( sym_len * dx / seg_len );
                        yst2 = yst1 + ( sym_len * dy / seg_len );
                    } else {
                        xst2 = ptp[iseg + inc].x;
                        yst2 = ptp[iseg + inc].y;
                    }

                    //      Enable anti-aliased lines, at best quality
#ifndef __OCPN__ANDROID__
                    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glEnable (GL_BLEND);
                    
                    if( m_GLLineSmoothing )
                    {
                        glEnable (GL_LINE_SMOOTH);
                        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
                    }
#endif

#ifdef USE_ANDROID_GLES2

                    glUseProgram(S52color_tri_shader_program);

                    glBindBuffer( GL_ARRAY_BUFFER, 0 );
                    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

                    //GLint matloc = glGetUniformLocation(S52color_tri_shader_program,"MVMatrix");
                    //glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)vp->vp_transform);

                    float colorv[4];
                    colorv[0] = color.Red() / float(256);
                    colorv[1] = color.Green() / float(256);
                    colorv[2] = color.Blue() / float(256);
                    colorv[3] = 1.0; //transparency;

                    GLint colloc = glGetUniformLocation(S52color_tri_shader_program,"color");
                    glUniform4fv(colloc, 1, colorv);

                    float pts[4];
                    pts[0] = xst1; pts[1] = yst1; pts[2] = xst2; pts[3] = yst2;

                    GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");
                    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), pts);
                    glEnableVertexAttribArray(pos);

                    glDrawArrays(GL_LINES, 0, 2);

#else
                    {
                        glBegin( GL_LINES );
                        glVertex2i( xst1, yst1 );
                        glVertex2i( (wxCoord) floor( xst2 ), (wxCoord) floor( yst2 ) );
                        glEnd();
                    }
#endif
                    
                    glDisable( GL_LINE_SMOOTH );
                    glDisable( GL_BLEND );
                } else {
                    float s = 0;
                    float xs = ptp[iseg].x;
                    float ys = ptp[iseg].y;

                    while( s + ( sym_len * sym_factor ) < seg_len ) {
                        r.x = (int) xs;
                        r.y = (int) ys;
                        char *str = draw_rule->vector.LVCT;
                        char *col = draw_rule->colRef.LCRF;
                        wxPoint pivot( draw_rule->pos.line.pivot_x.LICL,
                                draw_rule->pos.line.pivot_y.LIRW );

                        HPGL->SetTargetOpenGl();
                        HPGL->SetVP(vp);
                        theta = atan2f( dy, dx );
                        HPGL->Render( str, col, r, pivot, pivot, 1.0, theta * 180. / PI, false );

                        xs += sym_len * dx / seg_len * sym_factor;
                        ys += sym_len * dy / seg_len * sym_factor;
                        s += sym_len * sym_factor;
                    }
#ifndef __OCPN__ANDROID__
                    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glEnable (GL_BLEND);

                    if( m_GLLineSmoothing ) {
                        glEnable (GL_LINE_SMOOTH);
                        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
                    }

#endif
#ifdef USE_ANDROID_GLES2
                    glUseProgram(S52color_tri_shader_program);

                    glBindBuffer( GL_ARRAY_BUFFER, 0 );
                    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

                    //GLint matloc = glGetUniformLocation(S52color_tri_shader_program,"MVMatrix");
                    //glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)vp->vp_transform);

                    float colorv[4];
                    colorv[0] = color.Red() / float(256);
                    colorv[1] = color.Green() / float(256);
                    colorv[2] = color.Blue() / float(256);
                    colorv[3] = 1.0; //transparency;

                    GLint colloc = glGetUniformLocation(S52color_tri_shader_program,"color");
                    glUniform4fv(colloc, 1, colorv);

                    float pts[4];
                    pts[0] = xs; pts[1] = ys; pts[2] = ptp[iseg + inc].x; pts[3] =  ptp[iseg + inc].y;

                    GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");
                    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), pts);
                    glEnableVertexAttribArray(pos);

                    glDrawArrays(GL_LINES, 0, 2);

#else

                    {
                        glBegin( GL_LINES );
                        glVertex2i( xs, ys );
                        glVertex2i( ptp[iseg + inc].x, ptp[iseg + inc].y );
                        glEnd();
                    }
#endif
                    glDisable( GL_LINE_SMOOTH );
                    glDisable( GL_BLEND );
                }
            }
next_seg:            
            iseg += inc;
            if(iseg == end_seg)
                done = true;
        } // while

    } //opengl
#endif    
}

// Multipoint Sounding
int s52plib::RenderMPS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    if( !m_bShowSoundg )
        return 0;

    if( m_bUseSCAMIN ) {
        if( vp->chart_scale > rzRules->obj->Scamin )
            return 0;
    }
    
    
    int npt = rzRules->obj->npt;

    // this should never happen
    // But it seems that some PlugIns clear the mps rules without resetting the CS state machine
    // So fix it
    if( rzRules->obj->bCS_Added  && !rzRules->mps)
        rzRules->obj->bCS_Added = false;
        
    //  Build the cached rules list if necessary
    if( !rzRules->obj->bCS_Added ) {

        ObjRazRules point_rzRules;
        point_rzRules = *rzRules; // take a copy of attributes, etc
        
        S57Obj point_obj;
        point_obj = *( rzRules->obj );
        point_obj.bIsClone = true;
        point_rzRules.obj = &point_obj;
        
        Rules *ru_cs = StringToRules( _T ( "CS(SOUNDG03;" ) );

        wxPoint p;
        double *pd = rzRules->obj->geoPtz; // the SM points
        double *pdl = rzRules->obj->geoPtMulti; // and corresponding lat/lon
        

        mps_container *pmps = (mps_container *)calloc( sizeof(mps_container), 1);
        pmps->cs_rules = new ArrayOfRules;
        rzRules->mps = pmps;
        
        for( int ip = 0; ip < npt; ip++ ) {
            double east = *pd++;
            double nort = *pd++;
            double depth = *pd++;
            
            point_obj.x = east;
            point_obj.y = nort;
            point_obj.z = depth;
            
            double lon = *pdl++;
            double lat = *pdl++;
            point_obj.BBObj.Set( lat, lon, lat, lon );
            point_obj.BBObj.Invalidate();
            
            char *rule_str1 = RenderCS( &point_rzRules, ru_cs );
            wxString cs_string( rule_str1, wxConvUTF8 );
            free( rule_str1 ); 
    
            Rules *rule_chain = StringToRules( cs_string );
            
            rzRules->mps->cs_rules->Add( rule_chain );
            
        }

        DestroyRulesChain( ru_cs );
        rzRules->obj->bCS_Added = 1; // mark the object
    }
   
    

    double *pdl = rzRules->obj->geoPtMulti; // and corresponding lat/lon

    //  We need a private unrotated copy of the Viewport
    ViewPort vp_local = *vp;
    vp_local.SetRotationAngle( 0. );

    //  We may be rendering the soundings symbols scaled up, so
    //  adjust the inclusion test bounding box
    
    double scale_factor = vp->ref_scale/vp->chart_scale;
    double box_mult = wxMax((scale_factor - g_overzoom_emphasis_base), 1);
    int box_dim = 32 * box_mult;
    
    // We need a pixel bounding rectangle of the passed ViewPort.
    // Very important for partial screen renders, as with dc mode pans or OpenGL FBO operation.
   
    wxPoint cr0 = vp_local.GetPixFromLL( vp_local.GetBBox().GetMaxLat(), vp_local.GetBBox().GetMinLon());
    wxPoint cr1 = vp_local.GetPixFromLL( vp_local.GetBBox().GetMinLat(), vp_local.GetBBox().GetMaxLon());
    wxRect clip_rect(cr0, cr1);
    
    for( int ip = 0; ip < npt; ip++ ) {
        
        double lon = *pdl++;
        double lat = *pdl++;

        wxPoint r = vp_local.GetPixFromLL( lat, lon );
        //      Use estimated symbol size
        wxRect rr(r.x-(box_dim/2), r.y-(box_dim/2), box_dim, box_dim);
        
        //      After all the setup, the render inclusion test is trivial....
        if(!clip_rect.Intersects(rr))
            continue;
        
        double angle = 0;
        
        Rules *rules =  rzRules->mps->cs_rules->Item(ip);
        while( rules ){
            
            //  Render a raster or vector symbol, as specified by LUP rules
            if( rules->razRule->definition.SYDF == 'V' ){
                // On OpenGL, arrange to render the drying height "underline" symbol as un-rotated.
                double dryAngle = 0;
                if( !m_pdc && !strncmp(rules->razRule->name.SYNM, "SOUNDSA1", 8))
                    dryAngle = -vp->rotation * 180./PI;
                RenderHPGL( rzRules, rules->razRule, r, vp, dryAngle );
            }
            else if( rules->razRule->definition.SYDF == 'R' )
                RenderRasterSymbol( rzRules, rules->razRule, r, vp, angle );
            
            rules = rules->next;
        }
    }
    
    return 1;
}

int s52plib::RenderCARC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifdef USE_ANDROID_GLES2
    return RenderCARC_GLSL(rzRules, rules, vp);
#endif

    return RenderCARC_VBO(rzRules, rules, vp);
}
    
int s52plib::RenderCARC_GLSL( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifdef USE_ANDROID_GLES2

//    glDisable( GL_SCISSOR_TEST );

    char *str = (char*) rules->INSTstr;
    //    extract the parameters from the string
    //    And creating a unique string hash as we go
    wxString inst( str, wxConvUTF8 );
    wxString carc_hash;

    wxStringTokenizer tkz( inst, _T ( ",;" ) );

    //    outline color
    wxString outline_color = tkz.GetNextToken();
    carc_hash += outline_color;
    carc_hash += _T(".");

    //    outline width
    wxString slong = tkz.GetNextToken();
    long outline_width;
    slong.ToLong( &outline_width );
    carc_hash += slong;
    carc_hash += _T(".");

    //    arc color
    wxString arc_color = tkz.GetNextToken();
    carc_hash += arc_color;
    carc_hash += _T(".");

    //    arc width
    slong = tkz.GetNextToken();
    long arc_width;
    slong.ToLong( &arc_width );
    carc_hash += slong;
    carc_hash += _T(".");

    //    sectr1
    slong = tkz.GetNextToken();
    double sectr1;
    slong.ToDouble( &sectr1 );
    carc_hash += slong;
    carc_hash += _T(".");

    //    sectr2
    slong = tkz.GetNextToken();
    double sectr2;
    slong.ToDouble( &sectr2 );
    carc_hash += slong;
    carc_hash += _T(".");

    //    arc radius
    slong = tkz.GetNextToken();
    long radius;
    slong.ToLong( &radius );
    carc_hash += slong;
    carc_hash += _T(".");

    //    sector radius
    slong = tkz.GetNextToken();
    long sector_radius;
    slong.ToLong( &sector_radius );
    carc_hash += slong;
    carc_hash += _T(".");

    slong.Printf( _T("%d"), m_colortable_index );
    carc_hash += slong;

    // Center point
    wxPoint r;
    GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

    //  radius scaled to display
    float rad =  radius * canvas_pix_per_mm ;
    float arcw = arc_width * canvas_pix_per_mm;
    float sec_rad = sector_radius * canvas_pix_per_mm;
    
    // Adjust size
    //  Some plain lights have no SCAMIN attribute.
    //  This causes display congestion at small viewing scales, since the objects are rendered at fixed pixel dimensions from the LUP rules.
    //  As a correction, the idea is to not allow the rendered symbol to be larger than "X" meters on the chart.
    //   and scale it down when rendered if necessary.
    
    float xscale = 1.0;
    if(1/*rzRules->obj->Scamin > 10000000*/){                        // huge (unset) SCAMIN)
        float radius_meters_target = 200;
        
        float radius_meters = ( radius * canvas_pix_per_mm ) / vp->view_scale_ppm;
        
        xscale = radius_meters_target / radius_meters;
        xscale = wxMin(xscale, 1.0);
        xscale = wxMax(.4, xscale);
        
        rad *= xscale;
        arcw *= xscale;
        arcw =wxMin(arcw, rad/10);
        sec_rad *= xscale;
    }
    //      Enable anti-aliased lines, at best quality
    glEnable( GL_BLEND );

    // Rotate the center point about vp center
    wxPoint point = r;
    double sin_rot = sin( vp->rotation );
    double cos_rot = cos( vp->rotation );

    double xp = ( (point.x - vp->pix_width/2) * cos_rot ) - ( (point.y - vp->pix_height/2) * sin_rot );
    double yp = ( (point.x - vp->pix_width/2) * sin_rot ) + ( (point.y - vp->pix_height/2) * cos_rot );

    point.x = (int) xp + vp->pix_width/2;
    point.y = (int) yp + vp->pix_height/2;

    float coords[8];
    coords[0] = -rad;  coords[1] =  rad;
    coords[2] =  rad;  coords[3] =  rad;
    coords[4] = -rad;  coords[5] = -rad;
    coords[6] =  rad;  coords[7] = -rad;

    glUseProgram( S52ring_shader_program );

    // Get pointers to the attributes in the program.
    GLint mPosAttrib = glGetAttribLocation( S52ring_shader_program, "aPos" );

    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
    glEnableVertexAttribArray( mPosAttrib );

    //  Circle radius
    GLint radiusloc = glGetUniformLocation(S52ring_shader_program,"circle_radius");
    glUniform1f(radiusloc, rad);

    //  Circle center point, physical
    GLint centerloc = glGetUniformLocation(S52ring_shader_program,"circle_center");
    float ctrv[2];
    ctrv[0] = point.x; ctrv[1] = GetOCPNCanvasWindow()->GetSize().y - point.y;
    glUniform2fv(centerloc, 1, ctrv);

    //  Circle color
    wxColour colorb = getwxColour( arc_color );
    float colorv[4];
    colorv[0] = colorb.Red() / float(256);
    colorv[1] = colorb.Green() / float(256);
    colorv[2] = colorb.Blue() / float(256);
    colorv[3] = 1.0;

    GLint colloc = glGetUniformLocation(S52ring_shader_program,"circle_color");
    glUniform4fv(colloc, 1, colorv);

    //  Border color
    float bcolorv[4];
    bcolorv[0] = 0;
    bcolorv[1] = 0;
    bcolorv[2] = 0;
    bcolorv[3] = 1.0;

    GLint bcolloc = glGetUniformLocation(S52ring_shader_program,"border_color");
    glUniform4fv(bcolloc, 1, bcolorv);

    //  Border Width
    GLint borderWidthloc = glGetUniformLocation(S52ring_shader_program,"border_width");
    glUniform1f(borderWidthloc, 2);

    //  Ring width
    GLint ringWidthloc = glGetUniformLocation(S52ring_shader_program,"ring_width");
    glUniform1f(ringWidthloc, arcw);

    //  Visible sectors, rotated to vp orientation
    float sr1 = sectr1 + (vp->rotation * 180 / PI);
    if(sr1 > 360.) sr1 -= 360.;
    float sr2 = sectr2 + (vp->rotation * 180 / PI);
    if(sr2 > 360.) sr2 -= 360.;

    float sb, se;
    if ( sr2 > sr1 ){
        sb = sr1;
        se = sr2;
    }
    else{
        sb = sr1;
        se = sr2 + 360;
    }

    //  Shader can handle angles > 360.
    if( (sb < 0) || (se < 0)){
        sb += 360.;
        se += 360.;
    }
    
    GLint sector1loc = glGetUniformLocation(S52ring_shader_program,"sector_1");
    glUniform1f(sector1loc, (sb * PI / 180.));
    GLint sector2loc = glGetUniformLocation(S52ring_shader_program,"sector_2");
    glUniform1f(sector2loc, (se * PI / 180.));


    // Rotate and translate
    mat4x4 I, Q;
    mat4x4_identity(I);

    mat4x4_translate_in_place(I, r.x, r.y, 0);

    GLint matloc = glGetUniformLocation(S52ring_shader_program,"TransformMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)I);

    // Perform the actual drawing.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Restore the per-object transform to Identity Matrix
    mat4x4 IM;
    mat4x4_identity(IM);
    GLint matlocf = glGetUniformLocation(S52ring_shader_program,"TransformMatrix");
    glUniformMatrix4fv( matlocf, 1, GL_FALSE, (const GLfloat*)IM);

    //    Draw the sector legs directly on the target DC
    if( sector_radius > 0 ) {
        int leg_len = (int) ( sec_rad );

        wxDash dash1[2];
        dash1[0] = (int) ( 3.6 * canvas_pix_per_mm / 3 ); //8// Long dash  <---------+
        dash1[1] = (int) ( 1.8 * canvas_pix_per_mm / 3); //2// Short gap            |

        wxPen thispen = *wxBLACK_PEN;
        thispen.SetDashes( 2, dash1 );
        thispen.SetWidth(3);
        thispen.SetStyle(wxPENSTYLE_USER_DASH);
        
        float a = ( sectr1 - 90 ) * PI / 180;
        a += vp->rotation;
        int x = point.x + (int) ( leg_len * cosf( a ) );
        int y = point.y + (int) ( leg_len * sinf( a ) );
        DrawDashLine(thispen, point.x, point.y, x, y, vp);
        
        a = ( sectr2 - 90 ) * PI / 180.;
        a += vp->rotation;
        x = point.x + (int) ( leg_len * cosf( a ) );
        y = point.y + (int) ( leg_len * sinf( a ) );
        DrawDashLine(thispen, point.x, point.y, x, y, vp);
    }
    glDisable( GL_BLEND );

    //  Update the object Bounding box,
    //  so that subsequent drawing operations will redraw the item fully

    double latmin, lonmin, latmax, lonmax;

    GetPixPointSingleNoRotate( r.x - rad, r.y + rad, &latmin, &lonmin, vp );
    GetPixPointSingleNoRotate( r.x + rad, r.y - rad, &latmax, &lonmax, vp );
    LLBBox symbox;
    symbox.Set( latmin, lonmin, latmax, lonmax );
    rzRules->obj->BBObj.Expand( symbox );

//    glEnable( GL_SCISSOR_TEST );
    
#endif

    return 1;
}


int s52plib::RenderCARC_VBO( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifndef USE_ANDROID_GLES2
    char *str = (char*) rules->INSTstr;
    //    extract the parameters from the string
    //    And creating a unique string hash as we go
    wxString inst( str, wxConvUTF8 );
    wxString carc_hash;

    wxStringTokenizer tkz( inst, _T ( ",;" ) );

    //    outline color
    wxString outline_color = tkz.GetNextToken();
    carc_hash += outline_color;
    carc_hash += _T(".");

    //    outline width
    wxString slong = tkz.GetNextToken();
    long outline_width;
    slong.ToLong( &outline_width );
    carc_hash += slong;
    carc_hash += _T(".");

    //    arc color
    wxString arc_color = tkz.GetNextToken();
    carc_hash += arc_color;
    carc_hash += _T(".");

    //    arc width
    slong = tkz.GetNextToken();
    long arc_width;
    slong.ToLong( &arc_width );
    carc_hash += slong;
    carc_hash += _T(".");

    //    sectr1
    slong = tkz.GetNextToken();
    double sectr1;
    slong.ToDouble( &sectr1 );
    carc_hash += slong;
    carc_hash += _T(".");

    //    sectr2
    slong = tkz.GetNextToken();
    double sectr2;
    slong.ToDouble( &sectr2 );
    carc_hash += slong;
    carc_hash += _T(".");

    //    arc radius
    slong = tkz.GetNextToken();
    long radius;
    slong.ToLong( &radius );
    carc_hash += slong;
    carc_hash += _T(".");

    //    sector radius
    slong = tkz.GetNextToken();
    long sector_radius;
    slong.ToLong( &sector_radius );
    carc_hash += slong;
    carc_hash += _T(".");

    slong.Printf( _T("%d"), m_colortable_index );
    carc_hash += slong;

    int width;
    int height;
    int rad;
    int bm_width;
    int bm_height;
    int bm_orgx;
    int bm_orgy;

    Rule *prule = rules->razRule;

    float scale_factor = 1.0;
    
    // The dimensions of the light are presented here as pixels on-screen.
    // We must scale the rendered size based on the device pixel density
    // Let us declare that the width of the arc should be no less than X mm
    float wx = 1.0;
    
    float pd_scale = 1.0;
    float nominal_arc_width_pix = wxMax(1.0, floor(GetPPMM() * wx));             // { wx } mm nominal, but not less than 1 pixel
    pd_scale = nominal_arc_width_pix / arc_width;
    
    //scale_factor *= pd_scale;
    //qDebug() << GetPPMM() << arc_width << nominal_arc_width_pix << pd_scale;
    
    
    // Adjust size
    //  Some plain lights have no SCAMIN attribute.
    //  This causes display congestion at small viewing scales, since the objects are rendered at fixed pixel dimensions from the LUP rules.
    //  As a correction, the idea is to not allow the rendered symbol to be larger than "X" meters on the chart.
    //   and scale it down when rendered if necessary.

    float xscale = 1.0;
    if(rzRules->obj->Scamin > 10000000){                        // huge (unset) SCAMIN)
        float radius_meters_target = 200;

         float radius_meters = ( radius * canvas_pix_per_mm ) / vp->view_scale_ppm;

        xscale = radius_meters_target / radius_meters;
        xscale = wxMin(xscale, 1.0);
        xscale = wxMax(.4, xscale);

        radius *= xscale;
        sector_radius *= xscale;
    }

    ///scale_factor *= xscale;
    
    carc_hash += _T(".");
    wxString xs;
    xs.Printf( _T("%5g"), xscale );
    carc_hash += xs;

    if(m_pdc){          // DC rendering
        if(fabs(prule->parm7 - xscale) > .00001){
            ClearRulesCache( prule );
        }

    //Instantiate the symbol if necessary
    if( ( rules->razRule->pixelPtr == NULL ) || ( rules->razRule->parm1 != m_colortable_index ) ) {
        //  Render the sector light to a bitmap

        rad = (int) ( radius * canvas_pix_per_mm );

        width = ( rad * 2 ) + 28;
        height = ( rad * 2 ) + 28;
        wxBitmap bm( width, height, -1 );
        wxMemoryDC mdc;
        mdc.SelectObject( bm );
        mdc.SetBackground( wxBrush( m_unused_wxColor ) );
        mdc.Clear();

        //    Adjust sector math for wxWidgets API
        float sb;
        float se;

        //      For some reason, the __WXMSW__ build flips the sense of
        //      start and end angles on DrawEllipticArc()
#ifndef __WXMSW__
        if ( sectr2 > sectr1 )
        {
            sb = 90 - sectr1;
            se = 90 - sectr2;
        }
        else
        {
            sb = 360 + ( 90 - sectr1 );
            se = 90 - sectr2;
        }
#else
        if( sectr2 > sectr1 ) {
            se = 90 - sectr1;
            sb = 90 - sectr2;
        } else {
            se = 360 + ( 90 - sectr1 );
            sb = 90 - sectr2;
        }
#endif

        //      Here is a goofy way of computing the dc drawing extents exactly
        //      Draw a series of fat line segments approximating the arc using dc.DrawLine()
        //      This will properly establish the drawing box in the dc

        int border_fluff = 4; // by how much should the blit bitmap be "fluffed"
        
        //  wxDC min/max calculations are currently broken in wxQT, so we use the entire circle instead of arcs...
#ifndef __WXQT__        
        if( fabs( sectr2 - sectr1 ) != 360 ) // not necessary for all-round lights
                {
            mdc.ResetBoundingBox();

            wxPen *pblockpen = wxThePenList->FindOrCreatePen( *wxBLACK, 10, wxPENSTYLE_SOLID );
            mdc.SetPen( *pblockpen );

            float start_angle, end_angle;
            if( se < sb ) {
                start_angle = se;
                end_angle = sb;
            } else {
                start_angle = sb;
                end_angle = se;
            }

            int x0 = ( width / 2 ) + (int) ( rad * cos( start_angle * PI / 180. ) );
            int y0 = ( height / 2 ) - (int) ( rad * sin( start_angle * PI / 180. ) );
            for( float a = start_angle + .1; a <= end_angle; a += 2.0 ) {
                int x = ( width / 2 ) + (int) ( rad * cosf( a * PI / 180. ) );
                int y = ( height / 2 ) - (int) ( rad * sinf( a * PI / 180. ) );
                mdc.DrawLine( x0, y0, x, y );
                x0 = x;
                y0 = y;
            }

            bm_width = ( mdc.MaxX() - mdc.MinX() ) + ( border_fluff * 2 );
            bm_height = ( mdc.MaxY() - mdc.MinY() ) + ( border_fluff * 2 );
            bm_orgx = mdc.MinX()-border_fluff - width/2; //wxMax ( 0, mdc.MinX()-border_fluff );
            bm_orgy = mdc.MinY()-border_fluff - height/2; //wxMax ( 0, mdc.MinY()-border_fluff );

            mdc.Clear();
        }

        else {
            bm_width = rad * 2 + ( border_fluff * 2 );
            bm_height = rad * 2 + ( border_fluff * 2 );
            bm_orgx = -bm_width / 2;
            bm_orgy = -bm_height / 2;

        }

#else
        bm_width = rad * 2 + ( border_fluff * 2 );
        bm_height = rad * 2 + ( border_fluff * 2 );
        bm_orgx = -bm_width / 2;
        bm_orgy = -bm_height / 2; 
#endif        

        wxBitmap *sbm = NULL;

            //    Draw the outer border
            wxColour color = getwxColour( outline_color );

                wxPen *pthispen = wxThePenList->FindOrCreatePen( color, outline_width * scale_factor, wxPENSTYLE_SOLID );
            mdc.SetPen( *pthispen );
            wxBrush *pthisbrush = wxTheBrushList->FindOrCreateBrush( color, wxBRUSHSTYLE_TRANSPARENT );
            mdc.SetBrush( *pthisbrush );

            mdc.DrawEllipticArc( width / 2 - rad, height / 2 - rad, rad * 2, rad * 2, sb, se );

            if( arc_width ) {
                wxColour colorb = getwxColour( arc_color );

                if( !colorb.IsOk() ) colorb = getwxColour( _T("CHMGD") );

                    pthispen = wxThePenList->FindOrCreatePen( colorb, arc_width * scale_factor, wxPENSTYLE_SOLID );
                mdc.SetPen( *pthispen );

                mdc.DrawEllipticArc( width / 2 - rad, height / 2 - rad, rad * 2, rad * 2, sb, se );

            }

            mdc.SelectObject( wxNullBitmap );

            //          Get smallest containing bitmap
            sbm = new wxBitmap(
                bm.GetSubBitmap( wxRect( width/2 + bm_orgx, height/2 + bm_orgy, bm_width, bm_height ) ) );

            //      Make the mask
            wxMask *pmask = new wxMask( *sbm, m_unused_wxColor );

            //      Associate the mask with the bitmap
            sbm->SetMask( pmask );

            // delete any old private data
            ClearRulesCache( rules->razRule );
            
        //      Save the bitmap ptr and aux parms in the rule
        prule->pixelPtr = sbm;
        prule->parm0 = ID_wxBitmap;
        prule->parm1 = m_colortable_index;
        prule->parm2 = bm_orgx;
        prule->parm3 = bm_orgy;
        prule->parm5 = bm_width;
        prule->parm6 = bm_height;
            prule->parm7 = xscale;
    } // instantiation
    }

#ifdef ocpnUSE_GL
    CARC_Buffer buffer;

    
    if( !m_pdc ) // opengl
    {
        //    Is there not already an generated vbo the CARC_hashmap for this object?
        rad = (int) ( radius * canvas_pix_per_mm );
        if( m_CARC_hashmap.find( carc_hash ) == m_CARC_hashmap.end() ) {
            
            if( sectr1 > sectr2 ) sectr2 += 360;
    
            /* to ensure that the final segment lands exactly on sectr2 */

            //    Draw wide outline arc
            wxColour colorb = getwxColour( outline_color );
            buffer.color[0][0] = colorb.Red();
            buffer.color[0][1] = colorb.Green();
            buffer.color[0][2] = colorb.Blue();
            buffer.color[0][3] = 150;
            buffer.line_width[0] = wxMax(g_GLMinSymbolLineWidth, outline_width * scale_factor);

            int steps = ceil((sectr2 - sectr1) / 12) + 1; // max of 12 degree step
            float step = (sectr2 - sectr1) / (steps - 1);

            buffer.steps = steps;
            buffer.size = 2*(steps + 4);
            buffer.data = new float[buffer.size];

            int s = 0;
            for(int i = 0; i < steps; i++) {
                float a = (sectr1 + i * step) * M_PI / 180.0;
                buffer.data[s++] = rad * sinf( a );
                buffer.data[s++] = -rad * cosf( a );
            }
    
            //    Draw narrower color arc, overlaying the drawn outline.
            colorb = getwxColour( arc_color );
            buffer.color[1][0] = colorb.Red();
            buffer.color[1][1] = colorb.Green();
            buffer.color[1][2] = colorb.Blue();
            buffer.color[1][3] = 150;
            buffer.line_width[1] = wxMax(g_GLMinSymbolLineWidth, (arc_width  * scale_factor) + .8);
        
            //    Draw the sector legs
            if( sector_radius > 0 ) {
                int leg_len = (int) ( sector_radius * canvas_pix_per_mm );
        
                //wxColour c = GetGlobalColor( _T ( "CHBLK" ) );
                wxColour c; GetGlobalColor( _T ( "CHBLK" ), &c);

                buffer.color[2][0] = c.Red();
                buffer.color[2][1] = c.Green();
                buffer.color[2][2] = c.Blue();
                buffer.color[2][3] = c.Alpha();
                //buffer.line_width[2] = wxMax(g_GLMinSymbolLineWidth, (float)0.5) * scale_factor;
                buffer.line_width[2] = wxMax(1.0, floor(GetPPMM() * 0.2));             //0.4 mm nominal, but not less than 1 pixel
        
                float a = ( sectr1 - 90 ) * PI / 180.;
                buffer.data[s++] = 0;
                buffer.data[s++] = 0;
                buffer.data[s++] = leg_len * cosf( a );
                buffer.data[s++] = leg_len * sinf( a );

                a = ( sectr2 - 90 ) * PI / 180.;
                buffer.data[s++] = 0;
                buffer.data[s++] = 0;
                buffer.data[s++] = leg_len * cosf( a );
                buffer.data[s++] = leg_len * sinf( a );
            } else
                buffer.line_width[2] = 0;

            m_CARC_hashmap[carc_hash] = buffer;

        } else
            buffer = m_CARC_hashmap[carc_hash];
        
        int border_fluff = 4; // by how much should the blit bitmap be "fluffed"
        bm_width = rad * 2 + ( border_fluff * 2 );
        bm_height = rad * 2 + ( border_fluff * 2 );
        bm_orgx = -bm_width / 2;
        bm_orgy = -bm_height / 2;
        
        prule->parm2 = bm_orgx;
        prule->parm3 = bm_orgy;
        prule->parm5 = bm_width;
        prule->parm6 = bm_height;
        prule->parm7 = xscale;
        
    } // instantiation
#endif


    //  Render arcs at object's x/y
    wxPoint r;
    GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

    //      Now render the symbol
    if( !m_pdc ) // opengl
    {
#ifdef ocpnUSE_GL
        glPushMatrix();
        glTranslatef( r.x, r.y, 0 );
         
//        glScalef(scale_factor, scale_factor, 1);
        glVertexPointer(2, GL_FLOAT, 2 * sizeof(float), buffer.data);

#ifndef __OCPN__ANDROID__
        glEnable( GL_BLEND );
        if( m_GLLineSmoothing )
            glEnable( GL_LINE_SMOOTH );
#endif        
        glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array

        glColor3ubv(buffer.color[0]);
        glLineWidth(buffer.line_width[0]);
        glDrawArrays(GL_LINE_STRIP, 0, buffer.steps);


        glColor3ubv(buffer.color[1]);
        glLineWidth(buffer.line_width[1]);
        glDrawArrays(GL_LINE_STRIP, 0, buffer.steps);

        //qDebug() << buffer.line_width[0] << buffer.line_width[1] << buffer.line_width[2];
        
        if(buffer.line_width[2]) {
#ifndef ocpnUSE_GLES // linestipple is emulated poorly
            glLineStipple( 1, 0x3F3F );
            glEnable( GL_LINE_STIPPLE );
#endif
            glColor3ubv(buffer.color[2]);
            glLineWidth(buffer.line_width[2]);
            glDrawArrays(GL_LINES, buffer.steps, 4);
#ifndef ocpnUSE_GLES
            glDisable( GL_LINE_STIPPLE );
#endif
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_BLEND );
    
        // Debug the symbol bounding box.....
/*        
        {
            int x0 = rules->razRule->parm2;
            int y0 = rules->razRule->parm3;

            glLineWidth( 2 );
            glColor4f( 0,0,0,0 );
            
            glBegin( GL_LINE_STRIP );
            glVertex2i( x0, y0 );
            glVertex2i( x0 + b_width, y0 );
            glVertex2i( x0 + b_width, y0 + b_height );
            glVertex2i( x0, y0 + b_height);
            glVertex2i( x0, y0 );
            glEnd();
        }
        */ 

//        glTranslatef( -r.x, -r.y, 0 );
        glPopMatrix();
#endif        
    } else {
        int b_width = prule->parm5;
        int b_height = prule->parm6;
        
        //      Get the bitmap into a memory dc
        wxMemoryDC mdc;
        mdc.SelectObject( (wxBitmap &) ( *( (wxBitmap *) ( rules->razRule->pixelPtr ) ) ) );

        //      Blit it into the target dc, using mask
        m_pdc->Blit( r.x + rules->razRule->parm2, r.y + rules->razRule->parm3, b_width, b_height,
                &mdc, 0, 0, wxCOPY, true );

        mdc.SelectObject( wxNullBitmap );

        //    Draw the sector legs directly on the target DC
        //    so that anti-aliasing works against the drawn image (cannot be cached...)
        if( sector_radius > 0 ) {
            int leg_len = (int) ( sector_radius * canvas_pix_per_mm );

            wxDash dash1[2];
            dash1[0] = (int) ( 3.6 * canvas_pix_per_mm ); //8// Long dash  <---------+
            dash1[1] = (int) ( 1.8 * canvas_pix_per_mm ); //2// Short gap            |

            /*
             wxPen *pthispen = new wxPen(*wxBLACK_PEN);
             pthispen->SetStyle(wxPENSTYLE_USER_DASH);
             pthispen->SetDashes( 2, dash1 );
             //      Undocumented "feature":  Pen must be fully specified <<<BEFORE>>> setting into DC
             pdc->SetPen ( *pthispen );
             */
            //wxColour c = GetGlobalColor( _T ( "CHBLK" ) );
            wxColour c; GetGlobalColor( _T ( "CHBLK" ), &c);

            float a = ( sectr1 - 90 ) * PI / 180;
            int x = r.x + (int) ( leg_len * cosf( a ) );
            int y = r.y + (int) ( leg_len * sinf( a ) );
            DrawAALine( m_pdc, r.x, r.y, x, y, c, dash1[0], dash1[1] );

            a = ( sectr2 - 90 ) * PI / 180.;
            x = r.x + (int) ( leg_len * cosf( a ) );
            y = r.y + (int) ( leg_len * sinf( a ) );
            DrawAALine( m_pdc, r.x, r.y, x, y, c, dash1[0], dash1[1] );
        }
        
        // Debug the symbol bounding box.....
/*        
        if(m_pdc){
            m_pdc->SetPen(wxPen(*wxGREEN, 1));
            m_pdc->SetBrush(wxBrush(*wxGREEN, wxTRANSPARENT));
            m_pdc->DrawRectangle( r.x + rules->razRule->parm2, r.y + rules->razRule->parm3, b_width, b_height);
        }
        */
    }

    //  Update the object Bounding box,
    //  so that subsequent drawing operations will redraw the item fully

    double latmin, lonmin, latmax, lonmax;

    GetPixPointSingleNoRotate( r.x + prule->parm2,                r.y + prule->parm3 + prule->parm6, &latmin, &lonmin, vp );
    GetPixPointSingleNoRotate( r.x + prule->parm2 + prule->parm5, r.y + prule->parm3,                &latmax, &lonmax, vp );
    LLBBox symbox;
    symbox.Set( latmin, lonmin, latmax, lonmax );
    rzRules->obj->BBObj.Expand( symbox );
#endif
    return 1;
}
    
// Conditional Symbology
char *s52plib::RenderCS( ObjRazRules *rzRules, Rules *rules )
{
    void *ret;
    void* (*f)( void* );

    static int f05;

    if( rules->razRule == NULL ) {
        if( !f05 )
        //                  CPLError ( ( CPLErr ) 0, 0,"S52plib:_renderCS(): ERROR no conditional symbology for: %s\n", rules->INSTstr );
        f05++;
        return 0;
    }

    void *g = (void *) rules->razRule;

#ifdef FIX_FOR_MSVC  //__WXMSW__
//#warning Fix this cast, somehow...
//      dsr             sigh... can't get the cast right
    _asm
    {
        mov eax,[dword ptr g]
        mov [dword ptr f],eax
    }
    ret = f ( ( void * ) rzRules ); // call cond symb
#else

    f = (void * (*)( void * ) ) g;ret
    = f( (void *) rzRules );

#endif

    return (char *) ret;
}

int s52plib::RenderObjectToDC( wxDC *pdcin, ObjRazRules *rzRules, ViewPort *vp )
{
    return DoRenderObject( pdcin, rzRules, vp );
}


int s52plib::RenderObjectToGL( const wxGLContext &glcc, ObjRazRules *rzRules, ViewPort *vp )
{
    m_glcc = (wxGLContext *) &glcc;
    return DoRenderObject( NULL, rzRules, vp );
}

int s52plib::RenderObjectToDCText( wxDC *pdcin, ObjRazRules *rzRules, ViewPort *vp )
{
    return DoRenderObjectTextOnly( pdcin, rzRules, vp );
}

int s52plib::RenderObjectToGLText( const wxGLContext &glcc, ObjRazRules *rzRules, ViewPort *vp )
{
    m_glcc = (wxGLContext *) &glcc;
    return DoRenderObjectTextOnly( NULL, rzRules, vp );
}



int s52plib::DoRenderObject( wxDC *pdcin, ObjRazRules *rzRules, ViewPort *vp )
{
    //TODO  Debugging
//      if(rzRules->obj->Index != 1103)
//          return 0; //int yyp = 0;

//        if(!strncmp(rzRules->obj->FeatureName, "berths", 6))
//            int yyp = 0;

    if( !ObjectRenderCheckRules( rzRules, vp, true ) )
        return 0;

    m_pdc = pdcin; // use this DC
    Rules *rules = rzRules->LUP->ruleList;

    while( rules != NULL ) {
        switch( rules->ruleType ){
            case RUL_TXT_TX:
                RenderTX( rzRules, rules, vp );
                break; // TX
            case RUL_TXT_TE:
                RenderTE( rzRules, rules, vp );
                break; // TE
            case RUL_SYM_PT:
                RenderSY( rzRules, rules, vp );
                break; // SY
            case RUL_SIM_LN:
                if(m_pdc)
                    RenderLS( rzRules, rules, vp );
                else
                    RenderGLLS( rzRules, rules, vp );
                break; // LS
            case RUL_COM_LN:
                RenderLC( rzRules, rules, vp );
                break; // LC
            case RUL_MUL_SG:
                RenderMPS( rzRules, rules, vp );
                break; // MultiPoint Sounding
            case RUL_ARC_2C:
                RenderCARC( rzRules, rules, vp );
                break; // Circular Arc, 2 colors

            case RUL_CND_SY: {
                if( !rzRules->obj->bCS_Added ) {
                    rzRules->obj->CSrules = NULL;
                    GetAndAddCSRules( rzRules, rules );
                    if(strncmp(rzRules->obj->FeatureName, "SOUNDG", 6))
                        rzRules->obj->bCS_Added = 1; // mark the object
                }

                Rules *rules_last = rules;
                rules = rzRules->obj->CSrules;

                while( NULL != rules ) {
                        switch( rules->ruleType ){
                            case RUL_TXT_TX:
                                RenderTX( rzRules, rules, vp );
                                break;
                            case RUL_TXT_TE:
                                RenderTE( rzRules, rules, vp );
                                break;
                            case RUL_SYM_PT:
                                RenderSY( rzRules, rules, vp );
                                break;
                            case RUL_SIM_LN:
                                if(m_pdc)
                                    RenderLS( rzRules, rules, vp );
                                else
                                    RenderGLLS( rzRules, rules, vp );
                                break; // LS
                            case RUL_COM_LN:
                                RenderLC( rzRules, rules, vp );
                                break;
                            case RUL_MUL_SG:
                                RenderMPS( rzRules, rules, vp );
                                break; // MultiPoint Sounding
                            case RUL_ARC_2C:
                                RenderCARC( rzRules, rules, vp );
                                break; // Circular Arc, 2 colors
                            case RUL_NONE:
                            default:
                                break; // no rule type (init)
                        }
                        rules_last = rules;
                        rules = rules->next;
                }

                rules = rules_last;
                break;
            }

            case RUL_NONE:
            default:
                break; // no rule type (init)
        } // switch

        rules = rules->next;
    }

    return 1;
}

int s52plib::DoRenderObjectTextOnly( wxDC *pdcin, ObjRazRules *rzRules, ViewPort *vp )
{
//    if(strncmp(rzRules->obj->FeatureName, "RDOCAL", 6))
//        return 0;

//    if(rzRules->obj->Index == 2766)
//        int yyp = 4;
    
    if( !ObjectRenderCheckRules( rzRules, vp, true ) )
        return 0;

    m_pdc = pdcin; // use this DC
    Rules *rules = rzRules->LUP->ruleList;
    
    while( rules != NULL ) {
        switch( rules->ruleType ){
            case RUL_TXT_TX:
                RenderTX( rzRules, rules, vp );
                break; // TX
            case RUL_TXT_TE:
                RenderTE( rzRules, rules, vp );
                break; // TE
            case RUL_CND_SY: {
                if( !rzRules->obj->bCS_Added ) {
                    rzRules->obj->CSrules = NULL;
                    GetAndAddCSRules( rzRules, rules );
                    if(strncmp(rzRules->obj->FeatureName, "SOUNDG", 6))
                        rzRules->obj->bCS_Added = 1; // mark the object
                }
                
                Rules *rules_last = rules;
                rules = rzRules->obj->CSrules;
                
                while( NULL != rules ) {
                    switch( rules->ruleType ){
                        case RUL_TXT_TX:
                            RenderTX( rzRules, rules, vp );
                            break;
                        case RUL_TXT_TE:
                            RenderTE( rzRules, rules, vp );
                            break;
                        default:
                            break; // no rule type (init)
                    }
                    rules_last = rules;
                    rules = rules->next;
                }
                
                rules = rules_last;
                break;
            }
            
            case RUL_NONE:
            default:
                 break; // no rule type (init)
        } // switch
        
        rules = rules->next;
    }
    
    return 1;
}

bool s52plib::PreloadOBJLFromCSV(const wxString &csv_file)
{
    wxTextFile file( csv_file );
    if( !file.Exists() ) return false;

    file.Open();

    wxString str;
    str = file.GetFirstLine();
    wxChar quote[] = { '\"', 0 };
    wxString description;
    wxString token;

    while( !file.Eof() ) {
        str = file.GetNextLine();

        wxStringTokenizer tkz( str, _T(",") );
        token = tkz.GetNextToken(); // code

        description = tkz.GetNextToken(); // May contain comma
        if( !description.EndsWith( quote ) ) description << tkz.GetNextToken();
        description.Replace( _T("\""), _T(""), true );

        token = tkz.GetNextToken(); // Acronym

        if( token.Len() ) {
            //    Filter out any duplicates, in a case insensitive way
            //    i.e. only the first of "DEPARE" and "depare" is added
            bool bdup = false;
            for( unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++ ) {
                OBJLElement *pOLEt = (OBJLElement *) ( pOBJLArray->Item( iPtr ) );
                if( !token.CmpNoCase( wxString( pOLEt->OBJLName, wxConvUTF8 ) ) ) {
                    bdup = true;
                    break;
                }
            }

            if( !bdup ) {
                wxCharBuffer buffer=token.ToUTF8();
                if(buffer.data()) {
                    OBJLElement *pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
                    memcpy( pOLE->OBJLName, buffer.data(), 6 );
                    pOLE->nViz = 0;

                    pOBJLArray->Add( (void *) pOLE );

                    OBJLDescriptions.push_back( description );
                }
            }
        }
    }
    return true;
}

void s52plib::UpdateOBJLArray( S57Obj *obj )
{
    //    Search the array for this object class

    bool bNeedNew = true;
    OBJLElement *pOLE;

    for( unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++ ) {
        pOLE = (OBJLElement *) ( pOBJLArray->Item( iPtr ) );
        if( !strncmp( pOLE->OBJLName, obj->FeatureName, 6 ) ) {
            obj->iOBJL = iPtr;
            bNeedNew = false;
            break;
        }
    }

    //    Not found yet, so add an element
    if( bNeedNew ) {
        pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
        memcpy( pOLE->OBJLName, obj->FeatureName, OBJL_NAME_LEN );
        pOLE->nViz = 1;

        pOBJLArray->Add( (void *) pOLE );
        obj->iOBJL = pOBJLArray->GetCount() - 1;
    }

}

int s52plib::SetLineFeaturePriority( ObjRazRules *rzRules, int npriority )
{

    int priority_set = npriority; // may be adjusted

    Rules *rules = rzRules->LUP->ruleList;

    //      Do Object Type Filtering
    //    If the object s not currently visible (i.e. classed as a not-currently visible category),
    //    then do not set the line segment priorities at all
    //    

    bool b_catfilter = true;

   // DEPCNT is mutable
    if( m_nDisplayCategory == STANDARD ) {
        if( ( DISPLAYBASE != rzRules->LUP->DISC ) && ( STANDARD != rzRules->LUP->DISC ) ) {
            b_catfilter = rzRules->obj->m_bcategory_mutable;
        }
    } else if( m_nDisplayCategory == DISPLAYBASE ) {
        if( DISPLAYBASE != rzRules->LUP->DISC ) {
            b_catfilter = rzRules->obj->m_bcategory_mutable;
        }
    }

    if( IsObjNoshow( rzRules->LUP->OBCL) )
        b_catfilter = false;
    
    if(!b_catfilter)            // No chance this object is visible
        return 0;


    while( rules != NULL ) {
        switch( rules->ruleType ){

            case RUL_SIM_LN:
            case RUL_COM_LN:
                PrioritizeLineFeature( rzRules, priority_set );
                break; // LC

            case RUL_CND_SY: {
                if( !rzRules->obj->bCS_Added ) {
                    rzRules->obj->CSrules = NULL;
                    GetAndAddCSRules( rzRules, rules );
                    rzRules->obj->bCS_Added = 1; // mark the object
                }
                Rules *rules_last = rules;
                rules = rzRules->obj->CSrules;

                while( NULL != rules ) {
                    switch( rules->ruleType ){
                        case RUL_SIM_LN:
                        case RUL_COM_LN:
                            PrioritizeLineFeature( rzRules, priority_set );
                            break;
                        case RUL_NONE:
                        default:
                            break; // no rule type (init)
                    }
                    rules_last = rules;
                    rules = rules->next;
                }

                rules = rules_last;
                break;
            }

            case RUL_NONE:
            default:
                break; // no rule type (init)
        } // switch

        rules = rules->next;
    }

    return 1;
}

int s52plib::PrioritizeLineFeature( ObjRazRules *rzRules, int npriority )
{
    if(rzRules->obj->m_ls_list){
        
        VE_Element *pedge;
        connector_segment *pcs;
        line_segment_element *ls = rzRules->obj->m_ls_list;
        while( ls ){
            switch (ls->ls_type){
                case TYPE_EE:
                case TYPE_EE_REV:
                    
                    pedge = ls->pedge; //(VE_Element *)ls->private0;
                    if(pedge)
                        pedge->max_priority = npriority;// wxMax(pedge->max_priority, npriority);
                    break;
                    
                default:
                    pcs = ls->pcs; //(connector_segment *)ls->private0;
                    if(pcs)
                        pcs->max_priority_cs = npriority; //wxMax(pcs->max_priority, npriority);
                    break;
            }
            
            ls = ls->next;
        }
    }

    else if(rzRules->obj->m_ls_list_legacy){            // PlugIn (S63)
        
        PI_connector_segment *pcs;
        VE_Element *pedge;
        
        PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;
        while( ls ){
            switch (ls->type){
                case TYPE_EE:
                    
                    pedge = (VE_Element *)ls->private0;
                    if(pedge)
                        pedge->max_priority = npriority;// wxMax(pedge->max_priority, npriority);
                    break;
                    
                default:
                    pcs = (PI_connector_segment *)ls->private0;
                    if(pcs)
                        pcs->max_priority = npriority; //wxMax(pcs->max_priority, npriority);
                    break;
            }
            
            ls = ls->next;
        }
    }
#if 0    
    else if( rzRules->obj->m_n_lsindex && rzRules->obj->m_lsindex_array) {
        VE_Hash *edge_hash; 
        
        if( rzRules->obj->m_chart_context->chart ){
            edge_hash = &rzRules->obj->m_chart_context->chart->Get_ve_hash(); 
        }
        else {
            edge_hash = (VE_Hash *)rzRules->obj->m_chart_context->m_pve_hash; 
        }
        
        int *index_run = rzRules->obj->m_lsindex_array;

        for( int iseg = 0; iseg < rzRules->obj->m_n_lsindex; iseg++ ) {
            //  Get first connected node
            int inode = *index_run++;

            VE_Element *pedge = 0;
            //  Get the edge
            int enode = *index_run++;
            if(enode)
                pedge = (*edge_hash)[enode];

            //    Set priority
            if(pedge){
                pedge->max_priority = npriority;
            }

            //  Get last connected node
            inode = *index_run++;

        }
    }
#endif
        
    return 1;
}

class XPOINT {
public:
    float x, y;
};

class XLINE {
public:
    XPOINT o, p;
    float m;
    float c;
};

bool TestLinesIntersection( XLINE &a, XLINE &b )
{
    XPOINT i;

    if( ( a.p.x == a.o.x ) && ( b.p.x == b.o.x ) ) // both vertical
            {
        return ( a.p.x == b.p.x );
    }

    if( a.p.x == a.o.x ) // a line a is vertical
            {
        // calculate b gradient
        b.m = ( b.p.y - b.o.y ) / ( b.p.x - b.o.x );
        // calculate axis intersect values
        b.c = b.o.y - ( b.m * b.o.x );
        // calculate y point of intercept
        i.y = b.o.y + ( ( a.o.x - b.o.x ) * b.m );
        if( i.y < wxMin(a.o.y, a.p.y) || i.y > wxMax(a.o.y, a.p.y) ) return false;
        return true;
    }

    if( b.p.x == b.o.x ) // line b is vertical
            {
        // calculate b gradient
        a.m = ( a.p.y - a.o.y ) / ( a.p.x - a.o.x );
        // calculate axis intersect values
        a.c = a.o.y - ( a.m * a.o.x );
        // calculate y point of intercept
        i.y = a.o.y + ( ( b.o.x - a.o.x ) * a.m );
        if( i.y < wxMin(b.o.y, b.p.y) || i.y > wxMax(b.o.y, b.p.y) ) return false;
        return true;
    }

// calculate gradients
    a.m = ( a.p.y - a.o.y ) / ( a.p.x - a.o.x );
    b.m = ( b.p.y - b.o.y ) / ( b.p.x - b.o.x );
// parallel lines can't intercept
    if( a.m == b.m ) {
        return false;
    }
    // calculate axis intersect values
    a.c = a.o.y - ( a.m * a.o.x );
    b.c = b.o.y - ( b.m * b.o.x );
// calculate x point of intercept
    i.x = ( b.c - a.c ) / ( a.m - b.m );
// is intersection point in segment
    if( i.x < wxMin(a.o.x, a.p.x) || i.x > wxMax(a.o.x, a.p.x) ) {
        return false;
    }
    if( i.x < wxMin(b.o.x, b.p.x) || i.x > wxMax(b.o.x, b.p.x) ) {
        return false;
    }
// points intercept
    return true;
}

//-----------------------------------------------------------------------
//    Check a triangle described by point array, and rectangle described by render_canvas_parms
//    for intersection
//    Return false if no intersection
//-----------------------------------------------------------------------
bool s52plib::inter_tri_rect( wxPoint *ptp, render_canvas_parms *pb_spec )
{
    //    First stage
    //    Check all three points of triangle to see it any are within the render rectangle

    wxBoundingBox rect( pb_spec->lclip, pb_spec->y, pb_spec->rclip, pb_spec->y + pb_spec->height );

    for( int i = 0; i < 3; i++ ) {
        if( rect.PointInBox( ptp[i].x, ptp[i].y ) ) return true;
    }

    //    Next stage
    //    Check all four points of rectangle to see it any are within the render triangle

    double p[6];
    MyPoint *pmp = (MyPoint *) p;

    for( int i = 0; i < 3; i++ ) {
        pmp[i].x = ptp[i].x;
        pmp[i].y = ptp[i].y;
    }

    if( G_PtInPolygon( pmp, 3, pb_spec->lclip, pb_spec->y ) ) return true;

    if( G_PtInPolygon( pmp, 3, pb_spec->lclip, pb_spec->y + pb_spec->height ) ) return true;

    if( G_PtInPolygon( pmp, 3, pb_spec->rclip, pb_spec->y ) ) return true;

    if( G_PtInPolygon( pmp, 3, pb_spec->rclip, pb_spec->y + pb_spec->height ) ) return true;

    //    last step
    //    Check triangle lines against rect lines for line intersect

    for( int i = 0; i < 3; i++ ) {
        XLINE a;
        a.o.x = ptp[i].x;
        a.o.y = ptp[i].y;
        if( i == 2 ) {
            a.p.x = ptp[0].x;
            a.p.y = ptp[0].y;
        } else {
            a.p.x = ptp[i + 1].x;
            a.p.y = ptp[i + 1].y;
        }

        XLINE b;

        //    top line
        b.o.x = pb_spec->lclip;
        b.o.y = pb_spec->y;
        b.p.x = pb_spec->rclip;
        b.p.y = pb_spec->y;

        if( TestLinesIntersection( a, b ) ) return true;

        //    right line
        b.o.x = pb_spec->rclip;
        b.o.y = pb_spec->y;
        b.p.x = pb_spec->rclip;
        b.p.y = pb_spec->y + pb_spec->height;

        if( TestLinesIntersection( a, b ) ) return true;

        //    bottom line
        b.o.x = pb_spec->rclip;
        b.o.y = pb_spec->y + pb_spec->height;
        b.p.x = pb_spec->lclip;
        b.p.y = pb_spec->y + pb_spec->height;

        if( TestLinesIntersection( a, b ) ) return true;

        //    left line
        b.o.x = pb_spec->lclip;
        b.o.y = pb_spec->y + pb_spec->height;
        b.p.x = pb_spec->lclip;
        b.p.y = pb_spec->y;

        if( TestLinesIntersection( a, b ) ) return true;
    }

    return false; // no Intersection

}

//----------------------------------------------------------------------------------
//
//              Fast Basic Canvas Rendering
//              Render triangle
//
//----------------------------------------------------------------------------------
int s52plib::dda_tri( wxPoint *ptp, S52color *c, render_canvas_parms *pb_spec,
        render_canvas_parms *pPatt_spec )
{
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;

    if( !inter_tri_rect( ptp, pb_spec ) ) return 0;

    if( NULL != c ) {
        if(pb_spec->b_revrgb) {
            r = c->R;
            g = c->G;
            b = c->B;
        }
        else {
            b = c->R;
            g = c->G;
            r = c->B;
        }
    }

    //      Color Debug
    /*    int fc = rand();
     b = fc & 0xff;
     g = fc & 0xff;
     r = fc & 0xff;
     */

    int color_int = 0;
    if( NULL != c ) color_int = ( ( r ) << 16 ) + ( ( g ) << 8 ) + ( b );

    //      Determine ymin and ymax indices

    int ymax = ptp[0].y;
    int ymin = ymax;
    int xmin, xmax, xmid, ymid;
    int imin = 0;
    int imax = 0;
    int imid;

    for( int ip = 1; ip < 3; ip++ ) {
        if( ptp[ip].y > ymax ) {
            imax = ip;
            ymax = ptp[ip].y;
        }
        if( ptp[ip].y <= ymin ) {
            imin = ip;
            ymin = ptp[ip].y;
        }
    }

    imid = 3 - ( imin + imax ); // do the math...

    xmax = ptp[imax].x;
    xmin = ptp[imin].x;
    xmid = ptp[imid].x;
    ymid = ptp[imid].y;

    //      Create edge arrays using fast integer DDA
    int m, x, dy, count;
    bool dda8 = false;
    bool cw;

    if( ( abs( xmax - xmin ) > 32768 ) || ( abs( xmid - xmin ) > 32768 )
            || ( abs( xmax - xmid ) > 32768 ) || ( abs( ymax - ymin ) > 32768 )
            || ( abs( ymid - ymin ) > 32768 ) || ( abs( ymax - ymid ) > 32768 ) || ( xmin > 32768 )
            || ( xmid > 32768 ) ) {
        dda8 = true;

        dy = ( ymax - ymin );
        if( dy ) {
            m = ( xmax - xmin ) << 8;
            m /= dy;

            x = xmin << 8;

            for( count = ymin; count <= ymax; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) ledge[count] = x >> 8;
                x += m;
            }
        }

        dy = ( ymid - ymin );
        if( dy ) {
            m = ( xmid - xmin ) << 8;
            m /= dy;

            x = xmin << 8;

            for( count = ymin; count <= ymid; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) redge[count] = x >> 8;
                x += m;
            }
        }

        dy = ( ymax - ymid );
        if( dy ) {
            m = ( xmax - xmid ) << 8;
            m /= dy;

            x = xmid << 8;

            for( count = ymid; count <= ymax; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) redge[count] = x >> 8;
                x += m;
            }
        }

        double ddfSum = 0;
        //      Check the triangle edge winding direction
        ddfSum += ( xmin / 1 ) * ( ymax / 1 ) - ( ymin / 1 ) * ( xmax / 1 );
        ddfSum += ( xmax / 1 ) * ( ymid / 1 ) - ( ymax / 1 ) * ( xmid / 1 );
        ddfSum += ( xmid / 1 ) * ( ymin / 1 ) - ( ymid / 1 ) * ( xmin / 1 );
        cw = ddfSum < 0;

    } else {

        dy = ( ymax - ymin );
        if( dy ) {
            m = ( xmax - xmin ) << 16;
            m /= dy;

            x = xmin << 16;

            for( count = ymin; count <= ymax; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) ledge[count] = x >> 16;
                x += m;
            }
        }

        dy = ( ymid - ymin );
        if( dy ) {
            m = ( xmid - xmin ) << 16;
            m /= dy;

            x = xmin << 16;

            for( count = ymin; count <= ymid; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) redge[count] = x >> 16;
                x += m;
            }
        }

        dy = ( ymax - ymid );
        if( dy ) {
            m = ( xmax - xmid ) << 16;
            m /= dy;

            x = xmid << 16;

            for( count = ymid; count <= ymax; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) redge[count] = x >> 16;
                x += m;
            }
        }

        //      Check the triangle edge winding direction
        long dfSum = 0;
        dfSum += xmin * ymax - ymin * xmax;
        dfSum += xmax * ymid - ymax * xmid;
        dfSum += xmid * ymin - ymid * xmin;

        cw = dfSum < 0;

    } // else

    //      if cw is true, redge is actually on the right

    int y1 = ymax;
    int y2 = ymin;

    int ybt = pb_spec->y;
    int yt = pb_spec->y + pb_spec->height;

    if( y1 > yt ) y1 = yt;
    if( y1 < ybt ) y1 = ybt;

    if( y2 > yt ) y2 = yt;
    if( y2 < ybt ) y2 = ybt;

    int lclip = pb_spec->lclip;
    int rclip = pb_spec->rclip;
    if (y1 == y2 )
        return 0;

    //              Clip the triangle
    if( cw ) {
        for( int iy = y2; iy <= y1; iy++ ) {
            if( ledge[iy] < lclip ) {
                if( redge[iy] < lclip ) ledge[iy] = -1;
                else
                    ledge[iy] = lclip;
            }

            if( redge[iy] > rclip ) {
                if( ledge[iy] > rclip ) ledge[iy] = -1;
                else
                    redge[iy] = rclip;
            }
        }
    } else {
        for( int iy = y2; iy <= y1; iy++ ) {
            if( redge[iy] < lclip ) {
                if( ledge[iy] < lclip ) ledge[iy] = -1;
                else
                    redge[iy] = lclip;
            }

            if( ledge[iy] > rclip ) {
                if( redge[iy] > rclip ) ledge[iy] = -1;
                else
                    ledge[iy] = rclip;
            }
        }
    }

    //              Fill the triangle

    int ya = y2;
    int yb = y1;

    unsigned char *pix_buff = pb_spec->pix_buff;

    int patt_size_x = 0, patt_size_y = 0, patt_pitch = 0;
    unsigned char *patt_s0 = NULL;
    if( pPatt_spec ) {
        patt_size_y = pPatt_spec->height;
        patt_size_x = pPatt_spec->width;
        patt_pitch = pPatt_spec->pb_pitch;
        patt_s0 = pPatt_spec->pix_buff;

        if(patt_size_y == 0) /* integer division by this value below */
            return false;
    }

    if( pb_spec->depth == 24 ) {
        for( int iyp = ya; iyp < yb; iyp++ ) {
            if( ( iyp >= ybt ) && ( iyp < yt ) ) {
                int yoff = ( iyp - pb_spec->y ) * pb_spec->pb_pitch;

                unsigned char *py = pix_buff + yoff;

                int ix, ixm;
                if( cw ) {
                    ix = ledge[iyp];
                    ixm = redge[iyp];
                } else {
                    ixm = ledge[iyp];
                    ix = redge[iyp];
                }

                if( ledge[iyp] != -1 ) {

                    //    This would be considered a failure of the dda algorithm
                    //    Happens on very high zoom, with very large triangles.
                    //    The integers of the dda algorithm don't have enough bits...
                    //    Anyway, just ignore this triangle if it happens
                    if( ix > ixm ) continue;

                    int xoff = ( ix - pb_spec->x ) * 3;

                    unsigned char *px = py + xoff;

                    if( pPatt_spec  ) // Pattern
                    {
                        int y_stagger = ( iyp - pPatt_spec->y ) / patt_size_y;
                        int x_stagger_off = 0;
                        if( ( y_stagger & 1 ) && pPatt_spec->b_stagger ) x_stagger_off =
                                pPatt_spec->width / 2;

                        int patt_y = abs( ( iyp - pPatt_spec->y ) ) % patt_size_y;

                        unsigned char *pp0 = patt_s0 + ( patt_y * patt_pitch );

                        while( ix <= ixm ) {
                            int patt_x = abs( ( ( ix - pPatt_spec->x ) + x_stagger_off ) % patt_size_x );

                            unsigned char *pp = pp0 + ( patt_x * 4 );
                            unsigned char alpha = pp[3];
                            double da = (double) alpha / 256.;

                            unsigned char r = (unsigned char) ( *px*(1.0-da) + pp[0] * da );
                            unsigned char g = (unsigned char) ( *(px+1)*(1.0-da) + pp[1] * da );
                            unsigned char b = (unsigned char) ( *(px+2)*(1.0-da) + pp[2] * da );

                            *px++ = r;
                            *px++ = g;
                            *px++ = b;
                            ix++;
                        }
                    }

                    else // No Pattern
                    {
#if defined( __WXGTK__) && defined(__INTEL__)
#define memset3(dest, value, count) \
__asm__ __volatile__ ( \
"cmp $0,%2\n\t" \
"jg 2f\n\t" \
"je 3f\n\t" \
"jmp 4f\n\t" \
"2:\n\t" \
"movl  %0,(%1)\n\t" \
"add $3,%1\n\t" \
"dec %2\n\t" \
"jnz 2b\n\t" \
"3:\n\t" \
"movb %b0,(%1)\n\t" \
"inc %1\n\t" \
"movb %h0,(%1)\n\t" \
"inc %1\n\t" \
"shr $16,%0\n\t" \
"movb %b0,(%1)\n\t" \
"4:\n\t" \
: : "a"(value), "D"(dest), "r"(count) :  );

                        int count = ixm-ix;
                        memset3 ( px, color_int, count )
#else

                        while( ix <= ixm ) {
                            *px++ = b;
                            *px++ = g;
                            *px++ = r;

                            ix++;
                        }
#endif
                    }
                }
            }
        }
    }

    if( pb_spec->depth == 32 ) {

        assert( ya <= yb );

        for( int iyp = ya; iyp < yb; iyp++ ) {
            if( ( iyp >= ybt ) && ( iyp < yt ) ) {
                int yoff = ( iyp - pb_spec->y ) * pb_spec->pb_pitch;

                unsigned char *py = pix_buff + yoff;

                int ix, ixm;
                if( cw ) {
                    ix = ledge[iyp];
                    ixm = redge[iyp];
                } else {
                    ixm = ledge[iyp];
                    ix = redge[iyp];
                }

                if( ledge[iyp] != -1 ) {
                    //    This would be considered a failure of the dda algorithm
                    //    Happens on very high zoom, with very large triangles.
                    //    The integers of the dda algorithm don't have enough bits...
                    //    Anyway, just ignore this triangle if it happens
                    if( ix > ixm ) continue;

                    int xoff = ( ix - pb_spec->x ) * pb_spec->depth / 8;

                    unsigned char *px = py + xoff;

                    if( pPatt_spec ) // Pattern
                    {
                        int y_stagger = ( iyp - pPatt_spec->y ) / patt_size_y;

                        int x_stagger_off = 0;
                        if( ( y_stagger & 1 ) && pPatt_spec->b_stagger ) x_stagger_off =
                                pPatt_spec->width / 2;

                        int patt_y = abs( ( iyp - pPatt_spec->y ) ) % patt_size_y;

                        unsigned char *pp0 = patt_s0 + ( patt_y * patt_pitch );

                        while( ix <= ixm ) {
                            int patt_x = abs(
                                    ( ( ix - pPatt_spec->x ) + x_stagger_off ) % patt_size_x );
                            /*
                             if(pPatt_spec->depth == 24)
                             {
                             unsigned char *pp = pp0 + (patt_x * 3);

                             //  Todo    This line assumes unused_color is always 0,0,0
                             if( pp[0] && pp[1] && pp[2] ) {
                             *px++ = *pp++;
                             *px++ = *pp++;
                             *px++ = *pp++;
                             px++;
                             } else {
                             px += 4;
                             //                                                      pp += 4;
                             }
                             }
                             else
                             */
                            {
                                unsigned char *pp = pp0 + ( patt_x * 4 );
                                unsigned char alpha = pp[3];
                                if( alpha > 128 ) {
                                    double da = (double) alpha / 256.;

                                    unsigned char r = (unsigned char) ( pp[0] * da );
                                    unsigned char g = (unsigned char) ( pp[1] * da );
                                    unsigned char b = (unsigned char) ( pp[2] * da );

                                    *px++ = r;
                                    *px++ = g;
                                    *px++ = b;
                                    px++;
                                } else
                                    px += 4;
                            }
                            ix++;
                        }
                    }

                    else // No Pattern
                    {
                        int *pxi = (int *) px;
                        while( ix <= ixm ) {
                            *pxi++ = color_int;
                            ix++;
                        }
                    }
                }
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------
//
//              Render Trapezoid
//
//----------------------------------------------------------------------------------
inline int s52plib::dda_trap( wxPoint *segs, int lseg, int rseg, int ytop, int ybot, S52color *c,
        render_canvas_parms *pb_spec, render_canvas_parms *pPatt_spec )
{
    unsigned char r = 0, g = 0, b = 0;

    if( NULL != c ) {
        if(pb_spec->b_revrgb) {
            r = c->R;
            g = c->G;
            b = c->B;
        }
        else {
            b = c->R;
            g = c->G;
            r = c->B;
        }
    }

    //      Color Debug
    /*    int fc = rand();
     b = fc & 0xff;
     g = fc & 0xff;
     r = fc & 0xff;
     */

//      int debug = 0;
    int ret_val = 0;

    int color_int = 0;
    if( NULL != c ) color_int = ( ( r ) << 16 ) + ( ( g ) << 8 ) + ( b );

    //      Create edge arrays using fast integer DDA

    int lclip = pb_spec->lclip;
    int rclip = pb_spec->rclip;

    int m, x, dy, count;

    //    Left edge
    int xmax = segs[lseg].x;
    int xmin = segs[lseg + 1].x;
    int ymax = segs[lseg].y;
    int ymin = segs[lseg + 1].y;

    if( ymax < ymin ) {
        int a = ymax;
        ymax = ymin;
        ymin = a;

        a = xmax;
        xmax = xmin;
        xmin = a;
    }

    int y_dda_limit = wxMin ( ybot, ymax );
    y_dda_limit = wxMin ( y_dda_limit, 1499 ); // don't overrun edge array

    //    Some peephole optimization:
    //    if xmax and xmin are both < 0, arrange to simply fill the ledge array with 0
    if( ( xmax < 0 ) && ( xmin < 0 ) ) {
        xmax = -2;
        xmin = -2;
    }
    //    if xmax and xmin are both > rclip, arrange to simply fill the ledge array with rclip + 1
    //    This may induce special clip case below, and cause trap not to be rendered
    else
        if( ( xmax > rclip ) && ( xmin > rclip ) ) {
            xmax = rclip + 1;
            xmin = rclip + 1;
        }

    dy = ( ymax - ymin );
    if( dy ) {
        m = ( xmax - xmin ) << 16;
        m /= dy;

        x = xmin << 16;

        //TODO implement this logic in dda_tri also
        count = ymin;
        while( count < 0 ) {
            x += m;
            count++;
        }

        while( count < y_dda_limit ) {
            ledge[count] = x >> 16;
            x += m;
            count++;
        }
    }

    if( ( ytop < ymin ) || ( ybot > ymax ) ) {
//            printf ( "### ledge out of range\n" );
        ret_val = 1;
//            r=255;
//            g=0;
//            b=0;
    }

    //    Right edge
    xmax = segs[rseg].x;
    xmin = segs[rseg + 1].x;
    ymax = segs[rseg].y;
    ymin = segs[rseg + 1].y;

//Note this never gets hit???
    if( ymax < ymin ) {
        int a = ymax;
        ymax = ymin;
        ymin = a;

        a = xmax;
        xmax = xmin;
        xmin = a;
    }

    //    Some peephole optimization:
    //    if xmax and xmin are both < 0, arrange to simply fill the redge array with -1
    //    This may induce special clip case below, and cause trap not to be rendered
    if( ( xmax < 0 ) && ( xmin < 0 ) ) {
        xmax = -1;
        xmin = -1;
    }

    //    if xmax and xmin are both > rclip, arrange to simply fill the redge array with rclip + 1
    //    This may induce special clip case below, and cause trap not to be rendered
    else
        if( ( xmax > rclip ) && ( xmin > rclip ) ) {
            xmax = rclip + 1;
            xmin = rclip + 1;
        }

    y_dda_limit = wxMin ( ybot, ymax );
    y_dda_limit = wxMin ( y_dda_limit, 1499 ); // don't overrun edge array

    dy = ( ymax - ymin );
    if( dy ) {
        m = ( xmax - xmin ) << 16;
        m /= dy;

        x = xmin << 16;

        count = ymin;
        while( count < 0 ) {
            x += m;
            count++;
        }

        while( count < y_dda_limit ) {
            redge[count] = x >> 16;
            x += m;
            count++;
        }
    }

    if( ( ytop < ymin ) || ( ybot > ymax ) ) {
//            printf ( "### redge out of range\n" );
        ret_val = 1;
//            r=255;
//            g=0;
//            b=0;
    }

    //    Clip trapezoid to height spec
    int y1 = ybot;
    int y2 = ytop;

    int ybt = pb_spec->y;
    int yt = pb_spec->y + pb_spec->height;

    if( y1 > yt ) y1 = yt;
    if( y1 < ybt ) y1 = ybt;

    if( y2 > yt ) y2 = yt;
    if( y2 < ybt ) y2 = ybt;

    //   Clip the trapezoid to width
    for( int iy = y2; iy <= y1; iy++ ) {
        if( ledge[iy] < lclip ) {
            if( redge[iy] < lclip ) ledge[iy] = -1;
            else
                ledge[iy] = lclip;
        }

        if( redge[iy] > rclip ) {
            if( ledge[iy] > rclip ) ledge[iy] = -1;
            else
                redge[iy] = rclip;
        }
    }

    //    Fill the trapezoid

    int ya = y2;
    int yb = y1;

    unsigned char *pix_buff = pb_spec->pix_buff;

    int patt_size_x = 0;
    int patt_size_y = 0;
    int patt_pitch = 0;
    unsigned char *patt_s0 = NULL;
    if( pPatt_spec ) {
        patt_size_y = pPatt_spec->height;
        patt_size_x = pPatt_spec->width;
        patt_pitch = pPatt_spec->pb_pitch;
        patt_s0 = pPatt_spec->pix_buff;
    }

    if( pb_spec->depth == 24 ) {
        for( int iyp = ya; iyp < yb; iyp++ ) {
            if( ( iyp >= ybt ) && ( iyp < yt ) ) {
                int yoff = ( iyp - pb_spec->y ) * pb_spec->pb_pitch;

                unsigned char *py = pix_buff + yoff;

                int ix, ixm;
                ix = ledge[iyp];
                ixm = redge[iyp];

//                        if(debug) printf("iyp %d, ix %d, ixm %d\n", iyp, ix, ixm);
//                           int ix = ledge[iyp];
//                            if(ix != -1)                    // special clip case
                if( ledge[iyp] != -1 ) {
                    int xoff = ( ix - pb_spec->x ) * 3;

                    unsigned char *px = py + xoff;

                    if( pPatt_spec ) // Pattern
                    {
                        int y_stagger = ( iyp - pPatt_spec->y ) / patt_size_y;
                        int x_stagger_off = 0;
                        if( ( y_stagger & 1 ) && pPatt_spec->b_stagger ) x_stagger_off =
                                pPatt_spec->width / 2;

                        int patt_y = abs( ( iyp - pPatt_spec->y ) ) % patt_size_y;
                        unsigned char *pp0 = patt_s0 + ( patt_y * patt_pitch );

                        while( ix <= ixm ) {
                            int patt_x = abs(
                                    ( ( ix - pPatt_spec->x ) + x_stagger_off ) % patt_size_x );
                            /*
                             if(pPatt_spec->depth == 24)
                             {
                             unsigned char *pp = pp0 + (patt_x * 3);

                             //  Todo    This line assumes unused_color is always 0,0,0
                             if( pp[0] && pp[1] && pp[2] ) {
                             *px++ = *pp++;
                             *px++ = *pp++;
                             *px++ = *pp++;
                             } else {
                             px += 3;
                             pp += 3;
                             }
                             }
                             else
                             */
                            {
                                unsigned char *pp = pp0 + ( patt_x * 4 );
                                unsigned char alpha = pp[3];
                                if( alpha > 128 ) {
                                    double da = (double) alpha / 256.;

                                    unsigned char r = (unsigned char) ( pp[0] * da );
                                    unsigned char g = (unsigned char) ( pp[1] * da );
                                    unsigned char b = (unsigned char) ( pp[2] * da );

                                    *px++ = r;
                                    *px++ = g;
                                    *px++ = b;
                                } else
                                    px += 3;
                            }

                            ix++;
                        }
                    }

                    else // No Pattern
                    {
#if defined(__WXGTK__WITH_OPTIMIZE_0) && defined(__INTEL__)
#define memset3d(dest, value, count) \
                                    __asm__ __volatile__ ( \
                                    "cmp $0,%2\n\t" \
                                    "jg ld0\n\t" \
                                    "je ld1\n\t" \
                                    "jmp ld2\n\t" \
                                    "ld0:\n\t" \
                                    "movl  %0,(%1)\n\t" \
                                    "add $3,%1\n\t" \
                                    "dec %2\n\t" \
                                    "jnz ld0\n\t" \
                                    "ld1:\n\t" \
                                    "movb %b0,(%1)\n\t" \
                                    "inc %1\n\t" \
                                    "movb %h0,(%1)\n\t" \
                                    "inc %1\n\t" \
                                    "shr $16,%0\n\t" \
                                    "movb %b0,(%1)\n\t" \
                                    "ld2:\n\t" \
                                          : : "a"(value), "D"(dest), "r"(count) :  );
                        int count = ixm-ix;
                        memset3d ( px, color_int, count )
#else

                        while( ix <= ixm ) {
                            *px++ = b;
                            *px++ = g;
                            *px++ = r;

                            ix++;
                        }
#endif
                    }
                }
            }
        }
    }

    if( pb_spec->depth == 32 ) {

        assert( ya <= yb );

        for( int iyp = ya; iyp < yb; iyp++ ) {
            if( ( iyp >= ybt ) && ( iyp < yt ) ) {
                int yoff = ( iyp - pb_spec->y ) * pb_spec->pb_pitch;

                unsigned char *py = pix_buff + yoff;

                int ix, ixm;
                ix = ledge[iyp];
                ixm = redge[iyp];

                if( ledge[iyp] != -1 ) {
                    int xoff = ( ix - pb_spec->x ) * pb_spec->depth / 8;

                    unsigned char *px = py + xoff;

                    if( pPatt_spec ) // Pattern
                    {
                        int y_stagger = ( iyp - pPatt_spec->y ) / patt_size_y;
                        int x_stagger_off = 0;
                        if( ( y_stagger & 1 ) && pPatt_spec->b_stagger ) x_stagger_off =
                                pPatt_spec->width / 2;

                        int patt_y = abs( ( iyp - pPatt_spec->y ) ) % patt_size_y;
                        unsigned char *pp0 = patt_s0 + ( patt_y * patt_pitch );

                        while( ix <= ixm ) {
                            int patt_x = abs(
                                    ( ( ix - pPatt_spec->x ) + x_stagger_off ) % patt_size_x );
                            /*
                             if(pPatt_spec->depth == 24)
                             {
                             unsigned char *pp = pp0 + (patt_x * 3);

                             //  Todo    This line assumes unused_color is always 0,0,0
                             if( pp[0] && pp[1] && pp[2] ) {
                             *px++ = *pp++;
                             *px++ = *pp++;
                             *px++ = *pp++;
                             px++;
                             } else {
                             px += 4;
                             //                                                      pp += 3;
                             }
                             }
                             else
                             */
                            {
                                unsigned char *pp = pp0 + ( patt_x * 4 );
                                unsigned char alpha = pp[3];
                                if( alpha > 128 ) {
                                    double da = (double) alpha / 256.;

                                    unsigned char r = (unsigned char) ( pp[0] * da );
                                    unsigned char g = (unsigned char) ( pp[1] * da );
                                    unsigned char b = (unsigned char) ( pp[2] * da );

                                    *px++ = r;
                                    *px++ = g;
                                    *px++ = b;
                                    px++;
                                } else
                                    px += 4;
                            }
                            ix++;
                        }
                    }

                    else // No Pattern
                    {
                        int *pxi = (int *) px;
                        while( ix <= ixm ) {
                            *pxi++ = color_int;
                            ix++;
                        }
                    }

                }
            }
        }
    }

    return ret_val;
}

void s52plib::RenderToBufferFilledPolygon( ObjRazRules *rzRules, S57Obj *obj, S52color *c,
                                           render_canvas_parms *pb_spec, render_canvas_parms *pPatt_spec, ViewPort *vp )
{
//    LLBBox BBView = vp->GetBBox();
        LLBBox BBView = vp->GetBBox();
    // please untangle this logic with the logic below
    if(BBView.GetMaxLon()+180 < vp->clon)
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() + 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() + 360);
    else if(BBView.GetMinLon()-180 > vp->clon)
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() - 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() - 360);


    S52color cp;
    if( NULL != c ) {
        cp.R = c->R;
        cp.G = c->G;
        cp.B = c->B;
    }

    if( obj->pPolyTessGeo ) {
        if( !rzRules->obj->pPolyTessGeo->IsOk() ){ // perform deferred tesselation
            rzRules->obj->pPolyTessGeo->BuildDeferredTess();
        }

        wxPoint *pp3 = (wxPoint *) malloc( 3 * sizeof(wxPoint) );
        wxPoint *ptp = (wxPoint *) malloc(
                ( obj->pPolyTessGeo->GetnVertexMax() + 1 ) * sizeof(wxPoint) );

        //  Allow a little slop in calculating whether a triangle
        //  is within the requested Viewport
        double margin = BBView.GetLonRange() * .05;

        PolyTriGroup *ppg = obj->pPolyTessGeo->Get_PolyTriGroup_head();

        TriPrim *p_tp = ppg->tri_prim_head;
        while( p_tp ) {
            LLBBox box;
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
            }                
            else
                box = p_tp->tri_box;

            if(!BBView.IntersectOut(box)) {
                //      Get and convert the points
                wxPoint *pr = ptp;

                if(ppg->data_type == DATA_TYPE_DOUBLE){
                    double *pvert_list = p_tp->p_vertex;

                    for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                        double lon = *pvert_list++;
                        double lat = *pvert_list++;
                        GetPointPixSingle( rzRules, lat, lon, pr, vp );

                        pr++;
                    }
                }
                else {
                    float *pvert_list = (float *)p_tp->p_vertex;
                    
                    for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                        double lon = *pvert_list++;
                        double lat = *pvert_list++;
                        GetPointPixSingle( rzRules, lat, lon, pr, vp );
                        
                        pr++;
                    }
                }
                
                switch( p_tp->type ){
                    case PTG_TRIANGLE_FAN: {
                        for( int it = 0; it < p_tp->nVert - 2; it++ ) {
                            pp3[0].x = ptp[0].x;
                            pp3[0].y = ptp[0].y;

                            pp3[1].x = ptp[it + 1].x;
                            pp3[1].y = ptp[it + 1].y;

                            pp3[2].x = ptp[it + 2].x;
                            pp3[2].y = ptp[it + 2].y;

                            dda_tri( pp3, &cp, pb_spec, pPatt_spec );
                        }
                        break;
                    }
                    case PTG_TRIANGLE_STRIP: {
                        for( int it = 0; it < p_tp->nVert - 2; it++ ) {
                            pp3[0].x = ptp[it].x;
                            pp3[0].y = ptp[it].y;

                            pp3[1].x = ptp[it + 1].x;
                            pp3[1].y = ptp[it + 1].y;

                            pp3[2].x = ptp[it + 2].x;
                            pp3[2].y = ptp[it + 2].y;

                            dda_tri( pp3, &cp, pb_spec, pPatt_spec );
                        }
                        break;
                    }
                    case PTG_TRIANGLES: {

                        for( int it = 0; it < p_tp->nVert; it += 3 ) {
                            pp3[0].x = ptp[it].x;
                            pp3[0].y = ptp[it].y;

                            pp3[1].x = ptp[it + 1].x;
                            pp3[1].y = ptp[it + 1].y;

                            pp3[2].x = ptp[it + 2].x;
                            pp3[2].y = ptp[it + 2].y;

                            dda_tri( pp3, &cp, pb_spec, pPatt_spec );
                        }
                        break;

                    }
                }
            } // if bbox
            
            // pick up the next in chain
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                p_tp = (TriPrim *)p_ltp->p_next;
            }
            else
                p_tp = p_tp->p_next; 
                
        } // while
        
        free( ptp );
        free( pp3 );
    } // if pPolyTessGeo
}

int s52plib::RenderToGLAC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifdef ocpnUSE_GL    
    GLenum reset_err = glGetError();
    
    S52color *c;
    char *str = (char*) rules->INSTstr;

    c = getColor( str );

#ifndef ocpnUSE_GLES // linestipple is emulated poorly
    glColor3ub( c->R, c->G, c->B );
#endif

    LLBBox BBView = vp->GetBBox();
    // please untangle this logic with the logic below
    if(BBView.GetMaxLon()+180 < vp->clon)
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() + 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() + 360);
    else if(BBView.GetMinLon()-180 > vp->clon)
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() - 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() - 360);

    //  Allow a little slop in calculating whether a triangle
    //  is within the requested Viewport
    double margin = BBView.GetLonRange() * .05;
    BBView.EnLarge( margin );

    bool b_useVBO = m_useVBO && !rzRules->obj->auxParm1 && vp->m_projection_type == PROJECTION_MERCATOR;
    
    if( rzRules->obj->pPolyTessGeo ) {
        
        bool b_temp_vbo = false;
        bool b_transform = false;
        
        // Set up the OpenGL transform matrix for this object
        // We transform from SENC SM vertex data to screen.

#ifndef USE_ANDROID_GLES2

        glColor3ub( c->R, c->G, c->B );

        //  First, the VP transform
        if(b_useVBO || vp->m_projection_type == PROJECTION_MERCATOR) {
            b_transform = true;
            glPushMatrix();

            glTranslatef( vp->pix_width / 2, vp->pix_height/2, 0 );
            glScalef( vp->view_scale_ppm, -vp->view_scale_ppm, 0 );
            glTranslatef( -rzRules->sm_transform_parms->easting_vp_center, -rzRules->sm_transform_parms->northing_vp_center, 0 );
            //  Next, the per-object transform

            float x_origin = rzRules->obj->x_origin;
            
            if(rzRules->obj->m_chart_context->chart) {          // not a PlugIn Chart
                if( ( (int)rzRules->obj->auxParm3 == (int)PI_CHART_TYPE_CM93 )
                    || ( (int)rzRules->obj->auxParm3 == (int)PI_CHART_TYPE_CM93COMP ) )
                {
                    //      We may need to translate object coordinates by 360 degrees to conform.
                    if( BBView.GetMaxLon() >= 180. ) {
                        if(rzRules->obj->BBObj.GetMinLon() < BBView.GetMaxLon() - 360.)
                            x_origin += (float)(mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI);
                    }
                    else
                    if( (BBView.GetMinLon() <= -180. && rzRules->obj->BBObj.GetMaxLon() > BBView.GetMinLon() + 360.)
                    || (rzRules->obj->BBObj.GetMaxLon() > 180 && BBView.GetMinLon() + 360 < rzRules->obj->BBObj.GetMaxLon() )
                    )
                    x_origin -= (float)(mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI);
                }
            }
            
            glTranslatef( x_origin, rzRules->obj->y_origin, 0);
            glScalef( rzRules->obj->x_rate, rzRules->obj->y_rate, 0 );
        }
#endif
        
        // perform deferred tesselation
        if( !rzRules->obj->pPolyTessGeo->IsOk() ){
            rzRules->obj->pPolyTessGeo->BuildDeferredTess();
        }

        //  Get the vertex data
        PolyTriGroup *ppg_vbo = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
            
            //  Has the input vertex buffer been converted to "single_alloc float" model?
            //  and is it allowed?
        if(!ppg_vbo->bsingle_alloc && (rzRules->obj->auxParm1 >= 0) ){
                
                int data_size = sizeof(float);
                
                //  First calculate the required total byte size
                    int total_byte_size = 0;
                    TriPrim *p_tp = ppg_vbo->tri_prim_head;
                    while( p_tp ) {
                        total_byte_size += p_tp->nVert * 2 * data_size;
                        p_tp = p_tp->p_next; // pick up the next in chain
                    }
                    
                    float *vbuf = (float *)malloc(total_byte_size);
                    p_tp = ppg_vbo->tri_prim_head;
                    
                    if( ppg_vbo->data_type == DATA_TYPE_DOUBLE){  //DOUBLE to FLOAT
                            float *p_run = vbuf;
                            while( p_tp ) {
                                float *pfbuf = p_run;
                                for( int i=0 ; i < p_tp->nVert * 2 ; ++i){
                                    float x = (float)(p_tp->p_vertex[i]);
                                    *p_run++ = x;
                                }
                                
                                free(p_tp->p_vertex);
                                p_tp->p_vertex = (double *)pfbuf;
                                
                                p_tp = p_tp->p_next; // pick up the next in chain
                            }
                    }
                    else {          // FLOAT to FLOAT
                            float *p_run = vbuf;
                            while( p_tp ) {
                                memcpy( p_run, p_tp->p_vertex, p_tp->nVert * 2 * sizeof(float) );
                                
                                free(p_tp->p_vertex);
                                p_tp->p_vertex = (double *)p_run;
                                
                                p_run += p_tp->nVert * 2;
                                
                                p_tp = p_tp->p_next; // pick up the next in chain
                            }
                    }
                    
                    
                    ppg_vbo->bsingle_alloc = true;
                    ppg_vbo->single_buffer = (unsigned char *)vbuf;
                    ppg_vbo->single_buffer_size = total_byte_size;
                    ppg_vbo->data_type = DATA_TYPE_FLOAT;
                    
        }
            

        if( b_useVBO ){        
        //  Has a VBO been built for this object?
            if( 1 ) {
                glGetError(); // clear it
                 
                 if(rzRules->obj->auxParm0 <= 0) {
#ifdef xUSE_ANDROID_GLES2
                  if(ppg_vbo->data_type != DATA_TYPE_SHORT){
                    // We convert the vertex data from FLOAT to GL_SHORT to make the VBO smaller, but still keeping enough precision
                    //  This requires a scale factor to reduce the range from existing  data to +/- 32K

                    size_t np =  ppg_vbo->single_buffer_size / (2 * sizeof(float));
                    np --;

                    PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
                    TriPrim *p_tp = ppg->tri_prim_head;

                    size_t npp = 0;
                    while( p_tp ) {
                        npp += p_tp->nVert;
                        p_tp = (TriPrim *)p_tp->p_next;
                    }

                    //  Get the data range
                    float * pRun = (float *)ppg_vbo->single_buffer;
                    float north_max = -1e8;
                    float north_min = 1e8;
                    float east_max = -1e8;
                    float east_min = 1e8;

                    for( size_t i = 0 ; i < np ; i++){
                        float east = *pRun++;
                        float north = *pRun++;
                        north_max = wxMax(north, north_max);
                        north_min = wxMin(north, north_min);
                        east_max = wxMax(east, east_max);
                        east_min = wxMin(east, east_min);
                    }

                    float cfactx = wxMax(fabs(east_max), fabs(east_min));
                    float cfacty = wxMax(fabs(north_max), fabs(north_min));
                    float cfact = wxMax(cfactx, cfacty);

                    float sfact = cfact / 32700.0;

                    sfact = wxMax(sfact, 1.0);

                    //  Copy/convert the data
                    unsigned char *new_buf = (unsigned char *)malloc(np * 2 * sizeof(short));
                    pRun = (float *)ppg_vbo->single_buffer;
                    short *pd = (short *)new_buf;
                    for( size_t i = 0 ; i < np ; i++){
                        float east = *pRun++;
                        float north = *pRun++;
 //                       short a = (east / sfact);
 //                       short b = (north / sfact);
                        *pd++ = (east / sfact);
                        *pd++ = (north / sfact);

                    }

                    // replace the buffer
                    free(ppg_vbo->single_buffer);
                    ppg_vbo->single_buffer = new_buf;
                    ppg_vbo->single_buffer_size /= 2;

                    // Record the scale/offset factors
                    ppg_vbo->sfactor = sfact;
                    ppg_vbo->soffset = 0.;

                    ppg_vbo->data_type = DATA_TYPE_SHORT;

                  }



#endif
                    b_temp_vbo = (rzRules->obj->auxParm0 == -5);   // Must we use a temporary VBO?  Probably slower than simple glDrawArrays
                   
                    GLuint vboId = 0;
                    // generate a new VBO and get the associated ID
                    glGenBuffers(1, &vboId);
                    
                    rzRules->obj->auxParm0 = vboId;
                    
                    // bind VBO in order to use
                    glBindBuffer(GL_ARRAY_BUFFER, vboId);
                    GLenum err = glGetError();
                    if(err){
                        wxString msg;
                        msg.Printf(_T("VBO Error A: %d"), err);
                        wxLogMessage(msg);
                        return 0;
                    }
                    
                    // upload data to VBO
#ifndef USE_ANDROID_GLES2
                    glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
#endif
                    glBufferData(GL_ARRAY_BUFFER,
                                    ppg_vbo->single_buffer_size, ppg_vbo->single_buffer, GL_STATIC_DRAW);
                    err = glGetError();
                    if(err){
                        wxString msg;
                        msg.Printf(_T("VBO Error B: %d"), err);
                        wxLogMessage(msg);
                        return 0;
                    }
                    
                }
                else {
                    glBindBuffer(GL_ARRAY_BUFFER, rzRules->obj->auxParm0);
                    GLenum err = glGetError();
                    if(err){
                        wxString msg;
                        msg.Printf(_T("VBO Error C: %d"), err);
                        wxLogMessage(msg);
                        return 0;
                    }
                    
#ifndef USE_ANDROID_GLES2
                    glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
#endif
                }                    
             }
        }

        

        PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

        TriPrim *p_tp = ppg->tri_prim_head;
        GLintptr vbo_offset = 0;
        
#ifndef USE_ANDROID_GLES2
        glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
#endif
        //      Set up the stride sizes for the array
        int array_data_size = sizeof(float);
        GLint array_gl_type = GL_FLOAT;
        
        if(ppg->data_type == DATA_TYPE_DOUBLE){
            array_data_size = sizeof(double);
            array_gl_type = GL_DOUBLE;
        }
            
        if(ppg->data_type == DATA_TYPE_SHORT){
            array_data_size = sizeof(short);
            array_gl_type = GL_SHORT;
        }

#ifdef USE_ANDROID_GLES2
        glUseProgram(S52color_tri_shader_program);

                // Disable VBO's (vertex buffer objects) for attributes.
        if(!b_useVBO)
            glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

        GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");
        glEnableVertexAttribArray(pos);

        float angle = 0;

        // Build Transform matrix
        mat4x4 I, Q;
        mat4x4_identity(I);


        // Scale
        I[0][0] *= rzRules->obj->x_rate;
        I[1][1] *= rzRules->obj->y_rate;

        // Translate
        float x_origin = rzRules->obj->x_origin;
        
        if(rzRules->obj->m_chart_context->chart) {          // not a PlugIn Chart
                if( ( (int)rzRules->obj->m_chart_context->chart->GetChartType() == (int)PI_CHART_TYPE_CM93 )
                    || ( (int)rzRules->obj->m_chart_context->chart->GetChartType() == (int)PI_CHART_TYPE_CM93COMP ) )
                {
                    //      We may need to translate object coordinates by 360 degrees to conform.
                    if( BBView.GetMaxLon() >= 180. ) {
                        if(rzRules->obj->BBObj.GetMinLon() < BBView.GetMaxLon() - 360.)
                            x_origin += mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
                    }
                    else
                        if( (BBView.GetMinLon() <= -180. && rzRules->obj->BBObj.GetMaxLon() > BBView.GetMinLon() + 360.)
                            || (rzRules->obj->BBObj.GetMaxLon() > 180 && BBView.GetMinLon() + 360 < rzRules->obj->BBObj.GetMaxLon() )
                        )
                            x_origin -= mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
                }
        }
        
        I[3][0] = -(rzRules->sm_transform_parms->easting_vp_center - x_origin) * vp->view_scale_ppm ;
        I[3][1] = -(rzRules->sm_transform_parms->northing_vp_center - rzRules->obj->y_origin) * -vp->view_scale_ppm ;

        // Scale
        I[0][0] *= vp->view_scale_ppm * ppg->sfactor;
        I[1][1] *= -vp->view_scale_ppm * ppg->sfactor;

        //Rotate
        mat4x4_rotate_Z(Q, I, angle);

        // Translate
        Q[3][0] += vp->pix_width / 2;
        Q[3][1] += vp->pix_height / 2;

        GLint matloc = glGetUniformLocation(S52color_tri_shader_program,"TransformMatrix");
        glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);


        float colorv[4];
        colorv[0] = c->R / float(256);
        colorv[1] = c->G / float(256);
        colorv[2] = c->B / float(256);
        colorv[3] = 1.0;

        GLint colloc = glGetUniformLocation(S52color_tri_shader_program,"color");
        glUniform4fv(colloc, 1, colorv);


#endif
        while( p_tp ) {
            LLBBox box;
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
            }                
            else
                box = p_tp->tri_box;
            
            if(!BBView.IntersectOut(box)) {
#ifdef USE_ANDROID_GLES2

                if(b_useVBO) {
                    glVertexAttribPointer(pos, 2, array_gl_type, GL_FALSE, 0, (GLvoid *)(vbo_offset));
                    glDrawArrays(p_tp->type, 0, p_tp->nVert);
                }
                else {
                    float *bufOffset = (float *)(&ppg->single_buffer[vbo_offset]);
                    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, bufOffset);
                    glDrawArrays(p_tp->type, 0, p_tp->nVert);
                }


#else
                if(b_useVBO) {
                    glVertexPointer(2, array_gl_type, 2 * array_data_size, (GLvoid *)(vbo_offset));
                    if(vbo_offset + p_tp->nVert * 2 * array_data_size <= ppg_vbo->single_buffer_size) 
                        glDrawArrays(p_tp->type, 0, p_tp->nVert);
                }
                else {
                    if(vp->m_projection_type == PROJECTION_MERCATOR) {
                        glVertexPointer(2, array_gl_type, 2 * array_data_size, p_tp->p_vertex);
                        glDrawArrays(p_tp->type, 0, p_tp->nVert);
                    } else {
                        // temporary slow hack
                        glDisableClientState(GL_VERTEX_ARRAY);

                        glBegin(p_tp->type);
                        float *pvert_list = (float *)p_tp->p_vertex;
                        for(int i=0; i<p_tp->nVert; i++) {
                            float lon = *pvert_list++;
                            float lat = *pvert_list++;
                            wxPoint r;
                            GetPointPixSingle(rzRules, lat, lon, &r, vp);

                            if(r.x != INVALID_COORD)
                                glVertex2i(r.x, r.y);
                            else if(p_tp->type != GL_TRIANGLE_FAN) {
                                glEnd();
                                glBegin(p_tp->type);
                                if(p_tp->type == GL_TRIANGLES)
                                    while(i%3 < 2) i++;
                            }
                        }
                        glEnd();

                    }
                }
#endif
            }
            
            vbo_offset += p_tp->nVert * 2 * array_data_size;
            
            // pick up the next in chain
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                p_tp = (TriPrim *)p_ltp->p_next;
            }
            else
                p_tp = p_tp->p_next; 
            
        } // while
        
        if(b_useVBO)
            glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);

        
#ifndef USE_ANDROID_GLES2
        glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
        
        if(b_transform)
            glPopMatrix();
#else
        mat4x4 IM;
        mat4x4_identity(IM);
        GLint matlocf = glGetUniformLocation(S52color_tri_shader_program,"TransformMatrix");
        glUniformMatrix4fv( matlocf, 1, GL_FALSE, (const GLfloat*)IM);
#endif

        
        if( b_useVBO && b_temp_vbo){
            glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
            glDeleteBuffers(1, (unsigned int *)&rzRules->obj->auxParm0);
            rzRules->obj->auxParm0 = 0;
        }
    } // if pPolyTessGeo


#endif          //#ifdef ocpnUSE_GL

    return 1;
}

void s52plib::SetGLClipRect(const ViewPort &vp, const wxRect &rect)
{
#ifndef USE_ANDROID_GLES2    
    bool b_clear = false;
    bool s_b_useStencil = m_useStencil;
    
    #if 0
    /* for some reason this causes an occasional bug in depth mode, I cannot
     *       seem to solve it yet, so for now: */
    if(s_b_useStencil && s_b_useScissorTest) {
        wxRect vp_rect(0, 0, vp.pix_width, vp.pix_height);
        if(rect != vp_rect) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(rect.x, cc1->m_canvas_height-rect.height-rect.y, rect.width, rect.height);
}

if(b_clear) {
    glBegin(GL_QUADS);
    glVertex2i( rect.x, rect.y );
    glVertex2i( rect.x + rect.width, rect.y );
    glVertex2i( rect.x + rect.width, rect.y + rect.height );
    glVertex2i( rect.x, rect.y + rect.height );
    glEnd();
}

/* the code in s52plib depends on the depth buffer being
 *           initialized to this value, this code should go there instead and
 *           only a flag set here. */
if(!s_b_useStencil) {
    glClearDepth( 0.25 );
    glDepthMask( GL_TRUE );    // to allow writes to the depth buffer
    glClear( GL_DEPTH_BUFFER_BIT );
    glDepthMask( GL_FALSE );
    glClearDepth( 1 ); // set back to default of 1
    glDepthFunc( GL_GREATER );                          // Set the test value
}
return;
}
#endif
// slower way if there is no scissor support
    if(!b_clear)
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer
    
    if( s_b_useStencil ) {
        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );
        
        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
    } else              //  Use depth buffer for clipping
    {
        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer
        
        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start
        
        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
        // dep buffer clear = 1
        // 1 makes 0 in dep buffer, works
        // 0 make .5 in depth buffer
        // -1 makes 1 in dep buffer
        
        //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
        //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
        //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
        glTranslatef( 0, 0, .5 );
    }
    
    glBegin(GL_QUADS);
    glVertex2i( rect.x, rect.y );
    glVertex2i( rect.x + rect.width, rect.y );
    glVertex2i( rect.x + rect.width, rect.y + rect.height );
    glVertex2i( rect.x, rect.y + rect.height );
    glEnd();
    
    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }
    
    if(!b_clear)
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
#endif        
}

void RotateToViewPort(const ViewPort &vp)
{
#ifndef USE_ANDROID_GLES2
    bool g_bskew_comp = true;
    
    float angle = vp.rotation;
    if(g_bskew_comp)
        angle -= vp.skew;
    
    if( fabs( angle ) > 0.0001 )
    {
        //    Rotations occur around 0,0, so translate to rotate around screen center
        float xt = vp.pix_width / 2.0, yt = vp.pix_height / 2.0;
        
        glTranslatef( xt, yt, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -xt, -yt, 0 );
    }
#endif

}



int s52plib::RenderToGLAP( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifdef USE_ANDROID_GLES2
    return RenderToGLAP_GLSL( rzRules, rules, vp);
#endif

#ifdef ocpnUSE_GL
    if( rules->razRule == NULL )
        return 0;

    int obj_xmin = 10000;
    int obj_xmax = -10000;
    int obj_ymin = 10000;
    int obj_ymax = -10000;

    double z_clip_geom = 1.0;
    double z_tex_geom = 0.;

    GLuint clip_list = 0;

    LLBBox BBView = vp->GetBBox();

    wxPoint *ptp;
    if( rzRules->obj->pPolyTessGeo ) {
        if( !rzRules->obj->pPolyTessGeo->IsOk() ){ // perform deferred tesselation
            rzRules->obj->pPolyTessGeo->BuildDeferredTess();
        }

        ptp = (wxPoint *) malloc(
                ( rzRules->obj->pPolyTessGeo->GetnVertexMax() + 1 ) * sizeof(wxPoint) );
    } else
        return 0;

    if( m_useStencilAP ) {
        glPushAttrib( GL_STENCIL_BUFFER_BIT );          // See comment below
        //    Use masked bit "1" of the stencil buffer to create a stencil for the area of interest

        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x2 ); // write only into bit 1 of the stencil buffer
        glColorMask( false, false, false, false ); // Disable writing to the color buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    We are going to write "2" into the stencil buffer wherever the object is valid
        glStencilFunc( GL_ALWAYS, 2, 2 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

    } else {
        glEnable( GL_DEPTH_TEST ); // to use the depth test
        glDepthFunc( GL_GREATER ); // Respect global render mask in depth buffer
        glDepthMask( GL_TRUE ); // to allow writes to the depth buffer
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE ); // disable color buffer

        glColor3f( 1, 1, 0 );

        //  If we are using stencil for overall clipping, then we are only
        //  using depth buffer for AreaPattern rendering
        //  So, each AP render can start with a clear depth buffer
        
        if(m_useStencil){
             glClearDepth(.26);
             glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start
        }
            //    Overall chart clip buffer was set at z=0.5
        //    Draw this clip geometry at z = .25, so still respecting the previously established clip region
        //    Subsequent drawing to this area at z=.25  will pass only this area if using glDepthFunc(GL_EQUAL);

        z_clip_geom = .25;
        z_tex_geom = .25;
    }

        PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

        TriPrim *p_tp = ppg->tri_prim_head;
        while( p_tp ) {
            LLBBox box;
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
            }                
            else
                box = p_tp->tri_box;
            
            if(!BBView.IntersectOut(box)) {
                //      Get and convert the points

                wxPoint *pr = ptp;
                if( ppg->data_type == DATA_TYPE_FLOAT ){
                    float *pvert_list = (float *)p_tp->p_vertex;

                    for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                        float lon = *pvert_list++;
                        float lat = *pvert_list++;
                        GetPointPixSingle(rzRules, lat, lon, pr, vp );
                        
                        obj_xmin = wxMin(obj_xmin, pr->x);
                        obj_xmax = wxMax(obj_xmax, pr->x);
                        obj_ymin = wxMin(obj_ymin, pr->y);
                        obj_ymax = wxMax(obj_ymax, pr->y);

                        pr++;
                    }
                }
                else {
                    double *pvert_list = p_tp->p_vertex;
                    
                    for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                        double lon = *pvert_list++;
                        double lat = *pvert_list++;
                        GetPointPixSingle(rzRules, lat, lon, pr, vp );
                        
                        obj_xmin = wxMin(obj_xmin, pr->x);
                        obj_xmax = wxMax(obj_xmax, pr->x);
                        obj_ymin = wxMin(obj_ymin, pr->y);
                        obj_ymax = wxMax(obj_ymax, pr->y);
                        
                        pr++;
                    }
                }
                

                switch( p_tp->type ){
                    case PTG_TRIANGLE_FAN: {
                        glBegin( GL_TRIANGLE_FAN );
                        for( int it = 0; it < p_tp->nVert; it++ )
                            glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                        glEnd();
                        break;
                    }

                    case PTG_TRIANGLE_STRIP: {
                        glBegin( GL_TRIANGLE_STRIP );
                        for( int it = 0; it < p_tp->nVert; it++ )
                            glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                        glEnd();
                        break;
                    }
                    case PTG_TRIANGLES: {
                        glBegin( GL_TRIANGLES );
                        for( int it = 0; it < p_tp->nVert; it += 3 ) {
                            int xmin = wxMin(ptp[it].x, wxMin(ptp[it+1].x, ptp[it+2].x));
                            int xmax = wxMax(ptp[it].x, wxMax(ptp[it+1].x, ptp[it+2].x));
                            int ymin = wxMin(ptp[it].y, wxMin(ptp[it+1].y, ptp[it+2].y));
                            int ymax = wxMax(ptp[it].y, wxMax(ptp[it+1].y, ptp[it+2].y));

                            wxRect rect( xmin, ymin, xmax - xmin, ymax - ymin );
                            //if( rect.Intersects( m_render_rect ) )
                            {
                                glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                                glVertex3f( ptp[it + 1].x, ptp[it + 1].y, z_clip_geom );
                                glVertex3f( ptp[it + 2].x, ptp[it + 2].y, z_clip_geom );
                            }
                        }
                        glEnd();
                        break;
                    }
                }
            } // if bbox
            
            // pick up the next in chain
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                p_tp = (TriPrim *)p_ltp->p_next;
            }
            else
                p_tp = p_tp->p_next; 
            
        } // while

//        obj_xmin = 0;
//        obj_xmax = 2000;

        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ); // re-enable color buffer

        if( m_useStencilAP ) {
            //    Now set the stencil ops to subsequently render only where the stencil bit is "2"
            glStencilFunc( GL_EQUAL, 2, 2 );
            glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
        } else {
            glDepthFunc( GL_EQUAL ); // Set the test value
            glDepthMask( GL_FALSE ); // disable depth buffer
        }
        //    Get the pattern definition
        if( ( rules->razRule->pixelPtr == NULL ) || ( rules->razRule->parm1 != m_colortable_index )
                || ( rules->razRule->parm0 != ID_GL_PATT_SPEC ) ) {
            render_canvas_parms *patt_spec = CreatePatternBufferSpec( rzRules, rules, vp, false,
                    true );

            ClearRulesCache( rules->razRule ); //  Clear out any existing cached symbology

            rules->razRule->pixelPtr = patt_spec;
            rules->razRule->parm1 = m_colortable_index;
            rules->razRule->parm0 = ID_GL_PATT_SPEC;
        }

        //  Render the Area using the pattern spec stored in the rules
        render_canvas_parms *ppatt_spec = (render_canvas_parms *) rules->razRule->pixelPtr;

        //    Has the pattern been uploaded as a texture?
        if( !ppatt_spec->OGL_tex_name ) {
            GLuint tex_name;
            glGenTextures( 1, &tex_name );
            ppatt_spec->OGL_tex_name = tex_name;

            glBindTexture( GL_TEXTURE_2D, tex_name );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ppatt_spec->w_pot, ppatt_spec->h_pot, 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, ppatt_spec->pix_buff );
        }

        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, ppatt_spec->OGL_tex_name );

        glEnable( GL_BLEND );
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

        int h = ppatt_spec->height;
        int w = ppatt_spec->width;
        int xr = obj_xmin;
        int yr = obj_ymin;
        
        float ww = (float) ppatt_spec->width / (float) ppatt_spec->w_pot;
        float hh = (float) ppatt_spec->height / (float) ppatt_spec->h_pot;
        float x_stagger_off = 0;
        if( ppatt_spec->b_stagger ) x_stagger_off = (float) ppatt_spec->width / 2;
        int yc = 0;

        
        if(w>0 && h>0) {
            while( yr < vp->pix_height ) {
                if( ( (yr + h) >= 0 ) && ( yr <= obj_ymax ) )  {
                    xr = obj_xmin;   //reset
                    while( xr < vp->pix_width ) {
                    
                        int xp = xr;
                        if( yc & 1 ) xp += x_stagger_off;
                    
                    //    Render a quad.
                        if( ( (xr + w) >= 0 ) && ( xr <= obj_xmax ) ){
                            glBegin( GL_QUADS );
                            glTexCoord2f( 0, 0 );
                            glVertex3f( xp, yr, z_tex_geom );
                            glTexCoord2f( ww, 0 );
                            glVertex3f( xp + w, yr, z_tex_geom );
                            glTexCoord2f( ww, hh );
                            glVertex3f( xp + w, yr + h, z_tex_geom );
                            glTexCoord2f( 0, hh );
                            glVertex3f( xp, yr + h, z_tex_geom );
                            glEnd();
                        }
                        xr += ppatt_spec->width;
                    }
                }
                yr += ppatt_spec->height;
                yc++;
            }
        }

        glDisable( GL_TEXTURE_2D );
        glDisable( GL_BLEND );

        //    Restore the previous state
        
         if( m_useStencilAP ){
             //  Theoretically, it should be sufficient to simply reset the StencilFunc()...
             //  But I found one platform where this does not work, and we need to save and restore
             //  the entire STENCIL state.  I suspect bad GL drivers here, but we do what must needs...
             //glStencilFunc( GL_EQUAL, 1, 1 );
             
             glPopAttrib();
         }
         else {
             // restore clipping region
             glPopMatrix();
             SetGLClipRect( *vp, m_last_clip_rect);
             
             glPushMatrix();
             RotateToViewPort(*vp);
             glDisable( GL_DEPTH_TEST );
             
         }


    free( ptp );
#endif                  //#ifdef ocpnUSE_GL
    
    return 1;
}

int s52plib::RenderToGLAP_GLSL( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{

#ifdef USE_ANDROID_GLES2

    //    Get the pattern definition
    if( ( rules->razRule->pixelPtr == NULL ) || ( rules->razRule->parm1 != m_colortable_index )
        || ( rules->razRule->parm0 != ID_GL_PATT_SPEC ) ) {

        render_canvas_parms *patt_spec = CreatePatternBufferSpec( rzRules, rules, vp, false, true );

        ClearRulesCache( rules->razRule ); //  Clear out any existing cached symbology

        rules->razRule->pixelPtr = patt_spec;
        rules->razRule->parm1 = m_colortable_index;
        rules->razRule->parm0 = ID_GL_PATT_SPEC;
    }

    //  Render the Area using the pattern spec stored in the rules
    render_canvas_parms *ppatt_spec = (render_canvas_parms *) rules->razRule->pixelPtr;

    //    Has the pattern been uploaded as a texture?
    if( !ppatt_spec->OGL_tex_name ) {
        GLuint tex_name;
        glGenTextures( 1, &tex_name );
        ppatt_spec->OGL_tex_name = tex_name;

        glBindTexture( GL_TEXTURE_2D, tex_name );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ppatt_spec->w_pot, ppatt_spec->h_pot, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, ppatt_spec->pix_buff );
    }

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, ppatt_spec->OGL_tex_name );

    glEnable( GL_BLEND );
    //glBlendFunc(GL_SRC_COLOR, GL_ZERO);

    int textureWidth = ppatt_spec->width;    //ppatt_spec->w_pot;
    int textureHeight = ppatt_spec->height;  //ppatt_spec->h_pot;

    wxPoint pr;
    GetPointPixSingle(rzRules, rzRules->obj->m_lat, rzRules->obj->m_lat, &pr, vp );
    float xOff = pr.x;
    float yOff = pr.y;


    LLBBox BBView = vp->GetBBox();
    // please untangle this logic with the logic below
    if(BBView.GetMaxLon()+180 < vp->clon)
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() + 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() + 360);
        else if(BBView.GetMinLon()-180 > vp->clon)
            BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() - 360,
                       BBView.GetMaxLat(), BBView.GetMaxLon() - 360);

            //  Allow a little slop in calculating whether a triangle
            //  is within the requested Viewport
            double margin = BBView.GetLonRange() * .05;
        BBView.EnLarge( margin );

    bool b_useVBO = m_useVBO  && !rzRules->obj->auxParm1 && vp->m_projection_type == PROJECTION_MERCATOR;

    if( rzRules->obj->pPolyTessGeo ) {

        bool b_temp_vbo = false;
        bool b_transform = false;


        // perform deferred tesselation
        if( !rzRules->obj->pPolyTessGeo->IsOk() )
            rzRules->obj->pPolyTessGeo->BuildDeferredTess();

        //  Get the vertex data
        PolyTriGroup *ppg_vbo = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

            //  Has the input vertex buffer been converted to "single_alloc float" model?
            //  and is it allowed?
        if(!ppg_vbo->bsingle_alloc && (rzRules->obj->auxParm1 >= 0) ){

                int data_size = sizeof(float);

                //  First calculate the required total byte size
                int total_byte_size = 0;
                TriPrim *p_tp = ppg_vbo->tri_prim_head;
                while( p_tp ) {
                    total_byte_size += p_tp->nVert * 2 * data_size;
                    p_tp = p_tp->p_next; // pick up the next in chain
                }

                float *vbuf = (float *)malloc(total_byte_size);
                p_tp = ppg_vbo->tri_prim_head;

                if( ppg_vbo->data_type == DATA_TYPE_DOUBLE){  //DOUBLE to FLOAT
                            float *p_run = vbuf;
                            while( p_tp ) {
                                float *pfbuf = p_run;
                                for( int i=0 ; i < p_tp->nVert * 2 ; ++i){
                                    float x = (float)(p_tp->p_vertex[i]);
                                    *p_run++ = x;
                                }

                                free(p_tp->p_vertex);
                                p_tp->p_vertex = (double *)pfbuf;

                                p_tp = p_tp->p_next; // pick up the next in chain
                            }
                }
                else {          // FLOAT to FLOAT
                            float *p_run = vbuf;
                            while( p_tp ) {
                                memcpy( p_run, p_tp->p_vertex, p_tp->nVert * 2 * sizeof(float) );

                                free(p_tp->p_vertex);
                                p_tp->p_vertex = (double *)p_run;

                                p_run += p_tp->nVert * 2 * sizeof(float);

                                p_tp = p_tp->p_next; // pick up the next in chain
                            }
                }


                ppg_vbo->bsingle_alloc = true;
                ppg_vbo->single_buffer = (unsigned char *)vbuf;
                ppg_vbo->single_buffer_size = total_byte_size;
                ppg_vbo->data_type = DATA_TYPE_FLOAT;

        }


        if( b_useVBO ){
               //  Has a VBO been built for this object?
                if( 1 ) {

                    if(rzRules->obj->auxParm0 <= 0) {
#if 0                        
                        if(ppg_vbo->data_type != DATA_TYPE_SHORT){
                            // We convert the vertex data from FLOAT to GL_SHORT to make the VBO smaller, but still keeping enough precision
                            //  This requires a scale factor to reduce the range from existing  data to +/- 32K

                            size_t np =  ppg_vbo->single_buffer_size / (2 * sizeof(float));
                            np --;

                            PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
                            TriPrim *p_tp = ppg->tri_prim_head;

                            size_t npp = 0;
                            while( p_tp ) {
                                npp += p_tp->nVert;
                                p_tp = (TriPrim *)p_tp->p_next;
                            }

                            //  Get the data range
                            float * pRun = (float *)ppg_vbo->single_buffer;
                            float north_max = -1e8;
                            float north_min = 1e8;
                            float east_max = -1e8;
                            float east_min = 1e8;

                            for( size_t i = 0 ; i < np ; i++){
                                float east = *pRun++;
                                float north = *pRun++;
                                north_max = wxMax(north, north_max);
                                north_min = wxMin(north, north_min);
                                east_max = wxMax(east, east_max);
                                east_min = wxMin(east, east_min);
                            }

                            float cfactx = wxMax(fabs(east_max), fabs(east_min));
                            float cfacty = wxMax(fabs(north_max), fabs(north_min));
                            float cfact = wxMax(cfactx, cfacty);

                            float sfact = cfact / 32700.0;

                            sfact = wxMax(sfact, 1.0);

                            //  Copy/convert the data
                            unsigned char *new_buf = (unsigned char *)malloc(np * 2 * sizeof(short));
                            pRun = (float *)ppg_vbo->single_buffer;
                            short *pd = (short *)new_buf;
                            for( size_t i = 0 ; i < np ; i++){
                                float east = *pRun++;
                                float north = *pRun++;
                                //                       short a = (east / sfact);
                                //                       short b = (north / sfact);
                                *pd++ = (east / sfact);
                                *pd++ = (north / sfact);

                            }

                            // replace the buffer
                            free(ppg_vbo->single_buffer);
                            ppg_vbo->single_buffer = new_buf;
                            ppg_vbo->single_buffer_size /= 2;

                            // Record the scale/offset factors
                            ppg_vbo->sfactor = sfact;
                            ppg_vbo->soffset = 0.;

                            ppg_vbo->data_type = DATA_TYPE_SHORT;

                        }
#endif


                        GLuint vboId;
                        // generate a new VBO and get the associated ID
                        glGenBuffers(1, &vboId);

                        rzRules->obj->auxParm0 = vboId;

                        // bind VBO in order to use
                        glBindBuffer(GL_ARRAY_BUFFER, vboId);

                        // upload data to VBO
#ifndef USE_ANDROID_GLES2
                        glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
#endif
                        glBufferData(GL_ARRAY_BUFFER, ppg_vbo->single_buffer_size, ppg_vbo->single_buffer, GL_STATIC_DRAW);

                    }
                    else {
                        glBindBuffer(GL_ARRAY_BUFFER, rzRules->obj->auxParm0);
#ifndef USE_ANDROID_GLES2
                        glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
#endif
                    }
                }
        }



        PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

            TriPrim *p_tp = ppg->tri_prim_head;
            GLintptr vbo_offset = 0;

#ifndef USE_ANDROID_GLES2
            glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
#endif
            //      Set up the stride sizes for the array
            int array_data_size = sizeof(float);
            GLint array_gl_type = GL_FLOAT;

            if(ppg->data_type == DATA_TYPE_DOUBLE){
                array_data_size = sizeof(double);
                array_gl_type = GL_DOUBLE;
            }

            if(ppg->data_type == DATA_TYPE_SHORT){
                array_data_size = sizeof(short);
                array_gl_type = GL_SHORT;
            }

            GLint program = S52AP_shader_program;
            glUseProgram(program);

            // Disable VBO's (vertex buffer objects) for attributes.
            if(!b_useVBO)
                glBindBuffer( GL_ARRAY_BUFFER, 0 );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

            GLint pos = glGetAttribLocation(program, "position");
            glEnableVertexAttribArray(pos);

            // Select the active texture unit.
            glActiveTexture( GL_TEXTURE0 );

            // Bind our texture to the texturing target.
            glBindTexture( GL_TEXTURE_2D, ppatt_spec->OGL_tex_name );

            // Set up the texture sampler to texture unit 0
            GLint texUni = glGetUniformLocation( program, "uTex" );
            glUniform1i( texUni, 0 );

            GLint texWidth  = glGetUniformLocation( program, "texWidth" );
            GLint texHeight  = glGetUniformLocation( program, "texHeight" );
            glUniform1f(texWidth, textureWidth);
            glUniform1f(texHeight, textureHeight);

            GLint texPOTWidth  = glGetUniformLocation( program, "texPOTWidth" );
            GLint texPOTHeight  = glGetUniformLocation( program, "texPOTHeight" );
            glUniform1f(texPOTWidth, ppatt_spec->w_pot);
            glUniform1f(texPOTHeight, ppatt_spec->h_pot);
            
            GLint xo  = glGetUniformLocation( program, "xOff" );
            GLint yo  = glGetUniformLocation( program, "yOff" );

            glUniform1f(xo, fmod(xOff, ppatt_spec->w_pot));
            glUniform1f(yo, fmod(yOff, ppatt_spec->h_pot));

            GLint yom  = glGetUniformLocation( program, "yOffM" );
            glUniform1f(yom, yOff);
            

            if(ppatt_spec->b_stagger){
                GLint staggerFact  = glGetUniformLocation( program, "staggerFactor" );
                glUniform1f(staggerFact, 0.5);
            }

            float angle = 0;

            // Build Transform matrix
            mat4x4 I, Q;
            mat4x4_identity(I);


       // Scale
            I[0][0] *= rzRules->obj->x_rate;
            I[1][1] *= rzRules->obj->y_rate;

        // Translate
            I[3][0] = -(rzRules->sm_transform_parms->easting_vp_center - rzRules->obj->x_origin) * vp->view_scale_ppm ;
            I[3][1] = -(rzRules->sm_transform_parms->northing_vp_center - rzRules->obj->y_origin) * -vp->view_scale_ppm ;

        // Scale
            I[0][0] *= vp->view_scale_ppm * ppg->sfactor;
            I[1][1] *= -vp->view_scale_ppm * ppg->sfactor;

        //Rotate
            mat4x4_rotate_Z(Q, I, angle);

        // Translate
            Q[3][0] += vp->pix_width / 2;
            Q[3][1] += vp->pix_height / 2;

            GLint matloc = glGetUniformLocation(program,"TransformMatrix");
            glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);


            if(!b_useVBO){
                float *bufBase = (float *)(&ppg->single_buffer[vbo_offset]);
                glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), bufBase);
            }

            while( p_tp ) {
                LLBBox box;
                if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
                }
                else
                    box = p_tp->tri_box;

                if(!BBView.IntersectOut(box)) {



                    if(b_useVBO) {
                        glVertexAttribPointer(pos, 2, array_gl_type, GL_FALSE, 0/*2*sizeof(array_gl_type)*/, (GLvoid *)(vbo_offset));
                        glDrawArrays(p_tp->type, 0, p_tp->nVert);
                    }
                    else {
                        glDrawArrays(p_tp->type, vbo_offset/(2 * array_data_size), p_tp->nVert);
                    }



                }

                vbo_offset += p_tp->nVert * 2 * array_data_size;

                // pick up the next in chain
                if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                    LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                    p_tp = (TriPrim *)p_ltp->p_next;
                }
                else
                    p_tp = p_tp->p_next;

            } // while

            if(b_useVBO)
                glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);

#ifndef USE_ANDROID_GLES2
            glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
#endif
            //  Restore the transform matrix to identity
            mat4x4 IM;
            mat4x4_identity(IM);
            GLint matlocf = glGetUniformLocation(S52color_tri_shader_program,"TransformMatrix");
            glUniformMatrix4fv( matlocf, 1, GL_FALSE, (const GLfloat*)IM);


        } // if pPolyTessGeo


#endif
        glDisable( GL_TEXTURE_2D );

        return 1;
}


void s52plib::RenderPolytessGL(ObjRazRules *rzRules, ViewPort *vp, double z_clip_geom, wxPoint *ptp)
{
#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2

    LLBBox BBView = vp->GetBBox();

    //  Allow a little slop in calculating whether a triangle
    //  is within the requested Viewport
    double margin = BBView.GetLonRange() * .05;

    int obj_xmin = 10000;
    int obj_xmax = -10000;
    int obj_ymin = 10000;
    int obj_ymax = -10000;
    
    PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
    
    TriPrim *p_tp = ppg->tri_prim_head;
    while( p_tp ) {
        if(!BBView.IntersectOut( p_tp->tri_box)) {
            //      Get and convert the points
            
            wxPoint *pr = ptp;
            if( ppg->data_type == DATA_TYPE_FLOAT ){
                float *pvert_list = (float *)p_tp->p_vertex;
                
                for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                    float lon = *pvert_list++;
                    float lat = *pvert_list++;
                    GetPointPixSingle(rzRules, lat, lon, pr, vp );
                    
                    obj_xmin = wxMin(obj_xmin, pr->x);
                    obj_xmax = wxMax(obj_xmax, pr->x);
                    obj_ymin = wxMin(obj_ymin, pr->y);
                    obj_ymax = wxMax(obj_ymax, pr->y);
                    
                    pr++;
                }
            }
            else {
                double *pvert_list = p_tp->p_vertex;
                
                for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                    double lon = *pvert_list++;
                    double lat = *pvert_list++;
                    GetPointPixSingle(rzRules, lat, lon, pr, vp );
                    
                    obj_xmin = wxMin(obj_xmin, pr->x);
                    obj_xmax = wxMax(obj_xmax, pr->x);
                    obj_ymin = wxMin(obj_ymin, pr->y);
                    obj_ymax = wxMax(obj_ymax, pr->y);
                    
                    pr++;
                }
            }
            
            
            
            switch( p_tp->type ){
                case PTG_TRIANGLE_FAN: {
                    glBegin( GL_TRIANGLE_FAN );
                    for( int it = 0; it < p_tp->nVert; it++ )
                        glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                    glEnd();
                    break;
                }
                
                case PTG_TRIANGLE_STRIP: {
                    glBegin( GL_TRIANGLE_STRIP );
                    for( int it = 0; it < p_tp->nVert; it++ )
                        glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                    glEnd();
                    break;
                }
                case PTG_TRIANGLES: {
                    glBegin( GL_TRIANGLES );
                    for( int it = 0; it < p_tp->nVert; it += 3 ) {
                        int xmin = wxMin(ptp[it].x, wxMin(ptp[it+1].x, ptp[it+2].x));
                        int xmax = wxMax(ptp[it].x, wxMax(ptp[it+1].x, ptp[it+2].x));
                        int ymin = wxMin(ptp[it].y, wxMin(ptp[it+1].y, ptp[it+2].y));
                        int ymax = wxMax(ptp[it].y, wxMax(ptp[it+1].y, ptp[it+2].y));
                        
                        wxRect rect( xmin, ymin, xmax - xmin, ymax - ymin );
//                        if( rect.Intersects( m_render_rect ) )
                        {
                            glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                            glVertex3f( ptp[it + 1].x, ptp[it + 1].y, z_clip_geom );
                            glVertex3f( ptp[it + 2].x, ptp[it + 2].y, z_clip_geom );
                        }
                    }
                    glEnd();
                    break;
                }
            }
        } // if bbox
        p_tp = p_tp->p_next; // pick up the next in chain
    } // while
    
#endif    
#endif
}

#ifdef ocpnUSE_GL

int s52plib::RenderAreaToGL( const wxGLContext &glcc, ObjRazRules *rzRules, ViewPort *vp )
{
    if( !ObjectRenderCheckRules( rzRules, vp, true ) )
        return 0;

    Rules *rules = rzRules->LUP->ruleList;

    while( rules != NULL ) {
        switch( rules->ruleType ){
            case RUL_ARE_CO:
                RenderToGLAC( rzRules, rules, vp );
                break; // AC

            case RUL_ARE_PA:
                RenderToGLAP( rzRules, rules, vp );
                break; // AP

            case RUL_CND_SY: {
                if( !rzRules->obj->bCS_Added ) {
                    rzRules->obj->CSrules = NULL;
                    GetAndAddCSRules( rzRules, rules );
                    rzRules->obj->bCS_Added = 1; // mark the object
                }
                Rules *rules_last = rules;
                rules = rzRules->obj->CSrules;

                while( NULL != rules ) {
                        switch( rules->ruleType ){
                            case RUL_ARE_CO:
                                RenderToGLAC( rzRules, rules, vp );
                                break;
                            case RUL_ARE_PA:
                                RenderToGLAP( rzRules, rules, vp );
                                break;
                            case RUL_NONE:
                            default:
                                break; // no rule type (init)
                        }
                        rules_last = rules;
                        rules = rules->next;
                }

                rules = rules_last;
                break;
            }

            case RUL_NONE:
            default:
                break; // no rule type (init)
        } // switch

        rules = rules->next;
    }

    return 1;

}
#endif

render_canvas_parms* s52plib::CreatePatternBufferSpec( ObjRazRules *rzRules, Rules *rules,
                                                       ViewPort *vp, bool b_revrgb, bool b_pot )
{
    wxImage Image;
    
    Rule *prule = rules->razRule;
    
    bool bstagger_pattern = ( prule->fillType.PATP == 'S' );
    
    wxColour local_unused_wxColor = m_unused_wxColor;
    
    //      Create a wxImage of the pattern drawn on an "unused_color" field
    if( prule->definition.SYDF == 'R' ) {
        Image = useLegacyRaster ?
        RuleXBMToImage( prule ) : ChartSymbols::GetImage( prule->name.PANM );
    }
    
    else          // Vector
    {
        float fsf = 100 / canvas_pix_per_mm;
        
        // Base bounding box
        wxBoundingBox box( prule->pos.patt.bnbox_x.PBXC, prule->pos.patt.bnbox_y.PBXR,
                           prule->pos.patt.bnbox_x.PBXC + prule->pos.patt.bnbox_w.PAHL,
                           prule->pos.patt.bnbox_y.PBXR + prule->pos.patt.bnbox_h.PAVL );
        
        // Expand to include pivot
        box.Expand( prule->pos.patt.pivot_x.PACL, prule->pos.patt.pivot_y.PARW );
        
        //    Pattern bounding boxes may be offset from origin, to preset the spacing
        //    So, the bitmap must be delta based.
        double dwidth = box.GetWidth();
        double dheight = box.GetHeight();
        
        //  Add in the pattern spacing parameters
        dwidth += prule->pos.patt.minDist.PAMI;
        dheight += prule->pos.patt.minDist.PAMI;
        
        //  Prescale
        dwidth /= fsf;
        dheight /= fsf;
        
        int width = (int) dwidth + 1;
        int height = (int) dheight + 1;
        
         
        float render_scale = 1.0;
#ifdef sUSE_ANDROID_GLES2
        int width_pot = width;
        int height_pot = height;
        if( b_pot ) {
            int xp = width;
            if(((xp != 0) && !(xp & (xp - 1))))     // detect POT
                width_pot = xp;
            else{
                int a = 0;
                while( xp ) {
                    xp = xp >> 1;
                    a++;
                }
                width_pot = 1 << a;
            }
            
            xp = height;
            if(((xp != 0) && !(xp & (xp - 1))))
                height_pot = xp;
            else{
                int a = 0;
                while( xp ) {
                    xp = xp >> 1;
                    a++;
                }
                height_pot = 1 << a;
            }
        }

        // adjust scaler
        render_scale = (float) height_pot / (float) height;
        qDebug() << "first" << width << width_pot << height << height_pot << render_scale;
        
        width = width_pot;
        height = height_pot;
#endif        
        
        
        //      Instantiate the vector pattern to a wxBitmap
        wxMemoryDC mdc;
        
        //  TODO
        // This ought to work for wxOSX, but DOES NOT.
        // We we do not want anti-aliased lines drawn in the pattern spec, since we used the solid primary color
        // as a mask for manual blitting from the dc to memory buffer.
        
        // Best we can do is set the background color very dark, and hope for the best
        #ifdef __WXOSX__
        #if wxUSE_GRAPHICS_CONTEXT
        wxGraphicsContext* pgc = mdc.GetGraphicsContext();
        if(pgc)
            pgc->SetAntialiasMode(wxANTIALIAS_NONE); 
        #endif
            local_unused_wxColor.Set(2,2,2);
            #endif
            
            wxBitmap *pbm = NULL;
            
            if( ( 0 != width ) && ( 0 != height ) ) {
                pbm = new wxBitmap( width, height );
                
                mdc.SelectObject( *pbm );
                mdc.SetBackground( wxBrush( local_unused_wxColor ) );
                mdc.Clear();
                
                int pivot_x = prule->pos.patt.pivot_x.PACL;
                int pivot_y = prule->pos.patt.pivot_y.PARW;
                
                char *str = prule->vector.LVCT;
                char *col = prule->colRef.LCRF;
                wxPoint pivot( pivot_x, pivot_y );
                
                int origin_x = prule->pos.patt.bnbox_x.PBXC;
                int origin_y = prule->pos.patt.bnbox_y.PBXR;
                wxPoint origin(origin_x, origin_y);
                
                wxPoint r0( (int) ( ( pivot_x - box.GetMinX() ) / fsf ) + 1,
                            (int) ( ( pivot_y - box.GetMinY() ) / fsf ) + 1 );
                
                HPGL->SetTargetDC( &mdc );
                HPGL->SetVP(vp);
                HPGL->Render( str, col, r0, pivot, origin, 1.0 /*render_scale*/, 0, false);

//                 mdc.SetPen( wxPen( wxColor(0, 0, 250), 1, wxPENSTYLE_SOLID ) );
//                 mdc.SetBrush(*wxTRANSPARENT_BRUSH);
//                 mdc.DrawRectangle(0,0, width-1, height-1);
                
            } else {
                pbm = new wxBitmap( 2, 2 );       // substitute small, blank pattern
                mdc.SelectObject( *pbm );
                mdc.SetBackground( wxBrush( local_unused_wxColor ) );
                mdc.Clear();
            }
            
            mdc.SelectObject( wxNullBitmap );
            
            //    Build a wxImage from the wxBitmap
            Image = pbm->ConvertToImage();
            
            delete pbm;
    }
    
    //  Convert the wxImage to a populated render_canvas_parms struct
    
    int sizey = Image.GetHeight();
    int sizex = Image.GetWidth();
    
    render_canvas_parms *patt_spec = new render_canvas_parms;
    patt_spec->OGL_tex_name = 0;
    
    if( b_pot ) {
        int xp = sizex;
        if(((xp != 0) && !(xp & (xp - 1))))     // detect POT
            patt_spec->w_pot = sizex;
        else{
        int a = 0;
        while( xp ) {
            xp = xp >> 1;
            a++;
        }
        patt_spec->w_pot = 1 << a;
        }
        
        xp = sizey;
        if(((xp != 0) && !(xp & (xp - 1))))
            patt_spec->h_pot = sizey;
        else{
            int a = 0;
        while( xp ) {
            xp = xp >> 1;
            a++;
        }
        patt_spec->h_pot = 1 << a;
        }
        
    } else {
        patt_spec->w_pot = sizex;
        patt_spec->h_pot = sizey;
    }
    
    patt_spec->depth = 32;             // set the depth, always 32 bit
    
    patt_spec->pb_pitch = ( ( patt_spec->w_pot * patt_spec->depth / 8 ) );
    patt_spec->lclip = 0;
    patt_spec->rclip = patt_spec->w_pot - 1;
    patt_spec->pix_buff = (unsigned char *) malloc( patt_spec->h_pot * patt_spec->pb_pitch );
    
    // Preset background
    memset( patt_spec->pix_buff, 0, patt_spec->h_pot * patt_spec->pb_pitch );
    patt_spec->width = sizex;
    patt_spec->height = sizey;
    patt_spec->x = 0;
    patt_spec->y = 0;
    patt_spec->b_stagger = bstagger_pattern;
    
    unsigned char *pd0 = patt_spec->pix_buff;
    unsigned char *pd;
    unsigned char *ps0 = Image.GetData();
    unsigned char *imgAlpha = NULL;
    bool b_use_alpha = false;
    if( Image.HasAlpha() ) {
        imgAlpha = Image.GetAlpha();
        b_use_alpha = true;
    }
    
    #if defined(__WXMAC__) || defined(__WXQT__)
    
    if( prule->definition.SYDF == 'V' ) {
        b_use_alpha = true;
        imgAlpha = NULL;
    }
    #endif
    
    unsigned char primary_r = 0;
    unsigned char primary_g = 0;
    unsigned char primary_b = 0;
    double reference_value = 0.5;
    
    bool b_filter = false;
    #if defined(__WXMAC__)
    S52color *primary_color = 0;
    if( prule->definition.SYDF == 'V' ){
        b_filter = true;
        char *col = prule->colRef.LCRF;
        primary_color = getColor( col+1);
        if(primary_color){
            primary_r = primary_color->R;
            primary_g = primary_color->G;
            primary_b = primary_color->B;
            wxImage::RGBValue rgb(primary_r,primary_g,primary_b);
            wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
            reference_value = hsv.value;
        }
    }
    #endif
    
    unsigned char *ps;
    
    {
        unsigned char mr = local_unused_wxColor.Red();
        unsigned char mg = local_unused_wxColor.Green();
        unsigned char mb = local_unused_wxColor.Blue();
        
        if( pd0 && ps0 ){
            for( int iy = 0; iy < sizey; iy++ ) {
#ifdef __OCPN__ANDROID__
                pd = pd0 + ( (sizey - iy - 1) * patt_spec->pb_pitch );
#else
                pd = pd0 + ( iy * patt_spec->pb_pitch );
#endif                
                ps = ps0 + ( iy * sizex * 3 );
                for( int ix = 0; ix < sizex; ix++ ) {
                    if( ix < sizex ) {
                        unsigned char r = *ps++;
                        unsigned char g = *ps++;
                        unsigned char b = *ps++;
                        
                        if(b_filter){
                            wxImage::RGBValue rgb(r,g,b);
                            wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
                            double ratio = hsv.value/reference_value;
                            
                            if(ratio > 0.5){
                                *pd++ = primary_r;
                                *pd++ = primary_g;
                                *pd++ = primary_b;
                                *pd++ = 255;
                            }
                            else{
                                *pd++ = 0;
                                *pd++ = 0;
                                *pd++ = 0;
                                *pd++ = 0;
                            }
                        }
                        else{
                            #ifdef ocpnUSE_ocpnBitmap
                            if( b_revrgb ) {
                                *pd++ = b;
                                *pd++ = g;
                                *pd++ = r;
                            } else {
                                *pd++ = r;
                                *pd++ = g;
                                *pd++ = b;
                            }
                            
                            #else
                            *pd++ = r;
                            *pd++ = g;
                            *pd++ = b;
                            #endif
                            if( b_use_alpha && imgAlpha ) {
                                *pd++ = *imgAlpha++;
                            } else {
                                *pd++ = ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                            }
                        }
                    }
                }
            }
        }
    }
    
    return patt_spec;
    
}


int s52plib::RenderToBufferAP( ObjRazRules *rzRules, Rules *rules, ViewPort *vp,
        render_canvas_parms *pb_spec )
{
    if(vp->m_projection_type != PROJECTION_MERCATOR)
        return 1;

    wxImage Image;

    if( rules->razRule == NULL )
        return 0;
    
    if( ( rules->razRule->pixelPtr == NULL ) || ( rules->razRule->parm1 != m_colortable_index )
            || ( rules->razRule->parm0 != ID_RGB_PATT_SPEC ) ) {
        render_canvas_parms *patt_spec = CreatePatternBufferSpec( rzRules, rules, vp, true );

        ClearRulesCache( rules->razRule ); //  Clear out any existing cached symbology

        rules->razRule->pixelPtr = patt_spec;
        rules->razRule->parm1 = m_colortable_index;
        rules->razRule->parm0 = ID_RGB_PATT_SPEC;

    } // Instantiation done

    //  Render the Area using the pattern spec stored in the rules
    render_canvas_parms *ppatt_spec = (render_canvas_parms *) rules->razRule->pixelPtr;

    //  Set the pattern reference point

    wxPoint r;
    GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

    ppatt_spec->x = r.x - 2000000; // bias way down to avoid zero-crossing logic in dda
    ppatt_spec->y = r.y - 2000000;

    RenderToBufferFilledPolygon( rzRules, rzRules->obj, NULL, pb_spec, ppatt_spec, vp );

    return 1;
}

int s52plib::RenderToBufferAC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp,
        render_canvas_parms *pb_spec )
{
    if(vp->m_projection_type != PROJECTION_MERCATOR)
        return 1;

    S52color *c;
    char *str = (char*) rules->INSTstr;

    c = getColor( str );

    RenderToBufferFilledPolygon( rzRules, rzRules->obj, c, pb_spec, NULL, vp );

    //    At very small scales, the object could be visible on both the left and right sides of the screen.
    //    Identify this case......
    if( vp->chart_scale > 5e7 ) {
        //    Does the object hang out over the left side of the VP?
        if( ( rzRules->obj->BBObj.GetMaxLon() > vp->GetBBox().GetMinLon() )
                && ( rzRules->obj->BBObj.GetMinLon() < vp->GetBBox().GetMinLon() ) ) {
            //    If we add 360 to the objects lons, does it intersect the the right side of the VP?
            if( ( ( rzRules->obj->BBObj.GetMaxLon() + 360. ) > vp->GetBBox().GetMaxLon() )
                    && ( ( rzRules->obj->BBObj.GetMinLon() + 360. ) < vp->GetBBox().GetMaxLon() ) ) {
                //  If so, this area oject should be drawn again, this time for the left side
                //    Do this by temporarily adjusting the objects rendering offset
                rzRules->obj->x_origin -= mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
                RenderToBufferFilledPolygon( rzRules, rzRules->obj, c, pb_spec,
                        NULL, vp );
                rzRules->obj->x_origin += mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;

            }
        }
    }

    return 1;
}

int s52plib::RenderAreaToDC( wxDC *pdcin, ObjRazRules *rzRules, ViewPort *vp,
        render_canvas_parms *pb_spec )
{

    if( !ObjectRenderCheckRules( rzRules, vp, true ) )
        return 0;

    m_pdc = pdcin; // use this DC
    Rules *rules = rzRules->LUP->ruleList;

    while( rules != NULL ) {
        switch( rules->ruleType ){
            case RUL_ARE_CO:
                RenderToBufferAC( rzRules, rules, vp, pb_spec );
                break; // AC
            case RUL_ARE_PA:
                RenderToBufferAP( rzRules, rules, vp, pb_spec );
                break; // AP

            case RUL_CND_SY: {
                if( !rzRules->obj->bCS_Added ) {
                    rzRules->obj->CSrules = NULL;
                    GetAndAddCSRules( rzRules, rules );
                    rzRules->obj->bCS_Added = 1; // mark the object
                }
                Rules *rules_last = rules;
                rules = rzRules->obj->CSrules;

                //    The CS procedure may have changed the Display Category of the Object, need to check again for visibility
                if( ObjectRenderCheckCat( rzRules, vp ) ) {
                    while( NULL != rules ) {
                        //Hve seen drgare fault here, need to code area query to debug
                        //possible that RENDERtoBUFFERAP/AC is blowing obj->CSRules
                        //    When it faults here, look at new debug field obj->CSLUP
                        switch( rules->ruleType ){
                            case RUL_ARE_CO:
                                RenderToBufferAC( rzRules, rules, vp, pb_spec );
                                break;
                            case RUL_ARE_PA:
                                RenderToBufferAP( rzRules, rules, vp, pb_spec );
                                break;
                            case RUL_NONE:
                            default:
                                break; // no rule type (init)
                        }
                        rules_last = rules;
                        rules = rules->next;
                    }
                }

                rules = rules_last;
                break;
            }

            case RUL_NONE:
            default:
                break; // no rule type (init)
        } // switch

        rules = rules->next;
    }

    return 1;

}

void s52plib::GetAndAddCSRules( ObjRazRules *rzRules, Rules *rules )
{

    LUPrec *NewLUP;
    LUPrec *LUP;
    LUPrec *LUPCandidate;

    char *rule_str1 = RenderCS( rzRules, rules );
    wxString cs_string( rule_str1, wxConvUTF8 );
    free( rule_str1 ); //delete rule_str1;

//  Try to find a match for this object/attribute set in dynamic CS LUP Table

//  Do this by checking each LUP in the CS LUPARRAY and checking....
//  a) is Object Name the same? and
//  b) was LUP created earlier by exactly the same INSTruction string?
//  c) does LUP have same Display Category and Priority?

    wxArrayOfLUPrec *la = condSymbolLUPArray;
    int index = 0;
    int index_max = la->GetCount();
    LUP = NULL;

    while( ( index < index_max ) ) {
        LUPCandidate = la->Item( index );
        if( !strcmp( rzRules->LUP->OBCL, LUPCandidate->OBCL ) ) {
            if( LUPCandidate->INST->IsSameAs( cs_string ) ) {
                if( LUPCandidate->DISC == rzRules->LUP->DISC ) {
                    LUP = LUPCandidate;
                    break;
                }
            }
        }
        index++;
    }

//  If not found, need to create a dynamic LUP and add to CS LUP Table

    if( NULL == LUP ) // Not found
            {

        NewLUP = (LUPrec*) calloc( 1, sizeof(LUPrec) );
        pAlloc->Add( NewLUP );

        NewLUP->DISC = rzRules->LUP->DISC; // as a default

        //sscanf(pBuf+11, "%d", &LUP->RCID);

        memcpy( NewLUP->OBCL, rzRules->LUP->OBCL, 6 ); // the object class name

//      Add the complete CS string to the LUP
        wxString *pINST = new wxString( cs_string );
        NewLUP->INST = pINST;

        _LUP2rules( NewLUP, rzRules->obj );

// Add LUP to array
        wxArrayOfLUPrec *pLUPARRAYtyped = condSymbolLUPArray;

        pLUPARRAYtyped->Add( NewLUP );

        LUP = NewLUP;

    } // if (LUP = NULL)

    Rules *top = LUP->ruleList;

    rzRules->obj->CSrules = top; // patch in a new rule set

}

bool s52plib::ObjectRenderCheck( ObjRazRules *rzRules, ViewPort *vp )
{
    if( !ObjectRenderCheckPos( rzRules, vp ) ) return false;

    if( !ObjectRenderCheckCat( rzRules, vp ) ) return false;

    return true;
}

bool s52plib::ObjectRenderCheckCS( ObjRazRules *rzRules, ViewPort *vp )
{
//  We need to do this test since some CS procedures change the display category
//  So we need to tentatively process all objects with CS LUPs
    Rules *rules = rzRules->LUP->ruleList;
    while( rules != NULL ) {
        if( RUL_CND_SY == rules->ruleType ) return true;

        rules = rules->next;
    }

    return false;
}

bool s52plib::ObjectRenderCheckPos( ObjRazRules *rzRules, ViewPort *vp )
{
    if( rzRules->obj == NULL ) 
        return false;

    // Of course, the object must be at least partly visible in the viewport
    const LLBBox &vpBox = vp->GetBBox(), &testBox = rzRules->obj->BBObj;

    if(vpBox.GetMaxLat() < testBox.GetMinLat() || vpBox.GetMinLat() > testBox.GetMaxLat())
        return false;

    if(vpBox.GetMaxLon() >= testBox.GetMinLon() && vpBox.GetMinLon() <= testBox.GetMaxLon())
        return true;

    if(vpBox.GetMaxLon() >= testBox.GetMinLon()+360 && vpBox.GetMinLon() <= testBox.GetMaxLon()+360)
        return true;

    if(vpBox.GetMaxLon() >= testBox.GetMinLon()-360 && vpBox.GetMinLon() <= testBox.GetMaxLon()-360)
        return true;

    return false;
}

bool s52plib::ObjectRenderCheckCat( ObjRazRules *rzRules, ViewPort *vp )
{
    g_scaminScale = 1.0;
    
    if( rzRules->obj == NULL ) return false;

    bool b_catfilter = true;
    bool b_visible = false;

    //      Do Object Type Filtering
    DisCat obj_cat = rzRules->obj->m_DisplayCat;
    
    //  Meta object filter. 
    // Applied when showing display category OTHER, and
    // only for objects whose decoded S52 display category (by LUP) is also OTHER
    if( m_nDisplayCategory == OTHER ){
        if(OTHER == obj_cat){
            if( !strncmp( rzRules->LUP->OBCL, "M_", 2 ) )
                if( !m_bShowMeta &&  strncmp( rzRules->LUP->OBCL, "M_QUAL", 6 ))
                    return false;
        }
    }
    else{
    // We want to filter out M_NSYS objects everywhere except "OTHER" category
        if( !strncmp( rzRules->LUP->OBCL, "M_", 2 ) )
            if( !m_bShowMeta )
                return false;
    }

#ifdef __OCPN__ANDROID__    
    // We want to filter out M_NSYS objects on Android, as they are of limited use on a phone/tablet
    if( !strncmp( rzRules->LUP->OBCL, "M_", 2 ) )
        if( !m_bShowMeta ) return false;
#endif        

    if( m_nDisplayCategory == MARINERS_STANDARD ) {
        if( -1 == rzRules->obj->iOBJL ) UpdateOBJLArray( rzRules->obj );

        if( DISPLAYBASE == obj_cat ){        // always display individual objects that were moved to DISPLAYBASE by CS Procedures
            b_visible = true;
            b_catfilter = false;
        }
        else if( !( (OBJLElement *) ( pOBJLArray->Item( rzRules->obj->iOBJL ) ) )->nViz ){
                b_catfilter = false;
        }
    }

    else
        if( m_nDisplayCategory == OTHER ) {
            if( ( DISPLAYBASE != obj_cat ) && ( STANDARD != obj_cat ) && ( OTHER != obj_cat ) ) {
                b_catfilter = false;
            }
        }

        else
            if( m_nDisplayCategory == STANDARD ) {
                if( ( DISPLAYBASE != obj_cat ) && ( STANDARD != obj_cat ) ) {
                    b_catfilter = false;
                }
            } else
                if( m_nDisplayCategory == DISPLAYBASE ) {
                    if( DISPLAYBASE != obj_cat ) {
                        b_catfilter = false;
                    }
                }

//  Soundings override
    if( !strncmp( rzRules->LUP->OBCL, "SOUNDG", 6 ) )
        b_catfilter = m_bShowSoundg;
    
    if( b_catfilter ) {
        b_visible = true;

//      SCAMIN Filtering
        //      Implementation note:
        //      According to S52 specs, SCAMIN must not apply to GROUP1 objects, Meta Objects
        //      or DisplayCategoryBase objects.
        //      Occasionally, an ENC will encode a spurious SCAMIN value for one of these objects.
        //      see, for example, US5VA18M, in OpenCPN SENC as Feature 350(DEPARE), LNAM = 022608187ED20ACC.
        //      We shall explicitly ignore SCAMIN filtering for these types of objects.

        if( m_bUseSCAMIN ) {

               
            if( ( DISPLAYBASE == rzRules->LUP->DISC ) || ( PRIO_GROUP1 == rzRules->LUP->DPRI ) )
                b_visible = true;
            else{
//                if( vp->chart_scale > rzRules->obj->Scamin ) b_visible = false;


                double zoom_mod = (double)g_chart_zoom_modifier_vector;

                double modf = zoom_mod/5.;  // -1->1
                double mod = pow(8., modf);
                mod = wxMax(mod, .2);
                mod = wxMin(mod, 8.0);

                if(mod > 1){
                    if( vp->chart_scale  > rzRules->obj->Scamin * mod )
                        b_visible = false;                              // definitely invisible
                    else{
                        //  Theoretically invisible, however...
                        //  In the "zoom modified" scale region,
                        //  we render the symbol at reduced size, scaling down to no less than half normal size.
                        
                        if(vp->chart_scale  > rzRules->obj->Scamin){
                            double xs = vp->chart_scale - rzRules->obj->Scamin;
                            double xl = (rzRules->obj->Scamin * mod) - rzRules->obj->Scamin;
                            g_scaminScale = 1.0 - (0.5 * xs / xl);
                            
                        }
                    }
                }
                else{
                    if(vp->chart_scale  > rzRules->obj->Scamin)
                        b_visible = false;
                }
            }

            //      On the other hand, $TEXTS features need not really be displayed at all scales, always
            //      To do so makes a very cluttered display
            if( ( !strncmp( rzRules->LUP->OBCL, "$TEXTS", 6 ) )
                    && ( vp->chart_scale > rzRules->obj->Scamin ) ) b_visible = false;
        }

        return b_visible;
    }

    return b_visible;
}

bool s52plib::ObjectRenderCheckRules( ObjRazRules *rzRules, ViewPort *vp, bool check_noshow )
{
    if( !ObjectRenderCheckPos( rzRules, vp ) ) 
        return false;

    if( check_noshow && IsObjNoshow( rzRules->LUP->OBCL) )
        return false;

    if( ObjectRenderCheckCat( rzRules, vp ) ) 
        return true;

    //  If this object cannot be moved to a higher category by CS procedures,
    //  then we are done here
    if(!rzRules->obj->m_bcategory_mutable)
        return false;

    // already added, nothing below can change its display category        
    if(rzRules->obj->bCS_Added ) 
        return false;

    //  Otherwise, make sure the CS, if present, has been evaluated,
    //  and then check the category again    
    //  no rules 
    if( !ObjectRenderCheckCS( rzRules, vp ) )
        return false;

    rzRules->obj->CSrules = NULL;
    Rules *rules = rzRules->LUP->ruleList;
    while( rules != NULL ) {
        if( RUL_CND_SY ==  rules->ruleType ){
            GetAndAddCSRules( rzRules, rules );
            rzRules->obj->bCS_Added = 1; // mark the object
            break;
        }
        rules = rules->next;
    }
    
    // still not displayable    
    if( !ObjectRenderCheckCat( rzRules, vp ) ) 
        return false;

    return true;
}


void s52plib::SetDisplayCategory(enum _DisCat cat)
{
    enum _DisCat old = m_nDisplayCategory;
    m_nDisplayCategory = cat;
    
    if(old != cat){
        ClearNoshow();
    }
    GenerateStateHash();
}


bool s52plib::IsObjNoshow( const char *objcl )
{
    for(unsigned int i=0 ; i < m_noshow_array.GetCount() ; i++){
        if(!strncmp(m_noshow_array[i].obj, objcl, 6) )
            return true;
    }
    return false;
}

void s52plib::AddObjNoshow( const char *objcl )
{
    if( !IsObjNoshow( objcl ) ){
        noshow_element element;
        memcpy(element.obj, objcl, 6);
        m_noshow_array.Add( element );
    }
}

void s52plib::RemoveObjNoshow( const char *objcl )
{
    for(unsigned int i=0 ; i < m_noshow_array.GetCount() ; i++){
        if(!strncmp(m_noshow_array[i].obj, objcl, 6) ){
            m_noshow_array.RemoveAt(i);
            return;
        }
    }
}

void s52plib::ClearNoshow(void)
{
    m_noshow_array.Clear();
}

void s52plib::PLIB_LoadS57Config()
{
    //    Get a pointer to the opencpn configuration object
    wxFileConfig *pconfig = GetOCPNConfigObject();
    
    int read_int;
    double dval;
    
    pconfig->SetPath( _T ( "/Settings" ) );
    //pconfig->Read( _T ( "DebugS57" ), &g_PIbDebugS57, 0 );         // Show LUP and Feature info in object query
    
    pconfig->SetPath( _T ( "/Settings/GlobalState" ) );
    
    pconfig->Read( _T ( "bShowS57Text" ), &read_int, 0 );
    SetShowS57Text( !( read_int == 0 ) );
    
    pconfig->Read( _T ( "bShowS57ImportantTextOnly" ), &read_int, 0 );
    SetShowS57ImportantTextOnly( !( read_int == 0 ) );
    
    pconfig->Read( _T ( "bShowLightDescription" ), &read_int, 0 );
    SetShowLdisText( !( read_int == 0 ) );
    
    pconfig->Read( _T ( "bExtendLightSectors" ), &read_int, 0 );
    SetExtendLightSectors( !( read_int == 0 ) );
    
    pconfig->Read( _T ( "nDisplayCategory" ), &read_int, (enum _DisCat) STANDARD );
    SetDisplayCategory((enum _DisCat) read_int );
    
    pconfig->Read( _T ( "nSymbolStyle" ), &read_int, (enum _LUPname) PAPER_CHART );
    m_nSymbolStyle = (LUPname) read_int;
    
    pconfig->Read( _T ( "nBoundaryStyle" ), &read_int, PLAIN_BOUNDARIES );
    m_nBoundaryStyle = (LUPname) read_int;
    
    pconfig->Read( _T ( "bShowSoundg" ), &read_int, 1 );
    m_bShowSoundg = !( read_int == 0 );
    
    pconfig->Read( _T ( "bShowMeta" ), &read_int, 0 );
    m_bShowMeta = !( read_int == 0 );
    
    pconfig->Read( _T ( "bUseSCAMIN" ), &read_int, 1 );
    m_bUseSCAMIN = !( read_int == 0 );
    
    pconfig->Read( _T ( "bShowAtonText" ), &read_int, 1 );
    m_bShowAtonText = !( read_int == 0 );
    
    pconfig->Read( _T ( "bDeClutterText" ), &read_int, 0 );
    m_bDeClutterText = !( read_int == 0 );
    
    pconfig->Read( _T ( "bShowNationalText" ), &read_int, 0 );
    m_bShowNationalTexts = !( read_int == 0 );
    
    if( pconfig->Read( _T ( "S52_MAR_SAFETY_CONTOUR" ), &dval, 5.0 ) ) {
        S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval );
        S52_setMarinerParam( S52_MAR_SAFETY_DEPTH, dval ); // Set safety_contour and safety_depth the same
    }
    
    if( pconfig->Read( _T ( "S52_MAR_SHALLOW_CONTOUR" ), &dval, 3.0 ) ) S52_setMarinerParam(
        S52_MAR_SHALLOW_CONTOUR, dval );
    
    if( pconfig->Read( _T ( "S52_MAR_DEEP_CONTOUR" ), &dval, 10.0 ) ) S52_setMarinerParam(
        S52_MAR_DEEP_CONTOUR, dval );
    
    if( pconfig->Read( _T ( "S52_MAR_TWO_SHADES" ), &dval, 0.0 ) ) S52_setMarinerParam(
        S52_MAR_TWO_SHADES, dval );
    
    UpdateMarinerParams();
    
    pconfig->SetPath( _T ( "/Settings/GlobalState" ) );
    pconfig->Read( _T ( "S52_DEPTH_UNIT_SHOW" ), &read_int, 1 );   // default is metres
    read_int = wxMax(read_int, 0);                      // qualify value
    read_int = wxMin(read_int, 2);
    m_nDepthUnitDisplay = read_int;
    
    //    S57 Object Class Visibility
    
    OBJLElement *pOLE;
    
    pconfig->SetPath( _T ( "/Settings/ObjectFilter" ) );
    
    int iOBJMax = pconfig->GetNumberOfEntries();
    if( iOBJMax ) {
        
        wxString str;
        long val;
        long dummy;
        
        wxString sObj;
        
        bool bCont = pconfig->GetFirstEntry( str, dummy );
        while( bCont ) {
            pconfig->Read( str, &val );              // Get an Object Viz
            
            bool bNeedNew = true;
            
            if( str.StartsWith( _T ( "viz" ), &sObj ) ) {
                for( unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++ ) {
                    pOLE = (OBJLElement *) ( pOBJLArray->Item( iPtr ) );
                    if( !strncmp( pOLE->OBJLName, sObj.mb_str(), 6 ) ) {
                        pOLE->nViz = val;
                        bNeedNew = false;
                        break;
                    }
                }
                
                if( bNeedNew ) {
                    pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
                    memcpy( pOLE->OBJLName, sObj.mb_str(), OBJL_NAME_LEN );
                    pOLE->nViz = 1;
                    
                    pOBJLArray->Add( (void *) pOLE );
                }
            }
            bCont = pconfig->GetNextEntry( str, dummy );
        }
    }
}

//    Do all those things necessary to prepare for a new rendering
void s52plib::PrepareForRender( void )
{
    PrepareForRender(NULL);
}

void s52plib::PrepareForRender(ViewPort *vp)
{
    m_benableGLLS = true;               // default is to always use RenderToGLLS (VBO support)

#ifdef USE_ANDROID_GLES2
void PrepareS52ShaderUniforms(ViewPort *vp);
    if(vp)
        PrepareS52ShaderUniforms( vp );
#endif

#ifdef BUILDING_PLUGIN    
    // Has the core S52PLIB configuration changed?
    //  If it has, reload from global preferences file, and other dynamic status information.
    //  This additional step is only necessary for Plugin chart rendering, as core directly sets
    //  options and updates State Hash as needed.

    int core_config = PI_GetPLIBStateHash();
    if(core_config != m_myConfig){
        
        g_ChartScaleFactorExp = GetOCPNChartScaleFactor_Plugin();
        
        //  If a modern (> OCPN 4.4) version of the core is active,
        //  we may rely upon having been updated on S52PLIB state by means of PlugIn messaging scheme.
        if( ((m_coreVersionMajor == 4) && (m_coreVersionMinor >= 5)) || m_coreVersionMajor > 4 ){
            
            // Retain compatibility with O4.8.x
            if( (m_coreVersionMajor == 4) && (m_coreVersionMinor < 9)){
             // First, we capture some temporary values that were set by messaging, but would be overwritten by config read
             bool bTextOn = m_bShowS57Text;
             bool bSoundingsOn = m_bShowSoundg;
             enum _DisCat old = m_nDisplayCategory;
             
             PLIB_LoadS57Config();
             
             //  And then reset the temp values that were overwritten by config load
             m_bShowS57Text = bTextOn;
             m_bShowSoundg = bSoundingsOn;
             m_nDisplayCategory = old;
            }
            else
                PLIB_LoadS57GlobalConfig();
            
            
            // Pick up any changes in Mariner's Standard object list
            PLIB_LoadS57ObjectConfig();

            // Detect and manage "LIGHTS" toggle
             bool bshow_lights = !m_lightsOff;
             if(!bshow_lights)                     // On, going off
                 AddObjNoshow("LIGHTS");
             else{                                   // Off, going on
                 RemoveObjNoshow("LIGHTS");
             }

            const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL", "SBDARE" };
            unsigned int num = sizeof(categories) / sizeof(categories[0]);
            
            // Handle Anchor area toggle
            if( (m_nDisplayCategory == OTHER) || (m_nDisplayCategory == MARINERS_STANDARD) ){
                bool bAnchor = m_anchorOn;
                
                
                if(!bAnchor){
                    for( unsigned int c = 0; c < num; c++ )
                        AddObjNoshow(categories[c]);
                    }
                else{
                    for( unsigned int c = 0; c < num; c++ )
                    RemoveObjNoshow(categories[c]);

                    //  Force the USER STANDARD object list anchor detail items ON
                    unsigned int cnt = 0;
                for( unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++ ) {
                    OBJLElement *pOLE = (OBJLElement *) ( pOBJLArray->Item( iPtr ) );
                        for( unsigned int c = 0; c < num; c++ ) {
                            if( !strncmp( pOLE->OBJLName, categories[c], 6 ) ) {
                                pOLE->nViz = 1;         // force on
                                cnt++;
                                break;
                            }
                        }
                        if( cnt == num ) break;
                    }
                }
            }
        }
        m_myConfig = PI_GetPLIBStateHash();
    }

#endif          //BUILDING_PLUGIN

    // Reset the LIGHTS declutter machine
    lastLightLat = 0;
    lastLightLon = 0;
    
}

void s52plib::SetAnchorOn(bool val)
{
    OBJLElement *pOLE = NULL;
            
    const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL", "SBDARE" };
    unsigned int num = sizeof(categories) / sizeof(categories[0]);
            
    if( (m_nDisplayCategory == OTHER) || (m_nDisplayCategory == MARINERS_STANDARD) ){
        bool bAnchor = val;
                
        if(!bAnchor){
            for( unsigned int c = 0; c < num; c++ ) {
                AddObjNoshow(categories[c]);
            }
        }
        else{
            for( unsigned int c = 0; c < num; c++ ) {
                RemoveObjNoshow(categories[c]);
            }
        }
    }
    else{                               // if not category OTHER, then anchor-related features are always shown.
        for( unsigned int c = 0; c < num; c++ ) {
            RemoveObjNoshow(categories[c]);
        }
    }
    
    m_anchorOn = val;
}

void s52plib::SetQualityOfData(bool val)
{
    int old_vis = GetQualityOfData();
    if(old_vis == val)
        return;
    
    if(old_vis && !val){                            // On, going off
        AddObjNoshow("M_QUAL");
    }
    else if(!old_vis && val){                                   // Off, going on
        RemoveObjNoshow("M_QUAL");

        for( unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "M_QUAL", 6 ) ) {
                pOLE->nViz = 1;         // force on
                break;
            }
        }
    }

    m_qualityOfDataOn = val;
    
}

void s52plib::ClearTextList( void )
{
    //      Clear the current text rectangle list
    m_textObjList.Clear();

}

bool s52plib::EnableGLLS(bool b_enable)
{
    bool return_val = m_benableGLLS;
    m_benableGLLS = b_enable;
    return return_val;
}
    
void s52plib::AdjustTextList( int dx, int dy, int screenw, int screenh )
{
    return;
    wxRect rScreen( 0, 0, screenw, screenh );
    //    Iterate over the text rectangle list
    //        1.  Apply the specified offset to the list elements
    //        2.. Remove any list elements that are off screen after applied offset

    TextObjList::Node *node = m_textObjList.GetFirst();
    TextObjList::Node *next;
    while( node ) {
        next = node->GetNext();
        wxRect *pcurrent = &( node->GetData()->rText );
        pcurrent->Offset( dx, dy );
        if( !pcurrent->Intersects( rScreen ) ) {
            m_textObjList.DeleteNode( node );
        }
        node = next;
    }
}

bool s52plib::GetPointPixArray( ObjRazRules *rzRules, wxPoint2DDouble* pd, wxPoint *pp, int nv, ViewPort *vp )
{
        for( int i = 0; i < nv; i++ ) {
            GetPointPixSingle(rzRules, pd[i].m_y, pd[i].m_x, pp + i, vp);
        }
    
    return true;
}

bool s52plib::GetPointPixSingle( ObjRazRules *rzRules, float north, float east, wxPoint *r, ViewPort *vp )
{
        if(vp->m_projection_type == PROJECTION_MERCATOR) {
            
            double xr =  rzRules->obj->x_rate;
            double xo =  rzRules->obj->x_origin;
            double yr =  rzRules->obj->y_rate;
            double yo =  rzRules->obj->y_origin;
            
            if(fabs(xo) > 1){                           // cm93 hits this
                if ( vp->GetBBox().GetMaxLon() >= 180. && rzRules->obj->BBObj.GetMaxLon() < vp->GetBBox().GetMinLon() )
                    xo += mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
                else if( (vp->GetBBox().GetMinLon() <= -180. &&
                    rzRules->obj->BBObj.GetMinLon() > vp->GetBBox().GetMaxLon()) ||
                (rzRules->obj->BBObj.GetMaxLon() >= 180 && vp->GetBBox().GetMinLon() <= 0.))
                    xo -= mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
            }

            double valx = ( east * xr ) + xo;
            double valy = ( north * yr ) + yo;

            r->x = roundint(((valx - rzRules->sm_transform_parms->easting_vp_center) * vp->view_scale_ppm) + (vp->pix_width / 2) );
            r->y = roundint((vp->pix_height/2) - ((valy - rzRules->sm_transform_parms->northing_vp_center) * vp->view_scale_ppm));
        } else {
              double lat, lon;
              fromSM_Plugin(east - rzRules->sm_transform_parms->easting_vp_center,
                     north - rzRules->sm_transform_parms->northing_vp_center,
                     vp->clat, vp->clon, &lat, &lon);

              *r = vp->GetPixFromLL(north, east);
        }
    
    return true;
}

void s52plib::GetPixPointSingle( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
#if 1
    vpt->GetLLFromPix(wxPoint(pixx, pixy), plat, plon);
//    if(*plon < 0 && vpt->clon > 180)
    //      *plon += 360;
#else
    //    Use Mercator estimator
    int dx = pixx - ( vpt->pix_width / 2 );
    int dy = ( vpt->pix_height / 2 ) - pixy;
    
    double xp = ( dx * cos( vpt->skew ) ) - ( dy * sin( vpt->skew ) );
    double yp = ( dy * cos( vpt->skew ) ) + ( dx * sin( vpt->skew ) );
    
    double d_east = xp / vpt->view_scale_ppm;
    double d_north = yp / vpt->view_scale_ppm;
    
    double slat, slon;
    fromSM( d_east, d_north, vpt->clat, vpt->clon, &slat, &slon );
    
    *plat = slat;
    *plon = slon;
#endif    
}

void s52plib::GetPixPointSingleNoRotate( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
    if(vpt){
        double rotation = vpt->rotation;
        vpt->SetRotationAngle(0);
        vpt->GetLLFromPix(wxPoint(pixx, pixy), plat, plon);
        vpt->SetRotationAngle(rotation);
    }
}    


void DrawAALine( wxDC *pDC, int x0, int y0, int x1, int y1, wxColour clrLine, int dash, int space )
{

    int width = 1 + abs( x0 - x1 );
    int height = 1 + abs( y0 - y1 );
    wxPoint upperLeft( wxMin ( x0, x1 ), wxMin ( y0, y1 ) );

    wxBitmap bm( width, height );
    wxMemoryDC mdc( bm );

    mdc.Blit( 0, 0, width, height, pDC, upperLeft.x, upperLeft.y );

#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC gdc( mdc );
#else
    wxMemoryDC &gdc( mdc );
#endif

    wxPen pen( clrLine, 1, wxPENSTYLE_USER_DASH );
    wxDash dashes[2];
    dashes[0] = dash;
    dashes[1] = space;
    pen.SetDashes( 2, dashes );
    gdc.SetPen( pen );

    gdc.DrawLine( x0 - upperLeft.x, y0 - upperLeft.y, x1 - upperLeft.x, y1 - upperLeft.y );

    pDC->Blit( upperLeft.x, upperLeft.y, width, height, &mdc, 0, 0 );

    mdc.SelectObject( wxNullBitmap );

    return;
}

void s52plib::DrawDashLine( wxPen &pen, wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, ViewPort *vp)
{
#ifdef USE_ANDROID_GLES2    
    glLineWidth( pen.GetWidth() );

    glUseProgram(S52color_tri_shader_program);
    
    float fBuf[4];
    GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), fBuf);
    glEnableVertexAttribArray(pos);
    
 /*   GLint matloc = glGetUniformLocation(S52color_tri_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)cc1->GetpVP()->vp_transform); 
 */   
    float colorv[4];
    colorv[0] = pen.GetColour().Red() / float(256);
    colorv[1] = pen.GetColour().Green() / float(256);
    colorv[2] = pen.GetColour().Blue() / float(256);
    colorv[3] = 1.0;
    
    GLint colloc = glGetUniformLocation(S52color_tri_shader_program,"color");
    glUniform4fv(colloc, 1, colorv);
   
   
    if(fabs(vp->rotation) > 0.01){
        float cx = vp->pix_width/2.;
        float cy = vp->pix_height/2.;
        float c = cosf(-vp->rotation );
        float s = sinf(-vp->rotation );
        
        float xn = x1 - cx;
        float yn = y1 - cy;
        x1 =  xn*c - yn*s + cx;
        y1 =  xn*s + yn*c + cy;

        xn = x2 - cx;
        yn = y2 - cy;
        x2 =  xn*c - yn*s + cx;
        y2 =  xn*s + yn*c + cy;
    }    
        
    wxDash *dashes;
    int n_dashes = pen.GetDashes( &dashes );
    if( n_dashes ) {
        float angle = atan2f( (float) ( y2 - y1 ), (float) ( x2 - x1 ) );
        float cosa = cosf( angle );
        float sina = sinf( angle );
        float t1 = pen.GetWidth();
        
        float lpix = sqrtf( powf(x1 - x2, 2) + powf(y1 - y2, 2) );
        float lrun = 0.;
        float xa = x1;
        float ya = y1;
        float ldraw = t1 * dashes[0];
        float lspace = t1 * dashes[1];
        
        ldraw = wxMax(ldraw, 4.0);
        lspace = wxMax(lspace, 4.0);
        lpix = wxMin(lpix, 2000.0);
        
        while( lrun < lpix ) {
            //    Dash
            float xb = xa + ldraw * cosa;
            float yb = ya + ldraw * sina;
            
            if( ( lrun + ldraw ) >= lpix )         // last segment is partial draw
            {
                xb = x2;
                yb = y2;
            }
                    
            fBuf[0] = xa;
            fBuf[1] = ya;
            fBuf[2] = xb;
            fBuf[3] = yb;
            
            glDrawArrays(GL_LINES, 0, 2);
            
            xa = xa + ( lspace + ldraw ) * cosa;
            ya = ya + ( lspace + ldraw ) * sina;
            lrun += lspace + ldraw;
            
        }
    } else {                    // not dashed
        fBuf[0] = x1;
        fBuf[1] = y1;
        fBuf[2] = x2;
        fBuf[3] = y2;
                
        glDrawArrays(GL_LINES, 0, 2);
    }
#endif    
}



RenderFromHPGL::RenderFromHPGL( s52plib* plibarg )
{
    plib = plibarg;
    renderToDC = false;
    renderToOpenGl = false;
    renderToGCDC = false;

    if(plib)
        scaleFactor = 100.0 / plib->GetPPMM();
    else
        scaleFactor = 10.0;  //Nominal


    workBufSize = 0;
    workBufIndex = 0;
    workBuf = NULL;
    
    s_odc_tess_work_buf = NULL;
    s_odc_tess_vertex_idx = 0;
    s_odc_tess_vertex_idx_this = 0;
    s_odc_tess_buf_len = 0;
    
    transparency = 255;
}

RenderFromHPGL::~RenderFromHPGL( )
{
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        glDisable (GL_BLEND );
    }
    
    free(workBuf);
    free(s_odc_tess_work_buf);
    
#endif    
}
    
void RenderFromHPGL::SetTargetDC( wxDC* pdc )
{
    targetDC = pdc;
    renderToDC = true;
    renderToOpenGl = false;
    renderToGCDC = false;
}

void RenderFromHPGL::SetTargetOpenGl()
{
    renderToOpenGl = true;
    renderToDC = false;
    renderToGCDC = false;
}

#if wxUSE_GRAPHICS_CONTEXT
void RenderFromHPGL::SetTargetGCDC( wxGCDC* gdc )
{
    targetGCDC = gdc;
    renderToGCDC = true;
    renderToDC = false;
    renderToOpenGl = false;
}
#endif

const char* RenderFromHPGL::findColorNameInRef( char colorCode, char* col )
{
    int noColors = strlen( col ) / 6;
    for( int i = 0, j=0; i < noColors; i++, j += 6 ) {
        if( *(col + j) == colorCode ) return col + j + 1;
    }
    return col + 1; // Default to first color if not found.
}

wxPoint RenderFromHPGL::ParsePoint( wxString& argument )
{
    long x, y;
    int colon = argument.Index( ',' );
    argument.Left( colon ).ToLong( &x );
    argument.Mid( colon + 1 ).ToLong( &y );
    return wxPoint( x, y );
}

void RenderFromHPGL::SetPen()
{
    float nominal_line_width_pix = wxMax(1.0, floor(plib->GetPPMM() / 5.0));             //0.2 mm nominal, but not less than 1 pixel
    int pen_width_mod =  floor( penWidth * nominal_line_width_pix);
    
    pen = wxThePenList->FindOrCreatePen( penColor, pen_width_mod, wxPENSTYLE_SOLID );
    brush = wxTheBrushList->FindOrCreateBrush( penColor, wxBRUSHSTYLE_SOLID );

    if( renderToDC ) {
        targetDC->SetPen( *pen );
        targetDC->SetBrush( *brush );
    }
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        if( plib->GetGLPolygonSmoothing() )
            glEnable( GL_POLYGON_SMOOTH );
        
#ifndef USE_ANDROID_GLES2
        glColor4ub( penColor.Red(), penColor.Green(), penColor.Blue(), transparency );
#endif        
        int line_width = wxMax(g_GLMinSymbolLineWidth, (float) penWidth * 0.7);
        glLineWidth( line_width );
        
#ifdef __OCPN__ANDROID__
        //  Scale the pen width dependent on the platform display resolution
        float nominal_line_width_pix = wxMax(1.0, floor(plib->GetPPMM() / 5.0));             //0.2 mm nominal, but not less than 1 pixel
        //qDebug() << nominal_line_width_pix;
        line_width =  wxMax(g_GLMinSymbolLineWidth, (float) penWidth * nominal_line_width_pix);
        glLineWidth( line_width );
#endif
        
#ifndef __OCPN__ANDROID__
        if( line_width >= 2 && plib->GetGLLineSmoothing() )
            glEnable( GL_LINE_SMOOTH );
        else
            glDisable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
#endif        
    }
#endif    
#if wxUSE_GRAPHICS_CONTEXT
    if( renderToGCDC ) {
        pen = wxThePenList->FindOrCreatePen( penColor, penWidth, wxPENSTYLE_SOLID );
        brush = wxTheBrushList->FindOrCreateBrush( penColor, wxBRUSHSTYLE_SOLID );
        targetGCDC->SetPen( *pen );
        targetGCDC->SetBrush( *brush );
    }
#endif
}

void RenderFromHPGL::Line( wxPoint from, wxPoint to )
{
    if( renderToDC ) {
        targetDC->DrawLine( from, to );
    }
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {

#ifdef USE_ANDROID_GLES2
        glUseProgram(S52color_tri_shader_program);

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

//         mat4x4 Q;
//         mat4x4 X;
//         mat4x4_identity(Q);
//         mat4x4_mul(X, (float (*)[4])m_vp->vp_transform, Q);
//
        //GLint matloc = glGetUniformLocation(S52color_tri_shader_program,"MVMatrix");
        //glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)cc1->GetpVP()->vp_transform);

        float colorv[4];
        colorv[0] = penColor.Red() / float(256);
        colorv[1] = penColor.Green() / float(256);
        colorv[2] = penColor.Blue() / float(256);
        colorv[3] = transparency / float(256);

        GLint colloc = glGetUniformLocation(S52color_tri_shader_program,"color");
        glUniform4fv(colloc, 1, colorv);

        float pts[4];
        pts[0] = from.x; pts[1] = from.y; pts[2] = to.x; pts[3] = to.y;

        GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");
        glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), pts);
        glEnableVertexAttribArray(pos);

        glDrawArrays(GL_LINES, 0, 2);
#else
        glBegin( GL_LINES );
        glVertex2i( from.x, from.y );
        glVertex2i( to.x, to.y );
        glEnd();
#endif
    }
#endif
#if wxUSE_GRAPHICS_CONTEXT
    if( renderToGCDC ) {
        targetGCDC->DrawLine( from, to );
    }
#endif
}

void RenderFromHPGL::Circle( wxPoint center, int radius, bool filled )
{
    if( renderToDC ) {
        if( filled )
            targetDC->SetBrush( *brush );
        else
            targetDC->SetBrush( *wxTRANSPARENT_BRUSH );
        targetDC->DrawCircle( center, radius );
    }
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
#ifdef USE_ANDROID_GLES2
        if(!m_vp)               // oops, forgot to set the VP parameters
            return;
        
    //      Enable anti-aliased lines, at best quality
        glEnable( GL_BLEND );
        
        float coords[8];
        coords[0] = center.x - radius;  coords[1] = center.y + radius;
        coords[2] = center.x + radius;  coords[3] = center.y + radius;
        coords[4] = center.x - radius;  coords[5] = center.y - radius;
        coords[6] = center.x + radius;  coords[7] = center.y - radius;
        
        glUseProgram( S52circle_filled_shader_program );
            
        // Get pointers to the attributes in the program.
        GLint mPosAttrib = glGetAttribLocation( S52circle_filled_shader_program, "aPos" );
        
            // Disable VBO's (vertex buffer objects) for attributes.
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
            
        glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords );
        glEnableVertexAttribArray( mPosAttrib );

        //  Circle radius
        GLint radiusloc = glGetUniformLocation(S52circle_filled_shader_program,"circle_radius");
        glUniform1f(radiusloc, radius);

        //  Circle center point
        GLint centerloc = glGetUniformLocation(S52circle_filled_shader_program,"circle_center");
        float ctrv[2];
        ctrv[0] = center.x; ctrv[1] = m_vp->pix_height - center.y;
        glUniform2fv(centerloc, 1, ctrv);
        
        //  Circle fill color
        float colorv[4];
        colorv[3] = 0.0;        // transparent default
        
        if(brush){
            colorv[0] = brush->GetColour().Red() / float(256);
            colorv[1] = brush->GetColour().Green() / float(256);
            colorv[2] = brush->GetColour().Blue() / float(256);
            if(filled)
                colorv[3] = 1.0; 
        }

        GLint colloc = glGetUniformLocation(S52circle_filled_shader_program,"circle_color");
        glUniform4fv(colloc, 1, colorv);
        
        //  Border color
        float bcolorv[4];
        bcolorv[0] = penColor.Red() / float(256);
        bcolorv[1] = penColor.Green() / float(256);
        bcolorv[2] = penColor.Blue() / float(256);
        bcolorv[3] = penColor.Alpha() / float(256);
        
        GLint bcolloc = glGetUniformLocation(S52circle_filled_shader_program,"border_color");
        glUniform4fv(bcolloc, 1, bcolorv);
        
        //  Border Width
        float nominal_line_width_pix = wxMax(1.0, floor(plib->GetPPMM() / 5.0));             //0.2 mm nominal, but not less than 1 pixel
        float line_width =  wxMax(g_GLMinSymbolLineWidth, (float) penWidth * nominal_line_width_pix);
        
        GLint borderWidthloc = glGetUniformLocation(S52circle_filled_shader_program,"border_width");
        glUniform1f(borderWidthloc, line_width);
        
            // Perform the actual drawing.
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //      Enable anti-aliased lines, at best quality
        glDisable( GL_BLEND );

#else        
        int noSegments = 2 + ( radius * 4 );
        if( noSegments > 200 ) noSegments = 200;
        glBegin( GL_LINE_STRIP );
        for( float a = 0; a <= 2 * M_PI; a += 2 * M_PI / noSegments )
            glVertex2f( center.x + radius * sinf( a ),
                        center.y + radius * cosf( a ) );
        glEnd();
#endif        
    }
#endif    
#if wxUSE_GRAPHICS_CONTEXT
    if( renderToGCDC ) {
        if( filled ) targetGCDC->SetBrush( *brush );
        else
            targetGCDC->SetBrush( *wxTRANSPARENT_BRUSH );

        targetGCDC->DrawCircle( center, radius );

        // wxGCDC doesn't update min/max X/Y properly for DrawCircle.
        targetGCDC->SetPen( *wxTRANSPARENT_PEN );
        targetGCDC->DrawPoint( center.x - radius, center.y );
        targetGCDC->DrawPoint( center.x + radius, center.y );
        targetGCDC->DrawPoint( center.x, center.y - radius );
        targetGCDC->DrawPoint( center.x, center.y + radius );
        targetGCDC->SetPen( *pen );
    }
#endif
}

void RenderFromHPGL::Polygon()
{
    if( renderToDC ) {
        targetDC->DrawPolygon( noPoints, polygon );
    }
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
#ifdef USE_ANDROID_GLES2

        penColor.Set(penColor.Red(), penColor.Green(), penColor.Blue(), transparency);
        pen = wxThePenList->FindOrCreatePen( penColor, penWidth, wxPENSTYLE_SOLID );
        brush = wxTheBrushList->FindOrCreateBrush( penColor, wxBRUSHSTYLE_SOLID );
        DrawPolygon( noPoints, polygon, 0, 0, 1.0, 0 );

#else
        glColor4ub( penColor.Red(), penColor.Green(), penColor.Blue(), transparency );
        
        glBegin( GL_POLYGON );
        for( int ip = 1; ip < noPoints; ip++ )
            glVertex2i( polygon[ip].x, polygon[ip].y );
        glEnd();
#endif
    }
#endif  // OpenGL

#if wxUSE_GRAPHICS_CONTEXT
    if( renderToGCDC ) {
        targetGCDC->DrawPolygon( noPoints, polygon );
    }
#endif
}

void RenderFromHPGL::RotatePoint( wxPoint& point, wxPoint origin, double angle )
{
    if( angle == 0. ) return;
    double sin_rot = sin( angle * PI / 180. );
    double cos_rot = cos( angle * PI / 180. );

    double xp = ( (point.x - origin.x) * cos_rot ) - ( (point.y - origin.y) * sin_rot );
    double yp = ( (point.x - origin.x) * sin_rot ) + ( (point.y - origin.y) * cos_rot );

    point.x = (int) xp + origin.x;
    point.y = (int) yp + origin.y;
}

bool RenderFromHPGL::Render( char *str, char *col, wxPoint &r, wxPoint &pivot, wxPoint origin, float scale, double rot_angle, bool bSymbol )
{
#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2
    if( renderToOpenGl )
        glGetFloatv(GL_CURRENT_COLOR,m_currentColor);
#endif    
#endif        
    
    wxPoint lineStart;
    wxPoint lineEnd;

    scaleFactor = 100.0 / plib->GetPPMM();
    scaleFactor /= scale;
    scaleFactor /= g_scaminScale;
    
    if(bSymbol)
        scaleFactor /= plib->GetRVScaleFactor();
    
    // SW is not always defined, cf. US/US4CA17M/US4CA17M.000
    penWidth = 1;

    wxStringTokenizer commands( wxString( str, wxConvUTF8 ), _T(";") );
    while( commands.HasMoreTokens() ) {
        wxString command = commands.GetNextToken();
        wxString arguments = command.Mid( 2 );
        command = command.Left( 2 );

        if( command == _T("SP") ) {
            S52color* color = plib->getColor( findColorNameInRef( arguments.GetChar( 0 ), col ) );
            penColor = wxColor( color->R, color->G, color->B );
            brushColor = penColor;
            continue;
        }
        if( command == _T("SW") ) {
            arguments.ToLong( &penWidth );
            continue;
        }
        if( command == _T("ST") ) {
            long transIndex;
            arguments.ToLong( &transIndex );
            transparency = (4 - transIndex) * 64;
            transparency = wxMin(transparency, 255);
            transparency = wxMax(0, transparency);
            continue;
        }
        if( command == _T("PU") ) {
            SetPen();
            lineStart = ParsePoint( arguments );
            RotatePoint( lineStart, origin, rot_angle );
            lineStart -= pivot;
            lineStart.x /= scaleFactor;
            lineStart.y /= scaleFactor;
            lineStart += r;
            continue;
        }
        if( command == _T("PD") ) {
            if( arguments.Length() == 0 ) {
                lineEnd = lineStart;
                lineEnd.x++;
            } else {
                lineEnd = ParsePoint( arguments );
                RotatePoint( lineEnd, origin, rot_angle );
                lineEnd -= pivot;
                lineEnd.x /= scaleFactor;
                lineEnd.y /= scaleFactor;
                lineEnd += r;
            }
            Line( lineStart, lineEnd );
            lineStart = lineEnd; // For next line.
            continue;
        }
        if( command == _T("CI") ) {
            long radius;
            arguments.ToLong( &radius );
            radius = (int) radius / scaleFactor;
            Circle( lineStart, radius );
            continue;
        }
        if( command == _T("PM") ) {
            noPoints = 1;
            polygon[0] = lineStart;

            if( arguments == _T("0") ) {
                do {
                    command = commands.GetNextToken();
                    arguments = command.Mid( 2 );
                    command = command.Left( 2 );

                    if( command == _T("AA") ) {
                        wxLogWarning( _T("RenderHPGL: AA instruction not implemented.") );
                    }
                    if( command == _T("CI") ) {
                        long radius;
                        arguments.ToLong( &radius );
                        radius = (int) radius / scaleFactor;
                        Circle( lineStart, radius, HPGL_FILLED );
                    }
                    if( command == _T("PD") ) {
                        wxStringTokenizer points( arguments, _T(",") );
                        while( points.HasMoreTokens() ) {
                            long x, y;
                            points.GetNextToken().ToLong( &x );
                            points.GetNextToken().ToLong( &y );
                            lineEnd = wxPoint( x, y );
                            RotatePoint( lineEnd, origin, rot_angle );
                            lineEnd -= pivot;
                            lineEnd.x /= scaleFactor;
                            lineEnd.y /= scaleFactor;
                            lineEnd += r;
                            polygon[noPoints++] = lineEnd;
                        }
                    }
                } while( command != _T("PM") );
            }
            continue;
        }
        if( command == _T("FP") ) {
            SetPen();
            Polygon();
            continue;
        }

        // Only get here if non of the other cases did a continue.
//        wxString msg( _T("RenderHPGL: The '%s' instruction is not implemented.") );
//        msg += wxString( command );
//        wxLogWarning( msg );
    }
    
    transparency = 255;
    
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        glDisable (GL_BLEND );
#ifndef USE_ANDROID_GLES2
        glColor4fv( m_currentColor );
#endif
    }
#endif    

    return true;
}



wxArrayPtrVoid s52gTesselatorVertices;

void RenderFromHPGL::DrawPolygon( int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset, float scale, float angle )
{
//    if( 0 )
        //dc->DrawPolygon( n, points, xoffset, yoffset );
    #ifdef ocpnUSE_GL
 //       else
        {
            
 #ifdef __WXQT__        
            glDisable( GL_LINE_SMOOTH );
            glDisable( GL_POLYGON_SMOOTH );
            glDisable( GL_BLEND );
            
 #else
            glEnable( GL_LINE_SMOOTH );
            glEnable( GL_POLYGON_SMOOTH );
            glEnable( GL_BLEND );
            
  #endif        
            
 #ifdef USE_ANDROID_GLES2
            
            //ConfigurePen();
            glLineWidth( pen->GetWidth() );
            
            glEnable( GL_BLEND );
            
             if(n > 4)
                 DrawPolygonTessellated( n, points, xoffset, yoffset);
             else
            {           // n = 3 or 4, most common case for pre-tesselated shapes
        
        
            //  Grow the work buffer as necessary
            if( workBufSize < (size_t)n*2 ){
                workBuf = (float *)realloc(workBuf, (n*4) * sizeof(float));
                workBufSize = n*4;
            }
            
            for( int i = 0; i < n; i++ ){
                workBuf[i*2] = (points[i].x * scale); // + xoffset;
                workBuf[i*2 + 1] = (points[i].y * scale); // + yoffset;
            }
            
            glUseProgram( S52color_tri_shader_program );
            
            // Get pointers to the attributes in the program.
            GLint mPosAttrib = glGetAttribLocation( S52color_tri_shader_program, "position" );
            
            // Disable VBO's (vertex buffer objects) for attributes.
            glBindBuffer( GL_ARRAY_BUFFER, 0 );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
            
            glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, workBuf );
            glEnableVertexAttribArray( mPosAttrib );
            
            
            //  Border color
            float bcolorv[4];
            bcolorv[0] = pen->GetColour().Red() / float(256);
            bcolorv[1] = pen->GetColour().Green() / float(256);
            bcolorv[2] = pen->GetColour().Blue() / float(256);
            bcolorv[3] = pen->GetColour().Alpha() / float(256);
            
            GLint bcolloc = glGetUniformLocation(S52color_tri_shader_program,"color");
            glUniform4fv(bcolloc, 1, bcolorv);

/// 
#if 0            
            wxWindow *win = GetOCPNCanvasWindow();
            ChartCanvas *canvas = wxDynamicCast( win, ChartCanvas); //classname * wxDynamicCast(ptr, classname)
            
            //ViewPort *pvp = (ViewPort *)&vp;
            ViewPort *pvp = canvas->GetpVP();
            
            // Rotate 
            mat4x4 I, Q;
            mat4x4_identity(I);
            mat4x4_rotate_Z(Q, I, angle);
            
            // Translate
            Q[3][0] = xoffset;
            Q[3][1] = yoffset;
            
            mat4x4 X;
            mat4x4_mul(X, (float (*)[4])pvp->vp_transform, Q);
            
            GLint matloc = glGetUniformLocation(S52color_tri_shader_program,"MVMatrix");
            glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)X ); 
///  
#endif 
            // Perform the actual drawing.
            glDrawArrays(GL_LINE_LOOP, 0, n);
            
            //  Fill color
            bcolorv[0] = brush->GetColour().Red() / float(256);
            bcolorv[1] = brush->GetColour().Green() / float(256);
            bcolorv[2] = brush->GetColour().Blue() / float(256);
            bcolorv[3] = brush->GetColour().Alpha() / float(256);
            
            glUniform4fv(bcolloc, 1, bcolorv);
            
            // For the simple common case of a convex rectangle...
            //  swizzle the array points to enable GL_TRIANGLE_STRIP
            if(n == 4){
                float x1 = workBuf[4];
                float y1 = workBuf[5];
                workBuf[4] = workBuf[6];
                workBuf[5] = workBuf[7];
                workBuf[6] = x1;
                workBuf[7] = y1;
                
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
            else if(n == 3){
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
            }    
            
            
#else        
            
            wxColour c = brush->GetColour();
            glColor4ub( c.Red(), c.Green(), c.Blue(), c.Alpha() );
            
            glEnable( GL_POLYGON_SMOOTH );
            glBegin( GL_POLYGON );
            for( int i = 0; i < n; i++ )
                glVertex2f( (points[i].x * scale) + xoffset, (points[i].y * scale) + yoffset );
            glEnd();
            glDisable( GL_POLYGON_SMOOTH );

            int width = pen->GetWidth();
            glLineWidth( width );
            
            glEnable( GL_LINE_SMOOTH );
            glBegin( GL_LINE_LOOP );
            for( int i = 0; i < n; i++ )
                glVertex2f( (points[i].x * scale) + xoffset, (points[i].y * scale) + yoffset );
            glEnd();
            glDisable( GL_LINE_SMOOTH );
#endif
            
            glDisable( GL_LINE_SMOOTH );
            glDisable( GL_POLYGON_SMOOTH );
            glDisable( GL_BLEND );
            
        }
        #endif    
}

#ifdef ocpnUSE_GL

// GL callbacks

typedef union {
    GLdouble data[6];
    struct sGLvertex {
        GLdouble x;
        GLdouble y;
        GLdouble z;
        GLdouble r;
        GLdouble g;
        GLdouble b;
    } info;
} GLvertex;

#ifndef USE_ANDROID_GLES2
void APIENTRY s52DCcombineCallback( GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4],
                                     GLdouble **dataOut )
{
    GLvertex *vertex;
    
    vertex = new GLvertex();
    s52gTesselatorVertices.Add(vertex );
    
    vertex->info.x = coords[0];
    vertex->info.y = coords[1];
    vertex->info.z = coords[2];
    
    for( int i = 3; i < 6; i++ ) {
        vertex->data[i] = weight[0] * vertex_data[0][i] + weight[1] * vertex_data[1][i];
    }
    
    *dataOut = &(vertex->data[0]);
}

void APIENTRY s52DCvertexCallback( GLvoid* arg )
{
    GLvertex* vertex;
    vertex = (GLvertex*) arg;
    glVertex2f( (float)vertex->info.x, (float)vertex->info.y );
}

void APIENTRY s52DCerrorCallback( GLenum errorCode )
{
    const GLubyte *estring;
    estring = gluErrorString(errorCode);
    //wxLogMessage( _T("OpenGL Tessellation Error: %s"), (char *)estring );
}

void APIENTRY s52DCbeginCallback( GLenum type )
{
    glBegin( type );
}

void APIENTRY s52DCendCallback()
{
    glEnd();
}
#endif


// GLSL callbacks


#ifdef USE_ANDROID_GLES2

static std::list<double*> odc_combine_work_data;
static void s52_combineCallbackD(GLdouble coords[3],
                                 GLdouble *vertex_data[4],
                                 GLfloat weight[4], GLdouble **dataOut, void *data )
{
    //     double *vertex = new double[3];
    //     odc_combine_work_data.push_back(vertex);
    //     memcpy(vertex, coords, 3*(sizeof *coords)); 
    //     *dataOut = vertex;
}

void s52_vertexCallbackD_GLSL(GLvoid *vertex, void *data)
{
    RenderFromHPGL* plib = (RenderFromHPGL*)data;
    
    // Grow the work buffer if necessary
    if(plib->s_odc_tess_vertex_idx > plib->s_odc_tess_buf_len - 8)
    {
        int new_buf_len = plib->s_odc_tess_buf_len + 100;
        GLfloat * tmp = plib->s_odc_tess_work_buf;
        
        plib->s_odc_tess_work_buf = (GLfloat *)realloc(plib->s_odc_tess_work_buf, new_buf_len * sizeof(GLfloat));
        if (NULL == plib->s_odc_tess_work_buf)
        {
            free(tmp);
            tmp = NULL;
        }
        else
            plib->s_odc_tess_buf_len = new_buf_len;
    }
    
    GLdouble *pointer = (GLdouble *) vertex;
    
    plib->s_odc_tess_work_buf[plib->s_odc_tess_vertex_idx++] = (float)pointer[0];
    plib->s_odc_tess_work_buf[plib->s_odc_tess_vertex_idx++] = (float)pointer[1];
    
    plib->s_odc_nvertex++;
}

void s52_beginCallbackD_GLSL( GLenum mode, void *data)
{
    RenderFromHPGL* plib = (RenderFromHPGL*)data;
    plib->s_odc_tess_vertex_idx_this =  plib->s_odc_tess_vertex_idx;
    plib->s_odc_tess_mode = mode;
    plib->s_odc_nvertex = 0;
}

void s52_endCallbackD_GLSL(void *data)
{
    //qDebug() << "End" << s_odc_nvertex << s_odc_tess_buf_len << s_odc_tess_vertex_idx << s_odc_tess_vertex_idx_this;
    //End 5 100 10 0
    #if 1    
    RenderFromHPGL* plib = (RenderFromHPGL*)data;
    
    glUseProgram(S52color_tri_shader_program);
    
    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
    float *bufPt = &(plib->s_odc_tess_work_buf[plib->s_odc_tess_vertex_idx_this]);
    GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), bufPt);
    glEnableVertexAttribArray(pos);
    
    ///GLint matloc = glGetUniformLocation(color_tri_shader_program,"MVMatrix");
    ///glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)s_tessVP.vp_transform); 
    
    float colorv[4];
    wxColour c = plib->getBrush()->GetColour();
    
    colorv[0] = c.Red() / float(256);
    colorv[1] = c.Green() / float(256);
    colorv[2] = c.Blue() / float(256);
    colorv[3] = c.Alpha() / float(256);
    
    GLint colloc = glGetUniformLocation(S52color_tri_shader_program,"color");
    glUniform4fv(colloc, 1, colorv);
    
    glDrawArrays(plib->s_odc_tess_mode, 0, plib->s_odc_nvertex);
    #endif    
}
#endif


#endif          //#ifdef ocpnUSE_GL

void RenderFromHPGL::DrawPolygonTessellated( int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset )
{
//    if( 0 )
        //dc->DrawPolygon( n, points, xoffset, yoffset );
    #ifdef ocpnUSE_GL
        //else
        {
             #if !defined(ocpnUSE_GLES) || defined(USE_ANDROID_GLES2) // tessalator in glues is broken
             if( n < 5 )
             # endif
             {
                 DrawPolygon( n, points, xoffset, yoffset, 1.0, 0 );
                 return;
             }
            
            
            
 #ifdef USE_ANDROID_GLES2
            m_tobj = gluNewTess();
            s_odc_tess_vertex_idx = 0;
            
            gluTessCallback( m_tobj, GLU_TESS_VERTEX_DATA, (_GLUfuncptr) &s52_vertexCallbackD_GLSL  );
            gluTessCallback( m_tobj, GLU_TESS_BEGIN_DATA, (_GLUfuncptr) &s52_beginCallbackD_GLSL  );
            gluTessCallback( m_tobj, GLU_TESS_END_DATA, (_GLUfuncptr) &s52_endCallbackD_GLSL  );
            gluTessCallback( m_tobj, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) &s52_combineCallbackD );
            
            gluTessNormal( m_tobj, 0, 0, 1);
            gluTessProperty( m_tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO );
            
                gluTessBeginPolygon( m_tobj, this );
                gluTessBeginContour( m_tobj );
                
 //               ViewPort *pvp = cc1->GetpVP();
                
                for( int i = 0; i < n; i++ ) {
                    double *p = new double[6];
                    
//                     if(fabs(pvp->rotation) > 0.01){
//                         float cx = pvp->pix_width/2.;
//                         float cy = pvp->pix_height/2.;
//                         float c = cosf(pvp->rotation );
//                         float s = sinf(pvp->rotation );
//                         float xn = points[i].x - cx;
//                         float yn = points[i].y - cy;
//                         p[0] =  xn*c - yn*s + cx;
//                         p[1] =  xn*s + yn*c + cy;
//                         p[2] = 0;
//                     }
//                     else
                        p[0] = points[i].x, p[1] = points[i].y, p[2] = 0;
                    
                    gluTessVertex(m_tobj, p, p);
                }
                
                gluTessEndContour( m_tobj );
                gluTessEndPolygon( m_tobj );
            //}
            
            gluDeleteTess(m_tobj);
            
            
            //         for(std::list<double*>::iterator i = odc_combine_work_data.begin(); i!=odc_combine_work_data.end(); i++)
            //             delete [] *i;
            //         odc_combine_work_data.clear();
            
        }  
 #else    
        static GLUtesselator *tobj = NULL;
        if( ! tobj ) tobj = gluNewTess();
        
                        gluTessCallback( tobj, GLU_TESS_VERTEX, (_GLUfuncptr) &s52DCvertexCallback );
        gluTessCallback( tobj, GLU_TESS_BEGIN, (_GLUfuncptr) &s52DCbeginCallback );
        gluTessCallback( tobj, GLU_TESS_END, (_GLUfuncptr) &s52DCendCallback );
        gluTessCallback( tobj, GLU_TESS_COMBINE, (_GLUfuncptr) &s52DCcombineCallback );
        gluTessCallback( tobj, GLU_TESS_ERROR, (_GLUfuncptr) &s52DCerrorCallback );
        
        gluTessNormal( tobj, 0, 0, 1);
        gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO );
        
            wxColour c = brush->GetColour();
            glColor4ub( c.Red(), c.Green(), c.Blue(), c.Alpha() );
            
            gluTessBeginPolygon( tobj, NULL );
            gluTessBeginContour( tobj );
            
            for( int i = 0; i < n; i++ ) {
                GLvertex* vertex = new GLvertex();
                s52gTesselatorVertices.Add( vertex );
                vertex->info.x = (GLdouble) points[i].x;
                vertex->info.y = (GLdouble) points[i].y;
                vertex->info.z = (GLdouble) 0.0;
                vertex->info.r = (GLdouble) 0.0;
                vertex->info.g = (GLdouble) 0.0;
                vertex->info.b = (GLdouble) 0.0;
                gluTessVertex( tobj, (GLdouble*)vertex, (GLdouble*)vertex );
            }
            gluTessEndContour( tobj );
            gluTessEndPolygon( tobj );
        
            for( unsigned int i=0; i<s52gTesselatorVertices.Count(); i++ )
                delete (GLvertex*)s52gTesselatorVertices.Item(i);
            s52gTesselatorVertices.Clear();
        
        gluDeleteTess(tobj);
        
}
#endif    
#endif    
}




#ifdef ocpnUSE_GL
/* draw a half circle using triangles */
void PLIBDrawEndCap(float x1, float y1, float t1, float angle)
{
#ifndef USE_ANDROID_GLES2
    const int steps = 16;
    float xa, ya;
    bool first = true;
    for(int i = 0; i <= steps; i++) {
        float a = angle + M_PI/2 + M_PI/steps*i;
        
        float xb = x1 + t1 / 2 * cos( a );
        float yb = y1 + t1 / 2 * sin( a );
        if(first)
            first = false;
        else {
            glVertex2f( x1, y1 );
            glVertex2f( xa, ya );
            glVertex2f( xb, yb );
        }
        xa = xb, ya = yb;
    }
#endif    
}
#endif

// Draws a line between (x1,y1) - (x2,y2) with a start thickness of t1
void PLIBDrawGLThickLine( float x1, float y1, float x2, float y2, wxPen pen, bool b_hiqual )
{
#ifdef ocpnUSE_GL
#ifndef USE_ANDROID_GLES2
    float angle = atan2f( y2 - y1, x2 - x1 );
    float t1 = pen.GetWidth();
    float t2sina1 = t1 / 2 * sinf( angle );
    float t2cosa1 = t1 / 2 * cosf( angle );
    
    glBegin( GL_TRIANGLES );
    
    //    n.b.  The dwxDash interpretation for GL only allows for 2 elements in the dash table.
    //    The first is assumed drawn, second is assumed space
    wxDash *dashes;
    int n_dashes = pen.GetDashes( &dashes );
    if( n_dashes ) {
        float lpix = sqrtf( powf( (float) (x1 - x2), 2) + powf( (float) (y1 - y2), 2) );
        float lrun = 0.;
        float xa = x1;
        float ya = y1;
        float ldraw = t1 * (unsigned char)dashes[0];
        float lspace = t1 * (unsigned char)dashes[1];
        
        if((ldraw < 0) || (lspace < 0)){
            glEnd();
            return;
        }
        
        while( lrun < lpix ) {
            //    Dash
            float xb = xa + ldraw * cosf( angle );
            float yb = ya + ldraw * sinf( angle );
            
            if( ( lrun + ldraw ) >= lpix )         // last segment is partial draw
            {
                xb = x2;
                yb = y2;
            }
            
            glVertex2f( xa + t2sina1, ya - t2cosa1 );
            glVertex2f( xb + t2sina1, yb - t2cosa1 );
            glVertex2f( xb - t2sina1, yb + t2cosa1 );
            
            glVertex2f( xb - t2sina1, yb + t2cosa1 );
            glVertex2f( xa - t2sina1, ya + t2cosa1 );
            glVertex2f( xa + t2sina1, ya - t2cosa1 );
            
            xa = xb;
            ya = yb;
            lrun += ldraw;
            
            //    Space
            xb = xa + lspace * cos( angle );
            yb = ya + lspace * sin( angle );
            
            xa = xb;
            ya = yb;
            lrun += lspace;
        }
    } else {
        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );
        glVertex2f( x2 + t2sina1, y2 - t2cosa1 );
        glVertex2f( x2 - t2sina1, y2 + t2cosa1 );
        
        glVertex2f( x2 - t2sina1, y2 + t2cosa1 );
        glVertex2f( x1 - t2sina1, y1 + t2cosa1 );
        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );
        
        /* wx draws a nice rounded end in dc mode, so replicate
         *           this for opengl mode, should this be done for the dashed mode case? */
        if(pen.GetCap() == wxCAP_ROUND) {
            PLIBDrawEndCap( x1, y1, t1, angle);
            PLIBDrawEndCap( x2, y2, t1, angle + M_PI);
        }
        
    }
    
    glEnd();
#endif    
#endif    
}

#ifdef USE_ANDROID_GLES2

#include <gl2.h>

// Simple colored triangle shader

static const GLchar* S52color_tri_vertex_shader_source =
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "uniform vec4 color;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   fragColor = color;\n"
    "   gl_Position = MVMatrix * TransformMatrix  * vec4(position, 0.0, 1.0);\n"
    "}\n";

    static const GLchar* S52color_tri_fragment_shader_source =
    "precision highp float;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   gl_FragColor = fragColor;\n"
    "}\n";

    // Simple 2D texture shader
static const GLchar* S52texture_2D_vertex_shader_source =
    "attribute vec2 position;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "   //varCoord = aUV.st;\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar* S52texture_2D_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord);\n"
    "}\n";


    //  Circle shader

static const GLchar* S52circle_filled_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const GLchar* S52circle_filled_fragment_shader_source =
    "precision highp float;\n"
    "uniform float border_width;\n"
    "uniform float circle_radius;\n"
    "uniform vec4 circle_color;\n"
    "uniform vec4 border_color;\n"
    "uniform vec2 circle_center;\n"
    "void main(){\n"
    "float d = distance(gl_FragCoord.xy, circle_center);\n"
    "if (d < (circle_radius - border_width)) { gl_FragColor = circle_color; }\n"
    "else if (d < circle_radius) { gl_FragColor = border_color; }\n"
    "else { gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); }\n"
    "}\n";

    //  Ring shader

static const GLchar* S52ring_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const GLchar* S52ring_fragment_shader_source =
    "precision highp float;\n"
    "uniform float border_width;\n"
    "uniform float circle_radius;\n"
    "uniform float ring_width;\n"
    "uniform vec4 circle_color;\n"
    "uniform vec4 border_color;\n"
    "uniform vec2 circle_center;\n"
    "uniform float sector_1;\n"
    "uniform float sector_2;\n"

    "void main(){\n"
    "const float PI = 3.14159265358979323846264;\n"
    "bool bdraw = false;\n"

    "float angle = atan(gl_FragCoord.y-circle_center.y, gl_FragCoord.x-circle_center.x);\n"
    "angle = PI/2.0 - angle;\n"
    "if(angle < 0.0) angle += PI * 2.0;\n"

    "if(sector_2 > PI * 2.0){\n"
    "    if((angle > sector_1) && (angle < (PI * 2.0) )){\n"
    "        bdraw = true;\n"
    "    }\n"
    "    if(angle < sector_2 - (PI * 2.0)){\n"
    "        bdraw = true;\n"
    "    }\n"
    "} else {\n"
    "    if((angle > sector_1) && (angle < sector_2)){\n"
    "        bdraw = true;\n"
    "    }\n"
    "}\n"

    "if(bdraw){\n"
    "   float d = distance(gl_FragCoord.xy, circle_center);\n"
    "   if (d > circle_radius) {\n"
    "       discard;\n"
    "   } else if( d > (circle_radius - border_width)) {\n"
    "       gl_FragColor = border_color;\n"
    "   } else if( d > (circle_radius - border_width - ring_width)) {\n"
    "       gl_FragColor = circle_color;\n"
    "   } else if( d > (circle_radius - border_width - ring_width - border_width)) {\n"
    "       gl_FragColor = border_color;\n"
    "   } else  {\n"
    "       discard;\n"
    "   }\n"
    "} else{\n"
    "   discard;\n"
    "}\n"
    "}\n";


    // Dash/Dot line shader
static const GLchar* S52Dash_vertex_shader_source =
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "}\n";

static const GLchar* S52Dash_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "uniform vec2 startPos;\n"
    "uniform float texWidth;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    "   float d = distance(gl_FragCoord.xy, startPos);\n"
    "   float x = mod(d,texWidth) / texWidth;\n"
    "   if(x < 0.5) gl_FragColor = color;\n"
    "   else gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
    "}\n";


    // Texture shader for Area Pattern
static const GLchar* S52AP_vertex_shader_source =
    "attribute vec2 position;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "}\n";

#if 0
static const GLchar* S52AP_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "uniform float texWidth;\n"
    "uniform float texHeight;\n"
    "uniform float texPOTWidth;\n"
    "uniform float texPOTHeight;\n"
    "uniform float staggerFactor;\n"
    "uniform vec4 color;\n"
    "uniform float xOff;\n"
    "uniform float yOff;\n"
    "void main() {\n"
    "   float yp = floor((gl_FragCoord.y + yOff) / texHeight);\n"
    "   float fstagger = 0.0;\n"
    "   //if(mod(yp, 2.0) < 0.1) fstagger = staggerFactor;\n"
    "   float xStag = xOff + (fstagger * texWidth);\n"
    "   float x = mod((gl_FragCoord.x - xStag),texWidth) / texPOTWidth;\n"
    "   float y = mod((gl_FragCoord.y + yOff),texHeight) / texPOTHeight;\n"
     "   gl_FragColor = texture2D(uTex, vec2(x, (texHeight / texPOTHeight) - y));\n"
    "}\n";
#else

static const GLchar* S52AP_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "uniform float texWidth;\n"
    "uniform float texHeight;\n"
    "uniform float texPOTWidth;\n"
    "uniform float texPOTHeight;\n"
    "uniform float staggerFactor;\n"
    "uniform vec4 color;\n"
    "uniform float xOff;\n"
    "uniform float yOff;\n"
    "uniform float yOffM;\n"
    "void main() {\n"
    "   float yp = floor((gl_FragCoord.y + yOffM ) / texPOTHeight);\n"
    "   float fstagger = 0.0;\n"
    "   if(mod(yp, 2.0) < 0.1) fstagger = 0.5;\n"
    "   float x = (gl_FragCoord.x - xOff) / texPOTWidth;\n"
    "   float y = (gl_FragCoord.y + yOff) / texPOTHeight;\n"
    "   gl_FragColor = texture2D(uTex, vec2(x + fstagger, y));\n"
    "}\n";
#endif

    GLint S52color_tri_fragment_shader;
    GLint S52color_tri_vertex_shader;

    GLint S52texture_2D_fragment_shader;
    GLint S52texture_2D_vertex_shader;

    GLint S52circle_filled_vertex_shader;
    GLint S52circle_filled_fragment_shader;

    GLint S52ring_vertex_shader;
    GLint S52ring_fragment_shader;

    GLint S52Dash_vertex_shader;
    GLint S52Dash_fragment_shader;

    GLint S52AP_vertex_shader;
    GLint S52AP_fragment_shader;

bool loadS52Shaders()
{

    bool ret_val = true;
    GLint success;

    enum Consts {INFOLOG_LEN = 512};
    GLchar infoLog[INFOLOG_LEN];

    // Are the shaders ready?

    // Simple colored triangle shader

    if(!S52color_tri_vertex_shader){
       /* Vertex shader */
       S52color_tri_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
       glShaderSource(S52color_tri_vertex_shader, 1, &S52color_tri_vertex_shader_source, NULL);
       glCompileShader(S52color_tri_vertex_shader);
       glGetShaderiv(S52color_tri_vertex_shader, GL_COMPILE_STATUS, &success);
       if (!success) {
          glGetShaderInfoLog(S52color_tri_vertex_shader, INFOLOG_LEN, NULL, infoLog);
//        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }

    if(!S52color_tri_fragment_shader){
        /* Fragment shader */
        S52color_tri_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(S52color_tri_fragment_shader, 1, &S52color_tri_fragment_shader_source, NULL);
        glCompileShader(S52color_tri_fragment_shader);
        glGetShaderiv(S52color_tri_fragment_shader, GL_COMPILE_STATUS, &success);
      if (!success) {
          glGetShaderInfoLog(S52color_tri_fragment_shader, INFOLOG_LEN, NULL, infoLog);
//        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        ret_val = false;
     }
    }

    if(!S52color_tri_shader_program){
      /* Link shaders */
      S52color_tri_shader_program = glCreateProgram();
      glAttachShader(S52color_tri_shader_program, S52color_tri_fragment_shader);
      glAttachShader(S52color_tri_shader_program, S52color_tri_vertex_shader);
      glLinkProgram(S52color_tri_shader_program);
      glGetProgramiv(S52color_tri_shader_program, GL_LINK_STATUS, &success);
      if (!success) {
          glGetProgramInfoLog(S52color_tri_shader_program, INFOLOG_LEN, NULL, infoLog);
//        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }


        // Simple 2D texture shader

    if(!S52texture_2D_vertex_shader){
       /* Vertex shader */
       S52texture_2D_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
       glShaderSource(S52texture_2D_vertex_shader, 1, &S52texture_2D_vertex_shader_source, NULL);
       glCompileShader(S52texture_2D_vertex_shader);
       glGetShaderiv(S52texture_2D_vertex_shader, GL_COMPILE_STATUS, &success);
      if (!success) {
          glGetShaderInfoLog(S52texture_2D_vertex_shader, INFOLOG_LEN, NULL, infoLog);
//        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }

    if(!S52texture_2D_fragment_shader){
        /* Fragment shader */
        S52texture_2D_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(S52texture_2D_fragment_shader, 1, &S52texture_2D_fragment_shader_source, NULL);
        glCompileShader(S52texture_2D_fragment_shader);
        glGetShaderiv(S52texture_2D_fragment_shader, GL_COMPILE_STATUS, &success);
      if (!success) {
          glGetShaderInfoLog(S52texture_2D_fragment_shader, INFOLOG_LEN, NULL, infoLog);
//        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
}

    if(!S52texture_2D_shader_program){
      /* Link shaders */
      S52texture_2D_shader_program = glCreateProgram();
      glAttachShader(S52texture_2D_shader_program, S52texture_2D_vertex_shader);
      glAttachShader(S52texture_2D_shader_program, S52texture_2D_fragment_shader);
      glLinkProgram(S52texture_2D_shader_program);
      glGetProgramiv(S52texture_2D_shader_program, GL_LINK_STATUS, &success);
      if (!success) {
          glGetProgramInfoLog(S52texture_2D_shader_program, INFOLOG_LEN, NULL, infoLog);
//        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }



    // Circle shader
    if(!S52circle_filled_vertex_shader){
        /* Vertex shader */
        S52circle_filled_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(S52circle_filled_vertex_shader, 1, &S52circle_filled_vertex_shader_source, NULL);
        glCompileShader(S52circle_filled_vertex_shader);
        glGetShaderiv(S52circle_filled_vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(S52circle_filled_vertex_shader, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!S52circle_filled_fragment_shader){
        /* Fragment shader */
        S52circle_filled_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(S52circle_filled_fragment_shader, 1, &S52circle_filled_fragment_shader_source, NULL);
        glCompileShader(S52circle_filled_fragment_shader);
        glGetShaderiv(S52circle_filled_fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(S52circle_filled_fragment_shader, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!S52circle_filled_shader_program){
        /* Link shaders */
        S52circle_filled_shader_program = glCreateProgram();
        glAttachShader(S52circle_filled_shader_program, S52circle_filled_vertex_shader);
        glAttachShader(S52circle_filled_shader_program, S52circle_filled_fragment_shader);
        glLinkProgram(S52circle_filled_shader_program);
        glGetProgramiv(S52circle_filled_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(S52circle_filled_shader_program, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }


    // Ring shader
    if(!S52ring_vertex_shader){
        /* Vertex shader */
        S52ring_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(S52ring_vertex_shader, 1, &S52ring_vertex_shader_source, NULL);
        glCompileShader(S52ring_vertex_shader);
        glGetShaderiv(S52ring_vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(S52ring_vertex_shader, INFOLOG_LEN, NULL, infoLog);
 //           printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!S52ring_fragment_shader){
        /* Fragment shader */
        S52ring_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(S52ring_fragment_shader, 1, &S52ring_fragment_shader_source, NULL);
        glCompileShader(S52ring_fragment_shader);
        glGetShaderiv(S52ring_fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(S52ring_fragment_shader, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!S52ring_shader_program){
        /* Link shaders */
        S52ring_shader_program = glCreateProgram();
        glAttachShader(S52ring_shader_program, S52ring_vertex_shader);
        glAttachShader(S52ring_shader_program, S52ring_fragment_shader);
        glLinkProgram(S52ring_shader_program);
        glGetProgramiv(S52ring_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(S52ring_shader_program, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    // Dash shader
    if(!S52Dash_vertex_shader){
        /* Vertex shader */
        S52Dash_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(S52Dash_vertex_shader, 1, &S52Dash_vertex_shader_source, NULL);
        glCompileShader(S52Dash_vertex_shader);
        glGetShaderiv(S52Dash_vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(S52ring_vertex_shader, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!S52Dash_fragment_shader){
        /* Fragment shader */
        S52Dash_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(S52Dash_fragment_shader, 1, &S52Dash_fragment_shader_source, NULL);
        glCompileShader(S52Dash_fragment_shader);
        glGetShaderiv(S52Dash_fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(S52Dash_fragment_shader, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!S52Dash_shader_program){
        /* Link shaders */
        S52Dash_shader_program = glCreateProgram();
        glAttachShader(S52Dash_shader_program, S52Dash_vertex_shader);
        glAttachShader(S52Dash_shader_program, S52Dash_fragment_shader);
        glLinkProgram(S52Dash_shader_program);
        glGetProgramiv(S52Dash_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(S52Dash_shader_program, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }


    // AP shader
    if(!S52AP_vertex_shader){
        /* Vertex shader */
        S52AP_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(S52AP_vertex_shader, 1, &S52AP_vertex_shader_source, NULL);
        glCompileShader(S52AP_vertex_shader);
        glGetShaderiv(S52AP_vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(S52ring_vertex_shader, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!S52AP_fragment_shader){
        /* Fragment shader */
        S52AP_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(S52AP_fragment_shader, 1, &S52AP_fragment_shader_source, NULL);
        glCompileShader(S52AP_fragment_shader);
        glGetShaderiv(S52AP_fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(S52AP_fragment_shader, INFOLOG_LEN, NULL, infoLog);
  //          printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!S52AP_shader_program){
        /* Link shaders */
        S52AP_shader_program = glCreateProgram();
        glAttachShader(S52AP_shader_program, S52AP_vertex_shader);
        glAttachShader(S52AP_shader_program, S52AP_fragment_shader);
        glLinkProgram(S52AP_shader_program);
        glGetProgramiv(S52AP_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(S52AP_shader_program, INFOLOG_LEN, NULL, infoLog);
//            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    return ret_val;
}


void PrepareS52ShaderUniforms(ViewPort *vp)
{
    mat4x4 m;
    float    vp_transform[16];
    mat4x4_identity(m);
    mat4x4_scale_aniso((float (*)[4])vp_transform, m, 2.0 / (float)vp->pix_width, -2.0 / (float)vp->pix_height, 1.0);
    //Rotate
    mat4x4 Q;
    mat4x4_rotate_Z(Q, (float (*)[4])vp_transform, vp->rotation);
    mat4x4_translate_in_place(Q, -vp->pix_width / 2.0, -vp->pix_height / 2.0, 0);
    

    mat4x4 I;
    mat4x4_identity(I);
    
    glUseProgram(S52color_tri_shader_program);
    GLint matloc = glGetUniformLocation(S52color_tri_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);
    GLint transloc = glGetUniformLocation(S52color_tri_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I);

    glUseProgram(S52texture_2D_shader_program);
    matloc = glGetUniformLocation(S52texture_2D_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);
    transloc = glGetUniformLocation(S52texture_2D_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I);

    glUseProgram(S52circle_filled_shader_program);
    matloc = glGetUniformLocation(S52circle_filled_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);
    transloc = glGetUniformLocation(S52circle_filled_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I);

    glUseProgram(S52ring_shader_program);
    matloc = glGetUniformLocation(S52ring_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);
    transloc = glGetUniformLocation(S52ring_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I);

    glUseProgram(S52Dash_shader_program);
    matloc = glGetUniformLocation(S52Dash_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);
    transloc = glGetUniformLocation(S52Dash_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I);

    glUseProgram(S52AP_shader_program);
    matloc = glGetUniformLocation(S52AP_shader_program,"MVMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q);
    transloc = glGetUniformLocation(S52AP_shader_program,"TransformMatrix");
    glUniformMatrix4fv( transloc, 1, GL_FALSE, (const GLfloat*)I);
}

#endif
