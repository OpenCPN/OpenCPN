/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  cm93 Chart Object
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/arrstr.h>
#include <wx/mstream.h>
#include <wx/spinctrl.h>
#include <wx/listctrl.h>

#include "ogr_api.h"
#include "s57chart.h"
#include "cm93.h"
#include "s52plib.h"
#include "georef.h"
#include "mygeom.h"
#include "cutil.h"
#include "navutil.h"                            // for LogMessageOnce
#include "ocpn_pixel.h"                         // for ocpnUSE_DIBSECTION
#include "ocpndc.h"
#include "pluginmanager.h"  // for PlugInManager
#include "OCPNPlatform.h"
#include "wx28compat.h"

#include <stdio.h>

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif


#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

extern ChartCanvas               *cc1;
extern CM93OffsetDialog          *g_pCM93OffsetDialog;
extern OCPNPlatform     *g_Platform;
extern wxString         g_SENCPrefix;
extern s52plib          *ps52plib;
extern MyConfig         *pConfig;
extern bool             g_bDebugCM93;
extern int              g_cm93_zoom_factor;
extern CM93DSlide       *pCM93DetailSlider;
extern int              g_cm93detail_dialog_x, g_cm93detail_dialog_y;
extern bool             g_bShowCM93DetailSlider;
extern wxString         g_PrivateDataDir;

// Flav add for CM93Offset manual setup
extern double           g_CM93Maps_Offset_x;
extern double           g_CM93Maps_Offset_y;
extern bool             g_CM93Maps_Offset_on;
extern bool             g_bopengl;
extern PlugInManager    *g_pi_manager;
extern float            g_GLMinSymbolLineWidth;

extern bool  g_b_EnableVBO;
extern PFNGLBINDBUFFERPROC                 s_glBindBuffer;

// TODO  These should be gotten from the ctor
extern MyFrame          *gFrame;


#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY ( Array_Of_M_COVR_Desc );

#include <wx/listimpl.cpp>
WX_DEFINE_LIST ( List_Of_M_COVR_Desc );


void appendOSDirSep ( wxString* pString )
{
      wxChar sep = wxFileName::GetPathSeparator();
      if ( pString->Last() != sep )
            pString->Append ( sep );
}


//----------------------------------------------------------------------------
//    M_COVR_Desc Implementation
//----------------------------------------------------------------------------

M_COVR_Desc::M_COVR_Desc()
{
      pvertices = NULL;
      gl_screen_vertices = NULL;
      gl_screen_projection_type = PROJECTION_UNKNOWN;

      user_xoff = 0.;
      user_yoff = 0.;
      m_centerlat_cos = 1.0;
      m_buser_offsets = false;
      
      m_ngl_vertices = 0;
      gl_screen_vertices = NULL;

}

M_COVR_Desc::~M_COVR_Desc()
{
      delete[] pvertices;
      delete[] gl_screen_vertices;
}

int M_COVR_Desc::GetWKBSize()
{
      int size = 0;

      size = sizeof ( int );                          // size itself
      size += sizeof ( int );                         // m_cell_index;
      size += sizeof ( int );                         // m_object_id;
      size += sizeof ( int );                        // m_subcell
      size += sizeof ( int );                         // m_nvertices;
      size += m_nvertices * sizeof ( float_2Dpt );    // pvertices;

      size += sizeof ( int );                         // m_npub_year;
      size += 8 * sizeof ( double );                  // all the rest

      return size;
}

bool M_COVR_Desc:: WriteWKB ( void *p )
{
      if ( p )
      {
            int *pr = ( int * ) p;
            *pr++ = GetWKBSize();

            *pr++ = m_cell_index;
            *pr++ = m_object_id;
            *pr++ = m_subcell;

            *pr++ = m_nvertices;

            float_2Dpt *pfo = ( float_2Dpt * ) pr;
            float_2Dpt *pfi = pvertices;
            for ( int i=0 ; i < m_nvertices ; i++ )
                  *pfo++ = *pfi++;

            int *pi = ( int * ) pfo;
            *pi++ = m_npub_year;


            double *pd = ( double * ) pi;
            *pd++ = transform_WGS84_offset_x;
            *pd++ = transform_WGS84_offset_y;
            *pd++ = m_covr_lat_min;
            *pd++ = m_covr_lat_max;
            *pd++ = m_covr_lon_min;
            *pd++ = m_covr_lon_max;

            double centerlat_cos = cos( ((m_covr_lat_min + m_covr_lat_max)/2.) * PI/180. );
            
            *pd++ = user_xoff * centerlat_cos;
            *pd++ = user_yoff * centerlat_cos;
      }

      return true;
}

int M_COVR_Desc:: ReadWKB ( wxFFileInputStream &ifs )
{
      //    Read the length of the WKB
      int length = 0;
      if ( !ifs.Read ( &length, sizeof ( int ) ).Eof() )
      {
            ifs.Read ( &m_cell_index, sizeof ( int ) );
            ifs.Read ( &m_object_id, sizeof ( int ) );
            ifs.Read ( &m_subcell, sizeof ( int ) );

            ifs.Read ( &m_nvertices, sizeof ( int ) );

            pvertices = new float_2Dpt[m_nvertices];

            ifs.Read ( pvertices,m_nvertices * sizeof ( float_2Dpt ) );

            ifs.Read ( &m_npub_year, sizeof ( int ) );

            ifs.Read ( &transform_WGS84_offset_x, sizeof ( double ) );
            ifs.Read ( &transform_WGS84_offset_y, sizeof ( double ) );
            ifs.Read ( &m_covr_lat_min, sizeof ( double ) );
            ifs.Read ( &m_covr_lat_max, sizeof ( double ) );
            ifs.Read ( &m_covr_lon_min, sizeof ( double ) );
            ifs.Read ( &m_covr_lon_max, sizeof ( double ) );

            m_centerlat_cos = cos( ((m_covr_lat_min + m_covr_lat_max)/2.) * PI/180. );
            
            ifs.Read ( &user_xoff, sizeof ( double ) );
            ifs.Read ( &user_yoff, sizeof ( double ) );

            user_xoff /= m_centerlat_cos;
            user_yoff /= m_centerlat_cos;
            
            if ( ( fabs ( user_xoff ) > 1. ) || ( fabs ( user_yoff ) > 1. ) )
                  m_buser_offsets = true;
            else
                  m_buser_offsets = false;

            m_covr_bbox.Set( m_covr_lat_min, m_covr_lon_min,
                             m_covr_lat_max, m_covr_lon_max );
      }
      return length;
}

OCPNRegion M_COVR_Desc::GetRegion ( const ViewPort &vp, wxPoint *pwp )
{
      float_2Dpt *p = pvertices;

      for ( int ip = 0 ; ip < m_nvertices ; ip++ )
      {

            double plon = p->x;
            if ( fabs ( plon - vp.clon ) > 180. )
            {
                  if ( plon > vp.clon )
                        plon -= 360.;
                  else
                        plon += 360.;
            }


            double easting, northing, epix, npix;
#if 0
            ViewPort avp = vp;
            wxPoint2DDouble q = avp.GetDoublePixFromLL( p->y, plon);
            easting = q.m_x, northing = q.m_y;
#else
            toSM ( p->y, plon + 360., vp.clat, vp.clon + 360, &easting, &northing );
#endif

//            easting -= transform_WGS84_offset_x;
            easting -=  user_xoff;
//            northing -= transform_WGS84_offset_y;
            northing -= user_yoff;

            epix = easting  * vp.view_scale_ppm;
            npix = northing * vp.view_scale_ppm;

            pwp[ip].x = ( int ) round ( ( vp.pix_width  / 2 ) + epix );
            pwp[ip].y = ( int ) round ( ( vp.pix_height / 2 ) - npix );

            p++;
      }

      return OCPNRegion ( m_nvertices, pwp );
}

//----------------------------------------------------------------------------
// cm93 covr_set object class
// This is a helper class which holds all the known information
// relating to cm93 cell MCOVR objects of a particular scale
//----------------------------------------------------------------------------

WX_DECLARE_HASH_MAP ( int, int, wxIntegerHash, wxIntegerEqual, cm93cell_hash );

char sig_version[] = "COVR1002";

class covr_set
{
      public:
            covr_set ( cm93chart *parent );
            ~covr_set();

            bool Init ( wxChar scale_char, wxString &prefix );
            unsigned int GetCoverCount() { return m_covr_array_outlines.GetCount(); }
            M_COVR_Desc *GetCover ( unsigned int im ) { return &m_covr_array_outlines.Item ( im ); }
            void Add_MCD ( M_COVR_Desc *pmcd );
            bool Add_Update_MCD ( M_COVR_Desc *pmcd );
            bool IsCovrLoaded ( int cell_index );
            int Find_MCD ( M_COVR_Desc *pmcd );
            M_COVR_Desc *Find_MCD ( int cell_index, int object_id, int sbcell );

            cm93chart         *m_pParent;
            wxChar            m_scale_char;
            int               m_scale;

            wxString          m_cachefile;

            Array_Of_M_COVR_Desc    m_covr_array_outlines;        // array, for chart outline rendering

            cm93cell_hash     m_cell_hash;                        // This is a hash, indexed by cell index, elements contain the number of M_COVRs
            // found on this particular cell
};

covr_set::covr_set ( cm93chart *parent )
{
      m_pParent = parent;
}

covr_set::~covr_set()
{
      //    Create/Update the cache
      if(m_cachefile.IsEmpty())
            return;                             // presumably for Z scale charts
                                                // for which we create no cache

      if ( m_covr_array_outlines.GetCount() )
      {
            wxFFileOutputStream ofs ( m_cachefile );
            if ( ofs.IsOk() )
            {
                  ofs.Write ( sig_version, 8 );             // write signature

                  for ( unsigned int i=0 ; i < m_covr_array_outlines.GetCount() ; i++ )
                  {
                        int wkbsize = m_covr_array_outlines[i].GetWKBSize();
                        if ( wkbsize )
                        {
                              char *p = ( char* ) malloc ( wkbsize * sizeof ( char ) );
                              m_covr_array_outlines[i].WriteWKB ( p );
                              ofs.Write ( p, wkbsize );
                              free ( p );
                        }
                  }
                  ofs.Close();
            }
      }
}

bool covr_set::Init ( wxChar scale_char, wxString &prefix )
{
      m_scale_char = scale_char;

      switch ( m_scale_char )
      {
            case 'Z': m_scale = 20000000;  break;
            case 'A': m_scale =  3000000;  break;
            case 'B': m_scale =  1000000;  break;
            case 'C': m_scale =   200000;  break;
            case 'D': m_scale =   100000;  break;
            case 'E': m_scale =    50000;  break;
            case 'F': m_scale =    20000;  break;
            case 'G': m_scale =     7500;  break;
            default:  m_scale = 20000000;  break;
      }

      //    Create the cache file name
      wxString prefix_string = prefix;
      wxString sep ( wxFileName::GetPathSeparator() );
      prefix_string.Replace ( sep, _T ( "_" ) );
      prefix_string.Replace ( _T ( ":" ), _T ( "_" ) );       // for Windows

      m_cachefile = g_Platform->GetPrivateDataDir();
      appendOSDirSep ( &m_cachefile );

      m_cachefile += _T ( "cm93" );
      appendOSDirSep ( &m_cachefile );

      m_cachefile += prefix_string;          // include the cm93 prefix string in the cache file name
      m_cachefile += _T ( "_" );             // to support multiple cm93 data sets

      wxString cache_old_old_name = m_cachefile;
      cache_old_old_name += _T ( "coverset." );
      cache_old_old_name += m_scale_char;

      wxString cache_old_name = m_cachefile;
      cache_old_name += _T ( "coverset_sig." );
      cache_old_name += m_scale_char;

      m_cachefile += _T ( "coverset_sigp." );
      m_cachefile += m_scale_char;

      wxFileName fn ( m_cachefile );
      if ( !fn.DirExists() )
            wxFileName::Mkdir ( fn.GetPath(), 0777, wxPATH_MKDIR_FULL );


      //    Preload the cache
      if ( !wxFileName::FileExists ( m_cachefile ) )
      {
            // The signed file does not exist
            // Check for an old style file, and delete if found.
            if ( wxFileName::FileExists ( cache_old_name ) )
                  ::wxRemoveFile ( cache_old_name );
            if ( wxFileName::FileExists ( cache_old_old_name ) )
                  ::wxRemoveFile ( cache_old_old_name );
            return false;
      }

      wxFFileInputStream ifs ( m_cachefile );
      if ( ifs.IsOk() )
      {

            char sig_bytes[9];
            //    Validate the file signature
            if ( !ifs.Read ( &sig_bytes, 8 ).Eof() )
            {
                  if ( strncmp ( sig_bytes, sig_version, 8 ) )
                  {
                        return false;                              // bad signature match
                  }
            }
            else
                  return false;                                    // short file


            bool b_cont = true;
            while ( b_cont )
            {
                  M_COVR_Desc *pmcd = new M_COVR_Desc;
                  int length = pmcd->ReadWKB ( ifs );

                  if ( length )
                  {
                        m_covr_array_outlines.Add ( pmcd );

                        if ( m_cell_hash.find ( pmcd->m_cell_index ) == m_cell_hash.end() )
                              m_cell_hash[pmcd->m_cell_index] = 0;      // initialize the element

                        m_cell_hash[pmcd->m_cell_index]++;        // add this M_COVR to the hash map

                  }
                  else
                  {
                        delete pmcd;
                        b_cont = false;
                  }
            }
      }

      return true;
}

void covr_set::Add_MCD ( M_COVR_Desc *pmcd )
{
      m_covr_array_outlines.Add ( pmcd );

      if ( m_cell_hash.find ( pmcd->m_cell_index ) == m_cell_hash.end() )     // not present yet?
            m_cell_hash[pmcd->m_cell_index] = 0;  // initialize

      m_cell_hash[pmcd->m_cell_index]++;        // add this M_COVR to the hash map
}

bool covr_set::IsCovrLoaded ( int cell_index )
{
      return ( m_cell_hash.find ( cell_index ) != m_cell_hash.end() );
}

bool covr_set::Add_Update_MCD ( M_COVR_Desc *pmcd )
{
      if ( m_cell_hash.find ( pmcd->m_cell_index ) == m_cell_hash.end() )     // not present yet?
      {
            m_covr_array_outlines.Add ( pmcd );
            m_cell_hash[pmcd->m_cell_index] = 1;  // initialize
            return true;
      }
      //    There is at least one MCD already in place for this cell index
      //    We need to search the entire table to see if any of those MCD's
      //    correspond to this MCD's object identifier and subcell, as well as cell index
      else
      {
            bool b_found = false;
            for ( unsigned int i=0 ; i < m_covr_array_outlines.GetCount() ; i++ )
            {
                  M_COVR_Desc *pmcd_candidate = &m_covr_array_outlines.Item ( i );
                  if ( ( pmcd_candidate->m_cell_index == pmcd->m_cell_index ) &&
                          ( pmcd_candidate->m_object_id == pmcd->m_object_id ) &&
                          ( pmcd_candidate->m_subcell == pmcd->m_subcell ) )

                  {
                        b_found = true;
                        break;
                  }
            }

            if ( !b_found )
            {
                  m_covr_array_outlines.Add ( pmcd );
                  m_cell_hash[pmcd->m_cell_index]++;        // add this M_COVR to the hash map
                  return true;
            }
            else
                  return false;
      }
}

int covr_set::Find_MCD ( M_COVR_Desc *pmcd )
{
      if ( m_cell_hash.find ( pmcd->m_cell_index ) == m_cell_hash.end() )     // not present?
            return -1;
      else
      {
            //    There is at least one MCD already in place for this cell index
            //    We need to search the entire table to see if any of those MCD's
            //    correspond to this MCD's object identifier as well as cell index

            for ( unsigned int i=0 ; i < m_covr_array_outlines.GetCount() ; i++ )
            {
                  M_COVR_Desc *pmcd_candidate = &m_covr_array_outlines.Item ( i );
                  if ( ( pmcd_candidate->m_cell_index == pmcd->m_cell_index ) &&
                          ( pmcd_candidate->m_object_id == pmcd->m_object_id ) &&
                          ( pmcd_candidate->m_subcell == pmcd->m_subcell ) )
                  {
                        return ( int ) i;
                  }
            }
      }
      return -1;
}

M_COVR_Desc *covr_set::Find_MCD ( int cell_index, int object_id, int subcell )
{
      if ( m_cell_hash.find ( cell_index ) == m_cell_hash.end() )     // not present?
            return NULL;

      for ( unsigned int i=0 ; i < m_covr_array_outlines.GetCount() ; i++ )
      {
            M_COVR_Desc *pmcd_candidate = &m_covr_array_outlines.Item ( i );
            if ( ( pmcd_candidate->m_cell_index == cell_index ) &&
                    ( pmcd_candidate->m_object_id == object_id ) &&
                    ( pmcd_candidate->m_subcell == subcell ) )

                  return pmcd_candidate;
      }

      return NULL;
}




//    CM93 Encode/Decode support tables


static unsigned char Table_0[] =
{
      0x0CD,0x0EA,0x0DC,0x048,0x03E,0x06D,0x0CA,0x07B,0x052,0x0E1,0x0A4,0x08E,0x0AB,0x005,0x0A7,0x097,
      0x0B9,0x060,0x039,0x085,0x07C,0x056,0x07A,0x0BA,0x068,0x06E,0x0F5,0x05D,0x002,0x04E,0x00F,0x0A1,
      0x027,0x024,0x041,0x034,0x000,0x05A,0x0FE,0x0CB,0x0D0,0x0FA,0x0F8,0x06C,0x074,0x096,0x09E,0x00E,
      0x0C2,0x049,0x0E3,0x0E5,0x0C0,0x03B,0x059,0x018,0x0A9,0x086,0x08F,0x030,0x0C3,0x0A8,0x022,0x00A,
      0x014,0x01A,0x0B2,0x0C9,0x0C7,0x0ED,0x0AA,0x029,0x094,0x075,0x00D,0x0AC,0x00C,0x0F4,0x0BB,0x0C5,
      0x03F,0x0FD,0x0D9,0x09C,0x04F,0x0D5,0x084,0x01E,0x0B1,0x081,0x069,0x0B4,0x009,0x0B8,0x03C,0x0AF,
      0x0A3,0x008,0x0BF,0x0E0,0x09A,0x0D7,0x0F7,0x08C,0x067,0x066,0x0AE,0x0D4,0x04C,0x0A5,0x0EC,0x0F9,
      0x0B6,0x064,0x078,0x006,0x05B,0x09B,0x0F2,0x099,0x0CE,0x0DB,0x053,0x055,0x065,0x08D,0x007,0x033,
      0x004,0x037,0x092,0x026,0x023,0x0B5,0x058,0x0DA,0x02F,0x0B3,0x040,0x05E,0x07F,0x04B,0x062,0x080,
      0x0E4,0x06F,0x073,0x01D,0x0DF,0x017,0x0CC,0x028,0x025,0x02D,0x0EE,0x03A,0x098,0x0E2,0x001,0x0EB,
      0x0DD,0x0BC,0x090,0x0B0,0x0FC,0x095,0x076,0x093,0x046,0x057,0x02C,0x02B,0x050,0x011,0x00B,0x0C1,
      0x0F0,0x0E7,0x0D6,0x021,0x031,0x0DE,0x0FF,0x0D8,0x012,0x0A6,0x04D,0x08A,0x013,0x043,0x045,0x038,
      0x0D2,0x087,0x0A0,0x0EF,0x082,0x0F1,0x047,0x089,0x06A,0x0C8,0x054,0x01B,0x016,0x07E,0x079,0x0BD,
      0x06B,0x091,0x0A2,0x071,0x036,0x0B7,0x003,0x03D,0x072,0x0C6,0x044,0x08B,0x0CF,0x015,0x09F,0x032,
      0x0C4,0x077,0x083,0x063,0x020,0x088,0x0F6,0x0AD,0x0F3,0x0E8,0x04A,0x0E9,0x035,0x01C,0x05F,0x019,
      0x01F,0x07D,0x070,0x0FB,0x0D1,0x051,0x010,0x0D3,0x02E,0x061,0x09D,0x05C,0x02A,0x042,0x0BE,0x0E6
};

static unsigned char Encode_table[256];
static unsigned char Decode_table[256];

static bool  cm93_decode_table_created;

// Case-insensitive cm93 directory tree depth-first traversal to find the dictionary...
// This could be made simpler, but matches the old code better as is
class FindCM93Dictionary : public wxDirTraverser
{
      public:
            FindCM93Dictionary ( wxString& path ) : m_path ( path ) {}

            virtual wxDirTraverseResult OnFile ( const wxString& filename )
            {
                  wxString name = filename.AfterLast ( wxFileName::GetPathSeparator() ).Lower();
                  if ( name == wxT ( "cm93obj.dic" ) )
                  {
                        m_path = filename;
                        return wxDIR_STOP;
                  }

                  return wxDIR_CONTINUE;
            }

            virtual wxDirTraverseResult OnDir ( const wxString& WXUNUSED ( dirname ) )
            {
                  return wxDIR_CONTINUE;
            }

      private:
            wxString& m_path;
};

cm93_dictionary::cm93_dictionary()
{
      m_S57ClassArray   = NULL;
      m_AttrArray       = NULL;
      m_GeomTypeArray   = NULL;;
      m_ValTypeArray    = NULL;
      m_max_class       = 0;
      m_ok = false;

}


bool cm93_dictionary::LoadDictionary(const wxString & dictionary_dir)
{
      int i, nline;
      wxString line;
      wxString dir ( dictionary_dir );    // a copy
      bool  ret_val = false;

      wxChar sep = wxFileName::GetPathSeparator();
      if ( dir.Last() != sep )
            dir.Append ( sep );

      m_dict_dir = dir;


      //    Build some array strings for Feature decoding

      wxString sf ( dir );
      sf.Append ( _T ( "CM93OBJ.DIC" ) );

      if ( !wxFileName::FileExists ( sf ) )
      {
            sf = dir;
            sf.Append ( _T ( "cm93obj.dic" ) );
            if ( !wxFileName::FileExists ( sf ) )
                  return false;
      }

      wxTextFile file;
      if ( !file.Open ( sf ) )
            return false;


      nline = file.GetLineCount();

      if ( !nline )
            return false;

      //    Read the file once to get the max class number
      int iclass_max = 0;

      for ( i=0 ; i<nline ; i++ )
      {
            line = file.GetLine ( i );

            wxStringTokenizer tkz ( line, wxT ( "|" ) );
//            while ( tkz.HasMoreTokens() )
            {
                  //  6 char class name
                  wxString class_name = tkz.GetNextToken();

                  //  class number, ascii
                  wxString token = tkz.GetNextToken();
                  long liclass;
                  token.ToLong ( &liclass );
                  int iclass = liclass;
                  if ( iclass > iclass_max )
                        iclass_max = iclass;

                  //  geom type, ascii
                  wxString geo_type = tkz.GetNextToken();

            }

      }

      m_max_class = iclass_max;

      //    Create the class name array
      m_S57ClassArray = new wxArrayString;
      m_S57ClassArray->Add ( _T ( "NULLNM" ), iclass_max+1 );

      //    And an array of ints describing the geometry type per class
      m_GeomTypeArray = ( int * ) malloc ( ( iclass_max + 1 ) * sizeof ( int ) );

      //    Iterate over the file, filling in the values
      for ( i=0 ; i<nline ; i++ )
      {
            line = file.GetLine ( i );

            wxStringTokenizer tkz ( line, wxT ( "|" ) );
//           while ( tkz.HasMoreTokens() )
            {
                  //  6 char class name
                  wxString class_name = tkz.GetNextToken();

                  //  class number, ascii
                  wxString token = tkz.GetNextToken();
                  long liclass;
                  token.ToLong ( &liclass );
                  int iclass = liclass;

                  //  geom type, ascii
                  wxString geo_type = tkz.GetNextToken();

                  m_S57ClassArray->Insert ( class_name, iclass );
                  m_S57ClassArray->RemoveAt ( iclass + 1 );

                  int igeom_type = -1;                            // default unknown
                  wxChar geo_type_primary = geo_type[0];

                  if ( geo_type_primary == 'A' )
                        igeom_type = 3;
                  else if ( geo_type_primary == 'L' )
                        igeom_type = 2;
                  else if ( geo_type_primary == 'P' )
                        igeom_type = 1;

                  //    Note:  there are other types in the file, e.g. 'C'.  Dunno what this is
                  //    Also, some object classes want multiple geometries, like PA, PLA, etc.
                  //    Take only primary, ignore the rest

                  m_GeomTypeArray[iclass] = igeom_type;

            }
      }
      file.Close();

      //    Build some array strings for Attribute decoding

      wxString sfa ( dir );
      sfa.Append ( _T ( "ATTRLUT.DIC" ) );

      if ( !wxFileName::FileExists ( sfa ) )
      {
            sfa = dir;
            sfa.Append ( _T ( "attrlut.dic" ) );
      }

      if ( wxFileName::FileExists ( sfa ) )
      {
            wxFFileInputStream filea ( sfa );

            if ( filea.IsOk() )
            {
                  //    Read the file once to get the max attr number
                  int iattr_max = 0;


                  while ( !filea.Eof() )
                  {

                        //read a line
                        line.Empty();
                        while ( 1 )
                        {
                              char a = filea.GetC();
                              if ( filea.Eof() )
                                    break;
                              line.Append ( a );
                              if ( a == 0x0a )
                                    break;
                        }


                        if ( !line.StartsWith ( ( const wxChar * ) wxT ( ";" ) ) )
                        {
                              wxStringTokenizer tkz ( line, wxT ( "|" ) );
                              {
                                    //  6 attribute label
                                    wxString class_name = tkz.GetNextToken();

                                    //  attribute number, ascii
                                    wxString token = tkz.GetNextToken();
                                    long liattr;
                                    token.ToLong ( &liattr );
                                    int iattr = liattr;
                                    if ( iattr > iattr_max )
                                          iattr_max = iattr;
                              }
                        }
                  }

                  m_max_attr = iattr_max;

                  filea.SeekI ( 0 );


                  //    Create the attribute label array

                  m_AttrArray = new wxArrayString;
                  m_AttrArray->Add ( _T ( "NULLNM" ), iattr_max+1 );

                  //    And an array of chars describing the attribute value type
                  m_ValTypeArray = ( char * ) malloc ( ( iattr_max + 1 ) * sizeof ( char ) );


                  //    Iterate over the file, filling in the values
                  while ( !filea.Eof() )
                  {
                        //read a line
                        line.Empty();
                        while ( 1 )
                        {
                              char a = filea.GetC();
                              if ( filea.Eof() )
                                    break;
                              line.Append ( a );
                              if ( a == 0x0a )
                                    break;
                        }


                        if ( !line.StartsWith ( ( const wxChar * ) wxT ( ";" ) ) )
                        {
                              wxStringTokenizer tkz ( line, wxT ( "|" ) );
                              {
                                    //  6 char class name
                                    wxString attr_name = tkz.GetNextToken();

                                    //  class number, ascii
                                    wxString token = tkz.GetNextToken();
                                    long liattr;
                                    token.ToLong ( &liattr );
                                    int iattr = liattr;

                                    m_AttrArray->Insert ( attr_name, iattr );
                                    m_AttrArray->RemoveAt ( iattr + 1 );

                                    //    Skip some
                                    token = tkz.GetNextToken();
                                    token = tkz.GetNextToken();
                                    token = tkz.GetNextToken();
                                    token = tkz.GetNextToken().Trim();

                                    char atype = '?';
                                    if ( token.IsSameAs ( _T ( "aFLOAT" ) ) )
                                          atype = 'R';
                                    else if ( token.IsSameAs ( _T ( "aBYTE" ) ) )
                                          atype = 'B';
                                    else if ( token.IsSameAs ( _T ( "aSTRING" ) ) )
                                          atype = 'S';
                                    else if ( token.IsSameAs ( _T ( "aCMPLX" ) ) )
                                          atype = 'C';
                                    else if ( token.IsSameAs ( _T ( "aLIST" ) ) )
                                          atype = 'L';
                                    else if ( token.IsSameAs ( _T ( "aWORD10" ) ) )
                                          atype = 'W';
                                    else if ( token.IsSameAs ( _T ( "aLONG" ) ) )
                                          atype = 'G';

                                    m_ValTypeArray[iattr] = atype;

                              }
                        }
                  }
                  ret_val = true;
            }
            else              // stream IsOK
            {
                  ret_val = false;
            }
      }


      else                    //    Look for alternate file
      {
            sfa = dir;
            sfa.Append ( _T ( "CM93ATTR.DIC" ) );

            if ( !wxFileName::FileExists ( sfa ) )
            {
                  sfa = dir;
                  sfa.Append ( _T ( "cm93attr.dic" ) );
            }

            if ( wxFileName::FileExists ( sfa ) )
            {
                  wxFFileInputStream filea ( sfa );

                  if ( filea.IsOk() )
                  {
                        //    Read the file once to get the max attr number
                        int iattr_max = 0;


                        while ( !filea.Eof() )
                        {

                              //read a line
                              line.Empty();
                              while ( 1 )
                              {
                                    char a = filea.GetC();
                                    if ( filea.Eof() )
                                          break;
                                    line.Append ( a );
                                    if ( a == 0x0a )
                                          break;
                              }


                              if ( !line.StartsWith ( ( const wxChar * ) wxT ( ";" ) ) )
                              {
                                    wxStringTokenizer tkz ( line, wxT ( "|" ) );
                                    if ( tkz.CountTokens() )
                                    {
                                          //  6 attribute label
                                          wxString class_name = tkz.GetNextToken();

                                          //  attribute number, ascii
                                          wxString token = tkz.GetNextToken();
                                          long liattr;
                                          token.ToLong ( &liattr );
                                          int iattr = liattr;
                                          if ( iattr > iattr_max )
                                                iattr_max = iattr;
                                    }
                              }
                        }

                        m_max_attr = iattr_max;

                        filea.SeekI ( 0 );


                        //    Create the attribute label array

                        m_AttrArray = new wxArrayString;
                        m_AttrArray->Add ( _T ( "NULLNM" ), iattr_max+1 );

                        //    And an array of chars describing the attribute value type
                        m_ValTypeArray = ( char * ) malloc ( ( iattr_max + 1 ) * sizeof ( char ) );
                        for ( int iat=0 ; iat < iattr_max + 1 ; iat++ )
                              m_ValTypeArray[iat] = '?';


                        //    Iterate over the file, filling in the values
                        while ( !filea.Eof() )
                        {
                              //read a line
                              line.Empty();
                              while ( 1 )
                              {
                                    char a = filea.GetC();
                                    if ( filea.Eof() )
                                          break;
                                    line.Append ( a );
                                    if ( a == 0x0a )
                                          break;
                              }


                              if ( !line.StartsWith ( ( const wxChar * ) wxT ( ";" ) ) )
                              {
                                    wxStringTokenizer tkz ( line, wxT ( "|\r\n" ) );
                                    if ( tkz.CountTokens() >= 3 )
                                    {
                                          //  6 char class name
                                          wxString attr_name = tkz.GetNextToken();

                                          //  class number, ascii
                                          wxString token = tkz.GetNextToken();
                                          long liattr;
                                          token.ToLong ( &liattr );
                                          int iattr = liattr;

                                          m_AttrArray->Insert ( attr_name, iattr );
                                          m_AttrArray->RemoveAt ( iattr + 1 );

                                          token = tkz.GetNextToken().Trim();

                                          char atype = '?';
                                          if ( token.IsSameAs ( _T ( "aFLOAT" ) ) )
                                                atype = 'R';
                                          else if ( token.IsSameAs ( _T ( "aBYTE" ) ) )
                                                atype = 'B';
                                          else if ( token.IsSameAs ( _T ( "aSTRING" ) ) )
                                                atype = 'S';
                                          else if ( token.IsSameAs ( _T ( "aCMPLX" ) ) )
                                                atype = 'C';
                                          else if ( token.IsSameAs ( _T ( "aLIST" ) ) )
                                                atype = 'L';
                                          else if ( token.IsSameAs ( _T ( "aWORD10" ) ) )
                                                atype = 'W';
                                          else if ( token.IsSameAs ( _T ( "aLONG" ) ) )
                                                atype = 'G';

                                          m_ValTypeArray[iattr] = atype;

                                    }
                              }
                        }
                        ret_val = true;
                  }
                  else              // stream IsOK
                        ret_val = false;
            }
      }

      if ( ret_val )
      {
            m_ok = true;

            wxString msg ( _T ( "Loaded CM93 Dictionary from " ) );
            msg.Append ( dir );
            wxLogMessage ( msg );
      }

      return ret_val;
}

wxString cm93_dictionary::GetClassName ( int iclass )
{
      if ( ( iclass > m_max_class ) || ( iclass < 0 ) )
            return ( _T ( "Unknown" ) );
      else
            return ( m_S57ClassArray->Item ( iclass ) );
}

wxString cm93_dictionary::GetAttrName ( int iattr )
{
      if ( ( iattr > m_max_attr ) || ( iattr < 0 ) )
            return ( _T ( "UnknownAttr" ) );
      else
            return ( m_AttrArray->Item ( iattr ) );
}

//      char vtype = m_pDict->m_ValTypeArray[iattr];
char cm93_dictionary::GetAttrType ( int iattr )
{
      if ( ( iattr > m_max_attr ) || ( iattr < 0 ) )
            return ( '?' );
      else
            return ( m_ValTypeArray[iattr] );
}




cm93_dictionary::~cm93_dictionary()
{
      delete m_S57ClassArray;
      free ( m_GeomTypeArray );
      delete m_AttrArray;
      free ( m_ValTypeArray );
}





//    CM93 Decode support routines

void CreateDecodeTable ( void )
{
      int i;
      for ( i=0 ; i < 256 ; i++ )
      {
            Encode_table[i] = Table_0[i] ^ 8;
      }

      for ( i=0 ; i < 256 ; i++ )
      {
            unsigned char a = Encode_table[i];
            Decode_table[ ( int ) a] = ( unsigned char ) i;
      }
}


static int   read_and_decode_bytes ( FILE *stream, void *p, int nbytes )
{
      if ( 0 == nbytes )                  // declare victory if no bytes requested
            return 1;

      //    read into callers buffer
      if ( !fread ( p, nbytes, 1, stream ) )
            return 0;

      //    decode inplace
      unsigned char *q = ( unsigned char * ) p;

      for ( int i=0 ; i < nbytes ; i++ )
      {
            unsigned char a = *q;
            int b = a;
            unsigned char c = Decode_table[b];
            *q = c;

            q++;
      }
      return 1;
}


static int read_and_decode_double ( FILE *stream, double *p )
{
      double t;
      //    read into temp buffer
      if ( !fread ( &t, sizeof ( double ), 1, stream ) )
            return 0;

      //    decode inplace
      unsigned char *q = ( unsigned char * ) &t;

      for ( unsigned int i=0 ; i < sizeof ( double ) ; i++ )
      {
            unsigned char a = *q;
            int b = a;
            unsigned char c = Decode_table[b];
            *q = c;

            q++;
      }

      //    copy to target
      *p = t;

      return 1;
}

static int read_and_decode_int ( FILE *stream, int *p )
{
      int t;
      //    read into temp buffer
      if ( !fread ( &t, sizeof ( int ), 1, stream ) )
            return 0;

      //    decode inplace
      unsigned char *q = ( unsigned char * ) &t;

      for ( unsigned int i=0 ; i < sizeof ( int ) ; i++ )
      {
            unsigned char a = *q;
            int b = a;
            unsigned char c = Decode_table[b];
            *q = c;

            q++;
      }

      //    copy to target
      *p = t;

      return 1;
}

static int read_and_decode_ushort ( FILE *stream, unsigned short *p )
{
      unsigned short t;
      //    read into temp buffer
      if ( !fread ( &t, sizeof ( unsigned short ), 1, stream ) )
            return 0;

      //    decode inplace
      unsigned char *q = ( unsigned char * ) &t;

      for ( unsigned int i=0 ; i < sizeof ( unsigned short ) ; i++ )
      {
            unsigned char a = *q;
            int b = a;
            unsigned char c = Decode_table[b];
            *q = c;

            q++;
      }

      //    copy to target
      *p = t;

      return 1;
}



//    Calculate the CM93 CellIndex integer for a given Lat/Lon, at a given scale

int Get_CM93_CellIndex ( double lat, double lon, int scale )
{
      int retval = 0;

      int dval;
      switch ( scale )
      {
            case 20000000: dval = 120; break;         // Z
            case  3000000: dval =  60; break;         // A
            case  1000000: dval =  30; break;         // B
            case   200000: dval =  12; break;         // C
            case   100000: dval =   3; break;         // D
            case    50000: dval =   1; break;         // E
            case    20000: dval =   1; break;         // F
            case     7500: dval =   1; break;         // G
            default: dval =   1; break;
      }

      //    Longitude
      double lon1 = ( lon + 360. ) * 3.;                    // basic cell size is 20 minutes
      while ( lon1 >= 1080.0 )
            lon1 -= 1080.0;
      unsigned short lon2 = ( unsigned short ) floor ( lon1 /dval );      // normalize
      unsigned short lon3 = lon2 * dval;

      retval = lon3;

      //    Latitude
      double lat1 = ( lat * 3. ) + 270. - 30;
      unsigned short lat2 = ( unsigned short ) floor ( lat1 / dval );      // normalize
      unsigned short lat3 = lat2 * dval;


      retval += ( lat3 + 30 ) * 10000;

      return retval;
}

//    Calculate the Lat/Lon of the lower left corner of a CM93 cell,
//    given a CM93 CellIndex and scale
//    Returned longitude value is always > 0
void Get_CM93_Cell_Origin ( int cellindex, int scale, double *lat, double *lon )
{
      //    Longitude
      double idx1 = cellindex % 10000;
      double lont = ( idx1 / 3. );

      *lon = lont;

      //    Latitude
      int idx2 = cellindex / 10000;
      double lat1 = idx2 - 270.;
      *lat = lat1 / 3.;
}


//    Answer the query: "Is there a cm93 cell at the specified scale which contains a given lat/lon?"
bool Is_CM93Cell_Present ( wxString &fileprefix, double lat, double lon, int scale_index )
{
      int scale;
      int dval;
      wxChar scale_char;

      switch ( scale_index )
      {
            case 0: scale =  20000000; dval = 120; scale_char = 'Z'; break;         // Z
            case 1: scale =   3000000; dval =  60; scale_char = 'A'; break;         // A
            case 2: scale =   1000000; dval =  30; scale_char = 'B'; break;         // B
            case 3: scale =    200000; dval =  12; scale_char = 'C'; break;         // C
            case 4: scale =    100000; dval =   3; scale_char = 'D'; break;         // D
            case 5: scale =     50000; dval =   1; scale_char = 'E'; break;         // E
            case 6: scale =     20000; dval =   1; scale_char = 'F'; break;         // F
            case 7: scale =      7500; dval =   1; scale_char = 'G'; break;         // G
            default: scale = 20000000; dval = 120; scale_char = ' '; break;
      }

      int cellindex = Get_CM93_CellIndex ( lat, lon, scale );


      //    Create the file name
      wxString file;

      int ilat = cellindex / 10000;
      int ilon = cellindex % 10000;

      int jlat = ( ( ( ilat - 30 ) / dval ) * dval ) + 30;        // normalize
      int jlon = ( ilon / dval ) * dval;

      int ilatroot = ( ( ( ilat - 30 ) / 60 ) * 60 ) + 30;
      int ilonroot = ( ilon / 60 ) * 60;

      wxString fileroot;
      fileroot.Printf ( _T ( "%04d%04d/" ), ilatroot, ilonroot );


      wxString sdir ( fileprefix );
      sdir += fileroot;
      sdir += scale_char;

      wxString tfile;
      tfile.Printf ( _T ( "?%03d%04d." ), jlat, jlon );
      tfile += scale_char;

      //    Validate that the directory exists, adjusting case if necessary
      if ( !::wxDirExists ( sdir ) )
      {
            wxString old_scalechar ( scale_char );
            wxString new_scalechar = old_scalechar.Lower();

            sdir = fileprefix;
            sdir += fileroot;
            sdir += new_scalechar;
      }


      if ( ::wxDirExists ( sdir ) )
      {
            wxDir dir ( sdir );

            wxArrayString file_array;
            int n_files = dir.GetAllFiles ( sdir, &file_array, tfile, wxDIR_FILES );

            if ( n_files )
                  return true;

            else
            {

                  //    Try with alternate case of m_scalechar
                  wxString old_scalechar ( scale_char );
                  wxString new_scalechar = old_scalechar.Lower();

                  wxString tfile1;
                  tfile1.Printf ( _T ( "?%03d%04d." ), jlat, jlon );
                  tfile1 += new_scalechar;

                  int n_files1 = dir.GetAllFiles ( sdir, &file_array, tfile1, wxDIR_FILES );

                  return ( n_files1 > 0 );
            }
      }
      else
            return false;
}


static int get_dval ( int native_scale )
{
      int dval;
      switch ( native_scale )
      {
            case 20000000: dval = 120; break;         // Z
            case  3000000: dval =  60; break;         // A
            case  1000000: dval =  30; break;         // B
            case   200000: dval =  12; break;         // C
            case   100000: dval =   3; break;         // D
            case    50000: dval =   1; break;         // E
            case    20000: dval =   1; break;         // F
            case     7500: dval =   1; break;         // G
            default: dval =   1; break;
      }
      return dval;
}


static bool read_header_and_populate_cib ( FILE *stream, Cell_Info_Block *pCIB )
{
      //    Read header, populate Cell_Info_Block

      //    This 128 byte block is read element-by-element, to allow for
      //    endian-ness correction by element.
      //    Unused elements are read and, well, unused.

      header_struct header;

      memset ( ( void * ) &header, 0, sizeof ( header ) );

      read_and_decode_double ( stream,&header.lon_min );
      read_and_decode_double ( stream,&header.lat_min );
      read_and_decode_double ( stream,&header.lon_max );
      read_and_decode_double ( stream,&header.lat_max );

      read_and_decode_double ( stream,&header.easting_min );
      read_and_decode_double ( stream,&header.northing_min );
      read_and_decode_double ( stream,&header.easting_max );
      read_and_decode_double ( stream,&header.northing_max );

      read_and_decode_ushort ( stream,&header.usn_vector_records );
      read_and_decode_int ( stream,&header.n_vector_record_points );
      read_and_decode_int ( stream,&header.m_46 );
      read_and_decode_int ( stream,&header.m_4a );
      read_and_decode_ushort ( stream,&header.usn_point3d_records );
      read_and_decode_int ( stream,&header.m_50 );
      read_and_decode_int ( stream,&header.m_54 );
      read_and_decode_ushort ( stream,&header.usn_point2d_records );
      read_and_decode_ushort ( stream,&header.m_5a );
      read_and_decode_ushort ( stream,&header.m_5c );
      read_and_decode_ushort ( stream,&header.usn_feature_records );

      read_and_decode_int ( stream,&header.m_60 );
      read_and_decode_int ( stream,&header.m_64 );
      read_and_decode_ushort ( stream,&header.m_68 );
      read_and_decode_ushort ( stream,&header.m_6a );
      read_and_decode_ushort ( stream,&header.m_6c );
      read_and_decode_int ( stream,&header.m_nrelated_object_pointers );

      read_and_decode_int ( stream,&header.m_72 );
      read_and_decode_ushort ( stream,&header.m_76 );

      read_and_decode_int ( stream,&header.m_78 );
      read_and_decode_int ( stream,&header.m_7c );


      //    Calculate and record the cell coordinate transform coefficients


      double delta_x = header.easting_max - header.easting_min;
      if ( delta_x < 0 )
          delta_x += CM93_semimajor_axis_meters * 2.0 * PI;              // add one trip around

      pCIB->transform_x_rate = delta_x / 65535;
      pCIB->transform_y_rate = ( header.northing_max - header.northing_min ) / 65535;

      pCIB->transform_x_origin = header.easting_min;
      pCIB->transform_y_origin = header.northing_min;

//      pCIB->m_cell_mcovr_array.Empty();

      //    Extract some table sizes from the header, and pre-allocate the tables
      //    We do it this way to avoid incremental realloc() calls, which are expensive

      pCIB->m_nfeature_records = header.usn_feature_records;
      pCIB->pobject_block = ( Object * ) calloc ( pCIB->m_nfeature_records * sizeof ( Object ), 1 );

      pCIB->m_n_point2d_records = header.usn_point2d_records;
      pCIB->p2dpoint_array = ( cm93_point * ) malloc ( pCIB->m_n_point2d_records * sizeof ( cm93_point ) );

      pCIB->pprelated_object_block = ( Object ** ) malloc ( header.m_nrelated_object_pointers * sizeof ( Object * ) );

      pCIB->object_vector_record_descriptor_block = ( vector_record_descriptor * ) malloc ( ( header.m_4a + header.m_46 ) * sizeof ( vector_record_descriptor ) );

      pCIB->attribute_block_top = ( unsigned char * ) calloc ( header.m_78, 1 );

      pCIB->m_nvector_records = header.usn_vector_records;
      pCIB->edge_vector_descriptor_block = ( geometry_descriptor * ) malloc ( header.usn_vector_records * sizeof ( geometry_descriptor ) );

      pCIB->pvector_record_block_top = ( cm93_point * ) malloc ( header.n_vector_record_points * sizeof ( cm93_point ) );

      pCIB->m_n_point3d_records = header.usn_point3d_records;
      pCIB->point3d_descriptor_block = ( geometry_descriptor * ) malloc ( pCIB->m_n_point3d_records * sizeof ( geometry_descriptor ) );

      pCIB->p3dpoint_array = ( cm93_point_3d * ) malloc ( header.m_50 * sizeof ( cm93_point_3d ) );

      return true;
}

static bool read_vector_record_table ( FILE *stream, int count, Cell_Info_Block *pCIB )
{
      bool brv;

      geometry_descriptor *p = pCIB->edge_vector_descriptor_block;
      cm93_point *q = pCIB->pvector_record_block_top;

      for ( int iedge=0 ; iedge < count ; iedge++ )
      {

            p->index = iedge;

            unsigned short npoints;
            brv = ! ( read_and_decode_ushort ( stream, &npoints ) == 0 );
            if ( !brv )
                  return false;

            p->n_points = npoints;
            p->p_points = q;

//           brv = read_and_decode_bytes(stream, q, p->n_points * sizeof(cm93_point));
//            if(!brv)
//                  return false;

            unsigned short x, y;
            for ( int index = 0 ; index <  p->n_points ; index++ )
            {
                  if ( !read_and_decode_ushort ( stream, &x ) )
                        return false;
                  if ( !read_and_decode_ushort ( stream, &y ) )
                        return false;

                  q[index].x = x;
                  q[index].y = y;
            }


            //    Compute and store the min/max of this block of n_points
            cm93_point *t = p->p_points;

            p->x_max = t->x;
            p->x_min = t->x;
            p->y_max = t->y;
            p->y_min = t->y;

            t++;

            for ( int j=0 ; j < p->n_points-1 ; j++ )
            {
                  if ( t->x >= p->x_max )
                        p->x_max = t->x;

                  if ( t->x <= p->x_min )
                        p->x_min = t->x;

                  if ( t->y >= p->y_max )
                        p->y_max = t->y;

                  if ( t->y <= p->y_max )
                        p->y_min = t->y;

                  t++;
            }

            //    Advance the block pointer
            q += p->n_points;

            //    Advance the geometry descriptor pointer
            p++;

      }

      return true;
}


static bool read_3dpoint_table ( FILE *stream, int count, Cell_Info_Block *pCIB )
{
      geometry_descriptor *p = pCIB->point3d_descriptor_block;
      cm93_point_3d *q = pCIB->p3dpoint_array;

      for ( int i = 0 ; i < count ; i++ )
      {
            unsigned short npoints;
            if ( !read_and_decode_ushort ( stream, &npoints ) )
                  return false;

            p->n_points = npoints;
            p->p_points = ( cm93_point * ) q;       // might not be the right cast

//            unsigned short t = p->n_points;

//            if(!read_and_decode_bytes(stream, q, t*6))
//                  return false;

            unsigned short x, y, z;
            for ( int index = 0 ; index < p->n_points ; index++ )
            {
                  if ( !read_and_decode_ushort ( stream, &x ) )
                        return false;
                  if ( !read_and_decode_ushort ( stream, &y ) )
                        return false;
                  if ( !read_and_decode_ushort ( stream, &z ) )
                        return false;

                  q[index].x = x;
                  q[index].y = y;
                  q[index].z = z;
            }


            p++;
            q++;
      }

      return true;
}


static bool read_2dpoint_table ( FILE *stream, int count, Cell_Info_Block *pCIB )
{

//      int rv = read_and_decode_bytes(stream, pCIB->p2dpoint_array, count * 4);

      unsigned short x, y;
      for ( int index = 0 ; index < count ; index++ )
      {
            if ( !read_and_decode_ushort ( stream, &x ) )
                  return false;
            if ( !read_and_decode_ushort ( stream, &y ) )
                  return false;

            pCIB->p2dpoint_array[index].x = x;
            pCIB->p2dpoint_array[index].y = y;
      }


      return true;
}


static bool read_feature_record_table ( FILE *stream, int n_features, Cell_Info_Block *pCIB )
{
      try
      {

            Object *pobj = pCIB->pobject_block;                // head of object array

            vector_record_descriptor *pobject_vector_collection = pCIB->object_vector_record_descriptor_block;

            Object **p_relob = pCIB->pprelated_object_block;             // head of previously allocated related object pointer block

            unsigned char *puc_var10 = pCIB->attribute_block_top;       //m_3a;
            int puc10count = 0;                 // should be same as header.m_78

            unsigned char object_type;
            unsigned char geom_prim;
            unsigned short obj_desc_bytes = 0;

            unsigned int t;
            unsigned short index;
            unsigned short n_elements;


            for ( int iobject = 0 ; iobject < n_features ; iobject++ )
            {

                  // read the object definition
                  read_and_decode_bytes ( stream, &object_type, 1 );           // read the object type
                  read_and_decode_bytes ( stream, &geom_prim, 1 );             // read the object geometry primitive type
                  read_and_decode_ushort ( stream, &obj_desc_bytes );          // read the object byte count

                  pobj->otype = object_type;
                  pobj->geotype = geom_prim;


                  switch ( pobj->geotype & 0x0f )
                  {
                        case 4:              // AREA
                        {

                              if ( !read_and_decode_ushort ( stream, &n_elements ) )
                                    return false;

                              pobj->n_geom_elements = n_elements;
                              t = ( pobj->n_geom_elements * 2 ) + 2;
                              obj_desc_bytes -= t;

                              pobj->pGeometry = pobject_vector_collection;           // save pointer to created vector_record_descriptor in the object

                              for ( unsigned short i = 0 ; i < pobj->n_geom_elements ; i++ )
                              {
                                    if ( !read_and_decode_ushort ( stream, &index ) )
                                          return false;

                                    if ( ( index & 0x1fff ) > pCIB->m_nvector_records )
                                          return false;               // error in this cell, ignore all of it

                                    geometry_descriptor *u = &pCIB->edge_vector_descriptor_block[ ( index & 0x1fff ) ];   //point to the vector descriptor

                                    pobject_vector_collection->pGeom_Description = u;
                                    pobject_vector_collection->segment_usage = ( unsigned char ) ( index >> 13 );

                                    pobject_vector_collection++;
                              }

                              break;
                        }                                   // AREA geom



                        case 2:                                         // LINE geometry
                        {

                              if ( !read_and_decode_ushort ( stream, &n_elements ) )      // read geometry element count
                                    return false;

                              pobj->n_geom_elements = n_elements;
                              t = ( pobj->n_geom_elements * 2 ) + 2;
                              obj_desc_bytes -= t;

                              pobj->pGeometry = pobject_vector_collection;                     // save pointer to created vector_record_descriptor in the object

                              for ( unsigned short i = 0 ; i < pobj->n_geom_elements ; i++ )
                              {
                                    unsigned short geometry_index;

                                    if ( !read_and_decode_ushort ( stream, &geometry_index ) )
                                          return false;


                                    if ( ( geometry_index & 0x1fff ) > pCIB->m_nvector_records )
//                                    *(int *)(0) = 0;                              // error
                                          return 0;               // error, bad pointer

                                    geometry_descriptor *u = &pCIB->edge_vector_descriptor_block[ ( geometry_index & 0x1fff ) ];  //point to the vector descriptor

                                    pobject_vector_collection->pGeom_Description = u;
                                    pobject_vector_collection->segment_usage = ( unsigned char ) ( geometry_index >> 13 );

                                    pobject_vector_collection++;
                              }

                              break;


                        }

                        case 1:
                        {
                              if ( !read_and_decode_ushort ( stream, &index ) )
                                    return false;

                              obj_desc_bytes -= 2;

                              pobj->n_geom_elements = 1;                 // one point

                              pobj->pGeometry = &pCIB->p2dpoint_array[index];            //cm93_point *

                              break;
                        }

                        case 8:
                        {
                              if ( !read_and_decode_ushort ( stream, &index ) )
                                    return false;
                              obj_desc_bytes -= 2;

                              pobj->n_geom_elements = 1;                 // one point

                              pobj->pGeometry = &pCIB->point3d_descriptor_block[index];          //geometry_descriptor *

                              break;
                        }

                  }           // switch



                  if ( ( pobj->geotype & 0x10 ) == 0x10 )        // children/related
                  {
                        unsigned char nrelated;
                        if ( !read_and_decode_bytes ( stream, &nrelated, 1 ) )
                              return false;

                        pobj->n_related_objects = nrelated;
                        t = ( pobj->n_related_objects * 2 ) + 1;
                        obj_desc_bytes -= t;

                        pobj->p_related_object_pointer_array = p_relob;
                        p_relob += pobj->n_related_objects;

                        Object **w = ( Object ** ) pobj->p_related_object_pointer_array;
                        for ( unsigned char j = 0 ; j < pobj->n_related_objects ; j++ )
                        {
                              if ( !read_and_decode_ushort ( stream, &index ) )
                                    return false;

                              if ( index > pCIB->m_nfeature_records )
//                              *(int *)(0) = 0;                              // error
                                    return false;

                              Object *prelated_object = &pCIB->pobject_block[index];
                              *w = prelated_object;                       // fwd link

                              prelated_object->p_related_object_pointer_array = pobj;              // back link, array of 1 element
                              w++;
                        }
                  }

                  if ( ( pobj->geotype & 0x20 ) == 0x20 )
                  {
                        unsigned short nrelated;
                        if ( !read_and_decode_ushort ( stream, &nrelated ) )
                              return false;

                        pobj->n_related_objects = ( unsigned char ) ( nrelated & 0xFF );
                        obj_desc_bytes -= 2;
                  }

                  if ( ( pobj->geotype & 0x40 ) == 0x40 )
                  {
                  }


                  if ( ( pobj->geotype & 0x80 ) == 0x80 )        // attributes
                  {

                        unsigned char nattr;
                        if ( !read_and_decode_bytes ( stream, &nattr, 1 ) )
                              return false;        //m_od

                        pobj->n_attributes = nattr;
                        obj_desc_bytes -= 5;

                        pobj->attributes_block = puc_var10;
                        puc_var10 += obj_desc_bytes;

                        puc10count += obj_desc_bytes;


                        if ( !read_and_decode_bytes ( stream, pobj->attributes_block, obj_desc_bytes ) )
                              return false;           // the attributes....

                        if ( ( pobj->geotype & 0x0f ) == 1 )
                        {
                        }

                  }


                  pobj++;                       // next object
            }

//      wxASSERT(puc10count == pCIB->m_22->m_78);
      }

      catch ( ... )
      {
            printf ( "catch on read_feature_record_table\n" );
      }

      return true;
}







bool Ingest_CM93_Cell ( const char * cell_file_name, Cell_Info_Block *pCIB )
{

      try
      {

            int file_length;

            //    Get the file length
            FILE *flstream = fopen ( cell_file_name, "rb" );
            if ( !flstream )
                  return false;

            fseek ( flstream, 0, SEEK_END );
            file_length = ftell ( flstream );
            fclose ( flstream );

            //    Open the file
            FILE *stream = fopen ( cell_file_name, "rb" );
            if ( !stream )
                  return false;

            //    Validate the integrity of the cell file

            unsigned short word0 = 0;;
            int int0 = 0;
            int int1 = 0;;

            read_and_decode_ushort ( stream, &word0 );     // length of prolog + header (10 + 128)
            read_and_decode_int ( stream, &int0 );         // length of table 1
            read_and_decode_int ( stream, &int1 );         // length of table 2

            int test = word0 + int0 + int1;
            if ( test != file_length )
            {
                  fclose ( stream );
                  return false;                           // file is corrupt
            }

            //    Cell is OK, proceed to ingest


            if ( !read_header_and_populate_cib ( stream, pCIB ) )
            {
                  fclose ( stream );
                  return false;
            }


            if ( !read_vector_record_table ( stream, pCIB->m_nvector_records, pCIB ) )
            {
                  fclose ( stream );
                  return false;
            }

            if ( !read_3dpoint_table ( stream, pCIB->m_n_point3d_records, pCIB ) )
            {
                  fclose ( stream );
                  return false;
            }

            if ( !read_2dpoint_table ( stream, pCIB->m_n_point2d_records, pCIB ) )
            {
                  fclose ( stream );
                  return false;
            }

            if ( !read_feature_record_table ( stream, pCIB->m_nfeature_records, pCIB ) )
            {
                  fclose ( stream );
                  return false;
            }

//      int file_end = ftell(stream);

//      wxASSERT(file_end == file_length);

            fclose ( stream );


            return true;
      }

      catch ( ... )
      {
            return false;
      }


}




//----------------------------------------------------------------------------------
//      cm93chart Implementation
//----------------------------------------------------------------------------------

cm93chart::cm93chart()
{
      m_ChartType = CHART_TYPE_CM93;

//    Create the decode table once, if needed
      if ( !cm93_decode_table_created )
      {
            CreateDecodeTable();
            cm93_decode_table_created = true;
      }

      m_pDict = NULL;
      m_pManager = NULL;

      m_current_cell_vearray_offset = 0;

      m_ncontour_alloc = 100;                   // allocate inital vertex count container array
      m_pcontour_array = ( int * ) malloc ( m_ncontour_alloc * sizeof ( int ) );

      //  Establish a common reference point for the cell
      ref_lat = 0.;
      ref_lon = 0.;



      //  Need a covr_set
      m_pcovr_set = new covr_set ( this );


      //    Make initial allocation of shared outline drawing buffer
      m_pDrawBuffer = ( wxPoint * ) malloc ( 4 * sizeof ( wxPoint ) );
      m_nDrawBufferSize = 1;

      //  Set up the chart context
      m_this_chart_context = (chart_context *)calloc( sizeof(chart_context), 1);
      m_this_chart_context->chart = this;
      
}

cm93chart::~cm93chart()
{
      free ( m_pcontour_array );

      delete m_pcovr_set;

      free ( m_pDrawBuffer );

}

void  cm93chart::Unload_CM93_Cell ( void )
{
      free ( m_CIB.pobject_block );
//      free(m_CIB.m_2a);
      free ( m_CIB.p2dpoint_array );
      free ( m_CIB.pprelated_object_block );
      free ( m_CIB.object_vector_record_descriptor_block );
      free ( m_CIB.attribute_block_top );
      free ( m_CIB.edge_vector_descriptor_block );
      free ( m_CIB.pvector_record_block_top );
      free ( m_CIB.point3d_descriptor_block );
      free ( m_CIB.p3dpoint_array );
}


//    The idea here is to suggest to upper layers the appropriate scale values to be used with this chart
//    If max is too large, performance suffers, and the charts are very cluttered onscreen.
//    If the min is too small, then the chart rendereding will be over-scaled, and accuracy suffers.
//    In some ways, this is subjective.....

double cm93chart::GetNormalScaleMin ( double canvas_scale_factor, bool b_allow_overzoom )
{
      switch ( GetNativeScale() )
      {
            case 20000000: return 3000000.;           // Z
            case  3000000: return 1000000.;           // A
            case  1000000: return 200000.;            // B
            case   200000: return 40000.;             // C
            case   100000: return 20000.;             // D
            case    50000: return 10000.;             // E
            case    20000: return 5000.;              // F
            case     7500: return 3500.;              // G
      }

      return 1.0;
}

double cm93chart::GetNormalScaleMax ( double canvas_scale_factor, int canvas_width )
{
      /* 
         XXX previous declaration hides overloaded virtual function 
            and it was calling:
         s57chart::GetNormalScaleMax( canvas_scale_factor, canvas_width )
         should we restore this behavior?
      */
      switch ( GetNativeScale() )
      {
            case 20000000: return 50000000.;          // Z
            case  3000000: return 6000000.;           // A
            case  1000000: return 2000000.;           // B
            case   200000: return 400000.;            // C
            case   100000: return 200000.;            // D
            case    50000: return 100000.;            // E
            case    20000: return 40000.;             // F
            case     7500: return 15000.;             // G
      }

      return 1.0e7;
}


void cm93chart::GetPointPix ( ObjRazRules *rzRules, float north, float east, wxPoint *r )
{
    wxPoint2DDouble en(east, north);
    GetPointPix(rzRules, &en, r, 1);
}

void cm93chart::GetPointPix ( ObjRazRules *rzRules, wxPoint2DDouble *en, wxPoint *r, int nPoints )
{
      S57Obj *obj = rzRules->obj;

      double xr =  obj->x_rate;
      double xo =  obj->x_origin;
      double yr =  obj->y_rate;
      double yo =  obj->y_origin;

      if(m_vp_current.m_projection_type == PROJECTION_MERCATOR) {
          if ( m_vp_current.GetBBox().GetMaxLon() >= 180. &&
               rzRules->obj->BBObj.GetMaxLon() < m_vp_current.GetBBox().GetMinLon() )
              xo += mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
          else
          if ( (m_vp_current.GetBBox().GetMinLon() <= -180. &&
                rzRules->obj->BBObj.GetMinLon() > m_vp_current.GetBBox().GetMaxLon()) ||
               (rzRules->obj->BBObj.GetMaxLon() >= 180 && m_vp_current.GetBBox().GetMinLon() <= 0.))
              xo -= mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;

          for ( int i=0 ; i < nPoints ; i++ )
          {
              double valx = ( en[i].m_x * xr ) + xo;
              double valy = ( en[i].m_y * yr ) + yo;

              r[i].x = ( ( valx - m_easting_vp_center ) * m_view_scale_ppm ) + m_pixx_vp_center + 0.5;
              r[i].y = m_pixy_vp_center - ( ( valy - m_northing_vp_center ) * m_view_scale_ppm ) + 0.5;
          }
      } else {
          for ( int i=0 ; i < nPoints ; i++ ) {
              double valx = ( en[i].m_x * xr ) + xo;
              double valy = ( en[i].m_y * yr ) + yo;

              double lat, lon;
              fromSM(valx - m_easting_vp_center, valy - m_northing_vp_center, m_vp_current.clat, m_vp_current.clon, &lat, &lon);

              r[i] = m_vp_current.GetPixFromLL(lat, lon);
          }
      }
}

void cm93chart::GetPixPoint ( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
#if 1
    vpt->GetLLFromPix(wxPoint(pixx, pixy), plat, plon);

//    if ( *plon < 0. )
//        *plon += 360.;

#else
      //    Use Mercator estimator
      int dx = pixx - ( vpt->pix_width / 2 );
      int dy = ( vpt->pix_height / 2 ) - pixy;

      double xp = ( dx * cos ( vpt->skew ) ) - ( dy * sin ( vpt->skew ) );
      double yp = ( dy * cos ( vpt->skew ) ) + ( dx * sin ( vpt->skew ) );

      double d_east = xp / vpt->view_scale_ppm;
      double d_north = yp / vpt->view_scale_ppm;

      double slat, slon;
      fromSM ( d_east, d_north, vpt->clat, vpt->clon, &slat, &slon );

      if ( slon > 360. )
            slon -= 360.;

      *plat = slat;
      *plon = slon;
#endif
}

bool cm93chart::AdjustVP ( ViewPort &vp_last, ViewPort &vp_proposed )
{
      if ( IsCacheValid() )
      {

            //      If this viewpoint is same scale as last...
            if ( vp_last.view_scale_ppm == vp_proposed.view_scale_ppm )
            {
                  //  then require this viewport to be exact integral pixel difference from last
                  //  adjusting clat/clat and SM accordingly
#if 1
                wxPoint2DDouble p = vp_proposed.GetDoublePixFromLL(ref_lat, ref_lon) -
                    vp_last.GetDoublePixFromLL(ref_lat, ref_lon);

                double xlat, xlon;
                vp_last.GetLLFromPix(wxPoint(round(p.m_x), round(p.m_y)), &xlat, &xlon);
#else
                  double prev_easting_c, prev_northing_c;
                  toSM ( vp_last.clat, vp_last.clon, ref_lat, ref_lon, &prev_easting_c, &prev_northing_c );

                  double easting_c, northing_c;
                  toSM ( vp_proposed.clat, vp_proposed.clon,  ref_lat, ref_lon, &easting_c, &northing_c );

                  double delta_pix_x = ( easting_c - prev_easting_c ) * vp_proposed.view_scale_ppm;
                  int dpix_x = ( int ) round ( delta_pix_x );
                  double dpx = dpix_x;

                  double delta_pix_y = ( northing_c - prev_northing_c ) * vp_proposed.view_scale_ppm;
                  int dpix_y = ( int ) round ( delta_pix_y );
                  double dpy = dpix_y;

                  double c_east_d = ( dpx / vp_proposed.view_scale_ppm ) + prev_easting_c;
                  double c_north_d = ( dpy / vp_proposed.view_scale_ppm ) + prev_northing_c;

                  double xlat, xlon;
                  fromSM ( c_east_d, c_north_d, ref_lat, ref_lon, &xlat, &xlon );
#endif
                  vp_proposed.clon = xlon;
                  vp_proposed.clat = xlat;

                  return true;
            }
      }

      return false;
}

//-----------------------------------------------------------------------
//              Calculate and Set ViewPoint Constants
//-----------------------------------------------------------------------

void cm93chart::SetVPParms ( const ViewPort &vpt )
{
      //    Save a copy for later reference

      m_vp_current = vpt;



      //  Set up local SM rendering constants
      m_pixx_vp_center = vpt.pix_width / 2;
      m_pixy_vp_center = vpt.pix_height / 2;
      m_view_scale_ppm = vpt.view_scale_ppm;

      toSM ( vpt.clat, vpt.clon, ref_lat, ref_lon, &m_easting_vp_center, &m_northing_vp_center );

      vp_transform.easting_vp_center = m_easting_vp_center;
      vp_transform.northing_vp_center = m_northing_vp_center;
      
      if ( g_bDebugCM93 )
      {
            //    Fetch the lat/lon of the screen corner points
            ViewPort vptl = vpt;
            LLBBox box = vptl.GetBBox();
            double ll_lon = box.GetMinLon();
            double ll_lat = box.GetMinLat();

            double ur_lon = box.GetMaxLon();
            double ur_lat = box.GetMaxLat();

            printf ( "cm93chart::SetVPParms   ll_lon: %g  ll_lat: %g   ur_lon: %g   ur_lat:  %g  m_dval: %g\n", ll_lon, ll_lat, ur_lon, ur_lat, m_dval );
      }


      //    Create an array of CellIndexes covering the current viewport
      ArrayOfInts vpcells = GetVPCellArray ( vpt );

      //    Check the member array to see if all these viewport cells have been loaded
      bool bcell_is_in;
      bool recalc_depth = false;

      for ( unsigned int i=0 ; i < vpcells.GetCount() ; i++ )
      {
            bcell_is_in = false;
            for ( unsigned int j=0 ; j < m_cells_loaded_array.GetCount() ; j++ )
            {
                  if ( vpcells.Item ( i ) == m_cells_loaded_array.Item ( j ) )
                  {
                        bcell_is_in = true;
                        break;
                  }
            }

            //    The cell is not in place, so go load it
            if ( !bcell_is_in )
            {
                  int cell_index = vpcells.Item ( i );

                  if ( loadcell_in_sequence ( cell_index, '0' ) ) // Base cell
                  {
                        ProcessVectorEdges();
                        CreateObjChain ( cell_index, ( int ) '0', vpt.view_scale_ppm );

                        ForceEdgePriorityEvaluate();              // need to re-evaluate priorities
                        recalc_depth = true;

                        m_cells_loaded_array.Add ( cell_index );

                        Unload_CM93_Cell();
                  }

                  char loadcell_key = 'A';               // starting

                  //    Load any subcells in sequence
                  //    On successful load, add it to the member list and process the cell
                  while ( loadcell_in_sequence ( cell_index, loadcell_key ) )
                  {
                        ProcessVectorEdges();
                        CreateObjChain ( cell_index, ( int ) loadcell_key, vpt.view_scale_ppm );

                        ForceEdgePriorityEvaluate();              // need to re-evaluate priorities

                        if ( wxNOT_FOUND == m_cells_loaded_array.Index ( cell_index ) )
                              m_cells_loaded_array.Add ( cell_index );

                        Unload_CM93_Cell();

                        loadcell_key++;
                  }
            }
      }
      if (recalc_depth) {
          ClearDepthContourArray();
          BuildDepthContourArray();
      }
}


ArrayOfInts cm93chart::GetVPCellArray ( const ViewPort &vpt )
{
      //    Fetch the lat/lon of the screen corner points
      ViewPort vptl = vpt;
      LLBBox box = vptl.GetBBox();
      double ll_lon = box.GetMinLon();
      double ll_lat = box.GetMinLat();

      double ur_lon = box.GetMaxLon();
      double ur_lat = box.GetMaxLat();

      //    Adjust to always positive for easier cell calculations
      if ( ll_lon < 0 )
      {
            ll_lon += 360;
            ur_lon += 360;
      }

      //    Create an array of CellIndexes covering the current viewport
      ArrayOfInts vpcells;

      int lower_left_cell = Get_CM93_CellIndex ( ll_lat, ll_lon, GetNativeScale() );
      vpcells.Add ( lower_left_cell );                // always add the lower left cell

      if ( g_bDebugCM93 )
            printf ( "cm93chart::GetVPCellArray   Adding %d\n", lower_left_cell );

      double rlat, rlon;
      Get_CM93_Cell_Origin ( lower_left_cell, GetNativeScale(), &rlat, &rlon );


      // Use exact integer math here
      //    It is more obtuse, but it removes dependency on FP rounding policy

      int loni_0 = ( int ) wxRound ( rlon * 3 );
      int loni_20 = loni_0 + ( int ) m_dval;            // already added the lower left cell
      int lati_20 = ( int ) wxRound ( rlat * 3 );


      while ( lati_20 < ( ur_lat * 3. ) )
      {
            while ( loni_20 < ( ur_lon * 3. ) )
            {
                  unsigned int next_lon = loni_20 + 1080;
                  while ( next_lon >= 1080 )
                        next_lon -= 1080;

                  unsigned int next_cell = next_lon;

                  next_cell += ( lati_20 + 270 ) * 10000;

                  vpcells.Add ( ( int ) next_cell );
                  if ( g_bDebugCM93 )
                        printf ( "cm93chart::GetVPCellArray   Adding %d\n", next_cell );

                  loni_20 += ( int ) m_dval;
            }
            lati_20 += ( int ) m_dval;
            loni_20 = loni_0;
      }

      return vpcells;
}



void cm93chart::ProcessVectorEdges ( void )
{
      //    Create the vector(edge) map for this cell, appending to the existing member hash map
      VE_Hash &vehash = Get_ve_hash();

      m_current_cell_vearray_offset = vehash.size();           // keys start at the current size
      geometry_descriptor *pgd = m_CIB.edge_vector_descriptor_block;

      for ( int iedge = 0 ; iedge < m_CIB.m_nvector_records ; iedge++ )
      {
            VE_Element *vep = new VE_Element;
            vep->index = iedge + m_current_cell_vearray_offset;
            vep->nCount = pgd->n_points;
            vep->pPoints = NULL;
            vep->max_priority = -99;            // Default

            if ( pgd->n_points )
            {
                  double *pPoints = ( double * ) malloc ( pgd->n_points * 2 * sizeof ( double ) );
                  vep->pPoints = pPoints;

                  cm93_point *ppt = pgd->p_points;
                  
                  //  Get a bounding box for the edge
                  double east_max = -1e7; double east_min = 1e7;
                  double north_max = -1e7; double north_min = 1e7;
                  
                  for ( int ip = 0 ; ip < pgd->n_points ; ip++ )
                  {
                        *pPoints++ = ppt->x;
                        *pPoints++ = ppt->y;
 
                        east_max = wxMax(east_max, ppt->x);
                        east_min = wxMin(east_min, ppt->x);
                        north_max = wxMax(north_max, ppt->y);
                        north_min = wxMin(north_min, ppt->y);
                        
                        ppt++;
                  }
                  
                  cm93_point p;
                  double lat1, lon1, lat2, lon2;
                  
                  //TODO  Not precisely correct, transform should account for "trans_WGS84_offset_x"
                  p.x = east_min;
                  p.y = north_min;
                  Transform ( &p, 0, 0, &lat1, &lon1 );
                  
                  p.x = east_max;
                  p.y = north_max;
                  Transform ( &p, 0, 0, &lat2, &lon2 );

//                  if(lon1 > lon2)
                  //                    lon2 += 360;

                  vep->BBox.Set( lat1, lon1, lat2, lon2);
                  
            }
            
            vehash[vep->index] = vep;

            pgd++;                              // next geometry descriptor
      }



}




int cm93chart::CreateObjChain ( int cell_index, int subcell, double view_scale_ppm )
{
      LUPrec           *LUP;
      LUPname          LUP_Name = PAPER_CHART;


      m_CIB.m_cell_mcovr_list.Clear();


//     CALLGRIND_START_INSTRUMENTATION

      Object *pobjectDef = m_CIB.pobject_block;           // head of object array
      m_CIB.b_have_offsets = false;                       // will be set if any M_COVRs in this cell have defined, non-zero WGS84 offsets
      m_CIB.b_have_user_offsets = false;                  // will be set if any M_COVRs in this cell have user defined offsets

      int iObj = 0;
      S57Obj *obj;
      
      double scale = gFrame->GetBestVPScale(this);
      int nativescale = GetNativeScale();

      while ( iObj < m_CIB.m_nfeature_records )
      {
            if ( ( pobjectDef != NULL ) )
            {
                  Extended_Geometry *xgeom = BuildGeom ( pobjectDef, NULL, iObj );

                  obj = NULL;
                  if ( NULL != xgeom )
                      obj = CreateS57Obj ( cell_index, iObj, subcell, pobjectDef, m_pDict, xgeom,
                                           ref_lat, ref_lon, GetNativeScale(), view_scale_ppm );

                  if ( obj )
                  {
                        wxString objnam  = obj->GetAttrValueAsString("OBJNAM");
                        wxString fe_name = wxString(obj->FeatureName, wxConvUTF8);
                        if ( fe_name == _T("_texto") )
                            objnam  = obj->GetAttrValueAsString("_texta");
                        if (objnam.Len() > 0) {
                            wxString cellname = wxString::Format(_T("%i_%i"), cell_index, subcell);
                            g_pi_manager->SendVectorChartObjectInfo( cellname, fe_name, objnam, obj->m_lat, obj->m_lon, scale, nativescale );
                        }
//      Build/Maintain the ATON floating/rigid arrays
                        if ( GEO_POINT == obj->Primitive_type )
                        {

                              // set floating platform
                              if ( ( !strncmp ( obj->FeatureName, "LITFLT", 6 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "LITVES", 6 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "BOY",    3 ) ) )
                              {
                                    pFloatingATONArray->Add ( obj );
                              }

                              // set rigid platform
                              if ( !strncmp ( obj->FeatureName, "BCN",    3 ) )
                                    pRigidATONArray->Add ( obj );


                              //    Mark the object as an ATON
                              if ( ( !strncmp ( obj->FeatureName,   "LIT",    3 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "LIGHTS", 6 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "BCN",    3 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "_slgto", 6 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "_boygn", 6 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "_bcngn", 6 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "_extgn", 6 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "TOWERS", 6 ) ) ||
                                      ( !strncmp ( obj->FeatureName, "BOY",    3 ) ) )
                              {
                                    obj->bIsAton = true;
                              }
                        }

                        //    Mark th object as an "associable depth area"
                        //    Flag is used by conditional symbology
                        if ( GEO_AREA == obj->Primitive_type )
                        {
                              if ( !strncmp ( obj->FeatureName, "DEPARE", 6 ) || !strncmp ( obj->FeatureName, "DRGARE", 6 ) )
                                    obj->bIsAssociable = true;
                        }


//      This is where Simplified or Paper-Type point features are selected
//      In the case where the chart needs alternate LUPS loaded, do so.
//      This case is triggered when the UpdateLUP() method has been called on a partially loaded chart.

                        switch ( obj->Primitive_type )
                        {
                              case GEO_POINT:
                              case GEO_META:
                              case GEO_PRIM:
                                  if ( PAPER_CHART == ps52plib->m_nSymbolStyle )
                                      LUP_Name = PAPER_CHART;
                                  else
                                      LUP_Name = SIMPLIFIED;
                                 
                                  if(m_b2pointLUPS)
                                  {
                                      LUPname  LUPO_Name;
                                      if ( PAPER_CHART == ps52plib->m_nSymbolStyle )
                                          LUPO_Name = SIMPLIFIED;
                                      else
                                          LUPO_Name = PAPER_CHART;
                                      
                                      //  Load the alternate LUP
                                      LUPrec *LUPO = ps52plib->S52_LUPLookup ( LUPO_Name, obj->FeatureName, obj );
                                      if( LUPO ) {
                                          ps52plib->_LUP2rules ( LUPO, obj );
                                          _insertRules ( obj,LUPO, this );
                                      }
                                  }
                                  break;

                              case GEO_LINE:
                                    LUP_Name = LINES;
                                    break;

                              case GEO_AREA:
                                    if ( PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle )
                                          LUP_Name = PLAIN_BOUNDARIES;
                                    else
                                          LUP_Name = SYMBOLIZED_BOUNDARIES;

                                    if(m_b2lineLUPS)
                                    {
                                        LUPname  LUPO_Name;
                                        if ( PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle )
                                            LUPO_Name = SYMBOLIZED_BOUNDARIES;
                                        else
                                            LUPO_Name = PLAIN_BOUNDARIES;
                                        
                                        //  Load the alternate LUP
                                        LUPrec *LUPO = ps52plib->S52_LUPLookup ( LUPO_Name, obj->FeatureName, obj );
                                        if( LUPO ) {
                                            ps52plib->_LUP2rules ( LUPO, obj );
                                            _insertRules ( obj,LUPO, this );
                                        }
                                    }
                                    break;
                        }

                        LUP = ps52plib->S52_LUPLookup ( LUP_Name, obj->FeatureName, obj );

                        if ( NULL == LUP )
                        {
                              if ( g_bDebugCM93 )
                              {
                                    wxString msg ( obj->FeatureName, wxConvUTF8 );
                                    msg.Prepend ( _T ( "   CM93 could not find LUP for " ) );
                                    LogMessageOnce ( msg );
                              }
                              if(0 == obj->nRef)
                                  delete obj;
                        }
                        else
                        {
//              Convert LUP to rules set
                              ps52plib->_LUP2rules ( LUP, obj );

//              Add linked object/LUP to the working set
                              _insertRules ( obj,LUP, this );

//              Establish Object's Display Category
                              obj->m_DisplayCat = LUP->DISC;
                              
//              Establish objects base display priority         
                              obj->m_DPRI = LUP->DPRI - '0';
                              
 //              Populate the chart context
                               obj->m_chart_context = m_this_chart_context;
                                          
                        }
                  }


            }

            else                    // objectdef == NULL
                  break;

            pobjectDef++;

            iObj++;
      }



//     CALLGRIND_STOP_INSTRUMENTATION

      return 1;
}



InitReturn cm93chart::Init ( const wxString& name, ChartInitFlag flags )
{

      m_FullPath = name;
      m_Description = m_FullPath;

      wxFileName fn ( name );

      if(!m_prefix.Len())
            m_prefix = fn.GetPath ( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );

      m_scalechar = fn.GetExt();



      //    Figure out the scale from the file name

      int scale;
      switch ( ( m_scalechar.mb_str() ) [ ( size_t ) 0] )
      {
            case 'Z': scale = 20000000;  break;
            case 'A': scale =  3000000;  break;
            case 'B': scale =  1000000;  break;
            case 'C': scale =   200000;  break;
            case 'D': scale =   100000;  break;
            case 'E': scale =    50000;  break;
            case 'F': scale =    20000;  break;
            case 'G': scale =     7500;  break;
            default:  scale = 20000000;  break;
      }

      m_Chart_Scale = scale;


      switch ( GetNativeScale() )
      {
            case 20000000: m_dval = 120; break;         // Z
            case  3000000: m_dval =  60; break;         // A
            case  1000000: m_dval =  30; break;         // B
            case   200000: m_dval =  12; break;         // C
            case   100000: m_dval =   3; break;         // D
            case    50000: m_dval =   1; break;         // E
            case    20000: m_dval =   1; break;         // F
            case     7500: m_dval =   1; break;         // G
            default: m_dval =   1; break;
      }

      //    Set the nice name
      wxString data = _T ( "CM93Chart " );
      data.Append ( m_scalechar );
      wxString s;
      s.Printf ( _T ( "  1/%d" ), m_Chart_Scale );
      data.Append ( s );
      m_Name = data;

      //    Initialize the covr_set
      if ( scale != 20000000 )
            m_pcovr_set->Init ( m_scalechar.mb_str() [ ( size_t ) 0], m_prefix );


      if ( flags == THUMB_ONLY )
      {
//            SetColorScheme(cs, false);

            return INIT_OK;
      }

      if(!m_pManager)
            m_pManager = new cm93manager;


      if ( flags == HEADER_ONLY )
            return CreateHeaderDataFromCM93Cell();


      //    Load the cm93 dictionary if necessary
      if ( !m_pDict )
      {
            if(m_pManager)
            {
                  if ( m_pManager->Loadcm93Dictionary ( name ) )
                        m_pDict = m_pManager->m_pcm93Dict;
                  else
                  {
                        wxLogMessage ( _T ( "   CM93Chart Init cannot locate CM93 dictionary." ) );
                        return INIT_FAIL_REMOVE;
                  }
            }
      }



      bReadyToRender = true;

      return INIT_OK;

}

Extended_Geometry *cm93chart::BuildGeom ( Object *pobject, wxFileOutputStream *postream, int iobject )

{
      wxString s;
      int geomtype;

      int geom_type_maybe = pobject->geotype;

      switch ( geom_type_maybe )
      {
            case 1:    geomtype = 1; break;
            case 2:    geomtype = 2; break;
            case 4:    geomtype = 3; break;
            case 129:  geomtype = 1; break;
            case 130:  geomtype = 2; break;
            case 132:  geomtype = 3; break;
            case 8:    geomtype = 8; break;
            case 16:   geomtype = 16; break;
            case 161:  geomtype = 1; break;    // lighthouse first child
            case 33:   geomtype = 1; break;
            default:
                  geomtype = -1; break;
      }

      int iseg;

      Extended_Geometry *ret_ptr = new Extended_Geometry;

      int lon_max, lat_max, lon_min, lat_min;
      lon_max = 0; lon_min = 65536; lat_max = 0; lat_min = 65536;

      switch ( geomtype )
      {

            case 3:                               // Areas
            {
                  vector_record_descriptor *psegs = ( vector_record_descriptor * ) pobject->pGeometry;

                  int nsegs = pobject->n_geom_elements;

                  ret_ptr->n_vector_indices = nsegs;
                  ret_ptr->pvector_index = ( int * ) malloc ( nsegs * 3 * sizeof ( int ) );

                  //Traverse the object once to get a maximum polygon vertex count
                  int n_maxvertex = 0;
                  for ( int i=0 ; i < nsegs ; i++ )
                  {
                        geometry_descriptor *pgd = ( geometry_descriptor* ) ( psegs[i].pGeom_Description );
                        n_maxvertex += pgd->n_points;
                  }

                  //TODO  May not need this fluff adder....
                  n_maxvertex += 1;       // fluff

                  wxPoint2DDouble *pPoints = ( wxPoint2DDouble * ) calloc ( ( n_maxvertex ) * sizeof ( wxPoint2DDouble ), 1 );

                  int ip = 1;
                  int n_prev_vertex_index = 1;
                  bool bnew_ring = true;
                  int ncontours = 0;
                  iseg = 0;

                  cm93_point start_point;
                  start_point.x = 0; start_point.y = 0;

                  cm93_point cur_end_point;
                  cur_end_point.x = 1; cur_end_point.y = 1;

                  int n_max_points = -1;
                  while ( iseg < nsegs )
                  {
                        int type_seg = psegs[iseg].segment_usage;

                        geometry_descriptor *pgd = ( geometry_descriptor* ) ( psegs[iseg].pGeom_Description );

                        int npoints = pgd->n_points;
                        cm93_point *rseg = pgd->p_points;

                        n_max_points = wxMax ( n_max_points, npoints );

                        //    Establish ring starting conditions
                        if ( bnew_ring )
                        {
                              bnew_ring = false;

                              if ( ( type_seg & 4 ) == 0 )
                                    start_point = rseg[0];
                              else
                                    start_point = rseg[npoints-1];
                        }



                        if ( ( ( type_seg & 4 ) == 0 ) )
                        {
                              cur_end_point = rseg[npoints-1];
                              for ( int j=0 ; j<npoints  ; j++ )
                              {
//                                    if(ncontours == 0)                             // outer ring describes envelope
                                    {
                                          lon_max = wxMax ( lon_max, rseg[j].x );
                                          lon_min = wxMin ( lon_min, rseg[j].x );
                                          lat_max = wxMax ( lat_max, rseg[j].y );
                                          lat_min = wxMin ( lat_min, rseg[j].y );
                                    }

                                    pPoints[ip].m_x = rseg[j].x;
                                    pPoints[ip].m_y = rseg[j].y;
                                    ip++;
                              }
                        }
                        else if ( ( type_seg & 4 ) == 4 )   // backwards
                        {
                              cur_end_point = rseg[0];
                              for ( int j=npoints-1 ; j>= 0  ; j-- )
                              {
//                                    if(ncontours == 0)                             // outer ring describes envelope
                                    {
                                          lon_max = wxMax ( lon_max, rseg[j].x );
                                          lon_min = wxMin ( lon_min, rseg[j].x );
                                          lat_max = wxMax ( lat_max, rseg[j].y );
                                          lat_min = wxMin ( lat_min, rseg[j].y );
                                    }

                                    pPoints[ip].m_x = rseg[j].x;
                                    pPoints[ip].m_y = rseg[j].y;
                                    ip++;
                              }
                        }

                        ip--;                                                 // skip the last point in each segment

                        ret_ptr->pvector_index[iseg * 3 + 0] = 0;//-1;                 // first connected node
                        ret_ptr->pvector_index[iseg * 3 + 1] = pgd->index + m_current_cell_vearray_offset;         // edge index
                        ret_ptr->pvector_index[iseg * 3 + 2] = 0;//-2;                 // last connected node

                        if ( ( cur_end_point.x == start_point.x ) && ( cur_end_point.y == start_point.y ) )
                        {
                              // done with a ring

                              ip++;                                                 // leave in ring closure point

                              int nRingVertex = ip - n_prev_vertex_index;

                              //    possibly increase contour array size
                              if ( ncontours > m_ncontour_alloc - 1 )
                              {
                                    m_ncontour_alloc *= 2;
                                    int * tmp = m_pcontour_array;
                                    m_pcontour_array = ( int * ) realloc ( m_pcontour_array, m_ncontour_alloc * sizeof ( int ) );
                                    if (NULL == tmp)
                                    {
                                        free (tmp);
                                        tmp = NULL;
                                    }
                              }
                              m_pcontour_array[ncontours] = nRingVertex;               // store the vertex count

                              bnew_ring = true;                                               // set for next ring
                              n_prev_vertex_index = ip;
                              ncontours++;

                        }
                        iseg++;
                  }           // while iseg


                  ret_ptr->n_max_edge_points = n_max_points;

                  ret_ptr->n_contours = ncontours;                          // parameters passed to trapezoid tesselator

                  if(0 == ncontours)
                      ncontours = 1;            // avoid 0 alloc
                  ret_ptr->contour_array = ( int * ) malloc ( ncontours * sizeof ( int ) );
                  memcpy ( ret_ptr->contour_array, m_pcontour_array, ncontours * sizeof ( int ) );

                  ret_ptr->vertex_array = pPoints;
                  ret_ptr->n_max_vertex = n_maxvertex;

                  ret_ptr->pogrGeom = NULL;

                  ret_ptr->xmin = lon_min;
                  ret_ptr->xmax = lon_max;
                  ret_ptr->ymin = lat_min;
                  ret_ptr->ymax = lat_max;

                  break;
            }           // case 3



            case 1:     //single points
            {
                  cm93_point *pt = ( cm93_point * ) pobject->pGeometry;
                  ret_ptr->pogrGeom = NULL; //t;

                  ret_ptr->pointx = pt->x;
                  ret_ptr->pointy = pt->y;
                  break;
            }

            case 2:                                               // LINE geometry
            {
                  vector_record_descriptor *psegs = ( vector_record_descriptor * ) pobject->pGeometry;

                  int nsegs = pobject->n_geom_elements;

                  ret_ptr->n_vector_indices = nsegs;
                  ret_ptr->pvector_index = ( int * ) malloc ( nsegs * 3 * sizeof ( int ) );

                  //    Calculate the number of points
                  int n_maxvertex = 0;
                  for ( int imseg = 0 ; imseg < nsegs ; imseg++ )
                  {
                        geometry_descriptor *pgd = ( geometry_descriptor* ) psegs->pGeom_Description;

                        n_maxvertex += pgd->n_points;
                        psegs++;
                  }


                  wxPoint2DDouble *pPoints = ( wxPoint2DDouble * ) malloc ( n_maxvertex * sizeof ( wxPoint2DDouble ) );

                  psegs = ( vector_record_descriptor * ) pobject->pGeometry;

                  int ip = 0;
                  int lon_max, lat_max, lon_min, lat_min;
                  lon_max = 0; lon_min = 65536; lat_max = 0; lat_min = 65536;
                  int n_max_points = -1;

                  for ( int iseg = 0 ; iseg < nsegs ; iseg++ )
                  {

                        int type_seg = psegs->segment_usage;

                        geometry_descriptor *pgd = ( geometry_descriptor* ) psegs->pGeom_Description;

                        psegs++;          // next segment

                        int npoints = pgd->n_points;
                        cm93_point *rseg = pgd->p_points;

                        n_max_points = wxMax ( n_max_points, npoints );

                        if ( ( ( type_seg & 4 ) != 4 ) )
                        {
                              for ( int j=0 ; j<npoints  ; j++ )
                              {
                                    lon_max = wxMax ( lon_max, rseg[j].x );
                                    lon_min = wxMin ( lon_min, rseg[j].x );
                                    lat_max = wxMax ( lat_max, rseg[j].y );
                                    lat_min = wxMin ( lat_min, rseg[j].y );

                                    pPoints[ip].m_x = rseg[j].x;
                                    pPoints[ip].m_y = rseg[j].y;
                                    ip++;

                              }
                        }

                        else if ( ( type_seg & 4 ) == 4 )   // backwards
                        {
                              for ( int j=npoints-1 ; j>= 0  ; j-- )
                              {
                                    lon_max = wxMax ( lon_max, rseg[j].x );
                                    lon_min = wxMin ( lon_min, rseg[j].x );
                                    lat_max = wxMax ( lat_max, rseg[j].y );
                                    lat_min = wxMin ( lat_min, rseg[j].y );

                                    pPoints[ip].m_x = rseg[j].x;
                                    pPoints[ip].m_y = rseg[j].y;
                                    ip++;
                              }
                        }

                        ret_ptr->pvector_index[iseg * 3 + 0] = 0;//-1;                 // first connected node
                        ret_ptr->pvector_index[iseg * 3 + 1] = pgd->index + m_current_cell_vearray_offset;         // edge index
                        ret_ptr->pvector_index[iseg * 3 + 2] = 0;//-2;                 // last connected node

                  }           //for

                  ret_ptr->n_max_edge_points = n_max_points;

                  ret_ptr->vertex_array = pPoints;
                  ret_ptr->n_max_vertex = n_maxvertex;

                  ret_ptr->pogrGeom = NULL;

                  ret_ptr->xmin = lon_min;
                  ret_ptr->xmax = lon_max;
                  ret_ptr->ymin = lat_min;
                  ret_ptr->ymax = lat_max;

                  break;
            }           //case 2  (lines)

            case 8:
            {
                  geometry_descriptor *pgd = ( geometry_descriptor* ) pobject->pGeometry;

                  int npoints = pgd->n_points;
                  cm93_point_3d *rseg = ( cm93_point_3d * ) pgd->p_points;

                  OGRMultiPoint *pSMP = new OGRMultiPoint;

                  int z;
                  double zp;
                  for ( int ip=0 ; ip < npoints ; ip++ )
                  {
                        z = rseg[ip].z;

                        //    This is a magic number if there ever was one.....
                        if ( z >= 12000 )
                              zp = double ( z - 12000 );
                        else
                              zp = z / 10.;

                        OGRPoint *ppoint = new OGRPoint ( rseg[ip].x, rseg[ip].y, zp );
                        pSMP->addGeometryDirectly ( ppoint );

                        lon_max = wxMax ( lon_max, rseg[ip].x );
                        lon_min = wxMin ( lon_min, rseg[ip].x );
                        lat_max = wxMax ( lat_max, rseg[ip].y );
                        lat_min = wxMin ( lat_min, rseg[ip].y );

                  }

                  ret_ptr->pogrGeom = pSMP;

                  ret_ptr->xmin = lon_min;
                  ret_ptr->xmax = lon_max;
                  ret_ptr->ymin = lat_min;
                  ret_ptr->ymax = lat_max;


                  break;
            }


            case 16:
                  break;                        // this is the case of objects with children
                  // the parent has no geometry.....

            default:
            {
                  wxPrintf ( _T ( "Unexpected geomtype %d for Feature %d\n" ), geomtype,iobject );
                  break;
            }

      }     // switch


      return ret_ptr;
}




void cm93chart::Transform ( cm93_point *s, double trans_x, double trans_y, double *lat, double *lon )
{
      //    Simple linear transform
      double valx = ( s->x * m_CIB.transform_x_rate ) + m_CIB.transform_x_origin;
      double valy = ( s->y * m_CIB.transform_y_rate ) + m_CIB.transform_y_origin;

      //    Add in the WGS84 offset corrections
      valx -= trans_x;
      valy -= trans_y;

      //    Convert to lat/lon
      *lat = ( 2.0 * atan ( exp ( valy/CM93_semimajor_axis_meters ) ) - PI/2. ) / DEGREE;
      *lon = ( valx / ( DEGREE * CM93_semimajor_axis_meters ) );

}


cm93_attr_block::cm93_attr_block ( void * block, cm93_dictionary *pdict )
{
      m_cptr = 0;
      m_block = ( unsigned char * ) block;
      m_pDict = pdict;
}


unsigned char *cm93_attr_block::GetNextAttr()
{
      //    return current pointer
      unsigned char *ret_val = m_block + m_cptr;

      //    Advance the pointer

      unsigned char iattr = * ( m_block + m_cptr );
      m_cptr++;

//      char vtype = m_pDict->m_ValTypeArray[iattr];
      char vtype = m_pDict->GetAttrType ( iattr );

      switch ( vtype )
      {
            case 'I':                           // never seen?
                  m_cptr += 2;
                  break;
            case 'B':
                  m_cptr += 1;
//                  pb = (unsigned char *)aval;
//                  sprintf(val, "%d", *pb);
//                  pvtype = 'I';                 // override
                  break;
            case 'S':
                  while ( * ( m_block + m_cptr ) )
                        m_cptr++;
                  m_cptr++;                           // skip terminator
//                  sprintf(val, "%s", aval);
                  break;
            case 'R':
                  m_cptr += 4;
//                  pf = (float *)aval;
//                  sprintf(val, "%g", *pf);
                  break;
            case 'W':
                  m_cptr += 2;
                  break;
            case 'G':
                  m_cptr += 4;
                  break;
            case 'C':
                  m_cptr += 3;
                  while ( * ( m_block + m_cptr ) )
                        m_cptr++;
                  m_cptr++;                           // skip terminator
//                  sprintf(val, "%s", &aval[3]);
//                  pvtype = 'S';                 // override
                  break;
            case 'L':
            {
                  unsigned char nl = * ( m_block + m_cptr );
                  m_cptr++;
                  m_cptr += nl;

//                  pb = (unsigned char *)aval;
//                  unsigned char nl = *pb++;
//                  char vi[20];
//                  val[0] = 0;
//                  for(int i=0 ; i<nl ; i++)
//                  {
//                        sprintf(vi, "%d,", *pb++);
//                        strcat(val, vi);
//                  }
//                  if(strlen(val))
//                        val[strlen(val)-1] = 0;         // strip last ","
//                  pvtype = 'S';                 // override
                  break;
            }
            default:
//                  sprintf(val, "Unknown Value Type");
                  break;
      }

      return ret_val;

}


wxString ParseSLGTA ( wxString& val )
{
      wxString result;
      char line[30];

      wxString s;
      wxStringTokenizer tkz ( val, wxT ( "|" ) );

      s = tkz.GetNextToken();
      s = tkz.GetNextToken();
      s = tkz.GetNextToken();             // Mark attributes

      //  Defaults, black can
      wxString sc, st, sp;
      int color = 0;  sc = _T ( "" );
      int type = 0;   st = _T ( "" );
      int colpat = 0; sp = _T ( "" );

      if ( s[0] == 'R' )
      {
            color = 3;
            sc = _T ( "3" );
      }

      else if ( s[0] == 'G' )
      {
            color = 4;
            sc = _T ( "4" );
      }
      else if ( s.Mid ( 0,3 ) == _T ( "W/O" ) )
      {
            color = 1;
            sc = _T ( "1,11" );

            colpat = 1;
            sp = _T ( "1" );
      }
      else if ( s.Mid ( 0,5 ) == _T ( "LIGHT" ) )
      {
            color = 0;
            type = 0;
      }


      if ( val.Find ( _T ( "Spar" ) ) !=  wxNOT_FOUND )
      {
            type = 5;
            st = _T ( "5" );
      }
      if ( val.Find ( _T ( "SPAR" ) ) !=  wxNOT_FOUND )
      {
            type = 5;
            st = _T ( "5" );
      }

      if ( ( type == 2 ) && ( color == 3 ) )     // red can?
      {
            type = 1;                      // change to nun
            st = _T ( "1" );
      }


      if ( color )
      {
            sprintf ( line, "  %s (%c) = %s", "COLOUR", 'I', ( const char * ) sc.mb_str() );
            result += wxString ( line, wxConvUTF8 );
            result += '\n';
            if ( !type )
            {
                  sprintf ( line, "  %s (%c) = %s", "BOYSHP", 'I', "4" );
                  result += wxString ( line, wxConvUTF8 );
                  result += '\n';
            }

      }

      if ( type )
      {
            sprintf ( line, "  %s (%c) = %s", "BOYSHP", 'I', ( const char * ) st.mb_str() );
            result += wxString ( line, wxConvUTF8 );
            result += '\n';
            if ( !color )
            {
                  sprintf ( line, "  %s (%c) = %s", "COLOUR", 'I', "2" );
                  result += wxString ( line, wxConvUTF8 );
                  result += '\n';
            }
      }


      if ( colpat )
      {
            sprintf ( line, "  %s (%c) = %s", "COLPAT", 'I', ( const char * ) sp.mb_str() );
            result += wxString ( line, wxConvUTF8 );
            result += '\n';
      }


      return result;
}


wxString ParseTEXTA ( wxString& val )
{
      wxString result;
      char line[30];

      if ( val.Contains ( _T ( "WK S" ) ) )
      {
            sprintf ( line, "  %s (%c) = %s", "WRKATT", 'I', "1" );
            result += wxString ( line, wxConvUTF8 );
            result += '\n';
      }

      return result;
}




void cm93chart::translate_colmar(const wxString &sclass, S57attVal *pattValTmp)
{
      int *pcur_attr = ( int * ) pattValTmp->value;
      int cur_attr = *pcur_attr;

      wxString lstring;

      switch ( cur_attr )
      {
            case 1: lstring = _T ( "4" ); break;            // green
            case 2: lstring = _T ( "2" ); break;            // black
            case 3: lstring = _T ( "3" ); break;            // red
            case 4: lstring = _T ( "6" ); break;            // yellow
            case 5: lstring = _T ( "1" ); break;            // white
            case 6: lstring = _T ( "11" ); break;           // orange
            case 7: lstring = _T ( "2,6" ); break;          // black/yellow
            case 8: lstring = _T ( "2,6,2" ); break;        // black/yellow/black
            case 9: lstring = _T ( "6,2" ); break;           // yellow/black
            case 10: lstring = _T ( "6,2,6" ); break;        // yellow/black/yellow
            case 11: lstring = _T ( "3,1" ); break;          // red/white
            case 12: lstring = _T ( "4,3,4" ); break;        // green/red/green
            case 13: lstring = _T ( "3,4,3" ); break;        // red/green/red
            case 14: lstring = _T ( "2,3,2" ); break;        // black/red/black
            case 15: lstring = _T ( "6,3,6" ); break;        // yellow/red/yellow
            case 16: lstring = _T ( "4,3" ); break;          // green/red
            case 17: lstring = _T ( "3,4" ); break;          // red/green
            case 18: lstring = _T ( "4,1" ); break;          // green/white
            default: break;
      }

      if ( lstring.Len() )
      {
            free ( pattValTmp->value );                       // free the old int pointer

            pattValTmp->valType = OGR_STR;
            pattValTmp->value = ( char * ) malloc ( lstring.Len() + 1 );      // create a new Lstring attribute
            strcpy ( ( char * ) pattValTmp->value, lstring.mb_str() );

      }
}


S57Obj *cm93chart::CreateS57Obj ( int cell_index, int iobject, int subcell, Object *pobject, cm93_dictionary *pDict, Extended_Geometry *xgeom,
                                  double ref_lat, double ref_lon, double scale, double view_scale_ppm )
{

#define MAX_HDR_LINE    4000

// printf("%d\n", iobject);


      int npub_year = 1993;                     // silly default

      int iclass = pobject->otype;
      int geomtype = pobject->geotype & 0x0f;

      double tmp_transform_x = 0.;
      double tmp_transform_y = 0.;

      //    Per object transfor offsets,
      double trans_WGS84_offset_x = 0.;
      double trans_WGS84_offset_y = 0.;

      wxString sclass = pDict->GetClassName ( iclass );
      if ( sclass == _T ( "Unknown" ) )
      {
            wxString msg;
            msg.Printf ( _T ( "   CM93 Error...object type %d not found in CM93OBJ.DIC" ), iclass );
            wxLogMessage ( msg );
            delete xgeom;
            return NULL;
      }

      wxString sclass_sub = sclass;

      //  Going to make some substitutions here
      if ( sclass.IsSameAs ( _T ( "ITDARE" ) ) )
            sclass_sub = _T ( "DEPARE" );

      if ( sclass.IsSameAs ( _T ( "_m_sor" ) ) )
            sclass_sub = _T ( "M_COVR" );

      if ( sclass.IsSameAs ( _T ( "SPOGRD" ) ) )
            sclass_sub = _T ( "DMPGRD" );

      if ( sclass.IsSameAs ( _T ( "FSHHAV" ) ) )
            sclass_sub = _T ( "FSHFAC" );

      if ( sclass.IsSameAs ( _T ( "OFSPRD" ) ) )
            sclass_sub = _T ( "CTNARE" );



      //    Create the S57 Object
      S57Obj *pobj = new S57Obj();

      pobj->Index = iobject;

      char u[201];
      strncpy ( u, sclass_sub.mb_str(), 199 );
      u[200] = '\0';
      strncpy ( pobj->FeatureName, u, 7 );

      //  Touch up the geom types
      int geomtype_sub = geomtype;
      if ( geomtype == 8 )                    // sounding....
            geomtype_sub = 1;

      if ( geomtype == 4 )                    // convert cm93 area(4) to GDAL area(3)...
            geomtype_sub = 3;

      pobj->attVal =  new wxArrayOfS57attVal();


      cm93_attr_block pab( pobject->attributes_block, pDict );


      for ( int jattr = 0 ; jattr  < pobject->n_attributes ; jattr++ )
      {

            unsigned char *curr_attr = pab.GetNextAttr();

            unsigned char iattr = *curr_attr;

            wxString sattr = pDict->GetAttrName ( iattr );

            char vtype = pDict->GetAttrType ( iattr );

            unsigned char *aval = curr_attr + 1;

            char val[4000];
            int *pi;
            float *pf;
            unsigned short *pw;
            unsigned char *pb;
            int *pAVI;
            char *pAVS;
            double *pAVR;
            int nlen;
            double dival;
            int ival;

            S57attVal *pattValTmp = new S57attVal;

            switch ( vtype )
            {
                  case 'I':                           // never seen?
                        pi = ( int * ) aval;
                        pAVI = ( int * ) malloc ( sizeof ( int ) );         //new int;
                        *pAVI = *pi;
                        pattValTmp->valType = OGR_INT;
                        pattValTmp->value   = pAVI;
                        break;
                  case 'B':
                        pb = ( unsigned char * ) aval;
                        pAVI = ( int * ) malloc ( sizeof ( int ) );         //new int;
                        *pAVI = ( int ) ( *pb );
                        pattValTmp->valType = OGR_INT;
                        pattValTmp->value   = pAVI;
                        break;
                  case 'W':                                       // aWORD10
                        pw = ( unsigned short * ) aval;
                        ival = ( int ) ( *pw );
                        dival = ival;

                        pAVR = ( double * ) malloc ( sizeof ( double ) );   //new double;
                        *pAVR = dival/10.;
                        pattValTmp->valType = OGR_REAL;
                        pattValTmp->value   = pAVR;
                        break;
                  case 'G':
                        pi = ( int * ) aval;
                        pAVI = ( int * ) malloc ( sizeof ( int ) );         //new int;
                        *pAVI = ( int ) ( *pi );
                        pattValTmp->valType = OGR_INT;
                        pattValTmp->value   = pAVI;
                        break;

                  case 'S':
                        nlen = strlen ( ( const char * ) aval );
                        pAVS = ( char * ) malloc ( nlen + 1 );          ;
                        strcpy ( pAVS, ( char * ) aval );
                        pattValTmp->valType = OGR_STR;
                        pattValTmp->value   = pAVS;
                        break;

                  case 'C':
                        nlen = strlen ( ( const char * ) &aval[3] );
                        pAVS = ( char * ) malloc ( nlen + 1 );          ;
                        strcpy ( pAVS, ( const char * ) &aval[3] );
                        pattValTmp->valType = OGR_STR;
                        pattValTmp->value   = pAVS;
                        break;
                  case 'L':
                  {
                        pb = ( unsigned char * ) aval;
                        unsigned char nl = *pb++;
                        char vi[20];
                        val[0] = 0;
                        for ( int i=0 ; i<nl ; i++ )
                        {
                              sprintf ( vi, "%d,", *pb++ );
                              strcat ( val, vi );
                        }
                        if ( strlen ( val ) )
                              val[strlen ( val )-1] = 0;      // strip last ","

                        int nlen = strlen ( val );
                        pAVS = ( char * ) malloc ( nlen + 1 );          ;
                        strcpy ( pAVS, val );
                        pattValTmp->valType = OGR_STR;
                        pattValTmp->value   = pAVS;
                        break;
                  }
                  case 'R':
                      pAVR = ( double * ) malloc ( sizeof ( double ) );   //new double;
                      pf = ( float * ) aval;
#ifdef ARMHF
                        float tf1;
                        memcpy(&tf1, pf, sizeof(float));
                        *pAVR = tf1;
#else
                        *pAVR = *pf;
#endif
                        pattValTmp->valType = OGR_REAL;
                        pattValTmp->value   = pAVR;
                        break;
                  default:
                        sattr.Clear();               // Unknown, TODO track occasional case '?'
                        break;
            }     // switch


            if ( sattr.IsSameAs ( _T ( "COLMAR" ) ) )
            {
                  translate_colmar ( sclass, pattValTmp );
                  sattr = _T ( "COLOUR" );
            }
            // XXX should be done from s57 list ans cm93 list for any mismatch
            // ie cm93 QUASOU is an enum s57 is a list
            if ( pattValTmp->valType == OGR_INT && 
                  (sattr.IsSameAs ( _T ( "QUASOU" ) ) || sattr.IsSameAs ( _T ( "CATLIT" ) ))
               ) 
            {
                  int v = *(int*)pattValTmp->value;
                  free(pattValTmp->value);
                  sprintf ( val, "%d", v );
                  int nlen = strlen ( val );
                  pAVS = ( char * ) malloc ( nlen + 1 );          ;
                  strcpy ( pAVS, val );
                  pattValTmp->valType = OGR_STR;
                  pattValTmp->value   = pAVS;
            }

            //    Do CM93 $SCODE attribute substitutions
            if ( sclass.IsSameAs ( _T ( "$AREAS" ) ) && ( vtype == 'S' ) && sattr.IsSameAs ( _T ( "$SCODE" ) ) )
            {
                  if ( !strcmp ( ( char * ) pattValTmp->value, "II25" ) )
                  {
                        free ( pattValTmp->value );
                        pattValTmp->value   = ( char * ) malloc ( strlen ( "BACKGROUND" ) + 1 );
                        strcpy ( ( char * ) pattValTmp->value, "BACKGROUND" );
                  }
            }


            //    Capture some attributes on the fly as needed
            if ( sattr.IsSameAs ( _T ( "RECDAT" ) ) || sattr.IsSameAs ( _T ( "_dgdat" ) ) )
            {
                  if ( sclass_sub.IsSameAs ( _T ( "M_COVR" ) ) && ( vtype == 'S' ) )
                  {
                        wxString pub_date ( ( char * ) pattValTmp->value, wxConvUTF8 );

                        wxDateTime upd;
                        upd.ParseFormat ( pub_date, _T ( "%Y%m%d" ) );
                        if ( !upd.IsValid() )
                              upd.ParseFormat ( _T ( "20000101" ), _T ( "%Y%m%d" ) );
                        m_EdDate = upd;

                        pub_date.Truncate ( 4 );

                        long nyear = 0;
                        pub_date.ToLong ( &nyear );
                        npub_year = nyear;

                  }
            }


            //    Capture the potential WGS84 transform offset for later use
            if ( sclass_sub.IsSameAs ( _T ( "M_COVR" ) ) && ( vtype == 'R' ) )
            {
                  if ( sattr.IsSameAs ( _T ( "_wgsox" ) ) )
                  {
                        tmp_transform_x = * ( double * ) pattValTmp->value;
                        if ( fabs ( tmp_transform_x ) > 1.0 )                 // metres
                              m_CIB.b_have_offsets = true;
                  }
                  else if ( sattr.IsSameAs ( _T ( "_wgsoy" ) ) )
                  {
                        tmp_transform_y = * ( double * ) pattValTmp->value;
                        if ( fabs ( tmp_transform_x ) > 1.0 )
                              m_CIB.b_have_offsets = true;
                  }
            }


            if ( sattr.Len() ) {
                wxASSERT( sattr.Len() == 6);
                wxCharBuffer dbuffer=sattr.ToUTF8();
                if(dbuffer.data()) {                
                    pobj->att_array = (char *)realloc(pobj->att_array, 6*(pobj->n_attr + 1));
                
                    strncpy(pobj->att_array + (6 * sizeof(char) * pobj->n_attr), dbuffer.data(), 6);
                    pobj->n_attr++;
                
                    pobj->attVal->Add ( pattValTmp );
                }
                else
                    delete pattValTmp;
            }
            else
                  delete pattValTmp;


      }     //for


      //    ATON label optimization:
      //    Some CM93 ATON objects do not contain OBJNAM attribute, which means that no label is shown
      //    for these objects when ATON labals are requested
      //    Look for these cases, and change the INFORM attribute label to OBJNAM, if present.


      if ( 1 == geomtype )
      {
            if ( ( !strncmp ( pobj->FeatureName,   "LIT",    3 ) ) ||
                    ( !strncmp ( pobj->FeatureName, "LIGHTS", 6 ) ) ||
                    ( !strncmp ( pobj->FeatureName, "BCN",    3 ) ) ||
                    ( !strncmp ( pobj->FeatureName, "_slgto", 6 ) ) ||
                    ( !strncmp ( pobj->FeatureName, "_boygn", 6 ) ) ||
                    ( !strncmp ( pobj->FeatureName, "_bcngn", 6 ) ) ||
                    ( !strncmp ( pobj->FeatureName, "_extgn", 6 ) ) ||
                    ( !strncmp ( pobj->FeatureName, "TOWERS", 6 ) ) ||
                    ( !strncmp ( pobj->FeatureName, "BOY",    3 ) ) )
            {
                
                  bool bfound_OBJNAM =  ( pobj->GetAttributeIndex("OBJNAM") != -1 );
                  bool bfound_INFORM =  ( pobj->GetAttributeIndex("INFORM") != -1 );

                  if ( ( !bfound_OBJNAM ) && ( bfound_INFORM ) )        // can make substitution
                  {
                      char *patl = pobj->att_array;
                      for(int i=0 ; i < pobj->n_attr ; i++) {           // find "INFORM"
                          if(!strncmp(patl, "INFORM", 6)){
                              memcpy ( patl, "OBJNAM", 6 );            // change to "OBJNAM"
                              break;
                          }
                          
                          patl += 6;
                      }
                      
                  }
            }
      }



      switch ( geomtype )
      {
            case 4:
            {
                  pobj->Primitive_type = GEO_AREA;

                  //    Check for and maintain the class array of M_COVR objects
                  if ( sclass_sub.IsSameAs ( _T ( "M_COVR" ) ) )
                  {
                        M_COVR_Desc *pmcd;

                        M_COVR_Desc *pmcd_look = GetCoverSet()->Find_MCD ( cell_index, iobject, subcell );
                        if ( NULL == pmcd_look )     // not found
                        {
                              double lat, lon;

                              pmcd = new M_COVR_Desc;

                              //    Record unique identifiers for this M_COVR object
                              pmcd->m_cell_index = cell_index;
                              pmcd->m_object_id = iobject;
                              pmcd->m_subcell = subcell;

                              //    User offsets start empty
                              pmcd->user_xoff = 0;
                              pmcd->user_yoff = 0;
                              pmcd->m_buser_offsets = false;

                              //    Record the Publication Year of this cell
                              pmcd->m_npub_year = npub_year;

                              //      Get number of exterior ring points(vertices)
                              int npta  = xgeom->contour_array[0];
                              float_2Dpt *geoPt = new float_2Dpt[npta + 2];     // vertex array
                              float_2Dpt *ppt = geoPt;

                              pmcd->m_covr_lon_max = -1000.;
                              pmcd->m_covr_lon_min = 1000.;
                              pmcd->m_covr_lat_max = -1000.;
                              pmcd->m_covr_lat_min = 1000.;

                              //  Transcribe exterior ring points to vertex array, in Lat/Lon coordinates
                              for ( int ip = 0 ; ip < npta ; ip++ )
                              {
                                    cm93_point p;
                                    p.x = ( int ) xgeom->vertex_array[ip + 1].m_x;
                                    p.y = ( int ) xgeom->vertex_array[ip + 1].m_y;

                                    Transform ( &p, 0, 0,/*tmp_transform_x, tmp_transform_y,*/ &lat, &lon );
                                    ppt->x = lon;
                                    ppt->y = lat;

                                    pmcd->m_covr_lon_max = wxMax ( pmcd->m_covr_lon_max, lon );
                                    pmcd->m_covr_lon_min = wxMin ( pmcd->m_covr_lon_min, lon );
                                    pmcd->m_covr_lat_max = wxMax ( pmcd->m_covr_lat_max, lat );
                                    pmcd->m_covr_lat_min = wxMin ( pmcd->m_covr_lat_min, lat );

                                    ppt++;

                              }
                              pmcd->m_nvertices = npta;
                              pmcd->pvertices = geoPt;

                              pmcd->m_covr_bbox.Set( pmcd->m_covr_lat_min, pmcd->m_covr_lon_min,
                                                     pmcd->m_covr_lat_max, pmcd->m_covr_lon_max );

                              //    Capture and store the potential WGS transform offsets grabbed during attribute decode
                              pmcd->transform_WGS84_offset_x = tmp_transform_x;
                              pmcd->transform_WGS84_offset_y = tmp_transform_y;

                              pmcd->m_centerlat_cos = cos( ((pmcd->m_covr_lat_min + pmcd->m_covr_lat_max)/2.) * PI/180. );
                              
                              //    Add this MCD to the persistent class covr_set
                              GetCoverSet()->Add_Update_MCD ( pmcd );

                        }
                        else
                        {
                              // If already in the coverset, are there user offsets applied to this MCD?
                              if ( pmcd_look->m_buser_offsets )
                              {
                                    m_CIB.b_have_user_offsets = true;

                                    m_CIB.user_xoff = pmcd_look->user_xoff;
                                    m_CIB.user_yoff = pmcd_look->user_yoff;
                              }

                              pmcd = pmcd_look;

                        }

                        //     Add this geometry to the currently loaded class M_COVR array
                        m_pcovr_array_loaded.Add ( pmcd );

                        // Update the covr region
                        unsigned int n = pmcd->m_nvertices;
                        double *pts = new double[2*n];
                        
                        // copy into array of doubles
                        for(size_t i=0; i<2*n; i++)
                            pts[i] = (( float * ) pmcd->pvertices)[i];

                        // normalize to 0-360 coords for areas that cross 180 (will be adjusted in LLRegion)
                        if(LLRegion::PointsCCW(n, pts))
                            for(size_t i=0; i<n; i++)
                                if(pts[2*i+1] < 0)
                                    pts[2*i+1] += 360;

                        // perform region union logic
                        LLRegion rgn_covr(n, pts);
                        m_region.Union( rgn_covr );
                        delete [] pts;

                        //    Add the MCD it to the current (temporary) per cell list
                        //    This array is used only to quickly find the M_COVR object parameters which apply to other objects
                        //    loaded from this cell.
                        //    We do this so we don't have to search the entire (worldwide) coverset for this chart scale
                        m_CIB.m_cell_mcovr_list.Append ( pmcd );

                  }



                  //  Declare x/y of the object to be average of all cm93points
                  pobj->x = ( xgeom->xmin + xgeom->xmax ) / 2.;
                  pobj->y = ( xgeom->ymin + xgeom->ymax ) / 2.;

                  //    associate the vector(edge) index table
                  pobj->m_n_lsindex = xgeom->n_vector_indices;
                  pobj->m_lsindex_array = xgeom->pvector_index;         // object now owns the array
                  pobj->m_n_edge_max_points = 0; //xgeom->n_max_edge_points;


                  //    Find the proper WGS offset for this object
                  if ( m_CIB.b_have_offsets || m_CIB.b_have_user_offsets )
                  {
                        double latc, lonc;
                        cm93_point pc;   pc.x = ( short unsigned int ) pobj->x;  pc.y = ( short unsigned int ) pobj->y;
                        Transform ( &pc, 0., 0., &latc, &lonc );

                        M_COVR_Desc *pmcd = FindM_COVR_InWorkingSet ( latc, lonc );
                        if ( pmcd )
                        {
                              trans_WGS84_offset_x = pmcd->user_xoff;
                              trans_WGS84_offset_y = pmcd->user_yoff;
                        }
                  }


                  //  Set the s57obj bounding box as lat/lon
                  double lat1, lon1, lat2, lon2;
                  cm93_point p;

                  p.x = ( int ) xgeom->xmin;
                  p.y = ( int ) xgeom->ymin;
                  Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat1, &lon1 );
                  p.x = ( int ) xgeom->xmax;
                  p.y = ( int ) xgeom->ymax;
                  Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat2, &lon2 );
                  pobj->BBObj.Set( lat1, lon1, lat2, lon2 );

                  //  Set the object base point
                  p.x = ( int ) pobj->x;
                  p.y = ( int ) pobj->y;
                  Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat1, &lon1 );
                  pobj->m_lon = lon1;
                  pobj->m_lat = lat1;


                  if(1)
                  {
                //    This will be a deferred tesselation.....

                        // Set up the conversion factors for use in the tesselator
                        xgeom->x_rate   = m_CIB.transform_x_rate;
                        xgeom->x_offset = m_CIB.transform_x_origin - trans_WGS84_offset_x;
                        xgeom->y_rate   = m_CIB.transform_y_rate;
                        xgeom->y_offset = m_CIB.transform_y_origin - trans_WGS84_offset_y;

                        //    Set up a deferred tesselation
                        //      If OpnGL is not available, use the trapezoid tesselator 
                        //        instead of the triangle tesselator
                        
                        //      Two reasons for this:
                        //      a.  Tri tesselator is buggy, some tris not rendered correctly
                        //      b.  Tri tesselator is slower than trapezoids for direct rendering
#ifdef ocpnUSE_GL
                        if(g_bopengl)
                            pobj->pPolyTessGeo = new PolyTessGeo ( xgeom );
                        else
                            pobj->pPolyTrapGeo = new PolyTessGeoTrap ( xgeom );
                        
#else                        
                            pobj->pPolyTrapGeo = new PolyTessGeoTrap ( xgeom );
#endif                        
                  }

                  break;
            }


            case 1:
            {
                  pobj->Primitive_type = GEO_POINT;
                  pobj->npt = 1;


                  pobj->x = xgeom->pointx;
                  pobj->y = xgeom->pointy;

                  double lat, lon;
                  cm93_point p;
                  p.x = xgeom->pointx;
                  p.y = xgeom->pointy;
                  Transform ( &p, 0., 0., &lat, &lon );

                  //    Find the proper WGS offset for this object
                  if ( m_CIB.b_have_offsets || m_CIB.b_have_user_offsets )
                  {
                        M_COVR_Desc *pmcd = FindM_COVR_InWorkingSet ( lat, lon );
                        if ( pmcd )
                        {
                              trans_WGS84_offset_x = pmcd->user_xoff;
                              trans_WGS84_offset_y = pmcd->user_yoff;
                        }
                  }

                  //    Transform again to pick up offsets
                  Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );

                  pobj->m_lat = lat;
                  pobj->m_lon = lon;

                  // make initial bounding box large enough for worst possible case
                  // it's not possible to know unless we knew the font, but this works
                  // except for huge font sizes
                  // this is not very good or accurate or efficient and hopefully we can
                  // replace the current bounding box logic with calculating logic
                  double llsize = 1e-3 / view_scale_ppm;

                  pobj->BBObj.Set ( lat, lon, lat, lon );
                  pobj->BBObj.EnLarge ( llsize );

                  break;
            }



            case 8:               //wkbMultiPoint25D:
            {


                  pobj->Primitive_type = GEO_POINT;

                  //  Set the s57obj bounding box as lat/lon
                  double lat1, lon1, lat2, lon2;
                  cm93_point p;

                  p.x = ( int ) xgeom->xmin;
                  p.y = ( int ) xgeom->ymin;
                  Transform ( &p, 0., 0., &lat1, &lon1 );

                  p.x = ( int ) xgeom->xmax;
                  p.y = ( int ) xgeom->ymax;
                  Transform ( &p, 0., 0., &lat2, &lon2 );
                  pobj->BBObj.Set ( lat1, lon1, lat2, lon2 );
                  
                  //  and declare x/y of the object to be average of all cm93points
                  pobj->x = ( xgeom->xmin + xgeom->xmax ) / 2.;
                  pobj->y = ( xgeom->ymin + xgeom->ymax ) / 2.;



                  OGRMultiPoint *pGeo = ( OGRMultiPoint * ) xgeom->pogrGeom;
                  pobj->npt = pGeo->getNumGeometries();

                  pobj->geoPtz = ( double * ) malloc ( pobj->npt * 3 * sizeof ( double ) );
                  pobj->geoPtMulti = ( double * ) malloc ( pobj->npt * 2 * sizeof ( double ) );

                  double *pdd = pobj->geoPtz;
                  double *pdl = pobj->geoPtMulti;

                  for ( int ip=0 ; ip<pobj->npt ; ip++ )
                  {
                        OGRPoint *ppt = ( OGRPoint * ) ( pGeo->getGeometryRef ( ip ) );

                        cm93_point p;
                        p.x = ( int ) ppt->getX();
                        p.y = ( int ) ppt->getY();
                        double depth = ppt->getZ();

                        double east  = p.x;
                        double north = p.y;

                        double snd_trans_x = 0.;
                        double snd_trans_y = 0.;

                        //    Find the proper offset for this individual sounding
                        if ( m_CIB.b_have_user_offsets )
                        {
                              double lats, lons;
                              Transform ( &p, 0., 0., &lats, &lons );

                              M_COVR_Desc *pmcd = FindM_COVR_InWorkingSet ( lats, lons );
                              if ( pmcd )
                              {
                                    // For lat/lon calculation below
                                    snd_trans_x = pmcd->user_xoff;
                                    snd_trans_y = pmcd->user_yoff;

                                    // Actual cm93 point of this sounding, back-converted from metres e/n
                                    east  -= pmcd->user_xoff / m_CIB.transform_x_rate;
                                    north -= pmcd->user_yoff / m_CIB.transform_y_rate;
                              }
                        }

                        *pdd++ = east;
                        *pdd++ = north;
                        *pdd++ = depth;

                        //  Save offset lat/lon of point in obj->geoPtMulti for later use in decomposed bboxes
                        Transform ( &p, snd_trans_x, snd_trans_y, &lat1, &lon1 );
                        *pdl++ = lon1;
                        *pdl++ = lat1;
                  }

                  //  Set the object base point
                  p.x = ( int ) pobj->x;
                  p.y = ( int ) pobj->y;
                  Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat1, &lon1 );
                  pobj->m_lon = lon1;
                  pobj->m_lat = lat1;


                  delete pGeo;

                  break;
            }         // case 8





            case 2:
            {
                  pobj->Primitive_type = GEO_LINE;

                  pobj->npt = xgeom->n_max_vertex;
                  pobj->geoPt = ( pt * ) xgeom->vertex_array;
                  xgeom->vertex_array = NULL;               // object now owns the array

                  //  Declare x/y of the object to be average of all cm93points
                  pobj->x = ( xgeom->xmin + xgeom->xmax ) / 2.;
                  pobj->y = ( xgeom->ymin + xgeom->ymax ) / 2.;

                  //    associate the vector(edge) index table
                  pobj->m_n_lsindex = xgeom->n_vector_indices;
                  pobj->m_lsindex_array = xgeom->pvector_index;         // object now owns the array
                  pobj->m_n_edge_max_points = 0; //xgeom->n_max_edge_points;


                  //    Find the proper WGS offset for this object
                  if ( m_CIB.b_have_offsets || m_CIB.b_have_user_offsets )
                  {
                        double latc, lonc;
                        cm93_point pc;   pc.x = ( short unsigned int ) pobj->x;  pc.y = ( short unsigned int ) pobj->y;
                        Transform ( &pc, 0., 0., &latc, &lonc );

                        M_COVR_Desc *pmcd = FindM_COVR_InWorkingSet ( latc, lonc );
                        if ( pmcd )
                        {
                              trans_WGS84_offset_x = pmcd->user_xoff;
                              trans_WGS84_offset_y = pmcd->user_yoff;
                        }
                  }



                  //  Set the s57obj bounding box as lat/lon
                  double lat1, lon1, lat2, lon2;
                  cm93_point p;

                  p.x = ( int ) xgeom->xmin;
                  p.y = ( int ) xgeom->ymin;
                  Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat1, &lon1 );

                  p.x = ( int ) xgeom->xmax;
                  p.y = ( int ) xgeom->ymax;
                  Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat2, &lon2 );
                  pobj->BBObj.Set ( lat1, lon1, lat2, lon2 );

                  //  Set the object base point
                  p.x = ( int ) pobj->x;
                  p.y = ( int ) pobj->y;
                  Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat1, &lon1 );
                  pobj->m_lon = lon1;
                  pobj->m_lat = lat1;

                  break;

            }                // case 2
            default:
            {
                  //TODO GEO_PRIM here is a placeholder.  Trace this code....
                  pobj->Primitive_type = GEO_PRIM;
                  break;
            }

      }         // geomtype switch


      
      //  Is this a catagory-movable object?
      if( !strncmp(pobj->FeatureName, "OBSTRN", 6) ||
          !strncmp(pobj->FeatureName, "WRECKS", 6) ||
          !strncmp(pobj->FeatureName, "DEPCNT", 6) ||
          !strncmp(pobj->FeatureName, "UWTROC", 6) )
      {
          pobj->m_bcategory_mutable = true;
      }
      else{
          pobj->m_bcategory_mutable = false;
      }
      
      //      Build/Maintain a list of found OBJL types for later use
      //      And back-reference the appropriate list index in S57Obj for Display Filtering

      pobj->iOBJL = -1; // deferred, done by OBJL filtering in the PLIB as needed

      // Everything in Xgeom that is needed later has been given to the object
      // So, the xgeom object can be deleted
      // Except for area features, which will get deferred tesselation, and so need the Extended geometry point
      // Those features will own the xgeom...
      if ( geomtype != 4 )
            delete xgeom;

      //    Set the per-object transform coefficients
      pobj->x_rate   = m_CIB.transform_x_rate * ( mercator_k0 * WGS84_semimajor_axis_meters / CM93_semimajor_axis_meters );
      pobj->y_rate   = m_CIB.transform_y_rate * ( mercator_k0 * WGS84_semimajor_axis_meters / CM93_semimajor_axis_meters );
      pobj->x_origin = m_CIB.transform_x_origin * ( mercator_k0 * WGS84_semimajor_axis_meters / CM93_semimajor_axis_meters );
      pobj->y_origin = m_CIB.transform_y_origin * ( mercator_k0 * WGS84_semimajor_axis_meters / CM93_semimajor_axis_meters );

      //    Add in the possible offsets to WGS84 which come from the proper M_COVR containing this feature
      pobj->x_origin -= trans_WGS84_offset_x;
      pobj->y_origin -= trans_WGS84_offset_y;

      return pobj;
}


//    Find the proper M_COVR record within this current cell for this lat/lon
M_COVR_Desc *cm93chart::FindM_COVR_InWorkingSet ( double lat, double lon )
{
      M_COVR_Desc *ret = NULL;
      //    Default is to use the first M_COVR, the usual case
      if ( m_CIB.m_cell_mcovr_list.GetCount() == 1 )
      {
            wxList_Of_M_COVR_DescNode *node0 = m_CIB.m_cell_mcovr_list.GetFirst();
            if ( node0 )
                  ret = node0->GetData();
      }
      else
      {
            wxList_Of_M_COVR_DescNode *node = m_CIB.m_cell_mcovr_list.GetFirst();
            while ( node )
            {
                  M_COVR_Desc *pmcd = node->GetData();

                  if ( G_PtInPolygon_FL ( pmcd->pvertices, pmcd->m_nvertices, lon, lat ) )
                  {
                        ret = pmcd;
                        break;
                  }

                  node = node->GetNext();
            }
      }
      return ret;
}



//    Find the proper M_COVR record within this current cell for this lat/lon
//    And return the WGS84 offsets contained within
wxPoint2DDouble cm93chart::FindM_COVROffset ( double lat, double lon )
{
      wxPoint2DDouble ret ( 0., 0. );


      //    Default is to use the first M_COVR, the usual case
      wxList_Of_M_COVR_DescNode *node0 = m_CIB.m_cell_mcovr_list.GetFirst();
      if ( node0 )
      {
            M_COVR_Desc *pmcd0 = node0->GetData();
            ret.m_x = pmcd0->transform_WGS84_offset_x;
            ret.m_y = pmcd0->transform_WGS84_offset_y;
      }

      //    If there are more than one M_COVR in this cell, need to search
      if ( m_CIB.m_cell_mcovr_list.GetCount() > 1 )
      {
            wxList_Of_M_COVR_DescNode *node = m_CIB.m_cell_mcovr_list.GetFirst();
            while ( node )
            {
                  M_COVR_Desc *pmcd = node->GetData();

                  if ( G_PtInPolygon_FL ( pmcd->pvertices, pmcd->m_nvertices, lon, lat ) )
                  {
                        ret.m_x = pmcd->transform_WGS84_offset_x;
                        ret.m_y = pmcd->transform_WGS84_offset_y;
                        break;
                  }

                  node = node->GetNext();
            }
      }
      return ret;
}




//    Read the cm93 cell file header and create required Chartbase data structures
InitReturn cm93chart::CreateHeaderDataFromCM93Cell ( void )
{

      //    Figure out the scale from the file name
      wxFileName fn ( m_FullPath );
      wxString ext = fn.GetExt();

      int scale;
      switch ( ( ext.mb_str() ) [ ( size_t ) 0] )
      {
            case 'Z': scale = 20000000;  break;
            case 'A': scale =  3000000;  break;
            case 'B': scale =  1000000;  break;
            case 'C': scale =   200000;  break;
            case 'D': scale =   100000;  break;
            case 'E': scale =    50000;  break;
            case 'F': scale =    20000;  break;
            case 'G': scale =     7500;  break;
            default:  scale = 20000000;  break;
      }

      m_Chart_Scale = scale;



      //    Check with the manager to see if a chart of this scale has been processed
      //    If there is no manager, punt and open the chart
      if(m_pManager)
      {
            bool bproc = false;
            switch ( m_Chart_Scale )
            {
                  case 20000000: bproc = m_pManager->m_bfoundZ; break;
                  case  3000000: bproc = m_pManager->m_bfoundA; break;
                  case  1000000: bproc = m_pManager->m_bfoundB; break;
                  case   200000: bproc = m_pManager->m_bfoundC; break;
                  case   100000: bproc = m_pManager->m_bfoundD; break;
                  case    50000: bproc = m_pManager->m_bfoundE; break;
                  case    20000: bproc = m_pManager->m_bfoundF; break;
                  case     7500: bproc = m_pManager->m_bfoundG; break;
            }


            if ( bproc )
                  return INIT_FAIL_NOERROR;




            //    Inform the manager that a chart of this scale has been processed
            switch ( m_Chart_Scale )
            {
                  case 20000000: m_pManager->m_bfoundZ = true; break;
                  case  3000000: m_pManager->m_bfoundA = true; break;
                  case  1000000: m_pManager->m_bfoundB = true; break;
                  case   200000: m_pManager->m_bfoundC = true; break;
                  case   100000: m_pManager->m_bfoundD = true; break;
                  case    50000: m_pManager->m_bfoundE = true; break;
                  case    20000: m_pManager->m_bfoundF = true; break;
                  case     7500: m_pManager->m_bfoundG = true; break;
            }
      }

      //    Specify the whole world as chart coverage
      m_FullExtent.ELON = 179.0;
      m_FullExtent.WLON = -179.0;
      m_FullExtent.NLAT = 80.0;
      m_FullExtent.SLAT = -80.0;
      m_bExtentSet = true;


      //    Populate one (huge) M_COVR Entry
      m_nCOVREntries = 1;
      m_pCOVRTablePoints = ( int * ) malloc ( sizeof ( int ) );
      *m_pCOVRTablePoints = 4;
      m_pCOVRTable = ( float ** ) malloc ( sizeof ( float * ) );
      float *pf = ( float * ) malloc ( 2 * 4 * sizeof ( float ) );
      *m_pCOVRTable = pf;
      float *pfe = pf;

      *pfe++ = m_FullExtent.NLAT; //LatMax;
      *pfe++ = m_FullExtent.WLON; //LonMin;

      *pfe++ = m_FullExtent.NLAT; //LatMax;
      *pfe++ = m_FullExtent.ELON; //LonMax;

      *pfe++ = m_FullExtent.SLAT; //LatMin;
      *pfe++ = m_FullExtent.ELON; //LonMax;

      *pfe++ = m_FullExtent.SLAT; //LatMin;
      *pfe++ = m_FullExtent.WLON; //LonMin;



      return INIT_OK;
}

void cm93chart::ProcessMCOVRObjects ( int cell_index, char subcell )
{
      //Extract the m_covr structures inline

      Object *pobject = m_CIB.pobject_block;           // head of object array

      int iObj = 0;
      while ( iObj < m_CIB.m_nfeature_records )
      {
            if ( ( pobject != NULL ) )
            {
                  //    Look for and process m_covr object(s)
                  int iclass = pobject->otype;

                  wxString sclass = m_pDict->GetClassName ( iclass );

                  if ( sclass.IsSameAs ( _T ( "_m_sor" ) ) )
                  {
                        M_COVR_Desc *pmcd = m_pcovr_set->Find_MCD ( cell_index, iObj, ( int ) subcell );
                        if ( NULL == pmcd )
                        {
                              Extended_Geometry *xgeom = BuildGeom ( pobject, NULL, iObj );

                              //    Decode the attributes, specifically looking for _wgsox, _wgsoy

                              double tmp_transform_x = 0.;
                              double tmp_transform_y = 0.;

                              cm93_attr_block pab( pobject->attributes_block, m_pDict );
                              for ( int jattr = 0 ; jattr  < pobject->n_attributes ; jattr++ )
                              {
                                    unsigned char *curr_attr = pab.GetNextAttr();
                                    unsigned char iattr = *curr_attr;
                                    wxString sattr = m_pDict->GetAttrName ( iattr );
                                    char vtype = m_pDict->GetAttrType ( iattr );
                                    unsigned char *aval = curr_attr + 1;

                                    if ( vtype == 'R' )
                                    {
                                          float *pf = ( float * ) aval;
#ifdef ARMHF
                                          float tf1;
                                          memcpy(&tf1, pf, sizeof(float));
                                          if ( sattr.IsSameAs ( _T ( "_wgsox" ) ) )
                                              tmp_transform_x = tf1;
                                          else if ( sattr.IsSameAs ( _T ( "_wgsoy" ) ) )
                                              tmp_transform_y = tf1;
#else
                                          if ( sattr.IsSameAs ( _T ( "_wgsox" ) ) )
                                                tmp_transform_x = *pf;
                                          else if ( sattr.IsSameAs ( _T ( "_wgsoy" ) ) )
                                                tmp_transform_y = *pf;
#endif
                                    }


                              }     //for all attributes


                              if ( NULL != xgeom )
                              {
                                    double lat, lon;

                                    pmcd = new M_COVR_Desc;

                                    //    Record unique identifiers for this M_COVR object
                                    pmcd->m_cell_index = cell_index;
                                    pmcd->m_object_id = iObj;
                                    pmcd->m_subcell = ( int ) subcell;

                                    //      Get number of exterior ring points(vertices)
                                    int npta  = xgeom->contour_array[0];
                                    float_2Dpt *geoPt = new float_2Dpt[npta + 2];     // vertex array
                                    float_2Dpt *ppt = geoPt;

                                    //  Transcribe exterior ring points to vertex array, in Lat/Lon coordinates
                                    pmcd->m_covr_lon_max = -1000.;
                                    pmcd->m_covr_lon_min = 1000.;
                                    pmcd->m_covr_lat_max = -1000.;
                                    pmcd->m_covr_lat_min = 1000.;


                                    for ( int ip = 0 ; ip < npta ; ip++ )
                                    {
                                          cm93_point p;
                                          p.x = ( int ) xgeom->vertex_array[ip + 1].m_x;
                                          p.y = ( int ) xgeom->vertex_array[ip + 1].m_y;

                                          Transform ( &p, 0., 0., &lat, &lon );
                                          ppt->x = lon;
                                          ppt->y = lat;

                                          pmcd->m_covr_lon_max = wxMax ( pmcd->m_covr_lon_max, lon );
                                          pmcd->m_covr_lon_min = wxMin ( pmcd->m_covr_lon_min, lon );
                                          pmcd->m_covr_lat_max = wxMax ( pmcd->m_covr_lat_max, lat );
                                          pmcd->m_covr_lat_min = wxMin ( pmcd->m_covr_lat_min, lat );

                                          ppt++;
                                    }
                                    pmcd->m_nvertices = npta;
                                    pmcd->pvertices = geoPt;

                                    pmcd->m_covr_bbox.Set( pmcd->m_covr_lat_min, pmcd->m_covr_lon_min,
                                                           pmcd->m_covr_lat_max, pmcd->m_covr_lon_max );


                                    //    Capture and store the potential WGS transform offsets grabbed during attribute decode
                                    pmcd->transform_WGS84_offset_x = tmp_transform_x;
                                    pmcd->transform_WGS84_offset_y = tmp_transform_y;
                                    
                                    pmcd->m_centerlat_cos = cos( ((pmcd->m_covr_lat_min + pmcd->m_covr_lat_max)/2.) * PI/180. );

                                    //     Add this object to the covr_set
                                    m_pcovr_set->Add_Update_MCD ( pmcd );

                                    
                                    //  Update the parent cell mcovr bounding box
                                    m_covr_bbox.Expand(pmcd->m_covr_bbox);
                                    
                                    //    Clean up the xgeom
                                    free ( xgeom->pvector_index );

                                    delete xgeom;
                              }
                        }
                  }
            }

            else                    // objectdef == NULL
                  break;

            pobject++;
            iObj++;
      }
}

bool cm93chart::UpdateCovrSet ( ViewPort *vpt )
{
      //    Create an array of CellIndexes covering the current viewport
      ArrayOfInts vpcells = GetVPCellArray ( *vpt );

      //    Check the member covr_set to see if all these viewport cells have had their m_covr loaded

      for ( unsigned int i=0 ; i < vpcells.GetCount() ; i++ )
      {
            //    If the cell is not already in the master coverset, go load enough of it to get the offsets and outlines.....
            if ( !m_pcovr_set->IsCovrLoaded ( vpcells.Item ( i ) ) )
            {
                  if ( loadcell_in_sequence ( vpcells.Item ( i ), '0' ) )
                  {
                        ProcessMCOVRObjects ( vpcells.Item ( i ), '0' );
                        Unload_CM93_Cell();           // all done with this (sub)cell
                  }

                  char loadcell_key = 'A';               // starting subcells

                  //    Load the subcells in sequence
                  //    On successful load, add it to the covr set and process the cell
                  while ( loadcell_in_sequence ( vpcells.Item ( i ), loadcell_key ) )
                  {
                        //Extract the m_covr structures inline

                        ProcessMCOVRObjects ( vpcells.Item ( i ), loadcell_key );

                        Unload_CM93_Cell();           // all done with this (sub)cell

                        loadcell_key++;

                  }     // while
            }           // cell is not in
      }                 // for cellindex array

      return true;
}



bool cm93chart::IsPointInLoadedM_COVR ( double xc, double yc )
{
    //  Provisionally revert to older method pending investigation.
#if 1
    return m_region.Contains(yc, xc);
#else
      for ( unsigned int im=0 ; im < m_pcovr_array_loaded.GetCount() ; im++ )
      {
            if ( G_PtInPolygon_FL ( m_pcovr_array_loaded.Item ( im )->pvertices, m_pcovr_array_loaded.Item ( im )->m_nvertices, xc, yc ) )
                  return true;
      }
      return false;
#endif
}

LLRegion cm93chart::GetValidRegion()
{
    return m_region;
}



int cm93chart::loadcell_in_sequence ( int cellindex, char subcell )
{
      int rv = loadsubcell ( cellindex, subcell );

      return rv;
}



int cm93chart::loadsubcell ( int cellindex, wxChar sub_char )
{

      //    Create the file name

      int ilat = cellindex / 10000;
      int ilon = cellindex % 10000;


      if ( g_bDebugCM93 )
      {
            double dlat = m_dval / 3.;
            double dlon = m_dval / 3.;
            double lat, lon;
            Get_CM93_Cell_Origin ( cellindex, GetNativeScale(), &lat, &lon );
            printf ( "\n   Attempting loadcell %d scale %lc, sub_char %lc at lat: %g/%g lon:%g/%g\n", cellindex, wxChar ( m_scalechar[0] ), sub_char, lat, lat + dlat, lon, lon+dlon );
      }

      int jlat = ( int ) ( ( ( ilat - 30 ) / m_dval ) * m_dval ) + 30;     // normalize
      int jlon = ( int ) ( ( ilon / m_dval ) * m_dval );

      int ilatroot = ( ( ( ilat - 30 ) / 60 ) * 60 ) + 30;
      int ilonroot = ( ilon / 60 ) * 60;

      wxString file;
      file.Printf ( _T ( "%04d%04d." ), jlat, jlon );
      file += m_scalechar;



      wxString fileroot;
      fileroot.Printf ( _T ( "%04d%04d/" ), ilatroot, ilonroot );
      fileroot += m_scalechar;
      fileroot += _T ( "/" );
      fileroot.Prepend ( m_prefix );

      file[0] = sub_char;
      file.Prepend ( fileroot );

      if ( g_bDebugCM93 )
      {
            char sfile[200];
            strncpy ( sfile, file.mb_str(), 199 );
            sfile[199] = 0;
            printf ( "    filename: %s\n", sfile );
      }

      if ( !::wxFileExists ( file ) )
      {
            //    Try with alternate case of m_scalechar
            wxString new_scalechar = m_scalechar.Lower();

            wxString file1;
            file1.Printf ( _T ( "%04d%04d." ), jlat, jlon );
            file1 += new_scalechar;

            file1[0] = sub_char;

            fileroot.Printf ( _T ( "%04d%04d/" ), ilatroot, ilonroot );
            fileroot += new_scalechar;
            fileroot += _T ( "/" );
            fileroot.Prepend ( m_prefix );

            file1.Prepend ( fileroot );

            if ( g_bDebugCM93 )
            {
                  char sfile[200];
                  strncpy ( sfile, file1.mb_str(), 199 );
                  sfile[199] = 0;
                  printf ( "    alternate filename: %s\n", sfile );
            }

            if ( !::wxFileExists ( file1 ) )
            {

                  //    This is not really an error if the sub_char is not '0'.  It just means there are no more subcells....
                  if ( g_bDebugCM93 )
                  {
                        if ( sub_char == '0' )
                              printf ( "   Tried to load non-existent CM93 cell\n" );
                        else
                              printf ( "   No sub_cells of scale(%lc) found\n", sub_char );
                  }

                  return 0;
            }
            else
                  file = file1;                       // found the file as lowercase, substitute the name
      }

      //    File is known to exist

      OCPNPlatform::ShowBusySpinner();
      
      wxString msg ( _T ( "Loading CM93 cell " ) );
      msg += file;
      wxLogMessage ( msg );

      //    Set the member variable to be the actual file name for use in single chart mode info display
      m_LastFileName = file;

      if ( g_bDebugCM93 )
      {
            char str[256];
            strncpy ( str, msg.mb_str(), 255 );
            str[255] = 0;
            printf ( "   %s\n", str );
      }

      //    Ingest it
      if ( !Ingest_CM93_Cell ( ( const char * ) file.mb_str(), &m_CIB ) )
      {
            wxString msg ( _T ( "   cm93chart  Error ingesting " ) );
            msg.Append ( file );
            wxLogMessage ( msg );
            return 0;
      }


      return 1;
}

void cm93chart::SetUserOffsets ( int cell_index, int object_id, int subcell, int xoff, int yoff )
{
      M_COVR_Desc *pmcd = GetCoverSet()->Find_MCD ( cell_index, object_id, subcell );
      if ( pmcd )
      {
            pmcd->user_xoff = xoff;
            pmcd->user_yoff = yoff;
            pmcd->m_buser_offsets = true;
      }
}

wxPoint *cm93chart::GetDrawBuffer ( int nSize )
{
//    Reallocate the cm93chart DrawBuffer if it is currently too small
      if ( nSize > m_nDrawBufferSize )
      {
            wxPoint * tmp = m_pDrawBuffer;
            m_pDrawBuffer = ( wxPoint * ) realloc ( m_pDrawBuffer, sizeof ( wxPoint ) * ( nSize + 1 ) );
            if (NULL == m_pDrawBuffer)
            {
                free (tmp);
                tmp = NULL;
            }
            else
                m_nDrawBufferSize = nSize + 1;
      }
      return m_pDrawBuffer;
}


//-----------------------------------------------------------------------------------------------
//  cm93manager Implementation
//-----------------------------------------------------------------------------------------------

cm93manager::cm93manager ( void )
{

      m_pcm93Dict = NULL;


      m_bfoundA = false;
      m_bfoundB = false;
      m_bfoundC = false;
      m_bfoundD = false;
      m_bfoundE = false;
      m_bfoundF = false;
      m_bfoundG = false;
      m_bfoundZ = false;

}

cm93manager::~cm93manager ( void )
{
      delete m_pcm93Dict;
}

bool cm93manager::Loadcm93Dictionary(const wxString & name)
{

      //  Find and load cm93_dictionary
      if ( !m_pcm93Dict )
      {
            m_pcm93Dict = FindAndLoadDict ( name );

            if ( !m_pcm93Dict )
            {
                  wxLogMessage ( _T ( "   Cannot load CM93 Dictionary." ) );
                  return false;
            }


            if ( !m_pcm93Dict->IsOk() )
            {
                  wxLogMessage ( _T ( "   Error in loading CM93 Dictionary." ) );
                  delete m_pcm93Dict;
                  m_pcm93Dict = NULL;
                  return false;;
            }
      }
      else if ( !m_pcm93Dict->IsOk() )
      {
            wxLogMessage ( _T ( "   CM93 Dictionary is not OK." ) );
            return false;
      }

      return true;
}

cm93_dictionary *cm93manager::FindAndLoadDict ( const wxString &file )
{
      cm93_dictionary *retval = NULL;
      cm93_dictionary *pdict = new cm93_dictionary();

      //    Search for the dictionary files all along the path of the passed parameter filename

      wxFileName fn ( file );
      wxString path = fn.GetPath ( ( int ) ( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME ) );
      wxString target;
      unsigned int i = 0;

      while ( i < path.Len() )
      {
            target.Append ( path[i] );
            if ( path[i] == fn.GetPathSeparator() )
            {
                  if ( pdict->LoadDictionary ( target ) )
                  {
                        retval = pdict;
                        break;
                  }
                  if ( pdict->LoadDictionary ( target + _T ( "CM93ATTR" ) ) )
                  {
                        retval = pdict;
                        break;
                  }
            }
            i++;
      }

      char t[100];
      strncpy ( t, target.mb_str(), 99 );

      if ( retval == NULL )
            delete pdict;

      return retval;


}


//----------------------------------------------------------------------------
// cm93 Composite Chart object class Implementation
//----------------------------------------------------------------------------
cm93compchart::cm93compchart()
{
      m_ChartType = CHART_TYPE_CM93COMP;
      m_pDictComposite = NULL;

      //    Supply a default name for status bar field
      m_FullPath = _T ( "CM93" );

      //    Set the "Description", so that it paints nice on the screen
      m_Description = _T ( "CM93Composite" );

      m_SE = _T ( "" );
      m_datum_str = _T ( "WGS84" );
      m_SoundingsDatum = _T ( "Unknown" );


      for ( int i = 0 ; i < 8 ; i++ )
            m_pcm93chart_array[i] = NULL;

      m_pcm93chart_current = NULL;

      m_cmscale = -1;
      m_Chart_Skew = 0.0;

      m_pDummyBM = NULL;

      SetSpecialOutlineCellIndex ( 0, 0, 0 );
      m_last_cell_adjustvp = NULL;

      m_pcm93mgr = new cm93manager();


}

cm93compchart::~cm93compchart()
{
    if( g_pCM93OffsetDialog ){
        g_pCM93OffsetDialog->Hide();
    }
       
      for ( int i = 0 ; i < 8 ; i++ )
            delete m_pcm93chart_array[i];

      delete m_pDictComposite;
      delete m_pDummyBM;
      delete m_pcm93mgr;

}


InitReturn cm93compchart::Init ( const wxString& name, ChartInitFlag flags )
{
      m_FullPath = name;

      wxFileName fn ( name );

      wxString target;
      wxString path;

      wxString sep(wxFileName::GetPathSeparator());

      //    Verify that the passed file name exists
      if ( !fn.FileExists() )
      {
            // It may be a directory
            if( wxDir::Exists(name) )
            {
                  target = name + sep;
                  path = name + sep;
            }
            else {
                  wxString msg ( _T ( "   CM93Composite Chart Init cannot find " ) );
                  msg.Append ( name );
                  wxLogMessage ( msg );
                  return  INIT_FAIL_REMOVE;
            }
      }
      else              // its a file that exists
      {
            //    Get the cm93 cell database prefix
            path = fn.GetPath ( ( int ) ( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME ) );

            //    Remove two subdirectories from the passed file name
            //    This will give a normal CM93 root
            wxFileName file_path ( path );
            file_path.RemoveLastDir();
            file_path.RemoveLastDir();

            target = file_path.GetPath ( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
      }

      m_prefixComposite = target;

      wxString msg ( _T ( "CM93Composite Chart Root is " ) );
      msg.Append ( m_prefixComposite );
      wxLogMessage ( msg );


      if ( flags == THUMB_ONLY )
      {
//            SetColorScheme(cs, false);

            return INIT_OK;
      }


      if ( flags == HEADER_ONLY )
            return CreateHeaderData();


      //    Load the cm93 dictionary if necessary
      if ( !m_pDictComposite )
      {
            if ( !m_pDictComposite )                           // second try from the file
                  m_pDictComposite = FindAndLoadDictFromDir ( path );

            if ( !m_pDictComposite )
            {
                  wxLogMessage ( _T ( "   CM93Composite Chart Init cannot locate CM93 dictionary." ) );
                  return INIT_FAIL_REMOVE;
            }
      }


//    Set the color scheme
      SetColorScheme ( m_global_color_scheme, false );

      bReadyToRender = true;

      return INIT_OK;


}

void cm93compchart::Activate ( void )
{
      if ( g_bShowCM93DetailSlider )
      {
            if ( !pCM93DetailSlider )
            {
                  pCM93DetailSlider = new CM93DSlide ( gFrame, -1 , 0, -CM93_ZOOM_FACTOR_MAX_RANGE, CM93_ZOOM_FACTOR_MAX_RANGE,
                                                       wxPoint ( g_cm93detail_dialog_x, g_cm93detail_dialog_y ), wxDefaultSize,
                                                                   wxSIMPLE_BORDER , _T ( "cm93 Detail" ) );
            }

            //    Here is an ugly piece of code which prevents the slider from taking the keyboard focus
            //    Only seems to work for Windows.....
            pCM93DetailSlider->Disable();
            pCM93DetailSlider->Show();
            pCM93DetailSlider->Enable();
      }
}

void cm93compchart::Deactivate ( void )
{
      if ( pCM93DetailSlider )
      {
            pCM93DetailSlider-> Destroy();
            pCM93DetailSlider = NULL;
      }
}

double scale_breaks[] =
{
      5000.,                  //G
      15000.,                 //F
      40000.,                 //E
      150000.,                //D
      300000.,                //C
      1000000.,               //B
      5000000.,               //A
      20000000.               //Z
};

//-----------------------------------------------------------------------
//              Calculate and Set ViewPoint Constants
//-----------------------------------------------------------------------

int cm93compchart::GetCMScaleFromVP ( const ViewPort &vpt )
{
      double scale_mpp = 3000 / vpt.view_scale_ppm;

      double scale_mpp_adj = scale_mpp;

      double scale_breaks_adj[7];

      for ( int i=0 ; i < 7 ; i++ )
            scale_breaks_adj[i] = scale_breaks[i];



      if ( g_cm93_zoom_factor )
      {
#if 0
            //    Completely intuitive exponential curve adjustment
            double efactor = ( double ) ( g_cm93_zoom_factor ) * ( .176 / 7. );
            for ( int i=0 ; i < 7 ; i++ )
            {
                  double efr = efactor * ( 7 - i );
                  scale_breaks_adj[i] = scale_breaks[i] * pow ( 10., efr );
                  if ( g_bDebugCM93 )
                        printf ( "g_cm93_zoom_factor: %2d  efactor: %6g efr:%6g, scale_breaks[i]:%6g  scale_breaks_adj[i]: %6g\n",
                                 g_cm93_zoom_factor, efactor, efr, scale_breaks[i], scale_breaks_adj[i] );
            }
#else
            // improved adjustment for small scales
            double efr = ( double ) g_cm93_zoom_factor * pow(scale_mpp, -.05);
            scale_mpp_adj *= pow(.6, efr );
#endif
      }

      int cmscale_calc = 7;
      int brk_index = 0;
      while ( cmscale_calc > 0 )
      {
            if ( scale_mpp_adj < scale_breaks_adj[brk_index] )
                  break;
            cmscale_calc--;
            brk_index++;
      }

      //        Check for overzoom at the theoretically calcuolated chart scale
      //        If overzoomed possible, switch to larger scale chart if available
      double zoom_factor = scale_breaks[7 - cmscale_calc] / vpt.chart_scale ;
      if( zoom_factor > 4.0) {
          if( cmscale_calc < 7 )
              cmscale_calc ++;
      }
      
      return cmscale_calc;
}

void cm93compchart::SetVPParms ( const ViewPort &vpt )
{
      m_vpt = vpt;                              // save a copy

      int cmscale = GetCMScaleFromVP ( vpt );         // First order calculation of cmscale
      m_cmscale = PrepareChartScale ( vpt, cmscale );

      //    Continuoesly update the composite chart edition date to the latest cell decoded
      if ( m_pcm93chart_array[cmscale] )
      {
            if ( m_pcm93chart_array[cmscale]->GetEditionDate().IsLaterThan ( m_EdDate ) )
                  m_EdDate = m_pcm93chart_array[cmscale]->GetEditionDate();
      }
}

int cm93compchart::PrepareChartScale ( const ViewPort &vpt, int cmscale, bool bOZ_protect )
{

      if ( g_bDebugCM93 )
            printf ( "\non SetVPParms, cmscale:%d, %c\n", cmscale, ( char ) ( 'A' + cmscale -1 ) );

      wxChar ext;
      bool cellscale_is_useable = false;
      bool b_nochart = false;

      while ( !cellscale_is_useable )
      {
            //    Open the proper scale chart, if not already open
            while ( NULL == m_pcm93chart_array[cmscale] )
            {
                  if ( Is_CM93Cell_Present ( m_prefixComposite, vpt.clat, vpt.clon, cmscale ) )
                  {
                        if ( g_bDebugCM93 )
                              printf ( " chart %c at VP clat/clon is present\n", ( char ) ( 'A' + cmscale -1 ) );

                        m_pcm93chart_array[cmscale] = new cm93chart();


                        ext = ( wxChar ) ( 'A' + cmscale - 1 );
                        if ( cmscale == 0 )
                              ext = 'Z';

                        wxString file_dummy = _T ( "CM93." );
                        file_dummy << ext;

                        m_pcm93chart_array[cmscale]->SetCM93Dict ( m_pDictComposite );
                        m_pcm93chart_array[cmscale]->SetCM93Prefix ( m_prefixComposite );
                        m_pcm93chart_array[cmscale]->SetCM93Manager ( m_pcm93mgr );

                        m_pcm93chart_array[cmscale]->SetColorScheme ( m_global_color_scheme );
                        m_pcm93chart_array[cmscale]->Init ( file_dummy, FULL_INIT );
                  }
                  else if ( cmscale == 0 )
                  {
//                        wxString msg;
//                        msg.Printf ( _T ( "   CM93 finds no chart of any scale present at Lat/Lon  %g %g" ), vpt.clat, vpt.clon );
//                        wxLogMessage ( msg );
                        if ( g_bDebugCM93 )
                              printf ( "   CM93 finds no chart of any scale present at Lat/Lon  %g %g\n", vpt.clat, vpt.clon );

                        b_nochart = true;
                        break;
                  }

                  else
                  {
                     cmscale--;                          // revert to smaller scale if selected is not present
                     if ( g_bDebugCM93 )
                              printf ( " no %c scale chart present, adjusting cmscale to %c\n", ( char ) ( 'A' + cmscale ), ( char ) ( 'A' + cmscale -1 ) );
                  }

            }



            m_pcm93chart_current = m_pcm93chart_array[cmscale];

            if ( b_nochart )
            {
                  if ( g_bDebugCM93 )
                        printf ( " b_nochart return\n" );

                  m_pcm93chart_current = NULL;
                  for ( int i = 0 ; i < 8 ; i++ ) {
                        delete m_pcm93chart_array[i];
                        m_pcm93chart_array[i] = NULL;
                  }

                  return cmscale;
            }

            if ( m_pcm93chart_current )
            {
                  //    Pass the parameters to the proper scale chart
                  //    Which will also load the needed cell(s)
                  m_pcm93chart_current->SetVPParms ( vpt );

                  //    Check to see if the viewpoint center is actually on the selected chart
                  float yc = vpt.clat;
                  float xc = vpt.clon;

                  if ( !m_pcm93chart_current->GetCoverSet()->GetCoverCount() )
                  {
                        if ( g_bDebugCM93 )
                              printf ( " chart %c has no M_COVR\n", ( char ) ( 'A' + cmscale -1 ) );
                  }


                  if ( m_pcm93chart_current->IsPointInLoadedM_COVR ( xc, yc ) )
                  {
                        if ( g_bDebugCM93 )
                              printf ( " chart %c contains clat/clon\n", ( char ) ( 'A' + cmscale -1 ) );

                        cellscale_is_useable = true;
                        break;
                  }

//    This commented block assumed that scale 0 coverage is available worlwide.....
//    Might not be so with partial CM93 sets
                  /*
                                    else if(cmscale == 0)
                                    {
                                          cellscale_is_useable = true;
                                          break;
                                    }
                  */

                  else if ( vpt.b_quilt &&  vpt.b_FullScreenQuilt )
                  {
                        ViewPort vp = vpt;

                        covr_set *pcover = m_pcm93chart_current->GetCoverSet();
                        if ( pcover )
                        {
                              bool boverlap = false;
                              for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
                              {
                                    M_COVR_Desc *mcd = pcover->GetCover ( im );

                                    if ( ! ( vp.GetBBox().IntersectOut ( mcd->m_covr_bbox ) ) )
                                    {
                                          boverlap = true;
                                          break;
                                    }
                              }
                              if ( boverlap )
                                    cellscale_is_useable = true;
                        }
                  }

                  if ( !cellscale_is_useable )
                  {
                        if ( cmscale > 0 )
                              cmscale--;        // revert to larger scale if the current scale cells do not contain VP
                        else
                              b_nochart = true;    // we have retired to scale 0, and still no chart coverage, so stop already...
                        if ( g_bDebugCM93 )
                              printf ( " VP is not in M_COVR, adjusting cmscale to %c\n", ( char ) ( 'A' + cmscale -1 ) );
                  }
            }
      }

      //        Final check the zoom factor
      if( bOZ_protect ){
        double zoom_factor = scale_breaks[7 - cmscale] / vpt.chart_scale ;
        
        if( zoom_factor > 4.0) {
            // See if there is a larger scale chart present that will avoid overzoom
            
            float yc = vpt.clat;
            float xc = vpt.clon;
            
            //    Find out what the smallest available scale is that is not overzoomed
            FillScaleArray ( vpt.clat,vpt.clon );
            int new_scale = cmscale;
            bool b_found = false;
            while ( new_scale <= 7 ){
                if ( m_bScale_Array[new_scale] ){
                    double new_zoom_factor = scale_breaks[7 - new_scale] / vpt.chart_scale ;
                    
                    //  Do not allow excessive "under-zoom", for performance reasons
                    if( new_zoom_factor  < 1.0 ){
                        b_found = true;
                        new_scale = cmscale;
                        break;
                    }
                        
                    if( new_zoom_factor < 4.0) {
                        if ( NULL == m_pcm93chart_array[new_scale] ) {
                            m_pcm93chart_array[new_scale] = new cm93chart();
                            
                            ext = ( wxChar ) ( 'A' + new_scale - 1 );
                            if ( new_scale == 0 )
                                ext = 'Z';
                            
                            wxString file_dummy = _T ( "CM93." );
                            file_dummy << ext;
                            
                            m_pcm93chart_array[new_scale]->SetCM93Dict ( m_pDictComposite );
                            m_pcm93chart_array[new_scale]->SetCM93Prefix ( m_prefixComposite );
                            m_pcm93chart_array[new_scale]->SetCM93Manager ( m_pcm93mgr );
                            
                            m_pcm93chart_array[new_scale]->SetColorScheme ( m_global_color_scheme );
                            m_pcm93chart_array[new_scale]->Init ( file_dummy, FULL_INIT );
                        }
    
                        m_pcm93chart_array[new_scale]->SetVPParms ( vpt );
                        if (m_pcm93chart_array[new_scale]->IsPointInLoadedM_COVR ( xc, yc ) ) {
                            b_found = true;
                            break;
                        }
                    }
                }
                new_scale++;
            }
            if( b_found ) {
                cmscale = new_scale;
                m_pcm93chart_current = m_pcm93chart_array[cmscale];
            }
        }
      }

      return cmscale;
}

//    Populate the member bool array describing which chart scales are available at any location
void cm93compchart::FillScaleArray ( double lat, double lon )
{
      for ( int cmscale = 0 ; cmscale < 8 ; cmscale++ )
            m_bScale_Array[cmscale] = Is_CM93Cell_Present ( m_prefixComposite, lat, lon, cmscale );
}

//    These methods simply pass the called parameters to the currently active cm93chart


wxString cm93compchart::GetPubDate()
{
      wxString data;

      if ( NULL != m_pcm93chart_current )

            data.Printf ( _T ( "%4d" ), m_current_cell_pub_date );
      else
            data = _T ( "????" );
      return data;
}

int cm93compchart::GetNativeScale()
{
      if ( m_pcm93chart_current )
            return m_pcm93chart_current->GetNativeScale();
      else
            return ( int ) 1e8;
}

double cm93compchart::GetNormalScaleMin ( double canvas_scale_factor, bool b_allow_overzoom )
{
      double oz_factor;
      oz_factor = 40.;

      if ( m_pcm93chart_current )
      {
            int cmscale = 0;
            if ( m_pcm93chart_current->m_last_vp.IsValid() ) {
                  FillScaleArray ( m_pcm93chart_current->m_last_vp.clat,m_pcm93chart_current-> m_last_vp.clon );

                  //    Find out what the smallest available scale is
                  cmscale = 7;
                  while ( cmscale > 0 )
                  {
                        if ( m_bScale_Array[cmscale] )
                              break;
                        cmscale--;
                  }
            }


            //    And return a sensible minimum scale, allowing selected overzoom.
            switch ( cmscale )
            {
                  case  0: return 20000000. / oz_factor;            // Z
                  case  1: return 3000000.  / oz_factor;            // A
                  case  2: return 1000000.  / oz_factor;            // B
                  case  3: return 200000.   / oz_factor;            // C
                  case  4: return 100000.   / oz_factor;            // D
                  case  5: return 50000.    / oz_factor;            // E
                  case  6: return 20000.    / oz_factor;            // F
                  case  7: return 500.;                             // G
                  default: return 500.     / oz_factor;
            }
      }
      else
            return 500.;
}

double cm93compchart::GetNormalScaleMax ( double canvas_scale_factor, int canvas_width )
{
      return ( 180. / 360. ) * PI  * 2 * ( WGS84_semimajor_axis_meters / ( canvas_width / canvas_scale_factor ) );
      //return 1.0e8;
}


wxPoint GetPixFromLLVP ( double lat, double lon, const ViewPort& VPoint )
{

//      Inline the Simple Mercator Transform for performance reasons
      double easting, northing;

      double s, y3, s0, y30;
      double z = WGS84_semimajor_axis_meters * mercator_k0;

      double xlon = lon;

      /*  Make sure lon and lon0 are same phase */
      if ( lon * VPoint.clon < 0. )
      {
            if ( lon < 0. )
                  xlon += 360.;
            else
                  xlon -= 360.;
      }

      //    And choose the closest direction
      if ( fabs ( xlon - VPoint.clon ) > 180. )
      {
            if ( xlon > VPoint.clon )
                  xlon -= 360.;
            else
                  xlon += 360.;
      }
      easting = ( xlon - VPoint.clon ) * DEGREE * z;

      s = sin ( lat * DEGREE );
      y3 = ( .5 * log ( ( 1 + s ) / ( 1 - s ) ) ) * z;

      s0 = sin ( VPoint.clat * DEGREE );
      y30 = ( .5 * log ( ( 1 + s0 ) / ( 1 - s0 ) ) ) * z;
      northing = y3 - y30;


      wxPoint r;

      double epix = easting  * VPoint.view_scale_ppm;
      double npix = northing * VPoint.view_scale_ppm;
      r.x = ( int ) round ( ( VPoint.pix_width  / 2 ) + epix );
      r.y = ( int ) round ( ( VPoint.pix_height / 2 ) - npix );

      return r;
}



//extern void catch_signals(int signo);


void cm93compchart::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion)
{
      OCPNRegion screen_region(0, 0, VPoint.pix_width, VPoint.pix_height);
      OCPNRegion ret = GetValidScreenCanvasRegion ( VPoint, g_bopengl ?
                                                    VPoint.rv_rect : screen_region );
      *pValidRegion = ret;
}



OCPNRegion cm93compchart::GetValidScreenCanvasRegion ( const ViewPort& VPoint, const OCPNRegion &ScreenRegion )
{
      OCPNRegion ret_region;

      ViewPort vp = VPoint;

      vp.rotation = 0.;

      if ( m_pcm93chart_current )
      {
            int chart_native_scale = m_pcm93chart_current->GetNativeScale();


            for ( unsigned int im=0 ; im < m_pcm93chart_current->m_pcovr_array_loaded.GetCount() ; im++ )
            {
                  M_COVR_Desc *pmcd = ( m_pcm93chart_current->m_pcovr_array_loaded.Item ( im ) );

                  //    We can make a quick test based on the bbox of the M_COVR and the bbox of the ViewPort

                  if ( vp.GetBBox().IntersectOut ( pmcd->m_covr_bbox ) )
                      continue;

                  wxPoint *DrawBuf = m_pcm93chart_current->GetDrawBuffer ( pmcd->m_nvertices );

                  OCPNRegion rgn_covr = vp.GetVPRegionIntersect ( ScreenRegion, pmcd->m_nvertices, ( float * ) pmcd->pvertices, chart_native_scale, DrawBuf );

                  if(rgn_covr.IsOk())           // not empty
                    ret_region.Union( rgn_covr );

            }

      }
      else
            ret_region.Union(OCPNRegion( 0, 0, 1,1 ));

      return ret_region;

}

LLRegion cm93compchart::GetValidRegion()
{
    if ( m_pcm93chart_current )
        return m_pcm93chart_current->GetValidRegion();

    return LLRegion(); // empty region
}

bool cm93compchart::RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint,
                                         const OCPNRegion &RectRegion, const LLRegion &Region)
{
      SetVPParms ( VPoint );

      if ( g_pCM93OffsetDialog && g_pCM93OffsetDialog->IsShown() )
            g_pCM93OffsetDialog->UpdateMCOVRList ( VPoint );

      return DoRenderRegionViewOnGL ( glc, VPoint, RectRegion, Region );

}

bool cm93compchart::DoRenderRegionViewOnGL (const wxGLContext &glc, const ViewPort& VPoint,
                                            const OCPNRegion &RectRegion, const LLRegion &Region)
{
//      g_bDebugCM93 = true;

//      CALLGRIND_START_INSTRUMENTATION

      ViewPort vp = VPoint;

      bool render_return = false;
      if ( m_pcm93chart_current )
      {
            m_pcm93chart_current->SetVPParms ( vp );

            //    Check the current chart scale to see if it covers the requested region totally
            if ( VPoint.b_quilt )
            {
                  LLRegion vpr_empty = Region;
                  LLRegion chart_region = GetValidRegion();

                  // old method which draws the regions from large to small scale, then finishes with the largest
                  // scale.  This is broken on systems with broken clipping regions
                  
                  // So we modify the algorithm as follows:
                  //  a. Calculate the region patches from large scale to small scale, starting with the Reference scale, and
                  //     ending when the total region requested is full.
                  //  b. Save the calculated patches in an array as they are generated.
                  //  c. Render the regions/scales saved in the array in reverse order, from small scale to large scale.
                  //  d. Finally, render the Reference region/scale.
                  //  
                  //  This logic has the advantage that only the minimum necessary Object rendering is actually performed, and
                  //  only within the minimum necessary region.
                  
                  if ( !chart_region.Empty() )
                        vpr_empty.Subtract ( chart_region );

                  if ( !vpr_empty.Empty() && m_cmscale )        // This chart scale does not fully cover the region
                  {
                        //    Save the current cm93 chart pointer for restoration later
                        cm93chart *m_pcm93chart_save = m_pcm93chart_current;
                        int cmscale_save = m_cmscale;

                        LLRegion region_vect[8];
                        
                        //    Render smaller scale cells the entire requested region is full
                        
                        while ( !vpr_empty.Empty() && m_cmscale )
                        {
                              //    get the next smaller scale chart
                              m_cmscale = PrepareChartScale ( vp, m_cmscale - 1, false );

                              if ( m_pcm93chart_current )
                              {
                                    LLRegion sscale_region = GetValidRegion();

                                    //  Save the calculated per-scale region in the array
                                    region_vect[m_cmscale] = sscale_region;
                                    region_vect[m_cmscale].Intersect ( vpr_empty );
                                    //    Only need to render that part of the vp that is not yet full
                                    //    Update the remaining empty region
                                    vpr_empty.Subtract ( sscale_region );
                              }

                        }     // while

                        //  Render all non-empty regions saved in the array, from small to large scale.
                        for( int i=0 ; i < 8 ; i++) {
                            if(!region_vect[i].Empty()){
                                m_cmscale = PrepareChartScale ( vp, i, false );
                                render_return |= m_pcm93chart_current->RenderRegionViewOnGL( glc, vp, RectRegion, region_vect[i] );
                            }
                        }

                        // restore the base chart pointer
                        m_pcm93chart_current = m_pcm93chart_save;
                        m_cmscale = cmscale_save;
                  }

                  //  Render the on-top Reference region/scale
                  render_return |= m_pcm93chart_current->RenderRegionViewOnGL( glc, vp, RectRegion, Region );

                  m_Name = m_pcm93chart_current->GetName();

            }
            else  // Single chart mode
            {
                render_return = m_pcm93chart_current->RenderRegionViewOnGL ( glc, vp, RectRegion, Region );
                m_Name = m_pcm93chart_current->GetLastFileName();
            }
      }

      if(VPoint.m_projection_type != PROJECTION_MERCATOR)
          return render_return; // TODO: fix below for non-mercator

      //    Render the cm93 cell's M_COVR outlines if called for
      if ( m_cell_index_special_outline )
      {
            ocpnDC dc;
            covr_set *pcover = m_pcm93chart_current->GetCoverSet();

            for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
            {
                  M_COVR_Desc *pmcd = pcover->GetCover ( im );
                  if ( ( pmcd->m_cell_index == m_cell_index_special_outline ) &&
                         ( pmcd->m_object_id == m_object_id_special_outline ) &&
                         ( pmcd->m_subcell == m_subcell_special_outline ) )

                  {
                        //    Draw this MCD's represented outline

                        //    Case:  vpBBox is completely inside the mcd box
//                        if(!( vp.vpBBox.IntersectOut(pmcd->m_covr_bbox)) || !( vp.vpBBox.IntersectOut(pmcd->m_covr_bbox)))
                        {

                              float_2Dpt *p = pmcd->pvertices;
                              wxPoint *pwp = m_pcm93chart_current->GetDrawBuffer ( pmcd->m_nvertices );

                              for ( int ip = 0 ; ip < pmcd->m_nvertices ; ip++ )
                              {

                                    double plon = p->x;
                                    if ( fabs ( plon - VPoint.clon ) > 180. )
                                    {
                                          if ( plon > VPoint.clon )
                                                plon -= 360.;
                                          else
                                                plon += 360.;
                                    }


                                    double easting, northing, epix, npix;
                                    toSM ( p->y, plon + 360., VPoint.clat, VPoint.clon + 360, &easting, &northing );

                                    //    Outlines stored in MCDs are not adjusted for offsets
//                                    easting -= pmcd->transform_WGS84_offset_x;
                                    easting -= pmcd->user_xoff;
//                                    northing -= pmcd->transform_WGS84_offset_y;
                                    northing -= pmcd->user_yoff;

                                    epix = easting  * VPoint.view_scale_ppm;
                                    npix = northing * VPoint.view_scale_ppm;

                                    pwp[ip].x = ( int ) round ( ( VPoint.pix_width  / 2 ) + epix );
                                    pwp[ip].y = ( int ) round ( ( VPoint.pix_height / 2 ) - npix );

                                    p++;
                              }

                              bool btest = true;
                              if ( btest )
                              {
                                    wxPen pen ( wxTheColourDatabase->Find ( _T ( "YELLOW" ) ), 3);
                                    wxDash dash1[2];
                                    dash1[0] = 4; // Long dash
                                    dash1[1] = 4; // Short gap
                                    pen.SetStyle(wxPENSTYLE_USER_DASH);
                                    pen.SetDashes( 2, dash1 );

                                    dc.SetPen ( pen );

                                    for ( int iseg=0 ; iseg < pmcd->m_nvertices-1 ; iseg++ )
                                    {

                                          int x0 = pwp[iseg].x;
                                          int y0 = pwp[iseg].y;
                                          int x1 = pwp[iseg+1].x;
                                          int y1 = pwp[iseg+1].y;

                                          ClipResult res = cohen_sutherland_line_clip_i ( &x0, &y0, &x1, &y1,
                                                      0, VPoint.pix_width, 0, VPoint.pix_height );

                                          if ( res == Invisible )                                                 // Do not bother with segments that are invisible
                                                continue;

                                          dc.DrawLine ( x0, y0, x1, y1 );
                                    }
                              }
                        }
                  }
            }
      }

      return render_return;
}



bool cm93compchart::RenderRegionViewOnDC ( wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region )
{
      SetVPParms ( VPoint );

      if ( g_pCM93OffsetDialog && g_pCM93OffsetDialog->IsShown() )
            g_pCM93OffsetDialog->UpdateMCOVRList ( VPoint );

      return DoRenderRegionViewOnDC ( dc, VPoint, Region );
}

bool cm93compchart::RenderViewOnDC ( wxMemoryDC& dc, const ViewPort& VPoint )
{
      const OCPNRegion vpr ( 0,0,VPoint.pix_width, VPoint.pix_height );

      SetVPParms ( VPoint );

      return DoRenderRegionViewOnDC ( dc, VPoint, vpr );

}

bool cm93compchart::DoRenderRegionViewOnDC ( wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region )
{
//      g_bDebugCM93 = true;

//      CALLGRIND_START_INSTRUMENTATION
      if ( g_bDebugCM93 )
      {
            printf ( "\nOn DoRenderRegionViewOnDC Ref scale is %d, %c\n", m_cmscale, ( char ) ( 'A' + m_cmscale -1 ) );
            OCPNRegionIterator upd ( Region );
            while ( upd.HaveRects() )
            {
                  wxRect rect = upd.GetRect();
                  printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
                  upd.NextRect();;
            }
      }


      ViewPort vp = VPoint;

      bool render_return = false;
      if ( m_pcm93chart_current )
      {
            m_pcm93chart_current->SetVPParms ( vp );

            //    Check the current chart scale to see if it covers the requested region totally
            if ( VPoint.b_quilt )
            {
                  OCPNRegion vpr_empty = Region;

                  OCPNRegion chart_region = GetValidScreenCanvasRegion ( vp, Region );

                  if ( g_bDebugCM93 )
                  {
                        printf ( "On DoRenderRegionViewOnDC : Intersecting Ref region rectangles\n" );
                        OCPNRegionIterator upd ( chart_region );
                        while ( upd.HaveRects() )
                        {
                              wxRect rect = upd.GetRect();
                              printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
                              upd.NextRect();
                        }
                  }

                  if ( !chart_region.IsEmpty() )
                        vpr_empty.Subtract ( chart_region );

                  if ( !vpr_empty.Empty() && m_cmscale )        // This chart scale does not fully cover the region
                  {
                        //    Render the target scale chart on a temp dc for safekeeping
#ifdef ocpnUSE_DIBSECTION
                        ocpnMemDC temp_dc;
#else
                        wxMemoryDC temp_dc;
#endif
                        if(!chart_region.IsEmpty())
                            render_return = m_pcm93chart_current->RenderRegionViewOnDC ( temp_dc, vp, chart_region );
                        else
                            render_return = false;

                        //    Save the current cm93 chart pointer for restoration later
                        cm93chart *m_pcm93chart_save = m_pcm93chart_current;

                        //    Prepare a blank quilt bitmap to build up the quilt upon
                        //    We need to do this in order to avoid polluting any of the sub-chart cached bitmaps
                        if ( m_pDummyBM )
                        {
                              if ( ( m_pDummyBM->GetWidth() != VPoint.rv_rect.width ) || ( m_pDummyBM->GetHeight() != VPoint.rv_rect.height ) )
                              {
                                    delete m_pDummyBM;
                                    m_pDummyBM = NULL;
                              }
                        }
                        if ( NULL == m_pDummyBM )
                              m_pDummyBM = new wxBitmap ( VPoint.rv_rect.width, VPoint.rv_rect.height,-1 );

                        //    Clear the quilt
#ifdef ocpnUSE_DIBSECTION
                        ocpnMemDC dumm_dc;
#else
                        wxMemoryDC dumm_dc;
#endif
                        dumm_dc.SelectObject ( *m_pDummyBM );
                        dumm_dc.SetBackground ( *wxBLACK_BRUSH );
                        dumm_dc.Clear();

                        int cmscale_next = m_cmscale;

                        //    Render smaller scale cells onto a temporary DC, blitting the valid region onto the quilt dc until the region is full
                        while ( !vpr_empty.Empty() && cmscale_next )
                        {
                              //    get the next smaller scale chart
                              cmscale_next--;
                              m_cmscale = PrepareChartScale ( vp, cmscale_next, false );
#ifdef ocpnUSE_DIBSECTION
                              ocpnMemDC build_dc;
#else
                              wxMemoryDC build_dc;
#endif

                              if ( m_pcm93chart_current )
                              {
                                    if ( g_bDebugCM93 )
                                          printf ( "  In DRRVOD,  add quilt patch at %d, %c\n", m_cmscale, ( char ) ( 'A' + m_cmscale -1 ) );

                                    m_pcm93chart_current->RenderRegionViewOnDC ( build_dc, vp, Region );

                                    OCPNRegion sscale_region = GetValidScreenCanvasRegion ( vp, Region );

                                    //    Only need to render that part of the vp that is not yet full
                                    sscale_region.Intersect ( vpr_empty );

                                    //    Blit the smaller scale chart patch onto the target DC
                                    OCPNRegionIterator upd ( sscale_region );
                                    while ( upd.HaveRects() )
                                    {
                                          wxRect rect = upd.GetRect();
                                          dumm_dc.Blit ( rect.x, rect.y, rect.width, rect.height, &build_dc, rect.x, rect.y );
                                          upd.NextRect();
                                    }
                                    build_dc.SelectObject ( wxNullBitmap );          // safely unmap the bmp

                                    //    Update the remaining empty region
                                    if ( !sscale_region.IsEmpty() )
                                          vpr_empty.Subtract ( sscale_region );
                              }

                        }     // while

                        //    Finally, Blit the target scale chart as saved on temp_dc to quilt dc
                        OCPNRegionIterator updt ( chart_region );
                        while ( updt.HaveRects() )
                        {
                              wxRect rect = updt.GetRect();
                              dumm_dc.Blit ( rect.x, rect.y, rect.width, rect.height, &temp_dc, rect.x, rect.y );
                              updt.NextRect();
                        }
                        temp_dc.SelectObject ( wxNullBitmap );          // safely unmap the base chart bmp


                        // restore the base chart pointer
                        m_pcm93chart_current = m_pcm93chart_save;

                        //  We can unselect the target from the dummy DC, to avoid having to copy it.
                        dumm_dc.SelectObject( wxNullBitmap );
                        
                        //    And the return dc is the quilt
                        dc.SelectObject ( *m_pDummyBM );

                        render_return = true;
                  }
                  else {
                        m_pcm93chart_current->RenderRegionViewOnDC ( dc, vp, Region );
                        render_return = true;
                  }
                  m_Name = m_pcm93chart_current->GetName();

            }
            else  // Single chart mode
            {
                  render_return = m_pcm93chart_current->RenderRegionViewOnDC ( dc, vp, Region );
                  m_Name = m_pcm93chart_current->GetLastFileName();
            }

      }
      else
      {
            //    one must always return a valid bitmap selected into the specified DC
            //    Since the CM93 cell is not available at this location, select a dummy placeholder
            if ( m_pDummyBM )
            {
                  if ( ( m_pDummyBM->GetWidth() != VPoint.pix_width ) || ( m_pDummyBM->GetHeight() != VPoint.pix_height ) )
                  {
                        delete m_pDummyBM;
                        m_pDummyBM = NULL;
                  }
            }

            if ( NULL == m_pDummyBM )
                  m_pDummyBM = new wxBitmap ( VPoint.pix_width, VPoint.pix_height,-1 );


            // Clear the bitmap
            wxMemoryDC mdc;
            mdc.SelectObject ( *m_pDummyBM );
            mdc.SetBackground ( *wxBLACK_BRUSH );
            mdc.Clear();
            mdc.SelectObject ( wxNullBitmap );


            dc.SelectObject ( *m_pDummyBM );
      }

//      CALLGRIND_STOP_INSTRUMENTATION

      //    Render the cm93 cell's M_COVR outlines if called for
      if ( m_cell_index_special_outline )
      {
            covr_set *pcover = m_pcm93chart_current->GetCoverSet();

            for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
            {
                  M_COVR_Desc *pmcd = pcover->GetCover ( im );
                  if ( ( pmcd->m_cell_index == m_cell_index_special_outline ) &&
                          ( pmcd->m_object_id == m_object_id_special_outline ) &&
                          ( pmcd->m_subcell == m_subcell_special_outline ) )

                  {
                        //    Draw this MCD's represented outline

                        //    Case:  vpBBox is completely inside the mcd box
//                        if(!( vp.vpBBox.IntersectOut(pmcd->m_covr_bbox)) || !( vp.vpBBox.IntersectOut(pmcd->m_covr_bbox)))
                        {

                              float_2Dpt *p = pmcd->pvertices;
                              wxPoint *pwp = m_pcm93chart_current->GetDrawBuffer ( pmcd->m_nvertices );

                              for ( int ip = 0 ; ip < pmcd->m_nvertices ; ip++ )
                              {

                                    double plon = p->x;
                                    if ( fabs ( plon - VPoint.clon ) > 180. )
                                    {
                                          if ( plon > VPoint.clon )
                                                plon -= 360.;
                                          else
                                                plon += 360.;
                                    }


                                    double easting, northing, epix, npix;
                                    toSM ( p->y, plon + 360., VPoint.clat, VPoint.clon + 360, &easting, &northing );

                                    //    Outlines stored in MCDs are not adjusted for offsets
//                                    easting -= pmcd->transform_WGS84_offset_x;
                                    easting -= pmcd->user_xoff;
//                                    northing -= pmcd->transform_WGS84_offset_y;
                                    northing -= pmcd->user_yoff;

                                    epix = easting  * VPoint.view_scale_ppm;
                                    npix = northing * VPoint.view_scale_ppm;

                                    pwp[ip].x = ( int ) round ( ( VPoint.pix_width  / 2 ) + epix );
                                    pwp[ip].y = ( int ) round ( ( VPoint.pix_height / 2 ) - npix );

                                    p++;
                              }

                              //    Scrub the points
                              //   looking for segments for which the wrong longitude decision was made
                              //    TODO all this mole needs to be rethought, again
                              bool btest = true;
                              /*
                                                            wxPoint p0 = pwp[0];
                                                            for(int ip = 1 ; ip < pmcd->m_nvertices ; ip++)
                                                            {
                               //                                   if(((p0.x > VPoint.pix_width) && (pwp[ip].x < 0)) || ((p0.x < 0) && (pwp[ip].x > VPoint.pix_width)))
                               //                                         btest = false;

                                                                  p0 = pwp[ip];
                                                            }
                              */
                              if ( btest )
                              {
                                    dc.SetPen ( wxPen ( wxTheColourDatabase->Find ( _T ( "YELLOW" ) ), 4, wxPENSTYLE_LONG_DASH ) );

                                    for ( int iseg=0 ; iseg < pmcd->m_nvertices-1 ; iseg++ )
                                    {

                                          int x0 = pwp[iseg].x;
                                          int y0 = pwp[iseg].y;
                                          int x1 = pwp[iseg+1].x;
                                          int y1 = pwp[iseg+1].y;

                                          ClipResult res = cohen_sutherland_line_clip_i ( &x0, &y0, &x1, &y1,
                                                           0, VPoint.pix_width, 0, VPoint.pix_height );

                                          if ( res == Invisible )                                                 // Do not bother with segments that are invisible
                                                continue;

                                          dc.DrawLine ( x0, y0, x1, y1 );
                                    }
                              }
                        }

                  }
            }
      }

      return render_return;
}


void cm93compchart::UpdateRenderRegions ( const ViewPort& VPoint )
{
      OCPNRegion full_screen_region(0,0,VPoint.rv_rect.width, VPoint.rv_rect.height);

      ViewPort vp = VPoint;

      SetVPParms ( VPoint );

      if ( m_pcm93chart_current )
      {
            m_pcm93chart_current->SetVPParms ( vp );

            //    Check the current chart scale to see if it covers the requested region totally
            if ( VPoint.b_quilt )
            {
                  //    Clear all the subchart regions
                  for ( int i = 0 ; i < 8 ; i++ )
                  {
                        if ( m_pcm93chart_array[i] )
                              m_pcm93chart_array[i]->m_render_region.Clear();
                  }

                  OCPNRegion vpr_empty = full_screen_region;

                  OCPNRegion chart_region = GetValidScreenCanvasRegion ( vp, full_screen_region );
                  m_pcm93chart_current->m_render_region = chart_region;       // update

                  if ( !chart_region.IsEmpty() )
                        vpr_empty.Subtract ( chart_region );


                  if ( !vpr_empty.Empty() && m_cmscale )        // This chart scale does not fully cover the region
                  {

                        //    Save the current cm93 chart pointer for restoration later
                        cm93chart *m_pcm93chart_save = m_pcm93chart_current;

                        int cmscale_next = m_cmscale;

                        while ( !vpr_empty.Empty() && cmscale_next )
                        {
                              //    get the next smaller scale chart
                              cmscale_next--;
                              m_cmscale = PrepareChartScale ( vp, cmscale_next, false );

                              if ( m_pcm93chart_current )
                              {
                                    OCPNRegion sscale_region = GetValidScreenCanvasRegion ( vp, full_screen_region );
                                    sscale_region.Intersect ( vpr_empty );
                                    m_pcm93chart_current->m_render_region = sscale_region;

                                    //    Update the remaining empty region
                                    if ( !sscale_region.IsEmpty() )
                                          vpr_empty.Subtract ( sscale_region );
                              }

                        }     // while

                        // restore the base chart pointer
                        m_pcm93chart_current = m_pcm93chart_save;
                  }
            }
      }
}





void cm93compchart::SetSpecialCellIndexOffset ( int cell_index, int object_id, int subcell, int xoff, int yoff )
{
      m_special_offset_x = xoff;
      m_special_offset_y = yoff;

      if ( m_pcm93chart_current )
            m_pcm93chart_current->SetUserOffsets ( cell_index, object_id, subcell, xoff, yoff );
}

bool cm93compchart::RenderNextSmallerCellOutlines ( ocpnDC &dc, ViewPort& vp )
{
      if ( m_cmscale >= 7 )
          return false;

#ifdef ocpnUSE_GL        
      ViewPort nvp;
      if(g_bopengl) /* opengl */ {
          wxPen pen = dc.GetPen();
          wxColour col = pen.GetColour();
          
          glEnable( GL_LINE_SMOOTH );
          glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
          glEnable( GL_BLEND );
          glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
          
          glColor3ub(col.Red(), col.Green(), col.Blue());
          glLineWidth( g_GLMinSymbolLineWidth );
          glDisable( GL_LINE_STIPPLE );
          dc.SetGLStipple();
          
          if(g_b_EnableVBO)
              s_glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
          glEnableClientState(GL_VERTEX_ARRAY);

          // use a viewport that allows the vertexes to be reused over many frames
          glPushMatrix();

          if(glChartCanvas::HasNormalizedViewPort(vp)) {
              glChartCanvas::MultMatrixViewPort(vp);
              nvp = glChartCanvas::NormalizedViewPort(vp);
          } else
              nvp = vp;
      }
#endif

      int nss_max;

      int nss = m_cmscale +1;

      //    A little magic here.
      //    Drawing all larger scale cell outlines is way too expensive.
      //    So, stop the loop after we have rendered "something"
      //    But don't stop at all if the viewport scale is less than 3 million.
      //    This will have the effect of bringing in outlines of isolated large scale cells
      //    embedded within small scale cells, like isolated islands in the Pacific.
      bool bdrawn = false;

      nss_max = 7;

#if 0 /* only if chart outlines are rendered grounded to the charts */
      if(g_bopengl) { /* for opengl: lets keep this simple yet also functioning
                         unlike the unbounded version (which is interesting)
                         the small update rectangles normally encountered when panning
                         can cause too many charts to load */
          if(nss_max > m_cmscale+3)
              nss_max = m_cmscale+3;
      }
#endif
      while ( nss <= nss_max && ( !bdrawn || ( vp.chart_scale < 3e6 ) ) )
      {
          cm93chart *psc = m_pcm93chart_array[nss];

          if ( !psc )
          {
              m_pcm93chart_array[nss] = new cm93chart();
              psc = m_pcm93chart_array[nss];

              wxChar ext = ( wxChar ) ( 'A' + nss - 1 );
              if ( nss == 0 )
                  ext = 'Z';

              wxString file_dummy = _T ( "CM93." );
              file_dummy << ext;

              psc->SetCM93Dict ( m_pDictComposite );
              psc->SetCM93Prefix ( m_prefixComposite );
              psc->SetCM93Manager ( m_pcm93mgr );

              psc->SetColorScheme ( m_global_color_scheme );
              psc->Init ( file_dummy, FULL_INIT );
                        
          }

          if ( nss != 1 ) {       // skip rendering the A scale outlines

              //      Make sure the covr bounding box is complete
              psc->UpdateCovrSet ( &vp );
                              
              /* test rectangle for entire set to reduce number of tests */
              if( !psc->m_covr_bbox.GetValid() ||
                  !vp.GetBBox().IntersectOut ( psc->m_covr_bbox ) ) 
              {
                  if ( psc ) 
                  {
                      //    Render the chart outlines
                      covr_set *pcover = psc->GetCoverSet();
                                  
                      for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ ){
                          M_COVR_Desc *mcd = pcover->GetCover ( im );
#ifdef ocpnUSE_GL        
                          if (g_bopengl) {
                              RenderCellOutlinesOnGL(nvp, mcd); 
                                      
                              // if signs don't agree we need to render a second pass
                              // translating around the world
                              if( (vp.m_projection_type == PROJECTION_MERCATOR ||
                                   vp.m_projection_type == PROJECTION_EQUIRECTANGULAR) &&
                                  ( vp.GetBBox().GetMinLon() < -180 ||
                                    vp.GetBBox().GetMaxLon() > 180) ) {
                                  #define NORM_FACTOR 4096.0                                              
                                  double ts = 40058986*NORM_FACTOR; /* 360 degrees in normalized viewport */
                                  glPushMatrix();
                                  glTranslated(vp.clon < 0 ? -ts : ts, 0, 0);
                                  RenderCellOutlinesOnGL(nvp, mcd); 
                                  glPopMatrix();
                              }
    
                              // TODO: this calculation doesn't work crossing IDL
                              // was anything actually drawn?
                              if(! ( vp.GetBBox().IntersectOut ( mcd->m_covr_bbox ) ) ) {
                                  bdrawn = true;

                                  //  Does current vp cross international dateline?
                                  // if so, translate to the other side of it.
                              }
                          } else
#endif
                              //    Anything actually to be drawn?
                              if(! ( vp.GetBBox().IntersectOut ( mcd->m_covr_bbox ) ) ) {
                                            
                                  wxPoint *pwp = psc->GetDrawBuffer ( mcd->m_nvertices );
                                  bdrawn = RenderCellOutlinesOnDC(dc, vp, pwp, mcd);
                              }
                      }
                  }                          
              }
          }
          nss ++;
      }

#ifdef ocpnUSE_GL        
      if(g_bopengl) {
          glPopMatrix();

          glDisableClientState(GL_VERTEX_ARRAY);
          glDisable( GL_LINE_STIPPLE );
          glDisable( GL_LINE_SMOOTH );
          glDisable( GL_BLEND );
      }
#endif

      return true;
}

bool cm93compchart::RenderCellOutlinesOnDC( ocpnDC &dc, ViewPort& vp, wxPoint *pwp, M_COVR_Desc *mcd )
{
    float_2Dpt *p = mcd->pvertices;
    int np = mcd->m_nvertices;
    
    for ( int ip = 0 ; ip < np ; ip++ ,  p++)
    {                                         
        pwp[ip] = vp.GetPixFromLL( p->y, p->x );
        
        //    Outlines stored in MCDs are not adjusted for offsets
        pwp[ip].x -= mcd->user_xoff * vp.view_scale_ppm;
        pwp[ip].y -= mcd->user_yoff * vp.view_scale_ppm;
    }
    //    Scrub the points
    //    looking for segments for which the wrong longitude decision was made
    //    TODO all this mole needs to be rethought, again
    wxPoint p0 = pwp[0];
    for ( int ip = 1 ; ip < np ; ip++ )
    {
        if ( ( ( p0.x > vp.pix_width ) && ( pwp[ip].x < 0 ) ) ||
             ( ( p0.x < 0 ) && ( pwp[ip].x > vp.pix_width ) ) )
            return false;
                                              
        p0 = pwp[ip];
    }
                                          
    dc.DrawLines ( mcd->m_nvertices, pwp, 0, 0, false );
    return true;
}


void cm93compchart::RenderCellOutlinesOnGL( ViewPort& vp, M_COVR_Desc *mcd )
{
#ifdef ocpnUSE_GL
    // cannot reuse coordinates
    if(vp.m_projection_type != mcd->gl_screen_projection_type ||
       !glChartCanvas::HasNormalizedViewPort(vp)) {
        delete [] mcd->gl_screen_vertices;
        mcd->gl_screen_vertices = NULL;
    }

    // if needed, cache normalized vertices
    if(!mcd->gl_screen_vertices) {
        // first compute a buffer size
        double lastlat, lastlon = 0;
        int count = 0;
        float_2Dpt *p = mcd->pvertices;
        for ( int ip = 0 ; ip < mcd->m_nvertices ; ip++, p++ ) {
            double lon = p->x;
            if(lon >= 180)
                lon -= 360;

            // crosses IDL? if so break up into two segments
            if(fabs(lon - lastlon) > 180)
                count++;

            count++;
            lastlon = lon;
        }

        mcd->gl_screen_vertices = new float_2Dpt[2*count];

        wxPoint2DDouble l;
        p = mcd->pvertices;
        float_2Dpt *q = mcd->gl_screen_vertices;
        lastlon = 0;

        bool lastvalid = false;
        for ( int ip = 0 ; ip < mcd->m_nvertices ; ip++, p++ ) {
            double lat = p->y;
            double lon = p->x;
            if(lon >= 180)
                lon -= 360;

            // crosses IDL? if so break up into two segments
            if(fabs(lon - lastlon) > 180) {
                if(lastvalid) {
                    wxPoint2DDouble r = vp.GetDoublePixFromLL(lastlat, lastlon > 0 ? 180 : -180);
                    if(!wxIsNaN(r.m_x)) {
                        q->y = l.m_x;
                        q->x = l.m_y;
                        q++;

                    //    Outlines stored in MCDs are not adjusted for offsets
                        r.m_x -= mcd->user_xoff * vp.view_scale_ppm;
                        r.m_y -= mcd->user_yoff * vp.view_scale_ppm;
                        
                        q->y = r.m_x;
                        q->x = r.m_y;
                        q++;
                    }
                }

                wxPoint2DDouble r = vp.GetDoublePixFromLL(lat, lon > 0 ? 180 : -180);
                if((lastvalid = !wxIsNaN(r.m_x))) {
                    r.m_x -= mcd->user_xoff * vp.view_scale_ppm;
                    r.m_y -= mcd->user_yoff * vp.view_scale_ppm;
                    l.m_x = r.m_x;
                }
            }

            lastlat = lat;
            lastlon = lon;
                                              
            wxPoint2DDouble s = vp.GetDoublePixFromLL( lat, lon );
            if(!wxIsNaN(s.m_x)) {
                //    Outlines stored in MCDs are not adjusted for offsets
                s.m_x -= mcd->user_xoff * vp.view_scale_ppm;
                s.m_y -= mcd->user_yoff * vp.view_scale_ppm;

                if(lastvalid) {
                    q->y = l.m_x;
                    q->x = l.m_y;
                    q++;
                
                    q->y = s.m_x;
                    q->x = s.m_y;
                    q++;
                }
                 
                l = s;
                lastvalid = true;
            } else
                lastvalid = false;
        }

        mcd->m_ngl_vertices = q - mcd->gl_screen_vertices;
        mcd->gl_screen_projection_type = vp.m_projection_type;
    }

#if 1 // Push array (faster)
    glVertexPointer(2, GL_FLOAT, 2*sizeof(float), mcd->gl_screen_vertices);
    glDrawArrays(GL_LINES, 0, mcd->m_ngl_vertices);
#else // immediate mode (may be useful for debugging buggy gfx cards)
    glBegin(GL_LINES);
    for(int i=0; i<mcd->m_ngl_vertices; i++)
        glVertex2f(mcd->gl_screen_vertices[i].y, mcd->gl_screen_vertices[i].x);
    glEnd();
#endif
#endif
}

void cm93compchart::GetPointPix ( ObjRazRules *rzRules, float rlat, float rlon, wxPoint *r )
{
      m_pcm93chart_current->GetPointPix ( rzRules, rlat, rlon, r );
}

void cm93compchart::GetPointPix ( ObjRazRules *rzRules, wxPoint2DDouble *en, wxPoint *r, int nPoints )
{
      m_pcm93chart_current->GetPointPix ( rzRules, en, r, nPoints );
}

void cm93compchart::GetPixPoint ( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
      m_pcm93chart_current->GetPixPoint ( pixx, pixy, plat, plon, vpt );
}

void cm93compchart::UpdateLUPs ( s57chart *pOwner )
{
      for ( int i = 0 ; i < 8 ; i++ )
      {
            if ( m_pcm93chart_array[i] )
                  m_pcm93chart_array[i]->UpdateLUPs ( pOwner );
      }
}

ListOfS57Obj *cm93compchart::GetAssociatedObjects ( S57Obj *obj )
{
      if ( m_pcm93chart_current )
            return  m_pcm93chart_current->GetAssociatedObjects ( obj );
      else
            return NULL;
}


void cm93compchart::InvalidateCache()
{
      for ( int i = 0 ; i < 8 ; i++ )
      {
            if ( m_pcm93chart_array[i] )
                  m_pcm93chart_array[i]->InvalidateCache();
      }
}

void cm93compchart::ForceEdgePriorityEvaluate ( void )
{
      for ( int i = 0 ; i < 8 ; i++ )
      {
            if ( m_pcm93chart_array[i] )
                  m_pcm93chart_array[i]->ForceEdgePriorityEvaluate();
      }
}

void cm93compchart::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
      m_global_color_scheme = cs;

      for ( int i = 0 ; i < 8 ; i++ )
      {
            if ( m_pcm93chart_array[i] )
                  m_pcm93chart_array[i]->SetColorScheme(cs, bApplyImmediate);
      }
}

ListOfObjRazRules *cm93compchart::GetObjRuleListAtLatLon ( float lat, float lon, float select_radius,
                                                           ViewPort *VPoint, int selection_mask )
{
      float alon = lon;

      ViewPort vp;          // needs a new ViewPort also for ObjectRenderCheck()
      vp = *VPoint;

      if ( !VPoint->b_quilt )
          if( m_pcm93chart_current )
              return  m_pcm93chart_current->GetObjRuleListAtLatLon ( lat, alon, select_radius, &vp );
          else {
              //     As default, return an empty list
              ListOfObjRazRules *ret_ptr = new ListOfObjRazRules;
              return ret_ptr;
          }
      else
      {
            UpdateRenderRegions ( *VPoint );

            //    Search all of the subcharts, looking for the one whose render region contains the requested point
            wxPoint p = VPoint->GetPixFromLL ( lat, lon );

            for ( int i = 0 ; i < 8 ; i++ )
            {
                  if ( m_pcm93chart_array[i] )
                  {

                        if ( !m_pcm93chart_array[i]->m_render_region.IsEmpty() )
                        {
                              if ( wxInRegion == m_pcm93chart_array[i]->m_render_region.Contains ( p ) )
                                    return  m_pcm93chart_array[i]->GetObjRuleListAtLatLon ( lat, alon,
                                                                                            select_radius, &vp,
                                                                                            selection_mask
                                                                                          );
                        }
                  }
            }

            //     As default, return an empty list
            ListOfObjRazRules *ret_ptr = new ListOfObjRazRules;

            return ret_ptr;

      }

}

VE_Hash& cm93compchart::Get_ve_hash ( void )
{
      return m_pcm93chart_current->Get_ve_hash();
}

VC_Hash& cm93compchart::Get_vc_hash ( void )
{
      return m_pcm93chart_current->Get_vc_hash();
}




bool cm93compchart::AdjustVP ( ViewPort &vp_last, ViewPort &vp_proposed )
{
#ifdef ocpnUSE_GL
      if(g_bopengl) {
          /* need a full refresh if not in quilted mode, and the cell changed */
          if ( !vp_last.b_quilt && m_last_cell_adjustvp != m_pcm93chart_current )
              glChartCanvas::Invalidate();

          m_last_cell_adjustvp = m_pcm93chart_current;
      }
#endif

    //  All the below logic is slow, and really redundant.
    //  so, declare that cm93 charts do not require adjustment for optimum performance.
    
    if( m_pcm93chart_current )
        return false;
    
      //    This may be a partial screen render
      //    If it is, the cmscale value on this render must match the same parameter
      //    on the last render.
      //    If it does not, the partial render will not quilt correctly with the previous data
      //    Detect this case, and indicate that the entire screen must be rendered.


      int cmscale = GetCMScaleFromVP ( vp_proposed );                   // This is the scale that should be used, based on the vp

      int cmscale_actual = PrepareChartScale ( vp_proposed, cmscale, false );  // this is the scale that will be used, based on cell coverage

      if ( g_bDebugCM93 )
            printf ( "  In AdjustVP,  adjustment subchart scale is %c\n", ( char ) ( 'A' + cmscale_actual -1 ) );

      //    We always need to do a VP adjustment, independent of this method's return value.
      //    so, do an AdjustVP() based on the chart scale that WILL BE USED
      //    And be sure to return false if that adjust method suggests so.

      bool single_adjust = false;
      if ( m_pcm93chart_array[cmscale_actual] )
            single_adjust = m_pcm93chart_array[cmscale_actual]->AdjustVP ( vp_last, vp_proposed );

      if ( m_cmscale != cmscale_actual )
            return false;

      //    In quilt mode, always indicate that the adjusted vp requires a full repaint
      if ( vp_last.b_quilt )
            return false;

      return single_adjust;
}

ThumbData *cm93compchart::GetThumbData ( int tnx, int tny, float lat, float lon )
{
      return ( ThumbData * ) NULL;
}

InitReturn cm93compchart::CreateHeaderData()
{

      m_Chart_Scale = 20000000;

      //        Read the root directory, getting subdirectories to build a small scale coverage region
      wxRect extent_rect;

      wxDir dirt(m_prefixComposite);
      wxString candidate;
      wxRegEx test(_T("[0-9]+"));

      bool b_cont = dirt.GetFirst(&candidate);

      while(b_cont) {
          if(test.Matches(candidate)&& (candidate.Len() == 8)) {
              wxString dir = m_prefixComposite;
              dir += candidate;
              if( wxDir::Exists(dir) ) {
                  wxFileName name( dir );
                  wxString num_name = name.GetName();
                  long number;
                  if( num_name.ToLong( &number ) ) {
                      int ilat = number / 10000;
                      int ilon = number % 10000;

                      int lat_base = ( ilat - 270 ) / 3.;
                      int lon_base = ilon / 3.;
                      extent_rect.Union(wxRect(lon_base, lat_base, 20, 20));
                  }
              }
          }
          b_cont = dirt.GetNext(&candidate);
      }

      //    Specify the chart coverage
      m_FullExtent.ELON = ((double)extent_rect.x + (double)extent_rect.width );
      m_FullExtent.WLON = ((double)extent_rect.x);
      m_FullExtent.NLAT = ((double)extent_rect.y + (double)extent_rect.height );
      m_FullExtent.SLAT = ((double)extent_rect.y);
      m_bExtentSet = true;


      //    Populate one M_COVR Entry
      m_nCOVREntries = 1;
      m_pCOVRTablePoints = ( int * ) malloc ( sizeof ( int ) );
      *m_pCOVRTablePoints = 4;
      m_pCOVRTable = ( float ** ) malloc ( sizeof ( float * ) );
      float *pf = ( float * ) malloc ( 2 * 4 * sizeof ( float ) );
      *m_pCOVRTable = pf;
      float *pfe = pf;

      *pfe++ = m_FullExtent.NLAT; //LatMax;
      *pfe++ = m_FullExtent.WLON; //LonMin;

      *pfe++ = m_FullExtent.NLAT; //LatMax;
      *pfe++ = m_FullExtent.ELON; //LonMax;

      *pfe++ = m_FullExtent.SLAT; //LatMin;
      *pfe++ = m_FullExtent.ELON; //LonMax;

      *pfe++ = m_FullExtent.SLAT; //LatMin;
      *pfe++ = m_FullExtent.WLON; //LonMin;


      return INIT_OK;
}

cm93_dictionary *cm93compchart::FindAndLoadDictFromDir ( const wxString &dir )
{
      cm93_dictionary *retval = NULL;
      cm93_dictionary *pdict = new cm93_dictionary();

      //    Quick look at the supplied directory...
      if ( pdict->LoadDictionary ( dir ) )
            return pdict;


      //    Otherwise, search for the dictionary files all along the path of the passed parameter

      wxString path = dir;
      wxString target;
      unsigned int i = 0;

      while ( i < path.Len() )
      {
            target.Append ( path[i] );
            if ( path[i] == wxFileName::GetPathSeparator() )
            {
//                  wxString msg = _T ( " Looking for CM93 dictionary in " );
//                  msg.Append ( target );
//                  wxLogMessage ( msg );

                  if ( pdict->LoadDictionary ( target ) )
                  {
                        retval = pdict;
                        break;
                  }
            }
            i++;
      }

      if ( NULL != retval )                           // Found it....
            return retval;




      //    Dictionary was not found in linear path of supplied dir.
      //    Could be on branch, so, look at entire tree the hard way.

      wxFileName fnc ( dir );
      wxString found_dict_file_name;

      bool bdone = false;
      while ( !bdone )
      {
            path = fnc.GetPath ( wxPATH_GET_VOLUME );     // get path without sep

            wxString msg = _T ( " Looking harder for CM93 dictionary in " );
            msg.Append ( path );
            wxLogMessage ( msg );


            if ( ( path.Len() == 0 ) || path.IsSameAs ( fnc.GetPathSeparator() ) )
            {
                  bdone = true;
                  wxLogMessage ( _T ( "Early break1" ) );
                  break;
            }

            //    Abort the search loop if the directory tree does not contain some indication of CM93
            if ( ( wxNOT_FOUND == path.Lower().Find ( _T ( "cm93" ) ) ) )
            {
                  bdone = true;
                  wxLogMessage ( _T ( "Early break2" ) );
                  break;
            }

//    Search here
//    This takes a while to search a fully populated cm93 tree....
            wxDir dir ( path );

            if ( dir.IsOpened() )
            {
                  // Find the dictionary name, case insensitively
                  FindCM93Dictionary cm93Dictionary ( found_dict_file_name );
                  dir.Traverse ( cm93Dictionary );
                  bdone = found_dict_file_name.Len() != 0;
            }

            fnc.Assign ( path );                              // convert the path to a filename for next loop
      }

      if ( found_dict_file_name.Len() )
      {
            wxFileName fnd ( found_dict_file_name );
            wxString dpath = fnd.GetPath ( ( int ) ( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME ) );

            if ( pdict->LoadDictionary ( dpath ) )
                  retval = pdict;
      }


      if ( NULL == retval )
            delete pdict;

      return retval;


}

void cm93compchart::CloseandReopenCurrentSubchart ( void )
{
      delete  m_pcm93chart_current;
      m_pcm93chart_current = NULL;
      m_pcm93chart_array[m_cmscale] = NULL;

      SetVPParms ( m_vpt );
      InvalidateCache();
}


class CM93OffsetDialog;

enum
{
      tlCELL = 0,
      tlMCOVR,
      tlSCALE,
      tlXOFF,
      tlYOFF,
      tlUXOFF,
      tlUYOFF,
};// OCPNOffsetListCtrl Columns;

//---------------------------------------------------------------------------------------
//          OCPNOffsetListCtrl Definition
//---------------------------------------------------------------------------------------
class OCPNOffsetListCtrl: public wxListCtrl
{
      public:
            OCPNOffsetListCtrl ( CM93OffsetDialog* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style );
            ~OCPNOffsetListCtrl();

            wxString OnGetItemText ( long item, long column ) const;
            int OnGetItemColumnImage ( long item, long column ) const;

            CM93OffsetDialog     *m_parent;


};


OCPNOffsetListCtrl::OCPNOffsetListCtrl ( CM93OffsetDialog* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
            wxListCtrl ( parent, id, pos, size, style )
{
      m_parent = parent;
}

OCPNOffsetListCtrl::~OCPNOffsetListCtrl()
{
}


wxString  OCPNOffsetListCtrl::OnGetItemText ( long item, long column ) const
{

      wxString ret;
      M_COVR_Desc *pmcd = m_parent->m_pcovr_array.Item ( item );

      switch ( column )
      {
            case tlCELL:
            {
                  ret.Printf ( _T ( "%d" ), pmcd->m_cell_index );
                  if ( ( ( int ) '0' ) == pmcd->m_subcell )
                        ret.Prepend ( _T ( "0" ) );
                  else
                  {
                        char t = ( char ) pmcd->m_subcell;
                        wxString p;
                        p.Printf ( _T ( "%c" ), t );
                        ret.Prepend ( p );
                  }

                  break;
            }
            case tlMCOVR:
                  ret.Printf ( _T ( "%d" ), pmcd->m_object_id );
                  break;

            case tlSCALE:
                  ret = m_parent->m_selected_chart_scale_char;
                  break;

            case tlXOFF:
                  ret.Printf ( _T ( "%g" ), pmcd->transform_WGS84_offset_x );
                  break;

            case tlYOFF:
                  ret.Printf ( _T ( "%g" ), pmcd->transform_WGS84_offset_y );
                  break;

            case tlUXOFF:
                  ret.Printf ( _T ( "%6.0f" ), pmcd->user_xoff * pmcd->m_centerlat_cos );
                  break;

            case tlUYOFF:
                  ret.Printf ( _T ( "%6.0f" ), pmcd->user_yoff * pmcd->m_centerlat_cos );
                  break;

            default:
                  break;
      }
      return ret;
}


int OCPNOffsetListCtrl::OnGetItemColumnImage ( long item, long column ) const
{
      return -1;
}



//---------------------------------------------------------------------------------------
//          CM93OffsetDialog Implementation
//---------------------------------------------------------------------------------------

IMPLEMENT_CLASS ( CM93OffsetDialog, wxDialog )

BEGIN_EVENT_TABLE ( CM93OffsetDialog, wxDialog )
      EVT_CLOSE ( CM93OffsetDialog::OnClose )
END_EVENT_TABLE()


CM93OffsetDialog::CM93OffsetDialog ( wxWindow *parent )
{
      m_pparent = parent;
      m_pcompchart = NULL;

      m_xoff = 0;
      m_yoff = 0;

      m_selected_list_index = -1;

      long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
      wxDialog::Create ( parent, -1, _ ( "OpenCPN CM93 Cell Offset Adjustments" ), wxPoint ( 0, 0 ), wxSize ( 800, 200 ), wstyle );



// A top-level sizer
      wxBoxSizer* topSizer = new wxBoxSizer ( wxHORIZONTAL );
      SetSizer ( topSizer );


      int width;

      m_pListCtrlMCOVRs = new OCPNOffsetListCtrl ( this, -1, wxDefaultPosition, wxDefaultSize,
              wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_HRULES|wxLC_VRULES|wxBORDER_SUNKEN|wxLC_VIRTUAL );

      m_pListCtrlMCOVRs->Connect ( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler ( CM93OffsetDialog::OnCellSelected ), NULL, this );

      width = 80;
      m_pListCtrlMCOVRs->InsertColumn ( tlCELL, _ ( "Cell" ), wxLIST_FORMAT_LEFT, width );

      width = 80;
      m_pListCtrlMCOVRs->InsertColumn ( tlMCOVR, _ ( "M_COVR ID" ), wxLIST_FORMAT_CENTER, width );

      width = 80;
      m_pListCtrlMCOVRs->InsertColumn ( tlSCALE, _ ( "Cell Scale" ), wxLIST_FORMAT_CENTER, width );

      width = 90;
      m_pListCtrlMCOVRs->InsertColumn ( tlXOFF, _ ( "wgsox" ), wxLIST_FORMAT_CENTER, width );

      width = 90;
      m_pListCtrlMCOVRs->InsertColumn ( tlYOFF, _ ( "wgsoy" ), wxLIST_FORMAT_CENTER, width );

      width = 90;
      m_pListCtrlMCOVRs->InsertColumn ( tlUXOFF, _ ( "User X Offset" ), wxLIST_FORMAT_CENTER, width );

      width = 90;
      m_pListCtrlMCOVRs->InsertColumn ( tlUYOFF, _ ( "User Y Offset" ), wxLIST_FORMAT_CENTER, width );


      topSizer->Add ( m_pListCtrlMCOVRs, 1, wxEXPAND|wxALL, 0 );

      wxBoxSizer* boxSizer02 = new wxBoxSizer ( wxVERTICAL );
      boxSizer02->AddSpacer ( 22 );

      wxStaticText *pStaticTextXoff = new wxStaticText ( this, wxID_ANY, _ ( "User X Offset (Metres)" ), wxDefaultPosition, wxDefaultSize, 0 );
      boxSizer02->Add ( pStaticTextXoff, 0, wxALL, 0 );

      m_pSpinCtrlXoff = new wxSpinCtrl ( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize ( 50, -1 ), wxSP_ARROW_KEYS, -10000, 10000, 0 );
      m_pSpinCtrlXoff->Connect ( wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler ( CM93OffsetDialog::OnOffSetSet ), NULL, this );
      boxSizer02->Add ( m_pSpinCtrlXoff, 0, wxEXPAND|wxALL, 0 );

      wxStaticText *pStaticTextYoff = new wxStaticText ( this, wxID_ANY, _ ( "User Y Offset (Metres)" ), wxDefaultPosition, wxDefaultSize, 0 );
      boxSizer02->Add ( pStaticTextYoff, 0, wxALL, 0 );

      m_pSpinCtrlYoff = new wxSpinCtrl ( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize ( 50, -1 ), wxSP_ARROW_KEYS, -10000, 10000, 0 );
      m_pSpinCtrlYoff->Connect ( wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler ( CM93OffsetDialog::OnOffSetSet ), NULL, this );
      boxSizer02->Add ( m_pSpinCtrlYoff, 0, wxEXPAND|wxALL, 0 );

      m_OKButton = new wxButton ( this, wxID_ANY, _ ( "OK" ), wxDefaultPosition, wxDefaultSize, 0 );
      m_OKButton->Connect ( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler ( CM93OffsetDialog::OnOK ), NULL, this );
      boxSizer02->Add ( m_OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
      m_OKButton->SetDefault();

      topSizer->Add ( boxSizer02, 0, wxEXPAND|wxALL, 2 );
      topSizer->Layout();

      //    This is silly, but seems to be required for __WXMSW__ build
      //    If not done, the SECOND invocation of dialog fails to expand the list to the full wxSizer size....
      SetSize ( GetSize().x, GetSize().y-1 );

      SetColorScheme();

//      GetSizer()->SetSizeHints(this);
      Centre();


}


CM93OffsetDialog::~CM93OffsetDialog( )
{
}

void CM93OffsetDialog::OnClose ( wxCloseEvent& event )
{
      if ( m_pcompchart )
      {
            m_pcompchart->SetSpecialOutlineCellIndex ( 0, 0, 0 );

            m_pcompchart->InvalidateCache();

            if ( m_pparent ) {
                  m_pparent->Refresh ( true );
                  cc1->InvalidateGL();
            }
      }

      if ( m_pListCtrlMCOVRs->GetItemCount() > m_selected_list_index )
            m_pListCtrlMCOVRs->SetItemState ( m_selected_list_index, 0, wxLIST_STATE_SELECTED );

      Hide();
}


void CM93OffsetDialog::OnOK ( wxCommandEvent& event )
{
      Close();
}

void CM93OffsetDialog::SetCM93Chart( cm93compchart *pchart )
{ 
    m_pcompchart = pchart;
}

void CM93OffsetDialog::OnOffSetSet ( wxCommandEvent& event )
{
    m_xoff = m_pSpinCtrlXoff->GetValue() / m_centerlat_cos;
    m_yoff = m_pSpinCtrlYoff->GetValue() / m_centerlat_cos;

      UpdateOffsets();

}

void CM93OffsetDialog::UpdateOffsets ( void )
{
      if ( m_pcompchart )
      {
            //    Set the offsets of the selected cell/object
            m_pcompchart->SetSpecialCellIndexOffset ( m_selected_cell_index, m_selected_object_id, m_selected_subcell, m_xoff, m_yoff );

            //    Closing the current cell will record the offsets in the M_COVR cache file
            //    Re-opening will then refresh the M_COVRs in the cover set
            OCPNPlatform::ShowBusySpinner();
            m_pcompchart->CloseandReopenCurrentSubchart();
            OCPNPlatform::ShowBusySpinner();

            if ( m_pparent ) {
                  m_pparent->Refresh ( true );
                  cc1->InvalidateGL();
            }
      }
}


void CM93OffsetDialog::SetColorScheme()
{

      DimeControl(this);

}



void CM93OffsetDialog::OnCellSelected ( wxListEvent &event )
{
    if ( m_pcompchart ){
        
      m_selected_list_index = event.GetIndex();

      M_COVR_Desc *mcd =  m_pcovr_array.Item ( event.GetIndex() );

      if ( m_selected_list_index > m_pListCtrlMCOVRs->GetItemCount() )
            return;            // error

      cm93chart *pchart = m_pcompchart->GetCurrentSingleScaleChart();
      if ( pchart )
      {
            M_COVR_Desc *cached_mcd = pchart->GetCoverSet()->Find_MCD ( mcd->m_cell_index, mcd->m_object_id, mcd->m_subcell );
            if ( cached_mcd )
            {
                m_pSpinCtrlXoff->SetValue ( wxRound ( cached_mcd->user_xoff * cached_mcd->m_centerlat_cos ) );
                m_pSpinCtrlYoff->SetValue ( wxRound ( cached_mcd->user_yoff * cached_mcd->m_centerlat_cos ) );
            }
      }

      m_pcompchart->SetSpecialOutlineCellIndex ( mcd->m_cell_index, mcd->m_object_id, mcd->m_subcell );

      m_selected_cell_index = mcd->m_cell_index;
      m_selected_object_id  = mcd->m_object_id;
      m_selected_subcell = mcd->m_subcell;
      m_centerlat_cos = mcd->m_centerlat_cos;
      
      m_pcompchart->InvalidateCache();

      if ( m_pparent ) {
            m_pparent->Refresh ( true );
            cc1->InvalidateGL();
      }
    }
}



void CM93OffsetDialog::UpdateMCOVRList ( const ViewPort &vpt )
{
      if ( m_pcompchart )
      {
            //    In single chart mode, there is but one cm93chart (i.e. one "scale value") shown at any one time
            cm93chart *pchart = m_pcompchart->GetCurrentSingleScaleChart();

            if ( pchart )
            {
                  m_selected_chart_scale_char = pchart->GetScaleChar();

                  m_pcovr_array.Clear();

                  //    Get an array of cell indicies at the current viewport
                  ArrayOfInts cell_array = pchart->GetVPCellArray ( vpt );

                  ViewPort vp;
                  vp = vpt;

                  //    Get the cover set for the cm93chart
                  //    and walk the set looking for matches to the viewport referenced cell array
                  //    This will give us the covr descriptors of interest
                  covr_set *pcover = pchart->GetCoverSet();

                  for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
                  {
                        M_COVR_Desc *mcd = pcover->GetCover ( im );

                        for ( unsigned int icell=0 ; icell < cell_array.GetCount() ; icell++ )
                        {
                              if ( cell_array.Item ( icell ) == mcd->m_cell_index )
                              {
                                    wxPoint *pwp = pchart->GetDrawBuffer ( mcd->m_nvertices );
                                    OCPNRegion rgn = mcd->GetRegion ( vp, pwp );

//                                    if( !vp.GetBBox().IntersectOut(mcd->m_covr_bbox))
                                    if ( rgn.Contains ( 0, 0, vpt.pix_width, vpt.pix_height ) != wxOutRegion )
                                          m_pcovr_array.Add ( mcd );
                              }
                        }
                  }

                  //    Try to find and maintain the correct list selection, even though the list contents may have changed
                  int sel_index = -1;
                  for ( unsigned int im=0 ; im < m_pcovr_array.GetCount() ; im++ )
                  {
                        M_COVR_Desc *mcd = m_pcovr_array.Item ( im );
                        if ( ( m_selected_cell_index == mcd->m_cell_index ) &&
                                ( m_selected_object_id == mcd->m_object_id ) &&
                                ( m_selected_subcell == mcd->m_subcell ) )
                        {
                              sel_index = im;
                              break;
                        }
                  }

                  m_pListCtrlMCOVRs->SetItemCount ( m_pcovr_array.GetCount() );
                  if ( -1 != sel_index )
                        m_pListCtrlMCOVRs->SetItemState ( sel_index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
                  else
                        m_pListCtrlMCOVRs->SetItemState ( sel_index, 0, wxLIST_STATE_SELECTED );   // deselect all

                  m_pListCtrlMCOVRs->Refresh ( true );


            }
#ifdef __WXMSW__
            m_pListCtrlMCOVRs->Refresh ( false );
#endif
      }

}




