/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __QUIT_H__
#define __QUIT_H__

#include <vector>
#include "LLRegion.h"
#include "OCPNRegion.h"
#include "chcanv.h"
#include "viewport.h"
#include "chartdb.h"

extern bool g_bopengl;

struct ChartTableEntry;

class QuiltPatch {
public:
  QuiltPatch() {
    b_Valid = false;
    b_eclipsed = false;
    b_overlay = false;
  }
  int dbIndex;
  LLRegion ActiveRegion;
  int ProjType;
  bool b_Valid;
  bool b_eclipsed;
  bool b_overlay;
  LLRegion quilt_region;
};

class QuiltCandidate {
public:
  QuiltCandidate() {
    b_include = false;
    b_eclipsed = false;
    last_factor = -1;
  }

  const LLRegion &GetCandidateRegion();
  LLRegion &GetReducedCandidateRegion(double factor);
  void SetScale(int scale);
  bool Scale_eq(int b) const { return abs(ChartScale - b) <= rounding; }
  bool Scale_ge(int b) const { return Scale_eq(b) || ChartScale > b; }

  int dbIndex;
  int ChartScale;
  int rounding;
  bool b_include;
  bool b_eclipsed;

private:
  double last_factor;
  LLRegion reduced_candidate_region;
};

WX_DECLARE_LIST(QuiltPatch, PatchList);
WX_DEFINE_SORTED_ARRAY(QuiltCandidate *, ArrayOfSortedQuiltCandidates);

class Quilt {
public:
  Quilt(ChartCanvas *parent);
  ~Quilt();

  void SetQuiltParameters(double CanvasScaleFactor, int CanvasWidth) {
    m_canvas_scale_factor = CanvasScaleFactor;
    m_canvas_width = CanvasWidth;
  }

  void EnableHighDefinitionZoom(bool value) { m_b_hidef = value; }

  void UnlockQuilt();
  bool Compose(const ViewPort &vp);
  bool IsComposed() { return m_bcomposed; }
  ChartBase *GetFirstChart();
  ChartBase *GetNextChart();
  ChartBase *GetLargestScaleChart();
  ChartBase *GetNextSmallerScaleChart();

  std::vector<int> GetQuiltIndexArray(void);
  bool IsQuiltDelta(ViewPort &vp);
  bool IsChartQuiltableRef(int db_index);
  ViewPort &GetQuiltVP() { return m_vp_quilt; }
  wxString GetQuiltDepthUnit() { return m_quilt_depth_unit; }
  void SetRenderedVP(ViewPort &vp) { m_vp_rendered = vp; }
  bool HasOverlays(void) { return m_bquilt_has_overlays; }

  int GetExtendedStackCount(void) { return m_extended_stack_array.size(); }
  int GetFullScreenIndexCount(void) { return m_fullscreen_index_array.size(); }

  int GetnCharts() { return m_PatchList.GetCount(); }
  double GetBestStartScale(int dbi_ref_hint, const ViewPort &vp_in);

  void ComputeRenderRegion(ViewPort &vp, OCPNRegion &chart_region);
  bool RenderQuiltRegionViewOnDCNoText(wxMemoryDC &dc, ViewPort &vp,
                                       OCPNRegion &chart_region);
  bool RenderQuiltRegionViewOnDCTextOnly(wxMemoryDC &dc, ViewPort &vp,
                                         OCPNRegion &chart_region);

  bool IsVPBlittable(ViewPort &VPoint, int dx, int dy,
                     bool b_allow_vector = false);
  ChartBase *GetChartAtPix(ViewPort &VPoint, wxPoint p);
  ChartBase *GetOverlayChartAtPix(ViewPort &VPoint, wxPoint p);
  int GetChartdbIndexAtPix(ViewPort &VPoint, wxPoint p);
  void InvalidateAllQuiltPatchs(void);
  void Invalidate(void) {
    m_bcomposed = false;
    m_vp_quilt.Invalidate();
    m_zout_dbindex = -1;

    //  Quilting of skewed raster charts is allowed for OpenGL only
    m_bquiltskew = g_bopengl;
    //  Quilting of different projections is allowed for OpenGL only
    m_bquiltanyproj = g_bopengl;
  }
  void AdjustQuiltVP(ViewPort &vp_last, ViewPort &vp_proposed);

  LLRegion &GetFullQuiltRegion(void) { return m_covered_region; }
  OCPNRegion &GetFullQuiltRenderedRegion(void) { return m_rendered_region; }
  bool IsChartSmallestScale(int dbIndex);

  int AdjustRefOnZoomOut(double proposed_scale_onscreen);
  int AdjustRefOnZoomIn(double proposed_scale_onscreen);
  //    int AdjustRefOnZoom( bool b_zin, ChartFamilyEnum family, ChartTypeEnum
  //    type, double proposed_scale_onscreen );
  int AdjustRefSelection(const ViewPort &vp_in);

  void SetHiliteIndex(int index) { m_nHiLiteIndex = index; }
  void SetHiliteIndexArray(const std::vector<int> &index_array) {
    m_HiLiteIndexArray = index_array; }
  void ClearHiliteIndexArray() { m_HiLiteIndexArray.clear(); }

  void SetReferenceChart(int dbIndex) {
    m_refchart_dbIndex = dbIndex;
    if (dbIndex >= 0) {
      m_zout_family = -1;
    }
  }
  int GetRefChartdbIndex(void) { return m_refchart_dbIndex; }

  ChartBase *GetRefChart();

  int GetQuiltProj(void) { return m_quilt_proj; }
  double GetMaxErrorFactor() { return m_max_error_factor; }
  double GetRefScale() { return m_reference_scale; }

  ChartFamilyEnum GetRefFamily() { return (ChartFamilyEnum)m_reference_family; }

  void SetPreferrefFamily(ChartFamilyEnum family) {
    m_preferred_family = family;
  }

  double GetRefNativeScale();

  std::vector<int> GetCandidatedbIndexArray(bool from_ref_chart,
                                            bool exclude_user_hidden);
  std::vector<int> &GetExtendedStackIndexArray() {
    return m_extended_stack_array;
  }
  std::vector<int> GetEclipsedStackIndexArray() {
    return m_eclipsed_stack_array;
  }
  std::vector<int> &GetFullscreenIndexArray() {
    return m_fullscreen_index_array;
  }

  unsigned long GetXStackHash() { return m_xa_hash; }

  bool IsBusy() { return m_bbusy; }
  QuiltPatch *GetCurrentPatch();
  bool IsChartInQuilt(ChartBase *pc);
  bool IsChartInQuilt(wxString &full_path);

  bool IsQuiltVector(void);
  bool DoesQuiltContainPlugins(void);

  LLRegion GetHiliteRegion();
  std::vector<int> &GetHiLiteIndexArray(){ return m_HiLiteIndexArray; }

  static LLRegion GetChartQuiltRegion(const ChartTableEntry &cte, ViewPort &vp);

  int GetNomScaleMin(int scale, ChartTypeEnum type, ChartFamilyEnum family);
  int GetNomScaleMax(int scale, ChartTypeEnum type, ChartFamilyEnum family);
  ChartFamilyEnum GetPreferredFamily(void) { return m_preferred_family; }

private:
  bool BuildExtendedChartStackAndCandidateArray(int ref_db_index,
                                                ViewPort &vp_in);
  int AdjustRefOnZoom(bool b_zin, ChartFamilyEnum family, ChartTypeEnum type,
                      double proposed_scale_onscreen);

  bool DoRenderQuiltRegionViewOnDC(wxMemoryDC &dc, ViewPort &vp,
                                   OCPNRegion &chart_region);
  bool DoRenderQuiltRegionViewOnDCTextOnly(wxMemoryDC &dc, ViewPort &vp,
                                           OCPNRegion &chart_region);

  void EmptyCandidateArray(void);
  void SubstituteClearDC(wxMemoryDC &dc, ViewPort &vp);
  int GetNewRefChart(void);
  const LLRegion &GetTilesetRegion(int dbIndex);

  bool IsChartS57Overlay(int db_index);

  LLRegion m_covered_region;
  OCPNRegion m_rendered_region;  // used only in dc mode

  PatchList m_PatchList;
  wxBitmap *m_pBM;

  bool m_bcomposed;
  wxPatchListNode *cnode;
  bool m_bbusy;
  int m_quilt_proj;

  ArrayOfSortedQuiltCandidates *m_pcandidate_array;
  std::vector<int> m_last_index_array;
  std::vector<int> m_index_array;
  std::vector<int> m_extended_stack_array;
  std::vector<int> m_eclipsed_stack_array;
  std::vector<int> m_fullscreen_index_array;

  ViewPort m_vp_quilt;
  ViewPort m_vp_rendered;  // last VP rendered

  int m_nHiLiteIndex;
  std::vector<int> m_HiLiteIndexArray;
  int m_refchart_dbIndex;
  int m_reference_scale;
  int m_reference_type;
  int m_reference_family;
  bool m_bneed_clear;
  LLRegion m_back_region;
  wxString m_quilt_depth_unit;
  double m_max_error_factor;
  double m_canvas_scale_factor;
  int m_canvas_width;
  bool m_bquilt_has_overlays;
  unsigned long m_xa_hash;
  int m_zout_dbindex;
  int m_zout_family;
  int m_zout_type;

  int m_lost_refchart_dbIndex;
  bool m_b_hidef;

  bool m_bquiltskew;
  bool m_bquiltanyproj;
  ChartFamilyEnum m_preferred_family;
  ChartCanvas *m_parent;
};

#endif
