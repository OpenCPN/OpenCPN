/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Object
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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "wx/image.h"  // for some reason, needed for msvc???
#include "wx/tokenzr.h"
#include <wx/textfile.h>
#include <wx/filename.h>

#include "dychart.h"
#include "OCPNPlatform.h"

#include "s52s57.h"
#include "s52plib.h"

#include "s57chart.h"

#include "mygeom.h"
#include "model/cutil.h"
#include "model/georef.h"
#include "navutil.h"  // for LogMessageOnce
#include "model/navutil_base.h"
#include "model/plugin_comm.h"
#include "ocpn_pixel.h"
#include "ocpndc.h"
#include "s52utils.h"
#include "model/wx28compat.h"
#include "model/chartdata_input_stream.h"

#include "gdal/cpl_csv.h"
#include "setjmp.h"

#include "ogr_s57.h"

#include "pluginmanager.h"  // for S57 lights overlay

#include "Osenc.h"
#include "chcanv.h"
#include "SencManager.h"
#include "gui_lib.h"
#include "model/logger.h"
#include "Quilt.h"
#include "ocpn_frame.h"

#ifdef __VISUALC__
#include <wx/msw/msvcrt.h>
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#include "linmath.h"
#endif

#include <algorithm>  // for std::sort
#include <map>

#include "ssl/sha1.h"
#ifdef ocpnUSE_GL
#include "shaders.h"
#endif
#include "chart_ctx_factory.h"

#ifdef __MSVC__
#define strncasecmp(x, y, z) _strnicmp(x, y, z)
#endif

#ifdef __ANDROID__
#include "crashlytics.h"
#endif

extern bool GetDoubleAttr(S57Obj *obj, const char *AttrName,
                          double &val);  // found in s52cnsy

void OpenCPN_OGRErrorHandler(
    CPLErr eErrClass, int nError,
    const char *pszErrorMsg);  // installed GDAL OGR library error handler

extern s52plib *ps52plib;
extern S57ClassRegistrar *g_poRegistrar;
extern wxString g_csv_locn;
extern wxString g_SENCPrefix;
extern bool g_bGDAL_Debug;
extern bool g_bDebugS57;
extern MyFrame *gFrame;
extern PlugInManager *g_pi_manager;
extern bool g_b_overzoom_x;
extern bool g_b_EnableVBO;
extern OCPNPlatform *g_Platform;
extern SENCThreadManager *g_SencThreadManager;

int g_SENC_LOD_pixels;

static jmp_buf env_ogrf;  // the context saved by setjmp();

#include <wx/arrimpl.cpp>  // Implement an array of S57 Objects
WX_DEFINE_OBJARRAY(ArrayOfS57Obj);

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ListOfPI_S57Obj);

WX_DEFINE_LIST(ListOfObjRazRules);  // Implement a list ofObjRazRules

#define S57_THUMB_SIZE 200

static int s_bInS57;  // Exclusion flag to prvent recursion in this class init
                      // call. Init() is not reentrant due to static
                      // wxProgressDialog callback....
int s_cnt;

static uint64_t hash_fast64(const void *buf, size_t len, uint64_t seed) {
  const uint64_t m = 0x880355f21e6d1965ULL;
  const uint64_t *pos = (const uint64_t *)buf;
  const uint64_t *end = pos + (len >> 3);
  const unsigned char *pc;
  uint64_t h = len * m ^ seed;
  uint64_t v;
  while (pos != end) {
    v = *pos++;
    v ^= v >> 23;
    v *= 0x2127599bf4325c37ULL;
    h ^= v ^ (v >> 47);
    h *= m;
  }
  pc = (const unsigned char *)pos;
  v = 0;
  switch (len & 7) {
    case 7:
      v ^= (uint64_t)pc[6] << 48;  // FALL THROUGH
    case 6:
      v ^= (uint64_t)pc[5] << 40;  // FALL THROUGH
    case 5:
      v ^= (uint64_t)pc[4] << 32;  // FALL THROUGH
    case 4:
      v ^= (uint64_t)pc[3] << 24;  // FALL THROUGH
    case 3:
      v ^= (uint64_t)pc[2] << 16;  // FALL THROUGH
    case 2:
      v ^= (uint64_t)pc[1] << 8;  // FALL THROUGH
    case 1:
      v ^= (uint64_t)pc[0];
      v ^= v >> 23;
      v *= 0x2127599bf4325c37ULL;
      h ^= v ^ (v >> 47);
      h *= m;
  }

  h ^= h >> 23;
  h *= 0x2127599bf4325c37ULL;
  h ^= h >> 47;
  return h;
}

static unsigned int hash_fast32(const void *buf, size_t len,
                                unsigned int seed) {
  uint64_t h = hash_fast64(buf, len, seed);
  /* The following trick converts the 64-bit hashcode to a
   * residue over a Fermat Number, in which information from
   * both the higher and lower parts of hashcode shall be
   * retained. */
  return h - (h >> 32);
}

unsigned long connector_key::hash() const {
  return hash_fast32(k, sizeof k, 0);
}

//----------------------------------------------------------------------------------
//      render_canvas_parms Implementation
//----------------------------------------------------------------------------------

render_canvas_parms::render_canvas_parms() { pix_buff = NULL; }

render_canvas_parms::~render_canvas_parms(void) {}

static void PrepareForRender(ViewPort *pvp, s52plib *plib) {
  if (!plib) return;

  plib->SetVPointCompat(pvp->pix_width, pvp->pix_height, pvp->view_scale_ppm,
                        pvp->rotation, pvp->clat, pvp->clon, pvp->chart_scale,
                        pvp->rv_rect, pvp->GetBBox(), pvp->ref_scale,
                        GetOCPNCanvasWindow()->GetContentScaleFactor());
  plib->PrepareForRender();
}

//----------------------------------------------------------------------------------
//      s57chart Implementation
//----------------------------------------------------------------------------------

s57chart::s57chart() {
  m_ChartType = CHART_TYPE_S57;
  m_ChartFamily = CHART_FAMILY_VECTOR;

  for (int i = 0; i < PRIO_NUM; i++)
    for (int j = 0; j < LUPNAME_NUM; j++) razRules[i][j] = NULL;

  m_Chart_Scale = 1;  // Will be fetched during Init()
  m_Chart_Skew = 0.0;

  pDIB = NULL;
  m_pCloneBM = NULL;

  // Create ATON arrays, needed by S52PLIB
  pFloatingATONArray = new wxArrayPtrVoid;
  pRigidATONArray = new wxArrayPtrVoid;

  m_tmpup_array = NULL;

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
  m_plib_state_hash = 0;

  m_btex_mem = false;

  ref_lat = 0.0;
  ref_lon = 0.0;

  m_b2pointLUPS = false;
  m_b2lineLUPS = false;

  m_next_safe_cnt = 1e6;
  m_LineVBO_name = -1;
  m_line_vertex_buffer = 0;
  m_this_chart_context = 0;
  m_Chart_Skew = 0;
  m_vbo_byte_length = 0;
  bReadyToRender = false;
  m_RAZBuilt = false;
  m_disableBackgroundSENC = false;
}

s57chart::~s57chart() {
  FreeObjectsAndRules();

  delete pDIB;

  delete m_pCloneBM;
  //    delete pFullPath;

  delete pFloatingATONArray;
  delete pRigidATONArray;

  delete m_pENCDS;

  free(m_pvaldco_array);

  free(m_line_vertex_buffer);

  delete m_pDIBThumbOrphan;

  for (unsigned i = 0; i < m_pcs_vector.size(); i++) delete m_pcs_vector.at(i);

  for (unsigned i = 0; i < m_pve_vector.size(); i++) delete m_pve_vector.at(i);

  m_pcs_vector.clear();
  m_pve_vector.clear();

  for (const auto &it : m_ve_hash) {
    VE_Element *pedge = it.second;
    if (pedge) {
      free(pedge->pPoints);
      delete pedge;
    }
  }
  m_ve_hash.clear();

  for (const auto &it : m_vc_hash) {
    VC_Element *pcs = it.second;
    if (pcs) {
      free(pcs->pPoint);
      delete pcs;
    }
  }
  m_vc_hash.clear();

#ifdef ocpnUSE_GL
  if ((m_LineVBO_name > 0)) glDeleteBuffers(1, (GLuint *)&m_LineVBO_name);
#endif
  free(m_this_chart_context);

  if (m_TempFilePath.Length() && (m_FullPath != m_TempFilePath)) {
    if (::wxFileExists(m_TempFilePath)) wxRemoveFile(m_TempFilePath);
  }

  //  Check the SENCThreadManager to see if this chart is queued or active
  if (g_SencThreadManager) {
    if (g_SencThreadManager->IsChartInTicketlist(this)) {
      g_SencThreadManager->SetChartPointer(this, NULL);
    }
  }
}

void s57chart::GetValidCanvasRegion(const ViewPort &VPoint,
                                    OCPNRegion *pValidRegion) {
  int rxl, rxr;
  int ryb, ryt;
  double easting, northing;
  double epix, npix;

  toSM(m_FullExtent.SLAT, m_FullExtent.WLON, VPoint.clat, VPoint.clon, &easting,
       &northing);
  epix = easting * VPoint.view_scale_ppm;
  npix = northing * VPoint.view_scale_ppm;

  rxl = (int)round((VPoint.pix_width / 2) + epix);
  ryb = (int)round((VPoint.pix_height / 2) - npix);

  toSM(m_FullExtent.NLAT, m_FullExtent.ELON, VPoint.clat, VPoint.clon, &easting,
       &northing);
  epix = easting * VPoint.view_scale_ppm;
  npix = northing * VPoint.view_scale_ppm;

  rxr = (int)round((VPoint.pix_width / 2) + epix);
  ryt = (int)round((VPoint.pix_height / 2) - npix);

  pValidRegion->Clear();
  pValidRegion->Union(rxl, ryt, rxr - rxl, ryb - ryt);
}

LLRegion s57chart::GetValidRegion() {
  double ll[8] = {m_FullExtent.SLAT, m_FullExtent.WLON, m_FullExtent.SLAT,
                  m_FullExtent.ELON, m_FullExtent.NLAT, m_FullExtent.ELON,
                  m_FullExtent.NLAT, m_FullExtent.WLON};
  return LLRegion(4, ll);
}

void s57chart::SetColorScheme(ColorScheme cs, bool bApplyImmediate) {
  if (!ps52plib) return;
  //  Here we convert (subjectively) the Global ColorScheme
  //  to an appropriate S52 Color scheme, by name.

  switch (cs) {
    case GLOBAL_COLOR_SCHEME_DAY:
      ps52plib->SetPLIBColorScheme("DAY", ChartCtxFactory());
      break;
    case GLOBAL_COLOR_SCHEME_DUSK:
      ps52plib->SetPLIBColorScheme("DUSK", ChartCtxFactory());
      break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
      ps52plib->SetPLIBColorScheme("NIGHT", ChartCtxFactory());
      break;
    default:
      ps52plib->SetPLIBColorScheme("DAY", ChartCtxFactory());
      break;
  }

  m_global_color_scheme = cs;

  if (bApplyImmediate) {
    delete pDIB;  // Toss any current cache
    pDIB = NULL;
  }

  //      Clear out any cached bitmaps in the text cache
  ClearRenderedTextCache();

  //      Setup the proper thumbnail bitmap pointer
  ChangeThumbColor(cs);
}

void s57chart::ChangeThumbColor(ColorScheme cs) {
  if (0 == m_pDIBThumbDay) return;

  switch (cs) {
    default:
    case GLOBAL_COLOR_SCHEME_DAY:
      pThumbData->pDIBThumb = m_pDIBThumbDay;
      m_pDIBThumbOrphan = m_pDIBThumbDim;
      break;
    case GLOBAL_COLOR_SCHEME_DUSK:
    case GLOBAL_COLOR_SCHEME_NIGHT: {
      if (NULL == m_pDIBThumbDim) {
        wxImage img = m_pDIBThumbDay->ConvertToImage();

#if wxCHECK_VERSION(2, 8, 0)
        wxImage gimg = img.ConvertToGreyscale(
            0.1, 0.1, 0.1);  // factors are completely subjective
#else
        wxImage gimg = img;
#endif

        // #ifdef ocpnUSE_ocpnBitmap
        //                       ocpnBitmap *pBMP =  new ocpnBitmap(gimg,
        //                       m_pDIBThumbDay->GetDepth());
        // #else
        wxBitmap *pBMP = new wxBitmap(gimg);
        // #endif
        m_pDIBThumbDim = pBMP;
        m_pDIBThumbOrphan = m_pDIBThumbDay;
      }

      pThumbData->pDIBThumb = m_pDIBThumbDim;
      break;
    }
  }
}

bool s57chart::GetChartExtent(Extent *pext) {
  if (m_bExtentSet) {
    *pext = m_FullExtent;
    return true;
  } else
    return false;
}

static void free_mps(mps_container *mps) {
  if (mps == 0) return;
  if (ps52plib && mps->cs_rules) {
    for (unsigned int i = 0; i < mps->cs_rules->GetCount(); i++) {
      Rules *rule_chain_top = mps->cs_rules->Item(i);
      ps52plib->DestroyRulesChain(rule_chain_top);
    }
    delete mps->cs_rules;
  }
  free(mps);
}

void s57chart::FreeObjectsAndRules() {
  //      Delete the created ObjRazRules, including the S57Objs
  //      and any child lists
  //      The LUPs of base elements are deleted elsewhere ( void
  //      s52plib::DestroyLUPArray ( wxArrayOfLUPrec *pLUPArray )) But we need
  //      to manually destroy any LUPS related to children

  ObjRazRules *top;
  ObjRazRules *nxx;
  for (int i = 0; i < PRIO_NUM; ++i) {
    for (int j = 0; j < LUPNAME_NUM; j++) {
      top = razRules[i][j];
      while (top != NULL) {
        top->obj->nRef--;
        if (0 == top->obj->nRef) delete top->obj;

        if (top->child) {
          ObjRazRules *ctop = top->child;
          while (ctop) {
            delete ctop->obj;

            if (ps52plib) ps52plib->DestroyLUP(ctop->LUP);

            ObjRazRules *cnxx = ctop->next;
            delete ctop;
            ctop = cnxx;
          }
        }
        free_mps(top->mps);

        nxx = top->next;
        free(top);
        top = nxx;
      }
    }
  }
}

void s57chart::ClearRenderedTextCache() {
  ObjRazRules *top;
  for (int i = 0; i < PRIO_NUM; ++i) {
    for (int j = 0; j < LUPNAME_NUM; j++) {
      top = razRules[i][j];
      while (top != NULL) {
        if (top->obj->bFText_Added) {
          top->obj->bFText_Added = false;
          delete top->obj->FText;
          top->obj->FText = NULL;
        }

        if (top->child) {
          ObjRazRules *ctop = top->child;
          while (ctop) {
            if (ctop->obj->bFText_Added) {
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

double s57chart::GetNormalScaleMin(double canvas_scale_factor,
                                   bool b_allow_overzoom) {
  //    if( b_allow_overzoom )
  return m_Chart_Scale * 0.125;
  //    else
  //        return m_Chart_Scale * 0.25;
}
double s57chart::GetNormalScaleMax(double canvas_scale_factor,
                                   int canvas_width) {
  return m_Chart_Scale * 4.0;
}

//-----------------------------------------------------------------------
//              Pixel to Lat/Long Conversion helpers
//-----------------------------------------------------------------------

void s57chart::GetPointPix(ObjRazRules *rzRules, float north, float east,
                           wxPoint *r) {
  r->x = roundint(((east - m_easting_vp_center) * m_view_scale_ppm) +
                  m_pixx_vp_center);
  r->y = roundint(m_pixy_vp_center -
                  ((north - m_northing_vp_center) * m_view_scale_ppm));
}

void s57chart::GetPointPix(ObjRazRules *rzRules, wxPoint2DDouble *en,
                           wxPoint *r, int nPoints) {
  for (int i = 0; i < nPoints; i++) {
    r[i].x = roundint(((en[i].m_x - m_easting_vp_center) * m_view_scale_ppm) +
                      m_pixx_vp_center);
    r[i].y = roundint(m_pixy_vp_center -
                      ((en[i].m_y - m_northing_vp_center) * m_view_scale_ppm));
  }
}

void s57chart::GetPixPoint(int pixx, int pixy, double *plat, double *plon,
                           ViewPort *vpt) {
  if (vpt->m_projection_type != PROJECTION_MERCATOR)
    printf("s57chart unhandled projection\n");

  //    Use Mercator estimator
  int dx = pixx - (vpt->pix_width / 2);
  int dy = (vpt->pix_height / 2) - pixy;

  double xp = (dx * cos(vpt->skew)) - (dy * sin(vpt->skew));
  double yp = (dy * cos(vpt->skew)) + (dx * sin(vpt->skew));

  double d_east = xp / vpt->view_scale_ppm;
  double d_north = yp / vpt->view_scale_ppm;

  double slat, slon;
  fromSM(d_east, d_north, vpt->clat, vpt->clon, &slat, &slon);

  *plat = slat;
  *plon = slon;
}

//-----------------------------------------------------------------------
//              Calculate and Set ViewPoint Constants
//-----------------------------------------------------------------------

void s57chart::SetVPParms(const ViewPort &vpt) {
  //  Set up local SM rendering constants
  m_pixx_vp_center = vpt.pix_width / 2.0;
  m_pixy_vp_center = vpt.pix_height / 2.0;
  m_view_scale_ppm = vpt.view_scale_ppm;

  toSM(vpt.clat, vpt.clon, ref_lat, ref_lon, &m_easting_vp_center,
       &m_northing_vp_center);

  vp_transform.easting_vp_center = m_easting_vp_center;
  vp_transform.northing_vp_center = m_northing_vp_center;
}

bool s57chart::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed) {
  if (IsCacheValid()) {
    //      If this viewpoint is same scale as last...
    if (vp_last.view_scale_ppm == vp_proposed.view_scale_ppm) {
      double prev_easting_c, prev_northing_c;
      toSM(vp_last.clat, vp_last.clon, ref_lat, ref_lon, &prev_easting_c,
           &prev_northing_c);

      double easting_c, northing_c;
      toSM(vp_proposed.clat, vp_proposed.clon, ref_lat, ref_lon, &easting_c,
           &northing_c);

      //  then require this viewport to be exact integral pixel difference from
      //  last adjusting clat/clat and SM accordingly

      double delta_pix_x =
          (easting_c - prev_easting_c) * vp_proposed.view_scale_ppm;
      int dpix_x = (int)round(delta_pix_x);
      double dpx = dpix_x;

      double delta_pix_y =
          (northing_c - prev_northing_c) * vp_proposed.view_scale_ppm;
      int dpix_y = (int)round(delta_pix_y);
      double dpy = dpix_y;

      double c_east_d = (dpx / vp_proposed.view_scale_ppm) + prev_easting_c;
      double c_north_d = (dpy / vp_proposed.view_scale_ppm) + prev_northing_c;

      double xlat, xlon;
      fromSM(c_east_d, c_north_d, ref_lat, ref_lon, &xlat, &xlon);

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
 double last_center_easting, last_center_northing, this_center_easting,
 this_center_northing; toSM ( vp_proposed.clat, vp_proposed.clon, ref_lat,
 ref_lon, &this_center_easting, &this_center_northing ); toSM ( vp_last.clat,
 vp_last.clon,     ref_lat, ref_lon, &last_center_easting, &last_center_northing
 );

 int dx = (int)round((last_center_easting  - this_center_easting)  *
 vp_proposed.view_scale_ppm); int dy = (int)round((last_center_northing -
 this_center_northing) * vp_proposed.view_scale_ppm);

 return((dx !=  0) || (dy != 0) || !(IsCacheValid()) ||
 (vp_proposed.view_scale_ppm != vp_last.view_scale_ppm));
 }
 */

void s57chart::LoadThumb() {
  wxFileName fn(m_FullPath);
  wxString SENCdir = g_SENCPrefix;

  if (SENCdir.Last() != fn.GetPathSeparator())
    SENCdir.Append(fn.GetPathSeparator());

  wxFileName tsfn(SENCdir);
  tsfn.SetFullName(fn.GetFullName());

  wxFileName ThumbFileNameLook(tsfn);
  ThumbFileNameLook.SetExt(_T("BMP"));

  wxBitmap *pBMP;
  if (ThumbFileNameLook.FileExists()) {
    pBMP = new wxBitmap;

    pBMP->LoadFile(ThumbFileNameLook.GetFullPath(), wxBITMAP_TYPE_BMP);
    m_pDIBThumbDay = pBMP;
    m_pDIBThumbOrphan = 0;
    m_pDIBThumbDim = 0;
  }
}

ThumbData *s57chart::GetThumbData(int tnx, int tny, float lat, float lon) {
  //  Plot the passed lat/lon at the thumbnail bitmap scale
  //  Using simple linear algorithm.
  if (pThumbData->pDIBThumb == 0) {
    LoadThumb();
    ChangeThumbColor(m_global_color_scheme);
  }

  UpdateThumbData(lat, lon);

  return pThumbData;
}

bool s57chart::UpdateThumbData(double lat, double lon) {
  //  Plot the passed lat/lon at the thumbnail bitmap scale
  //  Using simple linear algorithm.
  int test_x, test_y;
  if (pThumbData->pDIBThumb) {
    double lat_top = m_FullExtent.NLAT;
    double lat_bot = m_FullExtent.SLAT;
    double lon_left = m_FullExtent.WLON;
    double lon_right = m_FullExtent.ELON;

    // Build the scale factors just as the thumbnail was built
    double ext_max = fmax((lat_top - lat_bot), (lon_right - lon_left));

    double thumb_view_scale_ppm = (S57_THUMB_SIZE / ext_max) / (1852 * 60);
    double east, north;
    toSM(lat, lon, (lat_top + lat_bot) / 2., (lon_left + lon_right) / 2., &east,
         &north);

    test_x = pThumbData->pDIBThumb->GetWidth() / 2 +
             (int)(east * thumb_view_scale_ppm);
    test_y = pThumbData->pDIBThumb->GetHeight() / 2 -
             (int)(north * thumb_view_scale_ppm);

  } else {
    test_x = 0;
    test_y = 0;
  }

  if ((test_x != pThumbData->ShipX) || (test_y != pThumbData->ShipY)) {
    pThumbData->ShipX = test_x;
    pThumbData->ShipY = test_y;
    return TRUE;
  } else
    return FALSE;
}

void s57chart::SetFullExtent(Extent &ext) {
  m_FullExtent.NLAT = ext.NLAT;
  m_FullExtent.SLAT = ext.SLAT;
  m_FullExtent.WLON = ext.WLON;
  m_FullExtent.ELON = ext.ELON;

  m_bExtentSet = true;
}

void s57chart::ForceEdgePriorityEvaluate(void) { m_bLinePrioritySet = false; }

void s57chart::SetLinePriorities(void) {
  if (!ps52plib) return;

  //      If necessary.....
  //      Establish line feature rendering priorities

  if (!m_bLinePrioritySet) {
    ObjRazRules *top;
    ObjRazRules *crnt;

    for (int i = 0; i < PRIO_NUM; ++i) {
      top = razRules[i][2];  // LINES
      while (top != NULL) {
        ObjRazRules *crnt = top;
        top = top->next;
        ps52plib->SetLineFeaturePriority(crnt, i);
      }

      //    In the interest of speed, choose only the one necessary area
      //    boundary style index
      int j;
      if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
        j = 4;
      else
        j = 3;

      top = razRules[i][j];
      while (top != NULL) {
        crnt = top;
        top = top->next;  // next object
        ps52plib->SetLineFeaturePriority(crnt, i);
      }
    }

    // Traverse the entire object list again, setting the priority of each
    // line_segment_element to the maximum priority seen for that segment
    for (int i = 0; i < PRIO_NUM; ++i) {
      for (int j = 0; j < LUPNAME_NUM; j++) {
        ObjRazRules *top = razRules[i][j];
        while (top != NULL) {
          S57Obj *obj = top->obj;

          VE_Element *pedge;
          connector_segment *pcs;
          line_segment_element *list = obj->m_ls_list;
          while (list) {
            switch (list->ls_type) {
              case TYPE_EE:
              case TYPE_EE_REV:
                pedge = list->pedge;  // (VE_Element *)list->private0;
                if (pedge) list->priority = pedge->max_priority;
                break;

              default:
                pcs = list->pcs;  //(connector_segment *)list->private0;
                if (pcs) list->priority = pcs->max_priority_cs;
                break;
            }

            list = list->next;
          }

          top = top->next;
        }
      }
    }
  }

  //      Mark the priority as set.
  //      Generally only reset by Options Dialog post processing
  m_bLinePrioritySet = true;
}

#if 0
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
            if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
                j = 4;
            else
                j = 3;

            top = razRules[i][j];
            while( top != NULL ) {
                crnt = top;
                top = top->next;               // next object
                ps52plib->SetLineFeaturePriority( crnt, i );
            }

        }


        // Traverse the entire object list again, setting the priority of each line_segment_element
        // to the maximum priority seen for that segment
        for( int i = 0; i < PRIO_NUM; ++i ) {
            for( int j = 0; j < LUPNAME_NUM; j++ ) {
                ObjRazRules *top = razRules[i][j];
                while( top != NULL ) {
                    S57Obj *obj = top->obj;

                    VE_Element *pedge;
                    connector_segment *pcs;
                    line_segment_element *list = obj->m_ls_list;
                    while( list ){
                        switch (list->type){
                            case TYPE_EE:

                                pedge = (VE_Element *)list->private0;
                                if(pedge)
                                    list->priority = pedge->max_priority;
                                break;

                            default:
                                pcs = (connector_segment *)list->private0;
                                if(pcs)
                                    list->priority = pcs->max_priority;
                                break;
                        }

                        list = list->next;
                    }

                    top = top->next;
                }
            }
        }
    }

    //      Mark the priority as set.
    //      Generally only reset by Options Dialog post processing
    m_bLinePrioritySet = true;
}
#endif

int s57chart::GetLineFeaturePointArray(S57Obj *obj, void **ret_array) {
  //  Walk the line segment list once to get the required array size

  int nPoints = 0;
  line_segment_element *ls_list = obj->m_ls_list;
  while (ls_list) {
    if ((ls_list->ls_type == TYPE_EE) || (ls_list->ls_type == TYPE_EE_REV))
      nPoints += ls_list->pedge->nCount;
    else
      nPoints += 2;
    ls_list = ls_list->next;
  }

  if (!nPoints) {
    *ret_array = 0;
    return 0;
  }

  //  Allocate the buffer
  float *br = (float *)malloc(nPoints * 2 * sizeof(float));
  *ret_array = br;

  // populate the buffer
  unsigned char *source_buffer = (unsigned char *)GetLineVertexBuffer();
  ls_list = obj->m_ls_list;
  while (ls_list) {
    size_t vbo_offset = 0;
    size_t count = 0;
    if ((ls_list->ls_type == TYPE_EE) || (ls_list->ls_type == TYPE_EE_REV)) {
      vbo_offset = ls_list->pedge->vbo_offset;
      count = ls_list->pedge->nCount;
    } else {
      vbo_offset = ls_list->pcs->vbo_offset;
      count = 2;
    }

    memcpy(br, source_buffer + vbo_offset, count * 2 * sizeof(float));
    br += count * 2;
    ls_list = ls_list->next;
  }

  return nPoints;
}

#if 0
int s57chart::GetLineFeaturePointArray(S57Obj *obj, void **ret_array)
{
    //  Walk the line segment list once to get the required array size

    int nPoints = 0;
    line_segment_element *ls_list = obj->m_ls_list;
    while( ls_list){
        nPoints += ls_list->n_points;
        ls_list = ls_list->next;
    }

    if(!nPoints){
        *ret_array = 0;
        return 0;
    }

    //  Allocate the buffer
    float *br = (float *)malloc(nPoints * 2 * sizeof(float));
    *ret_array = br;

    // populate the buffer
    unsigned char *source_buffer = (unsigned char *)GetLineVertexBuffer();
    ls_list = obj->m_ls_list;
    while( ls_list){
        memcpy(br, source_buffer + ls_list->vbo_offset, ls_list->n_points * 2 * sizeof(float));
        br += ls_list->n_points * 2;
        ls_list = ls_list->next;
    }

    return nPoints;

}
#endif

typedef struct segment_pair {
  float e0, n0, e1, n1;
} _segment_pair;

void s57chart::AssembleLineGeometry(void) {
  // Walk the hash tables to get the required buffer size

  //  Start with the edge hash table
  size_t nPoints = 0;
  for (const auto &it : m_ve_hash) {
    VE_Element *pedge = it.second;
    if (pedge) {
      nPoints += pedge->nCount;
    }
  }

  //    printf("time0 %f\n", sw.GetTime());

  std::map<long long, connector_segment *> ce_connector_hash;
  std::map<long long, connector_segment *> ec_connector_hash;
  std::map<long long, connector_segment *> cc_connector_hash;

  std::map<long long, connector_segment *>::iterator csit;

  int ndelta = 0;

  //  Define a vector to temporarily hold the geometry for the created pcs
  //  elements

  std::vector<segment_pair> connector_segment_vector;
  size_t seg_pair_index = 0;

  //  Get the end node connected segments.  To do this, we
  //  walk the Feature array and process each feature that potentially has a
  //  LINE type element
  for (int i = 0; i < PRIO_NUM; ++i) {
    for (int j = 0; j < LUPNAME_NUM; j++) {
      ObjRazRules *top = razRules[i][j];
      while (top != NULL) {
        S57Obj *obj = top->obj;

        if ((!obj->m_ls_list) &&
            (obj->m_n_lsindex))  // object has not been processed yet
        {
          line_segment_element list_top;
          list_top.next = 0;

          line_segment_element *le_current = &list_top;

          for (int iseg = 0; iseg < obj->m_n_lsindex; iseg++) {
            if (!obj->m_lsindex_array) continue;

            int seg_index = iseg * 3;
            int *index_run = &obj->m_lsindex_array[seg_index];

            //  Get first connected node
            unsigned int inode = *index_run++;

            //  Get the edge
            bool edge_dir = true;
            int venode = *index_run++;
            if (venode < 0) {
              venode = -venode;
              edge_dir = false;
            }

            VE_Element *pedge = 0;
            if (venode) {
              if (m_ve_hash.find(venode) != m_ve_hash.end())
                pedge = m_ve_hash[venode];
            }

            //  Get end connected node
            unsigned int enode = *index_run++;

            //  Get first connected node
            VC_Element *ipnode = 0;
            ipnode = m_vc_hash[inode];

            //  Get end connected node
            VC_Element *epnode = 0;
            epnode = m_vc_hash[enode];

            if (ipnode) {
              if (pedge && pedge->nCount) {
                //      The initial node exists and connects to the start of an
                //      edge

                long long key = ((unsigned long long)inode << 32) + venode;

                connector_segment *pcs = NULL;
                csit = ce_connector_hash.find(key);
                if (csit == ce_connector_hash.end()) {
                  ndelta += 2;
                  pcs = new connector_segment;
                  ce_connector_hash[key] = pcs;

                  // capture and store geometry
                  segment_pair pair;
                  float *ppt = ipnode->pPoint;
                  pair.e0 = *ppt++;
                  pair.n0 = *ppt;

                  if (edge_dir) {
                    pair.e1 = pedge->pPoints[0];
                    pair.n1 = pedge->pPoints[1];
                  } else {
                    int last_point_index = (pedge->nCount - 1) * 2;
                    pair.e1 = pedge->pPoints[last_point_index];
                    pair.n1 = pedge->pPoints[last_point_index + 1];
                  }

                  connector_segment_vector.push_back(pair);
                  pcs->vbo_offset = seg_pair_index;  // use temporarily
                  seg_pair_index++;

                  // calculate the centroid of this connector segment, used for
                  // viz testing
                  double lat, lon;
                  fromSM_Plugin((pair.e0 + pair.e1) / 2,
                                (pair.n0 + pair.n1) / 2, ref_lat, ref_lon, &lat,
                                &lon);
                  pcs->cs_lat_avg = lat;
                  pcs->cs_lon_avg = lon;

                } else
                  pcs = csit->second;

                line_segment_element *pls = new line_segment_element;
                pls->next = 0;
                //                            pls->n_points = 2;
                pls->priority = 0;
                pls->pcs = pcs;
                pls->ls_type = TYPE_CE;

                le_current->next = pls;  // hook it up
                le_current = pls;
              }
            }

            if (pedge && pedge->nCount) {
              line_segment_element *pls = new line_segment_element;
              pls->next = 0;
              //                        pls->n_points = pedge->nCount;
              pls->priority = 0;
              pls->pedge = pedge;
              pls->ls_type = TYPE_EE;
              if (!edge_dir) pls->ls_type = TYPE_EE_REV;

              le_current->next = pls;  // hook it up
              le_current = pls;

            }  // pedge

            // end node
            if (epnode) {
              if (ipnode) {
                if (pedge && pedge->nCount) {
                  long long key = ((unsigned long long)venode << 32) + enode;

                  connector_segment *pcs = NULL;
                  csit = ec_connector_hash.find(key);
                  if (csit == ec_connector_hash.end()) {
                    ndelta += 2;
                    pcs = new connector_segment;
                    ec_connector_hash[key] = pcs;

                    // capture and store geometry
                    segment_pair pair;

                    if (!edge_dir) {
                      pair.e0 = pedge->pPoints[0];
                      pair.n0 = pedge->pPoints[1];
                    } else {
                      int last_point_index = (pedge->nCount - 1) * 2;
                      pair.e0 = pedge->pPoints[last_point_index];
                      pair.n0 = pedge->pPoints[last_point_index + 1];
                    }

                    float *ppt = epnode->pPoint;
                    pair.e1 = *ppt++;
                    pair.n1 = *ppt;

                    connector_segment_vector.push_back(pair);
                    pcs->vbo_offset = seg_pair_index;  // use temporarily
                    seg_pair_index++;

                    // calculate the centroid of this connector segment, used
                    // for viz testing
                    double lat, lon;
                    fromSM_Plugin((pair.e0 + pair.e1) / 2,
                                  (pair.n0 + pair.n1) / 2, ref_lat, ref_lon,
                                  &lat, &lon);
                    pcs->cs_lat_avg = lat;
                    pcs->cs_lon_avg = lon;

                  } else
                    pcs = csit->second;

                  line_segment_element *pls = new line_segment_element;
                  pls->next = 0;
                  pls->priority = 0;
                  pls->pcs = pcs;
                  pls->ls_type = TYPE_EC;

                  le_current->next = pls;  // hook it up
                  le_current = pls;

                } else {
                  long long key = ((unsigned long long)inode << 32) + enode;

                  connector_segment *pcs = NULL;
                  csit = cc_connector_hash.find(key);
                  if (csit == cc_connector_hash.end()) {
                    ndelta += 2;
                    pcs = new connector_segment;
                    cc_connector_hash[key] = pcs;

                    // capture and store geometry
                    segment_pair pair;

                    float *ppt = ipnode->pPoint;
                    pair.e0 = *ppt++;
                    pair.n0 = *ppt;

                    ppt = epnode->pPoint;
                    pair.e1 = *ppt++;
                    pair.n1 = *ppt;

                    connector_segment_vector.push_back(pair);
                    pcs->vbo_offset = seg_pair_index;  // use temporarily
                    seg_pair_index++;

                    // calculate the centroid of this connector segment, used
                    // for viz testing
                    double lat, lon;
                    fromSM_Plugin((pair.e0 + pair.e1) / 2,
                                  (pair.n0 + pair.n1) / 2, ref_lat, ref_lon,
                                  &lat, &lon);
                    pcs->cs_lat_avg = lat;
                    pcs->cs_lon_avg = lon;

                  } else
                    pcs = csit->second;

                  line_segment_element *pls = new line_segment_element;
                  pls->next = 0;
                  pls->priority = 0;
                  pls->pcs = pcs;
                  pls->ls_type = TYPE_CC;

                  le_current->next = pls;  // hook it up
                  le_current = pls;
                }
              }
            }

          }  // for

          //  All done, so assign the list to the object
          obj->m_ls_list =
              list_top.next;  // skipping the empty first placeholder element

          //  Rarely, some objects are improperly coded, e.g. cm93
          //  If found, signal this downstream for NIL processing
          if (obj->m_ls_list == NULL) {
            obj->m_n_lsindex = 0;
          }

          // we are all finished with the line segment index array, per object
          free(obj->m_lsindex_array);
          obj->m_lsindex_array = NULL;
        }

        top = top->next;
      }
    }
  }
  //    printf("time1 %f\n", sw.GetTime());

  //  We have the total VBO point count, and a nice hashmap of the connector
  //  segments
  nPoints += ndelta;  // allow for the connector segments

  size_t vbo_byte_length = 2 * nPoints * sizeof(float);

  unsigned char *buffer_offset;
  size_t offset;

  bool grow_buffer = false;

  if (0 == m_vbo_byte_length) {
    m_line_vertex_buffer = (float *)malloc(vbo_byte_length);
    m_vbo_byte_length = vbo_byte_length;
    buffer_offset = (unsigned char *)m_line_vertex_buffer;
    offset = 0;
  } else {
    m_line_vertex_buffer = (float *)realloc(
        m_line_vertex_buffer, m_vbo_byte_length + vbo_byte_length);
    buffer_offset = (unsigned char *)m_line_vertex_buffer + m_vbo_byte_length;
    offset = m_vbo_byte_length;
    m_vbo_byte_length = m_vbo_byte_length + vbo_byte_length;
    grow_buffer = true;
  }

  float *lvr = (float *)buffer_offset;

  //      Copy and edge points as floats,
  //      and recording each segment's offset in the array
  for (const auto &it : m_ve_hash) {
    VE_Element *pedge = it.second;
    if (pedge) {
      memcpy(lvr, pedge->pPoints, pedge->nCount * 2 * sizeof(float));
      lvr += pedge->nCount * 2;

      pedge->vbo_offset = offset;
      offset += pedge->nCount * 2 * sizeof(float);
    }
    //         else
    //             int yyp = 4;        //TODO Why are zero elements being
    //             inserted into m_ve_hash?
  }

  //      Now iterate on the hashmaps, adding the connector segments in the
  //      temporary vector to the VBO buffer At the  same time, populate a
  //      vector, storing the pcs pointers to allow destruction at this class
  //      dtor. This will allow us to destroy (automatically) the pcs hashmaps,
  //      and save some storage

  for (csit = ce_connector_hash.begin(); csit != ce_connector_hash.end();
       ++csit) {
    connector_segment *pcs = csit->second;
    m_pcs_vector.push_back(pcs);

    segment_pair pair = connector_segment_vector.at(pcs->vbo_offset);
    *lvr++ = pair.e0;
    *lvr++ = pair.n0;
    *lvr++ = pair.e1;
    *lvr++ = pair.n1;

    pcs->vbo_offset = offset;
    offset += 4 * sizeof(float);
  }

  for (csit = ec_connector_hash.begin(); csit != ec_connector_hash.end();
       ++csit) {
    connector_segment *pcs = csit->second;
    m_pcs_vector.push_back(pcs);

    segment_pair pair = connector_segment_vector.at(pcs->vbo_offset);
    *lvr++ = pair.e0;
    *lvr++ = pair.n0;
    *lvr++ = pair.e1;
    *lvr++ = pair.n1;

    pcs->vbo_offset = offset;
    offset += 4 * sizeof(float);
  }

  for (csit = cc_connector_hash.begin(); csit != cc_connector_hash.end();
       ++csit) {
    connector_segment *pcs = csit->second;
    m_pcs_vector.push_back(pcs);

    segment_pair pair = connector_segment_vector.at(pcs->vbo_offset);
    *lvr++ = pair.e0;
    *lvr++ = pair.n0;
    *lvr++ = pair.e1;
    *lvr++ = pair.n1;

    pcs->vbo_offset = offset;
    offset += 4 * sizeof(float);
  }

  // And so we can empty the temp buffer
  connector_segment_vector.clear();

  // We can convert the edge hashmap to a vector, to allow  us to destroy the
  // hashmap and at the same time free up the point storage in the VE_Elements,
  // since all the points are now in the VBO buffer
  for (const auto &it : m_ve_hash) {
    VE_Element *pedge = it.second;
    if (pedge) {
      m_pve_vector.push_back(pedge);
      free(pedge->pPoints);
    }
  }
  m_ve_hash.clear();

  // and we can empty the connector hashmap,
  // and at the same time free up the point storage in the VC_Elements, since
  // all the points are now in the VBO buffer
  for (const auto &it : m_vc_hash) {
    VC_Element *pcs = it.second;
    if (pcs) free(pcs->pPoint);
    delete pcs;
  }
  m_vc_hash.clear();

#ifdef ocpnUSE_GL
  if (g_b_EnableVBO) {
    if (grow_buffer) {
      if (m_LineVBO_name > 0) {
        glDeleteBuffers(1, (GLuint *)&m_LineVBO_name);
        m_LineVBO_name = -1;
      }
    }
  }
#endif
}

void s57chart::BuildLineVBO(void) {
#ifdef ocpnUSE_GL
  if (!g_b_EnableVBO) return;

  if (m_LineVBO_name == -1) {
    //      Create the VBO
    GLuint vboId;
    glGenBuffers(1, &vboId);

    // bind VBO in order to use
    glBindBuffer(GL_ARRAY_BUFFER, vboId);

    // upload data to VBO
    // Choice:  Line VBO only, or full VBO with areas.

#if 1
#ifndef USE_ANDROID_GLES2
    glEnableClientState(GL_VERTEX_ARRAY);  // activate vertex coords array
#endif
    glBufferData(GL_ARRAY_BUFFER, m_vbo_byte_length, m_line_vertex_buffer,
                 GL_STATIC_DRAW);

#else
    // get the size of VBO data block needed for all AREA objects
    ObjRazRules *top, *crnt;
    int vbo_area_size_bytes = 0;
    for (int i = 0; i < PRIO_NUM; ++i) {
      if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
        top = razRules[i][4];  // Area Symbolized Boundaries
      else
        top = razRules[i][3];  // Area Plain Boundaries

      while (top != NULL) {
        crnt = top;
        top = top->next;  // next object

        //  Get the vertex data for this object
        PolyTriGroup *ppg_vbo =
            crnt->obj->pPolyTessGeo->Get_PolyTriGroup_head();
        // add the byte length
        vbo_area_size_bytes += ppg_vbo->single_buffer_size;
      }
    }

    glGetError();  // clear it

    // Allocate the VBO
    glBufferData(GL_ARRAY_BUFFER, m_vbo_byte_length + vbo_area_size_bytes, NULL,
                 GL_STATIC_DRAW);

    GLenum err = glGetError();
    if (err) {
      wxString msg;
      msg.Printf(_T("S57 VBO Error 1: %d"), err);
      wxLogMessage(msg);
      printf("S57 VBO Error 1: %d", err);
    }

    // Upload the line vertex data
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vbo_byte_length,
                    m_line_vertex_buffer);

    err = glGetError();
    if (err) {
      wxString msg;
      msg.Printf(_T("S57 VBO Error 2: %d"), err);
      wxLogMessage(msg);
      printf("S57 VBO Error 2: %d", err);
    }

    // Get the Area Object vertices, and add to the VBO, one by one
    int vbo_load_offset = m_vbo_byte_length;

    for (int i = 0; i < PRIO_NUM; ++i) {
      if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
        top = razRules[i][4];  // Area Symbolized Boundaries
      else
        top = razRules[i][3];  // Area Plain Boundaries

      while (top != NULL) {
        crnt = top;
        top = top->next;  // next object

        //  Get the vertex data for this object
        PolyTriGroup *ppg_vbo =
            crnt->obj->pPolyTessGeo->Get_PolyTriGroup_head();

        // append  data to VBO
        glBufferSubData(GL_ARRAY_BUFFER, vbo_load_offset,
                        ppg_vbo->single_buffer_size, ppg_vbo->single_buffer);
        // store the VBO offset in the object
        crnt->obj->vboAreaOffset = vbo_load_offset;
        vbo_load_offset += ppg_vbo->single_buffer_size;
      }
    }

    err = glGetError();
    if (err) {
      wxString msg;
      msg.Printf(_T("S57 VBO Error 3: %d"), err);
      wxLogMessage(msg);
      printf("S57 VBO Error 3: %d", err);
    }

#endif

#ifndef USE_ANDROID_GLES2
    glDisableClientState(GL_VERTEX_ARRAY);  // deactivate vertex array
#endif
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //  Loop and populate all the objects
    //  with the name of the line/area vertex VBO
    for (int i = 0; i < PRIO_NUM; ++i) {
      for (int j = 0; j < LUPNAME_NUM; j++) {
        ObjRazRules *top = razRules[i][j];
        while (top != NULL) {
          S57Obj *obj = top->obj;
          obj->auxParm2 = vboId;
          top = top->next;
        }
      }
    }

    m_LineVBO_name = vboId;
    m_this_chart_context->vboID = vboId;
  }

#endif
}

/*              RectRegion:
 *                      This is the Screen region desired to be updated.  Will
 * be either 1 rectangle(full screen) or two rectangles (panning with FBO
 * accelerated pan logic)
 *
 *              Region:
 *                      This is the LLRegion describing the quilt active region
 * for this chart.
 *
 *              So, Actual rendering area onscreen should be clipped to the
 * intersection of the two regions.
 */

bool s57chart::RenderRegionViewOnGL(const wxGLContext &glc,
                                    const ViewPort &VPoint,
                                    const OCPNRegion &RectRegion,
                                    const LLRegion &Region) {
  if (!m_RAZBuilt) return false;

  return DoRenderRegionViewOnGL(glc, VPoint, RectRegion, Region, false);
}

bool s57chart::RenderOverlayRegionViewOnGL(const wxGLContext &glc,
                                           const ViewPort &VPoint,
                                           const OCPNRegion &RectRegion,
                                           const LLRegion &Region) {
  if (!m_RAZBuilt) return false;

  return DoRenderRegionViewOnGL(glc, VPoint, RectRegion, Region, true);
}

bool s57chart::RenderRegionViewOnGLNoText(const wxGLContext &glc,
                                          const ViewPort &VPoint,
                                          const OCPNRegion &RectRegion,
                                          const LLRegion &Region) {
  if (!m_RAZBuilt) return false;

  bool b_text = ps52plib->GetShowS57Text();
  ps52plib->m_bShowS57Text = false;
  bool b_ret = DoRenderRegionViewOnGL(glc, VPoint, RectRegion, Region, false);
  ps52plib->m_bShowS57Text = b_text;

  return b_ret;
}

bool s57chart::RenderViewOnGLTextOnly(const wxGLContext &glc,
                                      const ViewPort &VPoint) {
  if (!m_RAZBuilt) return false;

#ifdef ocpnUSE_GL

  if (!ps52plib) return false;

  SetVPParms(VPoint);
  PrepareForRender((ViewPort *)&VPoint, ps52plib);

  glChartCanvas::DisableClipRegion();
  DoRenderOnGLText(glc, VPoint);

#endif
  return true;
}

bool s57chart::DoRenderRegionViewOnGL(const wxGLContext &glc,
                                      const ViewPort &VPoint,
                                      const OCPNRegion &RectRegion,
                                      const LLRegion &Region, bool b_overlay) {
  if (!m_RAZBuilt) return false;

#ifdef ocpnUSE_GL

  if (!ps52plib) return false;

  if (g_bDebugS57) printf("\n");

  SetVPParms(VPoint);

  PrepareForRender((ViewPort *)&VPoint, ps52plib);

  if (m_plib_state_hash != ps52plib->GetStateHash()) {
    m_bLinePrioritySet = false;  // need to reset line priorities
    UpdateLUPs(this);            // and update the LUPs
    ClearRenderedTextCache();    // and reset the text renderer,
                               // for the case where depth(height) units change
    ResetPointBBoxes(m_last_vp, VPoint);
    SetSafetyContour();

    m_plib_state_hash = ps52plib->GetStateHash();
  }

  if (VPoint.view_scale_ppm != m_last_vp.view_scale_ppm) {
    ResetPointBBoxes(m_last_vp, VPoint);
  }

  BuildLineVBO();
  SetLinePriorities();

  //        Clear the text declutter list
  ps52plib->ClearTextList();

  ViewPort vp = VPoint;

  // printf("\n");
  // region always has either 1 or 2 rectangles (full screen or panning
  // rectangles)
  for (OCPNRegionIterator upd(RectRegion); upd.HaveRects(); upd.NextRect()) {
    wxRect upr = upd.GetRect();
    // printf("updRect: %d %d %d %d\n",upr.x, upr.y, upr.width, upr.height);

    LLRegion chart_region = vp.GetLLRegion(upd.GetRect());
    chart_region.Intersect(Region);

    if (!chart_region.Empty()) {
      // TODO  I think this needs nore work for alternate Projections...
      //  cm93 vpoint crossing Greenwich, panning east, was rendering areas
      //  incorrectly.
      ViewPort cvp = glChartCanvas::ClippedViewport(VPoint, chart_region);
      //  printf("CVP:  %g %g       %g %g\n",
      //         cvp.GetBBox().GetMinLat(),
      //         cvp.GetBBox().GetMaxLat(),
      //         cvp.GetBBox().GetMinLon(),
      //         cvp.GetBBox().GetMaxLon());

      if (CHART_TYPE_CM93 == GetChartType()) {
        // for now I will revert to the faster rectangle clipping now that
        // rendering order is resolved
        //                glChartCanvas::SetClipRegion(cvp, chart_region);
        glChartCanvas::SetClipRect(cvp, upd.GetRect(), false);
        // ps52plib->m_last_clip_rect = upd.GetRect();
      } else {
#ifdef OPT_USE_ANDROID_GLES2

        // GLES2 will be faster if we setup and use a smaller viewport for each
        // rectangle render. This is because when using shaders, clip operations
        // (e.g. scissor, stencil) happen after the fragment shader executes.
        // However, with a smaller viewport, the fragment shader will not be
        // invoked if the vertices are all outside the vieport.

        wxRect r = upd.GetRect();
        ViewPort *vp = &cvp;
        glViewport(r.x, vp->pix_height - (r.y + r.height), r.width, r.height);

        // mat4x4 m;
        // mat4x4_identity(m);

        mat4x4 I, Q;
        mat4x4_identity(I);

        float yp = vp->pix_height - (r.y + r.height);
        // Translate
        I[3][0] = (-r.x - (float)r.width / 2) * (2.0 / (float)r.width);
        I[3][1] = (r.y + (float)r.height / 2) * (2.0 / (float)r.height);

        // Scale
        I[0][0] *= 2.0 / (float)r.width;
        I[1][1] *= -2.0 / (float)r.height;

        // Rotate
        float angle = 0;
        mat4x4_rotate_Z(Q, I, angle);

        mat4x4_dup((float(*)[4])vp->vp_transform, Q);

#else
        ps52plib->SetReducedBBox(cvp.GetBBox());
        glChartCanvas::SetClipRect(cvp, upd.GetRect(), false);

#endif
      }

      DoRenderOnGL(glc, cvp);

      glChartCanvas::DisableClipRegion();
    }
  }

  //      Update last_vp to reflect current state
  m_last_vp = VPoint;

  //      CALLGRIND_STOP_INSTRUMENTATION

#endif
  return true;
}

bool s57chart::DoRenderOnGL(const wxGLContext &glc, const ViewPort &VPoint) {
#ifdef ocpnUSE_GL

  int i;
  ObjRazRules *top;
  ObjRazRules *crnt;
  ViewPort tvp = VPoint;  // undo const  TODO fix this in PLIB

#if 1

  //      Render the areas quickly
  // bind VBO in order to use

  for (i = 0; i < PRIO_NUM; ++i) {
    if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
      top = razRules[i][4];  // Area Symbolized Boundaries
    else
      top = razRules[i][3];  // Area Plain Boundaries

    while (top != NULL) {
      crnt = top;
      top = top->next;  // next object
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderAreaToGL(glc, crnt);
    }
  }

#else
  //      Render the areas quickly
  for (i = 0; i < PRIO_NUM; ++i) {
    if (PI_GetPLIBBoundaryStyle() == SYMBOLIZED_BOUNDARIES)
      top = razRules[i][4];  // Area Symbolized Boundaries
    else
      top = razRules[i][3];  // Area Plain Boundaries

    while (top != NULL) {
      crnt = top;
      top = top->next;  // next object
      crnt->sm_transform_parms = &vp_transform;

      // This may be a deferred tesselation
      // Don't pre-process the geometry unless the object is to be actually
      // rendered
      if (!crnt->obj->pPolyTessGeo->IsOk()) {
        if (ps52plib->ObjectRenderCheckRules(crnt, &tvp, true)) {
          if (!crnt->obj->pPolyTessGeo->m_pxgeom)
            crnt->obj->pPolyTessGeo->m_pxgeom = buildExtendedGeom(crnt->obj);
        }
      }
      ps52plib->RenderAreaToGL(glc, crnt, &tvp);
    }
  }
#endif
  // qDebug() << "Done areas" << sw.GetTime();

  //    Render the lines and points
  for (i = 0; i < PRIO_NUM; ++i) {
    if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
      top = razRules[i][4];  // Area Symbolized Boundaries
    else
      top = razRules[i][3];  // Area Plain Boundaries
    while (top != NULL) {
      crnt = top;
      top = top->next;  // next object
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToGL(glc, crnt);
    }
  }
  // qDebug() << "Done Boundaries" << sw.GetTime();

  for (i = 0; i < PRIO_NUM; ++i) {
    top = razRules[i][2];  // LINES
    while (top != NULL) {
      crnt = top;
      top = top->next;
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToGL(glc, crnt);
    }
  }

  // qDebug() << "Done Lines" << sw.GetTime();

  for (i = 0; i < PRIO_NUM; ++i) {
    if (ps52plib->m_nSymbolStyle == SIMPLIFIED)
      top = razRules[i][0];  // SIMPLIFIED Points
    else
      top = razRules[i][1];  // Paper Chart Points Points

    while (top != NULL) {
      crnt = top;
      top = top->next;
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToGL(glc, crnt);
    }
  }
  // qDebug() << "Done Points" << sw.GetTime();

#endif  // #ifdef ocpnUSE_GL

  return true;
}

bool s57chart::DoRenderOnGLText(const wxGLContext &glc,
                                const ViewPort &VPoint) {
#ifdef ocpnUSE_GL

  int i;
  ObjRazRules *top;
  ObjRazRules *crnt;
  ViewPort tvp = VPoint;  // undo const  TODO fix this in PLIB

#if 0
    //      Render the areas quickly
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
            top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3];           // Area Plain Boundaries

            while( top != NULL ) {
                crnt = top;
                top = top->next;               // next object
                crnt->sm_transform_parms = &vp_transform;
///                ps52plib->RenderAreaToGL( glc, crnt, &tvp );
            }
    }
#endif

  //    Render the lines and points
  for (i = 0; i < PRIO_NUM; ++i) {
    if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
      top = razRules[i][4];  // Area Symbolized Boundaries
    else
      top = razRules[i][3];  // Area Plain Boundaries

    while (top != NULL) {
      crnt = top;
      top = top->next;  // next object
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToGLText(glc, crnt);
    }

    top = razRules[i][2];  // LINES
    while (top != NULL) {
      crnt = top;
      top = top->next;
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToGLText(glc, crnt);
    }

    if (ps52plib->m_nSymbolStyle == SIMPLIFIED)
      top = razRules[i][0];  // SIMPLIFIED Points
    else
      top = razRules[i][1];  // Paper Chart Points Points

    while (top != NULL) {
      crnt = top;
      top = top->next;
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToGLText(glc, crnt);
    }
  }

#endif  // #ifdef ocpnUSE_GL

  return true;
}

bool s57chart::RenderRegionViewOnDCNoText(wxMemoryDC &dc,
                                          const ViewPort &VPoint,
                                          const OCPNRegion &Region) {
  if (!m_RAZBuilt) return false;

  bool b_text = ps52plib->GetShowS57Text();
  ps52plib->m_bShowS57Text = false;
  bool b_ret = DoRenderRegionViewOnDC(dc, VPoint, Region, false);
  ps52plib->m_bShowS57Text = b_text;

  return true;
}

bool s57chart::RenderRegionViewOnDCTextOnly(wxMemoryDC &dc,
                                            const ViewPort &VPoint,
                                            const OCPNRegion &Region) {
  if (!dc.IsOk()) return false;

  SetVPParms(VPoint);
  PrepareForRender((ViewPort *)&VPoint, ps52plib);

  //  If the viewport is rotated, there will only be one rectangle in the region
  //  so we can take a shortcut...
  if (fabs(VPoint.rotation) > .01) {
    DCRenderText(dc, VPoint);
  } else {
    ViewPort temp_vp = VPoint;
    double temp_lon_left, temp_lat_bot, temp_lon_right, temp_lat_top;

    //    Decompose the region into rectangles,
    OCPNRegionIterator upd(Region);  // get the requested rect list
    while (upd.HaveRects()) {
      wxRect rect = upd.GetRect();

      wxPoint p;
      p.x = rect.x;
      p.y = rect.y;

      temp_vp.GetLLFromPix(p, &temp_lat_top, &temp_lon_left);

      p.x += rect.width;
      p.y += rect.height;
      temp_vp.GetLLFromPix(p, &temp_lat_bot, &temp_lon_right);

      if (temp_lon_right < temp_lon_left)  // presumably crossing Greenwich
        temp_lon_right += 360.;

      temp_vp.GetBBox().Set(temp_lat_bot, temp_lon_left, temp_lat_top,
                            temp_lon_right);

      wxDCClipper clip(dc, rect);
      DCRenderText(dc, temp_vp);

      upd.NextRect();
    }
  }

  return true;
}

bool s57chart::RenderRegionViewOnDC(wxMemoryDC &dc, const ViewPort &VPoint,
                                    const OCPNRegion &Region) {
  if (!m_RAZBuilt) return false;

  return DoRenderRegionViewOnDC(dc, VPoint, Region, false);
}

bool s57chart::RenderOverlayRegionViewOnDC(wxMemoryDC &dc,
                                           const ViewPort &VPoint,
                                           const OCPNRegion &Region) {
  if (!m_RAZBuilt) return false;
  return DoRenderRegionViewOnDC(dc, VPoint, Region, true);
}

bool s57chart::DoRenderRegionViewOnDC(wxMemoryDC &dc, const ViewPort &VPoint,
                                      const OCPNRegion &Region,
                                      bool b_overlay) {
  SetVPParms(VPoint);

  bool force_new_view = false;

  if (Region != m_last_Region) force_new_view = true;

  PrepareForRender((ViewPort *)&VPoint, ps52plib);

  if (m_plib_state_hash != ps52plib->GetStateHash()) {
    m_bLinePrioritySet = false;  // need to reset line priorities
    UpdateLUPs(this);            // and update the LUPs
    ClearRenderedTextCache();    // and reset the text renderer,
                               // for the case where depth(height) units change
    ResetPointBBoxes(m_last_vp, VPoint);
    SetSafetyContour();
  }

  if (VPoint.view_scale_ppm != m_last_vp.view_scale_ppm) {
    ResetPointBBoxes(m_last_vp, VPoint);
  }

  SetLinePriorities();

  bool bnew_view = DoRenderViewOnDC(dc, VPoint, DC_RENDER_ONLY, force_new_view);

  //    If quilting, we need to return a cloned bitmap instead of the original
  //    golden item
  if (VPoint.b_quilt) {
    if (m_pCloneBM) {
      if ((m_pCloneBM->GetWidth() != VPoint.pix_width) ||
          (m_pCloneBM->GetHeight() != VPoint.pix_height)) {
        delete m_pCloneBM;
        m_pCloneBM = NULL;
      }
    }
    if (NULL == m_pCloneBM)
      m_pCloneBM = new wxBitmap(VPoint.pix_width, VPoint.pix_height, -1);

    wxMemoryDC dc_clone;
    dc_clone.SelectObject(*m_pCloneBM);

#ifdef ocpnUSE_DIBSECTION
    ocpnMemDC memdc, dc_org;
#else
    wxMemoryDC memdc, dc_org;
#endif

    pDIB->SelectIntoDC(dc_org);

    //    Decompose the region into rectangles, and fetch them into the target
    //    dc
    OCPNRegionIterator upd(Region);  // get the requested rect list
    while (upd.HaveRects()) {
      wxRect rect = upd.GetRect();
      dc_clone.Blit(rect.x, rect.y, rect.width, rect.height, &dc_org, rect.x,
                    rect.y);
      upd.NextRect();
    }

    dc_clone.SelectObject(wxNullBitmap);
    dc_org.SelectObject(wxNullBitmap);

    //    Create a mask
    if (b_overlay) {
      wxColour nodat = GetGlobalColor(_T ( "NODTA" ));
      wxColour nodat_sub = nodat;

#ifdef ocpnUSE_ocpnBitmap
      nodat_sub = wxColour(nodat.Blue(), nodat.Green(), nodat.Red());
#endif
      m_pMask = new wxMask(*m_pCloneBM, nodat_sub);
      m_pCloneBM->SetMask(m_pMask);
    }

    dc.SelectObject(*m_pCloneBM);
  } else
    pDIB->SelectIntoDC(dc);

  m_last_Region = Region;

  return true;
}

bool s57chart::RenderViewOnDC(wxMemoryDC &dc, const ViewPort &VPoint) {
  //    CALLGRIND_START_INSTRUMENTATION

  SetVPParms(VPoint);

  PrepareForRender((ViewPort *)&VPoint, ps52plib);

  if (m_plib_state_hash != ps52plib->GetStateHash()) {
    m_bLinePrioritySet = false;  // need to reset line priorities
    UpdateLUPs(this);            // and update the LUPs
    ClearRenderedTextCache();    // and reset the text renderer
    SetSafetyContour();
  }

  SetLinePriorities();

  bool bnew_view = DoRenderViewOnDC(dc, VPoint, DC_RENDER_ONLY, false);

  pDIB->SelectIntoDC(dc);

  return bnew_view;

  //    CALLGRIND_STOP_INSTRUMENTATION
}

bool s57chart::DoRenderViewOnDC(wxMemoryDC &dc, const ViewPort &VPoint,
                                RenderTypeEnum option, bool force_new_view) {
  bool bnewview = false;
  wxPoint rul, rlr;
  bool bNewVP = false;

  bool bReallyNew = false;

  double easting_ul, northing_ul;
  double easting_lr, northing_lr;
  double prev_easting_ul = 0., prev_northing_ul = 0.;

  if (ps52plib->GetPLIBColorScheme() != m_lastColorScheme) bReallyNew = true;
  m_lastColorScheme = ps52plib->GetPLIBColorScheme();

  if (VPoint.view_scale_ppm != m_last_vp.view_scale_ppm) bReallyNew = true;

  //      If the scale is very small, do not use the cache to avoid harmonic
  //      difficulties...
  if (VPoint.chart_scale > 1e8) bReallyNew = true;

  wxRect dest(0, 0, VPoint.pix_width, VPoint.pix_height);
  if (m_last_vprect != dest) bReallyNew = true;
  m_last_vprect = dest;

  if (m_plib_state_hash != ps52plib->GetStateHash()) {
    bReallyNew = true;
    m_plib_state_hash = ps52plib->GetStateHash();
  }

  if (bReallyNew) {
    bNewVP = true;
    delete pDIB;
    pDIB = NULL;
    bnewview = true;
  }

  //      Calculate the desired rectangle in the last cached image space
  if (m_last_vp.IsValid()) {
    easting_ul =
        m_easting_vp_center - ((VPoint.pix_width / 2) / m_view_scale_ppm);
    northing_ul =
        m_northing_vp_center + ((VPoint.pix_height / 2) / m_view_scale_ppm);
    easting_lr = easting_ul + (VPoint.pix_width / m_view_scale_ppm);
    northing_lr = northing_ul - (VPoint.pix_height / m_view_scale_ppm);

    double last_easting_vp_center, last_northing_vp_center;
    toSM(m_last_vp.clat, m_last_vp.clon, ref_lat, ref_lon,
         &last_easting_vp_center, &last_northing_vp_center);

    prev_easting_ul =
        last_easting_vp_center - ((m_last_vp.pix_width / 2) / m_view_scale_ppm);
    prev_northing_ul = last_northing_vp_center +
                       ((m_last_vp.pix_height / 2) / m_view_scale_ppm);

    double dx = (easting_ul - prev_easting_ul) * m_view_scale_ppm;
    double dy = (prev_northing_ul - northing_ul) * m_view_scale_ppm;

    rul.x = (int)round((easting_ul - prev_easting_ul) * m_view_scale_ppm);
    rul.y = (int)round((prev_northing_ul - northing_ul) * m_view_scale_ppm);

    rlr.x = (int)round((easting_lr - prev_easting_ul) * m_view_scale_ppm);
    rlr.y = (int)round((prev_northing_ul - northing_lr) * m_view_scale_ppm);

    if ((fabs(dx - wxRound(dx)) > 1e-5) || (fabs(dy - wxRound(dy)) > 1e-5)) {
      if (g_bDebugS57)
        printf(
            "s57chart::DoRender  Cache miss on non-integer pixel delta %g %g\n",
            dx, dy);
      rul.x = 0;
      rul.y = 0;
      rlr.x = 0;
      rlr.y = 0;
      bNewVP = true;
    }

    else if ((rul.x != 0) || (rul.y != 0)) {
      if (g_bDebugS57) printf("newvp due to rul\n");
      bNewVP = true;
    }
  } else {
    rul.x = 0;
    rul.y = 0;
    rlr.x = 0;
    rlr.y = 0;
    bNewVP = true;
  }

  if (force_new_view) bNewVP = true;

  //      Using regions, calculate re-usable area of pDIB

  OCPNRegion rgn_last(0, 0, VPoint.pix_width, VPoint.pix_height);
  OCPNRegion rgn_new(rul.x, rul.y, rlr.x - rul.x, rlr.y - rul.y);
  rgn_last.Intersect(rgn_new);  // intersection is reusable portion

  if (bNewVP && (NULL != pDIB) && !rgn_last.IsEmpty()) {
    int xu, yu, wu, hu;
    rgn_last.GetBox(xu, yu, wu, hu);

    int desx = 0;
    int desy = 0;
    int srcx = xu;
    int srcy = yu;

    if (rul.x < 0) {
      srcx = 0;
      desx = -rul.x;
    }
    if (rul.y < 0) {
      srcy = 0;
      desy = -rul.y;
    }

    ocpnMemDC dc_last;
    pDIB->SelectIntoDC(dc_last);

    ocpnMemDC dc_new;
    PixelCache *pDIBNew =
        new PixelCache(VPoint.pix_width, VPoint.pix_height, BPP);
    pDIBNew->SelectIntoDC(dc_new);

    //        printf("reuse blit %d %d %d %d %d %d\n",desx, desy, wu, hu,  srcx,
    //        srcy);
    dc_new.Blit(desx, desy, wu, hu, (wxDC *)&dc_last, srcx, srcy, wxCOPY);

    //        Ask the plib to adjust the persistent text rectangle list for this
    //        canvas shift This ensures that, on pans, the list stays in
    //        registration with the new text renders to come
    ps52plib->AdjustTextList(desx - srcx, desy - srcy, VPoint.pix_width,
                             VPoint.pix_height);

    dc_new.SelectObject(wxNullBitmap);
    dc_last.SelectObject(wxNullBitmap);

    delete pDIB;
    pDIB = pDIBNew;

    //              OK, now have the re-useable section in place
    //              Next, build the new sections

    pDIB->SelectIntoDC(dc);

    OCPNRegion rgn_delta(0, 0, VPoint.pix_width, VPoint.pix_height);
    OCPNRegion rgn_reused(desx, desy, wu, hu);
    rgn_delta.Subtract(rgn_reused);

    OCPNRegionIterator upd(rgn_delta);  // get the update rect list
    while (upd.HaveRects()) {
      wxRect rect = upd.GetRect();

      //      Build temp ViewPort on this region

      ViewPort temp_vp = VPoint;
      double temp_lon_left, temp_lat_bot, temp_lon_right, temp_lat_top;

      double temp_northing_ul = prev_northing_ul - (rul.y / m_view_scale_ppm) -
                                (rect.y / m_view_scale_ppm);
      double temp_easting_ul = prev_easting_ul + (rul.x / m_view_scale_ppm) +
                               (rect.x / m_view_scale_ppm);
      fromSM(temp_easting_ul, temp_northing_ul, ref_lat, ref_lon, &temp_lat_top,
             &temp_lon_left);

      double temp_northing_lr =
          temp_northing_ul - (rect.height / m_view_scale_ppm);
      double temp_easting_lr =
          temp_easting_ul + (rect.width / m_view_scale_ppm);
      fromSM(temp_easting_lr, temp_northing_lr, ref_lat, ref_lon, &temp_lat_bot,
             &temp_lon_right);

      temp_vp.GetBBox().Set(temp_lat_bot, temp_lon_left, temp_lat_top,
                            temp_lon_right);

      //      Allow some slop in the viewport
      //    TODO Investigate why this fails if greater than 5 percent
      double margin = wxMin(temp_vp.GetBBox().GetLonRange(),
                            temp_vp.GetBBox().GetLatRange()) *
                      0.05;
      temp_vp.GetBBox().EnLarge(margin);

      //      And Render it new piece on the target dc
      //     printf("New Render, rendering %d %d %d %d \n", rect.x, rect.y,
      //     rect.width, rect.height);

      DCRenderRect(dc, temp_vp, &rect);

      upd.NextRect();
    }

    dc.SelectObject(wxNullBitmap);

    bnewview = true;

    //      Update last_vp to reflect the current cached bitmap
    m_last_vp = VPoint;

  }

  else if (bNewVP || (NULL == pDIB)) {
    delete pDIB;
    pDIB = new PixelCache(VPoint.pix_width, VPoint.pix_height,
                          BPP);  // destination

    wxRect full_rect(0, 0, VPoint.pix_width, VPoint.pix_height);
    pDIB->SelectIntoDC(dc);

    //        Clear the text declutter list
    ps52plib->ClearTextList();

    DCRenderRect(dc, VPoint, &full_rect);

    dc.SelectObject(wxNullBitmap);

    bnewview = true;

    //      Update last_vp to reflect the current cached bitmap
    m_last_vp = VPoint;
  }

  return bnewview;
}

int s57chart::DCRenderRect(wxMemoryDC &dcinput, const ViewPort &vp,
                           wxRect *rect) {
  int i;
  ObjRazRules *top;
  ObjRazRules *crnt;

  wxASSERT(rect);
  ViewPort tvp = vp;  // undo const  TODO fix this in PLIB

  //    This does not work due to some issue with ref data of allocated
  //    buffer..... render_canvas_parms pb_spec( rect->x, rect->y, rect->width,
  //    rect->height,  GetGlobalColor ( _T ( "NODTA" ) ));

  render_canvas_parms pb_spec;

  pb_spec.depth = BPP;
  pb_spec.pb_pitch = ((rect->width * pb_spec.depth / 8));
  pb_spec.lclip = rect->x;
  pb_spec.rclip = rect->x + rect->width - 1;
  pb_spec.pix_buff = (unsigned char *)malloc(rect->height * pb_spec.pb_pitch);
  pb_spec.width = rect->width;
  pb_spec.height = rect->height;
  pb_spec.x = rect->x;
  pb_spec.y = rect->y;

#ifdef ocpnUSE_ocpnBitmap
  pb_spec.b_revrgb = true;
#else
  pb_spec.b_revrgb = false;
#endif

  // Preset background
  wxColour color = GetGlobalColor(_T ( "NODTA" ));
  unsigned char r, g, b;
  if (color.IsOk()) {
    r = color.Red();
    g = color.Green();
    b = color.Blue();
  } else
    r = g = b = 0;

  if (pb_spec.depth == 24) {
    for (int i = 0; i < pb_spec.height; i++) {
      unsigned char *p = pb_spec.pix_buff + (i * pb_spec.pb_pitch);
      for (int j = 0; j < pb_spec.width; j++) {
        *p++ = r;
        *p++ = g;
        *p++ = b;
      }
    }
  } else {
    int color_int = ((r) << 16) + ((g) << 8) + (b);

    for (int i = 0; i < pb_spec.height; i++) {
      int *p = (int *)(pb_spec.pix_buff + (i * pb_spec.pb_pitch));
      for (int j = 0; j < pb_spec.width; j++) {
        *p++ = color_int;
      }
    }
  }

  //      Render the areas quickly
  for (i = 0; i < PRIO_NUM; ++i) {
    if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
      top = razRules[i][4];  // Area Symbolized Boundaries
    else
      top = razRules[i][3];  // Area Plain Boundaries

    while (top != NULL) {
      crnt = top;
      top = top->next;  // next object
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderAreaToDC(&dcinput, crnt, &pb_spec);
    }
  }

//      Convert the Private render canvas into a bitmap
#ifdef ocpnUSE_ocpnBitmap
  ocpnBitmap *pREN = new ocpnBitmap(pb_spec.pix_buff, pb_spec.width,
                                    pb_spec.height, pb_spec.depth);
#else
  wxImage *prender_image = new wxImage(pb_spec.width, pb_spec.height, false);
  prender_image->SetData((unsigned char *)pb_spec.pix_buff);
  wxBitmap *pREN = new wxBitmap(*prender_image);

#endif

  //      Map it into a temporary DC
  wxMemoryDC dc_ren;
  dc_ren.SelectObject(*pREN);

  //      Blit it onto the target dc
  dcinput.Blit(pb_spec.x, pb_spec.y, pb_spec.width, pb_spec.height,
               (wxDC *)&dc_ren, 0, 0);

  //      And clean up the mess
  dc_ren.SelectObject(wxNullBitmap);

#ifdef ocpnUSE_ocpnBitmap
  free(pb_spec.pix_buff);
#else
  delete prender_image;  // the image owns the data
                         // and so will free it in due course
#endif

  delete pREN;

  //      Render the rest of the objects/primitives
  DCRenderLPB(dcinput, vp, rect);

  return 1;
}

bool s57chart::DCRenderLPB(wxMemoryDC &dcinput, const ViewPort &vp,
                           wxRect *rect) {
  int i;
  ObjRazRules *top;
  ObjRazRules *crnt;
  ViewPort tvp = vp;  // undo const  TODO fix this in PLIB

  for (i = 0; i < PRIO_NUM; ++i) {
    //      Set up a Clipper for Lines
    wxDCClipper *pdcc = NULL;
    //      if( rect ) {
    //         wxRect nr = *rect;
    //         pdcc = new wxDCClipper(dcinput, nr);
    //      }

    if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
      top = razRules[i][4];  // Area Symbolized Boundaries
    else
      top = razRules[i][3];  // Area Plain Boundaries
    while (top != NULL) {
      crnt = top;
      top = top->next;  // next object
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToDC(&dcinput, crnt);
    }

    top = razRules[i][2];  // LINES
    while (top != NULL) {
      crnt = top;
      top = top->next;
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToDC(&dcinput, crnt);
    }

    if (ps52plib->m_nSymbolStyle == SIMPLIFIED)
      top = razRules[i][0];  // SIMPLIFIED Points
    else
      top = razRules[i][1];  // Paper Chart Points Points

    while (top != NULL) {
      crnt = top;
      top = top->next;
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToDC(&dcinput, crnt);
    }

    //      Destroy Clipper
    if (pdcc) delete pdcc;
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

bool s57chart::DCRenderText(wxMemoryDC &dcinput, const ViewPort &vp) {
  int i;
  ObjRazRules *top;
  ObjRazRules *crnt;
  ViewPort tvp = vp;  // undo const  TODO fix this in PLIB

  for (i = 0; i < PRIO_NUM; ++i) {
    if (ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES)
      top = razRules[i][4];  // Area Symbolized Boundaries
    else
      top = razRules[i][3];  // Area Plain Boundaries

    while (top != NULL) {
      crnt = top;
      top = top->next;  // next object
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToDCText(&dcinput, crnt);
    }

    top = razRules[i][2];  // LINES
    while (top != NULL) {
      crnt = top;
      top = top->next;
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToDCText(&dcinput, crnt);
    }

    if (ps52plib->m_nSymbolStyle == SIMPLIFIED)
      top = razRules[i][0];  // SIMPLIFIED Points
    else
      top = razRules[i][1];  // Paper Chart Points Points

    while (top != NULL) {
      crnt = top;
      top = top->next;
      crnt->sm_transform_parms = &vp_transform;
      ps52plib->RenderObjectToDCText(&dcinput, crnt);
    }
  }

  return true;
}

bool s57chart::IsCellOverlayType(const wxString &FullPath) {
  wxFileName fn(FullPath);
  //      Get the "Usage" character
  wxString cname = fn.GetName();
  if (cname.Length() >= 3)
    return ((cname[2] == 'L') || (cname[2] == 'A'));
  else
    return false;
}

InitReturn s57chart::Init(const wxString &name, ChartInitFlag flags) {
  // Really can only Init and use S57 chart if the S52 Presentation Library is
  // present and OK
  if ((NULL == ps52plib) || !(ps52plib->m_bOK)) return INIT_FAIL_REMOVE;

  wxString ext;
  if (name.Upper().EndsWith(".XZ")) {
    ext = wxFileName(name.Left(name.Length() - 3)).GetExt();

    // decompress to temp file to allow seeking
    m_TempFilePath = wxFileName::GetTempDir() + wxFileName::GetPathSeparator() +
                     wxFileName(name).GetName();

    if (!wxFileExists(m_TempFilePath) &&
        !DecompressXZFile(name, m_TempFilePath)) {
      wxRemoveFile(m_TempFilePath);
      return INIT_FAIL_REMOVE;
    }
  } else {
    m_TempFilePath = name;
    ext = wxFileName(name).GetExt();
  }
  m_FullPath = name;

#ifdef __ANDROID__
  firebase::crashlytics::SetCustomKey("s57chartInit",
                                      name.ToStdString().c_str());
#endif

  //    Use a static semaphore flag to prevent recursion
  if (s_bInS57) {
    //          printf("s57chart::Init() recursion..., retry\n");
    //          wxLogMessage(_T("Recursion"));
    return INIT_FAIL_NOERROR;
  }

  s_bInS57++;

  InitReturn ret_value = INIT_OK;

  m_Description = name;

  wxFileName fn(m_TempFilePath);

  //      Get the "Usage" character
  wxString cname = fn.GetName();
  m_usage_char = cname[2];

  //  Establish a common reference point for the chart
  ref_lat = (m_FullExtent.NLAT + m_FullExtent.SLAT) / 2.;
  ref_lon = (m_FullExtent.WLON + m_FullExtent.ELON) / 2.;

  if (flags == THUMB_ONLY) {
    // Look for Thumbnail
    // LoadThumb();

    s_bInS57--;
    return INIT_OK;
  }

  if (flags == HEADER_ONLY) {
    if (ext == _T("000")) {
      if (!GetBaseFileAttr(fn.GetFullPath()))
        ret_value = INIT_FAIL_REMOVE;
      else {
        if (!CreateHeaderDataFromENC())
          ret_value = INIT_FAIL_REMOVE;
        else
          ret_value = INIT_OK;
      }
    } else if (ext == _T("S57")) {
      m_SENCFileName = m_TempFilePath;
      if (!CreateHeaderDataFromSENC())
        ret_value = INIT_FAIL_REMOVE;
      else
        ret_value = INIT_OK;
    }

    s_bInS57--;
    return ret_value;
  }

  //      Full initialization from here

  if (!m_bbase_file_attr_known) {
    if (!GetBaseFileAttr(m_TempFilePath))
      ret_value = INIT_FAIL_REMOVE;
    else
      m_bbase_file_attr_known = true;
  }

  if (ext == _T("000")) {
    if (m_bbase_file_attr_known) {
      int sret = FindOrCreateSenc(m_FullPath);
      if (sret == BUILD_SENC_PENDING) {
        s_bInS57--;
        return INIT_OK;
      }

      if (sret != BUILD_SENC_OK) {
        if (sret == BUILD_SENC_NOK_RETRY)
          ret_value = INIT_FAIL_RETRY;
        else
          ret_value = INIT_FAIL_REMOVE;
      } else
        ret_value = PostInit(flags, m_global_color_scheme);
    }

  }

  else if (ext == _T("S57")) {
    m_SENCFileName = m_TempFilePath;
    ret_value = PostInit(flags, m_global_color_scheme);
  }

  s_bInS57--;
  return ret_value;
}

wxString s57chart::buildSENCName(const wxString &name) {
  wxFileName fn(name);
  fn.SetExt(_T("S57"));
  wxString file_name = fn.GetFullName();

  //      Set the proper directory for the SENC files
  wxString SENCdir = g_SENCPrefix;

  if (SENCdir.Last() != wxFileName::GetPathSeparator())
    SENCdir.Append(wxFileName::GetPathSeparator());

#if 1
  wxString source_dir = fn.GetPath(wxPATH_GET_SEPARATOR);
  wxCharBuffer buf = source_dir.ToUTF8();
  unsigned char sha1_out[20];
  sha1((unsigned char *)buf.data(), strlen(buf.data()), sha1_out);

  wxString sha1;
  for (unsigned int i = 0; i < 6; i++) {
    wxString s;
    s.Printf(_T("%02X"), sha1_out[i]);
    sha1 += s;
  }
  sha1 += _T("_");
  file_name.Prepend(sha1);
#endif

  wxFileName tsfn(SENCdir);
  tsfn.SetFullName(file_name);

  return tsfn.GetFullPath();
}

//-----------------------------------------------------------------------------------------------
//    Find or Create a relevent SENC file from a given .000 ENC file
//    Returns with error code, and associated SENC file name in m_S57FileName
//-----------------------------------------------------------------------------------------------
int s57chart::FindOrCreateSenc(const wxString &name, bool b_progress) {
  //  This method may be called for a compressed .000 cell, so check and
  //  decompress if necessary
  wxString ext;
  if (name.Upper().EndsWith(".XZ")) {
    ext = wxFileName(name.Left(name.Length() - 3)).GetExt();

    // decompress to temp file to allow seeking
    m_TempFilePath = wxFileName::GetTempDir() + wxFileName::GetPathSeparator() +
                     wxFileName(name).GetName();

    if (!wxFileExists(m_TempFilePath) &&
        !DecompressXZFile(name, m_TempFilePath)) {
      wxRemoveFile(m_TempFilePath);
      return INIT_FAIL_REMOVE;
    }
  } else {
    m_TempFilePath = name;
    ext = wxFileName(name).GetExt();
  }
  m_FullPath = name;

  if (!m_bbase_file_attr_known) {
    if (!GetBaseFileAttr(m_TempFilePath))
      return INIT_FAIL_REMOVE;
    else
      m_bbase_file_attr_known = true;
  }

  //      Establish location for SENC files
  m_SENCFileName = buildSENCName(name);

  int build_ret_val = 1;

  bool bbuild_new_senc = false;
  m_bneed_new_thumbnail = false;

  wxFileName FileName000(m_TempFilePath);

  //      Look for SENC file in the target directory

  wxString msg(_T("S57chart::Checking SENC file: "));
  msg.Append(m_SENCFileName);
  wxLogMessage(msg);

  {
    int force_make_senc = 0;

    if (::wxFileExists(m_SENCFileName)) {  // SENC file exists

      Osenc senc;
      if (senc.ingestHeader(m_SENCFileName)) {
        bbuild_new_senc = true;
        wxLogMessage(_T("    Rebuilding SENC due to ingestHeader failure."));
      } else {
        int senc_file_version = senc.getSencReadVersion();

        int last_update = senc.getSENCReadLastUpdate();

        wxString str = senc.getSENCFileCreateDate();
        wxDateTime SENCCreateDate;
        SENCCreateDate.ParseFormat(str, _T("%Y%m%d"));

        if (SENCCreateDate.IsValid())
          SENCCreateDate.ResetTime();  // to midnight

        //                wxULongLong size000 = senc.getFileSize000();
        //                wxString ssize000 = senc.getsFileSize000();

        wxString senc_base_edtn = senc.getSENCReadBaseEdition();
        long isenc_edition;
        senc_base_edtn.ToLong(&isenc_edition);
        long ifile_edition;
        m_edtn000.ToLong(&ifile_edition);

        //              Anything to do?
        // force_make_senc = 1;
        //  SENC file version has to be correct for other tests to make sense
        if (senc_file_version != CURRENT_SENC_FORMAT_VERSION) {
          bbuild_new_senc = true;
          wxLogMessage(_T("    Rebuilding SENC due to SENC format update."));
        }

        //  Senc EDTN must be the same as .000 file EDTN.
        //  This test catches the usual case where the .000 file is updated from
        //  the web, and all updates (.001, .002, etc.)  are subsumed.

        else if (ifile_edition > isenc_edition) {
          bbuild_new_senc = true;
          wxLogMessage(_T("    Rebuilding SENC due to cell edition update."));
          wxString msg;
          msg = _T("    Last edition recorded in SENC: ");
          msg += senc_base_edtn;
          msg += _T("  most recent edition cell file: ");
          msg += m_edtn000;
          wxLogMessage(msg);
        } else {
          //    See if there are any new update files  in the ENC directory
          int most_recent_update_file =
              GetUpdateFileArray(FileName000, NULL, m_date000, m_edtn000);

          if (ifile_edition == isenc_edition) {
            if (most_recent_update_file > last_update) {
              bbuild_new_senc = true;
              wxLogMessage(
                  _T("    Rebuilding SENC due to incremental cell update."));
              wxString msg;
              msg.Printf(
                  _T("    Last update recorded in SENC: %d   most recent ")
                  _T("update file: %d"),
                  last_update, most_recent_update_file);
              wxLogMessage(msg);
            }
          }

          //          Make simple tests to see if the .000 file is "newer" than
          //          the SENC file representation These tests may be redundant,
          //          since the DSID:EDTN test above should catch new base files
          wxDateTime OModTime000;
          FileName000.GetTimes(NULL, &OModTime000, NULL);
          OModTime000.ResetTime();  // to midnight
          if (SENCCreateDate.IsValid()) {
            if (OModTime000.IsLaterThan(SENCCreateDate)) {
              wxLogMessage(
                  _T("    Rebuilding SENC due to Senc vs cell file time ")
                  _T("check."));
              bbuild_new_senc = true;
            }
          } else {
            bbuild_new_senc = true;
            wxLogMessage(
                _T("    Rebuilding SENC due to SENC create time invalid."));
          }

          //                     int Osize000l = FileName000.GetSize().GetLo();
          //                     int Osize000h = FileName000.GetSize().GetHi();
          //                     wxString t;
          //                     t.Printf(_T("%d%d"), Osize000h, Osize000l);
          //                     if( !t.IsSameAs( ssize000) )
          //                         bbuild_new_senc = true;
        }

        if (force_make_senc) bbuild_new_senc = true;
      }
    } else if (!::wxFileExists(m_SENCFileName))  // SENC file does not exist
    {
      wxLogMessage(_T("    Rebuilding SENC due to missing SENC file."));
      bbuild_new_senc = true;
    }
  }

  if (bbuild_new_senc) {
    m_bneed_new_thumbnail =
        true;  // force a new thumbnail to be built in PostInit()
    build_ret_val = BuildSENCFile(m_TempFilePath, m_SENCFileName, b_progress);

    if (BUILD_SENC_PENDING == build_ret_val) return BUILD_SENC_PENDING;
    if (BUILD_SENC_NOK_PERMANENT == build_ret_val) return INIT_FAIL_REMOVE;
    if (BUILD_SENC_NOK_RETRY == build_ret_val) return INIT_FAIL_RETRY;
  }

  return INIT_OK;
}

InitReturn s57chart::PostInit(ChartInitFlag flags, ColorScheme cs) {
  //    SENC file is ready, so build the RAZ structure
  if (0 != BuildRAZFromSENCFile(m_SENCFileName)) {
    wxString msg(_T("   Cannot load SENC file "));
    msg.Append(m_SENCFileName);
    wxLogMessage(msg);

    return INIT_FAIL_RETRY;
  }

//      Check for and if necessary rebuild Thumbnail
//      Going to be in the global (user) SENC file directory
#if 0
  wxString SENCdir = g_SENCPrefix;
  if (SENCdir.Last() != wxFileName::GetPathSeparator())
    SENCdir.Append(wxFileName::GetPathSeparator());

  wxFileName s57File(m_SENCFileName);
  wxFileName ThumbFileName(SENCdir, s57File.GetName().Mid(13), _T("BMP"));

  if (!ThumbFileName.FileExists() || m_bneed_new_thumbnail) {
    BuildThumbnail(ThumbFileName.GetFullPath());

    //  Update the member thumbdata structure
    if (ThumbFileName.FileExists()) {
      wxBitmap *pBMP_NEW;
#ifdef ocpnUSE_ocpnBitmap
      pBMP_NEW = new ocpnBitmap;
#else
      pBMP_NEW = new wxBitmap;
#endif
      if (pBMP_NEW->LoadFile(ThumbFileName.GetFullPath(), wxBITMAP_TYPE_BMP)) {
        delete pThumbData;
        pThumbData = new ThumbData;
        m_pDIBThumbDay = pBMP_NEW;
        //                    pThumbData->pDIBThumb = pBMP_NEW;
      }
    }
  }
#endif

  //    Set the color scheme
  m_global_color_scheme = cs;
  SetColorScheme(cs, false);

  //    Build array of contour values for later use by conditional symbology
  BuildDepthContourArray();

  CreateChartContext();
  PopulateObjectsWithContext();

  m_RAZBuilt = true;
  bReadyToRender = true;

  return INIT_OK;
}

void s57chart::ClearDepthContourArray(void) {
  if (m_nvaldco_alloc) {
    free(m_pvaldco_array);
  }
  m_nvaldco_alloc = 5;
  m_nvaldco = 0;
  m_pvaldco_array = (double *)calloc(m_nvaldco_alloc, sizeof(double));
}

void s57chart::BuildDepthContourArray(void) {
  //    Build array of contour values for later use by conditional symbology

  if (0 == m_nvaldco_alloc) {
    m_nvaldco_alloc = 5;
    m_pvaldco_array = (double *)calloc(m_nvaldco_alloc, sizeof(double));
  }

  ObjRazRules *top;
  // some ENC have a lot of DEPCNT objects but they seem to store them
  // in VALDCO order, try to take advantage of that.
  double prev_valdco = 0.0;

  for (int i = 0; i < PRIO_NUM; ++i) {
    for (int j = 0; j < LUPNAME_NUM; j++) {
      top = razRules[i][j];
      while (top != NULL) {
        if (!strncmp(top->obj->FeatureName, "DEPCNT", 6)) {
          double valdco = 0.0;
          if (GetDoubleAttr(top->obj, "VALDCO", valdco)) {
            if (valdco != prev_valdco) {
              prev_valdco = valdco;
              m_nvaldco++;
              if (m_nvaldco > m_nvaldco_alloc) {
                void *tr = realloc((void *)m_pvaldco_array,
                                   m_nvaldco_alloc * 2 * sizeof(double));
                m_pvaldco_array = (double *)tr;
                m_nvaldco_alloc *= 2;
              }
              m_pvaldco_array[m_nvaldco - 1] = valdco;
            }
          }
        }
        ObjRazRules *nxx = top->next;
        top = nxx;
      }
    }
  }
  std::sort(m_pvaldco_array, m_pvaldco_array + m_nvaldco);
  SetSafetyContour();
}

void s57chart::SetSafetyContour(void) {
  // Iterate through the array of contours in this cell, choosing the best one
  // to render as a bold "safety contour" in the PLIB.

  //    This method computes the smallest chart DEPCNT:VALDCO value which
  //    is greater than or equal to the current PLIB mariner parameter
  //    S52_MAR_SAFETY_CONTOUR

  double mar_safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);

  int i = 0;
  if (NULL != m_pvaldco_array) {
    for (i = 0; i < m_nvaldco; i++) {
      if (m_pvaldco_array[i] >= mar_safety_contour) break;
    }

    if (i < m_nvaldco)
      m_next_safe_cnt = m_pvaldco_array[i];
    else
      m_next_safe_cnt = (double)1e6;
  } else {
    m_next_safe_cnt = (double)1e6;
  }

  // A safety contour greater than "Deep Depth" makes no sense...
  // So, declare "no suitable safety depth contour"
  if (m_next_safe_cnt > S52_getMarinerParam(S52_MAR_DEEP_CONTOUR))
    m_next_safe_cnt = (double)1e6;
}

void s57chart::CreateChartContext() {
  //  Set up the chart context
  m_this_chart_context = (chart_context *)calloc(sizeof(chart_context), 1);
}

void s57chart::PopulateObjectsWithContext() {
  m_this_chart_context->chart = this;
  m_this_chart_context->chart_type = GetChartType();
  m_this_chart_context->vertex_buffer = GetLineVertexBuffer();
  m_this_chart_context->chart_scale = GetNativeScale();
  m_this_chart_context->pFloatingATONArray = pFloatingATONArray;
  m_this_chart_context->pRigidATONArray = pRigidATONArray;
  m_this_chart_context->safety_contour = m_next_safe_cnt;
  m_this_chart_context->pt2GetAssociatedObjects =
      &s57chart::GetAssociatedObjects;

  //  Loop and populate all the objects
  ObjRazRules *top;
  for (int i = 0; i < PRIO_NUM; ++i) {
    for (int j = 0; j < LUPNAME_NUM; j++) {
      top = razRules[i][j];
      while (top != NULL) {
        S57Obj *obj = top->obj;
        obj->m_chart_context = m_this_chart_context;
        top = top->next;
      }
    }
  }
}

void s57chart::InvalidateCache() {
  delete pDIB;
  pDIB = NULL;
}

bool s57chart::BuildThumbnail(const wxString &bmpname) {
  bool ret_code;

  wxFileName ThumbFileName(bmpname);

  //      Make the target directory if needed
  if (true != ThumbFileName.DirExists(ThumbFileName.GetPath())) {
    if (!ThumbFileName.Mkdir(ThumbFileName.GetPath())) {
      wxLogMessage(_T("   Cannot create BMP file directory for ") +
                   ThumbFileName.GetFullPath());
      return false;
    }
  }

  //      Set up a private ViewPort
  ViewPort vp;

  vp.clon = (m_FullExtent.ELON + m_FullExtent.WLON) / 2.;
  vp.clat = (m_FullExtent.NLAT + m_FullExtent.SLAT) / 2.;

  float ext_max = fmax((m_FullExtent.NLAT - m_FullExtent.SLAT),
                       (m_FullExtent.ELON - m_FullExtent.WLON));

  vp.view_scale_ppm = (S57_THUMB_SIZE / ext_max) / (1852 * 60);

  vp.pix_height = S57_THUMB_SIZE;
  vp.pix_width = S57_THUMB_SIZE;

  vp.m_projection_type = PROJECTION_MERCATOR;

  vp.GetBBox().Set(m_FullExtent.SLAT, m_FullExtent.WLON, m_FullExtent.NLAT,
                   m_FullExtent.ELON);

  vp.chart_scale = 10000000 - 1;
  vp.ref_scale = vp.chart_scale;
  vp.Validate();

  // cause a clean new render
  delete pDIB;
  pDIB = NULL;

  SetVPParms(vp);

  //      Borrow the OBJLArray temporarily to set the object type visibility for
  //      this render First, make a copy for the curent OBJLArray viz settings,
  //      setting current value to invisible

  unsigned int OBJLCount = ps52plib->pOBJLArray->GetCount();
  //      int *psave_viz = new int[OBJLCount];
  int *psave_viz = (int *)malloc(OBJLCount * sizeof(int));

  int *psvr = psave_viz;
  OBJLElement *pOLE;
  unsigned int iPtr;

  for (iPtr = 0; iPtr < OBJLCount; iPtr++) {
    pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));
    *psvr++ = pOLE->nViz;
    pOLE->nViz = 0;
  }

  //      Also, save some other settings
  bool bsavem_bShowSoundgp = ps52plib->m_bShowSoundg;
  bool bsave_text = ps52plib->m_bShowS57Text;

  // SetDisplayCategory may clear Noshow array
  ps52plib->SaveObjNoshow();

  //      Now, set up what I want for this render
  for (iPtr = 0; iPtr < OBJLCount; iPtr++) {
    pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));
    if (!strncmp(pOLE->OBJLName, "LNDARE", 6)) pOLE->nViz = 1;
    if (!strncmp(pOLE->OBJLName, "DEPARE", 6)) pOLE->nViz = 1;
  }

  ps52plib->m_bShowSoundg = false;
  ps52plib->m_bShowS57Text = false;

  //      Use display category MARINERS_STANDARD to force use of OBJLArray
  DisCat dsave = ps52plib->GetDisplayCategory();
  ps52plib->SetDisplayCategory(MARINERS_STANDARD);

  ps52plib->AddObjNoshow("BRIDGE");
  ps52plib->AddObjNoshow("GATCON");

  double safety_depth = S52_getMarinerParam(S52_MAR_SAFETY_DEPTH);
  S52_setMarinerParam(S52_MAR_SAFETY_DEPTH, -100);
  double safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);
  S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, -100);

#ifdef ocpnUSE_DIBSECTION
  ocpnMemDC memdc, dc_org;
#else
  wxMemoryDC memdc, dc_org;
#endif

  //      set the color scheme
  ps52plib->SaveColorScheme();
  ps52plib->SetPLIBColorScheme("DAY", ChartCtxFactory());
  //      Do the render
  DoRenderViewOnDC(memdc, vp, DC_RENDER_ONLY, true);

  //      Release the DIB
  memdc.SelectObject(wxNullBitmap);

  //      Restore the plib to previous state
  psvr = psave_viz;
  for (iPtr = 0; iPtr < OBJLCount; iPtr++) {
    pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));
    pOLE->nViz = *psvr++;
  }

  ps52plib->SetDisplayCategory(dsave);
  ps52plib->RestoreObjNoshow();

  ps52plib->RemoveObjNoshow("BRIDGE");
  ps52plib->RemoveObjNoshow("GATCON");

  ps52plib->m_bShowSoundg = bsavem_bShowSoundgp;
  ps52plib->m_bShowS57Text = bsave_text;

  S52_setMarinerParam(S52_MAR_SAFETY_DEPTH, safety_depth);
  S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, safety_contour);

  //      Reset the color scheme
  ps52plib->RestoreColorScheme();

  //       delete psave_viz;
  free(psave_viz);

  //      Clone pDIB into pThumbData;
  wxBitmap *pBMP;

  pBMP = new wxBitmap(vp.pix_width, vp.pix_height /*,  BPP*/);

  wxMemoryDC dc_clone;
  dc_clone.SelectObject(*pBMP);

  pDIB->SelectIntoDC(dc_org);

  dc_clone.Blit(0, 0, vp.pix_width, vp.pix_height, (wxDC *)&dc_org, 0, 0);

  dc_clone.SelectObject(wxNullBitmap);
  dc_org.SelectObject(wxNullBitmap);

  //   Save the file
  ret_code = pBMP->SaveFile(ThumbFileName.GetFullPath(), wxBITMAP_TYPE_BMP);

  delete pBMP;

  return ret_code;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_ARRAY_PTR(float *, MyFloatPtrArray);

//    Read the .000 ENC file and create required Chartbase data structures
bool s57chart::CreateHeaderDataFromENC(void) {
  if (!InitENCMinimal(m_TempFilePath)) {
    wxString msg(_T("   Cannot initialize ENC file "));
    msg.Append(m_TempFilePath);
    wxLogMessage(msg);

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
  std::vector<int> auxCntArray, noCovrCntArray;

  MyFloatPtrArray *pNoCovrPtrArray = new MyFloatPtrArray;

  // Get the first M_COVR object
  pFeat = GetChartFirstM_COVR(catcov);

  while (pFeat) {
    //    Get the next M_COVR feature, and create possible additional entries
    //    for COVR
    OGRPolygon *poly = (OGRPolygon *)(pFeat->GetGeometryRef());
    OGRLinearRing *xring = poly->getExteriorRing();

    int npt = xring->getNumPoints();
    int usedpts = 0;

    float *pf = NULL;
    float *pfr = NULL;

    if (npt >= 3) {
      // pf = (float *) malloc( 2 * sizeof(float) );

      OGRPoint last_p;
      OGRPoint p;
      for (int i = 0; i < npt; i++) {
        xring->getPoint(i, &p);
        if (i >
            3) {  // We need at least 3 points, so make sure the first 3 pass
          float xdelta =
              fmax(last_p.getX(), p.getX()) - fmin(last_p.getX(), p.getX());
          float ydelta =
              fmax(last_p.getY(), p.getY()) - fmin(last_p.getY(), p.getY());
          if (xdelta < 0.001 &&
              ydelta < 0.001) {  // Magic number, 0.001 degrees ~= 111 meters on
                                 // the equator...
            continue;
          }
        }
        last_p = p;
        usedpts++;
        pf = (float *)realloc(pf, 2 * usedpts * sizeof(float));
        pfr = &pf[2 * (usedpts - 1)];

        if (catcov == 1) {
          LatMax = fmax(LatMax, p.getY());
          LatMin = fmin(LatMin, p.getY());
          LonMax = fmax(LonMax, p.getX());
          LonMin = fmin(LonMin, p.getX());
        }

        pfr[0] = p.getY();  // lat
        pfr[1] = p.getX();  // lon
      }

      if (catcov == 1) {
        pAuxPtrArray->Add(pf);
        auxCntArray.push_back(usedpts);
      } else if (catcov == 2) {
        pNoCovrPtrArray->Add(pf);
        noCovrCntArray.push_back(usedpts);
      }
    }

    delete pFeat;
    pFeat = GetChartNextM_COVR(catcov);
    DEBUG_LOG << "used " << usedpts << " points";
  }  // while

  //    Allocate the storage

  m_nCOVREntries = auxCntArray.size();

  //    Create new COVR entries

  if (m_nCOVREntries >= 1) {
    m_pCOVRTablePoints = (int *)malloc(m_nCOVREntries * sizeof(int));
    m_pCOVRTable = (float **)malloc(m_nCOVREntries * sizeof(float *));

    for (unsigned int j = 0; j < (unsigned int)m_nCOVREntries; j++) {
      m_pCOVRTablePoints[j] = auxCntArray[j];
      m_pCOVRTable[j] = pAuxPtrArray->Item(j);
    }
  }

  else  // strange case, found no CATCOV=1 M_COVR objects
  {
    wxString msg(_T("   ENC contains no useable M_COVR, CATCOV=1 features:  "));
    msg.Append(m_TempFilePath);
    wxLogMessage(msg);
  }

  //      And for the NoCovr regions
  m_nNoCOVREntries = noCovrCntArray.size();

  if (m_nNoCOVREntries) {
    //    Create new NoCOVR entries
    m_pNoCOVRTablePoints = (int *)malloc(m_nNoCOVREntries * sizeof(int));
    m_pNoCOVRTable = (float **)malloc(m_nNoCOVREntries * sizeof(float *));

    for (unsigned int j = 0; j < (unsigned int)m_nNoCOVREntries; j++) {
      m_pNoCOVRTablePoints[j] = noCovrCntArray[j];
      m_pNoCOVRTable[j] = pNoCovrPtrArray->Item(j);
    }
  } else {
    m_pNoCOVRTablePoints = NULL;
    m_pNoCOVRTable = NULL;
  }

  delete pAuxPtrArray;
  delete pNoCovrPtrArray;

  if (0 == m_nCOVREntries) {  // fallback
    wxString msg(_T("   ENC contains no M_COVR features:  "));
    msg.Append(m_TempFilePath);
    wxLogMessage(msg);

    msg = _T("   Calculating Chart Extents as fallback.");
    wxLogMessage(msg);

    OGREnvelope Env;

    //    Get the reader
    S57Reader *pENCReader = m_pENCDS->GetModule(0);

    if (pENCReader->GetExtent(&Env, true) == OGRERR_NONE) {
      LatMax = Env.MaxY;
      LonMax = Env.MaxX;
      LatMin = Env.MinY;
      LonMin = Env.MinX;

      m_nCOVREntries = 1;
      m_pCOVRTablePoints = (int *)malloc(sizeof(int));
      *m_pCOVRTablePoints = 4;
      m_pCOVRTable = (float **)malloc(sizeof(float *));
      float *pf = (float *)malloc(2 * 4 * sizeof(float));
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
      wxString msg(_T("   Cannot calculate Extents for ENC:  "));
      msg.Append(m_TempFilePath);
      wxLogMessage(msg);

      return false;  // chart is completely unusable
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
  GetChartNameFromTXT(m_TempFilePath, nice_name);
  m_Name = nice_name;

  return true;
}

//    Read the .S57 oSENC file (CURRENT_SENC_FORMAT_VERSION >= 200) and create
//    required Chartbase data structures
bool s57chart::CreateHeaderDataFromoSENC(void) {
  bool ret_val = true;

  wxFFileInputStream fpx(m_SENCFileName);
  if (!fpx.IsOk()) {
    if (!::wxFileExists(m_SENCFileName)) {
      wxString msg(_T("   Cannot open SENC file "));
      msg.Append(m_SENCFileName);
      wxLogMessage(msg);
    }
    return false;
  }

  Osenc senc;
  if (senc.ingestHeader(m_SENCFileName)) {
    return false;
  } else {
    // Get Chartbase member elements from the oSENC file records in the header

    // Scale
    m_Chart_Scale = senc.getSENCReadScale();

    // Nice Name
    m_Name = senc.getReadName();

    // ID
    m_ID = senc.getReadID();

    // Extents
    Extent &ext = senc.getReadExtent();

    m_FullExtent.ELON = ext.ELON;
    m_FullExtent.WLON = ext.WLON;
    m_FullExtent.NLAT = ext.NLAT;
    m_FullExtent.SLAT = ext.SLAT;
    m_bExtentSet = true;

    // Coverage areas
    SENCFloatPtrArray &AuxPtrArray = senc.getSENCReadAuxPointArray();
    std::vector<int> &AuxCntArray = senc.getSENCReadAuxPointCountArray();

    m_nCOVREntries = AuxCntArray.size();

    m_pCOVRTablePoints = (int *)malloc(m_nCOVREntries * sizeof(int));
    m_pCOVRTable = (float **)malloc(m_nCOVREntries * sizeof(float *));

    for (unsigned int j = 0; j < (unsigned int)m_nCOVREntries; j++) {
      m_pCOVRTablePoints[j] = AuxCntArray[j];
      m_pCOVRTable[j] = (float *)malloc(AuxCntArray[j] * 2 * sizeof(float));
      memcpy(m_pCOVRTable[j], AuxPtrArray[j],
             AuxCntArray[j] * 2 * sizeof(float));
    }

    // NoCoverage areas
    SENCFloatPtrArray &NoCovrPtrArray = senc.getSENCReadNOCOVRPointArray();
    std::vector<int> &NoCovrCntArray = senc.getSENCReadNOCOVRPointCountArray();

    m_nNoCOVREntries = NoCovrCntArray.size();

    if (m_nNoCOVREntries) {
      //    Create new NoCOVR entries
      m_pNoCOVRTablePoints = (int *)malloc(m_nNoCOVREntries * sizeof(int));
      m_pNoCOVRTable = (float **)malloc(m_nNoCOVREntries * sizeof(float *));

      for (unsigned int j = 0; j < (unsigned int)m_nNoCOVREntries; j++) {
        int npoints = NoCovrCntArray[j];
        m_pNoCOVRTablePoints[j] = npoints;
        m_pNoCOVRTable[j] = (float *)malloc(npoints * 2 * sizeof(float));
        memcpy(m_pNoCOVRTable[j], NoCovrPtrArray[j],
               npoints * 2 * sizeof(float));
      }
    }

    //  Misc
    m_SE = m_edtn000;
    m_datum_str = _T("WGS84");
    m_SoundingsDatum = _T("MEAN LOWER LOW WATER");

    int senc_file_version = senc.getSencReadVersion();

    int last_update = senc.getSENCReadLastUpdate();

    wxString str = senc.getSENCFileCreateDate();
    wxDateTime SENCCreateDate;
    SENCCreateDate.ParseFormat(str, _T("%Y%m%d"));

    if (SENCCreateDate.IsValid()) SENCCreateDate.ResetTime();  // to midnight

    wxString senc_base_edtn = senc.getSENCReadBaseEdition();
  }

  return ret_val;
}

//    Read the .S57 SENC file and create required Chartbase data structures
bool s57chart::CreateHeaderDataFromSENC(void) {
  if (CURRENT_SENC_FORMAT_VERSION >= 200) return CreateHeaderDataFromoSENC();

  return false;
}

/*    This method returns the smallest chart DEPCNT:VALDCO value which
 is greater than or equal to the specified value
 */
bool s57chart::GetNearestSafeContour(double safe_cnt, double &next_safe_cnt) {
  int i = 0;
  if (NULL != m_pvaldco_array) {
    for (i = 0; i < m_nvaldco; i++) {
      if (m_pvaldco_array[i] >= safe_cnt) break;
    }

    if (i < m_nvaldco)
      next_safe_cnt = m_pvaldco_array[i];
    else
      next_safe_cnt = (double)1e6;
    return true;
  } else {
    next_safe_cnt = (double)1e6;
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

std::list<S57Obj *> *s57chart::GetAssociatedObjects(S57Obj *obj) {
  int disPrioIdx;
  bool gotit;

  std::list<S57Obj *> *pobj_list = new std::list<S57Obj *>();

  double lat, lon;
  fromSM((obj->x * obj->x_rate) + obj->x_origin,
         (obj->y * obj->y_rate) + obj->y_origin, ref_lat, ref_lon, &lat, &lon);
  //    What is the entry object geometry type?

  switch (obj->Primitive_type) {
    case GEO_POINT:
      //  n.b.  This logic not perfectly right for LINE and AREA features
      //  It uses the object reference point for testing, instead of the
      //  decomposed line or boundary geometry.  Thus, it may fail on some
      //  intersecting relationships. Judged acceptable, in favor of performance
      //  implications. DSR
    case GEO_LINE:
    case GEO_AREA:
      ObjRazRules *top;
      disPrioIdx = 1;  // PRIO_GROUP1:S57 group 1 filled areas

      gotit = false;
      top = razRules[disPrioIdx][3];  // PLAIN_BOUNDARIES
      while (top != NULL) {
        if (top->obj->bIsAssociable) {
          if (top->obj->BBObj.Contains(lat, lon)) {
            if (IsPointInObjArea(lat, lon, 0.0, top->obj)) {
              pobj_list->push_back(top->obj);
              gotit = true;
              break;
            }
          }
        }

        ObjRazRules *nxx = top->next;
        top = nxx;
      }

      if (!gotit) {
        top = razRules[disPrioIdx][4];  // SYMBOLIZED_BOUNDARIES
        while (top != NULL) {
          if (top->obj->bIsAssociable) {
            if (top->obj->BBObj.Contains(lat, lon)) {
              if (IsPointInObjArea(lat, lon, 0.0, top->obj)) {
                pobj_list->push_back(top->obj);
                break;
              }
            }
          }

          ObjRazRules *nxx = top->next;
          top = nxx;
        }
      }

      break;

    default:
      break;
  }

  return pobj_list;
}

void s57chart::GetChartNameFromTXT(const wxString &FullPath, wxString &Name) {
  wxFileName fn(FullPath);

  wxString target_name = fn.GetName();
  target_name.RemoveLast();

  wxString dir_name = fn.GetPath();

  wxDir dir(dir_name);  // The directory containing the file

  wxArrayString FileList;

  dir.GetAllFiles(fn.GetPath(), &FileList);  // list all the files

  //    Iterate on the file list...

  bool found_name = false;
  wxString name;
  name.Clear();

  for (unsigned int j = 0; j < FileList.GetCount(); j++) {
    wxFileName file(FileList[j]);
    if (((file.GetExt()).MakeUpper()) == _T("TXT")) {
      //  Look for the line beginning with the name of the .000 file
      wxTextFile text_file(file.GetFullPath());

      bool file_ok = true;
      //  Suppress log messages on bad file reads
      {
        wxLogNull logNo;
        if (!text_file.Open()) {
          if (!text_file.Open(wxConvISO8859_1)) file_ok = false;
        }
      }

      if (file_ok) {
        wxString str = text_file.GetFirstLine();
        while (!text_file.Eof()) {
          if (0 == target_name.CmpNoCase(
                       str.Mid(0, target_name.Len()))) {  // found it
            wxString tname = str.AfterFirst('-');
            name = tname.AfterFirst(' ');
            found_name = true;
            break;
          } else {
            str = text_file.GetNextLine();
          }
        }
      } else {
        wxString msg(_T("   Error Reading ENC .TXT file: "));
        msg.Append(file.GetFullPath());
        wxLogMessage(msg);
      }

      text_file.Close();

      if (found_name) break;
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

const char *s57chart::getName(OGRFeature *feature) {
  return feature->GetDefnRef()->GetName();
}

static int ExtensionCompare(const wxString &first, const wxString &second) {
  wxFileName fn1(first);
  wxFileName fn2(second);
  wxString ext1(fn1.GetExt());
  wxString ext2(fn2.GetExt());

  return ext1.Cmp(ext2);
}

int s57chart::GetUpdateFileArray(const wxFileName file000,
                                 wxArrayString *UpFiles, wxDateTime date000,
                                 wxString edtn000) {
  wxString DirName000 =
      file000.GetPath((int)(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));
  wxDir dir(DirName000);
  if (!dir.IsOpened()) {
    DirName000.Prepend(wxFileName::GetPathSeparator());
    DirName000.Prepend(_T("."));
    dir.Open(DirName000);
    if (!dir.IsOpened()) {
      return 0;
    }
  }

  int flags = wxDIR_DEFAULT;

  // Check dir structure
  //  We look to see if the directory one level above where the .000 file is
  //  located happens to be "perfectly numeric" in name. If so, the dataset is
  //  presumed to be organized with each update in its own directory. So, we
  //  search for updates from this level, recursing into subdirs.
  wxFileName fnDir(DirName000);
  fnDir.RemoveLastDir();
  wxString sdir = fnDir.GetPath();
  wxFileName fnTest(sdir);
  wxString sname = fnTest.GetName();
  long tmps;
  if (sname.ToLong(&tmps)) {
    dir.Open(sdir);
    DirName000 = sdir;
    flags |= wxDIR_DIRS;
  }

  wxString ext;
  wxArrayString *dummy_array;
  int retval = 0;

  if (UpFiles == NULL)
    dummy_array = new wxArrayString;
  else
    dummy_array = UpFiles;

  wxArrayString possibleFiles;
  wxDir::GetAllFiles(DirName000, &possibleFiles, wxEmptyString, flags);

  for (unsigned int i = 0; i < possibleFiles.GetCount(); i++) {
    wxString filename(possibleFiles[i]);

    wxFileName file(filename);
    ext = file.GetExt();

    long tmp;
    //  Files of interest have the same base name is the target .000 cell,
    //  and have numeric extension
    if (ext.ToLong(&tmp) && (file.GetName() == file000.GetName())) {
      wxString FileToAdd = filename;

      wxCharBuffer buffer =
          FileToAdd.ToUTF8();  // Check file namme for convertability

      if (buffer.data() && !filename.IsSameAs(_T("CATALOG.031"),
                                              false))  // don't process catalogs
      {
        //          We must check the update file for validity
        //          1.  Is update field DSID:EDTN  equal to base .000 file
        //          DSID:EDTN?
        //          2.  Is update file DSID.ISDT greater than or equal to base
        //          .000 file DSID:ISDT

        wxDateTime umdate;
        wxString sumdate;
        wxString umedtn;
        DDFModule *poModule = new DDFModule();
        if (!poModule->Open(FileToAdd.mb_str())) {
          wxString msg(
              _T("   s57chart::BuildS57File  Unable to open update file "));
          msg.Append(FileToAdd);
          wxLogMessage(msg);
        } else {
          poModule->Rewind();

          //    Read and parse DDFRecord 0 to get some interesting data
          //    n.b. assumes that the required fields will be in Record 0.... Is
          //    this always true?

          DDFRecord *pr = poModule->ReadRecord();  // Record 0
          //    pr->Dump(stdout);

          //  Fetch ISDT(Issue Date)
          char *u = NULL;
          if (pr) {
            u = (char *)(pr->GetStringSubfield("DSID", 0, "ISDT", 0));

            if (u) {
              if (strlen(u)) sumdate = wxString(u, wxConvUTF8);
            }
          } else {
            wxString msg(
                _T("   s57chart::BuildS57File  DDFRecord 0 does not contain ")
                _T("DSID:ISDT in update file "));
            msg.Append(FileToAdd);
            wxLogMessage(msg);

            sumdate = _T("20000101");  // backstop, very early, so wont be used
          }

          umdate.ParseFormat(sumdate, _T("%Y%m%d"));
          if (!umdate.IsValid())
            umdate.ParseFormat(_T("20000101"), _T("%Y%m%d"));

          umdate.ResetTime();
          if (!umdate.IsValid()) int yyp = 4;

          //    Fetch the EDTN(Edition) field
          if (pr) {
            u = NULL;
            u = (char *)(pr->GetStringSubfield("DSID", 0, "EDTN", 0));
            if (u) {
              if (strlen(u)) umedtn = wxString(u, wxConvUTF8);
            }
          } else {
            wxString msg(
                _T("   s57chart::BuildS57File  DDFRecord 0 does not contain ")
                _T("DSID:EDTN in update file "));
            msg.Append(FileToAdd);
            wxLogMessage(msg);

            umedtn = _T("1");  // backstop
          }
        }

        delete poModule;

        if ((!umdate.IsEarlierThan(date000)) &&
            (umedtn.IsSameAs(edtn000)))  // Note polarity on Date compare....
          dummy_array->Add(FileToAdd);   // Looking for umdate >= m_date000
      }
    }
  }

  //      Sort the candidates
  dummy_array->Sort(ExtensionCompare);

  //      Get the update number of the last in the list
  if (dummy_array->GetCount()) {
    wxString Last = dummy_array->Last();
    wxFileName fnl(Last);
    ext = fnl.GetExt();
    wxCharBuffer buffer = ext.ToUTF8();
    if (buffer.data()) retval = atoi(buffer.data());
  }

  if (UpFiles == NULL) delete dummy_array;

  return retval;
}

int s57chart::ValidateAndCountUpdates(const wxFileName file000,
                                      const wxString CopyDir,
                                      wxString &LastUpdateDate,
                                      bool b_copyfiles) {
  int retval = 0;

  //       wxString DirName000 = file000.GetPath((int)(wxPATH_GET_SEPARATOR |
  //       wxPATH_GET_VOLUME)); wxDir dir(DirName000);
  wxArrayString *UpFiles = new wxArrayString;
  retval = GetUpdateFileArray(file000, UpFiles, m_date000, m_edtn000);

  if (UpFiles->GetCount()) {
    //      The s57reader of ogr requires that update set be sequentially
    //      complete to perform all the updates.  However, some NOAA ENC
    //      distributions are not complete, as apparently some interim updates
    //      have been  withdrawn. Example:  as of 20 Dec, 2005, the update set
    //      for US5MD11M.000 includes US5MD11M.017, ...018, and ...019.  Updates
    //      001 through 016 are missing.
    //
    //      Workaround.
    //      Create temporary dummy update files to fill out the set before
    //      invoking ogr file open/ingest.  Delete after SENC file create
    //      finishes. Set starts with .000, which has the effect of copying the
    //      base file to the working dir

    bool chain_broken_mssage_shown = false;

    if (b_copyfiles) {
      m_tmpup_array =
          new wxArrayString;  // save a list of created files for later erase

      for (int iff = 0; iff < retval + 1; iff++) {
        wxFileName ufile(m_TempFilePath);
        wxString sext;
        sext.Printf(_T("%03d"), iff);
        ufile.SetExt(sext);

        //      Create the target update file name
        wxString cp_ufile = CopyDir;
        if (cp_ufile.Last() != ufile.GetPathSeparator())
          cp_ufile.Append(ufile.GetPathSeparator());

        cp_ufile.Append(ufile.GetFullName());

        //      Explicit check for a short update file, possibly left over from
        //      a crash...
        int flen = 0;
        if (ufile.FileExists()) {
          wxFile uf(ufile.GetFullPath());
          if (uf.IsOpened()) {
            flen = uf.Length();
            uf.Close();
          }
        }

        if (ufile.FileExists() &&
            (flen > 25))  // a valid update file or base file
        {
          //      Copy the valid file to the SENC directory
          bool cpok = wxCopyFile(ufile.GetFullPath(), cp_ufile);
          if (!cpok) {
            wxString msg(_T("   Cannot copy temporary working ENC file "));
            msg.Append(ufile.GetFullPath());
            msg.Append(_T(" to "));
            msg.Append(cp_ufile);
            wxLogMessage(msg);
          }
        }

        else {
          // Create a dummy ISO8211 file with no real content
          // Correct this.  We should break the walk, and notify the user  See
          // FS#1406

          if (!chain_broken_mssage_shown) {
            OCPNMessageBox(
                NULL,
                _("S57 Cell Update chain incomplete.\nENC features may be "
                  "incomplete or inaccurate.\nCheck the logfile for details."),
                _("OpenCPN Create SENC Warning"), wxOK | wxICON_EXCLAMATION,
                30);
            chain_broken_mssage_shown = true;
          }

          wxString msg(
              _T("WARNING---ENC Update chain incomplete. Substituting NULL ")
              _T("update file: "));
          msg += ufile.GetFullName();
          wxLogMessage(msg);
          wxLogMessage(_T("   Subsequent ENC updates may produce errors."));
          wxLogMessage(
              _T("   This ENC exchange set should be updated and SENCs ")
              _T("rebuilt."));

          bool bstat;
          DDFModule *dupdate = new DDFModule;
          dupdate->Initialize('3', 'L', 'E', '1', '0', "!!!", 3, 4, 4);
          bstat = !(dupdate->Create(cp_ufile.mb_str()) == 0);
          delete dupdate;

          if (!bstat) {
            wxString msg(_T("   Error creating dummy update file: "));
            msg.Append(cp_ufile);
            wxLogMessage(msg);
          }
        }

        m_tmpup_array->Add(cp_ufile);
      }
    }

    //      Extract the date field from the last of the update files
    //      which is by definition a valid, present update file....

    wxFileName lastfile(m_TempFilePath);
    wxString last_sext;
    last_sext.Printf(_T("%03d"), retval);
    lastfile.SetExt(last_sext);

    bool bSuccess;
    DDFModule oUpdateModule;

    //            bSuccess = !(oUpdateModule.Open(
    //            m_tmpup_array->Last().mb_str(), TRUE ) == 0);
    bSuccess =
        !(oUpdateModule.Open(lastfile.GetFullPath().mb_str(), TRUE) == 0);

    if (bSuccess) {
      //      Get publish/update date
      oUpdateModule.Rewind();
      DDFRecord *pr = oUpdateModule.ReadRecord();  // Record 0

      int nSuccess;
      char *u = NULL;

      if (pr)
        u = (char *)(pr->GetStringSubfield("DSID", 0, "ISDT", 0, &nSuccess));

      if (u) {
        if (strlen(u)) {
          LastUpdateDate = wxString(u, wxConvUTF8);
        }
      } else {
        wxDateTime now = wxDateTime::Now();
        LastUpdateDate = now.Format(_T("%Y%m%d"));
      }
    }
  }

  delete UpFiles;
  return retval;
}

wxString s57chart::GetISDT(void) {
  if (m_date000.IsValid())
    return m_date000.Format(_T("%Y%m%d"));
  else
    return _T("Unknown");
}

bool s57chart::GetBaseFileAttr(const wxString &file000) {
  if (!wxFileName::FileExists(file000)) return false;

  wxString FullPath000 = file000;
  DDFModule *poModule = new DDFModule();
  if (!poModule->Open(FullPath000.mb_str())) {
    wxString msg(_T("   s57chart::BuildS57File  Unable to open "));
    msg.Append(FullPath000);
    wxLogMessage(msg);
    delete poModule;
    return false;
  }

  poModule->Rewind();

  //    Read and parse DDFRecord 0 to get some interesting data
  //    n.b. assumes that the required fields will be in Record 0....  Is this
  //    always true?

  DDFRecord *pr = poModule->ReadRecord();  // Record 0
                                           //    pr->Dump(stdout);

  //    Fetch the Geo Feature Count, or something like it....
  m_nGeoRecords = pr->GetIntSubfield("DSSI", 0, "NOGR", 0);
  if (!m_nGeoRecords) {
    wxString msg(
        _T("   s57chart::BuildS57File  DDFRecord 0 does not contain ")
        _T("DSSI:NOGR "));
    wxLogMessage(msg);

    m_nGeoRecords = 1;  // backstop
  }

  //  Use ISDT(Issue Date) here, which is the same as UADT(Updates Applied) for
  //  .000 files
  wxString date000;
  char *u = (char *)(pr->GetStringSubfield("DSID", 0, "ISDT", 0));
  if (u)
    date000 = wxString(u, wxConvUTF8);
  else {
    wxString msg(
        _T("   s57chart::BuildS57File  DDFRecord 0 does not contain ")
        _T("DSID:ISDT "));
    wxLogMessage(msg);

    date000 =
        _T("20000101");  // backstop, very early, so any new files will update?
  }
  m_date000.ParseFormat(date000, _T("%Y%m%d"));
  if (!m_date000.IsValid()) m_date000.ParseFormat(_T("20000101"), _T("%Y%m%d"));

  m_date000.ResetTime();

  //    Fetch the EDTN(Edition) field
  u = (char *)(pr->GetStringSubfield("DSID", 0, "EDTN", 0));
  if (u)
    m_edtn000 = wxString(u, wxConvUTF8);
  else {
    wxString msg(
        _T("   s57chart::BuildS57File  DDFRecord 0 does not contain ")
        _T("DSID:EDTN "));
    wxLogMessage(msg);

    m_edtn000 = _T("1");  // backstop
  }

  m_SE = m_edtn000;

  //      Fetch the Native Scale by reading more records until DSPM is found
  m_native_scale = 0;
  for (; pr != NULL; pr = poModule->ReadRecord()) {
    if (pr->FindField("DSPM") != NULL) {
      m_native_scale = pr->GetIntSubfield("DSPM", 0, "CSCL", 0);
      break;
    }
  }
  if (!m_native_scale) {
    wxString msg(_T("   s57chart::BuildS57File  ENC not contain DSPM:CSCL "));
    wxLogMessage(msg);

    m_native_scale = 1000;  // backstop
  }

  delete poModule;

  return true;
}

int s57chart::BuildSENCFile(const wxString &FullPath000,
                            const wxString &SENCFileName, bool b_progress) {
  //  LOD calculation
  double display_pix_per_meter = g_Platform->GetDisplayDPmm() * 1000;
  double meters_per_pixel_max_scale =
      GetNormalScaleMin(0, g_b_overzoom_x) / display_pix_per_meter;
  m_LOD_meters = meters_per_pixel_max_scale * g_SENC_LOD_pixels;

  //  Establish a common reference point for the chart
  ref_lat = (m_FullExtent.NLAT + m_FullExtent.SLAT) / 2.;
  ref_lon = (m_FullExtent.WLON + m_FullExtent.ELON) / 2.;

  if (!m_disableBackgroundSENC) {
    if (g_SencThreadManager) {
      SENCJobTicket *ticket = new SENCJobTicket();
      ticket->m_LOD_meters = m_LOD_meters;
      ticket->ref_lat = ref_lat;
      ticket->ref_lon = ref_lon;
      ticket->m_FullPath000 = FullPath000;
      ticket->m_SENCFileName = SENCFileName;
      ticket->m_chart = this;

      g_SencThreadManager->ScheduleJob(ticket);
      bReadyToRender = true;
      return BUILD_SENC_PENDING;

    } else
      return BUILD_SENC_NOK_RETRY;

  } else {
    Osenc senc;

    senc.setRegistrar(g_poRegistrar);
    senc.setRefLocn(ref_lat, ref_lon);
    senc.SetLODMeters(m_LOD_meters);

    AbstractPlatform::ShowBusySpinner();

    int ret = senc.createSenc200(FullPath000, SENCFileName, b_progress);

    AbstractPlatform::HideBusySpinner();

    if (ret == ERROR_INGESTING000)
      return BUILD_SENC_NOK_PERMANENT;
    else
      return ret;
  }
}

int s57chart::BuildRAZFromSENCFile(const wxString &FullPath) {
  int ret_val = 0;  // default is OK

  Osenc sencfile;

  // Set up the containers for ingestion results.
  // These will be populated by Osenc, and owned by the caller (this).
  S57ObjVector Objects;
  VE_ElementVector VEs;
  VC_ElementVector VCs;

  sencfile.setRefLocn(ref_lat, ref_lon);

  int srv = sencfile.ingest200(FullPath, &Objects, &VEs, &VCs);

  if (srv != SENC_NO_ERROR) {
    wxLogMessage(sencfile.getLastError());
    // TODO  Clean up here, or massive leaks result
    return 1;
  }

  //  Get the cell Ref point as recorded in the SENC
  Extent ext = sencfile.getReadExtent();

  m_FullExtent.ELON = ext.ELON;
  m_FullExtent.WLON = ext.WLON;
  m_FullExtent.NLAT = ext.NLAT;
  m_FullExtent.SLAT = ext.SLAT;
  m_bExtentSet = true;

  ref_lat = (ext.NLAT + ext.SLAT) / 2.;
  ref_lon = (ext.ELON + ext.WLON) / 2.;

  //  Process the Edge feature arrays.

  //    Create a hash map of VE_Element pointers as a chart class member
  int n_ve_elements = VEs.size();

  double scale = gFrame->GetBestVPScale(this);
  int nativescale = GetNativeScale();

  for (int i = 0; i < n_ve_elements; i++) {
    VE_Element *vep = VEs.at(i);
    if (vep && vep->nCount) {
      //  Get a bounding box for the edge
      double east_max = -1e7;
      double east_min = 1e7;
      double north_max = -1e7;
      double north_min = 1e7;

      float *vrun = vep->pPoints;
      for (size_t i = 0; i < vep->nCount; i++) {
        east_max = wxMax(east_max, *vrun);
        east_min = wxMin(east_min, *vrun);
        vrun++;

        north_max = wxMax(north_max, *vrun);
        north_min = wxMin(north_min, *vrun);
        vrun++;
      }

      double lat1, lon1, lat2, lon2;
      fromSM(east_min, north_min, ref_lat, ref_lon, &lat1, &lon1);
      fromSM(east_max, north_max, ref_lat, ref_lon, &lat2, &lon2);
      vep->edgeBBox.Set(lat1, lon1, lat2, lon2);
    }

    m_ve_hash[vep->index] = vep;
  }

  //    Create a hash map VC_Element pointers as a chart class member
  int n_vc_elements = VCs.size();

  for (int i = 0; i < n_vc_elements; i++) {
    VC_Element *vcp = VCs.at(i);
    m_vc_hash[vcp->index] = vcp;
  }

  VEs.clear();  // destroy contents, no longer needed
  VCs.clear();

  // Walk the vector of S57Objs, associating LUPS, instructions, etc...

  for (unsigned int i = 0; i < Objects.size(); i++) {
    S57Obj *obj = Objects[i];

    //      This is where Simplified or Paper-Type point features are selected
    LUPrec *LUP;
    LUPname LUP_Name = PAPER_CHART;

    const wxString objnam = obj->GetAttrValueAsString("OBJNAM");
    if (objnam.Len() > 0) {
      const wxString fe_name = wxString(obj->FeatureName, wxConvUTF8);
      SendVectorChartObjectInfo(FullPath, fe_name, objnam, obj->m_lat,
                                obj->m_lon, scale, nativescale);
    }
    // If there is a localized object name and it actually is different from the
    // object name, send it as well...
    const wxString nobjnam = obj->GetAttrValueAsString("NOBJNM");
    if (nobjnam.Len() > 0 && nobjnam != objnam) {
      const wxString fe_name = wxString(obj->FeatureName, wxConvUTF8);
      SendVectorChartObjectInfo(FullPath, fe_name, nobjnam, obj->m_lat,
                                obj->m_lon, scale, nativescale);
    }

    switch (obj->Primitive_type) {
      case GEO_POINT:
      case GEO_META:
      case GEO_PRIM:

        if (PAPER_CHART == ps52plib->m_nSymbolStyle)
          LUP_Name = PAPER_CHART;
        else
          LUP_Name = SIMPLIFIED;

        break;

      case GEO_LINE:
        LUP_Name = LINES;
        break;

      case GEO_AREA:
        if (PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle)
          LUP_Name = PLAIN_BOUNDARIES;
        else
          LUP_Name = SYMBOLIZED_BOUNDARIES;

        break;
    }

    LUP = ps52plib->S52_LUPLookup(LUP_Name, obj->FeatureName, obj);

    if (NULL == LUP) {
      if (g_bDebugS57) {
        wxString msg(obj->FeatureName, wxConvUTF8);
        msg.Prepend(_T("   Could not find LUP for "));
        LogMessageOnce(msg);
      }
      delete obj;
      obj = NULL;
      Objects[i] = NULL;
    } else {
      //              Convert LUP to rules set
      ps52plib->_LUP2rules(LUP, obj);

      //              Add linked object/LUP to the working set
      _insertRules(obj, LUP, this);

      //              Establish Object's Display Category
      obj->m_DisplayCat = LUP->DISC;

      //              Establish objects base display priority
      obj->m_DPRI = LUP->DPRI - '0';

      //  Is this a category-movable object?
      if (!strncmp(obj->FeatureName, "OBSTRN", 6) ||
          !strncmp(obj->FeatureName, "WRECKS", 6) ||
          !strncmp(obj->FeatureName, "DEPCNT", 6) ||
          !strncmp(obj->FeatureName, "UWTROC", 6)) {
        obj->m_bcategory_mutable = true;
      } else {
        obj->m_bcategory_mutable = false;
      }
    }

    //      Build/Maintain the ATON floating/rigid arrays
    if (obj && (GEO_POINT == obj->Primitive_type)) {
      // set floating platform
      if ((!strncmp(obj->FeatureName, "LITFLT", 6)) ||
          (!strncmp(obj->FeatureName, "LITVES", 6)) ||
          (!strncasecmp(obj->FeatureName, "BOY", 3))) {
        pFloatingATONArray->Add(obj);
      }

      // set rigid platform
      if (!strncasecmp(obj->FeatureName, "BCN", 3)) {
        pRigidATONArray->Add(obj);
      }

      //    Mark the object as an ATON
      if ((!strncmp(obj->FeatureName, "LIT", 3)) ||
          (!strncmp(obj->FeatureName, "LIGHTS", 6)) ||
          (!strncasecmp(obj->FeatureName, "BCN", 3)) ||
          (!strncasecmp(obj->FeatureName, "BOY", 3))) {
        obj->bIsAton = true;
      }
    }

  }  // Objects iterator

  //   Decide on pub date to show

  wxDateTime d000;
  d000.ParseFormat(sencfile.getBaseDate(), _T("%Y%m%d"));
  if (!d000.IsValid()) d000.ParseFormat(_T("20000101"), _T("%Y%m%d"));

  wxDateTime updt;
  updt.ParseFormat(sencfile.getUpdateDate(), _T("%Y%m%d"));
  if (!updt.IsValid()) updt.ParseFormat(_T("20000101"), _T("%Y%m%d"));

  if (updt.IsLaterThan(d000))
    m_PubYear.Printf(_T("%4d"), updt.GetYear());
  else
    m_PubYear.Printf(_T("%4d"), d000.GetYear());

  //    Set some base class values
  wxDateTime upd = updt;
  if (!upd.IsValid()) upd.ParseFormat(_T("20000101"), _T("%Y%m%d"));

  upd.ResetTime();
  m_EdDate = upd;

  m_SE = sencfile.getSENCReadBaseEdition();

  wxString supdate;
  supdate.Printf(_T(" / %d"), sencfile.getSENCReadLastUpdate());
  m_SE += supdate;

  m_datum_str = _T("WGS84");

  m_SoundingsDatum = _T("MEAN LOWER LOW WATER");
  m_ID = sencfile.getReadID();
  m_Name = sencfile.getReadName();

  ObjRazRules *top;

  AssembleLineGeometry();

  return ret_val;
}

int s57chart::_insertRules(S57Obj *obj, LUPrec *LUP, s57chart *pOwner) {
  ObjRazRules *rzRules = NULL;
  int disPrioIdx = 0;
  int LUPtypeIdx = 0;

  if (LUP == NULL) {
    //      printf("SEQuencer:_insertRules(): ERROR no rules to insert!!\n");
    return 0;
  }

  // find display priority index       --talky version
  switch (LUP->DPRI) {
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
      printf("SEQuencer:_insertRules():ERROR no display priority!!!\n");
  }

  // find look up type index
  switch (LUP->TNAM) {
    case SIMPLIFIED:
      LUPtypeIdx = 0;
      break;  // points
    case PAPER_CHART:
      LUPtypeIdx = 1;
      break;  // points
    case LINES:
      LUPtypeIdx = 2;
      break;  // lines
    case PLAIN_BOUNDARIES:
      LUPtypeIdx = 3;
      break;  // areas
    case SYMBOLIZED_BOUNDARIES:
      LUPtypeIdx = 4;
      break;  // areas
    default:
      printf("SEQuencer:_insertRules():ERROR no look up type !!!\n");
  }

  // insert rules
  rzRules = (ObjRazRules *)malloc(sizeof(ObjRazRules));
  rzRules->obj = obj;
  obj->nRef++;  // Increment reference counter for delete check;
  rzRules->LUP = LUP;
  rzRules->child = NULL;
  rzRules->mps = NULL;

#if 0
    rzRules->next = razRules[disPrioIdx][LUPtypeIdx];
    razRules[disPrioIdx][LUPtypeIdx] = rzRules;
#else
  // Find the end of the list, and append the object
  // This is required to honor the "natural order" priority rules for objects of
  // same Display Priority
  ObjRazRules *rNext = NULL;
  ObjRazRules *rPrevious = NULL;
  if (razRules[disPrioIdx][LUPtypeIdx]) {
    rPrevious = razRules[disPrioIdx][LUPtypeIdx];
    rNext = rPrevious->next;
  }
  while (rNext) {
    rPrevious = rNext;
    rNext = rPrevious->next;
  }

  rzRules->next = NULL;
  if (rPrevious)
    rPrevious->next = rzRules;
  else
    razRules[disPrioIdx][LUPtypeIdx] = rzRules;

#endif

  return 1;
}

void s57chart::ResetPointBBoxes(const ViewPort &vp_last,
                                const ViewPort &vp_this) {
  ObjRazRules *top;
  ObjRazRules *nxx;

  if (vp_last.view_scale_ppm == 1.0)  // Skip the startup case
    return;

  double d = vp_last.view_scale_ppm / vp_this.view_scale_ppm;

  for (int i = 0; i < PRIO_NUM; ++i) {
    for (int j = 0; j < 2; ++j) {
      top = razRules[i][j];

      while (top != NULL) {
        if (!top->obj->geoPtMulti)  // do not reset multipoints
        {
          if (top->obj->BBObj.GetValid()) {  // scale bbobj
            double lat = top->obj->m_lat, lon = top->obj->m_lon;

            double lat1 = (lat - top->obj->BBObj.GetMinLat()) * d;
            double lat2 = (lat - top->obj->BBObj.GetMaxLat()) * d;

            double minlon = top->obj->BBObj.GetMinLon();
            double maxlon = top->obj->BBObj.GetMaxLon();

            double lon1 = (lon - minlon) * d;
            double lon2 = (lon - maxlon) * d;

            top->obj->BBObj.Set(lat - lat1, lon - lon1, lat - lat2, lon - lon2);

            // this method is very close, but errors accumulate
            top->obj->BBObj.Invalidate();
          }
        }

        nxx = top->next;
        top = nxx;
      }
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

void s57chart::UpdateLUPs(s57chart *pOwner) {
  ObjRazRules *top;
  ObjRazRules *nxx;
  LUPrec *LUP;
  for (int i = 0; i < PRIO_NUM; ++i) {
    //  SIMPLIFIED is set, PAPER_CHART is bare
    if ((razRules[i][0]) && (NULL == razRules[i][1])) {
      m_b2pointLUPS = true;
      top = razRules[i][0];

      while (top != NULL) {
        LUP = ps52plib->S52_LUPLookup(PAPER_CHART, top->obj->FeatureName,
                                      top->obj);
        if (LUP) {
          //  A POINT object can only appear in two places in the table,
          //  SIMPLIFIED or PAPER_CHART although it is allowed for the Display
          //  priority to be different for each
          if (top->obj->nRef < 2) {
            ps52plib->_LUP2rules(LUP, top->obj);
            _insertRules(top->obj, LUP, pOwner);
            top->obj->m_DisplayCat = LUP->DISC;
          }
        }

        nxx = top->next;
        top = nxx;
      }
    }

    //  PAPER_CHART is set, SIMPLIFIED is bare
    if ((razRules[i][1]) && (NULL == razRules[i][0])) {
      m_b2pointLUPS = true;
      top = razRules[i][1];

      while (top != NULL) {
        LUP = ps52plib->S52_LUPLookup(SIMPLIFIED, top->obj->FeatureName,
                                      top->obj);
        if (LUP) {
          if (top->obj->nRef < 2) {
            ps52plib->_LUP2rules(LUP, top->obj);
            _insertRules(top->obj, LUP, pOwner);
            top->obj->m_DisplayCat = LUP->DISC;
          }
        }

        nxx = top->next;
        top = nxx;
      }
    }

    //  PLAIN_BOUNDARIES is set, SYMBOLIZED_BOUNDARIES is bare
    if ((razRules[i][3]) && (NULL == razRules[i][4])) {
      m_b2lineLUPS = true;
      top = razRules[i][3];

      while (top != NULL) {
        LUP = ps52plib->S52_LUPLookup(SYMBOLIZED_BOUNDARIES,
                                      top->obj->FeatureName, top->obj);
        if (LUP) {
          ps52plib->_LUP2rules(LUP, top->obj);
          _insertRules(top->obj, LUP, pOwner);
          top->obj->m_DisplayCat = LUP->DISC;
        }

        nxx = top->next;
        top = nxx;
      }
    }

    //  SYMBOLIZED_BOUNDARIES is set, PLAIN_BOUNDARIES is bare
    if ((razRules[i][4]) && (NULL == razRules[i][3])) {
      m_b2lineLUPS = true;
      top = razRules[i][4];

      while (top != NULL) {
        LUP = ps52plib->S52_LUPLookup(PLAIN_BOUNDARIES, top->obj->FeatureName,
                                      top->obj);
        if (LUP) {
          ps52plib->_LUP2rules(LUP, top->obj);
          _insertRules(top->obj, LUP, pOwner);
          top->obj->m_DisplayCat = LUP->DISC;
        }

        nxx = top->next;
        top = nxx;
      }
    }

    //  Traverse this priority level again,
    //  clearing any object CS rules and flags,
    //  so that the next render operation will re-evaluate the CS

    for (int j = 0; j < LUPNAME_NUM; j++) {
      top = razRules[i][j];
      while (top != NULL) {
        top->obj->bCS_Added = 0;
        free_mps(top->mps);
        top->mps = 0;
        if (top->LUP) top->obj->m_DisplayCat = top->LUP->DISC;

        nxx = top->next;
        top = nxx;
      }
    }

    //  Traverse this priority level again,
    //  clearing any object CS rules and flags of any child list,
    //  so that the next render operation will re-evaluate the CS

    for (int j = 0; j < LUPNAME_NUM; j++) {
      top = razRules[i][j];
      while (top != NULL) {
        if (top->child) {
          ObjRazRules *ctop = top->child;
          while (NULL != ctop) {
            ctop->obj->bCS_Added = 0;
            free_mps(ctop->mps);
            ctop->mps = 0;

            if (ctop->LUP) ctop->obj->m_DisplayCat = ctop->LUP->DISC;
            ctop = ctop->next;
          }
        }
        nxx = top->next;
        top = nxx;
      }
    }
  }

  //    Clear the dynamically created Conditional Symbology LUP Array
  // This can not be done on a per-chart basis, since the plib services all
  // charts
  // TODO really should make the dynamic LUPs belong to the chart class that
  // created them
}

ListOfObjRazRules *s57chart::GetLightsObjRuleListVisibleAtLatLon(
    float lat, float lon, ViewPort *VPoint) {
  ListOfObjRazRules *ret_ptr = new ListOfObjRazRules;
  std::vector<ObjRazRules *> selected_rules;

  //    Iterate thru the razRules array, by object/rule type

  ObjRazRules *top;
  char *curr_att = NULL;
  int n_attr = 0;
  wxArrayOfS57attVal *attValArray = NULL;
  bool bleading_attribute = false;

  for (int i = 0; i < PRIO_NUM; ++i) {
    {
      // Points by type, array indices [0..1]

      int point_type = (ps52plib->m_nSymbolStyle == SIMPLIFIED) ? 0 : 1;
      top = razRules[i][point_type];

      while (top != NULL) {
        if (top->obj->npt == 1) {
          if (!strncmp(top->obj->FeatureName, "LIGHTS", 6)) {
            double sectrTest;
            bool hasSectors = GetDoubleAttr(top->obj, "SECTR1", sectrTest);
            if (hasSectors) {
              if (ps52plib->ObjectRenderCheckCat(top)) {
                int attrCounter;
                double valnmr = -1;
                wxString curAttrName;
                curr_att = top->obj->att_array;
                n_attr = top->obj->n_attr;
                attValArray = top->obj->attVal;

                if (curr_att) {
                  bool bviz = true;

                  attrCounter = 0;
                  int noAttr = 0;

                  bleading_attribute = false;

                  while (attrCounter < n_attr) {
                    curAttrName = wxString(curr_att, wxConvUTF8, 6);
                    noAttr++;

                    S57attVal *pAttrVal = NULL;
                    if (attValArray) {
                      // if(Chs57)
                      pAttrVal = attValArray->Item(attrCounter);
                      // else if( target_plugin_chart )
                      // pAttrVal = attValArray->Item(attrCounter);
                    }
                    wxString value = s57chart::GetAttributeValueAsString(
                        pAttrVal, curAttrName);

                    if (curAttrName == _T("LITVIS")) {
                      if (value.StartsWith(_T("obsc"))) bviz = false;
                    } else if (curAttrName == _T("VALNMR"))
                      value.ToDouble(&valnmr);

                    attrCounter++;
                    curr_att += 6;
                  }

                  if (bviz && (valnmr > 0.1)) {
                    // As a quick check, compare the mercator-manhattan distance
                    double olon, olat;
                    fromSM(
                        (top->obj->x * top->obj->x_rate) + top->obj->x_origin,
                        (top->obj->y * top->obj->y_rate) + top->obj->y_origin,
                        ref_lat, ref_lon, &olat, &olon);

                    double dlat = lat - olat;
                    double dy = dlat * 60 / cos(olat * PI / 180.);
                    double dlon = lon - olon;
                    double dx = dlon * 60;
                    double manhat = abs(dy) + abs(dx);
                    if (1 /*(abs(dy) + abs(dx)) < valnmr*/) {
                      // close...Check precisely
                      double br, dd;
                      DistanceBearingMercator(lat, lon, olat, olon, &br, &dd);
                      if (dd < valnmr) {
                        selected_rules.push_back(top);
                      }
                    }
                  }
                }
              }
            }
          }
        }

        top = top->next;
      }
    }
  }

  // Copy the rules in order into a wxList so the function returns the correct
  // type
  for (std::size_t i = 0; i < selected_rules.size(); ++i) {
    ret_ptr->Append(selected_rules[i]);
  }

  return ret_ptr;
}

ListOfObjRazRules *s57chart::GetObjRuleListAtLatLon(float lat, float lon,
                                                    float select_radius,
                                                    ViewPort *VPoint,
                                                    int selection_mask) {
  ListOfObjRazRules *ret_ptr = new ListOfObjRazRules;
  std::vector<ObjRazRules *> selected_rules;

  PrepareForRender(VPoint, ps52plib);

  //    Iterate thru the razRules array, by object/rule type

  ObjRazRules *top;

  for (int i = 0; i < PRIO_NUM; ++i) {
    if (selection_mask & MASK_POINT) {
      // Points by type, array indices [0..1]

      int point_type = (ps52plib->m_nSymbolStyle == SIMPLIFIED) ? 0 : 1;
      top = razRules[i][point_type];

      while (top != NULL) {
        if (top->obj->npt ==
            1)  // Do not select Multipoint objects (SOUNDG) yet.
        {
          if (ps52plib->ObjectRenderCheck(top)) {
            if (DoesLatLonSelectObject(lat, lon, select_radius, top->obj))
              selected_rules.push_back(top);
          }
        }

        //    Check the child branch, if any.
        //    This is where Multipoint soundings are captured individually
        if (top->child) {
          ObjRazRules *child_item = top->child;
          while (child_item != NULL) {
            if (ps52plib->ObjectRenderCheck(child_item)) {
              if (DoesLatLonSelectObject(lat, lon, select_radius,
                                         child_item->obj))
                selected_rules.push_back(child_item);
            }

            child_item = child_item->next;
          }
        }

        top = top->next;
      }
    }

    if (selection_mask & MASK_AREA) {
      // Areas by boundary type, array indices [3..4]

      int area_boundary_type =
          (ps52plib->m_nBoundaryStyle == PLAIN_BOUNDARIES) ? 3 : 4;
      top = razRules[i][area_boundary_type];  // Area nnn Boundaries
      while (top != NULL) {
        if (ps52plib->ObjectRenderCheck(top)) {
          if (DoesLatLonSelectObject(lat, lon, select_radius, top->obj))
            selected_rules.push_back(top);
        }

        top = top->next;
      }  // while
    }

    if (selection_mask & MASK_LINE) {
      // Finally, lines
      top = razRules[i][2];  // Lines

      while (top != NULL) {
        if (ps52plib->ObjectRenderCheck(top)) {
          if (DoesLatLonSelectObject(lat, lon, select_radius, top->obj))
            selected_rules.push_back(top);
        }

        top = top->next;
      }
    }
  }

  // Sort Point objects by distance to searched lat/lon
  // This lambda function could be modified to also sort GEO_LINES and GEO_AREAS
  // if needed
  auto sortObjs = [lat, lon, this](const ObjRazRules *obj1,
                                   const ObjRazRules *obj2) -> bool {
    double br1, dd1, br2, dd2;

    if (obj1->obj->Primitive_type == GEO_POINT &&
        obj2->obj->Primitive_type == GEO_POINT) {
      double lat1, lat2, lon1, lon2;
      fromSM((obj1->obj->x * obj1->obj->x_rate) + obj1->obj->x_origin,
             (obj1->obj->y * obj1->obj->y_rate) + obj1->obj->y_origin, ref_lat,
             ref_lon, &lat1, &lon1);

      if (lon1 > 180.0) lon1 -= 360.;

      fromSM((obj2->obj->x * obj2->obj->x_rate) + obj2->obj->x_origin,
             (obj2->obj->y * obj2->obj->y_rate) + obj2->obj->y_origin, ref_lat,
             ref_lon, &lat2, &lon2);

      if (lon2 > 180.0) lon2 -= 360.;

      DistanceBearingMercator(lat, lon, lat1, lon1, &br1, &dd1);
      DistanceBearingMercator(lat, lon, lat2, lon2, &br2, &dd2);
      return dd1 > dd2;
    }
    return false;
  };

  // Sort the selected rules by using the lambda sort function defined above
  std::sort(selected_rules.begin(), selected_rules.end(), sortObjs);

  // Copy the rules in order into a wxList so the function returns the correct
  // type
  for (std::size_t i = 0; i < selected_rules.size(); ++i) {
    ret_ptr->Append(selected_rules[i]);
  }

  return ret_ptr;
}

bool s57chart::DoesLatLonSelectObject(float lat, float lon, float select_radius,
                                      S57Obj *obj) {
  switch (obj->Primitive_type) {
    //  For single Point objects, the integral object bounding box contains the
    //  lat/lon of the object, possibly expanded by text or symbol rendering
    case GEO_POINT: {
      if (!obj->BBObj.GetValid()) return false;

      if (1 == obj->npt) {
        //  Special case for LIGHTS
        //  Sector lights have had their BBObj expanded to include the entire
        //  drawn sector This is too big for pick area, can be confusing.... So
        //  make a temporary box at the light's lat/lon, with select_radius size
        if (!strncmp(obj->FeatureName, "LIGHTS", 6)) {
          double sectrTest;
          bool hasSectors = GetDoubleAttr(obj, "SECTR1", sectrTest);
          if (hasSectors) {
            double olon, olat;
            fromSM((obj->x * obj->x_rate) + obj->x_origin,
                   (obj->y * obj->y_rate) + obj->y_origin, ref_lat, ref_lon,
                   &olat, &olon);

            // Double the select radius to adjust for the fact that LIGHTS has
            // a 0x0 BBox to start with, which makes it smaller than all other
            // rendered objects.
            LLBBox sbox;
            sbox.Set(olat, olon, olat, olon);

            if (sbox.ContainsMarge(lat, lon, select_radius)) return true;
          } else if (obj->BBObj.ContainsMarge(lat, lon, select_radius))
            return true;

        }

        else if (obj->BBObj.ContainsMarge(lat, lon, select_radius))
          return true;
      }

      //  For MultiPoint objects, make a bounding box from each point's lat/lon
      //  and check it
      else {
        if (!obj->BBObj.GetValid()) return false;

        //  Coarse test first
        if (!obj->BBObj.ContainsMarge(lat, lon, select_radius)) return false;
        //  Now decomposed soundings, one by one
        double *pdl = obj->geoPtMulti;
        for (int ip = 0; ip < obj->npt; ip++) {
          double lon_point = *pdl++;
          double lat_point = *pdl++;
          LLBBox BB_point;
          BB_point.Set(lat_point, lon_point, lat_point, lon_point);
          if (BB_point.ContainsMarge(lat, lon, select_radius)) {
            //                                  index = ip;
            return true;
          }
        }
      }

      break;
    }
    case GEO_AREA: {
      //  Coarse test first
      if (!obj->BBObj.ContainsMarge(lat, lon, select_radius))
        return false;
      else
        return IsPointInObjArea(lat, lon, select_radius, obj);
    }

    case GEO_LINE: {
      //  Coarse test first
      if (!obj->BBObj.ContainsMarge(lat, lon, select_radius)) return false;

      float sel_rad_meters = select_radius * 1852 * 60;  // approximately
      double easting, northing;
      toSM(lat, lon, ref_lat, ref_lon, &easting, &northing);

      if (obj->geoPt) {
        //  Line geometry is carried in SM or CM93 coordinates, so...
        //  make the hit test using SM coordinates, converting from object
        //  points to SM using per-object conversion factors.

        pt *ppt = obj->geoPt;
        int npt = obj->npt;

        double xr = obj->x_rate;
        double xo = obj->x_origin;
        double yr = obj->y_rate;
        double yo = obj->y_origin;

        double north0 = (ppt->y * yr) + yo;
        double east0 = (ppt->x * xr) + xo;
        ppt++;

        for (int ip = 1; ip < npt; ip++) {
          double north = (ppt->y * yr) + yo;
          double east = (ppt->x * xr) + xo;

          //    A slightly less coarse segment bounding box check
          if (northing >= (fmin(north, north0) - sel_rad_meters))
            if (northing <= (fmax(north, north0) + sel_rad_meters))
              if (easting >= (fmin(east, east0) - sel_rad_meters))
                if (easting <= (fmax(east, east0) + sel_rad_meters)) {
                  return true;
                }

          north0 = north;
          east0 = east;
          ppt++;
        }
      } else {  // in oSENC V2, Array of points is stored in prearranged VBO
                // array.
        if (obj->m_ls_list) {
          float *ppt;
          unsigned char *vbo_point =
              (unsigned char *)obj->m_chart_context
                  ->vertex_buffer;  // chart->GetLineVertexBuffer();
          line_segment_element *ls = obj->m_ls_list;

          while (ls && vbo_point) {
            int nPoints;
            if ((ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV)) {
              ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
              nPoints = ls->pedge->nCount;
            } else {
              ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
              nPoints = 2;
            }

            float north0 = ppt[1];
            float east0 = ppt[0];

            ppt += 2;

            for (int ip = 0; ip < nPoints - 1; ip++) {
              float north = ppt[1];
              float east = ppt[0];

              if (northing >= (fmin(north, north0) - sel_rad_meters))
                if (northing <= (fmax(north, north0) + sel_rad_meters))
                  if (easting >= (fmin(east, east0) - sel_rad_meters))
                    if (easting <= (fmax(east, east0) + sel_rad_meters)) {
                      return true;
                    }

              north0 = north;
              east0 = east;

              ppt += 2;
            }

            ls = ls->next;
          }
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

wxString s57chart::GetAttributeDecode(wxString &att, int ival) {
  wxString ret_val = _T("");

  //  Get the attribute code from the acronym
  const char *att_code;

  wxString file(g_csv_locn);
  file.Append(_T("/s57attributes.csv"));

  if (!wxFileName::FileExists(file)) {
    wxString msg(_T("   Could not open "));
    msg.Append(file);
    wxLogMessage(msg);

    return ret_val;
  }

  att_code = MyCSVGetField(file.mb_str(), "Acronym",  // match field
                           att.mb_str(),              // match value
                           CC_ExactString, "Code");   // return field

  // Now, get a nice description from s57expectedinput.csv
  //  This will have to be a 2-d search, using ID field and Code field

  // Ingest, and get a pointer to the ingested table for "Expected Input" file
  wxString ei_file(g_csv_locn);
  ei_file.Append(_T("/s57expectedinput.csv"));

  if (!wxFileName::FileExists(ei_file)) {
    wxString msg(_T("   Could not open "));
    msg.Append(ei_file);
    wxLogMessage(msg);

    return ret_val;
  }

  CSVTable *psTable = CSVAccess(ei_file.mb_str());
  CSVIngest(ei_file.mb_str());

  char **papszFields = NULL;
  int bSelected = FALSE;

  /* -------------------------------------------------------------------- */
  /*      Scan from in-core lines.                                        */
  /* -------------------------------------------------------------------- */
  int iline = 0;
  while (!bSelected && iline + 1 < psTable->nLineCount) {
    iline++;
    papszFields = CSVSplitLine(psTable->papszLines[iline]);

    if (!strcmp(papszFields[0], att_code)) {
      if (atoi(papszFields[1]) == ival) {
        ret_val = wxString(papszFields[2], wxConvUTF8);
        bSelected = TRUE;
      }
    }

    CSLDestroy(papszFields);
  }

  return ret_val;
}

//----------------------------------------------------------------------------------

bool s57chart::IsPointInObjArea(float lat, float lon, float select_radius,
                                S57Obj *obj) {
  bool ret = false;

  if (obj->pPolyTessGeo) {
    if (!obj->pPolyTessGeo->IsOk()) obj->pPolyTessGeo->BuildDeferredTess();

    PolyTriGroup *ppg = obj->pPolyTessGeo->Get_PolyTriGroup_head();

    TriPrim *pTP = ppg->tri_prim_head;

    MyPoint pvert_list[3];

    //  Polygon geometry is carried in SM coordinates, so...
    //  make the hit test thus.
    double easting, northing;
    toSM(lat, lon, ref_lat, ref_lon, &easting, &northing);

    //  On some chart types (e.g. cm93), the tesseleated coordinates are stored
    //  differently. Adjust the pick point (easting/northing) to correspond.
    if (!ppg->m_bSMSENC) {
      double y_rate = obj->y_rate;
      double y_origin = obj->y_origin;
      double x_rate = obj->x_rate;
      double x_origin = obj->x_origin;

      double northing_scaled = (northing - y_origin) / y_rate;
      double easting_scaled = (easting - x_origin) / x_rate;
      northing = northing_scaled;
      easting = easting_scaled;
    }

    while (pTP) {
      //  Coarse test
      if (pTP->tri_box.Contains(lat, lon)) {
        if (ppg->data_type == DATA_TYPE_DOUBLE) {
          double *p_vertex = pTP->p_vertex;

          switch (pTP->type) {
            case PTG_TRIANGLE_FAN: {
              for (int it = 0; it < pTP->nVert - 2; it++) {
                pvert_list[0].x = p_vertex[0];
                pvert_list[0].y = p_vertex[1];

                pvert_list[1].x = p_vertex[(it * 2) + 2];
                pvert_list[1].y = p_vertex[(it * 2) + 3];

                pvert_list[2].x = p_vertex[(it * 2) + 4];
                pvert_list[2].y = p_vertex[(it * 2) + 5];

                if (G_PtInPolygon((MyPoint *)pvert_list, 3, easting,
                                  northing)) {
                  ret = true;
                  break;
                }
              }
              break;
            }
            case PTG_TRIANGLE_STRIP: {
              for (int it = 0; it < pTP->nVert - 2; it++) {
                pvert_list[0].x = p_vertex[(it * 2)];
                pvert_list[0].y = p_vertex[(it * 2) + 1];

                pvert_list[1].x = p_vertex[(it * 2) + 2];
                pvert_list[1].y = p_vertex[(it * 2) + 3];

                pvert_list[2].x = p_vertex[(it * 2) + 4];
                pvert_list[2].y = p_vertex[(it * 2) + 5];

                if (G_PtInPolygon((MyPoint *)pvert_list, 3, easting,
                                  northing)) {
                  ret = true;
                  break;
                }
              }
              break;
            }
            case PTG_TRIANGLES: {
              for (int it = 0; it < pTP->nVert; it += 3) {
                pvert_list[0].x = p_vertex[(it * 2)];
                pvert_list[0].y = p_vertex[(it * 2) + 1];

                pvert_list[1].x = p_vertex[(it * 2) + 2];
                pvert_list[1].y = p_vertex[(it * 2) + 3];

                pvert_list[2].x = p_vertex[(it * 2) + 4];
                pvert_list[2].y = p_vertex[(it * 2) + 5];

                if (G_PtInPolygon((MyPoint *)pvert_list, 3, easting,
                                  northing)) {
                  ret = true;
                  break;
                }
              }
              break;
            }
          }
        } else if (ppg->data_type == DATA_TYPE_FLOAT) {
          float *p_vertex = (float *)pTP->p_vertex;

          switch (pTP->type) {
            case PTG_TRIANGLE_FAN: {
              for (int it = 0; it < pTP->nVert - 2; it++) {
                pvert_list[0].x = p_vertex[0];
                pvert_list[0].y = p_vertex[1];

                pvert_list[1].x = p_vertex[(it * 2) + 2];
                pvert_list[1].y = p_vertex[(it * 2) + 3];

                pvert_list[2].x = p_vertex[(it * 2) + 4];
                pvert_list[2].y = p_vertex[(it * 2) + 5];

                if (G_PtInPolygon((MyPoint *)pvert_list, 3, easting,
                                  northing)) {
                  ret = true;
                  break;
                }
              }
              break;
            }
            case PTG_TRIANGLE_STRIP: {
              for (int it = 0; it < pTP->nVert - 2; it++) {
                pvert_list[0].x = p_vertex[(it * 2)];
                pvert_list[0].y = p_vertex[(it * 2) + 1];

                pvert_list[1].x = p_vertex[(it * 2) + 2];
                pvert_list[1].y = p_vertex[(it * 2) + 3];

                pvert_list[2].x = p_vertex[(it * 2) + 4];
                pvert_list[2].y = p_vertex[(it * 2) + 5];

                if (G_PtInPolygon((MyPoint *)pvert_list, 3, easting,
                                  northing)) {
                  ret = true;
                  break;
                }
              }
              break;
            }
            case PTG_TRIANGLES: {
              for (int it = 0; it < pTP->nVert; it += 3) {
                pvert_list[0].x = p_vertex[(it * 2)];
                pvert_list[0].y = p_vertex[(it * 2) + 1];

                pvert_list[1].x = p_vertex[(it * 2) + 2];
                pvert_list[1].y = p_vertex[(it * 2) + 3];

                pvert_list[2].x = p_vertex[(it * 2) + 4];
                pvert_list[2].y = p_vertex[(it * 2) + 5];

                if (G_PtInPolygon((MyPoint *)pvert_list, 3, easting,
                                  northing)) {
                  ret = true;
                  break;
                }
              }
              break;
            }
          }
        } else {
          ret = true;  // Unknown data type, accept the entire TriPrim via
                       // coarse test.
          break;
        }
      }
      pTP = pTP->p_next;
    }

  }  // if pPolyTessGeo

  return ret;
}

wxString s57chart::GetObjectAttributeValueAsString(S57Obj *obj, int iatt,
                                                   wxString curAttrName) {
  wxString value;
  S57attVal *pval;

  pval = obj->attVal->Item(iatt);
  switch (pval->valType) {
    case OGR_STR: {
      if (pval->value) {
        wxString val_str((char *)(pval->value), wxConvUTF8);
        long ival;
        if (val_str.ToLong(&ival)) {
          if (0 == ival)
            value = _T("Unknown");
          else {
            wxString decode_val = GetAttributeDecode(curAttrName, ival);
            if (!decode_val.IsEmpty()) {
              value = decode_val;
              wxString iv;
              iv.Printf(_T(" (%d)"), (int)ival);
              value.Append(iv);
            } else
              value.Printf(_T("%d"), (int)ival);
          }
        }

        else if (val_str.IsEmpty())
          value = _T("Unknown");

        else {
          value.Clear();
          wxString value_increment;
          wxStringTokenizer tk(val_str, wxT(","));
          int iv = 0;
          if (tk.HasMoreTokens()) {
            while (tk.HasMoreTokens()) {
              wxString token = tk.GetNextToken();
              long ival;
              if (token.ToLong(&ival)) {
                wxString decode_val = GetAttributeDecode(curAttrName, ival);

                value_increment.Printf(_T(" (%d)"), (int)ival);

                if (!decode_val.IsEmpty()) value_increment.Prepend(decode_val);

                if (iv) value_increment.Prepend(wxT(", "));
                value.Append(value_increment);

              } else {
                if (iv) value.Append(_T(","));
                value.Append(token);
              }

              iv++;
            }
          } else
            value.Append(val_str);
        }
      } else
        value = _T("[NULL VALUE]");

      break;
    }

    case OGR_INT: {
      int ival = *((int *)pval->value);
      wxString decode_val = GetAttributeDecode(curAttrName, ival);

      if (!decode_val.IsEmpty()) {
        value = decode_val;
        wxString iv;
        iv.Printf(_T("(%d)"), ival);
        value.Append(iv);
      } else
        value.Printf(_T("(%d)"), ival);

      break;
    }
    case OGR_INT_LST:
      break;

    case OGR_REAL: {
      double dval = *((double *)pval->value);
      wxString val_suffix = _T(" m");

      //    As a special case, convert some attribute values to feet.....
      if ((curAttrName == _T("VERCLR")) || (curAttrName == _T("VERCCL")) ||
          (curAttrName == _T("VERCOP")) || (curAttrName == _T("HEIGHT")) ||
          (curAttrName == _T("HORCLR")) || (curAttrName == _T("ELEVAT"))) {
        switch (ps52plib->m_nDepthUnitDisplay) {
          case 0:                          // feet
          case 2:                          // fathoms
            dval = dval * 3 * 39.37 / 36;  // feet
            val_suffix = _T(" ft");
            break;
          default:
            break;
        }
      }

      else if ((curAttrName == _T("VALSOU")) || (curAttrName == _T("DRVAL1")) ||
               (curAttrName == _T("DRVAL2")) || (curAttrName == _T("VALDCO"))) {
        switch (ps52plib->m_nDepthUnitDisplay) {
          case 0:                          // feet
            dval = dval * 3 * 39.37 / 36;  // feet
            val_suffix = _T(" ft");
            break;
          case 2:                          // fathoms
            dval = dval * 3 * 39.37 / 36;  // fathoms
            dval /= 6.0;
            val_suffix = _T(" fathoms");
            break;
          default:
            break;
        }
      }

      else if (curAttrName == _T("SECTR1"))
        val_suffix = _T("&deg;");
      else if (curAttrName == _T("SECTR2"))
        val_suffix = _T("&deg;");
      else if (curAttrName == _T("ORIENT"))
        val_suffix = _T("&deg;");
      else if (curAttrName == _T("VALNMR"))
        val_suffix = _T(" Nm");
      else if (curAttrName == _T("SIGPER"))
        val_suffix = _T("s");
      else if (curAttrName == _T("VALACM"))
        val_suffix = _T(" Minutes/year");
      else if (curAttrName == _T("VALMAG"))
        val_suffix = _T("&deg;");
      else if (curAttrName == _T("CURVEL"))
        val_suffix = _T(" kt");

      if (dval - floor(dval) < 0.01)
        value.Printf(_T("%2.0f"), dval);
      else
        value.Printf(_T("%4.1f"), dval);

      value << val_suffix;

      break;
    }

    case OGR_REAL_LST: {
      break;
    }
  }
  return value;
}

wxString s57chart::GetAttributeValueAsString(S57attVal *pAttrVal,
                                             wxString AttrName) {
  if (NULL == pAttrVal) return _T("");

  wxString value;
  switch (pAttrVal->valType) {
    case OGR_STR: {
      if (pAttrVal->value) {
        wxString val_str((char *)(pAttrVal->value), wxConvUTF8);
        long ival;
        if (val_str.ToLong(&ival)) {
          if (0 == ival)
            value = _T("Unknown");
          else {
            wxString decode_val = GetAttributeDecode(AttrName, ival);
            if (!decode_val.IsEmpty()) {
              value = decode_val;
              wxString iv;
              iv.Printf(_T("(%d)"), (int)ival);
              value.Append(iv);
            } else
              value.Printf(_T("%d"), (int)ival);
          }
        }

        else if (val_str.IsEmpty())
          value = _T("Unknown");

        else {
          value.Clear();
          wxString value_increment;
          wxStringTokenizer tk(val_str, wxT(","));
          int iv = 0;
          while (tk.HasMoreTokens()) {
            wxString token = tk.GetNextToken();
            long ival;
            if (token.ToLong(&ival)) {
              wxString decode_val = GetAttributeDecode(AttrName, ival);
              if (!decode_val.IsEmpty())
                value_increment = decode_val;
              else
                value_increment.Printf(_T(" %d"), (int)ival);

              if (iv) value_increment.Prepend(wxT(", "));
            }
            value.Append(value_increment);

            iv++;
          }
          value.Append(val_str);
        }
      } else
        value = _T("[NULL VALUE]");

      break;
    }

    case OGR_INT: {
      int ival = *((int *)pAttrVal->value);
      wxString decode_val = GetAttributeDecode(AttrName, ival);

      if (!decode_val.IsEmpty()) {
        value = decode_val;
        wxString iv;
        iv.Printf(_T("(%d)"), ival);
        value.Append(iv);
      } else
        value.Printf(_T("(%d)"), ival);

      break;
    }
    case OGR_INT_LST:
      break;

    case OGR_REAL: {
      double dval = *((double *)pAttrVal->value);
      wxString val_suffix = _T(" m");

      //    As a special case, convert some attribute values to feet.....
      if ((AttrName == _T("VERCLR")) || (AttrName == _T("VERCCL")) ||
          (AttrName == _T("VERCOP")) || (AttrName == _T("HEIGHT")) ||
          (AttrName == _T("HORCLR")) || (AttrName == _T("ELEVAT"))) {
        switch (ps52plib->m_nDepthUnitDisplay) {
          case 0:                          // feet
          case 2:                          // fathoms
            dval = dval * 3 * 39.37 / 36;  // feet
            val_suffix = _T(" ft");
            break;
          default:
            break;
        }
      }

      else if ((AttrName == _T("VALSOU")) || (AttrName == _T("DRVAL1")) ||
               (AttrName == _T("DRVAL2"))) {
        switch (ps52plib->m_nDepthUnitDisplay) {
          case 0:                          // feet
            dval = dval * 3 * 39.37 / 36;  // feet
            val_suffix = _T(" ft");
            break;
          case 2:                          // fathoms
            dval = dval * 3 * 39.37 / 36;  // fathoms
            dval /= 6.0;
            val_suffix = _T(" fathoms");
            break;
          default:
            break;
        }
      }

      else if (AttrName == _T("SECTR1"))
        val_suffix = _T("&deg;");
      else if (AttrName == _T("SECTR2"))
        val_suffix = _T("&deg;");
      else if (AttrName == _T("ORIENT"))
        val_suffix = _T("&deg;");
      else if (AttrName == _T("VALNMR"))
        val_suffix = _T(" Nm");
      else if (AttrName == _T("SIGPER"))
        val_suffix = _T("s");
      else if (AttrName == _T("VALACM"))
        val_suffix = _T(" Minutes/year");
      else if (AttrName == _T("VALMAG"))
        val_suffix = _T("&deg;");
      else if (AttrName == _T("CURVEL"))
        val_suffix = _T(" kt");

      if (dval - floor(dval) < 0.01)
        value.Printf(_T("%2.0f"), dval);
      else
        value.Printf(_T("%4.1f"), dval);

      value << val_suffix;

      break;
    }

    case OGR_REAL_LST: {
      break;
    }
  }
  return value;
}

bool s57chart::CompareLights(const S57Light *l1, const S57Light *l2) {
  int positionDiff = l1->position.Cmp(l2->position);
  if (positionDiff < 0) return false;

  int attrIndex1 = l1->attributeNames.Index(_T("SECTR1"));
  int attrIndex2 = l2->attributeNames.Index(_T("SECTR1"));

  // This should put Lights without sectors last in the list.
  if (attrIndex1 == wxNOT_FOUND && attrIndex2 == wxNOT_FOUND) return false;
  if (attrIndex1 != wxNOT_FOUND && attrIndex2 == wxNOT_FOUND) return true;
  if (attrIndex1 == wxNOT_FOUND && attrIndex2 != wxNOT_FOUND) return false;

  double angle1, angle2;
  l1->attributeValues.Item(attrIndex1).ToDouble(&angle1);
  l2->attributeValues.Item(attrIndex2).ToDouble(&angle2);

  return angle1 < angle2;
}

static const char *type2str(GeoPrim_t type) {
  const char *r = "Unknown";
  switch (type) {
    case GEO_POINT:
      return "Point";
      break;
    case GEO_LINE:
      return "Line";
      break;
    case GEO_AREA:
      return "Area";
      break;
    case GEO_META:
      return "Meta";
      break;
    case GEO_PRIM:
      return "Prim";
      break;
  }
  return r;
}

wxString s57chart::CreateObjDescriptions(ListOfObjRazRules *rule_list) {
  wxString ret_val;
  int attrCounter;
  wxString curAttrName, value;
  bool isLight = false;
  wxString className;
  wxString classDesc;
  wxString classAttributes;
  wxString objText;
  wxString lightsHtml;
  wxString positionString;
  std::vector<S57Light *> lights;
  S57Light *curLight = nullptr;
  wxFileName file;

  for (ListOfObjRazRules::Node *node = rule_list->GetLast(); node;
       node = node->GetPrevious()) {
    ObjRazRules *current = node->GetData();
    positionString.Clear();
    objText.Clear();

    // Soundings have no information, so don't show them
    if (0 == strncmp(current->LUP->OBCL, "SOUND", 5)) continue;

    if (current->obj->Primitive_type == GEO_META) continue;
    if (current->obj->Primitive_type == GEO_PRIM) continue;

    className = wxString(current->obj->FeatureName, wxConvUTF8);

    // Lights get grouped together to make display look nicer.
    isLight = !strcmp(current->obj->FeatureName, "LIGHTS");

    //    Get the object's nice description from s57objectclasses.csv
    //    using cpl_csv from the gdal library

    const char *name_desc;
    if (g_csv_locn.Len()) {
      wxString oc_file(g_csv_locn);
      oc_file.Append(_T("/s57objectclasses.csv"));
      name_desc = MyCSVGetField(oc_file.mb_str(), "Acronym",     // match field
                                current->obj->FeatureName,       // match value
                                CC_ExactString, "ObjectClass");  // return field
    } else
      name_desc = "";

    // In case there is no nice description for this object class, use the 6
    // char class name
    if (0 == strlen(name_desc)) {
      name_desc = current->obj->FeatureName;
      classDesc = wxString(name_desc, wxConvUTF8, 1);
      classDesc << wxString(name_desc + 1, wxConvUTF8).MakeLower();
    } else {
      classDesc = wxString(name_desc, wxConvUTF8);
    }

    //    Show LUP
    if (g_bDebugS57) {
      wxString index;

      classAttributes = _T("");
      index.Printf(_T("Feature Index: %d<br>"), current->obj->Index);
      classAttributes << index;

      wxString LUPstring;
      LUPstring.Printf(_T("LUP RCID:  %d<br>"), current->LUP->RCID);
      classAttributes << LUPstring;

      wxString Bbox;
      LLBBox bbox = current->obj->BBObj;
      Bbox.Printf(_T("Lat/Lon box:  %g %g %g %g<br>"), bbox.GetMinLat(),
                  bbox.GetMaxLat(), bbox.GetMinLon(), bbox.GetMaxLon());
      classAttributes << Bbox;

      wxString Type;
      Type.Printf(_T(" Type:  %s<br>"), type2str(current->obj->Primitive_type));
      classAttributes << Type;

      LUPstring = _T("    LUP ATTC: ");
      if (current->LUP->ATTArray.size())
        LUPstring += wxString(current->LUP->ATTArray[0].c_str(), wxConvUTF8);
      LUPstring += _T("<br>");
      classAttributes << LUPstring;

      LUPstring = _T("    LUP INST: ");
      LUPstring += current->LUP->INST;
      LUPstring += _T("<br><br>");
      classAttributes << LUPstring;
    }

    if (GEO_POINT == current->obj->Primitive_type) {
      double lon, lat;
      fromSM((current->obj->x * current->obj->x_rate) + current->obj->x_origin,
             (current->obj->y * current->obj->y_rate) + current->obj->y_origin,
             ref_lat, ref_lon, &lat, &lon);

      if (lon > 180.0) lon -= 360.;

      positionString.Clear();
      positionString += toSDMM(1, lat);
      positionString << _T(" ");
      positionString += toSDMM(2, lon);

      if (isLight) {
        curLight = new S57Light;
        curLight->position = positionString;
        curLight->hasSectors = false;
        lights.push_back(curLight);
      }
    }

    //    Get the Attributes and values, making sure they can be converted from
    //    UTF8
    if (current->obj->att_array) {
      char *curr_att = current->obj->att_array;

      attrCounter = 0;

      wxString attribStr;
      int noAttr = 0;
      attribStr << _T("<table border=0 cellspacing=0 cellpadding=0>");

      if (g_bDebugS57) {
        ret_val << _T("<p>") << classAttributes;
      }

      bool inDepthRange = false;

      while (attrCounter < current->obj->n_attr) {
        //    Attribute name
        curAttrName = wxString(curr_att, wxConvUTF8, 6);
        noAttr++;

        // Sort out how some kinds of attibutes are displayed to get a more
        // readable look. DEPARE gets just its range. Lights are grouped.

        if (isLight) {
          assert(curLight != nullptr);
          curLight->attributeNames.Add(curAttrName);
          if (curAttrName.StartsWith(_T("SECTR"))) curLight->hasSectors = true;
        } else {
          if (curAttrName == _T("DRVAL1")) {
            attribStr << _T("<tr><td><font size=-1>");
            inDepthRange = true;
          } else if (curAttrName == _T("DRVAL2")) {
            attribStr << _T(" - ");
            inDepthRange = false;
          } else {
            if (inDepthRange) {
              attribStr << _T("</font></td></tr>\n");
              inDepthRange = false;
            }
            attribStr << _T("<tr><td valign=top><font size=-2>");
            if (curAttrName == _T("catgeo"))
              attribStr << _T("CATGEO");
            else
              attribStr << curAttrName;
            attribStr << _T("</font></td><td>&nbsp;&nbsp;</td><td ")
                         _T("valign=top><font size=-1>");
          }
        }

        // What we need to do...
        // Change senc format, instead of (S), (I), etc, use the attribute types
        // fetched from the S57attri...csv file This will be like (E), (L), (I),
        // (F)
        //  will affect lots of other stuff.  look for S57attVal.valType
        // need to do this in creatsencrecord above, and update the senc format.

        value = GetObjectAttributeValueAsString(current->obj, attrCounter,
                                                curAttrName);

        // If the atribute value is a filename, change the value into a link to
        // that file
        wxString AttrNamesFiles =
            _T("PICREP,TXTDSC,NTXTDS");  // AttrNames that might have a filename
                                         // as value
        if (AttrNamesFiles.Find(curAttrName) != wxNOT_FOUND)
          if (value.Find(_T(".XML")) == wxNOT_FOUND) {  // Don't show xml files
            file.Assign(GetFullPath());
            file.Assign(file.GetPath(), value);
            file.Normalize();
            // Make the filecheck case-unsensitive (linux)
            if (file.IsCaseSensitive()) {
              wxDir dir(file.GetPath());
              wxString filename;
              bool cont = dir.GetFirst(&filename, "", wxDIR_FILES);
              while (cont) {
                if (filename.IsSameAs(value, false)) {
                  value = filename;
                  file.Assign(file.GetPath(), value);
                  break;
                }
                cont = dir.GetNext(&filename);
              }
            }

            if (file.IsOk()) {
              if (file.Exists())
                value =
                    wxString::Format(_T("<a href=\"%s\">%s</a>"),
                                     file.GetFullPath(), file.GetFullName());
              else
                value = value + _T("&nbsp;&nbsp;<font color=\"red\">[ ") +
                        _("this file is not available") + _T(" ]</font>");
            }
          }
        AttrNamesFiles =
            _T("DATEND,DATSTA,PEREND,PERSTA");  // AttrNames with date info
        if (AttrNamesFiles.Find(curAttrName) != wxNOT_FOUND) {
          bool d = true;
          bool m = true;
          wxString ts = value;

          ts.Replace(wxT("--"),
                     wxT("0000"));  // make a valid year entry if not available
          if (ts.Length() < 5) {    //(no month set)
            m = false;
            ts.Append(
                wxT("01"));  // so we add a fictive month to get a valid date
          }
          if (ts.Length() < 7) {  //(no day set)
            d = false;
            ts.Append(
                wxT("01"));  // so we add a fictive day to get a valid date
          }
          wxString::const_iterator end;
          wxDateTime dt;
          if (dt.ParseFormat(ts, "%Y%m%d", &end)) {
            ts.Empty();
            if (m) ts = wxDateTime::GetMonthName(dt.GetMonth());
            if (d) ts.Append(wxString::Format(wxT(" %d"), dt.GetDay()));
            if (dt.GetYear() > 0)
              ts.Append(wxString::Format(wxT(",  %i"), dt.GetYear()));
            if (curAttrName == _T("PEREND"))
              ts = _("Period ends: ") + ts + wxT("  (") + value + wxT(")");
            if (curAttrName == _T("PERSTA"))
              ts = _("Period starts: ") + ts + wxT("  (") + value + wxT(")");
            if (curAttrName == _T("DATEND"))
              ts = _("Date ending: ") + ts + wxT("  (") + value + wxT(")");
            if (curAttrName == _T("DATSTA"))
              ts = _("Date starting: ") + ts + wxT("  (") + value + wxT(")");
            value = ts;
          }
        }
        if (curAttrName == _T("TS_TSP")) {  // Tidal current applet
          wxArrayString as;
          wxString ts, ts1;
          // value does look like: , 310, 310, 44, 44, 116, 116, 119, 119, 122,
          // 122, 125, 125, 130, 130, 270, 270, 299, 299, 300, 300, 301, 301,
          // 303, 303, 307,307509A,Helgoland,HW,310,0.9,044,0.2,116,1.5,
          // 119,2.2,122,1.9,125,1.5,130,0.9,270,0.1,299,1.4,300,2.1,301,2.0,303,1.7,307,1.2
          wxStringTokenizer tk(value, wxT(","));
          ts1 =
              tk.GetNextToken();  // get first token this will be skipped always
          long l;
          do {  // Skip up upto the first non number. This is Port Name
            ts1 = tk.GetNextToken().Trim(false);
            // some harbourID do have an alpha extension, therefore only check
            // the left(2)
          } while ((ts1.Left(2).ToLong(&l)));
          ts = _T("Tidal Streams referred to<br><b>");
          ts.Append(tk.GetNextToken()).Append(_T("</b> at <b>")).Append(ts1);
          ts.Append(_T("</b><br><table >"));
          int i = -6;
          while (tk.HasMoreTokens()) {  // fill the current table
            ts.Append(_T("<tr><td>"));
            wxString s1(wxString::Format(_T("%+dh "), i));
            ts.Append(s1);
            ts.Append(_T("</td><td>"));
            s1 = tk.GetNextToken();
            ts.Append(s1);
            s1 = "&#176</td><td>";
            ts.Append(s1);
            s1 = tk.GetNextToken();
            ts.Append(s1);
            ts.Append(" kn");
            ts.Append(_T("</td></tr>"));
            i++;
          }
          ts.Append(_T("</table>"));
          value = ts;
        }

        if (isLight) {
          assert(curLight != nullptr);
          curLight->attributeValues.Add(value);
        } else {
          if (curAttrName == _T("INFORM") || curAttrName == _T("NINFOM"))
            value.Replace(_T("|"), _T("<br>"));

          if (curAttrName == _T("catgeo"))
            attribStr << type2str(current->obj->Primitive_type);
          else
            attribStr << value;

          if (!(curAttrName == _T("DRVAL1"))) {
            attribStr << _T("</font></td></tr>\n");
          }
        }

        attrCounter++;
        curr_att += 6;

      }  // while attrCounter < current->obj->n_attr

      if (!isLight) {
        attribStr << _T("</table>\n");

        objText += _T("<b>") + classDesc + _T("</b> <font size=-2>(") +
                   className + _T(")</font>") + _T("<br>");

        if (positionString.Length())
          objText << _T("<font size=-2>") << positionString
                  << _T("</font><br>\n");

        if (noAttr > 0) objText << attribStr;

        if (node != rule_list->GetFirst()) objText += _T("<hr noshade>");
        objText += _T("<br>");
        ret_val << objText;
      }
    }
  }  // Object for loop

  if (!lights.empty()) {
    assert(curLight != nullptr);

    // For lights we now have all the info gathered but no HTML output yet, now
    // run through the data and build a merged table for all lights.

    std::sort(lights.begin(), lights.end(), s57chart::CompareLights);

    wxString lastPos;

    for (auto const &thisLight : lights) {
      int attrIndex;

      if (thisLight->position != lastPos) {
        lastPos = thisLight->position;

        if (thisLight != *lights.begin())
          lightsHtml << _T("</table>\n<hr noshade>\n");

        lightsHtml << _T("<b>Light</b> <font size=-2>(LIGHTS)</font><br>");
        lightsHtml << _T("<font size=-2>") << thisLight->position
                   << _T("</font><br>\n");

        if (curLight->hasSectors)
          lightsHtml << _(
              "<font size=-2>(Sector angles are True Bearings from "
              "Seaward)</font><br>");

        lightsHtml << _T("<table>");
      }

      lightsHtml << _T("<tr>");
      lightsHtml << _T("<td><font size=-1>");

      wxString colorStr;
      attrIndex = thisLight->attributeNames.Index(_T("COLOUR"));
      if (attrIndex != wxNOT_FOUND) {
        wxString color = thisLight->attributeValues.Item(attrIndex);
        if (color == _T("red (3)") || color == _T("red(3)"))
          colorStr =
              _T("<table border=0><tr><td ")
              _T("bgcolor=red>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
        else if (color == _T("green (4)") || color == _T("green(4)"))
          colorStr =
              _T("<table border=0><tr><td ")
              _T("bgcolor=green>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
        else if (color == _T("white (1)") || color == _T("white(1)"))
          colorStr =
              _T("<table border=0><tr><td ")
              _T("bgcolor=white>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
        else if (color == _T("yellow (6)") || color == _T("yellow(6)"))
          colorStr =
              _T("<table border=0><tr><td ")
              _T("bgcolor=yellow>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
        else if (color == _T("blue (5)") || color == _T("blue(5)"))
          colorStr =
              _T("<table border=0><tr><td ")
              _T("bgcolor=blue>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
        else if (color == _T("magenta (12)") || color == _T("magenta(12)"))
          colorStr =
              _T("<table border=0><tr><td ")
              _T("bgcolor=magenta>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
        else
          colorStr =
              _T("<table border=0><tr><td ")
              _T("bgcolor=grey>&nbsp;?&nbsp;</td></tr></table> ");
      }

      int visIndex = thisLight->attributeNames.Index(_T("LITVIS"));
      if (visIndex != wxNOT_FOUND) {
        wxString vis = thisLight->attributeValues.Item(visIndex);
        if (vis.Contains(_T("8"))) {
          if (attrIndex != wxNOT_FOUND) {
            wxString color = thisLight->attributeValues.Item(attrIndex);
            if ((color == _T("red (3)") || color == _T("red(3)")))
              colorStr =
                  _T("<table border=0><tr><td ")
                  _T("bgcolor=DarkRed>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
            if ((color == _T("green (4)") || color == _T("green(4)")))
              colorStr =
                  _T("<table border=0><tr><td ")
                  _T("bgcolor=DarkGreen>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
            if ((color == _T("white (1)") || color == _T("white(1)")))
              colorStr =
                  _T("<table border=0><tr><td ")
                  _T("bgcolor=GoldenRod>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
          }
        }
      }

      lightsHtml << colorStr;

      lightsHtml << _T("</font></td><td><font size=-1><nobr><b>");

      attrIndex = thisLight->attributeNames.Index(_T("LITCHR"));
      if (attrIndex != wxNOT_FOUND) {
        wxString character = thisLight->attributeValues[attrIndex];
        lightsHtml << character.BeforeFirst(wxChar('(')) << _T(" ");
      }

      attrIndex = thisLight->attributeNames.Index(_T("SIGGRP"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << thisLight->attributeValues[attrIndex];
        lightsHtml << _T(" ");
      }

      attrIndex = thisLight->attributeNames.Index(_T("COLOUR"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << _T(" ")
                   << thisLight->attributeValues.Item(attrIndex).Upper()[0];
        lightsHtml << _T(" ");
      }

      attrIndex = thisLight->attributeNames.Index(_T("SIGPER"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << thisLight->attributeValues[attrIndex];
        lightsHtml << _T(" ");
      }

      attrIndex = thisLight->attributeNames.Index(_T("HEIGHT"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << thisLight->attributeValues[attrIndex];
        lightsHtml << _T(" ");
      }

      attrIndex = thisLight->attributeNames.Index(_T("VALNMR"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << thisLight->attributeValues[attrIndex];
        lightsHtml << _T(" ");
      }

      lightsHtml << _T("</b>");

      attrIndex = thisLight->attributeNames.Index(_T("SECTR1"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << _T("(") << thisLight->attributeValues[attrIndex];
        lightsHtml << _T(" - ");
        attrIndex = thisLight->attributeNames.Index(_T("SECTR2"));
        lightsHtml << thisLight->attributeValues[attrIndex] << _T(") ");
      }

      lightsHtml << _T("</nobr>");

      attrIndex = thisLight->attributeNames.Index(_T("CATLIT"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << _T("<nobr>");
        lightsHtml << thisLight->attributeValues[attrIndex].BeforeFirst(
            wxChar('('));
        lightsHtml << _T("</nobr> ");
      }

      attrIndex = thisLight->attributeNames.Index(_T("EXCLIT"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << _T("<nobr>");
        lightsHtml << thisLight->attributeValues[attrIndex].BeforeFirst(
            wxChar('('));
        lightsHtml << _T("</nobr> ");
      }

      attrIndex = thisLight->attributeNames.Index(_T("OBJNAM"));
      if (attrIndex != wxNOT_FOUND) {
        lightsHtml << _T("<br><nobr>");
        lightsHtml << thisLight->attributeValues[attrIndex].Left(1).Upper();
        lightsHtml << thisLight->attributeValues[attrIndex].Mid(1);
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

    lights.clear();
  }

  return ret_val;
}

//------------------------------------------------------------------------
//
//          S57 ENC (i.e. "raw") DataSet support functions
//          Not bulletproof, so call carefully
//
//------------------------------------------------------------------------
bool s57chart::InitENCMinimal(const wxString &FullPath) {
  if (NULL == g_poRegistrar) {
    wxLogMessage(_T("   Error: No ClassRegistrar in InitENCMinimal."));
    return false;
  }

  m_pENCDS = new OGRS57DataSource;

  m_pENCDS->SetS57Registrar(g_poRegistrar);  /// 172

  if (!m_pENCDS->OpenMin(FullPath.mb_str(), TRUE))  /// 172
    return false;

  S57Reader *pENCReader = m_pENCDS->GetModule(0);
  pENCReader->SetClassBased(g_poRegistrar);

  pENCReader->Ingest();

  return true;
}

OGRFeature *s57chart::GetChartFirstM_COVR(int &catcov) {
  //    Get the reader
  S57Reader *pENCReader = m_pENCDS->GetModule(0);

  if ((NULL != pENCReader) && (NULL != g_poRegistrar)) {
    //      Select the proper class
    g_poRegistrar->SelectClass("M_COVR");

    //      Build a new feature definition for this class
    OGRFeatureDefn *poDefn = S57GenerateObjectClassDefn(
        g_poRegistrar, g_poRegistrar->GetOBJL(), pENCReader->GetOptionFlags());

    //      Add this feature definition to the reader
    pENCReader->AddFeatureDefn(poDefn);

    //    Also, add as a Layer to Datasource to ensure proper deletion
    m_pENCDS->AddLayer(new OGRS57Layer(m_pENCDS, poDefn, 1));

    //      find this feature
    OGRFeature *pobjectDef = pENCReader->ReadNextFeature(poDefn);
    if (pobjectDef) {
      //  Fetch the CATCOV attribute
      catcov = pobjectDef->GetFieldAsInteger("CATCOV");
      return pobjectDef;
    }

    else {
      return NULL;
    }
  } else
    return NULL;
}

OGRFeature *s57chart::GetChartNextM_COVR(int &catcov) {
  catcov = -1;

  //    Get the reader
  S57Reader *pENCReader = m_pENCDS->GetModule(0);

  //    Get the Feature Definition, stored in Layer 0
  OGRFeatureDefn *poDefn = m_pENCDS->GetLayer(0)->GetLayerDefn();

  if (pENCReader) {
    OGRFeature *pobjectDef = pENCReader->ReadNextFeature(poDefn);

    if (pobjectDef) {
      catcov = pobjectDef->GetFieldAsInteger("CATCOV");
      return pobjectDef;
    }

    return NULL;
  } else
    return NULL;
}

int s57chart::GetENCScale(void) {
  if (NULL == m_pENCDS) return 0;

  //    Assume that chart has been initialized for minimal ENC access
  //    which implies that the ENC has been fully ingested, and some
  //    interesting values have been extracted thereby.

  //    Get the reader
  S57Reader *pENCReader = m_pENCDS->GetModule(0);

  if (pENCReader)
    return pENCReader->GetCSCL();  /// 172
  else
    return 1;
}

/************************************************************************/
/*                       OpenCPN_OGRErrorHandler()                      */
/*                       Use Global wxLog Class                         */
/************************************************************************/

void OpenCPN_OGRErrorHandler(CPLErr eErrClass, int nError,
                             const char *pszErrorMsg) {
#define ERR_BUF_LEN 2000

  char buf[ERR_BUF_LEN + 1];

  if (eErrClass == CE_Debug)
    sprintf(buf, " %s", pszErrorMsg);
  else if (eErrClass == CE_Warning)
    sprintf(buf, "   Warning %d: %s\n", nError, pszErrorMsg);
  else
    sprintf(buf, "   ERROR %d: %s\n", nError, pszErrorMsg);

  if (g_bGDAL_Debug || (CE_Debug != eErrClass)) {  // log every warning or error
    wxString msg(buf, wxConvUTF8);
    wxLogMessage(msg);
  }

  //      Do not simply return on CE_Fatal errors, as we don't want to abort()

  if (eErrClass == CE_Fatal) {
    longjmp(env_ogrf, 1);  // jump back to the setjmp() point
  }
}

//      In GDAL-1.2.0, CSVGetField is not exported.......
//      So, make my own simplified copy
/************************************************************************/
/*                           MyCSVGetField()                            */
/*                                                                      */
/************************************************************************/

const char *MyCSVGetField(const char *pszFilename, const char *pszKeyFieldName,
                          const char *pszKeyFieldValue,
                          CSVCompareCriteria eCriteria,
                          const char *pszTargetField)

{
  char **papszRecord;
  int iTargetField;

  /* -------------------------------------------------------------------- */
  /*      Find the correct record.                                        */
  /* -------------------------------------------------------------------- */
  papszRecord = CSVScanFileByName(pszFilename, pszKeyFieldName,
                                  pszKeyFieldValue, eCriteria);

  if (papszRecord == NULL) return "";

  /* -------------------------------------------------------------------- */
  /*      Figure out which field we want out of this.                     */
  /* -------------------------------------------------------------------- */
  iTargetField = CSVGetFileFieldId(pszFilename, pszTargetField);
  if (iTargetField < 0) return "";

  if (iTargetField >= CSLCount(papszRecord)) return "";

  return (papszRecord[iTargetField]);
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

bool s57_GetChartExtent(const wxString &FullPath, Extent *pext) {
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

void s57_DrawExtendedLightSectors(ocpnDC &dc, ViewPort &viewport,
                                  std::vector<s57Sector_t> &sectorlegs) {
  float rangeScale = 0.0;

  if (sectorlegs.size() > 0) {
    std::vector<int> sectorangles;
    for (unsigned int i = 0; i < sectorlegs.size(); i++) {
      if (fabs(sectorlegs[i].sector1 - sectorlegs[i].sector2) < 0.3) continue;

      double endx, endy;
      ll_gc_ll(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
               sectorlegs[i].sector1 + 180.0, sectorlegs[i].range, &endy,
               &endx);

      wxPoint end1 = viewport.GetPixFromLL(endy, endx);

      ll_gc_ll(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
               sectorlegs[i].sector2 + 180.0, sectorlegs[i].range, &endy,
               &endx);

      wxPoint end2 = viewport.GetPixFromLL(endy, endx);

      wxPoint lightPos =
          viewport.GetPixFromLL(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x);

      // Make sure arcs are well inside viewport.
      float rangePx = sqrtf(powf((float)(lightPos.x - end1.x), 2) +
                            powf((float)(lightPos.y - end1.y), 2));
      rangePx /= 3.0;
      if (rangeScale == 0.0) {
        rangeScale = 1.0;
        if (rangePx > viewport.pix_height / 3) {
          rangeScale *= (viewport.pix_height / 3) / rangePx;
        }
      }

      rangePx = rangePx * rangeScale;

      int penWidth = rangePx / 8;
      penWidth = wxMin(20, penWidth);
      penWidth = wxMax(5, penWidth);

      int legOpacity;
      wxPen *arcpen = wxThePenList->FindOrCreatePen(sectorlegs[i].color,
                                                    penWidth, wxPENSTYLE_SOLID);
      arcpen->SetCap(wxCAP_BUTT);
      dc.SetPen(*arcpen);

      float angle1, angle2;
      angle1 = -(sectorlegs[i].sector2 + 90.0) - viewport.rotation * 180.0 / PI;
      angle2 = -(sectorlegs[i].sector1 + 90.0) - viewport.rotation * 180.0 / PI;
      if (angle1 > angle2) {
        angle2 += 360.0;
      }
      int lpx = lightPos.x;
      int lpy = lightPos.y;
      int npoints = 0;
      wxPoint arcpoints[150];  // Size relates to "step" below.

      float step = 3.0;
      while ((step < 15) && ((rangePx * sin(step * PI / 180.)) < 10))
        step += 2.0;  // less points on small arcs

      // Make sure we start and stop exactly on the leg lines.
      int narc = (angle2 - angle1) / step;
      narc++;
      step = (angle2 - angle1) / (float)narc;

      if (sectorlegs[i].isleading && (angle2 - angle1 < 60)) {
        wxPoint yellowCone[3];
        yellowCone[0] = lightPos;
        yellowCone[1] = end1;
        yellowCone[2] = end2;
        arcpen = wxThePenList->FindOrCreatePen(wxColor(0, 0, 0, 0), 1,
                                               wxPENSTYLE_SOLID);
        dc.SetPen(*arcpen);
        wxColor c = sectorlegs[i].color;
        c.Set(c.Red(), c.Green(), c.Blue(), 0.6 * c.Alpha());
        dc.SetBrush(wxBrush(c));
        dc.StrokePolygon(3, yellowCone, 0, 0);
        legOpacity = 50;
      } else {
        for (float a = angle1; a <= angle2 + 0.1; a += step) {
          int x = lpx + (int)(rangePx * cos(a * PI / 180.));
          int y = lpy - (int)(rangePx * sin(a * PI / 180.));
          arcpoints[npoints].x = x;
          arcpoints[npoints].y = y;
          npoints++;
        }
        dc.StrokeLines(npoints, arcpoints);
        legOpacity = 128;
      }

      arcpen = wxThePenList->FindOrCreatePen(wxColor(0, 0, 0, legOpacity), 1,
                                             wxPENSTYLE_SOLID);
      dc.SetPen(*arcpen);

      // Only draw each leg line once.

      bool haveAngle1 = false;
      bool haveAngle2 = false;
      int sec1 = (int)sectorlegs[i].sector1;
      int sec2 = (int)sectorlegs[i].sector2;
      if (sec1 > 360) sec1 -= 360;
      if (sec2 > 360) sec2 -= 360;

      if ((sec2 == 360) && (sec1 == 0))  // FS#1437
        continue;

      for (unsigned int j = 0; j < sectorangles.size(); j++) {
        if (sectorangles[j] == sec1) haveAngle1 = true;
        if (sectorangles[j] == sec2) haveAngle2 = true;
      }

      if (!haveAngle1) {
        dc.StrokeLine(lightPos, end1);
        sectorangles.push_back(sec1);
      }

      if (!haveAngle2) {
        dc.StrokeLine(lightPos, end2);
        sectorangles.push_back(sec2);
      }
    }
  }
}

#ifdef ocpnUSE_GL
void s57_DrawExtendedLightSectorsGL(ocpnDC &dc, ViewPort &viewport,
                                    std::vector<s57Sector_t> &sectorlegs) {
  float rangeScale = 0.0;

  if (sectorlegs.size() > 0) {
    std::vector<int> sectorangles;
    for (unsigned int i = 0; i < sectorlegs.size(); i++) {
      if (fabs(sectorlegs[i].sector1 - sectorlegs[i].sector2) < 0.3) continue;

      double endx, endy;
      ll_gc_ll(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
               sectorlegs[i].sector1 + 180.0, sectorlegs[i].range, &endy,
               &endx);

      wxPoint end1 = viewport.GetPixFromLL(endy, endx);

      ll_gc_ll(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
               sectorlegs[i].sector2 + 180.0, sectorlegs[i].range, &endy,
               &endx);

      wxPoint end2 = viewport.GetPixFromLL(endy, endx);

      wxPoint lightPos =
          viewport.GetPixFromLL(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x);

      // Make sure arcs are well inside viewport.
      float rangePx = sqrtf(powf((float)(lightPos.x - end1.x), 2) +
                            powf((float)(lightPos.y - end1.y), 2));
      rangePx /= 3.0;
      if (rangeScale == 0.0) {
        rangeScale = 1.0;
        if (rangePx > viewport.pix_height / 3) {
          rangeScale *= (viewport.pix_height / 3) / rangePx;
        }
      }

      rangePx = rangePx * rangeScale;

      float arcw = rangePx / 10;
      arcw = wxMin(20, arcw);
      arcw = wxMax(5, arcw);

      int legOpacity;

      float angle1, angle2;
      angle1 = -(sectorlegs[i].sector2 + 90.0) - viewport.rotation * 180.0 / PI;
      angle2 = -(sectorlegs[i].sector1 + 90.0) - viewport.rotation * 180.0 / PI;
      if (angle1 > angle2) {
        angle2 += 360.0;
      }
      int lpx = lightPos.x;
      int lpy = lightPos.y;

      if (sectorlegs[i].isleading && (angle2 - angle1 < 60)) {
        wxPoint yellowCone[3];
        yellowCone[0] = lightPos;
        yellowCone[1] = end1;
        yellowCone[2] = end2;
        wxPen *arcpen = wxThePenList->FindOrCreatePen(wxColor(0, 0, 0, 0), 1,
                                                      wxPENSTYLE_SOLID);
        dc.SetPen(*arcpen);
        wxColor c = sectorlegs[i].color;
        c.Set(c.Red(), c.Green(), c.Blue(), 0.6 * c.Alpha());
        dc.SetBrush(wxBrush(c));
        dc.StrokePolygon(3, yellowCone, 0, 0);
        legOpacity = 50;
      } else {
        // Center point
        wxPoint r(lpx, lpy);

        //  radius scaled to display
        float rad = rangePx;

        // float arcw = arc_width * canvas_pix_per_mm;
        //  On larger screens, make the arc_width 1.0 mm
        // if ( m_display_size_mm > 200)     //200 mm, about 8 inches
        // arcw = canvas_pix_per_mm;

        //      Enable anti-aliased lines, at best quality
        glEnable(GL_BLEND);

        float coords[8];
        coords[0] = -rad;
        coords[1] = rad;
        coords[2] = rad;
        coords[3] = rad;
        coords[4] = -rad;
        coords[5] = -rad;
        coords[6] = rad;
        coords[7] = -rad;

        GLShaderProgram *shader = pring_shader_program[0 /*GetCanvasIndex()*/];
        shader->Bind();

        // Get pointers to the attributes in the program.
        GLint mPosAttrib = glGetAttribLocation(shader->programId(), "aPos");

        // Disable VBO's (vertex buffer objects) for attributes.
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords);
        glEnableVertexAttribArray(mPosAttrib);

        //  Circle radius
        GLint radiusloc =
            glGetUniformLocation(shader->programId(), "circle_radius");
        glUniform1f(radiusloc, rad);

        //  Circle center point, physical
        GLint centerloc =
            glGetUniformLocation(shader->programId(), "circle_center");
        float ctrv[2];
        ctrv[0] = r.x;
        ctrv[1] = viewport.pix_height - r.y;
        glUniform2fv(centerloc, 1, ctrv);

        //  Circle color
        wxColour colorb = sectorlegs[i].color;
        float colorv[4];
        colorv[0] = colorb.Red() / float(256);
        colorv[1] = colorb.Green() / float(256);
        colorv[2] = colorb.Blue() / float(256);
        colorv[3] = colorb.Alpha() / float(256);

        GLint colloc =
            glGetUniformLocation(shader->programId(), "circle_color");
        glUniform4fv(colloc, 1, colorv);

        //  Border color
        float bcolorv[4];
        bcolorv[0] = 0;
        bcolorv[1] = 0;
        bcolorv[2] = 0;
        bcolorv[3] = 0;

        GLint bcolloc =
            glGetUniformLocation(shader->programId(), "border_color");
        glUniform4fv(bcolloc, 1, bcolorv);

        //  Border Width
        GLint borderWidthloc =
            glGetUniformLocation(shader->programId(), "border_width");
        glUniform1f(borderWidthloc, 2);

        //  Ring width
        GLint ringWidthloc =
            glGetUniformLocation(shader->programId(), "ring_width");
        glUniform1f(ringWidthloc, arcw);

        //  Visible sectors, rotated to vp orientation
        float sr1 =
            sectorlegs[i].sector1 + (viewport.rotation * 180 / PI) + 180;
        if (sr1 > 360.) sr1 -= 360.;
        float sr2 =
            sectorlegs[i].sector2 + (viewport.rotation * 180 / PI) + 180;
        if (sr2 > 360.) sr2 -= 360.;

        float sb, se;
        if (sr2 > sr1) {
          sb = sr1;
          se = sr2;
        } else {
          sb = sr1;
          se = sr2 + 360;
        }

        //  Shader can handle angles > 360.
        if ((sb < 0) || (se < 0)) {
          sb += 360.;
          se += 360.;
        }

        GLint sector1loc =
            glGetUniformLocation(shader->programId(), "sector_1");
        glUniform1f(sector1loc, (sb * PI / 180.));
        GLint sector2loc =
            glGetUniformLocation(shader->programId(), "sector_2");
        glUniform1f(sector2loc, (se * PI / 180.));

        // Rotate and translate
        mat4x4 I;
        mat4x4_identity(I);
        mat4x4_translate_in_place(I, r.x, r.y, 0);

        GLint matloc =
            glGetUniformLocation(shader->programId(), "TransformMatrix");
        glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)I);

        // Perform the actual drawing.
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Restore the per-object transform to Identity Matrix
        mat4x4 IM;
        mat4x4_identity(IM);
        GLint matlocf =
            glGetUniformLocation(shader->programId(), "TransformMatrix");
        glUniformMatrix4fv(matlocf, 1, GL_FALSE, (const GLfloat *)IM);

        glDisableVertexAttribArray(mPosAttrib);
        shader->UnBind();
      }

#if 1

      wxPen *arcpen = wxThePenList->FindOrCreatePen(wxColor(0, 0, 0, 128), 1,
                                                    wxPENSTYLE_SOLID);
      dc.SetPen(*arcpen);

      // Only draw each leg line once.
      bool haveAngle1 = false;
      bool haveAngle2 = false;
      int sec1 = (int)sectorlegs[i].sector1;
      int sec2 = (int)sectorlegs[i].sector2;
      if (sec1 > 360) sec1 -= 360;
      if (sec2 > 360) sec2 -= 360;

      if ((sec2 == 360) && (sec1 == 0))  // FS#1437
        continue;

      for (unsigned int j = 0; j < sectorangles.size(); j++) {
        if (sectorangles[j] == sec1) haveAngle1 = true;
        if (sectorangles[j] == sec2) haveAngle2 = true;
      }

      if (!haveAngle1) {
        dc.StrokeLine(lightPos, end1);
        sectorangles.push_back(sec1);
      }

      if (!haveAngle2) {
        dc.StrokeLine(lightPos, end2);
        sectorangles.push_back(sec2);
      }
#endif
    }
  }
}
#endif

bool s57_ProcessExtendedLightSectors(ChartCanvas *cc,
                                     ChartPlugInWrapper *target_plugin_chart,
                                     s57chart *Chs57,
                                     ListOfObjRazRules *rule_list,
                                     ListOfPI_S57Obj *pi_rule_list,
                                     std::vector<s57Sector_t> &sectorlegs) {
  bool newSectorsNeedDrawing = false;

  bool bhas_red_green = false;
  bool bleading_attribute = false;

  int opacity = 100;
  if (cc->GetColorScheme() == GLOBAL_COLOR_SCHEME_DUSK) opacity = 50;
  if (cc->GetColorScheme() == GLOBAL_COLOR_SCHEME_NIGHT) opacity = 20;

  int yOpacity = (float)opacity *
                 1.3;  // Matched perception of white/yellow with red/green

  if (target_plugin_chart || Chs57) {
    sectorlegs.clear();

    wxPoint2DDouble objPos;

    char *curr_att = NULL;
    int n_attr = 0;
    wxArrayOfS57attVal *attValArray = NULL;

    ListOfObjRazRules::Node *snode = NULL;
    ListOfPI_S57Obj::Node *pnode = NULL;

    if (Chs57 && rule_list)
      snode = rule_list->GetLast();
    else if (target_plugin_chart && pi_rule_list)
      pnode = pi_rule_list->GetLast();

    while (1) {
      wxPoint2DDouble lightPosD(0, 0);
      bool is_light = false;
      if (Chs57) {
        if (!snode) break;

        ObjRazRules *current = snode->GetData();
        S57Obj *light = current->obj;
        if (!strcmp(light->FeatureName, "LIGHTS")) {
          objPos = wxPoint2DDouble(light->m_lat, light->m_lon);
          curr_att = light->att_array;
          n_attr = light->n_attr;
          attValArray = light->attVal;
          is_light = true;
        }
      } else if (target_plugin_chart) {
        if (!pnode) break;
        PI_S57Obj *light = pnode->GetData();
        if (!strcmp(light->FeatureName, "LIGHTS")) {
          objPos = wxPoint2DDouble(light->m_lat, light->m_lon);
          curr_att = light->att_array;
          n_attr = light->n_attr;
          attValArray = light->attVal;
          is_light = true;
        }
      }

      //  Ready to go
      int attrCounter;
      double sectr1 = -1;
      double sectr2 = -1;
      double valnmr = -1;
      wxString curAttrName;
      wxColor color;

      if (lightPosD.m_x == 0 && lightPosD.m_y == 0.0) lightPosD = objPos;

      if (is_light && (lightPosD == objPos)) {
        if (curr_att) {
          bool bviz = true;

          attrCounter = 0;
          int noAttr = 0;
          s57Sector_t sector;

          bleading_attribute = false;

          while (attrCounter < n_attr) {
            curAttrName = wxString(curr_att, wxConvUTF8, 6);
            noAttr++;

            S57attVal *pAttrVal = NULL;
            if (attValArray) {
              if (Chs57)
                pAttrVal = attValArray->Item(attrCounter);
              else if (target_plugin_chart)
                pAttrVal = attValArray->Item(attrCounter);
            }

            wxString value =
                s57chart::GetAttributeValueAsString(pAttrVal, curAttrName);

            if (curAttrName == _T("LITVIS")) {
              if (value.StartsWith(_T("obsc"))) bviz = false;
            }
            if (curAttrName == _T("SECTR1")) value.ToDouble(&sectr1);
            if (curAttrName == _T("SECTR2")) value.ToDouble(&sectr2);
            if (curAttrName == _T("VALNMR")) value.ToDouble(&valnmr);
            if (curAttrName == _T("COLOUR")) {
              if (value == _T("red(3)")) {
                color = wxColor(255, 0, 0, opacity);
                sector.iswhite = false;
                bhas_red_green = true;
              }

              if (value == _T("green(4)")) {
                color = wxColor(0, 255, 0, opacity);
                sector.iswhite = false;
                bhas_red_green = true;
              }
            }

            if (curAttrName == _T("EXCLIT")) {
              if (value.Find(_T("(3)"))) valnmr = 1.0;  // Fog lights.
            }

            if (curAttrName == _T("CATLIT")) {
              if (value.Upper().StartsWith(_T("DIRECT")) ||
                  value.Upper().StartsWith(_T("LEAD")))
                bleading_attribute = true;
            }

            attrCounter++;
            curr_att += 6;
          }

          if ((sectr1 >= 0) && (sectr2 >= 0)) {
            if (sectr1 > sectr2) {  // normalize
              sectr2 += 360.0;
            }

            sector.pos.m_x = objPos.m_y;  // lon
            sector.pos.m_y = objPos.m_x;

            sector.range =
                (valnmr > 0.0) ? valnmr : 2.5;  // Short default range.
            sector.sector1 = sectr1;
            sector.sector2 = sectr2;

            if (!color.IsOk()) {
              color = wxColor(255, 255, 0, yOpacity);
              sector.iswhite = true;
            }
            sector.color = color;
            sector.isleading = false;  // tentative judgment, check below

            if (bleading_attribute) sector.isleading = true;

            bool newsector = true;
            for (unsigned int i = 0; i < sectorlegs.size(); i++) {
              if (sectorlegs[i].pos == sector.pos &&
                  sectorlegs[i].sector1 == sector.sector1 &&
                  sectorlegs[i].sector2 == sector.sector2) {
                newsector = false;
                //  In the case of duplicate sectors, choose the instance with
                //  largest range. This applies to the case where day and night
                //  VALNMR are different, and so makes the vector result
                //  independent of the order of day/night light features.
                sectorlegs[i].range = wxMax(sectorlegs[i].range, sector.range);
              }
            }

            if (!bviz) newsector = false;

            if ((sector.sector2 == 360) && (sector.sector1 == 0))  // FS#1437
              newsector = false;

            if (newsector) {
              sectorlegs.push_back(sector);
              newSectorsNeedDrawing = true;
            }
          }
        }
      }

      if (Chs57)
        snode = snode->GetPrevious();
      else if (target_plugin_chart)
        pnode = pnode->GetPrevious();

    }  // end of while
  }

  //  Work with the sector legs vector to identify  and mark "Leading Lights"
  //  Sectors with CATLIT "Leading" or "Directional" attribute set have already
  //  been marked
  for (unsigned int i = 0; i < sectorlegs.size(); i++) {
    if (((sectorlegs[i].sector2 - sectorlegs[i].sector1) < 15)) {
      if (sectorlegs[i].iswhite && bhas_red_green)
        sectorlegs[i].isleading = true;
    }
  }

  return newSectorsNeedDrawing;
}

bool s57_GetVisibleLightSectors(ChartCanvas *cc, double lat, double lon,
                                ViewPort &viewport,
                                std::vector<s57Sector_t> &sectorlegs) {
  if (!cc) return false;

  static float lastLat, lastLon;

  if (!ps52plib) return false;

  ChartPlugInWrapper *target_plugin_chart = NULL;
  s57chart *Chs57 = NULL;

  // Find the chart that is currently shown at the given lat/lon
  wxPoint calcPoint = viewport.GetPixFromLL(lat, lon);
  ChartBase *target_chart;
  if (cc->m_singleChart &&
      (cc->m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR))
    target_chart = cc->m_singleChart;
  else if (viewport.b_quilt)
    target_chart = cc->m_pQuilt->GetChartAtPix(viewport, calcPoint);
  else
    target_chart = NULL;

  if (target_chart) {
    if ((target_chart->GetChartType() == CHART_TYPE_PLUGIN) &&
        (target_chart->GetChartFamily() == CHART_FAMILY_VECTOR))
      target_plugin_chart = dynamic_cast<ChartPlugInWrapper *>(target_chart);
    else
      Chs57 = dynamic_cast<s57chart *>(target_chart);
  }

  bool newSectorsNeedDrawing = false;

  if (target_plugin_chart || Chs57) {
    ListOfObjRazRules *rule_list = NULL;
    ListOfPI_S57Obj *pi_rule_list = NULL;

    // Go get the array of all objects at the cursor lat/lon
    float selectRadius = 16 / (viewport.view_scale_ppm * 1852 * 60);

    if (Chs57)
      rule_list =
          Chs57->GetLightsObjRuleListVisibleAtLatLon(lat, lon, &viewport);
    else if (target_plugin_chart)
      pi_rule_list = g_pi_manager->GetLightsObjRuleListVisibleAtLatLon(
          target_plugin_chart, lat, lon, viewport);

    newSectorsNeedDrawing = s57_ProcessExtendedLightSectors(
        cc, target_plugin_chart, Chs57, rule_list, pi_rule_list, sectorlegs);

    if (rule_list) {
      rule_list->Clear();
      delete rule_list;
    }

    if (pi_rule_list) {
      pi_rule_list->Clear();
      delete pi_rule_list;
    }
  }

  return newSectorsNeedDrawing;
}

bool s57_CheckExtendedLightSectors(ChartCanvas *cc, int mx, int my,
                                   ViewPort &viewport,
                                   std::vector<s57Sector_t> &sectorlegs) {
  if (!cc) return false;

  double cursor_lat, cursor_lon;
  static float lastLat, lastLon;

  if (!ps52plib || !ps52plib->m_bExtendLightSectors) return false;

  ChartPlugInWrapper *target_plugin_chart = NULL;
  s57chart *Chs57 = NULL;

  ChartBase *target_chart = cc->GetChartAtCursor();
  if (target_chart) {
    if ((target_chart->GetChartType() == CHART_TYPE_PLUGIN) &&
        (target_chart->GetChartFamily() == CHART_FAMILY_VECTOR))
      target_plugin_chart = dynamic_cast<ChartPlugInWrapper *>(target_chart);
    else
      Chs57 = dynamic_cast<s57chart *>(target_chart);
  }

  cc->GetCanvasPixPoint(mx, my, cursor_lat, cursor_lon);

  if (lastLat == cursor_lat && lastLon == cursor_lon) return false;

  lastLat = cursor_lat;
  lastLon = cursor_lon;
  bool newSectorsNeedDrawing = false;

  if (target_plugin_chart || Chs57) {
    ListOfObjRazRules *rule_list = NULL;
    ListOfPI_S57Obj *pi_rule_list = NULL;

    // Go get the array of all objects at the cursor lat/lon
    float selectRadius = 16 / (viewport.view_scale_ppm * 1852 * 60);

    if (Chs57)
      rule_list = Chs57->GetObjRuleListAtLatLon(
          cursor_lat, cursor_lon, selectRadius, &viewport, MASK_POINT);
    else if (target_plugin_chart)
      pi_rule_list = g_pi_manager->GetPlugInObjRuleListAtLatLon(
          target_plugin_chart, cursor_lat, cursor_lon, selectRadius, viewport);

    newSectorsNeedDrawing = s57_ProcessExtendedLightSectors(
        cc, target_plugin_chart, Chs57, rule_list, pi_rule_list, sectorlegs);

    if (rule_list) {
      rule_list->Clear();
      delete rule_list;
    }

    if (pi_rule_list) {
      pi_rule_list->Clear();
      delete pi_rule_list;
    }
  }

  return newSectorsNeedDrawing;
}
