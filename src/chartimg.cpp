/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  ChartBase, ChartBaseBSB and Friends
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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


// ============================================================================
// declarations
// ============================================================================


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers


//  Why are these not in wx/prec.h?
#include "wx/dir.h"
#include "wx/stream.h"
#include "wx/wfstream.h"
#include "wx/tokenzr.h"
#include "wx/filename.h"
#include <wx/image.h>
#include <wx/fileconf.h>
#include <sys/stat.h>


#include "chartimg.h"
#include "ocpn_pixel.h"
#include "ChartDataInputStream.h"

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>

#define OCPN_USE_CONFIG 1

struct sigaction sa_all_chart;
struct sigaction sa_all_previous;

sigjmp_buf           env_chart;                 // the context saved by sigsetjmp();

void catch_signals_chart(int signo)
{
      switch(signo)
      {
            case SIGSEGV:
                  siglongjmp(env_chart, 1);     // jump back to the setjmp() point
                  break;

            default:
                  break;
      }
}

#endif

//  Missing from MSW include files
#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

// ----------------------------------------------------------------------------
// Random Prototypes
// ----------------------------------------------------------------------------

#ifdef OCPN_USE_CONFIG
class MyConfig;
extern MyConfig        *pConfig;
#endif

typedef struct  {
      float y;
      float x;
} MyFlPoint;


bool G_FloatPtInPolygon(MyFlPoint *rgpts, int wnumpts, float x, float y) ;


// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------


// ============================================================================
// ThumbData implementation
// ============================================================================

ThumbData::ThumbData()
{
    pDIBThumb = NULL;
}

ThumbData::~ThumbData()
{
    delete pDIBThumb;
}

// ============================================================================
// Palette implementation
// ============================================================================
opncpnPalette::opncpnPalette()
{
    // Index into palette is 1-based, so predefine the first entry as null
    nFwd = 1;
    nRev = 1;
    FwdPalette =(int *)malloc( sizeof(int));
    RevPalette =(int *)malloc( sizeof(int));
    FwdPalette[0] = 0;
    RevPalette[0] = 0;
}

opncpnPalette::~opncpnPalette()
{
    if(NULL != FwdPalette)
        free( FwdPalette );
    if(NULL != RevPalette)
        free( RevPalette ) ;
}

// ============================================================================
// ChartBase implementation
// ============================================================================
ChartBase::ChartBase()
{
      m_depth_unit_id = DEPTH_UNIT_UNKNOWN;

      pThumbData = new ThumbData;

      m_global_color_scheme = GLOBAL_COLOR_SCHEME_RGB;

      bReadyToRender = false;

      Chart_Error_Factor = 0;

      m_Chart_Scale = 10000;              // a benign value
      m_Chart_Skew = 0.0;

      m_nCOVREntries = 0;
      m_pCOVRTable = NULL;
      m_pCOVRTablePoints = NULL;

      m_nNoCOVREntries = 0;
      m_pNoCOVRTable = NULL;
      m_pNoCOVRTablePoints = NULL;
      
      m_EdDate.Set(1, wxDateTime::Jan, 2000);

      m_lon_datum_adjust = 0.;
      m_lat_datum_adjust = 0.;

      m_projection = PROJECTION_MERCATOR;             // default
}

ChartBase::~ChartBase()
{
      delete pThumbData;

      //    Free the COVR tables

      for(unsigned int j=0 ; j<(unsigned int)m_nCOVREntries ; j++)
            free( m_pCOVRTable[j] );

      free( m_pCOVRTable );
      free( m_pCOVRTablePoints );

      //    Free the No COVR tables

      for(unsigned int j=0 ; j<(unsigned int)m_nNoCOVREntries ; j++)
            free( m_pNoCOVRTable[j] );

      free( m_pNoCOVRTable );
      free( m_pNoCOVRTablePoints );

}
/*
int ChartBase::Continue_BackgroundHiDefRender(void)
{
      return BR_DONE_NOP;            // signal "done, no refresh"
}
*/

// ============================================================================
// ChartDummy implementation
// ============================================================================

ChartDummy::ChartDummy()
{
      m_pBM = NULL;
      m_ChartType = CHART_TYPE_DUMMY;
      m_ChartFamily = CHART_FAMILY_UNKNOWN;
      m_Chart_Scale = 22000000;

      m_FullPath = _("No Chart Available");
      m_Description = m_FullPath;

}

ChartDummy::~ChartDummy()
{
      delete m_pBM;
}


InitReturn ChartDummy::Init( const wxString& name, ChartInitFlag init_flags )
{
      return INIT_OK;
}

void ChartDummy::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
}


ThumbData *ChartDummy::GetThumbData(int tnx, int tny, float lat, float lon)
{
      return (ThumbData *)NULL;
}

bool ChartDummy::UpdateThumbData(double lat, double lon)
{
      return FALSE;
}


bool ChartDummy::GetChartExtent(Extent *pext)
{
    pext->NLAT = 80;
    pext->SLAT = -80;
    pext->ELON = 179;
    pext->WLON = -179;

    return true;
}

bool ChartDummy::RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint, const OCPNRegion &RectRegion, const LLRegion &Region)
{
      return true;
}




bool ChartDummy::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region)
{
      return RenderViewOnDC(dc, VPoint);
}

bool ChartDummy::RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint)
{
      if( m_pBM  && m_pBM->IsOk() )
      {
            if((m_pBM->GetWidth() != VPoint.pix_width) || (m_pBM->GetHeight() != VPoint.pix_height))
            {
                  delete m_pBM;
                  m_pBM = NULL;
            }
      }
      else {
          delete m_pBM;
          m_pBM =NULL;
      }

      if( VPoint.pix_width && VPoint.pix_height ) {
        if(NULL == m_pBM)
            m_pBM = new wxBitmap(VPoint.pix_width, VPoint.pix_height,-1);

        dc.SelectObject(*m_pBM);

        dc.SetBackground(*wxBLACK_BRUSH);
        dc.Clear();
      }

      return true;
}


bool ChartDummy::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed)
{
      return false;
}


void ChartDummy::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion)
{
      pValidRegion->Clear();
      pValidRegion->Union(0, 0, 1, 1);
}

LLRegion ChartDummy::GetValidRegion()
{
    return LLRegion();
}






// ============================================================================
// ChartGEO implementation
// ============================================================================
ChartGEO::ChartGEO()
{
      m_ChartType = CHART_TYPE_GEO;
}


ChartGEO::~ChartGEO()
{
}

InitReturn ChartGEO::Init( const wxString& name, ChartInitFlag init_flags)
{
      #define BUF_LEN_MAX 4096

      PreInit(name, init_flags, GLOBAL_COLOR_SCHEME_DAY);

      char buffer[BUF_LEN_MAX];

      ifs_hdr = new wxFFileInputStream(name);          // open the file as a read-only stream

      m_filesize = wxFileName::GetSize( name );
      
      if(!ifs_hdr->IsOk())
            return INIT_FAIL_REMOVE;

      int nPlypoint = 0;
      Plypoint *pPlyTable = (Plypoint *)malloc(sizeof(Plypoint));

      m_FullPath = name;
      m_Description = m_FullPath;

      wxFileName GEOFile(m_FullPath);

      wxString Path;
      Path = GEOFile.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);


//    Read the GEO file, extracting useful information

      ifs_hdr->SeekI(0, wxFromStart);                 // rewind

      Size_X = Size_Y = 0;

      while( (ReadBSBHdrLine(ifs_hdr, &buffer[0], BUF_LEN_MAX)) != 0 )
    {
          wxString str_buf(buffer, wxConvUTF8);
            if(!strncmp(buffer, "Bitmap", 6))
            {
                wxStringTokenizer tkz(str_buf, _T("="));
                  wxString token = tkz.GetNextToken();
                  if(token.IsSameAs(_T("Bitmap"), TRUE))
                  {
                        pBitmapFilePath = new wxString();

                        int i;
                        i = tkz.GetPosition();
                        pBitmapFilePath->Clear();
                        while(buffer[i])
                        {
                              pBitmapFilePath->Append(buffer[i]);
                              i++;
                        }
                  }
            }


            else if(!strncmp(buffer, "Scale", 5))
            {
                wxStringTokenizer tkz(str_buf, _T("="));
                  wxString token = tkz.GetNextToken();
                  if(token.IsSameAs(_T("Scale"), TRUE))               // extract Scale
                  {
                        int i;
                        i = tkz.GetPosition();
                        m_Chart_Scale = atoi(&buffer[i]);
                  }
            }

            else if(!strncmp(buffer, "Depth", 5))
            {
                wxStringTokenizer tkz(str_buf, _T("="));
                wxString token = tkz.GetNextToken();
                if(token.IsSameAs(_T("Depth Units"), FALSE))               // extract Depth Units
                {
                    int i;
                    i = tkz.GetPosition();
                    wxString str(&buffer[i],  wxConvUTF8);
                    m_DepthUnits = str.Trim();
                }
            }

            else if (!strncmp(buffer, "Point", 5))                // Extract RefPoints
            {
                  int i, xr, yr;
                  float ltr,lnr;
                  sscanf(&buffer[0], "Point%d=%f %f %d %d", &i, &lnr, &ltr, &yr, &xr);
                  pRefTable = (Refpoint *)realloc(pRefTable, sizeof(Refpoint) * (nRefpoint+1));
                  pRefTable[nRefpoint].xr = xr;
                  pRefTable[nRefpoint].yr = yr;
                  pRefTable[nRefpoint].latr = ltr;
                  pRefTable[nRefpoint].lonr = lnr;
                  pRefTable[nRefpoint].bXValid = 1;
                  pRefTable[nRefpoint].bYValid = 1;

                  nRefpoint++;

            }

            else if (!strncmp(buffer, "Vertex", 6))
            {
                  int i;
                  float ltp,lnp;
                  sscanf(buffer, "Vertex%d=%f %f", &i, &ltp, &lnp);
                  Plypoint *tmp = pPlyTable;
                  pPlyTable = (Plypoint *)realloc(pPlyTable, sizeof(Plypoint) * (nPlypoint+1));
                  if (NULL == pPlyTable)
                  {
                      free(tmp);
                      tmp = NULL;
                  } else
                  {
                      pPlyTable[nPlypoint].ltp = ltp;
                      pPlyTable[nPlypoint].lnp = lnp;
                      nPlypoint++;
                  }
            }

            else if (!strncmp(buffer, "Date Pub", 8))
            {
                  char date_string[40];
                  char date_buf[10];
                  sscanf(buffer, "Date Published=%s\r\n", &date_string[0]);
                  wxString date_wxstr(date_string,  wxConvUTF8);
                  wxDateTime dt;
                  if(dt.ParseDate(date_wxstr))       // successful parse?
                  {
                        sprintf(date_buf, "%d", dt.GetYear());
                  }
                  else
                  {
                        sscanf(date_string, "%s", date_buf);
                  }
                  m_PubYear = wxString(date_buf, wxConvUTF8);
            }

            else if (!strncmp(buffer, "Skew", 4))
            {
                wxStringTokenizer tkz(str_buf, _T("="));
                  wxString token = tkz.GetNextToken();
                  if(token.IsSameAs(_T("Skew Angle"), FALSE))               // extract Skew Angle
                  {
                        int i;
                        i = tkz.GetPosition();
                        float fcs;
                        sscanf(&buffer[i], "%f,", &fcs);
                        m_Chart_Skew = fcs;
                  }
            }

            else if (!strncmp(buffer, "Latitude Offset", 15))
            {
                  wxStringTokenizer tkz(str_buf, _T("="));
                  wxString token = tkz.GetNextToken();
                  if(token.IsSameAs(_T("Latitude Offset"), FALSE))
                  {
                        int i;
                        i = tkz.GetPosition();
                        float lto;
                        sscanf(&buffer[i], "%f,", &lto);
                        m_dtm_lat = lto;
                  }
            }


            else if (!strncmp(buffer, "Longitude Offset", 16))
            {
                  wxStringTokenizer tkz(str_buf, _T("="));
                  wxString token = tkz.GetNextToken();
                  if(token.IsSameAs(_T("Longitude Offset"), FALSE))
                  {
                        int i;
                        i = tkz.GetPosition();
                        float lno;
                        sscanf(&buffer[i], "%f,", &lno);
                        m_dtm_lon = lno;
                  }
            }

            else if (!strncmp(buffer, "Datum", 5))
            {
                  wxStringTokenizer tkz(str_buf, _T("="));
                  wxString token = tkz.GetNextToken();
                  if(token.IsSameAs(_T("Datum"), FALSE))
                  {
                        token = tkz.GetNextToken();
                        m_datum_str = token;
                  }
            }


            else if (!strncmp(buffer, "Name", 4))
            {
                wxStringTokenizer tkz(str_buf, _T("="));
              wxString token = tkz.GetNextToken();
              if(token.IsSameAs(_T("Name"), FALSE))                         // Name
              {
                int i;
                i = tkz.GetPosition();
                m_Name.Clear();
                while(isprint(buffer[i]) && (i < 80))
                      m_Name.Append(buffer[i++]);
               }
            }
      }     //while



//          Extract the remaining data from .NOS Bitmap file
      ifs_bitmap = NULL;

//      Something wrong with the .geo file, there is no Bitmap reference
//      This is where the arbitrarily bad file is caught, such as
//      a file with.GEO extension that is not really a chart

      if(pBitmapFilePath == NULL)
      {
            free(pPlyTable);
            return INIT_FAIL_REMOVE;
      }

      wxString NOS_Name(*pBitmapFilePath);            // take a copy

      wxDir target_dir(Path);
      wxArrayString file_array;
      int nfiles = wxDir::GetAllFiles(Path, &file_array);
      int ifile;

      pBitmapFilePath->Prepend(Path);

      wxFileName NOS_filename(*pBitmapFilePath);
      if(NOS_filename.FileExists())
      {
            ifss_bitmap = new wxFFileInputStream(*pBitmapFilePath); // open the bitmap file
            ifs_bitmap = new wxBufferedInputStream(*ifss_bitmap);
      }
//    File as fetched verbatim from the .geo file doesn't exist.
//    Try all possible upper/lower cases
      else
      {
//    Extract the filename and extension
            wxString fname(NOS_filename.GetName());
            wxString fext(NOS_filename.GetExt());

//    Try all four combinations, the hard way
// case 1
            fname.MakeLower();
            fext.MakeLower();
            NOS_filename.SetName(fname);
            NOS_filename.SetExt(fext);

            if(NOS_filename.FileExists())
                  goto found_uclc_file;

// case 2
            fname.MakeLower();
            fext.MakeUpper();
            NOS_filename.SetName(fname);
            NOS_filename.SetExt(fext);

            if(NOS_filename.FileExists())
                  goto found_uclc_file;

// case 3
            fname.MakeUpper();
            fext.MakeLower();
            NOS_filename.SetName(fname);
            NOS_filename.SetExt(fext);

            if(NOS_filename.FileExists())
                  goto found_uclc_file;

// case 4
            fname.MakeUpper();
            fext.MakeUpper();
            NOS_filename.SetName(fname);
            NOS_filename.SetExt(fext);

            if(NOS_filename.FileExists())
                  goto found_uclc_file;


//      Search harder

            for(ifile = 0 ; ifile < nfiles ; ifile++)
            {
                wxString file_up = file_array.Item(ifile);
                file_up.MakeUpper();

                wxString target_up = *pBitmapFilePath;
                target_up.MakeUpper();

                if(file_up.IsSameAs( target_up))
                {
                    NOS_filename.Clear();
                    NOS_filename.Assign(file_array.Item(ifile));
                    goto found_uclc_file;
                }

            }

            free(pPlyTable);
            return INIT_FAIL_REMOVE;                  // not found at all

found_uclc_file:

            delete pBitmapFilePath;                   // fix up the member element
            pBitmapFilePath = new wxString(NOS_filename.GetFullPath());
            ifss_bitmap = new wxFFileInputStream(*pBitmapFilePath); // open the bitmap file
            ifs_bitmap = new wxBufferedInputStream(*ifss_bitmap);

      }           //else


      if(ifs_bitmap == NULL)
      {
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }

      if(!ifss_bitmap->IsOk())
      {
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }


      while( (ReadBSBHdrLine(ifss_bitmap, &buffer[0], BUF_LEN_MAX)) != 0 )
      {
          wxString str_buf(buffer,  wxConvUTF8);

            if(!strncmp(buffer, "NOS", 3))
            {
                wxStringTokenizer tkz(str_buf, _T(",="));
                  while ( tkz.HasMoreTokens() )
                  {
                        wxString token = tkz.GetNextToken();
                        if(token.IsSameAs(_T("RA"), TRUE))                  // extract RA=x,y
                        {
                              int i;
                              tkz.GetNextToken();
                              tkz.GetNextToken();
                              i = tkz.GetPosition();
                              Size_X = atoi(&buffer[i]);
                              wxString token = tkz.GetNextToken();
                              i = tkz.GetPosition();
                              Size_Y = atoi(&buffer[i]);
                        }
                        else if(token.IsSameAs(_T("DU"), TRUE))                  // extract DU=n
                        {
                            token = tkz.GetNextToken();
                            long temp_du;
                            if(token.ToLong(&temp_du))
                                m_Chart_DU = temp_du;
                        }
                  }

            }

            else if (!strncmp(buffer, "RGB", 3))
                  CreatePaletteEntry(buffer, COLOR_RGB_DEFAULT);

            else if (!strncmp(buffer, "DAY", 3))
                  CreatePaletteEntry(buffer, DAY);

            else if (!strncmp(buffer, "DSK", 3))
                  CreatePaletteEntry(buffer, DUSK);

            else if (!strncmp(buffer, "NGT", 3))
                  CreatePaletteEntry(buffer, NIGHT);

            else if (!strncmp(buffer, "NGR", 3))
                  CreatePaletteEntry(buffer, NIGHTRED);

            else if (!strncmp(buffer, "GRY", 3))
                  CreatePaletteEntry(buffer, GRAY);

            else if (!strncmp(buffer, "PRC", 3))
                  CreatePaletteEntry(buffer, PRC);

            else if (!strncmp(buffer, "PRG", 3))
                  CreatePaletteEntry(buffer, PRG);
      }


//    Validate some of the header data
      if((Size_X == 0) || (Size_Y == 0))
      {
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }

      if(nPlypoint < 3)
      {
          wxString msg(_T("   Chart File contains less than 3 PLY points: "));
          msg.Append(m_FullPath);
          wxLogMessage(msg);
          free(pPlyTable);

          return INIT_FAIL_REMOVE;
      }

      if(m_datum_str.IsEmpty()){
          wxString msg(_T("   Chart datum not specified on chart "));
          msg.Append(m_FullPath);
          wxLogMessage(msg);
          wxLogMessage(_T("   Default datum (WGS84) substituted."));
          
          //          return INIT_FAIL_REMOVE;
      }
      else {
          char d_str[100];
          strncpy(d_str, m_datum_str.mb_str(), 99);
          d_str[99] = 0;
          
          int datum_index = GetDatumIndex(d_str);
          
          if(datum_index < 0){
              wxString msg(_T("   Chart datum {"));
              msg += m_datum_str;
              msg += _T("} invalid on chart ");
              msg.Append(m_FullPath);
              wxLogMessage(msg);
              wxLogMessage(_T("   Default datum (WGS84) substituted."));
              
              //          return INIT_FAIL_REMOVE;
              }
          }

//    Convert captured plypoint information into chart COVR structures
      m_nCOVREntries = 1;
      m_pCOVRTablePoints = (int *)malloc(sizeof(int));
      *m_pCOVRTablePoints = nPlypoint;
      m_pCOVRTable = (float **)malloc(sizeof(float *));
      *m_pCOVRTable = (float *)malloc(nPlypoint * 2 * sizeof(float));
      memcpy(*m_pCOVRTable, pPlyTable, nPlypoint * 2 * sizeof(float));

      free(pPlyTable);

      if(!SetMinMax())
            return INIT_FAIL_REMOVE;          // have to bail here

      AnalyzeSkew();
      
      if(init_flags == HEADER_ONLY)
            return INIT_OK;

//    Advance to the data
      char c;
      if((c = ifs_bitmap->GetC()) != 0x1a){ return INIT_FAIL_REMOVE;}
      if((c = ifs_bitmap->GetC()) == 0x0d)
      {
            if((c = ifs_bitmap->GetC()) != 0x0a){  return INIT_FAIL_REMOVE;}
            if((c = ifs_bitmap->GetC()) != 0x1a){  return INIT_FAIL_REMOVE;}
            if((c = ifs_bitmap->GetC()) != 0x00){  return INIT_FAIL_REMOVE;}
      }

      else if(c != 0x00){  return INIT_FAIL_REMOVE;}

//    Read the Color table bit size
      nColorSize = ifs_bitmap->GetC();


//    Perform common post-init actions in ChartBaseBSB
      InitReturn pi_ret = PostInit();
      if( pi_ret  != INIT_OK)
            return pi_ret;
      else
            return INIT_OK;

}




// ============================================================================
// ChartKAP implementation
// ============================================================================


ChartKAP::ChartKAP()
{
     m_ChartType = CHART_TYPE_KAP;
}


ChartKAP::~ChartKAP()
{
}


InitReturn ChartKAP::Init( const wxString& name, ChartInitFlag init_flags )
{
      #define BUF_LEN_MAX 4096

      ifs_hdr = new ChartDataNonSeekableInputStream(name);          // open the Header file as a read-only stream
      
      if(!ifs_hdr->IsOk())
            return INIT_FAIL_REMOVE;

    
      int nPlypoint = 0;
      Plypoint *pPlyTable = (Plypoint *)malloc(sizeof(Plypoint));

      PreInit(name, init_flags, GLOBAL_COLOR_SCHEME_DAY);

      char buffer[BUF_LEN_MAX];


      m_FullPath = name;
      m_Description = m_FullPath;

      //    Clear georeferencing coefficients
      for(int icl=0 ; icl< 12 ; icl++)
      {
          wpx[icl] = 0;
          wpy[icl] = 0;
          pwx[icl] = 0;
          pwy[icl] = 0;
      }


//    Validate the BSB header
//    by reading some characters into a buffer and looking for BSB\ keyword


      unsigned int TestBlockSize = 1999;
      ifs_hdr->Read(buffer, TestBlockSize);

      if(ifs_hdr->LastRead() != TestBlockSize)
      {
          wxString msg;
          msg.Printf(_("   Could not read first %d bytes of header for chart file: "), TestBlockSize);
          msg.Append(name);
          wxLogMessage(msg);
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }

      unsigned int i;
      for( i = 0; i < TestBlockSize - 4; i++ )
      {
        //Test for "BSB/"
            if( buffer[i+0] == 'B' && buffer[i+1] == 'S'
                          && buffer[i+2] == 'B' && buffer[i+3] == '/' )
                  break;

        // Test for "NOS/"
            if( buffer[i+0] == 'N' && buffer[i+1] == 'O'
                          && buffer[i+2] == 'S' && buffer[i+3] == '/' )
                  break;

      }
      if( i == TestBlockSize - 4 )
      {
          wxString msg(_("   Chart file has no BSB header, cannot Init."));
          msg.Append(name);
          wxLogMessage(msg);
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }



//    Read and Parse Chart Header, line by line
      ifs_hdr->SeekI(0, wxFromStart);                                         // rewind

      Size_X = Size_Y = 0;

      int done_header_parse = 0;
      wxCSConv iso_conv(wxT("ISO-8859-1"));                 // we will need a converter

      while(done_header_parse == 0)
      {
            if(ReadBSBHdrLine(ifs_hdr, buffer, BUF_LEN_MAX) == 0)
            {
                  unsigned char c;
                  c = ifs_hdr->GetC();
                  ifs_hdr->Ungetch(c);

                  if(0x1a == c)
                      done_header_parse = 1;
                  else
                  {
                      free(pPlyTable);
                      return INIT_FAIL_REMOVE;
                  }

                  continue;
            }

            wxString str_buf(buffer,  wxConvUTF8);
            if(!str_buf.Len())                                    // failed conversion
                  str_buf = wxString(buffer, iso_conv);

            if(str_buf.Find(_T("SHOM")) != wxNOT_FOUND)
                  m_b_SHOM = true;

            if(!strncmp(buffer, "BSB", 3))
            {
                  wxString clip_str_buf(&buffer[0],  iso_conv);  // for single byte French encodings of NAme field
                  wxStringTokenizer tkz(clip_str_buf, _T("/,="));
                  while ( tkz.HasMoreTokens() )
                  {
                        wxString token = tkz.GetNextToken();
                        if(token.IsSameAs(_T("RA"), TRUE))                  // extract RA=x,y
                        {
                              int i;
                              i = tkz.GetPosition();
                              Size_X = atoi(&buffer[i]);
                              wxString token = tkz.GetNextToken();
                              i = tkz.GetPosition();
                              Size_Y = atoi(&buffer[i]);
                        }
                        else if(token.IsSameAs(_T("NA"), TRUE))                  // extract NA=str
                        {
                          int i = tkz.GetPosition();
                          char nbuf[81];
                          int j=0;
                          while((buffer[i] != ',') && (i < 80))
                                nbuf[j++] = buffer[i++];
                          nbuf[j] = 0;
                          wxString n_str(nbuf,  iso_conv);
                          m_Name = n_str;
                        }
                        else if(token.IsSameAs(_T("NU"), TRUE))                  // extract NU=str
                        {
                              int i = tkz.GetPosition();
                              char nbuf[81];
                              int j=0;
                              while((buffer[i] != ',') && (i < 80))
                                    nbuf[j++] = buffer[i++];
                              nbuf[j] = 0;
                              wxString n_str(nbuf,  iso_conv);
                              m_ID = n_str;
                        }
                        else if(token.IsSameAs(_T("DU"), TRUE))                  // extract DU=n
                        {
                          token = tkz.GetNextToken();
                          long temp_du;
                          if(token.ToLong(&temp_du))
                                m_Chart_DU = temp_du;
                        }

                  }
            }

            else if(!strncmp(buffer, "KNP", 3))
            {
                  wxString conv_buf(buffer,  iso_conv);
                  wxStringTokenizer tkz(conv_buf, _T("/,="));
                  while ( tkz.HasMoreTokens() )
                  {
                        wxString token = tkz.GetNextToken();
                        if(token.IsSameAs(_T("SC"), TRUE))                  // extract Scale
                        {
                              int i;
                              i = tkz.GetPosition();
                              m_Chart_Scale = atoi(&buffer[i]);
                              if(0 == m_Chart_Scale)
                                    m_Chart_Scale = 100000000;
                        }
                        else if(token.IsSameAs(_T("SK"), TRUE))                  // extract Skew
                        {
                              int i;
                              i = tkz.GetPosition();
                              float fcs;
                              sscanf(&buffer[i], "%f,", &fcs);
                              m_Chart_Skew = fcs;
                        }
                        else if(token.IsSameAs(_T("UN"), TRUE))                  // extract Depth Units
                        {
                            int i;
                            i = tkz.GetPosition();
                            wxString str(&buffer[i], iso_conv);
                            m_DepthUnits = str.BeforeFirst(',');
                        }
                        else if(token.IsSameAs(_T("GD"), TRUE))                  // extract Datum
                        {
                              int i;
                              i = tkz.GetPosition();
                              wxString str(&buffer[i], iso_conv);
                              m_datum_str = str.BeforeFirst(',').Trim();
                        }
                        else if(token.IsSameAs(_T("SD"), TRUE))                  // extract Soundings Datum
                        {
                              int i;
                              i = tkz.GetPosition();
                              wxString str(&buffer[i], iso_conv);
                              m_SoundingsDatum = str.BeforeFirst(',').Trim();
                        }
                        else if(token.IsSameAs(_T("PP"), TRUE))                  // extract Projection Parameter
                        {
                              int i;
                              i = tkz.GetPosition();
                              double fcs;
                              wxString str(&buffer[i], iso_conv);
                              wxString str1 = str.BeforeFirst(',').Trim();
                              if(str1.ToDouble(&fcs))
                                    m_proj_parameter = fcs;
                        }
                        else if(token.IsSameAs(_T("PR"), TRUE))                  // extract Projection Type
                        {
                              int i;
                              i = tkz.GetPosition();
                              wxString str(&buffer[i], iso_conv);
                              wxString stru = str.MakeUpper();
                              bool bp_set = false;;

                              if(stru.Matches(_T("*MERCATOR*")))
                              {
                                    m_projection = PROJECTION_MERCATOR;
                                    bp_set = true;
                              }

                              if(stru.Matches(_T("*TRANSVERSE*")))
                              {
                                    m_projection = PROJECTION_TRANSVERSE_MERCATOR;
                                    bp_set = true;
                              }

                              if(stru.Matches(_T("*POLYCONIC*")))
                              {
                                    m_projection = PROJECTION_POLYCONIC;
                                    bp_set = true;
                              }

                              if(stru.Matches(_T("*TM*")))
                              {
                                    m_projection = PROJECTION_TRANSVERSE_MERCATOR;
                                    bp_set = true;
                              }

                              if(stru.Matches(_T("*GAUSS CONFORMAL*")))
                              {
                                    m_projection = PROJECTION_TRANSVERSE_MERCATOR;
                                    bp_set = true;
                              }

                              if(!bp_set)
                              {
                                  m_projection = PROJECTION_UNKNOWN;
                                  wxString msg(_T("   Chart projection is "));
                                  msg += tkz.GetNextToken();
                                  msg += _T(" which is unsupported.  Disabling chart ");
                                  msg += m_FullPath;
                                  wxLogMessage(msg);

                                  return INIT_FAIL_REMOVE;
                              }
                        }
                        else if(token.IsSameAs(_T("DX"), TRUE))                  // extract Pixel scale parameter, if present
                        {
                              int i;
                              i = tkz.GetPosition();
                              float x;
                              sscanf(&buffer[i], "%f,", &x);
                              m_dx = x;
                        }
                        else if(token.IsSameAs(_T("DY"), TRUE))                  // extract Pixel scale parameter, if present
                        {
                              int i;
                              i = tkz.GetPosition();
                              float x;
                              sscanf(&buffer[i], "%f,", &x);
                              m_dy = x;
                        }


                 }
            }


            else if (!strncmp(buffer, "RGB", 3))
                  CreatePaletteEntry(buffer, COLOR_RGB_DEFAULT);

            else if (!strncmp(buffer, "DAY", 3))
                  CreatePaletteEntry(buffer, DAY);

            else if (!strncmp(buffer, "DSK", 3))
                  CreatePaletteEntry(buffer, DUSK);

            else if (!strncmp(buffer, "NGT", 3))
                  CreatePaletteEntry(buffer, NIGHT);

            else if (!strncmp(buffer, "NGR", 3))
                  CreatePaletteEntry(buffer, NIGHTRED);

            else if (!strncmp(buffer, "GRY", 3))
                  CreatePaletteEntry(buffer, GRAY);

            else if (!strncmp(buffer, "PRC", 3))
                  CreatePaletteEntry(buffer, PRC);

            else if (!strncmp(buffer, "PRG", 3))
                  CreatePaletteEntry(buffer, PRG);


            else if (!strncmp(buffer, "REF", 3))
            {
                  int i, xr, yr;
                  float ltr,lnr;
                  sscanf(&buffer[4], "%d,%d,%d,%f,%f", &i, &xr, &yr, &ltr, &lnr);
                  pRefTable = (Refpoint *)realloc(pRefTable, sizeof(Refpoint) * (nRefpoint+1));
                  pRefTable[nRefpoint].xr = xr;
                  pRefTable[nRefpoint].yr = yr;
                  pRefTable[nRefpoint].latr = ltr;
                  pRefTable[nRefpoint].lonr = lnr;
                  pRefTable[nRefpoint].bXValid = 1;
                  pRefTable[nRefpoint].bYValid = 1;

                  nRefpoint++;

            }

            else if (!strncmp(buffer, "WPX", 3))
            {
                int idx = 0;
                double d;
                wxStringTokenizer tkz(str_buf.Mid(4), _T(","));
                wxString token = tkz.GetNextToken();

                if(token.ToLong((long int *)&wpx_type))
                {
                    while ( tkz.HasMoreTokens() && (idx < 12) )
                    {
                        token = tkz.GetNextToken();
                        if(token.ToDouble(&d))
                        {
                            wpx[idx] = d;
                            idx++;
                        }
                    }
                }
                n_wpx = idx;
            }

            else if (!strncmp(buffer, "WPY", 3))
            {
                int idx = 0;
                double d;
                wxStringTokenizer tkz(str_buf.Mid(4), _T(","));
                wxString token = tkz.GetNextToken();

                if(token.ToLong((long int *)&wpy_type))
                {
                    while ( tkz.HasMoreTokens() && (idx < 12) )
                    {
                        token = tkz.GetNextToken();
                        if(token.ToDouble(&d))
                        {
                            wpy[idx] = d;
                            idx++;
                        }
                    }
                }
                n_wpy = idx;
            }

            else if (!strncmp(buffer, "PWX", 3))
            {
                int idx = 0;
                double d;
                wxStringTokenizer tkz(str_buf.Mid(4), _T(","));
                wxString token = tkz.GetNextToken();

                if(token.ToLong((long int *)&pwx_type))
                {
                    while ( tkz.HasMoreTokens() && (idx < 12) )
                    {
                        token = tkz.GetNextToken();
                        if(token.ToDouble(&d))
                        {
                            pwx[idx] = d;
                            idx++;
                        }
                    }
                }
                n_pwx = idx;
            }

            else if (!strncmp(buffer, "PWY", 3))
            {
                int idx = 0;
                double d;
                wxStringTokenizer tkz(str_buf.Mid(4), _T(","));
                wxString token = tkz.GetNextToken();

                if(token.ToLong((long int *)&pwy_type))
                {
                    while ( tkz.HasMoreTokens() && (idx < 12) )
                    {
                        token = tkz.GetNextToken();
                        if(token.ToDouble(&d))
                        {
                            pwy[idx] = d;
                            idx++;
                        }
                    }
                }
                n_pwy = idx;
            }


            else if (!strncmp(buffer, "CPH", 3))
            {
                float float_cph;
                sscanf(&buffer[4], "%f", &float_cph);
                m_cph = float_cph;
            }

            else if (!strncmp(buffer, "VER", 3))
            {
                  wxStringTokenizer tkz(str_buf, _T("/,="));
                  wxString token = tkz.GetNextToken();

                  m_bsb_ver = tkz.GetNextToken();
            }

            else if (!strncmp(buffer, "DTM", 3))
            {
                  double val;
                  wxStringTokenizer tkz(str_buf, _T("/,="));
                  wxString token = tkz.GetNextToken();

                  token = tkz.GetNextToken();
                  if(token.ToDouble(&val))
                        m_dtm_lat = val;

                  token = tkz.GetNextToken();
                  if(token.ToDouble(&val))
                        m_dtm_lon = val;


//                  float fdtmlat, fdtmlon;
//                  sscanf(&buffer[4], "%f,%f", &fdtmlat, &fdtmlon);
//                  m_dtm_lat = fdtmlat;
//                  m_dtm_lon = fdtmlon;
            }


            else if (!strncmp(buffer, "PLY", 3))
            {
                  int i;
                  float ltp,lnp;
                  sscanf(&buffer[4], "%d,%f,%f", &i, &ltp, &lnp);
                  Plypoint *tmp = pPlyTable;
                  pPlyTable = (Plypoint *)realloc(pPlyTable, sizeof(Plypoint) * (nPlypoint+1));
                  if (NULL == pPlyTable)
                  {
                      free(tmp);
                      tmp = NULL;
                  } else
                  {
                      pPlyTable[nPlypoint].ltp = ltp;
                      pPlyTable[nPlypoint].lnp = lnp;
                      nPlypoint++;
                  }
            }

            else if(!strncmp(buffer, "CED", 3))
            {
                wxStringTokenizer tkz(str_buf, _T("/,="));
                  while ( tkz.HasMoreTokens() )
                  {
                        wxString token = tkz.GetNextToken();
                        if(token.IsSameAs(_T("ED"), TRUE))                  // extract Edition Date
                        {

                              int i;
                              i = tkz.GetPosition();

                              char date_string[40];
                              char date_buf[10];
                              sscanf(&buffer[i], "%s\r\n", date_string);
                              wxString date_wxstr(date_string,  wxConvUTF8);

                              wxDateTime dt;
                              if(dt.ParseDate(date_wxstr))       // successful parse?
                              {
                                  int iyear = dt.GetYear(); // GetYear() fails on W98, DMC compiler, wx2.8.3
                                  //    BSB charts typically list publish date as xx/yy/zz
                                  //  This our own little version of the Y2K problem.
                                  //  Just apply some sensible logic
                                  
                                  if(iyear < 50){
                                      iyear += 2000;
                                      dt.SetYear(iyear);
                                  }
                                  else if((iyear >= 50) && (iyear < 100)){
                                      iyear += 1900;
                                      dt.SetYear(iyear);
                                  }
                                  sprintf(date_buf, "%d", iyear);

                              //    Initialize the wxDateTime menber for Edition Date
                                  m_EdDate = dt;
                              }
                              else
                              {
                                 sscanf(date_string, "%s", date_buf);
                                 m_EdDate.Set(1, wxDateTime::Jan, 2000);                    //Todo this could be smarter
                              }

                              m_PubYear = wxString(date_buf,  wxConvUTF8);
                        }
                        else if(token.IsSameAs(_T("SE"), TRUE))                  // extract Source Edition
                        {
                              int i;
                              i = tkz.GetPosition();
                              wxString str(&buffer[i], iso_conv);
                              m_SE = str.BeforeFirst(',');
                        }

                  }
            }

      }

      //    Some charts improperly encode the DTM parameters.
      //    Identify them as necessary, for further processing
      if(m_b_SHOM && (m_bsb_ver == _T("1.1")))
            m_b_apply_dtm = false;

      //    If imbedded coefficients are found,
      //    then use the polynomial georeferencing algorithms
      if(n_pwx && n_pwy && n_pwx && n_pwy)
          bHaveEmbeddedGeoref = true;


      //    Set up the projection point according to the projection parameter
      if(m_projection == PROJECTION_MERCATOR)
            m_proj_lat = m_proj_parameter;
      else if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
            m_proj_lon = m_proj_parameter;
      else if(m_projection == PROJECTION_POLYCONIC)
            m_proj_lon = m_proj_parameter;

      //    We have seen improperly coded charts, with non-sense value of PP parameter
      //    FS#1251      
      //    Check and override if necessary      
      if(m_proj_lat >82.0 || m_proj_lat < -82.0)
        m_proj_lat = 0.0;
            

//    Validate some of the header data
      if((Size_X == 0) || (Size_Y == 0))
      {
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }

      if(nPlypoint < 3)
      {
            wxString msg(_("   Chart File contains less than 3 PLY points: "));
            msg.Append(m_FullPath);
            wxLogMessage(msg);
            free(pPlyTable);
            return INIT_FAIL_REMOVE;
      }

      if(m_datum_str.IsEmpty()){
          wxString msg(_T("   Chart datum not specified on chart "));
          msg.Append(m_FullPath);
          wxLogMessage(msg);
          wxLogMessage(_T("   Default datum (WGS84) substituted."));
          
//          return INIT_FAIL_REMOVE;
      }
      else {
        char d_str[100];
        strncpy(d_str, m_datum_str.mb_str(), 99);
        d_str[99] = 0;
        
        int datum_index = GetDatumIndex(d_str);
        
        if(datum_index < 0){
            wxString msg(_T("   Chart datum {"));
            msg += m_datum_str;
            msg += _T("} invalid on chart ");
            msg.Append(m_FullPath);
            wxLogMessage(msg);
            wxLogMessage(_T("   Default datum (WGS84) substituted."));
            
    //          return INIT_FAIL_REMOVE;
        }
      }

      /* Augment ply points
           This is needed for example on polyconic charts or skewed charts because
           straight lines in the chart coordinates can not use simple
           interpolation in lat/lon or mercator coordinate space to draw the
           borders or be used for quilting operation.
           TODO: should this be added as a subroutine for GEO chartso? */
      if((m_projection != PROJECTION_MERCATOR && m_projection != PROJECTION_TRANSVERSE_MERCATOR)
          || m_Chart_Skew > 2) {
          //   Analyze Refpoints early because we need georef coefficient here.
          AnalyzeRefpoints( false );              // no post test needed
     
          //  We need to compute a tentative min/max lat/lon to perform georefs
          //  These lat/lon extents will be more accurately updated later.
          m_LonMax = -360.0;
          m_LonMin = 360.0;
          m_LatMax = -90.0;
          m_LatMin = 90.0;
      
          for(int i=0 ; i < nPlypoint ; i++){
              m_LatMax = wxMax(m_LatMax, pPlyTable[i].ltp);
              m_LatMin = wxMin(m_LatMin, pPlyTable[i].ltp);
              m_LonMax = wxMax(m_LonMax, pPlyTable[i].lnp);
              m_LonMin = wxMin(m_LonMin, pPlyTable[i].lnp);
          }
          
          int count = nPlypoint;
          nPlypoint = 0;
          Plypoint *pOldPlyTable = pPlyTable;
          pPlyTable = NULL;
          double lastplylat = 0.0, lastplylon = 0.0, x1 = 0.0, y1 = 0.0, x2, y2;
          double plylat, plylon;
          for( int i = 0; i < count+1; i++ ) {
              plylat = pOldPlyTable[i%count].ltp;
              plylon = pOldPlyTable[i%count].lnp;
              latlong_to_chartpix(plylat, plylon, x2, y2);
              if( i > 0 ) {
                  if( lastplylon - plylon > 180. )
                      lastplylon -= 360.;
                  else if( lastplylon - plylon < -180. )
                      lastplylon += 360.;

                  // use 2 degree steps
                  double steps = ceil( (fabs(lastplylat-plylat) + fabs(lastplylon-plylon)) / 2 );
                  for( double c = 0; c < steps; c++ ) {
                      double d = c/steps, lat, lon;
                      wxPoint2DDouble s;
                      double x = (1-d)*x1 + d*x2, y = (1-d)*y1 + d*y2;
                      chartpix_to_latlong(x, y, &lat, &lon);
                      pPlyTable = (Plypoint *)realloc(pPlyTable, sizeof(Plypoint) * (nPlypoint+1));
                      pPlyTable[nPlypoint].ltp = lat;
                      pPlyTable[nPlypoint].lnp = lon;
                      nPlypoint++;
                  }
              }
              x1 = x2, y1 = y2;
              lastplylat = plylat, lastplylon = plylon;
          }
          free(pOldPlyTable);
      }

//    Convert captured plypoint information into chart COVR structures
      m_nCOVREntries = 1;
      m_pCOVRTablePoints = (int *)malloc(sizeof(int));
      *m_pCOVRTablePoints = nPlypoint;
      m_pCOVRTable = (float **)malloc(sizeof(float *));
      *m_pCOVRTable = (float *)malloc(nPlypoint * 2 * sizeof(float));
      memcpy(*m_pCOVRTable, pPlyTable, nPlypoint * 2 * sizeof(float));
      free(pPlyTable);


      //    Setup the datum transform parameters
      char d_str[100];
      strncpy(d_str, m_datum_str.mb_str(), 99);
      d_str[99] = 0;
      
      int datum_index = GetDatumIndex(d_str);
      m_datum_index = datum_index; 
      
      if(datum_index < 0)
          m_ExtraInfo = _("---<<< Warning:  Chart Datum may be incorrect. >>>---");
 
      //    Establish defaults, may be overridden later
      m_lon_datum_adjust = (-m_dtm_lon) / 3600.;
      m_lat_datum_adjust = (-m_dtm_lat) / 3600.;
          
      //    Adjust the PLY points to WGS84 datum
      Plypoint *ppp = (Plypoint *)GetCOVRTableHead(0);
      int cnPlypoint = GetCOVRTablenPoints(0);


      for(int u=0 ; u<cnPlypoint ; u++)
      {
          double dlat = 0;
          double dlon = 0;
          
          if(m_datum_index == DATUM_INDEX_WGS84 || m_datum_index == DATUM_INDEX_UNKNOWN)
          {
              dlon = m_dtm_lon / 3600.;
              dlat = m_dtm_lat / 3600.;
          }
          
          
          else{
            double to_lat, to_lon;
            MolodenskyTransform (ppp->ltp, ppp->lnp, &to_lat, &to_lon, m_datum_index, DATUM_INDEX_WGS84);
            dlon = (to_lon - ppp->lnp);
            dlat = (to_lat - ppp->ltp);
            if(m_b_apply_dtm)
            {
                dlon += m_dtm_lon / 3600.;
                dlat += m_dtm_lat / 3600.;
            }
          }
          
          ppp->lnp += dlon;
          ppp->ltp += dlat;
          ppp++;
      }


      if(!SetMinMax())
            return INIT_FAIL_REMOVE;          // have to bail here

      AnalyzeSkew();
      
      if(init_flags == HEADER_ONLY)
            return INIT_OK;

//    Advance to the data
      unsigned char c;
      bool bcorrupt = false;

      if((c = ifs_hdr->GetC()) != 0x1a){ bcorrupt = true; }
      if((c = ifs_hdr->GetC()) == 0x0d)
      {
            if((c = ifs_hdr->GetC()) != 0x0a){ bcorrupt = true; }
            if((c = ifs_hdr->GetC()) != 0x1a){ bcorrupt = true; }
            if((c = ifs_hdr->GetC()) != 0x00){ bcorrupt = true; }
      }

      else if(c != 0x00){ bcorrupt = true; }


      if(bcorrupt)
      {
            wxString msg(_("   Chart File RLL data corrupt on chart "));
            msg.Append(m_FullPath);
            wxLogMessage(msg);

            return INIT_FAIL_REMOVE;
      }


//    Read the Color table bit size
      nColorSize = ifs_hdr->GetC();

      nFileOffsetDataStart = ifs_hdr->TellI();
      delete ifs_hdr;
      ifs_hdr = NULL;

      
      ChartDataInputStream *stream = new ChartDataInputStream(name); // Open again, as the bitmap
      wxString tempfile;
#ifdef USE_LZMA      
      tempfile = stream->TempFileName();
#endif
      m_filesize = wxFileName::GetSize( tempfile.empty() ? name : tempfile );

      ifss_bitmap = stream;
      ifs_bitmap = new wxBufferedInputStream(*ifss_bitmap);


//    Perform common post-init actions in ChartBaseBSB
      InitReturn pi_ret = PostInit();
      if( pi_ret  != INIT_OK)
            return pi_ret;
      else
            return INIT_OK;
}



// ============================================================================
// ChartBaseBSB implementation
// ============================================================================


ChartBaseBSB::ChartBaseBSB()
{
      //    Init some private data
      m_ChartFamily = CHART_FAMILY_RASTER;

      pBitmapFilePath = NULL;

      pline_table = NULL;
      ifs_buf = NULL;

      cached_image_ok = 0;

      pRefTable = (Refpoint *)malloc(sizeof(Refpoint));
      nRefpoint = 0;
      cPoints.status = 0;
      bHaveEmbeddedGeoref = false;
      n_wpx = 0;
      n_wpy = 0;
      n_pwx = 0;
      n_pwy = 0;

#ifdef __OCPN__ANDROID__
      bUseLineCache = false;
#else
      bUseLineCache = true;
#endif

      m_Chart_Skew = 0.0;

      pPixCache = NULL;

      pLineCache = NULL;

      m_bilinear_limit = 8;         // bilinear scaling only up to n

      ifs_bitmap = NULL;
      ifss_bitmap = NULL;
      ifs_hdr = NULL;

      for(int i = 0 ; i < N_BSB_COLORS ; i++)
            pPalettes[i] = NULL;

      bGeoErrorSent = false;
      m_Chart_DU = 0;
      m_cph = 0.;

      m_mapped_color_index = COLOR_RGB_DEFAULT;

      m_datum_str = _T("WGS84");                // assume until proven otherwise

      m_dtm_lat = 0.;
      m_dtm_lon = 0.;

      m_dx = 0.;
      m_dy = 0.;
      m_proj_lat = 0.;
      m_proj_lon = 0.;
      m_proj_parameter = 0.;
      m_b_SHOM = false;
      m_b_apply_dtm = true;

      m_b_cdebug = 0;

#ifdef OCPN_USE_CONFIG
      wxFileConfig *pfc = (wxFileConfig *)pConfig;
      pfc->SetPath ( _T ( "/Settings" ) );
      pfc->Read ( _T ( "DebugBSBImg" ),  &m_b_cdebug, 0 );
#endif

}

ChartBaseBSB::~ChartBaseBSB()
{

      if(pBitmapFilePath)
            delete pBitmapFilePath;

      if(pline_table)
            free(pline_table);

      if(ifs_buf)
            free(ifs_buf);

      free(pRefTable);
//      free(pPlyTable);

      delete ifs_bitmap;
      delete ifs_hdr;
      delete ifss_bitmap;

      if(cPoints.status)
      {
          free(cPoints.tx );
          free(cPoints.ty );
          free(cPoints.lon );
          free(cPoints.lat );

          free(cPoints.pwx );
          free(cPoints.wpx );
          free(cPoints.pwy );
          free(cPoints.wpy );
      }

//    Free the line cache
      FreeLineCacheRows();
      free (pLineCache);

      delete pPixCache;


      for(int i = 0 ; i < N_BSB_COLORS ; i++)
            delete pPalettes[i];

}

void ChartBaseBSB::FreeLineCacheRows(int start, int end)
{
    if(pLineCache)
    {
        if(end < 0)
            end = Size_Y;
        else
            end = wxMin(end, Size_Y);
        for(int ylc = start ; ylc < end ; ylc++) {
            CachedLine *pt = &pLineCache[ylc];
            if(pt->bValid) {
                free (pt->pTileOffset);
                free (pt->pPix);
                pt->bValid = false;
            }
        }
    }
}

bool ChartBaseBSB::HaveLineCacheRow(int row)
{
    if(pLineCache)
    {
        CachedLine *pt = &pLineCache[row];
        return pt->bValid;
    }
    return false;
}


//    Report recommended minimum and maximum scale values for which use of this chart is valid

double ChartBaseBSB::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom)
{
//      if(b_allow_overzoom)
            return (canvas_scale_factor / m_ppm_avg) / 32;         // allow wide range overzoom overscale
//      else
//            return (canvas_scale_factor / m_ppm_avg) / 2;         // don't suggest too much overscale

}

double ChartBaseBSB::GetNormalScaleMax(double canvas_scale_factor, int canvas_width)
{
      return (canvas_scale_factor / m_ppm_avg) * 4.0;        // excessive underscale is slow, and unreadable
}


double ChartBaseBSB::GetNearestPreferredScalePPM(double target_scale_ppm)
{
      return GetClosestValidNaturalScalePPM(target_scale_ppm, .01, 64.);            // changed from 32 to 64 to allow super small
                                                                                    // scale BSB charts as quilt base
}



double ChartBaseBSB::GetClosestValidNaturalScalePPM(double target_scale, double scale_factor_min, double scale_factor_max)
{
      double chart_1x_scale = GetPPM();

      double binary_scale_factor = 1.;



      //    Overzoom....
      if(chart_1x_scale > target_scale)
      {
            double binary_scale_factor_max = 1 / scale_factor_min;

            while(binary_scale_factor < binary_scale_factor_max)
            {
                  if(fabs((chart_1x_scale / binary_scale_factor ) - target_scale) < (target_scale * 0.05))
                        break;
                  if((chart_1x_scale / binary_scale_factor ) < target_scale)
                        break;
                  else
                        binary_scale_factor *= 2.;
            }
      }


      //    Underzoom.....
      else
      {
            int ibsf = 1;
            int isf_max = (int)scale_factor_max;
            while(ibsf < isf_max)
            {
                  if(fabs((chart_1x_scale * ibsf ) - target_scale) < (target_scale * 0.05))
                        break;

                  else if((chart_1x_scale * ibsf ) > target_scale)
                  {
                        if(ibsf > 1)
                              ibsf /= 2;
                        break;
                  }
                  else
                        ibsf *= 2;
            }

            binary_scale_factor = 1. / ibsf;
      }

      return  chart_1x_scale / binary_scale_factor;
}



InitReturn ChartBaseBSB::Init( const wxString& name, ChartInitFlag init_flags )
{
      m_global_color_scheme = GLOBAL_COLOR_SCHEME_RGB;
      return INIT_OK;
}

InitReturn ChartBaseBSB::PreInit( const wxString& name, ChartInitFlag init_flags, ColorScheme cs )
{
      m_global_color_scheme = cs;
      return INIT_OK;
}

void ChartBaseBSB::CreatePaletteEntry(char *buffer, int palette_index)
{
    if(palette_index < N_BSB_COLORS)
    {
      if(!pPalettes[palette_index])
            pPalettes[palette_index] = new opncpnPalette;
      opncpnPalette *pp = pPalettes[palette_index];

      pp->FwdPalette = (int *)realloc(pp->FwdPalette, (pp->nFwd + 1) * sizeof(int));
      pp->RevPalette = (int *)realloc(pp->RevPalette, (pp->nRev + 1) * sizeof(int));
      pp->nFwd++;
      pp->nRev++;

      int i;
      int n,r,g,b;
      sscanf(&buffer[4], "%d,%d,%d,%d", &n, &r, &g, &b);

      i=n;

      int fcolor, rcolor;
      fcolor = (b << 16) + (g << 8) + r;
      rcolor = (r << 16) + (g << 8) + b;

      pp->RevPalette[i] = rcolor;
      pp->FwdPalette[i] = fcolor;
    }
}



InitReturn ChartBaseBSB::PostInit(void)
{
     //    Validate the palette array, substituting DEFAULT for missing entries
     int nfwd_def = 1;
     int nrev_def = 1;
     if(pPalettes[COLOR_RGB_DEFAULT]){
         nrev_def = pPalettes[COLOR_RGB_DEFAULT]->nRev;
         nfwd_def = pPalettes[COLOR_RGB_DEFAULT]->nFwd;
     }
         
      for(int i = 0 ; i < N_BSB_COLORS ; i++)
      {
            if(pPalettes[i] == NULL)
            {
                opncpnPalette *pNullSubPal = new opncpnPalette;

                pNullSubPal->nFwd = nfwd_def;        // copy the palette count
                pNullSubPal->nRev = nrev_def;        // copy the palette count
                //  Deep copy the palette rgb tables
                free( pNullSubPal->FwdPalette );
                pNullSubPal->FwdPalette = (int *)malloc(pNullSubPal->nFwd * sizeof(int));
                if( pPalettes[COLOR_RGB_DEFAULT] )
                    memcpy(pNullSubPal->FwdPalette, pPalettes[COLOR_RGB_DEFAULT]->FwdPalette, pNullSubPal->nFwd * sizeof(int));

                free( pNullSubPal->RevPalette );
                pNullSubPal->RevPalette = (int *)malloc(pNullSubPal->nRev * sizeof(int));
                if( pPalettes[COLOR_RGB_DEFAULT] )
                    memcpy(pNullSubPal->RevPalette, pPalettes[COLOR_RGB_DEFAULT]->RevPalette, pNullSubPal->nRev * sizeof(int));

                pPalettes[i] = pNullSubPal;
            }
      }

      // Establish the palette type and default palette
      palette_direction = GetPaletteDir();

      SetColorScheme(m_global_color_scheme, false);

      //    Allocate memory for ifs file buffering
      ifs_bufsize = Size_X * 4;
      ifs_buf = (unsigned char *)malloc(ifs_bufsize);
      if(!ifs_buf)
            return INIT_FAIL_REMOVE;

      ifs_bufend = ifs_buf + ifs_bufsize;
      ifs_lp = ifs_bufend;
      ifs_file_offset = -ifs_bufsize;


      //    Create and load the line offset index table
      pline_table = NULL;
      pline_table = (int *)malloc((Size_Y+1) * sizeof(int) );               //Ugly....
      if(!pline_table)
            return INIT_FAIL_REMOVE;

      ifs_bitmap->SeekI((Size_Y+1) * -4, wxFromEnd);                 // go to Beginning of offset table
      pline_table[Size_Y] = ifs_bitmap->TellI();                     // fill in useful last table entry

      unsigned char *tmp = (unsigned char*)malloc(Size_Y * sizeof(int));
      ifs_bitmap->Read(tmp, Size_Y * sizeof(int));
      if ( ifs_bitmap->LastRead() != Size_Y * sizeof(int)) {
             wxString msg(_("   Chart File corrupt in PostInit() on chart "));
             msg.Append(m_FullPath);
             wxLogMessage(msg);
             free(tmp);
              
             return INIT_FAIL_REMOVE;
      }

      int offset;
      unsigned char *b = tmp;
      for(int ifplt=0 ; ifplt<Size_Y ; ifplt++)
      {
          offset = 0;
          offset += *b++ * 256 * 256 * 256;
          offset += *b++ * 256 * 256 ;
          offset += *b++ * 256 ;
          offset += *b++;

          pline_table[ifplt] = offset;
      }
      free(tmp);
      //    Try to validate the line index

      bool bline_index_ok = true;
      m_nLineOffset = 0;

      wxULongLong bitmap_filesize = m_filesize;
      if( (m_ChartType == CHART_TYPE_GEO) && pBitmapFilePath )
          bitmap_filesize = wxFileName::GetSize( *pBitmapFilePath );
      
      //  look logically at the line offset table 
      for(int iplt=0 ; iplt< Size_Y - 1 ; iplt++)
      {
          if( pline_table[iplt] > bitmap_filesize )
          {
              wxString msg(_("   Chart File corrupt in PostInit() on chart "));
              msg.Append(m_FullPath);
              wxLogMessage(msg);
              
              return INIT_FAIL_REMOVE;
          }
          
          int thisline_size = pline_table[iplt+1] - pline_table[iplt] ;
          if(thisline_size < 0)
          {
              wxString msg(_("   Chart File corrupt in PostInit() on chart "));
              msg.Append(m_FullPath);
              wxLogMessage(msg);
              
              return INIT_FAIL_REMOVE;
          }
      }

      
      //  For older charts, say Version 1.x, we will try to read the chart and check the lines for coherence
      //  These older charts are more likely to have index troubles....
      //  We only need to check a few lines.  Errors are quickly apparent.
      double ver;
      m_bsb_ver.ToDouble(&ver);
      if( ver < 2.0){
        for(int iplt=0 ; iplt< 10 ; iplt++)
        {
            if( wxInvalidOffset == ifs_bitmap->SeekI(pline_table[iplt], wxFromStart))
            {
                wxString msg(_("   Chart File corrupt in PostInit() on chart "));
                msg.Append(m_FullPath);
                wxLogMessage(msg);
                
                return INIT_FAIL_REMOVE;
            }
            
            int thisline_size = pline_table[iplt+1] - pline_table[iplt] ;
            ifs_bitmap->Read(ifs_buf, thisline_size);
                
            unsigned char *lp = ifs_buf;
                
            unsigned char byNext;
            int nLineMarker = 0;
            do
            {
                byNext = *lp++;
                nLineMarker = nLineMarker * 128 + (byNext & 0x7f);
            } while( (byNext & 0x80) != 0 );
                
                
            //  Linemarker Correction factor needed here
            //  Some charts start with LineMarker = 0, some with LineMarker = 1
            //  Assume the first LineMarker found is the index base, and use
            //  as a correction offset
                
            if(iplt == 0)
                m_nLineOffset = nLineMarker;
                
            if(nLineMarker != iplt + m_nLineOffset)
            {
                bline_index_ok = false;
                break;
            }
        }
      }
      
        // Recreate the scan line index if the embedded version seems corrupt
      if(!bline_index_ok)
      {
          wxString msg(_("   Line Index corrupt, recreating Index for chart "));
          msg.Append(m_FullPath);
          wxLogMessage(msg);
          if(!CreateLineIndex())
          {
                wxString msg(_("   Error creating Line Index for chart "));
                msg.Append(m_FullPath);
                wxLogMessage(msg);
                return INIT_FAIL_REMOVE;
          }
      }



      //    Allocate the Line Cache
      if(bUseLineCache)
      {
            pLineCache = (CachedLine *)malloc(Size_Y * sizeof(CachedLine));
            CachedLine *pt;

            for(int ylc = 0 ; ylc < Size_Y ; ylc++)
            {
                  pt = &pLineCache[ylc];
                  pt->bValid = false;
                  pt->pPix = NULL;        //(unsigned char *)malloc(1);
                  pt->pTileOffset = NULL;
            }
      }
      else
            pLineCache = NULL;


      //    Validate/Set Depth Unit Type
      wxString test_str = m_DepthUnits.Upper();
      if(test_str.IsSameAs(_T("FEET"), FALSE))
          m_depth_unit_id = DEPTH_UNIT_FEET;
      else if(test_str.IsSameAs(_T("METERS"), FALSE))
          m_depth_unit_id = DEPTH_UNIT_METERS;
      else if(test_str.IsSameAs(_T("METRES"), FALSE))                  // Special case for alternate spelling
          m_depth_unit_id = DEPTH_UNIT_METERS;
      else if(test_str.IsSameAs(_T("METRIC"), FALSE))
          m_depth_unit_id = DEPTH_UNIT_METERS;
      else if(test_str.IsSameAs(_T("FATHOMS"), FALSE))
          m_depth_unit_id = DEPTH_UNIT_FATHOMS;
      else if(test_str.Find(_T("FATHOMS")) != wxNOT_FOUND)             // Special case for "Fathoms and Feet"
          m_depth_unit_id = DEPTH_UNIT_FATHOMS;
      else if(test_str.Find(_T("METERS")) != wxNOT_FOUND)             // Special case for "Meters and decimeters"
            m_depth_unit_id = DEPTH_UNIT_METERS;

           
      //   Analyze Refpoints
      int analyze_ret_val = AnalyzeRefpoints();
      if(0 != analyze_ret_val)
            return INIT_FAIL_REMOVE;


      bReadyToRender = true;
      return INIT_OK;
}


bool ChartBaseBSB::CreateLineIndex()
{
    //  Assumes file stream ifs_bitmap is currently open

//    wxBufferedInputStream *pbis = new wxBufferedInputStream(*ifss_bitmap);

    //  Seek to start of data
    ifs_bitmap->SeekI(nFileOffsetDataStart);                 // go to Beginning of data

    for(int iplt=0 ; iplt<Size_Y ; iplt++)
    {
        int offset = ifs_bitmap->TellI();

        int iscan;
        iscan = BSBScanScanline(ifs_bitmap);

        //  There is no sense reporting an error here, since we are recreating after an error
/*
        if(iscan > Size_Y)
        {

            wxString msg(_("CreateLineIndex() failed on chart "));
            msg.Append(m_FullPath);
            wxLogMessage(msg);
           return false;
        }

        //  Skipped lines?
        if(iscan != iplt)
        {
            while((iplt < iscan) && (iplt < Size_Y))
            {
                pline_table[iplt] = 0;
                iplt++;
            }
        }
*/
        pline_table[iplt] = offset;

    }

    return true;
}


//    Invalidate and Free the line cache contents
void ChartBaseBSB::InvalidateLineCache(void)
{
      if(pLineCache)
      {
            CachedLine *pt;
            for(int ylc = 0 ; ylc < Size_Y ; ylc++)
            {
                  pt = &pLineCache[ylc];
                  if(pt)
                  {
                      free (pt->pPix);
                      pt->pPix = NULL;
                      free (pt->pTileOffset);
                      pt->pTileOffset = NULL;
                      pt->bValid = false;
                  }
            }
      }
}

bool ChartBaseBSB::GetChartExtent(Extent *pext)
{
      pext->NLAT = m_LatMax;
      pext->SLAT = m_LatMin;
      pext->ELON = m_LonMax;
      pext->WLON = m_LonMin;

      return true;
}


bool ChartBaseBSB::SetMinMax(void)
{
      //    Calculate the Chart Extents(M_LatMin, M_LonMin, etc.)
      //     from the COVR data, for fast database search
      m_LonMax = -360.0;
      m_LonMin = 360.0;
      m_LatMax = -90.0;
      m_LatMin = 90.0;

      Plypoint *ppp = (Plypoint *)GetCOVRTableHead(0);
      int cnPlypoint = GetCOVRTablenPoints(0);

      for(int u=0 ; u<cnPlypoint ; u++)
      {
            if(ppp->lnp > m_LonMax)
                  m_LonMax = ppp->lnp;
            if(ppp->lnp < m_LonMin)
                  m_LonMin = ppp->lnp;

            if(ppp->ltp > m_LatMax)
                  m_LatMax = ppp->ltp;
            if(ppp->ltp < m_LatMin)
                  m_LatMin = ppp->ltp;

            ppp++;
      }

      //    Check for special cases

      //    Case 1:  Chart spans International Date Line or Greenwich, Longitude min/max is non-obvious.
      if((m_LonMax * m_LonMin) < 0)              // min/max are opposite signs
      {
            //    Georeferencing is not yet available, so find the reference points closest to min/max ply points

            if(0 == nRefpoint)
                  return false;        // have to bail here

                  //    for m_LonMax
            double min_dist_x = 360;
            int imaxclose = 0;
            for(int ic=0 ; ic<nRefpoint ; ic++)
            {
                  double dist = sqrt(((m_LatMax - pRefTable[ic].latr) * (m_LatMax - pRefTable[ic].latr))
                                    + ((m_LonMax - pRefTable[ic].lonr) * (m_LonMax - pRefTable[ic].lonr)));

                  if(dist < min_dist_x)
                  {
                        min_dist_x = dist;
                        imaxclose = ic;
                  }
            }

                  //    for m_LonMin
            double min_dist_n = 360;
            int iminclose = 0;
            for(int id=0 ; id<nRefpoint ; id++)
            {
                  double dist = sqrt(((m_LatMin - pRefTable[id].latr) * (m_LatMin - pRefTable[id].latr))
                                    + ((m_LonMin - pRefTable[id].lonr) * (m_LonMin - pRefTable[id].lonr)));

                  if(dist < min_dist_n)
                  {
                        min_dist_n = dist;
                        iminclose = id;
                  }
            }

            //    Is this chart crossing IDL or Greenwich?
            // Make the check
            if(pRefTable[imaxclose].xr < pRefTable[iminclose].xr)
            {
                  //    This chart crosses IDL and needs a flip, meaning that all negative longitudes need to be normalized
                  //    and the min/max relcalculated
                  //    This code added to correct non-rectangular charts crossing IDL, such as nz14605.kap

                  m_LonMax = -360.0;
                  m_LonMin = 360.0;
                  m_LatMax = -90.0;
                  m_LatMin = 90.0;


                  Plypoint *ppp = (Plypoint *)GetCOVRTableHead(0);      // Normalize the plypoints
                  int cnPlypoint = GetCOVRTablenPoints(0);


                  for(int u=0 ; u<cnPlypoint ; u++)
                  {
                        if( ppp->lnp < 0.)
                              ppp->lnp += 360.;

                        if(ppp->lnp > m_LonMax)
                              m_LonMax = ppp->lnp;
                        if(ppp->lnp < m_LonMin)
                              m_LonMin = ppp->lnp;

                        if(ppp->ltp > m_LatMax)
                              m_LatMax = ppp->ltp;
                        if(ppp->ltp < m_LatMin)
                              m_LatMin = ppp->ltp;

                        ppp++;
                  }
            }


      }

      // Case 2 Lons are both < -180, which means the extent will be reported incorrectly
      // and the plypoint structure will be wrong
      // This case is seen first on 81004_1.KAP, (Mariannas)

      if((m_LonMax < -180.) && (m_LonMin < -180.))
      {
            m_LonMin += 360.;               // Normalize the extents
            m_LonMax += 360.;

            Plypoint *ppp = (Plypoint *)GetCOVRTableHead(0);      // Normalize the plypoints
            int cnPlypoint = GetCOVRTablenPoints(0);

            for(int u=0 ; u<cnPlypoint ; u++)
            {
                  ppp->lnp += 360.;
                  ppp++;
            }
      }

      return true;
}

void ChartBaseBSB::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
    //  Here we convert (subjectively) the Global ColorScheme
    //  to an appropriate BSB_Color_Capability index.

    switch(cs)
    {
        case GLOBAL_COLOR_SCHEME_RGB:
            m_mapped_color_index = COLOR_RGB_DEFAULT;
            break;
        case GLOBAL_COLOR_SCHEME_DAY:
            m_mapped_color_index = DAY;
            break;
        case GLOBAL_COLOR_SCHEME_DUSK:
            m_mapped_color_index = DUSK;
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            m_mapped_color_index = NIGHT;
            break;
        default:
            m_mapped_color_index = DAY;
            break;
    }

    pPalette = GetPalettePtr(m_mapped_color_index);

    m_global_color_scheme = cs;

    //      Force a cache dump in a simple sideways manner
    if(bApplyImmediate)
    {
          m_cached_scale_ppm = 1.0;
    }


    //      Force a new thumbnail
    if(pThumbData)
          pThumbData->pDIBThumb = NULL;
}


wxBitmap *ChartBaseBSB::CreateThumbnail(int tnx, int tny, ColorScheme cs)
{

//    Calculate the size and divisors

      int divx = wxMax(1, Size_X / (4 * tnx) );
      int divy = wxMax(1, Size_Y / (4 * tny) );

      int div_factor = __min(divx, divy);

      int des_width = Size_X / div_factor;
      int des_height = Size_Y / div_factor;

      wxRect gts;
      gts.x = 0;                                // full chart
      gts.y = 0;
      gts.width = Size_X;
      gts.height = Size_Y;

      int this_bpp = 24;                       // for wxImage
//    Allocate the pixel storage needed for one line of chart bits
      unsigned char *pLineT = (unsigned char *)malloc((Size_X+1) * BPP/8);


//    Scale the data quickly
      unsigned char *pPixTN = (unsigned char *)malloc(des_width * des_height * this_bpp/8 );

      int ix = 0;
      int iy = 0;
      int iyd = 0;
      int ixd = 0;
      int yoffd;
      unsigned char *pxs;
      unsigned char *pxd;

      //    Temporarily set the color scheme
      ColorScheme cs_tmp = m_global_color_scheme;
      SetColorScheme(cs, false);


      while(iyd < des_height)
      {
            if(0 == BSBGetScanline( pLineT, iy, 0, Size_X, 1))          // get a line
            {
                  free(pLineT);
                  free(pPixTN);
                  return NULL;
            }


            yoffd = iyd * des_width * this_bpp/8;                 // destination y

            ix = 0;
            ixd = 0;
            while(ixd < des_width )
            {
                  pxs = pLineT + (ix * BPP/8);
                  pxd = pPixTN + (yoffd + (ixd * this_bpp/8));
                  *pxd++ = *pxs++;
                  *pxd++ = *pxs++;
                  *pxd = *pxs;

                  ix += div_factor;
                  ixd++;

            }

            iy += div_factor;
            iyd++;
      }

      free(pLineT);

      //    Reset ColorScheme
      SetColorScheme(cs_tmp, false);



      wxBitmap *retBMP;

#ifdef ocpnUSE_ocpnBitmap
      wxBitmap* bmx2 = new ocpnBitmap(pPixTN, des_width, des_height, -1);
      wxImage imgx2 = bmx2->ConvertToImage();
      imgx2.Rescale( des_width/4, des_height/4, wxIMAGE_QUALITY_HIGH );
      retBMP = new wxBitmap( imgx2 );
      delete bmx2;
#else
      wxImage thumb_image(des_width, des_height, pPixTN, true);
      thumb_image.Rescale( des_width/4, des_height/4, wxIMAGE_QUALITY_HIGH );
      retBMP = new wxBitmap(thumb_image);
#endif


      free(pPixTN);

      return retBMP;

}


//-------------------------------------------------------------------------------------------------
//          Get the Chart thumbnail data structure
//          Creating the thumbnail bitmap as required
//-------------------------------------------------------------------------------------------------

ThumbData *ChartBaseBSB::GetThumbData(int tnx, int tny, float lat, float lon)
{
//    Create the bitmap if needed
      if(!pThumbData->pDIBThumb)
            pThumbData->pDIBThumb = CreateThumbnail(tnx, tny, m_global_color_scheme);


      pThumbData->Thumb_Size_X = tnx;
      pThumbData->Thumb_Size_Y = tny;

//    Plot the supplied Lat/Lon on the thumbnail
      int divx = Size_X / tnx;
      int divy = Size_Y / tny;

      int div_factor = __min(divx, divy);

      double pixx, pixy;


      //    Using a temporary synthetic ViewPort and source rectangle,
      //    calculate the ships position on the thumbnail
      ViewPort tvp;
      tvp.pix_width = tnx;
      tvp.pix_height = tny;
      tvp.view_scale_ppm = GetPPM() / div_factor;
      wxRect trex = Rsrc;
      Rsrc.x = 0;
      Rsrc.y = 0;
      latlong_to_pix_vp(lat, lon, pixx, pixy, tvp);
      Rsrc = trex;

      pThumbData->ShipX = pixx;// / div_factor;
      pThumbData->ShipY = pixy;// / div_factor;


      return pThumbData;
}

bool ChartBaseBSB::UpdateThumbData(double lat, double lon)
{
//    Plot the supplied Lat/Lon on the thumbnail
//  Return TRUE if the pixel location of ownship has changed

    int divx = Size_X / pThumbData->Thumb_Size_X;
    int divy = Size_Y / pThumbData->Thumb_Size_Y;

    int div_factor = __min(divx, divy);

    double pixx_test, pixy_test;


      //    Using a temporary synthetic ViewPort and source rectangle,
      //    calculate the ships position on the thumbnail
    ViewPort tvp;
    tvp.pix_width =  pThumbData->Thumb_Size_X;
    tvp.pix_height =  pThumbData->Thumb_Size_Y;
    tvp.view_scale_ppm = GetPPM() / div_factor;
    wxRect trex = Rsrc;
    Rsrc.x = 0;
    Rsrc.y = 0;
    latlong_to_pix_vp(lat, lon, pixx_test, pixy_test, tvp);
    Rsrc = trex;

    if((pixx_test != pThumbData->ShipX) || (pixy_test != pThumbData->ShipY))
    {
        pThumbData->ShipX = pixx_test;
        pThumbData->ShipY = pixy_test;
        return TRUE;
    }
    else
        return FALSE;
}







//-----------------------------------------------------------------------
//          Pixel to Lat/Long Conversion helpers
//-----------------------------------------------------------------------
static double polytrans( double* coeff, double lon, double lat );

int ChartBaseBSB::vp_pix_to_latlong(ViewPort& vp, double pixx, double pixy, double *plat, double *plon)
{
      if(bHaveEmbeddedGeoref)
      {
            double raster_scale = GetPPM() / vp.view_scale_ppm;

            double px = pixx*raster_scale + Rsrc.x;
            double py = pixy*raster_scale + Rsrc.y;
//            pix_to_latlong(px, py, plat, plon);

            if(1)
            {
                  double lon = polytrans( pwx, px, py );
                  lon = (lon < 0) ? lon + m_cph : lon - m_cph;
                  *plon = lon - m_lon_datum_adjust;
                  *plat = polytrans( pwy, px, py ) - m_lat_datum_adjust;
            }

            return 0;
      }
      else
      {
            double slat, slon;
            double xp, yp;

            if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
            {
                   //      Use Projected Polynomial algorithm

                  double raster_scale = GetPPM() / vp.view_scale_ppm;

                  //      Apply poly solution to vp center point
                  double easting, northing;
                  toTM(vp.clat + m_lat_datum_adjust, vp.clon + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
                  double xc = polytrans( cPoints.wpx, easting, northing );
                  double yc = polytrans( cPoints.wpy, easting, northing );

                  //    convert screen pixels to chart pixmap relative
                  double px = xc + (pixx- (vp.pix_width / 2))*raster_scale;
                  double py = yc + (pixy- (vp.pix_height / 2))*raster_scale;

                  //    Apply polynomial solution to chart relative pixels to get e/n
                  double east  = polytrans( cPoints.pwx, px, py );
                  double north = polytrans( cPoints.pwy, px, py );

                  //    Apply inverse Projection to get lat/lon
                  double lat,lon;
                  fromTM ( east, north, m_proj_lat, m_proj_lon, &lat, &lon );

                  //    Datum adjustments.....
//??                  lon = (lon < 0) ? lon + m_cph : lon - m_cph;
                  double slon_p = lon - m_lon_datum_adjust;
                  double slat_p = lat - m_lat_datum_adjust;

//                  printf("%8g %8g %8g %8g %g\n", slat, slat_p, slon, slon_p, slon - slon_p);
                  slon = slon_p;
                  slat = slat_p;

            }
            else if(m_projection == PROJECTION_MERCATOR)
            {
                   //      Use Projected Polynomial algorithm

                  double raster_scale = GetPPM() / vp.view_scale_ppm;

                  //      Apply poly solution to vp center point
                  double easting, northing;
                  toSM_ECC(vp.clat + m_lat_datum_adjust, vp.clon + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
                  double xc = polytrans( cPoints.wpx, easting, northing );
                  double yc = polytrans( cPoints.wpy, easting, northing );

                  //    convert screen pixels to chart pixmap relative
                  double px = xc + (pixx- (vp.pix_width / 2))*raster_scale;
                  double py = yc + (pixy- (vp.pix_height / 2))*raster_scale;

                  //    Apply polynomial solution to chart relative pixels to get e/n
                  double east  = polytrans( cPoints.pwx, px, py );
                  double north = polytrans( cPoints.pwy, px, py );

                  //    Apply inverse Projection to get lat/lon
                  double lat,lon;
                  fromSM_ECC ( east, north, m_proj_lat, m_proj_lon, &lat, &lon );

                  //    Make Datum adjustments.....
                  double slon_p = lon - m_lon_datum_adjust;
                  double slat_p = lat - m_lat_datum_adjust;

                  slon = slon_p;
                  slat = slat_p;

//                  printf("vp.clon  %g    xc  %g   px   %g   east  %g  \n", vp.clon, xc, px, east);

            }
            else if(m_projection == PROJECTION_POLYCONIC)
            {
                   //      Use Projected Polynomial algorithm

                  double raster_scale = GetPPM() / vp.view_scale_ppm;

                  //      Apply poly solution to vp center point
                  double easting, northing;
                  toPOLY(vp.clat + m_lat_datum_adjust, vp.clon + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
                  double xc = polytrans( cPoints.wpx, easting, northing );
                  double yc = polytrans( cPoints.wpy, easting, northing );

                  //    convert screen pixels to chart pixmap relative
                  double px = xc + (pixx- (vp.pix_width / 2))*raster_scale;
                  double py = yc + (pixy- (vp.pix_height / 2))*raster_scale;

                  //    Apply polynomial solution to chart relative pixels to get e/n
                  double east  = polytrans( cPoints.pwx, px, py );
                  double north = polytrans( cPoints.pwy, px, py );

                  //    Apply inverse Projection to get lat/lon
                  double lat,lon;
                  fromPOLY ( east, north, m_proj_lat, m_proj_lon, &lat, &lon );

                  //    Make Datum adjustments.....
                  double slon_p = lon - m_lon_datum_adjust;
                  double slat_p = lat - m_lat_datum_adjust;

                  slon = slon_p;
                  slat = slat_p;

            }
            else
            {
                  // Use a Mercator estimator, with Eccentricity corrrection applied
                  int dx = pixx - ( vp.pix_width  / 2 );
                  int dy = ( vp.pix_height / 2 ) - pixy;

                  xp = ( dx * cos ( vp.skew ) ) - ( dy * sin ( vp.skew ) );
                  yp = ( dy * cos ( vp.skew ) ) + ( dx * sin ( vp.skew ) );

                  double d_east = xp / vp.view_scale_ppm;
                  double d_north = yp / vp.view_scale_ppm;

                  fromSM_ECC ( d_east, d_north, vp.clat, vp.clon, &slat, &slon );
            }

            *plat = slat;

            if(slon < -180.)
                  slon += 360.;
            else if(slon > 180.)
                  slon -= 360.;
            *plon = slon;

            return 0;
      }

}




int ChartBaseBSB::latlong_to_pix_vp(double lat, double lon, double &pixx, double &pixy, ViewPort& vp)
{
    double alat, alon;

    if(bHaveEmbeddedGeoref)
    {
          double alat, alon;

          alon = lon + m_lon_datum_adjust;
          alat = lat + m_lat_datum_adjust;

          AdjustLongitude(alon);

          if(1)
          {
                /* change longitude phase (CPH) */
                double lonp = (alon < 0) ? alon + m_cph : alon - m_cph;
                double xd = polytrans( wpx, lonp, alat );
                double yd = polytrans( wpy, lonp, alat );

                double raster_scale = GetPPM() / vp.view_scale_ppm;

                pixx = (xd - Rsrc.x) / raster_scale;
                pixy = (yd - Rsrc.y) / raster_scale;

            return 0;
          }
    }
    else
    {
          double easting, northing;
          double xlon = lon;

                //  Make sure lon and lon0 are same phase
/*
          if((xlon * vp.clon) < 0.)
          {
                if(xlon < 0.)
                      xlon += 360.;
                else
                      xlon -= 360.;
          }

          if(fabs(xlon - vp.clon) > 180.)
          {
                if(xlon > vp.clon)
                      xlon -= 360.;
                else
                      xlon += 360.;
          }
*/


          if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
          {
                //      Use Projected Polynomial algorithm

                alon = lon + m_lon_datum_adjust;
                alat = lat + m_lat_datum_adjust;

                //      Get e/n from TM Projection
                toTM(alat, alon, m_proj_lat, m_proj_lon, &easting, &northing);

                //      Apply poly solution to target point
                double xd = polytrans( cPoints.wpx, easting, northing );
                double yd = polytrans( cPoints.wpy, easting, northing );

                //      Apply poly solution to vp center point
                toTM(vp.clat + m_lat_datum_adjust, vp.clon + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
                double xc = polytrans( cPoints.wpx, easting, northing );
                double yc = polytrans( cPoints.wpy, easting, northing );

                //      Calculate target point relative to vp center
                double raster_scale = GetPPM() / vp.view_scale_ppm;

                double xs = xc - vp.pix_width  * raster_scale / 2;
                double ys = yc - vp.pix_height * raster_scale / 2;

                pixx = (xd - xs) / raster_scale;
                pixy = (yd - ys) / raster_scale;

          }
          else if(m_projection == PROJECTION_MERCATOR)
          {
                //      Use Projected Polynomial algorithm

                alon = lon + m_lon_datum_adjust;
                alat = lat + m_lat_datum_adjust;

                //      Get e/n from  Projection
                xlon = alon;
                AdjustLongitude(xlon);
                toSM_ECC(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

                //      Apply poly solution to target point
                double xd = polytrans( cPoints.wpx, easting, northing );
                double yd = polytrans( cPoints.wpy, easting, northing );

                //      Apply poly solution to vp center point
                double xlonc = vp.clon;
                AdjustLongitude(xlonc);

                toSM_ECC(vp.clat + m_lat_datum_adjust, xlonc + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
                double xc = polytrans( cPoints.wpx, easting, northing );
                double yc = polytrans( cPoints.wpy, easting, northing );

                //      Calculate target point relative to vp center
                double raster_scale = GetPPM() / vp.view_scale_ppm;

                double xs = xc - vp.pix_width  * raster_scale / 2;
                double ys = yc - vp.pix_height * raster_scale / 2;

                pixx = (xd - xs) / raster_scale;
                pixy = (yd - ys) / raster_scale;

          }
          else if(m_projection == PROJECTION_POLYCONIC)
          {
                //      Use Projected Polynomial algorithm

                alon = lon + m_lon_datum_adjust;
                alat = lat + m_lat_datum_adjust;

                //      Get e/n from  Projection
                xlon = AdjustLongitude(alon);
                toPOLY(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

                //      Apply poly solution to target point
                double xd = polytrans( cPoints.wpx, easting, northing );
                double yd = polytrans( cPoints.wpy, easting, northing );

                //      Apply poly solution to vp center point
                double xlonc = AdjustLongitude(vp.clon);

                toPOLY(vp.clat + m_lat_datum_adjust, xlonc + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
                double xc = polytrans( cPoints.wpx, easting, northing );
                double yc = polytrans( cPoints.wpy, easting, northing );

                //      Calculate target point relative to vp center
                double raster_scale = GetPPM() / vp.view_scale_ppm;

                double xs = xc - vp.pix_width  * raster_scale / 2;
                double ys = yc - vp.pix_height * raster_scale / 2;

                pixx = (xd - xs) / raster_scale;
                pixy = (yd - ys) / raster_scale;

          }
          else
          {
                toSM_ECC(lat, xlon, vp.clat, vp.clon, &easting, &northing);

                double epix = easting  * vp.view_scale_ppm;
                double npix = northing * vp.view_scale_ppm;

                double dx = epix * cos ( vp.skew ) + npix * sin ( vp.skew );
                double dy = npix * cos ( vp.skew ) - epix * sin ( vp.skew );

                pixx = ( (double)vp.pix_width  / 2 ) + dx;
                pixy = ( (double)vp.pix_height / 2 ) - dy;
          }
                return 0;
    }

    return 1;
}


void ChartBaseBSB::latlong_to_chartpix(double lat, double lon, double &pixx, double &pixy)
{
      double alat, alon;
      pixx = 0.0;
      pixy = 0.0;

      if(bHaveEmbeddedGeoref)
      {
            double alat, alon;

            alon = lon + m_lon_datum_adjust;
            alat = lat + m_lat_datum_adjust;

            alon = AdjustLongitude(alon);

            /* change longitude phase (CPH) */
            double lonp = (alon < 0) ? alon + m_cph : alon - m_cph;
            pixx = polytrans( wpx, lonp, alat );
            pixy = polytrans( wpy, lonp, alat );
      }
      else
      {
            double easting, northing;
            double xlon = lon;

            if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
            {
                //      Use Projected Polynomial algorithm

                  alon = lon + m_lon_datum_adjust;
                  alat = lat + m_lat_datum_adjust;

                //      Get e/n from TM Projection
                  toTM(alat, alon, m_proj_lat, m_proj_lon, &easting, &northing);

                //      Apply poly solution to target point
                  pixx = polytrans( cPoints.wpx, easting, northing );
                  pixy = polytrans( cPoints.wpy, easting, northing );


            }
            else if(m_projection == PROJECTION_MERCATOR)
            {
                //      Use Projected Polynomial algorithm

                  alon = lon + m_lon_datum_adjust;
                  alat = lat + m_lat_datum_adjust;

                //      Get e/n from  Projection
                  xlon = AdjustLongitude(alon);

                  toSM_ECC(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

                //      Apply poly solution to target point
                  pixx = polytrans( cPoints.wpx, easting, northing );
                  pixy = polytrans( cPoints.wpy, easting, northing );


            }
            else if(m_projection == PROJECTION_POLYCONIC)
            {
                //      Use Projected Polynomial algorithm

                  alon = lon + m_lon_datum_adjust;
                  alat = lat + m_lat_datum_adjust;

                //      Get e/n from  Projection
                  xlon = AdjustLongitude(alon);
                  toPOLY(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

                //      Apply poly solution to target point
                  pixx = polytrans( cPoints.wpx, easting, northing );
                  pixy = polytrans( cPoints.wpy, easting, northing );

            }
      }
}

void ChartBaseBSB::chartpix_to_latlong(double pixx, double pixy, double *plat, double *plon)
{
      if(bHaveEmbeddedGeoref)
      {
            double lon = polytrans( pwx, pixx, pixy );
            lon = (lon < 0) ? lon + m_cph : lon - m_cph;
            *plon = lon - m_lon_datum_adjust;
            *plat = polytrans( pwy, pixx, pixy ) - m_lat_datum_adjust;
      }
      else
      {
            double slat, slon;
            if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
            {
                   //      Use Projected Polynomial algorithm

                  //    Apply polynomial solution to chart relative pixels to get e/n
                  double east  = polytrans( cPoints.pwx, pixx, pixy );
                  double north = polytrans( cPoints.pwy, pixx, pixy );

                  //    Apply inverse Projection to get lat/lon
                  double lat,lon;
                  fromTM ( east, north, m_proj_lat, m_proj_lon, &lat, &lon );

                  //    Datum adjustments.....
//??                  lon = (lon < 0) ? lon + m_cph : lon - m_cph;
                  slon = lon - m_lon_datum_adjust;
                  slat = lat - m_lat_datum_adjust;


            }
            else if(m_projection == PROJECTION_MERCATOR)
            {
                   //      Use Projected Polynomial algorithm
                  //    Apply polynomial solution to chart relative pixels to get e/n
                  double east  = polytrans( cPoints.pwx, pixx, pixy );
                  double north = polytrans( cPoints.pwy, pixx, pixy );

                  //    Apply inverse Projection to get lat/lon
                  double lat,lon;
                  fromSM_ECC ( east, north, m_proj_lat, m_proj_lon, &lat, &lon );

                  //    Make Datum adjustments.....
                  slon = lon - m_lon_datum_adjust;
                  slat = lat - m_lat_datum_adjust;
            }
            else if(m_projection == PROJECTION_POLYCONIC)
            {
                   //      Use Projected Polynomial algorithm
                  //    Apply polynomial solution to chart relative pixels to get e/n
                  double east  = polytrans( cPoints.pwx, pixx, pixy );
                  double north = polytrans( cPoints.pwy, pixx, pixy );

                  //    Apply inverse Projection to get lat/lon
                  double lat,lon;
                  fromPOLY ( east, north, m_proj_lat, m_proj_lon, &lat, &lon );

                  //    Make Datum adjustments.....
                  slon = lon - m_lon_datum_adjust;
                  slat = lat - m_lat_datum_adjust;

            }
            else
            {
                  slon = 0.;
                  slat = 0.;
            }

            *plat = slat;

            if(slon < -180.)
                  slon += 360.;
            else if(slon > 180.)
                  slon -= 360.;
            *plon = slon;

      }

}

void ChartBaseBSB::ComputeSourceRectangle(const ViewPort &vp, wxRect *pSourceRect)
{
      m_raster_scale_factor = GetRasterScaleFactor(vp);
      double xd, yd;
      latlong_to_chartpix(vp.clat, vp.clon, xd, yd);

      wxRealPoint pos, size;

      pos.x = xd - (vp.pix_width  * m_raster_scale_factor / 2);
      pos.y = yd - (vp.pix_height * m_raster_scale_factor / 2);

      size.x = vp.pix_width  * m_raster_scale_factor;
      size.y = vp.pix_height * m_raster_scale_factor;

      *pSourceRect = wxRect(wxRound(pos.x), wxRound(pos.y), wxRound(size.x), wxRound(size.y));
}


double ChartBaseBSB::GetRasterScaleFactor(const ViewPort &vp)
{
    //      This funny contortion is necessary to allow scale factors < 1, i.e. overzoom
      return (wxRound(100000 * GetPPM() / vp.view_scale_ppm)) / 100000.;
}

void ChartBaseBSB::SetVPRasterParms(const ViewPort &vpt)
{
      //    Calculate the potential datum offset parameters for this viewport, if not WGS84

      if(m_datum_index == DATUM_INDEX_WGS84 || m_datum_index == DATUM_INDEX_UNKNOWN)
      {
            m_lon_datum_adjust = (-m_dtm_lon) / 3600.;
            m_lat_datum_adjust = (-m_dtm_lat) / 3600.;
      }

      else
      {
            double to_lat, to_lon;
            MolodenskyTransform (vpt.clat, vpt.clon, &to_lat, &to_lon, m_datum_index, DATUM_INDEX_WGS84);
            m_lon_datum_adjust = -(to_lon - vpt.clon);
            m_lat_datum_adjust = -(to_lat - vpt.clat);
            if(m_b_apply_dtm)
            {
                  m_lon_datum_adjust -= m_dtm_lon / 3600.;
                  m_lat_datum_adjust -= m_dtm_lat / 3600.;
            }
      }

      ComputeSourceRectangle(vpt, &Rsrc);

      if(vpt.IsValid())
            m_vp_render_last = vpt;

}

bool ChartBaseBSB::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed)
{
      bool bInside = G_FloatPtInPolygon ( ( MyFlPoint * ) GetCOVRTableHead ( 0 ), GetCOVRTablenPoints ( 0 ), vp_proposed.clon, vp_proposed.clat );
      if(!bInside)
            return false;

      ViewPort vp_save = vp_proposed;                 // save a copy

      int ret_val = 0;
      double binary_scale_factor = GetPPM() / vp_proposed.view_scale_ppm;

      if(vp_last.IsValid())
      {
                  //    We only need to adjust the VP if the cache is valid and potentially usable, i.e. the scale factor is integer...
                  //    The objective here is to ensure that the VP center falls on an exact pixel boundary within the cache


                  if(cached_image_ok && (binary_scale_factor > 1.0) && (fabs(binary_scale_factor - wxRound(binary_scale_factor)) < 1e-5))
                  {
                        if(m_b_cdebug)printf(" Possible Adjust VP for integer scale: %g\n", binary_scale_factor);

                        wxRect rprop;
                        ComputeSourceRectangle(vp_proposed, &rprop);

                        double pixx, pixy;
                        double lon_adj, lat_adj;
                        latlong_to_pix_vp(vp_proposed.clat, vp_proposed.clon, pixx, pixy, vp_proposed);
                        vp_pix_to_latlong(vp_proposed, pixx, pixy, &lat_adj, &lon_adj);

                        vp_proposed.clat = lat_adj;
                        vp_proposed.clon = lon_adj;
                        ret_val = 1;
                  }
      }

      return (ret_val > 0);
}

bool ChartBaseBSB::IsRenderCacheable( wxRect& source, wxRect& dest )
{
      double scale_x = (double)source.width / (double)dest.width;

      if(scale_x <= 1.0)                                        // overzoom
      {
//            if(m_b_cdebug)printf("    MISS<<<>>>GVUC:  Overzoom\n");
            return false;
      }


      //    Using the cache only works for pure binary scale factors......
      if((fabs(scale_x - wxRound(scale_x))) > .0001)
      {
//            if(m_b_cdebug)printf("   MISS<<<>>>GVUC: Not digital scale test 1\n");
            return false;
      }


      //    Scale must be exactly digital...
      if((int)(source.width/dest.width) != (int)wxRound(scale_x))
      {
//            if(m_b_cdebug)printf("   MISS<<<>>>GVUC: Not digital scale test 2\n");
            return false;
      }

      return true;
}


void ChartBaseBSB::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion)
{
      SetVPRasterParms(VPoint);

      double raster_scale =  VPoint.view_scale_ppm / GetPPM();

      int rxl, rxr;
      int ryb, ryt;

      rxl = wxMax(-Rsrc.x * raster_scale, VPoint.rv_rect.x);
      rxr = wxMin((Size_X - Rsrc.x) * raster_scale, VPoint.rv_rect.width + VPoint.rv_rect.x);
 
      ryt = wxMax(-Rsrc.y * raster_scale, VPoint.rv_rect.y);
      ryb = wxMin((Size_Y - Rsrc.y) * raster_scale, VPoint.rv_rect.height + VPoint.rv_rect.y);
      
      
      
      pValidRegion->Clear();
      pValidRegion->Union(rxl, ryt, rxr - rxl, ryb - ryt);
}

LLRegion ChartBaseBSB::GetValidRegion()
{
    // should we cache this?
    double ll[8];
    chartpix_to_latlong(0,      0,      ll+0, ll+1);
    chartpix_to_latlong(0,      Size_Y, ll+2, ll+3);
    chartpix_to_latlong(Size_X, Size_Y, ll+4, ll+5);
    chartpix_to_latlong(Size_X, 0,      ll+6, ll+7);

    // for now don't allow raster charts to cross both 0 meridian and IDL (complicated to deal with)
    for(int i=1; i<6; i+=2)
        if(fabs(ll[i] - ll[i+2]) > 180) {
            // we detect crossing idl here, make all longitudes positive
            for(int i=1; i<8; i+=2)
                if(ll[i] < 0)
                    ll[i] += 360;
            break;
        }

    return LLRegion(4, ll);
}

bool ChartBaseBSB::GetViewUsingCache( wxRect& source, wxRect& dest, const OCPNRegion& Region, ScaleTypeEnum scale_type )
{
      wxRect s1;
      ScaleTypeEnum scale_type_corrected;

      if(m_b_cdebug)printf(" source:  %d %d\n", source.x, source.y);
      if(m_b_cdebug)printf(" cache:   %d %d\n", cache_rect.x, cache_rect.y);

//    Anything to do?
      if((source == cache_rect) /*&& (cache_scale_method == scale_type)*/ && (cached_image_ok) )
      {
            if(m_b_cdebug)printf("    GVUC: Cache is good, nothing to do\n");
            return false;
      }

      double scale_x = (double)source.width / (double)dest.width;

      if(m_b_cdebug)printf("GVUC: scale_x: %g\n", scale_x);

      //    Enforce a limit on bilinear scaling, for performance reasons
      scale_type_corrected = scale_type; //RENDER_LODEF; //scale_type;
      if(scale_x > m_bilinear_limit)
            scale_type_corrected = RENDER_LODEF;

      {
//            if(b_cdebug)printf("   MISS<<<>>>GVUC: Intentional out\n");
//            return GetView( source, dest, scale_type_corrected );
      }


      //    Using the cache only works for pure binary scale factors......
      if((fabs(scale_x - wxRound(scale_x))) > .0001)
      {
            if(m_b_cdebug)printf("   MISS<<<>>>GVUC: Not digital scale test 1\n");
            return GetView( source, dest, scale_type_corrected );
      }

//      scale_type_corrected = RENDER_LODEF;


      if(!cached_image_ok)
      {
            if(m_b_cdebug)printf("    MISS<<<>>>GVUC:  Cache NOk\n");
            return GetView( source, dest, scale_type_corrected );
      }

      if(scale_x < 1.0)                                        // overzoom
      {
            if(m_b_cdebug)printf("    MISS<<<>>>GVUC:  Overzoom\n");
            return GetView( source, dest, scale_type_corrected );
      }

      //    Scale must be exactly digital...
      if((int)(source.width/dest.width) != (int)wxRound(scale_x))
      {
            if(m_b_cdebug)printf("   MISS<<<>>>GVUC: Not digital scale test 2\n");
            return GetView( source, dest, scale_type_corrected );
      }

//    Calculate the digital scale, e.g. 1,2,4,8,,,
      int cs1d = source.width/dest.width;
      if(abs(source.x - cache_rect.x) % cs1d)
      {
            if(m_b_cdebug)printf("   source.x: %d  cache_rect.x: %d  cs1d: %d\n", source.x, cache_rect.x, cs1d);
            if(m_b_cdebug)printf("   MISS<<<>>>GVUC: x mismatch\n");
            return GetView( source, dest, scale_type_corrected );
      }
      if(abs(source.y - cache_rect.y) % cs1d)
      {
            if(m_b_cdebug)printf("   MISS<<<>>>GVUC: y mismatch\n");
            return GetView( source, dest, scale_type_corrected );
      }

      if(pPixCache && ((pPixCache->GetWidth() != dest.width) || (pPixCache->GetHeight() != dest.height)))
      {
            if(m_b_cdebug)printf("   MISS<<<>>>GVUC: dest size mismatch\n");
            return GetView( source, dest, scale_type_corrected );
      }

      int stride_rows = (source.y + source.height) - (cache_rect.y + cache_rect.height);
      int scaled_stride_rows = (int)(stride_rows / scale_x);

      if(abs(stride_rows) >= source.height)                       // Pan more than one screen
            return GetView( source, dest, scale_type_corrected );

      int stride_pixels = (source.x + source.width) - (cache_rect.x + cache_rect.width);
      int scaled_stride_pixels = (int)(stride_pixels / scale_x);


      if(abs(stride_pixels) >= source.width)                      // Pan more than one screen
            return GetView( source, dest, scale_type_corrected );

      if(m_b_cdebug)printf("    GVUC Using raster data cache\n");

      ScaleTypeEnum pan_scale_type_x = scale_type_corrected;
      ScaleTypeEnum pan_scale_type_y = scale_type_corrected;


      //    "Blit" the valid pixels out of the way
    if( pPixCache )
    {
        int height = pPixCache->GetHeight();
        int width = pPixCache->GetWidth();
        int buffer_stride_bytes = pPixCache->GetLinePitch();
        
        unsigned char *ps;
        unsigned char *pd;
        
        if(stride_rows > 0)                             // pan down
        {
            ps = pPixCache->GetpData() +  (abs(scaled_stride_rows) * buffer_stride_bytes);
            if(stride_pixels > 0)
                ps += scaled_stride_pixels * BPP/8;
            
            pd = pPixCache->GetpData();
            if(stride_pixels <= 0)
                pd += abs(scaled_stride_pixels) * BPP/8;
            
            for(int iy=0 ; iy< (height - abs(scaled_stride_rows)) ; iy++)
            {
                memmove(pd, ps, (width - abs(scaled_stride_pixels)) *BPP/8);
                ps += buffer_stride_bytes;
                pd += buffer_stride_bytes;
            }
            
        }
        else
        {
            ps = pPixCache->GetpData() + ((height - abs(scaled_stride_rows)-1) * buffer_stride_bytes);
            if(stride_pixels > 0)               // make a hole on right
                ps += scaled_stride_pixels * BPP/8;
            
            pd = pPixCache->GetpData() +  ((height -1) * buffer_stride_bytes);
            if(stride_pixels <= 0)              // make a hole on the left
                pd += abs(scaled_stride_pixels) * BPP/8;
            
            
            for(int iy=0 ; iy< (height - abs(scaled_stride_rows)) ; iy++)
            {
                memmove(pd, ps, (width - abs(scaled_stride_pixels)) *BPP/8);
                ps -= buffer_stride_bytes;
                pd -= buffer_stride_bytes;
            }
        }
        
        //    Y Pan
        if(source.y != cache_rect.y)
        {
            wxRect sub_dest = dest;
            sub_dest.height = abs(scaled_stride_rows);
            
            if(stride_rows > 0)                             // pan down
            {
                sub_dest.y = height - scaled_stride_rows;
                
            }
            else
            {
                sub_dest.y = 0;
                
            }
            
            //    Get the new bits needed
            
            //    A little optimization...
            //    No sense in fetching bits that are not part of the ultimate render region
            wxRegionContain rc = Region.Contains(sub_dest);
            if((wxPartRegion == rc) || (wxInRegion == rc))
            {
                GetAndScaleData(pPixCache->GetpData(), pPixCache->GetLength(), source, source.width, sub_dest, width, cs1d, pan_scale_type_y);
            }
            pPixCache->Update();
            
            //    Update the cached parameters, Y only
            
            cache_rect.y = source.y;
            //          cache_rect = source;
            cache_rect_scaled = dest;
            cached_image_ok = 1;
            
        }                 // Y Pan
        
        
        
        
        //    X Pan
        if(source.x != cache_rect.x)
        {
            wxRect sub_dest = dest;
            sub_dest.width = abs(scaled_stride_pixels);
            
            if(stride_pixels > 0)                           // pan right
            {
                sub_dest.x = width - scaled_stride_pixels;
            }
            else                                                  // pan left
            {
                sub_dest.x = 0;
            }
            
            //    Get the new bits needed
            
            //    A little optimization...
            //    No sense in fetching bits that are not part of the ultimate render region
            wxRegionContain rc = Region.Contains(sub_dest);
            if((wxPartRegion == rc) || (wxInRegion == rc))
            {
                GetAndScaleData(pPixCache->GetpData(), pPixCache->GetLength(), source, source.width, sub_dest, width, cs1d, pan_scale_type_x);
            }
            
            pPixCache->Update();
            
            //    Update the cached parameters
            cache_rect = source;
            cache_rect_scaled = dest;
            cached_image_ok = 1;
            
        }           // X pan
        
        return true;
    }
    return false;
}






int s_dc;

bool ChartBaseBSB::RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint)
{
      SetVPRasterParms(VPoint);

      OCPNRegion rgn(0,0,VPoint.pix_width, VPoint.pix_height);

      bool bsame_region = (rgn == m_last_region);          // only want to do this once


      if(!bsame_region)
            cached_image_ok = false;

      m_last_region = rgn;

      return RenderRegionViewOnDC(dc, VPoint, rgn);

}



bool ChartBaseBSB::RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint, const OCPNRegion &RectRegion, const LLRegion &Region)
{
      return true;
}


bool ChartBaseBSB::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region)
{
      SetVPRasterParms(VPoint);

      wxRect dest(0,0,VPoint.pix_width, VPoint.pix_height);
//      double factor = ((double)Rsrc.width)/((double)dest.width);
      double factor = GetRasterScaleFactor(VPoint);
      if(m_b_cdebug)
      {
            printf("%d RenderRegion  ScaleType:  %d   factor:  %g\n", s_dc++, RENDER_HIDEF, factor );
            printf("Rect list:\n");
            OCPNRegionIterator upd ( Region ); // get the requested rect list
            while ( upd.HaveRects() )
            {
                  wxRect rect = upd.GetRect();
                  printf("   %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height);
                  upd.NextRect() ;
            }
      }

            //    Invalidate the cache if the scale has changed or the viewport size has changed....
      if((fabs(m_cached_scale_ppm - VPoint.view_scale_ppm) > 1e-9) || (m_last_vprect != dest))
      {
            cached_image_ok = false;
            m_vp_render_last.Invalidate();
      }
/*
      if(pPixCache)
      {
            if((pPixCache->GetWidth() != dest.width) || (pPixCache->GetHeight() != dest.height))
            {
                  delete pPixCache;
                  pPixCache = new PixelCache(dest.width, dest.height, BPP);
            }
      }
      else
            pPixCache = new PixelCache(dest.width, dest.height, BPP);
*/

      m_cached_scale_ppm = VPoint.view_scale_ppm;
      m_last_vprect = dest;


      if(cached_image_ok)
      {
            //    Anything to do?
           bool bsame_region = (Region == m_last_region);          // only want to do this once

           if((bsame_region) && (Rsrc == cache_rect)  )
           {
              pPixCache->SelectIntoDC(dc);
              if(m_b_cdebug)printf("  Using Current PixelCache\n");
              return false;
           }
      }

     m_last_region = Region;


     //     Analyze the region requested
     //     When rendering complex regions, (more than say 4 rectangles)
     //     .OR. small proportions, then rectangle rendering may be faster
     //     Also true  if the scale is less than near unity, or overzoom.
     //     This will be the case for backgrounds of the quilt.


     /*  Update for Version 2.4.0
     This logic seems flawed, at least for quilts which contain charts having non-rectangular coverage areas.
     These quilt regions decompose to ...LOTS... of rectangles, most of which are 1 pixel in height.
     This is very slow, due to the overhead of GetAndScaleData().
     However, remember that overzoom never uses the cache, nor does non-binary scale factors..
     So, we check to see if this is a cacheable render, and that the number of rectangles is "reasonable"
     */

     //     Get the region rectangle count

     int n_rect =0;
     OCPNRegionIterator upd ( Region ); // get the requested rect list
     while ( upd.HaveRects() )
     {
           n_rect++;
           upd.NextRect();
     }

     if((!IsRenderCacheable( Rsrc, dest ) && ( n_rect > 4 ) && (n_rect < 20)) || ( factor < 1))
     {
         if(m_b_cdebug)printf("   RenderRegion by rect iterator   n_rect: %d\n", n_rect);
                              
           // Verify that the persistent pixel cache is at least as large as the largest rectangle in the region
           wxRect dest_check_rect = dest;
           OCPNRegionIterator upd_check ( Region ); // get the requested rect list
           while ( upd_check.HaveRects() )
           {
               wxRect rect = upd_check.GetRect();
               dest_check_rect.Union(rect);
               upd_check.NextRect();
           }
 
            if(pPixCache)
            {
                if((pPixCache->GetWidth() != dest_check_rect.width) || (pPixCache->GetHeight() != dest_check_rect.height))
                {
                    delete pPixCache;
                    pPixCache = new PixelCache(dest_check_rect.width, dest_check_rect.height, BPP);
                }
            }
            else
                pPixCache = new PixelCache(dest_check_rect.width, dest_check_rect.height, BPP);

           
           ScaleTypeEnum ren_type = RENDER_LODEF;



      //    Decompose the region into rectangles, and fetch them into the target dc
           OCPNRegionIterator upd ( Region ); // get the requested rect list
           int ir = 0;
           while ( upd.HaveRects() )
           {
                 wxRect rect = upd.GetRect();
                 
                 //  Floating point math can lead to negative rectangle origin.
                 //  If this happens, we arbitrarily shift the rectangle to be positive semidefinite.
                 //  This will cause at most a 1 pixlel error onscreen.
                 if(rect.y < 0) rect.Offset(0, -rect.y);
                 if(rect.x < 0) rect.Offset(-rect.x, 0);
           
                 
                 GetAndScaleData(pPixCache->GetpData(), pPixCache->GetLength(),
                        Rsrc, Rsrc.width, rect, pPixCache->GetWidth(), factor, ren_type);
                 
                 ir++;
                 upd.NextRect();;
           }

           pPixCache->Update();

      //    Update cache parameters
           cache_rect = Rsrc;
           cache_scale_method = ren_type;
           cached_image_ok = false;            // Never cache this type of render

      //    Select the data into the dc
           pPixCache->SelectIntoDC(dc);

           return true;
     }


     //     Default is to try using the cache
     
     if(pPixCache)
     {
         if((pPixCache->GetWidth() != dest.width) || (pPixCache->GetHeight() != dest.height))
         {
             delete pPixCache;
             pPixCache = new PixelCache(dest.width, dest.height, BPP);
         }
     }
     else
         pPixCache = new PixelCache(dest.width, dest.height, BPP);
     
     

     if(m_b_cdebug)printf("  Render Region By GVUC\n");

     //     A performance enhancement.....
     ScaleTypeEnum scale_type_zoom = RENDER_HIDEF;
     double binary_scale_factor = VPoint.view_scale_ppm / GetPPM();
     if(binary_scale_factor < .20)
           scale_type_zoom = RENDER_LODEF;

     bool bnewview = GetViewUsingCache(Rsrc, dest, Region, scale_type_zoom);

     //    Select the data into the dc
     pPixCache->SelectIntoDC(dc);

     return bnewview;

}

wxImage *ChartBaseBSB::GetImage()
{
      int img_size_x = ((Size_X >> 2) * 4) + 4;
      wxImage *img = new wxImage( img_size_x, Size_Y, false);

      unsigned char *ppnx = img->GetData();


      for(int i=0 ; i < Size_Y ; i++)
      {
            wxRect source_rect(0,i,Size_X, 1);
            wxRect dest_rect(0,0,Size_X, 1);

            GetAndScaleData(img->GetData(), img_size_x * Size_Y * 3, source_rect, Size_X, dest_rect, Size_X, 1.0, RENDER_HIDEF);

            ppnx += img_size_x * 3;
      }

      return img;
}


bool ChartBaseBSB::GetView( wxRect& source, wxRect& dest, ScaleTypeEnum scale_type )
{
//      PixelCache *pPixCacheTemp = new PixelCache(dest.width, dest.height, BPP);

//    Get and Rescale the data directly into the temporary PixelCache data buffer
      double factor = ((double)source.width)/((double)dest.width);

/*
      if(!GetAndScaleData(&ppnx, source, source.width, dest, dest.width, factor, scale_type))
      {
           delete pPixCacheTemp;                       // Some error, retain old cache
           return false;
      }
      else
      {
           delete pPixCache;                           // new cache is OK
           pPixCache = pPixCacheTemp;
      }
*/
      GetAndScaleData(pPixCache->GetpData(), pPixCache->GetLength(), source, source.width, dest, dest.width, factor, scale_type);
      pPixCache->Update();

//    Update cache parameters

      cache_rect = source;
      cache_rect_scaled = dest;
      cache_scale_method = scale_type;

      cached_image_ok = 1;


      return TRUE;
}


bool ChartBaseBSB::GetAndScaleData(unsigned char *ppn, size_t data_size, wxRect& source, int source_stride,
                                   wxRect& dest, int dest_stride, double scale_factor, ScaleTypeEnum scale_type)
{

      unsigned char *s_data = NULL;

      double factor = scale_factor;
      int Factor =  (int)factor;

      int target_width = (int)wxRound((double)source.width  / factor) ;
      int target_height = (int)wxRound((double)source.height / factor);

      int dest_line_length = dest_stride * BPP/8;
      
      //  On MSW, if using DibSections, each scan line starts on a DWORD boundary.
      //  The DibSection has been allocated to conform with this requirement.
#ifdef __PIX_CACHE_DIBSECTION__      
      dest_line_length = (((dest_stride * 24) + 31) & ~31) >> 3;
#endif      
      
      if((target_height == 0) || (target_width == 0))
            return false;

      unsigned char *target_data = ppn;
      unsigned char *data = ppn;

      if(factor > 1)                // downsampling
      {

            if(scale_type == RENDER_HIDEF)
            {
//    Allocate a working buffer based on scale factor
                  int blur_factor = wxMax(2, Factor);
                  int wb_size = (source.width) * (blur_factor * 2) * BPP/8 ;
                  s_data = (unsigned char *) malloc( wb_size ); // work buffer
                  unsigned char *pixel;
                  int y_offset;

                  for (int y = dest.y; y < (dest.y + dest.height); y++)
                  {
                  //    Read "blur_factor" lines

                        wxRect s1;
                        s1.x = source.x;
                        s1.y = source.y  + (int)(y * factor);
                        s1.width = source.width;
                        s1.height = blur_factor;
                        GetChartBits(s1, s_data, 1);

                        target_data = data + (y * dest_line_length/*dest_stride * BPP/8*/);

                        for (int x = 0; x < target_width; x++)
                        {
                              unsigned int avgRed = 0 ;
                              unsigned int avgGreen = 0;
                              unsigned int avgBlue = 0;
                              unsigned int pixel_count = 0;
                              unsigned char *pix0 = s_data +  BPP/8 * ((int)( x * factor )) ;
                              y_offset = 0;

                              if((x * Factor) < (Size_X - source.x))
                              {
            // determine average
                                    for ( int y1 = 0 ; y1 < blur_factor ; ++y1 )
                                    {
                                        pixel = pix0 + (BPP/8 * y_offset ) ;
                                        for ( int x1 = 0 ; x1 < blur_factor ; ++x1 )
                                        {
                                            avgRed   += pixel[0] ;
                                            avgGreen += pixel[1] ;
                                            avgBlue  += pixel[2] ;

                                            pixel += BPP/8;

                                            pixel_count++;
                                        }
                                        y_offset += source.width ;
                                    }

                                    if(0 == pixel_count)                // Protect
                                        pixel_count = 1;
                                    
                                    target_data[0] = avgRed / pixel_count;     // >> scounter;
                                    target_data[1] = avgGreen / pixel_count;   // >> scounter;
                                    target_data[2] = avgBlue / pixel_count;    // >> scounter;
                                    target_data += BPP/8;
                              }
                              else
                              {
                                    target_data[0] = 0;
                                    target_data[1] = 0;
                                    target_data[2] = 0;
                                    target_data += BPP/8;
                              }

                        }  // for x

                  }  // for y

            }           // SCALE_BILINEAR

            else if (scale_type == RENDER_LODEF)
            {
                        int get_bits_submap = 1;

                        int scaler = 16;

                        if(source.width > 32767)                  // High underscale can exceed signed math bits
                              scaler = 8;

                        int wb_size = (Size_X) * ((/*Factor +*/ 1) * 2) * BPP/8 ;
                        s_data = (unsigned char *) malloc( wb_size ); // work buffer

                        long x_delta = (source.width<<scaler) / target_width;
                        long y_delta = (source.height<<scaler) / target_height;

                        int y = dest.y;                // starting here
                        long ys = dest.y * y_delta;

                        while ( y < dest.y + dest.height)
                        {
                        //    Read 1 line at the right place from the source

                              wxRect s1;
                              s1.x = 0;
                              s1.y = source.y + (ys >> scaler);
                              s1.width = Size_X;
                              s1.height = 1;
                              GetChartBits(s1, s_data, get_bits_submap);

                              target_data = data + (y * dest_line_length/*dest_stride * BPP/8*/) + (dest.x * BPP / 8);

                              long x = (source.x << scaler) + (dest.x * x_delta);
                              long sizex16 = Size_X << scaler;
                              int xt = dest.x;

                              while((xt < dest.x + dest.width) && (x < 0))
                              {
                                    target_data[0] = 0;
                                    target_data[1] = 0;
                                    target_data[2] = 0;

                                    target_data += BPP/8;
                                    x += x_delta;
                                    xt++;
                              }

                              while ((xt < dest.x + dest.width) && ( x < sizex16))
                              {

                                    unsigned char* src_pixel = &s_data[(x>>scaler)*BPP/8];

                                    target_data[0] = src_pixel[0];
                                    target_data[1] = src_pixel[1];
                                    target_data[2] = src_pixel[2];

                                    target_data += BPP/8;
                                    x += x_delta;
                                    xt++;
                              }

                              while(xt < dest.x + dest.width)
                              {
                                    target_data[0] = 0;
                                    target_data[1] = 0;
                                    target_data[2] = 0;

                                    target_data += BPP/8;
                                    xt++;
                              }

                              y++;
                              ys += y_delta;
                        }

            }     // SCALE_SUBSAMP

      }
      else  //factor < 1, overzoom
      {
            int i=0;
            int j=0;
            unsigned char *target_line_start = NULL;
            unsigned char *target_data_x = NULL;
            int y_offset = 0;


#ifdef __WXGTK__
            sigaction(SIGSEGV, NULL, &sa_all_previous);             // save existing action for this signal

            struct sigaction temp;
            sigaction(SIGSEGV, NULL,  &temp);             // inspect existing action for this signal

            temp.sa_handler = catch_signals_chart ;     // point to my handler
            sigemptyset(&temp.sa_mask);                 // make the blocking set
                                                        // empty, so that all
                                                        // other signals will be
                                                        // unblocked during my handler
            temp.sa_flags = 0;
            sigaction(SIGSEGV, &temp, NULL);

            if(sigsetjmp(env_chart, 1))                 //  Something in the below code block faulted....
            {
                  sigaction(SIGSEGV, &sa_all_previous, NULL);        // reset signal handler

                  wxString msg;
                  msg.Printf(_T("   Caught SIGSEGV on GetandScaleData, Factor < 1"));
                  wxLogMessage(msg);

                  msg.Printf(_T("   m_raster_scale_factor:  %g   source.width: %d  dest.y: %d dest.x: %d dest.width: %d  dest.height: %d "),
                             m_raster_scale_factor, source.width, dest.y, dest.x, dest.width, dest.height);
                  wxLogMessage(msg);

                  msg.Printf(_T("   i: %d  j: %d dest_stride: %d  target_line_start: %p  target_data_x:  %p  y_offset: %d"),
                              i, j, dest_stride, target_line_start, target_data_x, y_offset);
                  wxLogMessage(msg);

                  free(s_data);
                  return true;

            }

            else
#endif
            {



                  double xd, yd;
                  latlong_to_chartpix(m_vp_render_last.clat, m_vp_render_last.clon, xd, yd);
                  double xrd = xd - (m_vp_render_last.pix_width  * m_raster_scale_factor / 2);
                  double yrd = yd - (m_vp_render_last.pix_height * m_raster_scale_factor / 2);
                  double x_vernier = (xrd - wxRound(xrd));
                  double y_vernier = (yrd - wxRound(yrd));
                  int x_vernier_i =  (int)wxRound(x_vernier / m_raster_scale_factor);
                  int y_vernier_i =  (int)wxRound(y_vernier / m_raster_scale_factor);

                  //    Seems safe enough to read all the required data
                  //    Although we must adjust (increase) temporary allocation for negative source.x
                  //    and for vernier
                  int sx = wxMax(source.x, 0);
                  s_data = (unsigned char *) malloc( (sx + source.width + 2) * (source.height + 2) * BPP/8 );

                  wxRect vsource = source;
                  vsource.height += 2;                // get more bits to allow for vernier
                  vsource.width += 2;
                  vsource.x -= 1;
                  vsource.y -= 1;

                  GetChartBits(vsource, s_data, 1);
                  unsigned char *source_data =  s_data;

                  j = dest.y;

                  while(j < dest.y + dest.height )
                  {
                        y_offset = (int)((j - y_vernier_i) * m_raster_scale_factor) * vsource.width;        // into the source data

                        target_line_start = target_data + (j * dest_line_length /*dest_stride * BPP / 8*/);
                        target_data_x = target_line_start + ((dest.x) * BPP / 8);

                        i = dest.x;

                        // Check data bounds to be sure of not overrunning the upstream buffer
                        if( (target_data_x + ( dest.width * BPP/8)) > (target_data + data_size) ) {
                            j = dest.y + dest.height;
                        }
                        else{ 
                            while( i < dest.x + dest.width ){
                                memcpy( target_data_x,
                                            source_data + BPP/8*(y_offset + (int)((i + x_vernier_i) * m_raster_scale_factor)),
                                        BPP/8 );
                                                            
                                target_data_x += BPP/8;

                                i++;
                            }
                        }
                        
                        j++;
                  }
            }
#ifdef __WXGTK__
            sigaction(SIGSEGV, &sa_all_previous, NULL);        // reset signal handler
#endif

      }

      free(s_data);

      return true;

}



bool ChartBaseBSB::GetChartBits(wxRect& source, unsigned char *pPix, int sub_samp)
{
    wxCriticalSectionLocker locker(m_critSect);
    
      int iy;
#define FILL_BYTE 0

//    Decode the KAP file RLL stream into image pPix

      unsigned char *pCP;
      pCP = pPix;

      iy = source.y;

      while (iy < source.y + source.height)
      {
            if((iy >= 0) && (iy < Size_Y))
            {
                    if(source.x >= 0)
                    {
                            if((source.x + source.width) > Size_X)
                            {
                                if((Size_X - source.x) < 0)
                                        memset(pCP, FILL_BYTE, source.width  * BPP/8);
                                else
                                {

                                        BSBGetScanline( pCP,  iy, source.x, Size_X, sub_samp);
                                        memset(pCP + (Size_X - source.x) * BPP/8, FILL_BYTE,
                                               (source.x + source.width - Size_X) * BPP/8);
                                }
                            }
                            else
                                BSBGetScanline( pCP, iy, source.x, source.x + source.width, sub_samp);
                    }
                    else
                    {
                            if((source.width + source.x) >= 0)
                            {
                                // Special case, black on left side
                                //  must ensure that (black fill length % sub_samp) == 0

                                int xfill_corrected = -source.x + (source.x % sub_samp);    //+ve
                                memset(pCP, FILL_BYTE, (xfill_corrected * BPP/8));
                                BSBGetScanline( pCP + (xfill_corrected * BPP/8),  iy, 0,
                                        source.width + source.x , sub_samp);

                            }
                            else
                            {
                                memset(pCP, FILL_BYTE, source.width  * BPP/8);
                            }
                    }
            }

            else              // requested y is off chart
            {
                  memset(pCP, FILL_BYTE, source.width  * BPP/8);

            }

            pCP += source.width * BPP/8 * sub_samp;

            iy += sub_samp;
      }     // while iy


      return true;
}






//-----------------------------------------------------------------------------------------------
//    BSB File Read Support
//-----------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------
//    ReadBSBHdrLine
//
//    Read and return count of a line of BSB header file
//-----------------------------------------------------------------------------------------------

int ChartBaseBSB::ReadBSBHdrLine(wxInputStream* ifs, char* buf, int buf_len_max)

{
      char  read_char;
      char  cr_test;
      int   line_length = 0;
      char  *lbuf;

      lbuf = buf;


      while( !ifs->Eof() && line_length < buf_len_max )
      {
            read_char = ifs->GetC();
            if(0x1A == read_char)
            {
                  ifs->Ungetch( read_char );
                        return(0);
            }

            if(0 == read_char)            // embedded erroneous unicode character?
                  read_char = 0x20;

            //    Manage continued lines
            if( read_char == 10 || read_char == 13 )
            {

                  //    Check to see if there is an extra CR
                  cr_test = ifs->GetC( );
                  if(cr_test == 13)
                        cr_test = ifs->GetC( );             // skip any extra CR

                  if( cr_test != 10 && cr_test != 13 )
                        ifs->Ungetch( cr_test );
                  read_char = '\n';
            }

      //    Look for continued lines, indicated by ' ' in first position
            if( read_char == '\n' )
            {
                  cr_test = 0;
                  cr_test = ifs->GetC( );

                  if( cr_test != ' ' )
                        {
                        ifs->Ungetch( cr_test );
                        *lbuf = '\0';
                        return line_length;
                        }

      //    Merge out leading spaces
                  while( cr_test == ' ' )
                        cr_test = ifs->GetC( );
                  ifs->Ungetch( cr_test );

      //    Add a comma
                  *lbuf = ',';
                  lbuf++;
            }

            else
            {
                  *lbuf = read_char;
                  lbuf++;
                  line_length++;
            }

      }     // while


      // Terminate line
      if(line_length)
          *(lbuf-1) = '\0';

      return line_length;
}


//-----------------------------------------------------------------------
//    Scan a BSB Scan Line from raw data
//      Leaving stream pointer at start of next line
//-----------------------------------------------------------------------
int   ChartBaseBSB::BSBScanScanline(wxInputStream *pinStream )
{
      int nLineMarker, nValueShift, iPixel = 0;
      unsigned char byValueMask, byCountMask;
      unsigned char byNext;
      int coffset;

//      if(1)
      {
//      Read the line number.
            nLineMarker = 0;
            do
            {
                  byNext = pinStream->GetC();
                  nLineMarker = nLineMarker * 128 + (byNext & 0x7f);
            } while( (byNext & 0x80) != 0 );

//      Setup masking values.
            nValueShift = 7 - nColorSize;
            byValueMask = (((1 << nColorSize)) - 1) << nValueShift;
            byCountMask = (1 << (7 - nColorSize)) - 1;

//      Read and simulate expansion of runs.

            while( ((byNext = pinStream->GetC()) != 0 ) && (iPixel < Size_X))
            {

                  int   nPixValue;
                  int nRunCount;
                  nPixValue = (byNext & byValueMask) >> nValueShift;

                  nRunCount = byNext & byCountMask;

                  while( (byNext & 0x80) != 0 )
                  {
                        byNext = pinStream->GetC();
                        nRunCount = nRunCount * 128 + (byNext & 0x7f);
                  }

                  if( iPixel + nRunCount + 1 > Size_X )
                        nRunCount = Size_X - iPixel - 1;


//          Store nPixValue in the destination
//                  memset(pCL, nPixValue, nRunCount+1);
//                  pCL += nRunCount+1;
                  iPixel += nRunCount+1;
            }
            coffset = pinStream->TellI();

      }


      return nLineMarker;
}
//      MSVC compiler makes a bad decision about when to inline (or not) some intrinsics, like memset().
//      So,...
//      Here is a little hand-crafted memset() substitue for known short strings.
//      It will be inlined by MSVC compiler using /02 settings 

inline void memset_short(unsigned char *dst, unsigned char cbyte, int count)
{
#ifdef __MSVC__
    __asm {
        pushf                           // save Direction flag
        cld                             // set direction "up"
        mov edi, dst
        mov ecx, count
        mov al, cbyte
        rep stosb
        popf
    }
#else    
    memset(dst, cbyte, count);
#endif    
}
// could use a larger value for slightly less ram but slower random access,
// this is chosen as it is also the opengl tile size so should work well
#define TILE_SIZE 512

//#define USE_OLD_CACHE  // removed this (and simplify code below) once the new method is verified
//#define PRINT_TIMINGS  // enable for profiling

#ifdef PRINT_TIMINGS
class OCPNStopWatch
{
    public:
        OCPNStopWatch() { Reset(); }
        void Reset() { clock_gettime(CLOCK_REALTIME, &tp); }

    double Time() {
        timespec tp_end;
        clock_gettime(CLOCK_REALTIME, &tp_end);
        return (tp_end.tv_sec - tp.tv_sec) * 1.e3 + (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
    }

private:
    timespec tp;
};
#endif

#define FAIL \
    do { \
      free(pt->pTileOffset); \
      pt->pTileOffset = NULL; \
      free(pt->pPix); \
      pt->pPix = NULL; \
      pt->bValid = false; \
      return 0; \
    } while(0)

//-----------------------------------------------------------------------
//    Get a BSB Scan Line Using Cache and scan line index if available
//-----------------------------------------------------------------------
int   ChartBaseBSB::BSBGetScanline( unsigned char *pLineBuf, int y, int xs, int xl, int sub_samp)

{
      unsigned char *prgb = pLineBuf;
      int nValueShift, iPixel = 0;
      unsigned char byValueMask, byCountMask;
      unsigned char byNext;
      CachedLine *pt = NULL, cached_line;
      unsigned char *pCL;
      int rgbval;
      unsigned char *lp;
      register int ix = xs;

      if(bUseLineCache && pLineCache)
      {
//    Is the requested line in the cache, and valid?
          pt = &pLineCache[y];
      } else {
          pt = &cached_line;
          pt->bValid = false;
      }

#ifdef PRINT_TIMINGS
      OCPNStopWatch sw;
      static double ttime;
      static int cnt;
      cnt++;
#endif

      if(!pt->bValid) // not valid, allocate
      {
          int thisline_size = pline_table[y+1] - pline_table[y] ;

#ifdef USE_OLD_CACHE
          pt->pPix = (unsigned char *)malloc(Size_X);
#else
          pt->pTileOffset = (TileOffsetCache *)calloc(sizeof(TileOffsetCache)*(Size_X/TILE_SIZE + 1), 1);
          pt->pPix = (unsigned char *)malloc(thisline_size);
#endif
          if(pline_table[y] == 0 || pline_table[y+1] == 0)
              FAIL;

          // as of 2015, in wxWidgets buffered streams don't test for a zero seek
          // so we check here to possibly avoid this seek with a measured performance gain
          if(ifs_bitmap->TellI() != pline_table[y] &&
             wxInvalidOffset == ifs_bitmap->SeekI(pline_table[y], wxFromStart))
              FAIL;

#ifdef USE_OLD_CACHE
          if(thisline_size > ifs_bufsize)
          {
              unsigned char * tmp = ifs_buf;
              if(!(ifs_buf = (unsigned char *)realloc(ifs_buf, thisline_size))) {
                  free(tmp);
                  FAIL;
              }
              ifs_bufsize = thisline_size;
          }

          lp = ifs_buf;
#else
          lp = pt->pPix;
#endif
          ifs_bitmap->Read(lp, thisline_size);

#ifdef USE_OLD_CACHE
          pCL = pt->pPix;
#else
          if(!bUseLineCache) {
              ix = 0;
              //      skip the line number.
              do byNext = *lp++; while( (byNext & 0x80) != 0 );
              goto nocachestart;
          }
          pCL = ifs_buf;

          if(Size_X > ifs_bufsize)
          {
              unsigned char * tmp = ifs_buf;
              if(!(ifs_buf = (unsigned char *)realloc(ifs_buf, Size_X))) {
                  free(tmp);
                  FAIL;
              }
              ifs_bufsize = Size_X;
          }
#endif
          //    At this point, the unexpanded, raw line is at *lp, and the expansion destination is pCL
          
          //      skip the line number.
          do byNext = *lp++; while( (byNext & 0x80) != 0 );

          //      Setup masking values.
          nValueShift = 7 - nColorSize;
          byValueMask = (((1 << nColorSize)) - 1) << nValueShift;
          byCountMask = (1 << (7 - nColorSize)) - 1;

          //      Read and expand runs.
          unsigned int iPixel = 0;

#ifndef USE_OLD_CACHE
          pt->pTileOffset[0].offset = lp - pt->pPix;
          pt->pTileOffset[0].pixel = 0;
          unsigned int tileindex = 1, nextTile = TILE_SIZE;
#endif
          unsigned int nRunCount;
          unsigned char *end = pt->pPix+thisline_size;
          while(iPixel < (unsigned int)Size_X)
#ifdef USE_OLD_CACHE
          {
              nPixValue = (byNext & byValueMask) >> nValueShift;

              nRunCount = byNext & byCountMask;

              while( (byNext & 0x80) != 0 )
              {
                  byNext = *lp++;
                  nRunCount = nRunCount * 128 + (byNext & 0x7f);
              }

              nRunCount++;

              if( iPixel + nRunCount > (unsigned int)Size_X ) // protection against corrupt data
                  nRunCount = nRunCount - iPixel;

              //          Store nPixValue in the destination
              memset_short(pCL + iPixel, nPixValue, nRunCount);
              iPixel += nRunCount;
          }
#else
          // build tile offset table for faster random access
          {
              byNext = *lp++;
              unsigned char *offset = lp - 1;
              if(byNext == 0 || lp == end) {
                  // finished early...corrupt?
                  while(tileindex < (unsigned int)Size_X/TILE_SIZE + 1) {
                      pt->pTileOffset[tileindex].offset = pt->pTileOffset[0].offset;
                      pt->pTileOffset[tileindex].pixel = 0;
                      tileindex++;
                  }
                  break;
              }

              nRunCount = byNext & byCountMask;

              while( (byNext & 0x80) != 0 )
              {
                  byNext = *lp++;
                  nRunCount = nRunCount * 128 + (byNext & 0x7f);
              }

              nRunCount++;

              if( iPixel + nRunCount > (unsigned int)Size_X ) // protection against corrupt data
                  nRunCount = Size_X - iPixel;

              while( iPixel + nRunCount > nextTile) {
                  pt->pTileOffset[tileindex].offset = offset - pt->pPix;
                  pt->pTileOffset[tileindex].pixel = iPixel;
                  tileindex++;
                  nextTile += TILE_SIZE;
              }
              iPixel += nRunCount;
          }
#endif

          pt->bValid = true;
      }

#if 0
      //    Here is some test code, using full RGB line buffers in LineCache
      //    instead of pallete dereferencing for every access....
      //    Uses lots of memory, needs ColorScheme considerations
      if(pt->pRGB == NULL)
      {
            pt->pRGB = (unsigned char *)malloc(Size_X * BPP/8);

            ix = 0;
            unsigned char *prgb = pt->pRGB;           // destination
            unsigned char *pCL = xtemp_line;          // line of pallet pointers

            while(ix < Size_X-1)
            {
                  unsigned char cur_by = *pCL;
                  rgbval = (int)(pPalette[cur_by]);
                  while((ix < Size_X-1))
                  {
                        if(cur_by != *pCL)
                              break;
                        *((int *)prgb) = rgbval;
                        prgb += BPP/8 ;
                        pCL ++;
                        ix  ++;
                  }

                  // Get the last pixel explicitely

                  unsigned char *pCLast = xtemp_line + (Size_X - 1);
                  unsigned char *prgb_last = pt->pRGB + ((Size_X - 1)) * BPP/8;

                  rgbval = (int)(pPalette[*pCLast]);        // last pixel
                  unsigned char a = rgbval & 0xff;
                  *prgb_last++ = a;
                  a = (rgbval >> 8) & 0xff;
                  *prgb_last++ = a;
                  a = (rgbval >> 16) & 0xff;
                  *prgb_last = a;

            }
      }

      if(pt->pRGB)
      {
            unsigned char *ps = pt->pRGB + (xs * BPP/8);
            int len = wxMin((xl - xs), (Size_X - xs));
            memmove(pLineBuf, ps, len * BPP/8);
            return 1;
      }
#endif

//          Line is valid, de-reference thru proper pallete directly to target

      if(xl > Size_X)
            xl = Size_X;

#ifdef USE_OLD_CACHE
      pCL = pt->pPix + xs;

      //    Optimization for most usual case
      if((BPP == 24) && (1 == sub_samp))
      {
            ix = xs;
            while(ix < xl-1)
            {
                  unsigned char cur_by = *pCL;
                  rgbval = (int)(pPalette[cur_by]);
                  while((ix < xl-1))
                  {
                        if(cur_by != *pCL)
                              break;
                        *((int *)prgb) = rgbval;
                        prgb += 3;
                        pCL ++;
                        ix  ++;
                  }
            }
      }
      else
      {
            int dest_inc_val_bytes = (BPP/8) * sub_samp;
            ix = xs;
            while(ix < xl-1)
            {
                  unsigned char cur_by = *pCL;
                  rgbval = (int)(pPalette[cur_by]);
                  while((ix < xl-1))
                  {
                        if(cur_by != *pCL)
                              break;
                        *((int *)prgb) = rgbval;
                        prgb+=dest_inc_val_bytes ;
                        pCL += sub_samp;
                        ix  += sub_samp;
                  }
            }
      }

// Get the last pixel explicitely
//  irrespective of the sub_sampling factor

      if(xs < xl-1)
      {
        unsigned char *pCLast = pt->pPix + (xl - 1);
        unsigned char *prgb_last = pLineBuf + ((xl - 1)-xs) * BPP/8;

        rgbval = (int)(pPalette[*pCLast]);        // last pixel
        unsigned char a = rgbval & 0xff;
        *prgb_last++ = a;
        a = (rgbval >> 8) & 0xff;
        *prgb_last++ = a;
        a = (rgbval >> 16) & 0xff;
        *prgb_last = a;
      }
#else
      {
          int tileindex = xs / TILE_SIZE;
          int tileoffset = pt->pTileOffset[tileindex].offset;

          lp = pt->pPix + tileoffset;
          ix = pt->pTileOffset[tileindex].pixel;
      }

nocachestart:
      unsigned int i = 0;

      nValueShift = 7 - nColorSize;
      byValueMask = (((1 << nColorSize)) - 1) << nValueShift;
      byCountMask = (1 << (7 - nColorSize)) - 1;
      int nPixValue = 0; // satisfy stupid compiler warning
      bool bLastPixValueValid = false;

      while(ix < xl - 1 ) {
          byNext = *lp++;

          nPixValue = (byNext & byValueMask) >> nValueShift;
          unsigned int nRunCount;

          if(byNext == 0)
              nRunCount = xl - ix; // corrupted chart, just run to the end
          else {
              nRunCount = byNext & byCountMask;
              while( (byNext & 0x80) != 0 )
              {
                  byNext = *lp++;
                  nRunCount = nRunCount * 128 + (byNext & 0x7f);
              }

              nRunCount++;
          }

          if(ix < xs) {
              if(ix + nRunCount <= (unsigned int)xs) {
                  ix += nRunCount;
                  continue;
              }
              nRunCount -= xs - ix;
              ix = xs;
          }

          if(ix + nRunCount >= (unsigned int)xl) {
              nRunCount = xl - 1 - ix;
              bLastPixValueValid = true;
          }

          rgbval = (int)(pPalette[nPixValue]);

          //    Optimization for most usual case
// currently this is the only case possible...
//          if((BPP == 24) && (1 == sub_samp))
          {
              int count = nRunCount;
              if( count < 16 ) {
                  // for short runs, use simple loop
                  while(count--) {
                      *(uint32_t*)prgb = rgbval;
                      prgb += 3;
                  } 
              } else if(rgbval == 0 || rgbval == 0xffffff) {
                  // optimization for black or white (could work for any gray too)
                  memset(prgb, rgbval, nRunCount*3);
                  prgb += nRunCount*3;
              } else {
                  // note: this may not be optimal for all processors and compilers
                  // I optimized for x86_64 using gcc with -O3
                  // it is probably possible to gain even faster performance by ensuring alignment
                  // to 16 or 32byte boundary (depending on processor) then using inline assembly

#ifdef ARMHF          
//  ARM needs 8 byte alignment for *(uint64_T *x) = *(uint64_T *y)
//  because the compiler will (probably) use the ldrd/strd instuction pair.
//  So, advance the prgb pointer until it is 8-byte aligned,
//  and then carry on if enough bytes are left to process as 64 bit elements
                  
                  if((long)prgb & 7){
                    while(count--) {
                        *(uint32_t*)prgb = rgbval;
                        prgb += 3;
                        if( !((long)prgb & 7) ){
                            if(count >= 8)
                                break;
                        }
                    }
                  }
#endif                  
                  

                  // fill first 24 bytes
                  uint64_t *b = (uint64_t*)prgb;
                  for(int i=0; i < 8; i++) {
                      *(uint32_t*)prgb = rgbval;
                      prgb += 3;
                  }
                  count -= 8;

                  // fill in blocks of 24 bytes
                  uint64_t *y = (uint64_t*)prgb;
                  int count_d8 = count >> 3;
                  prgb += 24*count_d8;
                  while(count_d8--) {
                      *y++ = b[0];
                      *y++ = b[1];
                      *y++ = b[2];
                  }

                  // fill remaining bytes
                  int rcount = count & 0x7;
                  while(rcount--) {
                      *(uint32_t*)prgb = rgbval;
                      prgb += 3;
                  }
              }
          }
#if 0
          else {
              int dest_inc_val_bytes = (BPP/8) * sub_samp;
              for(;i<nRunCount; i+=sub_samp) {
                  *(uint32_t*)prgb = rgbval;
                  prgb+=dest_inc_val_bytes ;
              }
              i -= nRunCount;
          }
#endif

          ix += nRunCount;
      }

// Get the last pixel explicitely
//  irrespective of the sub_sampling factor

    if(ix < xl) {
        if(!bLastPixValueValid) {
            byNext = *lp++;
            nPixValue = (byNext & byValueMask) >> nValueShift;
        }
        rgbval = (int)(pPalette[nPixValue]);        // last pixel
        unsigned char a = rgbval & 0xff;

        *prgb++ = a;
        a = (rgbval >> 8) & 0xff;
        *prgb++ = a;
        a = (rgbval >> 16) & 0xff;
        *prgb = a;
    }
#endif

#ifdef PRINT_TIMINGS
    ttime += sw.Time();

    if(cnt == 500000) {
        static int d;
        printf("cache time: %d %f\n", d, ttime / 1000.0);
        cnt = 0;
        d++;
//        ttime = 0;
    }
#endif

    if(!bUseLineCache) {
#ifndef USE_OLD_CACHE
        free(pt->pTileOffset);
#endif
        free(pt->pPix);
    }

    return 1;
}



int  *ChartBaseBSB::GetPalettePtr(BSB_Color_Capability color_index)
{
      if(pPalettes[color_index])
      {

            if(palette_direction == PaletteFwd)
                  return (int *)(pPalettes[color_index]->FwdPalette);
            else
                  return (int *)(pPalettes[color_index]->RevPalette);
      }
      else
            return NULL;
 }


PaletteDir ChartBaseBSB::GetPaletteDir(void)
 {
  // make a pixel cache
       PixelCache *pc = new PixelCache(4,4,BPP);
       RGBO r = pc->GetRGBO();
       delete pc;

       if(r == RGB)
             return PaletteFwd;
       else
             return PaletteRev;
 }

bool ChartBaseBSB::AnalyzeSkew(void)
{
    double lonmin = 1000;
    double lonmax = -1000;
    double latmin = 90.;
    double latmax = -90.;
    
    int plonmin = 100000;
    int plonmax = 0;
    int platmin = 100000;
    int platmax = 0;
    int nlonmin, nlonmax, nlatmax, nlatmin;
    nlonmin =0; nlonmax=0; nlatmax=0; nlatmin=0;
    
    if(0 == nRefpoint)                  // bad chart georef...
            return (1);
    
    for(int n=0 ; n<nRefpoint ; n++)
    {
        //    Longitude
        if(pRefTable[n].lonr > lonmax)
        {
            lonmax = pRefTable[n].lonr;
            plonmax = (int)pRefTable[n].xr;
            nlonmax = n;
        }
        if(pRefTable[n].lonr < lonmin)
        {
            lonmin = pRefTable[n].lonr;
            plonmin = (int)pRefTable[n].xr;
            nlonmin = n;
        }
        
        //    Latitude
        if(pRefTable[n].latr < latmin)
        {
            latmin = pRefTable[n].latr;
            platmin = (int)pRefTable[n].yr;
            nlatmin = n;
        }
        if(pRefTable[n].latr > latmax)
        {
            latmax = pRefTable[n].latr;
            platmax = (int)pRefTable[n].yr;
            nlatmax = n;
        }
    }

    //    Special case for charts which cross the IDL
    if((lonmin * lonmax) < 0)
    {
        if(pRefTable[nlonmin].xr > pRefTable[nlonmax].xr)
        {
            //    walk the reference table and add 360 to any longitude which is < 0
            for(int n=0 ; n<nRefpoint ; n++)
            {
                if(pRefTable[n].lonr < 0.0)
                    pRefTable[n].lonr += 360.;
            }
            
            //    And recalculate the  min/max
            lonmin = 1000;
            lonmax = -1000;
            
            for(int n=0 ; n<nRefpoint ; n++)
            {
                //    Longitude
                if(pRefTable[n].lonr > lonmax)
                {
                    lonmax = pRefTable[n].lonr;
                    plonmax = (int)pRefTable[n].xr;
                    nlonmax = n;
                }
                if(pRefTable[n].lonr < lonmin)
                {
                    lonmin = pRefTable[n].lonr;
                    plonmin = (int)pRefTable[n].xr;
                    nlonmin = n;
                }
                
                //    Latitude
                if(pRefTable[n].latr < latmin)
                {
                    latmin = pRefTable[n].latr;
                    platmin = (int)pRefTable[n].yr;
                    nlatmin = n;
                }
                if(pRefTable[n].latr > latmax)
                {
                    latmax = pRefTable[n].latr;
                    platmax = (int)pRefTable[n].yr;
                    nlatmax = n;
                }
            }
        }
    }
    
    
    
    //  Find the two REF points that are farthest apart
    double dist_max = 0.;
    int imax = 0;
    int jmax = 0;
    
    for(int i=0 ; i<nRefpoint ; i++)
    {
        for(int j=i+1 ; j < nRefpoint ; j++)
        {
            double dx = pRefTable[i].xr - pRefTable[j].xr;
            double dy = pRefTable[i].yr - pRefTable[j].yr;
            double dist = (dx * dx) + (dy * dy);
            if(dist > dist_max)
            {
                dist_max = dist;
                imax = i;
                jmax = j;
            }
        }
    }
 
    double apparent_skew =  0;
 
    if(m_projection == PROJECTION_MERCATOR)
    {
        double easting0, easting1, northing0, northing1;
        //  Get the Merc projection of the two REF points
        toSM_ECC(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, m_proj_lon, &easting0, &northing0);
        toSM_ECC(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, m_proj_lon, &easting1, &northing1);
     
        double skew_proj = atan2( (easting1-easting0), (northing1 - northing0) ) * 180./PI;
        double skew_points = atan2( (pRefTable[jmax].yr - pRefTable[imax].yr), (pRefTable[jmax].xr - pRefTable[imax].xr) ) * 180./PI; 
        
        apparent_skew =  skew_points - skew_proj + 90.;
        
        // normalize to +/- 180.
        if(fabs(apparent_skew) > 180.){
            if(apparent_skew < 0.)
                apparent_skew += 360.;
            else
                apparent_skew -= 360.;
        }
    }
    
    else if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
    {
        double easting0, easting1, northing0, northing1;
        //  Get the TMerc projection of the two REF points
        toTM(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, m_proj_lon, &easting0, &northing0);
        toTM(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, m_proj_lon, &easting1, &northing1);
        
        double skew_proj = atan2( (easting1-easting0), (northing1 - northing0) ) * 180./PI;
        double skew_points = atan2( (pRefTable[jmax].yr - pRefTable[imax].yr), (pRefTable[jmax].xr - pRefTable[imax].xr) ) * 180./PI; 
        
        apparent_skew =  skew_points - skew_proj + 90.;
        
        // normalize to +/- 180.
        if(fabs(apparent_skew) > 180.){
            if(apparent_skew < 0.)
                apparent_skew += 360.;
            else
                apparent_skew -= 360.;
        }

        if(fabs( apparent_skew - m_Chart_Skew ) > 2) {    // measured skew OK?
        // it may be that the projection longitude is simply wrong.
        // Check it.
            double dskew = fabs( apparent_skew - m_Chart_Skew );
            if( (m_proj_lon < lonmin) || (m_proj_lon > lonmax) ) {
                
                // Try a projection longitude that is mid-meridian in the chart.
                double tentative_proj_lon = (lonmin + lonmax)/2.;  
                
                toTM(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, tentative_proj_lon, &easting0, &northing0);
                toTM(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, tentative_proj_lon, &easting1, &northing1);
                
                skew_proj = atan2( (easting1-easting0), (northing1 - northing0) ) * 180./PI;
                skew_points = atan2( (pRefTable[jmax].yr - pRefTable[imax].yr), (pRefTable[jmax].xr - pRefTable[imax].xr) ) * 180./PI; 
                
                apparent_skew =  skew_points - skew_proj + 90.;
                
                // normalize to +/- 180.
                if(fabs(apparent_skew) > 180.){
                    if(apparent_skew < 0.)
                        apparent_skew += 360.;
                    else
                        apparent_skew -= 360.;
                }
 
                // Better?  If so, adopt the adjusted projection longitude
                if(fabs( apparent_skew - m_Chart_Skew ) < dskew) { 
                   m_proj_lon = tentative_proj_lon;
                }
            }
        }
    }
    else                        // For all other projections, assume that skew specified in header is correct
        apparent_skew = m_Chart_Skew;

    if(fabs( apparent_skew - m_Chart_Skew ) > 2) {           // measured skew is more than 2 degrees 
           m_Chart_Skew = apparent_skew;                         // different from stated skew
           
           wxString msg = _T("   Warning: Skew override on chart ");
           msg.Append(m_FullPath);
           wxString msg1;
           msg1.Printf(_T(" is %5g degrees"), apparent_skew);
           msg.Append(msg1);
           
           wxLogMessage(msg);
           
           return false;
           
    }
    
    return true;
}


int   ChartBaseBSB::AnalyzeRefpoints(bool b_testSolution)
{
      int i,n;
      double elt, elg;

//    Calculate the max/min reference points

      float lonmin = 1000;
      float lonmax = -1000;
      float latmin = 90.;
      float latmax = -90.;

      int plonmin = 100000;
      int plonmax = 0;
      int platmin = 100000;
      int platmax = 0;
      int nlonmin, nlonmax, nlatmax, nlatmin;
      nlonmin =0; nlonmax=0; nlatmax=0; nlatmin=0;

      if(0 == nRefpoint)                  // bad chart georef...
            return (1);

      for(n=0 ; n<nRefpoint ; n++)
      {
            //    Longitude
            if(pRefTable[n].lonr > lonmax)
            {
                  lonmax = pRefTable[n].lonr;
                  plonmax = (int)pRefTable[n].xr;
                  nlonmax = n;
            }
            if(pRefTable[n].lonr < lonmin)
            {
                  lonmin = pRefTable[n].lonr;
                  plonmin = (int)pRefTable[n].xr;
                  nlonmin = n;
            }

            //    Latitude
            if(pRefTable[n].latr < latmin)
            {
                  latmin = pRefTable[n].latr;
                  platmin = (int)pRefTable[n].yr;
                  nlatmin = n;
            }
            if(pRefTable[n].latr > latmax)
            {
                  latmax = pRefTable[n].latr;
                  platmax = (int)pRefTable[n].yr;
                  nlatmax = n;
            }
      }

      //    Special case for charts which cross the IDL
      if((lonmin * lonmax) < 0)
      {
            if(pRefTable[nlonmin].xr > pRefTable[nlonmax].xr)
            {
                  //    walk the reference table and add 360 to any longitude which is < 0
                  for(n=0 ; n<nRefpoint ; n++)
                  {
                        if(pRefTable[n].lonr < 0.0)
                              pRefTable[n].lonr += 360.;
                  }

                  //    And recalculate the  min/max
                  lonmin = 1000;
                  lonmax = -1000;

                  for(n=0 ; n<nRefpoint ; n++)
                  {
            //    Longitude
                        if(pRefTable[n].lonr > lonmax)
                        {
                              lonmax = pRefTable[n].lonr;
                              plonmax = (int)pRefTable[n].xr;
                              nlonmax = n;
                        }
                        if(pRefTable[n].lonr < lonmin)
                        {
                              lonmin = pRefTable[n].lonr;
                              plonmin = (int)pRefTable[n].xr;
                              nlonmin = n;
                        }

            //    Latitude
                        if(pRefTable[n].latr < latmin)
                        {
                              latmin = pRefTable[n].latr;
                              platmin = (int)pRefTable[n].yr;
                              nlatmin = n;
                        }
                        if(pRefTable[n].latr > latmax)
                        {
                              latmax = pRefTable[n].latr;
                              platmax = (int)pRefTable[n].yr;
                              nlatmax = n;
                        }
                  }
            }
      }


//          Build the Control Point Structure, etc
        cPoints.count = nRefpoint;
        if (cPoints.status)
        {
              // AnalyzeRefpoints can be called twice
              free(cPoints.tx );
              free(cPoints.ty );
              free(cPoints.lon );
              free(cPoints.lat );

              free(cPoints.pwx );
              free(cPoints.wpx );
              free(cPoints.pwy );
              free(cPoints.wpy );
        }

        cPoints.tx  = (double *)malloc(nRefpoint * sizeof(double));
        cPoints.ty  = (double *)malloc(nRefpoint * sizeof(double));
        cPoints.lon = (double *)malloc(nRefpoint * sizeof(double));
        cPoints.lat = (double *)malloc(nRefpoint * sizeof(double));

        cPoints.pwx = (double *)malloc(12 * sizeof(double));
        cPoints.wpx = (double *)malloc(12 * sizeof(double));
        cPoints.pwy = (double *)malloc(12 * sizeof(double));
        cPoints.wpy = (double *)malloc(12 * sizeof(double));
        cPoints.status = 1;

        //  Find the two REF points that are farthest apart
        double dist_max = 0.;
        int imax = 0;
        int jmax = 0;

        for(i=0 ; i<nRefpoint ; i++)
        {
              for(int j=i+1 ; j < nRefpoint ; j++)
              {
                    double dx = pRefTable[i].xr - pRefTable[j].xr;
                    double dy = pRefTable[i].yr - pRefTable[j].yr;
                    double dist = (dx * dx) + (dy * dy);
                    if(dist > dist_max)
                    {
                          dist_max = dist;
                          imax = i;
                          jmax = j;
                    }
              }
        }

        //  Georef solution depends on projection type

        if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
        {
              double easting0, easting1, northing0, northing1;
              //  Get the TMerc projection of the two REF points
              toTM(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, m_proj_lon, &easting0, &northing0);
              toTM(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math
              double dx2 =  (pRefTable[jmax].xr - pRefTable[imax].xr) *  (pRefTable[jmax].xr - pRefTable[imax].xr);
              double dy2 =  (pRefTable[jmax].yr - pRefTable[imax].yr) *  (pRefTable[jmax].yr - pRefTable[imax].yr);
              double dn2 =  (northing1 - northing0) * (northing1 - northing0);
              double de2 =  (easting1 - easting0) * (easting1 - easting0);

              m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);

              //  Set up and solve polynomial solution for pix<->east/north as projected
              // Fill the cpoints structure with pixel points and transformed lat/lon

              for(int n=0 ; n<nRefpoint ; n++)
              {
                    double easting, northing;
                    toTM(pRefTable[n].latr, pRefTable[n].lonr, m_proj_lat, m_proj_lon, &easting, &northing);

                    cPoints.tx[n] = pRefTable[n].xr;
                    cPoints.ty[n] = pRefTable[n].yr;
                    cPoints.lon[n] = easting;
                    cPoints.lat[n] = northing;
              }

        //      Helper parameters
              cPoints.txmax = plonmax;
              cPoints.txmin = plonmin;
              cPoints.tymax = platmax;
              cPoints.tymin = platmin;
              toTM(latmax, lonmax, m_proj_lat, m_proj_lon, &cPoints.lonmax, &cPoints.latmax);
              toTM(latmin, lonmin, m_proj_lat, m_proj_lon, &cPoints.lonmin, &cPoints.latmin);

              Georef_Calculate_Coefficients_Proj(&cPoints);

       }


       else if(m_projection == PROJECTION_MERCATOR)
       {


             double easting0, easting1, northing0, northing1;
              //  Get the Merc projection of the two REF points
             toSM_ECC(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, m_proj_lon, &easting0, &northing0);
             toSM_ECC(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math
//             double dx =  (pRefTable[jmax].xr - pRefTable[imax].xr);
//             double de =  (easting1 - easting0);
//             m_ppm_avg = fabs(dx / de);

             double dx2 =  (pRefTable[jmax].xr - pRefTable[imax].xr) *  (pRefTable[jmax].xr - pRefTable[imax].xr);
             double dy2 =  (pRefTable[jmax].yr - pRefTable[imax].yr) *  (pRefTable[jmax].yr - pRefTable[imax].yr);
             double dn2 =  (northing1 - northing0) * (northing1 - northing0);
             double de2 =  (easting1 - easting0) * (easting1 - easting0);

             m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);


              //  Set up and solve polynomial solution for pix<->east/north as projected
              // Fill the cpoints structure with pixel points and transformed lat/lon

             for(int n=0 ; n<nRefpoint ; n++)
             {
                   double easting, northing;
                   toSM_ECC(pRefTable[n].latr, pRefTable[n].lonr, m_proj_lat, m_proj_lon, &easting, &northing);

                   cPoints.tx[n] = pRefTable[n].xr;
                   cPoints.ty[n] = pRefTable[n].yr;
                   cPoints.lon[n] = easting;
                   cPoints.lat[n] = northing;
//                   printf(" x: %g  y: %g  east: %g  north: %g\n",pRefTable[n].xr, pRefTable[n].yr, easting, northing);
             }

        //      Helper parameters
             cPoints.txmax = plonmax;
             cPoints.txmin = plonmin;
             cPoints.tymax = platmax;
             cPoints.tymin = platmin;
             toSM_ECC(latmax, lonmax, m_proj_lat, m_proj_lon, &cPoints.lonmax, &cPoints.latmax);
             toSM_ECC(latmin, lonmin, m_proj_lat, m_proj_lon, &cPoints.lonmin, &cPoints.latmin);

             Georef_Calculate_Coefficients_Proj(&cPoints);

//              for(int h=0 ; h < 10 ; h++)
//                    printf("pix to east %d  %g\n",  h, cPoints.pwx[h]);          // pix to lon
 //             for(int h=0 ; h < 10 ; h++)
//                    printf("east to pix %d  %g\n",  h, cPoints.wpx[h]);          // lon to pix

/*
             if ((0 != m_Chart_DU ) && (0 != m_Chart_Scale))
             {
                   double m_ppm_avg1 = m_Chart_DU * 39.37 / m_Chart_Scale;
                   m_ppm_avg1 *= cos(m_proj_lat * PI / 180.);                    // correct to chart centroid

                   printf("BSB chart ppm_avg:%g ppm_avg1:%g\n", m_ppm_avg, m_ppm_avg1);
                   m_ppm_avg = m_ppm_avg1;
             }
*/
       }

       else if(m_projection == PROJECTION_POLYCONIC)
       {
             //   This is interesting
             //   On some BSB V 1.0 Polyconic charts (e.g. 14500_1, 1995), the projection parameter
             //   Which is taken to be the central meridian of the projection is of the wrong sign....

             //   We check for this case, and make a correction if necessary.....
             //   Obviously, the projection meridian should be on the chart, i.e. between the min and max longitudes....
             double proj_meridian = m_proj_lon;

             if((pRefTable[nlonmax].lonr >= -proj_meridian) && (-proj_meridian >= pRefTable[nlonmin].lonr))
                   m_proj_lon = -m_proj_lon;


             double easting0, easting1, northing0, northing1;
             //  Get the Poly projection of the two REF points
             toPOLY(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, m_proj_lon, &easting0, &northing0);
             toPOLY(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math
             double dx2 =  (pRefTable[jmax].xr - pRefTable[imax].xr) *  (pRefTable[jmax].xr - pRefTable[imax].xr);
             double dy2 =  (pRefTable[jmax].yr - pRefTable[imax].yr) *  (pRefTable[jmax].yr - pRefTable[imax].yr);
             double dn2 =  (northing1 - northing0) * (northing1 - northing0);
             double de2 =  (easting1 - easting0) * (easting1 - easting0);

             m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);

             // Sanity check
//             double ref_dist = DistGreatCircle(pRefTable[imax].latr, pRefTable[imax].lonr, pRefTable[jmax].latr, pRefTable[jmax].lonr);
//             ref_dist *= 1852;                                    //To Meters
//             double ref_dist_transform = sqrt(dn2 + de2);         //Also meters
//             double error = (ref_dist - ref_dist_transform)/ref_dist;

              //  Set up and solve polynomial solution for pix<->cartesian east/north as projected
              // Fill the cpoints structure with pixel points and transformed lat/lon

             for(int n=0 ; n<nRefpoint ; n++)
             {
                   double lata, lona;
                   lata = pRefTable[n].latr;
                   lona = pRefTable[n].lonr;

                   double easting, northing;
                   toPOLY(pRefTable[n].latr, pRefTable[n].lonr, m_proj_lat, m_proj_lon, &easting, &northing);

                   //   Round trip check for debugging....
//                   double lat, lon;
//                   fromPOLY(easting, northing, m_proj_lat, m_proj_lon, &lat, &lon);

                   cPoints.tx[n] = pRefTable[n].xr;
                   cPoints.ty[n] = pRefTable[n].yr;
                   cPoints.lon[n] = easting;
                   cPoints.lat[n] = northing;
//                   printf(" x: %g  y: %g  east: %g  north: %g\n",pRefTable[n].xr, pRefTable[n].yr, easting, northing);
             }

                     //      Helper parameters
             cPoints.txmax = plonmax;
             cPoints.txmin = plonmin;
             cPoints.tymax = platmax;
             cPoints.tymin = platmin;
             toPOLY(latmax, lonmax, m_proj_lat, m_proj_lon, &cPoints.lonmax, &cPoints.latmax);
             toPOLY(latmin, lonmin, m_proj_lat, m_proj_lon, &cPoints.lonmin, &cPoints.latmin);

             Georef_Calculate_Coefficients_Proj(&cPoints);

//              for(int h=0 ; h < 10 ; h++)
//                    printf("pix to east %d  %g\n",  h, cPoints.pwx[h]);          // pix to lon
//              for(int h=0 ; h < 10 ; h++)
//                    printf("east to pix %d  %g\n",  h, cPoints.wpx[h]);          // lon to pix

       }

       //   Any other projection had better have embedded coefficients
       else if(bHaveEmbeddedGeoref)
       {
             //   Use a Mercator Projection to get a rough ppm.
             double easting0, easting1, northing0, northing1;
              //  Get the Merc projection of the two REF points
             toSM_ECC(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, m_proj_lon, &easting0, &northing0);
             toSM_ECC(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math in x(longitude) direction

             double dx =  (pRefTable[jmax].xr - pRefTable[imax].xr);
             double de =  (easting1 - easting0);

             m_ppm_avg = fabs(dx / de);

             m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy may be poor. >>>---");
       }

       else
             m_ppm_avg = 1.0;                      // absolute fallback to prevent div-0 errors

#if 0
       // Alternate Skew verification
       ViewPort vps;
       vps.clat = pRefTable[0].latr;
       vps.clon = pRefTable[0].lonr;
       vps.view_scale_ppm = m_ppm_avg;
       vps.skew = 0.;
       vps.pix_width = 1000;
       vps.pix_height = 1000;
       
       int x1, y1, x2, y2;
       latlong_to_pix_vp(latmin, (lonmax + lonmin)/2., x1, y1, vps);
       latlong_to_pix_vp(latmax, (lonmax + lonmin)/2., x2, y2, vps);
      
       double apparent_skew = (atan2( (y2-y1), (x2-x1) ) * 180./PI) + 90.;
       if(apparent_skew < 0.)
           apparent_skew += 360;
       if(apparent_skew > 360.)
           apparent_skew -= 360;
       
       if(fabs( apparent_skew - m_Chart_Skew ) > 2) {           // measured skew is more than 2 degress different
           m_Chart_Skew = apparent_skew;
       }
#endif       

        if(!b_testSolution)
            return(0);
        
        // Do a last little test using a synthetic ViewPort of nominal size.....
        ViewPort vp;
        vp.clat = pRefTable[0].latr;
        vp.clon = pRefTable[0].lonr;
        vp.view_scale_ppm = m_ppm_avg;
        vp.skew = 0.;
        vp.pix_width = 1000;
        vp.pix_height = 1000;
//        vp.rv_rect = wxRect(0,0, vp.pix_width, vp.pix_height);
        SetVPRasterParms(vp);


        double xpl_err_max = 0;
        double ypl_err_max = 0;
        double xpl_err_max_meters, ypl_err_max_meters;
        int px, py;

        int pxref, pyref;
        pxref = (int)pRefTable[0].xr;
        pyref = (int)pRefTable[0].yr;

        for(i=0 ; i<nRefpoint ; i++)
        {
              px = (int)(vp.pix_width/2 + pRefTable[i].xr) - pxref;
              py = (int)(vp.pix_height/2 + pRefTable[i].yr) - pyref;

              vp_pix_to_latlong(vp, px, py, &elt, &elg);

              double lat_error  = elt - pRefTable[i].latr;
              pRefTable[i].ypl_error = lat_error;

              double lon_error = elg - pRefTable[i].lonr;

                    //  Longitude errors could be compounded by prior adjustment to ref points
              if(fabs(lon_error) > 180.)
              {
                    if(lon_error > 0.)
                          lon_error -= 360.;
                    else if(lon_error < 0.)
                          lon_error += 360.;
              }
              pRefTable[i].xpl_error = lon_error;

              if(fabs(pRefTable[i].ypl_error) > fabs(ypl_err_max))
                    ypl_err_max = pRefTable[i].ypl_error;
              if(fabs(pRefTable[i].xpl_error) > fabs(xpl_err_max))
                    xpl_err_max = pRefTable[i].xpl_error;

              xpl_err_max_meters = fabs(xpl_err_max * 60 * 1852.0);
              ypl_err_max_meters = fabs(ypl_err_max * 60 * 1852.0);

        }

        Chart_Error_Factor = fmax(fabs(xpl_err_max/(lonmax - lonmin)), fabs(ypl_err_max/(latmax - latmin)));
        double chart_error_meters = fmax(fabs(xpl_err_max * 60. * 1852.),
                                         fabs(ypl_err_max * 60. * 1852.));
        //      calculate a nominal pixel error
        //      Assume a modern display has about 4000 pixels/meter.
        //      Assume the chart is to be displayed at nominal printed scale
        double chart_error_pixels = chart_error_meters * 4000. / m_Chart_Scale;
        
        //        Good enough for navigation?
        int max_pixel_error = 4;
        
        if(chart_error_pixels > max_pixel_error)
        {
                    wxString msg = _("   VP Final Check: Georeference Chart_Error_Factor on chart ");
                    msg.Append(m_FullPath);
                    wxString msg1;
                    msg1.Printf(_T(" is %5g \n     nominal pixel error is: %5g"), Chart_Error_Factor, chart_error_pixels);
                    msg.Append(msg1);

                    wxLogMessage(msg);

                    m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy is poor. >>>---");
        }

        //  Try again with my calculated georef
        //  This problem was found on NOAA 514_1.KAP.  The embedded coefficients are just wrong....
        if((chart_error_pixels > max_pixel_error) && bHaveEmbeddedGeoref)
        {
              wxString msg = _("   Trying again with internally calculated georef solution ");
              wxLogMessage(msg);

              bHaveEmbeddedGeoref = false;
              SetVPRasterParms(vp);

              xpl_err_max = 0;
              ypl_err_max = 0;

              pxref = (int)pRefTable[0].xr;
              pyref = (int)pRefTable[0].yr;

              for(i=0 ; i<nRefpoint ; i++)
              {
                    px = (int)(vp.pix_width/2 + pRefTable[i].xr) - pxref;
                    py = (int)(vp.pix_height/2 + pRefTable[i].yr) - pyref;

                    vp_pix_to_latlong(vp, px, py, &elt, &elg);

                    double lat_error  = elt - pRefTable[i].latr;
                    pRefTable[i].ypl_error = lat_error;

                    double lon_error = elg - pRefTable[i].lonr;

                    //  Longitude errors could be compounded by prior adjustment to ref points
                    if(fabs(lon_error) > 180.)
                    {
                          if(lon_error > 0.)
                                lon_error -= 360.;
                          else if(lon_error < 0.)
                                lon_error += 360.;
                    }
                    pRefTable[i].xpl_error = lon_error;

                    if(fabs(pRefTable[i].ypl_error) > fabs(ypl_err_max))
                          ypl_err_max = pRefTable[i].ypl_error;
                    if(fabs(pRefTable[i].xpl_error) > fabs(xpl_err_max))
                          xpl_err_max = pRefTable[i].xpl_error;

                    xpl_err_max_meters = fabs(xpl_err_max * 60 * 1852.0);
                    ypl_err_max_meters = fabs(ypl_err_max * 60 * 1852.0);

              }

              Chart_Error_Factor = fmax(fabs(xpl_err_max/(lonmax - lonmin)), fabs(ypl_err_max/(latmax - latmin)));

              chart_error_meters = fmax(fabs(xpl_err_max * 60. * 1852.),
                                               fabs(ypl_err_max * 60. * 1852.));
              chart_error_pixels = chart_error_meters * 4000. / m_Chart_Scale;
              
        //        Good enough for navigation?
              if(chart_error_pixels > max_pixel_error)
              {
                    wxString msg = _("   VP Final Check with internal georef: Georeference Chart_Error_Factor on chart ");
                    msg.Append(m_FullPath);
                    wxString msg1;
                    msg1.Printf(_T(" is %5g\n     nominal pixel error is: %5g"), Chart_Error_Factor, chart_error_pixels);
                    msg.Append(msg1);

                    wxLogMessage(msg);

                    m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy is poor. >>>---");
              }
              else
              {
                    wxString msg = _("   Result: OK, Internal georef solution used.");

                    wxLogMessage(msg);
                    
                    m_ExtraInfo = _T("");
              }

        }


      return(0);

}

double ChartBaseBSB::AdjustLongitude(double lon)
{
    double lond = (m_LonMin + m_LonMax)/2 - lon;
    if(lond > 180)
        return lon + 360;
    else if(lond < -180)
        return lon - 360;
    return lon;
}

/*
*  Extracted from bsb_io.c - implementation of libbsb reading and writing
*
*  Copyright (C) 2000  Stuart Cunningham <stuart_hc@users.sourceforge.net>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*
*/


/**
 * generic polynomial to convert georeferenced lat/lon to char's x/y
 *
 * @param coeff list of polynomial coefficients
 * @param lon longitute or x
 * @param lat latitude or y
 *
 * @return coordinate corresponding to the coeff list
 */
static double polytrans( double* coeff, double lon, double lat )
{
    double ret = coeff[0] + coeff[1]*lon + coeff[2]*lat;
    ret += coeff[3]*lon*lon;
    ret += coeff[4]*lon*lat;
    ret += coeff[5]*lat*lat;
    ret += coeff[6]*lon*lon*lon;
    ret += coeff[7]*lon*lon*lat;
    ret += coeff[8]*lon*lat*lat;
    ret += coeff[9]*lat*lat*lat;
//    ret += coeff[10]*lat*lat*lat*lat;
//    ret += coeff[11]*lat*lat*lat*lat*lat;

//    for(int n = 0 ; n < 10 ; n++)
//          printf("  %g\n", coeff[n]);

    return ret;
}


#if 0
/**
 * converts Lon/Lat to chart's X/Y
 *
 * @param p pointer to a BSBImage structure
 * @param lon longitude (-180.0 to 180.0)
 * @param lat latitude (-180.0 to 180.0)
 * @param x  output chart X coordinate
 * @param y  output chart Y coordinate
 *
 * @return 1 on success and 0 on error
 */
extern int bsb_LLtoXY(BSBImage *p, double lon, double  lat, int* x, int* y)
{
    /* change longitude phase (CPH) */
    lon = (lon < 0) ? lon + p->cph : lon - p->cph;
    double xd = polytrans( p->wpx, lon, lat );
    double yd = polytrans( p->wpy, lon, lat );
    *x = (int)(xd + 0.5);
    *y = (int)(yd + 0.5);
    return 1;
}

/**
 * converts chart's X/Y to Lon/Lat
 *
 * @param p pointer to a BSBImage structure
 * @param x chart X coordinate
 * @param y chart Y coordinate
 * @param lon output longitude (-180.0 to 180.0)
 * @param lat output latitude (-180.0 to 180.0)
 *
 * @return 1 on success and 0 on error
 */
extern int bsb_XYtoLL(BSBImage *p, int x, int y, double* lonout, double*  latout)
{
    double lon = polytrans( p->pwx, x, y );
    lon = (lon < 0) ? lon + p->cph : lon - p->cph;
    *lonout = lon;
    *latout = polytrans( p->pwy, x, y );
    return 1;
}

#endif





