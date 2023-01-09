/******************************************************************************
 *
 * Project:  OpenCPN
 * Authors:  David Register
 *           Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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
 ***************************************************************************
 */

#include <wx/wxprec.h>
#include <wx/progdlg.h>
#include <wx/wx.h>
#include <wx/thread.h>

#if defined(__OCPN__ANDROID__)
#include <GLES2/gl2.h>
#elif defined(__WXQT__) || defined(__WXGTK__)
#include <GL/glew.h>
#endif

#include "dychart.h"
#include "viewport.h"
#include "glTexCache.h"
#include "glTextureDescriptor.h"

#include "chcanv.h"
#include "glChartCanvas.h"
#include "Quilt.h"
#include "chartbase.h"
#include "chartimg.h"
#include "chartdb.h"
#include "OCPNPlatform.h"
#include "FontMgr.h"
#include "mipmap/mipmap.h"
#include "gui_lib.h"
#include "ocpn_frame.h"
#include "own_ship.h"

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES 0x8D64
#endif

#include "squish.h"
#include "lz4.h"
#include "lz4hc.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(JobList);
WX_DEFINE_LIST(ProgressInfoList);

WX_DEFINE_ARRAY_PTR(ChartCanvas *, arrayofCanvasPtr);

extern int g_mipmap_max_level;
extern GLuint g_raster_format;
extern int g_memCacheLimit;
extern ChartDB *ChartData;
extern ocpnGLOptions g_GLOptions;
extern long g_tex_mem_used;
extern int g_tile_size;
extern int g_uncompressed_tile_size;
extern int g_nCPUCount;

extern bool b_inCompressAllCharts;
extern MyFrame *gFrame;
extern arrayofCanvasPtr g_canvasArray;

extern OCPNPlatform *g_Platform;
extern ColorScheme global_color_scheme;

extern bool GetMemoryStatus(int *mem_total, int *mem_used);

bool bthread_debug;
bool g_throttle_squish;

glTextureManager *g_glTextureManager;

#include "ssl/sha1.h"

wxString CompressedCachePath(wxString path) {
#if defined(__WXMSW__)
  int colon = path.find(':', 0);
  if (colon != wxNOT_FOUND) path.Remove(colon, 1);
#endif

  /* replace path separators with ! */
  wxChar separator = wxFileName::GetPathSeparator();
  for (unsigned int pos = 0; pos < path.size(); pos = path.find(separator, pos))
    path.replace(pos, 1, _T("!"));

  //  Obfuscate the compressed chart file name, to (slightly) protect some
  //  encrypted raster chart data.
  wxCharBuffer buf = path.ToUTF8();
  unsigned char sha1_out[20];
  sha1((unsigned char *)buf.data(), strlen(buf.data()), sha1_out);

  wxString sha1;
  for (unsigned int i = 0; i < 20; i++) {
    wxString s;
    s.Printf(_T("%02X"), sha1_out[i]);
    sha1 += s;
  }

  return g_Platform->GetPrivateDataDir() + separator +
         _T("raster_texture_cache") + separator + sha1;
}

int g_mipmap_max_level = 4;

#if 0
OCPN_CompressProgressEvent::OCPN_CompressProgressEvent(wxEventType commandType, int id)
:wxEvent(id, commandType)
{
}

OCPN_CompressProgressEvent::~OCPN_CompressProgressEvent()
{
}

wxEvent* OCPN_CompressProgressEvent::Clone() const
{
    OCPN_CompressProgressEvent *newevent=new OCPN_CompressProgressEvent(*this);
    newevent->m_string=this->m_string;
    newevent->count=this->count;
    newevent->thread=this->thread;
    return newevent;
}
#endif

static double chart_dist(int index) {
  double d;
  float clon;
  float clat;
  const ChartTableEntry &cte = ChartData->GetChartTableEntry(index);
  // if the chart contains ownship position set the distance to 0
  if (cte.GetBBox().Contains(gLat, gLon))
    d = 0.;
  else {
    // find the nearest edge
    double t;
    clon = (cte.GetLonMax() + cte.GetLonMin()) / 2;
    d = DistGreatCircle(cte.GetLatMax(), clon, gLat, gLon);
    t = DistGreatCircle(cte.GetLatMin(), clon, gLat, gLon);
    if (t < d) d = t;

    clat = (cte.GetLatMax() + cte.GetLatMin()) / 2;
    t = DistGreatCircle(clat, cte.GetLonMin(), gLat, gLon);
    if (t < d) d = t;
    t = DistGreatCircle(clat, cte.GetLonMax(), gLat, gLon);
    if (t < d) d = t;
  }
  return d;
}

WX_DEFINE_SORTED_ARRAY_INT(int, MySortedArrayInt);
int CompareInts(int n1, int n2) {
  double d1 = chart_dist(n1);
  double d2 = chart_dist(n2);
  return (int)(d1 - d2);
}

static MySortedArrayInt idx_sorted_by_distance(CompareInts);

class compress_target {
public:
  wxString chart_path;
  double distance;
};

#include <wx/arrimpl.cpp>

WX_DECLARE_OBJARRAY(compress_target, ArrayOfCompressTargets);
// WX_DEFINE_OBJARRAY(ArrayOfCompressTargets);

JobTicket::JobTicket() {
  for (int i = 0; i < 10; i++) {
    compcomp_size_array[i] = 0;
    comp_bits_array[i] = NULL;
    compcomp_bits_array[i] = NULL;
  }
}

#if 0
/* reduce pixel values to 5/6/5, because this is the format they are stored
 *   when compressed anyway, and this way the compression algorithm will use
 *   the exact same color in  adjacent 4x4 tiles and the result is nicer for our purpose.
 *   the lz4 compressed texture is smaller as well. */
static
void FlattenColorsForCompression(unsigned char *data, int dim, bool swap_colors=true)
{
#ifdef __WXMSW__ /* undo BGR flip from ocpn_pixel (if ocpnUSE_ocpnBitmap is \
                    defined) */
    if(swap_colors)
        for(int i = 0; i<dim*dim; i++) {
            int off = 3*i;
            unsigned char t = data[off + 0];
            data[off + 0] = data[off + 2] & 0xfc;
            data[off + 1] &= 0xf8;
            data[off + 2] = t & 0xfc;
        }
        else
#endif
            for(int i = 0; i<dim*dim; i++) {
                int off = 3*i;
                data[off + 0] &= 0xfc;
                data[off + 1] &= 0xf8;
                data[off + 2] &= 0xfc;
            }
}
#endif

/* return malloced data which is the etc compressed texture of the source */
static void CompressDataETC(const unsigned char *data, int dim, int size,
                            unsigned char *tex_data, volatile bool &b_abort) {
  wxASSERT(dim * dim == 2 * size || (dim < 4 && size == 8));  // must be 4bpp
  uint64_t *tex_data64 = (uint64_t *)tex_data;

  int mbrow = wxMin(4, dim), mbcol = wxMin(4, dim);
  uint8_t block[48] = {};
  for (int row = 0; row < dim; row += 4) {
    for (int col = 0; col < dim; col += 4) {
      for (int brow = 0; brow < mbrow; brow++)
        for (int bcol = 0; bcol < mbcol; bcol++)
          memcpy(block + (bcol * 4 + brow) * 3,
                 data + ((row + brow) * dim + col + bcol) * 3, 3);

      extern uint64_t ProcessRGB(const uint8_t *src);
      *tex_data64++ = ProcessRGB(block);
    }
    if (b_abort) break;
  }
}

static bool CompressUsingGPU(const unsigned char *data, int dim, int size,
                             unsigned char *tex_data, int level, bool inplace) {
#ifndef USE_ANDROID_GLES2

  GLuint comp_tex;
  if (!inplace) {
    glGenTextures(1, &comp_tex);
    glBindTexture(GL_TEXTURE_2D, comp_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    level = 0;
  }

  glTexImage2D(GL_TEXTURE_2D, level, g_raster_format, dim, dim, 0, GL_RGB,
               GL_UNSIGNED_BYTE, data);

  GLint compressed;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB,
                           &compressed);
  /* if the compression has been successful */
  if (compressed == GL_TRUE) {
    // If our compressed size is reasonable, save it.
    GLint compressedSize;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
                             GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedSize);

    if (compressedSize != size) return false;

    // Read back the compressed texture.
    glGetCompressedTexImage(GL_TEXTURE_2D, level, tex_data);
  }

  if (!inplace) glDeleteTextures(1, &comp_tex);

  return true;
#else
  return false;
#endif
}

static void GetLevel0Map(glTextureDescriptor *ptd, const wxRect &rect,
                         wxString &chart_path) {
  // Load level 0 uncompressed data
  wxRect ncrect(rect);
  ptd->map_array[0] = 0;

  ChartBase *pChart = ChartData->OpenChartFromDB(chart_path, FULL_INIT);
  if (!pChart) {
    ptd->map_array[0] =
        (unsigned char *)calloc(ncrect.width * ncrect.height * 4, 1);
    return;
  }

  //    Prime the pump with the "zero" level bits, ie. 1x native chart bits
  ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB *>(pChart);

  if (pBSBChart) {
    unsigned char *t_buf =
        (unsigned char *)malloc(ncrect.width * ncrect.height * 4);
    pBSBChart->GetChartBits(ncrect, t_buf, 1);

    //    and cache them here
    ptd->map_array[0] = t_buf;
  } else {
    ptd->map_array[0] =
        (unsigned char *)calloc(ncrect.width * ncrect.height * 4, 1);
    return;
  }
}

void GetFullMap(glTextureDescriptor *ptd, const wxRect &rect,
                wxString chart_path, int level) {
  //  Confirm that the uncompressed bits are all available, get them if not
  //  there yet
  if (ptd->map_array[level]) return;

  // find next lower level with map_array
  int first_level;
  for (first_level = level; first_level; first_level--)
    if (ptd->map_array[first_level - 1]) break;

  // Get level 0 bits from chart?
  if (!first_level) {
    GetLevel0Map(ptd, rect, chart_path);
    first_level = 1;
  }

  int dim = g_GLOptions.m_iTextureDimension;
  for (int i = 0; i <= level; i++) {
    if (i >= first_level) {
      ptd->map_array[i] = (unsigned char *)malloc(dim * dim * 3);
      MipMap_24(2 * dim, 2 * dim, ptd->map_array[i - 1], ptd->map_array[i]);
    }
    dim /= 2;
  }
}

int TextureDim(int level) {
  int dim = g_GLOptions.m_iTextureDimension;
  for (int i = 0; i < level; i++) dim /= 2;
  return dim;
}

int TextureTileSize(int level, bool compressed) {
  if (level == g_mipmap_max_level + 1) return 0;

  int size;
  if (compressed) {
    size = g_tile_size;
    for (int i = 0; i < level; i++) {
      size /= 4;
      if (size < 8) size = 8;
    }
  } else {
    size = g_uncompressed_tile_size;
    for (int i = 0; i < level; i++) size /= 4;
  }

  return size;
}

bool JobTicket::DoJob() {
  if (!m_rect.IsEmpty()) return DoJob(m_rect);

  // otherwise this ticket covers all the rects in the chart
  ChartBase *pchart = ChartData->OpenChartFromDB(m_ChartPath, FULL_INIT);
  if (!pchart) return false;

  ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB *>(pchart);
  if (!pBSBChart) return false;

  int size_X = pBSBChart->GetSize_X();
  int size_Y = pBSBChart->GetSize_Y();

  int dim = g_GLOptions.m_iTextureDimension;

  int nx_tex = ceil((float)size_X / dim);
  int ny_tex = ceil((float)size_Y / dim);

  wxRect rect;
  rect.y = 0;
  rect.width = dim;
  rect.height = dim;
  for (int y = 0; y < ny_tex; y++) {
    if (pthread && pthread->m_pMessageTarget) {
      OCPN_CompressionThreadEvent Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
      Nevent.nstat = y;
      Nevent.nstat_max = ny_tex;
      Nevent.type = 1;
      Nevent.SetTicket(this);
      pthread->m_pMessageTarget->AddPendingEvent(Nevent);
    }

    rect.x = 0;
    for (int x = 0; x < nx_tex; x++) {
      if (!DoJob(rect)) return false;

      pFact->UpdateCacheAllLevels(rect, global_color_scheme,
                                  compcomp_bits_array, compcomp_size_array);

      for (int i = 0; i < g_mipmap_max_level + 1; i++) {
        free(comp_bits_array[i]), comp_bits_array[i] = 0;
        free(compcomp_bits_array[i]), compcomp_bits_array[i] = 0;
      }

      rect.x += rect.width;
    }
    rect.y += rect.height;
  }

  return true;
}

#if 0  // defined( __UNIX__ ) && !defined(__WXOSX__)  // high resolution
       // stopwatch for pro
class OCPNStopWatch
{
public:
    OCPNStopWatch() { Start(); }
    void Start() { clock_gettime(CLOCK_REALTIME, &tp); }

    double Time() {
        timespec tp_end;
        clock_gettime(CLOCK_REALTIME, &tp_end);
        return (tp_end.tv_sec - tp.tv_sec) * 1.e3 + (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
    }

private:
    timespec tp;
};
#else
class OCPNStopWatch : public wxStopWatch {};
#endif

static void throttle_func(void *data) {
  if (!wxThread::IsMain()) {
    OCPNStopWatch *sww = (OCPNStopWatch *)data;
    if (sww->Time() > 1) {
      sww->Start();
      wxThread::Sleep(2);
    }
  }
}

static wxMutex s_mutexProtectingChartBitRead;

bool JobTicket::DoJob(const wxRect &rect) {
  unsigned char *bit_array[10];
  for (int i = 0; i < 10; i++) bit_array[i] = 0;

  wxRect ncrect(rect);

  bit_array[0] = level0_bits;
  level0_bits = NULL;

  if (!bit_array[0]) {
    //  Grab a copy of the level0 chart bits
    // we could alternately subsample grabbing leveln chart bits
    // directly here to speed things up...
    ChartBase *pchart;
    int index;

    if (ChartData) {
      wxMutexLocker lock(s_mutexProtectingChartBitRead);

      index = ChartData->FinddbIndex(m_ChartPath);
      pchart = ChartData->OpenChartFromDBAndLock(index, FULL_INIT);

      if (pchart && ChartData->IsChartLocked(index)) {
        ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB *>(pchart);
        if (pBSBChart) {
          bit_array[0] =
              (unsigned char *)malloc(ncrect.width * ncrect.height * 4);
          pBSBChart->GetChartBits(ncrect, bit_array[0], 1);
        }
        ChartData->UnLockCacheChart(index);
      } else
        bit_array[0] = NULL;
    }
  }

  // OK, got the bits?
  int dim;
  if (!bit_array[0]) return false;

  //  Fill in the rest of the private uncompressed array
  dim = g_GLOptions.m_iTextureDimension;
  dim /= 2;
  for (int i = 1; i < g_mipmap_max_level + 1; i++) {
    size_t nmalloc = wxMax(dim * dim * 3, 4 * 4 * 3);
    bit_array[i] = (unsigned char *)malloc(nmalloc);
    MipMap_24(2 * dim, 2 * dim, bit_array[i - 1], bit_array[i]);
    dim /= 2;
  }

  int texture_level = 0;
  for (int level = level_min_request; level < g_mipmap_max_level + 1; level++) {
    int dim = TextureDim(level);
    int size = TextureTileSize(level, true);
    unsigned char *tex_data = (unsigned char *)malloc(size);
    if (g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
      // color range fit is worse quality but twice as fast
      int flags = squish::kDxt1 | squish::kColourRangeFit;

      if (g_GLOptions.m_bTextureCompressionCaching) {
        /* use slower cluster fit since we are building the cache for
         * better quality, this takes roughly 25% longer and uses about
         * 10% more disk space (result doesn't compress as well with lz4) */
        flags = squish::kDxt1 | squish::kColourClusterFit;
      }

      OCPNStopWatch sww;
      squish::CompressImageRGBpow2_Flatten_Throttle_Abort(
          bit_array[level], dim, dim, tex_data, flags, true,
          b_throttle ? throttle_func : 0, &sww, b_abort);

    } else if (g_raster_format == GL_ETC1_RGB8_OES)
      CompressDataETC(bit_array[level], dim, size, tex_data, b_abort);
    else if (g_raster_format == GL_COMPRESSED_RGB_FXT1_3DFX) {
      if (!CompressUsingGPU(bit_array[level], dim, size, tex_data,
                            texture_level, binplace)) {
        b_abort = true;
        break;
      }

      if (binplace) g_tex_mem_used += size;

      texture_level++;
    }
    comp_bits_array[level] = tex_data;

    if (b_abort) {
      for (int i = 0; i < g_mipmap_max_level + 1; i++) {
        free(bit_array[i]);
        bit_array[i] = 0;
      }
      return false;
    }
  }

  //  All done with the uncompressed data in the thread
  for (int i = 0; i < g_mipmap_max_level + 1; i++) {
    free(bit_array[i]);
    bit_array[i] = 0;
  }

  if (b_throttle) wxThread::Sleep(1);

  if (b_abort) return false;

  if (bpost_zip_compress) {
    int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
    for (int level = level_min_request; level < g_mipmap_max_level + 1;
         level++) {
      if (b_abort) return false;

      unsigned char *compressed_data =
          (unsigned char *)malloc(max_compressed_size);
      int csize = TextureTileSize(level, true);

      char *src = (char *)comp_bits_array[level];
      int compressed_size =
          LZ4_compressHC2(src, (char *)compressed_data, csize, 4);
      // shrink buffer to actual size.
      // This will greatly reduce ram usage, ratio usually 10:1
      // there might be a more efficient way than realloc...
      compressed_data =
          (unsigned char *)realloc(compressed_data, compressed_size);
      compcomp_bits_array[level] = compressed_data;
      compcomp_size_array[level] = compressed_size;
    }
  }

  return true;
}

//  On Windows, we will use a translator to convert SEH exceptions (e.g. access
//  violations),
//    into c++ standard exception handling method.
//  This class and helper function facilitate the conversion.

//  We only do this in the compression worker threads, as they are vulnerable
//  due to possibly errant code in the chart database management class,
//  especially on low memory systems where chart cahing is stressed heavily.

#ifdef __WXMSW__
class SE_Exception {
private:
  unsigned int nSE;

public:
  SE_Exception() {}
  SE_Exception(unsigned int n) : nSE(n) {}
  ~SE_Exception() {}
  unsigned int getSeNumber() { return nSE; }
};

void my_translate(unsigned int code, _EXCEPTION_POINTERS *ep) {
  throw SE_Exception();
}
#endif

OCPN_CompressionThreadEvent::OCPN_CompressionThreadEvent(
    wxEventType commandType, int id)
    : wxEvent(id, commandType) {
  type = 0;
}

OCPN_CompressionThreadEvent::~OCPN_CompressionThreadEvent() {}

wxEvent *OCPN_CompressionThreadEvent::Clone() const {
  OCPN_CompressionThreadEvent *newevent =
      new OCPN_CompressionThreadEvent(*this);
  newevent->m_ticket = this->m_ticket;
  newevent->type = this->type;
  newevent->nstat = this->nstat;
  newevent->nstat_max = this->nstat_max;
  /*
      newevent->m_ticket = new JobTicket;

      newevent->m_ticket->pFact = this->m_ticket->pFact;
      newevent->m_ticket->rect = this->m_ticket->rect;
      newevent->m_ticket->level_min_request = this->m_ticket->level_min_request;
      newevent->m_ticket->ident = this->m_ticket->ident;
      newevent->m_ticket->b_throttle = this->m_ticket->b_throttle;
      newevent->m_ticket->pthread = this->m_ticket->pthread;
      newevent->m_ticket->level0_bits = this->m_ticket->level0_bits;
      newevent->m_ticket->m_ChartPath = this->m_ticket->m_ChartPath;
      newevent->m_ticket->b_abort = this->m_ticket->b_abort;
      newevent->m_ticket->b_isaborted = this->m_ticket->b_isaborted;
      newevent->m_ticket->bpost_zip_compress =
     this->m_ticket->bpost_zip_compress; newevent->m_ticket->state =
     this->m_ticket->state; newevent->m_ticket->tx = this->m_ticket->tx;
      newevent->m_ticket->nx = this->m_ticket->nx;
      newevent->m_ticket->ty = this->m_ticket->ty;
      newevent->m_ticket->ny = this->m_ticket->ny;
      for(int i = 0 ; i < 10 ; i++){
          newevent->m_ticket->comp_bits_array[i] =
     this->m_ticket->comp_bits_array[i];
          newevent->m_ticket->compcomp_bits_array[i] =
     this->m_ticket->compcomp_bits_array[i];
          newevent->m_ticket->compcomp_size_array[i] =
     this->m_ticket->compcomp_size_array[i];
      }
  */
  return newevent;
}

CompressionPoolThread::CompressionPoolThread(JobTicket *ticket,
                                             wxEvtHandler *message_target) {
  m_pMessageTarget = message_target;
  m_ticket = ticket;

  Create();
}

void *CompressionPoolThread::Entry() {
#ifdef __MSVC__
  _set_se_translator(my_translate);

  //  On Windows, if anything in this thread produces a SEH exception (like
  //  access violation) we handle the exception locally, and simply alow the
  //  thread to exit smoothly with no results. Upstream will notice that nothing
  //  got done, and maybe try again later.

  try
#endif
  {
    SetPriority(WXTHREAD_MIN_PRIORITY);

    if (!m_ticket->DoJob()) m_ticket->b_isaborted = true;

    if (m_pMessageTarget) {
      OCPN_CompressionThreadEvent Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
      Nevent.SetTicket(m_ticket);
      Nevent.type = 0;
      m_pMessageTarget->QueueEvent(Nevent.Clone());
      // from here m_ticket is undefined (if deleted in event handler)
    }

    return 0;

  }  // try
#ifdef __MSVC__
  catch (SE_Exception e) {
    if (m_pMessageTarget) {
      OCPN_CompressionThreadEvent Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
      m_ticket->b_isaborted = true;
      Nevent.SetTicket(m_ticket);
      Nevent.type = 0;
      m_pMessageTarget->QueueEvent(Nevent.Clone());
    }

    return 0;
  }
#endif
}

//      ProgressInfoItem Implementation

//      glTextureManager Implementation
glTextureManager::glTextureManager() {
  // ideally we would use the cpu count -1, and only launch jobs
  // when the idle load average is sufficient (greater than 1)
  int nCPU = wxMax(1, wxThread::GetCPUCount());
  if (g_nCPUCount > 0) nCPU = g_nCPUCount;

  if (nCPU < 1)
    // obviously there's at least one CPU!
    nCPU = 1;

  m_max_jobs = wxMax(nCPU, 1);
  m_prevMemUsed = 0;

  if (bthread_debug) printf(" nCPU: %d    m_max_jobs :%d\n", nCPU, m_max_jobs);

  m_progDialog = NULL;

  for (int i = 0; i < m_max_jobs; i++) progList.Append(new ProgressInfoItem);

  //  Create/connect a dynamic event handler slot for messages from the worker
  //  threads
  Connect(
      wxEVT_OCPN_COMPRESSIONTHREAD,
      (wxObjectEventFunction)(wxEventFunction)&glTextureManager::OnEvtThread);

  m_ticks = 0;
  m_skip = false;
  m_bcompact = false;
  m_skipout = false;

  m_timer.Connect(wxEVT_TIMER, wxTimerEventHandler(glTextureManager::OnTimer),
                  NULL, this);
  m_timer.Start(500);
}

glTextureManager::~glTextureManager() {
  //    ClearAllRasterTextures();
  ClearJobList();
}

#define NBAR_LENGTH 40

void glTextureManager::OnEvtThread(OCPN_CompressionThreadEvent &event) {
  JobTicket *ticket = event.GetTicket();

  if (event.type == 1) {
    if (!m_progDialog) {
      // currently unreachable, but...
      return;
    }
    // Look for a matching entry...
    bool bfound = false;
    ProgressInfoItem *item;
    wxProgressInfoListNode *tnode = progList.GetFirst();
    while (tnode) {
      item = tnode->GetData();
      if (item->file_path == ticket->m_ChartPath) {
        bfound = true;
        break;
      }
      tnode = tnode->GetNext();
    }

    if (!bfound) {
      // look for an empty slot
      tnode = progList.GetFirst();
      while (tnode) {
        item = tnode->GetData();
        if (item->file_path.IsEmpty()) {
          bfound = true;
          item->file_path = ticket->m_ChartPath;
          break;
        }
        tnode = tnode->GetNext();
      }
    }

    if (bfound) {
      wxString msgx;
      if (1) {
        int bar_length = NBAR_LENGTH;
        if (m_bcompact) bar_length = 20;

        msgx += _T("\n[");
        wxString block = wxString::Format(_T("%c"), 0x2588);
        float cutoff = -1.;
        if (event.nstat_max != 0)
          cutoff = ((event.nstat + 1) / (float)event.nstat_max) * bar_length;
        for (int i = 0; i < bar_length; i++) {
          if (i <= cutoff)
            msgx += block;
          else
            msgx += _T("-");
        }
        msgx += _T("]");

        if (!m_bcompact) {
          wxString msgy;
          msgy.Printf(_T("  [%3d/%3d]  "), event.nstat + 1, event.nstat_max);
          msgx += msgy;

          wxFileName fn(ticket->m_ChartPath);
          msgx += fn.GetFullName();
        }
      } else
        msgx.Printf(_T("\n %3d/%3d"), event.nstat + 1, event.nstat_max);

      item->msgx = msgx;
    }

    // Ready to compose
    wxString msg;
    tnode = progList.GetFirst();
    while (tnode) {
      item = tnode->GetData();
      msg += item->msgx + _T("\n");
      tnode = tnode->GetNext();
    }

    if (m_skipout) m_progMsg = _T("Skipping, please wait...\n\n");

    if (!m_progDialog->Update(m_jcnt, m_progMsg + msg, &m_skip)) m_skip = true;
    if (m_skip) m_skipout = true;
    return;
  }

  if (ticket->b_isaborted || ticket->b_abort) {
    for (int i = 0; i < g_mipmap_max_level + 1; i++) {
      free(ticket->comp_bits_array[i]);
      free(ticket->compcomp_bits_array[i]);
    }

    if (bthread_debug)
      printf(
          "    Abort job: %08X  Jobs running: %d             Job count: %lu   "
          "\n",
          ticket->ident, GetRunningJobCount(),
          (unsigned long)todo_list.GetCount());
  } else if (!ticket->b_inCompressAll) {
    //   Normal completion from here
    glTextureDescriptor *ptd = ticket->pFact->GetpTD(ticket->m_rect);
    if (ptd) {
      for (int i = 0; i < g_mipmap_max_level + 1; i++)
        ptd->comp_array[i] = ticket->comp_bits_array[i];

      if (ticket->bpost_zip_compress) {
        for (int i = 0; i < g_mipmap_max_level + 1; i++) {
          ptd->compcomp_array[i] = ticket->compcomp_bits_array[i];
          ptd->compcomp_size[i] = ticket->compcomp_size_array[i];
        }
      }

      // We need to force a refresh to replace the uncompressed texture
      // This frees video memory and is also really required if we had
      // gone up a mipmap level
      gFrame->InvalidateAllGL();
      ptd->compdata_ticks = 10;
    }

    if (bthread_debug)
      printf(
          "    Finished job: %08X  Jobs running: %d             Job count: %lu "
          "  \n",
          ticket->ident, GetRunningJobCount(),
          (unsigned long)todo_list.GetCount());
  }

  //      Free all possible memory
  if (ticket->b_inCompressAll) {  // if compressing all write cache here
    ChartBase *pchart =
        ChartData->OpenChartFromDB(ticket->m_ChartPath, FULL_INIT);
    ChartData->DeleteCacheChart(pchart);
    delete ticket->pFact;
  }

  wxProgressInfoListNode *tnode = progList.GetFirst();
  while (tnode) {
    ProgressInfoItem *item = tnode->GetData();
    if (item->file_path == ticket->m_ChartPath) item->file_path = _T("");
    tnode = tnode->GetNext();
  }

  delete ticket;

  if (g_raster_format != GL_COMPRESSED_RGB_FXT1_3DFX) {
    running_list.DeleteObject(ticket);
    StartTopJob();
  }
}

void glTextureManager::OnTimer(wxTimerEvent &event) {
  m_ticks++;

  //  Scrub all the TD's, looking for any completed compression jobs
  //  that have finished
  //  In the interest of not disturbing the GUI, process only one TD per tick
  if (g_GLOptions.m_bTextureCompression) {
    for (ChartPathHashTexfactType::iterator itt =
             m_chart_texfactory_hash.begin();
         itt != m_chart_texfactory_hash.end(); ++itt) {
      glTexFactory *ptf = itt->second;
      if (ptf && ptf->OnTimer()) {
        // break;
      }
    }
  }

#if 0
    if((m_ticks % 4/*120*/) == 0){

    // inventory
    int mem_total, mem_used;
    GetMemoryStatus(&mem_total, &mem_used);

    int map_size = 0;
    int comp_size = 0;
    int compcomp_size = 0;

    for(ChartPathHashTexfactType::iterator itt = m_chart_texfactory_hash.begin();
        itt != m_chart_texfactory_hash.end(); ++itt ) {
        glTexFactory *ptf = itt->second;

        ptf->AccumulateMemStatistics(map_size, comp_size, compcomp_size);
    }

    int m1 = 1024 * 1024;
//    wxString path = wxFileName(m_ChartPath).GetName();
    printf("%6d %6ld Map: %10d  Comp:%10d  CompComp: %10d \n", mem_used/1024, g_tex_mem_used/m1, map_size, comp_size, compcomp_size);//, path.mb_str().data());

///    qDebug() << "inv" << map_size/m1 << comp_size/m1 << compcomp_size/m1 << g_tex_mem_used/m1 << mem_used/1024;
    }
#endif
}

bool glTextureManager::ScheduleJob(glTexFactory *client, const wxRect &rect,
                                   int level, bool b_throttle_thread,
                                   bool b_nolimit, bool b_postZip,
                                   bool b_inplace) {
  wxString chart_path = client->GetChartPath();
  if (!b_nolimit) {
    if (todo_list.GetCount() >= 50) {
      // remove last job which is least important
      wxJobListNode *node = todo_list.GetLast();
      JobTicket *ticket = node->GetData();
      todo_list.DeleteNode(node);
      delete ticket;
    }

    //  Avoid adding duplicate jobs, i.e. the same chart_path, and the same
    //  rectangle
    wxJobListNode *node = todo_list.GetFirst();
    while (node) {
      JobTicket *ticket = node->GetData();
      if ((ticket->m_ChartPath == chart_path) && (ticket->m_rect == rect)) {
        // bump to front
        todo_list.DeleteNode(node);
        todo_list.Insert(ticket);
        ticket->level_min_request = level;
        return false;
      }

      node = node->GetNext();
    }

    // avoid duplicate worker jobs
    wxJobListNode *tnode = running_list.GetFirst();
    while (tnode) {
      JobTicket *ticket = tnode->GetData();
      if (ticket->m_rect == rect && ticket->m_ChartPath == chart_path) {
        return false;
      }
      tnode = tnode->GetNext();
    }
  }

  JobTicket *pt = new JobTicket;
  pt->pFact = client;
  pt->m_rect = rect;
  pt->level_min_request = level;
  glTextureDescriptor *ptd = client->GetOrCreateTD(pt->m_rect);
  pt->ident = (ptd->tex_name << 16) + level;
  pt->b_throttle = b_throttle_thread;
  pt->m_ChartPath = chart_path;

  pt->level0_bits = NULL;
  pt->b_abort = false;
  pt->b_isaborted = false;
  pt->bpost_zip_compress = b_postZip;
  pt->binplace = b_inplace;
  pt->b_inCompressAll = b_inCompressAllCharts;

  /* do we compress in ram using builtin libraries, or do we
     upload to the gpu and use the driver to perform compression?
     we have builtin libraries for DXT1 (squish) and ETC1 (etcpak)
     FXT1 must use the driver, ETC1 cannot, and DXT1 can use the driver
     but the results are worse and don't compress well.

  additionally, if we use the driver we must stay single threaded in this thread
  (unless we created multiple opengl contexts), but with with our own libraries,
  we can use multiple threads to take advantage of multiple cores */

  if (g_raster_format != GL_COMPRESSED_RGB_FXT1_3DFX) {
    todo_list.Insert(pt);  // push to front as a stack
    if (bthread_debug) {
      int mem_used;
      GetMemoryStatus(0, &mem_used);
      printf("Adding job: %08X  Job Count: %lu  mem_used %d\n", pt->ident,
             (unsigned long)todo_list.GetCount(), mem_used);
    }

    StartTopJob();
  } else {
    // give level 0 buffer to the ticket
    pt->level0_bits = ptd->map_array[0];
    ptd->map_array[0] = NULL;

    pt->DoJob();

    OCPN_CompressionThreadEvent Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
    Nevent.type = 0;
    Nevent.SetTicket(pt);
    ProcessEventLocally(Nevent);
    // from here m_ticket is undefined (if deleted in event handler)
  }
  return true;
}

bool glTextureManager::StartTopJob() {
  wxJobListNode *node = todo_list.GetFirst();
  if (!node) return false;

  JobTicket *ticket = node->GetData();

  //  Is it possible to start another job?
  if (GetRunningJobCount() >= wxMax(m_max_jobs - ticket->b_throttle, 1))
    return false;

  todo_list.DeleteNode(node);

  glTextureDescriptor *ptd = ticket->pFact->GetpTD(ticket->m_rect);
  // don't need the job if we already have the compressed data
  if (ptd->comp_array[0]) {
    delete ticket;
    return StartTopJob();
  }

  if (ptd->map_array[0]) {
    if (ticket->level_min_request == 0) {
      // give level 0 buffer to the ticket
      ticket->level0_bits = ptd->map_array[0];
      ptd->map_array[0] = NULL;
    } else {
      // would be nicer to use reference counters
      int size = TextureTileSize(0, false);
      ticket->level0_bits = (unsigned char *)malloc(size);
      memcpy(ticket->level0_bits, ptd->map_array[0], size);
    }
  }

  running_list.Append(ticket);
  DoThreadJob(ticket);

  return true;
}

bool glTextureManager::DoThreadJob(JobTicket *pticket) {
  if (bthread_debug)
    printf("  Starting job: %08X  Jobs running: %d Jobs left: %lu\n",
           pticket->ident, GetRunningJobCount(),
           (unsigned long)todo_list.GetCount());

  ///    qDebug() << "Starting job" << GetRunningJobCount() <<  (unsigned
  ///    long)todo_list.GetCount() << g_tex_mem_used;
  CompressionPoolThread *t = new CompressionPoolThread(pticket, this);
  pticket->pthread = t;

  t->Run();

  return true;
}

bool glTextureManager::AsJob(wxString const &chart_path) const {
  if (chart_path.Len()) {
    wxJobListNode *tnode = running_list.GetFirst();
    while (tnode) {
      JobTicket *ticket = tnode->GetData();
      if (ticket->m_ChartPath.IsSameAs(chart_path)) {
        return true;
      }
      tnode = tnode->GetNext();
    }
  }
  return false;
}

void glTextureManager::PurgeJobList(wxString chart_path) {
  if (chart_path.Len()) {
    //  Remove all pending jobs relating to the passed chart path
    wxJobListNode *next, *tnode = todo_list.GetFirst();
    while (tnode) {
      JobTicket *ticket = tnode->GetData();
      next = tnode->GetNext();
      if (ticket->m_ChartPath.IsSameAs(chart_path)) {
        if (bthread_debug)
          printf("Pool:  Purge pending job for purged chart\n");
        todo_list.DeleteNode(tnode);
        delete ticket;
      }
      tnode = next;
    }

    wxJobListNode *node = running_list.GetFirst();
    while (node) {
      JobTicket *ticket = node->GetData();
      if (ticket->m_ChartPath.IsSameAs(chart_path)) {
        ticket->b_abort = true;
      }
      node = node->GetNext();
    }

    if (bthread_debug)
      printf("Pool:  Purge, todo count: %lu\n",
             (long unsigned)todo_list.GetCount());
  } else {
    wxJobListNode *node = todo_list.GetFirst();
    while (node) {
      JobTicket *ticket = node->GetData();
      delete ticket;
      node = node->GetNext();
    }
    todo_list.Clear();
    //  Mark all running tasks for "abort"
    node = running_list.GetFirst();
    while (node) {
      JobTicket *ticket = node->GetData();
      ticket->b_abort = true;
      node = node->GetNext();
    }
  }
}

void glTextureManager::ClearJobList() {
  wxJobListNode *node = todo_list.GetFirst();
  while (node) {
    JobTicket *ticket = node->GetData();
    delete ticket;
    node = node->GetNext();
  }
  todo_list.Clear();
}

void glTextureManager::ClearAllRasterTextures(void) {
  //     Delete all the TexFactory instances
  ChartPathHashTexfactType::iterator itt;
  for (itt = m_chart_texfactory_hash.begin();
       itt != m_chart_texfactory_hash.end(); ++itt) {
    glTexFactory *ptf = itt->second;

    delete ptf;
  }
  m_chart_texfactory_hash.clear();

  if (g_tex_mem_used != 0)
    wxLogMessage(_T("Texture memory use calculation error\n"));
}

bool glTextureManager::PurgeChartTextures(ChartBase *pc, bool b_purge_factory) {
  //    Look for the texture factory for this chart
  ChartPathHashTexfactType::iterator ittf =
      m_chart_texfactory_hash.find(pc->GetHashKey());

  //    Found ?
  if (ittf != m_chart_texfactory_hash.end()) {
    glTexFactory *pTexFact = ittf->second;

    if (pTexFact) {
      if (b_purge_factory) {
        m_chart_texfactory_hash.erase(ittf);  // This chart  becoming invalid

        delete pTexFact;
      }

      return true;
    } else {
      m_chart_texfactory_hash.erase(ittf);
      return false;
    }
  } else
    return false;
}

bool glTextureManager::TextureCrunch(double factor) {
  double hysteresis = 0.90;

  bool bGLMemCrunch =
      g_tex_mem_used >
      (double)(g_GLOptions.m_iTextureMemorySize * 1024 * 1024) * factor;
  if (!bGLMemCrunch) return false;

  ChartPathHashTexfactType::iterator it0;
  for (it0 = m_chart_texfactory_hash.begin();
       it0 != m_chart_texfactory_hash.end(); ++it0) {
    glTexFactory *ptf = it0->second;
    if (!ptf) continue;
    wxString chart_full_path = ptf->GetChartPath();

    bGLMemCrunch = g_tex_mem_used >
                   (double)(g_GLOptions.m_iTextureMemorySize * 1024 * 1024) *
                       factor * hysteresis;
    if (!bGLMemCrunch) break;

    // For each canvas
    for (unsigned int i = 0; i < g_canvasArray.GetCount(); i++) {
      ChartCanvas *cc = g_canvasArray.Item(i);
      if (cc) {
        if (cc->GetVP().b_quilt)  // quilted
        {
          if (cc->m_pQuilt && cc->m_pQuilt->IsComposed() &&
              !cc->m_pQuilt->IsChartInQuilt(chart_full_path)) {
            ptf->DeleteSomeTextures(g_GLOptions.m_iTextureMemorySize * 1024 *
                                    1024 * factor * hysteresis);
          }
        } else  // not quilted
        {
          if (!cc->m_singleChart->GetFullPath().IsSameAs(chart_full_path)) {
            ptf->DeleteSomeTextures(g_GLOptions.m_iTextureMemorySize * 1024 *
                                    1024 * factor * hysteresis);
          }
        }
      }
    }
  }

  return true;
}

#define MAX_CACHE_FACTORY 50
bool glTextureManager::FactoryCrunch(double factor) {
  if (m_chart_texfactory_hash.size() == 0) {
    /* nothing to free */
    return false;
  }

  int mem_used;
  GetMemoryStatus(0, &mem_used);
  double hysteresis = 0.90;
  ChartPathHashTexfactType::iterator it0;

  bool bMemCrunch =
      (g_memCacheLimit &&
       ((mem_used > (double)(g_memCacheLimit)*factor * hysteresis &&
         mem_used > (double)(m_prevMemUsed)*factor * hysteresis) ||
        (m_chart_texfactory_hash.size() > MAX_CACHE_FACTORY)));

  if (!bMemCrunch) return false;

  //  Need more, so delete the oldest factory
  //      Find the oldest unused factory
  int lru_oldest = 2147483647;
  glTexFactory *ptf_oldest = NULL;

  for (it0 = m_chart_texfactory_hash.begin();
       it0 != m_chart_texfactory_hash.end(); ++it0) {
    glTexFactory *ptf = it0->second;
    if (!ptf) continue;
    wxString chart_full_path = ptf->GetChartPath();

    // we better have to find one because glTexFactory keep cache texture open
    // and ocpn will eventually run out of file descriptors

    // For each canvas
    for (unsigned int i = 0; i < g_canvasArray.GetCount(); i++) {
      ChartCanvas *cc = g_canvasArray.Item(i);
      if (cc) {
        if (cc->GetVP().b_quilt)  // quilted
        {
          if (cc->m_pQuilt && cc->m_pQuilt->IsComposed() &&
              !cc->m_pQuilt->IsChartInQuilt(chart_full_path)) {
            int lru = ptf->GetLRUTime();
            if (lru < lru_oldest && !ptf->BackgroundCompressionAsJob()) {
              lru_oldest = lru;
              ptf_oldest = ptf;
            }
          }
        } else {
          if (!cc->m_singleChart->GetFullPath().IsSameAs(chart_full_path)) {
            int lru = ptf->GetLRUTime();
            if (lru < lru_oldest && !ptf->BackgroundCompressionAsJob()) {
              lru_oldest = lru;
              ptf_oldest = ptf;
            }
          }
        }
      }
    }
  }

  //      Found one?
  if (!ptf_oldest) return false;

  ptf_oldest->FreeSome(g_memCacheLimit * factor * hysteresis);

  GetMemoryStatus(0, &mem_used);

  bMemCrunch = (g_memCacheLimit &&
                ((mem_used > (double)(g_memCacheLimit)*factor * hysteresis &&
                  mem_used > (double)(m_prevMemUsed)*factor * hysteresis) ||
                 (m_chart_texfactory_hash.size() > MAX_CACHE_FACTORY)));

  if (!bMemCrunch) return false;

  //  Need more, so delete the oldest chart too

  m_chart_texfactory_hash.erase(
      ptf_oldest->GetHashKey());  // This chart  becoming invalid

  delete ptf_oldest;

  return true;
}

void glTextureManager::BuildCompressedCache() {
  idx_sorted_by_distance.Clear();

  // Building the cache may take a long time....
  // Be a little smarter.
  // Build a sorted array of chart database indices, sorted on distance from the
  // ownship currently. This way, a user may build a few charts textures for
  // immediate use, then "skip" out on the rest until later.
  int count = 0;
  for (int i = 0; i < ChartData->GetChartTableEntries(); i++) {
    /* skip if not kap */
    const ChartTableEntry &cte = ChartData->GetChartTableEntry(i);
    ChartTypeEnum chart_type = (ChartTypeEnum)cte.GetChartType();
    if (chart_type == CHART_TYPE_PLUGIN) {
      if (cte.GetChartFamily() != CHART_FAMILY_RASTER) continue;
    } else {
      if (chart_type != CHART_TYPE_KAP) continue;
    }

    wxString CompressedCacheFilePath =
        CompressedCachePath(ChartData->GetDBChartFileName(i));
    wxFileName fn(CompressedCacheFilePath);
    //        if(fn.FileExists()) /* skip if file exists */
    //            continue;

    idx_sorted_by_distance.Add(i);

    count++;
  }

  if (count == 0) return;

  wxLogMessage(
      wxString::Format(_T("BuildCompressedCache() count = %d"), count));

  m_timer.Stop();
  PurgeJobList();
  if (GetRunningJobCount()) {
    wxLogMessage(_T("Starting compressor pool drain"));
    wxDateTime now = wxDateTime::Now();
    time_t stall = now.GetTicks();
#define THREAD_WAIT_SECONDS 5
    time_t end = stall + THREAD_WAIT_SECONDS;

    int n_comploop = 0;
    while (stall < end) {
      wxDateTime later = wxDateTime::Now();
      stall = later.GetTicks();

      wxString msg;
      msg.Printf(_T("Time: %d  Job Count: %d"), n_comploop,
                 GetRunningJobCount());
      wxLogMessage(msg);
      if (!GetRunningJobCount()) break;
      wxYield();
      wxSleep(1);
    }

    wxString fmsg;
    fmsg.Printf(_T("Finished compressor pool drain..Time: %d  Job Count: %d"),
                n_comploop, GetRunningJobCount());
    wxLogMessage(fmsg);
  }
  ClearAllRasterTextures();
  b_inCompressAllCharts = true;

  //  Build another array of sorted compression targets.
  //  We need to do this, as the chart table will not be invariant
  //  after the compression threads start, so our index array will be invalid.

  ArrayOfCompressTargets ct_array;
  for (unsigned int j = 0; j < idx_sorted_by_distance.GetCount(); j++) {
    int i = idx_sorted_by_distance[j];

    const ChartTableEntry &cte = ChartData->GetChartTableEntry(i);
    double distance = chart_dist(i);

    wxString filename = cte.GetFullSystemPath();

    compress_target *pct = new compress_target;
    pct->distance = distance;
    pct->chart_path = filename;

    ct_array.Add(pct);
  }

  // create progress dialog
  long style = wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME |
               wxPD_REMAINING_TIME | wxPD_CAN_ABORT;

  wxString msg0;
  msg0 =
      _T("                                                                    ")
      _T("           \n  \n  ");

#ifdef __WXQT__
  msg0 =
      _T("Very ")
      _T("longgggggggggggggggggggggggggggggggggggggggggggg\ngggggggggggggggggg")
      _T("gggggggggggggggggggggggggg top line ");
#endif

  for (int i = 0; i < m_max_jobs + 1; i++)
    msg0 += _T("\n                                             ");

  m_progDialog = new wxGenericProgressDialog();

  wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
  int fontSize = qFont->GetPointSize();
  wxFont *sFont;
  wxSize csz = gFrame->GetClientSize();
  if (csz.x < 500 || csz.y < 500)
    sFont = FontMgr::Get().FindOrCreateFont(
        10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  else
    sFont = FontMgr::Get().FindOrCreateFont(fontSize, wxFONTFAMILY_TELETYPE,
                                            wxFONTSTYLE_NORMAL,
                                            wxFONTWEIGHT_NORMAL);

  m_progDialog->SetFont(*sFont);

  //  Should we use "compact" screen layout?
  wxScreenDC sdc;
  int height, width;
  sdc.GetTextExtent(_T("[WWWWWWWWWWWWWWWWWWWWWWWWWWWWWW]"), &width, &height,
                    NULL, NULL, sFont);
  if (width > (csz.x / 2)) m_bcompact = true;

  m_progDialog->Create(_("OpenCPN Compressed Cache Update"), msg0, count + 1,
                       NULL, style);

  //    Make sure the dialog is big enough to be readable
  m_progDialog->Hide();
  wxSize sz = m_progDialog->GetSize();
  sz.x = csz.x * 9 / 10;
  m_progDialog->SetSize(sz);

  m_progDialog->Layout();
  wxSize sza = m_progDialog->GetSize();

  m_progDialog->Centre();
  m_progDialog->Show();
  m_progDialog->Raise();

  m_skipout = false;
  m_skip = false;
  int yield = 0;

  for (m_jcnt = 0; m_jcnt < ct_array.GetCount(); m_jcnt++) {
    wxString filename = ct_array[m_jcnt].chart_path;
    wxString CompressedCacheFilePath = CompressedCachePath(filename);
    double distance = ct_array[m_jcnt].distance;

    ChartBase *pchart = ChartData->OpenChartFromDBAndLock(filename, FULL_INIT);
    if (!pchart) /* probably a corrupt chart */
      continue;

    // bad things if more than one texfactory for a chart
    g_glTextureManager->PurgeChartTextures(pchart, true);

    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB *>(pchart);
    if (pBSBChart == 0) continue;

    glTexFactory *tex_fact = new glTexFactory(pchart, g_raster_format);

    m_progMsg.Printf(_("Distance from Ownship:  %4.0f NMi"), distance);
    m_progMsg += "\n";
    m_progMsg.Prepend(_T("Preparing RNC Cache...\n"));

    if (m_skipout) {
      g_glTextureManager->PurgeJobList();
      ChartData->DeleteCacheChart(pchart);
      delete tex_fact;
      break;
    }

    int size_X = pBSBChart->GetSize_X();
    int size_Y = pBSBChart->GetSize_Y();

    int tex_dim = g_GLOptions.m_iTextureDimension;

    int nx_tex = ceil((float)size_X / tex_dim);
    int ny_tex = ceil((float)size_Y / tex_dim);

    wxRect rect;
    rect.y = 0;
    rect.width = tex_dim;
    rect.height = tex_dim;
    for (int y = 0; y < ny_tex; y++) {
      rect.x = 0;
      for (int x = 0; x < nx_tex; x++) {
        for (int level = 0; level < g_mipmap_max_level + 1; level++) {
          if (!tex_fact->IsLevelInCache(level, rect, global_color_scheme)) {
            goto schedule;
          }
        }
        rect.x += rect.width;
      }
      rect.y += rect.height;
    }
    //  Nothing to do
    //  Free all possible memory
    ChartData->DeleteCacheChart(pchart);
    delete tex_fact;
    yield++;
    if (yield == 200) {
      ::wxYield();
      yield = 0;
      if (!m_progDialog->Update(m_jcnt)) {
        m_skip = true;
        m_skipout = true;
      }
    }
    continue;

  // some work to do
  schedule:

    yield = 0;
    ScheduleJob(tex_fact, wxRect(), 0, false, true, true, false);
    while (!m_skip) {
      ::wxYield();
      int cnt = GetJobCount() - GetRunningJobCount();
      if (!cnt) break;
      wxThread::Sleep(1);
    }

    if (m_skipout) {
      g_glTextureManager->PurgeJobList();
      ChartData->DeleteCacheChart(pchart);
      delete tex_fact;
      break;
    }
  }

  while (GetRunningJobCount()) {
    wxThread::Sleep(1);
    ::wxYield();
  }

  b_inCompressAllCharts = false;
  m_timer.Start(500);

  delete m_progDialog;
  m_progDialog = nullptr;
}
