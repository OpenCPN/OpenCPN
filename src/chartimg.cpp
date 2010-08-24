/******************************************************************************
 * $Id: chartimg.cpp,v 1.57 2010/06/25 13:28:46 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  ChartBase, ChartBaseBSB and Friends
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 * $Log: chartimg.cpp,v $
 * Revision 1.57  2010/06/25 13:28:46  bdbcat
 * 624 Quick Fix
 *
 * Revision 1.56  2010/06/25 02:02:45  bdbcat
 * 624
 *
 * Revision 1.55  2010/06/24 02:00:56  bdbcat
 * 623
 *
 * Revision 1.53  2010/06/21 02:43:09  bdbcat
 * 620
 *
 * Revision 1.52  2010/06/21 02:02:09  bdbcat
 * 620
 *
 * Revision 1.51  2010/06/07 15:29:19  bdbcat
 * 607a
 *
 * Revision 1.50  2010/06/06 20:50:35  bdbcat
 * 606a
 *
 * Revision 1.49  2010/06/04 22:35:12  bdbcat
 * 604
 *
 * Revision 1.48  2010/05/29 17:32:18  bdbcat
 * 529a
 *
 * Revision 1.47  2010/05/28 15:09:08  bdbcat
 * 528a Catch SIGSEGV
 *
 * Revision 1.46  2010/05/27 18:58:47  bdbcat
 * 527a
 *
 * Revision 1.45  2010/05/23 23:09:14  bdbcat
 * Build 523a
 *
 * Revision 1.44  2010/05/19 01:07:02  bdbcat
 * Build 518
 *
 * Revision 1.43  2010/05/15 03:58:59  bdbcat
 * Build 514
 *
 * Revision 1.42  2010/05/02 03:02:41  bdbcat
 * Build 501
 *
 * Revision 1.41  2010/04/27 01:40:44  bdbcat
 * Build 426
 *
 * Revision 1.40  2010/04/15 15:49:49  bdbcat
 * Build 415.
 *
 * Revision 1.39  2010/03/29 03:28:25  bdbcat
 * 2.1.0 Beta Initial
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

#include "wx/dir.h"

#include "chartimg.h"
#include "ocpn_pixel.h"

//  Why are these not in wx/prec.h?
#include "wx/stream.h"
#include "wx/wfstream.h"
#include "wx/tokenzr.h"
#include "wx/filename.h"
#include <wx/image.h>

#include <sys/stat.h>

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>

      extern struct sigaction sa_all;
      extern struct sigaction sa_all_old;

      extern sigjmp_buf           env;                    // the context saved by sigsetjmp();

      extern void catch_signals(int signo);

#endif


// ----------------------------------------------------------------------------
// Random Prototypes
// ----------------------------------------------------------------------------

extern void *x_malloc(size_t t);
extern "C"  double     round_msvc (double flt);
extern int g_BSBImgDebug;
int b_cdebug;


CPL_CVSID("$Id: chartimg.cpp,v 1.57 2010/06/25 13:28:46 bdbcat Exp $");

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
      pcached_bitmap = NULL;

      m_depth_unit_id = DEPTH_UNIT_UNKNOWN;

      pThumbData = new ThumbData;

      m_global_color_scheme = GLOBAL_COLOR_SCHEME_RGB;

      bReadyToRender = false;

      Chart_Error_Factor = 0;

      m_Chart_Scale = 10000;              // a benign value

      m_nCOVREntries = 0;
      m_pCOVRTable = NULL;
      m_pCOVRTablePoints = NULL;

      m_EdDate.Set(1, wxDateTime::Jan, 2000);

      m_lon_datum_adjust = 0.;
      m_lat_datum_adjust = 0.;

      m_projection = PROJECTION_MERCATOR;             // default
}

ChartBase::~ChartBase()
{
      if(pcached_bitmap)
             delete pcached_bitmap;

      delete pThumbData;

      //    Free the COVR tables

      for(unsigned int j=0 ; j<(unsigned int)m_nCOVREntries ; j++)
            free( m_pCOVRTable[j] );

      free( m_pCOVRTable );
      free( m_pCOVRTablePoints );

}

int ChartBase::Continue_BackgroundHiDefRender(void)
{
      return BR_DONE_NOP;            // signal "done, no refresh"
}

// ============================================================================
// ChartDummy implementation
// ============================================================================

ChartDummy::ChartDummy()
{
      m_pBM = NULL;
      m_ChartType = CHART_TYPE_DUMMY;
      m_ChartFamily = CHART_FAMILY_UNKNOWN;


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

bool ChartDummy::UpdateThumbData(float lat, float lon)
{
      return FALSE;
}


int ChartDummy::GetNativeScale()
{
      return 22000000;
}

wxString ChartDummy::GetPubDate()
{
      return _T("");
}


void ChartDummy::InvalidateCache(void)
{
      delete m_pBM;
      m_pBM = NULL;
}

bool ChartDummy::GetChartExtent(Extent *pext)
{
    pext->NLAT = 80;
    pext->SLAT = -80;
    pext->ELON = 179;
    pext->WLON = -179;

    return true;
}


bool ChartDummy::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const wxRegion &Region, ScaleTypeEnum scale_type)
{
      return RenderViewOnDC(dc, VPoint, scale_type);
}

bool ChartDummy::RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, ScaleTypeEnum scale_type)
{
      if(m_pBM)
      {
            if((m_pBM->GetWidth() != VPoint.pix_width) || (m_pBM->GetHeight() != VPoint.pix_height))
            {
                  delete m_pBM;
                  m_pBM = NULL;
            }
      }

      if(NULL == m_pBM)
            m_pBM = new wxBitmap(VPoint.pix_width, VPoint.pix_height,-1);

      dc.SelectObject(*m_pBM);

      dc.SetBackground(*wxBLACK_BRUSH);
      dc.Clear();

      return true;
}


void ChartDummy::SetVPParms(const ViewPort &vpt)
{
}

bool ChartDummy::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed)
{
      return false;
}

bool ChartDummy::IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed)
{
      if((vp_last.clat == vp_proposed.clat)  &&
          (vp_last.clon == vp_proposed.clon) &&
          (vp_last.view_scale_ppm == vp_proposed.view_scale_ppm))
            return false;
      else
            return true;
}


void ChartDummy::GetValidCanvasRegion(const ViewPort& VPoint, wxRegion *pValidRegion)
{
      pValidRegion->Clear();
      pValidRegion->Union(0, 0, 1, 1);
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
      #define BUF_LEN_MAX 4000

      PreInit(name, init_flags, GLOBAL_COLOR_SCHEME_DAY);

      char buffer[BUF_LEN_MAX];

      ifs_hdr = new wxFileInputStream(name);          // open the file as a read-only stream

      if(!ifs_hdr->Ok())
            return INIT_FAIL_REMOVE;

      int nPlypoint = 0;
      Plypoint *pPlyTable = (Plypoint *)x_malloc(sizeof(Plypoint));

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
                  pPlyTable = (Plypoint *)realloc(pPlyTable, sizeof(Plypoint) * (nPlypoint+1));
                  pPlyTable[nPlypoint].ltp = ltp;
                  pPlyTable[nPlypoint].lnp = lnp;

                  nPlypoint++;

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
                        Chart_Skew = fcs;
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
            return INIT_FAIL_REMOVE;

      wxString NOS_Name(*pBitmapFilePath);            // take a copy

      wxDir target_dir(Path);
      wxArrayString file_array;
      int nfiles = wxDir::GetAllFiles(Path, &file_array);
      int ifile;

      pBitmapFilePath->Prepend(Path);

      wxFileName NOS_filename(*pBitmapFilePath);
      if(NOS_filename.FileExists())
      {
            ifss_bitmap = new wxFileInputStream(*pBitmapFilePath); // open the bitmap file
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

            return INIT_FAIL_REMOVE;                  // not found at all

found_uclc_file:

            delete pBitmapFilePath;                   // fix up the member element
            pBitmapFilePath = new wxString(NOS_filename.GetFullPath());
            ifss_bitmap = new wxFileInputStream(*pBitmapFilePath); // open the bitmap file
            ifs_bitmap = new wxBufferedInputStream(*ifss_bitmap);

      }           //else


      if(ifs_bitmap == NULL)
            return INIT_FAIL_REMOVE;

      if(!ifss_bitmap->Ok())
            return INIT_FAIL_REMOVE;


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
          return INIT_FAIL_REMOVE;

      if(nPlypoint < 3)
      {
            wxString msg(_("   Chart File contains less than 3 PLY points: "));
            msg.Append(m_FullPath);
            wxLogMessage(msg);

            return INIT_FAIL_REMOVE;
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
     b_cdebug = g_BSBImgDebug;

}


ChartKAP::~ChartKAP()
{
}


InitReturn ChartKAP::Init( const wxString& name, ChartInitFlag init_flags )
{
      #define BUF_LEN_MAX 4000

      int nPlypoint = 0;
      Plypoint *pPlyTable = (Plypoint *)x_malloc(sizeof(Plypoint));

      PreInit(name, init_flags, GLOBAL_COLOR_SCHEME_DAY);

      char buffer[BUF_LEN_MAX];

      ifs_hdr = new wxFileInputStream(name);          // open the Header file as a read-only stream

      if(!ifs_hdr->Ok())
            return INIT_FAIL_REMOVE;

      m_FullPath = name;
      m_Description = m_FullPath;

      ifss_bitmap = new wxFileInputStream(name); // Open again, as the bitmap
      ifs_bitmap = new wxBufferedInputStream(*ifss_bitmap);

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
            return INIT_FAIL_REMOVE;
      }



//    Read and Parse Chart Header, line by line
      ifs_hdr->SeekI(0, wxFromStart);                                         // rewind

      Size_X = Size_Y = 0;

      int done_header_parse = 0;

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
                      return INIT_FAIL_REMOVE;

                  continue;
            }


            wxString str_buf(buffer,  wxConvUTF8);
            wxCSConv iso_conv(wxT("ISO-8859-1"));                 // we will need a converter

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
                              Chart_Skew = fcs;
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

                              if(!bp_set)
                              {
                                    wxString msg(_("   Chart projection is UNKNOWN, assuming Mercator: "));
                                    msg.Append(m_FullPath);
                                    wxLogMessage(msg);
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

            else if (!strncmp(buffer, "DTM", 3))
            {
                  float fdtmlat, fdtmlon;
                  sscanf(&buffer[4], "%f,%f", &fdtmlat, &fdtmlon);
                  m_dtm_lat = fdtmlat;
                  m_dtm_lon = fdtmlon;
            }


            else if (!strncmp(buffer, "PLY", 3))
            {
                  int i;
                  float ltp,lnp;
                  sscanf(&buffer[4], "%d,%f,%f", &i, &ltp, &lnp);
                  pPlyTable = (Plypoint *)realloc(pPlyTable, sizeof(Plypoint) * (nPlypoint+1));
                  pPlyTable[nPlypoint].ltp = ltp;
                  pPlyTable[nPlypoint].lnp = lnp;

                  nPlypoint++;
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
//    BSB charts typically list publish date as xx/yy/zz, we want 19zz.
                                  if(iyear < 100)
                                  {
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


//    Validate some of the header data
      if((Size_X == 0) || (Size_Y == 0))
      {
          return INIT_FAIL_REMOVE;
      }

      if(nPlypoint < 3)
      {
            wxString msg(_("   Chart File contains less than 3 PLY points: "));
            msg.Append(m_FullPath);
            wxLogMessage(msg);

            return INIT_FAIL_REMOVE;
      }


//    Convert captured plypoint information into chart COVR structures
      m_nCOVREntries = 1;
      m_pCOVRTablePoints = (int *)malloc(sizeof(int));
      *m_pCOVRTablePoints = nPlypoint;
      m_pCOVRTable = (float **)malloc(sizeof(float *));
      *m_pCOVRTable = (float *)malloc(nPlypoint * 2 * sizeof(float));
      memcpy(*m_pCOVRTable, pPlyTable, nPlypoint * 2 * sizeof(float));
      free(pPlyTable);


      //    Adjust the PLY points to WGS84 datum
      Plypoint *ppp = (Plypoint *)GetCOVRTableHead(0);
      int cnPlypoint = GetCOVRTablenPoints(0);

      for(int u=0 ; u<cnPlypoint ; u++)
      {
            ppp->lnp += m_dtm_lon / 3600;
            ppp->ltp += m_dtm_lat / 3600;
            ppp++;
      }


      if(!SetMinMax())
            return INIT_FAIL_REMOVE;          // have to bail here

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

      pRefTable = (Refpoint *)x_malloc(sizeof(Refpoint));
      nRefpoint = 0;
      cPoints.status = 0;
      bHaveEmbeddedGeoref = false;
      n_wpx = 0;
      n_wpy = 0;
      n_pwx = 0;
      n_pwy = 0;


      bUseLineCache = true;
      Chart_Skew = 0.0;

      pPixCache = NULL;
      pPixCacheBackground = NULL;
      background_work_buffer = NULL;

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

      m_bIDLcross = false;

      m_dx = 0.;
      m_dy = 0.;
      m_proj_lat = 0.;
      m_proj_lon = 0.;
      m_proj_parameter = 0.;

      m_br_scale = -1.0;
      br_target_y = -1;
      m_br_bpending = false;


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

      if(pLineCache)
      {
            CachedLine *pt;
            for(int ylc = 0 ; ylc < Size_Y ; ylc++)
            {
                  pt = &pLineCache[ylc];
                  if(pt->pPix)
                        free (pt->pPix);
            }
            free (pLineCache);
      }



      delete pPixCache;

      delete pPixCacheBackground;
      free(background_work_buffer);


      for(int i = 0 ; i < N_BSB_COLORS ; i++)
            delete pPalettes[i];

}

//    Report recommended minimum and maximum scale values for which use of this chart is valid

double ChartBaseBSB::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom)
{
      if(b_allow_overzoom)
            return (canvas_scale_factor / m_ppm_avg) / 32;         // allow wide range overzoom overscale
      else
            return (canvas_scale_factor / m_ppm_avg) / 2;         // don't suggest too much overscale

}

double ChartBaseBSB::GetNormalScaleMax(double canvas_scale_factor, int canvas_width)
{
      return (canvas_scale_factor / m_ppm_avg) * 8.0;        // excessive underscale is slow, and unreadable
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
      for(int i = 0 ; i < N_BSB_COLORS ; i++)
      {
            if(pPalettes[i] == NULL)
            {
                opncpnPalette *pNullSubPal = new opncpnPalette;

                pNullSubPal->nFwd = pPalettes[COLOR_RGB_DEFAULT]->nFwd;        // copy the palette count
                pNullSubPal->nRev = pPalettes[COLOR_RGB_DEFAULT]->nRev;        // copy the palette count
                //  Deep copy the palette rgb tables
                free( pNullSubPal->FwdPalette );
                pNullSubPal->FwdPalette = (int *)malloc(pNullSubPal->nFwd * sizeof(int));
                memcpy(pNullSubPal->FwdPalette, pPalettes[COLOR_RGB_DEFAULT]->FwdPalette, pNullSubPal->nFwd * sizeof(int));

                free( pNullSubPal->RevPalette );
                pNullSubPal->RevPalette = (int *)malloc(pNullSubPal->nRev * sizeof(int));
                memcpy(pNullSubPal->RevPalette, pPalettes[COLOR_RGB_DEFAULT]->RevPalette, pNullSubPal->nRev * sizeof(int));

                pPalettes[i] = pNullSubPal;
            }
      }

      // Establish the palette type and default palette
      palette_direction = GetPaletteDir();

      SetColorScheme(m_global_color_scheme, false);

      //    Allocate memory for ifs file buffering
      ifs_bufsize = Size_X * 4;
      ifs_buf = (unsigned char *)x_malloc(ifs_bufsize);
      if(!ifs_buf)
            return INIT_FAIL_REMOVE;

      ifs_bufend = ifs_buf + ifs_bufsize;
      ifs_lp = ifs_bufend;
      ifs_file_offset = -ifs_bufsize;


      //    Create and load the line offset index table
      pline_table = NULL;
      pline_table = (int *)x_malloc((Size_Y+1) * sizeof(int) );               //Ugly....
      if(!pline_table)
            return INIT_FAIL_REMOVE;

      ifs_bitmap->SeekI((Size_Y+1) * -4, wxFromEnd);                 // go to Beginning of offset table
      pline_table[Size_Y] = ifs_bitmap->TellI();                     // fill in useful last table entry

      int offset;
      for(int ifplt=0 ; ifplt<Size_Y ; ifplt++)
      {
          offset = 0;
          offset += (unsigned char)ifs_bitmap->GetC() * 256 * 256 * 256;
          offset += (unsigned char)ifs_bitmap->GetC() * 256 * 256 ;
          offset += (unsigned char)ifs_bitmap->GetC() * 256 ;
          offset += (unsigned char)ifs_bitmap->GetC();

          pline_table[ifplt] = offset;
      }

      //    Try to validate the line index

      bool bline_index_ok = true;
      m_nLineOffset = 0;

      for(int iplt=0 ; iplt<Size_Y - 1 ; iplt++)
      {
            if( wxInvalidOffset == ifs_bitmap->SeekI(pline_table[iplt], wxFromStart))
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

            if(thisline_size > ifs_bufsize)
            {
                  wxString msg(_T("   ifs_bufsize too small PostInit() on chart "));
                  msg.Append(m_FullPath);
                  wxLogMessage(msg);

                  return INIT_FAIL_REMOVE;
            }

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
/*
      if(!bline_index_ok)
      {
            wxString msg(_T("   Line Index corrupt on chart "));
            msg.Append(m_FullPath);
            wxLogMessage(msg);

            wxLogMessage(_T("   Assuming chart data is otherwise OK."));
            bline_index_ok = true;
      }
*/
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
            pLineCache = (CachedLine *)x_malloc(Size_Y * sizeof(CachedLine));
            CachedLine *pt;

            for(int ylc = 0 ; ylc < Size_Y ; ylc++)
            {
                  pt = &pLineCache[ylc];
                  pt->bValid = false;
                  pt->xstart = 0;
                  pt->xlength = 1;
                  pt->pPix = NULL;        //(unsigned char *)malloc(1);
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
      else if(test_str.IsSameAs(_T("FATHOMS"), FALSE))
          m_depth_unit_id = DEPTH_UNIT_FATHOMS;
      else if(test_str.Find(_T("FATHOMS")) != wxNOT_FOUND)             // Special case for "Fathoms and Feet"
          m_depth_unit_id = DEPTH_UNIT_FATHOMS;
      else if(test_str.Find(_T("METERS")) != wxNOT_FOUND)             // Special case for "Meters and decimeters"
            m_depth_unit_id = DEPTH_UNIT_METERS;

           //    Setup the datum transform parameters
      char d_str[100];
      strncpy(d_str, m_datum_str.mb_str(), 99);
      d_str[99] = 0;

      m_datum_index = GetDatumIndex(d_str);


      //   Analyze Refpoints
      int analyze_ret_val = AnalyzeRefpoints();
      if(0 != analyze_ret_val)
            return INIT_FAIL_REMOVE;

      //    Establish defaults, may be overridden later
      m_lon_datum_adjust = (-m_dtm_lon) / 3600.;
      m_lat_datum_adjust = (-m_dtm_lat) / 3600.;

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
                        if(pt->pPix)
                        {
                              free (pt->pPix);
                              pt->pPix = NULL;
                        }
                        pt->bValid = 0;
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

      int divx = Size_X / tnx;
      int divy = Size_Y / tny;

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
      unsigned char *pLineT = (unsigned char *)x_malloc((Size_X+1) * BPP/8);


//    Scale the data quickly
      unsigned char *pPixTN = (unsigned char *)x_malloc(des_width * des_height * this_bpp/8 );

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
      retBMP = new ocpnBitmap(pPixTN, des_width, des_height, -1);
#else
      wxImage thumb_image(des_width, des_height, pPixTN, true);
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

      int pixx, pixy;


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

bool ChartBaseBSB::UpdateThumbData(float lat, float lon)
{
//    Plot the supplied Lat/Lon on the thumbnail
//  Return TRUE if the pixel location of ownship has changed

    int divx = Size_X / pThumbData->Thumb_Size_X;
    int divy = Size_Y / pThumbData->Thumb_Size_Y;

    int div_factor = __min(divx, divy);

    int pixx_test, pixy_test;


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

int ChartBaseBSB::vp_pix_to_latlong(ViewPort& vp, int pixx, int pixy, double *plat, double *plon)
{
      if(bHaveEmbeddedGeoref)
      {
            double raster_scale = GetPPM() / vp.view_scale_ppm;

            int px = (int)(pixx*raster_scale) + Rsrc.x;
            int py = (int)(pixy*raster_scale) + Rsrc.y;
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




int ChartBaseBSB::latlong_to_pix_vp(double lat, double lon, int &pixx, int &pixy, ViewPort& vp)
{
    int px, py;

    double alat, alon;

    if(bHaveEmbeddedGeoref)
    {
          double alat, alon;

          alon = lon + m_lon_datum_adjust;
          alat = lat + m_lat_datum_adjust;

          if(m_bIDLcross)
          {
                if(alon < 0.)
                      alon += 360.;
          }

          if(1)
          {
                /* change longitude phase (CPH) */
                double lonp = (alon < 0) ? alon + m_cph : alon - m_cph;
                double xd = polytrans( wpx, lonp, alat );
                double yd = polytrans( wpy, lonp, alat );
                px = (int)(xd + 0.5);
                py = (int)(yd + 0.5);


                double raster_scale = GetPPM() / vp.view_scale_ppm;

                pixx = (int)(((px - Rsrc.x) / raster_scale) + 0.5);
                pixy = (int)(((py - Rsrc.y) / raster_scale) + 0.5);

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

                int xs = (int)xc - (int)(vp.pix_width  * raster_scale / 2);
                int ys = (int)yc - (int)(vp.pix_height * raster_scale / 2);

                int pixx_p = (int)(((xd - xs) / raster_scale) + 0.5);
                int pixy_p = (int)(((yd - ys) / raster_scale) + 0.5);

//                printf("  %d  %d  %d  %d\n", pixx, pixx_p, pixy, pixy_p);

                pixx = pixx_p;
                pixy = pixy_p;

          }
          else if(m_projection == PROJECTION_MERCATOR)
          {
                //      Use Projected Polynomial algorithm

                alon = lon + m_lon_datum_adjust;
                alat = lat + m_lat_datum_adjust;

                //      Get e/n from  Projection
                xlon = alon;
                if(m_bIDLcross)
                {
                      if(xlon < 0.)
                            xlon += 360.;
                }
                toSM_ECC(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

                //      Apply poly solution to target point
                double xd = polytrans( cPoints.wpx, easting, northing );
                double yd = polytrans( cPoints.wpy, easting, northing );

                //      Apply poly solution to vp center point
                double xlonc = vp.clon;
                if(m_bIDLcross)
                {
                      if(xlonc < 0.)
                            xlonc += 360.;
                }

                toSM_ECC(vp.clat + m_lat_datum_adjust, xlonc + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
                double xc = polytrans( cPoints.wpx, easting, northing );
                double yc = polytrans( cPoints.wpy, easting, northing );

                //      Calculate target point relative to vp center
                double raster_scale = GetPPM() / vp.view_scale_ppm;

                int xs = (int)xc - (int)(vp.pix_width  * raster_scale / 2);
                int ys = (int)yc - (int)(vp.pix_height * raster_scale / 2);

                int pixx_p = (int)(((xd - xs) / raster_scale) + 0.5);
                int pixy_p = (int)(((yd - ys) / raster_scale) + 0.5);

                pixx = pixx_p;
                pixy = pixy_p;

          }
          else
         {
                toSM_ECC(lat, xlon, vp.clat, vp.clon, &easting, &northing);

                double epix = easting  * vp.view_scale_ppm;
                double npix = northing * vp.view_scale_ppm;

                double dx = epix * cos ( vp.skew ) + npix * sin ( vp.skew );
                double dy = npix * cos ( vp.skew ) - epix * sin ( vp.skew );

                pixx = ( int ) /*rint*/( ( vp.pix_width  / 2 ) + dx );
                pixy = ( int ) /*rint*/( ( vp.pix_height / 2 ) - dy );
         }
                return 0;
    }

    return 1;
}

void ChartBaseBSB::ComputeSourceRectangle(const ViewPort &vp, wxRect *pSourceRect)
{

    int pixxd, pixyd;

    //      This funny contortion is necessary to allow scale factors < 1, i.e. overzoom
    double binary_scale_factor = (round(100000 * GetPPM() / vp.view_scale_ppm)) / 100000.;

    m_raster_scale_factor = binary_scale_factor;

//    printf(" in ComputeSourceRect bsf %g  1_over_bsf %g   m_raster...:%g\n", binary_scale_factor, 1./binary_scale_factor, m_raster_scale_factor);

    if(bHaveEmbeddedGeoref)
    {

          /* change longitude phase (CPH) */
          double lonp = (vp.clon < 0) ? vp.clon + m_cph : vp.clon - m_cph;
          double xd = polytrans( wpx, lonp + m_lon_datum_adjust,  vp.clat + m_lat_datum_adjust );
          double yd = polytrans( wpy, lonp + m_lon_datum_adjust,  vp.clat + m_lat_datum_adjust );
          pixxd = (int)wxRound(xd);
          pixyd = (int)wxRound(yd);

          pSourceRect->x = pixxd - (int)wxRound(vp.pix_width  * binary_scale_factor / 2);
          pSourceRect->y = pixyd - (int)wxRound(vp.pix_height * binary_scale_factor / 2);

          pSourceRect->width =  (int)wxRound(vp.pix_width  * binary_scale_factor) ;
          pSourceRect->height = (int)wxRound(vp.pix_height * binary_scale_factor) ;

    }

    else
    {
        if(m_projection == PROJECTION_MERCATOR)
        {
                  //      Apply poly solution to vp center point
              double easting, northing;
              double xlon = vp.clon;
              if(m_bIDLcross)
              {
                    if(xlon < 0)
                        xlon += 360.;
              }
              toSM_ECC(vp.clat + m_lat_datum_adjust, xlon + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
              double xc = polytrans( cPoints.wpx, easting, northing );
              double yc = polytrans( cPoints.wpy, easting, northing );

                  //    convert screen pixels to chart pixmap relative
              pSourceRect->x = (int)(xc - (vp.pix_width / 2)*binary_scale_factor);
              pSourceRect->y = (int)(yc - (vp.pix_height / 2)*binary_scale_factor);

              pSourceRect->width =  (int)(vp.pix_width  * binary_scale_factor) ;
              pSourceRect->height = (int)(vp.pix_height * binary_scale_factor) ;

 //             printf("Compute Rsrc:  vp.clat:  %g    Rsrc.y: %d  \n", vp.clat, pSourceRect->y);

        }


        if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
        {
                  //      Apply poly solution to vp center point
              double easting, northing;
              toTM(vp.clat + m_lat_datum_adjust, vp.clon + m_lon_datum_adjust, m_proj_lat, m_proj_lon, &easting, &northing);
              double xc = polytrans( cPoints.wpx, easting, northing );
              double yc = polytrans( cPoints.wpy, easting, northing );

                  //    convert screen pixels to chart pixmap relative
              pSourceRect->x = (int)(xc - (vp.pix_width / 2)*binary_scale_factor);
              pSourceRect->y = (int)(yc - (vp.pix_height / 2)*binary_scale_factor);

              pSourceRect->width =  (int)(vp.pix_width  * binary_scale_factor) ;
              pSourceRect->height = (int)(vp.pix_height * binary_scale_factor) ;

        }
    }

//    printf("Compute Rsrc:  vp.clat:  %g  clon: %g     Rsrc.y: %d  Rsrc.x:  %d\n", vp.clat, vp.clon, pSourceRect->y, pSourceRect->x);

}


void ChartBaseBSB::SetVPParms(const ViewPort &vpt)
{
      //    Calculate the potential datum offset parameters for this viewport, if not WGS84

      if(m_datum_index == DATUM_INDEX_WGS84)
      {
            m_lon_datum_adjust = 0.;
            m_lat_datum_adjust = 0.;
      }

      else if(m_datum_index == DATUM_INDEX_UNKNOWN)
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

      }

      ComputeSourceRectangle(vpt, &Rsrc);

}

bool ChartBaseBSB::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed)
{

      ViewPort vp_save = vp_proposed;                 // save a copy

      int ret_val = 0;

      if(vp_last.bValid)
      {

                  double binary_scale_factor = GetPPM() / vp_proposed.view_scale_ppm;

                  //    We only need to adjust the VP if the cache is valid and potentially usable, i.e. the scale factor is integer...
                  //    The objective here is to ensure that the VP center falls on an exact pixel boundary within the cache

                  double dscale = fabs(binary_scale_factor - wxRound(binary_scale_factor));
                  if(b_cdebug)printf(" Adjust VP dscale: %g\n", dscale);

                  if(cached_image_ok && (binary_scale_factor > 1.0) && (fabs(binary_scale_factor - wxRound(binary_scale_factor)) < 1e-5))
                  {
                        wxRect rprop;
                        ComputeSourceRectangle(vp_proposed, &rprop);

                        int cs1d = rprop.width/vp_proposed.pix_width;

                        if(cs1d > 0)
                        {
                              double new_lat = vp_proposed.clat;
                              double new_lon = vp_proposed.clon;


                              int dx = (rprop.x - cache_rect.x) % cs1d;
                              if(dx)
                              {
                                    fromSM((double)-dx / m_ppm_avg, 0., vp_proposed.clat, vp_proposed.clon, &new_lat, &new_lon);
                                    vp_proposed.clon = new_lon;
                                    ret_val++;

                              }

                              ComputeSourceRectangle(vp_proposed, &rprop);
                              int dy = (rprop.y - cache_rect.y) % cs1d;
                              if(dy)
                              {
                                    fromSM(0, (double)dy / m_ppm_avg, vp_proposed.clat, vp_proposed.clon, &new_lat, &new_lon);
                                    vp_proposed.clat = new_lat;
                                    ret_val++;

                              }

                              if(b_cdebug)printf(" Adjust VP dx: %d  dy:%d\n", dx, dy);

                              //    Check the results...if not good(i.e. VP center is not on cache pixel boundary),
                              //    then leave the vp unmodified by restoring from the saved copy...
                              if(ret_val > 0)
                              {
                                    wxRect rprop_cor;
                                    ComputeSourceRectangle(vp_proposed, &rprop_cor);
                                    int cs2d = rprop_cor.width/vp_proposed.pix_width;
                                    int dxc = (rprop_cor.x - cache_rect.x) % cs2d;
                                    int dyc = (rprop_cor.y - cache_rect.y) % cs2d;

                                    if(b_cdebug)printf(" Adjust VP dxc: %d  dyc:%d\n", dxc, dyc);
                                    if(dxc || dyc)
                                    {
                                          vp_proposed.clat = vp_save.clat;
                                          vp_proposed.clon = vp_save.clon;
                                          ret_val = 0;
                                          if(b_cdebug)printf(" Adjust VP failed\n");
                                    }
                                    else
                                          if(b_cdebug)printf(" Adjust VP succeeded \n");

                              }

                        }
                  }
      }

      return (ret_val > 0);
}

bool ChartBaseBSB::IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed)
{
      wxRect rlast, rthis;

      ComputeSourceRectangle(vp_last, &rlast);
      ComputeSourceRectangle(vp_proposed, &rthis);

      return ((rlast != rthis) || !(IsCacheValid()) || (vp_last.view_scale_ppm != vp_proposed.view_scale_ppm));
}


void ChartBaseBSB::GetValidCanvasRegion(const ViewPort& VPoint, wxRegion *pValidRegion)
{
      double raster_scale =  VPoint.view_scale_ppm / GetPPM();

      int rxl, rxr;
      if(Rsrc.x < 0)
            rxl = (int)(-Rsrc.x * raster_scale);
      else
            rxl = 0;

      if(((Size_X - Rsrc.x) * raster_scale) < VPoint.pix_width)
            rxr = (int)((Size_X - Rsrc.x) * raster_scale);
      else
            rxr = VPoint.pix_width;

      int ryb, ryt;
      if(Rsrc.y < 0)
            ryt = (int)(-Rsrc.y * raster_scale);
      else
            ryt = 0;

      if(((Size_Y - Rsrc.y) * raster_scale) < VPoint.pix_height)
            ryb = (int)((Size_Y - Rsrc.y) * raster_scale);
      else
            ryb = VPoint.pix_height;

      pValidRegion->Clear();
      pValidRegion->Union(rxl, ryt, rxr - rxl, ryb - ryt);
}


bool ChartBaseBSB::GetViewUsingCache( wxRect& source, wxRect& dest, ScaleTypeEnum scale_type )
{
      wxRect s1;
//      unsigned char *pCP;
//      unsigned char *pPix;
      ScaleTypeEnum scale_type_corrected;
//      int xsoff, ysoff;

      if(b_cdebug)printf(" source:  %d %d\n", source.x, source.y);
      if(b_cdebug)printf(" cache:   %d %d\n", cache_rect.x, cache_rect.y);

//    Anything to do?
      if((source == cache_rect) /*&& (cache_scale_method == scale_type)*/ && (cached_image_ok) )
      {
            if(b_cdebug)printf("    GVUC: Nothing to do\n");
            return false;
      }

      double scale_x = (double)source.width / (double)dest.width;

      if(b_cdebug)printf("GVUC: scale_x: %g\n", scale_x);

      //    Enforce a limit on bilinear scaling, for performance reasons
      scale_type_corrected = RENDER_LODEF; //scale_type;
      if(scale_x > m_bilinear_limit)
            scale_type_corrected = RENDER_LODEF;

      {
//            if(b_cdebug)printf("   MISS<<<>>>GVUC: Intentional out\n");
//            return GetView( source, dest, scale_type_corrected );
      }


      //    Using the cache only works for pure binary scale factors......
      if((fabs(scale_x - wxRound(scale_x))) > .0001)
      {
            if(b_cdebug)printf("   MISS<<<>>>GVUC: Not digital scale test 1\n");
            return GetView( source, dest, scale_type_corrected );
      }

      scale_type_corrected = RENDER_LODEF;


      if(!cached_image_ok)
      {
            if(b_cdebug)printf("    MISS<<<>>>GVUC:  Cache NOk\n");
            return GetView( source, dest, scale_type_corrected );
      }

      if(scale_x <= 1.0)                                        // overzoom
      {
            if(b_cdebug)printf("    MISS<<<>>>GVUC:  Overzoom\n");
            return GetView( source, dest, scale_type_corrected );
      }

      //    Scale must be exactly digital...
      if((int)(source.width/dest.width) != (int)wxRound(scale_x))
      {
            if(b_cdebug)printf("   MISS<<<>>>GVUC: Not digital scale test 2\n");
            return GetView( source, dest, scale_type_corrected );
      }

//    Calculate the digital scale, e.g. 1,2,4,8,,,
      int cs1d = source.width/dest.width;
      if(abs(source.x - cache_rect.x) % cs1d)
      {
            if(b_cdebug)printf("   source.x: %d  cache_rect.x: %d  cs1d: %d\n", source.x, cache_rect.x, cs1d);
            if(b_cdebug)printf("   MISS<<<>>>GVUC: x mismatch\n");
            return GetView( source, dest, scale_type_corrected );
      }
      if(abs(source.y - cache_rect.y) % cs1d)
      {
            if(b_cdebug)printf("   MISS<<<>>>GVUC: y mismatch\n");
            return GetView( source, dest, scale_type_corrected );
      }

      if(pPixCache && ((pPixCache->GetWidth() != dest.width) || (pPixCache->GetHeight() != dest.height)))
      {
            if(b_cdebug)printf("   MISS<<<>>>GVUC: dest size mismatch\n");
            return GetView( source, dest, scale_type_corrected );
      }

      int stride_rows = (source.y + source.height) - (cache_rect.y + cache_rect.height);
//      int yo = (source.y - cache_rect.y) % cs1d;
      int scaled_stride_rows = (int)(stride_rows / scale_x);

      if(abs(stride_rows) >= source.height)                       // Pan more than one screen
            return GetView( source, dest, scale_type_corrected );

      int stride_pixels = (source.x + source.width) - (cache_rect.x + cache_rect.width);
      int scaled_stride_pixels = (int)(stride_pixels / scale_x);


      if(abs(stride_pixels) >= source.width)                      // Pan more than one screen
            return GetView( source, dest, scale_type_corrected );

      if(b_cdebug)printf("    GVUC Using raster data cache\n");

      ScaleTypeEnum pan_scale_type_x = scale_type_corrected;
      ScaleTypeEnum pan_scale_type_y = scale_type_corrected;


      //    "Blit" the valid pixels out of the way
      int height = pPixCache->GetHeight();
      int width = pPixCache->GetWidth();
      int stride = pPixCache->GetLinePitch();

      unsigned char *ps;
      unsigned char *pd;

      if(stride_rows > 0)                             // pan down
      {
           ps = pPixCache->GetpData() +  (abs(scaled_stride_rows) * stride);
           if(stride_pixels > 0)
                 ps += scaled_stride_pixels * BPP/8;

           pd = pPixCache->GetpData();
           if(stride_pixels <= 0)
                 pd += abs(scaled_stride_pixels) * BPP/8;

           for(int iy=0 ; iy< (height - abs(scaled_stride_rows)) ; iy++)
           {
                 memmove(pd, ps, (width - abs(scaled_stride_pixels)) *BPP/8);

                 ps += width * BPP/8;
                 pd += width * BPP/8;
           }

      }
      else
      {
            ps = pPixCache->GetpData() + ((height - abs(scaled_stride_rows)-1) * stride);
            if(stride_pixels > 0)               // make a hole on right
                  ps += scaled_stride_pixels * BPP/8;

            pd = pPixCache->GetpData() +  ((height -1) * stride);
            if(stride_pixels <= 0)              // make a hole on the left
                  pd += abs(scaled_stride_pixels) * BPP/8;


            for(int iy=0 ; iy< (height - abs(scaled_stride_rows)) ; iy++)
            {
                  memmove(pd, ps, (width - abs(scaled_stride_pixels)) *BPP/8);

                  ps -= width * BPP/8;
                  pd -= width * BPP/8;
            }
      }







//    Y Pan
      if(source.y != cache_rect.y)
      {
            if(abs(scaled_stride_rows) < 4)
            {
                  if(b_cdebug)printf("    GVUC Filling cache(y) at HIDEF %d rows\n", abs(scaled_stride_rows));
                  pan_scale_type_y = RENDER_HIDEF;
            }

#if 0
            //    Get the new bits needed
            if(stride_rows > 0)                             // pan down
                  s1 = wxRect(cache_rect.x, cache_rect.y + cache_rect.height, cache_rect.width, stride_rows + yo);
            else                                                  // pan up
                  s1 = wxRect(cache_rect.x, source.y, cache_rect.width,  -(stride_rows + yo));

            wxRect sub_dest = dest;
            sub_dest.y = 0;
                  sub_dest.height = abs(scaled_stride_rows);

            unsigned char *ppnx = (unsigned char *) x_malloc( dest.width*(dest.height+2)*BPP/8 );
            GetAndScaleData(&ppnx, s1, source.width, sub_dest, dest.width, cs1d, pan_scale_type_y);

//    Now, concatenate the data

            unsigned char *ppc = (unsigned char *)pPixCache->GetpData();     // source
            PixelCache *pPCtemp = new PixelCache(dest.width, dest.height, BPP);     // destination
            pPix = pPCtemp->GetpData();


            unsigned char *ppn = ppnx;
            pCP = pPix;
            if(stride_rows > 0)                             // pan down
            {
                  unsigned char *pcac = ppc + (scaled_stride_rows * dest.width * BPP/8);  //pointer into current cache
                  unsigned char *pnew = ppn;

                  memcpy(pCP, pcac, dest.width * (dest.height - scaled_stride_rows)*BPP/8);
                  pCP += dest.width * (dest.height - scaled_stride_rows) *BPP/8;
                  memcpy(pCP, pnew, dest.width * (scaled_stride_rows)*BPP/8);

            }
            else                                                  // pan up
            {
                  unsigned char *pcac = ppc;
                  unsigned char *pnew = ppn;

                  memcpy(pCP, pnew, dest.width * (-scaled_stride_rows)*BPP/8);
                  pCP += dest.width * (-scaled_stride_rows) *BPP/8;
                  memcpy(pCP, pcac, dest.width * (dest.height + scaled_stride_rows)*BPP/8);

            }
#endif
            //    Move the data in the cache out of the way
//            int height = pPixCache->GetHeight();
//            int width = pPixCache->GetWidth();

            wxRect sub_dest = dest;
            sub_dest.height = abs(scaled_stride_rows);

            if(stride_rows > 0)                             // pan down
            {
//                  unsigned char *ps = pPixCache->GetpData() +  (scaled_stride_rows * width * BPP/8);
//                  memmove(pPixCache->GetpData(), ps, width * (height - scaled_stride_rows)*BPP/8);
                  sub_dest.y = height - scaled_stride_rows;

            }
            else
            {
//                  unsigned char *pd = pPixCache->GetpData() - (scaled_stride_rows * width * BPP/8);
//                  memmove(pd, pPixCache->GetpData(), width * (height + scaled_stride_rows)*BPP/8);
                  sub_dest.y = 0;

            }


            //    Get the new bits needed
/*
            if(stride_rows > 0)                             // pan down
                  s1 = wxRect(cache_rect.x, cache_rect.y + cache_rect.height, cache_rect.width, stride_rows + yo);
            else                                                  // pan up
                  s1 = wxRect(cache_rect.x, source.y, cache_rect.width,  -(stride_rows + yo));

            wxRect sub_dest = dest;
            sub_dest.y = 0;
            sub_dest.height = abs(scaled_stride_rows);
*/
//            unsigned char *ppnx = (unsigned char *) x_malloc( pPixCache->GetWidth()*(pPixCache->GetHeight()+2)*BPP/8 );
            unsigned char *ppn = pPixCache->GetpData();
            GetAndScaleData(&ppn, source, source.width, sub_dest, width, cs1d, pan_scale_type_y);
/*
//    Now, concatenate the data

            unsigned char *ppc = (unsigned char *)pPixCache->GetpData();     // source

            PixelCache *pPCtemp = new PixelCache(pPixCache->GetWidth(), pPixCache->GetHeight(), BPP);     // destination
            pPix = pPCtemp->GetpData();


            unsigned char *ppn = ppnx;
            pCP = pPix;
            if(stride_rows > 0)                             // pan down
            {
                  unsigned char *pcac = ppc + (scaled_stride_rows * pPixCache->GetWidth() * BPP/8);  //pointer into current cache
                  unsigned char *pnew = ppn + (scaled_stride_rows * pPixCache->GetWidth() * BPP/8);

                  memcpy(pCP, pcac, dest.width * (dest.height - scaled_stride_rows)*BPP/8);
                  pCP += dest.width * (dest.height - scaled_stride_rows) *BPP/8;
                  memcpy(pCP, pnew, dest.width * (scaled_stride_rows)*BPP/8);

            }
            else                                                  // pan up
            {
                  unsigned char *pcac = ppc;
                  unsigned char *pnew = ppn;

                  memcpy(pCP, pnew, dest.width * (-scaled_stride_rows)*BPP/8);
                  pCP += dest.width * (-scaled_stride_rows) *BPP/8;
                  memcpy(pCP, pcac, dest.width * (dest.height + scaled_stride_rows)*BPP/8);

            }
*/

//    Delete working data
//            free(ppnx);


//            delete pPixCache;
//            pPixCache = pPCtemp;
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
            if(abs(scaled_stride_pixels) < 4)
            {
                  if(b_cdebug)printf("    GVUC Filling cache(x) at HIDEF %d columns\n",abs(scaled_stride_pixels) );
                  pan_scale_type_x = RENDER_HIDEF;
            }
            //    Move the data in the cache out of the way
 //           int height = pPixCache->GetHeight();
 //           int width = pPixCache->GetWidth();

            wxRect sub_dest = dest;
            sub_dest.width = abs(scaled_stride_pixels);

            if(stride_pixels > 0)                           // pan right
            {
/*
                  unsigned char *ps = pPixCache->GetpData() + scaled_stride_pixels * BPP/8;
                  unsigned char *pd = pPixCache->GetpData();

                  for(int iy=0 ; iy<height ; iy++)
                  {
                        memmove(pd, ps, (width - scaled_stride_pixels) *BPP/8);

                        ps += width * BPP/8;
                        pd += width * BPP/8;
                  }
*/
                  sub_dest.x = width - scaled_stride_pixels;
            }
            else                                                  // pan left
            {
/*
                  unsigned char *pd = pPixCache->GetpData() - scaled_stride_pixels * BPP/8;
                  unsigned char *ps = pPixCache->GetpData();

                  for(int iy=0 ; iy<height ; iy++)
                  {
                        memmove(pd, ps, (width + scaled_stride_pixels) *BPP/8);

                        ps += width * BPP/8;
                        pd += width * BPP/8;
                  }
*/
                  sub_dest.x = 0;
            }

            //    Get the new bits needed

            unsigned char *ppnx = pPixCache->GetpData();
            GetAndScaleData(&ppnx, source, source.width, sub_dest, width, cs1d, pan_scale_type_x);

            pPixCache->Update();

#if 0
            //    Get the new bits needed
            if(stride_pixels > 0)                           // pan right
                  s1 = wxRect(cache_rect.x + cache_rect.width, source.y, stride_pixels, source.height);
            else                                                  // pan left
                  s1 = wxRect(source.x, source.y, -stride_pixels, source.height);

            wxRect sub_dest = dest;
            sub_dest.x = 0;
            sub_dest.width = abs(scaled_stride_pixels);

            unsigned char *ppnx = (unsigned char *) x_malloc( dest.width*(dest.height+2)*BPP/8 );
            GetAndScaleData(&ppnx, s1, source.width, sub_dest, abs(scaled_stride_pixels), cs1d, pan_scale_type_x);


//    Now, concatenate the data

            unsigned char *ppc = (unsigned char *)pPixCache->GetpData();     // source
            PixelCache *pPCtemp = new PixelCache(dest.width, dest.height, BPP);     // destination
            pPix = pPCtemp->GetpData();

            unsigned char *ppn = ppnx;
            pCP = pPix;
            if(stride_pixels > 0)                           // pan right
            {
                  unsigned char *pcac = ppc + scaled_stride_pixels * BPP/8;
                  unsigned char *pnew = ppn;

                  for(int iy=0 ; iy<cache_rect_scaled.height ; iy++)
                  {
                        memcpy(pCP, pcac, (cache_rect_scaled.width-scaled_stride_pixels) *BPP/8);

                        pCP += (cache_rect_scaled.width-scaled_stride_pixels) *BPP/8;

                        memcpy(pCP, pnew, scaled_stride_pixels * BPP/8);

                        pcac += cache_rect_scaled.width * BPP/8;
                        pnew += scaled_stride_pixels * BPP/8;

                        pCP += scaled_stride_pixels * BPP/8;
                  }
            }
            else                                                  // pan left
            {
                  unsigned char *pcac = ppc;
                  unsigned char *pnew = ppn;

                  for(int iy=0 ; iy<cache_rect_scaled.height ; iy++)
                  {
                        memcpy(pCP, pnew, scaled_stride_pixels * -BPP/8);
                        pCP += scaled_stride_pixels * -BPP/8;

                        memcpy(pCP, pcac, (cache_rect_scaled.width+scaled_stride_pixels) * BPP/8);

                        pCP += (cache_rect_scaled.width+scaled_stride_pixels) * BPP/8;


                        pcac += cache_rect_scaled.width * BPP/8;
                        pnew += scaled_stride_pixels * -BPP/8;

                  }
            }


//    Delete working data
            free(ppnx);


            delete pPixCache;
            pPixCache = pPCtemp;
#endif

//    Update the cached parameters
            cache_rect = source;
            cache_rect_scaled = dest;
            cached_image_ok = 1;

      }           // X pan

      if(cache_scale_method == RENDER_HIDEF)          // as set by background render
      {
            if( (abs(scaled_stride_pixels) && (pan_scale_type_x == RENDER_LODEF))
                 || (abs(scaled_stride_rows) && (pan_scale_type_y == RENDER_LODEF)) )
               cache_scale_method = RENDER_LODEF;
      }

      return true;
}






int s_dc;

bool ChartBaseBSB::RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, ScaleTypeEnum scale_type)
{
      wxRegion rgn(0,0,VPoint.pix_width, VPoint.pix_height);

      bool bsame_region = (rgn == m_last_region);          // only want to do this once


      if(!bsame_region)
            cached_image_ok = false;

      m_last_region = rgn;

      return RenderRegionViewOnDC(dc, VPoint, rgn,  scale_type);

/*
      wxRect dest(0,0,VPoint.pix_width, VPoint.pix_height);
      double factor = ((double)Rsrc.width)/((double)dest.width);
      factor = m_raster_scale_factor;

      if(b_cdebug)printf("RenderView  ScaleType:  %d   factor:  %g\n", scale_type, factor );

      //    Invalidate the cache if the scale has changed....
      if(fabs(m_cached_scale_ppm - VPoint.view_scale_ppm) > 1e-9)
            cached_image_ok = false;

      m_cached_scale_ppm = VPoint.view_scale_ppm;

      bool bnewview;
//    Get the view into the pixel buffer
      bnewview = GetViewUsingCache(Rsrc, dest, scale_type);


      //    It could happen that this is the first render of this chart,
      //    .AND. scale_type is bi-linear
      //    .AND.  the render is interrupted by mouse movement.
      //    In this case, there is will be no pPixCache yet....

      //    So, force a subsample render which cannot be interrupted

      if(pPixCache == NULL)
            bnewview = GetViewUsingCache(Rsrc, dest, RENDER_LODEF);

//    Select the data into the dc
      pPixCache->SelectIntoDC(dc);

      Initialize_BackgroundHiDefRender(VPoint);

      return bnewview;
*/
}






bool ChartBaseBSB::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const wxRegion &Region, ScaleTypeEnum scale_type)
{
      wxRect dest(0,0,VPoint.pix_width, VPoint.pix_height);
//      double factor = ((double)Rsrc.width)/((double)dest.width);
      double factor = m_raster_scale_factor;
      if(b_cdebug)printf("%d RenderRegion  ScaleType:  %d   factor:  %g\n", s_dc++, scale_type, factor );

            //    Invalidate the cache if the scale has changed or the viewport size has changed....
      if((fabs(m_cached_scale_ppm - VPoint.view_scale_ppm) > 1e-9) || (m_last_vprect != dest))
      {
            cached_image_ok = false;

            //    Cancel any background render in process
            delete pPixCacheBackground;
            pPixCacheBackground = NULL;

            free(background_work_buffer);
            background_work_buffer = NULL;

            br_Rsrc.SetSize(wxSize(1,1));             // mark this a definitely different
            br_target_y = 0;
            m_br_bpending = false;;
      }


      m_cached_scale_ppm = VPoint.view_scale_ppm;
      m_last_vprect = dest;


      bool bsame_region = (Region == m_last_region);          // only want to do this once

     //    Anything to do?
     if((bsame_region) && (Rsrc == cache_rect)  && (cached_image_ok) )
     {
              pPixCache->SelectIntoDC(dc);
              if(b_cdebug)printf("  Using Current PixelCache\n");
              return false;
     }

//     if(!bsame_region)
//           cached_image_ok = false;

     m_last_region = Region;

     //     Is this render interrupting a background HIDEF render at the same scale?
     //     If so, then go ahead and do a HIDEF render to this viewport, and return same
     if(m_br_bpending && (0 != br_target_y) && (fabs(m_cached_scale_ppm - VPoint.view_scale_ppm) < 1e-9))
     {
           if(b_cdebug)printf("  Underway bbr interrupted, creating new HIDEF render\n");

           PixelCache *pPixCacheTemp = new PixelCache(dest.width, dest.height, BPP);

           unsigned char *ppnx = pPixCacheTemp->GetpData();

           if(!GetAndScaleData(&ppnx, Rsrc, Rsrc.width, dest, dest.width, factor, RENDER_HIDEF))
           {
                  delete pPixCacheTemp;                       // Some error, retain old cache
                  return false;
           }
           else
           {
                  delete pPixCache;                           // new cache is OK
                  pPixCache = pPixCacheTemp;
           }

//    Update cache parameters
           cache_rect = Rsrc;
           cache_rect_scaled = dest;
           cache_scale_method = RENDER_HIDEF;
           cached_image_ok = 1;

//    Mark the background as done
           delete pPixCacheBackground;
           pPixCacheBackground = NULL;

           free(background_work_buffer);
           background_work_buffer = NULL;

           br_target_y = 0;
           m_br_bpending = false;

//          Return the best pixel cache
           pPixCache->SelectIntoDC(dc);

           return true;
     }

     //     Analyze the region requested
     //     Get the region rectanglel count and the proportion of the vp covered
     int pix_area = 0;
     int n_rect =0;
     wxRegionIterator upd ( Region ); // get the requested rect list
     while ( upd )
     {
           wxRect rect = upd.GetRect();
           pix_area += rect.width * rect.height;
           n_rect++;
           upd ++ ;
     }
     double area_fraction = (double)pix_area / (double)(VPoint.pix_width * VPoint.pix_height);

     //     When rendering complex regions, (more than say 4 rectangles)
     //     .OR. small proportions, then rectangle rendering may be faster
     //     Also true  if the scale is less than near unity, or overzoom.
     //     This will be the case for backgrounds of the quilt.


     if((factor < 1) || (n_rect > 4) || (area_fraction < .4))
     {
           if(b_cdebug)printf("   RenderRegion by rect iterator   n_rect: %d\n", n_rect);

           PixelCache *pPixCacheTemp = new PixelCache(dest.width, dest.height, BPP);

      //    Decompose the region into rectangles, and fetch them into the target dc
           wxRegionIterator upd ( Region ); // get the requested rect list
           int ir = 0;
           while ( upd )
           {
                 wxRect rect = upd.GetRect();
                 unsigned char *ppnx = pPixCacheTemp->GetpData();

                 GetAndScaleData(&ppnx, Rsrc, Rsrc.width, rect, dest.width, factor, RENDER_LODEF);

                 ir++;
                 upd ++ ;
           }

           delete pPixCache;                           // new cache is OK
           pPixCache = pPixCacheTemp;



      //    Update cache parameters
           cache_rect = Rsrc;
           cache_scale_method = RENDER_LODEF;
           cached_image_ok = true;


//    Select the data into the dc
           pPixCache->SelectIntoDC(dc);

           Initialize_BackgroundHiDefRender(VPoint);

           return true;
     }



     //     Default is to try using the cache
     {
           if(b_cdebug)printf("  Render Region By GVUC\n");
           bool bnewview = GetViewUsingCache(Rsrc, dest, scale_type);

                  //    Select the data into the dc
           pPixCache->SelectIntoDC(dc);

           Initialize_BackgroundHiDefRender(VPoint);

           return bnewview;
     }

      return true;
}



bool ChartBaseBSB::GetView( wxRect& source, wxRect& dest, ScaleTypeEnum scale_type )
{
//      int cs1 = source.width/dest.width;

//    Setup the scale_type dependencies
//      int get_bits_submap = 1;            // default, bi-linear

//    n.b if cs1 ==0, we are oversampling, scale > 1
//      if( (scale_type == SCALE_SUBSAMP) && cs1)
//            get_bits_submap = cs1;

//      float canvas_scale = (float)dest.width / (float)source.width;

//      printf("canvas_scale %g\n", canvas_scale);

//    Create a temporary PixelCache
//      delete pPixCache;
      PixelCache *pPixCacheTemp = new PixelCache(dest.width, dest.height, BPP);


//    Special case for unity scale
//    In this case, need only one buffer, so make it the PixelCache buffer directly
//      if(canvas_scale == 1)
//      {
//    Get the chart bits directly into the PixelCache data buffer
//          GetChartBits(source, pPixCacheTemp->GetpData(), get_bits_submap);
//          delete pPixCache;
//          pPixCache = pPixCacheTemp;
//      }

//    Otherwise, non-unity scale, so need a temp buffer
//      else
      {
//    Get and Rescale the data directly into the temporary PixelCache data buffer

            unsigned char *ppnx = pPixCacheTemp->GetpData();
            double factor = ((double)source.width)/((double)dest.width);

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
      }

//    Update cache parameters

      cache_rect = source;
      cache_rect_scaled = dest;
      cache_scale_method = scale_type;

      cached_image_ok = 1;


      return TRUE;
}


bool ChartBaseBSB::GetAndScaleData(unsigned char **ppn, wxRect& source, int source_stride,
                                   wxRect& dest, int dest_stride, double scale_factor, ScaleTypeEnum scale_type)
{

      unsigned char *s_data = NULL;

      double factor = scale_factor;  //((double)source_stride)/((double)dest.width);
      int Factor =  (int)factor;

      int target_width = (int)wxRound((double)source.width  / factor) ;
      int target_height = (int)wxRound((double)source.height / factor);


      if((target_height == 0) || (target_width == 0))
            return false;

      unsigned char *target_data;
      unsigned char *data;
      int malloc_size;

      if(*ppn)                                        // Caller is supplying buffer
      {
            data = *ppn;
            target_data = data;
      }


      else                                            // else get a buffer here (and return it)
      {
//            data = (unsigned char *) x_malloc( target_width*(target_height+2)*BPP/8 );
            malloc_size = dest.width*(dest.height+2)*BPP/8;
            data = (unsigned char *) x_malloc( dest.width*(dest.height+2)*BPP/8 );
            target_data = data;
            *ppn = data;
      }




      if(factor > 1)                // downsampling
      {

            if(scale_type == RENDER_HIDEF)
            {
//    Allocate a working buffer based on scale factor
                  int blur_factor = wxMax(2, Factor);
                  int wb_size = (source.width) * (blur_factor * 2) * BPP/8 ;
                  s_data = (unsigned char *) x_malloc( wb_size ); // work buffer
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

                        target_data = data + (y * dest_stride * BPP/8);

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
                        s_data = (unsigned char *) x_malloc( wb_size ); // work buffer

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

                              target_data = data + (y * dest_stride * BPP/8) + (dest.x * BPP / 8);

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
            sigaction(SIGSEGV, NULL, &sa_all_old);             // save existing action for this signal

            struct sigaction temp;
            sigaction(SIGSEGV, NULL,  &temp);             // inspect existing action for this signal

            temp.sa_handler = catch_signals ;             // point to my handler
            sigemptyset(&temp.sa_mask);                  // make the blocking set
                                                        // empty, so that all
                                                        // other signals will be
                                                        // unblocked during my handler
            temp.sa_flags = 0;
            sigaction(SIGSEGV, &temp, NULL);

            if(sigsetjmp(env, 1))             //  Something in the below code block faulted....
            {
                  sigaction(SIGSEGV, &sa_all_old, NULL);        // reset signal handler

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

                  //    Seems safe enough to read all the required data
                  int sx = wxMax(source.x, 0);

                  //    Although we must adjust (increase) temporary allocation for negative source.x

      //            int s_data_size = (sx + source.width) * source.height * BPP/8;
                  s_data = (unsigned char *) malloc( (sx + source.width) * source.height * BPP/8 );

                  GetChartBits(source, s_data, 1);


                  int s_data_offset = (int)(1./ m_raster_scale_factor);
                  s_data_offset /= 2;
                  s_data_offset *= source.width * BPP/8;

                  unsigned char *source_data =  s_data; //+ s_data_offset;

      //            target_height = dest.height;
      //            target_width = dest.width;

                  j = dest.y;

                  while( j < dest.y + dest.height)
                  {
                        y_offset = (int)(j *m_raster_scale_factor) * source.width;

                        target_line_start = target_data + (j * dest_stride * BPP / 8);
                        target_data_x = target_line_start + (dest.x * BPP / 8);

                        i = dest.x;
                        while( i < dest.x + dest.width)
                        {
      //                        int into_target_buffer = target_data_x - target_data;

      //                        int into_source_buffer =  BPP/8*(y_offset + (int)(i * m_raster_scale_factor));

                              memcpy( target_data_x,
                                    source_data + BPP/8*(y_offset + (int)(i * m_raster_scale_factor)),
                                    BPP/8 );
                              target_data_x += BPP/8;

                              i++;
                        }

      //                  target_line_start += target_width * BPP / 8;
                        j++;
                  }

            }
#ifdef __WXGTK__
            sigaction(SIGSEGV, &sa_all_old, NULL);        // reset signal handler
#endif

      }

      free(s_data);

      return true;

}



bool ChartBaseBSB::Initialize_BackgroundHiDefRender(const ViewPort &VPoint)
{
    if(br_Rsrc == Rsrc)
    {
          if(b_cdebug)printf("    %d No Init bbr due to Rsrc == br_Rsrc \n",s_dc++);
          return false;;
    }

    if(cache_scale_method == RENDER_HIDEF)
    {
          if(cached_image_ok && (cache_rect == Rsrc))
          {
                if(b_cdebug)printf("    %d No Init bbr due to good cache \n",s_dc++);
                return false;;
          }
    }

    wxRect dest(0,0,VPoint.pix_width, VPoint.pix_height);
    double factor = ((float)Rsrc.width)/((float)dest.width);

    if(factor < 1.0)            // it should never happen that we try to
        return false;           // re-scale an overzoomed chart, but....

    if(factor >=8.0 )            // set an upper limit on rescale for performance reasons
          return false;

    m_br_bpending = true;
    br_factor = factor;
    br_Rsrc = Rsrc;

    br_target_width = dest.width;
    br_target_height = dest.height;

    delete pPixCacheBackground;
    pPixCacheBackground = NULL;     //new PixelCache(dest.width, dest.height, BPP);

    free(background_work_buffer);
    background_work_buffer = NULL; //(unsigned char *) x_malloc( bwb_size ); // work buffer

    //  And the scale....
    m_br_scale = VPoint.view_scale_ppm;

    //  Set starting points
    br_target_y = 0;

    if(b_cdebug)printf(" on bbr start, br_Rsrc: %d %d\n", br_Rsrc.x, br_Rsrc.y);

    return true;
}

bool ChartBaseBSB::Finish_BackgroundHiDefRender(void)
{
    if(b_cdebug)printf("    ......Finish bbr\n");
    cache_scale_method= RENDER_HIDEF;             // the cache is set
    cache_rect = br_Rsrc; //Rsrc;

    cached_image_ok = true;

    delete pPixCache;
    pPixCache = pPixCacheBackground;
    pPixCacheBackground = NULL;

    free(background_work_buffer);
    background_work_buffer = NULL;

    br_target_y = 0;
    m_br_bpending = false;;

    return true;
}

int ChartBaseBSB::Continue_BackgroundHiDefRender(void)
        /*
            Return BR_DONE_NOP or BR_DONE_REFRESH when finished
        */
{
    if(!m_br_bpending)
    {
          if(b_cdebug)printf("bbr Continue reports nothing in process\n");
          return BR_DONE_NOP;
    }

//    if(br_Rsrc != Rsrc)
//          printf("Rsrs mismatch\n");

    if(!pPixCacheBackground)
    {
          if(b_cdebug)printf("    ......bbr Continue creates new PixelCache\n");
          pPixCacheBackground = new PixelCache(br_target_width, br_target_height, BPP);
          br_target_data = pPixCacheBackground->GetpData();
    }

    if(!background_work_buffer)
    {
      int bwb_size = (int)((Rsrc.width) * ((br_factor + 1) * 2) * BPP/8 );
      background_work_buffer = (unsigned char *) x_malloc( bwb_size ); // work buffer
    }

    if (br_target_y < br_target_height)
    {
        int y_source = Rsrc.y + (int)(br_target_y * br_factor);
                  //    Read "factor + 1" lines
        wxRect s1;
        s1.x = Rsrc.x;
        s1.y = y_source;
        s1.width = Rsrc.width;
        s1.height = (int)(br_factor + 1);
        GetChartBits(s1, background_work_buffer, 1);

        unsigned char *pixel;

        for (int x = 0; x < br_target_width; x++)
        {
            unsigned int avgRed = 0 ;
            unsigned int avgGreen = 0;
            unsigned int avgBlue = 0;
            unsigned char *pix0 = background_work_buffer + BPP/8 * ((int)( x * br_factor )) ;

            int pixel_count = 0;
            int y_offset = 0;
            if((x * br_factor) < (Size_X - Rsrc.x))
            {
            // determine average
                for ( int y1 = 0 ; y1 < (int)br_factor ; ++y1 )
                {
                    pixel = pix0 + (BPP/8 * y_offset ) ;
                    for ( int x1 = 0 ; x1 < (int)br_factor ; ++x1 )
                    {
                        avgRed   += pixel[0] ;
                        avgGreen += pixel[1] ;
                        avgBlue  += pixel[2] ;

                        pixel += BPP/8;
                        pixel_count++;
                    }
                    y_offset += Rsrc.width ;
                }


                br_target_data[0] = avgRed   / pixel_count;
                br_target_data[1] = avgGreen / pixel_count;
                br_target_data[2] = avgBlue  / pixel_count;

                br_target_data += BPP/8;
            }

            else
            {
                  br_target_data[0] = 0;
                  br_target_data[1] = 0;
                  br_target_data[2] = 0;

                  br_target_data += BPP/8;
            }

        }  // for x

        br_target_y++;

        return BR_CONTINUE;                   // more to come

    }  // if y on range

    Finish_BackgroundHiDefRender();
    return BR_DONE_REFRESH;                        // done
}


bool ChartBaseBSB::GetChartBits(wxRect& source, unsigned char *pPix, int sub_samp)
{

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

int ChartBaseBSB::ReadBSBHdrLine(wxFileInputStream* ifs, char* buf, int buf_len_max)

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



//-----------------------------------------------------------------------
//    Get a BSB Scan Line Using Cache and scan line index if available
//-----------------------------------------------------------------------
int   ChartBaseBSB::BSBGetScanline( unsigned char *pLineBuf, int y, int xs, int xl, int sub_samp)

{
      int nLineMarker, nValueShift, iPixel = 0;
      unsigned char byValueMask, byCountMask;
      unsigned char byNext;
      CachedLine *pt = NULL;
      unsigned char *pCL;
      int rgbval;
      unsigned char *lp;
      unsigned char *xtemp_line;
      register int ix = xs;

      if(bUseLineCache && pLineCache)
      {
//    Is the requested line in the cache, and valid?
            pt = &pLineCache[y];
            if(!pt->bValid)                                 // not valid, so get it
            {
                  if(pt->pPix)
                        free(pt->pPix);
                  pt->pPix = (unsigned char *)x_malloc(Size_X);
            }

            xtemp_line = pt->pPix;
      }
      else
            xtemp_line = (unsigned char *)x_malloc(Size_X);


      if((bUseLineCache && !pt->bValid) || (!bUseLineCache))
      {
          if(pline_table[y] == 0)
              return 0;

          if(pline_table[y+1] == 0)
              return 0;

            int thisline_size = pline_table[y+1] - pline_table[y] ;

            if(thisline_size > ifs_bufsize)
                ifs_buf = (unsigned char *)realloc(ifs_buf, thisline_size);

            if( wxInvalidOffset == ifs_bitmap->SeekI(pline_table[y], wxFromStart))
                  return 0;

            ifs_bitmap->Read(ifs_buf, thisline_size);
            lp = ifs_buf;

//    At this point, the unexpanded, raw line is at *lp, and the expansion destination is xtemp_line

//      Read the line number.
            nLineMarker = 0;
            do
            {
                  byNext = *lp++;
                  nLineMarker = nLineMarker * 128 + (byNext & 0x7f);
            } while( (byNext & 0x80) != 0 );

//      Setup masking values.
            nValueShift = 7 - nColorSize;
            byValueMask = (((1 << nColorSize)) - 1) << nValueShift;
            byCountMask = (1 << (7 - nColorSize)) - 1;

//      Read and expand runs.

            pCL = xtemp_line;

            while( ((byNext = *lp++) != 0 ) && (iPixel < Size_X))
            {
                  int   nPixValue;
                  int nRunCount;
                  nPixValue = (byNext & byValueMask) >> nValueShift;

                  nRunCount = byNext & byCountMask;

                  while( (byNext & 0x80) != 0 )
                  {
                        byNext = *lp++;
                        nRunCount = nRunCount * 128 + (byNext & 0x7f);
                  }

                  if( iPixel + nRunCount + 1 > Size_X )     // protection
                        nRunCount = Size_X - iPixel - 1;

                  if(nRunCount < 0)                         // against corrupt data
                      nRunCount = 0;

//          Store nPixValue in the destination
                  memset(pCL, nPixValue, nRunCount+1);
                  pCL += nRunCount+1;
                  iPixel += nRunCount+1;
            }
      }

      if(bUseLineCache)
            pt->bValid = true;

//          Line is valid, de-reference thru proper pallete directly to target


      if(xl > Size_X-1)
            xl = Size_X-1;

      pCL = xtemp_line + xs;
      unsigned char *prgb = pLineBuf;

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

// Get the last pixel explicitely
//  irrespective of the sub_sampling factor

      if(xs < xl-1)
      {
        unsigned char *pCLast = xtemp_line + (xl - 1);
        unsigned char *prgb_last = pLineBuf + ((xl - 1)-xs) * BPP/8;

        rgbval = (int)(pPalette[*pCLast]);        // last pixel
        unsigned char a = rgbval & 0xff;
        *prgb_last++ = a;
        a = (rgbval >> 8) & 0xff;
        *prgb_last++ = a;
        a = (rgbval >> 16) & 0xff;
        *prgb_last = a;
      }

      if(!bUseLineCache)
          free (xtemp_line);

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



int   ChartBaseBSB::AnalyzeRefpoints(void)
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
                  m_bIDLcross = true;
            }
      }


//          Build the Control Point Structure, etc
        cPoints.count = nRefpoint;

        cPoints.tx  = (double *)x_malloc(nRefpoint * sizeof(double));
        cPoints.ty  = (double *)x_malloc(nRefpoint * sizeof(double));
        cPoints.lon = (double *)x_malloc(nRefpoint * sizeof(double));
        cPoints.lat = (double *)x_malloc(nRefpoint * sizeof(double));

        cPoints.pwx = (double *)x_malloc(12 * sizeof(double));
        cPoints.wpx = (double *)x_malloc(12 * sizeof(double));
        cPoints.pwy = (double *)x_malloc(12 * sizeof(double));
        cPoints.wpy = (double *)x_malloc(12 * sizeof(double));


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

              cPoints.status = 1;

              Georef_Calculate_Coefficients_Proj(&cPoints);

       }


       else if(m_projection == PROJECTION_MERCATOR)
       {


             double easting0, easting1, northing0, northing1;
              //  Get the Merc projection of the two REF points
             toSM_ECC(pRefTable[imax].latr, pRefTable[imax].lonr, m_proj_lat, m_proj_lon, &easting0, &northing0);
             toSM_ECC(pRefTable[jmax].latr, pRefTable[jmax].lonr, m_proj_lat, m_proj_lon, &easting1, &northing1);

              //  Calculate the scale factor using exact REF point math in x(longitude) direction


             double dx =  (pRefTable[jmax].xr - pRefTable[imax].xr);
             double de =  (easting1 - easting0);

             m_ppm_avg = fabs(dx / de);

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

             cPoints.status = 1;

             Georef_Calculate_Coefficients_Proj(&cPoints);

//              for(int h=0 ; h < 10 ; h++)
//                    printf("pix to east %d  %g\n",  h, cPoints.pwx[h]);          // pix to lon
 //             for(int h=0 ; h < 10 ; h++)
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


/*
       if ((0 != m_Chart_DU ) && (0 != m_Chart_Scale))
       {
             double m_ppm_avg1 = m_Chart_DU * 39.37 / m_Chart_Scale;
             m_ppm_avg1 *= cos(m_proj_lat * PI / 180.);                    // correct to chart centroid

             printf("%g %g\n", m_ppm_avg, m_ppm_avg1);
       }
*/

        // Do a last little test using a synthetic ViewPort of nominal size.....
        ViewPort vp;
        vp.clat = pRefTable[0].latr;
        vp.clon = pRefTable[0].lonr;
        vp.view_scale_ppm = m_ppm_avg;
        vp.skew = 0.;
        vp.pix_width = 1000;
        vp.pix_height = 1000;
//        vp.rv_rect = wxRect(0,0, vp.pix_width, vp.pix_height);
        SetVPParms(vp);


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

        //        Good enough for navigation?
        if(Chart_Error_Factor > .02)
        {
                    wxString msg = _("   VP Final Check: Georeference Chart_Error_Factor on chart ");
                    msg.Append(m_FullPath);
                    wxString msg1;
                    msg1.Printf(_T(" is %5g"), Chart_Error_Factor);
                    msg.Append(msg1);

                    wxLogMessage(msg);

                    m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy is poor. >>>---");
        }

        //  Try again with my calculated georef
        //  This problem was found on NOAA 514_1.KAP.  The embedded coefficients are just wrong....
        if((Chart_Error_Factor > .02) && bHaveEmbeddedGeoref)
        {
              wxString msg = _("   Trying again with internally calculated georef solution ");
              wxLogMessage(msg);

              bHaveEmbeddedGeoref = false;
              SetVPParms(vp);

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

        //        Good enough for navigation?
              if(Chart_Error_Factor > .02)
              {
                    wxString msg = _("   VP Final Check with internal georef: Georeference Chart_Error_Factor on chart ");
                    msg.Append(m_FullPath);
                    wxString msg1;
                    msg1.Printf(_(" is %5g"), Chart_Error_Factor);
                    msg.Append(msg1);

                    wxLogMessage(msg);

                    m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy is poor. >>>---");
              }
              else
              {
                    wxString msg = _("   Result: OK, Internal georef solution used.");

                    wxLogMessage(msg);
              }

        }


      return(0);

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
*  $Id: chartimg.cpp,v 1.57 2010/06/25 13:28:46 bdbcat Exp $
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





