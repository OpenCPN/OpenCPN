/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  RAZ Symbology Parser
 * Author:   David Register
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/tokenzr.h"

#include <stdio.h>
#include <math.h>

#include "s52plib.h"
#include "razdsparser.h"
#include "chartsymbols.h"

RazdsParser::RazdsParser()
{
    ColorTableArray = ChartSymbols::GetColorTables();
    pBuf = buffer;
}

RazdsParser::~RazdsParser()
{
}

// MAX_BUF == 1024 --for buffer overflow
#define ENDLN   "%1024[^\037]"
#define NEWLN  "%1024[^\n]"

int RazdsParser::ReadS52Line( char *pBuffer, const char *delim, int nCount, FILE *fp )
{
    int ret;

    ret = fscanf( fp, delim, pBuffer );

    fgetc( fp );

    if( nCount ) // skip \n
    fgetc( fp );

    return ret;
}

int RazdsParser::ChopS52Line( char *pBuffer, char c )
{
    int i;

    for( i = 0; pBuffer[i] != '\0'; ++i )
        if( pBuffer[i] == '\037' ) pBuffer[i] = c;

    return i;
}

int RazdsParser::ParsePos( position *pos, char *buf, bool patt )
{
    if( patt ) {
        sscanf( buf, "%5d%5d", &pos->minDist.PAMI, &pos->maxDist.PAMA );
        buf += 10;
    }

    sscanf( buf, "%5d%5d%5d%5d%5d%5d", &pos->pivot_x.PACL, &pos->pivot_y.PARW, &pos->bnbox_w.PAHL,
            &pos->bnbox_h.PAVL, &pos->bnbox_x.PBXC, &pos->bnbox_y.PBXR );
    return 1;
}

#define MOD_REC(str)    if(0==strncmp(#str,pBuf,4))

int RazdsParser::ParseLBID( FILE *fp )
{

    wxString s( pBuf, wxConvUTF8 );
    wxStringTokenizer tkz( s, _T ( '\037' ) );

    wxString token = tkz.GetNextToken(); // something like "113LI00001REVIHO"
    token = tkz.GetNextToken(); // ESID
    token = tkz.GetNextToken();

    //    Get PLIB version number
    double version;
    if( token.ToDouble( &version ) ) {
        plib->m_VersionMajor = ( (int) ( version * 10 ) ) / 10;
        plib->m_VersionMinor = (int) floor( ( ( version - plib->m_VersionMajor ) * 10 ) + 0.5 );
    } else {
        plib->m_VersionMajor = 0;
        plib->m_VersionMinor = 0;
    }

    return 1;
}

int RazdsParser::ParseCOLS( FILE *fp )
{
    int ret;
    colTable *ct = new colTable;

    // get color table name
    ChopS52Line( pBuf, '\0' );

    ct->tableName = new wxString( pBuf + 19, wxConvUTF8 );

    ColorTableArray->Add( (void *) ct );

    // read color
    ret = ReadS52Line( pBuf, NEWLN, 0, fp );
    while( 0 != strncmp( pBuf, "****", 4 ) ) {
        double x, y, L;

        S52color c;

        ChopS52Line( pBuf, ' ' );
        strncpy( c.colName, pBuf + 9, 5 );
        c.colName[5] = 0;

        sscanf( pBuf + 14, "%lf %lf %lf", &x, &y, &L );

        _CIE2RGB( &c, x, y, L );

        wxString colorName( c.colName, wxConvUTF8 );
        ct->colors[colorName] = c;
        wxColour wxcolor( c.R, c.G, c.B );
        ct->wxColors[colorName] = wxcolor;

        ret = ReadS52Line( pBuf, NEWLN, 0, fp );
    }
    return ret;
}

int RazdsParser::ParseLUPT( FILE *fp )
{
    int ret;

    bool inserted = FALSE;

    LUPrec *LUP = (LUPrec*) calloc( 1, sizeof(LUPrec) );
    plib->pAlloc->Add( LUP );

    LUP->nSequence = m_LUPSequenceNumber++;

    LUP->DISC = (enum _DisCat) OTHER; // as a default

    sscanf( pBuf + 11, "%d", &LUP->RCID );
    strncpy( LUP->OBCL, pBuf + 19, 6 );

    LUP->FTYP = (enum _Object_t) pBuf[25];
    LUP->DPRI = (enum _DisPrio) pBuf[30];
    LUP->RPRI = (enum _RadPrio) pBuf[31];
    LUP->TNAM = (enum _LUPname) pBuf[36];

    ret = ReadS52Line( pBuf, NEWLN, 0, fp );

    do {
        MOD_REC ( ATTC ) {
            if( '\037' != pBuf[9] ) // could be empty!
                    {

                wxArrayString *pAS = new wxArrayString();
                char *p = &pBuf[9];

                wxString *st1 = new wxString;

                while( ( *p != '\r' ) && ( *p ) ) {
                    while( *p != 0x1f ) {
                        st1->Append( *p );
                        p++;
                    }

                    pAS->Add( *st1 );
                    st1->Clear();
                    p++;
                }

                delete st1;

                LUP->ATTCArray = pAS;

                ChopS52Line( pBuf, ' ' );
            }
        }

        MOD_REC ( INST ) LUP->INST = new wxString( pBuf + 9, wxConvUTF8 );
        MOD_REC ( DISC ) LUP->DISC = (enum _DisCat) pBuf[9];
        MOD_REC ( LUCM ) sscanf( pBuf + 9, "%d", &LUP->LUCM );

        MOD_REC ( **** ) {

            // Add LUP to array
            wxArrayOfLUPrec *pLUPARRAYtyped = plib->SelectLUPARRAY( LUP->TNAM );

            // Search the LUPArray to see if there is already a LUP with this RCID
            // If found, replace it with the new LUP
            // This provides a facility for updating the LUP tables after loading a basic set

            unsigned int index = 0;

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

            inserted = TRUE;

        } // MOD_REC

        ret = ReadS52Line( pBuf, NEWLN, 0, fp );

    } while( inserted == FALSE );

    return 1;
}

int RazdsParser::ParseLNST( FILE *fp )
{
    int ret;

    char strk[20];

    bool inserted = FALSE;
    Rule *lnstmp = NULL;
    Rule *lnst = (Rule*) calloc( 1, sizeof(Rule) );
    plib->pAlloc->Add( lnst );

    lnst->exposition.LXPO = new wxString;
    wxString LVCT;
    wxString LCRF;

    sscanf( pBuf + 11, "%d", &lnst->RCID );

    ret = ReadS52Line( pBuf, NEWLN, 0, fp );
    do {
        MOD_REC ( LIND ) {
            strncpy( lnst->name.LINM, pBuf + 9, 8 ); // could be empty!
            ParsePos( &lnst->pos.line, pBuf + 17, FALSE );
        }

        MOD_REC ( LXPO ) lnst->exposition.LXPO->Append( wxString( pBuf + 9, wxConvUTF8 ) );
        MOD_REC ( LCRF ) LCRF.Append( wxString( pBuf + 9, wxConvUTF8 ) ); // CIDX + CTOK
        MOD_REC ( LVCT ) LVCT.Append( wxString( pBuf + 9, wxConvUTF8 ) );
        MOD_REC ( **** ) {

            lnst->vector.LVCT = (char *) calloc( LVCT.Len() + 1, 1 );
            strncpy( lnst->vector.LVCT, LVCT.mb_str(), LVCT.Len() );

            lnst->colRef.LCRF = (char *) calloc( LCRF.Len() + 1, 1 );
            strncpy( lnst->colRef.LCRF, LCRF.mb_str(), LCRF.Len() );

            // check if key already there
            strncpy( strk, lnst->name.LINM, 8 );
            strk[8] = 0;
            wxString key( strk, wxConvUTF8 );

            //wxString key((lnst->name.LINM), 8);
            lnstmp = ( *plib->_line_sym )[key];

            // insert in Hash Table
            if( NULL == lnstmp ) ( *plib->_line_sym )[key] = lnst;
            else
                if( lnst->name.LINM != lnstmp->name.LINM ) ( *plib->_line_sym )[key] = lnst;
                else
                    assert( 0 );
            // key must be unique --should not reach this

            inserted = TRUE;
        }
        ret = ReadS52Line( pBuf, NEWLN, 0, fp );
        ChopS52Line( pBuf, '\0' );
    } while( inserted == FALSE );

    return ret;
}

int RazdsParser::ParsePATT( FILE *fp )
{
    int ret;

    int bitmap_width;
    char pbm_line[200]; // max bitmap width...
    char strk[20];

    bool inserted = FALSE;
    Rule *pattmp = NULL;
    Rule *patt = (Rule*) calloc( 1, sizeof(Rule) );
    plib->pAlloc->Add( patt );

    patt->exposition.PXPO = new wxString;
    patt->bitmap.PBTM = new wxString;
    wxString PVCT;
    wxString PCRF;

    sscanf( pBuf + 11, "%d", &patt->RCID );

    ret = ReadS52Line( pBuf, NEWLN, 0, fp );

    do {
        MOD_REC ( PATD ) {
            strncpy( patt->name.PANM, pBuf + 9, 8 );
            patt->definition.PADF = pBuf[17];
            patt->fillType.PATP = pBuf[18]; // first character 'S' or 'L', for staggered or linear
            patt->spacing.PASP = pBuf[21];
            ParsePos( &patt->pos.patt, pBuf + 24, TRUE );
        }

        MOD_REC ( PXPO ) patt->exposition.PXPO->Append( wxString( pBuf + 9, wxConvUTF8 ) );
        MOD_REC ( PCRF ) PCRF.Append( wxString( pBuf + 9, wxConvUTF8 ) ); // CIDX+CTOK
        MOD_REC ( PVCT ) PVCT.Append( wxString( pBuf + 9, wxConvUTF8 ) );

        MOD_REC ( PBTM ) {
            bitmap_width = patt->pos.patt.bnbox_w.SYHL;
            strncpy( pbm_line, pBuf + 9, bitmap_width );
            pbm_line[bitmap_width] = 0;
            patt->bitmap.SBTM->Append( wxString( pbm_line, wxConvUTF8 ) );
        }

        MOD_REC ( **** ) {

            patt->vector.PVCT = (char *) calloc( PVCT.Len() + 1, 1 );
            strncpy( patt->vector.PVCT, PVCT.mb_str(), PVCT.Len() );

            patt->colRef.PCRF = (char *) calloc( PCRF.Len() + 1, 1 );
            strncpy( patt->colRef.PCRF, PCRF.mb_str(), PCRF.Len() );

            // check if key already there
            strncpy( strk, patt->name.PANM, 8 );
            strk[8] = 0;
            wxString key( strk, wxConvUTF8 );

            pattmp = ( *plib->_patt_sym )[key];

            if( NULL == pattmp ) // not there, so....
            ( *plib->_patt_sym )[key] = patt; // insert in hash table

            else // already something here with same key...
            {
                if( patt->name.PANM != pattmp->name.PANM ) // if the pattern names are not identical
                        {
                    ( *plib->_patt_sym )[key] = patt; // replace the pattern
                    plib->DestroyPatternRuleNode( pattmp ); // remember to free to replaced node
                    // the node itself is destroyed as part of pAlloc
                }

            }

            inserted = TRUE;
        }
        ret = ReadS52Line( pBuf, NEWLN, 0, fp );
        ChopS52Line( pBuf, '\0' );

    } while( inserted == FALSE );

    return ret;
}

int RazdsParser::ParseSYMB( FILE *fp, RuleHash *pHash )
{
    int ret;

    int bitmap_width;
    char pbm_line[200]; // max bitmap width...
    bool inserted = FALSE;
    Rule *symb = (Rule*) calloc( 1, sizeof(Rule) );
    plib->pAlloc->Add( symb );
    Rule *symbtmp = NULL;

    symb->exposition.SXPO = new wxString;
    symb->bitmap.SBTM = new wxString;
    wxString SVCT;
    wxString SCRF;

    sscanf( pBuf + 11, "%d", &symb->RCID );

    ret = ReadS52Line( pBuf, NEWLN, 0, fp );

    do {
        MOD_REC ( SYMD ) {
            strncpy( symb->name.SYNM, pBuf + 9, 8 );
            symb->definition.SYDF = pBuf[17];
            ParsePos( &symb->pos.symb, pBuf + 18, FALSE );
        }

        MOD_REC ( SXPO ) symb->exposition.SXPO->Append( wxString( pBuf + 9, wxConvUTF8 ) );

        MOD_REC ( SBTM ) {
            bitmap_width = symb->pos.symb.bnbox_w.SYHL;
            if( bitmap_width > 200 ) wxLogMessage( _T ( "ParseSymb....bitmap too wide." ) );
            strncpy( pbm_line, pBuf + 9, bitmap_width );
            pbm_line[bitmap_width] = 0;
            symb->bitmap.SBTM->Append( wxString( pbm_line, wxConvUTF8 ) );
        }

        MOD_REC ( SCRF ) SCRF.Append( wxString( pBuf + 9, wxConvUTF8 ) ); // CIDX+CTOK

        MOD_REC ( SVCT ) SVCT.Append( wxString( pBuf + 9, wxConvUTF8 ) );

        if( ( 0 == strncmp( "****", pBuf, 4 ) ) || ( ret == -1 ) ) {
            symb->vector.SVCT = (char *) calloc( SVCT.Len() + 1, 1 );
            strncpy( symb->vector.SVCT, SVCT.mb_str(), SVCT.Len() );

            symb->colRef.SCRF = (char *) calloc( SCRF.Len() + 1, 1 );
            strncpy( symb->colRef.SCRF, SCRF.mb_str(), SCRF.Len() );

            // Create a key
            char keyt[20];
            strncpy( keyt, symb->name.SYNM, 8 );
            keyt[8] = 0;
            wxString key( keyt, wxConvUTF8 );

            symbtmp = ( *pHash )[key];

            if( NULL == symbtmp ) // not there, so....
            ( *pHash )[key] = symb; // insert in hash table

            else // already something here with same key...
            {
                if( symb->name.SYNM != symbtmp->name.SYNM ) // if the pattern names are not identical
                        {
                    ( *pHash )[key] = symb; // replace the pattern
                    plib->DestroyRuleNode( symbtmp ); // remember to free to replaced node
                    // the node itself is destroyed as part of pAlloc
                }
            }
            inserted = TRUE;
        }
        ret = ReadS52Line( pBuf, NEWLN, 0, fp );
        ChopS52Line( pBuf, '\0' );

    } while( inserted == FALSE );

    return ret;
}

int RazdsParser::LoadFile( s52plib* plibArg, const wxString& PLib )
{

    plib = plibArg;

    FILE *fp = NULL;
    int nRead;

    fp = fopen( PLib.mb_str(), "r" );

    if( fp == NULL ) {
        wxString msg;
        msg = _T("   S52PLIB: Cannot open S52 rules file: ");
        msg += PLib;
        wxLogMessage( msg );
        return 0;
    }

    m_LUPSequenceNumber = 0;

    while( 1 == ( nRead = ReadS52Line( pBuf, NEWLN, 0, fp ) ) ) {
        // !!! order important !!!
        MOD_REC ( LBID ) ParseLBID( fp );
        MOD_REC ( COLS ) ParseCOLS( fp );
        MOD_REC ( LUPT ) ParseLUPT( fp );
        MOD_REC ( LNST ) ParseLNST( fp );
        MOD_REC ( PATT ) ParsePATT( fp );
        MOD_REC ( SYMB ) ParseSYMB( fp, plib->_symb_sym );

        MOD_REC ( 0001 ) continue;
        MOD_REC ( **** ) continue;

    }
    fclose( fp );
    return 1;
}

/*  CIE->RGB color transformation matrix courtesy of
 The BREP Library.
 Copyright (C) 1996 Philippe Bekaert
 */

#define  CIE_x_r                0.640            // nominal CRT primaries
#define  CIE_y_r                0.330
#define  CIE_x_g                0.290
#define  CIE_y_g                0.600
#define  CIE_x_b                0.150
#define  CIE_y_b                0.060
#define  CIE_x_w                0.295 //0.3333333333          // monitor white point
#define  CIE_y_w                0.315// 0.3333333333
#define CIE_D           (       CIE_x_r*(CIE_y_g - CIE_y_b) + \
            CIE_x_g*(CIE_y_b - CIE_y_r) + \
            CIE_x_b*(CIE_y_r - CIE_y_g)     )
#define CIE_C_rD        ( (1./CIE_y_w) * \
            ( CIE_x_w*(CIE_y_g - CIE_y_b) - \
            CIE_y_w*(CIE_x_g - CIE_x_b) + \
            CIE_x_g*CIE_y_b - CIE_x_b*CIE_y_g     ) )
#define CIE_C_gD        ( (1./CIE_y_w) * \
            ( CIE_x_w*(CIE_y_b - CIE_y_r) - \
            CIE_y_w*(CIE_x_b - CIE_x_r) - \
            CIE_x_r*CIE_y_b + CIE_x_b*CIE_y_r     ) )
#define CIE_C_bD        ( (1./CIE_y_w) * \
            ( CIE_x_w*(CIE_y_r - CIE_y_g) - \
            CIE_y_w*(CIE_x_r - CIE_x_g) + \
            CIE_x_r*CIE_y_g - CIE_x_g*CIE_y_r     ) )

#define CIE_rf          (CIE_y_r*CIE_C_rD/CIE_D)
#define CIE_gf          (CIE_y_g*CIE_C_gD/CIE_D)
#define CIE_bf          (CIE_y_b*CIE_C_bD/CIE_D)

static double tmat[3][3] =       //XYZ to RGB
        { { ( CIE_y_g - CIE_y_b - CIE_x_b * CIE_y_g + CIE_y_b * CIE_x_g ) / CIE_C_rD, ( CIE_x_b
                - CIE_x_g - CIE_x_b * CIE_y_g + CIE_x_g * CIE_y_b ) / CIE_C_rD, ( CIE_x_g * CIE_y_b
                - CIE_x_b * CIE_y_g ) / CIE_C_rD }, { ( CIE_y_b - CIE_y_r - CIE_y_b * CIE_x_r
                + CIE_y_r * CIE_x_b ) / CIE_C_gD, ( CIE_x_r - CIE_x_b - CIE_x_r * CIE_y_b
                + CIE_x_b * CIE_y_r ) / CIE_C_gD, ( CIE_x_b * CIE_y_r - CIE_x_r * CIE_y_b )
                / CIE_C_gD }, { ( CIE_y_r - CIE_y_g - CIE_y_r * CIE_x_g + CIE_y_g * CIE_x_r )
                / CIE_C_bD,
                ( CIE_x_g - CIE_x_r - CIE_x_g * CIE_y_r + CIE_x_r * CIE_y_g ) / CIE_C_bD, ( CIE_x_r
                        * CIE_y_g - CIE_x_g * CIE_y_r ) / CIE_C_bD } };

static double c_gamma = 2.20;

int RazdsParser::_CIE2RGB( S52color* toRGB, double x, double y, double L )
{
    int R, G, B;
    double dR, dG, dB;
    double X, Y, Z;

    //    Transform CIE xyL into CIE XYZ

    if( y != 0 ) {
        X = ( x * L ) / y;
        Y = L;
        Z = ( ( ( 1.0 - x ) - y ) * L ) / y;
    } else {
        X = 0;
        Y = 0;
        Z = 0;
    }

    //    Transform CIE XYZ into RGB

    dR = ( X * tmat[0][0] ) + ( Y * tmat[0][1] ) + ( Z * tmat[0][2] );
    dG = ( X * tmat[1][0] ) + ( Y * tmat[1][1] ) + ( Z * tmat[1][2] );
    dB = ( X * tmat[2][0] ) + ( Y * tmat[2][1] ) + ( Z * tmat[2][2] );

    //       Arbitrarily clip the luminance values to 100
    if( dR > 100 ) dR = 100;
    if( dG > 100 ) dB = 100;
    if( dB > 100 ) dB = 100;

    //       And scale
    dR /= 100;
    dG /= 100;
    dB /= 100;

    dR = pow( dR, 1.0 / c_gamma );
    dG = pow( dG, 1.0 / c_gamma );
    dB = pow( dB, 1.0 / c_gamma );

    R = (int) ( dR * 255 );
    G = (int) ( dG * 255 );
    B = (int) ( dB * 255 );

    // A special case:
    // MSW has trouble blitting with a mask if src color is 0,0,0 ????
    if( ( R == 0 ) && ( G == 0 ) && ( B == 0 ) ) {
        R = (unsigned char) 7;
        G = (unsigned char) 7;
        B = (unsigned char) 7;
    }

    toRGB->R = (unsigned char) R;
    toRGB->G = (unsigned char) G;
    toRGB->B = (unsigned char) B;

    return true;
}

