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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <math.h>
#include <stdlib.h>


#ifdef __OCPN__ANDROID__
// Handle occasional SIG on Android
#include <signal.h>
#include <setjmp.h>

struct sigaction sa_all_plib;
struct sigaction sa_all_plib_previous;

sigjmp_buf env_plib;  // the context saved by sigsetjmp();

void catch_signals_plib(int signo) {
  switch (signo) {
    case SIGSEGV:
      siglongjmp(env_plib, 1);  // jump back to the setjmp() point
      break;

    default:
      break;
  }
}
#endif

#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif
#define DEGREE (PI / 180.0)
#define RADIAN (180.0 / PI)

static const double WGS84_semimajor_axis_meters =
    6378137.0;  // WGS84 semimajor axis
static const double mercator_k0 = 0.9996;

#include "s52plib.h"
#include "mygeom.h"
#include "s52utils.h"
#include "chartsymbols.h"
#include "TexFont.h"
#include "line_clip.h"
#include "poly_math.h"
#include "LOD_reduce.h"
#include "linmath.h"
#ifdef ocpnUSE_GL
#include "Cs52_shaders.h"
#endif

#include <wx/image.h>
#include <wx/tokenzr.h>
#include <wx/fileconf.h>
#include <fstream>

#ifndef PROJECTION_MERCATOR
#define PROJECTION_MERCATOR 1
#endif

// Some methods that are required to be declared differently
// depending on whether the library is build for core, or plugin
#ifdef BUILDING_PLUGIN
#include "ocpn_plugin.h"
#else
wxColour GetFontColour_PlugIn(wxString TextElement);
extern "C" bool GetGlobalColor(wxString colorName, wxColour *pcolour);
wxFont *FindOrCreateFont_PlugIn(
    int point_size, wxFontFamily family, wxFontStyle style, wxFontWeight weight,
    bool underline = false, const wxString &facename = wxEmptyString,
    wxFontEncoding encoding = wxFONTENCODING_DEFAULT);
wxFont *GetOCPNScaledFont_PlugIn(wxString TextElement, int default_size = 0);
float GetOCPNChartScaleFactor_Plugin();
extern "C" wxString *GetpSharedDataLocation();

#endif

#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#endif

float g_scaminScale;

//#ifndef __MSVC__
#define _GLUfuncptrA _GLUfuncptr
//#endif

void DrawAALine(wxDC *pDC, int x0, int y0, int x1, int y1, wxColour clrLine,
                int dash, int space);
extern bool GetDoubleAttr(S57Obj *obj, const char *AttrName, double &val);

void LoadS57Config();
bool loadS52Shaders();

//      Simple and fast CRC32 calculator
unsigned int crc32buf(unsigned char *buf, size_t len);


GLint S52color_tri_shader_program;
GLint S52texture_2D_shader_program;
GLint S52texture_2D_ColorMod_shader_program;
GLint S52circle_filled_shader_program;
GLint S52ring_shader_program;
GLint S52Dash_shader_program;
GLint S52AP_shader_program;

//    Implement all lists
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(TextObjList);

//    Implement all arrays
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfNoshow);

//  S52_TextC Implementation
S52_TextC::S52_TextC() {
  pcol = NULL;
  pFont = NULL;
  texobj = 0;
  bnat = false;
  bspecial_char = false;
}

S52_TextC::~S52_TextC() {
  if (texobj) {
    glDeleteTextures(1, (GLuint *)(&this->texobj));
  }
}

// This is a verbatim copy of same struct found in ocpn_plugin.h
// Used for some types of plugin charts
class PI_line_segment_element {
public:
  size_t vbo_offset;
  size_t n_points;
  int priority;
  float lat_max;  // segment bounding box
  float lat_min;
  float lon_max;
  float lon_min;
  int type;
  void *private0;

  PI_line_segment_element *next;
};


//-----------------------------------------------------------------------------
//      Comparison Function for LUPArray sorting
//      Note Global Scope
//-----------------------------------------------------------------------------
#ifndef _COMPARE_LUP_DEFN_
#define _COMPARE_LUP_DEFN_

int CompareLUPObjects(LUPrec *item1, LUPrec *item2) {
  // sort the items by their name...
  int ir = strcmp(item1->OBCL, item2->OBCL);

  if (ir != 0) return ir;
  int c1 = item1->ATTArray.size();
  int c2 = item2->ATTArray.size();

  if (c1 != c2) return c2 - c1;
  return item1->nSequence - item2->nSequence;
}

#endif

//-----------------------------------------------------------------------------
//      LUPArrayContainer implementation
//-----------------------------------------------------------------------------
LUPArrayContainer::LUPArrayContainer() {
  //   Build the initially empty sorted arrays of LUP Records, per LUP type.
  //   Sorted on object name, e.g. ACHARE.  Why sorted?  Helps in the
  //   S52_LUPLookup method....
  LUPArray = new wxArrayOfLUPrec(CompareLUPObjects);
}

LUPArrayContainer::~LUPArrayContainer() {
  if (LUPArray) {
    for (unsigned int il = 0; il < LUPArray->GetCount(); il++)
      s52plib::DestroyLUP(LUPArray->Item(il));

    LUPArray->Clear();
    delete LUPArray;
  }

  LUPArrayIndexHash::iterator it;
  for (it = IndexHash.begin(); it != IndexHash.end(); ++it) {
    free(it->second);
  }
}

LUPHashIndex *LUPArrayContainer::GetArrayIndexHelper(const char *objectName) {
  // Look for the key
  wxString key(objectName, wxConvUTF8);
  LUPArrayIndexHash::iterator it = IndexHash.find(key);

  if (it == IndexHash.end()) {
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

    while (!first_match && (index < index_max)) {
      LUPCandidate = LUPArray->Item(index);
      if (!strcmp(objectName, LUPCandidate->OBCL)) {
        pindex->n_start = index;
        first_match = 1;
        ocnt++;
        index++;
        break;
      }
      index++;
    }

    while (first_match && (index < index_max)) {
      LUPCandidate = LUPArray->Item(index);
      if (!strcmp(objectName, LUPCandidate->OBCL)) {
        ocnt++;
      } else {
        break;
      }

      index++;
    }

    pindex->count = ocnt;

    return pindex;
  } else
    return it->second;  // return a pointer to the found record
}

//-----------------------------------------------------------------------------
//      s52plib implementation
//-----------------------------------------------------------------------------
s52plib::s52plib(const wxString &PLib, bool b_forceLegacy) {
  m_plib_file = PLib;

  pOBJLArray = new wxArrayPtrVoid;

  condSymbolLUPArray = NULL;  // Dynamic Conditional Symbology

  _symb_sym = NULL;

  m_txf_ready = false;
  m_txf = NULL;

  m_chartSymbols.InitializeTables();
  m_chartSymbols.SetTextureFormat(GL_TEXTURE_2D);
  InitializeNatsurHash();

  m_bOK = !(S52_load_Plib(PLib, b_forceLegacy) == 0);

  m_bShowS57Text = false;
  m_bShowS57ImportantTextOnly = false;
  m_colortable_index = 0;

  _symb_symR = NULL;
  bUseRasterSym = false;

  //      Sensible defaults
  m_nSymbolStyle = PAPER_CHART;
  m_nBoundaryStyle = PLAIN_BOUNDARIES;
  m_nDisplayCategory = OTHER;
  m_nDepthUnitDisplay = 1;  // metres

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
  AddObjNoshow("M_QUAL");
  m_lightsOff = false;
  m_anchorOn = true;
  m_qualityOfDataOn = false;

  m_SoundingsScaleFactor = 1.0;
  m_SoundingsFontSizeMM = 0;
  m_soundFontDelta = 0;

  GenerateStateHash();

  HPGL = new RenderFromHPGL(this);

  //  Set defaults for OCPN version, may be overridden later
  m_coreVersionMajor = 4;
  m_coreVersionMinor = 6;
  m_coreVersionPatch = 0;

  m_myConfig = GetStateHash();

  // GL Options/capabilities
  m_useStencil = false;
  m_useStencilAP = false;
  m_useScissors = false;
  m_useFBO = false;
  m_GLAC_VBO = false;
  m_useVBO = false;
  m_useGLSL = false;
  m_TextureFormat = -1;
  m_GLMinCartographicLineWidth = 1.0;
  m_GLMinSymbolLineWidth = 1.0;

  m_display_size_mm = 300;
  SetGLPolygonSmoothing(false);
  SetGLLineSmoothing(false);

  m_displayScale = 1.0;
  m_display_width = 0;

  // Clear the TexFont cache
  unsigned int i;
  for (i = 0; i < TXF_CACHE; i++) {
     s_txf[i].key = 0;
     s_txf[i].cache = 0;
  }
  m_dipfactor = 1.0;
  m_ContentScaleFactor = 1.0;
  m_FinalTextScaleFactor = 0;
  m_TextScaleFactor = 1;
  m_nTextFactor = 0;
}

s52plib::~s52plib() {
  delete areaPlain_LAC;
  delete line_LAC;
  delete areaSymbol_LAC;
  delete pointSimple_LAC;
  delete pointPaper_LAC;

  S52_flush_Plib();

  //      Free the OBJL Array Elements
  for (unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++)
    free(pOBJLArray->Item(iPtr));

  delete pOBJLArray;

  delete[] ledge;
  delete[] redge;

  m_chartSymbols.DeleteGlobals();

  delete HPGL;
  for(int i = 0; i < TXF_CACHE; i++) {
    if(s_txf[i].cache) {
      delete(s_txf[i].cache);
    }
  }
}

void s52plib::InitializeNatsurHash() {
  std::unordered_map<int, std::string> surmap({
                                                  {1, "mud"},
                                                  {2, "clay"},
                                                  {3, "silt"},
                                                  {4, "sand"},
                                                  {5, "stone"},
                                                  {6, "gravel"},
                                                  {7, "pebbles"},
                                                  {8, "cobbles"},
                                                  {9, "rock"},
                                                  {11, "lava"},
                                                  {14, "coral"},
                                                  {17, "shells"},
                                                  {18, "boulder"},
                                                  {56, "Bo"},
                                                  {51, "Wd"}  });
  m_natsur_hash = surmap;
}

void s52plib::SetVPointCompat(int pix_width,int pix_height,
                      double view_scale_ppm, double rotation,
                      double clat, double clon,
                      double chart_scale,
                      wxRect rv_rect, LLBBox &bbox, double ref_scale,
                      double display_scale
                      ) {
  vp_plib.pix_width = pix_width;
  vp_plib.pix_height = pix_height;
  vp_plib.view_scale_ppm = view_scale_ppm;
  vp_plib.rotation = rotation;
  vp_plib.clat = clat;
  vp_plib.clon = clon;
  vp_plib.chart_scale = chart_scale;
  vp_plib.rv_rect = rv_rect;
  BBox = bbox;
  reducedBBox = bbox;
  vp_plib.ref_scale = ref_scale;
  m_displayScale = display_scale;
}


void s52plib::SetOCPNVersion(int major, int minor, int patch) {
  m_coreVersionMajor = major;
  m_coreVersionMinor = minor;
  m_coreVersionPatch = patch;
}

void s52plib::SetGLOptions(bool b_useStencil, bool b_useStencilAP,
                           bool b_useScissors, bool b_useFBO, bool b_useVBO,
                           int nTextureFormat,
                           float MinCartographicLineWidth,
                           float MinSymbolLineWidth)
{
  // Set GL Options/capabilities
  m_useStencil = b_useStencil;
  m_useStencilAP = b_useStencilAP;
  m_useScissors = b_useScissors;
  m_useFBO = b_useFBO;
  m_useVBO = b_useVBO;
  m_useGLSL = true;
  m_TextureFormat = nTextureFormat;
  m_chartSymbols.SetTextureFormat(nTextureFormat);
  m_GLMinCartographicLineWidth = MinCartographicLineWidth;
  m_GLMinSymbolLineWidth = MinSymbolLineWidth;

}

void s52plib::SetDIPFactor( double factor) {
  m_dipfactor = factor;
}

void s52plib::SetContentScaleFactor( double factor) {
  m_ContentScaleFactor = factor;
}

void s52plib::SetPPMM(float ppmm) {

  canvas_pix_per_mm = ppmm;

  // We need a supplemental scale factor for HPGL vector symbol rendering.
  //  to allow raster and vector symbols to be rendered harmoniously
  //  We do this empirically, making it look "nice" on average displays.
  m_rv_scale_factor = 0.8;

  // Estimate the display size
  m_display_size_mm = m_display_width / GetPPMM();  // accurate enough for internal use

  m_display_size_mm /= m_displayScale;
}

void s52plib::SetScaleFactorExp(double ChartScaleFactorExp) {
  m_ChartScaleFactorExp = ChartScaleFactorExp;
}

void s52plib::SetScaleFactorZoomMod(int chart_zoom_modifier_vector) {
  m_chart_zoom_modifier_vector = chart_zoom_modifier_vector;
}

//      Various static helper methods

void s52plib::DestroyLUP(LUPrec *pLUP) {
  Rules *top = pLUP->ruleList;
  DestroyRulesChain(top);

  delete pLUP;
}

void s52plib::DestroyRulesChain(Rules *top) {
  while (top != NULL) {
    Rules *Rtmp = top->next;

    free(top->INST0);  // free the Instruction string head

    if (top->b_private_razRule)  // need to free razRule?
    {
      Rule *pR = top->razRule;
      delete pR->exposition.LXPO;

      free(pR->vector.LVCT);

      delete pR->bitmap.SBTM;

      free(pR->colRef.SCRF);

      ClearRulesCache(pR);

      free(pR);
    }

    free(top);
    top = Rtmp;
  }
}

DisCat s52plib::findLUPDisCat(const char *objectName, LUPname TNAM) {
  LUPArrayContainer *plac = SelectLUPArrayContainer(TNAM);

  wxArrayOfLUPrec *LUPArray = SelectLUPARRAY(TNAM);

  //      Find the first matching entry in the LUP Array
  int index = 0;
  int index_max = LUPArray->GetCount();
  LUPrec *LUPCandidate;

  while (index < index_max) {
    LUPCandidate = LUPArray->Item(index);
    if (!strcmp(objectName, LUPCandidate->OBCL)) {
      return LUPCandidate->DISC;
    }
    index++;
  }

  return (DisCat)(-1);
}

bool s52plib::GetAnchorOn() {
  //  Investigate and report the logical condition that "Anchoring Condition" is
  //  shown

  int old_vis = 0;

  if (MARINERS_STANDARD == GetDisplayCategory()) {
    old_vis = m_anchorOn;
  } else if (OTHER == GetDisplayCategory())
    old_vis = true;

  // other cat
  // const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE",
  // "PIPSOL", "TUNNEL", "SBDARE" };

  old_vis &= !IsObjNoshow("SBDARE");

  return (old_vis != 0);
}

bool s52plib::GetQualityOfData() {
  //  Investigate and report the logical condition that "Quality of Data
  //  Condition" is shown

  int old_vis = 0;

  if (MARINERS_STANDARD == GetDisplayCategory()) {
    for (unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++) {
      OBJLElement *pOLE = (OBJLElement *)(pOBJLArray->Item(iPtr));
      if (!strncmp(pOLE->OBJLName, "M_QUAL", 6)) {
        old_vis = pOLE->nViz;
        break;
      }
    }
  } else if (OTHER == GetDisplayCategory())
    old_vis = true;

  old_vis &= !IsObjNoshow("M_QUAL");

  return (old_vis != 0);
}

void s52plib::SetGLRendererString(const wxString &renderer) {
  m_renderer_string = renderer;

  // No chart type in current use requires VBO for GLAC rendering
  // Experimentation has shown that VBO is slower for GLAC rendering,
  //  since the per-object state change of glBindBuffer() is slow
  //  on most hardware, especially RPi.
  // However, we have found that NVidea GPUs
  //  perform much better with VBO on GLAC operations, so set that up.

  if ((renderer.Upper().Contains("NVIDIA")) ||
      (renderer.Upper().Contains("QUADRO")) ||
      (renderer.Upper().Contains("GEFORCE")))
    m_GLAC_VBO = true;

}

/*
 Update the S52 Conditional Symbology Parameter Set to reflect the
 current state of the library member options.
 */

void s52plib::UpdateMarinerParams(void) {
  //      Symbol Style
  if (SIMPLIFIED == m_nSymbolStyle)
    S52_setMarinerParam(S52_MAR_SYMPLIFIED_PNT, 1.0);
  else
    S52_setMarinerParam(S52_MAR_SYMPLIFIED_PNT, 0.0);

  //      Boundary Style
  if (SYMBOLIZED_BOUNDARIES == m_nBoundaryStyle)
    S52_setMarinerParam(S52_MAR_SYMBOLIZED_BND, 1.0);
  else
    S52_setMarinerParam(S52_MAR_SYMBOLIZED_BND, 0.0);
}

void s52plib::GenerateStateHash() {
  unsigned char state_buffer[512];  // Needs to be at least this big...
  memset(state_buffer, 0, sizeof(state_buffer));

//  int time = ::wxGetUTCTime();
//  memcpy(state_buffer, &time, sizeof(int));

  size_t offset = sizeof(int);  // skipping the time int, first element

  for (int i = 0; i < S52_MAR_NUM; i++) {
    if ((offset + sizeof(double)) < sizeof(state_buffer)) {
      double t = S52_getMarinerParam((S52_MAR_param_t)i);
      memcpy(&state_buffer[offset], &t, sizeof(double));
      offset += sizeof(double);
    }
  }

  for (unsigned int i = 0; i < m_noshow_array.GetCount(); i++) {
    if ((offset + 6) < sizeof(state_buffer)) {
      memcpy(&state_buffer[offset], m_noshow_array[i].obj, 6);
      offset += 6;
    }
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_bShowSoundg, sizeof(bool));
    offset += sizeof(bool);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_bShowS57Text, sizeof(bool));
    offset += sizeof(bool);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_bShowS57ImportantTextOnly, sizeof(bool));
    offset += sizeof(bool);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_bDeClutterText, sizeof(bool));
    offset += sizeof(bool);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_bShowNationalTexts, sizeof(bool));
    offset += sizeof(bool);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_bShowAtonText, sizeof(bool));
    offset += sizeof(bool);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_bShowLdisText, sizeof(bool));
    offset += sizeof(bool);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_bExtendLightSectors, sizeof(bool));
    offset += sizeof(bool);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_nSoundingFactor, sizeof(int));
    offset += sizeof(int);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_nTextFactor, sizeof(int));
    offset += sizeof(int);
  }

  if (offset + sizeof(bool) < sizeof(state_buffer)) {
    memcpy(&state_buffer[offset], &m_nDisplayCategory, sizeof(int));
    offset += sizeof(int);
  }

  m_state_hash = crc32buf(state_buffer, offset);
}

wxArrayOfLUPrec *s52plib::SelectLUPARRAY(LUPname TNAM) {
  switch (TNAM) {
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

LUPArrayContainer *s52plib::SelectLUPArrayContainer(LUPname TNAM) {
  switch (TNAM) {
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

LUPrec *s52plib::FindBestLUP(wxArrayOfLUPrec *LUPArray, unsigned int startIndex,
                             unsigned int count, S57Obj *pObj, bool bStrict) {
  //  Check the parameters
  if (0 == count) return NULL;
  if (startIndex >= LUPArray->GetCount()) return NULL;

  // setup default return to the first LUP that matches Feature name.
  LUPrec *LUP = LUPArray->Item(startIndex);

  int nATTMatch = 0;
  int countATT = 0;
  bool bmatch_found = false;

  if (pObj->att_array == NULL)
    goto check_LUP;  // object has no attributes to compare, so return "best"
                     // LUP

  for (unsigned int i = 0; i < count; ++i) {
    LUPrec *LUPCandidate = LUPArray->Item(startIndex + i);

    if (!LUPCandidate->ATTArray.size())
      continue;  // this LUP has no attributes coded

    countATT = 0;
    char *currATT = pObj->att_array;
    int attIdx = 0;

    for (unsigned int iLUPAtt = 0; iLUPAtt < LUPCandidate->ATTArray.size();
         iLUPAtt++) {
      // Get the LUP attribute name
      const char *slatc = LUPCandidate->ATTArray[iLUPAtt].c_str();

      if (slatc && (strlen(slatc) < 6))
        goto next_LUP_Attr;  // LUP attribute value not UTF8 convertible (never
                             // seen in PLIB 3.x)

      if (slatc) {
        const char *slatv = slatc + 6;
        while (attIdx < pObj->n_attr) {
          if (0 == strncmp(slatc, currATT, 6)) {
            // OK we have an attribute name match

            bool attValMatch = false;

            // special case (i)
            if (!strncmp(slatv, " ", 1)) {  // any object value will match wild
                                            // card (S52 para 8.3.3.4)
              ++countATT;
              goto next_LUP_Attr;
            }

            // special case (ii)
            // TODO  Find an ENC with "UNKNOWN" DRVAL1 or DRVAL2 and debug this
            // code
            if (!strncmp(slatv, "?",
                         1)) {  // if LUP attribute value is "undefined"

              //  Match if the object does NOT contain this attribute
              goto next_LUP_Attr;
            }

            // checking against object attribute value
            S57attVal *v = (pObj->attVal->Item(attIdx));

            switch (v->valType) {
              case OGR_INT:  // S57 attribute type 'E' enumerated, 'I' integer
              {
                int LUP_att_val = atoi(slatv);
                if (LUP_att_val == *(int *)(v->value)) attValMatch = true;
                break;
              }

              case OGR_INT_LST:  // S57 attribute type 'L' list: comma separated
                                 // integer
              {
                int a;
                char ss[41];
                strncpy(ss, slatv, 39);
                ss[40] = '\0';
                char *s = &ss[0];

                int *b = (int *)v->value;
                sscanf(s, "%d", &a);

                while (*s != '\0') {
                  if (a == *b) {
                    sscanf(++s, "%d", &a);
                    b++;
                    attValMatch = true;

                  } else
                    attValMatch = false;
                }
                break;
              }
              case OGR_REAL:  // S57 attribute type'F' float
              {
                double obj_val = *(double *)(v->value);
                float att_val = atof(slatv);
                if (fabs(obj_val - att_val) < 1e-6)
                  if (obj_val == att_val) attValMatch = true;
                break;
              }

              case OGR_STR:  // S57 attribute type'A' code string, 'S' free text
              {
                //    Strings must be exact match
                //    n.b. OGR_STR is used for S-57 attribute type 'L',
                //    comma-separated list

                // wxString cs( (char *) v->value, wxConvUTF8 ); // Attribute
                // from object if( LATTC.Mid( 6 ) == cs )
                if (!strcmp((char *)v->value, slatv)) attValMatch = true;
                break;
              }

              default:
                break;
            }  // switch

            // value match
            if (attValMatch) ++countATT;

            goto next_LUP_Attr;
          }  // if attribute name match

          //  Advance to the next S57obj attribute
          currATT += 6;
          ++attIdx;

        }  // while
      }    // if

    next_LUP_Attr:

      currATT = pObj->att_array;  // restart the object attribute list
      attIdx = 0;
    }  // for iLUPAtt

    //      Create a "match score", defined as fraction of candidate LUP
    //      attributes actually matched by feature. Used later for resolving
    //      "ties"

    int nattr_matching_on_candidate = countATT;
    int nattrs_on_candidate = LUPCandidate->ATTArray.size();
    double candidate_score =
        (1. * nattr_matching_on_candidate) / (1. * nattrs_on_candidate);

    //       According to S52 specs, match must be perfect,
    //         and the first 100% match is selected
    if (candidate_score == 1.0) {
      LUP = LUPCandidate;
      bmatch_found = true;
      break;  // selects the first 100% match
    }

  }  // for loop

check_LUP:
  //  In strict mode, we require at least one attribute to match exactly

  if (bStrict) {
    if (nATTMatch == 0)  // nothing matched
      LUP = NULL;
  } else {
    //      If no match found, return the first LUP in the list which has no
    //      attributes
    if (!bmatch_found) {
      for (unsigned int j = 0; j < count; ++j) {
        LUPrec *LUPtmp = NULL;

        LUPtmp = LUPArray->Item(startIndex + j);
        if (!LUPtmp->ATTArray.size()) {
          return LUPtmp;
        }
      }
    }
  }

  return LUP;
}

// scan foward stop on ; or end-of-record
#define SCANFWRD \
  while (!(*str == ';' || *str == '\037')) ++str;

#define INSTRUCTION(s, t)        \
  if (0 == strncmp(s, str, 2)) { \
    str += 3;                    \
    r->ruleType = t;             \
    r->INSTstr = str;

Rules *s52plib::StringToRules(const wxString &str_in) {
  wxCharBuffer buffer = str_in.ToUTF8();
  if (!buffer.data()) return NULL;

  size_t len = strlen(buffer.data());
  char *str0 = (char *)calloc(len + 1, 1);
  memcpy(str0, buffer.data(), len);
  char *str = str0;

  Rules *top;
  Rules *last;
  char strk[20];

  //    Allocate and pre-clear the Rules structure
  Rules *r = (Rules *)calloc(1, sizeof(Rules));
  top = r;
  last = top;

  r->INST0 = str0;  // save the head for later free

  while (*str != '\0') {
    if (r->ruleType)  // in the loop, r has been used
    {
      r = (Rules *)calloc(1, sizeof(Rules));
      last->next = r;
      last = r;
    }

    // parse Symbology instruction in string

    // Special Case for Circular Arc,  (opencpn private)
    // Allocate a Rule structure to be used to hold a cached bitmap of the
    // created symbol
    INSTRUCTION("CA", RUL_ARC_2C)
    r->razRule = (Rule *)calloc(1, sizeof(Rule));
    r->b_private_razRule = true;  // mark this raxRule to be free'd later
    SCANFWRD
  }

  // Special Case for MultPoint Soundings
  INSTRUCTION("MP", RUL_MUL_SG)
  SCANFWRD
}

// SHOWTEXT
INSTRUCTION("TX", RUL_TXT_TX)
SCANFWRD
}

INSTRUCTION("TE", RUL_TXT_TE)
SCANFWRD
}

// SHOWPOINT

if (0 == strncmp("SY", str, 2)) {
  str += 3;
  r->ruleType = RUL_SYM_PT;
  r->INSTstr = str;

  strncpy(strk, str, 8);
  strk[8] = 0;
  wxString key(strk, wxConvUTF8);

  r->razRule = (*_symb_sym)[key];

  if (r->razRule == NULL) r->razRule = (*_symb_sym)[_T ( "QUESMRK1" )];

  SCANFWRD
}

// SHOWLINE
INSTRUCTION("LS", RUL_SIM_LN)
SCANFWRD
}

INSTRUCTION("LC", RUL_COM_LN)
strncpy(strk, str, 8);
strk[8] = 0;
wxString key(strk, wxConvUTF8);

r->razRule = (*_line_sym)[key];

if (r->razRule == NULL) r->razRule = (*_symb_sym)[_T ( "QUESMRK1" )];
SCANFWRD
}

// SHOWAREA
INSTRUCTION("AC", RUL_ARE_CO)
SCANFWRD
}

INSTRUCTION("AP", RUL_ARE_PA)
strncpy(strk, str, 8);
strk[8] = 0;
wxString key(strk, wxConvUTF8);

r->razRule = (*_patt_sym)[key];
if (r->razRule == NULL) r->razRule = (*_patt_sym)[_T ( "QUESMRK1V" )];
SCANFWRD
}

// CALLSYMPROC

if (0 == strncmp("CS", str, 2)) {
  str += 3;
  r->ruleType = RUL_CND_SY;
  r->INSTstr = str;

  //      INSTRUCTION("CS",RUL_CND_SY)
  char stt[9];
  strncpy(stt, str, 8);
  stt[8] = 0;
  wxString index(stt, wxConvUTF8);
  r->razRule = (*_cond_sym)[index];
  if (r->razRule == NULL) r->razRule = (*_cond_sym)[_T ( "QUESMRK1" )];
  SCANFWRD
}

++str;
}

//  If it should happen that no rule is built, delete the initially allocated
//  rule
if (0 == top->ruleType) {
  if (top->INST0) free(top->INST0);

  free(top);

  top = NULL;
}

//   Traverse the entire rule set tree, pruning after first unallocated (dead)
//   rule
r = top;
while (r) {
  if (0 == r->ruleType) {
    free(r);
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
while (r) {
  r->n_sequence = i++;

  r = r->next;
}

return top;
}

int s52plib::_LUP2rules(LUPrec *LUP, S57Obj *pObj) {
  if (NULL == LUP) return -1;
  // check if already parsed
  if (LUP->ruleList != NULL) {
    return 0;
  }

  if (!LUP->INST.IsEmpty()) {
    Rules *top = StringToRules(LUP->INST);
    LUP->ruleList = top;

    return 1;
  } else
    return 0;
}

int s52plib::S52_load_Plib(const wxString &PLib, bool b_forceLegacy) {
  pAlloc = new wxArrayPtrVoid;

  //   Create the Rule Lookup Hash Tables
  _line_sym = new RuleHash;  // line
  _patt_sym = new RuleHash;  // pattern
  _symb_sym = new RuleHash;  // symbol
  _cond_sym = new RuleHash;  // conditional

  line_LAC = new LUPArrayContainer;
  areaPlain_LAC = new LUPArrayContainer;
  areaSymbol_LAC = new LUPArrayContainer;
  pointSimple_LAC = new LUPArrayContainer;
  pointPaper_LAC = new LUPArrayContainer;

  condSymbolLUPArray =
      new wxArrayOfLUPrec(CompareLUPObjects);  // dynamic Cond Sym LUPs

  m_unused_color.R = 2;
  m_unused_color.G = 2;
  m_unused_color.B = 2;
  m_unused_wxColor.Set(2, 2, 2);

  if (!m_chartSymbols.LoadConfigFile(this, PLib)) {
    wxString msg(_T("Could not load XML PLib symbol file: "));
    msg += PLib;
    wxLogMessage(msg);

    return 0;
  }

  //   Initialize the _cond_sym Hash Table from the jump table found in
  //   S52CNSY.CPP Hash Table indices are the literal CS Strings, e.g.
  //   "RESARE02" Hash Results Values are the Rule *, i.e. the CS procedure
  //   entry point

  for (int i = 0; condTable[i].condInst != NULL; ++i) {
    wxString index(condTable[i].name, wxConvUTF8);
    (*_cond_sym)[index] = (Rule *)(condTable[i].condInst);
  }

  wxString s57data_dir = *GetpSharedDataLocation();
  s57data_dir += _T("s57data");

  wxString oc_file(s57data_dir);
  oc_file.Append(_T("/s57objectclasses.csv"));

  PreloadOBJLFromCSV(oc_file);

  return 1;
}

void s52plib::ClearRulesCache(
    Rule *pR)  //  Clear out any existing cached symbology
{
  switch (pR->parm0) {
    case ID_wxBitmap: {
      wxBitmap *pbm = (wxBitmap *)(pR->pixelPtr);
      delete pbm;
      pR->pixelPtr = NULL;
      pR->parm0 = ID_EMPTY;
      break;
    }
    case ID_RGBA: {
      unsigned char *p = (unsigned char *)(pR->pixelPtr);
      free(p);
      pR->pixelPtr = NULL;
      pR->parm0 = ID_EMPTY;
      break;
    }
    case ID_GL_PATT_SPEC: {
      render_canvas_parms *pp = (render_canvas_parms *)(pR->pixelPtr);
      free(pp->pix_buff);
#ifdef ocpnUSE_GL
      if (pp->OGL_tex_name) glDeleteTextures(1, (GLuint *)&pp->OGL_tex_name);
#endif
      delete pp;
      pR->pixelPtr = NULL;
      pR->parm0 = ID_EMPTY;
      break;
    }
    case ID_RGB_PATT_SPEC: {
      render_canvas_parms *pp = (render_canvas_parms *)(pR->pixelPtr);
      free(pp->pix_buff);
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

void s52plib::DestroyPatternRuleNode(Rule *pR) { DestroyRuleNode(pR); }

void s52plib::DestroyRuleNode(Rule *pR) {
  if (!pR) return;

  delete pR->exposition.LXPO;

  free(pR->vector.LVCT);

  delete pR->bitmap.SBTM;

  free(pR->colRef.SCRF);

  ClearRulesCache(pR);  //  Clear out any existing cached symbology
}

void s52plib::DestroyRules(RuleHash *rh) {
  RuleHash::iterator it;
  Rule *pR;

  for (it = (*rh).begin(); it != (*rh).end(); ++it) {
    pR = it->second;
    DestroyRuleNode(pR);
  }

  rh->clear();
  delete rh;
}

void s52plib::FlushSymbolCaches(const ChartCtx& ctx) {
  m_chartSymbols.LoadRasterFileForColorTable(m_colortable_index, true, ctx);
  RuleHash *rh = _symb_sym;

  if (!rh) return;

  RuleHash::iterator it;
  Rule *pR;

  for (it = (*rh).begin(); it != (*rh).end(); ++it) {
    pR = it->second;
    if (pR) ClearRulesCache(pR);
  }

  //    Flush any pattern definitions
  rh = _patt_sym;

  if (!rh) return;

  for (it = (*rh).begin(); it != (*rh).end(); ++it) {
    pR = it->second;
    if (pR) ClearRulesCache(pR);
  }

  //    OpenGL Hashmaps
  CARC_Hash::iterator ita;
  for (ita = m_CARC_hashmap.begin(); ita != m_CARC_hashmap.end(); ++ita) {
    CARC_Buffer buffer = ita->second;
    delete[] buffer.data;
  }
  m_CARC_hashmap.clear();

  // Flush all texFonts
  TexFont *f_cache = 0;
  unsigned int i;
  for (i = 0; i < TXF_CACHE; i++) {
    if (s_txf[i].key != 0) {
      f_cache = s_txf[i].cache;
      f_cache->Delete();
      s_txf[i].key = 0;
    }
  }
}

void s52plib::DestroyPattRules(RuleHash *rh) { DestroyRules(rh); }

void s52plib::DestroyLUPArray(wxArrayOfLUPrec *pLUPArray) {
  if (pLUPArray) {
    for (unsigned int il = 0; il < pLUPArray->GetCount(); il++)
      DestroyLUP(pLUPArray->Item(il));

    pLUPArray->Clear();

    delete pLUPArray;
  }
}

void s52plib::ClearCNSYLUPArray(void) {
  if (condSymbolLUPArray) {
    for (unsigned int i = 0; i < condSymbolLUPArray->GetCount(); i++)
      DestroyLUP(condSymbolLUPArray->Item(i));

    condSymbolLUPArray->Clear();
  }
}

bool s52plib::S52_flush_Plib() {
  if (!m_bOK) return false;

#ifdef ocpnUSE_GL
  //    OpenGL Hashmaps
  CARC_Hash::iterator ita;
  for (ita = m_CARC_hashmap.begin(); ita != m_CARC_hashmap.end(); ++ita) {
    CARC_Buffer buffer = ita->second;
    delete[] buffer.data;
  }
  m_CARC_hashmap.clear();

#endif

  DestroyLUPArray(condSymbolLUPArray);

  //      Destroy Rules
  DestroyRules(_line_sym);
  DestroyPattRules(_patt_sym);
  DestroyRules(_symb_sym);

  if (_symb_symR) DestroyRules(_symb_symR);

  //      Special case for CS
  _cond_sym->clear();
  delete (_cond_sym);

  for (unsigned int ipa = 0; ipa < pAlloc->GetCount(); ipa++) {
    void *t = pAlloc->Item(ipa);
    free(t);
  }

  pAlloc->clear();
  delete pAlloc;

  return TRUE;
}

LUPrec *s52plib::S52_LUPLookup(LUPname LUP_Name, const char *objectName,
                               S57Obj *pObj, bool bStrict) {
  LUPrec *LUP = NULL;

  LUPArrayContainer *plac = SelectLUPArrayContainer(LUP_Name);

  LUPHashIndex *hip = plac->GetArrayIndexHelper(objectName);
  int nLUPs = hip->count;
  int nStartIndex = hip->n_start;

  LUP = FindBestLUP(plac->GetLUPArray(), nStartIndex, nLUPs, pObj, bStrict);

  return LUP;
}

void s52plib::SetPLIBColorScheme(ColorScheme cs, const ChartCtx& ctx) {
  wxString SchemeName;
  switch (cs) {
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

  SetPLIBColorScheme(SchemeName, ctx);
}

void s52plib::SetPLIBColorScheme(wxString scheme, const ChartCtx& ctx) {
  wxString str_find;
  str_find = scheme;
  m_colortable_index = 0;  // default is the first color in the table

  // Of course, it also depends on the plib version...
  // plib version 3.2 calls "DAY" colr as "DAY_BRIGHT"

  if ((GetMajorVersion() == 3) && (GetMinorVersion() == 2)) {
    if (scheme.IsSameAs(_T ( "DAY" ))) str_find = _T ( "DAY_BRIGHT" );
  }
  m_colortable_index = m_chartSymbols.FindColorTable(scheme);

  m_chartSymbols.SetColorTableIndex(m_colortable_index, false, ctx);

  m_ColorScheme = scheme;
}

S52color *s52plib::getColor(const char *colorName) {
  S52color *c;
  c = m_chartSymbols.GetColor(colorName, m_colortable_index);
  return c;
}

wxColour s52plib::getwxColour(const wxString &colorName) {
  wxColor c;
  c = m_chartSymbols.GetwxColor(colorName, m_colortable_index);
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
#define APOS '\047'
#define MAXL 512

char *s52plib::_getParamVal(ObjRazRules *rzRules, char *str, char *buf, int bsz)
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
  int defval = 0;  // default value
  int len = 0;
  char *ret_ptr = str;
  char *tmp = buf;

  if (!buf) return NULL;

  buf[0] = 0;
  // parse constant parameter with concatenation operator "'"
  if (str != NULL) {
    if (*ret_ptr == APOS) {
      ret_ptr++;
      while (*ret_ptr != APOS && *ret_ptr != '\0' && len < (bsz - 1)) {
        ++len;
        *tmp++ = *ret_ptr++;
      }
      *tmp = '\0';
      ret_ptr++;  // skip "'"
      ret_ptr++;  // skip ","

      return ret_ptr;
    }

    while (*ret_ptr != ',' && *ret_ptr != ')' && *ret_ptr != '\0' &&
           len < (bsz - 1)) {
      *tmp++ = *ret_ptr++;
      ++len;
    }
    *tmp = '\0';

    ret_ptr++;  // skip ',' or ')'
  }
  if (len < 6) return ret_ptr;

  // chop string if default value present
  if (len > 6 && *(buf + 6) == '=') {
    *(buf + 6) = '\0';
    defval = 1;
  }

  value = rzRules->obj->GetAttrValueAsString(buf);
  wxCharBuffer buffer = value.ToUTF8();
  if (!buffer.data()) return ret_ptr;

  if (value.IsEmpty()) {
    if (defval)
      _getParamVal(rzRules, buf + 7, buf,
                   bsz - 7);  // default value --recursion
    else {
      return NULL;  // abort
    }
  } else {
    //    Special case for conversion of some vertical (height) attributes to
    //    feet
    if ((!strncmp(buf, "VERCLR", 6)) || (!strncmp(buf, "VERCCL", 6)) ||
        (!strncmp(buf, "VERCOP", 6)) || (!strncmp(buf, "ELEVAT", 6)) ) {
      switch (m_nDepthUnitDisplay) {
        case 0:  // feet
        case 2:  // fathoms
          double ft_val;
          value.ToDouble(&ft_val);
          ft_val = ft_val * 3 * 39.37 / 36;  // feet
          value.Printf(_T("%4.1f"), ft_val);
          break;
        default:
          break;
      }
    }

    // special case when ENC returns an index for particular attribute types
    if (!strncmp(buf, "NATSUR", 6)) {
      wxString natsur_att(_T ( "NATSUR" ));
      wxString result;
      wxString svalue = value;
      wxStringTokenizer tkz(svalue, _T ( "," ));

      int icomma = 0;
      while (tkz.HasMoreTokens()) {
        if (icomma) result += _T ( "," );

        wxString token = tkz.GetNextToken();
        long i;
        if (token.ToLong(&i)) {
          std::string snat;
          snat = m_natsur_hash[(int)i];

          wxString nat(snat.c_str());
          if (!nat.IsEmpty())
            result += nat;  // value from ENC
          else
            result += _T ( "unk" );
        } else
          result += _T ( "unk" );

        icomma++;
      }

      value = result;
    }

    wxCharBuffer buffer = value.ToUTF8();
    if (buffer.data()) {
      unsigned int len = wxMin(strlen(buffer.data()), (unsigned int)bsz - 1);
      memcpy(buf, buffer.data(), len);
      buf[len] = 0;
    } else
      *buf = 0;
  }

  return ret_ptr;
}

char *s52plib::_parseTEXT(ObjRazRules *rzRules, S52_TextC *text, char *str0) {
  char buf[MAXL];  // output string

  char *str = str0;
  if (text) {
    memset(buf, 0, 4);
    str = _getParamVal(rzRules, str, &text->hjust, MAXL);  // HJUST
    str = _getParamVal(rzRules, str, &text->vjust, MAXL);  // VJUST
    str = _getParamVal(rzRules, str, &text->space, MAXL);  // SPACE

    // CHARS
    str = _getParamVal(rzRules, str, buf, MAXL);
    text->style = buf[0];
    text->weight = buf[1];
    text->width = buf[2];
    text->bsize = atoi(buf + 3);

    str = _getParamVal(rzRules, str, buf, MAXL);
    text->xoffs = atoi(buf);
    str = _getParamVal(rzRules, str, buf, MAXL);
    text->yoffs = atoi(buf);
    str = _getParamVal(rzRules, str, buf, MAXL);
    text->pcol = getColor(buf);
    str = _getParamVal(rzRules, str, buf, MAXL);
    text->dis = atoi(buf);  // Text Group, used for "Important" text detection
  }
  return str;
}

S52_TextC *s52plib::S52_PL_parseTX(ObjRazRules *rzRules, Rules *rules,
                                   char *cmd) {
  S52_TextC *text = NULL;
  char *str = NULL;
  char val[MAXL];  // value of arg
  char strnobjnm[7] = {"NOBJNM"};
  char valn[MAXL];  // value of arg

  valn[0] = 0;
  str = (char *)rules->INSTstr;

  if (m_bShowNationalTexts &&
      NULL !=
          strstr(str,
                 "OBJNAM"))  // in case user wants the national text shown and
                             // the rule contains OBJNAM, try to get the value
  {
    _getParamVal(rzRules, strnobjnm, valn, MAXL);
    if (!strcmp(strnobjnm, valn))
      valn[0] = '\0';  // NOBJNM is not defined
    else
      valn[MAXL - 1] = '\0';  // make sure the string terminates
  }

  str = _getParamVal(rzRules, str, val, MAXL);  // get ATTRIB list

  if (NULL == str)
    return 0;  // abort this command word if mandatory param absent

  val[MAXL - 1] = '\0';  // make sure the string terminates

  text = new S52_TextC;
  str = _parseTEXT(rzRules, text, str);
  if (NULL != text) {
    if (valn[0] != '\0') {
      text->frmtd = wxString(valn, wxConvUTF8);
      text->bnat = true;
    } else {
      text->frmtd = wxString(val, wxConvUTF8);
      text->bnat = false;
    }
  }

  //  We check to see if the formatted text has any "special" characters
  wxCharBuffer buf = text->frmtd.ToUTF8();

  unsigned int n = text->frmtd.Length();
  for (unsigned int i = 0; i < n; ++i) {
    unsigned char c = buf.data()[i];
    if (c > 127) {
      text->bspecial_char = true;
      break;
    }
  }

  return text;
}

S52_TextC *s52plib::S52_PL_parseTE(ObjRazRules *rzRules, Rules *rules,
                                   char *cmd)
// same as S52_PL_parseTX put parse 'C' format first
{
  char arg[MAXL];  // ATTRIB list
  char fmt[MAXL];  // FORMAT
  char buf[MAXL];  // output string
  char *b = buf;
  char *parg = arg;
  char *pf = fmt;
  S52_TextC *text = NULL;

  char *str = (char *)rules->INSTstr;

  if (str && *str) {
    str = _getParamVal(rzRules, str, fmt, MAXL);  // get FORMAT

    str = _getParamVal(rzRules, str, arg, MAXL);  // get ATTRIB list
    if (NULL == str)
      return 0;  // abort this command word if mandatory param absent

    //*b = *pf;
    while (*pf != '\0') {
      // begin a convertion specification
      if (*pf == '%') {
        char val[MAXL];           // value of arg
        char tmp[MAXL] = {'\0'};  // temporary format string
        char *t = tmp;
        int cc = 0;  // 1 == Conversion Character found
        //*t = *pf;

        // get value for this attribute
        parg = _getParamVal(rzRules, parg, val, MAXL);
        if (NULL == parg) return 0;  // abort

        if (0 == strcmp(val, "2147483641")) return 0;

        *t = *pf;  // stuff the '%'

        // scan for end at convertion character
        do {
          *++t = *++pf;  // fill conver spec

          switch (*pf) {
            case 'c':
            case 's':
              b += sprintf(b, tmp, val);
              cc = 1;
              break;
            case 'f':
              b += sprintf(b, tmp, atof(val));
              cc = 1;
              break;
            case 'd':
            case 'i':
              b += sprintf(b, tmp, atoi(val));
              cc = 1;
              break;
          }
        } while (!cc);
        pf++;  // skip conv. char

      } else
        *b++ = *pf++;
    }

    *b = 0;
    text = new S52_TextC;
    str = _parseTEXT(rzRules, text, str);
    if (NULL != text) text->frmtd = wxString(buf, wxConvUTF8);

    //  We check to see if the formatted text has any "special" characters
    wxCharBuffer buf = text->frmtd.ToUTF8();

    unsigned int n = text->frmtd.Length();
    for (unsigned int i = 0; i < n; ++i) {
      unsigned char c = buf.data()[i];
      if (c > 127) {
        text->bspecial_char = true;
        break;
      }
    }
  }

  return text;
}

static void rotate(wxRect *r, VPointCompat const *vp) {
  float cx = vp->pix_width / 2.;
  float cy = vp->pix_height / 2.;
  float c = cosf(vp->rotation);
  float s = sinf(vp->rotation);
  float x = r->GetX() - cx;
  float y = r->GetY() - cy;
  r->SetX(x * c - y * s + cx);
  r->SetY(x * s + y * c + cy);
}

bool s52plib::RenderText(wxDC *pdc, S52_TextC *ptext, int x, int y,
                         wxRect *pRectDrawn, S57Obj *pobj, bool bCheckOverlap) {
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

  //FIXME(plib)
  double sfactor = 1; //vp_plib.ref_scale / vp_plib.chart_scale;
  double scale_factor = wxMax((sfactor) / 4., 1.);

  //  Place an upper bound on the scaled text size
  scale_factor = wxMin(scale_factor, 4);

  scale_factor /= m_dipfactor;
  scale_factor *= m_TextScaleFactor;

  // Has there been a change in scale factor by UI?
  if (scale_factor != m_FinalTextScaleFactor){
    ptext->texobj = 0;    // This will leak, but only a little
    m_FinalTextScaleFactor = scale_factor;

    for (unsigned int i = 0; i < TXF_CACHE; i++) {
     s_txf[i].key = 0;
     s_txf[i].cache = 0;
    }
  }


  if (!pdc)  // OpenGL
  {
#ifdef ocpnUSE_GL

    bool b_force_no_texfont = false;
    if (ptext->bspecial_char)
      b_force_no_texfont = true;

     //Fixme (dave)
    // We also do this the hard way for rotation of strings.  Very slow.
//#ifdef __OCPN__ANDROID__
    if (fabs(vp_plib.rotation) > .01)
      b_force_no_texfont = true;
//#endif

    if (b_force_no_texfont) {
      if (!ptext->texobj){  // is texture ready?

        int old_size = ptext->pFont->GetPointSize();
        int new_size = old_size * scale_factor / m_ContentScaleFactor;

        scaled_font = FindOrCreateFont_PlugIn(
          new_size, ptext->pFont->GetFamily(), ptext->pFont->GetStyle(),
          ptext->pFont->GetWeight(), false, ptext->pFont->GetFaceName());
        wxScreenDC sdc;
        sdc.GetTextExtent(ptext->frmtd, &w_scaled, &h_scaled, &descent, &exlead,
                        scaled_font);  // measure the text

        // We cannot get the font ascent value to remove the interline spacing
        // from the font "height". So we have to estimate based on conventional
        // Arial metrics
        ptext->rendered_char_height = (h_scaled - descent) * 8 / 10 * m_dipfactor;

        ptext->text_width = w_scaled * m_dipfactor;
        ptext->text_height = h_scaled * m_dipfactor;

        /* make power of 2 */
        int tex_w, tex_h;
        for (tex_w = 1; tex_w < w_scaled; tex_w *= 2);
        for (tex_h = 1; tex_h < h_scaled; tex_h *= 2);

        wxMemoryDC mdc;
        wxBitmap bmp(tex_w, tex_h);
        mdc.SelectObject(bmp);
        mdc.SetFont(*(scaled_font));

        if (mdc.IsOk()) {
          //  Render the text as white on black, so that underlying
          //  anti-aliasing of wxDC text rendering can be extracted and
          //  converted to alpha-channel values.

          mdc.SetBackground(wxBrush(wxColour(0, 0, 0)));
          mdc.SetBackgroundMode(wxTRANSPARENT);

          mdc.SetTextForeground(wxColour(255, 255, 255));

          mdc.Clear();

          mdc.DrawText(ptext->frmtd, 0, 0);
          mdc.SelectObject(wxNullBitmap);

          wxImage image = bmp.ConvertToImage();
          int ws = image.GetWidth(), hs = image.GetHeight();

          ptext->RGBA_width = ws;
          ptext->RGBA_height = hs;
          unsigned char *pRGBA = (unsigned char *)malloc(4 * ws * hs);

          unsigned char *d = image.GetData();
          unsigned char *pdest = pRGBA;
          S52color *ccolor = ptext->pcol;

          if (d) {
            for (int y = 0; y < hs; y++)
              for (int x = 0; x < ws; x++) {
                unsigned char r, g, b;
                int off = (y * ws + x);

                r = d[off * 3 + 0];
                g = d[off * 3 + 1];
                b = d[off * 3 + 2];

                pdest[off * 4 + 0] = ccolor->R;
                pdest[off * 4 + 1] = ccolor->G;
                pdest[off * 4 + 2] = ccolor->B;

                int alpha = (r + g + b) / 3;
                pdest[off * 4 + 3] = (unsigned char)(alpha & 0xff);
              }
          }

          int draw_width = ptext->RGBA_width;
          int draw_height = ptext->RGBA_height;

          glEnable(GL_TEXTURE_2D);

          GLuint texobj;
          glGenTextures(1, &texobj);

          glBindTexture(GL_TEXTURE_2D, texobj);

          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                          GL_NEAREST /*GL_LINEAR*/);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, draw_width, draw_height, 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, pRGBA);

          free(pRGBA);

          ptext->texobj = texobj;

        }  // mdc OK
      }    // Building texobj

      //    Render the texture
      if (ptext->texobj) {
        int yadjust = 0;
        int xadjust = 0;

        //  Adjust the y position to account for the convention that S52 text is
        //  drawn with the lower left corner at the specified point, instead of
        //  the wx convention using upper left corner. Also, allow for full text
        //  height in the bitmap/texture, not the estimated "rendered" height.

        yadjust = -ptext->rendered_char_height * 10 / 8;

        //  Add in the offsets, specified in units of nominal font height
        yadjust += ptext->yoffs * (ptext->rendered_char_height);
        //  X offset specified in units of average char width
        xadjust += ptext->xoffs * ptext->avgCharWidth;

        // adjust for text justification
        int w = ptext->text_width;
        switch (ptext->hjust) {
          case '1':  // centered
            xadjust -= w / 2;
            break;
          case '2':  // right
            xadjust -= w;
            break;
          case '3':  // left (default)
          default:
            break;
        }

        switch (ptext->vjust) {
          case '3':  // top
            yadjust += ptext->rendered_char_height;
            break;
          case '2':  // centered
            yadjust += ptext->rendered_char_height / 2;
            break;
          case '1':  // bottom (default)
          default:
            break;
        }

        int xp = x;
        int yp = y;

        if (fabs(vp_plib.rotation) > 0.01) {
          float c = cosf(-vp_plib.rotation);
          float s = sinf(-vp_plib.rotation);
          float x = xadjust;
          float y = yadjust;
          xp += x * c - y * s;
          yp += x * s + y * c;

        } else {
          xp += xadjust;
          yp += yadjust;
        }

        pRectDrawn->SetX(xp);
        pRectDrawn->SetY(yp);
        pRectDrawn->SetWidth(ptext->text_width);
        pRectDrawn->SetHeight(ptext->text_height);

        if (bCheckOverlap) {
          if (CheckTextRectList(*pRectDrawn, ptext)) bdraw = false;
        }

        if (bdraw) {
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
#else
          glEnable(GL_BLEND);
          glEnable(GL_TEXTURE_2D);

          float uv[8];
          float coords[8];

          // Note swizzle of points to allow TRIANGLE_STRIP drawing
          // normal uv
          uv[0] = 0;
          uv[1] = 0;
          uv[2] = 1;
          uv[3] = 0;
          uv[6] = 1;
          uv[7] = 1;
          uv[4] = 0;
          uv[5] = 1;

          // pixels
          coords[0] = 0;
          coords[1] = 0;
          coords[2] = ptext->RGBA_width;
          coords[3] = 0;
          coords[6] = ptext->RGBA_width;
          coords[7] = ptext->RGBA_height;
          coords[4] = 0;
          coords[5] = ptext->RGBA_height;

          glUseProgram(S52texture_2D_shader_program);

          // Get pointers to the attributes in the program.
          GLint mPosAttrib =
              glGetAttribLocation(S52texture_2D_shader_program, "position");
          GLint mUvAttrib =
              glGetAttribLocation(S52texture_2D_shader_program, "aUV");

          // Select the active texture unit.
          glActiveTexture(GL_TEXTURE0);

          // Bind our texture to the texturing target.
          glBindTexture(GL_TEXTURE_2D, ptext->texobj);

          // Set up the texture sampler to texture unit 0
          GLint texUni =
              glGetUniformLocation(S52texture_2D_shader_program, "uTex");
          glUniform1i(texUni, 0);

          // Disable VBO's (vertex buffer objects) for attributes.
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

          // Set the attribute mPosAttrib with the vertices in the screen
          // coordinates...
          glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords);
          // ... and enable it.
          glEnableVertexAttribArray(mPosAttrib);

          // Set the attribute mUvAttrib with the vertices in the GL
          // coordinates...
          glVertexAttribPointer(mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, uv);
          // ... and enable it.
          glEnableVertexAttribArray(mUvAttrib);

          // Rotate
          mat4x4 I, Q;
          mat4x4_identity(I);

          mat4x4_translate_in_place(I, x, y, 0);
          mat4x4_rotate_Z(Q, I, -vp_plib.rotation);
          mat4x4_translate_in_place(Q, xadjust, yadjust, 0);

          GLint matloc = glGetUniformLocation(S52texture_2D_shader_program,
                                              "TransformMatrix");
          glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);

          // Perform the actual drawing.
          glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

          // Restore the per-object transform to Identity Matrix
          mat4x4 IM;
          mat4x4_identity(IM);
          GLint matlocf = glGetUniformLocation(S52texture_2D_shader_program,
                                               "TransformMatrix");
          glUniformMatrix4fv(matlocf, 1, GL_FALSE, (const GLfloat *)IM);

          glDisableVertexAttribArray(mPosAttrib);
          glDisableVertexAttribArray(mUvAttrib);

          glDisable(GL_TEXTURE_2D);
          glDisable(GL_BLEND);
#endif

        }  // bdraw
      }

      bdraw = true;
    }

    else {  // render using cached texture glyphs
      // rebuild font if needed
      TexFont *f_cache = 0;
      unsigned int i;

      for (i = 0; i < TXF_CACHE; i++) {
        if (s_txf[i].key == ptext->pFont) {
          f_cache = s_txf[i].cache;
          break;
        }
        if (s_txf[i].key == 0) {
          break;
        }
      }
      if (i == TXF_CACHE) {
        i = rand() & (TXF_CACHE - 1);
      }
      if (f_cache == 0) {
        s_txf[i].key = ptext->pFont;
        if(s_txf[i].cache)
          delete s_txf[i].cache;
        s_txf[i].cache = new TexFont();
        s_txf[i].cache->SetContentScaleFactor(m_ContentScaleFactor);

        f_cache = s_txf[i].cache;
        f_cache->Build(*ptext->pFont, m_TextScaleFactor, m_dipfactor);

        int wac;
        f_cache->GetTextExtent(_T("M"), &wac, 0);
        ptext->avgCharWidth = wac * m_dipfactor;
      }

      int w, h;
      f_cache->GetTextExtent(ptext->frmtd, &w, &h);
      h *= m_dipfactor;

      // We don't store descent/ascent info for font texture cache
      // So we have to estimate based on conventional Arial metrics
      ptext->rendered_char_height = h * 65 / 100;

      //  Adjust the y position to account for the convention that S52 text is
      //  drawn with the lower left corner at the specified point, instead of
      //  the wx convention using upper right corner
      int yadjust = 0;
      int xadjust = 0;

      yadjust = -ptext->rendered_char_height;

      //  Add in the offsets, specified in units of nominal font height
      yadjust += ptext->yoffs * (ptext->rendered_char_height);
      //  X offset specified in units of average char width
      xadjust += ptext->xoffs * ptext->avgCharWidth;

      // adjust for text justification
      switch (ptext->hjust) {
        case '1':  // centered
          xadjust -= w / 2;
          break;
        case '2':  // right
          xadjust -= w;
          break;
        case '3':  // left (default)
        default:
          break;
      }

      switch (ptext->vjust) {
        case '3':  // top
          yadjust += ptext->rendered_char_height;
          break;
        case '2':  // centered
          yadjust += ptext->rendered_char_height / 2;
          break;
        case '1':  // bottom (default)
        default:
          break;
      }

      int xp = x;
      int yp = y;

      if (fabs(vp_plib.rotation) > 0.01) {
        float c = cosf(-vp_plib.rotation);
        float s = sinf(-vp_plib.rotation);
        float x = xadjust;
        float y = yadjust;
        xadjust = x * c - y * s;
        yadjust = x * s + y * c;
      }

      xp += xadjust;
      yp += yadjust;

      pRectDrawn->SetX(xp);
      pRectDrawn->SetY(yp);
      pRectDrawn->SetWidth(w);
      pRectDrawn->SetHeight(h);

      if (bCheckOverlap) {
        if (fabs(vp_plib.rotation) > .01) {
          rotate(pRectDrawn, &vp_plib);
        }
        if (CheckTextRectList(*pRectDrawn, ptext)) bdraw = false;
      }

      if (bdraw) {
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
#else
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        wxColour wcolor = GetFontColour_PlugIn(_("ChartTexts"));
        f_cache->SetColor(wcolor);

        /* undo previous rotation to make text level */
        // glRotatef(vp->rotation*180/PI, 0, 0, -1);

        f_cache->RenderString(ptext->frmtd, xp, yp);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
#endif
      }
    }

#endif
  } else {                            // Not OpenGL
    wxFont oldfont = pdc->GetFont();  // save current font

    if (scale_factor > 1) {
      wxFont *pf = ptext->pFont;
      int old_size = pf->GetPointSize();
      int new_size = old_size * scale_factor;
      wxFont *scaled_font =
          FindOrCreateFont_PlugIn(new_size, pf->GetFamily(), pf->GetStyle(),
                                  pf->GetWeight(), false, pf->GetFaceName());
      pdc->SetFont(*scaled_font);
    } else {
      pdc->SetFont(*(ptext->pFont));
    }

    wxCoord w, h, descent, exlead;
    pdc->GetTextExtent(ptext->frmtd, &w, &h, &descent,
                       &exlead);  // measure the text

    // We cannot get the font ascent value to remove the interline spacing.
    // So we have to estimate based on conventional Arial metrics
    int rendered_text_height = (h - descent) * 8 / 10;

    //  Adjust the y position to account for the convention that S52 text is
    //  drawn with the lower left corner at the specified point, instead of the
    //  wx convention using upper right corner
    int yadjust = 0;
    int xadjust = 0;

    yadjust = -(rendered_text_height);

    //  Add in the offsets, specified in units of nominal font height
    yadjust += ptext->yoffs * (rendered_text_height);

    //  X offset specified in units of average char width
    xadjust += ptext->xoffs * ptext->avgCharWidth * m_dipfactor;

    // adjust for text justification
    switch (ptext->hjust) {
      case '1':  // centered
        xadjust -= w / 2;
        break;
      case '2':  // right
        xadjust -= w;
        break;
      case '3':  // left (default)
      default:
        break;
    }

    switch (ptext->vjust) {
      case '3':  // top
        yadjust += rendered_text_height;
        break;
      case '2':  // centered
        yadjust += rendered_text_height / 2;
        break;
      case '1':  // bottom (default)
      default:
        break;
    }

    int xp = x;
    int yp = y;

    if (fabs(vp_plib.rotation) > 0.01) {
      float cx = vp_plib.pix_width / 2.;
      float cy = vp_plib.pix_height / 2.;
      float c = cosf(vp_plib.rotation);
      float s = sinf(vp_plib.rotation);
      float x = xp - cx;
      float y = yp - cy;
      xp = x * c - y * s + cx + vp_plib.rv_rect.x;
      yp = x * s + y * c + cy + vp_plib.rv_rect.y;
    }

    xp += xadjust;
    yp += yadjust;

    pRectDrawn->SetX(xp);
    pRectDrawn->SetY(yp);
    pRectDrawn->SetWidth(w);
    pRectDrawn->SetHeight(h);

    if (bCheckOverlap) {
      if (CheckTextRectList(*pRectDrawn, ptext)) bdraw = false;
    }

    if (bdraw) {
      wxColour wcolor = GetFontColour_PlugIn(_("ChartTexts"));

      // If the user has not changed the color from BLACK, then use the color
      // specified in the S52 LUP
      if (wcolor == *wxBLACK)
        wcolor = wxColour(ptext->pcol->R, ptext->pcol->G, ptext->pcol->B);
      pdc->SetTextForeground(wcolor);

      pdc->DrawText(ptext->frmtd, xp, yp);
    }

    pdc->SetFont(oldfont);  // restore last font
  }
  return bdraw;

#ifdef FIXIT
#undef FIXIT
#define DrawText DrawTextA
#endif
}

//    Return true if test_rect overlaps any rect in the current text rectangle
//    list, except itself
bool s52plib::CheckTextRectList(const wxRect &test_rect, S52_TextC *ptext) {
  //    Iterate over the current object list, looking at rText

  for (TextObjList::Node *node = m_textObjList.GetFirst(); node;
       node = node->GetNext()) {
    wxRect *pcurrent_rect = &(node->GetData()->rText);

    if (pcurrent_rect->Intersects(test_rect)) {
      if (node->GetData() != ptext) return true;
    }
  }
  return false;
}

bool s52plib::TextRenderCheck(ObjRazRules *rzRules) {
  if (!m_bShowS57Text) return false;

  if (rzRules->obj->bIsAton) {
    if (!strncmp(rzRules->obj->FeatureName, "LIGHTS", 6)) {
      if (!m_bShowLdisText) return false;
    } else {
      if (!m_bShowAtonText) return false;
    }
  }

  // Declutter LIGHTS descriptions
  if ((rzRules->obj->bIsAton) &&
      (!strncmp(rzRules->obj->FeatureName, "LIGHTS", 6))) {
    if (lastLightLat == rzRules->obj->m_lat &&
        lastLightLon == rzRules->obj->m_lon) {
      return false;  // only render text for the first object at this lat/lon
    } else {
      lastLightLat = rzRules->obj->m_lat;
      lastLightLon = rzRules->obj->m_lon;
    }
  }

  //    An optimization for CM93 charts.
  //    Don't show the text associated with some objects, since CM93 database
  //    includes _texto objects aplenty
  // FIXME VBO Dave
  // chart type is available in member m_chart_context
  //  Do not use auxParm3
  if (((int)rzRules->obj->auxParm3 == (int)S52_ChartTypeEnum::S52_CHART_TYPE_CM93) ||
      ((int)rzRules->obj->auxParm3 == (int)S52_ChartTypeEnum::S52_CHART_TYPE_CM93COMP)) {
    if (!strncmp(rzRules->obj->FeatureName, "BUAARE", 6))
      return false;
    else if (!strncmp(rzRules->obj->FeatureName, "SEAARE", 6))
      return false;
    else if (!strncmp(rzRules->obj->FeatureName, "LNDRGN", 6))
      return false;
    else if (!strncmp(rzRules->obj->FeatureName, "LNDARE", 6))
      return false;
  }

  return true;
}

int s52plib::RenderT_All(ObjRazRules *rzRules, Rules *rules,
                         bool bTX) {
  if (!TextRenderCheck(rzRules)) return 0;

  S52_TextC *text = NULL;
  bool b_free_text = false;

  //  The first Ftext object is cached in the S57Obj.
  //  If not present, create it on demand
  if (!rzRules->obj->bFText_Added) {
    if (bTX)
      text = S52_PL_parseTX(rzRules, rules, NULL);
    else
      text = S52_PL_parseTE(rzRules, rules, NULL);

    if (text) {
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
    if (rules->n_sequence == rzRules->obj->FText->rul_seq_creator)
      text = rzRules->obj->FText;
    else {
      if (bTX)
        text = S52_PL_parseTX(rzRules, rules, NULL);
      else
        text = S52_PL_parseTE(rzRules, rules, NULL);

      b_free_text = true;
    }
  }

  if (text) {
    if (m_bShowS57ImportantTextOnly && (text->dis >= 20)) {
      if (b_free_text) delete text;
      return 0;
    }

    //    Establish a font
    if (!text->pFont) {
      // Process the font specifications from the LUP symbolization rule
      int spec_weight = text->weight - 0x30;
      wxFontWeight fontweight;
      if (spec_weight < 5)
        fontweight = wxFONTWEIGHT_LIGHT;
      else {
        if (spec_weight == 5)
          fontweight = wxFONTWEIGHT_NORMAL;
        else
          fontweight = wxFONTWEIGHT_BOLD;
      }

      wxFont *specFont = FindOrCreateFont_PlugIn(
          text->bsize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, fontweight);

      // Get the width of a single average character in the spec font
      wxScreenDC dc;
      dc.SetFont(*specFont);
      int width;
      dc.GetTextExtent(_T("X"), &width, NULL, NULL, NULL, specFont);
      text->avgCharWidth = width;

      int spec_weight_t = text->weight - 0x30;
      wxFontWeight fontweight_t;
      if (spec_weight_t < 5)
        fontweight_t = wxFONTWEIGHT_LIGHT;
      else if (spec_weight_t == 5)
        fontweight_t = wxFONTWEIGHT_NORMAL;
      else
        fontweight_t = wxFONTWEIGHT_BOLD;

      // Get the configured "ChartText" font, which may be the default system
      // font or another font that has been customized by the user.
      // The user may have customized both the font face and the font size.
      // We will use the user-configured font size as the base size for the
      // text rendering, and adjust the size based on the S52 text size
      // specification.
      wxFont *templateFont = GetOCPNScaledFont_PlugIn(_("ChartTexts"));
      int default_size = templateFont->GetPointSize();

      // In S52, the "body size" refers to the base size of the text characters
      // in display units (typically pixels). This is part of the text style
      // parameters that control how chart text elements are rendered.
      // The S52 standard (IHO S-52 Presentation Library) uses this parameter
      // as part of the text placement and styling rules for chart text elements.
      // The body size is used as a reference unit for Text scaling.
      //
      // NOAA ENC files requests font size up to 20 points, which looks very
      // disproportioned. Let's scale those sizes down to more reasonable
      // values.
      int fontSize = text->bsize;

      const int maxInputSize = 20; // Largest expected input size.
      const int normalizedRange = 4;
      // Clamp the input size.
      fontSize = wxMin(maxInputSize, fontSize);

      // Normalize the font size by mapping from [0, maxInputSize] to
      // [0, normalizedRange] in a linear, proportional way.
      // The transformation is inherently monotonic - larger input values will
      // always produce larger output values before clamping.
      fontSize = (static_cast<float>(fontSize) / static_cast<float>(maxInputSize)) * normalizedRange;
      // The normalized value is added to the default font size to get the final
      // font size.
      // This respects user preferences while still adapting to chart specifications.
      fontSize += default_size;

      // In no case should font size be less than 10, since it becomes
      // unreadable
      fontSize = wxMax(10, fontSize);

      text->pFont = FindOrCreateFont_PlugIn(
            fontSize, templateFont->GetFamily(), templateFont->GetStyle(), fontweight_t,
            false, templateFont->GetFaceName());
  }

    //  Render text at declared x/y of object
    wxPoint r;
    GetPointPixSingle(rzRules, rzRules->obj->y, rzRules->obj->x, &r);

    wxRect rect;
    bool bwas_drawn = RenderText(m_pdc, text, r.x, r.y, &rect, rzRules->obj,
                                 m_bDeClutterText);

    //  If this is an un-cached text render, it probably means that a single
    //  object has two or more text renders in its rule set.  RDOCAL is one
    //  example.  There are others We need to cache only the first text
    //  structure, but should update the render rectangle to reflect all texts
    //  rendered for this object,  in order to process the declutter logic.
    bool b_dupok = false;
    if (b_free_text) {
      delete text;

      if (!bwas_drawn) {
        return 1;
      } else {  // object was drawn
        text = rzRules->obj->FText;

        wxRect r0 = text->rText;
        r0 = r0.Union(rect);
        text->rText = r0;

        b_dupok = true;  // OK to add a duplicate text structure to the
                         // declutter list, just for this case.
      }
    } else
      text->rText = rect;

    //      If this text was actually drawn, add a pointer to its rect to the
    //      de-clutter list if it doesn't already exist
    if (m_bDeClutterText) {
      if (bwas_drawn) {
        bool b_found = false;
        for (TextObjList::Node *node = m_textObjList.GetFirst(); node;
             node = node->GetNext()) {
          S52_TextC *oc = node->GetData();

          if (oc == text) {
            if (!b_dupok) b_found = true;
            break;
          }
        }
        if (!b_found) m_textObjList.Append(text);
      }
    }

    //  Update the object Bounding box
    //  so that subsequent drawing operations will redraw the item fully
    //  and so that cursor hit testing includes both the text and the object

    //            if ( rzRules->obj->Primitive_type == GEO_POINT )
    {
      double latmin, lonmin, latmax, lonmax;

      GetPixPointSingleNoRotate(rect.GetX(), rect.GetY() + rect.GetHeight(),
                                &latmin, &lonmin);
      GetPixPointSingleNoRotate(rect.GetX() + rect.GetWidth(), rect.GetY(),
                                &latmax, &lonmax);
      LLBBox bbtext;
      bbtext.Set(latmin, lonmin, latmax, lonmax);

      rzRules->obj->BBObj.Expand(bbtext);
    }
  }

  return 1;
}

// Text
int s52plib::RenderTX(ObjRazRules *rzRules, Rules *rules) {
  return RenderT_All(rzRules, rules, true);
}

// Text formatted
int s52plib::RenderTE(ObjRazRules *rzRules, Rules *rules) {
  return RenderT_All(rzRules, rules, false);
}

bool s52plib::RenderHPGL(ObjRazRules *rzRules, Rule *prule, wxPoint &r,
                         float rot_angle, double uScale) {
  float fsf = 100 / canvas_pix_per_mm;

  float xscale = 1.0;

  if ((!strncmp(rzRules->obj->FeatureName, "TSSLPT", 6)) ||
      (!strncmp(rzRules->obj->FeatureName, "DWRTPT", 6)) ||
      (!strncmp(rzRules->obj->FeatureName, "TWRTPT", 6)) ||
      (!strncmp(rzRules->obj->FeatureName, "RCTLPT", 6))) {
    // assume the symbol length
    float sym_length = 30;
    float scaled_length = sym_length / vp_plib.view_scale_ppm;
    float target_length = 800;

    xscale = target_length / scaled_length;
    xscale = wxMin(xscale, 1.0);
    xscale = wxMax(.4, xscale);

    // printf("scaled length: %g   xscale: %g\n", scaled_length, xscale);

    fsf *= xscale;
  }

  xscale *= uScale;

  //  Special case for GEO_AREA objects with centred symbols
  if (rzRules->obj->Primitive_type == GEO_AREA) {
    wxPoint r;
    GetPointPixSingle(rzRules, rzRules->obj->y, rzRules->obj->x, &r);

    double latdraw, londraw;  // position of the drawn symbol with pivot applied
    GetPixPointSingleNoRotate(
        r.x + ((prule->pos.symb.pivot_x.SYCL - prule->pos.symb.bnbox_x.SBXC) /
               fsf),
        r.y + ((prule->pos.symb.pivot_y.SYRW - prule->pos.symb.bnbox_y.SBXR) /
               fsf),
        &latdraw, &londraw);

    if (!rzRules->obj->BBObj.Contains(
            latdraw,
            londraw))  // Symbol reference point is outside base area object
      return 1;
  }

  double render_angle = rot_angle;

  //  Very special case for ATON flare lights at 135 degrees, the standard
  //  render angle. We don't want them to rotate with the viewport.
  if (rzRules->obj->bIsAton &&
      (!strncmp(rzRules->obj->FeatureName, "LIGHTS", 6))){

#ifdef __OCPN__ANDROID__
      //  Due to popular request, we make the flare lights a little bit
      //  smaller than S52 specifications
      xscale = xscale * 5. / 7.;
#endif

      if( fabs(rot_angle - 135.0) < 1.)
        render_angle -= vp_plib.rotation * 180. / PI;
  }

  int width = prule->pos.symb.bnbox_x.SBXC + prule->pos.symb.bnbox_w.SYHL;
  width *= 4;  // Grow the drawing bitmap to allow for rotation of symbols with
               // highly offset pivot points
  width = (int)(width / fsf);

  int height = prule->pos.symb.bnbox_y.SBXR + prule->pos.symb.bnbox_h.SYVL;
  height *= 4;
  height = (int)(height / fsf);

  int origin_x = prule->pos.symb.bnbox_x.SBXC;
  int origin_y = prule->pos.symb.bnbox_y.SBXR;
  wxPoint origin(origin_x, origin_y);

  int pivot_x = prule->pos.symb.pivot_x.SYCL;
  int pivot_y = prule->pos.symb.pivot_y.SYRW;
  wxPoint pivot(pivot_x, pivot_y);

  char *str = prule->vector.LVCT;
  char *col = prule->colRef.LCRF;
  wxPoint r0((int)(pivot_x / fsf), (int)(pivot_y / fsf));

  HPGL->SetVP(&vp_plib);

  if (!m_pdc) {  // OpenGL Mode, do a direct render
    HPGL->SetTargetOpenGl();
    HPGL->Render(str, col, r, pivot, origin, xscale, render_angle, true);

    //  Update the object Bounding box
    //  so that subsequent drawing operations will redraw the item fully

    int r_width = prule->pos.symb.bnbox_w.SYHL;
    r_width = (int)(r_width / fsf);
    int r_height = prule->pos.symb.bnbox_h.SYVL;
    r_height = (int)(r_height / fsf);
    int maxDim = wxMax(r_height, r_width);

    double latmin, lonmin, latmax, lonmax;
    GetPixPointSingleNoRotate(r.x - maxDim, r.y + maxDim, &latmin, &lonmin);
    GetPixPointSingleNoRotate(r.x + maxDim, r.y - maxDim, &latmax, &lonmax);
    LLBBox symbox;
    symbox.Set(latmin, lonmin, latmax, lonmax);

    rzRules->obj->BBObj.Expand(symbox);

  } else {
#if ((defined(__WXGTK__) || defined(__WXMAC__)) && !wxCHECK_VERSION(2, 9, 4))
    wxBitmap *pbm = new wxBitmap(width, height);
#else
    wxBitmap *pbm = new wxBitmap(width, height, 32);
#if !wxCHECK_VERSION(2, 9, 4)
    pbm->UseAlpha();
#endif
#endif
    wxMemoryDC mdc(*pbm);
    if (!mdc.IsOk()) {
      wxString msg;
      msg.Printf(_T("RenderHPGL: width %d  height %d"), width, height);
      wxLogMessage(msg);
      return false;
    }

#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC gdc(mdc);
    HPGL->SetTargetGCDC(&gdc);
#else
    wxMemoryDC &gdc(mdc);
    HPGL->SetTargetDC(&gdc);
#endif
    HPGL->Render(str, col, r0, pivot, origin, xscale, (double)rot_angle, true);

    int bm_width = (gdc.MaxX() - gdc.MinX()) + 4;
    int bm_height = (gdc.MaxY() - gdc.MinY()) + 4;
    int bm_orgx = wxMax(0, gdc.MinX() - 2);
    int bm_orgy = wxMax(0, gdc.MinY() - 2);
    int screenOriginX = r.x + (bm_orgx - (int)(pivot_x / fsf));
    int screenOriginY = r.y + (bm_orgy - (int)(pivot_y / fsf));

    //      Pre-clip the sub-bitmap to avoid assert errors
    if ((bm_height + bm_orgy) > height) bm_height = height - bm_orgy;
    if ((bm_width + bm_orgx) > width) bm_width = width - bm_orgx;

    mdc.SelectObject(wxNullBitmap);

    //  Grab a copy of the existing screen DC rectangle
    wxBitmap targetBm(bm_width, bm_height, 24);
    wxMemoryDC targetDc(targetBm);
    if (!targetDc.IsOk()) return false;
    targetDc.Blit(0, 0, bm_width, bm_height, m_pdc, screenOriginX,
                  screenOriginY);

#if wxUSE_GRAPHICS_CONTEXT /*&& (( defined(__WXGTK__) || defined(__WXMAC__) ) \
                              && !wxCHECK_VERSION(2,9,4))*/
    //  Re-render onto the screen-grab copy, since wxDC::DrawBitmap() for alpha
    //  channel bitmaps is broken somehow in wxGCDC
    wxGCDC targetGcdc(targetDc);
    r0 -= wxPoint(bm_orgx, bm_orgy);
    HPGL->SetTargetGCDC(&targetGcdc);
    HPGL->Render(str, col, r0, pivot, origin, xscale, (double)rot_angle, true);
#else
    //  We can use the bitmap already rendered
    //  Get smallest containing bitmap
    wxBitmap *sbm = new wxBitmap(
        pbm->GetSubBitmap(wxRect(bm_orgx, bm_orgy, bm_width, bm_height)));

    //  render the symbol graphics onto the screen-grab copy, with
    //  transparency...
    targetDc.DrawBitmap(*sbm, 0, 0);
    delete sbm;
#endif

    //  Render the final bitmap onto the screen DC
    m_pdc->Blit(screenOriginX, screenOriginY, bm_width, bm_height, &targetDc, 0,
                0);

    // Clean up
    delete pbm;
    targetDc.SelectObject(wxNullBitmap);

    //  Update the object Bounding box
    //  so that subsequent drawing operations will redraw the item fully

    double latmin, lonmin, latmax, lonmax;
    GetPixPointSingleNoRotate(r.x + prule->parm2,
                              r.y + prule->parm3 + bm_height, &latmin, &lonmin);
    GetPixPointSingleNoRotate(r.x + prule->parm2 + bm_width, r.y + prule->parm3,
                              &latmax, &lonmax);
    LLBBox symbox;
    symbox.Set(latmin, lonmin, latmax, lonmax);

    rzRules->obj->BBObj.Expand(symbox);
  }

  return true;
}

//-----------------------------------------------------------------------------------------
//      Instantiate a Symbol or Pattern stored as XBM ascii in a rule
//      Producing a wxImage
//-----------------------------------------------------------------------------------------
wxImage s52plib::RuleXBMToImage(Rule *prule) {
  //      Decode the color definitions
  wxArrayPtrVoid *pColorArray = new wxArrayPtrVoid;

  int i = 0;
  char *cstr = prule->colRef.SCRF;

  char colname[6];
  int nl = strlen(cstr);

  while (i < nl) {
    i++;

    strncpy(colname, &cstr[i], 5);
    colname[5] = 0;
    S52color *pColor = getColor(colname);

    pColorArray->Add((void *)pColor);

    i += 5;
  }

  //      Get geometry
  int width = prule->pos.line.bnbox_w.SYHL;
  int height = prule->pos.line.bnbox_h.SYVL;

  wxString gstr(*prule->bitmap.SBTM);  // the bit array

  wxImage Image(width, height);

  for (int iy = 0; iy < height; iy++) {
    wxString thisrow = gstr(iy * width, width);  // extract a row

    for (int ix = 0; ix < width; ix++) {
      int cref = (int)(thisrow[ix] - 'A');  // make an index
      if (cref >= 0) {
        S52color *pthisbitcolor = (S52color *)(pColorArray->Item(cref));
        Image.SetRGB(ix, iy, pthisbitcolor->R, pthisbitcolor->G,
                     pthisbitcolor->B);
      } else {
        Image.SetRGB(ix, iy, m_unused_color.R, m_unused_color.G,
                     m_unused_color.B);
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
bool s52plib::RenderRasterSymbol(ObjRazRules *rzRules, Rule *prule, wxPoint &r,
                                 float rot_angle) {

  double scale_factor = 1.0;

  scale_factor *= m_ChartScaleFactorExp;

    // Correct scale factor for symbolized soundings,
    //  as sometimes found in objects OBSTRN, WRECKS, UWTROC
  if (!strncmp(prule->name.SYNM, "SOUND", 5)) {
      scale_factor /= m_ChartScaleFactorExp;
      scale_factor *= m_SoundingsScaleFactor;
  }


  scale_factor *= g_scaminScale;
  scale_factor /= m_dipfactor;

  if (m_display_size_mm <
      200) {  // about 8 inches, implying some sort of smaller mobile device
    //  Set the onscreen size of the symbol
    //  Compensate for various display resolutions
    //  Develop empirically, making a buoy about 4 mm tall
    double boyHeight =
        21. /
        GetPPMM();  // from raster symbol definitions, boylat is xx pix high

    double targetHeight0 = 4.0;

    // But we want to scale the size for smaller displays
    double displaySize = m_display_size_mm;
    displaySize = wxMax(displaySize, 100);

    float targetHeight = wxMin(targetHeight0, displaySize / 30);

    double pix_factor = targetHeight / boyHeight;

    // qDebug() << "scaleing" << m_display_size_mm  << targetHeight0 <<
    // targetHeight << GetPPMM() << boyHeight << pix_factor;

    // for Hubert, and my moto
    // scaleing 93.98 93 4 3.33333 12.7312 1.64949 2.02082
    // My nvidia tab
    // scaleing 144.78 144 4 4 12.6667 1.65789 2.4127
    // judgement: all OK

    scale_factor *= pix_factor;
  }

  scale_factor *= m_ContentScaleFactor;

  // a few special cases here
  if (!strncmp(rzRules->obj->FeatureName, "notmrk", 6) ||
      !strncmp(rzRules->obj->FeatureName, "NOTMRK", 6) ||
      !strncmp(prule->name.SYNM, "ADDMRK", 6)) {
    // get the symbol size
    wxRect trect;
    m_chartSymbols.GetGLTextureRect(trect, prule->name.SYNM);

    int scale_dim = wxMax(trect.width, trect.height);

    double scaled_size = scale_dim / vp_plib.view_scale_ppm;

    double target_size =
        100;  // roughly, meters maximum scaled size for these inland signs

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
  if (!m_pdc) {
    texture = m_chartSymbols.GetGLTextureRect(texrect, prule->name.SYNM);
    if (texture) {
      prule->parm2 = texrect.width * scale_factor;
      prule->parm3 = texrect.height * scale_factor;
    }
  }

  if (m_pdc || !texture) {
    //    Check to see if any cached data is valid
    bool b_dump_cache = false;
    if (prule->pixelPtr) {
      // Detect switches between DC and GL modes, flush if switch occurs
      if (m_pdc) {
        if (prule->parm0 != ID_wxBitmap) b_dump_cache = true;
      } else {
        if (prule->parm0 != ID_RGBA) b_dump_cache = true;
      }
    }

    // If the requested scaled symbol size is not the same as is currently
    // cached, we have to dump the cache
    wxRect trect;
    m_chartSymbols.GetGLTextureRect(trect, prule->name.SYNM);
    if (prule->parm2 != (int)(trect.width * scale_factor)) b_dump_cache = true;

    wxBitmap *pbm = NULL;

    // Instantiate the symbol if necessary
    if ((prule->pixelPtr == NULL) || (prule->parm1 != m_colortable_index) ||
        b_dump_cache) {
      wxImage Image =  m_chartSymbols.GetImage(prule->name.SYNM);

      // delete any old private data
      ClearRulesCache(prule);

      // always display something, TMARDEF1 as width of 2
      int w0 = wxMax(1, Image.GetWidth() * scale_factor);
      int h0 = wxMax(1, Image.GetHeight() * scale_factor);
      Image.Rescale(w0, h0, wxIMAGE_QUALITY_HIGH);

      int w = Image.GetWidth();
      int h = Image.GetHeight();

      if (!m_pdc)  // opengl, not using textures
      {
        //    Get the glRGBA format data from the wxImage
        unsigned char *d = Image.GetData();
        unsigned char *a = Image.GetAlpha();

        Image.SetMaskColour(m_unused_wxColor.Red(), m_unused_wxColor.Green(),
                            m_unused_wxColor.Blue());
        unsigned char mr, mg, mb;
        if (!a && !Image.GetOrFindMaskColour(&mr, &mg, &mb))
          printf("trying to use mask to draw a bitmap without alpha or mask\n");

        unsigned char *e = (unsigned char *)malloc(w * h * 4);
        // XXX FIXME a or e ?
        if (d && a) {
          for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
              unsigned char r, g, b;
              int off = (y * w + x);
              r = d[off * 3 + 0];
              g = d[off * 3 + 1];
              b = d[off * 3 + 2];

              e[off * 4 + 0] = r;
              e[off * 4 + 1] = g;
              e[off * 4 + 2] = b;

              e[off * 4 + 3] =
                  a ? a[off] : ((r == mr) && (g == mg) && (b == mb) ? 0 : 255);
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
        bool b_has_trans = false;
#if (defined(__WXGTK__) || defined(__WXMAC__))

        //    Blitting of wxBitmap with transparency in wxGTK is broken....
        //    We can do it the hard way, by manually alpha blending the
        //    symbol with a clip taken from the current screen DC contents.

        //    Inspect the symbol image, to see if it actually has alpha
        //    transparency
        if (Image.HasAlpha()) {
          unsigned char *a = Image.GetAlpha();
          for (int i = 0; i < Image.GetHeight(); i++, a++) {
            for (int j = 0; j < Image.GetWidth(); j++) {
              if ((*a) && (*a != 255)) {
                b_has_trans = true;
                break;
              }
            }
            if (b_has_trans) break;
          }
        }
#ifdef __WXMAC__
        b_has_trans = true;
#endif

        //    If the symbol image has no transparency, then a standard
        //    wxDC:Blit() will work
        if (!b_has_trans) {
          pbm = new wxBitmap(Image, -1);
          wxMask *pmask = new wxMask(*pbm, m_unused_wxColor);
          pbm->SetMask(pmask);
        }

#else
        pbm = new wxBitmap(Image, 32);  // windows
        wxMask *pmask = new wxMask(*pbm, m_unused_wxColor);
        pbm->SetMask(pmask);
#endif

        //      Save the bitmap ptr and aux parms in the rule
        prule->pixelPtr = pbm;
        prule->parm0 = ID_wxBitmap;
        prule->parm1 = m_colortable_index;
        prule->parm2 = w;
        prule->parm3 = h;
      }
    }  // instantiation
  }

  //        Get the bounding box for the to-be-drawn symbol
  int b_width, b_height;
  b_width = prule->parm2;
  b_height = prule->parm3;

  LLBBox symbox;
  double latmin, lonmin, latmax, lonmax;

  if (!m_pdc && fabs(vp_plib.rotation) > .01)  // opengl
  {
    float cx = vp_plib.pix_width / 2.;
    float cy = vp_plib.pix_height / 2.;
    float c = cosf(vp_plib.rotation);
    float s = sinf(vp_plib.rotation);
    float x = r.x - pivot_x - cx;
    float y = r.y - pivot_y + b_height - cy;
    GetPixPointSingle(x * c - y * s + cx, x * s + y * c + cy, &latmin, &lonmin);

    x = r.x - pivot_x + b_width - cx;
    y = r.y - pivot_y - cy;
    GetPixPointSingle(x * c - y * s + cx, x * s + y * c + cy, &latmax, &lonmax);
  } else {
    GetPixPointSingle(r.x - pivot_x, r.y - pivot_y + b_height, &latmin, &lonmin);
    GetPixPointSingle(r.x - pivot_x + b_width, r.y - pivot_y, &latmax, &lonmax);
  }
  symbox.Set(latmin, lonmin, latmax, lonmax);

  //  Special case for GEO_AREA objects with centred symbols
  if (rzRules->obj->Primitive_type == GEO_AREA) {
    if (!rzRules->obj->BBObj.IntersectIn(
            symbox))  // Symbol is wholly outside base object
      return true;
  }

  //      Now render the symbol

  if (!m_pdc)  // opengl
  {
#ifdef ocpnUSE_GL
    glEnable(GL_BLEND);

    if (texture) {

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture);

      int w = texrect.width, h = texrect.height;

      float tx1 = texrect.x, ty1 = texrect.y;
      float tx2 = tx1 + w, ty2 = ty1 + h;

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
#else

      if (m_TextureFormat == GL_TEXTURE_2D) {
        // Normalize the sybmol texture coordinates against the next higher POT
        // size
        wxSize size = m_chartSymbols.GLTextureSize();
        int rb_x = size.x;
        int rb_y = size.y;
        tx1 /= rb_x, tx2 /= rb_x;
        ty1 /= rb_y, ty2 /= rb_y;
      }

      float uv[8];
      float coords[8];

      // Note swizzle of points to allow TRIANGLE_STRIP drawing
      // normal uv
      uv[0] = tx1;
      uv[1] = ty1;
      uv[2] = tx2;
      uv[3] = ty1;
      uv[6] = tx2;
      uv[7] = ty2;
      uv[4] = tx1;
      uv[5] = ty2;

      w *= scale_factor;
      h *= scale_factor;

      // pixels
      coords[0] = 0;
      coords[1] = 0;
      coords[2] = w;
      coords[3] = 0;
      coords[6] = w;
      coords[7] = h;
      coords[4] = 0;
      coords[5] = h;

      if (pCtexture_2D_shader_program[0]){
        pCtexture_2D_shader_program[0]->Bind();

        // Select the active texture unit.
        glActiveTexture(GL_TEXTURE0);

        pCtexture_2D_shader_program[0]->SetUniform1i( "uTex", 0);

        // Disable VBO's (vertex buffer objects) for attributes.
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        pCtexture_2D_shader_program[0]->SetAttributePointerf( "position", coords);
        pCtexture_2D_shader_program[0]->SetAttributePointerf( "aUV", uv);

        // Rotate
        mat4x4 I, Q;
        mat4x4_identity(I);

        mat4x4_translate_in_place(I, r.x, r.y, 0);
        if (abs(vp_plib.rotation) > 0)
          mat4x4_rotate_Z(Q, I, -vp_plib.rotation);
        else
          mat4x4_dup(Q, I);
        mat4x4_translate_in_place(Q, -pivot_x, -pivot_y, 0);

        pCtexture_2D_shader_program[0]->SetUniformMatrix4fv( "TransformMatrix", (GLfloat *)Q);

        // Perform the actual drawing.
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Restore the per-object transform to Identity Matrix
        mat4x4 IM;
        mat4x4_identity(IM);
        pCtexture_2D_shader_program[0]->SetUniformMatrix4fv( "TransformMatrix", (GLfloat *)IM);

        // Clean up the GL state
        pCtexture_2D_shader_program[0]->UnBind();
      }

#endif  // GLES2
      glDisable(m_TextureFormat);
    } else { /* this is only for legacy mode, or systems without NPOT textures
              */
      float cr = cosf(vp_plib.rotation);
      float sr = sinf(vp_plib.rotation);
      float ddx = pivot_x * cr + pivot_y * sr;
      float ddy = pivot_y * cr - pivot_x * sr;
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
#endif
    }

    glDisable(GL_BLEND);
#endif
  } else {
    if (!(prule->pixelPtr))  // This symbol requires manual alpha blending
    {
      //    Don't bother if the symbol is off the true screen,
      //    as for instance when an area-centered symbol is called for.
      if ((r.x - pivot_x < vp_plib.pix_width) && (r.y - pivot_y < vp_plib.pix_height)) {
        // Get the current screen contents to a wxImage
        wxBitmap b1(b_width, b_height, -1);
        wxMemoryDC mdc1(b1);
        mdc1.Blit(0, 0, b_width, b_height, m_pdc, r.x - pivot_x, r.y - pivot_y,
                  wxCOPY);
        wxImage im_back = b1.ConvertToImage();

        //    Get the scaled symbol as a wxImage
        wxImage im_sym = m_chartSymbols.GetImage(prule->name.SYNM);
        im_sym.Rescale(b_width, b_height, wxIMAGE_QUALITY_HIGH);

        wxImage im_result(b_width, b_height);
        unsigned char *pdest = im_result.GetData();
        unsigned char *pback = im_back.GetData();
        unsigned char *psym = im_sym.GetData();

        unsigned char *asym = NULL;
        if (im_sym.HasAlpha()) asym = im_sym.GetAlpha();

        //    Do alpha blending, the hard way

        if (pdest && psym && pback) {
          for (int i = 0; i < b_height; i++) {
            for (int j = 0; j < b_width; j++) {
              double alpha = 1.0;
              if (asym) alpha = (*asym++) / 256.0;
              unsigned char r = (*psym++ * alpha) + (*pback++ * (1.0 - alpha));
              *pdest++ = r;
              unsigned char g = (*psym++ * alpha) + (*pback++ * (1.0 - alpha));
              *pdest++ = g;
              unsigned char b = (*psym++ * alpha) + (*pback++ * (1.0 - alpha));
              *pdest++ = b;
            }
          }
        }

        wxBitmap result(im_result);
        wxMemoryDC result_dc(result);

        m_pdc->Blit(r.x - pivot_x, r.y - pivot_y, b_width, b_height, &result_dc,
                    0, 0, wxCOPY, false);

        result_dc.SelectObject(wxNullBitmap);
        mdc1.SelectObject(wxNullBitmap);
      }
    } else {
      //      Get the symbol bitmap into a memory dc
      wxBitmap &bmp = (wxBitmap &)(*((wxBitmap *)(prule->pixelPtr)));
      wxMemoryDC mdc(bmp);

      //      Blit it into the target dc, with mask
      m_pdc->Blit(r.x - pivot_x, r.y - pivot_y, bmp.GetWidth(), bmp.GetHeight(),
                  &mdc, 0, 0, wxCOPY, true);

      mdc.SelectObject(wxNullBitmap);
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
  //  We expand the object's BBox to account for objects rendered by multiple
  //  symbols, such as SOUNGD. so that expansions are cumulative.
  if (rzRules->obj->Primitive_type == GEO_POINT)
    rzRules->obj->BBObj.Expand(symbox);

  return true;
}

// SYmbol
int s52plib::RenderSY(ObjRazRules *rzRules, Rules *rules) {
  float angle = 0;
  double orient;

  if (rules->razRule != NULL) {
    if (rules->INSTstr[8] ==
        ',')  // supplementary parameter assumed to be angle, seen in LIGHTSXX
    {
      char sangle[10];
      int cp = 0;
      while (rules->INSTstr[cp + 9] && (rules->INSTstr[cp + 9] != ')')) {
        sangle[cp] = rules->INSTstr[cp + 9];
        cp++;
      }
      sangle[cp] = 0;
      int angle_i = atoi(sangle);
      angle = angle_i;
    }

    if (GetDoubleAttr(
            rzRules->obj, "ORIENT",
            orient))  // overriding any LIGHTSXX angle, probably TSSLPT
    {
      angle = orient;
      if (strncmp(rzRules->obj->FeatureName, "LIGHTS", 6) == 0) {
        angle += 180;
        if (angle > 360) angle -= 360;
      }
    }

    //  Render symbol at object's x/y
    wxPoint r, r1;
    GetPointPixSingle(rzRules, rzRules->obj->y, rzRules->obj->x, &r);

    //  Render a raster or vector symbol, as specified by LUP rules
    if (rules->razRule->definition.SYDF == 'V') {
      RenderHPGL(rzRules, rules->razRule, r, angle, m_ChartScaleFactorExp);
    } else {
      if (rules->razRule->definition.SYDF == 'R')
        RenderRasterSymbol(rzRules, rules->razRule, r, angle);
    }
  }

  return 0;
}

void s52plib::SetupSoundingFont() {
  double scale_factor = 1.0;

  //scale_factor *= m_ChartScaleFactorExp;
  //scale_factor *= g_scaminScale;

  if (m_display_size_mm <
      200) {  // about 8 inches, implying some sort of smaller mobile device
    //  Set the onscreen size of the symbol
    //  Compensate for various display resolutions
    //  Develop empirically, making a buoy about 4 mm tall
    double boyHeight =
        21. /
        GetPPMM();  // from raster symbol definitions, boylat is xx pix high

    double targetHeight0 = 4.0;

    // But we want to scale the size for smaller displays
    double displaySize = m_display_size_mm;
    displaySize = wxMax(displaySize, 100);

    float targetHeight = wxMin(targetHeight0, displaySize / 30);

    double pix_factor = targetHeight / boyHeight;

    // qDebug() << "scaleing" << m_display_size_mm  << targetHeight0 <<
    // targetHeight << GetPPMM() << boyHeight << pix_factor;

    // for Hubert, and my moto
    // scaleing 93.98 93 4 3.33333 12.7312 1.64949 2.02082
    // My nvidia tab
    // scaleing 144.78 144 4 4 12.6667 1.65789 2.4127
    // judgement: all OK

    // scale_factor *= pix_factor;
  }

  wxFontWeight fontWeight = wxFONTWEIGHT_NORMAL;
  wxString fontFacename = wxEmptyString;
  double defaultHeight = 3.0;

#ifdef __OCPN__ANDROID__
  fontWeight = wxFONTWEIGHT_BOLD;
  fontFacename = _T("Roboto");
  defaultHeight = 2.2;
#endif

  int point_size = 6;
  int charWidth, charHeight, charDescent;
  charWidth = 1;

  double target = defaultHeight * scale_factor;

  if (abs(m_SoundingsFontSizeMM - target) > (m_soundFontDelta + .5)) {

    // Recalculate the required point size to give specified height
    wxScreenDC sdc;

    double font_size_mm = 0;
    bool not_done = true;
    while ((point_size < 32) && not_done) {
      wxFont *tentativeFont = FindOrCreateFont_PlugIn(
          point_size, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, fontWeight, false,
          fontFacename);
      sdc.GetTextExtent(_T("0"), &charWidth, &charHeight, &charDescent, NULL,
                        tentativeFont);  // measure the text
      double font_size_mm = (double)(charHeight - charDescent) / GetPPMM();

      if (font_size_mm >= target) {
        not_done = false;
        m_SoundingsPointSize = point_size;
        m_SoundingsFontSizeMM = font_size_mm;
        m_soundFont = tentativeFont;
        m_soundFontDelta = abs(font_size_mm - target);
        break;
      }
      point_size++;
    }
  }
  else {
    if (m_pdc) {
      wxScreenDC sdc;
      sdc.GetTextExtent(_T("0"), &charWidth, &charHeight, &charDescent, NULL,
                        m_soundFont);  // measure the text for DC mode
    }
    point_size = m_SoundingsPointSize;
    point_size /= m_dipfactor;    // Apply Windows display scaling.
  }
  double postmult = m_SoundingsScaleFactor;
  if ((postmult <= 2.0) && (postmult >= 0.5)) {
    point_size *= postmult;
    scale_factor *= postmult;
    charWidth *= postmult;
  }

  // Build the texDepth object, if required
  if (!m_pdc) {  // OpenGL
    if (!m_texSoundings.IsBuilt() ||
        (fabs(m_texSoundings.GetScale() - scale_factor) > 0.05)) {
      m_texSoundings.Delete();
      m_texSoundings.SetContentScaleFactor(m_ContentScaleFactor);

      m_soundFont = FindOrCreateFont_PlugIn(point_size, wxFONTFAMILY_SWISS,
                                            wxFONTSTYLE_NORMAL, fontWeight,
                                            false, fontFacename);
      m_texSoundings.Build(m_soundFont,
                           scale_factor, m_dipfactor);  // texSounding owns the font
    }
  } else {
    m_soundFont = FindOrCreateFont_PlugIn(point_size, wxFONTFAMILY_SWISS,
                                          wxFONTSTYLE_NORMAL, fontWeight, false,
                                          fontFacename);
    m_pdc->SetFont(*m_soundFont);
  }
}


bool s52plib::RenderSoundingSymbol(ObjRazRules *rzRules, Rule *prule,
                                   wxPoint &r, wxColor symColor,
                                   float rot_angle) {

  // Get some metrics
  int charWidth, charHeight, charDescent;
  wxScreenDC sdc;
  sdc.GetTextExtent(_T("0"), &charWidth, &charHeight, &charDescent, NULL,
                    m_soundFont);  // measure the text

  int pivot_x;
  int pivot_y;

  // Parse the symbol name

  //  The digit
  char symDigit = prule->name.SYNM[7];
  int symIndex = symDigit - 0x30;

  //  The pivot point offset group
  char symCPivot = prule->name.SYNM[6];
  int symPivot = symCPivot - 0x30;

  int pivotWidth, pivotHeight;
  // For opengl, the symbols are loaded in a texture
  unsigned int texture = 0;
  wxRect texrect;
  if (!m_pdc) {  // GL
    texture = m_texSoundings.GetTexture();
    m_texSoundings.GetGLTextureRect(texrect, symIndex);

    if (texture) {
      prule->parm2 = texrect.width;
      prule->parm3 = texrect.height;
    }

    pivotWidth = texrect.width;
    pivotHeight = texrect.height;

  } else {
    pivotWidth = charWidth;
    pivotHeight = charHeight - charDescent;
  }

  if (symPivot < 4) {
    pivot_x = (pivotWidth * symPivot); // - (pivotWidth / 4);
    pivot_y = pivotHeight / 2;
  } else if (symPivot == 4){
    pivot_x = -pivotWidth; // - (pivotWidth / 4);
    pivot_y = pivotHeight / 2;
  } else {
    pivot_x = 0; //-(pivotWidth / 4);
    pivot_y = pivotHeight / 5;
  }
  pivot_x *= m_dipfactor;
  pivot_y *= m_dipfactor;

  //        Get the bounding box for the to-be-drawn symbol
  int b_width, b_height;
  b_width = prule->parm2;
  b_height = prule->parm3;

  LLBBox symbox;
  double latmin, lonmin, latmax, lonmax;

  if (!m_pdc && fabs(vp_plib.rotation) > .01)  // opengl
  {
    float cx = vp_plib.pix_width / 2.;
    float cy = vp_plib.pix_height / 2.;
    float c = cosf(vp_plib.rotation);
    float s = sinf(vp_plib.rotation);
    float x = r.x - pivot_x - cx;
    float y = r.y - pivot_y + b_height - cy;
    GetPixPointSingle(x * c - y * s + cx, x * s + y * c + cy, &latmin, &lonmin);

    x = r.x - pivot_x + b_width - cx;
    y = r.y - pivot_y - cy;
    GetPixPointSingle(x * c - y * s + cx, x * s + y * c + cy, &latmax, &lonmax);
  } else {
    GetPixPointSingle(r.x - pivot_x, r.y - pivot_y + b_height, &latmin, &lonmin);
    GetPixPointSingle(r.x - pivot_x + b_width, r.y - pivot_y, &latmax, &lonmax);
  }
  symbox.Set(latmin, lonmin, latmax, lonmax);

  //      Now render the symbol

  if (!m_pdc)  // opengl
  {
#ifdef ocpnUSE_GL
    glEnable(GL_BLEND);

    if (texture) {
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBindTexture(GL_TEXTURE_2D, texture);

      int w = texrect.width, h = texrect.height;

      float tx1 = texrect.x, ty1 = texrect.y;
      float tx2 = tx1 + w, ty2 = ty1 + h;

      if (m_TextureFormat == GL_TEXTURE_2D) {
        // Normalize the sybmol texture coordinates against the next higher POT
        // size
        wxSize size = m_texSoundings.GLTextureSize();
        int rb_x = size.x;
        int rb_y = size.y;

        tx1 /= rb_x, tx2 /= rb_x;
        ty1 /= rb_y, ty2 /= rb_y;
      }

      float uv[8];
      float coords[8];

      // Note swizzle of points to allow TRIANGLE_STRIP drawing
      // normal uv
      uv[0] = tx1;
      uv[1] = ty1;
      uv[2] = tx2;
      uv[3] = ty1;
      uv[6] = tx2;
      uv[7] = ty2;
      uv[4] = tx1;
      uv[5] = ty2;

      // pixels
      coords[0] = 0;
      coords[1] = 0;
      coords[2] = w;
      coords[3] = 0;
      coords[6] = w;
      coords[7] = h;
      coords[4] = 0;
      coords[5] = h;

      float colorv[4];
      colorv[0] = symColor.Red() / float(256);
      colorv[1] = symColor.Green() / float(256);
      colorv[2] = symColor.Blue() / float(256);
      colorv[3] = 1.0;

      pCtexture_2D_Color_shader_program[0]->Bind();

        // Select the active texture unit.
      glActiveTexture(GL_TEXTURE0);

      pCtexture_2D_Color_shader_program[0]->SetUniform4fv( "color", colorv);

      pCtexture_2D_Color_shader_program[0]->SetUniform1i( "uTex", 0);

        // Disable VBO's (vertex buffer objects) for attributes.
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      pCtexture_2D_Color_shader_program[0]->SetAttributePointerf( "position", coords);
      pCtexture_2D_Color_shader_program[0]->SetAttributePointerf( "aUV", uv);

      // Rotate
       mat4x4 I, Q;
       mat4x4_identity(I);
       mat4x4_identity(Q);

      mat4x4_translate_in_place(I, r.x, r.y, 0);
      mat4x4_rotate_Z(Q, I, -vp_plib.rotation);
      mat4x4_translate_in_place(Q, -pivot_x, -pivot_y, 0);

      pCtexture_2D_Color_shader_program[0]->SetUniformMatrix4fv( "TransformMatrix", (GLfloat *)Q);

      // Perform the actual drawing.
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      // Restore the per-object transform to Identity Matrix
      mat4x4 IM;
      mat4x4_identity(IM);
      pCtexture_2D_Color_shader_program[0]->SetUniformMatrix4fv( "TransformMatrix", (GLfloat *)IM);
      pCtexture_2D_Color_shader_program[0]->UnBind();
      glDisable(m_TextureFormat);
    } else { /* this is only for legacy mode, or systems without NPOT textures
              */
      float cr = cosf(vp_plib.rotation);
      float sr = sinf(vp_plib.rotation);
      float ddx = pivot_x * cr + pivot_y * sr;
      float ddy = pivot_y * cr - pivot_x * sr;
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
#endif
    }

    glDisable(GL_BLEND);
#endif
  } else {
    wxString text;
    text.Printf(_T("%d"), symIndex);
    m_pdc->SetTextForeground(symColor);

    m_pdc->DrawText(text, r.x - pivot_x, r.y - pivot_y);
  }

  return true;
}

// Line Simple Style, OpenGL
int s52plib::RenderGLLS(ObjRazRules *rzRules, Rules *rules) {
  // for now don't use vbo model in non-mercator
//   if (vp->m_projection_type != PROJECTION_MERCATOR)
//     return RenderLS(rzRules, rules, vp);

  if (!ObjectRenderCheckPosReduced(rzRules))
    return false;

  if (!m_benableGLLS)  // root chart cannot support VBO model
    return RenderLS(rzRules, rules);

  if (!rzRules->obj->m_chart_context->chart)
    return RenderLS(rzRules, rules);  // this is where S63 PlugIn gets caught

  if ((GetBBox().GetMaxLon() >= 180.) ||
      (GetBBox().GetMinLon() <= -180.))
    return RenderLS(rzRules, rules);  // cm93 has trouble at IDL

  bool b_useVBO = false;
  float *vertex_buffer = 0;

  if (rzRules->obj->auxParm2 > 0)  // Has VBO been defined and uploaded?
    b_useVBO = true;

  if (!b_useVBO) {
    vertex_buffer = rzRules->obj->m_chart_context->vertex_buffer;
  }

#ifdef ocpnUSE_GL

  char *str = (char *)rules->INSTstr;

  if ((!strncmp(str, "DASH", 4)) || (!strncmp(str, "DOTT", 4)))
    return RenderLS_Dash_GLSL(rzRules, rules);

  LLBBox BBView = GetReducedBBox();

  //  Allow a little slop in calculating whether a segment
  //  is within the requested VPointCompat
  double margin = BBView.GetLonRange() * .05;
  BBView.EnLarge(margin);

  //  Get the current display priority
  //  Default comes from the LUP, unless overridden
  int priority_current = rzRules->LUP->DPRI - '0';
  if (rzRules->obj->m_DPRI >= 0) priority_current = rzRules->obj->m_DPRI;

  line_segment_element *ls_list = rzRules->obj->m_ls_list;

  S52color *c = getColor(str + 7);  // Colour
  int w = atoi(str + 5);            // Width

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
#endif

  //    Set drawing width
  float lineWidth = w;
  lineWidth = wxMax(m_GLMinCartographicLineWidth, w);

  // Manage super high density displays
  float target_w_mm = 0.5 * w;
  if (GetPPMM() > 7) {  // arbitrary
    target_w_mm =
        ((float)w) /
        6.0;  // Target width in mm
              //  The value "w" comes from S52 library CNSY procedures, in
              //  "nominal" pixels
              // the value "6" comes from semi-standard LCD display densities
              // or something like 0.18 mm pitch, or 6 pix per mm.
    lineWidth = wxMax(m_GLMinCartographicLineWidth, target_w_mm * GetPPMM());
  }

  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);

#ifdef __OCPN__ANDROID__
  //     if( w > 1 )
  //         lineWidth = wxMin(lineWidth, parms[1]);
  glLineWidth(lineWidth);

#else
  glLineWidth(lineWidth);
  if (lineWidth > 4.0 && m_GLLineSmoothing) {
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
  }
#endif

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
#endif

#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
#endif

  //   Has line segment PBO been allocated for this chart?
  if (b_useVBO) {
    glBindBuffer(GL_ARRAY_BUFFER, rzRules->obj->auxParm2);
  }

  CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
  shader->Bind();

  // Disable VBO's (vertex buffer objects) for attributes.
  if (!b_useVBO) glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  GLint pos = shader->getAttributeLocation("position");
  float angle = 0;

  // We cannot use the prepared shader uniforms, as we can (and should for
  // performance) do the per-object transforms all at once in the shader matrix.
  // But we also must restore the prepared matrix for later rendering of other
  // object classes.

  // Build Transform matrix
  //  First, the VP transform
  mat4x4 I, Q;
  mat4x4_identity(I);

  // Scale per object
  I[0][0] *= rzRules->obj->x_rate;
  I[1][1] *= rzRules->obj->y_rate;

  // Translate per object
  I[3][0] = -(rzRules->sm_transform_parms->easting_vp_center -
              rzRules->obj->x_origin) *
            vp_plib.view_scale_ppm;
  I[3][1] = -(rzRules->sm_transform_parms->northing_vp_center -
              rzRules->obj->y_origin) *
            -vp_plib.view_scale_ppm;

  // Scale for screen
  I[0][0] *= vp_plib.view_scale_ppm;
  I[1][1] *= -vp_plib.view_scale_ppm;

  // Rotate
  mat4x4_rotate_Z(Q, I, angle);

  // Translate for screen
  Q[3][0] += vp_plib.pix_width / 2;
  Q[3][1] += vp_plib.pix_height / 2;

  shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)Q);

  float colorv[4];
  colorv[0] = c->R / float(256);
  colorv[1] = c->G / float(256);
  colorv[2] = c->B / float(256);
  colorv[3] = 1.0;

  shader->SetUniform4fv("color", colorv);

  if (!b_useVBO) {
    unsigned char *buffer = (unsigned char *)vertex_buffer;
    float *bufBase = (float *)buffer;
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          bufBase);
  }
  else {
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                                (GLvoid *)(0));
  }
  glEnableVertexAttribArray(pos);

  // from above ls_list is the first drawable segment
  while (ls_list) {
    if (ls_list->priority == priority_current) {
      size_t seg_vbo_offset = 0;
      size_t point_count = 0;

      //  Check visibility of the segment
      bool b_drawit = false;
      if ((ls_list->ls_type == TYPE_EE) || (ls_list->ls_type == TYPE_EE_REV)) {
        if(BBView.GetMinLat() < ls_list->pedge->edgeBBox.GetMaxLat() &&
           BBView.GetMaxLat() > ls_list->pedge->edgeBBox.GetMinLat() &&
           BBView.GetMinLon() <= ls_list->pedge->edgeBBox.GetMaxLon() &&
           BBView.GetMaxLon() >= ls_list->pedge->edgeBBox.GetMinLon())
        {
          // render the segment
          b_drawit = true;
          seg_vbo_offset = ls_list->pedge->vbo_offset;
          point_count = ls_list->pedge->nCount;
        }

      } else {
        if(BBView.GetMinLat() < ls_list->pcs->cs_lat_avg &&
           BBView.GetMaxLat() > ls_list->pcs->cs_lat_avg &&
           BBView.GetMinLon() <= ls_list->pcs->cs_lon_avg &&
           BBView.GetMaxLon() >= ls_list->pcs->cs_lon_avg)
        {
          // render the segment
          b_drawit = true;
          seg_vbo_offset = ls_list->pcs->vbo_offset;
          point_count = 2;
        }
      }

      if (b_drawit) {
        // render the segment
        glDrawArrays(GL_LINE_STRIP, seg_vbo_offset / (2 * sizeof(float)) , point_count);
      }
    }
    ls_list = ls_list->next;
  }

  if (b_useVBO) glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Restore shader TransForm Matrix to identity.
  mat4x4 IM;
  mat4x4_identity(IM);
  shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)IM);
  shader->UnBind();

  glDisable(GL_LINE_STIPPLE);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);

#endif  // OpenGL

  return 1;
}

// Line Simple Style
int s52plib::RenderLS(ObjRazRules *rzRules, Rules *rules) {
  // catch legacy PlugIns (e.g.s63_pi)
  if (rzRules->obj->m_n_lsindex && !rzRules->obj->m_ls_list)
    return RenderLSLegacy(rzRules, rules);

  // catch improperly coded edge arrays, usually seen on cm93
  if (!rzRules->obj->m_n_lsindex && !rzRules->obj->m_ls_list) return 0;

  S52color *c;
  int w;

  char *str = (char *)rules->INSTstr;
  c = getColor(str + 7);  // Colour
  wxColour color(c->R, c->G, c->B);
  w = atoi(str + 5);  // Width

  double scale_factor = vp_plib.ref_scale / vp_plib.chart_scale;
  double scaled_line_width =
      wxMax((scale_factor), 1);
  bool b_wide_line = false;

  wxPen wide_pen(*wxBLACK_PEN);
  wxDash dashw[2];
  dashw[0] = 3;
  dashw[1] = 1;

  if (b_wide_line) {
    int w = wxMax(scaled_line_width, 2);  // looks better
    w = wxMin(w, 50);                     // upper bound
    wide_pen.SetWidth(w);
    wide_pen.SetColour(color);

    if (!strncmp(str, "DOTT", 4)) {
      dashw[0] = 1;
      wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
      wide_pen.SetDashes(2, dashw);
    } else if (!strncmp(str, "DASH", 4)) {
      wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
      if (m_pdc) {  // DC mode
        dashw[0] = 1;
        dashw[1] = 2;
      }

      wide_pen.SetDashes(2, dashw);
    }
  }

  wxPen thispen(color, w, wxPENSTYLE_SOLID);
  wxDash dash1[2];

  if (m_pdc)  // DC mode
  {
    if (!strncmp(str, "DOTT", 4)) {
      thispen.SetStyle(wxPENSTYLE_USER_DASH);
      dash1[0] = 1;
      dash1[1] = 2;
      thispen.SetDashes(2, dash1);
    } else if (!strncmp(str, "DASH", 4)) {
      thispen.SetStyle(wxPENSTYLE_SHORT_DASH);
    }

    if (b_wide_line)
      m_pdc->SetPen(wide_pen);
    else
      m_pdc->SetPen(thispen);

  }
#ifdef ocpnUSE_GL
  else  // OpenGL mode
  {
    glDisable(GL_LINE_SMOOTH);

    //    Set drawing width
    if (w > 1) {
      GLint parms[2];
      glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
      if (w > parms[1])
        glLineWidth(wxMax(m_GLMinCartographicLineWidth, parms[1]));
      else
        glLineWidth(wxMax(m_GLMinCartographicLineWidth, w));
    } else
      glLineWidth(wxMax(m_GLMinCartographicLineWidth, 1));

#ifndef __OCPN__ANDROID__
    if (w >= 2 && m_GLLineSmoothing) {
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
    }
#endif
  }
#endif

  //    Get a true pixel clipping/bounding box from the vp
  wxPoint pbb = GetPixFromLL(vp_plib.clat, vp_plib.clon);
  int xmin_ = pbb.x - (vp_plib.rv_rect.width / 2) - (4 * scaled_line_width);
  int xmax_ = xmin_ + vp_plib.rv_rect.width + (8 * scaled_line_width);
  int ymin_ = pbb.y - (vp_plib.rv_rect.height / 2) - (4 * scaled_line_width);
  int ymax_ = ymin_ + vp_plib.rv_rect.height + (8 * scaled_line_width);

  int x0, y0, x1, y1;

  //  Get the current display priority
  //  Default comes from the LUP, unless overridden
  int priority_current = rzRules->LUP->DPRI - '0';
  if (rzRules->obj->m_DPRI >= 0) priority_current = rzRules->obj->m_DPRI;

  if (rzRules->obj->m_ls_list) {
    float *ppt;

    unsigned char *vbo_point =
        (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
    line_segment_element *ls = rzRules->obj->m_ls_list;

    while (ls) {
      if (ls->priority == priority_current) {
        int nPoints;
        // fetch the first point
        if ((ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV)) {
          ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
          nPoints = ls->pedge->nCount;
        } else {
          ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
          nPoints = 2;
        }

        wxPoint l;
        GetPointPixSingle(rzRules, ppt[1], ppt[0], &l);
        ppt += 2;

        for (int ip = 0; ip < nPoints - 1; ip++) {
          wxPoint r;
          GetPointPixSingle(rzRules, ppt[1], ppt[0], &r);
          //        Draw the edge as point-to-point
          x0 = l.x, y0 = l.y;
          x1 = r.x, y1 = r.y;

          // Do not draw null segments
          if ((x0 != x1) || (y0 != y1)) {
            if (m_pdc) {
              if (cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, xmin_, xmax_,
                                               ymin_, ymax_) != Invisible)
                m_pdc->DrawLine(x0, y0, x1, y1);
            }
          }

          l = r;
          ppt += 2;
        }
      }

      ls = ls->next;
    }
  }

#ifdef ocpnUSE_GL
  if (!m_pdc) {
    glDisable(GL_LINE_STIPPLE);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
  }
#endif
  return 1;
}

// Line Simple Style
int s52plib::RenderLSLegacy(ObjRazRules *rzRules, Rules *rules) {
  if (!rzRules->obj->m_chart_context->chart)
    return RenderLSPlugIn(rzRules, rules);

  // Must be cm93
  S52color *c;
  int w;
  GLint pos = 0;

  char *str = (char *)rules->INSTstr;
  c = getColor(str + 7);  // Colour
  wxColour color(c->R, c->G, c->B);
  w = atoi(str + 5);  // Width

  double scale_factor = vp_plib.ref_scale / vp_plib.chart_scale;
  double scaled_line_width =
      wxMax((scale_factor), 1);
  bool b_wide_line = false;

  wxPen wide_pen(*wxBLACK_PEN);
  wxDash dashw[2];
  dashw[0] = 3;
  dashw[1] = 1;

  if (b_wide_line) {
    int w = wxMax(scaled_line_width, 2);  // looks better
    w = wxMin(w, 50);                     // upper bound
    wide_pen.SetWidth(w);
    wide_pen.SetColour(color);

    if (!strncmp(str, "DOTT", 4)) {
      dashw[0] = 1;
      wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
      wide_pen.SetDashes(2, dashw);
    } else if (!strncmp(str, "DASH", 4)) {
      wide_pen.SetStyle(wxPENSTYLE_USER_DASH);
      if (m_pdc) {  // DC mode
        dashw[0] = 1;
        dashw[1] = 2;
      }

      wide_pen.SetDashes(2, dashw);
    }
  }

  wxPen thispen(color, w, wxPENSTYLE_SOLID);
  wxDash dash1[2];

  if (m_pdc)  // DC mode
  {
    if (!strncmp(str, "DOTT", 4)) {
      thispen.SetStyle(wxPENSTYLE_USER_DASH);
      dash1[0] = 1;
      dash1[1] = 2;
      thispen.SetDashes(2, dash1);
    } else if (!strncmp(str, "DASH", 4)) {
      thispen.SetStyle(wxPENSTYLE_SHORT_DASH);
    }

    if (b_wide_line)
      m_pdc->SetPen(wide_pen);
    else
      m_pdc->SetPen(thispen);

  }
#ifdef ocpnUSE_GL
  else  // OpenGL mode
  {
    glDisable(GL_LINE_SMOOTH);

    //    Set drawing width
    if (w > 1) {
      GLint parms[2];
      glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
      if (w > parms[1])
        glLineWidth(wxMax(m_GLMinCartographicLineWidth, parms[1]));
      else
        glLineWidth(wxMax(m_GLMinCartographicLineWidth, w));
    } else
      glLineWidth(wxMax(m_GLMinCartographicLineWidth, 1));

#ifndef __OCPN__ANDROID__
    if (w >= 2 && m_GLLineSmoothing) {
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
    }
#endif
  }
#endif

  //    Get a true pixel clipping/bounding box from the vp
  wxPoint pbb = GetPixFromLL(vp_plib.clat, vp_plib.clon);
  int xmin_ = pbb.x - (vp_plib.rv_rect.width / 2) - (4 * scaled_line_width);
  int xmax_ = xmin_ + vp_plib.rv_rect.width + (8 * scaled_line_width);
  int ymin_ = pbb.y - (vp_plib.rv_rect.height / 2) - (4 * scaled_line_width);
  int ymax_ = ymin_ + vp_plib.rv_rect.height + (8 * scaled_line_width);

  int x0, y0, x1, y1;

  if (rzRules->obj->m_n_lsindex) {
    // This is cm93
    auto ve_hash = rzRules->obj->m_chart_context->m_pve_hash;
    auto vc_hash = rzRules->obj->m_chart_context->m_pvc_hash;

    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if (rzRules->obj->m_DPRI >= 0) priority_current = rzRules->obj->m_DPRI;

    int *index_run;
    float *ppt;
    VC_Element *pnode;
#ifdef ocpnUSE_GL
    CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
    shader->Bind();

    float fBuf[4];
    pos = shader->getAttributeLocation("position");
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), fBuf);
    glEnableVertexAttribArray(pos);

    float colorv[4];
    colorv[0] = c->R / float(256);
    colorv[1] = c->G / float(256);
    colorv[2] = c->B / float(256);
    colorv[3] = 1.0;

    shader->SetUniform4fv("color", colorv);
#endif

    for (int iseg = 0; iseg < rzRules->obj->m_n_lsindex; iseg++) {
      int seg_index = iseg * 3;
      index_run = &rzRules->obj->m_lsindex_array[seg_index];

      //  Get first connected node
      unsigned int inode = *index_run++;

      //  Get the edge
      unsigned int enode = *index_run++;
      VE_Element *pedge = 0;
      if (enode) pedge = (*ve_hash)[enode];

      //  Get last connected node
      unsigned int jnode = *index_run++;

      int nls;
      if (pedge) {
        //  Here we decide to draw or not based on the highest priority seen for
        //  this segment That is, if this segment is going to be drawn at a
        //  higher priority later, then "continue", and don't draw it here.

        // This logic is not perfectly right for one case:
        // If the segment has only two end connected nodes, and no intermediate
        // edge, then we have no good way to evaluate the priority. This is due
        // to the fact that priority is only precalculated for edge segments,
        // not connector nodes. Only thing to do is take the conservative
        // approach and draw the segment, in this case.
        if (pedge->nCount && pedge->max_priority != priority_current) continue;
        nls = pedge->nCount + 1;
      } else
        nls = 1;

      wxPoint l;
      bool lastvalid = false;
      for (int ipc = 0; ipc < nls + 1; ipc++) {
        ppt = 0;
        if (ipc == 0) {
          if (inode) {
            pnode = (*vc_hash)[inode];
            if (pnode) ppt = pnode->pPoint;
          }
        } else if (ipc == nls) {
          if ((jnode)) {
            pnode = (*vc_hash)[jnode];
            if (pnode) ppt = pnode->pPoint;
          }
        } else if (pedge)
          ppt = pedge->pPoints + 2 * (ipc - 1);

        if (ppt) {
          wxPoint r;
          GetPointPixSingle(rzRules, ppt[1], ppt[0], &r);

          if (1 /*r.x != INVALID_COORD*/)
          {
            if (lastvalid) {
              //        Draw the edge as point-to-point
              x0 = l.x, y0 = l.y;
              x1 = r.x, y1 = r.y;

              // Do not draw null segments
              if ((x0 == x1) && (y0 == y1)) continue;

              if (m_pdc) {
                if (cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, xmin_,
                                                 xmax_, ymin_,
                                                 ymax_) != Invisible)
                  m_pdc->DrawLine(x0, y0, x1, y1);
              }
              else {
#ifdef ocpnUSE_GL
                fBuf[0] = x0;
                fBuf[1] = y0;
                fBuf[2] = x1;
                fBuf[3] = y1;

                glDrawArrays(GL_LINES, 0, 2);
#endif

              }
            }

            l = r;
            lastvalid = true;
          } else
            lastvalid = false;
        } else
          lastvalid = false;
      }  //for
    }   //for

#ifdef ocpnUSE_GL
    glDisableVertexAttribArray(pos);
    shader->UnBind();
#endif
  }

#ifdef ocpnUSE_GL
  if (!m_pdc) {
    glDisable(GL_LINE_STIPPLE);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
  }
#endif
  return 1;
}

class PI_connector_segment  // This was extracted verbatim from S63_pi private
                            // definition
{
public:
  void *start;
  void *end;
  SegmentType type;
  int vbo_offset;
  int max_priority;
};

int s52plib::RenderLSPlugIn(ObjRazRules *rzRules, Rules *rules) {
  S52color *c;
  int w;

  char *str = (char *)rules->INSTstr;
  c = getColor(str + 7);  // Colour
  wxColour color(c->R, c->G, c->B);
  w = atoi(str + 5);  // Width

  double scale_factor = vp_plib.ref_scale / vp_plib.chart_scale;
  double scaled_line_width =
      wxMax((scale_factor), 1);
  bool b_wide_line = false;

  wxPen thispen(color, w, wxPENSTYLE_SOLID);
  wxDash dash1[2];

  if (m_pdc)  // DC mode
  {
    if (!strncmp(str, "DOTT", 4)) {
      thispen.SetStyle(wxPENSTYLE_USER_DASH);
      dash1[0] = 1;
      dash1[1] = 2;
      thispen.SetDashes(2, dash1);
    } else if (!strncmp(str, "DASH", 4)) {
      thispen.SetStyle(wxPENSTYLE_SHORT_DASH);
    }

    m_pdc->SetPen(thispen);

  }
  else  // OpenGL mode
  {
#ifdef ocpnUSE_GL
    //    Set drawing width
    if (w > 1) {
      GLint parms[2];
      glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
      if (w > parms[1])
        glLineWidth(wxMax(m_GLMinCartographicLineWidth, parms[1]));
      else
        glLineWidth(wxMax(m_GLMinCartographicLineWidth, w));
    } else
      glLineWidth(wxMax(m_GLMinCartographicLineWidth, 1));
#endif
  }

  //    Get a true pixel clipping/bounding box from the vp
  wxPoint pbb = GetPixFromLL(vp_plib.clat, vp_plib.clon);
  int xmin_ = pbb.x - (vp_plib.rv_rect.width / 2) - (4 * scaled_line_width);
  int xmax_ = xmin_ + vp_plib.rv_rect.width + (8 * scaled_line_width);
  int ymin_ = pbb.y - (vp_plib.rv_rect.height / 2) - (4 * scaled_line_width);
  int ymax_ = ymin_ + vp_plib.rv_rect.height + (8 * scaled_line_width);

  int x0, y0, x1, y1;

  //  Get the current display priority
  //  Default comes from the LUP, unless overridden
  int priority_current = rzRules->LUP->DPRI - '0';
  if (rzRules->obj->m_DPRI >= 0) priority_current = rzRules->obj->m_DPRI;

  if (rzRules->obj->m_ls_list_legacy) {
#ifdef ocpnUSE_GL
    CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
    shader->Bind();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    float colorv[4];
    colorv[0] = color.Red() / float(256);
    colorv[1] = color.Green() / float(256);
    colorv[2] = color.Blue() / float(256);
    colorv[3] = 1.0;  // transparency;

    shader->SetUniform4fv("color", colorv);

    GLint pos = shader->getAttributeLocation("position");
    glEnableVertexAttribArray(pos);
#endif
    float *ppt;
    VE_Element *pedge;
    PI_connector_segment *pcs;

    unsigned char *vbo_point =
        (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
    PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;

    while (ls) {
      if (ls->priority == priority_current) {
        int nPoints;
        // fetch the first point
        if (ls->type == TYPE_EE) {
          pedge = (VE_Element *)ls->private0;
          ppt = (float *)(vbo_point + pedge->vbo_offset);
          nPoints = pedge->nCount;
        } else {
          pcs = (PI_connector_segment *)ls->private0;
          ppt = (float *)(vbo_point + pcs->vbo_offset);
          nPoints = 2;
        }

        wxPoint l;
        GetPointPixSingle(rzRules, ppt[1], ppt[0], &l);
        ppt += 2;

        for (int ip = 0; ip < nPoints - 1; ip++) {
          wxPoint r;
          GetPointPixSingle(rzRules, ppt[1], ppt[0], &r);
          //        Draw the edge as point-to-point
          x0 = l.x, y0 = l.y;
          x1 = r.x, y1 = r.y;

          // Do not draw null segments
          if ((x0 != x1) || (y0 != y1)) {
            if (m_pdc) {
              if (cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, xmin_, xmax_,
                                               ymin_, ymax_) != Invisible)
                m_pdc->DrawLine(x0, y0, x1, y1);
            }
            else {
#ifdef ocpnUSE_GL
              // simplified faster test, let opengl do the rest
              if ((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                  (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {
                float pts[4];
                pts[0] = x0;
                pts[1] = y0;
                pts[2] = x1;
                pts[3] = y1;

                glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                                      pts);

                glDrawArrays(GL_LINES, 0, 2);

              }
#endif
            }
          }

          l = r;
          ppt += 2;
        }
      }
      ls = ls->next;
    }

#ifdef ocpnUSE_GL
    shader->UnBind();
#endif
  }

  return 1;
}

// Line Simple Style, Dashed, using GLSL
#ifdef ocpnUSE_GL
int s52plib::RenderLS_Dash_GLSL(ObjRazRules *rzRules, Rules *rules) {

  //  Retrieve the current clipping rectangle
  wxRect clip_rect = wxRect(0, 0, vp_plib.pix_width, vp_plib.pix_height);
  if (glIsEnabled(GL_SCISSOR_TEST)){
    int rect[4];
    glGetIntegerv(GL_SCISSOR_BOX, rect);
    clip_rect.x = rect[0];
    clip_rect.y = vp_plib.pix_height - (rect[1] + rect[3]);
    clip_rect.width = rect[2];
    clip_rect.height = rect[3];
  }

  S52color *c;

  char *instr_str = (char *)rules->INSTstr;
  c = getColor(instr_str + 7);  // Colour
  wxColour color(c->R, c->G, c->B);
  int w = atoi(instr_str + 5);  // Width

  double scale_factor = vp_plib.ref_scale / vp_plib.chart_scale;
  double scaled_line_width =
      wxMax((scale_factor), 1);


  //    Set drawing width

  float lineWidth = w;
  GLint parms[2];
  glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
  GLint parmsa[2];
  glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, &parmsa[0]);
  GLint parmsb[2];
  glGetIntegerv(GL_SMOOTH_LINE_WIDTH_GRANULARITY, &parmsb[0]);

  if (w > 1) {
    if (w > parms[1])
      lineWidth = wxMax(m_GLMinCartographicLineWidth, parms[1]);
    else
      lineWidth = wxMax(m_GLMinCartographicLineWidth, w);
  } else
    lineWidth = wxMax(m_GLMinCartographicLineWidth, 1);

  // Manage super high density displays
  float target_w_mm = 0.5 * w;
  ;
  if (GetPPMM() >
      7) {  // arbitrary, leaves average desktop/laptop display untweaked...
    target_w_mm = ((float)w) / 6.0;  // Target width in mm
    //  The value "w" comes from S52 library CNSY procedures, in "nominal"
    //  pixels
    // the value "6" comes from semi-standard LCD display densities
    // or something like 0.18 mm pitch, or 6 pix per mm.
    lineWidth = wxMax(m_GLMinCartographicLineWidth, target_w_mm * GetPPMM());
  }

  glDisable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);  // for shader

#ifdef __OCPN__ANDROID__
  lineWidth = wxMin(lineWidth, parms[1]);
  glLineWidth(lineWidth);

#else
  glLineWidth(lineWidth);
  if (lineWidth > 4.0) {
    glEnable(GL_LINE_SMOOTH);
  }
#endif

  //    Get a true pixel clipping/bounding box from the vp
  wxPoint pbb = GetPixFromLL(vp_plib.clat, vp_plib.clon);
  int xmin_ = pbb.x - (vp_plib.rv_rect.width / 2) - (4 * scaled_line_width);
  int xmax_ = xmin_ + vp_plib.rv_rect.width + (8 * scaled_line_width);
  int ymin_ = pbb.y - (vp_plib.rv_rect.height / 2) - (4 * scaled_line_width);
  int ymax_ = ymin_ + vp_plib.rv_rect.height + (8 * scaled_line_width);

  int x0, y0, x1, y1;

  //  Get the current display priority
  //  Default comes from the LUP, unless overridden
  int priority_current = rzRules->LUP->DPRI - '0';
  if (rzRules->obj->m_DPRI >= 0) priority_current = rzRules->obj->m_DPRI;

  glUseProgram(S52Dash_shader_program);

  // Get pointers to the attributes in the program.
  GLint mPosAttrib = glGetAttribLocation(S52Dash_shader_program, "position");
  glEnableVertexAttribArray(mPosAttrib);

  GLint startPos = glGetUniformLocation(S52Dash_shader_program, "startPos");
  GLint texWidth = glGetUniformLocation(S52Dash_shader_program, "texWidth");
  GLint dashFactor = glGetUniformLocation(S52Dash_shader_program, "dashFactor");

  float colorv[4];
  colorv[0] = color.Red() / float(256);
  colorv[1] = color.Green() / float(256);
  colorv[2] = color.Blue() / float(256);
  colorv[3] = 1.0;  // transparency;

  GLint colloc = glGetUniformLocation(S52Dash_shader_program, "color");
  glUniform4fv(colloc, 1, colorv);

  // Select the active texture unit.
  glActiveTexture(GL_TEXTURE0);

  // Set up the texture sampler to texture unit 0
  GLint texUni = glGetUniformLocation(S52Dash_shader_program, "uTex");
  glUniform1i(texUni, 0);

  if (!strncmp(instr_str, "DASH", 4)){
   //reduced from s52 specs (5.4), 3.6mm dash, 1.8mm space
   float width = GetPPMM() * 3; //looks better
   glUniform1f(texWidth, width);
   glUniform1f(dashFactor, 0.66);
  }
  else {
   float width = GetPPMM() * 1;
   glUniform1f(texWidth, width);
   glUniform1f(dashFactor, 0.5);
  }

  // Disable VBO's (vertex buffer objects) for attributes.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Rotate
  mat4x4 I;
  mat4x4_identity(I);

  GLint matloc =
      glGetUniformLocation(S52Dash_shader_program, "TransformMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)I);

  if (rzRules->obj->m_ls_list) {
    float *ppt;

    unsigned char *vbo_point =
        (unsigned char *)
            rzRules->obj->m_chart_context->vertex_buffer;
    line_segment_element *ls = rzRules->obj->m_ls_list;

    while (ls) {
      if (ls->priority == priority_current) {
        int nPoints;
        // fetch the first point
        if ((ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV)) {
          ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
          nPoints = ls->pedge->nCount;
        } else {
          ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
          nPoints = 2;
        }

        wxPoint l;
        GetPointPixSingle(rzRules, ppt[1], ppt[0], &l);
        ppt += 2;

        for (int ip = 0; ip < nPoints - 1; ip++) {
          wxPoint r;
          GetPointPixSingle(rzRules, ppt[1], ppt[0], &r);
          //        Draw the edge as point-to-point
          x0 = l.x, y0 = l.y;
          x1 = r.x, y1 = r.y;

          // Do not draw null segments
          if ((x0 != x1) || (y0 != y1)) {

            // segment must be at least on-screen....
            if ((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {
              //  And intersecting the current clip rectangle
              int xa = x0;
              int xw = x1 - x0;
              if (xw < 0) {
                xa = x1;
                xw = -xw;
              }
              int ya = y0;
              int yh = y1 - y0;
              if (yh < 0) {
                ya = y1;
                yh = -yh;
              }

              wxRect rseg(xa, ya, xw, yh);
              rseg.Inflate(1);  // avoid zero width/height

              if (rseg.Intersects(clip_rect)) {
                float coords[4];

                coords[0] = x0;
                coords[1] = y0;
                coords[2] = x1;
                coords[3] = y1;

                float start[2];
                start[0] = x0;
                start[1] = vp_plib.pix_height - y0;
                glUniform2fv(startPos, 1, start);

                // Set the attribute mPosAttrib with the vertices in the screen
                // coordinates...
                glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0,
                                      coords);
                glEnableVertexAttribArray(mPosAttrib);

                // Perform the actual drawing.
                glDrawArrays(GL_LINES, 0, 2);
              }
            }
          }

          l = r;
          ppt += 2;
        }
      }

      ls = ls->next;
    }
  }

  glUseProgram(0);
  glDisableVertexAttribArray(mPosAttrib);

  return 1;
}
#endif

// Line Complex
int s52plib::RenderLC(ObjRazRules *rzRules, Rules *rules) {
  //if(rzRules->obj->Index != 1779)
    //return 0;

  // catch cm93 and legacy PlugIns (e.g.s63_pi)
  if (rzRules->obj->m_n_lsindex && !rzRules->obj->m_ls_list)
    return RenderLCLegacy(rzRules, rules);

  wxPoint r;

  int isym_len = rules->razRule->pos.line.bnbox_w.SYHL +
                 (rules->razRule->pos.line.bnbox_x.LBXC -
                  rules->razRule->pos.line.pivot_x.LICL);
  float sym_len = isym_len * canvas_pix_per_mm / 100;
  float sym_factor = 1.0;  /// 1.50;    // gives nicer effect

  //      Create a color for drawing adjustments outside of HPGL renderer
  char *tcolptr = rules->razRule->colRef.LCRF;
  S52color *c = getColor(tcolptr + 1);  // +1 skips "n" in HPGL SPn format
  int w = 1;                            // arbitrary width
  wxColour color(c->R, c->G, c->B);

  double meters_per_senc_unit = rzRules->obj->x_rate;     // meters per senc-unit
  double lod_2pixel_meters = 2 / vp_plib.view_scale_ppm;    // LOD set to 2 pixels, nominal mercator projected
  double LOD = lod_2pixel_meters / meters_per_senc_unit;

  //  Get the current display priority
  //  Default comes from the LUP, unless overridden
  int priority_current = rzRules->LUP->DPRI - '0';
  if (rzRules->obj->m_DPRI >= 0) priority_current = rzRules->obj->m_DPRI;

  if (rzRules->obj->m_n_lsindex) {
    // Calculate the size of a work buffer
    int max_points = 0;
    if (rzRules->obj->m_n_edge_max_points > 0)
      max_points = rzRules->obj->m_n_edge_max_points;
    else {
      line_segment_element *lsa = rzRules->obj->m_ls_list;

      while (lsa) {
        if ((lsa->ls_type == TYPE_EE) || (lsa->ls_type == TYPE_EE_REV))
          max_points += lsa->pedge->nCount;
        else
          max_points += 2;

        lsa = lsa->next;
      }
    }

    float *ppt;
    unsigned char *vbo_point =
        (unsigned char *)rzRules->obj->m_chart_context
            ->vertex_buffer;  // chart->GetLineVertexBuffer();

    //  Allocate some storage for converted points
    wxPoint *ptp = (wxPoint *)malloc((max_points) * sizeof(wxPoint));
    double *pdp = (double *)malloc(2 * (max_points) * sizeof(double));
    int *mask = (int *)malloc((max_points) * sizeof(int));

    line_segment_element *ls = rzRules->obj->m_ls_list;

    unsigned int index = 0;
    unsigned int idouble = 0;
    int nls = 0;
    wxPoint lp;

    int ndraw = 0;
    while (ls) {
      if ( ls->priority == priority_current) {
        // transcribe the segment in the proper order into the output buffer
        int nPoints;
        int idir = 1;
        bool bcon = false;
        // fetch the first point
        if ((ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV)) {
          ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
          nPoints = ls->pedge->nCount;
          if (ls->ls_type == TYPE_EE_REV) idir = -1;

        } else {
          ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
          nPoints = 2;
          bcon = true;
        }

        int vbo_index = 0;
        int vbo_inc = 2;
        if ((idir == -1) && !bcon) {
          vbo_index = (nPoints - 1) * 2;
          vbo_inc = -2;
        }

        double offset = 0;
        for (int ip = 0; ip < nPoints; ip++) {
          wxPoint r;
          GetPointPixSingle(rzRules, ppt[vbo_index + 1], ppt[vbo_index], &r);
          if ((r.x != lp.x) || (r.y != lp.y)) {
            mask[index] = (ls->priority == priority_current) ? 1 : 0;
            ptp[index++] = r;
            pdp[idouble++] = ppt[vbo_index];
            pdp[idouble++] = ppt[vbo_index + 1];

            nls++;
          } else {  // sKipping point
          }

          lp = r;
          vbo_index += vbo_inc;
        }

      }  // priority

      // inspect the next segment to see if it can be connected, or if the chain
      // breaks
      int idir = 1;
      bool bcon = false;
      if (ls->next) {
        int nPoints_next;
        line_segment_element *lsn = ls->next;
        // fetch the first point
        if ((lsn->ls_type == TYPE_EE) || (lsn->ls_type == TYPE_EE_REV)) {
          ppt = (float *)(vbo_point + lsn->pedge->vbo_offset);
          nPoints_next = lsn->pedge->nCount;
          if (lsn->ls_type == TYPE_EE_REV) idir = -1;

        } else {
          ppt = (float *)(vbo_point + lsn->pcs->vbo_offset);
          nPoints_next = 2;
          bcon = true;
        }

        wxPoint ptest;
        if (bcon)
          GetPointPixSingle(rzRules, ppt[1], ppt[0], &ptest);

        else {
          if (idir == 1)
            GetPointPixSingle(rzRules, ppt[1], ppt[0], &ptest);

          else {
            // fetch the last point
            int index_last_next = (nPoints_next - 1) * 2;
            GetPointPixSingle(rzRules, ppt[index_last_next + 1],
                              ppt[index_last_next], &ptest);
          }
        }

        // try to match the correct point in this segment with the last point in
        // the previous segment

        if (lp != ptest)  // not connectable?
        {
          if (nls) {
            wxPoint2DDouble *pReduced = 0;
            int *pMaskOut = 0;
            int nPointReduced =
                reduceLOD(LOD, nls, pdp, &pReduced, mask, &pMaskOut);

            wxPoint *ptestp = (wxPoint *)malloc((max_points) * sizeof(wxPoint));
            GetPointPixArray(rzRules, pReduced, ptestp, nPointReduced);
            free(pReduced);

            draw_lc_poly(m_pdc, color, w, ptestp, pMaskOut, nPointReduced,
                         sym_len, sym_factor, rules->razRule);
            free(ptestp);
            free(pMaskOut);

            ndraw++;
          }

          nls = 0;
          index = 0;
          idouble = 0;
          lp = wxPoint(0, 0);
        }

      } else {
        // no more segments, so render what is available
        if (nls) {
          wxPoint2DDouble *pReduced = 0;
          int *pMaskOut = 0;
          int nPointReduced =
              reduceLOD(LOD, nls, pdp, &pReduced, mask, &pMaskOut);

          wxPoint *ptestp = (wxPoint *)malloc((max_points) * sizeof(wxPoint));
          GetPointPixArray(rzRules, pReduced, ptestp, nPointReduced);
          free(pReduced);

          draw_lc_poly(m_pdc, color, w, ptestp, pMaskOut, nPointReduced,
                       sym_len, sym_factor, rules->razRule);
          free(ptestp);
          free(pMaskOut);
        }
      }

      ls = ls->next;
    }

    free(ptp);
    free(pdp);
    free(mask);
  }

  return 1;
}

int s52plib::reduceLOD(double LOD_meters, int nPoints, double *source,
                       wxPoint2DDouble **dest, int *maskIn, int **maskOut) {
  //      Reduce the LOD of this linestring
  std::vector<int> index_keep;
  if (nPoints > 5 && (LOD_meters > .01)) {
    index_keep.push_back(0);
    index_keep.push_back(nPoints - 1);
    index_keep.push_back(nPoints - 2);

    DouglasPeucker(source, 0, nPoints - 2, LOD_meters, &index_keep);

  } else {
    index_keep.resize(nPoints);
    // Consider using std::iota here when there is C++11 support.
    for (int i = 0; i < nPoints; i++) index_keep[i] = i;
  }

  wxPoint2DDouble *pReduced =
      (wxPoint2DDouble *)malloc((index_keep.size()) * sizeof(wxPoint2DDouble));
  *dest = pReduced;

  int *pmaskOut = NULL;
  if (maskIn) {
    *maskOut = (int *)malloc((index_keep.size()) * sizeof(int));
    pmaskOut = *maskOut;
  }

  double *ppr = source;
  int ir = 0;
  for (int ip = 0; ip < nPoints; ip++) {
    double x = *ppr++;
    double y = *ppr++;
    int maskval = 1;
    if (maskIn) maskval = maskIn[ip];
    // printf("LOD:  %10g  %10g\n", x, y);

    for (unsigned int j = 0; j < index_keep.size(); j++) {
      if (index_keep[j] == ip) {
        if (pmaskOut) pmaskOut[ir] = maskval;
        pReduced[ir++] = wxPoint2DDouble(x, y);
        break;
      }
    }
  }

  return index_keep.size();
}

// Line Complex
int s52plib::RenderLCLegacy(ObjRazRules *rzRules, Rules *rules) {
  if (!rzRules->obj->m_chart_context->chart)
    return RenderLCPlugIn(rzRules, rules);

  //  Must be cm93

  wxPoint r;

  int isym_len = rules->razRule->pos.line.bnbox_w.SYHL;
  float sym_len = isym_len * canvas_pix_per_mm / 100;
  float sym_factor = 1.0;  /// 1.50;                        // gives nicer
                           /// effect

  //      Create a color for drawing adjustments outside of HPGL renderer
  char *tcolptr = rules->razRule->colRef.LCRF;
  S52color *c = getColor(tcolptr + 1);  // +1 skips "n" in HPGL SPn format
  int w = 1;                            // arbitrary width
  wxColour color(c->R, c->G, c->B);

  //  Get the current display priority
  //  Default comes from the LUP, unless overridden
  int priority_current = rzRules->LUP->DPRI - '0';
  if (rzRules->obj->m_DPRI >= 0) priority_current = rzRules->obj->m_DPRI;

  if (rzRules->obj->m_n_lsindex) {
    auto ve_hash = rzRules->obj->m_chart_context->m_pve_hash;
    auto vc_hash = rzRules->obj->m_chart_context->m_pvc_hash;

    unsigned int nls_max;
    if (rzRules->obj->m_n_edge_max_points >
        0)  // size has been precalculated on SENC load
      nls_max = rzRules->obj->m_n_edge_max_points;
    else {
      //  Calculate max malloc size required
      nls_max = 0;
      int *index_run_x = rzRules->obj->m_lsindex_array;
      for (int imseg = 0; imseg < rzRules->obj->m_n_lsindex; imseg++) {
        index_run_x++;  // Skip cNode
        //  Get the edge
        unsigned int enode = *index_run_x;
        if (enode) {
          VE_Element *pedge = (*ve_hash)[enode];
          if (pedge) {
            if (pedge->nCount > nls_max) nls_max = pedge->nCount;
          }
        }
        index_run_x += 2;
      }
      rzRules->obj->m_n_edge_max_points =
          nls_max;  // Got it, cache for next time
    }

    //  Allocate some storage for converted points
    wxPoint *ptp = (wxPoint *)malloc(
        (nls_max + 2) * sizeof(wxPoint));  // + 2 allows for end nodes

    int *index_run;
    float *ppt;
    double easting, northing;
    wxPoint pra(0, 0);
    VC_Element *pnode;

    for (int iseg = 0; iseg < rzRules->obj->m_n_lsindex; iseg++) {
      int seg_index = iseg * 3;
      index_run = &rzRules->obj->m_lsindex_array[seg_index];

      //  Get first connected node
      unsigned int inode = *index_run++;
      if (inode) {
        pnode = (*vc_hash)[inode];
        if (pnode) {
          ppt = pnode->pPoint;
          easting = *ppt++;
          northing = *ppt;
          GetPointPixSingle(rzRules, (float)northing, (float)easting, &pra);
        }
        ptp[0] = pra;  // insert beginning node
      }

      //  Get the edge
      unsigned int enode = *index_run++;
      VE_Element *pedge = 0;
      if (enode) pedge = (*ve_hash)[enode];

      int nls = 0;
      if (pedge) {
        //  Here we decide to draw or not based on the highest priority seen for
        //  this segment That is, if this segment is going to be drawn at a
        //  higher priority later, then don't draw it here.

        // This logic is not perfectly right for one case:
        // If the segment has only two end connected nodes, and no intermediate
        // edge, then we have no good way to evaluate the priority. This is due
        // to the fact that priority is only precalculated for edge segments,
        // not connector nodes. Only thing to do is take the conservative
        // approach and draw the segment, in this case.
        if (pedge->nCount) {
          if (pedge->max_priority != priority_current) continue;
        }

        if (pedge->max_priority != priority_current) continue;

        nls = pedge->nCount;

        ppt = pedge->pPoints;
        for (int ip = 0; ip < nls; ip++) {
          easting = *ppt++;
          northing = *ppt++;
          GetPointPixSingle(rzRules, (float)northing, (float)easting,
                            &ptp[ip + 1]);
        }
      }

      //  Get last connected node
      unsigned int jnode = *index_run++;
      if (jnode) {
        pnode = (*vc_hash)[jnode];
        if (pnode) {
          ppt = pnode->pPoint;
          easting = *ppt++;
          northing = *ppt;
          GetPointPixSingle(rzRules, (float)northing, (float)easting, &pra);
        }
        ptp[nls + 1] = pra;  // insert ending node
      }

      if ((inode) && (jnode)) {
        draw_lc_poly(m_pdc, color, w, ptp, NULL, nls + 2, sym_len, sym_factor,
                     rules->razRule);
      } else if (nls) {
        draw_lc_poly(m_pdc, color, w, &ptp[1], NULL, nls, sym_len, sym_factor,
                     rules->razRule);
      }
    }
    free(ptp);
  }

  else if (rzRules->obj->pPolyTessGeo) {
    if (!rzRules->obj->pPolyTessGeo->IsOk()) {  // perform deferred tesselation
      rzRules->obj->pPolyTessGeo->BuildDeferredTess();
    }

    PolyTriGroup *pptg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
    float *ppolygeo = pptg->pgroup_geom;
    if (ppolygeo) {
      int ctr_offset = 0;
      for (int ic = 0; ic < pptg->nContours; ic++) {
        int npt = pptg->pn_vertex[ic];
        wxPoint *ptp = (wxPoint *)malloc((npt + 1) * sizeof(wxPoint));
        wxPoint *pr = ptp;
        for (int ip = 0; ip < npt; ip++) {
          float plon = ppolygeo[(2 * ip) + ctr_offset];
          float plat = ppolygeo[(2 * ip) + ctr_offset + 1];

          GetPointPixSingle(rzRules, plat, plon, pr);
          pr++;
        }
        float plon = ppolygeo[ctr_offset];  // close the polyline
        float plat = ppolygeo[ctr_offset + 1];
        GetPointPixSingle(rzRules, plat, plon, pr);

        draw_lc_poly(m_pdc, color, w, ptp, NULL, npt + 1, sym_len, sym_factor,
                     rules->razRule);

        free(ptp);

        ctr_offset += npt * 2;
      }
    }
  }
  return 1;
}

int s52plib::RenderLCPlugIn(ObjRazRules *rzRules, Rules *rules) {
  wxPoint r;

  int isym_len = rules->razRule->pos.line.bnbox_w.SYHL;
  float sym_len = isym_len * canvas_pix_per_mm / 100;
  float sym_factor = 1.0;  /// 1.50;                        // gives nicer
                           /// effect

  //      Create a color for drawing adjustments outside of HPGL renderer
  char *tcolptr = rules->razRule->colRef.LCRF;
  S52color *c = getColor(tcolptr + 1);  // +1 skips "n" in HPGL SPn format
  int w = 1;                            // arbitrary width
  wxColour color(c->R, c->G, c->B);
  double LOD = 2.0 / vp_plib.view_scale_ppm;  // empirical value, by experiment
  LOD = 0;                                // wxMin(LOD, 10.0);

  //  Get the current display priority
  //  Default comes from the LUP, unless overridden
  int priority_current = rzRules->LUP->DPRI - '0';
  if (rzRules->obj->m_DPRI >= 0) priority_current = rzRules->obj->m_DPRI;

  //  Calculate max malloc size required

  int max_points = 0;
  if (rzRules->obj->m_ls_list_legacy) {
    VE_Element *pedge;
    PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;

    while (ls) {
      int nPoints;
      // fetch the first point
      if (ls->type == TYPE_EE) {
        pedge = (VE_Element *)ls->private0;
        nPoints = pedge->nCount;
      } else {
        nPoints = 2;
      }

      max_points += nPoints;

      ls = ls->next;
    }
  }

  if (rzRules->obj->m_ls_list_legacy) {
    float *ppt;

    VE_Element *pedge;

    //  Allocate some storage for converted points
    wxPoint *ptp = (wxPoint *)malloc(
        (max_points + 2) * sizeof(wxPoint));  // + 2 allows for end nodes
    double *pdp = (double *)malloc(2 * (max_points + 2) * sizeof(double));

    PI_connector_segment *pcs;

    unsigned char *vbo_point =
        (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
    PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;

    unsigned int index = 0;
    unsigned int idouble = 0;
    int nls = 0;
    wxPoint lp;

    ls = rzRules->obj->m_ls_list_legacy;
    while (ls) {
      if (ls->priority == priority_current) {
        // transcribe the segment in the proper order into the output buffer
        int nPoints;
        int idir = 1;
        bool bcon = false;
        // fetch the first point
        if ((ls->type == TYPE_EE) || (ls->type == TYPE_EE_REV)) {
          pedge = (VE_Element *)ls->private0;
          ppt = (float *)(vbo_point + pedge->vbo_offset);
          nPoints = pedge->nCount;
          if (ls->type == TYPE_EE_REV) idir = -1;

        } else {
          pcs = (PI_connector_segment *)ls->private0;
          ppt = (float *)(vbo_point + pcs->vbo_offset);
          nPoints = 2;
          bcon = true;
        }

        int vbo_index = 0;
        int vbo_inc = 2;
        if ((idir == -1) && !bcon) {
          vbo_index = (nPoints - 1) * 2;
          vbo_inc = -2;
        }
        for (int ip = 0; ip < nPoints; ip++) {
          wxPoint r;
          GetPointPixSingle(rzRules, ppt[vbo_index + 1], ppt[vbo_index], &r);

          if (1 /*(r.x != lp.x) || (r.y != lp.y)*/) {
            ptp[index++] = r;
            pdp[idouble++] = ppt[vbo_index];
            pdp[idouble++] = ppt[vbo_index + 1];

            nls++;
          } else {  // sKipping point
          }

          lp = r;
          vbo_index += vbo_inc;
        }

      }  // priority

      // inspect the next segment to see if it can be connected, or if the chain
      // breaks
      int idir = 1;
      bool bcon = false;
      if (ls->next) {
        int nPoints_next;
        PI_line_segment_element *lsn = ls->next;
        // fetch the first point
        if ((lsn->type == TYPE_EE) || (lsn->type == TYPE_EE_REV)) {
          pedge = (VE_Element *)lsn->private0;
          ppt = (float *)(vbo_point + pedge->vbo_offset);
          nPoints_next = pedge->nCount;
          if (lsn->type == TYPE_EE_REV) idir = -1;

        } else {
          pcs = (PI_connector_segment *)lsn->private0;
          ppt = (float *)(vbo_point + pcs->vbo_offset);
          nPoints_next = 2;
          bcon = true;
        }

        wxPoint ptest;
        if (bcon)
          GetPointPixSingle(rzRules, ppt[1], ppt[0], &ptest);

        else {
          if (idir == 1)
            GetPointPixSingle(rzRules, ppt[1], ppt[0], &ptest);

          else {
            // fetch the last point
            int index_last_next = (nPoints_next - 1) * 2;
            GetPointPixSingle(rzRules, ppt[index_last_next + 1],
                              ppt[index_last_next], &ptest);
          }
        }

        // try to match the correct point in this segment with the last point in
        // the previous segment

        if (lp != ptest)  // not connectable?
        {
          if (nls) {
            wxPoint2DDouble *pReduced = 0;
            int nPointReduced = reduceLOD(LOD, nls, pdp, &pReduced, NULL, NULL);

            wxPoint *ptestp =
                (wxPoint *)malloc((2 * (nPointReduced + 2)) * sizeof(wxPoint));
            GetPointPixArray(rzRules, pReduced, ptestp, nPointReduced);
            free(pReduced);

            draw_lc_poly(m_pdc, color, w, ptestp, NULL, nPointReduced, sym_len,
                         sym_factor, rules->razRule);
            free(ptestp);
          }

          nls = 0;
          index = 0;
          idouble = 0;
          lp = wxPoint(0, 0);
        }

      } else {
        // no more segments, so render what is available
        if (nls) {
          wxPoint2DDouble *pReduced = 0;
          int nPointReduced = reduceLOD(LOD, nls, pdp, &pReduced, NULL, NULL);

          wxPoint *ptestp =
              (wxPoint *)malloc((2 * (max_points + 2)) * sizeof(wxPoint));
          GetPointPixArray(rzRules, pReduced, ptestp, nPointReduced);
          free(pReduced);

          draw_lc_poly(m_pdc, color, w, ptestp, NULL, nPointReduced, sym_len,
                       sym_factor, rules->razRule);
          free(ptestp);
        }
      }

      ls = ls->next;
    }

    free(ptp);
    free(pdp);
  }
  return 1;
}

//      Render Line Complex Polyline

void s52plib::draw_lc_poly(wxDC *pdc, wxColor &color, int width, wxPoint *ptp,
                           int *mask, int npt, float sym_len, float sym_factor,
                           Rule *draw_rule) {
  if (npt < 2) return;

  wxPoint r;

  //  We calculate the winding direction of the poly
  //  in order to know which side to draw symbol on
  double dfSum = 0.0;

  for (int iseg = 0; iseg < npt - 1; iseg++) {
    dfSum += ptp[iseg].x * ptp[iseg + 1].y - ptp[iseg].y * ptp[iseg + 1].x;
  }
  dfSum += ptp[npt - 1].x * ptp[0].y - ptp[npt - 1].y * ptp[0].x;

  bool cw = dfSum < 0.;

  //    Get a true pixel clipping/bounding box from the vp
  wxPoint pbb = GetPixFromLL(vp_plib.clat, vp_plib.clon);
  int xmin_ = pbb.x - vp_plib.rv_rect.width / 2;
  int xmax_ = xmin_ + vp_plib.rv_rect.width;
  int ymin_ = pbb.y - vp_plib.rv_rect.height / 2;
  int ymax_ = ymin_ + vp_plib.rv_rect.height;

  int x0, y0, x1, y1;

  if (pdc) {
    wxPen *pthispen =
        wxThePenList->FindOrCreatePen(color, width, wxPENSTYLE_SOLID);
    m_pdc->SetPen(*pthispen);

    int start_seg = 0;
    int end_seg = npt - 1;
    int inc = 1;

    if (cw) {
      start_seg = npt - 1;
      end_seg = 0;
      inc = -1;
    }

    float dx, dy, seg_len, theta;

    bool done = false;
    ClipResult res;
    int iseg = start_seg;
    while (!done) {
      // Do not bother with segments that are invisible

      x0 = ptp[iseg].x;
      y0 = ptp[iseg].y;
      x1 = ptp[iseg + inc].x;
      y1 = ptp[iseg + inc].y;

      //  Also, segments marked (by mask) as invisible
      if (mask && !mask[iseg]) goto next_seg_dc;

      res = cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, xmin_, xmax_,
                                         ymin_, ymax_);

      if (res == Invisible) goto next_seg_dc;

      dx = ptp[iseg + inc].x - ptp[iseg].x;
      dy = ptp[iseg + inc].y - ptp[iseg].y;
      seg_len = sqrt(dx * dx + dy * dy);
      theta = atan2f(dy, dx);

      if (seg_len >= 1.0) {
        if (seg_len <= sym_len * sym_factor) {
          int xst1 = ptp[iseg].x;
          int yst1 = ptp[iseg].y;
          float xst2, yst2;
          if (seg_len >= sym_len) {
            xst2 = xst1 + (sym_len * dx / seg_len);
            yst2 = yst1 + (sym_len * dy / seg_len);
          } else {
            xst2 = ptp[iseg + inc].x;
            yst2 = ptp[iseg + inc].y;
          }

          pdc->DrawLine(xst1, yst1, (wxCoord)floor(xst2), (wxCoord)floor(yst2));
        }

        else {
          float s = 0;
          float xs = ptp[iseg].x;
          float ys = ptp[iseg].y;

          while (s + (sym_len * sym_factor) < seg_len) {
            r.x = (int)xs;
            r.y = (int)ys;
            char *str = draw_rule->vector.LVCT;
            char *col = draw_rule->colRef.LCRF;
            wxPoint pivot(draw_rule->pos.line.pivot_x.LICL,
                          draw_rule->pos.line.pivot_y.LIRW);

            HPGL->SetTargetDC(pdc);
            HPGL->SetVP(&vp_plib);
            HPGL->Render(str, col, r, pivot, pivot, 1.0, theta * 180. / PI,
                         false);

            xs += sym_len * dx / seg_len * sym_factor;
            ys += sym_len * dy / seg_len * sym_factor;
            s += sym_len * sym_factor;
          }

          pdc->DrawLine((int)xs, (int)ys, ptp[iseg + inc].x, ptp[iseg + inc].y);
        }
      }
    next_seg_dc:
      iseg += inc;
      if (iseg == end_seg) done = true;

    }  // while
  }    // if pdc
  else  // opengl
  {
    //    Set up the color
#ifdef ocpnUSE_GL
    // Adjust line width up a bit, to improve render quality for
    // GL_BLEND/GL_LINE_SMOOTH
    float awidth = wxMax(m_GLMinCartographicLineWidth, (float)width * 0.7);
    awidth = wxMax(awidth, 1.5);
    glLineWidth(awidth);
#endif

    int start_seg = 0;
    int end_seg = npt - 1;
    int inc = 1;

    if (cw) {
      start_seg = npt - 1;
      end_seg = 0;
      inc = -1;
    }

    float dx, dy, seg_len, theta;
    ClipResult res;

    bool done = false;
    int iseg = start_seg;
    while (!done) {
      // Do not bother with segments that are invisible

      x0 = ptp[iseg].x;
      y0 = ptp[iseg].y;
      x1 = ptp[iseg + inc].x;
      y1 = ptp[iseg + inc].y;

      //  Also, segments marked (by mask) as invisible
      if (mask && !mask[iseg]) goto next_seg;

      res = cohen_sutherland_line_clip_i(&x0, &y0, &x1, &y1, xmin_, xmax_,
                                         ymin_, ymax_);

      if (res == Invisible) goto next_seg;

      dx = ptp[iseg + inc].x - ptp[iseg].x;
      dy = ptp[iseg + inc].y - ptp[iseg].y;
      seg_len = sqrt(dx * dx + dy * dy);

      if (seg_len >= 1.0) {
        if (seg_len <= sym_len * sym_factor) {
          int xst1 = ptp[iseg].x;
          int yst1 = ptp[iseg].y;
          float xst2, yst2;

          if (seg_len >= sym_len) {
            xst2 = xst1 + (sym_len * dx / seg_len);
            yst2 = yst1 + (sym_len * dy / seg_len);
          } else {
            xst2 = ptp[iseg + inc].x;
            yst2 = ptp[iseg + inc].y;
          }

          //      Enable anti-aliased lines, at best quality
#ifndef __OCPN__ANDROID__
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glEnable(GL_BLEND);

          if (m_GLLineSmoothing) {
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
          }
#endif

#ifdef ocpnUSE_GL
          CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
          shader->Bind();

          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

          float colorv[4];
          colorv[0] = color.Red() / float(256);
          colorv[1] = color.Green() / float(256);
          colorv[2] = color.Blue() / float(256);
          colorv[3] = 1.0;  // transparency;

          shader->SetUniform4fv("color", colorv);

          float pts[4];
          pts[0] = xst1;
          pts[1] = yst1;
          pts[2] = xst2;
          pts[3] = yst2;

          GLint pos = shader->getAttributeLocation("position");
          glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                                pts);
          glEnableVertexAttribArray(pos);

          glDrawArrays(GL_LINES, 0, 2);

          shader->UnBind();

          glDisable(GL_LINE_SMOOTH);
          glDisable(GL_BLEND);
#endif
        } else {
          float s = 0;
          float xs = ptp[iseg].x;
          float ys = ptp[iseg].y;

          while (s + (sym_len * sym_factor) < seg_len) {
            r.x = (int)xs;
            r.y = (int)ys;
            char *str = draw_rule->vector.LVCT;
            char *col = draw_rule->colRef.LCRF;
            wxPoint pivot(draw_rule->pos.line.pivot_x.LICL,
                          draw_rule->pos.line.pivot_y.LIRW);

            HPGL->SetTargetOpenGl();
            HPGL->SetVP(&vp_plib);
            theta = atan2f(dy, dx);
            HPGL->Render(str, col, r, pivot, pivot, 1.0, theta * 180. / PI,
                         false);

            xs += sym_len * dx / seg_len * sym_factor;
            ys += sym_len * dy / seg_len * sym_factor;
            s += sym_len * sym_factor;
          }
#ifndef __OCPN__ANDROID__
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glEnable(GL_BLEND);

          if (m_GLLineSmoothing) {
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
          }

#endif

#ifdef ocpnUSE_GL
          CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
          shader->Bind();

          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

          float colorv[4];
          colorv[0] = color.Red() / float(256);
          colorv[1] = color.Green() / float(256);
          colorv[2] = color.Blue() / float(256);
          colorv[3] = 1.0;  // transparency;

          shader->SetUniform4fv("color", colorv);

          float pts[4];
          pts[0] = xs;
          pts[1] = ys;
          pts[2] = ptp[iseg + inc].x;
          pts[3] = ptp[iseg + inc].y;

          GLint pos = shader->getAttributeLocation("position");
          glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                                pts);
          glEnableVertexAttribArray(pos);

          glDrawArrays(GL_LINES, 0, 2);
          glDisableVertexAttribArray(pos);
          shader->UnBind();

          glDisable(GL_LINE_SMOOTH);
          glDisable(GL_BLEND);
#endif
        }
      }
    next_seg:
      iseg += inc;
      if (iseg == end_seg) done = true;
    }  // while

  }  // opengl
}

// Multipoint Sounding
int s52plib::RenderMPS(ObjRazRules *rzRules, Rules *rules) {
  if (!m_bShowSoundg) return 0;

  if (m_bUseSCAMIN) {
    if (vp_plib.chart_scale > rzRules->obj->Scamin) return 0;
  }

  int npt = rzRules->obj->npt;

  // this should never happen
  // But it seems that some PlugIns clear the mps rules without resetting the CS
  // state machine So fix it
  if (rzRules->obj->bCS_Added && !rzRules->mps) rzRules->obj->bCS_Added = false;

  //  Build the cached rules list if necessary
  if (!rzRules->obj->bCS_Added) {
    ObjRazRules point_rzRules;
    point_rzRules = *rzRules;  // take a copy of attributes, etc

    S57Obj point_obj;
    point_obj = *(rzRules->obj);
    point_obj.bIsClone = true;
    point_rzRules.obj = &point_obj;

    Rules *ru_cs = StringToRules(_T ( "CS(SOUNDG03;" ));

    wxPoint p;
    double *pd = rzRules->obj->geoPtz;       // the SM points
    double *pdl = rzRules->obj->geoPtMulti;  // and corresponding lat/lon

    mps_container *pmps = (mps_container *)calloc(sizeof(mps_container), 1);
    pmps->cs_rules = new ArrayOfRules;
    rzRules->mps = pmps;

    for (int ip = 0; ip < npt; ip++) {
      double east = *pd++;
      double nort = *pd++;
      double depth = *pd++;

      point_obj.x = east;
      point_obj.y = nort;
      point_obj.z = depth;

      double lon = *pdl++;
      double lat = *pdl++;
      point_obj.BBObj.Set(lat, lon, lat, lon);
      point_obj.BBObj.Invalidate();

      char *rule_str1 = RenderCS(&point_rzRules, ru_cs);
      wxString cs_string(rule_str1, wxConvUTF8);
      free(rule_str1);

      Rules *rule_chain = StringToRules(cs_string);

      rzRules->mps->cs_rules->Add(rule_chain);
    }

    DestroyRulesChain(ru_cs);
    rzRules->obj->bCS_Added = 1;  // mark the object
  }

  double *pdl = rzRules->obj->geoPtMulti;  // and corresponding lat/lon
  double *pd = rzRules->obj->geoPtz;       // the SM points

  // FIXME (plib)
  // Need to un-rotate sounding rendering
  //  We need a private unrotated copy of the Viewport
  //VPointCompat vp_local = *vp;
  //vp_local.SetRotationAngle(0.);

  SetupSoundingFont();
  // Measure the size of a character
  int charWidth, charHeight;
  wxScreenDC sdc;
  sdc.GetTextExtent("0", &charWidth, &charHeight, NULL, NULL, m_soundFont);
  wxRect soundBox = wxRect(0, 0, charWidth * 4, charHeight * 3/2);

  // We need a pixel bounding rectangle of the passed ViewPort.
  // Very important for partial screen renders, as with dc mode pans or OpenGL
  // FBO operation.

  wxPoint cr0 = GetPixFromLLROT(GetReducedBBox().GetMaxLat(),
                                      GetReducedBBox().GetMinLon(), 0);
  wxPoint cr1 = GetPixFromLLROT(GetReducedBBox().GetMinLat(),
                                      GetReducedBBox().GetMaxLon(), 0);
  wxRect clip_rect(cr0, cr1);

  double box_margin = wxMax(fabs(GetBBox().GetMaxLon() - GetBBox().GetMinLon()),
                              fabs(GetBBox().GetMaxLat() - GetBBox().GetMinLat()));
  LLBBox screen_box = GetBBox();

  for (int ip = 0; ip < npt; ip++) {
    double lon = *pdl++;
    double lat = *pdl++;

    double east = *pd++;
    double nort = *pd++;
    double depth = *pd++;

    // Make a rough inclusion test from lat/lon
    // onto the screen coordinates, enlarged a bit
    if (!screen_box.ContainsMarge(lat, lon, box_margin))
      continue;

    // Some simple inclusion tests
    wxPoint r = GetPixFromLLROT(lat, lon, 0);
    if ((r.x == INVALID_COORD) || (r.y == INVALID_COORD))
      continue;

    // Use measured symbol size
    wxRect rr(r.x - (soundBox.width / 2), r.y - (soundBox.height / 2), soundBox.width, soundBox.height);

    //      After all the setup, the render inclusion test is trivial....
    if (!clip_rect.Intersects(rr)) continue;

    double angle = 0;
    if (depth < 0) int yyp = 4;

    Rules *rules = rzRules->mps->cs_rules->Item(ip);
    bool bColorSet = false;
    wxColor symColor;
    GetGlobalColor(_T("SNDG2"), &symColor);

    while (rules) {
      //  Render a raster or vector symbol, as specified by LUP rules
      if (rules->razRule->definition.SYDF == 'V') {
        // On OpenGL, arrange to render the drying height "underline" symbol as
        // un-rotated.
        double dryAngle = 0;
        if (!m_pdc && !strncmp(rules->razRule->name.SYNM, "SOUNDSA1", 8))
          dryAngle = -vp_plib.rotation * 180. / PI;
        // FIXME (dave) drying height symbol should be wider/bolder.

        RenderHPGL(rzRules, rules->razRule, r, dryAngle, m_SoundingsScaleFactor);
      } else if (rules->razRule->definition.SYDF == 'R') {
        // Parse the first rule to determine the color
        if (!bColorSet) {
          char symColorT = rules->razRule->name.SYNM[5];
          if (symColorT == 'G') GetGlobalColor(_T("SNDG1"), &symColor);
          bColorSet = true;
        }

        if (!strncmp(rules->razRule->name.SYNM, "SOUNDGC2", 8))
          RenderRasterSymbol(rzRules, rules->razRule, r, angle);
        else
          RenderSoundingSymbol(rzRules, rules->razRule, r, symColor, angle);
      }

      // Debug
      //if(m_pdc){
        //m_pdc->SetPen(wxPen(*wxRED, 1));
        //m_pdc->SetBrush(wxBrush(*wxRED, wxTRANSPARENT));
        //m_pdc->DrawRectangle(rr);
      //}

      rules = rules->next;
    }
  }
  return 1;
}

int s52plib::RenderCARC(ObjRazRules *rzRules, Rules *rules) {
#ifdef ocpnUSE_GL
  if (m_useGLSL)
    return RenderCARC_GLSL(rzRules, rules);
  else
#endif
    return RenderCARC_VBO(rzRules, rules);
}

#ifdef ocpnUSE_GL
int s52plib::RenderCARC_GLSL(ObjRazRules *rzRules, Rules *rules) {

  //    glDisable( GL_SCISSOR_TEST );

  char *str = (char *)rules->INSTstr;
  //    extract the parameters from the string
  //    And creating a unique string hash as we go
  wxString inst(str, wxConvUTF8);

  wxStringTokenizer tkz(inst, _T ( ",;" ));

  //    outline color
  wxString outline_color = tkz.GetNextToken();

  //    outline width
  wxString slong = tkz.GetNextToken();
  long outline_width;
  slong.ToLong(&outline_width);

  //    arc color
  wxString arc_color = tkz.GetNextToken();

  //    arc width
  slong = tkz.GetNextToken();
  long arc_width;
  slong.ToLong(&arc_width);

  //    sectr1
  slong = tkz.GetNextToken();
  double sectr1;
  slong.ToDouble(&sectr1);

  //    sectr2
  slong = tkz.GetNextToken();
  double sectr2;
  slong.ToDouble(&sectr2);

  //    arc radius
  slong = tkz.GetNextToken();
  long radius;
  slong.ToLong(&radius);

  //    sector radius
  slong = tkz.GetNextToken();
  long sector_radius;
  slong.ToLong(&sector_radius);

  // Center point
  wxPoint r;
  GetPointPixSingle(rzRules, rzRules->obj->y, rzRules->obj->x, &r);

  if (radius > m_display_size_mm / 10){
    double fact = radius / (m_display_size_mm / 10);
    radius /= fact;
    sector_radius /= fact;
    arc_width /= fact;
    arc_width = wxMax(arc_width, 1);
  }

  //  radius scaled to display
  float rad = radius * canvas_pix_per_mm;
  float sec_rad = sector_radius * canvas_pix_per_mm;

  float arcw = arc_width * canvas_pix_per_mm;
  // On larger screens, make the arc_width 1.0 mm
  if ( m_display_size_mm > 200)     //200 mm, about 8 inches
    arcw = canvas_pix_per_mm;


  // Adjust size
  //  Some lights have no SCAMIN attribute. e.g. cm93
  //  This causes display congestion at small viewing scales, since the objects
  //  are rendered at fixed pixel dimensions from the LUP rules. As a
  //  correction, the idea is to not allow the rendered symbol to be larger than
  //  "X" meters on the chart.
  //   and scale it down when rendered if necessary.

  float xscale = 1.0;
  if (rzRules->obj->Scamin > 1e8) {  // huge (unset) SCAMIN)
    float radius_meters_target = 1000;

    float radius_meters = (radius * canvas_pix_per_mm) / vp_plib.view_scale_ppm;

    xscale = radius_meters_target / radius_meters;
    xscale = wxMin(xscale, 1.0);
    xscale = wxMax(.5, xscale);

    rad *= xscale;
    arcw *= xscale;
    arcw = wxMin(arcw, rad / 10);
    sec_rad *= xscale;
  }

  //      Enable anti-aliased lines, at best quality
  glEnable(GL_BLEND);

  // Rotate the center point about vp center
  wxPoint point = r;
  double sin_rot = sin(vp_plib.rotation);
  double cos_rot = cos(vp_plib.rotation);

  double xp = ((point.x - vp_plib.pix_width / 2) * cos_rot) -
              ((point.y - vp_plib.pix_height / 2) * sin_rot);
  double yp = ((point.x - vp_plib.pix_width / 2) * sin_rot) +
              ((point.y - vp_plib.pix_height / 2) * cos_rot);

  point.x = (int)xp + vp_plib.pix_width / 2;
  point.y = (int)yp + vp_plib.pix_height / 2;

  float rad_fluff = rad + 20;
  float coords[8];
  coords[0] = -rad_fluff;
  coords[1] = rad_fluff;
  coords[2] = rad_fluff;
  coords[3] = rad_fluff;
  coords[4] = -rad_fluff;
  coords[5] = -rad_fluff;
  coords[6] = rad_fluff;
  coords[7] = -rad_fluff;

  glUseProgram(S52ring_shader_program);

  // Get pointers to the attributes in the program.
  GLint mPosAttrib = glGetAttribLocation(S52ring_shader_program, "aPos");

  // Disable VBO's (vertex buffer objects) for attributes.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords);
  glEnableVertexAttribArray(mPosAttrib);

  //  Circle radius
  GLint radiusloc =
      glGetUniformLocation(S52ring_shader_program, "circle_radius");
  glUniform1f(radiusloc, rad);

  //  Circle center point, physical
  GLint centerloc =
      glGetUniformLocation(S52ring_shader_program, "circle_center");
  float ctrv[2];
  ctrv[0] = point.x;
  ctrv[1] = vp_plib.pix_height - point.y;
  glUniform2fv(centerloc, 1, ctrv);

  //  Border color
  float bcolorv[4];
  bcolorv[0] = 0;
  bcolorv[1] = 0;
  bcolorv[2] = 0;
  bcolorv[3] = 1.0;

  GLint ring_colloc = glGetUniformLocation(S52ring_shader_program, "ring_color");
  glUniform4fv(ring_colloc, 1, bcolorv);

  //  Border Width
  GLint ringWidthloc =
      glGetUniformLocation(S52ring_shader_program, "ring_width");
  glUniform1f(ringWidthloc, arcw + (1 * outline_width));

  //  Visible sectors, rotated to vp orientation
  float sr1 = sectr1 + (vp_plib.rotation * 180 / PI);
  if (sr1 > 360.) sr1 -= 360.;
  float sr2 = sectr2 + (vp_plib.rotation * 180 / PI);
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

  GLint sector1loc = glGetUniformLocation(S52ring_shader_program, "sector_1");
  glUniform1f(sector1loc, (sb * PI / 180.));
  GLint sector2loc = glGetUniformLocation(S52ring_shader_program, "sector_2");
  glUniform1f(sector2loc, (se * PI / 180.));

  // Rotate and translate
  mat4x4 I;
  mat4x4_identity(I);

  mat4x4_translate_in_place(I, r.x, r.y, 0);

  GLint matloc =
      glGetUniformLocation(S52ring_shader_program, "TransformMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)I);

  // Perform the actual drawing.
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Now draw color ring
  //  Circle color
  wxColour colorb = getwxColour(arc_color);
  float colorv[4];
  colorv[0] = colorb.Red() / float(256);
  colorv[1] = colorb.Green() / float(256);
  colorv[2] = colorb.Blue() / float(256);
  colorv[3] = 1.0;
  glUniform4fv(ring_colloc, 1, colorv);

  //  arc Width
  glUniform1f(ringWidthloc, arcw);

   // Perform the actual drawing.
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


  // Restore the per-object transform to Identity Matrix
  mat4x4 IM;
  mat4x4_identity(IM);
  GLint matlocf =
      glGetUniformLocation(S52ring_shader_program, "TransformMatrix");
  glUniformMatrix4fv(matlocf, 1, GL_FALSE, (const GLfloat *)IM);
  glUseProgram(0);
  glDisableVertexAttribArray(mPosAttrib);


  //    Draw the sector legs directly on the target DC
  if (sector_radius > 0) {
    int leg_len = (int)(sec_rad);

    wxDash dash1[2];
    dash1[0] = (int)(3.6 * canvas_pix_per_mm / 3);  // 8// Long dash <---------+
    dash1[1] = (int)(1.8 * canvas_pix_per_mm / 3);  // 2// Short gap |

    wxPen thispen = *wxBLACK_PEN;
    thispen.SetDashes(2, dash1);
    thispen.SetWidth(2);
    thispen.SetStyle(wxPENSTYLE_USER_DASH);

    float a = (sectr1 - 90) * PI / 180;
    a += vp_plib.rotation;
    int x = point.x + (int)(leg_len * cosf(a));
    int y = point.y + (int)(leg_len * sinf(a));
    DrawDashLine(thispen, point.x, point.y, x, y);

    a = (sectr2 - 90) * PI / 180.;
    a += vp_plib.rotation;
    x = point.x + (int)(leg_len * cosf(a));
    y = point.y + (int)(leg_len * sinf(a));
    DrawDashLine(thispen, point.x, point.y, x, y);
  }
  glDisable(GL_BLEND);

  //  Update the object Bounding box,
  //  so that subsequent drawing operations will redraw the item fully

  double latmin, lonmin, latmax, lonmax;

  GetPixPointSingleNoRotate(r.x - rad, r.y + rad, &latmin, &lonmin);
  GetPixPointSingleNoRotate(r.x + rad, r.y - rad, &latmax, &lonmax);
  LLBBox symbox;
  symbox.Set(latmin, lonmin, latmax, lonmax);
  rzRules->obj->BBObj.Expand(symbox);

  //    glEnable( GL_SCISSOR_TEST );

  return 1;
}
#endif

int s52plib::RenderCARC_VBO(ObjRazRules *rzRules, Rules *rules) {

  char *str = (char *)rules->INSTstr;
  //    extract the parameters from the string
  //    And creating a unique string hash as we go
  wxString inst(str, wxConvUTF8);
  wxString carc_hash;

  wxStringTokenizer tkz(inst, _T ( ",;" ));

  //    outline color
  wxString outline_color = tkz.GetNextToken();
  carc_hash += outline_color;
  carc_hash += _T(".");

  //    outline width
  wxString slong = tkz.GetNextToken();
  long outline_width;
  slong.ToLong(&outline_width);
  carc_hash += slong;
  carc_hash += _T(".");

  //    arc color
  wxString arc_color = tkz.GetNextToken();
  carc_hash += arc_color;
  carc_hash += _T(".");

  //    arc width
  slong = tkz.GetNextToken();
  long arc_width;
  slong.ToLong(&arc_width);
  carc_hash += slong;
  carc_hash += _T(".");

  //    sectr1
  slong = tkz.GetNextToken();
  double sectr1;
  slong.ToDouble(&sectr1);
  carc_hash += slong;
  carc_hash += _T(".");

  //    sectr2
  slong = tkz.GetNextToken();
  double sectr2;
  slong.ToDouble(&sectr2);
  carc_hash += slong;
  carc_hash += _T(".");

  //    arc radius
  slong = tkz.GetNextToken();
  long radius;
  slong.ToLong(&radius);
  carc_hash += slong;
  carc_hash += _T(".");

  //    sector radius
  slong = tkz.GetNextToken();
  long sector_radius;
  slong.ToLong(&sector_radius);
  carc_hash += slong;
  carc_hash += _T(".");

  slong.Printf(_T("%d"), m_colortable_index);
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

  // float pd_scale = 1.0;
  float nominal_arc_width_pix = wxMax(
      1.0,
      floor(GetPPMM() * wx));  // { wx } mm nominal, but not less than 1 pixel
  // pd_scale = nominal_arc_width_pix / arc_width;

  // scale_factor *= pd_scale;
  // qDebug() << GetPPMM() << arc_width << nominal_arc_width_pix << pd_scale;

  // Adjust size
  //  Some plain lights have no SCAMIN attribute.
  //  This causes display congestion at small viewing scales, since the objects
  //  are rendered at fixed pixel dimensions from the LUP rules. As a
  //  correction, the idea is to not allow the rendered symbol to be larger than
  //  "X" meters on the chart.
  //   and scale it down when rendered if necessary.

  float xscale = 1.0;
  if (rzRules->obj->Scamin > 1e8) {  // huge (unset) SCAMIN)
    float radius_meters_target = 200;

    float radius_meters = (radius * canvas_pix_per_mm) / vp_plib.view_scale_ppm;

    xscale = radius_meters_target / radius_meters;
    xscale = wxMin(xscale, 1.0);
    xscale = wxMax(.4, xscale);

    radius *= xscale;
    sector_radius *= xscale;
  }

  /// scale_factor *= xscale;

  carc_hash += _T(".");
  wxString xs;
  xs.Printf(_T("%5g"), xscale);
  carc_hash += xs;

  if (m_pdc) {  // DC rendering
    if (fabs(prule->parm7 - xscale) > .00001) {
      ClearRulesCache(prule);
    }

    // Instantiate the symbol if necessary
    if ((rules->razRule->pixelPtr == NULL) ||
        (rules->razRule->parm1 != m_colortable_index)) {
      //  Render the sector light to a bitmap

      rad = (int)(radius * canvas_pix_per_mm);

      width = (rad * 2) + 28;
      height = (rad * 2) + 28;
      wxBitmap bm(width, height, -1);
      wxMemoryDC mdc;
      mdc.SelectObject(bm);
      mdc.SetBackground(wxBrush(m_unused_wxColor));
      mdc.Clear();

      //    Adjust sector math for wxWidgets API
      float sb;
      float se;

      //      For some reason, the __WXMSW__ build flips the sense of
      //      start and end angles on DrawEllipticArc()
#ifndef __WXMSW__
      if (sectr2 > sectr1) {
        sb = 90 - sectr1;
        se = 90 - sectr2;
      } else {
        sb = 360 + (90 - sectr1);
        se = 90 - sectr2;
      }
#else
      if (sectr2 > sectr1) {
        se = 90 - sectr1;
        sb = 90 - sectr2;
      } else {
        se = 360 + (90 - sectr1);
        sb = 90 - sectr2;
      }
#endif

      //      Here is a goofy way of computing the dc drawing extents exactly
      //      Draw a series of fat line segments approximating the arc using
      //      dc.DrawLine() This will properly establish the drawing box in the
      //      dc

      int border_fluff = 4;  // by how much should the blit bitmap be "fluffed"

      //  wxDC min/max calculations are currently broken in wxQT, so we use the
      //  entire circle instead of arcs...
#ifndef __WXQT__
      if (fabs(sectr2 - sectr1) != 360)  // not necessary for all-round lights
      {
        mdc.ResetBoundingBox();

        wxPen *pblockpen =
            wxThePenList->FindOrCreatePen(*wxBLACK, 10, wxPENSTYLE_SOLID);
        mdc.SetPen(*pblockpen);

        float start_angle, end_angle;
        if (se < sb) {
          start_angle = se;
          end_angle = sb;
        } else {
          start_angle = sb;
          end_angle = se;
        }

        int x0 = (width / 2) + (int)(rad * cos(start_angle * PI / 180.));
        int y0 = (height / 2) - (int)(rad * sin(start_angle * PI / 180.));
        for (float a = start_angle + .1; a <= end_angle; a += 2.0) {
          int x = (width / 2) + (int)(rad * cosf(a * PI / 180.));
          int y = (height / 2) - (int)(rad * sinf(a * PI / 180.));
          mdc.DrawLine(x0, y0, x, y);
          x0 = x;
          y0 = y;
        }

        bm_width = (mdc.MaxX() - mdc.MinX()) + (border_fluff * 2);
        bm_height = (mdc.MaxY() - mdc.MinY()) + (border_fluff * 2);
        bm_orgx = mdc.MinX() - border_fluff -
                  width / 2;  // wxMax ( 0, mdc.MinX()-border_fluff );
        bm_orgy = mdc.MinY() - border_fluff -
                  height / 2;  // wxMax ( 0, mdc.MinY()-border_fluff );

        mdc.Clear();
      }

      else {
        bm_width = rad * 2 + (border_fluff * 2);
        bm_height = rad * 2 + (border_fluff * 2);
        bm_orgx = -bm_width / 2;
        bm_orgy = -bm_height / 2;
      }

#else
      bm_width = rad * 2 + (border_fluff * 2);
      bm_height = rad * 2 + (border_fluff * 2);
      bm_orgx = -bm_width / 2;
      bm_orgy = -bm_height / 2;
#endif

      wxBitmap *sbm = NULL;

      //    Draw the outer border
      wxColour color = getwxColour(outline_color);

      wxPen *pthispen = wxThePenList->FindOrCreatePen(
          color, outline_width * scale_factor, wxPENSTYLE_SOLID);
      mdc.SetPen(*pthispen);
      wxBrush *pthisbrush =
          wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_TRANSPARENT);
      mdc.SetBrush(*pthisbrush);

      mdc.DrawEllipticArc(width / 2 - rad, height / 2 - rad, rad * 2, rad * 2,
                          sb, se);

      if (arc_width) {
        wxColour colorb = getwxColour(arc_color);

        if (!colorb.IsOk()) colorb = getwxColour(_T("CHMGD"));

        pthispen = wxThePenList->FindOrCreatePen(
            colorb, arc_width * scale_factor, wxPENSTYLE_SOLID);
        mdc.SetPen(*pthispen);

        mdc.DrawEllipticArc(width / 2 - rad, height / 2 - rad, rad * 2, rad * 2,
                            sb, se);
      }

      mdc.SelectObject(wxNullBitmap);

      //          Get smallest containing bitmap
      sbm = new wxBitmap(bm.GetSubBitmap(wxRect(
          width / 2 + bm_orgx, height / 2 + bm_orgy, bm_width, bm_height)));

      //      Make the mask
      wxMask *pmask = new wxMask(*sbm, m_unused_wxColor);

      //      Associate the mask with the bitmap
      sbm->SetMask(pmask);

      // delete any old private data
      ClearRulesCache(rules->razRule);

      //      Save the bitmap ptr and aux parms in the rule
      prule->pixelPtr = sbm;
      prule->parm0 = ID_wxBitmap;
      prule->parm1 = m_colortable_index;
      prule->parm2 = bm_orgx;
      prule->parm3 = bm_orgy;
      prule->parm5 = bm_width;
      prule->parm6 = bm_height;
      prule->parm7 = xscale;
    }  // instantiation
  }

  //  Render arcs at object's x/y
  wxPoint r;
  GetPointPixSingle(rzRules, rzRules->obj->y, rzRules->obj->x, &r);

  //      Now render the symbol
  if (!m_pdc)  // opengl
  {
  } else {
    int b_width = prule->parm5;
    int b_height = prule->parm6;

    //      Get the bitmap into a memory dc
    wxMemoryDC mdc;
    mdc.SelectObject((wxBitmap &)(*((wxBitmap *)(rules->razRule->pixelPtr))));

    //      Blit it into the target dc, using mask
    m_pdc->Blit(r.x + rules->razRule->parm2, r.y + rules->razRule->parm3,
                b_width, b_height, &mdc, 0, 0, wxCOPY, true);

    mdc.SelectObject(wxNullBitmap);

    //    Draw the sector legs directly on the target DC
    //    so that anti-aliasing works against the drawn image (cannot be
    //    cached...)
    if (sector_radius > 0) {
      int leg_len = (int)(sector_radius * canvas_pix_per_mm);

      wxDash dash1[2];
      dash1[0] = (int)(3.6 * canvas_pix_per_mm);  // 8// Long dash  <---------+
      dash1[1] = (int)(1.8 * canvas_pix_per_mm);  // 2// Short gap            |

      /*
       wxPen *pthispen = new wxPen(*wxBLACK_PEN);
       pthispen->SetStyle(wxPENSTYLE_USER_DASH);
       pthispen->SetDashes( 2, dash1 );
       //      Undocumented "feature":  Pen must be fully specified <<<BEFORE>>>
       setting into DC pdc->SetPen ( *pthispen );
       */
      // wxColour c = GetGlobalColor( _T ( "CHBLK" ) );
      wxColour c;
      GetGlobalColor(_T ( "CHBLK" ), &c);

      float a = (sectr1 - 90) * PI / 180;
      int x = r.x + (int)(leg_len * cosf(a));
      int y = r.y + (int)(leg_len * sinf(a));
      DrawAALine(m_pdc, r.x, r.y, x, y, c, dash1[0], dash1[1]);

      a = (sectr2 - 90) * PI / 180.;
      x = r.x + (int)(leg_len * cosf(a));
      y = r.y + (int)(leg_len * sinf(a));
      DrawAALine(m_pdc, r.x, r.y, x, y, c, dash1[0], dash1[1]);
    }

    // Debug the symbol bounding box.....
    /*
            if(m_pdc){
                m_pdc->SetPen(wxPen(*wxGREEN, 1));
                m_pdc->SetBrush(wxBrush(*wxGREEN, wxTRANSPARENT));
                m_pdc->DrawRectangle( r.x + rules->razRule->parm2, r.y +
       rules->razRule->parm3, b_width, b_height);
            }
            */
  }

  //  Update the object Bounding box,
  //  so that subsequent drawing operations will redraw the item fully

  double latmin, lonmin, latmax, lonmax;

  GetPixPointSingleNoRotate(r.x + prule->parm2,
                            r.y + prule->parm3 + prule->parm6, &latmin, &lonmin);
  GetPixPointSingleNoRotate(r.x + prule->parm2 + prule->parm5,
                            r.y + prule->parm3, &latmax, &lonmax);
  LLBBox symbox;
  symbox.Set(latmin, lonmin, latmax, lonmax);
  rzRules->obj->BBObj.Expand(symbox);
  return 1;
}

// Conditional Symbology
char *s52plib::RenderCS(ObjRazRules *rzRules, Rules *rules) {
  void *ret;
  void *(*f)(void *);

  static int f05;

  if (rules->razRule == NULL) {
    if (!f05)
      //                  CPLError ( ( CPLErr ) 0, 0,"S52plib:_renderCS(): ERROR
      //                  no conditional symbology for: %s\n", rules->INSTstr );
      f05++;
    return 0;
  }

  void *g = (void *)rules->razRule;

#ifdef FIX_FOR_MSVC  //__WXMSW__
                     //#warning Fix this cast, somehow...
                     //      dsr             sigh... can't get the cast right
  _asm
      {
        mov eax,[dword ptr g]
        mov [dword ptr f],eax
      }
  ret = f((void *)rzRules);  // call cond symb
#else

  f = (void *(*)(void *))g;
  ret = f((void *)rzRules);

#endif

  return (char *)ret;
}

int s52plib::RenderObjectToDC(wxDC *pdcin, ObjRazRules *rzRules) {
  return DoRenderObject(pdcin, rzRules);
}

int s52plib::RenderObjectToGL(const wxGLContext &glcc, ObjRazRules *rzRules) {
  m_glcc = (wxGLContext *)&glcc;
  return DoRenderObject(NULL, rzRules);
}

int s52plib::RenderObjectToDCText(wxDC *pdcin, ObjRazRules *rzRules) {
  return DoRenderObjectTextOnly(pdcin, rzRules);
}

int s52plib::RenderObjectToGLText(const wxGLContext &glcc, ObjRazRules *rzRules) {
  m_glcc = (wxGLContext *)&glcc;
  return DoRenderObjectTextOnly(NULL, rzRules);
}

int s52plib::DoRenderObject(wxDC *pdcin, ObjRazRules *rzRules) {
  // TODO  Debugging
        //if(rzRules->obj->Index == 6775)
        //  int yyp = 0;

  //        if(!strncmp(rzRules->obj->FeatureName, "berths", 6))
  //            int yyp = 0;

  //return 0;
  if (!ObjectRenderCheckRules(rzRules, true)) return 0;

  m_pdc = pdcin;  // use this DC
  Rules *rules = rzRules->LUP->ruleList;

  while (rules != NULL) {
    switch (rules->ruleType) {
      case RUL_TXT_TX:
        RenderTX(rzRules, rules);
        break;  // TX
      case RUL_TXT_TE:
        RenderTE(rzRules, rules);
        break;  // TE
      case RUL_SYM_PT:
        RenderSY(rzRules, rules);
        break;  // SY
      case RUL_SIM_LN:
        if (m_pdc)
          RenderLS(rzRules, rules);
        else
          RenderGLLS(rzRules, rules);
        break;  // LS
      case RUL_COM_LN:
        RenderLC(rzRules, rules);
        break;  // LC
      case RUL_MUL_SG:
        RenderMPS(rzRules, rules);
        break;  // MultiPoint Sounding
      case RUL_ARC_2C:
        RenderCARC(rzRules, rules);
        break;  // Circular Arc, 2 colors

      case RUL_CND_SY: {
        if (!rzRules->obj->bCS_Added) {
          rzRules->obj->CSrules = NULL;
          GetAndAddCSRules(rzRules, rules);
          if (strncmp(rzRules->obj->FeatureName, "SOUNDG", 6))
            rzRules->obj->bCS_Added = 1;  // mark the object
        }

        Rules *rules_last = rules;
        rules = rzRules->obj->CSrules;

        while (NULL != rules) {
          switch (rules->ruleType) {
            case RUL_TXT_TX:
              RenderTX(rzRules, rules);
              break;
            case RUL_TXT_TE:
              RenderTE(rzRules, rules);
              break;
            case RUL_SYM_PT:
              RenderSY(rzRules, rules);
              break;
            case RUL_SIM_LN:
              if (m_pdc)
                RenderLS(rzRules, rules);
              else
                RenderGLLS(rzRules, rules);
              break;  // LS
            case RUL_COM_LN:
              RenderLC(rzRules, rules);
              break;
            case RUL_MUL_SG:
              RenderMPS(rzRules, rules);
              break;  // MultiPoint Sounding
            case RUL_ARC_2C:
              RenderCARC(rzRules, rules);
              break;  // Circular Arc, 2 colors
            case RUL_NONE:
            default:
              break;  // no rule type (init)
          }
          rules_last = rules;
          rules = rules->next;
        }

        rules = rules_last;
        break;
      }

      case RUL_NONE:
      default:
        break;  // no rule type (init)
    }           // switch

    rules = rules->next;
  }

  return 1;
}

int s52plib::DoRenderObjectTextOnly(wxDC *pdcin, ObjRazRules *rzRules) {
  //    if(strncmp(rzRules->obj->FeatureName, "RDOCAL", 6))
  //        return 0;

  //    if(rzRules->obj->Index == 2766)
  //        int yyp = 4;

  if (!ObjectRenderCheckRules(rzRules, true)) return 0;

  m_pdc = pdcin;  // use this DC
  Rules *rules = rzRules->LUP->ruleList;

  while (rules != NULL) {
    switch (rules->ruleType) {
      case RUL_TXT_TX:
        RenderTX(rzRules, rules);
        break;  // TX
      case RUL_TXT_TE:
        RenderTE(rzRules, rules);
        break;  // TE
      case RUL_CND_SY: {
        if (!rzRules->obj->bCS_Added) {
          rzRules->obj->CSrules = NULL;
          GetAndAddCSRules(rzRules, rules);
          if (strncmp(rzRules->obj->FeatureName, "SOUNDG", 6))
            rzRules->obj->bCS_Added = 1;  // mark the object
        }

        Rules *rules_last = rules;
        rules = rzRules->obj->CSrules;

        while (NULL != rules) {
          switch (rules->ruleType) {
            case RUL_TXT_TX:
              RenderTX(rzRules, rules);
              break;
            case RUL_TXT_TE:
              RenderTE(rzRules, rules);
              break;
            default:
              break;  // no rule type (init)
          }
          rules_last = rules;
          rules = rules->next;
        }

        rules = rules_last;
        break;
      }

      case RUL_NONE:
      default:
        break;  // no rule type (init)
    }           // switch

    rules = rules->next;
  }

  return 1;
}

bool s52plib::PreloadOBJLFromCSV(const wxString &csv_file) {
  wxTextFile file(csv_file);
  if (!file.Exists()) return false;

  file.Open();

  wxString str;
  str = file.GetFirstLine();
  wxChar quote[] = {'\"', 0};
  wxString description;
  wxString token;

  while (!file.Eof()) {
    str = file.GetNextLine();

    wxStringTokenizer tkz(str, _T(","));
    token = tkz.GetNextToken();  // code

    description = tkz.GetNextToken();  // May contain comma
    if (!description.EndsWith(quote)) description << tkz.GetNextToken();
    description.Replace(_T("\""), _T(""), true);

    token = tkz.GetNextToken();  // Acronym

    if (token.Len()) {
      //    Filter out any duplicates, in a case insensitive way
      //    i.e. only the first of "DEPARE" and "depare" is added
      bool bdup = false;
      for (unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++) {
        OBJLElement *pOLEt = (OBJLElement *)(pOBJLArray->Item(iPtr));
        if (!token.CmpNoCase(wxString(pOLEt->OBJLName, wxConvUTF8))) {
          bdup = true;
          break;
        }
      }

      if (!bdup) {
        wxCharBuffer buffer = token.ToUTF8();
        if (buffer.data()) {
          OBJLElement *pOLE = (OBJLElement *)calloc(sizeof(OBJLElement), 1);
          memcpy(pOLE->OBJLName, buffer.data(), 6);
          pOLE->nViz = 0;

          pOBJLArray->Add((void *)pOLE);

          OBJLDescriptions.push_back(description);
        }
      }
    }
  }
  return true;
}

void s52plib::UpdateOBJLArray(S57Obj *obj) {
  //    Search the array for this object class

  bool bNeedNew = true;
  OBJLElement *pOLE;

  for (unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++) {
    pOLE = (OBJLElement *)(pOBJLArray->Item(iPtr));
    if (!strncmp(pOLE->OBJLName, obj->FeatureName, 6)) {
      obj->iOBJL = iPtr;
      bNeedNew = false;
      break;
    }
  }

  //    Not found yet, so add an element
  if (bNeedNew) {
    pOLE = (OBJLElement *)calloc(sizeof(OBJLElement), 1);
    memcpy(pOLE->OBJLName, obj->FeatureName, OBJL_NAME_LEN);
    pOLE->nViz = 1;

    pOBJLArray->Add((void *)pOLE);
    obj->iOBJL = pOBJLArray->GetCount() - 1;
  }
}

int s52plib::SetLineFeaturePriority(ObjRazRules *rzRules, int npriority) {
  int priority_set = npriority;  // may be adjusted

  Rules *rules = rzRules->LUP->ruleList;

  //      Do Object Type Filtering
  //    If the object s not currently visible (i.e. classed as a not-currently
  //    visible category), then do not set the line segment priorities at all
  //

  bool b_catfilter = true;

  // DEPCNT is mutable
  if (m_nDisplayCategory == STANDARD) {
    if ((DISPLAYBASE != rzRules->LUP->DISC) &&
        (STANDARD != rzRules->LUP->DISC)) {
      b_catfilter = rzRules->obj->m_bcategory_mutable;
    }
  } else if (m_nDisplayCategory == DISPLAYBASE) {
    if (DISPLAYBASE != rzRules->LUP->DISC) {
      b_catfilter = rzRules->obj->m_bcategory_mutable;
    }
  }

  if (IsObjNoshow(rzRules->LUP->OBCL)) b_catfilter = false;

  if (!b_catfilter)  // No chance this object is visible
    return 0;

  while (rules != NULL) {
    switch (rules->ruleType) {
      case RUL_SIM_LN:
      case RUL_COM_LN:
        PrioritizeLineFeature(rzRules, priority_set);
        break;  // LC

      case RUL_CND_SY: {
        if (!rzRules->obj->bCS_Added) {
          rzRules->obj->CSrules = NULL;
          GetAndAddCSRules(rzRules, rules);
          rzRules->obj->bCS_Added = 1;  // mark the object
        }
        Rules *rules_last = rules;
        rules = rzRules->obj->CSrules;

        while (NULL != rules) {
          switch (rules->ruleType) {
            case RUL_SIM_LN:
            case RUL_COM_LN:
              PrioritizeLineFeature(rzRules, priority_set);
              break;
            case RUL_NONE:
            default:
              break;  // no rule type (init)
          }
          rules_last = rules;
          rules = rules->next;
        }

        rules = rules_last;
        break;
      }

      case RUL_NONE:
      default:
        break;  // no rule type (init)
    }           // switch

    rules = rules->next;
  }

  return 1;
}

int s52plib::PrioritizeLineFeature(ObjRazRules *rzRules, int npriority) {
  if (rzRules->obj->m_ls_list) {
    VE_Element *pedge;
    connector_segment *pcs;
    line_segment_element *ls = rzRules->obj->m_ls_list;
    while (ls) {
      switch (ls->ls_type) {
        case TYPE_EE:
        case TYPE_EE_REV:

          pedge = ls->pedge;  //(VE_Element *)ls->private0;
          if (pedge)
            pedge->max_priority =
                npriority;  // wxMax(pedge->max_priority, npriority);
          break;

        default:
          pcs = ls->pcs;  //(connector_segment *)ls->private0;
          if (pcs)
            pcs->max_priority_cs =
                npriority;  // wxMax(pcs->max_priority, npriority);
          break;
      }

      ls = ls->next;
    }
  }

  else if (rzRules->obj->m_ls_list_legacy) {  // PlugIn (S63)

    PI_connector_segment *pcs;
    VE_Element *pedge;

    PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;
    while (ls) {
      switch (ls->type) {
        case TYPE_EE:

          pedge = (VE_Element *)ls->private0;
          if (pedge)
            pedge->max_priority =
                npriority;  // wxMax(pedge->max_priority, npriority);
          break;

        default:
          pcs = (PI_connector_segment *)ls->private0;
          if (pcs)
            pcs->max_priority =
                npriority;  // wxMax(pcs->max_priority, npriority);
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

bool TestLinesIntersection(XLINE &a, XLINE &b) {
  XPOINT i;

  if ((a.p.x == a.o.x) && (b.p.x == b.o.x))  // both vertical
  {
    return (a.p.x == b.p.x);
  }

  if (a.p.x == a.o.x)  // a line a is vertical
  {
    // calculate b gradient
    b.m = (b.p.y - b.o.y) / (b.p.x - b.o.x);
    // calculate axis intersect values
    b.c = b.o.y - (b.m * b.o.x);
    // calculate y point of intercept
    i.y = b.o.y + ((a.o.x - b.o.x) * b.m);
    if (i.y < wxMin(a.o.y, a.p.y) || i.y > wxMax(a.o.y, a.p.y)) return false;
    return true;
  }

  if (b.p.x == b.o.x)  // line b is vertical
  {
    // calculate b gradient
    a.m = (a.p.y - a.o.y) / (a.p.x - a.o.x);
    // calculate axis intersect values
    a.c = a.o.y - (a.m * a.o.x);
    // calculate y point of intercept
    i.y = a.o.y + ((b.o.x - a.o.x) * a.m);
    if (i.y < wxMin(b.o.y, b.p.y) || i.y > wxMax(b.o.y, b.p.y)) return false;
    return true;
  }

  // calculate gradients
  a.m = (a.p.y - a.o.y) / (a.p.x - a.o.x);
  b.m = (b.p.y - b.o.y) / (b.p.x - b.o.x);
  // parallel lines can't intercept
  if (a.m == b.m) {
    return false;
  }
  // calculate axis intersect values
  a.c = a.o.y - (a.m * a.o.x);
  b.c = b.o.y - (b.m * b.o.x);
  // calculate x point of intercept
  i.x = (b.c - a.c) / (a.m - b.m);
  // is intersection point in segment
  if (i.x < wxMin(a.o.x, a.p.x) || i.x > wxMax(a.o.x, a.p.x)) {
    return false;
  }
  if (i.x < wxMin(b.o.x, b.p.x) || i.x > wxMax(b.o.x, b.p.x)) {
    return false;
  }
  // points intercept
  return true;
}

//-----------------------------------------------------------------------
//    Check a triangle described by point array, and rectangle described by
//    render_canvas_parms for intersection Return false if no intersection
//-----------------------------------------------------------------------
bool s52plib::inter_tri_rect(wxPoint *ptp, render_canvas_parms *pb_spec) {
  //    First stage
  //    Check all three points of triangle to see it any are within the render
  //    rectangle

  BoundingBox rect(pb_spec->lclip, pb_spec->y, pb_spec->rclip,
                     pb_spec->y + pb_spec->height);

  for (int i = 0; i < 3; i++) {
    if (rect.PointInBox(ptp[i].x, ptp[i].y)) return true;
  }

  //    Next stage
  //    Check all four points of rectangle to see it any are within the render
  //    triangle

  double p[6];
  MyPoint *pmp = (MyPoint *)p;

  for (int i = 0; i < 3; i++) {
    pmp[i].x = ptp[i].x;
    pmp[i].y = ptp[i].y;
  }

  if (G_PtInPolygon(pmp, 3, pb_spec->lclip, pb_spec->y)) return true;

  if (G_PtInPolygon(pmp, 3, pb_spec->lclip, pb_spec->y + pb_spec->height))
    return true;

  if (G_PtInPolygon(pmp, 3, pb_spec->rclip, pb_spec->y)) return true;

  if (G_PtInPolygon(pmp, 3, pb_spec->rclip, pb_spec->y + pb_spec->height))
    return true;

  //    last step
  //    Check triangle lines against rect lines for line intersect

  for (int i = 0; i < 3; i++) {
    XLINE a;
    a.o.x = ptp[i].x;
    a.o.y = ptp[i].y;
    if (i == 2) {
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

    if (TestLinesIntersection(a, b)) return true;

    //    right line
    b.o.x = pb_spec->rclip;
    b.o.y = pb_spec->y;
    b.p.x = pb_spec->rclip;
    b.p.y = pb_spec->y + pb_spec->height;

    if (TestLinesIntersection(a, b)) return true;

    //    bottom line
    b.o.x = pb_spec->rclip;
    b.o.y = pb_spec->y + pb_spec->height;
    b.p.x = pb_spec->lclip;
    b.p.y = pb_spec->y + pb_spec->height;

    if (TestLinesIntersection(a, b)) return true;

    //    left line
    b.o.x = pb_spec->lclip;
    b.o.y = pb_spec->y + pb_spec->height;
    b.p.x = pb_spec->lclip;
    b.p.y = pb_spec->y;

    if (TestLinesIntersection(a, b)) return true;
  }

  return false;  // no Intersection
}

//----------------------------------------------------------------------------------
//
//              Fast Basic Canvas Rendering
//              Render triangle
//
//----------------------------------------------------------------------------------
int s52plib::dda_tri(wxPoint *ptp, S52color *c, render_canvas_parms *pb_spec,
                     render_canvas_parms *pPatt_spec) {
  unsigned char r = 0;
  unsigned char g = 0;
  unsigned char b = 0;

  if (!inter_tri_rect(ptp, pb_spec)) return 0;

  if (NULL != c) {
    if (pb_spec->b_revrgb) {
      r = c->R;
      g = c->G;
      b = c->B;
    } else {
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
  if (NULL != c) color_int = ((r) << 16) + ((g) << 8) + (b);

  //      Determine ymin and ymax indices

  int ymax = ptp[0].y;
  int ymin = ymax;
  int xmin, xmax, xmid, ymid;
  int imin = 0;
  int imax = 0;
  int imid;

  for (int ip = 1; ip < 3; ip++) {
    if (ptp[ip].y > ymax) {
      imax = ip;
      ymax = ptp[ip].y;
    }
    if (ptp[ip].y <= ymin) {
      imin = ip;
      ymin = ptp[ip].y;
    }
  }

  imid = 3 - (imin + imax);  // do the math...

  xmax = ptp[imax].x;
  xmin = ptp[imin].x;
  xmid = ptp[imid].x;
  ymid = ptp[imid].y;

  //      Create edge arrays using fast integer DDA
  int m, x, dy, count;
  bool cw;

  if ((abs(xmax - xmin) > 32768) || (abs(xmid - xmin) > 32768) ||
      (abs(xmax - xmid) > 32768) || (abs(ymax - ymin) > 32768) ||
      (abs(ymid - ymin) > 32768) || (abs(ymax - ymid) > 32768) ||
      (xmin > 32768) || (xmid > 32768)) {
    dy = (ymax - ymin);
    if (dy) {
      m = (xmax - xmin) << 8;
      m /= dy;

      x = xmin << 8;

      for (count = ymin; count <= ymax; count++) {
        if ((count >= 0) && (count < 1500)) ledge[count] = x >> 8;
        x += m;
      }
    }

    dy = (ymid - ymin);
    if (dy) {
      m = (xmid - xmin) << 8;
      m /= dy;

      x = xmin << 8;

      for (count = ymin; count <= ymid; count++) {
        if ((count >= 0) && (count < 1500)) redge[count] = x >> 8;
        x += m;
      }
    }

    dy = (ymax - ymid);
    if (dy) {
      m = (xmax - xmid) << 8;
      m /= dy;

      x = xmid << 8;

      for (count = ymid; count <= ymax; count++) {
        if ((count >= 0) && (count < 1500)) redge[count] = x >> 8;
        x += m;
      }
    }

    double ddfSum = 0;
    //      Check the triangle edge winding direction
    ddfSum += (xmin / 1) * (ymax / 1) - (ymin / 1) * (xmax / 1);
    ddfSum += (xmax / 1) * (ymid / 1) - (ymax / 1) * (xmid / 1);
    ddfSum += (xmid / 1) * (ymin / 1) - (ymid / 1) * (xmin / 1);
    cw = ddfSum < 0;

  } else {
    dy = (ymax - ymin);
    if (dy) {
      m = (xmax - xmin) << 16;
      m /= dy;

      x = xmin << 16;

      for (count = ymin; count <= ymax; count++) {
        if ((count >= 0) && (count < 1500)) ledge[count] = x >> 16;
        x += m;
      }
    }

    dy = (ymid - ymin);
    if (dy) {
      m = (xmid - xmin) << 16;
      m /= dy;

      x = xmin << 16;

      for (count = ymin; count <= ymid; count++) {
        if ((count >= 0) && (count < 1500)) redge[count] = x >> 16;
        x += m;
      }
    }

    dy = (ymax - ymid);
    if (dy) {
      m = (xmax - xmid) << 16;
      m /= dy;

      x = xmid << 16;

      for (count = ymid; count <= ymax; count++) {
        if ((count >= 0) && (count < 1500)) redge[count] = x >> 16;
        x += m;
      }
    }

    //      Check the triangle edge winding direction
    long dfSum = 0;
    dfSum += xmin * ymax - ymin * xmax;
    dfSum += xmax * ymid - ymax * xmid;
    dfSum += xmid * ymin - ymid * xmin;

    cw = dfSum < 0;

  }  // else

  //      if cw is true, redge is actually on the right

  int y1 = ymax;
  int y2 = ymin;

  int ybt = pb_spec->y;
  int yt = pb_spec->y + pb_spec->height;

  if (y1 > yt) y1 = yt;
  if (y1 < ybt) y1 = ybt;

  if (y2 > yt) y2 = yt;
  if (y2 < ybt) y2 = ybt;

  int lclip = pb_spec->lclip;
  int rclip = pb_spec->rclip;
  if (y1 == y2) return 0;

  //              Clip the triangle
  if (cw) {
    for (int iy = y2; iy <= y1; iy++) {
      if (ledge[iy] < lclip) {
        if (redge[iy] < lclip)
          ledge[iy] = -1;
        else
          ledge[iy] = lclip;
      }

      if (redge[iy] > rclip) {
        if (ledge[iy] > rclip)
          ledge[iy] = -1;
        else
          redge[iy] = rclip;
      }
    }
  } else {
    for (int iy = y2; iy <= y1; iy++) {
      if (redge[iy] < lclip) {
        if (ledge[iy] < lclip)
          ledge[iy] = -1;
        else
          redge[iy] = lclip;
      }

      if (ledge[iy] > rclip) {
        if (redge[iy] > rclip)
          ledge[iy] = -1;
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
  if (pPatt_spec) {
    patt_size_y = pPatt_spec->height;
    patt_size_x = pPatt_spec->width;
    patt_pitch = pPatt_spec->pb_pitch;
    patt_s0 = pPatt_spec->pix_buff;

    if (patt_size_y == 0) /* integer division by this value below */
      return false;
  }

  if (pb_spec->depth == 24) {
    for (int iyp = ya; iyp < yb; iyp++) {
      if ((iyp >= ybt) && (iyp < yt)) {
        int yoff = (iyp - pb_spec->y) * pb_spec->pb_pitch;

        unsigned char *py = pix_buff + yoff;

        int ix, ixm;
        if (cw) {
          ix = ledge[iyp];
          ixm = redge[iyp];
        } else {
          ixm = ledge[iyp];
          ix = redge[iyp];
        }

        if (ledge[iyp] != -1) {
          //    This would be considered a failure of the dda algorithm
          //    Happens on very high zoom, with very large triangles.
          //    The integers of the dda algorithm don't have enough bits...
          //    Anyway, just ignore this triangle if it happens
          if (ix > ixm) continue;

          int xoff = (ix - pb_spec->x) * 3;

          unsigned char *px = py + xoff;

          if (pPatt_spec)  // Pattern
          {
            int y_stagger = (iyp - pPatt_spec->y) / patt_size_y;
            int x_stagger_off = 0;
            if ((y_stagger & 1) && pPatt_spec->b_stagger)
              x_stagger_off = pPatt_spec->width / 2;

            int patt_y = abs((iyp - pPatt_spec->y)) % patt_size_y;

            unsigned char *pp0 = patt_s0 + (patt_y * patt_pitch);

            while (ix <= ixm) {
              int patt_x =
                  abs(((ix - pPatt_spec->x) + x_stagger_off) % patt_size_x);

              unsigned char *pp = pp0 + (patt_x * 4);
              unsigned char alpha = pp[3];
              double da = (double)alpha / 256.;

              unsigned char r = (unsigned char)(*px * (1.0 - da) + pp[0] * da);
              unsigned char g =
                  (unsigned char)(*(px + 1) * (1.0 - da) + pp[1] * da);
              unsigned char b =
                  (unsigned char)(*(px + 2) * (1.0 - da) + pp[2] * da);

              *px++ = r;
              *px++ = g;
              *px++ = b;
              ix++;
            }
          }

          else  // No Pattern
          {
#if defined(__WXGTK__) && defined(__INTEL__)
#define memset3(dest, value, count)       \
  __asm__ __volatile__(                   \
      "cmp $0,%2\n\t"                     \
      "jg 2f\n\t"                         \
      "je 3f\n\t"                         \
      "jmp 4f\n\t"                        \
      "2:\n\t"                            \
      "movl  %0,(%1)\n\t"                 \
      "add $3,%1\n\t"                     \
      "dec %2\n\t"                        \
      "jnz 2b\n\t"                        \
      "3:\n\t"                            \
      "movb %b0,(%1)\n\t"                 \
      "inc %1\n\t"                        \
      "movb %h0,(%1)\n\t"                 \
      "inc %1\n\t"                        \
      "shr $16,%0\n\t"                    \
      "movb %b0,(%1)\n\t"                 \
      "4:\n\t"                            \
      :                                   \
      : "a"(value), "D"(dest), "r"(count) \
      :);

            int count = ixm - ix;
            memset3(px, color_int, count)
#else

            while (ix <= ixm) {
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

  if (pb_spec->depth == 32) {
    assert(ya <= yb);

    for (int iyp = ya; iyp < yb; iyp++) {
      if ((iyp >= ybt) && (iyp < yt)) {
        int yoff = (iyp - pb_spec->y) * pb_spec->pb_pitch;

        unsigned char *py = pix_buff + yoff;

        int ix, ixm;
        if (cw) {
          ix = ledge[iyp];
          ixm = redge[iyp];
        } else {
          ixm = ledge[iyp];
          ix = redge[iyp];
        }

        if (ledge[iyp] != -1) {
          //    This would be considered a failure of the dda algorithm
          //    Happens on very high zoom, with very large triangles.
          //    The integers of the dda algorithm don't have enough bits...
          //    Anyway, just ignore this triangle if it happens
          if (ix > ixm) continue;

          int xoff = (ix - pb_spec->x) * pb_spec->depth / 8;

          unsigned char *px = py + xoff;

          if (pPatt_spec)  // Pattern
          {
            int y_stagger = (iyp - pPatt_spec->y) / patt_size_y;

            int x_stagger_off = 0;
            if ((y_stagger & 1) && pPatt_spec->b_stagger)
              x_stagger_off = pPatt_spec->width / 2;

            int patt_y = abs((iyp - pPatt_spec->y)) % patt_size_y;

            unsigned char *pp0 = patt_s0 + (patt_y * patt_pitch);

            while (ix <= ixm) {
              int patt_x =
                  abs(((ix - pPatt_spec->x) + x_stagger_off) % patt_size_x);
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
                unsigned char *pp = pp0 + (patt_x * 4);
                unsigned char alpha = pp[3];
                if (alpha > 128) {
                  double da = (double)alpha / 256.;

                  unsigned char r = (unsigned char)(pp[0] * da);
                  unsigned char g = (unsigned char)(pp[1] * da);
                  unsigned char b = (unsigned char)(pp[2] * da);

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

          else  // No Pattern
          {
            int *pxi = (int *)px;
            while (ix <= ixm) {
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
inline int s52plib::dda_trap(wxPoint *segs, int lseg, int rseg, int ytop,
                             int ybot, S52color *c,
                             render_canvas_parms *pb_spec,
                             render_canvas_parms *pPatt_spec) {
  unsigned char r = 0, g = 0, b = 0;

  if (NULL != c) {
    if (pb_spec->b_revrgb) {
      r = c->R;
      g = c->G;
      b = c->B;
    } else {
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
  if (NULL != c) color_int = ((r) << 16) + ((g) << 8) + (b);

  //      Create edge arrays using fast integer DDA

  int lclip = pb_spec->lclip;
  int rclip = pb_spec->rclip;

  int m, x, dy, count;

  //    Left edge
  int xmax = segs[lseg].x;
  int xmin = segs[lseg + 1].x;
  int ymax = segs[lseg].y;
  int ymin = segs[lseg + 1].y;

  if (ymax < ymin) {
    int a = ymax;
    ymax = ymin;
    ymin = a;

    a = xmax;
    xmax = xmin;
    xmin = a;
  }

  int y_dda_limit = wxMin(ybot, ymax);
  y_dda_limit = wxMin(y_dda_limit, 1499);  // don't overrun edge array

  //    Some peephole optimization:
  //    if xmax and xmin are both < 0, arrange to simply fill the ledge array
  //    with 0
  if ((xmax < 0) && (xmin < 0)) {
    xmax = -2;
    xmin = -2;
  }
  //    if xmax and xmin are both > rclip, arrange to simply fill the ledge
  //    array with rclip + 1 This may induce special clip case below, and cause
  //    trap not to be rendered
  else if ((xmax > rclip) && (xmin > rclip)) {
    xmax = rclip + 1;
    xmin = rclip + 1;
  }

  dy = (ymax - ymin);
  if (dy) {
    m = (xmax - xmin) << 16;
    m /= dy;

    x = xmin << 16;

    // TODO implement this logic in dda_tri also
    count = ymin;
    while (count < 0) {
      x += m;
      count++;
    }

    while (count < y_dda_limit) {
      ledge[count] = x >> 16;
      x += m;
      count++;
    }
  }

  if ((ytop < ymin) || (ybot > ymax)) {
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

  // Note this never gets hit???
  if (ymax < ymin) {
    int a = ymax;
    ymax = ymin;
    ymin = a;

    a = xmax;
    xmax = xmin;
    xmin = a;
  }

  //    Some peephole optimization:
  //    if xmax and xmin are both < 0, arrange to simply fill the redge array
  //    with -1 This may induce special clip case below, and cause trap not to
  //    be rendered
  if ((xmax < 0) && (xmin < 0)) {
    xmax = -1;
    xmin = -1;
  }

  //    if xmax and xmin are both > rclip, arrange to simply fill the redge
  //    array with rclip + 1 This may induce special clip case below, and cause
  //    trap not to be rendered
  else if ((xmax > rclip) && (xmin > rclip)) {
    xmax = rclip + 1;
    xmin = rclip + 1;
  }

  y_dda_limit = wxMin(ybot, ymax);
  y_dda_limit = wxMin(y_dda_limit, 1499);  // don't overrun edge array

  dy = (ymax - ymin);
  if (dy) {
    m = (xmax - xmin) << 16;
    m /= dy;

    x = xmin << 16;

    count = ymin;
    while (count < 0) {
      x += m;
      count++;
    }

    while (count < y_dda_limit) {
      redge[count] = x >> 16;
      x += m;
      count++;
    }
  }

  if ((ytop < ymin) || (ybot > ymax)) {
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

  if (y1 > yt) y1 = yt;
  if (y1 < ybt) y1 = ybt;

  if (y2 > yt) y2 = yt;
  if (y2 < ybt) y2 = ybt;

  //   Clip the trapezoid to width
  for (int iy = y2; iy <= y1; iy++) {
    if (ledge[iy] < lclip) {
      if (redge[iy] < lclip)
        ledge[iy] = -1;
      else
        ledge[iy] = lclip;
    }

    if (redge[iy] > rclip) {
      if (ledge[iy] > rclip)
        ledge[iy] = -1;
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
  if (pPatt_spec) {
    patt_size_y = pPatt_spec->height;
    patt_size_x = pPatt_spec->width;
    patt_pitch = pPatt_spec->pb_pitch;
    patt_s0 = pPatt_spec->pix_buff;
  }

  if (pb_spec->depth == 24) {
    for (int iyp = ya; iyp < yb; iyp++) {
      if ((iyp >= ybt) && (iyp < yt)) {
        int yoff = (iyp - pb_spec->y) * pb_spec->pb_pitch;

        unsigned char *py = pix_buff + yoff;

        int ix, ixm;
        ix = ledge[iyp];
        ixm = redge[iyp];

        //                        if(debug) printf("iyp %d, ix %d, ixm %d\n",
        //                        iyp, ix, ixm);
        //                           int ix = ledge[iyp];
        //                            if(ix != -1)                    // special
        //                            clip case
        if (ledge[iyp] != -1) {
          int xoff = (ix - pb_spec->x) * 3;

          unsigned char *px = py + xoff;

          if (pPatt_spec)  // Pattern
          {
            int y_stagger = (iyp - pPatt_spec->y) / patt_size_y;
            int x_stagger_off = 0;
            if ((y_stagger & 1) && pPatt_spec->b_stagger)
              x_stagger_off = pPatt_spec->width / 2;

            int patt_y = abs((iyp - pPatt_spec->y)) % patt_size_y;
            unsigned char *pp0 = patt_s0 + (patt_y * patt_pitch);

            while (ix <= ixm) {
              int patt_x =
                  abs(((ix - pPatt_spec->x) + x_stagger_off) % patt_size_x);
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
                unsigned char *pp = pp0 + (patt_x * 4);
                unsigned char alpha = pp[3];
                if (alpha > 128) {
                  double da = (double)alpha / 256.;

                  unsigned char r = (unsigned char)(pp[0] * da);
                  unsigned char g = (unsigned char)(pp[1] * da);
                  unsigned char b = (unsigned char)(pp[2] * da);

                  *px++ = r;
                  *px++ = g;
                  *px++ = b;
                } else
                  px += 3;
              }

              ix++;
            }
          }

          else  // No Pattern
          {
#if defined(__WXGTK__WITH_OPTIMIZE_0) && defined(__INTEL__)
#define memset3d(dest, value, count)      \
  __asm__ __volatile__(                   \
      "cmp $0,%2\n\t"                     \
      "jg ld0\n\t"                        \
      "je ld1\n\t"                        \
      "jmp ld2\n\t"                       \
      "ld0:\n\t"                          \
      "movl  %0,(%1)\n\t"                 \
      "add $3,%1\n\t"                     \
      "dec %2\n\t"                        \
      "jnz ld0\n\t"                       \
      "ld1:\n\t"                          \
      "movb %b0,(%1)\n\t"                 \
      "inc %1\n\t"                        \
      "movb %h0,(%1)\n\t"                 \
      "inc %1\n\t"                        \
      "shr $16,%0\n\t"                    \
      "movb %b0,(%1)\n\t"                 \
      "ld2:\n\t"                          \
      :                                   \
      : "a"(value), "D"(dest), "r"(count) \
      :);
            int count = ixm - ix;
            memset3d(px, color_int, count)
#else

            while (ix <= ixm) {
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

  if (pb_spec->depth == 32) {
    assert(ya <= yb);

    for (int iyp = ya; iyp < yb; iyp++) {
      if ((iyp >= ybt) && (iyp < yt)) {
        int yoff = (iyp - pb_spec->y) * pb_spec->pb_pitch;

        unsigned char *py = pix_buff + yoff;

        int ix, ixm;
        ix = ledge[iyp];
        ixm = redge[iyp];

        if (ledge[iyp] != -1) {
          int xoff = (ix - pb_spec->x) * pb_spec->depth / 8;

          unsigned char *px = py + xoff;

          if (pPatt_spec)  // Pattern
          {
            int y_stagger = (iyp - pPatt_spec->y) / patt_size_y;
            int x_stagger_off = 0;
            if ((y_stagger & 1) && pPatt_spec->b_stagger)
              x_stagger_off = pPatt_spec->width / 2;

            int patt_y = abs((iyp - pPatt_spec->y)) % patt_size_y;
            unsigned char *pp0 = patt_s0 + (patt_y * patt_pitch);

            while (ix <= ixm) {
              int patt_x =
                  abs(((ix - pPatt_spec->x) + x_stagger_off) % patt_size_x);
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
                unsigned char *pp = pp0 + (patt_x * 4);
                unsigned char alpha = pp[3];
                if (alpha > 128) {
                  double da = (double)alpha / 256.;

                  unsigned char r = (unsigned char)(pp[0] * da);
                  unsigned char g = (unsigned char)(pp[1] * da);
                  unsigned char b = (unsigned char)(pp[2] * da);

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

          else  // No Pattern
          {
            int *pxi = (int *)px;
            while (ix <= ixm) {
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

void s52plib::RenderToBufferFilledPolygon(ObjRazRules *rzRules, S57Obj *obj,
                                          S52color *c,
                                          render_canvas_parms *pb_spec,
                                          render_canvas_parms *pPatt_spec) {
  LLBBox BBView = GetBBox();
  // please untangle this logic with the logic below
  if (BBView.GetMaxLon() + 180 < vp_plib.clon)
    BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() + 360, BBView.GetMaxLat(),
               BBView.GetMaxLon() + 360);
  else if (BBView.GetMinLon() - 180 > vp_plib.clon)
    BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() - 360, BBView.GetMaxLat(),
               BBView.GetMaxLon() - 360);

  S52color cp;
  if (NULL != c) {
    cp.R = c->R;
    cp.G = c->G;
    cp.B = c->B;
  }

  if (obj->pPolyTessGeo) {
    if (!rzRules->obj->pPolyTessGeo->IsOk()) {  // perform deferred tesselation
      rzRules->obj->pPolyTessGeo->BuildDeferredTess();
    }

    wxPoint *pp3 = (wxPoint *)malloc(3 * sizeof(wxPoint));
    wxPoint *ptp = (wxPoint *)malloc((obj->pPolyTessGeo->GetnVertexMax() + 1) *
                                     sizeof(wxPoint));

    PolyTriGroup *ppg = obj->pPolyTessGeo->Get_PolyTriGroup_head();

    TriPrim *p_tp = ppg->tri_prim_head;
    while (p_tp) {
      LLBBox box;
      if (!rzRules->obj->m_chart_context->chart) {  // This is a PlugIn Chart
        LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
        box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
      } else
        box = p_tp->tri_box;

      if (!BBView.IntersectOut(box)) {
        //      Get and convert the points
        wxPoint *pr = ptp;

        if (ppg->data_type == DATA_TYPE_DOUBLE) {
          double *pvert_list = p_tp->p_vertex;

          for (int iv = 0; iv < p_tp->nVert; iv++) {
            double lon = *pvert_list++;
            double lat = *pvert_list++;
            GetPointPixSingle(rzRules, lat, lon, pr);

            pr++;
          }
        } else {
          float *pvert_list = (float *)p_tp->p_vertex;

          for (int iv = 0; iv < p_tp->nVert; iv++) {
            double lon = *pvert_list++;
            double lat = *pvert_list++;
            GetPointPixSingle(rzRules, lat, lon, pr);

            pr++;
          }
        }

        switch (p_tp->type) {
          case PTG_TRIANGLE_FAN: {
            for (int it = 0; it < p_tp->nVert - 2; it++) {
              pp3[0].x = ptp[0].x;
              pp3[0].y = ptp[0].y;

              pp3[1].x = ptp[it + 1].x;
              pp3[1].y = ptp[it + 1].y;

              pp3[2].x = ptp[it + 2].x;
              pp3[2].y = ptp[it + 2].y;

              dda_tri(pp3, &cp, pb_spec, pPatt_spec);
            }
            break;
          }
          case PTG_TRIANGLE_STRIP: {
            for (int it = 0; it < p_tp->nVert - 2; it++) {
              pp3[0].x = ptp[it].x;
              pp3[0].y = ptp[it].y;

              pp3[1].x = ptp[it + 1].x;
              pp3[1].y = ptp[it + 1].y;

              pp3[2].x = ptp[it + 2].x;
              pp3[2].y = ptp[it + 2].y;

              dda_tri(pp3, &cp, pb_spec, pPatt_spec);
            }
            break;
          }
          case PTG_TRIANGLES: {
            for (int it = 0; it < p_tp->nVert; it += 3) {
              pp3[0].x = ptp[it].x;
              pp3[0].y = ptp[it].y;

              pp3[1].x = ptp[it + 1].x;
              pp3[1].y = ptp[it + 1].y;

              pp3[2].x = ptp[it + 2].x;
              pp3[2].y = ptp[it + 2].y;

              dda_tri(pp3, &cp, pb_spec, pPatt_spec);
            }
            break;
          }
        }
      }  // if bbox

      // pick up the next in chain
      if (!rzRules->obj->m_chart_context->chart) {  // This is a PlugIn Chart
        LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
        p_tp = (TriPrim *)p_ltp->p_next;
      } else
        p_tp = p_tp->p_next;

    }  // while

    free(ptp);
    free(pp3);
  }  // if pPolyTessGeo
}

#ifdef ocpnUSE_GL
int s52plib::RenderToGLAC(ObjRazRules *rzRules, Rules *rules) {
  return RenderToGLAC_GLSL(rzRules, rules);
}
#endif

int n_areaObjs;
int n_areaTris;

#ifdef ocpnUSE_GL
int s52plib::RenderToGLAC_GLSL(ObjRazRules *rzRules, Rules *rules) {
  if (!ObjectRenderCheckPosReduced(rzRules))
    return false;

  n_areaObjs++;

  GLenum reset_err = glGetError();

  S52color *c;
  char *str = (char *)rules->INSTstr;

  c = getColor(str);


  LLBBox BBView = GetBBox();
  // please untangle this logic with the logic below
  if (BBView.GetMaxLon() + 180 < vp_plib.clon)
    BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() + 360, BBView.GetMaxLat(),
               BBView.GetMaxLon() + 360);
  else if (BBView.GetMinLon() - 180 > vp_plib.clon)
    BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() - 360, BBView.GetMaxLat(),
               BBView.GetMaxLon() - 360);

  //  Allow a little slop in calculating whether a triangle
  //  is within the requested VPointCompat
  double margin = BBView.GetLonRange() * .05;
  BBView.EnLarge(margin);

  //  Use VBO if instructed by hardware renderer specification
  bool b_useVBO = m_GLAC_VBO && !rzRules->obj->auxParm1;

  if (rzRules->obj->pPolyTessGeo) {
    bool b_temp_vbo = false;
    bool b_transform = false;


    // perform deferred tesselation
    if (!rzRules->obj->pPolyTessGeo->IsOk()) {
      rzRules->obj->pPolyTessGeo->BuildDeferredTess();
    }

    //  Get the vertex data
    PolyTriGroup *ppg_vbo = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

    //  Has the input vertex buffer been converted to "single_alloc float"
    //  model? and is it allowed?
    if (!ppg_vbo->bsingle_alloc && (rzRules->obj->auxParm1 >= 0)) {
      int data_size = sizeof(float);

      //  First calculate the required total byte size
      int total_byte_size = 0;
      TriPrim *p_tp = ppg_vbo->tri_prim_head;
      while (p_tp) {
        total_byte_size += p_tp->nVert * 2 * data_size;
        p_tp = p_tp->p_next;  // pick up the next in chain
      }

      float *vbuf = (float *)malloc(total_byte_size);
      p_tp = ppg_vbo->tri_prim_head;

      if (ppg_vbo->data_type == DATA_TYPE_DOUBLE) {  // DOUBLE to FLOAT
        float *p_run = vbuf;
        while (p_tp) {
          float *pfbuf = p_run;
          for (int i = 0; i < p_tp->nVert * 2; ++i) {
            float x = (float)(p_tp->p_vertex[i]);
            *p_run++ = x;
          }

          free(p_tp->p_vertex);
          p_tp->p_vertex = (double *)pfbuf;

          p_tp = p_tp->p_next;  // pick up the next in chain
        }
      } else {  // FLOAT to FLOAT
        float *p_run = vbuf;
        while (p_tp) {
          memcpy(p_run, p_tp->p_vertex, p_tp->nVert * 2 * sizeof(float));

          free(p_tp->p_vertex);
          p_tp->p_vertex = (double *)p_run;

          p_run += p_tp->nVert * 2 * sizeof(float);

          p_tp = p_tp->p_next;  // pick up the next in chain
        }
      }

      ppg_vbo->bsingle_alloc = true;
      ppg_vbo->single_buffer = (unsigned char *)vbuf;
      ppg_vbo->single_buffer_size = total_byte_size;
      ppg_vbo->data_type = DATA_TYPE_FLOAT;
    }

    if (b_useVBO) {
      //  Has a VBO been built for this object?
      if (1) {
        glGetError();  // clear it

        if (rzRules->obj->auxParm0 <= 0) {
#ifdef xUSE_ANDROID_GLES2
          if (ppg_vbo->data_type != DATA_TYPE_SHORT) {
            // We convert the vertex data from FLOAT to GL_SHORT to make the VBO
            // smaller, but still keeping enough precision
            //  This requires a scale factor to reduce the range from existing
            //  data to +/- 32K

            size_t np = ppg_vbo->single_buffer_size / (2 * sizeof(float));
            np--;

            PolyTriGroup *ppg =
                rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
            TriPrim *p_tp = ppg->tri_prim_head;

            size_t npp = 0;
            while (p_tp) {
              npp += p_tp->nVert;
              p_tp = (TriPrim *)p_tp->p_next;
            }

            //  Get the data range
            float *pRun = (float *)ppg_vbo->single_buffer;
            float north_max = -1e8;
            float north_min = 1e8;
            float east_max = -1e8;
            float east_min = 1e8;

            for (size_t i = 0; i < np; i++) {
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
            unsigned char *new_buf =
                (unsigned char *)malloc(np * 2 * sizeof(short));
            pRun = (float *)ppg_vbo->single_buffer;
            short *pd = (short *)new_buf;
            for (size_t i = 0; i < np; i++) {
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
          b_temp_vbo = (rzRules->obj->auxParm0 ==
                        -5);  // Must we use a temporary VBO?  Probably slower
                              // than simple glDrawArrays

          GLuint vboId = 0;
          // generate a new VBO and get the associated ID
          glGenBuffers(1, &vboId);

          rzRules->obj->auxParm0 = vboId;

          // bind VBO in order to use
          glBindBuffer(GL_ARRAY_BUFFER, vboId);
          GLenum err = glGetError();
          if (err) {
            wxString msg;
            msg.Printf(_T("VBO Error A: %d"), err);
            wxLogMessage(msg);
            return 0;
          }

          // upload data to VBO
          glBufferData(GL_ARRAY_BUFFER, ppg_vbo->single_buffer_size,
                       ppg_vbo->single_buffer, GL_STATIC_DRAW);
          err = glGetError();
          if (err) {
            wxString msg;
            msg.Printf(_T("VBO Error B: %d"), err);
            wxLogMessage(msg);
            return 0;
          }

        } else {
          glBindBuffer(GL_ARRAY_BUFFER, rzRules->obj->auxParm0);
          GLenum err = glGetError();
          if (err) {
            wxString msg;
            msg.Printf(_T("VBO Error C: %d"), err);
            wxLogMessage(msg);
            return 0;
          }
        }
      }
    }

    PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

    TriPrim *p_tp = ppg->tri_prim_head;
    GLintptr vbo_offset = 0;

    //      Set up the stride sizes for the array
    int array_data_size = sizeof(float);
    GLint array_gl_type = GL_FLOAT;

    if (ppg->data_type == DATA_TYPE_DOUBLE) {
      array_data_size = sizeof(double);
      array_gl_type = GL_DOUBLE;
    }

    if (ppg->data_type == DATA_TYPE_SHORT) {
      array_data_size = sizeof(short);
      array_gl_type = GL_SHORT;
    }

    CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
    shader->Bind();

    // Disable VBO's (vertex buffer objects) for attributes.
    if (!b_useVBO) glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLint pos = shader->getAttributeLocation("position");
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

    if (rzRules->obj->m_chart_context->chart) {  // not a PlugIn Chart
      if (((int)rzRules->obj->m_chart_context->chart_type ==
           (int)S52_ChartTypeEnum::S52_CHART_TYPE_CM93) ||
          ((int)rzRules->obj->m_chart_context->chart_type ==
           (int)S52_ChartTypeEnum::S52_CHART_TYPE_CM93COMP)) {
        //      We may need to translate object coordinates by 360 degrees to
        //      conform.
        if (BBView.GetMaxLon() >= 180.) {
          if (rzRules->obj->BBObj.GetMinLon() < BBView.GetMaxLon() - 360.)
            x_origin += (float)(mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI);
        } else if ((BBView.GetMinLon() <= -180. &&
                    rzRules->obj->BBObj.GetMaxLon() >
                        BBView.GetMinLon() + 360.) ||
                   (rzRules->obj->BBObj.GetMaxLon() > 180 &&
                    BBView.GetMinLon() + 360 < rzRules->obj->BBObj.GetMaxLon()))
          x_origin -= (float)(mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI);
      }
    }

    double ppg_scale_factor = 1.0;
    if (rzRules->obj->m_chart_context->chart)   //  This is not a plugin chart
      ppg_scale_factor = ppg->sfactor;


    I[3][0] = -(rzRules->sm_transform_parms->easting_vp_center - x_origin) *
              vp_plib.view_scale_ppm;
    I[3][1] = -(rzRules->sm_transform_parms->northing_vp_center -
                rzRules->obj->y_origin) *
              -vp_plib.view_scale_ppm;

    // Scale
    I[0][0] *= vp_plib.view_scale_ppm * ppg_scale_factor;
    I[1][1] *= -vp_plib.view_scale_ppm * ppg_scale_factor;

    // Rotate
    mat4x4_rotate_Z(Q, I, angle);

    // Translate
    Q[3][0] += vp_plib.pix_width / 2;
    Q[3][1] += vp_plib.pix_height / 2;

    shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)Q);

    float colorv[4];
    colorv[0] = c->R / float(256);
    colorv[1] = c->G / float(256);
    colorv[2] = c->B / float(256);
    colorv[3] = 1.0;

    shader->SetUniform4fv("color", colorv);

    if (b_useVBO){
      glBindBuffer(GL_ARRAY_BUFFER, rzRules->obj->auxParm0);
      glVertexAttribPointer(pos, 2, array_gl_type, GL_FALSE, 0,
                                (GLvoid *)(0));
    }
    else
      glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0,
                            (GLvoid *)(ppg->single_buffer));
   int VBO_offset_index = 0;

    while (p_tp) {
      LLBBox box;
      if (!rzRules->obj->m_chart_context->chart) {  // This is s63 PlugIn Chart
        LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
        box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
      } else
        box = p_tp->tri_box;

      if (!BBView.IntersectOut(box)) {
        glDrawArrays(p_tp->type, VBO_offset_index, p_tp->nVert);
      }

      VBO_offset_index += p_tp->nVert;

      // pick up the next in chain
      if (!rzRules->obj->m_chart_context->chart) {  // This is a PlugIn Chart
        LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
        p_tp = (TriPrim *)p_ltp->p_next;
      } else
        p_tp = p_tp->p_next;

    }  // while

    if (b_useVBO)
      glBindBuffer(GL_ARRAY_BUFFER, 0);

    mat4x4 IM;
    mat4x4_identity(IM);
    shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)IM);

    glDisableVertexAttribArray(pos);
    shader->UnBind();

    if (b_useVBO && b_temp_vbo) {
      glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
      glDeleteBuffers(1, (unsigned int *)&rzRules->obj->auxParm0);
      rzRules->obj->auxParm0 = 0;
    }
  }  // if pPolyTessGeo

  return 1;
}
#endif

void s52plib::SetGLClipRect(const VPointCompat &vp, const wxRect &rect) {
}

void RotateToViewPort(const VPointCompat &vp) {
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  bool g_bskew_comp = true;

  float angle = vp.rotation;
  //if (g_bskew_comp) angle -= vp.skew;

  if (fabs(angle) > 0.0001) {
    //    Rotations occur around 0,0, so translate to rotate around screen
    //    center
    float xt = vp.pix_width / 2.0, yt = vp.pix_height / 2.0;

    glTranslatef(xt, yt, 0);
    glRotatef(angle * 180. / PI, 0, 0, 1);
    glTranslatef(-xt, -yt, 0);
  }
#endif
}

int s52plib::RenderToGLAP(ObjRazRules *rzRules, Rules *rules) {
#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  return RenderToGLAP_GLSL(rzRules, rules);
#else
  return 1;
#endif  // #ifdef ocpnUSE_GLSL

}
#ifdef ocpnUSE_GL
int s52plib::RenderToGLAP_GLSL(ObjRazRules *rzRules, Rules *rules) {
  if (!ObjectRenderCheckPosReduced(rzRules))
    return false;

  //    Get the pattern definition
  if ((rules->razRule->pixelPtr == NULL) ||
      (rules->razRule->parm1 != m_colortable_index) ||
      (rules->razRule->parm0 != ID_GL_PATT_SPEC)) {
    render_canvas_parms *patt_spec =
        CreatePatternBufferSpec(rzRules, rules, false, true);

    ClearRulesCache(
        rules->razRule);  //  Clear out any existing cached symbology

    rules->razRule->pixelPtr = patt_spec;
    rules->razRule->parm1 = m_colortable_index;
    rules->razRule->parm0 = ID_GL_PATT_SPEC;
  }

  //  Render the Area using the pattern spec stored in the rules
  render_canvas_parms *ppatt_spec =
      (render_canvas_parms *)rules->razRule->pixelPtr;

  //    Has the pattern been uploaded as a texture?
  if (!ppatt_spec->OGL_tex_name) {
    GLuint tex_name;
    glGenTextures(1, &tex_name);
    ppatt_spec->OGL_tex_name = tex_name;

    glBindTexture(GL_TEXTURE_2D, tex_name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ppatt_spec->w_pot,
                 ppatt_spec->h_pot, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 ppatt_spec->pix_buff);
  }

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, ppatt_spec->OGL_tex_name);

  glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_COLOR, GL_ZERO);

  int textureWidth = ppatt_spec->width;    // ppatt_spec->w_pot;
  int textureHeight = ppatt_spec->height;  // ppatt_spec->h_pot;

  wxPoint pr;
  GetPointPixSingle(rzRules, rzRules->obj->m_lat, rzRules->obj->m_lat, &pr);
  float xOff = pr.x;
  float yOff = pr.y;

  LLBBox BBView = GetReducedBBox();
  // please untangle this logic with the logic below
  if (BBView.GetMaxLon() + 180 < vp_plib.clon)
    BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() + 360, BBView.GetMaxLat(),
               BBView.GetMaxLon() + 360);
  else if (BBView.GetMinLon() - 180 > vp_plib.clon)
    BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() - 360, BBView.GetMaxLat(),
               BBView.GetMaxLon() - 360);

  //  Allow a little slop in calculating whether a triangle
  //  is within the requested VPointCompat
  double margin = BBView.GetLonRange() * .05;
  BBView.EnLarge(margin);

  bool b_useVBO = m_GLAC_VBO && !rzRules->obj->auxParm1;

  if (rzRules->obj->pPolyTessGeo) {
    //bool b_temp_vbo = false;
    //bool b_transform = false;

    // perform deferred tesselation
    if (!rzRules->obj->pPolyTessGeo->IsOk())
      rzRules->obj->pPolyTessGeo->BuildDeferredTess();

    //  Get the vertex data
    PolyTriGroup *ppg_vbo = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

    //  Has the input vertex buffer been converted to "single_alloc float"
    //  model? and is it allowed?
    if (!ppg_vbo->bsingle_alloc && (rzRules->obj->auxParm1 >= 0)) {
      int data_size = sizeof(float);

      //  First calculate the required total byte size
      int total_byte_size = 0;
      TriPrim *p_tp = ppg_vbo->tri_prim_head;
      while (p_tp) {
        total_byte_size += p_tp->nVert * 2 * data_size;
        p_tp = p_tp->p_next;  // pick up the next in chain
      }

      float *vbuf = (float *)malloc(total_byte_size);
      p_tp = ppg_vbo->tri_prim_head;

      if (ppg_vbo->data_type == DATA_TYPE_DOUBLE) {  // DOUBLE to FLOAT
        float *p_run = vbuf;
        while (p_tp) {
          float *pfbuf = p_run;
          for (int i = 0; i < p_tp->nVert * 2; ++i) {
            float x = (float)(p_tp->p_vertex[i]);
            *p_run++ = x;
          }

          free(p_tp->p_vertex);
          p_tp->p_vertex = (double *)pfbuf;

          p_tp = p_tp->p_next;  // pick up the next in chain
        }
      } else {  // FLOAT to FLOAT
        float *p_run = vbuf;
        while (p_tp) {
          memcpy(p_run, p_tp->p_vertex, p_tp->nVert * 2 * sizeof(float));

          free(p_tp->p_vertex);
          p_tp->p_vertex = (double *)p_run;

          p_run += p_tp->nVert * 2 * sizeof(float);

          p_tp = p_tp->p_next;  // pick up the next in chain
        }
      }

      ppg_vbo->bsingle_alloc = true;
      ppg_vbo->single_buffer = (unsigned char *)vbuf;
      ppg_vbo->single_buffer_size = total_byte_size;
      ppg_vbo->data_type = DATA_TYPE_FLOAT;
    }

    if (b_useVBO) {
      //  Has a VBO been built for this object?
      if (1) {
        if (rzRules->obj->auxParm0 <= 0) {

          GLuint vboId;
          // generate a new VBO and get the associated ID
          glGenBuffers(1, &vboId);

          rzRules->obj->auxParm0 = vboId;

          // bind VBO in order to use
          glBindBuffer(GL_ARRAY_BUFFER, vboId);

          // upload data to VBO
          glBufferData(GL_ARRAY_BUFFER, ppg_vbo->single_buffer_size,
                       ppg_vbo->single_buffer, GL_STATIC_DRAW);

        } else {
          glBindBuffer(GL_ARRAY_BUFFER, rzRules->obj->auxParm0);
        }
      }
    }

    PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
    double ppg_scale_factor = 1.0;
    if (rzRules->obj->m_chart_context->chart)   //  This is not a plugin chart
      ppg_scale_factor = ppg->sfactor;

    TriPrim *p_tp = ppg->tri_prim_head;
    GLintptr vbo_offset = 0;

    //      Set up the stride sizes for the array
    int array_data_size = sizeof(float);
    GLint array_gl_type = GL_FLOAT;

    if (ppg->data_type == DATA_TYPE_DOUBLE) {
      array_data_size = sizeof(double);
      array_gl_type = GL_DOUBLE;
    }

    if (ppg->data_type == DATA_TYPE_SHORT) {
      array_data_size = sizeof(short);
      array_gl_type = GL_SHORT;
    }

    GLint program = S52AP_shader_program;
    glUseProgram(program);

    // Disable VBO's (vertex buffer objects) for attributes.
    if (!b_useVBO) glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLint pos = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(pos);

    // Select the active texture unit.
    glActiveTexture(GL_TEXTURE0);

    // Bind our texture to the texturing target.
    glBindTexture(GL_TEXTURE_2D, ppatt_spec->OGL_tex_name);

    // Set up the texture sampler to texture unit 0
    GLint texUni = glGetUniformLocation(program, "uTex");
    glUniform1i(texUni, 0);

    GLint texWidth = glGetUniformLocation(program, "texWidth");
    GLint texHeight = glGetUniformLocation(program, "texHeight");
    glUniform1f(texWidth, textureWidth);
    glUniform1f(texHeight, textureHeight);

    GLint texPOTWidth = glGetUniformLocation(program, "texPOTWidth");
    GLint texPOTHeight = glGetUniformLocation(program, "texPOTHeight");
    glUniform1f(texPOTWidth, ppatt_spec->w_pot);
    glUniform1f(texPOTHeight, ppatt_spec->h_pot);

    GLint xo = glGetUniformLocation(program, "xOff");
    GLint yo = glGetUniformLocation(program, "yOff");

    glUniform1f(xo, fmod(xOff, ppatt_spec->w_pot));
    glUniform1f(yo, fmod(yOff, ppatt_spec->h_pot));

    GLint yom = glGetUniformLocation(program, "yOffM");
    glUniform1f(yom, yOff);

    if (ppatt_spec->b_stagger) {
      GLint staggerFact = glGetUniformLocation(program, "staggerFactor");
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
    I[3][0] = -(rzRules->sm_transform_parms->easting_vp_center -
                rzRules->obj->x_origin) *
              vp_plib.view_scale_ppm;
    I[3][1] = -(rzRules->sm_transform_parms->northing_vp_center -
                rzRules->obj->y_origin) *
              -vp_plib.view_scale_ppm;

    // Scale
    I[0][0] *= vp_plib.view_scale_ppm * ppg_scale_factor;
    I[1][1] *= -vp_plib.view_scale_ppm * ppg_scale_factor;

    // Rotate
    mat4x4_rotate_Z(Q, I, angle);

    // Translate
    Q[3][0] += vp_plib.pix_width / 2;
    Q[3][1] += vp_plib.pix_height / 2;

    GLint matloc = glGetUniformLocation(program, "TransformMatrix");
    glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);

    if (!b_useVBO) {
      float *bufBase = (float *)(&ppg->single_buffer[vbo_offset]);
      glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                            bufBase);
    }

    while (p_tp) {
      LLBBox box;
      if (!rzRules->obj->m_chart_context->chart) {  // This is a PlugIn Chart
        LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
        box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
      } else
        box = p_tp->tri_box;

      if (!BBView.IntersectOut(box)) {
        if (b_useVBO) {
          glVertexAttribPointer(pos, 2, array_gl_type, GL_FALSE,
                                0 /*2*sizeof(array_gl_type)*/,
                                (GLvoid *)(vbo_offset));
          glDrawArrays(p_tp->type, 0, p_tp->nVert);
        } else {
          glDrawArrays(p_tp->type, vbo_offset / (2 * array_data_size),
                       p_tp->nVert);
        }
      }

      vbo_offset += p_tp->nVert * 2 * array_data_size;

      // pick up the next in chain
      if (!rzRules->obj->m_chart_context->chart) {  // This is a PlugIn Chart
        LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
        p_tp = (TriPrim *)p_ltp->p_next;
      } else
        p_tp = p_tp->p_next;

    }  // while

    if (b_useVBO) glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);

    //  Restore the transform matrix to identity
    mat4x4 IM;
    mat4x4_identity(IM);
    GLint matlocf =
        glGetUniformLocation(S52color_tri_shader_program, "TransformMatrix");
    glUniformMatrix4fv(matlocf, 1, GL_FALSE, (const GLfloat *)IM);
    glUseProgram(0);
    glDisableVertexAttribArray(pos);


  }  // if pPolyTessGeo

  glDisable(GL_TEXTURE_2D);

  return 1;
}
#endif

#ifdef ocpnUSE_GL

int s52plib::RenderAreaToGL(const wxGLContext &glcc, ObjRazRules *rzRules) {
  if (!ObjectRenderCheckRules(rzRules, true)) return 0;

  Rules *rules = rzRules->LUP->ruleList;

  while (rules != NULL) {
    switch (rules->ruleType) {
      case RUL_ARE_CO:
        RenderToGLAC(rzRules, rules);
        break;  // AC

      case RUL_ARE_PA:
        RenderToGLAP(rzRules, rules);
        break;  // AP

      case RUL_CND_SY: {
#ifdef __OCPN__ANDROID__
        // Catch a difficult to trap SIGSEGV, avoiding app crash
        sigaction(SIGSEGV, NULL,
                  &sa_all_plib_previous);  // save existing action for this signal

        struct sigaction temp;
        sigaction(SIGSEGV, NULL,
                  &temp);  // inspect existing action for this signal

        temp.sa_handler = catch_signals_plib;  // point to my handler
        sigemptyset(&temp.sa_mask);             // make the blocking set
                                                // empty, so that all
                                                // other signals will be
                                                // unblocked during my handler
        temp.sa_flags = 0;
        sigaction(SIGSEGV, &temp, NULL);

        if (sigsetjmp(env_plib, 1))  //  Something in the
                                //  code block below this on caused SIGSEGV...
        {
          // reset signal handler
          sigaction(SIGSEGV, &sa_all_plib_previous, NULL);
          return 0;
        }
        else
#endif
        {
          if (!rzRules->obj->bCS_Added) {
            rzRules->obj->CSrules = NULL;
            GetAndAddCSRules(rzRules, rules);
            rzRules->obj->bCS_Added = 1;  // mark the object
          }
          Rules *rules_last = rules;
          rules = rzRules->obj->CSrules;

          while (NULL != rules) {
            switch (rules->ruleType) {
              case RUL_ARE_CO:
                RenderToGLAC(rzRules, rules);
                break;
              case RUL_ARE_PA:
                RenderToGLAP(rzRules, rules);
                break;
              case RUL_NONE:
              default:
                break;  // no rule type (init)
            }
            rules_last = rules;
            rules = rules->next;
          }

          rules = rules_last;
          break;
        }
#ifdef __OCPN__ANDROID__
        // reset signal handler
        sigaction(SIGSEGV, &sa_all_plib_previous, NULL);
#endif

      }

      case RUL_NONE:
      default:
        break;  // no rule type (init)
    }           // switch

    rules = rules->next;
  }

  return 1;
}
#endif

render_canvas_parms *s52plib::CreatePatternBufferSpec(ObjRazRules *rzRules,
                                                      Rules *rules,
                                                      bool b_revrgb,
                                                      bool b_pot) {
  wxImage Image;

  Rule *prule = rules->razRule;

  bool bstagger_pattern = (prule->fillType.PATP == 'S');

  wxColour local_unused_wxColor = m_unused_wxColor;

  //      Create a wxImage of the pattern drawn on an "unused_color" field
  if (prule->definition.SYDF == 'R') {
    Image = m_chartSymbols.GetImage(prule->name.PANM);
  }

  else  // Vector
  {
    float fsf = 100 / canvas_pix_per_mm;

    // Base bounding box
    BoundingBox box(
        prule->pos.patt.bnbox_x.PBXC, prule->pos.patt.bnbox_y.PBXR,
        prule->pos.patt.bnbox_x.PBXC + prule->pos.patt.bnbox_w.PAHL,
        prule->pos.patt.bnbox_y.PBXR + prule->pos.patt.bnbox_h.PAVL);

    // Expand to include pivot
    box.Expand(prule->pos.patt.pivot_x.PACL, prule->pos.patt.pivot_y.PARW);

    //    Pattern bounding boxes may be offset from origin, to preset the
    //    spacing So, the bitmap must be delta based.
    double dwidth = box.GetWidth();
    double dheight = box.GetHeight();

    //  Add in the pattern spacing parameters
    dwidth += prule->pos.patt.minDist.PAMI;
    dheight += prule->pos.patt.minDist.PAMI;

    //  Prescale
    dwidth /= fsf;
    dheight /= fsf;

    int width = (int)dwidth + 1;
    int height = (int)dheight + 1;

    //float render_scale = 1.0;
#ifdef sUSE_ANDROID_GLES2
    int width_pot = width;
    int height_pot = height;
    if (b_pot) {
      int xp = width;
      if (((xp != 0) && !(xp & (xp - 1))))  // detect POT
        width_pot = xp;
      else {
        int a = 0;
        while (xp) {
          xp = xp >> 1;
          a++;
        }
        width_pot = 1 << a;
      }

      xp = height;
      if (((xp != 0) && !(xp & (xp - 1))))
        height_pot = xp;
      else {
        int a = 0;
        while (xp) {
          xp = xp >> 1;
          a++;
        }
        height_pot = 1 << a;
      }
    }

    // adjust scaler
    render_scale = (float)height_pot / (float)height;
    qDebug() << "first" << width << width_pot << height << height_pot
             << render_scale;

    width = width_pot;
    height = height_pot;
#endif

    //      Instantiate the vector pattern to a wxBitmap
    wxMemoryDC mdc;

//  TODO
// This ought to work for wxOSX, but DOES NOT.
// We we do not want anti-aliased lines drawn in the pattern spec, since we used
// the solid primary color as a mask for manual blitting from the dc to memory
// buffer.

// Best we can do is set the background color very dark, and hope for the best
#ifdef __WXOSX__
#if wxUSE_GRAPHICS_CONTEXT
    wxGraphicsContext *pgc = mdc.GetGraphicsContext();
    if (pgc) pgc->SetAntialiasMode(wxANTIALIAS_NONE);
#endif
    local_unused_wxColor.Set(2, 2, 2);
#endif

    wxBitmap *pbm = NULL;

    if ((0 != width) && (0 != height)) {
      pbm = new wxBitmap(width, height);

      mdc.SelectObject(*pbm);
      mdc.SetBackground(wxBrush(local_unused_wxColor));
      mdc.Clear();

      int pivot_x = prule->pos.patt.pivot_x.PACL;
      int pivot_y = prule->pos.patt.pivot_y.PARW;

      char *str = prule->vector.LVCT;
      char *col = prule->colRef.LCRF;
      wxPoint pivot(pivot_x, pivot_y);

      int origin_x = prule->pos.patt.bnbox_x.PBXC;
      int origin_y = prule->pos.patt.bnbox_y.PBXR;
      wxPoint origin(origin_x, origin_y);

      wxPoint r0((int)((pivot_x - box.GetMinX()) / fsf) + 1,
                 (int)((pivot_y - box.GetMinY()) / fsf) + 1);

      HPGL->SetTargetDC(&mdc);
      HPGL->SetVP(&vp_plib);
      HPGL->Render(str, col, r0, pivot, origin, 1.0 /*render_scale*/, 0, false);

      //                 mdc.SetPen( wxPen( wxColor(0, 0, 250), 1,
      //                 wxPENSTYLE_SOLID ) );
      //                 mdc.SetBrush(*wxTRANSPARENT_BRUSH);
      //                 mdc.DrawRectangle(0,0, width-1, height-1);

    } else {
      pbm = new wxBitmap(2, 2);  // substitute small, blank pattern
      mdc.SelectObject(*pbm);
      mdc.SetBackground(wxBrush(local_unused_wxColor));
      mdc.Clear();
    }

    mdc.SelectObject(wxNullBitmap);

    //    Build a wxImage from the wxBitmap
    Image = pbm->ConvertToImage();

    delete pbm;
  }

  //  Convert the wxImage to a populated render_canvas_parms struct

  int sizey = Image.GetHeight();
  int sizex = Image.GetWidth();

  render_canvas_parms *patt_spec = new render_canvas_parms;
  patt_spec->OGL_tex_name = 0;

  if (b_pot) {
    int xp = sizex;
    if (((xp != 0) && !(xp & (xp - 1))))  // detect POT
      patt_spec->w_pot = sizex;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      patt_spec->w_pot = 1 << a;
    }

    xp = sizey;
    if (((xp != 0) && !(xp & (xp - 1))))
      patt_spec->h_pot = sizey;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      patt_spec->h_pot = 1 << a;
    }

  } else {
    patt_spec->w_pot = sizex;
    patt_spec->h_pot = sizey;
  }

  patt_spec->depth = 32;  // set the depth, always 32 bit

  patt_spec->pb_pitch = ((patt_spec->w_pot * patt_spec->depth / 8));
  patt_spec->lclip = 0;
  patt_spec->rclip = patt_spec->w_pot - 1;
  patt_spec->pix_buff =
      (unsigned char *)malloc(patt_spec->h_pot * patt_spec->pb_pitch);

  // Preset background
  memset(patt_spec->pix_buff, 0, patt_spec->h_pot * patt_spec->pb_pitch);
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
  if (Image.HasAlpha()) {
    imgAlpha = Image.GetAlpha();
    b_use_alpha = true;
  }

#if defined(__WXMAC__) || defined(__WXQT__)

  if (prule->definition.SYDF == 'V') {
    b_use_alpha = true;
    imgAlpha = NULL;
  }
#endif

  unsigned char primary_r = 0;
  unsigned char primary_g = 0;
  unsigned char primary_b = 0;
  double reference_value = 0.5;

  bool b_filter = false;
#if defined(__WXMAC__) || defined(__WXGTK3__)
  S52color *primary_color = 0;
  if (prule->definition.SYDF == 'V') {
    b_filter = true;
    char *col = prule->colRef.LCRF;
    primary_color = getColor(col + 1);
    if (primary_color) {
      primary_r = primary_color->R;
      primary_g = primary_color->G;
      primary_b = primary_color->B;
      wxImage::RGBValue rgb(primary_r, primary_g, primary_b);
      wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
      reference_value = hsv.value;
    }
  }
#endif

  unsigned char *ps;

  {
    unsigned char mr = local_unused_wxColor.Red();
    unsigned char mg = local_unused_wxColor.Green();
    unsigned char mb = local_unused_wxColor.Blue();

    if (pd0 && ps0) {
      for (int iy = 0; iy < sizey; iy++) {
#ifdef __OCPN__ANDROID__
        pd = pd0 + ((sizey - iy - 1) * patt_spec->pb_pitch);
#else
        pd = pd0 + (iy * patt_spec->pb_pitch);
#endif
        ps = ps0 + (iy * sizex * 3);
        for (int ix = 0; ix < sizex; ix++) {
          if (ix < sizex) {
            unsigned char r = *ps++;
            unsigned char g = *ps++;
            unsigned char b = *ps++;

            if (b_filter) {
              wxImage::RGBValue rgb(r, g, b);
              wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
              double ratio = hsv.value / reference_value;

              if (ratio > 0.5) {
                *pd++ = primary_r;
                *pd++ = primary_g;
                *pd++ = primary_b;
                *pd++ = 255;
              } else {
                *pd++ = 0;
                *pd++ = 0;
                *pd++ = 0;
                *pd++ = 0;
              }
            } else {
#ifdef ocpnUSE_ocpnBitmap
              if (b_revrgb) {
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
              if (b_use_alpha && imgAlpha) {
                *pd++ = *imgAlpha++;
              } else {
                *pd++ = ((r == mr) && (g == mg) && (b == mb) ? 0 : 255);
              }
            }
          }
        }
      }
    }
  }

  return patt_spec;
}

int s52plib::RenderToBufferAP(ObjRazRules *rzRules, Rules *rules,
                              render_canvas_parms *pb_spec) {
  //if (vp->m_projection_type != PROJECTION_MERCATOR) return 1;

  wxImage Image;

  if (rules->razRule == NULL) return 0;

  if ((rules->razRule->pixelPtr == NULL) ||
      (rules->razRule->parm1 != m_colortable_index) ||
      (rules->razRule->parm0 != ID_RGB_PATT_SPEC)) {
    render_canvas_parms *patt_spec =
        CreatePatternBufferSpec(rzRules, rules, true);

    ClearRulesCache(
        rules->razRule);  //  Clear out any existing cached symbology

    rules->razRule->pixelPtr = patt_spec;
    rules->razRule->parm1 = m_colortable_index;
    rules->razRule->parm0 = ID_RGB_PATT_SPEC;

  }  // Instantiation done

  //  Render the Area using the pattern spec stored in the rules
  render_canvas_parms *ppatt_spec =
      (render_canvas_parms *)rules->razRule->pixelPtr;

  //  Set the pattern reference point

  wxPoint r;
  GetPointPixSingle(rzRules, rzRules->obj->y, rzRules->obj->x, &r);

  ppatt_spec->x =
      r.x - 2000000;  // bias way down to avoid zero-crossing logic in dda
  ppatt_spec->y = r.y - 2000000;

  RenderToBufferFilledPolygon(rzRules, rzRules->obj, NULL, pb_spec, ppatt_spec);

  return 1;
}

int s52plib::RenderToBufferAC(ObjRazRules *rzRules, Rules *rules,
                              render_canvas_parms *pb_spec) {
  //if (vp->m_projection_type != PROJECTION_MERCATOR) return 1;

  S52color *c;
  char *str = (char *)rules->INSTstr;

  c = getColor(str);

  RenderToBufferFilledPolygon(rzRules, rzRules->obj, c, pb_spec, NULL);

  //    At very small scales, the object could be visible on both the left and
  //    right sides of the screen. Identify this case......
  if (vp_plib.chart_scale > 5e7) {
    //    Does the object hang out over the left side of the VP?
    if ((rzRules->obj->BBObj.GetMaxLon() > GetBBox().GetMinLon()) &&
        (rzRules->obj->BBObj.GetMinLon() < GetBBox().GetMinLon())) {
      //    If we add 360 to the objects lons, does it intersect the the right
      //    side of the VP?
      if (((rzRules->obj->BBObj.GetMaxLon() + 360.) >
           GetBBox().GetMaxLon()) &&
          ((rzRules->obj->BBObj.GetMinLon() + 360.) <
           GetBBox().GetMaxLon())) {
        //  If so, this area oject should be drawn again, this time for the left
        //  side
        //    Do this by temporarily adjusting the objects rendering offset
        rzRules->obj->x_origin -=
            mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
        RenderToBufferFilledPolygon(rzRules, rzRules->obj, c, pb_spec, NULL);
        rzRules->obj->x_origin +=
            mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
      }
    }
  }

  return 1;
}

int s52plib::RenderAreaToDC(wxDC *pdcin, ObjRazRules *rzRules,
                            render_canvas_parms *pb_spec) {
  if (!ObjectRenderCheckRules(rzRules, true)) return 0;

  m_pdc = pdcin;  // use this DC
  Rules *rules = rzRules->LUP->ruleList;

  while (rules != NULL) {
    switch (rules->ruleType) {
      case RUL_ARE_CO:
        RenderToBufferAC(rzRules, rules, pb_spec);
        break;  // AC
      case RUL_ARE_PA:
        RenderToBufferAP(rzRules, rules, pb_spec);
        break;  // AP

      case RUL_CND_SY: {
        if (!rzRules->obj->bCS_Added) {
          rzRules->obj->CSrules = NULL;
          GetAndAddCSRules(rzRules, rules);
          rzRules->obj->bCS_Added = 1;  // mark the object
        }
        Rules *rules_last = rules;
        rules = rzRules->obj->CSrules;

        //    The CS procedure may have changed the Display Category of the
        //    Object, need to check again for visibility
        if (ObjectRenderCheckCat(rzRules)) {
          while (NULL != rules) {
            // Hve seen drgare fault here, need to code area query to debug
            // possible that RENDERtoBUFFERAP/AC is blowing obj->CSRules
            //    When it faults here, look at new debug field obj->CSLUP
            switch (rules->ruleType) {
              case RUL_ARE_CO:
                RenderToBufferAC(rzRules, rules, pb_spec);
                break;
              case RUL_ARE_PA:
                RenderToBufferAP(rzRules, rules, pb_spec);
                break;
              case RUL_NONE:
              default:
                break;  // no rule type (init)
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
        break;  // no rule type (init)
    }           // switch

    rules = rules->next;
  }

  return 1;
}

void s52plib::GetAndAddCSRules(ObjRazRules *rzRules, Rules *rules) {
  LUPrec *NewLUP;
  LUPrec *LUP;
  LUPrec *LUPCandidate;
  wxString cs_string;

  char *rule_str1 = RenderCS(rzRules, rules);
  if (rule_str1)
    cs_string = wxString(rule_str1, wxConvUTF8);
  free(rule_str1);  // delete rule_str1;

  //  Try to find a match for this object/attribute set in dynamic CS LUP Table

  //  Do this by checking each LUP in the CS LUPARRAY and checking....
  //  a) is Object Name the same? and
  //  b) was LUP created earlier by exactly the same INSTruction string?
  //  c) does LUP have same Display Category and Priority?

  wxArrayOfLUPrec *la = condSymbolLUPArray;
  int index = 0;
  int index_max = la->GetCount();
  LUP = NULL;

  while ((index < index_max)) {
    LUPCandidate = la->Item(index);
    if (!strcmp(rzRules->LUP->OBCL, LUPCandidate->OBCL)) {
      if (LUPCandidate->INST.IsSameAs(cs_string)) {
        if (LUPCandidate->DISC == rzRules->LUP->DISC) {
          LUP = LUPCandidate;
          break;
        }
      }
    }
    index++;
  }

  //  If not found, need to create a dynamic LUP and add to CS LUP Table

  if (NULL == LUP)  // Not found
  {
    NewLUP = new LUPrec();
    NewLUP->DISC = rzRules->LUP->DISC;  // as a default

    memcpy(NewLUP->OBCL, rzRules->LUP->OBCL, 6);  // the object class name

    //      Add the complete CS string to the LUP
    if(cs_string.Length())
      NewLUP->INST = cs_string;

    _LUP2rules(NewLUP, rzRules->obj);

    // Add LUP to array
    wxArrayOfLUPrec *pLUPARRAYtyped = condSymbolLUPArray;

    pLUPARRAYtyped->Add(NewLUP);

    LUP = NewLUP;

  }  // if (LUP = NULL)

  Rules *top = LUP->ruleList;

  rzRules->obj->CSrules = top;  // patch in a new rule set
}

bool s52plib::ObjectRenderCheck(ObjRazRules *rzRules) {
  if (!ObjectRenderCheckPos(rzRules)) return false;

  if (!ObjectRenderCheckCat(rzRules)) return false;

  return true;
}

bool s52plib::ObjectRenderCheckCS(ObjRazRules *rzRules) {
  //  We need to do this test since some CS procedures change the display
  //  category So we need to tentatively process all objects with CS LUPs
  Rules *rules = rzRules->LUP->ruleList;
  while (rules != NULL) {
    if (RUL_CND_SY == rules->ruleType) return true;

    rules = rules->next;
  }

  return false;
}

bool s52plib::ObjectRenderCheckPos(ObjRazRules *rzRules) {
  if (rzRules->obj == NULL) return false;

  // Of course, the object must be at least partly visible in the VPointCompat
  const LLBBox &vpBox = GetBBox();
  const LLBBox &testBox = rzRules->obj->BBObj;

  if (vpBox.GetMaxLat() < testBox.GetMinLat() ||
      vpBox.GetMinLat() > testBox.GetMaxLat())
    return false;

  if (vpBox.GetMaxLon() >= testBox.GetMinLon() &&
      vpBox.GetMinLon() <= testBox.GetMaxLon())
    return true;

  if (vpBox.GetMaxLon() >= testBox.GetMinLon() + 360 &&
      vpBox.GetMinLon() <= testBox.GetMaxLon() + 360)
    return true;

  if (vpBox.GetMaxLon() >= testBox.GetMinLon() - 360 &&
      vpBox.GetMinLon() <= testBox.GetMaxLon() - 360)
    return true;

  return false;
}

bool s52plib::ObjectRenderCheckPosReduced(ObjRazRules *rzRules) {
  if (rzRules->obj == NULL) return false;

  // Of course, the object must be at least partly visible in the VPointCompat
  LLBBox vpBox = GetReducedBBox();
  const LLBBox &testBox = rzRules->obj->BBObj;

  if (vpBox.GetMaxLat() < testBox.GetMinLat() ||
      vpBox.GetMinLat() > testBox.GetMaxLat())
    return false;

  if (vpBox.GetMaxLon() >= testBox.GetMinLon() &&
      vpBox.GetMinLon() <= testBox.GetMaxLon())
    return true;

  if (vpBox.GetMaxLon() >= testBox.GetMinLon() + 360 &&
      vpBox.GetMinLon() <= testBox.GetMaxLon() + 360)
    return true;

  if (vpBox.GetMaxLon() >= testBox.GetMinLon() - 360 &&
      vpBox.GetMinLon() <= testBox.GetMaxLon() - 360)
    return true;

  return false;
}

bool s52plib::ObjectRenderCheckCat(ObjRazRules *rzRules) {
  g_scaminScale = 1.0;

  if (rzRules->obj == NULL) return false;

  bool b_catfilter = true;
  bool b_visible = false;

  //      Do Object Type Filtering
  DisCat obj_cat = rzRules->obj->m_DisplayCat;

  //  Meta object filter.
  // Applied when showing display category OTHER, and
  // only for objects whose decoded S52 display category (by LUP) is also OTHER
  if (m_nDisplayCategory == OTHER) {
    if (OTHER == obj_cat) {
      if (!strncmp(rzRules->LUP->OBCL, "M_", 2)){
        if (!m_bShowMeta)
          return false;
        else {
          if (!strncmp(rzRules->LUP->OBCL, "M_QUAL", 6) && !m_qualityOfDataOn)
            return false;
        }
      }
    }
  } else {
    // We want to filter out M_NSYS objects everywhere except "OTHER" category
    if (!strncmp(rzRules->LUP->OBCL, "M_", 2))
      if (!m_bShowMeta) return false;
  }

#ifdef __OCPN__ANDROID__
  // We want to filter out M_NSYS objects on Android, as they are of limited use
  // on a phone/tablet
  if (!strncmp(rzRules->LUP->OBCL, "M_", 2))
    if (!m_bShowMeta) return false;
#endif

  if (m_nDisplayCategory == MARINERS_STANDARD) {
    if (-1 == rzRules->obj->iOBJL) UpdateOBJLArray(rzRules->obj);

    if (DISPLAYBASE == obj_cat) {  // always display individual objects that
                                   // were moved to DISPLAYBASE by CS Procedures
      b_visible = true;
      b_catfilter = false;
    } else if (!((OBJLElement *)(pOBJLArray->Item(rzRules->obj->iOBJL)))
                    ->nViz) {
      b_catfilter = false;
    }
  }

  else if (m_nDisplayCategory == OTHER) {
    if ((DISPLAYBASE != obj_cat) && (STANDARD != obj_cat) &&
        (OTHER != obj_cat)) {
      b_catfilter = false;
    }
  }

  else if (m_nDisplayCategory == STANDARD) {
    if ((DISPLAYBASE != obj_cat) && (STANDARD != obj_cat)) {
      b_catfilter = false;
    }
  } else if (m_nDisplayCategory == DISPLAYBASE) {
    if (DISPLAYBASE != obj_cat) {
      b_catfilter = false;
    }
  }

  //  Soundings override
  if (!strncmp(rzRules->LUP->OBCL, "SOUNDG", 6)) b_catfilter = m_bShowSoundg;

  if (b_catfilter) {
    b_visible = true;

    //      SCAMIN Filtering
    //      Implementation note:
    //      According to S52 specs, SCAMIN must not apply to GROUP1 objects,
    //      Meta Objects or DisplayCategoryBase objects. Occasionally, an ENC
    //      will encode a spurious SCAMIN value for one of these objects. see,
    //      for example, US5VA18M, in OpenCPN SENC as Feature 350(DEPARE), LNAM
    //      = 022608187ED20ACC. We shall explicitly ignore SCAMIN filtering for
    //      these types of objects.

    if (m_bUseSCAMIN) {
      if ((DISPLAYBASE == rzRules->LUP->DISC) ||
          (PRIO_GROUP1 == rzRules->LUP->DPRI))
        b_visible = true;
      else {
        //                if( vp->chart_scale > rzRules->obj->Scamin ) b_visible
        //                = false;

        double zoom_mod = (double)m_chart_zoom_modifier_vector;

        double modf = zoom_mod / 5.;  // -1->1
        double mod = pow(8., modf);
        mod = wxMax(mod, .2);
        mod = wxMin(mod, 8.0);

        if (mod > 1) {
          if (vp_plib.chart_scale > rzRules->obj->Scamin * mod)
            b_visible = false;  // definitely invisible
          else {
            //  Theoretically invisible, however...
            //  In the "zoom modified" scale region,
            //  we render the symbol at reduced size, scaling down to no less
            //  than half normal size.

            if (vp_plib.chart_scale > rzRules->obj->Scamin) {
              double xs = vp_plib.chart_scale - rzRules->obj->Scamin;
              double xl = (rzRules->obj->Scamin * mod) - rzRules->obj->Scamin;
              g_scaminScale = 1.0 - (0.5 * xs / xl);
            }
          }
        } else {
          if (vp_plib.chart_scale > rzRules->obj->Scamin) b_visible = false;
        }
      }

      // Check for SUPER_SCAMIN, apply if enabled
      if (m_bUseSUPER_SCAMIN){
        if (rzRules->obj->SuperScamin < 0){
          if ( (strncmp(rzRules->obj->FeatureName, "LNDARE", 6) &&
                strncmp(rzRules->obj->FeatureName, "DEPARE", 6) &&
                strncmp(rzRules->obj->FeatureName, "SWPARE", 6) &&
                strncmp(rzRules->obj->FeatureName, "RECTRK", 6) &&
                strncmp(rzRules->obj->FeatureName, "TSS",    3) &&
                strncmp(rzRules->obj->FeatureName, "TSEZNE", 6) &&
                strncmp(rzRules->obj->FeatureName, "DRGARE", 6) &&
                strncmp(rzRules->obj->FeatureName, "COALNE", 6)) ||
              (!strncmp(rzRules->obj->FeatureName, "LNDARE", 6) && (rzRules->LUP->ruleList->ruleType != RUL_ARE_CO))) {

            double chart_ref_scale = rzRules->obj->m_chart_context->chart_scale;

            // Is the ENC cell SCAMIN for this object un-defined?
            if (rzRules->obj->Scamin > 1e8) {   // undefined default value is 1e8+2
              // Get the scale of the ENC, and establish SUPERSCAMIN
              double super_scamin = chart_ref_scale * 4;
              rzRules->obj->SuperScamin = super_scamin;
            }
            if (rzRules->obj->Scamin > 9e6) {   // Presumed undefined value for Greek ENC Lights
              // Get the scale of the ENC, and establish SUPERSCAMIN
              double super_scamin = chart_ref_scale * 2;
              rzRules->obj->SuperScamin = super_scamin;
            }
            if (!strncmp(rzRules->obj->FeatureName, "SOUNDG", 6)){
                if (rzRules->obj->Scamin > 4e6) {   // Presumed undefined value for Greek ENC soundings
                  // Get the scale of the ENC, and establish SUPERSCAMIN
                  double super_scamin = chart_ref_scale * 2;
                  rzRules->obj->SuperScamin = super_scamin;
              }
            }
          }
        }

        // Make the test
        if ((rzRules->obj->SuperScamin > 0) &&
             (vp_plib.chart_scale > rzRules->obj->SuperScamin))
            b_visible = false;

      }


      //      On the other hand, $TEXTS features need not really be displayed at
      //      all scales, always To do so makes a very cluttered display
      if ((!strncmp(rzRules->LUP->OBCL, "$TEXTS", 6)) &&
          (vp_plib.chart_scale > rzRules->obj->Scamin))
        b_visible = false;
    }

    return b_visible;
  }

  return b_visible;
}

bool s52plib::ObjectRenderCheckDates(ObjRazRules *rzRules) {
  // Check DATSTA/DATEND attributes, if present
  // DATSTA
  wxString datsta = rzRules->obj->GetAttrValueAsString("DATSTA");
  if (datsta.Len() > 0) {
    bool bDateValid = false;

    // CCYYMMDD
    wxDateTime objDate;
    wxString::const_iterator end;
    if (objDate.ParseFormat(datsta, "%Y%m%d", &end)) {
      if (end == datsta.end()) {  // Require perfect parsing
        if (objDate.IsValid()) bDateValid = true;
      }
    }
    if (bDateValid) {
      objDate.ResetTime();  // DATSTA specifications take effect at 00:01- on
                            // the specified date
      wxDateTime now = wxDateTime::Now();
      if (now.IsEarlierThan(objDate)) return false;  // No Show
    }
  }

  // DATEND
  datsta = rzRules->obj->GetAttrValueAsString("DATEND");
  if (datsta.Len() > 0) {
    bool bDateValid = false;

    // CCYYMMDD
    wxDateTime objDate;
    wxString::const_iterator end;
    if (objDate.ParseFormat(datsta, "%Y%m%d", &end)) {
      if (end == datsta.end()) {  // Require perfect parsing
        if (objDate.IsValid()) bDateValid = true;
      }
    }
    if (bDateValid) {
      objDate.ResetTime();  // DATEND specifications take effect at 23:59+ on
                            // the specified date.
      objDate += wxTimeSpan(24);
      wxDateTime now = wxDateTime::Now();
      if (now.IsLaterThan(objDate)) return false;  // No Show
    }
  }

  // PEREND
  datsta = rzRules->obj->GetAttrValueAsString("PEREND");
  if ((datsta.Len() > 0) && !datsta.StartsWith(_T("--"))) {
    bool bDateValid = false;

    // CCYYMMDD
    wxDateTime objDate;
    wxString::const_iterator end;
    if (objDate.ParseFormat(datsta, "%Y%m%d", &end)) {
      if (end == datsta.end()) {  // Require perfect parsing
        if (objDate.IsValid()) bDateValid = true;
      }
    }
    if (bDateValid) {
      objDate.ResetTime();  // PEREND specifications take effect at 23:59+ on
                            // the specified date.
      objDate += wxTimeSpan(24);
      wxDateTime now = wxDateTime::Now();
      if (now.IsLaterThan(objDate)) return false;  // No Show
    }
  }

  return true;
}

bool s52plib::ObjectRenderCheckRules(ObjRazRules *rzRules, bool check_noshow) {
  if (!ObjectRenderCheckPos(rzRules)) return false;

  // The Feature M_QUAL, in MARINERS_STANDARD catagory, is a special case,
  // since it is also controlled by a global hotkey in display category ALL and
  // MARINERS_STANDARD
  if (m_nDisplayCategory == MARINERS_STANDARD) {
    if (strncmp(rzRules->obj->FeatureName, "M_QUAL",
                6)) {  // Anything other than M_QUAL
      if (check_noshow && IsObjNoshow(rzRules->LUP->OBCL)) return 0;
    } else {
      if (!m_qualityOfDataOn) return 0;
    }
  } else {
    if (check_noshow && IsObjNoshow(rzRules->LUP->OBCL)) return false;
  }

  if (ObjectRenderCheckCat(rzRules)) {
    return ObjectRenderCheckDates(rzRules);
  }

  //  If this object cannot be moved to a higher category by CS procedures,
  //  then we are done here
  if (!rzRules->obj->m_bcategory_mutable) return false;

  // already added, nothing below can change its display category
  if (rzRules->obj->bCS_Added) return false;

  //  Otherwise, make sure the CS, if present, has been evaluated,
  //  and then check the category again
  //  no rules
  if (!ObjectRenderCheckCS(rzRules)) return false;

  rzRules->obj->CSrules = NULL;
  Rules *rules = rzRules->LUP->ruleList;
  while (rules != NULL) {
    if (RUL_CND_SY == rules->ruleType) {
      GetAndAddCSRules(rzRules, rules);
      rzRules->obj->bCS_Added = 1;  // mark the object
      break;
    }
    rules = rules->next;
  }

  // still not displayable
  if (!ObjectRenderCheckCat(rzRules)) return false;

  return ObjectRenderCheckDates(rzRules);
}

void s52plib::SetDisplayCategory(enum _DisCat cat) {
  enum _DisCat old = m_nDisplayCategory;
  m_nDisplayCategory = cat;

  if (old != cat) {
    ClearNoshow();
  }
  GenerateStateHash();
}

bool s52plib::IsObjNoshow(const char *objcl) {
  for (unsigned int i = 0; i < m_noshow_array.GetCount(); i++) {
    if (!strncmp(m_noshow_array[i].obj, objcl, 6)) return true;
  }
  return false;
}

void s52plib::AddObjNoshow(const char *objcl) {
  if (!IsObjNoshow(objcl)) {
    noshow_element element;
    memcpy(element.obj, objcl, 6);
    m_noshow_array.Add(element);
  }
}

void s52plib::RemoveObjNoshow(const char *objcl) {
  for (unsigned int i = 0; i < m_noshow_array.GetCount(); i++) {
    if (!strncmp(m_noshow_array[i].obj, objcl, 6)) {
      m_noshow_array.RemoveAt(i);
      return;
    }
  }
}

void s52plib::ClearNoshow(void) { m_noshow_array.Clear(); }

#if 0
void s52plib::PLIB_LoadS57Config() {
  //    Get a pointer to the opencpn configuration object
  wxFileConfig *pconfig = GetOCPNConfigObject();

  int read_int;
  double dval;

  pconfig->SetPath(_T ( "/Settings" ));
  // pconfig->Read( _T ( "DebugS57" ), &g_PIbDebugS57, 0 );         // Show LUP
  // and Feature info in object query

  pconfig->SetPath(_T ( "/Settings/GlobalState" ));

  pconfig->Read(_T ( "bShowS57Text" ), &read_int, 0);
  SetShowS57Text(!(read_int == 0));

  pconfig->Read(_T ( "bShowS57ImportantTextOnly" ), &read_int, 0);
  SetShowS57ImportantTextOnly(!(read_int == 0));

  pconfig->Read(_T ( "bShowLightDescription" ), &read_int, 0);
  SetShowLdisText(!(read_int == 0));

  pconfig->Read(_T ( "bExtendLightSectors" ), &read_int, 0);
  SetExtendLightSectors(!(read_int == 0));

  pconfig->Read(_T ( "nDisplayCategory" ), &read_int, (enum _DisCat)STANDARD);
  SetDisplayCategory((enum _DisCat)read_int);

  pconfig->Read(_T ( "nSymbolStyle" ), &read_int, (enum _LUPname)PAPER_CHART);
  m_nSymbolStyle = (LUPname)read_int;

  pconfig->Read(_T ( "nBoundaryStyle" ), &read_int, PLAIN_BOUNDARIES);
  m_nBoundaryStyle = (LUPname)read_int;

  pconfig->Read(_T ( "bShowSoundg" ), &read_int, 1);
  m_bShowSoundg = !(read_int == 0);

  pconfig->Read(_T ( "bShowMeta" ), &read_int, 0);
  m_bShowMeta = !(read_int == 0);

  pconfig->Read(_T ( "bUseSCAMIN" ), &read_int, 1);
  m_bUseSCAMIN = !(read_int == 0);

  pconfig->Read(_T ( "bShowAtonText" ), &read_int, 1);
  m_bShowAtonText = !(read_int == 0);

  pconfig->Read(_T ( "bDeClutterText" ), &read_int, 0);
  m_bDeClutterText = !(read_int == 0);

  pconfig->Read(_T ( "bShowNationalText" ), &read_int, 0);
  m_bShowNationalTexts = !(read_int == 0);

  if (pconfig->Read(_T ( "S52_MAR_SAFETY_CONTOUR" ), &dval, 5.0)) {
    S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, dval);
    S52_setMarinerParam(S52_MAR_SAFETY_DEPTH,
                        dval);  // Set safety_contour and safety_depth the same
  }

  if (pconfig->Read(_T ( "S52_MAR_SHALLOW_CONTOUR" ), &dval, 3.0))
    S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval);

  if (pconfig->Read(_T ( "S52_MAR_DEEP_CONTOUR" ), &dval, 10.0))
    S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval);

  if (pconfig->Read(_T ( "S52_MAR_TWO_SHADES" ), &dval, 0.0))
    S52_setMarinerParam(S52_MAR_TWO_SHADES, dval);

  UpdateMarinerParams();

  pconfig->SetPath(_T ( "/Settings/GlobalState" ));
  pconfig->Read(_T ( "S52_DEPTH_UNIT_SHOW" ), &read_int,
                1);               // default is metres
  read_int = wxMax(read_int, 0);  // qualify value
  read_int = wxMin(read_int, 2);
  m_nDepthUnitDisplay = read_int;

  //    S57 Object Class Visibility

  OBJLElement *pOLE;

  pconfig->SetPath(_T ( "/Settings/ObjectFilter" ));

  int iOBJMax = pconfig->GetNumberOfEntries();
  if (iOBJMax) {
    wxString str;
    long val;
    long dummy;

    wxString sObj;

    bool bCont = pconfig->GetFirstEntry(str, dummy);
    while (bCont) {
      pconfig->Read(str, &val);  // Get an Object Viz

      bool bNeedNew = true;

      if (str.StartsWith(_T ( "viz" ), &sObj)) {
        for (unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++) {
          pOLE = (OBJLElement *)(pOBJLArray->Item(iPtr));
          if (!strncmp(pOLE->OBJLName, sObj.mb_str(), 6)) {
            pOLE->nViz = val;
            bNeedNew = false;
            break;
          }
        }

        if (bNeedNew) {
          pOLE = (OBJLElement *)calloc(sizeof(OBJLElement), 1);
          memcpy(pOLE->OBJLName, sObj.mb_str(), OBJL_NAME_LEN);
          pOLE->nViz = 1;

          pOBJLArray->Add((void *)pOLE);
        }
      }
      bCont = pconfig->GetNextEntry(str, dummy);
    }
  }
}
#endif

void s52plib::PLIB_LoadS57GlobalConfig(wxFileConfig *pconfig)
{
    int read_int;
    double dval;

    pconfig->SetPath( _T ( "/Settings" ) );

    pconfig->SetPath( _T ( "/Settings/GlobalState" ) );

    pconfig->Read( _T ( "bShowS57ImportantTextOnly" ), &read_int, 0 );
    SetShowS57ImportantTextOnly( !( read_int == 0 ) );

    pconfig->Read( _T ( "nSymbolStyle" ), &read_int, (enum _LUPname) PAPER_CHART );
    m_nSymbolStyle = (LUPname) read_int;

    pconfig->Read( _T ( "nBoundaryStyle" ), &read_int, PLAIN_BOUNDARIES );
    m_nBoundaryStyle = (LUPname) read_int;

    pconfig->Read( _T ( "bShowMeta" ), &read_int, 0 );
    m_bShowMeta = !( read_int == 0 );

    pconfig->Read( _T ( "bUseSCAMIN" ), &read_int, 1 );
    m_bUseSCAMIN = !( read_int == 0 );

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

}


void s52plib::PLIB_LoadS57ObjectConfig(wxFileConfig *pconfig)
{
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
                    strncpy( pOLE->OBJLName, sObj.mb_str(), 6 );
                    pOLE->nViz = 1;

                    pOBJLArray->Add( (void *) pOLE );
                }
            }
            bCont = pconfig->GetNextEntry( str, dummy );
        }
    }
}

//    Do all those things necessary to prepare for a new rendering
void s52plib::PrepareForRender(void) { PrepareForRender( &vp_plib); }

void s52plib::PrepareForRender(VPointCompat *vp) {
  m_benableGLLS = true;  // default is to always use RenderToGLLS (VBO support)

#ifdef ocpnUSE_GL
  void PrepareS52ShaderUniforms(VPointCompat * vp);
  if (m_useGLSL && vp){
    PrepareS52ShaderUniforms(vp);

    // Prepare TexFont shader
    for (unsigned int i = 0; i < TXF_CACHE; i++) {
      if(s_txf[i].cache)
        s_txf[i].cache->PrepareShader(vp->pix_width, vp->pix_height, vp->rotation);
    }
  }
#endif

  m_ChartScaleFactorExp = GetOCPNChartScaleFactor_Plugin();

  // Reset the LIGHTS declutter machine
  lastLightLat = 0;
  lastLightLon = 0;

  // Precalulate the ENC scale factors
  m_SoundingsScaleFactor = (m_nSoundingFactor * .1) + 1; //exp(m_nSoundingFactor * (log(2.0) / 5.0));
  m_TextScaleFactor = exp(m_nTextFactor * (log(2.0) / 5.0));
}

void s52plib::SetAnchorOn(bool val) {
  const char *categories[] = {"ACHBRT", "ACHARE", "CBLSUB", "PIPARE",
                              "PIPSOL", "TUNNEL", "SBDARE"};
  unsigned int num = sizeof(categories) / sizeof(categories[0]);

  if ((m_nDisplayCategory == OTHER) ||
      (m_nDisplayCategory == MARINERS_STANDARD)) {
    bool bAnchor = val;

    if (!bAnchor) {
      for (unsigned int c = 0; c < num; c++) {
        AddObjNoshow(categories[c]);
      }
    } else {
      for (unsigned int c = 0; c < num; c++) {
        RemoveObjNoshow(categories[c]);
      }
    }
  } else {  // if not category OTHER, then anchor-related features are always
            // shown.
    for (unsigned int c = 0; c < num; c++) {
      RemoveObjNoshow(categories[c]);
    }
  }

  m_anchorOn = val;
}

void s52plib::SetQualityOfData(bool val) {

  if (!val) {  // going off
    AddObjNoshow("M_QUAL");
  } else {  // Off, going on
    RemoveObjNoshow("M_QUAL");

    for (unsigned int iPtr = 0; iPtr < pOBJLArray->GetCount(); iPtr++) {
      OBJLElement *pOLE = (OBJLElement *)(pOBJLArray->Item(iPtr));
      if (!strncmp(pOLE->OBJLName, "M_QUAL", 6)) {
        pOLE->nViz = 1;  // force on
        break;
      }
    }
  }

  m_qualityOfDataOn = val;
}

void s52plib::ClearTextList(void) {
  //      Clear the current text rectangle list
  m_textObjList.Clear();
}

bool s52plib::EnableGLLS(bool b_enable) {
  bool return_val = m_benableGLLS;
  m_benableGLLS = b_enable;
  return return_val;
}

void s52plib::AdjustTextList(int dx, int dy, int screenw, int screenh) {
  return;
  wxRect rScreen(0, 0, screenw, screenh);
  //    Iterate over the text rectangle list
  //        1.  Apply the specified offset to the list elements
  //        2.. Remove any list elements that are off screen after applied
  //        offset

  TextObjList::Node *node = m_textObjList.GetFirst();
  TextObjList::Node *next;
  while (node) {
    next = node->GetNext();
    wxRect *pcurrent = &(node->GetData()->rText);
    pcurrent->Offset(dx, dy);
    if (!pcurrent->Intersects(rScreen)) {
      m_textObjList.DeleteNode(node);
    }
    node = next;
  }
}

bool s52plib::GetPointPixArray(ObjRazRules *rzRules, wxPoint2DDouble *pd,
                               wxPoint *pp, int nv) {
  for (int i = 0; i < nv; i++) {
    GetPointPixSingle(rzRules, pd[i].m_y, pd[i].m_x, pp + i);
  }

  return true;
}

bool s52plib::GetPointPixSingle(ObjRazRules *rzRules, float north, float east,
                                wxPoint *r) {
  //if (vp->m_projection_type == PROJECTION_MERCATOR)
  {
    double xr = rzRules->obj->x_rate;
    double xo = rzRules->obj->x_origin;
    double yr = rzRules->obj->y_rate;
    double yo = rzRules->obj->y_origin;

    if (fabs(xo) > 1) {  // cm93 hits this
      if (GetBBox().GetMaxLon() >= 180. &&
          rzRules->obj->BBObj.GetMaxLon() < GetBBox().GetMinLon())
        xo += mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
      else if ((GetBBox().GetMinLon() <= -180. &&
                rzRules->obj->BBObj.GetMinLon() > GetBBox().GetMaxLon()) ||
               (rzRules->obj->BBObj.GetMaxLon() >= 180 &&
                GetBBox().GetMinLon() <= 0.))
        xo -= mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
    }

    double valx = (east * xr) + xo;
    double valy = (north * yr) + yo;

    r->x = roundint(((valx - rzRules->sm_transform_parms->easting_vp_center) *
                     vp_plib.view_scale_ppm) +
                    (vp_plib.pix_width / 2));
    r->y = roundint((vp_plib.pix_height / 2) -
                    ((valy - rzRules->sm_transform_parms->northing_vp_center) *
                     vp_plib.view_scale_ppm));
  }
//   else {
//     double lat, lon;
//     fromSM_Plugin(east - rzRules->sm_transform_parms->easting_vp_center,
//                   north - rzRules->sm_transform_parms->northing_vp_center,
//                   vp->clat, vp->clon, &lat, &lon);
//
//     *r = GetPixFromLL(north, east);
//   }

  return true;
}

void s52plib::GetPixPointSingle(int pixx, int pixy, double *plat, double *plon) {
#if 1
  GetLLFromPix(wxPoint(pixx, pixy), plat, plon);
//    if(*plon < 0 && vpt->clon > 180)
//      *plon += 360;
#else
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
#endif
}

void s52plib::GetPixPointSingleNoRotate(int pixx, int pixy, double *plat,
                                        double *plon) {
  double rotation = vp_plib.rotation;
  vp_plib.rotation = 0;
  GetLLFromPix(wxPoint(pixx, pixy), plat, plon);
  vp_plib.rotation = rotation;

}

void DrawAALine(wxDC *pDC, int x0, int y0, int x1, int y1, wxColour clrLine,
                int dash, int space) {
  int width = 1 + abs(x0 - x1);
  int height = 1 + abs(y0 - y1);
  wxPoint upperLeft(wxMin(x0, x1), wxMin(y0, y1));

  wxBitmap bm(width, height);
  wxMemoryDC mdc(bm);

  mdc.Blit(0, 0, width, height, pDC, upperLeft.x, upperLeft.y);

#if wxUSE_GRAPHICS_CONTEXT
  wxGCDC gdc(mdc);
#else
  wxMemoryDC &gdc(mdc);
#endif

  wxPen pen(clrLine, 1, wxPENSTYLE_USER_DASH);
  wxDash dashes[2];
  dashes[0] = dash;
  dashes[1] = space;
  pen.SetDashes(2, dashes);
  gdc.SetPen(pen);

  gdc.DrawLine(x0 - upperLeft.x, y0 - upperLeft.y, x1 - upperLeft.x,
               y1 - upperLeft.y);

  pDC->Blit(upperLeft.x, upperLeft.y, width, height, &mdc, 0, 0);

  mdc.SelectObject(wxNullBitmap);

  return;
}

void s52plib::DrawDashLine(wxPen &pen, wxCoord x1, wxCoord y1, wxCoord x2,
                           wxCoord y2) {
#ifdef ocpnUSE_GL
  glLineWidth(pen.GetWidth());

  CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
  shader->Bind();

  float fBuf[4];
  GLint pos = shader->getAttributeLocation("position");
  glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), fBuf);
  glEnableVertexAttribArray(pos);

  float colorv[4];
  colorv[0] = pen.GetColour().Red() / float(256);
  colorv[1] = pen.GetColour().Green() / float(256);
  colorv[2] = pen.GetColour().Blue() / float(256);
  colorv[3] = 1.0;

  shader->SetUniform4fv("color", colorv);

  if (fabs(vp_plib.rotation) > 0.01) {
    float cx = vp_plib.pix_width / 2.;
    float cy = vp_plib.pix_height / 2.;
    float c = cosf(-vp_plib.rotation);
    float s = sinf(-vp_plib.rotation);

    float xn = x1 - cx;
    float yn = y1 - cy;
    x1 = xn * c - yn * s + cx;
    y1 = xn * s + yn * c + cy;

    xn = x2 - cx;
    yn = y2 - cy;
    x2 = xn * c - yn * s + cx;
    y2 = xn * s + yn * c + cy;
  }

  wxDash *dashes;
  int n_dashes = pen.GetDashes(&dashes);
  if (n_dashes) {
    float angle = atan2f((float)(y2 - y1), (float)(x2 - x1));
    float cosa = cosf(angle);
    float sina = sinf(angle);
    float t1 = pen.GetWidth();

    float lpix = sqrtf(powf(x1 - x2, 2) + powf(y1 - y2, 2));
    float lrun = 0.;
    float xa = x1;
    float ya = y1;
    float ldraw = t1 * dashes[0];
    float lspace = t1 * dashes[1];

    ldraw = wxMax(ldraw, 4.0);
    lspace = wxMax(lspace, 4.0);
    lpix = wxMin(lpix, 2000.0);

    while (lrun < lpix) {
      //    Dash
      float xb = xa + ldraw * cosa;
      float yb = ya + ldraw * sina;

      if ((lrun + ldraw) >= lpix)  // last segment is partial draw
      {
        xb = x2;
        yb = y2;
      }

      fBuf[0] = xa;
      fBuf[1] = ya;
      fBuf[2] = xb;
      fBuf[3] = yb;

      glDrawArrays(GL_LINES, 0, 2);

      xa = xa + (lspace + ldraw) * cosa;
      ya = ya + (lspace + ldraw) * sina;
      lrun += lspace + ldraw;
    }
  } else {  // not dashed
    fBuf[0] = x1;
    fBuf[1] = y1;
    fBuf[2] = x2;
    fBuf[3] = y2;

    glDrawArrays(GL_LINES, 0, 2);
  }

  shader->UnBind();
#endif
}

/****************************************************************************/
/* Convert Lat/Lon <-> Simple Mercator                                      */
/****************************************************************************/
void toSM_plib(double lat, double lon, double lat0, double lon0, double *x,
          double *y) {
  double xlon = lon;

  /*  Make sure lon and lon0 are same phase */

  if ((lon * lon0 < 0.) && (fabs(lon - lon0) > 180.)) {
    lon < 0.0 ? xlon += 360.0 : xlon -= 360.0;
  }

  const double z = WGS84_semimajor_axis_meters * mercator_k0;

  *x = (xlon - lon0) * DEGREE * z;

  // y =.5 ln( (1 + sin t) / (1 - sin t) )
  const double s = sin(lat * DEGREE);
  const double y3 = (.5 * log((1 + s) / (1 - s))) * z;

  const double s0 = sin(lat0 * DEGREE);
  const double y30 = (.5 * log((1 + s0) / (1 - s0))) * z;
  *y = y3 - y30;
}

void fromSM_plib(double x, double y, double lat0, double lon0, double *lat,
            double *lon) {
  const double z = WGS84_semimajor_axis_meters * mercator_k0;

  const double s0 = sin(lat0 * DEGREE);
  const double y0 = (.5 * log((1 + s0) / (1 - s0))) * z;

  *lat = (2.0 * atan(exp((y0 + y) / z)) - PI / 2.) / DEGREE;

  *lon = lon0 + (x / (DEGREE * z));
}


// TODO: eliminate the use of this function
wxPoint s52plib::GetPixFromLL(double lat, double lon) {
  wxPoint2DDouble p = GetDoublePixFromLL(lat, lon);
  if (wxFinite(p.m_x) && wxFinite(p.m_y)){
    if( (abs(p.m_x) < 10000) && (abs(p.m_y) < 10000) )
      return wxPoint(wxRound(p.m_x), wxRound(p.m_y));
  }
  return wxPoint(INVALID_COORD, INVALID_COORD);
}

wxPoint2DDouble s52plib::GetDoublePixFromLL(double lat, double lon) {
  double easting = 0;
  double northing = 0;
  double xlon = lon;

  /*  Make sure lon and lon0 are same phase */
  if (xlon * vp_plib.clon < 0.) {
    if (xlon < 0.)
      xlon += 360.;
    else
      xlon -= 360.;
  }

  if (fabs(xlon - vp_plib.clon) > 180.) {
    if (xlon > vp_plib.clon)
      xlon -= 360.;
    else
      xlon += 360.;
  }

  toSM_plib(lat, xlon, vp_plib.clat, vp_plib.clon, &easting, &northing);

  if (!wxFinite(easting) || !wxFinite(northing))
    return wxPoint2DDouble(easting, northing);

  double epix = easting * vp_plib.view_scale_ppm;
  double npix = northing * vp_plib.view_scale_ppm;
  double dxr = epix;
  double dyr = npix;

  //    Apply VP Rotation
  double angle = vp_plib.rotation;

  if (angle) {
    dxr = epix * cos(angle) + npix * sin(angle);
    dyr = npix * cos(angle) - epix * sin(angle);
  }

  return wxPoint2DDouble((vp_plib.pix_width / 2.0) + dxr, (vp_plib.pix_height / 2.0) - dyr);
}

wxPoint s52plib::GetPixFromLLROT(double lat, double lon, double rotation) {
  wxPoint2DDouble p = GetDoublePixFromLLROT(lat, lon, rotation);
  if (wxFinite(p.m_x) && wxFinite(p.m_y)){
    if( (abs(p.m_x) < 10000) && (abs(p.m_y) < 10000) )
      return wxPoint(wxRound(p.m_x), wxRound(p.m_y));
  }
  return wxPoint(INVALID_COORD, INVALID_COORD);
}

wxPoint2DDouble s52plib::GetDoublePixFromLLROT(double lat, double lon, double rotation) {
  double easting = 0;
  double northing = 0;
  double xlon = lon;

  /*  Make sure lon and lon0 are same phase */
  if (xlon * vp_plib.clon < 0.) {
    if (xlon < 0.)
      xlon += 360.;
    else
      xlon -= 360.;
  }

  if (fabs(xlon - vp_plib.clon) > 180.) {
    if (xlon > vp_plib.clon)
      xlon -= 360.;
    else
      xlon += 360.;
  }

  toSM_plib(lat, xlon, vp_plib.clat, vp_plib.clon, &easting, &northing);

  if (!wxFinite(easting) || !wxFinite(northing))
    return wxPoint2DDouble(easting, northing);

  double epix = easting * vp_plib.view_scale_ppm;
  double npix = northing * vp_plib.view_scale_ppm;
  double dxr = epix;
  double dyr = npix;

  //    Apply VP Rotation
  double angle = rotation;

  if (angle) {
    dxr = epix * cos(angle) + npix * sin(angle);
    dyr = npix * cos(angle) - epix * sin(angle);
  }

  return wxPoint2DDouble((vp_plib.pix_width / 2.0) + dxr, (vp_plib.pix_height / 2.0) - dyr);
}

void s52plib::GetLLFromPix(const wxPoint2DDouble &p, double *lat,
                            double *lon) {
  double dx = p.m_x - (vp_plib.pix_width / 2.0);
  double dy = (vp_plib.pix_height / 2.0) - p.m_y;

  double xpr = dx;
  double ypr = dy;

  //    Apply VP Rotation
  double angle = vp_plib.rotation;

  if (angle) {
    xpr = (dx * cos(angle)) - (dy * sin(angle));
    ypr = (dy * cos(angle)) + (dx * sin(angle));
  }
  double d_east = xpr / vp_plib.view_scale_ppm;
  double d_north = ypr / vp_plib.view_scale_ppm;

  double slat = 0.0, slon = 0.0;
  fromSM_plib(d_east, d_north, vp_plib.clat, vp_plib.clon, &slat, &slon);

  *lat = slat;

  if (slon < -180.)
    slon += 360.;
  else if (slon > 180.)
    slon -= 360.;
  *lon = slon;
}



RenderFromHPGL::RenderFromHPGL(s52plib *plibarg) {
  plib = plibarg;
  renderToDC = false;
  renderToOpenGl = false;
  renderToGCDC = false;

  if (plib)
    scaleFactor = 100.0 / plib->GetPPMM();
  else
    scaleFactor = 10.0;  // Nominal

  workBufSize = 0;
  workBufIndex = 0;
  workBuf = NULL;

  s_odc_tess_work_buf = NULL;
  s_odc_tess_vertex_idx = 0;
  s_odc_tess_vertex_idx_this = 0;
  s_odc_tess_buf_len = 0;

  transparency = 255;
}

RenderFromHPGL::~RenderFromHPGL() {
#ifdef ocpnUSE_GL
  if (renderToOpenGl) {
    glDisable(GL_BLEND);
  }

  free(workBuf);
  free(s_odc_tess_work_buf);

#endif
}

void RenderFromHPGL::SetTargetDC(wxDC *pdc) {
  targetDC = pdc;
  renderToDC = true;
  renderToOpenGl = false;
  renderToGCDC = false;
}

void RenderFromHPGL::SetTargetOpenGl() {
  renderToOpenGl = true;
  renderToDC = false;
  renderToGCDC = false;
}

#if wxUSE_GRAPHICS_CONTEXT
void RenderFromHPGL::SetTargetGCDC(wxGCDC *gdc) {
  targetGCDC = gdc;
  renderToGCDC = true;
  renderToDC = false;
  renderToOpenGl = false;
}
#endif

const char *RenderFromHPGL::findColorNameInRef(char colorCode, char *col) {
  int noColors = strlen(col) / 6;
  for (int i = 0, j = 0; i < noColors; i++, j += 6) {
    if (*(col + j) == colorCode) return col + j + 1;
  }
  return col + 1;  // Default to first color if not found.
}

wxPoint RenderFromHPGL::ParsePoint(wxString &argument) {
  long x, y;
  int colon = argument.Index(',');
  argument.Left(colon).ToLong(&x);
  argument.Mid(colon + 1).ToLong(&y);
  return wxPoint(x, y);
}

void RenderFromHPGL::SetPen() {
  float nominal_line_width_pix =
      wxMax(1.0, floor(plib->GetPPMM() /
                       5.0));  // 0.2 mm nominal, but not less than 1 pixel
  int pen_width_mod = floor(penWidth * nominal_line_width_pix);

  pen =
      wxThePenList->FindOrCreatePen(penColor, pen_width_mod, wxPENSTYLE_SOLID);
  brush = wxTheBrushList->FindOrCreateBrush(penColor, wxBRUSHSTYLE_SOLID);

  if (renderToDC) {
    targetDC->SetPen(*pen);
    targetDC->SetBrush(*brush);
  }
#ifdef ocpnUSE_GL
  if (renderToOpenGl) {
    if (plib->GetGLPolygonSmoothing()) glEnable(GL_POLYGON_SMOOTH);

    int line_width = wxMax(plib->m_GLMinSymbolLineWidth, (float)penWidth * 0.7);
    glLineWidth(line_width);

    //  Scale the pen width dependent on the platform display resolution
    float nominal_line_width_pix =
        wxMax(1.0, floor(plib->GetPPMM() /
                         5.0));  // 0.2 mm nominal, but not less than 1 pixel
    // qDebug() << nominal_line_width_pix;
    line_width =
        wxMax(1/*m_GLMinSymbolLineWidth*/, (float)penWidth * nominal_line_width_pix);
    glLineWidth(line_width);

#ifndef __OCPN__ANDROID__
    if (line_width >= 2 && plib->GetGLLineSmoothing())
      glEnable(GL_LINE_SMOOTH);
    else
      glDisable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
#endif
  }
#endif
#if wxUSE_GRAPHICS_CONTEXT
  if (renderToGCDC) {
    pen = wxThePenList->FindOrCreatePen(penColor, penWidth, wxPENSTYLE_SOLID);
    brush = wxTheBrushList->FindOrCreateBrush(penColor, wxBRUSHSTYLE_SOLID);
    targetGCDC->SetPen(*pen);
    targetGCDC->SetBrush(*brush);
  }
#endif
}

void RenderFromHPGL::Line(wxPoint from, wxPoint to) {
  if (renderToDC) {
    targetDC->DrawLine(from, to);
  }
#ifdef ocpnUSE_GL
  if (renderToOpenGl) {
    CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
    shader->Bind();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    float colorv[4];
    colorv[0] = penColor.Red() / float(256);
    colorv[1] = penColor.Green() / float(256);
    colorv[2] = penColor.Blue() / float(256);
    colorv[3] = transparency / float(256);

    shader->SetUniform4fv("color", colorv);

    float pts[4];
    pts[0] = from.x;
    pts[1] = from.y;
    pts[2] = to.x;
    pts[3] = to.y;

    GLint pos = shader->getAttributeLocation("position");
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), pts);
    glEnableVertexAttribArray(pos);

    glDrawArrays(GL_LINES, 0, 2);

    shader->UnBind();
  }
#endif
#if wxUSE_GRAPHICS_CONTEXT
  if (renderToGCDC) {
    targetGCDC->DrawLine(from, to);
  }
#endif
}

void RenderFromHPGL::Circle(wxPoint center, int radius, bool filled) {
  if (renderToDC) {
    if (filled)
      targetDC->SetBrush(*brush);
    else
      targetDC->SetBrush(*wxTRANSPARENT_BRUSH);
    targetDC->DrawCircle(center, radius);
  }
#ifdef ocpnUSE_GL
  if (renderToOpenGl) {
    if (!m_vp)  // oops, forgot to set the VP parameters
      return;

    //      Enable anti-aliased lines, at best quality
    glEnable(GL_BLEND);

    float coords[8];
    coords[0] = center.x - radius;
    coords[1] = center.y + radius;
    coords[2] = center.x + radius;
    coords[3] = center.y + radius;
    coords[4] = center.x - radius;
    coords[5] = center.y - radius;
    coords[6] = center.x + radius;
    coords[7] = center.y - radius;

    glUseProgram(S52circle_filled_shader_program);

    // Get pointers to the attributes in the program.
    GLint mPosAttrib =
        glGetAttribLocation(S52circle_filled_shader_program, "aPos");

    // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords);
    glEnableVertexAttribArray(mPosAttrib);

    //  Circle radius
    GLint radiusloc =
        glGetUniformLocation(S52circle_filled_shader_program, "circle_radius");
    glUniform1f(radiusloc, radius);

    //  Circle center point
    GLint centerloc =
        glGetUniformLocation(S52circle_filled_shader_program, "circle_center");
    float ctrv[2];
    ctrv[0] = center.x;
    ctrv[1] = m_vp->pix_height - center.y;
    glUniform2fv(centerloc, 1, ctrv);

    //  Circle fill color
    float colorv[4];
    colorv[3] = 0.0;  // transparent default

    if (brush) {
      colorv[0] = brush->GetColour().Red() / float(256);
      colorv[1] = brush->GetColour().Green() / float(256);
      colorv[2] = brush->GetColour().Blue() / float(256);
      if (filled) colorv[3] = 1.0;
    }

    GLint colloc =
        glGetUniformLocation(S52circle_filled_shader_program, "circle_color");
    glUniform4fv(colloc, 1, colorv);

    //  Border color
    float bcolorv[4];
    bcolorv[0] = penColor.Red() / float(256);
    bcolorv[1] = penColor.Green() / float(256);
    bcolorv[2] = penColor.Blue() / float(256);
    bcolorv[3] = penColor.Alpha() / float(256);

    GLint bcolloc =
        glGetUniformLocation(S52circle_filled_shader_program, "border_color");
    glUniform4fv(bcolloc, 1, bcolorv);

    //  Border Width
    float nominal_line_width_pix =
        wxMax(1.0, floor(plib->GetPPMM() /
                         5.0));  // 0.2 mm nominal, but not less than 1 pixel
    float line_width =
        wxMax(1/*g_GLMinSymbolLineWidth*/, (float)penWidth * nominal_line_width_pix);

    GLint borderWidthloc =
        glGetUniformLocation(S52circle_filled_shader_program, "border_width");
    glUniform1f(borderWidthloc, line_width);

    // Perform the actual drawing.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //      Enable anti-aliased lines, at best quality
    glDisable(GL_BLEND);

  }
#endif
#if wxUSE_GRAPHICS_CONTEXT
  if (renderToGCDC) {
    if (filled)
      targetGCDC->SetBrush(*brush);
    else
      targetGCDC->SetBrush(*wxTRANSPARENT_BRUSH);

    targetGCDC->DrawCircle(center, radius);

    // wxGCDC doesn't update min/max X/Y properly for DrawCircle.
    targetGCDC->SetPen(*wxTRANSPARENT_PEN);
    targetGCDC->DrawPoint(center.x - radius, center.y);
    targetGCDC->DrawPoint(center.x + radius, center.y);
    targetGCDC->DrawPoint(center.x, center.y - radius);
    targetGCDC->DrawPoint(center.x, center.y + radius);
    targetGCDC->SetPen(*pen);
  }
#endif
}

void RenderFromHPGL::Polygon() {
  if (renderToDC) {
    targetDC->DrawPolygon(noPoints, polygon);
  }
#ifdef ocpnUSE_GL
  if (renderToOpenGl) {
    penColor.Set(penColor.Red(), penColor.Green(), penColor.Blue(),
                 transparency);
    pen = wxThePenList->FindOrCreatePen(penColor, penWidth, wxPENSTYLE_SOLID);
    brush = wxTheBrushList->FindOrCreateBrush(penColor, wxBRUSHSTYLE_SOLID);
    DrawPolygon(noPoints, polygon, 0, 0, 1.0, 0);

  }
#endif  // OpenGL

#if wxUSE_GRAPHICS_CONTEXT
  if (renderToGCDC) {
    targetGCDC->DrawPolygon(noPoints, polygon);
  }
#endif
}

void RenderFromHPGL::RotatePoint(wxPoint &point, wxPoint origin, double angle) {
  if (angle == 0.) return;
  double sin_rot = sin(angle * PI / 180.);
  double cos_rot = cos(angle * PI / 180.);

  double xp =
      ((point.x - origin.x) * cos_rot) - ((point.y - origin.y) * sin_rot);
  double yp =
      ((point.x - origin.x) * sin_rot) + ((point.y - origin.y) * cos_rot);

  point.x = (int)xp + origin.x;
  point.y = (int)yp + origin.y;
}

bool RenderFromHPGL::Render(char *str, char *col, wxPoint &r, wxPoint &pivot,
                            wxPoint origin, float scale, double rot_angle,
                            bool bSymbol) {

  wxPoint lineStart;
  wxPoint lineEnd;

  scaleFactor = 100.0 / plib->GetPPMM();

  scaleFactor /= scale;
  scaleFactor /= g_scaminScale;

  if (bSymbol) scaleFactor /= plib->GetRVScaleFactor();

  // SW is not always defined, cf. US/US4CA17M/US4CA17M.000
  penWidth = 1;

  wxStringTokenizer commands(wxString(str, wxConvUTF8), _T(";"));
  while (commands.HasMoreTokens()) {
    wxString command = commands.GetNextToken();
    wxString arguments = command.Mid(2);
    command = command.Left(2);

    if (command == _T("SP")) {
      S52color *color =
          plib->getColor(findColorNameInRef(arguments.GetChar(0), col));
      penColor = wxColor(color->R, color->G, color->B);
      brushColor = penColor;
      continue;
    }
    if (command == _T("SW")) {
      arguments.ToLong(&penWidth);
      continue;
    }
    if (command == _T("ST")) {
      long transIndex;
      arguments.ToLong(&transIndex);
      transparency = (4 - transIndex) * 64;
      transparency = wxMin(transparency, 255);
      transparency = wxMax(0, transparency);
      continue;
    }
    if (command == _T("PU")) {
      SetPen();
      lineStart = ParsePoint(arguments);
      RotatePoint(lineStart, origin, rot_angle);
      lineStart -= pivot;
      lineStart.x /= scaleFactor;
      lineStart.y /= scaleFactor;
      lineStart += r;
      continue;
    }
    if (command == _T("PD")) {
      if (arguments.Length() == 0) {
        lineEnd = lineStart;
        lineEnd.x++;
      } else {
        lineEnd = ParsePoint(arguments);
        RotatePoint(lineEnd, origin, rot_angle);
        lineEnd -= pivot;
        lineEnd.x /= scaleFactor;
        lineEnd.y /= scaleFactor;
        lineEnd += r;
      }
      Line(lineStart, lineEnd);
      lineStart = lineEnd;  // For next line.
      continue;
    }
    if (command == _T("CI")) {
      long radius;
      arguments.ToLong(&radius);
      radius = (int)radius / scaleFactor;
      Circle(lineStart, radius);
      continue;
    }
    if (command == _T("PM")) {
      noPoints = 1;
      polygon[0] = lineStart;

      if (arguments == _T("0")) {
        do {
          command = commands.GetNextToken();
          arguments = command.Mid(2);
          command = command.Left(2);

          if (command == _T("AA")) {
            wxLogWarning(_T("RenderHPGL: AA instruction not implemented."));
          }
          if (command == _T("CI")) {
            long radius;
            arguments.ToLong(&radius);
            radius = (int)radius / scaleFactor;
            Circle(lineStart, radius, HPGL_FILLED);
          }
          if (command == _T("PD")) {
            wxStringTokenizer points(arguments, _T(","));
            while (points.HasMoreTokens()) {
              long x, y;
              points.GetNextToken().ToLong(&x);
              points.GetNextToken().ToLong(&y);
              lineEnd = wxPoint(x, y);
              RotatePoint(lineEnd, origin, rot_angle);
              lineEnd -= pivot;
              lineEnd.x /= scaleFactor;
              lineEnd.y /= scaleFactor;
              lineEnd += r;
              polygon[noPoints++] = lineEnd;
            }
          }
        } while (command != _T("PM"));
      }
      continue;
    }
    if (command == _T("FP")) {
      SetPen();
      Polygon();
      continue;
    }

    // Only get here if non of the other cases did a continue.
    //        wxString msg( _T("RenderHPGL: The '%s' instruction is not
    //        implemented.") ); msg += wxString( command ); wxLogWarning( msg );
  }

  transparency = 255;

#ifdef ocpnUSE_GL
  if (renderToOpenGl) {
    glDisable(GL_BLEND);
    glUseProgram(0);
  }
#endif

  return true;
}

wxArrayPtrVoid s52gTesselatorVertices;

void RenderFromHPGL::DrawPolygon(int n, wxPoint points[], wxCoord xoffset,
                                 wxCoord yoffset, float scale, float angle) {
#ifdef ocpnUSE_GL
  //    if( 0 )
  // dc->DrawPolygon( n, points, xoffset, yoffset );
  //       else
  {
#ifdef __WXQT__
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_BLEND);

#else
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_BLEND);

#endif

    // ConfigurePen();
    glLineWidth(pen->GetWidth());

    glEnable(GL_BLEND);

    if (n > 4)
      DrawPolygonTessellated(n, points, xoffset, yoffset);
    else {  // n = 3 or 4, most common case for pre-tesselated shapes

      //  Grow the work buffer as necessary
      if (workBufSize < (size_t)n * 2) {
        workBuf = (float *)realloc(workBuf, (n * 4) * sizeof(float));
        workBufSize = n * 4;
      }

      for (int i = 0; i < n; i++) {
        workBuf[i * 2] = (points[i].x * scale);      // + xoffset;
        workBuf[i * 2 + 1] = (points[i].y * scale);  // + yoffset;
      }

      CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
      shader->Bind();

      // Get pointers to the attributes in the program.
      GLint mPosAttrib = shader->getAttributeLocation("position");

      // Disable VBO's (vertex buffer objects) for attributes.
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, workBuf);
      glEnableVertexAttribArray(mPosAttrib);

      //  Border color
      float bcolorv[4];
      bcolorv[0] = pen->GetColour().Red() / float(256);
      bcolorv[1] = pen->GetColour().Green() / float(256);
      bcolorv[2] = pen->GetColour().Blue() / float(256);
      bcolorv[3] = pen->GetColour().Alpha() / float(256);

      shader->SetUniform4fv("color", bcolorv);

      // Perform the actual drawing.
      glDrawArrays(GL_LINE_LOOP, 0, n);

      //  Fill color
      bcolorv[0] = brush->GetColour().Red() / float(256);
      bcolorv[1] = brush->GetColour().Green() / float(256);
      bcolorv[2] = brush->GetColour().Blue() / float(256);
      bcolorv[3] = brush->GetColour().Alpha() / float(256);

      shader->SetUniform4fv("color", bcolorv);

      // For the simple common case of a convex rectangle...
      //  swizzle the array points to enable GL_TRIANGLE_STRIP
      if (n == 4) {
        float x1 = workBuf[4];
        float y1 = workBuf[5];
        workBuf[4] = workBuf[6];
        workBuf[5] = workBuf[7];
        workBuf[6] = x1;
        workBuf[7] = y1;

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      } else if (n == 3) {
        glDrawArrays(GL_TRIANGLES, 0, 3);
      }

      shader->UnBind();

    }

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_BLEND);
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


// GLSL callbacks


static std::list<double *> odc_combine_work_data;
static void xs52_combineCallbackD(GLdouble coords[3], GLdouble *vertex_data[4],
                                 GLfloat weight[4], GLdouble **dataOut,
                                 void *data) {
  //     double *vertex = new double[3];
  //     odc_combine_work_data.push_back(vertex);
  //     memcpy(vertex, coords, 3*(sizeof *coords));
  //     *dataOut = vertex;
}

void xs52_vertexCallbackD_GLSL(GLvoid *vertex, void *data) {
  RenderFromHPGL *plib = (RenderFromHPGL *)data;

  // Grow the work buffer if necessary
  if (plib->s_odc_tess_vertex_idx > plib->s_odc_tess_buf_len - 8) {
    int new_buf_len = plib->s_odc_tess_buf_len + 100;
    GLfloat *tmp = plib->s_odc_tess_work_buf;

    plib->s_odc_tess_work_buf = (GLfloat *)realloc(
        plib->s_odc_tess_work_buf, new_buf_len * sizeof(GLfloat));
    if (NULL == plib->s_odc_tess_work_buf) {
      free(tmp);
      tmp = NULL;
    } else
      plib->s_odc_tess_buf_len = new_buf_len;
  }

  GLdouble *pointer = (GLdouble *)vertex;

  plib->s_odc_tess_work_buf[plib->s_odc_tess_vertex_idx++] = (float)pointer[0];
  plib->s_odc_tess_work_buf[plib->s_odc_tess_vertex_idx++] = (float)pointer[1];

  plib->s_odc_nvertex++;
}

void xs52_beginCallbackD_GLSL(GLenum mode, void *data) {
  RenderFromHPGL *plib = (RenderFromHPGL *)data;
  plib->s_odc_tess_vertex_idx_this = plib->s_odc_tess_vertex_idx;
  plib->s_odc_tess_mode = mode;
  plib->s_odc_nvertex = 0;
}

void xs52_endCallbackD_GLSL(void *data) {
// qDebug() << "End" << s_odc_nvertex << s_odc_tess_buf_len <<
// s_odc_tess_vertex_idx << s_odc_tess_vertex_idx_this; End 5 100 10 0
  RenderFromHPGL *plib = (RenderFromHPGL *)data;

  CGLShaderProgram *shader = pCcolor_tri_shader_program[0/*GetCanvasIndex()*/];
  shader->Bind();

  // Disable VBO's (vertex buffer objects) for attributes.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  float *bufPt = &(plib->s_odc_tess_work_buf[plib->s_odc_tess_vertex_idx_this]);
  GLint pos = glGetAttribLocation(S52color_tri_shader_program, "position");
  glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), bufPt);
  glEnableVertexAttribArray(pos);

  float colorv[4];
  wxColour c = plib->getBrush()->GetColour();

  colorv[0] = c.Red() / float(256);
  colorv[1] = c.Green() / float(256);
  colorv[2] = c.Blue() / float(256);
  colorv[3] = c.Alpha() / float(256);

  shader->SetUniform4fv("color", colorv);

  glDrawArrays(plib->s_odc_tess_mode, 0, plib->s_odc_nvertex);
  shader->UnBind();
}

#endif  //#ifdef ocpnUSE_GL

#ifdef __WXMSW__
// int filterException(int code, PEXCEPTION_POINTERS ex) {
//     //std::cout << "Filtering " << std::hex << code << std::endl;
//     return EXCEPTION_EXECUTE_HANDLER;
// }
#endif

void RenderFromHPGL::DrawPolygonTessellated(int n, wxPoint points[],
                                            wxCoord xoffset, wxCoord yoffset) {
#ifndef ocpnUSE_GL
   //dc->DrawPolygon( n, points, xoffset, yoffset );
#else
#if !defined(ocpnUSE_GLES) || \
    defined(USE_ANDROID_GLES2)  // tessalator in glues is broken
    if (n < 5)
#endif
    {
      DrawPolygon(n, points, xoffset, yoffset, 1.0, 0);
      return;
    }

    m_tobj = gluNewTess();
    s_odc_tess_vertex_idx = 0;

    gluTessCallback(m_tobj, GLU_TESS_VERTEX_DATA,
                    (_GLUfuncptrA) &xs52_vertexCallbackD_GLSL);
    gluTessCallback(m_tobj, GLU_TESS_BEGIN_DATA,
                    (_GLUfuncptrA)&xs52_beginCallbackD_GLSL);
    gluTessCallback(m_tobj, GLU_TESS_END_DATA,
                    (_GLUfuncptrA)&xs52_endCallbackD_GLSL);
    gluTessCallback(m_tobj, GLU_TESS_COMBINE_DATA,
                    (_GLUfuncptrA)&xs52_combineCallbackD);

    gluTessNormal(m_tobj, 0, 0, 1);
    gluTessProperty(m_tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

    gluTessBeginPolygon(m_tobj, this);
    gluTessBeginContour(m_tobj);

    //               ViewPort *pvp = cc1->GetpVP();
    double *p = new double [n * 3];
    for (int i = 0; i < n; i++) {
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
      p[3 * i] = points[i].x, p[3 * i + 1] = points[i].y, p[3 * i + 2] = 0;

      gluTessVertex(m_tobj, p + 3 * i, p + 3 * i);
    }

#ifdef __WXMSW__
//https://stackoverflow.com/questions/7049502/c-try-and-try-catch-finally
  __try{
    gluTessEndContour(m_tobj);
    gluTessEndPolygon(m_tobj);
    gluDeleteTess(m_tobj);
  }
//   __except(filterException(GetExceptionCode(), GetExceptionInformation())) {
  __except(EXCEPTION_EXECUTE_HANDLER) {
    int yyp = 4;
  }
#else
    gluTessEndContour(m_tobj);
    gluTessEndPolygon(m_tobj);
    gluDeleteTess(m_tobj);
#endif

    delete[] p;

    //         for(std::list<double*>::iterator i =
    //         odc_combine_work_data.begin(); i!=odc_combine_work_data.end();
    //         i++)
    //             delete [] *i;
    //         odc_combine_work_data.clear();

#endif
}

#ifdef ocpnUSE_GL
void PrepareS52ShaderUniforms(VPointCompat *vp) {

  loadS52Shaders();

  mat4x4 m;
  float vp_transform[16];
  mat4x4_identity(m);
  mat4x4_scale_aniso((float(*)[4])vp_transform, m, 2.0 / (float)vp->pix_width,
                     -2.0 / (float)vp->pix_height, 1.0);
  // Rotate
  mat4x4 Q;
  mat4x4_rotate_Z(Q, (float(*)[4])vp_transform, vp->rotation);
  mat4x4_translate_in_place(Q, -vp->pix_width / 2.0, -vp->pix_height / 2.0, 0);

  mat4x4 I;
  mat4x4_identity(I);

//   glUseProgram(S52color_tri_shader_program);
//   GLint matloc = glGetUniformLocation(S52color_tri_shader_program, "MVMatrix");
//   glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);
//   GLint transloc =
//       glGetUniformLocation(S52color_tri_shader_program, "TransformMatrix");
//   glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat *)I);

  glUseProgram(S52texture_2D_shader_program);
  GLint matloc = glGetUniformLocation(S52texture_2D_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);
  GLint transloc =
      glGetUniformLocation(S52texture_2D_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat *)I);

  glUseProgram(S52texture_2D_ColorMod_shader_program);
  matloc =
      glGetUniformLocation(S52texture_2D_ColorMod_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);
  transloc = glGetUniformLocation(S52texture_2D_ColorMod_shader_program,
                                  "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat *)I);

  glUseProgram(S52circle_filled_shader_program);
  matloc = glGetUniformLocation(S52circle_filled_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);
  transloc =
      glGetUniformLocation(S52circle_filled_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat *)I);

  glUseProgram(S52ring_shader_program);
  matloc = glGetUniformLocation(S52ring_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);
  transloc = glGetUniformLocation(S52ring_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat *)I);

  glUseProgram(S52Dash_shader_program);
  matloc = glGetUniformLocation(S52Dash_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);
  transloc = glGetUniformLocation(S52Dash_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat *)I);

  glUseProgram(S52AP_shader_program);
  matloc = glGetUniformLocation(S52AP_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)Q);
  transloc = glGetUniformLocation(S52AP_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat *)I);

  glUseProgram(0);

  // Load special shaders
  loadCShaders(0);

  CGLShaderProgram *shader = pCcolor_tri_shader_program[0];
  shader->Bind();
  shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)Q);
  shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);
  shader->UnBind();

  shader = pCtexture_2D_shader_program[0];
  shader->Bind();
  shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)Q);
  shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);
  shader->UnBind();

  shader = pCtexture_2D_Color_shader_program[0];
  shader->Bind();
  shader->SetUniformMatrix4fv("MVMatrix", (GLfloat *)Q);
  shader->SetUniformMatrix4fv("TransformMatrix", (GLfloat *)I);
  shader->UnBind();
}
#endif

//      CRC calculation for a byte buffer

static unsigned int crc_32_tab[] =
    {/* CRC polynomial 0xedb88320 */
     0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
     0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
     0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
     0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
     0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
     0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
     0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
     0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
     0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
     0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
     0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
     0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
     0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
     0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
     0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
     0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
     0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
     0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
     0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
     0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
     0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
     0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
     0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
     0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
     0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
     0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
     0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
     0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
     0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
     0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
     0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
     0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
     0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
     0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
     0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
     0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
     0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
     0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
     0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
     0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
     0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
     0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
     0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

#define UPDC32(octet, crc) \
  (crc_32_tab[((crc) ^ ((unsigned char)octet)) & 0xff] ^ ((crc) >> 8))

unsigned int crc32buf(unsigned char *buf, size_t len) {
  unsigned int oldcrc32;

  oldcrc32 = 0xFFFFFFFF;

  for (; len; --len, ++buf) {
    oldcrc32 = UPDC32(*buf, oldcrc32);
  }

  return ~oldcrc32;
}
