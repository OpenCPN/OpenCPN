/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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


// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "wx/image.h"                           // for some reason, needed for msvc???
#include "wx/tokenzr.h"
#include <wx/textfile.h>

#include "dychart.h"

#include "s52s57.h"
#include "s52plib.h"

#include "s57chart.h"

#include "mygeom.h"
#include "cutil.h"
#include "georef.h"
#include "navutil.h"                            // for LogMessageOnce
#include "ocpn_pixel.h"
#include "ocpndc.h"

#include "cpl_csv.h"
#include "setjmp.h"

#include "mygdal/ogr_s57.h"

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

#include <algorithm>          // for std::sort

extern bool GetDoubleAttr(S57Obj *obj, const char *AttrName, double &val);      // found in s52cnsy


void OpenCPN_OGRErrorHandler( CPLErr eErrClass, int nError,
                              const char * pszErrorMsg );               // installed GDAL OGR library error handler

const char *MyCSVGetField( const char * pszFilename,
                         const char * pszKeyFieldName,
                         const char * pszKeyFieldValue,
                         CSVCompareCriteria eCriteria,
                         const char * pszTargetField ) ;


extern s52plib           *ps52plib;
extern S57ClassRegistrar *g_poRegistrar;
extern wxString          g_csv_locn;
extern wxString          g_SENCPrefix;
extern FILE              *s_fpdebug;
extern bool              g_bGDAL_Debug;
extern bool              g_bDebugS57;
extern bool              g_b_useStencil;
extern ChartCanvas       *cc1;
extern ChartBase         *Current_Ch;
extern MyFrame*          gFrame;

extern wxProgressDialog *s_ProgDialog;

static jmp_buf env_ogrf;                    // the context saved by setjmp();

#include <wx/arrimpl.cpp>                   // Implement an array of S57 Objects
WX_DEFINE_OBJARRAY(ArrayOfS57Obj);

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ListOfS57Obj);                // Implement a list of S57 Objects

WX_DEFINE_LIST(ListOfObjRazRules);   // Implement a list ofObjRazRules

//    Arrays to temporarily hold SENC geometry
WX_DEFINE_OBJARRAY(ArrayOfVE_Elements);
WX_DEFINE_OBJARRAY(ArrayOfVC_Elements);

#define S57_THUMB_SIZE  200

static int              s_bInS57;         // Exclusion flag to prvent recursion in this class init call.
                                          // Init() is not reentrant due to static wxProgressDialog callback....
int s_cnt;

static bool s_ProgressCallBack( void )
{
    bool ret = true;
    s_cnt++;
    if( ( s_cnt % 100 ) == 0 ) {
        if( s_ProgDialog ) ret = s_ProgDialog->Pulse();         // return false if cancel is pressed
    }
    return ret;
}

//----------------------------------------------------------------------------------
//      S57Obj CTOR
//----------------------------------------------------------------------------------

S57Obj::S57Obj()
{
    attList = NULL;
    attVal = NULL;

    pPolyTessGeo = NULL;
    pPolyTrapGeo = NULL;

    bCS_Added = 0;
    CSrules = NULL;
    FText = NULL;
    bFText_Added = 0;
    geoPtMulti = NULL;
    geoPtz = NULL;
    geoPt = NULL;
    bIsClone = false;
    Scamin = 10000000;                              // ten million enough?
    nRef = 0;

    bIsAton = false;
    bIsAssociable = false;
    m_n_lsindex = 0;
    m_lsindex_array = NULL;
    m_n_edge_max_points = 0;

    bBBObj_valid = false;

    //        Set default (unity) auxiliary transform coefficients
    x_rate = 1.0;
    y_rate = 1.0;
    x_origin = 0.0;
    y_origin = 0.0;
}

//----------------------------------------------------------------------------------
//      S57Obj DTOR
//----------------------------------------------------------------------------------

S57Obj::~S57Obj()
{
    //  Don't delete any allocated records of simple copy clones
    if( !bIsClone ) {
        if( attVal ) {
            for( unsigned int iv = 0; iv < attVal->GetCount(); iv++ ) {
                S57attVal *vv = attVal->Item( iv );
                void *v2 = vv->value;
                free( v2 );
                delete vv;
            }
            delete attVal;
        }
        if( attList ) delete attList;

        if( pPolyTessGeo ) delete pPolyTessGeo;

        if( pPolyTrapGeo ) delete pPolyTrapGeo;

        if( FText ) delete FText;

        if( geoPt ) free( geoPt );
        if( geoPtz ) free( geoPtz );
        if( geoPtMulti ) free( geoPtMulti );

        if( m_lsindex_array ) free( m_lsindex_array );
    }
}

//----------------------------------------------------------------------------------
//      S57Obj CTOR from SENC file
//----------------------------------------------------------------------------------

S57Obj::S57Obj( char *first_line, wxInputStream *pfpx, double dummy, double dummy2 )
{
    attList = NULL;
    attVal = NULL;
    pPolyTessGeo = NULL;
    pPolyTrapGeo = NULL;
    bCS_Added = 0;
    CSrules = NULL;
    FText = NULL;
    bFText_Added = 0;
    bIsClone = false;

    geoPtMulti = NULL;
    geoPtz = NULL;
    geoPt = NULL;
    Scamin = 10000000;                              // ten million enough?
    nRef = 0;
    bIsAton = false;
    m_n_lsindex = 0;
    m_lsindex_array = NULL;

    //        Set default (unity) auxiliary transform coefficients
    x_rate = 1.0;
    y_rate = 1.0;
    x_origin = 0.0;
    y_origin = 0.0;

    if( strlen( first_line ) == 0 ) return;

    int FEIndex;

    int MAX_LINE = 499999;
    char *buf = (char *) malloc( MAX_LINE + 1 );
    int llmax = 0;

    char szFeatureName[20];

    char *br;
    char szAtt[20];
    char geoMatch[20];

    bool bMulti = false;

    char *hdr_buf = (char *) malloc( 1 );

    strcpy( buf, first_line );

//    while(!dun)
    {

        if( !strncmp( buf, "OGRF", 4 ) ) {
            attList = new wxString();
            attVal = new wxArrayOfS57attVal();

            FEIndex = atoi( buf + 19 );

            strncpy( szFeatureName, buf + 11, 6 );
            szFeatureName[6] = 0;
            strcpy( FeatureName, szFeatureName );

            //      Build/Maintain a list of found OBJL types for later use
            //      And back-reference the appropriate list index in S57Obj for Display Filtering

            iOBJL = -1; // deferred, done by OBJL filtering in the PLIB as needed

            //      Walk thru the attributes, adding interesting ones
            int hdr_len = 0;
            char *mybuf_ptr = NULL;
            char *hdr_end = NULL;

            int prim = -1;
            int attdun = 0;

            strcpy( geoMatch, "Dummy" );

            while( !attdun ) {
                if( hdr_len ) {
                    int nrl = my_bufgetl( mybuf_ptr, hdr_end, buf, MAX_LINE );
                    mybuf_ptr += nrl;
                    if( 0 == nrl ) {
                        attdun = 1;
                        my_fgets( buf, MAX_LINE, *pfpx );     // this will be PolyGeo
                        break;
                    }
                }

                else
                    my_fgets( buf, MAX_LINE, *pfpx );

                if( !strncmp( buf, "HDRLEN", 6 ) ) {
                    hdr_len = atoi( buf + 7 );
                    hdr_buf = (char *) realloc( hdr_buf, hdr_len );
                    pfpx->Read( hdr_buf, hdr_len );
                    mybuf_ptr = hdr_buf;
                    hdr_end = hdr_buf + hdr_len;
                }

                else if( !strncmp( buf, geoMatch, 6 ) ) {
                    attdun = 1;
                    break;
                }

                else if( !strncmp( buf, "  MULT", 6 ) )         // Special multipoint
                        {
                    bMulti = true;
                    attdun = 1;
                    break;
                }

                else if( !strncmp( buf, "  PRIM", 6 ) ) {
                    prim = atoi( buf + 13 );
                    switch( prim ){
                        case 1: {
                            strcpy( geoMatch, "  POIN" );
                            break;
                        }

                        case 2:                            // linestring
                        {
                            strcpy( geoMatch, "  LINE" );
                            break;
                        }

                        case 3:                            // area as polygon
                        {
                            strcpy( geoMatch, "  POLY" );
                            break;
                        }

                        default:                            // unrecognized
                        {
                            break;
                        }

                    }       //switch
                }               // if PRIM

                bool iua = IsUsefulAttribute( buf );

                szAtt[0] = 0;

                if( iua ) {
                    S57attVal *pattValTmp = new S57attVal;

                    if( buf[10] == 'I' ) {
                        br = buf + 2;
                        int i = 0;
                        while( *br != ' ' ) {
                            szAtt[i++] = *br;
                            br++;
                        }

                        szAtt[i] = 0;

                        while( *br != '=' )
                            br++;

                        br += 2;

                        int AValInt = atoi( br );
                        int *pAVI = (int *) malloc( sizeof(int) );         //new int;
                        *pAVI = AValInt;
                        pattValTmp->valType = OGR_INT;
                        pattValTmp->value = pAVI;

                        //      Capture SCAMIN on the fly during load
                        if( !strcmp( szAtt, "SCAMIN" ) ) Scamin = AValInt;
                    }

                    else if( buf[10] == 'S' ) {
                        strncpy( szAtt, &buf[2], 6 );
                        szAtt[6] = 0;

                        br = buf + 15;

                        int nlen = strlen( br );
                        br[nlen - 1] = 0;                                 // dump the NL char
                        char *pAVS = (char *) malloc( nlen + 1 );
                        ;
                        strcpy( pAVS, br );

                        pattValTmp->valType = OGR_STR;
                        pattValTmp->value = pAVS;
                    }

                    else if( buf[10] == 'R' ) {
                        br = buf + 2;
                        int i = 0;
                        while( *br != ' ' ) {
                            szAtt[i++] = *br;
                            br++;
                        }

                        szAtt[i] = 0;

                        while( *br != '=' )
                            br++;

                        br += 2;

                        float AValfReal;
                        sscanf( br, "%f", &AValfReal );

                        double AValReal = AValfReal;        //FIXME this cast leaves trash in double

                        double *pAVR = (double *) malloc( sizeof(double) );   //new double;
                        *pAVR = AValReal;

                        pattValTmp->valType = OGR_REAL;
                        pattValTmp->value = pAVR;
                    }

                    else {
                        // unknown attribute type
                        //                        CPLError((CPLErr)0, 0,"Unknown Attribute Type %s", buf);
                    }

                    if( strlen( szAtt ) ) {
                        attList->Append( wxString( szAtt, wxConvUTF8 ) );
                        attList->Append( '\037' );

                        attVal->Add( pattValTmp );
                    } else
                        delete pattValTmp;

                }        //useful
            }               // while attdun

            //              Develop Geometry

            switch( prim ){
                case 1: {
                    if( !bMulti ) {
                        Primitive_type = GEO_POINT;

                        char tbuf[40];
                        float point_ref_lat, point_ref_lon;
                        sscanf( buf, "%s %f %f", tbuf, &point_ref_lat, &point_ref_lon );

                        my_fgets( buf, MAX_LINE, *pfpx );
                        int wkb_len = atoi( buf + 2 );
                        pfpx->Read( buf, wkb_len );

                        npt = 1;
                        float *pfs = (float *) ( buf + 5 );                // point to the point

                        float easting, northing;
                        easting = *pfs++;
                        northing = *pfs;

                        x = easting;                                    // and save as SM
                        y = northing;

                        //  Convert from SM to lat/lon for bbox
                        double xll, yll;
                        fromSM( easting, northing, point_ref_lat, point_ref_lon, &yll, &xll );

                        m_lon = xll;
                        m_lat = yll;
                        BBObj.SetMin( m_lon - .25, m_lat - .25 );
                        BBObj.SetMax( m_lon + .25, m_lat + .25 );

                    } else {
                        Primitive_type = GEO_POINT;

                        char tbuf[40];
                        float point_ref_lat, point_ref_lon;
                        sscanf( buf, "%s %f %f", tbuf, &point_ref_lat, &point_ref_lon );

                        my_fgets( buf, MAX_LINE, *pfpx );
                        int wkb_len = atoi( buf + 2 );
                        pfpx->Read( buf, wkb_len );

                        npt = *( (int *) ( buf + 5 ) );

                        geoPtz = (double *) malloc( npt * 3 * sizeof(double) );
                        geoPtMulti = (double *) malloc( npt * 2 * sizeof(double) );

                        double *pdd = geoPtz;
                        double *pdl = geoPtMulti;

                        float *pfs = (float *) ( buf + 9 );                 // start of data
                        for( int ip = 0; ip < npt; ip++ ) {
                            float easting, northing;
                            easting = *pfs++;
                            northing = *pfs++;
                            float depth = *pfs++;

                            *pdd++ = easting;
                            *pdd++ = northing;
                            *pdd++ = depth;

                            //  Convert point from SM to lat/lon for later use in decomposed bboxes
                            double xll, yll;
                            fromSM( easting, northing, point_ref_lat, point_ref_lon, &yll, &xll );

                            *pdl++ = xll;
                            *pdl++ = yll;
                        }

                        // Capture bbox limits recorded in SENC record as lon/lat
                        float xmax = *pfs++;
                        float xmin = *pfs++;
                        float ymax = *pfs++;
                        float ymin = *pfs;

                        BBObj.SetMin( xmin, ymin );
                        BBObj.SetMax( xmax, ymax );

                    }
                    break;
                }

                case 2:                                                // linestring
                {
                    Primitive_type = GEO_LINE;

                    if( !strncmp( buf, "  LINESTRING", 12 ) ) {

                        char tbuf[40];
                        float line_ref_lat, line_ref_lon;
                        sscanf( buf, "%s %f %f", tbuf, &line_ref_lat, &line_ref_lon );

                        my_fgets( buf, MAX_LINE, *pfpx );
                        int sb_len = atoi( buf + 2 );

                        unsigned char *buft = (unsigned char *) malloc( sb_len );
                        pfpx->Read( buft, sb_len );

                        npt = *( (int *) ( buft + 5 ) );

                        geoPt = (pt*) malloc( ( npt ) * sizeof(pt) );
                        pt *ppt = geoPt;
                        float *pf = (float *) ( buft + 9 );

                        // Capture SM points
                        for( int ip = 0; ip < npt; ip++ ) {
                            ppt->x = *pf++;
                            ppt->y = *pf++;
                            ppt++;
                        }

                        // Capture bbox limits recorded as lon/lat
                        float xmax = *pf++;
                        float xmin = *pf++;
                        float ymax = *pf++;
                        float ymin = *pf;

                        free( buft );

                        // set s57obj bbox as lat/lon
                        BBObj.SetMin( xmin, ymin );
                        BBObj.SetMax( xmax, ymax );
                        bBBObj_valid = true;

                        //  and declare x/y of the object to be average east/north of all points
                        double e1, e2, n1, n2;
                        toSM( ymax, xmax, line_ref_lat, line_ref_lon, &e1, &n1 );
                        toSM( ymin, xmin, line_ref_lat, line_ref_lon, &e2, &n2 );

                        x = ( e1 + e2 ) / 2.;
                        y = ( n1 + n2 ) / 2.;

                        //  Set the object base point
                        double xll, yll;
                        fromSM( x, y, line_ref_lat, line_ref_lon, &yll, &xll );
                        m_lon = xll;
                        m_lat = yll;

                        //  Capture the edge and connected node table indices
                        my_fgets( buf, MAX_LINE, *pfpx );     // this will be "\n"
                        my_fgets( buf, MAX_LINE, *pfpx );     // this will be "LSINDEXLIST nnn"

//                          char test[100];
//                          strncpy(test, buf, 98);
//                          strcat(test, "\n");
//                          printf("%s", test);

                        sscanf( buf, "%s %d ", tbuf, &m_n_lsindex );

                        m_lsindex_array = (int *) malloc( 3 * m_n_lsindex * sizeof(int) );
                        pfpx->Read( m_lsindex_array, 3 * m_n_lsindex * sizeof(int) );
                        m_n_edge_max_points = 0; //TODO this could be precalulated and added to next SENC format

                        my_fgets( buf, MAX_LINE, *pfpx );     // this should be \n

                    }

                    break;
                }

                case 3:                                                           // area as polygon
                {
                    Primitive_type = GEO_AREA;

                    if( !strncmp( FeatureName, "DEPARE", 6 )
                            || !strncmp( FeatureName, "DRGARE", 6 ) ) bIsAssociable = true;

                    int ll = strlen( buf );
                    if( ll > llmax ) llmax = ll;

                    my_fgets( buf, MAX_LINE, *pfpx );     // this will be "  POLYTESSGEO"

                    if( !strncmp( buf, "  POLYTESSGEO", 13 ) ) {
                        float area_ref_lat, area_ref_lon;
                        int nrecl;
                        char tbuf[40];

                        sscanf( buf, " %s %d %f %f", tbuf, &nrecl, &area_ref_lat, &area_ref_lon );

                        if( nrecl ) {
                            unsigned char *polybuf = (unsigned char *) malloc( nrecl + 1 );
                            pfpx->Read( polybuf, nrecl );
                            polybuf[nrecl] = 0;                     // endit
                            PolyTessGeo *ppg = new PolyTessGeo( polybuf, nrecl, FEIndex );
                            free( polybuf );

                            pPolyTessGeo = ppg;

                            //  Set the s57obj bounding box as lat/lon
                            BBObj.SetMin( ppg->Get_xmin(), ppg->Get_ymin() );
                            BBObj.SetMax( ppg->Get_xmax(), ppg->Get_ymax() );
                            bBBObj_valid = true;

                            //  and declare x/y of the object to be average east/north of all points
                            double e1, e2, n1, n2;
                            toSM( ppg->Get_ymax(), ppg->Get_xmax(), area_ref_lat, area_ref_lon, &e1,
                                    &n1 );
                            toSM( ppg->Get_ymin(), ppg->Get_xmin(), area_ref_lat, area_ref_lon, &e2,
                                    &n2 );

                            x = ( e1 + e2 ) / 2.;
                            y = ( n1 + n2 ) / 2.;

                            //  Set the object base point
                            double xll, yll;
                            fromSM( x, y, area_ref_lat, area_ref_lon, &yll, &xll );
                            m_lon = xll;
                            m_lat = yll;

                            //  Capture the edge and connected node table indices
//                            my_fgets(buf, MAX_LINE, *pfpx);     // this will be "\n"
                            my_fgets( buf, MAX_LINE, *pfpx );     // this will be "LSINDEXLIST nnn"

                            sscanf( buf, "%s %d ", tbuf, &m_n_lsindex );

                            m_lsindex_array = (int *) malloc( 3 * m_n_lsindex * sizeof(int) );
                            pfpx->Read( m_lsindex_array, 3 * m_n_lsindex * sizeof(int) );
                            m_n_edge_max_points = 0; //TODO this could be precalulated and added to next SENC format

                            my_fgets( buf, MAX_LINE, *pfpx );     // this should be \n

                        }
                    }

                    break;
                }
            }       //switch

            if( prim > 0 ) {
                Index = FEIndex;
            }
        }               //OGRF
    }                       //while(!dun)

    free( buf );
    free( hdr_buf );

}

//-------------------------------------------------------------------------------------------
//      Attributes in SENC file may not be needed, and can be safely ignored when creating S57Obj
//      Look at a buffer, and return true or false according to a (default) definition
//-------------------------------------------------------------------------------------------

bool S57Obj::IsUsefulAttribute( char *buf )
{

    if( !strncmp( buf, "HDRLEN", 6 ) ) return false;

//      Dump the first 8 standard attributes
    /* -------------------------------------------------------------------- */
    /*      RCID                                                            */
    /* -------------------------------------------------------------------- */
    if( !strncmp( buf + 2, "RCID", 4 ) ) return false;

    /* -------------------------------------------------------------------- */
    /*      LNAM                                                            */
    /* -------------------------------------------------------------------- */
    if( !strncmp( buf + 2, "LNAM", 4 ) ) return false;

    /* -------------------------------------------------------------------- */
    /*      PRIM                                                            */
    /* -------------------------------------------------------------------- */
    else if( !strncmp( buf + 2, "PRIM", 4 ) ) return false;

    /* -------------------------------------------------------------------- */
    /*      SORDAT                                                          */
    /* -------------------------------------------------------------------- */
    else if( !strncmp( buf + 2, "SORDAT", 6 ) ) return false;

    /* -------------------------------------------------------------------- */
    /*      SORIND                                                          */
    /* -------------------------------------------------------------------- */
    else if( !strncmp( buf + 2, "SORIND", 6 ) ) return false;

    //      All others are "Useful"
    else
        return true;

#if (0)
    /* -------------------------------------------------------------------- */
    /*      GRUP                                                            */
    /* -------------------------------------------------------------------- */
    else if(!strncmp(buf, "  GRUP", 6))
    return false;

    /* -------------------------------------------------------------------- */
    /*      OBJL                                                            */
    /* -------------------------------------------------------------------- */
    else if(!strncmp(buf, "  OBJL", 6))
    return false;

    /* -------------------------------------------------------------------- */
    /*      RVER                                                            */
    /* -------------------------------------------------------------------- */
    else if(!strncmp(buf, "  RVER", 6))
    return false;

    /* -------------------------------------------------------------------- */
    /*      AGEN                                                            */
    /* -------------------------------------------------------------------- */
    else if(!strncmp(buf, "  AGEN", 6))
    return false;

    /* -------------------------------------------------------------------- */
    /*      FIDN                                                            */
    /* -------------------------------------------------------------------- */
    else if(!strncmp(buf, "  FIDN", 6))
    return false;

    /* -------------------------------------------------------------------- */
    /*      FIDS                                                            */
    /* -------------------------------------------------------------------- */
    else if(!strncmp(buf, "  FIDS", 6))
    return false;

//      UnPresent data
    else if(strstr(buf, "(null)"))
    return false;

    else
    return true;
#endif
}

//------------------------------------------------------------------------------
//      Local version of fgets for Binary Mode (SENC) file
//------------------------------------------------------------------------------
int S57Obj::my_fgets( char *buf, int buf_len_max, wxInputStream& ifs )

{
    char chNext;
    int nLineLen = 0;
    char *lbuf;

    lbuf = buf;

    while( !ifs.Eof() && nLineLen < buf_len_max ) {
        chNext = (char) ifs.GetC();

        /* each CR/LF (or LF/CR) as if just "CR" */
        if( chNext == 10 || chNext == 13 ) {
            chNext = '\n';
        }

        *lbuf = chNext;
        lbuf++, nLineLen++;

        if( chNext == '\n' ) {
            *lbuf = '\0';
            return nLineLen;
        }
    }

    *( lbuf ) = '\0';

    return nLineLen;
}

//------------------------------------------------------------------------------
//      Local version of bufgetl for Binary Mode (SENC) file
//------------------------------------------------------------------------------
int S57Obj::my_bufgetl( char *ib_read, char *ib_end, char *buf, int buf_len_max )
{
    char chNext;
    int nLineLen = 0;
    char *lbuf;
    char *ibr = ib_read;

    lbuf = buf;

    while( ( nLineLen < buf_len_max ) && ( ibr < ib_end ) ) {
        chNext = *ibr++;

        /* each CR/LF (or LF/CR) as if just "CR" */
        if( chNext == 10 || chNext == 13 ) chNext = '\n';

        *lbuf++ = chNext;
        nLineLen++;

        if( chNext == '\n' ) {
            *lbuf = '\0';
            return nLineLen;
        }
    }

    *( lbuf ) = '\0';
    return nLineLen;
}

wxString S57Obj::GetAttrValueAsString( char *AttrName )
{
    wxString str;
    char *tattList = (char *) calloc( attList->Len() + 1, 1 );
    strncpy( tattList, attList->mb_str(), attList->Len() );

    char *patl = tattList;
    char *patr;
    int idx = 0;
    while( *patl ) {
        patr = patl;
        while( *patr != '\037' )
            patr++;

        if( !strncmp( patl, AttrName, 6 ) ) break;

        patl = patr + 1;
        idx++;
    }

    if( !*patl ) {
        free( tattList );
        return str;
    }

//      using idx to get the attribute value

    S57attVal *v = attVal->Item( idx );

    switch( v->valType ){
        case OGR_STR: {
            char *val = (char *) ( v->value );
            str.Append( wxString( val, wxConvUTF8 ) );
            break;
        }
        case OGR_REAL: {
            double dval = *(double*) ( v->value );
            str.Printf( _T("%g"), dval );
            break;
        }
        case OGR_INT: {
            int ival = *( (int *) v->value );
            str.Printf( _T("%d"), ival );
            break;
        }
        default: {
            str.Printf( _T("Unknown attribute type") );
            break;
        }
    }

    free( tattList );
    return str;
}

//----------------------------------------------------------------------------------
//      render_canvas_parms Implementation
//----------------------------------------------------------------------------------

render_canvas_parms::render_canvas_parms()
{
    pix_buff = NULL;
}

render_canvas_parms::render_canvas_parms( int xr, int yr, int widthr, int heightr, wxColour color )
{
    depth = BPP;
    pb_pitch = ( widthr * depth / 8 );
    lclip = x;
    rclip = x + widthr - 1;
    pix_buff = (unsigned char *) malloc( heightr * pb_pitch );
    width = widthr;
    height = heightr;
    x = xr;
    y = yr;

    unsigned char r, g, b;
    if( color.IsOk() ) {
        r = color.Red();
        g = color.Green();
        b = color.Blue();
    } else
        r = g = b = 0;

    if( depth == 24 ) {
        for( int i = 0; i < height; i++ ) {
            unsigned char *p = pix_buff + ( i * pb_pitch );
            for( int j = 0; j < width; j++ ) {
                *p++ = r;
                *p++ = g;
                *p++ = b;
            }
        }
    } else {
        for( int i = 0; i < height; i++ ) {
            unsigned char *p = pix_buff + ( i * pb_pitch );
            for( int j = 0; j < width; j++ ) {
                *p++ = r;
                *p++ = g;
                *p++ = b;
                *p++ = 0;
            }
        }
    }

}

render_canvas_parms::~render_canvas_parms( void )
{
}

//----------------------------------------------------------------------------------
//      s57chart Implementation
//----------------------------------------------------------------------------------

s57chart::s57chart()
{

    m_ChartType = CHART_TYPE_S57;
    m_ChartFamily = CHART_FAMILY_VECTOR;

    for( int i = 0; i < PRIO_NUM; i++ )
        for( int j = 0; j < LUPNAME_NUM; j++ )
            razRules[i][j] = NULL;

    m_Chart_Scale = 1;                              // Will be fetched during Init()
    m_Chart_Skew = 0.0;

    pDIB = NULL;
    m_pCloneBM = NULL;

// Create ATON arrays, needed by S52PLIB
    pFloatingATONArray = new wxArrayPtrVoid;
    pRigidATONArray = new wxArrayPtrVoid;

    m_tmpup_array = NULL;
    m_pcsv_locn = new wxString( g_csv_locn );

    m_DepthUnits = _T("METERS");
    m_depth_unit_id = DEPTH_UNIT_METERS;

    bGLUWarningSent = false;

    m_pENCDS = NULL;

    m_nvaldco = 0;
    m_nvaldco_alloc = 0;
    m_pvaldco_array = NULL;

    m_bExtentSet = false;

    m_pDIBThumbDay = NULL;
    m_pDIBThumbDim = NULL;
    m_pDIBThumbOrphan = NULL;
    m_bbase_file_attr_known = false;

    m_bLinePrioritySet = false;
    if( ps52plib ) m_plib_state_hash = ps52plib->GetStateHash();
    else
        m_plib_state_hash = 0;

    glGenTextures( 1, (GLuint *) &m_tex_name );
    m_btex_mem = false;

    ref_lat = 0.0;
    ref_lon = 0.0;
}

s57chart::~s57chart()
{

    FreeObjectsAndRules();

    delete pDIB;

    delete m_pCloneBM;
//    delete pFullPath;

    delete pFloatingATONArray;
    delete pRigidATONArray;

    delete m_pcsv_locn;

    delete m_pENCDS;

    free( m_pvaldco_array );

    delete m_pDIBThumbOrphan;

    VE_Hash::iterator it;
    for( it = m_ve_hash.begin(); it != m_ve_hash.end(); ++it ) {
        VE_Element *value = it->second;
        if( value ) {
            free( value->pPoints );
            delete value;
        }
    }
    m_ve_hash.clear();

    VC_Hash::iterator itc;
    for( itc = m_vc_hash.begin(); itc != m_vc_hash.end(); ++itc ) {
        VC_Element *value = itc->second;
        if( value ) {
            free( value->pPoint );
            delete value;
        }
    }
    m_vc_hash.clear();

}

void s57chart::GetValidCanvasRegion( const ViewPort& VPoint, wxRegion *pValidRegion )
{
    int rxl, rxr;
    int ryb, ryt;
    double easting, northing;
    double epix, npix;

    toSM( m_FullExtent.SLAT, m_FullExtent.WLON, VPoint.clat, VPoint.clon, &easting, &northing );
    epix = easting * VPoint.view_scale_ppm;
    npix = northing * VPoint.view_scale_ppm;

    rxl = (int) round((VPoint.pix_width / 2) + epix);
    ryb = (int) round((VPoint.pix_height / 2) - npix);

    toSM( m_FullExtent.NLAT, m_FullExtent.ELON, VPoint.clat, VPoint.clon, &easting, &northing );
    epix = easting * VPoint.view_scale_ppm;
    npix = northing * VPoint.view_scale_ppm;

    rxr = (int) round((VPoint.pix_width / 2) + epix);
    ryt = (int) round((VPoint.pix_height / 2) - npix);

    pValidRegion->Clear();
    pValidRegion->Union( rxl, ryt, rxr - rxl, ryb - ryt );
}

void s57chart::SetColorScheme( ColorScheme cs, bool bApplyImmediate )
{
    if( !ps52plib ) return;
    //  Here we convert (subjectively) the Global ColorScheme
    //  to an appropriate S52 Color scheme, by name.

    switch( cs ){
        case GLOBAL_COLOR_SCHEME_DAY:
            ps52plib->SetPLIBColorScheme( _T("DAY") );
            break;
        case GLOBAL_COLOR_SCHEME_DUSK:
            ps52plib->SetPLIBColorScheme( _T("DUSK") );
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            ps52plib->SetPLIBColorScheme( _T("NIGHT") );
            break;
        default:
            ps52plib->SetPLIBColorScheme( _T("DAY") );
            break;
    }

    m_global_color_scheme = cs;

    if( bApplyImmediate ) {
        delete pDIB;        // Toss any current cache
        pDIB = NULL;
    }

    //      Clear out any cached bitmaps in the text cache
    ClearRenderedTextCache();

    //      Setup the proper thumbnail bitmap pointer

    if( NULL != m_pDIBThumbDay ) {
        switch( cs ){
            default:
            case GLOBAL_COLOR_SCHEME_DAY:
                pThumbData->pDIBThumb = m_pDIBThumbDay;
                m_pDIBThumbOrphan = m_pDIBThumbDim;
                break;
            case GLOBAL_COLOR_SCHEME_DUSK:
            case GLOBAL_COLOR_SCHEME_NIGHT: {
                if( NULL == m_pDIBThumbDim ) {
                    wxImage img = m_pDIBThumbDay->ConvertToImage();

#if wxCHECK_VERSION(2, 8, 0)
                    wxImage gimg = img.ConvertToGreyscale( 0.1, 0.1, 0.1 ); // factors are completely subjective
#else
                            wxImage gimg = img;
#endif

//#ifdef ocpnUSE_ocpnBitmap
//                      ocpnBitmap *pBMP =  new ocpnBitmap(gimg, m_pDIBThumbDay->GetDepth());
//#else
                    wxBitmap *pBMP = new wxBitmap( gimg );
//#endif
                    m_pDIBThumbDim = pBMP;
                    m_pDIBThumbOrphan = m_pDIBThumbDay;
                }

                pThumbData->pDIBThumb = m_pDIBThumbDim;
                break;
            }
        }
    }
}

bool s57chart::GetChartExtent( Extent *pext )
{
    if( m_bExtentSet ) {
        *pext = m_FullExtent;
        return true;
    } else
        return false;
}

void s57chart::FreeObjectsAndRules()
{
//      Delete the created ObjRazRules, including the S57Objs
//      and any child lists
//      The LUPs of base elements are deleted elsewhere ( void s52plib::DestroyLUPArray ( wxArrayOfLUPrec *pLUPArray ))
//      But we need to manually destroy any LUPS related to children

    ObjRazRules *top;
    ObjRazRules *nxx;
    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {

            top = razRules[i][j];
            while( top != NULL ) {
                top->obj->nRef--;
                if( 0 == top->obj->nRef ) delete top->obj;

                if( top->child ) {
                    ObjRazRules *ctop = top->child;
                    while( ctop ) {
                        delete ctop->obj;

                        if( ps52plib ) ps52plib->DestroyLUP( ctop->LUP );
                        delete ctop->LUP;

                        ObjRazRules *cnxx = ctop->next;
                        delete ctop;
                        ctop = cnxx;
                    }
                }

                nxx = top->next;
                free( top );
                top = nxx;
            }
        }
    }
}

void s57chart::ClearRenderedTextCache()
{
    ObjRazRules *top;
    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            top = razRules[i][j];
            while( top != NULL ) {
                if( top->obj->bFText_Added ) {
                    top->obj->bFText_Added = false;
                    delete top->obj->FText;
                    top->obj->FText = NULL;
                }

                if( top->child ) {
                    ObjRazRules *ctop = top->child;
                    while( ctop ) {
                        if( ctop->obj->bFText_Added ) {
                            ctop->obj->bFText_Added = false;
                            delete ctop->obj->FText;
                            ctop->obj->FText = NULL;
                        }
                        ctop = ctop->next;
                    }
                }

                top = top->next;
            }
        }
    }
}

double s57chart::GetNormalScaleMin( double canvas_scale_factor, bool b_allow_overzoom )
{
    double ppm = canvas_scale_factor / m_Chart_Scale; // true_chart_scale_on_display   = m_canvas_scale_factor / pixels_per_meter of displayed chart

    //Adjust overzoom factor based on  b_allow_overzoom option setting
    double oz_factor;
    if( b_allow_overzoom ) oz_factor = 256.;
    else
        oz_factor = 4.;

    ppm *= oz_factor;

    return canvas_scale_factor / ppm;
}
double s57chart::GetNormalScaleMax( double canvas_scale_factor, int canvas_width )
{
    return 1.0e7;
}

//-----------------------------------------------------------------------
//              Pixel to Lat/Long Conversion helpers
//-----------------------------------------------------------------------

void s57chart::GetPointPix( ObjRazRules *rzRules, float north, float east, wxPoint *r )
{
    r->x = (int) round(((east - m_easting_vp_center) * m_view_scale_ppm) + m_pixx_vp_center);
    r->y = (int) round(m_pixy_vp_center - ((north - m_northing_vp_center) * m_view_scale_ppm));
}

void s57chart::GetPointPix( ObjRazRules *rzRules, wxPoint2DDouble *en, wxPoint *r, int nPoints )
{
    for( int i = 0; i < nPoints; i++ ) {
        r[i].x =
                (int) round(((en[i].m_x - m_easting_vp_center) * m_view_scale_ppm) + m_pixx_vp_center);
        r[i].y =
                (int) round(m_pixy_vp_center - ((en[i].m_y - m_northing_vp_center) * m_view_scale_ppm));
    }
}

void s57chart::GetPixPoint( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
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

}

//-----------------------------------------------------------------------
//              Calculate and Set ViewPoint Constants
//-----------------------------------------------------------------------

void s57chart::SetVPParms( const ViewPort &vpt )
{
    //  Set up local SM rendering constants
    m_pixx_vp_center = vpt.pix_width / 2;
    m_pixy_vp_center = vpt.pix_height / 2;
    m_view_scale_ppm = vpt.view_scale_ppm;

    toSM( vpt.clat, vpt.clon, ref_lat, ref_lon, &m_easting_vp_center, &m_northing_vp_center );
}

bool s57chart::AdjustVP( ViewPort &vp_last, ViewPort &vp_proposed )
{
    if( IsCacheValid() ) {

        //      If this viewpoint is same scale as last...
        if( vp_last.view_scale_ppm == vp_proposed.view_scale_ppm ) {

            double prev_easting_c, prev_northing_c;
            toSM( vp_last.clat, vp_last.clon, ref_lat, ref_lon, &prev_easting_c, &prev_northing_c );

            double easting_c, northing_c;
            toSM( vp_proposed.clat, vp_proposed.clon, ref_lat, ref_lon, &easting_c, &northing_c );

            //  then require this viewport to be exact integral pixel difference from last
            //  adjusting clat/clat and SM accordingly

            double delta_pix_x = ( easting_c - prev_easting_c ) * vp_proposed.view_scale_ppm;
            int dpix_x = (int) round ( delta_pix_x );
            double dpx = dpix_x;

            double delta_pix_y = ( northing_c - prev_northing_c ) * vp_proposed.view_scale_ppm;
            int dpix_y = (int) round ( delta_pix_y );
            double dpy = dpix_y;

            double c_east_d = ( dpx / vp_proposed.view_scale_ppm ) + prev_easting_c;
            double c_north_d = ( dpy / vp_proposed.view_scale_ppm ) + prev_northing_c;

            double xlat, xlon;
            fromSM( c_east_d, c_north_d, ref_lat, ref_lon, &xlat, &xlon );

            vp_proposed.clon = xlon;
            vp_proposed.clat = xlat;

            return true;
        }
    }

    return false;
}

/*
 bool s57chart::IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed)
 {
 double last_center_easting, last_center_northing, this_center_easting, this_center_northing;
 toSM ( vp_proposed.clat, vp_proposed.clon, ref_lat, ref_lon, &this_center_easting, &this_center_northing );
 toSM ( vp_last.clat,     vp_last.clon,     ref_lat, ref_lon, &last_center_easting, &last_center_northing );

 int dx = (int)round((last_center_easting  - this_center_easting)  * vp_proposed.view_scale_ppm);
 int dy = (int)round((last_center_northing - this_center_northing) * vp_proposed.view_scale_ppm);

 return((dx !=  0) || (dy != 0) || !(IsCacheValid()) || (vp_proposed.view_scale_ppm != vp_last.view_scale_ppm));
 }
 */

ThumbData *s57chart::GetThumbData( int tnx, int tny, float lat, float lon )
{
    //  Plot the passed lat/lon at the thumbnail bitmap scale
    //  Using simple linear algorithm.
    if( pThumbData->pDIBThumb ) {
        float lat_top = m_FullExtent.NLAT;
        float lat_bot = m_FullExtent.SLAT;
        float lon_left = m_FullExtent.WLON;
        float lon_right = m_FullExtent.ELON;

        // Build the scale factors just as the thumbnail was built
        float ext_max = fmax((lat_top - lat_bot), (lon_right - lon_left));

        float thumb_view_scale_ppm = ( S57_THUMB_SIZE / ext_max ) / ( 1852 * 60 );
        double east, north;
        toSM( lat, lon, ( lat_top + lat_bot ) / 2., ( lon_left + lon_right ) / 2., &east, &north );

        pThumbData->ShipX = pThumbData->pDIBThumb->GetWidth() / 2
                + (int) ( east * thumb_view_scale_ppm );
        pThumbData->ShipY = pThumbData->pDIBThumb->GetHeight() / 2
                - (int) ( north * thumb_view_scale_ppm );

    } else {
        pThumbData->ShipX = 0;
        pThumbData->ShipY = 0;
    }

    return pThumbData;
}

bool s57chart::UpdateThumbData( double lat, double lon )
{
    //  Plot the passed lat/lon at the thumbnail bitmap scale
    //  Using simple linear algorithm.
    int test_x, test_y;
    if( pThumbData->pDIBThumb ) {
        double lat_top = m_FullExtent.NLAT;
        double lat_bot = m_FullExtent.SLAT;
        double lon_left = m_FullExtent.WLON;
        double lon_right = m_FullExtent.ELON;

        // Build the scale factors just as the thumbnail was built
        double ext_max = fmax((lat_top - lat_bot), (lon_right - lon_left));

        double thumb_view_scale_ppm = ( S57_THUMB_SIZE / ext_max ) / ( 1852 * 60 );
        double east, north;
        toSM( lat, lon, ( lat_top + lat_bot ) / 2., ( lon_left + lon_right ) / 2., &east, &north );

        test_x = pThumbData->pDIBThumb->GetWidth() / 2 + (int) ( east * thumb_view_scale_ppm );
        test_y = pThumbData->pDIBThumb->GetHeight() / 2 - (int) ( north * thumb_view_scale_ppm );

    } else {
        test_x = 0;
        test_y = 0;
    }

    if( ( test_x != pThumbData->ShipX ) || ( test_y != pThumbData->ShipY ) ) {
        pThumbData->ShipX = test_x;
        pThumbData->ShipY = test_y;
        return TRUE;
    } else
        return FALSE;
}

void s57chart::SetFullExtent( Extent& ext )
{
    m_FullExtent.NLAT = ext.NLAT;
    m_FullExtent.SLAT = ext.SLAT;
    m_FullExtent.WLON = ext.WLON;
    m_FullExtent.ELON = ext.ELON;

    m_bExtentSet = true;
}

void s57chart::ForceEdgePriorityEvaluate( void )
{
    m_bLinePrioritySet = false;
}

void s57chart::SetLinePriorities( void )
{
    if( !ps52plib ) return;

    //      If necessary.....
    //      Establish line feature rendering priorities

    if( !m_bLinePrioritySet ) {
        ObjRazRules *top;
        ObjRazRules *crnt;

        for( int i = 0; i < PRIO_NUM; ++i ) {

            top = razRules[i][2];           //LINES
            while( top != NULL ) {
                ObjRazRules *crnt = top;
                top = top->next;
                ps52plib->SetLineFeaturePriority( crnt, i );
            }

            //    In the interest of speed, choose only the one necessary area boundary style index
            int j;
            if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES ) j = 4;
            else
                j = 3;

            top = razRules[i][j];
            while( top != NULL ) {
                crnt = top;
                top = top->next;               // next object
                ps52plib->SetLineFeaturePriority( crnt, i );
            }

        }
    }

    //      Mark the priority as set.
    //      Generally only reset by Options Dialog post processing
    m_bLinePrioritySet = true;
}

bool s57chart::RenderRegionViewOnGL( const wxGLContext &glc, const ViewPort& VPoint,
        const wxRegion &Region )
{
    return DoRenderRegionViewOnGL( glc, VPoint, Region, false );
}

bool s57chart::RenderOverlayRegionViewOnGL( const wxGLContext &glc, const ViewPort& VPoint,
        const wxRegion &Region )
{
    return DoRenderRegionViewOnGL( glc, VPoint, Region, true );
}

bool s57chart::DoRenderRegionViewOnGL( const wxGLContext &glc, const ViewPort& VPoint,
        const wxRegion &Region, bool b_overlay )
{
//     CALLGRIND_START_INSTRUMENTATION
//      g_bDebugS57 = true;

    if( !ps52plib ) return false;

    if( g_bDebugS57 ) printf( "\n" );

    SetVPParms( VPoint );

    bool force_new_view = false;

    if( Region != m_last_Region ) force_new_view = true;

    ps52plib->PrepareForRender();

    if( m_plib_state_hash != ps52plib->GetStateHash() ) {
        m_bLinePrioritySet = false;                     // need to reset line priorities
        UpdateLUPs( this );                               // and update the LUPs
        ClearRenderedTextCache();                       // and reset the text renderer,
                                                        //for the case where depth(height) units change
        ResetPointBBoxes( m_last_vp, VPoint );
        m_plib_state_hash = ps52plib->GetStateHash();

    }

    if( VPoint.view_scale_ppm != m_last_vp.view_scale_ppm ) {
        ResetPointBBoxes( m_last_vp, VPoint );
    }

    SetLinePriorities();

    //        Clear the text declutter list
    ps52plib->ClearTextList();

    //    How many rectangles in the Region?
    int n_rect = 0;
    wxRegionIterator clipit( Region );
    while( clipit ) {
        wxRect rect = clipit.GetRect();
        clipit++;
        n_rect++;
    }
    /*
     if(n_rect > 1)
     {
     printf("S57 n_rect: %d\n", n_rect);

     wxRegionIterator upd ( Region );
     while ( upd )
     {
     wxRect rect = upd.GetRect();
     printf ( "   S57 Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
     upd ++ ;
     }
     }
     */

    //    Adjust for rotation
    glPushMatrix();

    if( fabs( VPoint.rotation ) > 0.01 ) {

        double w = VPoint.pix_width;
        double h = VPoint.pix_height;

        //    Rotations occur around 0,0, so calculate a post-rotate translation factor
        double angle = VPoint.rotation;
        angle -= VPoint.skew;

        double ddx = ( w * cos( -angle ) - h * sin( -angle ) - w ) / 2;
        double ddy = ( h * cos( -angle ) + w * sin( -angle ) - h ) / 2;

        glRotatef( angle * 180. / PI, 0, 0, 1 );

        glTranslatef( ddx, ddy, 0 );                 // post rotate translation
    }

    //    Arbitrary optimization....
    //    It is cheaper to draw the entire screen if the rectangle count is large,
    //    as is the case for CM93 charts with non-rectilinear borders
    //    However, most (all?) pan operations on "normal" charts will be small rect count
    if( n_rect < 4 ) {
        wxRegionIterator upd( Region ); // get the Region rect list
        while( upd ) {
            wxRect rect = upd.GetRect();

            //  Build synthetic ViewPort on this rectangle
            //  Especially, we want the BBox to be accurate in order to
            //  render only those objects actually visible in this region

            ViewPort temp_vp = VPoint;
            double temp_lon_left, temp_lat_bot, temp_lon_right, temp_lat_top;

            GetPixPoint( rect.x, rect.y, &temp_lat_top, &temp_lon_left, (ViewPort *) &VPoint );
            GetPixPoint( rect.x + rect.width, rect.y + rect.height, &temp_lat_bot, &temp_lon_right,
                    (ViewPort *) &VPoint );

            if( temp_lon_right < temp_lon_left )        // presumably crossing Greenwich
            temp_lon_right += 360.;

            temp_vp.GetBBox().SetMin( temp_lon_left, temp_lat_bot );
            temp_vp.GetBBox().SetMax( temp_lon_right, temp_lat_top );

            //      Allow some slop in the viewport
            //            double margin = wxMin(temp_vp.GetBBox().GetWidth(), temp_vp.GetBBox().GetHeight()) * 0.05;
            //            temp_vp.GetBBox().EnLarge(margin);

            if( g_bDebugS57 ) printf( "   S57 Render Box:  %d %d %d %d\n", rect.x, rect.y,
                    rect.width, rect.height );

            SetClipRegionGL( glc, temp_vp, rect, !b_overlay );
            DoRenderRectOnGL( glc, temp_vp, rect );

            upd++;
        }
    } else {
        wxRect rect = Region.GetBox();
        if( g_bDebugS57 ) printf( "   S57 Render GetBox:  %d %d %d %d\n", rect.x, rect.y,
                rect.width, rect.height );

        //  Build synthetic ViewPort on this rectangle
        //  Especially, we want the BBox to be accurate in order to
        //  render only those objects actually visible in this region

        ViewPort temp_vp = VPoint;
        double temp_lon_left, temp_lat_bot, temp_lon_right, temp_lat_top;

        GetPixPoint( rect.x, rect.y, &temp_lat_top, &temp_lon_left, (ViewPort *) &VPoint );
        GetPixPoint( rect.x + rect.width, rect.y + rect.height, &temp_lat_bot, &temp_lon_right,
                (ViewPort *) &VPoint );

        if( temp_lon_right < temp_lon_left )        // presumably crossing Greenwich
        temp_lon_right += 360.;

        temp_vp.GetBBox().SetMin( temp_lon_left, temp_lat_bot );
        temp_vp.GetBBox().SetMax( temp_lon_right, temp_lat_top );

        //      Allow some slop in the viewport
        //            double margin = wxMin(temp_vp.GetBBox().GetWidth(), temp_vp.GetBBox().GetHeight()) * 0.05;
        //            temp_vp.GetBBox().EnLarge(margin);

        SetClipRegionGL( glc, VPoint, Region, !b_overlay );
        DoRenderRectOnGL( glc, temp_vp, rect );

    }
//      Update last_vp to reflect current state
    m_last_vp = VPoint;
    m_last_Region = Region;

    glPopMatrix();

//      CALLGRIND_STOP_INSTRUMENTATION

    return true;
}

void s57chart::SetClipRegionGL( const wxGLContext &glc, const ViewPort& VPoint,
        const wxRegion &Region, bool b_render_nodta )
{
    if( g_b_useStencil ) {
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
    }

    //    As a convenience, while we are creating the stencil or depth mask,
    //    also render the default "NODTA" background if selected
    if( b_render_nodta ) {
        wxColour color = GetGlobalColor( _T ( "NODTA" ) );
        float r, g, b;
        if( color.IsOk() ) {
            r = color.Red() / 255.;
            g = color.Green() / 255.;
            b = color.Blue() / 255.;
        } else
            r = g = b = 0;

        glColor3f( r, g, b );
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // enable color buffer

    } else {
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer
    }

    wxRegionIterator upd( Region ); // get the Region rect list
    while( upd ) {
        wxRect rect = upd.GetRect();

        if( g_b_useStencil ) {
            glBegin( GL_QUADS );

            glVertex2f( rect.x, rect.y );
            glVertex2f( rect.x + rect.width, rect.y );
            glVertex2f( rect.x + rect.width, rect.y + rect.height );
            glVertex2f( rect.x, rect.y + rect.height );
            glEnd();

        } else              //  Use depth buffer for clipping
        {
            if( g_bDebugS57 ) printf( "   Depth buffer Region rect:  %d %d %d %d\n", rect.x, rect.y,
                    rect.width, rect.height );

            glBegin( GL_QUADS );

            //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
            //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
            //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
            glVertex3f( rect.x, rect.y, 0.5 );
            glVertex3f( rect.x + rect.width, rect.y, 0.5 );
            glVertex3f( rect.x + rect.width, rect.y + rect.height, 0.5 );
            glVertex3f( rect.x, rect.y + rect.height, 0.5 );
            glEnd();

        }

        upd++;

    }

    if( g_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
    }

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer

}

void s57chart::SetClipRegionGL( const wxGLContext &glc, const ViewPort& VPoint, const wxRect &Rect,
        bool b_render_nodta )
{
    if( g_b_useStencil ) {
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
    }

    //    As a convenience, while we are creating the stencil or depth mask,
    //    also render the default "NODTA" background if selected
    if( b_render_nodta ) {
        wxColour color = GetGlobalColor( _T ( "NODTA" ) );
        float r, g, b;
        if( color.IsOk() ) {
            r = color.Red() / 255.;
            g = color.Green() / 255.;
            b = color.Blue() / 255.;
        } else
            r = g = b = 0;
        glColor3f( r, g, b );
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // enable color buffer

    } else {
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer
    }

    if( g_b_useStencil ) {
        glBegin( GL_QUADS );

        glVertex2f( Rect.x, Rect.y );
        glVertex2f( Rect.x + Rect.width, Rect.y );
        glVertex2f( Rect.x + Rect.width, Rect.y + Rect.height );
        glVertex2f( Rect.x, Rect.y + Rect.height );
        glEnd();

    } else              //  Use depth buffer for clipping
    {
        if( g_bDebugS57 ) printf( "   Depth buffer rect:  %d %d %d %d\n", Rect.x, Rect.y,
                Rect.width, Rect.height );

        glBegin( GL_QUADS );

        //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
        //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
        //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
        glVertex3f( Rect.x, Rect.y, 0.5 );
        glVertex3f( Rect.x + Rect.width, Rect.y, 0.5 );
        glVertex3f( Rect.x + Rect.width, Rect.y + Rect.height, 0.5 );
        glVertex3f( Rect.x, Rect.y + Rect.height, 0.5 );
        glEnd();

    }

    if( g_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
    }

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer

}

bool s57chart::DoRenderRectOnGL( const wxGLContext &glc, const ViewPort& VPoint, wxRect &rect )
{
    int i;
    ObjRazRules *top;
    ObjRazRules *crnt;

    ViewPort tvp = VPoint;                    // undo const  TODO fix this in PLIB

    //    If the ViewPort is unrotated, we can use a simple (fast) scissor test instead
    //    of a stencil or depth buffer clipping algorithm.....
    /*
     if(fabs(VPoint.rotation) < 0.01)
     {

     glScissor(rect.x, VPoint.pix_height-rect.height-rect.y, rect.width, rect.height);
     glEnable(GL_SCISSOR_TEST);
     glDisable (GL_STENCIL_TEST);
     glDisable (GL_DEPTH_TEST);

     }
     */
    if( g_b_useStencil ) glEnable( GL_STENCIL_TEST );
    else
        glEnable( GL_DEPTH_TEST );

    //      Render the areas quickly
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES ) top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3];           // Area Plain Boundaries

        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            ps52plib->RenderAreaToGL( glc, crnt, &tvp, rect );
        }
    }

    //    Render the lines and points
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES ) top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3];           // Area Plain Boundaries
        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            ps52plib->RenderObjectToGL( glc, crnt, &tvp, rect );
        }

        top = razRules[i][2];           //LINES
        while( top != NULL ) {
            ObjRazRules *crnt = top;
            top = top->next;
            ps52plib->RenderObjectToGL( glc, crnt, &tvp, rect );
        }

        if( ps52plib->m_nSymbolStyle == SIMPLIFIED ) top = razRules[i][0];       //SIMPLIFIED Points
        else
            top = razRules[i][1];           //Paper Chart Points Points

        while( top != NULL ) {
            crnt = top;
            top = top->next;
            ps52plib->RenderObjectToGL( glc, crnt, &tvp, rect );
        }

    }

    glDisable( GL_STENCIL_TEST );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_SCISSOR_TEST );

    return true;
}

bool s57chart::RenderRegionViewOnDC( wxMemoryDC& dc, const ViewPort& VPoint,
        const wxRegion &Region )
{
    return DoRenderRegionViewOnDC( dc, VPoint, Region, false );
}

bool s57chart::RenderOverlayRegionViewOnDC( wxMemoryDC& dc, const ViewPort& VPoint,
        const wxRegion &Region )
{
    return DoRenderRegionViewOnDC( dc, VPoint, Region, true );
}

bool s57chart::DoRenderRegionViewOnDC( wxMemoryDC& dc, const ViewPort& VPoint,
        const wxRegion &Region, bool b_overlay )
{
    SetVPParms( VPoint );

    bool force_new_view = false;

    if( Region != m_last_Region ) force_new_view = true;

    ps52plib->PrepareForRender();

    if( m_plib_state_hash != ps52plib->GetStateHash() ) {
        m_bLinePrioritySet = false;                     // need to reset line priorities
        UpdateLUPs( this );                               // and update the LUPs
        ClearRenderedTextCache();                       // and reset the text renderer,
                                                        //for the case where depth(height) units change
        ResetPointBBoxes( m_last_vp, VPoint );
    }

    if( VPoint.view_scale_ppm != m_last_vp.view_scale_ppm ) {
        ResetPointBBoxes( m_last_vp, VPoint );
    }

    SetLinePriorities();

    bool bnew_view = DoRenderViewOnDC( dc, VPoint, DC_RENDER_ONLY, force_new_view );

    //    If quilting, we need to return a cloned bitmap instead of the original golden item
    if( VPoint.b_quilt ) {
        if( m_pCloneBM ) {
            if( ( m_pCloneBM->GetWidth() != VPoint.pix_width )
                    || ( m_pCloneBM->GetHeight() != VPoint.pix_height ) ) {
                delete m_pCloneBM;
                m_pCloneBM = NULL;
            }
        }
        if( NULL == m_pCloneBM ) m_pCloneBM = new wxBitmap( VPoint.pix_width, VPoint.pix_height,
                -1 );

        wxMemoryDC dc_clone;
        dc_clone.SelectObject( *m_pCloneBM );

#ifdef ocpnUSE_DIBSECTION
        ocpnMemDC memdc, dc_org;
#else
        wxMemoryDC memdc, dc_org;
#endif

        pDIB->SelectIntoDC( dc_org );

        //    Decompose the region into rectangles, and fetch them into the target dc
        wxRegionIterator upd( Region ); // get the requested rect list
        while( upd ) {
            wxRect rect = upd.GetRect();
            dc_clone.Blit( rect.x, rect.y, rect.width, rect.height, &dc_org, rect.x, rect.y );
            upd++;
        }

        dc_clone.SelectObject( wxNullBitmap );
        dc_org.SelectObject( wxNullBitmap );

        //    Create a mask
        if( b_overlay ) {
            wxColour nodat = GetGlobalColor( _T ( "NODTA" ) );
            wxColour nodat_sub = nodat;

#ifdef ocpnUSE_ocpnBitmap
            nodat_sub = wxColour( nodat.Blue(), nodat.Green(), nodat.Red() );
#endif
            m_pMask = new wxMask( *m_pCloneBM, nodat_sub );
            m_pCloneBM->SetMask( m_pMask );
        }

        dc.SelectObject( *m_pCloneBM );
    } else
        pDIB->SelectIntoDC( dc );

    m_last_Region = Region;

    return bnew_view;

}

bool s57chart::RenderViewOnDC( wxMemoryDC& dc, const ViewPort& VPoint )
{
//    CALLGRIND_START_INSTRUMENTATION

    SetVPParms( VPoint );

    ps52plib->PrepareForRender();

    if( m_plib_state_hash != ps52plib->GetStateHash() ) {
        m_bLinePrioritySet = false;                     // need to reset line priorities
        UpdateLUPs( this );                               // and update the LUPs
        ClearRenderedTextCache();                       // and reset the text renderer
    }

    SetLinePriorities();

    bool bnew_view = DoRenderViewOnDC( dc, VPoint, DC_RENDER_ONLY, false );

    pDIB->SelectIntoDC( dc );

    return bnew_view;

//    CALLGRIND_STOP_INSTRUMENTATION

}

bool s57chart::DoRenderViewOnDC( wxMemoryDC& dc, const ViewPort& VPoint, RenderTypeEnum option,
        bool force_new_view )
{
    bool bnewview = false;
    wxPoint rul, rlr;
    bool bNewVP = false;

    bool bReallyNew = false;

    double easting_ul, northing_ul;
    double easting_lr, northing_lr;
    double prev_easting_ul = 0., prev_northing_ul = 0.;
    double prev_easting_lr, prev_northing_lr;

    if( ps52plib->GetPLIBColorScheme() != m_lastColorScheme ) bReallyNew = true;
    m_lastColorScheme = ps52plib->GetPLIBColorScheme();

    if( VPoint.view_scale_ppm != m_last_vp.view_scale_ppm ) bReallyNew = true;

    //      If the scale is very small, do not use the cache to avoid harmonic difficulties...
    if( VPoint.chart_scale > 1e8 ) bReallyNew = true;

    wxRect dest( 0, 0, VPoint.pix_width, VPoint.pix_height );
    if( m_last_vprect != dest ) bReallyNew = true;
    m_last_vprect = dest;

    if( m_plib_state_hash != ps52plib->GetStateHash() ) {
        bReallyNew = true;
        m_plib_state_hash = ps52plib->GetStateHash();
    }

    if( bReallyNew ) {
        bNewVP = true;
        delete pDIB;
        pDIB = NULL;
        bnewview = true;
    }

//      Calculate the desired rectangle in the last cached image space
    if( m_last_vp.IsValid() ) {
        easting_ul = m_easting_vp_center - ( ( VPoint.pix_width / 2 ) / m_view_scale_ppm );
        northing_ul = m_northing_vp_center + ( ( VPoint.pix_height / 2 ) / m_view_scale_ppm );
        easting_lr = easting_ul + ( VPoint.pix_width / m_view_scale_ppm );
        northing_lr = northing_ul - ( VPoint.pix_height / m_view_scale_ppm );

        double last_easting_vp_center, last_northing_vp_center;
        toSM( m_last_vp.clat, m_last_vp.clon, ref_lat, ref_lon, &last_easting_vp_center,
                &last_northing_vp_center );

        prev_easting_ul = last_easting_vp_center
                - ( ( m_last_vp.pix_width / 2 ) / m_view_scale_ppm );
        prev_northing_ul = last_northing_vp_center
                + ( ( m_last_vp.pix_height / 2 ) / m_view_scale_ppm );
        prev_easting_lr = easting_ul + ( m_last_vp.pix_width / m_view_scale_ppm );
        prev_northing_lr = northing_ul - ( m_last_vp.pix_height / m_view_scale_ppm );

        double dx = ( easting_ul - prev_easting_ul ) * m_view_scale_ppm;
        double dy = ( prev_northing_ul - northing_ul ) * m_view_scale_ppm;

        rul.x = (int) round((easting_ul - prev_easting_ul) * m_view_scale_ppm);
        rul.y = (int) round((prev_northing_ul - northing_ul) * m_view_scale_ppm);

        rlr.x = (int) round((easting_lr - prev_easting_ul) * m_view_scale_ppm);
        rlr.y = (int) round((prev_northing_ul - northing_lr) * m_view_scale_ppm);

        if( ( fabs( dx - wxRound( dx ) ) > 1e-5 ) || ( fabs( dy - wxRound( dy ) ) > 1e-5 ) ) {
            if( g_bDebugS57 ) printf(
                    "s57chart::DoRender  Cache miss on non-integer pixel delta %g %g\n", dx, dy );
            rul.x = 0;
            rul.y = 0;
            rlr.x = 0;
            rlr.y = 0;
            bNewVP = true;
        }

        else if( ( rul.x != 0 ) || ( rul.y != 0 ) ) {
            if( g_bDebugS57 ) printf( "newvp due to rul\n" );
            bNewVP = true;
        }
    } else {
        rul.x = 0;
        rul.y = 0;
        rlr.x = 0;
        rlr.y = 0;
        bNewVP = true;
    }

    if( force_new_view ) bNewVP = true;

    //      Using regions, calculate re-usable area of pDIB

    wxRegion rgn_last( 0, 0, VPoint.pix_width, VPoint.pix_height );
    wxRegion rgn_new( rul.x, rul.y, rlr.x - rul.x, rlr.y - rul.y );
    rgn_last.Intersect( rgn_new );            // intersection is reusable portion

    if( bNewVP && ( NULL != pDIB ) && !rgn_last.IsEmpty() ) {
        int xu, yu, wu, hu;
        rgn_last.GetBox( xu, yu, wu, hu );

        int desx = 0;
        int desy = 0;
        int srcx = xu;
        int srcy = yu;

        if( rul.x < 0 ) {
            srcx = 0;
            desx = -rul.x;
        }
        if( rul.y < 0 ) {
            srcy = 0;
            desy = -rul.y;
        }

        ocpnMemDC dc_last;
        pDIB->SelectIntoDC( dc_last );

        ocpnMemDC dc_new;
        PixelCache *pDIBNew = new PixelCache( VPoint.pix_width, VPoint.pix_height, BPP );
        pDIBNew->SelectIntoDC( dc_new );

//        printf("reuse blit %d %d %d %d %d %d\n",desx, desy, wu, hu,  srcx, srcy);
        dc_new.Blit( desx, desy, wu, hu, (wxDC *) &dc_last, srcx, srcy, wxCOPY );

        //        Ask the plib to adjust the persistent text rectangle list for this canvas shift
        //        This ensures that, on pans, the list stays in registration with the new text renders to come
        ps52plib->AdjustTextList( desx - srcx, desy - srcy, VPoint.pix_width, VPoint.pix_height );

        dc_new.SelectObject( wxNullBitmap );
        dc_last.SelectObject( wxNullBitmap );

        delete pDIB;
        pDIB = pDIBNew;

//              OK, now have the re-useable section in place
//              Next, build the new sections

        pDIB->SelectIntoDC( dc );

        wxRegion rgn_delta( 0, 0, VPoint.pix_width, VPoint.pix_height );
        wxRegion rgn_reused( desx, desy, wu, hu );
        rgn_delta.Subtract( rgn_reused );

        wxRegionIterator upd( rgn_delta ); // get the update rect list
        while( upd ) {
            wxRect rect = upd.GetRect();

//      Build temp ViewPort on this region

            ViewPort temp_vp = VPoint;
            double temp_lon_left, temp_lat_bot, temp_lon_right, temp_lat_top;

            double temp_northing_ul = prev_northing_ul - ( rul.y / m_view_scale_ppm )
                    - ( rect.y / m_view_scale_ppm );
            double temp_easting_ul = prev_easting_ul + ( rul.x / m_view_scale_ppm )
                    + ( rect.x / m_view_scale_ppm );
            fromSM( temp_easting_ul, temp_northing_ul, ref_lat, ref_lon, &temp_lat_top,
                    &temp_lon_left );

            double temp_northing_lr = temp_northing_ul - ( rect.height / m_view_scale_ppm );
            double temp_easting_lr = temp_easting_ul + ( rect.width / m_view_scale_ppm );
            fromSM( temp_easting_lr, temp_northing_lr, ref_lat, ref_lon, &temp_lat_bot,
                    &temp_lon_right );

            temp_vp.GetBBox().SetMin( temp_lon_left, temp_lat_bot );
            temp_vp.GetBBox().SetMax( temp_lon_right, temp_lat_top );

            //      Allow some slop in the viewport
            //    TODO Investigate why this fails if greater than 5 percent
            double margin = wxMin(temp_vp.GetBBox().GetWidth(), temp_vp.GetBBox().GetHeight())
                    * 0.05;
            temp_vp.GetBBox().EnLarge( margin );

//      And Render it new piece on the target dc
//     printf("New Render, rendering %d %d %d %d \n", rect.x, rect.y, rect.width, rect.height);

            DCRenderRect( dc, temp_vp, &rect );

            upd++;
        }

        dc.SelectObject( wxNullBitmap );

        bnewview = true;

//      Update last_vp to reflect the current cached bitmap
        m_last_vp = VPoint;

    }

    else if( bNewVP || ( NULL == pDIB ) ) {
        delete pDIB;
        pDIB = new PixelCache( VPoint.pix_width, VPoint.pix_height, BPP );     // destination

        wxRect full_rect( 0, 0, VPoint.pix_width, VPoint.pix_height );
        pDIB->SelectIntoDC( dc );

        //        Clear the text declutter list
        ps52plib->ClearTextList();

        DCRenderRect( dc, VPoint, &full_rect );

        dc.SelectObject( wxNullBitmap );

        bnewview = true;

//      Update last_vp to reflect the current cached bitmap
        m_last_vp = VPoint;

    }

    return bnewview;

}

int s57chart::DCRenderRect( wxMemoryDC& dcinput, const ViewPort& vp, wxRect* rect )
{

    int i;
    ObjRazRules *top;
    ObjRazRules *crnt;

    wxASSERT(rect);
    ViewPort tvp = vp;                    // undo const  TODO fix this in PLIB

//    This does not work due to some issue with ref data of allocated buffer.....
//    render_canvas_parms pb_spec( rect->x, rect->y, rect->width, rect->height,  GetGlobalColor ( _T ( "NODTA" ) ));

    render_canvas_parms pb_spec;

    pb_spec.depth = BPP;
    pb_spec.pb_pitch = ( ( rect->width * pb_spec.depth / 8 ) );
    pb_spec.lclip = rect->x;
    pb_spec.rclip = rect->x + rect->width - 1;
    pb_spec.pix_buff = (unsigned char *) malloc( rect->height * pb_spec.pb_pitch );
    pb_spec.width = rect->width;
    pb_spec.height = rect->height;
    pb_spec.x = rect->x;
    pb_spec.y = rect->y;

    // Preset background
    wxColour color = GetGlobalColor( _T ( "NODTA" ) );
    unsigned char r, g, b;
    if( color.IsOk() ) {
        r = color.Red();
        g = color.Green();
        b = color.Blue();
    } else
        r = g = b = 0;

    if( pb_spec.depth == 24 ) {
        for( int i = 0; i < pb_spec.height; i++ ) {
            unsigned char *p = pb_spec.pix_buff + ( i * pb_spec.pb_pitch );
            for( int j = 0; j < pb_spec.width; j++ ) {
                *p++ = r;
                *p++ = g;
                *p++ = b;
            }
        }
    } else {
        int color_int = ( ( r ) << 16 ) + ( ( g ) << 8 ) + ( b );

        for( int i = 0; i < pb_spec.height; i++ ) {
            int *p = (int *) ( pb_spec.pix_buff + ( i * pb_spec.pb_pitch ) );
            for( int j = 0; j < pb_spec.width; j++ ) {
                *p++ = color_int;
            }
        }
    }

//      Render the areas quickly
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES ) top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3];           // Area Plain Boundaries

        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            ps52plib->RenderAreaToDC( &dcinput, crnt, &tvp, &pb_spec );
        }
    }

//      Convert the Private render canvas into a bitmap
#ifdef ocpnUSE_ocpnBitmap
    ocpnBitmap *pREN = new ocpnBitmap( pb_spec.pix_buff, pb_spec.width, pb_spec.height,
            pb_spec.depth );
#else
    wxImage *prender_image = new wxImage(pb_spec.width, pb_spec.height, false);
    prender_image->SetData((unsigned char*)pb_spec.pix_buff);
    wxBitmap *pREN = new wxBitmap(*prender_image);

#endif

//      Map it into a temporary DC
    wxMemoryDC dc_ren;
    dc_ren.SelectObject( *pREN );

//      Blit it onto the target dc
    dcinput.Blit( pb_spec.x, pb_spec.y, pb_spec.width, pb_spec.height, (wxDC *) &dc_ren, 0, 0 );

//      And clean up the mess
    dc_ren.SelectObject( wxNullBitmap );

#ifdef ocpnUSE_ocpnBitmap
    free( pb_spec.pix_buff );
#else
    delete prender_image;           // the image owns the data
                                    // and so will free it in due course
#endif

    delete pREN;

//      Render the rest of the objects/primitives
    DCRenderLPB( dcinput, vp, rect );

    return 1;
}

bool s57chart::DCRenderLPB( wxMemoryDC& dcinput, const ViewPort& vp, wxRect* rect )
{
    int i;
    ObjRazRules *top;
    ObjRazRules *crnt;
    ViewPort tvp = vp;                    // undo const  TODO fix this in PLIB

    for( i = 0; i < PRIO_NUM; ++i ) {
//      Set up a Clipper for Lines
        wxDCClipper *pdcc = NULL;
        if( rect ) {
            wxRect nr = *rect;
//         pdcc = new wxDCClipper(dcinput, nr);
        }

        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES ) top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3];           // Area Plain Boundaries
        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            ps52plib->RenderObjectToDC( &dcinput, crnt, &tvp );
        }

        top = razRules[i][2];           //LINES
        while( top != NULL ) {
            ObjRazRules *crnt = top;
            top = top->next;
            ps52plib->RenderObjectToDC( &dcinput, crnt, &tvp );
        }

        if( ps52plib->m_nSymbolStyle == SIMPLIFIED ) top = razRules[i][0];       //SIMPLIFIED Points
        else
            top = razRules[i][1];           //Paper Chart Points Points

        while( top != NULL ) {
            crnt = top;
            top = top->next;
            ps52plib->RenderObjectToDC( &dcinput, crnt, &tvp );
        }

        //      Destroy Clipper
        if( pdcc ) delete pdcc;
    }

    /*
     printf("Render Lines                  %ldms\n", stlines.Time());
     printf("Render Simple Points          %ldms\n", stsim_pt.Time());
     printf("Render Paper Points           %ldms\n", stpap_pt.Time());
     printf("Render Symbolized Boundaries  %ldms\n", stasb.Time());
     printf("Render Plain Boundaries       %ldms\n\n", stapb.Time());
     */
    return true;
}

bool s57chart::IsCellOverlayType( char *pFullPath )
{
    wxFileName fn( wxString( pFullPath, wxConvUTF8 ) );
    //      Get the "Usage" character
    wxString cname = fn.GetName();
    return ( cname[2] == 'L' );
}

InitReturn s57chart::Init( const wxString& name, ChartInitFlag flags )
{
    //    Use a static semaphore flag to prevent recursion
    if( s_bInS57 ) {
//          printf("s57chart::Init() recursion..., retry\n");
//          wxLogMessage(_T("Recursion"));
        return INIT_FAIL_NOERROR;
    }

    s_bInS57++;

    InitReturn ret_value = INIT_OK;

    m_FullPath = name;
    m_Description = m_FullPath;

    wxFileName fn( name );

    //      Get the "Usage" character
    wxString cname = fn.GetName();
    m_usage_char = cname[2];

    //  Establish a common reference point for the chart
    ref_lat = ( m_FullExtent.NLAT + m_FullExtent.SLAT ) / 2.;
    ref_lon = ( m_FullExtent.WLON + m_FullExtent.ELON ) / 2.;

    if( flags == THUMB_ONLY ) {

        // Look for Thumbnail
        // Set the proper directory for the SENC/BMP files
        wxString SENCdir = g_SENCPrefix;

        if( SENCdir.Last() != fn.GetPathSeparator() ) SENCdir.Append( fn.GetPathSeparator() );

        wxFileName tsfn( SENCdir );
        tsfn.SetFullName( fn.GetFullName() );

        wxFileName ThumbFileNameLook( tsfn );
        ThumbFileNameLook.SetExt( _T("BMP") );

        wxBitmap *pBMP;
        if( ThumbFileNameLook.FileExists() ) {
            pBMP = new wxBitmap;

            pBMP->LoadFile( ThumbFileNameLook.GetFullPath(), wxBITMAP_TYPE_BMP );
            m_pDIBThumbDay = pBMP;
        }

        s_bInS57--;
        return INIT_OK;
    }

    if( flags == HEADER_ONLY ) {
        if( fn.GetExt() == _T("000") ) {
            if( !GetBaseFileAttr( fn ) ) ret_value = INIT_FAIL_REMOVE;

            if( !CreateHeaderDataFromENC() ) ret_value = INIT_FAIL_REMOVE;
            else
                ret_value = INIT_OK;
        } else if( fn.GetExt() == _T("S57") ) {
            m_SENCFileName = name;
            if( !CreateHeaderDataFromSENC() ) ret_value = INIT_FAIL_REMOVE;
            else
                ret_value = INIT_OK;
        }

        s_bInS57--;
        return ret_value;

    }

    //      Full initialization from here

    if( !m_bbase_file_attr_known ) {
        if( !GetBaseFileAttr( fn ) ) ret_value = INIT_FAIL_REMOVE;
        else
            m_bbase_file_attr_known = true;
    }

    if( fn.GetExt() == _T("000") ) {
        if( m_bbase_file_attr_known ) {
            ::wxBeginBusyCursor();

            int sret = FindOrCreateSenc( name );
            if( sret != BUILD_SENC_OK ) {
                if( sret == BUILD_SENC_NOK_RETRY ) ret_value = INIT_FAIL_RETRY;
                else
                    ret_value = INIT_FAIL_REMOVE;
            } else
                ret_value = PostInit( flags, m_global_color_scheme );

            ::wxEndBusyCursor();
        }

    }

    else if( fn.GetExt() == _T("S57") ) {
        ::wxBeginBusyCursor();

        m_SENCFileName = name;
        ret_value = PostInit( flags, m_global_color_scheme );

        ::wxEndBusyCursor();

    }

    s_bInS57--;
    return ret_value;

}

//-----------------------------------------------------------------------------------------------
//    Find or Create a relevent SENC file from a given .000 ENC file
//    Returns with error code, and associated SENC file name in m_S57FileName
//-----------------------------------------------------------------------------------------------
InitReturn s57chart::FindOrCreateSenc( const wxString& name )
{
    //      Establish location for SENC files
    m_SENCFileName = name;
    m_SENCFileName.SetExt( _T("S57") );

    //      Set the proper directory for the SENC files
    wxString SENCdir = g_SENCPrefix;

    if( SENCdir.Last() != m_SENCFileName.GetPathSeparator() ) SENCdir.Append(
            m_SENCFileName.GetPathSeparator() );

    wxFileName tsfn( SENCdir );
    tsfn.SetFullName( m_SENCFileName.GetFullName() );
    m_SENCFileName = tsfn;

    // Really can only Init and use S57 chart if the S52 Presentation Library is OK
    if( !ps52plib->m_bOK ) return INIT_FAIL_REMOVE;

    int build_ret_val = 1;

    bool bbuild_new_senc = false;
    m_bneed_new_thumbnail = false;

    wxFileName FileName000( name );

//      Look for SENC file in the target directory

    if( m_SENCFileName.FileExists() ) {
        wxFile f;
        if( f.Open( m_SENCFileName.GetFullPath() ) ) {
            if( f.Length() == 0 ) {
                f.Close();
                build_ret_val = BuildSENCFile( name, m_SENCFileName.GetFullPath() );
            } else                                      // file exists, non-zero
            {                                         // so check for new updates

                f.Seek( 0 );
                wxFileInputStream *pfpx_u = new wxFileInputStream( f );
                wxBufferedInputStream *pfpx = new wxBufferedInputStream( *pfpx_u );
                int dun = 0;
                int last_update = 0;
                int senc_file_version = 0;
                int force_make_senc = 0;
                char buf[256];
                char *pbuf = buf;
                wxDateTime ModTime000;
                int size000 = 0;
                wxString senc_base_edtn;

                while( !dun ) {
                    if( my_fgets( pbuf, 256, *pfpx ) == 0 ) {
                        dun = 1;
                        force_make_senc = 1;
                        break;
                    } else {
                        if( !strncmp( pbuf, "OGRF", 4 ) ) {
                            dun = 1;
                            break;
                        }

                        wxString str_buf( pbuf, wxConvUTF8 );
                        wxStringTokenizer tkz( str_buf, _T("=") );
                        wxString token = tkz.GetNextToken();

                        if( token.IsSameAs( _T("UPDT"), TRUE ) ) {
                            int i;
                            i = tkz.GetPosition();
                            last_update = atoi( &pbuf[i] );
                        }

                        else if( token.IsSameAs( _T("SENC Version"), TRUE ) ) {
                            int i;
                            i = tkz.GetPosition();
                            senc_file_version = atoi( &pbuf[i] );
                        }

                        else if( token.IsSameAs( _T("FILEMOD000"), TRUE ) ) {
                            int i;
                            i = tkz.GetPosition();
                            wxString str( &pbuf[i], wxConvUTF8 );
                            str.Trim();                               // gets rid of newline, etc...
                            if( !ModTime000.ParseFormat( str,
                                    _T("%Y%m%d")/*(const wxChar *)"%Y%m%d"*/) ) ModTime000.SetToCurrent();
                            ModTime000.ResetTime();                   // to midnight
                        }

                        else if( token.IsSameAs( _T("FILESIZE000"), TRUE ) ) {
                            int i;
                            i = tkz.GetPosition();
                            size000 = atoi( &pbuf[i] );
                        }

                        else if( token.IsSameAs( _T("EDTN000"), TRUE ) ) {
                            int i;
                            i = tkz.GetPosition();
                            wxString str( &pbuf[i], wxConvUTF8 );
                            str.Trim();                               // gets rid of newline, etc...
                            senc_base_edtn = str;
                        }

                    }
                }

                delete pfpx;
                delete pfpx_u;
                f.Close();
//              Anything to do?
// force_make_senc = 1;
                //  SENC file version has to be correct for other tests to make sense
                if( senc_file_version != CURRENT_SENC_FORMAT_VERSION ) bbuild_new_senc = true;

                //  Senc EDTN must be the same as .000 file EDTN.
                //  This test catches the usual case where the .000 file is updated from the web,
                //  and all updates (.001, .002, etc.)  are subsumed.
                else if( !senc_base_edtn.IsSameAs( m_edtn000 ) ) bbuild_new_senc = true;

                else {
                    //    See if there are any new update files  in the ENC directory
                    int most_recent_update_file = GetUpdateFileArray( FileName000, NULL );

                    if( last_update != most_recent_update_file ) bbuild_new_senc = true;

//          Make two simple tests to see if the .000 file is "newer" than the SENC file representation
//          These tests may be redundant, since the DSID:EDTN test above should catch new base files
                    wxDateTime OModTime000;
                    FileName000.GetTimes( NULL, &OModTime000, NULL );
                    OModTime000.ResetTime();                      // to midnight
                    if( ModTime000.IsValid() ) if( OModTime000.IsLaterThan( ModTime000 ) ) bbuild_new_senc =
                            true;

                    int Osize000l = FileName000.GetSize().GetLo();
                    if( size000 != Osize000l ) bbuild_new_senc = true;
                }

                if( force_make_senc ) bbuild_new_senc = true;

                if( bbuild_new_senc ) build_ret_val = BuildSENCFile( name,
                        m_SENCFileName.GetFullPath() );

            }
        }
    }

    else                    // SENC file does not exist
    {
        build_ret_val = BuildSENCFile( name, m_SENCFileName.GetFullPath() );
        bbuild_new_senc = true;
    }

    if( bbuild_new_senc ) m_bneed_new_thumbnail = true; // force a new thumbnail to be built in PostInit()

    if( bbuild_new_senc ) {
        if( BUILD_SENC_NOK_PERMANENT == build_ret_val ) return INIT_FAIL_REMOVE;
        if( BUILD_SENC_NOK_RETRY == build_ret_val ) return INIT_FAIL_RETRY;
    }

    return INIT_OK;
}

InitReturn s57chart::PostInit( ChartInitFlag flags, ColorScheme cs )
{

//    SENC file is ready, so build the RAZ structure
    if( 0 != BuildRAZFromSENCFile( m_SENCFileName.GetFullPath() ) ) {
        wxString msg( _T("   Cannot load SENC file ") );
        msg.Append( m_SENCFileName.GetFullPath() );
        wxLogMessage( msg );

        return INIT_FAIL_RETRY;
    }

//      Check for and if necessary rebuild Thumbnail
//      Going to be in the global (user) SENC file directory

    wxString SENCdir = g_SENCPrefix;
    if( SENCdir.Last() != m_SENCFileName.GetPathSeparator() ) SENCdir.Append(
            m_SENCFileName.GetPathSeparator() );

    wxFileName ThumbFileName( SENCdir, m_SENCFileName.GetName(), _T("BMP") );

    if( !ThumbFileName.FileExists() || m_bneed_new_thumbnail ) BuildThumbnail(
            ThumbFileName.GetFullPath() );

//  Update the member thumbdata structure
    if( ThumbFileName.FileExists() ) {
        wxBitmap *pBMP_NEW;
#ifdef ocpnUSE_ocpnBitmap
        pBMP_NEW = new ocpnBitmap;
#else
        pBMP_NEW = new wxBitmap;
#endif
        if( pBMP_NEW->LoadFile( ThumbFileName.GetFullPath(), wxBITMAP_TYPE_BMP ) ) {
            delete pThumbData;
            pThumbData = new ThumbData;
            m_pDIBThumbDay = pBMP_NEW;
//                    pThumbData->pDIBThumb = pBMP_NEW;
        }
    }

//    Set the color scheme
    m_global_color_scheme = cs;
    SetColorScheme( cs, false );

//    Build array of contour values for later use by conditional symbology

    BuildDepthContourArray();
    bReadyToRender = true;

    return INIT_OK;
}

void s57chart::BuildDepthContourArray( void )
{
    //    Build array of contour values for later use by conditional symbology

    if( 0 == m_nvaldco_alloc ) {
        m_nvaldco_alloc = 5;
        m_pvaldco_array = (double *) calloc( m_nvaldco_alloc, sizeof(double) );
    }

    ObjRazRules *top;
    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {

            top = razRules[i][j];
            while( top != NULL ) {
                if( !strncmp( top->obj->FeatureName, "DEPCNT", 6 ) ) {
                    double valdco = 0.0;
                    if( GetDoubleAttr( top->obj, "VALDCO", valdco ) ) {
                        m_nvaldco++;
                        if( m_nvaldco > m_nvaldco_alloc ) {
                            void *tr = realloc( (void *) m_pvaldco_array,
                                    m_nvaldco_alloc * 2 * sizeof(double) );
                            m_pvaldco_array = (double *) tr;
                            m_nvaldco_alloc *= 2;
                        }
                        m_pvaldco_array[m_nvaldco - 1] = valdco;
                    }
                }
                ObjRazRules *nxx = top->next;
                top = nxx;
            }
        }
    }

    /*
     //      And bubble sort it
     bool swap = true;
     int isort;

     while(swap == true)
     {
     swap = false;
     isort = 0;
     while(isort < m_nvaldco - 1)
     {
     if(m_pvaldco_array[isort+1] < m_pvaldco_array[isort])
     {
     double t = m_pvaldco_array[isort];
     m_pvaldco_array[isort] = m_pvaldco_array[isort+1];
     m_pvaldco_array[isort+1] = t;
     swap = true;
     }
     isort++;
     }
     }
     */
    std::sort( m_pvaldco_array, m_pvaldco_array + m_nvaldco );

}

void s57chart::InvalidateCache()
{
    delete pDIB;
    pDIB = NULL;
}

bool s57chart::BuildThumbnail( const wxString &bmpname )
{
    bool ret_code;

    wxFileName ThumbFileName( bmpname );

    //      Make the target directory if needed
    if( true != ThumbFileName.DirExists( ThumbFileName.GetPath() ) ) {
        if( !ThumbFileName.Mkdir( ThumbFileName.GetPath() ) ) {
            wxLogMessage(
                    _T("   Cannot create BMP file directory for ")
                            + ThumbFileName.GetFullPath() );
            return false;
        }
    }

    //      Set up a private ViewPort
    ViewPort vp;

    vp.clon = ( m_FullExtent.ELON + m_FullExtent.WLON ) / 2.;
    vp.clat = ( m_FullExtent.NLAT + m_FullExtent.SLAT ) / 2.;

    float ext_max =
            fmax((m_FullExtent.NLAT - m_FullExtent.SLAT), (m_FullExtent.ELON - m_FullExtent.WLON));

    vp.view_scale_ppm = ( S57_THUMB_SIZE / ext_max ) / ( 1852 * 60 );

    vp.pix_height = S57_THUMB_SIZE;
    vp.pix_width = S57_THUMB_SIZE;

    vp.GetBBox().SetMin( m_FullExtent.WLON, m_FullExtent.SLAT );
    vp.GetBBox().SetMax( m_FullExtent.ELON, m_FullExtent.NLAT );

    vp.chart_scale = 10000000 - 1;
    vp.Validate();

    // cause a clean new render
    delete pDIB;
    pDIB = NULL;

    SetVPParms( vp );

//      Borrow the OBJLArray temporarily to set the object type visibility for this render
//      First, make a copy for the curent OBJLArray viz settings, setting current value to invisible

    unsigned int OBJLCount = ps52plib->pOBJLArray->GetCount();
//      int *psave_viz = new int[OBJLCount];
    int *psave_viz = (int *) malloc( OBJLCount * sizeof(int) );

    int *psvr = psave_viz;
    OBJLElement *pOLE;
    unsigned int iPtr;

    for( iPtr = 0; iPtr < OBJLCount; iPtr++ ) {
        pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
        *psvr++ = pOLE->nViz;
        pOLE->nViz = 0;
    }

//      Also, save some other settings
    bool bsavem_bShowSoundgp = ps52plib->m_bShowSoundg;

//      Now, set up what I want for this render
    for( iPtr = 0; iPtr < OBJLCount; iPtr++ ) {
        pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
        if( !strncmp( pOLE->OBJLName, "LNDARE", 6 ) ) pOLE->nViz = 1;
        if( !strncmp( pOLE->OBJLName, "DEPARE", 6 ) ) pOLE->nViz = 1;
    }

    ps52plib->m_bShowSoundg = false;

//      Use display category MARINERS_STANDARD to force use of OBJLArray
    DisCat dsave = ps52plib->m_nDisplayCategory;
    ps52plib->m_nDisplayCategory = MARINERS_STANDARD;

#ifdef ocpnUSE_DIBSECTION
    ocpnMemDC memdc, dc_org;
#else
    wxMemoryDC memdc, dc_org;
#endif

//      set the color scheme
    ps52plib->SaveColorScheme();
    ps52plib->SetPLIBColorScheme( _T("DAY") );
//      Do the render
    DoRenderViewOnDC( memdc, vp, DC_RENDER_ONLY, true );

//      Release the DIB
    memdc.SelectObject( wxNullBitmap );

//      Restore the plib to previous state
    psvr = psave_viz;
    for( iPtr = 0; iPtr < OBJLCount; iPtr++ ) {
        pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
        pOLE->nViz = *psvr++;
    }

    ps52plib->m_nDisplayCategory = dsave;
    ps52plib->m_bShowSoundg = bsavem_bShowSoundgp;

//      Reset the color scheme
    ps52plib->RestoreColorScheme();

//       delete psave_viz;
    free( psave_viz );

//      Clone pDIB into pThumbData;
    wxBitmap *pBMP;

    pBMP = new wxBitmap( vp.pix_width, vp.pix_height/*,  BPP*/);

    wxMemoryDC dc_clone;
    dc_clone.SelectObject( *pBMP );

    pDIB->SelectIntoDC( dc_org );

    dc_clone.Blit( 0, 0, vp.pix_width, vp.pix_height, (wxDC *) &dc_org, 0, 0 );

    dc_clone.SelectObject( wxNullBitmap );
    dc_org.SelectObject( wxNullBitmap );

    //   Save the file
    ret_code = pBMP->SaveFile( ThumbFileName.GetFullPath(), wxBITMAP_TYPE_BMP );

    delete pBMP;

    return ret_code;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_ARRAY( float*, MyFloatPtrArray );

//    Read the .000 ENC file and create required Chartbase data structures
bool s57chart::CreateHeaderDataFromENC( void )
{
    if( !InitENCMinimal( m_FullPath ) ) {
        wxString msg( _T("   Cannot initialize ENC file ") );
        msg.Append( m_FullPath );
        wxLogMessage( msg );

        return false;
    }

    OGRFeature *pFeat;
    int catcov;
    float LatMax, LatMin, LonMax, LonMin;
    LatMax = -90.;
    LatMin = 90.;
    LonMax = -179.;
    LonMin = 179.;

    m_pCOVRTablePoints = NULL;
    m_pCOVRTable = NULL;

    //  Create arrays to hold geometry objects temporarily
    MyFloatPtrArray *pAuxPtrArray = new MyFloatPtrArray;
    wxArrayInt *pAuxCntArray = new wxArrayInt;

    MyFloatPtrArray *pNoCovrPtrArray = new MyFloatPtrArray;
    wxArrayInt *pNoCovrCntArray = new wxArrayInt;

    //Get the first M_COVR object
    pFeat = GetChartFirstM_COVR( catcov );

    while( pFeat ) {

                //    Get the next M_COVR feature, and create possible additional entries for COVR
                OGRPolygon *poly = (OGRPolygon *) ( pFeat->GetGeometryRef() );
                OGRLinearRing *xring = poly->getExteriorRing();

                int npt = xring->getNumPoints();

                float *pf = NULL;

                if( npt >= 3 ) {
                    pf = (float *) malloc( 2 * npt * sizeof(float) );
                    float *pfr = pf;

                    for( int i = 0; i < npt; i++ ) {
                        OGRPoint p;
                        xring->getPoint( i, &p );

                        if( catcov == 1 ) {
                            LatMax = fmax(LatMax, p.getY());
                            LatMin = fmin(LatMin, p.getY());
                            LonMax = fmax(LonMax, p.getX());
                            LonMin = fmin(LonMin, p.getX());
                        }

                        pfr[0] = p.getY();             // lat
                        pfr[1] = p.getX();             // lon

                        pfr += 2;
                    }
                }

            delete pFeat;

            if( catcov == 1 ) {
                pAuxPtrArray->Add( pf );
                pAuxCntArray->Add( npt );
            }
            else if( catcov == 2 ){
                pNoCovrPtrArray->Add( pf );
                pNoCovrCntArray->Add( npt );
            }

            pFeat = GetChartNextM_COVR( catcov );
        }         // while

        //    Allocate the storage

        m_nCOVREntries = pAuxCntArray->GetCount();

        //    If only one M_COVR,CATCOV=1 object was found,
        //    assign the geometry to the one and only COVR

        if( m_nCOVREntries == 1 ) {
            m_pCOVRTablePoints = (int *) malloc( sizeof(int) );
            *m_pCOVRTablePoints = pAuxCntArray->Item( 0 );
            m_pCOVRTable = (float **) malloc( sizeof(float *) );
            *m_pCOVRTable = (float *) malloc( pAuxCntArray->Item( 0 ) * 2 * sizeof(float) );
            memcpy( *m_pCOVRTable, pAuxPtrArray->Item( 0 ),
                    pAuxCntArray->Item( 0 ) * 2 * sizeof(float) );
        }

        else if( m_nCOVREntries > 1 ) {
            //    Create new COVR entries
            m_pCOVRTablePoints = (int *) malloc( m_nCOVREntries * sizeof(int) );
            m_pCOVRTable = (float **) malloc( m_nCOVREntries * sizeof(float *) );

            for( unsigned int j = 0; j < (unsigned int) m_nCOVREntries; j++ ) {
                m_pCOVRTablePoints[j] = pAuxCntArray->Item( j );
                m_pCOVRTable[j] = (float *) malloc( pAuxCntArray->Item( j ) * 2 * sizeof(float) );
                memcpy( m_pCOVRTable[j], pAuxPtrArray->Item( j ),
                        pAuxCntArray->Item( j ) * 2 * sizeof(float) );
            }
        }

        else                                     // strange case, found no CATCOV=1 M_COVR objects
        {
            wxString msg( _T("   ENC contains no useable M_COVR, CATCOV=1 features:  ") );
            msg.Append( m_FullPath );
            wxLogMessage( msg );
        }


        //      And for the NoCovr regions
        m_nNoCOVREntries = pNoCovrCntArray->GetCount();

        if( m_nNoCOVREntries ) {
            //    Create new NoCOVR entries
            m_pNoCOVRTablePoints = (int *) malloc( m_nNoCOVREntries * sizeof(int) );
            m_pNoCOVRTable = (float **) malloc( m_nNoCOVREntries * sizeof(float *) );

            for( unsigned int j = 0; j < (unsigned int) m_nNoCOVREntries; j++ ) {
                int npoints = pNoCovrCntArray->Item( j );
                m_pNoCOVRTablePoints[j] = npoints;
                m_pNoCOVRTable[j] = (float *) malloc( npoints * 2 * sizeof(float) );
                memcpy( m_pNoCOVRTable[j], pNoCovrPtrArray->Item( j ),
                        npoints * 2 * sizeof(float) );
            }
        }
        else {
            m_pNoCOVRTablePoints = NULL;
            m_pNoCOVRTable = NULL;
        }

        delete pAuxPtrArray;
        delete pAuxCntArray;
        delete pNoCovrPtrArray;
        delete pNoCovrCntArray;


    if( 0 == m_nCOVREntries ) {                        // fallback
        wxString msg( _T("   ENC contains no M_COVR features:  ") );
        msg.Append( m_FullPath );
        wxLogMessage( msg );

        msg =  _T("   Calculating Chart Extents as fallback.");
        wxLogMessage( msg );

        OGREnvelope Env;

        //    Get the reader
        S57Reader *pENCReader = m_pENCDS->GetModule( 0 );

        if( pENCReader->GetExtent( &Env, true ) == OGRERR_NONE ) {

            LatMax = Env.MaxY;
            LonMax = Env.MaxX;
            LatMin = Env.MinY;
            LonMin = Env.MinX;

            m_nCOVREntries = 1;
            m_pCOVRTablePoints = (int *) malloc( sizeof(int) );
            *m_pCOVRTablePoints = 4;
            m_pCOVRTable = (float **) malloc( sizeof(float *) );
            float *pf = (float *) malloc( 2 * 4 * sizeof(float) );
            *m_pCOVRTable = pf;
            float *pfe = pf;

            *pfe++ = LatMax;
            *pfe++ = LonMin;

            *pfe++ = LatMax;
            *pfe++ = LonMax;

            *pfe++ = LatMin;
            *pfe++ = LonMax;

            *pfe++ = LatMin;
            *pfe++ = LonMin;

        } else {
            wxString msg( _T("   Cannot calculate Extents for ENC:  ") );
            msg.Append( m_FullPath );
            wxLogMessage( msg );

            return false;                     // chart is completely unusable
        }
    }

    //    Populate the chart's extent structure
    m_FullExtent.NLAT = LatMax;
    m_FullExtent.SLAT = LatMin;
    m_FullExtent.ELON = LonMax;
    m_FullExtent.WLON = LonMin;
    m_bExtentSet = true;

    //    Set the chart scale
    m_Chart_Scale = GetENCScale();

    wxString nice_name;
    GetChartNameFromTXT( m_FullPath, nice_name );
    m_Name = nice_name;


    return true;
}

//    Read the .S57 SENC file and create required Chartbase data structures
bool s57chart::CreateHeaderDataFromSENC( void )
{
    bool ret_val = true;
    //    Sanity check for existence of file

    if( !m_SENCFileName.FileExists() ) {
        wxString msg( _T("   Cannot open SENC file ") );
        msg.Append( m_SENCFileName.GetFullPath() );
        wxLogMessage( msg );

        return 1;
    }

    wxString ifs( m_SENCFileName.GetFullPath() );

    wxFileInputStream fpx_u( ifs );
    wxBufferedInputStream fpx( fpx_u );

    int MAX_LINE = 499999;
    char *buf = (char *) malloc( MAX_LINE + 1 );

    int dun = 0;

    hdr_buf = (char *) malloc( 1 );
    wxString date_000, date_upd;

    while( !dun ) {

        if( my_fgets( buf, MAX_LINE, fpx ) == 0 ) {
            dun = 1;
            break;
        }

        if( !strncmp( buf, "OGRF", 4 ) ) {
            dun = 1;
            break;
        }               //OGRF

        else if( !strncmp( buf, "SENC", 4 ) ) {
            int senc_file_version;
            sscanf( buf, "SENC Version=%i", &senc_file_version );
            if( senc_file_version != CURRENT_SENC_FORMAT_VERSION ) {
                wxString msg( _T("   Wrong version on SENC file ") );
                msg.Append( m_SENCFileName.GetFullPath() );
                wxLogMessage( msg );

                dun = 1;
                ret_val = false;                   // error
                break;
            }
        }

        else if( !strncmp( buf, "DATEUPD", 7 ) ) {
            date_upd.Append( wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' ) );
        }

        else if( !strncmp( buf, "DATE000", 7 ) ) {
            date_000.Append( wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' ) );
        }

        else if( !strncmp( buf, "SCALE", 5 ) ) {
            int ins;
            sscanf( buf, "SCALE=%d", &ins );
            m_Chart_Scale = ins;

        }

        else if( !strncmp( buf, "NAME", 4 ) ) {
            m_Name = wxString( &buf[5], wxConvUTF8 ).BeforeFirst( '\n' );
        }

        else if( !strncmp( buf, "Chart Extents:", 14 ) ) {
            float elon, wlon, nlat, slat;
            sscanf( buf, "Chart Extents: %g %g %g %g", &elon, &wlon, &nlat, &slat );
            m_FullExtent.ELON = elon;
            m_FullExtent.WLON = wlon;
            m_FullExtent.NLAT = nlat;
            m_FullExtent.SLAT = slat;
            m_bExtentSet = true;
        }

    }                       //while(!dun)

    //    Populate COVR structures
    if( m_bExtentSet ) {
        m_nCOVREntries = 1;                       // this is the default for pre-made SENCs

        if( m_nCOVREntries == 1 ) {
            m_pCOVRTablePoints = (int *) malloc( sizeof(int) );
            *m_pCOVRTablePoints = 4;
            m_pCOVRTable = (float **) malloc( sizeof(float *) );

            float *pf = (float *) malloc( 2 * 4 * sizeof(float) );
            *m_pCOVRTable = pf;
            float *pfe = pf;

            *pfe++ = m_FullExtent.NLAT;
            *pfe++ = m_FullExtent.ELON;

            *pfe++ = m_FullExtent.NLAT;
            *pfe++ = m_FullExtent.WLON;

            *pfe++ = m_FullExtent.SLAT;
            *pfe++ = m_FullExtent.WLON;

            *pfe++ = m_FullExtent.SLAT;
            *pfe++ = m_FullExtent.ELON;
        }
    }

    free( buf );

    free( hdr_buf );

    //   Decide on pub date to show

    int d000 = atoi(
            ( date_000/*(wxString((const wchar_t *)date_000, wxConvUTF8)*/.Mid( 0, 4 ) ).mb_str() );
    int dupd = atoi(
            ( date_upd/*(wxString((const wchar_t *)date_upd, wxConvUTF8)*/.Mid( 0, 4 ) ).mb_str() );

    if( dupd > d000 ) m_PubYear = date_upd/*wxString((const wchar_t *)date_upd, wxConvUTF8)*/.Mid(
            0, 4 );
    else
        m_PubYear = date_000/*wxString((const wchar_t *)date_000, wxConvUTF8)*/.Mid( 0, 4 );

    wxDateTime dt;
    dt.ParseDate( date_000 );

    if( !ret_val ) return false;

    return true;
}

/*    This method returns the smallest chart DEPCNT:VALDCO value which
 is greater than or equal to the specified value
 */
bool s57chart::GetNearestSafeContour( double safe_cnt, double &next_safe_cnt )
{
    int i = 0;
    if( NULL != m_pvaldco_array ) {
        for( i = 0; i < m_nvaldco; i++ ) {
            if( m_pvaldco_array[i] >= safe_cnt ) break;
        }

        if( i < m_nvaldco ) next_safe_cnt = m_pvaldco_array[i];
        else
            next_safe_cnt = (double) 1e6;
        return true;
    } else {
        next_safe_cnt = (double) 1e6;
        return false;
    }
}

/*
 --------------------------------------------------------------------------
 Build a list of "associated" DEPARE and DRGARE objects from a given
 object. to be "associated" means to be physically intersecting,
 overlapping, or contained within, depending upon the geometry type
 of the given object.
 --------------------------------------------------------------------------
 */

ListOfS57Obj *s57chart::GetAssociatedObjects( S57Obj *obj )
{
    int disPrioIdx;
    bool gotit;

    ListOfS57Obj *pobj_list = new ListOfS57Obj;
    pobj_list->Clear();

    double lat, lon;
    fromSM( ( obj->x * obj->x_rate ) + obj->x_origin, ( obj->y * obj->y_rate ) + obj->y_origin,
            ref_lat, ref_lon, &lat, &lon );
    //    What is the entry object geometry type?

    switch( obj->Primitive_type ){
        case GEO_POINT:
            ObjRazRules *top;
            disPrioIdx = 1;         // PRIO_GROUP1:S57 group 1 filled areas

            /*
             for(j=0 ; j<LUPNAME_NUM ; j++)
             {
             top = razRules[disPrioIdx][j];
             while ( top != NULL)
             {
             //                              if(!strncmp(top->obj->FeatureName, "DEPARE", 6) || !strncmp(top->obj->FeatureName, "DRGARE", 6))
             if(top->obj->bIsAssociable)
             {
             if(top->obj->BBObj.PointInBox( lon, lat, 0.0))
             {
             if(IsPointInObjArea(lat, lon, 0.0, top->obj))
             {
             pobj_list->Append(top->obj);
             break;
             }
             }
             }

             ObjRazRules *nxx  = top->next;
             top = nxx;
             }
             }
             */

            gotit = false;
            top = razRules[disPrioIdx][3];     // PLAIN_BOUNDARIES
            while( top != NULL ) {
                if( top->obj->bIsAssociable ) {
                    if( top->obj->BBObj.PointInBox( lon, lat, 0.0 ) ) {
                        if( IsPointInObjArea( lat, lon, 0.0, top->obj ) ) {
                            pobj_list->Append( top->obj );
                            gotit = true;
                            break;
                        }
                    }
                }

                ObjRazRules *nxx = top->next;
                top = nxx;
            }

            if( !gotit ) {
                top = razRules[disPrioIdx][4];     // SYMBOLIZED_BOUNDARIES
                while( top != NULL ) {
                    if( top->obj->bIsAssociable ) {
                        if( top->obj->BBObj.PointInBox( lon, lat, 0.0 ) ) {
                            if( IsPointInObjArea( lat, lon, 0.0, top->obj ) ) {
                                pobj_list->Append( top->obj );
                                break;
                            }
                        }
                    }

                    ObjRazRules *nxx = top->next;
                    top = nxx;
                }
            }

            break;

        case GEO_LINE:
            break;

        case GEO_AREA:
            break;

        default:
            break;
    }

    return pobj_list;
}

void s57chart::GetChartNameFromTXT( const wxString& FullPath, wxString &Name )
{

    wxFileName fn( FullPath );

    wxString target_name = fn.GetName();
    target_name.RemoveLast();

    wxString dir_name = fn.GetPath();

    wxDir dir( dir_name );                                  // The directory containing the file

    wxArrayString FileList;

    dir.GetAllFiles( fn.GetPath(), &FileList );             // list all the files

    //    Iterate on the file list...

    bool found_name = false;
    wxString name;
    name.Clear();

    for( unsigned int j = 0; j < FileList.GetCount(); j++ ) {
        wxFileName file( FileList.Item( j ) );
        if( ( ( file.GetExt() ).MakeUpper() ) == _T("TXT") ) {
            //  Look for the line beginning with the name of the .000 file
            wxTextFile text_file( file.GetFullPath() );

            bool file_ok = true;
            //  Suppress log messages on bad file reads
            {
                wxLogNull logNo;
                if( !text_file.Open() ) file_ok = false;
            }

            if( file_ok ) {
                wxString str = text_file.GetFirstLine();
                while( !text_file.Eof() ) {
                    if( 0 == target_name.CmpNoCase( str.Mid( 0, target_name.Len() ) ) ) { // found it
                        wxString tname = str.AfterFirst( '-' );
                        name = tname.AfterFirst( ' ' );
                        found_name = true;
                        break;
                    } else {
                        str = text_file.GetNextLine();
                    }
                }
            } else {
                wxString msg( _T("   Error Reading ENC .TXT file: ") );
                msg.Append( file.GetFullPath() );
                wxLogMessage( msg );
            }

            text_file.Close();

            if( found_name ) break;
        }
    }

    Name = name;

}

//---------------------------------------------------------------------------------
//      S57 Database methods
//---------------------------------------------------------------------------------

//-------------------------------
//
// S57 OBJECT ACCESSOR SECTION
//
//-------------------------------

const char *s57chart::getName( OGRFeature *feature )
{
    return feature->GetDefnRef()->GetName();
}

static int ExtensionCompare( const wxString& first, const wxString& second )
{
    wxFileName fn1( first );
    wxFileName fn2( second );
    wxString ext1( fn1.GetExt() );
    wxString ext2( fn2.GetExt() );

    return ext1.Cmp( ext2 );
}

int s57chart::GetUpdateFileArray( const wxFileName file000, wxArrayString *UpFiles )
{
    wxString DirName000 = file000.GetPath( (int) ( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME ) );
    wxDir dir( DirName000 );
    wxString ext;
    wxArrayString *dummy_array;
    int retval = 0;

    if( UpFiles == NULL ) dummy_array = new wxArrayString;
    else
        dummy_array = UpFiles;

    wxString filename;
    bool cont = dir.GetFirst( &filename );
    while( cont ) {
        wxFileName file( filename );
        ext = file.GetExt();

        long tmp;
        //  Files of interest have the same base name is the target .000 cell,
        //  and have numeric extension
        if( ext.ToLong( &tmp ) && ( file.GetName() == file000.GetName() ) ) {
            wxString FileToAdd( DirName000 );
            FileToAdd.Append( file.GetFullName() );

            if( !filename.IsSameAs( _T("CATALOG.031"), false ) )           // don't process catalogs
                    {
//          We must check the update file for validity
//          1.  Is update field DSID:EDTN  equal to base .000 file DSID:EDTN?
//          2.  Is update file DSID.ISDT greater than or equal to base .000 file DSID:ISDT

                wxDateTime umdate;
                wxString sumdate;
                wxString umedtn;
                DDFModule *poModule = new DDFModule();
                if( !poModule->Open( FileToAdd.mb_str() ) ) {
                    wxString msg( _T("   s57chart::BuildS57File  Unable to open update file ") );
                    msg.Append( FileToAdd );
                    wxLogMessage( msg );
                } else {
                    poModule->Rewind();

                    //    Read and parse DDFRecord 0 to get some interesting data
                    //    n.b. assumes that the required fields will be in Record 0....  Is this always true?

                    DDFRecord *pr = poModule->ReadRecord();                              // Record 0
                    //    pr->Dump(stdout);

                    //  Fetch ISDT(Issue Date)
                    char *u = NULL;
                    if( pr ) {
                        u = (char *) ( pr->GetStringSubfield( "DSID", 0, "ISDT", 0 ) );

                        if( u ) {
                            if( strlen( u ) ) sumdate = wxString( u, wxConvUTF8 );
                        }
                    } else {
                        wxString msg(
                                _T("   s57chart::BuildS57File  DDFRecord 0 does not contain DSID:ISDT in update file ") );
                        msg.Append( FileToAdd );
                        wxLogMessage( msg );

                        sumdate = _T("20000101");           // backstop, very early, so wont be used
                    }

                    umdate.ParseFormat( sumdate, _T("%Y%m%d") );
                    if( !umdate.IsValid() ) umdate.ParseFormat( _T("20000101"), _T("%Y%m%d") );

                    umdate.ResetTime();

                    //    Fetch the EDTN(Edition) field
                    if( pr ) {
                        u = NULL;
                        u = (char *) ( pr->GetStringSubfield( "DSID", 0, "EDTN", 0 ) );
                        if( u ) {
                            if( strlen( u ) ) umedtn = wxString( u, wxConvUTF8 );
                        }
                    } else {
                        wxString msg(
                                _T("   s57chart::BuildS57File  DDFRecord 0 does not contain DSID:EDTN in update file ") );
                        msg.Append( FileToAdd );
                        wxLogMessage( msg );

                        umedtn = _T("1");                // backstop
                    }

                    delete poModule;
                }

                if( ( !umdate.IsEarlierThan( m_date000 ) ) && ( umedtn.IsSameAs( m_edtn000 ) ) ) // Note polarity on Date compare....
                dummy_array->Add( FileToAdd );                    // Looking for umdate >= m_date000
            }
        }

        cont = dir.GetNext( &filename );
    }

//      Sort the candidates
    dummy_array->Sort( ExtensionCompare );

//      Get the update number of the last in the list
    if( dummy_array->GetCount() ) {
        wxString Last = dummy_array->Last();
        wxFileName fnl( Last );
        ext = fnl.GetExt();
        retval = atoi( ext.mb_str() );
    }

    if( UpFiles == NULL ) delete dummy_array;

    return retval;
}

int s57chart::ValidateAndCountUpdates( const wxFileName file000, const wxString CopyDir,
        wxString &LastUpdateDate, bool b_copyfiles )
{

    int retval = 0;

    //       wxString DirName000 = file000.GetPath((int)(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));
    //       wxDir dir(DirName000);
    wxArrayString *UpFiles = new wxArrayString;
    retval = GetUpdateFileArray( file000, UpFiles );

    if( UpFiles->GetCount() ) {
        //      The s57reader of ogr requires that update set be sequentially complete
        //      to perform all the updates.  However, some NOAA ENC distributions are
        //      not complete, as apparently some interim updates have been  withdrawn.
        //      Example:  as of 20 Dec, 2005, the update set for US5MD11M.000 includes
        //      US5MD11M.017, ...018, and ...019.  Updates 001 through 016 are missing.
        //
        //      Workaround.
        //      Create temporary dummy update files to fill out the set before invoking
        //      ogr file open/ingest.  Delete after SENC file create finishes.
        //      Set starts with .000, which has the effect of copying the base file to the working dir

        if( b_copyfiles ) {
            m_tmpup_array = new wxArrayString;       // save a list of created files for later erase

            for( int iff = 0; iff < retval + 1; iff++ ) {
                wxFileName ufile( m_FullPath );
                wxString sext;
                sext.Printf( _T("%03d"), iff );
                ufile.SetExt( sext );

                //      Create the target update file name
                wxString cp_ufile = CopyDir;
                if( cp_ufile.Last() != ufile.GetPathSeparator() ) cp_ufile.Append(
                        ufile.GetPathSeparator() );

                cp_ufile.Append( ufile.GetFullName() );

                //      Explicit check for a short update file, possibly left over from a crash...
                int flen = 0;
                if( ufile.FileExists() ) {
                    wxFile uf( ufile.GetFullPath() );
                    if( uf.IsOpened() ) {
                        flen = uf.Length();
                        uf.Close();
                    }
                }

                if( ufile.FileExists() && ( flen > 25 ) )        // a valid update file or base file
                        {
                    //      Copy the valid file to the SENC directory
                    bool cpok = wxCopyFile( ufile.GetFullPath(), cp_ufile );
                    if( !cpok ) {
                        wxString msg( _T("   Cannot copy temporary working ENC file ") );
                        msg.Append( ufile.GetFullPath() );
                        msg.Append( _T(" to ") );
                        msg.Append( cp_ufile );
                        wxLogMessage( msg );
                    }
                }

                else {
                    // Create a dummy ISO8211 file with no real content

                    bool bstat;
                    DDFModule *dupdate = new DDFModule;
                    dupdate->Initialize( '3', 'L', 'E', '1', '0', "!!!", 3, 4, 4 );
                    bstat = !( dupdate->Create( cp_ufile.mb_str() ) == 0 );
                    dupdate->Close();

                    if( !bstat ) {
                        wxString msg( _T("   Error creating dummy update file: ") );
                        msg.Append( cp_ufile );
                        wxLogMessage( msg );
                    }

                }

                m_tmpup_array->Add( cp_ufile );
            }
        }

        //      Extract the date field from the last of the update files
        //      which is by definition a valid, present update file....

        wxFileName lastfile( m_FullPath );
        wxString last_sext;
        last_sext.Printf( _T("%03d"), retval );
        lastfile.SetExt( last_sext );

        bool bSuccess;
        DDFModule oUpdateModule;

//            bSuccess = !(oUpdateModule.Open( m_tmpup_array->Last().mb_str(), TRUE ) == 0);
        bSuccess = !( oUpdateModule.Open( lastfile.GetFullPath().mb_str(), TRUE ) == 0 );

        if( bSuccess ) {
//      Get publish/update date
            oUpdateModule.Rewind();
            DDFRecord *pr = oUpdateModule.ReadRecord();                     // Record 0

            int nSuccess;
            char *u = NULL;

            if( pr ) u = (char *) ( pr->GetStringSubfield( "DSID", 0, "ISDT", 0, &nSuccess ) );

            if( u ) {
                if( strlen( u ) ) {
                    LastUpdateDate = wxString( u, wxConvUTF8 );
                }
            } else {
                wxDateTime now = wxDateTime::Now();
                LastUpdateDate = now.Format( _T("%Y%m%d") );
            }
        }
    }

    delete UpFiles;
    return retval;
}

wxString s57chart::GetISDT( void )
{
    if( m_date000.IsValid() ) return m_date000.Format( _T("%Y%m%d") );
    else
        return _T("Unknown");
}

bool s57chart::GetBaseFileAttr( wxFileName fn )
{
    if( !wxFileName::FileExists( fn.GetFullPath() ) ) return false;

    wxString FullPath000 = fn.GetFullPath();
    DDFModule *poModule = new DDFModule();
    if( !poModule->Open( FullPath000.mb_str() ) ) {
        wxString msg( _T("   s57chart::BuildS57File  Unable to open ") );
        msg.Append( FullPath000 );
        wxLogMessage( msg );
        return false;
    }

    poModule->Rewind();

//    Read and parse DDFRecord 0 to get some interesting data
//    n.b. assumes that the required fields will be in Record 0....  Is this always true?

    DDFRecord *pr = poModule->ReadRecord();                               // Record 0
//    pr->Dump(stdout);

//    Fetch the Geo Feature Count, or something like it....
    m_nGeoRecords = pr->GetIntSubfield( "DSSI", 0, "NOGR", 0 );
    if( !m_nGeoRecords ) {
        wxString msg( _T("   s57chart::BuildS57File  DDFRecord 0 does not contain DSSI:NOGR ") );
        wxLogMessage( msg );

        m_nGeoRecords = 1;                // backstop
    }

//  Use ISDT(Issue Date) here, which is the same as UADT(Updates Applied) for .000 files
    wxString date000;
    char *u = (char *) ( pr->GetStringSubfield( "DSID", 0, "ISDT", 0 ) );
    if( u ) date000 = wxString( u, wxConvUTF8 );
    else {
        wxString msg( _T("   s57chart::BuildS57File  DDFRecord 0 does not contain DSID:ISDT ") );
        wxLogMessage( msg );

        date000 = _T("20000101");             // backstop, very early, so any new files will update?
    }
    m_date000.ParseFormat( date000, _T("%Y%m%d") );
    if( !m_date000.IsValid() ) m_date000.ParseFormat( _T("20000101"), _T("%Y%m%d") );

    m_date000.ResetTime();

//    Fetch the EDTN(Edition) field
    u = (char *) ( pr->GetStringSubfield( "DSID", 0, "EDTN", 0 ) );
    if( u ) m_edtn000 = wxString( u, wxConvUTF8 );
    else {
        wxString msg( _T("   s57chart::BuildS57File  DDFRecord 0 does not contain DSID:EDTN ") );
        wxLogMessage( msg );

        m_edtn000 = _T("1");                // backstop
    }

    m_SE = m_edtn000;

//      Fetch the Native Scale by reading more records until DSPM is found
    m_native_scale = 0;
    for( ; pr != NULL; pr = poModule->ReadRecord() ) {
        if( pr->FindField( "DSPM" ) != NULL ) {
            m_native_scale = pr->GetIntSubfield( "DSPM", 0, "CSCL", 0 );
            break;
        }
    }
    if( !m_native_scale ) {
        wxString msg( _T("   s57chart::BuildS57File  ENC not contain DSPM:CSCL ") );
        wxLogMessage( msg );

        m_native_scale = 1000;                // backstop
    }

    delete poModule;

    return true;
}

int s57chart::BuildSENCFile( const wxString& FullPath000, const wxString& SENCFileName )
{
    OGRFeature *objectDef;
    OGRFeature *pEdgeVectorRecordFeature;
    S57Reader *poReader;
    int feid = 0;

    int nProg = 0;
    wxString nice_name;
    int bbad_update = false;

    wxString msg0( _T("Building SENC file for ") );
    msg0.Append( FullPath000 );
    msg0.Append( _T(" to ") );
    msg0.Append( SENCFileName );
    wxLogMessage( msg0 );

    wxFileName SENCfile = wxFileName( SENCFileName );
    wxFileName file000 = wxFileName( FullPath000 );

    //      Make the target directory if needed
    if( true != SENCfile.DirExists( SENCfile.GetPath() ) ) {
        if( !SENCfile.Mkdir( SENCfile.GetPath() ) ) {
            wxLogMessage(
                    _T("   Cannot create SENC file directory for ") + SENCfile.GetFullPath() );
            return BUILD_SENC_NOK_RETRY;
        }
    }

    GetChartNameFromTXT( FullPath000, nice_name );

    wxFileName tfn;
    wxString tmp_file = tfn.CreateTempFileName( _T("") );

    FILE *fps57;
    const char *pp = "wb";
    fps57 = fopen( tmp_file.mb_str(), pp );

    char t[200];
    strcpy( t, tmp_file.mb_str() );

    if( fps57 == NULL ) {
        wxString msg( _T("   s57chart::BuildS57File  Unable to create temp SENC file ") );
        msg.Append( tfn.GetFullPath() );
        wxLogMessage( msg );
        return 0;
    }

    char temp[201];

    fprintf( fps57, "SENC Version= %d\n", CURRENT_SENC_FORMAT_VERSION );

    strncpy( temp, nice_name.mb_str(), 200 );
    temp[200] = '\0';
    fprintf( fps57, "NAME=%s\n", temp );

    wxString date000 = m_date000.Format( _T("%Y%m%d") );
    strncpy( temp, date000.mb_str(), 200 );
    fprintf( fps57, "DATE000=%s\n", temp );

    strncpy( temp, m_edtn000.mb_str(), 200 );
    fprintf( fps57, "EDTN000=%s\n", temp );

    //      Record .000 file date and size for primitive detection of updates to .000 file
    wxDateTime ModTime000;
    wxString mt = _T("20000101");
    if( file000.GetTimes( NULL, &ModTime000, NULL ) ) mt = ModTime000.Format( _T("%Y%m%d") );
    strncpy( temp, mt.mb_str(), 200 );
    fprintf( fps57, "FILEMOD000=%s\n", temp );

    int size000 = file000.GetSize().GetHi();
    int size000l = file000.GetSize().GetLo();
    fprintf( fps57, "FILESIZE000=%d%d\n", size000, size000l );

    fprintf( fps57, "NOGR=%d\n", m_nGeoRecords );
    fprintf( fps57, "SCALE=%d\n", m_native_scale );

    wxString Message = SENCfile.GetFullPath();
    Message.Append( _T("...Ingesting") );

    wxString Title( _("OpenCPN S57 SENC File Create...") );
    Title.append( SENCfile.GetFullPath() );

    s_ProgDialog = new wxProgressDialog( Title, Message, m_nGeoRecords, NULL,
            wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME
                    | wxPD_SMOOTH | wxSTAY_ON_TOP );

    //      Analyze Updates
    //      The OGR library will apply updates automatically, if enabled.
    //      Alternatively, we can explicitely find and apply updates from any source directory.
    //      We need to keep track of the last sequential update applied, to look out for new updates

    int last_applied_update = 0;
    wxString LastUpdateDate = date000;
    last_applied_update = ValidateAndCountUpdates( file000, SENCfile.GetPath(), LastUpdateDate,
            true );

    wxString msg1;
    msg1.Printf( _T("ENC update number is %3d"), last_applied_update );
    wxLogMessage( msg1 );

    fprintf( fps57, "UPDT=%d\n", last_applied_update );

    strncpy( temp, LastUpdateDate.mb_str(), 200 );
    fprintf( fps57, "DATEUPD=%s\n", temp );

    //      Insert my local error handler to catch OGR errors,
    //      Especially CE_Fatal type errors
    //      Discovered/debugged on US5MD11M.017.  VI 548 geometry deleted
    CPLPushErrorHandler( OpenCPN_OGRErrorHandler );

    bool bcont = true;
    int iObj = 0;
    OGRwkbGeometryType geoType;
    wxString sobj;

    bcont = s_ProgDialog->Update( 1, _T("") );

    //  Here comes the actual ISO8211 file reading
    OGRS57DataSource *poS57DS = new OGRS57DataSource;
    poS57DS->SetS57Registrar( g_poRegistrar );

    //  Set up the options
    char ** papszReaderOptions = NULL;
//    papszReaderOptions = CSLSetNameValue(papszReaderOptions, S57O_LNAM_REFS, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON" );
    poS57DS->SetOptionList( papszReaderOptions );

    //      Open the OGRS57DataSource
    //      This will ingest the .000 file from the working dir, and apply updates

    int open_return = poS57DS->Open( m_tmpup_array->Item( 0 ).mb_str(), TRUE, &s_ProgressCallBack ); ///172
    if( open_return == BAD_UPDATE )         ///172
    bbad_update = true;

    bcont = s_ProgDialog->Update( 2, _T("") );
    if( !bcont ) goto abort_point;

    //      Get a pointer to the reader
    poReader = poS57DS->GetModule( 0 );

//        Prepare Vector Edge Helper table

    /*
     //        First, find the max RCID field by looking at each element in the index in turn

     pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );

     while(NULL != pEdgeVectorRecordFeature)
     {
     //          pEdgeVectorRecordFeature->DumpReadable( stdout );

     //          int record_id = pEdgeVectorRecordFeature-> GetFieldAsInteger( "RCID" );
     //          if(record_id > rcid_max)
     //                rcid_max = record_id;
     feid++;
     pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );

     }

     //    m_nvector_table_size = feid + 1;

     //        Allocate a table
     //    m_pVectorEdgeHelperTable = (int *)calloc((m_nvector_table_size) * sizeof(int), 1);

     */
    //        And fill in the table
    feid = 0;
    pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );
    while( NULL != pEdgeVectorRecordFeature ) {
        int record_id = pEdgeVectorRecordFeature->GetFieldAsInteger( "RCID" );

        m_vector_helper_hash[record_id] = feid;

        feid++;
        delete pEdgeVectorRecordFeature;
        pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );
    }

    //      Update the options, removing the RETURN_PRIMITIVES flags
    //      This flag needed to be set on ingest() to create the proper field defns,
    //      but cleared to fetch normal features

    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF" );
    poReader->SetOptions( papszReaderOptions );
    CSLDestroy( papszReaderOptions );

    while( bcont ) {
        //  Prepare for possible CE_Fatal error in GDAL
        //  Corresponding longjmp() is in the local error handler
        int setjmp_ret = 0;
        setjmp_ret = setjmp( env_ogrf );
        if( setjmp_ret == 1 )             //  CE_Fatal happened in GDAL library
                                          //  in the ReadNextFeature() call below.
                                          //  Seems odd, but that's setjmp/longjmp behaviour
                                          //      Discovered/debugged on US5MD11M.017.  VI 548 geometry deleted

                {
//                TODO need to debug thissssssssss
            wxLogMessage( _T("   s57chart(): GDAL/OGR Fatal Error caught on Obj #%d"), iObj );
        }

        objectDef = poReader->ReadNextFeature();

        if( objectDef != NULL ) {

            iObj++;

//  Update the progress dialog
            sobj = wxString( objectDef->GetDefnRef()->GetName(), wxConvUTF8 );
            wxString idx;
            idx.Printf( _T("  %d/%d       "), iObj, m_nGeoRecords );
            sobj += idx;

            nProg = iObj;
            if( nProg > m_nGeoRecords - 1 ) nProg = m_nGeoRecords - 1;
            if( s_ProgDialog ) bcont = s_ProgDialog->Update( nProg, sobj );

            geoType = wkbUnknown;
//      This test should not be necessary for real (i.e not C_AGGR) features
//      However... some update files contain errors, and have deleted some
//      geometry without deleting the corresponding feature(s).
//      So, GeometryType becomes Unknown.
//      e.g. US5MD11M.017
//      In this case, all we can do is skip the feature....sigh.

            if( objectDef->GetGeometryRef() != NULL ) geoType =
                    objectDef->GetGeometryRef()->getGeometryType();

//      n.b  This next line causes skip of C_AGGR features w/o geometry
            if( geoType != wkbUnknown )                             // Write only if has wkbGeometry
                    {
                CreateSENCRecord( objectDef, fps57, 1, poReader );
            }

            delete objectDef;

        } else
            break;

    }

    if( bcont ) {
        //      Create and write the Vector Edge Table
        CreateSENCVectorEdgeTable( fps57, poReader );

        //      Create and write the Connected NodeTable
        CreateSENCConnNodeTable( fps57, poReader );
    }

    abort_point: delete poS57DS;

    delete s_ProgDialog;
    s_ProgDialog = NULL;

    fclose( fps57 );

    CPLPopErrorHandler();

    //  Delete any temporary (working) real and dummy update files,
    //  as well as .000 file created by ValidateAndCountUpdates()
    if( m_tmpup_array ) {
        for( unsigned int iff = 0; iff < m_tmpup_array->GetCount(); iff++ )
            remove( m_tmpup_array->Item( iff ).mb_str() );
        delete m_tmpup_array;
    }

    int ret_code = 0;

    if( !bcont )                // aborted
    {
        wxRemoveFile( tmp_file );                     // kill the temp file
        ret_code = BUILD_SENC_NOK_RETRY;
    }

    if( bcont ) {
        remove( SENCfile.GetFullPath().mb_str() );
        unlink( SENCfile.GetFullPath().mb_str() );       //  Delete any existing SENC file....

        bool cpok = wxCopyFile( tmp_file, SENCfile.GetFullPath() );         // Delete temp file too?
        wxRemoveFile( tmp_file );

        if( !cpok ) {
            wxString msg( _T("   Cannot copy temporary SENC file ") );
            msg.Append( tmp_file );
            msg.Append( _T(" to ") );
            msg.Append( SENCfile.GetFullPath() );
            wxLogMessage( msg );
            ret_code = BUILD_SENC_NOK_RETRY;
        } else
            ret_code = BUILD_SENC_OK;

    }

    if( bbad_update ) OCPNMessageBox(
            _T("Errors encountered processing ENC update file(s).\nENC features may be incomplete or inaccurate."),
            _T("OpenCPN Create SENC"), wxOK | wxICON_EXCLAMATION );

    return ret_code;
}

int s57chart::BuildRAZFromSENCFile( const wxString& FullPath )
{
    int ret_val = 0;                    // default is OK

    //    Sanity check for existence of file
    wxFileName SENCFileName( FullPath );
    if( !SENCFileName.FileExists() ) {
        wxString msg( _T("   Cannot open SENC file ") );
        msg.Append( SENCFileName.GetFullPath() );
        wxLogMessage( msg );

        return 1;
    }

    int nProg = 0;

    wxString ifs( FullPath );

    wxFileInputStream fpx_u( ifs );
    wxBufferedInputStream fpx( fpx_u );

    int MAX_LINE = 499999;
    char *buf = (char *) malloc( MAX_LINE + 1 );

    LUPrec *LUP;
    LUPname LUP_Name = PAPER_CHART;

    int nGeoFeature;

    int object_count = 0;

    OGREnvelope Envelope;

    int dun = 0;

    hdr_buf = (char *) malloc( 1 );
    wxProgressDialog *SENC_prog = NULL;
    int nGeo1000;
    wxString date_000, date_upd;

//        if(my_fgets(buf, MAX_LINE, fpx) == 0)
//           dun = 1;

    while( !dun ) {

        if( my_fgets( buf, MAX_LINE, fpx ) == 0 ) {
            dun = 1;
            break;
        }

        if( !strncmp( buf, "OGRF", 4 ) ) {

            S57Obj *obj = new S57Obj( buf, &fpx, 0, 0 );
            if( obj ) {

//      Build/Maintain the ATON floating/rigid arrays
                if( GEO_POINT == obj->Primitive_type ) {

// set floating platform
                    if( ( !strncmp( obj->FeatureName, "LITFLT", 6 ) )
                            || ( !strncmp( obj->FeatureName, "LITVES", 6 ) )
                            || ( !strncmp( obj->FeatureName, "BOY", 3 ) ) ) {
                        pFloatingATONArray->Add( obj );
                    }

// set rigid platform
                    if( !strncmp( obj->FeatureName, "BCN", 3 ) ) {
                        pRigidATONArray->Add( obj );
                    }

                    //    Mark the object as an ATON
                    if( ( !strncmp( obj->FeatureName, "LIT", 3 ) )
                            || ( !strncmp( obj->FeatureName, "LIGHTS", 6 ) )
                            || ( !strncmp( obj->FeatureName, "BCN", 3 ) )
                            || ( !strncmp( obj->FeatureName, "BOY", 3 ) ) ) {
                        obj->bIsAton = true;
                    }

                }

//      This is where Simplified or Paper-Type point features are selected
                switch( obj->Primitive_type ){
                    case GEO_POINT:
                    case GEO_META:
                    case GEO_PRIM:

                        if( PAPER_CHART == ps52plib->m_nSymbolStyle ) LUP_Name = PAPER_CHART;
                        else
                            LUP_Name = SIMPLIFIED;

                        break;

                    case GEO_LINE:
                        LUP_Name = LINES;
                        break;

                    case GEO_AREA:
                        if( PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle ) LUP_Name =
                                PLAIN_BOUNDARIES;
                        else
                            LUP_Name = SYMBOLIZED_BOUNDARIES;

                        break;
                }

                LUP = ps52plib->S52_LUPLookup( LUP_Name, obj->FeatureName, obj );

                if( NULL == LUP ) {
                    if( g_bDebugS57 ) {
                        wxString msg( obj->FeatureName, wxConvUTF8 );
                        msg.Prepend( _T("   Could not find LUP for ") );
                        LogMessageOnce( msg );
                    }
                    delete obj;
                } else {
//              Convert LUP to rules set
                    ps52plib->_LUP2rules( LUP, obj );

//              Add linked object/LUP to the working set
                    _insertRules( obj, LUP, this );

//              Establish Object's Display Category
                    obj->m_DisplayCat = LUP->DISC;
                }
            }

            object_count++;

            if( ( object_count % 500 ) == 0 ) {
                nProg = object_count / 500;
                if( nProg > nGeo1000 - 1 ) nProg = nGeo1000 - 1;

                if( SENC_prog ) SENC_prog->Update( nProg );
            }

            continue;

        }               //OGRF

        else if( !strncmp( buf, "VETableStart", 12 ) ) {
            //    Use a wxArray for temp storage
            //    then transfer to a simple linear array
            ArrayOfVE_Elements ve_array;

            int index = -1;
            int index_max = -1;
            int count;

            fpx.Read( &index, sizeof(int) );

            while( -1 != index ) {
                fpx.Read( &count, sizeof(int) );

                double *pPoints = NULL;
                if( count ) {
                    pPoints = (double *) malloc( count * 2 * sizeof(double) );
                    fpx.Read( pPoints, count * 2 * sizeof(double) );
                }

                VE_Element vee;
                vee.index = index;
                vee.nCount = count;
                vee.pPoints = pPoints;
                vee.max_priority = -99;            // Default

                ve_array.Add( vee );

                if( index > index_max ) index_max = index;

                //    Next element
                fpx.Read( &index, sizeof(int) );
            }

            //    Create a hash map of VE_Element pointers as a chart class member
            int n_ve_elements = ve_array.GetCount();

            for( int i = 0; i < n_ve_elements; i++ ) {
                VE_Element ve_from_array = ve_array.Item( i );
                VE_Element *vep = new VE_Element;
                vep->index = ve_from_array.index;
                vep->nCount = ve_from_array.nCount;
                vep->pPoints = ve_from_array.pPoints;

//                        m_pve_array[vep->index] = vep;
                m_ve_hash[vep->index] = vep;

            }

        }

        else if( !strncmp( buf, "VCTableStart", 12 ) ) {
            //    Use a wxArray for temp storage
            //    then transfer to a simple linear array
            ArrayOfVC_Elements vc_array;

            int index = -1;
            int index_max = -1;

            fpx.Read( &index, sizeof(int) );

            while( -1 != index ) {

                double *pPoint = NULL;
                pPoint = (double *) malloc( 2 * sizeof(double) );
                fpx.Read( pPoint, 2 * sizeof(double) );

                VC_Element vce;
                vce.index = index;
                vce.pPoint = pPoint;

                vc_array.Add( vce );

                if( index > index_max ) index_max = index;

                //    Next element
                fpx.Read( &index, sizeof(int) );
            }

            //    Create a hash map VC_Element pointers as a chart class member
            int n_vc_elements = vc_array.GetCount();

            for( int i = 0; i < n_vc_elements; i++ ) {
                VC_Element vc_from_array = vc_array.Item( i );
                VC_Element *vcp = new VC_Element;
                vcp->index = vc_from_array.index;
                vcp->pPoint = vc_from_array.pPoint;

                m_vc_hash[vcp->index] = vcp;
            }
        }

        else if( !strncmp( buf, "SENC", 4 ) ) {
            int senc_file_version;
            sscanf( buf, "SENC Version=%i", &senc_file_version );
            if( senc_file_version != CURRENT_SENC_FORMAT_VERSION ) {
                wxString msg( _T("   Wrong version on SENC file ") );
                msg.Append( SENCFileName.GetFullPath() );
                wxLogMessage( msg );

                dun = 1;
                ret_val = 1;                   // error
            }
        }

        else if( !strncmp( buf, "DATEUPD", 7 ) ) {
            date_upd.Append( wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' ) );
        }

        else if( !strncmp( buf, "DATE000", 7 ) ) {
            date_000.Append( wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' ) );
        }

        else if( !strncmp( buf, "SCALE", 5 ) ) {
            int ins;
            sscanf( buf, "SCALE=%d", &ins );
            m_Chart_Scale = ins;
        }

        else if( !strncmp( buf, "NAME", 4 ) ) {
            m_Name = wxString( &buf[5], wxConvUTF8 ).BeforeFirst( '\n' );
        }

        else if( !strncmp( buf, "NOGR", 4 ) ) {
            sscanf( buf, "NOGR=%d", &nGeoFeature );

            nGeo1000 = nGeoFeature / 500;

#ifdef __WXMSW__
            wxBeginBusyCursor();
            /*
             SENC_prog = new wxProgressDialog(  _("OpenCPN S57 SENC File Load"), FullPath, nGeo1000, NULL,
             wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME | wxPD_SMOOTH);
             */
            wxEndBusyCursor();

#endif
        }
    }                       //while(!dun)

//      fclose(fpx);

    free( buf );

    free( hdr_buf );

    delete SENC_prog;

    //   Decide on pub date to show

    int d000 = atoi(
            ( date_000/*(wxString((const wchar_t *)date_000, wxConvUTF8)*/.Mid( 0, 4 ) ).mb_str() );
    int dupd = atoi(
            ( date_upd/*(wxString((const wchar_t *)date_upd, wxConvUTF8)*/.Mid( 0, 4 ) ).mb_str() );

    if( dupd > d000 ) m_PubYear = date_upd/*wxString((const wchar_t *)date_upd, wxConvUTF8)*/.Mid(
            0, 4 );
    else
        m_PubYear = date_000/*wxString((const wchar_t *)date_000, wxConvUTF8)*/.Mid( 0, 4 );

    //    Set some base class values
    wxDateTime upd;
    upd.ParseFormat( date_upd, _T("%Y%m%d") );
    if( !upd.IsValid() ) upd.ParseFormat( _T("20000101"), _T("%Y%m%d") );

    upd.ResetTime();
    m_EdDate = upd;

    m_SE = m_edtn000;
    m_datum_str = _T("WGS84");

    m_SoundingsDatum = _T("MEAN LOWER LOW WATER");
    m_ID = SENCFileName.GetName();

    // Validate hash maps....

    ObjRazRules *top;
    ObjRazRules *nxx;

    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            top = razRules[i][j];
            while( top != NULL ) {
                S57Obj *obj = top->obj;

///
                for( int iseg = 0; iseg < obj->m_n_lsindex; iseg++ ) {
                    int seg_index = iseg * 3;
                    int *index_run = &obj->m_lsindex_array[seg_index];

                    //  Get first connected node
                    int inode = *index_run++;
                    if( ( inode >= 0 ) ) {
                        if( m_vc_hash.find( inode ) == m_vc_hash.end() ) {
                            //    Must be a bad index in the SENC file
                            //    Stuff a recognizable flag to indicate invalidity
                            index_run--;
                            *index_run = -1;
                            index_run++;
                        }
                    }

                    //  Get the edge
//                              int enode = *index_run++;
                    index_run++;

                    //  Get last connected node
                    int jnode = *index_run++;
                    if( ( jnode >= 0 ) ) {
                        if( m_vc_hash.find( jnode ) == m_vc_hash.end() ) {
                            //    Must be a bad index in the SENC file
                            //    Stuff a recognizable flag to indicate invalidity
                            index_run--;
                            *index_run = -2;
                            index_run++;
                        }

                    }
                }
///
                nxx = top->next;
                top = nxx;
            }
        }
    }

    return ret_val;
}

//------------------------------------------------------------------------------
//      Local version of fgets for Binary Mode (SENC) file
//------------------------------------------------------------------------------
int s57chart::my_fgets( char *buf, int buf_len_max, wxInputStream& ifs )

{
    char chNext;
    int nLineLen = 0;
    char *lbuf;

    lbuf = buf;

    while( !ifs.Eof() && nLineLen < buf_len_max ) {
        chNext = (char) ifs.GetC();

        /* each CR/LF (or LF/CR) as if just "CR" */
        if( chNext == 10 || chNext == 13 ) {
            chNext = '\n';
        }

        *lbuf = chNext;
        lbuf++, nLineLen++;

        if( chNext == '\n' ) {
            *lbuf = '\0';
            return nLineLen;
        }
    }

    *( lbuf ) = '\0';

    return nLineLen;
}

int s57chart::_insertRules( S57Obj *obj, LUPrec *LUP, s57chart *pOwner )
{
    ObjRazRules *rzRules = NULL;
    int disPrioIdx = 0;
    int LUPtypeIdx = 0;

    if( LUP == NULL ) {
//      printf("SEQuencer:_insertRules(): ERROR no rules to insert!!\n");
        return 0;
    }

    // find display priority index       --talky version
    switch( LUP->DPRI ){
        case PRIO_NODATA:
            disPrioIdx = 0;
            break;  // no data fill area pattern
        case PRIO_GROUP1:
            disPrioIdx = 1;
            break;  // S57 group 1 filled areas
        case PRIO_AREA_1:
            disPrioIdx = 2;
            break;  // superimposed areas
        case PRIO_AREA_2:
            disPrioIdx = 3;
            break;  // superimposed areas also water features
        case PRIO_SYMB_POINT:
            disPrioIdx = 4;
            break;  // point symbol also land features
        case PRIO_SYMB_LINE:
            disPrioIdx = 5;
            break;  // line symbol also restricted areas
        case PRIO_SYMB_AREA:
            disPrioIdx = 6;
            break;  // area symbol also traffic areas
        case PRIO_ROUTEING:
            disPrioIdx = 7;
            break;  // routeing lines
        case PRIO_HAZARDS:
            disPrioIdx = 8;
            break;  // hazards
        case PRIO_MARINERS:
            disPrioIdx = 9;
            break;  // VRM & EBL, own ship
        default:
            printf( "SEQuencer:_insertRules():ERROR no display priority!!!\n" );
    }

    // find look up type index
    switch( LUP->TNAM ){
        case SIMPLIFIED:
            LUPtypeIdx = 0;
            break; // points
        case PAPER_CHART:
            LUPtypeIdx = 1;
            break; // points
        case LINES:
            LUPtypeIdx = 2;
            break; // lines
        case PLAIN_BOUNDARIES:
            LUPtypeIdx = 3;
            break; // areas
        case SYMBOLIZED_BOUNDARIES:
            LUPtypeIdx = 4;
            break; // areas
        default:
            printf( "SEQuencer:_insertRules():ERROR no look up type !!!\n" );
    }

    // insert rules
    rzRules = (ObjRazRules *) malloc( sizeof(ObjRazRules) );
    rzRules->obj = obj;
    obj->nRef++;                         // Increment reference counter for delete check;
    rzRules->LUP = LUP;
    rzRules->chart = pOwner;
    rzRules->next = razRules[disPrioIdx][LUPtypeIdx];
    rzRules->child = NULL;
    razRules[disPrioIdx][LUPtypeIdx] = rzRules;

    return 1;
}

void s57chart::ResetPointBBoxes( const ViewPort &vp_last, const ViewPort &vp_this )
{
    ObjRazRules *top;
    ObjRazRules *nxx;

    double box_margin = 0.25;

    //    Assume a 50x50 pixel box
    box_margin = ( 50. / vp_this.view_scale_ppm ) / ( 1852. * 60. );  //degrees

    for( int i = 0; i < PRIO_NUM; ++i ) {
        top = razRules[i][0];

        while( top != NULL ) {
            if( !top->obj->geoPtMulti )                      // do not reset multipoints
            {
                top->obj->bBBObj_valid = false;
                top->obj->BBObj.SetMin( top->obj->m_lon - box_margin,
                        top->obj->m_lat - box_margin );
                top->obj->BBObj.SetMax( top->obj->m_lon + box_margin,
                        top->obj->m_lat + box_margin );
            }

            nxx = top->next;
            top = nxx;
        }

        top = razRules[i][1];

        while( top != NULL ) {
            if( !top->obj->geoPtMulti )                      // do not reset multipoints
            {
                top->obj->bBBObj_valid = false;
                top->obj->BBObj.SetMin( top->obj->m_lon - box_margin,
                        top->obj->m_lat - box_margin );
                top->obj->BBObj.SetMax( top->obj->m_lon + box_margin,
                        top->obj->m_lat + box_margin );
            }

            nxx = top->next;
            top = nxx;
        }
    }
}

//      Traverse the ObjRazRules tree, and fill in
//      any Lups/rules not linked on initial chart load.
//      For example, if chart was loaded with PAPER_CHART symbols,
//      locate and load the equivalent SIMPLIFIED symbology.
//      Likewise for PLAIN/SYMBOLIZED boundaries.
//
//      This method is usually called after a chart display style
//      change via the "Options" dialog, to ensure all symbology is
//      present iff needed.

void s57chart::UpdateLUPs( s57chart *pOwner )
{
    ObjRazRules *top;
    ObjRazRules *nxx;
    LUPrec *LUP;

    for( int i = 0; i < PRIO_NUM; ++i ) {
        //  SIMPLIFIED is set, PAPER_CHART is bare
        if( ( razRules[i][0] ) && ( NULL == razRules[i][1] ) ) {
            top = razRules[i][0];

            while( top != NULL ) {
                LUP = ps52plib->S52_LUPLookup( PAPER_CHART, top->obj->FeatureName, top->obj );
                if( LUP ) {
                    //  A POINT object can only appear in two places in the table, SIMPLIFIED or PAPER_CHART
                    //  although it is allowed for the Display priority to be different for each
                    if(top->obj->nRef < 2) {
                        ps52plib->_LUP2rules( LUP, top->obj );
                        _insertRules( top->obj, LUP, pOwner );
                        top->obj->m_DisplayCat = LUP->DISC;
                    }
                }

                nxx = top->next;
                top = nxx;
            }
        }

        //  PAPER_CHART is set, SIMPLIFIED is bare
        if( ( razRules[i][1] ) && ( NULL == razRules[i][0] ) ) {
            top = razRules[i][1];

            while( top != NULL ) {
                LUP = ps52plib->S52_LUPLookup( SIMPLIFIED, top->obj->FeatureName, top->obj );
                if( LUP ) {
                    if(top->obj->nRef < 2) {
                        ps52plib->_LUP2rules( LUP, top->obj );
                        _insertRules( top->obj, LUP, pOwner );
                        top->obj->m_DisplayCat = LUP->DISC;
                    }
                }

                nxx = top->next;
                top = nxx;
            }
        }

        //  PLAIN_BOUNDARIES is set, SYMBOLIZED_BOUNDARIES is bare
        if( ( razRules[i][3] ) && ( NULL == razRules[i][4] ) ) {
            top = razRules[i][3];

            while( top != NULL ) {
                LUP = ps52plib->S52_LUPLookup( SYMBOLIZED_BOUNDARIES, top->obj->FeatureName,
                        top->obj );
                if( LUP ) {
                    ps52plib->_LUP2rules( LUP, top->obj );
                    _insertRules( top->obj, LUP, pOwner );
                    top->obj->m_DisplayCat = LUP->DISC;
                }

                nxx = top->next;
                top = nxx;
            }
        }

        //  SYMBOLIZED_BOUNDARIES is set, PLAIN_BOUNDARIES is bare
        if( ( razRules[i][4] ) && ( NULL == razRules[i][3] ) ) {
            top = razRules[i][4];

            while( top != NULL ) {
                LUP = ps52plib->S52_LUPLookup( PLAIN_BOUNDARIES, top->obj->FeatureName, top->obj );
                if( LUP ) {
                    ps52plib->_LUP2rules( LUP, top->obj );
                    _insertRules( top->obj, LUP, pOwner );
                    top->obj->m_DisplayCat = LUP->DISC;
                }

                nxx = top->next;
                top = nxx;
            }
        }

        //  Traverse this priority level again,
        //  clearing any object CS rules and flags,
        //  so that the next render operation will re-evaluate the CS

        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            top = razRules[i][j];
            while( top != NULL ) {
                top->obj->bCS_Added = 0;

                nxx = top->next;
                top = nxx;
            }
        }

        //  Traverse this priority level again,
        //  clearing any object CS rules and flags of any child list,
        //  so that the next render operation will re-evaluate the CS

        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            top = razRules[i][j];
            while( top != NULL ) {
                if( top->child ) {
                    ObjRazRules *ctop = top->child;
                    while( NULL != ctop ) {
                        ctop->obj->bCS_Added = 0;
                        ctop = ctop->next;
                    }
                }
                nxx = top->next;
                top = nxx;
            }
        }

    }

    //    Clear the dynamically created Conditional Symbology LUP Array
    // This can not be done on a per-chart basis, since the plib services all charts
    // TODO really should make the dynamic LUPs belong to the chart class that created them
}

void s57chart::CreateSENCRecord( OGRFeature *pFeature, FILE * fpOut, int mode, S57Reader *poReader )
{

#define MAX_HDR_LINE    400

    char line[MAX_HDR_LINE + 1];
    wxString sheader;

    fprintf( fpOut, "OGRFeature(%s):%ld\n", pFeature->GetDefnRef()->GetName(), pFeature->GetFID() );

//      In the interests of output file size, DO NOT report fields that are not set.
    for( int iField = 0; iField < pFeature->GetFieldCount(); iField++ ) {
        if( pFeature->IsFieldSet( iField ) ) {
            if( ( iField == 1 ) || ( iField > 7 ) ) {
                OGRFieldDefn *poFDefn = pFeature->GetDefnRef()->GetFieldDefn( iField );

                const char *pType = OGRFieldDefn::GetFieldTypeName( poFDefn->GetType() );

                snprintf( line, MAX_HDR_LINE - 2, "  %s (%c) = %s", poFDefn->GetNameRef(), *pType,
                        pFeature->GetFieldAsString( iField ) );

				// Attempt different conversions to accomodate different language encodings in
				// theoriginal files.

				wxString wxAttrValue( line, wxConvUTF8 );

				if( wxAttrValue.Length() < strlen(line) )
					wxAttrValue = wxString( line, wxConvISO8859_1 );

				if( wxAttrValue.Length() < strlen(line) )
					wxLogError( _T("Warning: CreateSENCRecord(): Failed to convert string value to wxString.") );

                sheader += wxAttrValue;
                sheader += '\n';
            }
        }
    }

    OGRGeometry *pGeo = pFeature->GetGeometryRef();

//    Special geometry cases
///172
    if( wkbPoint == pGeo->getGeometryType() ) {
        OGRPoint *pp = (OGRPoint *) pGeo;
        int nqual = pp->getnQual();
        if( 10 != nqual )                    // only add attribute if nQual is not "precisely known"
                {
            snprintf( line, MAX_HDR_LINE - 2, "  %s (%c) = %d", "QUALTY", 'I', nqual );
            sheader += wxString( line, wxConvUTF8 );
            sheader += '\n';
        }

    }

    if( mode == 1 ) {
        sprintf( line, "  %s %g %g\n", pGeo->getGeometryName(), ref_lat, ref_lon );
        sheader += wxString( line, wxConvUTF8 );
    }
    fprintf( fpOut, "HDRLEN=%lu\n", (unsigned long) sheader.Len() );
    fwrite( sheader.mb_str( wxConvUTF8 ), 1, sheader.Len(), fpOut );

    if( ( pGeo != NULL ) /*&& (mode == 1)*/) {
        int wkb_len = pGeo->WkbSize();
        unsigned char *pwkb_buffer = (unsigned char *) malloc( wkb_len );

//  Get the GDAL data representation
        pGeo->exportToWkb( wkbNDR, pwkb_buffer );

        //  Convert to opencpn SENC representation

        //  Set absurd bbox starting limits
        float lonmax = -1000;
        float lonmin = 1000;
        float latmax = -1000;
        float latmin = 1000;

        int i, ip, sb_len;
        float *pdf;
        double *psd;
        unsigned char *ps;
        unsigned char *pd;
        unsigned char *psb_buffer;
        double lat, lon;
        int nPoints;
        wxString msg;

        OGRwkbGeometryType gType = pGeo->getGeometryType();
        switch( gType ){
            case wkbLineString: {
                sb_len = ( ( wkb_len - 9 ) / 2 ) + 9 + 16;  // data will be 4 byte float, not double
                                                            // and bbox limits are tacked on end
                fprintf( fpOut, "  %d\n", sb_len );

                psb_buffer = (unsigned char *) malloc( sb_len );
                pd = psb_buffer;
                ps = pwkb_buffer;

                memcpy( pd, ps, 9 );                                  // byte order, type, and count

                ip = *( (int *) ( ps + 5 ) );                              // point count

                pd += 9;
                ps += 9;
                psd = (double *) ps;
                pdf = (float *) pd;

                for( i = 0; i < ip; i++ )                           // convert doubles to floats
                        {                                                 // computing bbox as we go

                    float lon = (float) *psd++;
                    float lat = (float) *psd++;

                    //  Calculate SM from chart common reference point
                    double easting, northing;
                    toSM( lat, lon, ref_lat, ref_lon, &easting, &northing );

                    *pdf++ = easting;
                    *pdf++ = northing;

                    lonmax = fmax(lon, lonmax);
                    lonmin = fmin(lon, lonmin);
                    latmax = fmax(lat, latmax);
                    latmin = fmin(lat, latmin);

                }

                //      Store the Bounding Box as lat/lon
                *pdf++ = lonmax;
                *pdf++ = lonmin;
                *pdf++ = latmax;
                *pdf = latmin;

                fwrite( psb_buffer, 1, sb_len, fpOut );
                fprintf( fpOut, "\n" );
                free( psb_buffer );

//    Capture the Vector Table geometry indices
                int *pNAME_RCID;
                int *pORNT;
                int nEdgeVectorRecords;
                OGRFeature *pEdgeVectorRecordFeature;

                pNAME_RCID = (int *) pFeature->GetFieldAsIntegerList( "NAME_RCID",
                        &nEdgeVectorRecords );

                pORNT = (int *) pFeature->GetFieldAsIntegerList( "ORNT", NULL );

                fprintf( fpOut, "LSINDEXLIST %d\n", nEdgeVectorRecords );
//                    fwrite(pNAME_RCID, 1, nEdgeVectorRecords * sizeof(int), fpOut);

                //  Set up the options, adding RETURN_PRIMITIVES
                char ** papszReaderOptions = NULL;
                papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
                papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES,
                        "ON" );
                papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,
                        "ON" );
                poReader->SetOptions( papszReaderOptions );

//    Capture the beginning and end point connected nodes for each edge vector record
                for( i = 0; i < nEdgeVectorRecords; i++ ) {
                    int start_rcid, end_rcid;
                    int target_record_feid = m_vector_helper_hash[pNAME_RCID[i]];
                    pEdgeVectorRecordFeature = poReader->ReadVector( target_record_feid, RCNM_VE );

                    if( NULL != pEdgeVectorRecordFeature ) {
                        start_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_0" );
                        end_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_1" );

                        //    Make sure the start and end points exist....
                        //    Note this poReader method was converted to Public access to
                        //     facilitate this test.  There might be another clean way....
                        //    Problem first found on Holand ENC 1R5YM009.000
                        if( !poReader->FetchPoint( RCNM_VC, start_rcid, NULL, NULL, NULL, NULL ) ) start_rcid =
                                -1;
                        if( !poReader->FetchPoint( RCNM_VC, end_rcid, NULL, NULL, NULL, NULL ) ) end_rcid =
                                -2;

                        int edge_ornt = 1;
                        //  Allocate some storage for converted points

                        if( edge_ornt == 1 )                                    // forward
                                {
                            fwrite( &start_rcid, 1, sizeof(int), fpOut );
                            fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                            fwrite( &end_rcid, 1, sizeof(int), fpOut );
                        } else                                                  // reverse
                        {
                            fwrite( &end_rcid, 1, sizeof(int), fpOut );
                            fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                            fwrite( &start_rcid, 1, sizeof(int), fpOut );
                        }

                        delete pEdgeVectorRecordFeature;
                    } else {
                        start_rcid = -1;                                    // error indication
                        end_rcid = -2;

                        fwrite( &start_rcid, 1, sizeof(int), fpOut );
                        fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                        fwrite( &end_rcid, 1, sizeof(int), fpOut );
                    }
                }

                fprintf( fpOut, "\n" );

                //  Reset the options
                papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,
                        "OFF" );
                poReader->SetOptions( papszReaderOptions );
                CSLDestroy( papszReaderOptions );

//                    free(pVectorEdgeHelperTable);

                break;
            }
            case wkbMultiLineString:
                msg = _T("   Warning: Unimplemented SENC wkbMultiLineString record in file ");
                msg.Append( m_SENCFileName.GetFullPath() );
                wxLogMessage( msg );

                wkb_len = pGeo->WkbSize();
                fprintf( fpOut, "  %d\n", wkb_len );
                fwrite( pwkb_buffer, 1, wkb_len, fpOut );

                break;

            case wkbPoint: {
                int nq_len = 4;                                     // nQual length
//                    int nqual = *(int *)(pwkb_buffer + 5);              // fetch nqual

                sb_len = ( ( wkb_len - ( 5 + nq_len ) ) / 2 ) + 5; // data will be 4 byte float, not double
                                                                   // and skipping nQual

                fprintf( fpOut, "  %d\n", sb_len );

                psb_buffer = (unsigned char *) malloc( sb_len );
                pd = psb_buffer;
                ps = pwkb_buffer;

                memcpy( pd, ps, 5 );                                 // byte order, type

                pd += 5;
                ps += 5 + nq_len;
                psd = (double *) ps;
                pdf = (float *) pd;

                lon = *psd++;                                      // fetch the point
                lat = *psd;

//                    if(314 == pFeature->GetFID())
//                          int yyp = 4;

                //  Calculate SM from chart common reference point
                double easting, northing;
                toSM( lat, lon, ref_lat, ref_lon, &easting, &northing );

                *pdf++ = easting;
                *pdf = northing;

                //  And write it out
                fwrite( psb_buffer, 1, sb_len, fpOut );
                fprintf( fpOut, "\n" );

                free( psb_buffer );

                break;
            }

            case wkbMultiPoint25D:
                ps = pwkb_buffer;
                ps += 5;
                nPoints = *( (int *) ps );                     // point count

                sb_len = ( 9 + nPoints * 3 * sizeof(float) ) + 16; // GTYPE and count, points as floats
                                                                   // and trailing bbox
                fprintf( fpOut, "  %d\n", sb_len );

                psb_buffer = (unsigned char *) malloc( sb_len );
                pd = psb_buffer;
                ps = pwkb_buffer;

                memcpy( pd, ps, 9 );                                  // byte order, type, count

                ps += 9;
                pd += 9;

                pdf = (float *) pd;

                for( ip = 0; ip < nPoints; ip++ ) {

                    // Workaround a bug?? in OGRGeometryCollection
                    // While exporting point geometries serially, OGRPoint->exportToWkb assumes that
                    // if Z is identically 0, then the point must be a 2D point only.
                    // So, the collection Wkb is corrupted with some 3D, and some 2D points.
                    // Workaround:  Get reference to the points serially, and explicitly read X,Y,Z
                    // Ignore the previously read Wkb buffer

                    OGRGeometryCollection *temp_geometry_collection = (OGRGeometryCollection *) pGeo;
                    OGRGeometry *temp_geometry = temp_geometry_collection->getGeometryRef( ip );
                    OGRPoint *pt_geom = (OGRPoint *) temp_geometry;

                    lon = pt_geom->getX();
                    lat = pt_geom->getY();
                    double depth = pt_geom->getZ();

                    /*
                     ps += 5;
                     psd = (double *)ps;

                     lon = *psd++;
                     lat = *psd++;
                     double depth = *psd;
                     */

                    //  Calculate SM from chart common reference point
                    double easting, northing;
                    toSM( lat, lon, ref_lat, ref_lon, &easting, &northing );

                    *pdf++ = easting;
                    *pdf++ = northing;
                    *pdf++ = (float) depth;

//                        ps += 3 * sizeof(double);

                    //  Keep a running calculation of min/max
                    lonmax = fmax(lon, lonmax);
                    lonmin = fmin(lon, lonmin);
                    latmax = fmax(lat, latmax);
                    latmin = fmin(lat, latmin);
                }

                //      Store the Bounding Box as lat/lon
                *pdf++ = lonmax;
                *pdf++ = lonmin;
                *pdf++ = latmax;
                *pdf = latmin;

                //  And write it out
                fwrite( psb_buffer, 1, sb_len, fpOut );
                free( psb_buffer );
                fprintf( fpOut, "\n" );

                break;

                //      Special case, polygons are handled separately
            case wkbPolygon: {
                int error_code;
                PolyTessGeo *ppg = NULL;

                OGRPolygon *poly = (OGRPolygon *) ( pGeo );

                ppg = new PolyTessGeo( poly, true, ref_lat, ref_lon, 0 );   //try to use glu library

                error_code = ppg->ErrorCode;
                if( error_code == ERROR_NO_DLL ) {
                    if( !bGLUWarningSent ) {
                        wxLogMessage(
                                _T("   Warning...Could not find glu32.dll, trying internal tess.") );
                        bGLUWarningSent = true;
                    }

                    delete ppg;
                    //  Try with internal tesselator
                    ppg = new PolyTessGeo( poly, true, ref_lat, ref_lon, 1 );
                    error_code = ppg->ErrorCode;
                }

                if( error_code ) wxLogMessage( _T("   Error: S57 SENC Create Error %d"),
                        ppg->ErrorCode );

                else
                    ppg->Write_PolyTriGroup( fpOut );
                delete ppg;

                //    Capture the Vector Table geometry indices

                int *pNAME_RCID;
                int nEdgeVectorRecords;
                OGRFeature *pEdgeVectorRecordFeature;

                pNAME_RCID = (int *) pFeature->GetFieldAsIntegerList( "NAME_RCID",
                        &nEdgeVectorRecords );

                fprintf( fpOut, "LSINDEXLIST %d\n", nEdgeVectorRecords );

                //  Set up the options, adding RETURN_PRIMITIVES
                char ** papszReaderOptions = NULL;
                papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
                papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES,
                        "ON" );
                papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,
                        "ON" );
                poReader->SetOptions( papszReaderOptions );

//    Capture the beginning and end point connected nodes for each edge vector record
                for( i = 0; i < nEdgeVectorRecords; i++ ) {
                    int start_rcid, end_rcid;
                    /*
                     //  Filter out absurd RCIDs, found first on cell IT300017.000
                     if((pNAME_RCID[i] >= 0) && (pNAME_RCID[i] <= m_nvector_table_size))
                     {
                     int target_record_feid = m_pVectorEdgeHelperTable[pNAME_RCID[i]];
                     pEdgeVectorRecordFeature = poReader->ReadVector( target_record_feid, RCNM_VE );
                     }
                     else
                     pEdgeVectorRecordFeature = NULL;
                     */
                    int target_record_feid = m_vector_helper_hash[pNAME_RCID[i]];
                    pEdgeVectorRecordFeature = poReader->ReadVector( target_record_feid, RCNM_VE );

                    if( NULL != pEdgeVectorRecordFeature ) {
                        start_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_0" );
                        end_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_1" );

                        fwrite( &start_rcid, 1, sizeof(int), fpOut );
                        fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                        fwrite( &end_rcid, 1, sizeof(int), fpOut );

                        delete pEdgeVectorRecordFeature;
                    } else {
                        start_rcid = -1;                                    // error indication
                        end_rcid = -2;

                        fwrite( &start_rcid, 1, sizeof(int), fpOut );
                        fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                        fwrite( &end_rcid, 1, sizeof(int), fpOut );
                    }
                }

                fprintf( fpOut, "\n" );

                //  Reset the options
                papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,
                        "OFF" );
                poReader->SetOptions( papszReaderOptions );
                CSLDestroy( papszReaderOptions );

                break;
            }

                //      All others
            default:
                msg = _T("   Warning: Unimplemented ogr geotype record in file ");
                msg.Append( m_SENCFileName.GetFullPath() );
                wxLogMessage( msg );

                wkb_len = pGeo->WkbSize();
                fprintf( fpOut, "  %d\n", wkb_len );
                fwrite( pwkb_buffer, 1, wkb_len, fpOut );
                break;
        }       // switch

        free( pwkb_buffer );
    }
}

void s57chart::CreateSENCVectorEdgeTable( FILE * fpOut, S57Reader *poReader )
{
    fprintf( fpOut, "VETableStart\n" );

    //  Set up the options, adding RETURN_PRIMITIVES
    char ** papszReaderOptions = NULL;
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON" );
    poReader->SetOptions( papszReaderOptions );

    int feid = 0;
    OGRLineString *pLS = NULL;
    OGRGeometry *pGeo;
    OGRFeature *pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );

    while( NULL != pEdgeVectorRecordFeature ) {
        int record_id = pEdgeVectorRecordFeature->GetFieldAsInteger( "RCID" );
        fwrite( &record_id, 1, sizeof(int), fpOut );

        int nPoints = 0;
        if( pEdgeVectorRecordFeature->GetGeometryRef() != NULL ) {
            pGeo = pEdgeVectorRecordFeature->GetGeometryRef();
            if( pGeo->getGeometryType() == wkbLineString ) {
                pLS = (OGRLineString *) pGeo;
                nPoints = pLS->getNumPoints();
            } else
                nPoints = 0;
        }

        fwrite( &nPoints, 1, sizeof(int), fpOut );

        for( int i = 0; i < nPoints; i++ ) {
            OGRPoint p;
            pLS->getPoint( i, &p );

            //  Calculate SM from chart common reference point
            double easting, northing;
            toSM( p.getY(), p.getX(), ref_lat, ref_lon, &easting, &northing );

            MyPoint pd;
            pd.x = easting;
            pd.y = northing;
            fwrite( &pd, 1, sizeof(MyPoint), fpOut );
        }

        //    Next vector record
        feid++;
        delete pEdgeVectorRecordFeature;
        pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );
    }

    //    Write a finishing record
    int last_rcid = -1;
    fwrite( &last_rcid, 1, sizeof(int), fpOut );
    fprintf( fpOut, "\nVETableEnd\n" );

    //  Reset the options
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF" );
    poReader->SetOptions( papszReaderOptions );
    CSLDestroy( papszReaderOptions );
}

void s57chart::CreateSENCConnNodeTable( FILE * fpOut, S57Reader *poReader )
{
    fprintf( fpOut, "VCTableStart\n" );

    //  Set up the options, adding RETURN_PRIMITIVES
    char ** papszReaderOptions = NULL;
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON" );
    poReader->SetOptions( papszReaderOptions );

    int feid = 0;
    OGRPoint *pP;
    OGRGeometry *pGeo;
    OGRFeature *pConnNodeRecordFeature = poReader->ReadVector( feid, RCNM_VC );

    while( NULL != pConnNodeRecordFeature ) {
        if( pConnNodeRecordFeature->GetGeometryRef() != NULL ) {
            pGeo = pConnNodeRecordFeature->GetGeometryRef();
            if( pGeo->getGeometryType() == wkbPoint ) {
                pP = (OGRPoint *) pGeo;

                int record_id = pConnNodeRecordFeature->GetFieldAsInteger( "RCID" );

                fwrite( &record_id, 1, sizeof(int), fpOut );

                //  Calculate SM from chart common reference point
                double easting, northing;
                toSM( pP->getY(), pP->getX(), ref_lat, ref_lon, &easting, &northing );

                MyPoint pd;
                pd.x = easting;
                pd.y = northing;
                fwrite( &pd, 1, sizeof(MyPoint), fpOut );
            }
//                  else
//                        int yyp = 5;
        }
//            else
//                  int eep = 4;

        //    Next vector record
        feid++;
        delete pConnNodeRecordFeature;
        pConnNodeRecordFeature = poReader->ReadVector( feid, RCNM_VC );
    }

    //    Write a finishing record
    int last_rcid = -1;
    fwrite( &last_rcid, 1, sizeof(int), fpOut );
    fprintf( fpOut, "\nVCTableEnd\n" );

    //  Reset the options
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF" );
    poReader->SetOptions( papszReaderOptions );
    CSLDestroy( papszReaderOptions );
}

ListOfObjRazRules *s57chart::GetObjRuleListAtLatLon( float lat, float lon, float select_radius,
        ViewPort *VPoint )
{

    ListOfObjRazRules *ret_ptr = new ListOfObjRazRules;

//    Iterate thru the razRules array, by object/rule type

    ObjRazRules *top;

    for( int i = 0; i < PRIO_NUM; ++i ) {
        // Points by type, array indices [0..1]

        int point_type = ( ps52plib->m_nSymbolStyle == SIMPLIFIED ) ? 0 : 1;
        top = razRules[i][point_type];

        while( top != NULL ) {
            if( top->obj->npt == 1 )       // Do not select Multipoint objects (SOUNDG) yet.
                    {
                if( ps52plib->ObjectRenderCheck( top, VPoint ) ) {
                    if( DoesLatLonSelectObject( lat, lon, select_radius, top->obj ) )
                        ret_ptr->Append( top );
                }
            }

            //    Check the child branch, if any.
            //    This is where Multipoint soundings are captured individually
            if( top->child ) {
                ObjRazRules *child_item = top->child;
                while( child_item != NULL ) {
                    if( ps52plib->ObjectRenderCheck( child_item, VPoint ) ) {
                        if( DoesLatLonSelectObject( lat, lon, select_radius, child_item->obj ) )
                            ret_ptr->Append( child_item );
                    }

                    child_item = child_item->next;
                }
            }

            top = top->next;
        }

        // Areas by boundary type, array indices [3..4]

        int area_boundary_type = ( ps52plib->m_nBoundaryStyle == PLAIN_BOUNDARIES ) ? 3 : 4;
        top = razRules[i][area_boundary_type];           // Area nnn Boundaries
        while( top != NULL ) {
            if( ps52plib->ObjectRenderCheck( top, VPoint ) ) {
                if( DoesLatLonSelectObject( lat, lon, select_radius, top->obj ) ) ret_ptr->Append(
                        top );
            }

            top = top->next;
        }         // while

        // Finally, lines
        top = razRules[i][2];           // Lines

        while( top != NULL ) {
            if( ps52plib->ObjectRenderCheck( top, VPoint ) ) {
                if( DoesLatLonSelectObject( lat, lon, select_radius, top->obj ) ) ret_ptr->Append(
                        top );
            }

            top = top->next;
        }
    }

    return ret_ptr;
}

bool s57chart::DoesLatLonSelectObject( float lat, float lon, float select_radius, S57Obj *obj )
{
    switch( obj->Primitive_type ){
        //  For single Point objects, the integral object bounding box contains the lat/lon of the object,
        //  possibly expanded by text or symbol rendering
        case GEO_POINT: {
            if( 1 == obj->npt ) {
                //  Special case for LIGHTS
                //  Sector lights have had their BBObj expanded to include the entire drawn sector
                //  This is too big for pick area, can be confusing....
                //  So make a temporary box at the light's lat/lon, with select_radius size
                if( !strncmp( obj->FeatureName, "LIGHTS", 6 ) ) {
                    double olon, olat;
                    fromSM( ( obj->x * obj->x_rate ) + obj->x_origin,
                            ( obj->y * obj->y_rate ) + obj->y_origin, ref_lat, ref_lon, &olat,
                            &olon );

                    // Double the select radius to adjust for the fact that LIGHTS has
                    // a 0x0 BBox to start with, which makes it smaller than all other
                    // rendered objects.
                    wxBoundingBox sbox( olon - 2*select_radius, olat - 2*select_radius,
                            olon + 2*select_radius, olat + 2*select_radius );

                    if( sbox.PointInBox( lon, lat, 0 ) ) return true;
                }

                else if( obj->BBObj.PointInBox( lon, lat, select_radius ) ) return true;
            }

            //  For MultiPoint objects, make a bounding box from each point's lat/lon
            //  and check it
            else {
                if( !obj->bBBObj_valid ) return false;

                //  Coarse test first
                if( !obj->BBObj.PointInBox( lon, lat, select_radius ) ) return false;
                //  Now decomposed soundings, one by one
                double *pdl = obj->geoPtMulti;
                for( int ip = 0; ip < obj->npt; ip++ ) {
                    double lon_point = *pdl++;
                    double lat_point = *pdl++;
                    wxBoundingBox BB_point( lon_point, lat_point, lon_point, lat_point );
                    if( BB_point.PointInBox( lon, lat, select_radius ) ) {
//                                  index = ip;
                        return true;
                    }
                }
            }

            break;
        }
        case GEO_AREA: {
            //  Coarse test first
            if( !obj->BBObj.PointInBox( lon, lat, select_radius ) ) return false;
            else
                return IsPointInObjArea( lat, lon, select_radius, obj );
        }

        case GEO_LINE: {
            if( obj->geoPt ) {
                //  Coarse test first
                if( !obj->BBObj.PointInBox( lon, lat, select_radius ) ) return false;

                //  Line geometry is carried in SM or CM93 coordinates, so...
                //  make the hit test using SM coordinates, converting from object points to SM using per-object conversion factors.

                double easting, northing;
                toSM( lat, lon, ref_lat, ref_lon, &easting, &northing );

                pt *ppt = obj->geoPt;
                int npt = obj->npt;

                double xr = obj->x_rate;
                double xo = obj->x_origin;
                double yr = obj->y_rate;
                double yo = obj->y_origin;

                double north0 = ( ppt->y * yr ) + yo;
                double east0 = ( ppt->x * xr ) + xo;
                ppt++;

                for( int ip = 1; ip < npt; ip++ ) {
                    double north = ( ppt->y * yr ) + yo;
                    double east = ( ppt->x * xr ) + xo;

                    //    A slightly less coarse segment bounding box check
                    if( northing >= ( fmin(north, north0) - select_radius ) ) if( northing
                            <= ( fmax(north, north0) + select_radius ) ) if( easting
                            >= ( fmin(east, east0) - select_radius ) ) if( easting
                            <= ( fmax(east, east0) + select_radius ) ) {
                        //                                                    index = ip;
                        return true;
                    }

                    north0 = north;
                    east0 = east;
                    ppt++;
                }
            }

            break;
        }

        case GEO_META:
        case GEO_PRIM:

            break;
    }

    return false;
}

wxString s57chart::GetAttributeDecode( wxString& att, int ival )
{
    wxString ret_val = _T("");

    //    Special case for "NATSUR", cacheing the strings for faster chart rendering
    if( att.IsSameAs( _T("NATSUR") ) ) {
        if( !m_natsur_hash[ival].IsEmpty() )            // entry available?
        {
            return m_natsur_hash[ival];
        }
    }

    if( NULL == m_pcsv_locn ) return ret_val;

    //  Get the attribute code from the acronym
    const char *att_code;

    wxString file( *m_pcsv_locn );
    file.Append( _T("/s57attributes.csv") );

    if( !wxFileName::FileExists( file ) ) {
        wxString msg( _T("   Could not open ") );
        msg.Append( file );
        wxLogMessage( msg );

        return ret_val;
    }

    att_code = MyCSVGetField( file.mb_str(), "Acronym",                  // match field
            att.mb_str(),               // match value
            CC_ExactString, "Code" );             // return field

    // Now, get a nice description from s57expectedinput.csv
    //  This will have to be a 2-d search, using ID field and Code field

    // Ingest, and get a pointer to the ingested table for "Expected Input" file
    wxString ei_file( *m_pcsv_locn );
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

    if( att.IsSameAs( _T("NATSUR") ) ) m_natsur_hash[ival] = ret_val;            // cache the entry

    return ret_val;

}

//----------------------------------------------------------------------------------

bool s57chart::IsPointInObjArea( float lat, float lon, float select_radius, S57Obj *obj )
{
    bool ret = false;

    if( obj->pPolyTessGeo ) {
        if( !obj->pPolyTessGeo->IsOk() ) obj->pPolyTessGeo->BuildTessGL();

        PolyTriGroup *ppg = obj->pPolyTessGeo->Get_PolyTriGroup_head();

        TriPrim *pTP = ppg->tri_prim_head;

        MyPoint pvert_list[3];

        //  Polygon geometry is carried in SM coordinates, so...
        //  make the hit test thus.
        double easting, northing;
        toSM( lat, lon, ref_lat, ref_lon, &easting, &northing );

        //  On some chart types (e.g. cm93), the tesseleated coordinates are stored differently.
        //  Adjust the pick point (easting/northing) to correspond.
        if( !ppg->m_bSMSENC ) {
            double y_rate = obj->y_rate;
            double y_origin = obj->y_origin;
            double x_rate = obj->x_rate;
            double x_origin = obj->x_origin;

            double northing_scaled = ( northing - y_origin ) / y_rate;
            double easting_scaled = ( easting - x_origin ) / x_rate;
            northing = northing_scaled;
            easting = easting_scaled;
        }

        while( pTP ) {
//  Coarse test
            if( pTP->p_bbox->PointInBox( lon, lat, 0 ) ) {
                double *p_vertex = pTP->p_vertex;

                switch( pTP->type ){
                    case PTG_TRIANGLE_FAN: {
                        for( int it = 0; it < pTP->nVert - 2; it++ ) {
                            pvert_list[0].x = p_vertex[0];
                            pvert_list[0].y = p_vertex[1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                    case PTG_TRIANGLE_STRIP: {
                        for( int it = 0; it < pTP->nVert - 2; it++ ) {
                            pvert_list[0].x = p_vertex[( it * 2 )];
                            pvert_list[0].y = p_vertex[( it * 2 ) + 1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                    case PTG_TRIANGLES: {
                        for( int it = 0; it < pTP->nVert; it += 3 ) {
                            pvert_list[0].x = p_vertex[( it * 2 )];
                            pvert_list[0].y = p_vertex[( it * 2 ) + 1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                }

            }
            pTP = pTP->p_next;
        }

    }           // if pPolyTessGeo

    else if( obj->pPolyTrapGeo ) {
        if( !obj->pPolyTrapGeo->IsOk() ) obj->pPolyTrapGeo->BuildTess();

        PolyTrapGroup *ptg = obj->pPolyTrapGeo->Get_PolyTrapGroup_head();

        //  Polygon geometry is carried in SM coordinates, so...
        //  make the hit test thus.
        //  However, since PolyTrapGeo geometry is (always??) in cm-93 coordinates, convert to sm as necessary
        double easting, northing;
        toSM( lat, lon, ref_lat, ref_lon, &easting, &northing );

        int ntraps = ptg->ntrap_count;
        trapz_t *ptraps = ptg->trap_array;
        MyPoint *segs = (MyPoint *) ptg->ptrapgroup_geom; //TODO convert MyPoint to wxPoint2DDouble globally

        MyPoint pvert_list[4];

        double y_rate = obj->y_rate;
        double y_origin = obj->y_origin;

        for( int i = 0; i < ntraps; i++, ptraps++ ) {
            //      Y test

            double hiy = ( ptraps->hiy * y_rate ) + y_origin;
            if( northing > hiy ) continue;

            double loy = ( ptraps->loy * y_rate ) + y_origin;
            if( northing < loy ) continue;

            //      Use the segment endpoints to calculate the corners of a trapezoid
            int lseg = ptraps->ilseg;
            int rseg = ptraps->irseg;

            //    Left edge
            double xmax = segs[lseg].x;
            double xmin = segs[lseg + 1].x;

            double ymax = segs[lseg].y;
            double ymin = segs[lseg + 1].y;

            double xt, yt, xca, xcb;

            if( ymax < ymin ) {
                xt = xmin;
                xmin = xmax;
                xmax = xt;                // interchange min/max
                yt = ymin;
                ymin = ymax;
                ymax = yt;
            }

            if( xmin == xmax ) {
                xca = xmin;
                xcb = xmin;
            } else {
                double slope = ( ymax - ymin ) / ( xmax - xmin );
                xca = xmin + ( ptraps->loy - ymin ) / slope;
                xcb = xmin + ( ptraps->hiy - ymin ) / slope;
            }

            pvert_list[0].x = ( xca * obj->x_rate ) + obj->x_origin;
            pvert_list[0].y = loy;

            pvert_list[1].x = ( xcb * obj->x_rate ) + obj->x_origin;
            pvert_list[1].y = hiy;

            //    Right edge
            xmax = segs[rseg].x;
            xmin = segs[rseg + 1].x;
            ymax = segs[rseg].y;
            ymin = segs[rseg + 1].y;

            if( ymax < ymin ) {
                xt = xmin;
                xmin = xmax;
                xmax = xt;
                yt = ymin;
                ymin = ymax;
                ymax = yt;
            }

            if( xmin == xmax ) {
                xca = xmin;
                xcb = xmin;
            } else {
                double slope = ( ymax - ymin ) / ( xmax - xmin );
                xca = xmin + ( ptraps->hiy - ymin ) / slope;
                xcb = xmin + ( ptraps->loy - ymin ) / slope;
            }

            pvert_list[2].x = ( xca * obj->x_rate ) + obj->x_origin;
            pvert_list[2].y = hiy;

            pvert_list[3].x = ( xcb * obj->x_rate ) + obj->x_origin;
            pvert_list[3].y = loy;

            if( G_PtInPolygon( (MyPoint *) pvert_list, 4, easting, northing ) ) {
                ret = true;
                break;
            }
        }
    }           // if pPolyTrapGeo

    return ret;
}

//------------------------------------------------------------------------
//
//          Minimally initialize an S57 chart from a prebuilt SENC file
//          Mainly used to get extents, scale, etc from SENC header
//
//------------------------------------------------------------------------
bool s57chart::InitFromSENCMinimal( const wxString &FullPath )
{
    bool ret_val = true;
    int check_val = 0;

    m_FullPath = FullPath;
    m_Description = m_FullPath;

    wxFileName S57FileName( FullPath );

    if( !S57FileName.FileExists() ) return false;

    wxFile f;
    if( f.Open( S57FileName.GetFullPath() ) ) {
        if( f.Length() == 0 ) {
            f.Close();
            ret_val = false;
        } else                                      // file exists, non-zero
        {                                         // so check for new updates

            f.Seek( 0 );
            wxFileInputStream *pfpx_u = new wxFileInputStream( f );
            wxBufferedInputStream *pfpx = new wxBufferedInputStream( *pfpx_u );
            int dun = 0;
            int last_update = 0;
            int senc_file_version = 0;
            char buf[256];
            char *pbuf = buf;

            while( !dun ) {
                if( my_fgets( pbuf, 256, *pfpx ) == 0 ) {
                    dun = 1;
                    ret_val = false;
                    break;
                } else {
                    if( !strncmp( pbuf, "OGRF", 4 ) ) {
                        dun = 1;
                        break;
                    } else if( !strncmp( pbuf, "UPDT", 4 ) ) {
                        sscanf( pbuf, "UPDT=%i", &last_update );
                    } else if( !strncmp( pbuf, "SENC", 4 ) ) {
                        sscanf( pbuf, "SENC Version=%i", &senc_file_version );
                    } else if( !strncmp( pbuf, "Chart Extents:", 13 ) ) {
                        double xmin, ymin, xmax, ymax;
                        wxString l( pbuf, wxConvUTF8 );

                        wxStringTokenizer tkz( l, _T(" ") );
                        wxString token = tkz.GetNextToken();  //Chart
                        token = tkz.GetNextToken();           //Extents:

                        token = tkz.GetNextToken();
                        token.ToDouble( &xmax );
                        token = tkz.GetNextToken();
                        token.ToDouble( &xmin );
                        token = tkz.GetNextToken();
                        token.ToDouble( &ymax );
                        token = tkz.GetNextToken();
                        token.ToDouble( &ymin );

                        Extent ext;
                        ext.WLON = xmin;
                        ext.SLAT = ymin;
                        ext.ELON = xmax;
                        ext.NLAT = ymax;
                        SetFullExtent( ext );

                        check_val |= 1;
                    } else if( !strncmp( pbuf, "SCALE=", 6 ) ) {
                        int scale;
                        sscanf( pbuf, "SCALE=%i", &scale );
                        m_Chart_Scale = scale;
                        check_val |= 2;
                    }

                }
            }

            delete pfpx;
            delete pfpx_u;
            f.Close();
        }
    } else
        ret_val = false;                // file did not open

    if( false == ret_val ) return false;                   // some other read problem

    if( 3 != check_val ) return false;                   // did not get all required parameters

    return true;

}

wxString s57chart::GetObjectAttributeValueAsString( S57Obj *obj, int iatt, wxString curAttrName )
{
    wxString value;
    S57attVal *pval;

    pval = obj->attVal->Item( iatt );
    switch( pval->valType ){
        case OGR_STR: {
            if( pval->value ) {
                wxString val_str( (char *) ( pval->value ), wxConvUTF8 );
                long ival;
                if( val_str.ToLong( &ival ) ) {
                    if( 0 == ival ) value = _T("Unknown");
                    else {
                        wxString decode_val = GetAttributeDecode( curAttrName, ival );
                        if( !decode_val.IsEmpty() ) {
                            value = decode_val;
                            wxString iv;
                            iv.Printf( _T("(%d)"), (int) ival );
                            value.Append( iv );
                        } else
                            value.Printf( _T("%d"), (int) ival );
                    }
                }

                else if( val_str.IsEmpty() ) value = _T("Unknown");

                else {
                    value.Clear();
                    wxString value_increment;
                    wxStringTokenizer tk( val_str, wxT(",") );
                    int iv = 0;
                    while( tk.HasMoreTokens() ) {
                        wxString token = tk.GetNextToken();
                        long ival;
                        if( token.ToLong( &ival ) ) {
                            wxString decode_val = GetAttributeDecode( curAttrName, ival );
                            if( !decode_val.IsEmpty() ) value_increment = decode_val;
                            else
                                value_increment.Printf( _T(" %d"), (int) ival );

                            if( iv ) value_increment.Prepend( wxT(", ") );
                        }
                        value.Append( value_increment );

                        iv++;
                    }
                    value.Append( val_str );
                }
            } else
                value = _T("[NULL VALUE]");

            break;
        }

        case OGR_INT: {
            int ival = *( (int *) pval->value );
            wxString decode_val = GetAttributeDecode( curAttrName, ival );

            if( !decode_val.IsEmpty() ) {
                value = decode_val;
                wxString iv;
                iv.Printf( _T("(%d)"), ival );
                value.Append( iv );
            } else
                value.Printf( _T("(%d)"), ival );

            break;
        }
        case OGR_INT_LST:
            break;

        case OGR_REAL: {
            double dval = *( (double *) pval->value );
            wxString val_suffix = _T(" m");

            //    As a special case, convert some attribute values to feet.....
            if( ( curAttrName == _T("VERCLR") ) || ( curAttrName == _T("VERCLL") )
                    || ( curAttrName == _T("HEIGHT") ) || ( curAttrName == _T("HORCLR") ) ) {
                switch( ps52plib->m_nDepthUnitDisplay ){
                    case 0:                       // feet
                    case 2:                       // fathoms
                        dval = dval * 3 * 39.37 / 36;              // feet
                        val_suffix = _T(" ft");
                        break;
                    default:
                        break;
                }
            }

            else if( ( curAttrName == _T("VALSOU") ) || ( curAttrName == _T("DRVAL1") )
                    || ( curAttrName == _T("DRVAL2") ) ) {
                switch( ps52plib->m_nDepthUnitDisplay ){
                    case 0:                       // feet
                        dval = dval * 3 * 39.37 / 36;              // feet
                        val_suffix = _T(" ft");
                        break;
                    case 2:                       // fathoms
                        dval = dval * 3 * 39.37 / 36;              // fathoms
                        dval /= 6.0;
                        val_suffix = _T(" fathoms");
                        break;
                    default:
                        break;
                }
            }

            else if( curAttrName == _T("SECTR1") ) val_suffix = _T("&deg;");
            else if( curAttrName == _T("SECTR2") ) val_suffix = _T("&deg;");
            else if( curAttrName == _T("ORIENT") ) val_suffix = _T("&deg;");
            else if( curAttrName == _T("VALNMR") ) val_suffix = _T(" Nm");
            else if( curAttrName == _T("SIGPER") ) val_suffix = _T("s");

            if( dval - floor( dval ) < 0.01 ) value.Printf( _T("%2.0f"), dval );
            else
                value.Printf( _T("%4.1f"), dval );

            value << val_suffix;

            break;
        }

        case OGR_REAL_LST: {
            break;
        }
    }
    return value;
}

int s57chart::CompareLights( const void** l1ptr, const void** l2ptr )
{
    S57Light l1 = *(S57Light*) *l1ptr;
    S57Light l2 = *(S57Light*) *l2ptr;

    int positionDiff = l1.position.Cmp( l2.position );
    if( positionDiff != 0 ) return positionDiff;

    double angle1, angle2;
    int attrIndex1 = l1.attributeNames.Index( _T("SECTR1") );
    int attrIndex2 = l2.attributeNames.Index( _T("SECTR1") );

    // This should put Lights without sectors last in the list.
    if( attrIndex1 == wxNOT_FOUND && attrIndex2 == wxNOT_FOUND ) return 0;
    if( attrIndex1 != wxNOT_FOUND && attrIndex2 == wxNOT_FOUND ) return -1;
    if( attrIndex1 == wxNOT_FOUND && attrIndex2 != wxNOT_FOUND ) return 1;

    l1.attributeValues.Item( attrIndex1 ).ToDouble( &angle1 );
    l2.attributeValues.Item( attrIndex2 ).ToDouble( &angle2 );

    if( angle1 == angle2 ) return 0;
    if( angle1 > angle2 ) return 1;
    return -1;
}

wxString s57chart::CreateObjDescriptions( ListOfObjRazRules* rule_list )
{
    wxString ret_val;
    char *curr_att;
    int attrCounter;
    wxString curAttrName, value;
    bool isLight = false;
    wxString className;
    wxString classDesc;
    wxString classAttributes;
    wxString objText;
    wxString lightsHtml;
    wxString positionString;
    wxArrayPtrVoid lights;
    S57Light* curLight;

    for( ListOfObjRazRules::Node *node = rule_list->GetLast(); node; node = node->GetPrevious() ) {
        ObjRazRules *current = node->GetData();
        positionString.Clear();
        objText.Clear();

        // Soundings have no information, so don't show them
        if( 0 == strncmp( current->LUP->OBCL, "SOUND", 5 ) ) continue;

        if( current->obj->Primitive_type == GEO_META ) continue;
        if( current->obj->Primitive_type == GEO_PRIM ) continue;

        className = wxString( current->obj->FeatureName, wxConvUTF8 );

        // Lights get grouped together to make display look nicer.
        isLight = !strcmp( current->obj->FeatureName, "LIGHTS" );

        //    Get the object's nice description from s57objectclasses.csv
        //    using cpl_csv from the gdal library

        const char *name_desc;
        if( NULL != m_pcsv_locn ) {
            wxString oc_file( *m_pcsv_locn );
            oc_file.Append( _T("/s57objectclasses.csv") );
            name_desc = MyCSVGetField( oc_file.mb_str(), "Acronym",                  // match field
                    current->obj->FeatureName,            // match value
                    CC_ExactString, "ObjectClass" );             // return field
        } else
            name_desc = "";

        // In case there is no nice description for this object class, use the 6 char class name
        if( 0 == strlen( name_desc ) ) {
            name_desc = current->obj->FeatureName;
            classDesc = wxString( name_desc, wxConvUTF8, 1 );
            classDesc << wxString( name_desc + 1, wxConvUTF8 ).MakeLower();
        } else {
            classDesc = wxString( name_desc, wxConvUTF8 );
        }

        //    Show LUP
        if( g_bDebugS57 ) {
            wxString index;
            index.Printf( _T("Feature Index: %d\n"), current->obj->Index );
            classAttributes << index;

            wxString LUPstring;
            LUPstring.Printf( _T("LUP RCID:  %d\n"), current->LUP->RCID );
            classAttributes << LUPstring;

            LUPstring = _T("    LUP ATTC: ");
            if( current->LUP->ATTCArray ) LUPstring += current->LUP->ATTCArray->Item( 0 );
            LUPstring += _T("\n");
            classAttributes << LUPstring;

            LUPstring = _T("    LUP INST: ");
            if( current->LUP->INST ) LUPstring += *( current->LUP->INST );
            LUPstring += _T("\n\n");
            classAttributes << LUPstring;

        }

        if( GEO_POINT == current->obj->Primitive_type ) {
            double lon, lat;
            fromSM( ( current->obj->x * current->obj->x_rate ) + current->obj->x_origin,
                    ( current->obj->y * current->obj->y_rate ) + current->obj->y_origin, ref_lat,
                    ref_lon, &lat, &lon );

            if( lon > 180.0 ) lon -= 360.;

            positionString.Clear();
            positionString += toSDMM( 1, lat );
            positionString << _T(" ");
            positionString += toSDMM( 2, lon );

            if( isLight ) {
                curLight = new S57Light;
                curLight->position = positionString;
                curLight->hasSectors = false;
                lights.Add( curLight );
            }
        }

        //    Get the Attributes and values

        char *curr_att0 = (char *) calloc( current->obj->attList->Len() + 1, 1 );
        strncpy( curr_att0, current->obj->attList->mb_str(), current->obj->attList->Len() );
        curr_att = curr_att0;

        attrCounter = 0;

        wxString attribStr;
        int noAttr = 0;
        attribStr << _T("<table border=0 cellspacing=0 cellpadding=0>");

        bool inDepthRange = false;

        while( *curr_att ) {
            //    Attribute name
            curAttrName.Clear();
            noAttr++;
            while( ( *curr_att ) && ( *curr_att != '\037' ) ) {
                char t = *curr_att++;
                curAttrName.Append( t );
            }

            if( *curr_att == '\037' ) curr_att++;

            // Sort out how some kinds of attibutes are displayed to get a more readable look.
            // DEPARE gets just its range. Lights are grouped.

            if( isLight ) {
                curLight->attributeNames.Add( curAttrName );
                if( curAttrName.StartsWith( _T("SECTR") ) ) curLight->hasSectors = true;
            } else {
                if( curAttrName == _T("DRVAL1") ) {
                    attribStr << _T("<tr><td><font size=-1>");
                    inDepthRange = true;
                } else if( curAttrName == _T("DRVAL2") ) {
                    attribStr << _T(" - ");
                    inDepthRange = false;
                } else {
                    if( inDepthRange ) {
                        attribStr << _T("</font></td></tr>\n");
                        inDepthRange = false;
                    }
                    attribStr << _T("<tr><td valign=top><font size=-2>") << curAttrName;
                    attribStr << _T("</font></td><td>&nbsp;&nbsp;</td><td valign=top><font size=-1>");
                }
            }

            // What we need to do...
            // Change senc format, instead of (S), (I), etc, use the attribute types fetched from the S57attri...csv file
            // This will be like (E), (L), (I), (F)
            //  will affect lots of other stuff.  look for S57attVal.valType
            // need to do this in creatsencrecord above, and update the senc format.

            value = GetObjectAttributeValueAsString( current->obj, attrCounter, curAttrName );

            if( isLight ) {
                curLight->attributeValues.Add( value );
            } else {
                if( curAttrName == _T("INFORM") || curAttrName == _T("NINFOM") ) value.Replace(
                        _T("|"), _T("<br>") );
                attribStr << value;

                if( !( curAttrName == _T("DRVAL1") ) ) {
                    attribStr << _T("</font></td></tr>\n");
                }
            }

            attrCounter++;

        }             //while *curr_att

        if( !isLight ) {
            attribStr << _T("</table>\n");

            objText += _T("<b>") + classDesc + _T("</b> <font size=-2>(") + className
                    + _T(")</font>") + _T("<br>");

            if( positionString.Length() ) objText << _T("<font size=-2>") << positionString
                    << _T("</font><br>\n");

            if( noAttr > 0 ) objText << attribStr;

            if( node != rule_list->GetFirst() ) objText += _T("<hr noshade>");
            objText += _T("<br>");
            ret_val << objText;
        }

        free( curr_att0 );

    } // Object for loop

    if( lights.Count() > 0 ) {

        // For lights we now have all the info gathered but no HTML output yet, now
        // run through the data and build a merged table for all lights.

        lights.Sort( ( CMPFUNC_wxArraywxArrayPtrVoid )( &s57chart::CompareLights ) );

        wxString lastPos;

        for( unsigned int curLightNo = 0; curLightNo < lights.Count(); curLightNo++ ) {
            S57Light* thisLight = (S57Light*) lights.Item( curLightNo );
            int attrIndex;

            if( thisLight->position != lastPos ) {

                lastPos = thisLight->position;

                if( curLightNo > 0 ) lightsHtml << _T("</table>\n<hr noshade>\n");

                lightsHtml << _T("<b>Light</b> <font size=-2>(LIGHTS)</font><br>");
                lightsHtml << _T("<font size=-2>") << thisLight->position << _T("</font><br>\n");

                if( curLight->hasSectors ) lightsHtml
                        <<_("<font size=-2>(Sector angles are True Bearings from Seaward)</font><br>");

                lightsHtml << _T("<table>");
            }

            lightsHtml << _T("<tr>");
            lightsHtml << _T("<td><font size=-1>");

            attrIndex = thisLight->attributeNames.Index( _T("COLOUR") );
            if( attrIndex != wxNOT_FOUND ) {
                wxString color = thisLight->attributeValues.Item( attrIndex );
                if( color == _T("red(3)") ) lightsHtml
                        << _T("<table border=0><tr><td bgcolor=red>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                if( color == _T("green(4)") ) lightsHtml
                        << _T("<table border=0><tr><td bgcolor=green>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                if( color == _T("white(1)") ) lightsHtml
                        << _T("<table border=0><tr><td bgcolor=yellow>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
            }

            lightsHtml << _T("</font></td><td><font size=-1><nobr><b>");

            attrIndex = thisLight->attributeNames.Index( _T("LITCHR") );
            if( attrIndex != wxNOT_FOUND ) {
                wxString character = thisLight->attributeValues.Item( attrIndex );
                lightsHtml << character.BeforeFirst( wxChar( '(' ) ) << _T(" ");
            }

            attrIndex = thisLight->attributeNames.Index( _T("SIGGRP") );
            if( attrIndex != wxNOT_FOUND ) {
                lightsHtml << thisLight->attributeValues.Item( attrIndex );
                lightsHtml << _T(" ");
            }

            attrIndex = thisLight->attributeNames.Index( _T("SIGPER") );
            if( attrIndex != wxNOT_FOUND ) {
                lightsHtml << thisLight->attributeValues.Item( attrIndex );
                lightsHtml << _T(" ");
            }

            attrIndex = thisLight->attributeNames.Index( _T("HEIGHT") );
            if( attrIndex != wxNOT_FOUND ) {
                lightsHtml << thisLight->attributeValues.Item( attrIndex );
                lightsHtml << _T(" ");
            }

            attrIndex = thisLight->attributeNames.Index( _T("VALNMR") );
            if( attrIndex != wxNOT_FOUND ) {
                lightsHtml << thisLight->attributeValues.Item( attrIndex );
                lightsHtml << _T(" ");
            }

            lightsHtml << _T("</b>");

            attrIndex = thisLight->attributeNames.Index( _T("SECTR1") );
            if( attrIndex != wxNOT_FOUND ) {
                lightsHtml << _T("(") <<thisLight->attributeValues.Item( attrIndex );
                lightsHtml << _T(" - ");
                attrIndex = thisLight->attributeNames.Index( _T("SECTR2") );
                lightsHtml << thisLight->attributeValues.Item( attrIndex ) << _T(") ");
            }

            lightsHtml << _T("</nobr>");

            attrIndex = thisLight->attributeNames.Index( _T("CATLIT") );
            if( attrIndex != wxNOT_FOUND ) {
                lightsHtml << _T("<nobr>");
                lightsHtml
                        << thisLight->attributeValues.Item( attrIndex ).BeforeFirst(
                                wxChar( '(' ) );
                lightsHtml << _T("</nobr> ");
            }

            attrIndex = thisLight->attributeNames.Index( _T("EXCLIT") );
            if( attrIndex != wxNOT_FOUND ) {
                lightsHtml << _T("<nobr>");
                lightsHtml
                        << thisLight->attributeValues.Item( attrIndex ).BeforeFirst(
                                wxChar( '(' ) );
                lightsHtml << _T("</nobr> ");
            }

            attrIndex = thisLight->attributeNames.Index( _T("OBJNAM") );
            if( attrIndex != wxNOT_FOUND ) {
                lightsHtml << _T("<br><nobr>");
                lightsHtml << thisLight->attributeValues.Item( attrIndex ).Left( 1 ).Upper();
                lightsHtml << thisLight->attributeValues.Item( attrIndex ).Mid( 1 );
                lightsHtml << _T("</nobr> ");
            }

            lightsHtml << _T("</font></td>");
            lightsHtml << _T("</tr>");

            thisLight->attributeNames.Clear();
            thisLight->attributeValues.Clear();
            delete thisLight;
        }
        lightsHtml << _T("</table><hr noshade>\n");
        ret_val = lightsHtml << ret_val;

        lights.Clear();
    }

    return ret_val;
}

//------------------------------------------------------------------------
//
//          S57 ENC (i.e. "raw") DataSet support functions
//          Not bulletproof, so call carefully
//
//------------------------------------------------------------------------
bool s57chart::InitENCMinimal( const wxString &FullPath )
{
    if( NULL == g_poRegistrar ) {
        wxLogMessage( _T("   Error: No ClassRegistrar in InitENCMinimal.") );
        return false;
    }

    m_pENCDS = new OGRS57DataSource;

    m_pENCDS->SetS57Registrar( g_poRegistrar );             ///172

    if( !m_pENCDS->OpenMin( FullPath.mb_str(), TRUE ) )       ///172
    return false;

    S57Reader *pENCReader = m_pENCDS->GetModule( 0 );
    pENCReader->SetClassBased( g_poRegistrar );

    pENCReader->Ingest();

    return true;
}

OGRFeature *s57chart::GetChartFirstM_COVR( int &catcov )
{
//    Get the reader
    S57Reader *pENCReader = m_pENCDS->GetModule( 0 );

    if( ( NULL != pENCReader ) && ( NULL != g_poRegistrar ) ) {

//      Select the proper class
        g_poRegistrar->SelectClass( "M_COVR" );

//      Build a new feature definition for this class
        OGRFeatureDefn *poDefn = S57GenerateObjectClassDefn( g_poRegistrar,
                g_poRegistrar->GetOBJL(), pENCReader->GetOptionFlags() );

//      Add this feature definition to the reader
        pENCReader->AddFeatureDefn( poDefn );

//    Also, add as a Layer to Datasource to ensure proper deletion
        m_pENCDS->AddLayer( new OGRS57Layer( m_pENCDS, poDefn, 1 ) );

//      find this feature
        OGRFeature *pobjectDef = pENCReader->ReadNextFeature( poDefn );
        if( pobjectDef ) {
            //  Fetch the CATCOV attribute
            catcov = pobjectDef->GetFieldAsInteger( "CATCOV" );
            return pobjectDef;
        }

        else {
            return NULL;
        }
    } else
        return NULL;
}

OGRFeature *s57chart::GetChartNextM_COVR( int &catcov )
{
    catcov = -1;

//    Get the reader
    S57Reader *pENCReader = m_pENCDS->GetModule( 0 );

//    Get the Feature Definition, stored in Layer 0
    OGRFeatureDefn *poDefn = m_pENCDS->GetLayer( 0 )->GetLayerDefn();

    if( pENCReader ) {
        OGRFeature *pobjectDef = pENCReader->ReadNextFeature( poDefn );

        if( pobjectDef ) {
            catcov = pobjectDef->GetFieldAsInteger( "CATCOV" );
            return pobjectDef;
        }

        return NULL;
    } else
        return NULL;

}

int s57chart::GetENCScale( void )
{
    if( NULL == m_pENCDS ) return 0;

    //    Assume that chart has been initialized for minimal ENC access
    //    which implies that the ENC has been fully ingested, and some
    //    interesting values have been extracted thereby.

//    Get the reader
    S57Reader *pENCReader = m_pENCDS->GetModule( 0 );

    if( pENCReader ) return pENCReader->GetCSCL();       ///172
    else
        return 1;
}

extern wxLog *logger;

/************************************************************************/
/*                       OpenCPN_OGRErrorHandler()                      */
/*                       Use Global wxLog Class                         */
/************************************************************************/

void OpenCPN_OGRErrorHandler( CPLErr eErrClass, int nError, const char * pszErrorMsg )
{

#define ERR_BUF_LEN 2000

    char buf[ERR_BUF_LEN + 1];

    if( eErrClass == CE_Debug ) sprintf( buf, " %s", pszErrorMsg );
    else if( eErrClass == CE_Warning ) sprintf( buf, "   Warning %d: %s\n", nError, pszErrorMsg );
    else
        sprintf( buf, "   ERROR %d: %s\n", nError, pszErrorMsg );

    if( g_bGDAL_Debug ) {
        wxString msg( buf, wxConvUTF8 );
        wxLogMessage( msg );
    }

    //      Do not simply return on CE_Fatal errors, as we don't want to abort()

    if( eErrClass == CE_Fatal ) {
        longjmp( env_ogrf, 1 );                  // jump back to the setjmp() point
    }

}

//      In GDAL-1.2.0, CSVGetField is not exported.......
//      So, make my own simplified copy
/************************************************************************/
/*                           MyCSVGetField()                            */
/*                                                                      */
/************************************************************************/

const char *MyCSVGetField( const char * pszFilename, const char * pszKeyFieldName,
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

//------------------------------------------------------------------------
//  s57RegistrarMgr Implementation
//  This is a class holding the ctor and dtor for the global registrar
//------------------------------------------------------------------------

s57RegistrarMgr::s57RegistrarMgr( const wxString& csv_dir, FILE *flog )
{
    s57_initialize( csv_dir, flog );
}

s57RegistrarMgr::~s57RegistrarMgr()
{
    delete g_poRegistrar;
    g_poRegistrar = NULL;
}

//------------------------------------------------------------------------
//  Initialize GDAL/OGR S57ENC support
//------------------------------------------------------------------------

int s57_initialize( const wxString& csv_dir, FILE *flog )
{

    //      Get one instance of the s57classregistrar,
    //      And be prepared to give it to any module that needs it

    if( g_poRegistrar == NULL ) {
        g_poRegistrar = new S57ClassRegistrar();

        if( !g_poRegistrar->LoadInfo( csv_dir.mb_str(), FALSE ) ) {
            wxString msg( _T("   Error: Could not load S57 ClassInfo from ") );
            msg.Append( csv_dir );
            wxLogMessage( msg );

            delete g_poRegistrar;
            g_poRegistrar = NULL;
        }
    }

    return 0;
}

//------------------------------------------------------------------------
//
//          Some s57 Utilities
//          Meant to be called "bare", usually with no class instance.
//
//------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Get Chart Extents
//----------------------------------------------------------------------------------

bool s57_GetChartExtent( const wxString& FullPath, Extent *pext )
{
    //   Fix this  find extents of which?? layer??
    /*
     OGRS57DataSource *poDS = new OGRS57DataSource;
     poDS->Open(pFullPath, TRUE);

     if( poDS == NULL )
     return false;

     OGREnvelope Env;
     S57Reader   *poReader = poDS->GetModule(0);
     poReader->GetExtent(&Env, true);

     pext->NLAT = Env.MaxY;
     pext->ELON = Env.MaxX;
     pext->SLAT = Env.MinY;
     pext->WLON = Env.MinX;

     delete poDS;
     */
    return false;
}

void s57_DrawExtendedLightSectors( ocpnDC& dc, ViewPort& viewport, std::vector<s57Sector_t>& sectorlegs ) {
    double rangeScale = 0.0;

    if( sectorlegs.size() > 0 ) {
        std::vector<int> sectorangles;
        for( unsigned int i=0; i<sectorlegs.size(); i++ ) {
            double endx, endy;
            ll_gc_ll( sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
                    sectorlegs[i].sector1 + 180.0, sectorlegs[i].range,
                    &endy, &endx );

            wxPoint end1 = viewport.GetPixFromLL( endy, endx );

            ll_gc_ll( sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
                    sectorlegs[i].sector2 + 180.0, sectorlegs[i].range,
                    &endy, &endx );

            wxPoint end2 = viewport.GetPixFromLL( endy, endx );

            wxPoint lightPos = viewport.GetPixFromLL( sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x );

            // Make sure arcs are well inside viewport.
            double rangePx = sqrt( pow(lightPos.x - end1.x, 2.0) + pow(lightPos.y - end1.y, 2.0) );
            rangePx /= 3.0;
            if( rangeScale == 0.0 ) {
                rangeScale = 1.0;
                if( rangePx > viewport.pix_height / 3 ) {
                    rangeScale *= (viewport.pix_height / 3) / rangePx;
                }
            }

            rangePx = rangePx * rangeScale;

            int legOpacity;
            wxPen *arcpen = wxThePenList->FindOrCreatePen( sectorlegs[i].color, 12, wxSOLID );
            arcpen->SetCap( wxCAP_BUTT );
            dc.SetPen( *arcpen );

            double angle1, angle2;
            angle1 = -(sectorlegs[i].sector2 + 90.0) - viewport.rotation * 180.0 / PI;
            angle2 = -(sectorlegs[i].sector1 + 90.0) - viewport.rotation * 180.0 / PI;
            if( angle1 > angle2 ) {
                angle2 += 360.0;
            }
            int lpx = lightPos.x;
            int lpy = lightPos.y;
            int npoints = 1;
            wxPoint arcpoints[150]; // Size relates to "step" below.

            arcpoints[0].x = lpx + (int) ( rangePx * cos( angle1 * PI / 180. ) );
            arcpoints[0].y = lpy - (int) ( rangePx * sin( angle1 * PI / 180. ) );
            double step = 3.0;
            while( (step < 15) && ((rangePx * sin(step * PI / 180.)) < 10) ) step += 2.0; // less points on small arcs

            // Make sure we start and stop exactly on the leg lines.
            int narc = ( angle2 - angle1 ) / step;
            narc++;
            step = ( angle2 - angle1 ) / (double)narc;

            if( angle2 - angle1 < 15 && sectorlegs[i].fillSector ) {
                wxPoint yellowCone[3];
                yellowCone[0] = lightPos;
                yellowCone[1] = end1;
                yellowCone[2] = end2;
                arcpen = wxThePenList->FindOrCreatePen( wxColor( 0,0,0,0 ), 1, wxSOLID );
                dc.SetPen( *arcpen );
                wxColor c = sectorlegs[i].color;
                c.Set( c.Red(), c.Green(), c.Blue(), 0.6*c.Alpha() );
                dc.SetBrush( wxBrush( c ) );
                dc.StrokePolygon( 3, yellowCone, 0, 0 );
                legOpacity = 50;
            } else {
                for( double a = angle1; a <= angle2 + 0.1; a += step ) {
                    int x = lpx + (int) ( rangePx * cos( a * PI / 180. ) );
                    int y = lpy - (int) ( rangePx * sin( a * PI / 180. ) );
                    arcpoints[npoints].x = x;
                    arcpoints[npoints].y = y;
                    npoints++;
                }
                dc.StrokeLines( npoints, arcpoints );
                legOpacity = 128;
            }

            arcpen = wxThePenList->FindOrCreatePen( wxColor( 0,0,0,legOpacity ), 1, wxSOLID );
            dc.SetPen( *arcpen );

            // Only draw each leg line once.

            bool haveAngle1 = false;
            bool haveAngle2 = false;
            for( unsigned int j=0; j<sectorangles.size(); j++ ) {
                if( sectorangles[j] == (int)sectorlegs[i].sector1 ) haveAngle1 = true;
                if( sectorangles[j] == (int)sectorlegs[i].sector2 ) haveAngle2 = true;
            }

            if( ! haveAngle1 ) {
                dc.StrokeLine( lightPos, end1 );
                sectorangles.push_back( (int) sectorlegs[i].sector1 );
            }

            if( ! haveAngle2 ) {
                dc.StrokeLine( lightPos, end2 );
                sectorangles.push_back( (int) sectorlegs[i].sector2 );
            }
        }
    }
}

bool s57_CheckExtendedLightSectors( int mx, int my, ViewPort& viewport, std::vector<s57Sector_t>& sectorlegs ) {
    double cursor_lat, cursor_lon;
    static double lastLat, lastLon;

    if( !ps52plib || !ps52plib->m_bExtendLightSectors ) return false;

    cc1->GetCanvasPixPoint ( mx, my, cursor_lat, cursor_lon );

    if( lastLat == cursor_lat && lastLon == cursor_lon ) return false;

    lastLat = cursor_lat;
    lastLon = cursor_lon;
    bool newSectorsNeedDrawing = false;

    ChartBase *targetchart = cc1->GetChartAtCursor();
    s57chart *chart = dynamic_cast<s57chart*>( targetchart );

    if( chart ) {
        sectorlegs.clear();

        float selectRadius = 16 / ( viewport.view_scale_ppm * 1852 * 60 );

        ListOfObjRazRules* rule_list =
                chart->GetObjRuleListAtLatLon( cursor_lat, cursor_lon, selectRadius, &viewport );

        wxPoint2DDouble lightPosD(0,0);

        for( ListOfObjRazRules::Node *node = rule_list->GetLast(); node; node = node->GetPrevious() ) {
            ObjRazRules *current = node->GetData();
            S57Obj* light = current->obj;
            char* curr_att;
            int attrCounter;
            double sectr1 = -1;
            double sectr2 = -1;
            double valnmr = -1;
            wxString curAttrName;
            wxColor color;

            if( !strcmp( light->FeatureName, "LIGHTS" ) ) {
                wxPoint2DDouble objPos( light->m_lat, light->m_lon );
                if( lightPosD.m_x == 0 && lightPosD.m_y == 0.0 ) lightPosD = objPos;
                if( lightPosD == objPos ) {
                    char *curr_att0 = (char *) calloc( light->attList->Len() + 1, 1 );
                    strncpy( curr_att0, light->attList->mb_str(), light->attList->Len() );
                    curr_att = curr_att0;

                    attrCounter = 0;
                    int noAttr = 0;
                    bool inDepthRange = false;
                    s57Sector_t sector;

                    while( *curr_att ) {
                        curAttrName.Clear();
                        noAttr++;
                        while( ( *curr_att ) && ( *curr_att != '\037' ) ) {
                            char t = *curr_att++;
                            curAttrName.Append( t );
                        }

                        if( *curr_att == '\037' ) curr_att++;

                        wxString value = chart->GetObjectAttributeValueAsString( light, attrCounter, curAttrName );
                        int opacity = 100;
                        if( cc1->GetColorScheme() == GLOBAL_COLOR_SCHEME_DUSK ) opacity = 50;
                        if( cc1->GetColorScheme() == GLOBAL_COLOR_SCHEME_NIGHT) opacity = 20;

                        int yOpacity = (float)opacity*1.3; // Matched perception with red/green

                        if( curAttrName == _T("SECTR1") ) value.ToDouble( &sectr1 );
                        if( curAttrName == _T("SECTR2") ) value.ToDouble( &sectr2 );
                        if( curAttrName == _T("VALNMR") ) value.ToDouble( &valnmr );
                        if( curAttrName == _T("COLOUR") ) {
                            sector.fillSector = true;
                            color = wxColor( 255, 255, 0, yOpacity );
                            if( value == _T("red(3)") ) {
                                color = wxColor( 255, 0, 0, opacity );
                                sector.fillSector = false;
                            }
                            if( value == _T("green(4)") ) {
                                color = wxColor( 0, 255, 0, opacity );
                                sector.fillSector = false;
                            }
                        }
                        if( curAttrName == _T("EXCLIT") ) {
                            if( value.Find( _T("(3)") ) ) valnmr = 1.0;  // Fog lights.
                        }
                        attrCounter++;
                    }

                    free(curr_att0);

                    if( ( sectr1 >= 0 ) && ( sectr2 >= 0 ) ) {
                        sector.pos.m_x = light->m_lon;
                        sector.pos.m_y = light->m_lat;

                        sector.range = (valnmr > 0.0) ? valnmr : 2.5; // Short default range.
                        sector.sector1 = sectr1;
                        sector.sector2 = sectr2;
                        sector.color = color;

                        bool newsector = true;
                        for( unsigned int i=0; i<sectorlegs.size(); i++ ) {
                            if( sectorlegs[i].pos == sector.pos &&
                                sectorlegs[i].sector1 == sector.sector1 &&
                                sectorlegs[i].sector2 == sector.sector2 ) newsector = false;
                        }
                        if( newsector ) {
                            sectorlegs.push_back( sector );
                            newSectorsNeedDrawing = true;
                        }
                    }
                }
            }
        }

        rule_list->Clear();
        delete rule_list;
    }
    return newSectorsNeedDrawing;
}
