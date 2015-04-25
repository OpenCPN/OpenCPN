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

#include "chart1.h"
#include "OCPNRegion.h"

struct ChartTableEntry;

class QuiltPatch
{
public:
    QuiltPatch()
    {
        b_Valid = false;
        b_eclipsed = false;
        b_overlay = false;
    }
    int dbIndex;
    OCPNRegion ActiveRegion;
    int ProjType;
    bool b_Valid;
    bool b_eclipsed;
    bool b_overlay;
    OCPNRegion quilt_region;
};

class QuiltCandidate
{
public:
    QuiltCandidate()
    {
        b_include = false;
        b_eclipsed = false;
    }

    OCPNRegion &GetCandidateVPRegion( ViewPort &vp );
    
    int dbIndex;
    int ChartScale;
    bool b_include;
    bool b_eclipsed;
    
private:    
    OCPNRegion candidate_region;

};

WX_DECLARE_LIST( QuiltPatch, PatchList );
WX_DEFINE_SORTED_ARRAY( QuiltCandidate *, ArrayOfSortedQuiltCandidates );

class Quilt
{
public:

    Quilt();
    ~Quilt();

    void SetQuiltParameters( double CanvasScaleFactor, int CanvasWidth )
    {
        m_canvas_scale_factor = CanvasScaleFactor;
        m_canvas_width = CanvasWidth;
    }

    void EnableHighDefinitionZoom( bool value ) { m_b_hidef = value;}
    
    bool BuildExtendedChartStackAndCandidateArray(bool b_fullscreen, int ref_db_index, ViewPort &vp_in);
    bool Compose( const ViewPort &vp );
    bool IsComposed() {
        return m_bcomposed;
    }
    ChartBase *GetFirstChart();
    ChartBase *GetNextChart();
    ChartBase *GetLargestScaleChart();
    ArrayOfInts GetQuiltIndexArray( void );
    bool IsQuiltDelta( ViewPort &vp );
    bool IsChartQuiltableRef( int db_index );
    ViewPort &GetQuiltVP() {
        return m_vp_quilt;
    }
    wxString GetQuiltDepthUnit() {
        return m_quilt_depth_unit;
    }
    void SetRenderedVP( ViewPort &vp ) {
        m_vp_rendered = vp;
    }
    bool HasOverlays( void ) {
        return m_bquilt_has_overlays;
    }

    int GetExtendedStackCount(void) {
        return m_extended_stack_array.GetCount();
    }

    int GetnCharts() {
        return m_PatchList.GetCount();
    }
    double GetBestStartScale(int dbi_ref_hint, const ViewPort &vp_in);
    

    void ComputeRenderRegion( ViewPort &vp, OCPNRegion &chart_region );
    bool RenderQuiltRegionViewOnDC( wxMemoryDC &dc, ViewPort &vp, OCPNRegion &chart_region );
    bool IsVPBlittable( ViewPort &VPoint, int dx, int dy, bool b_allow_vector = false );
    ChartBase *GetChartAtPix( wxPoint p );
    ChartBase *GetOverlayChartAtPix( wxPoint p );
    int GetChartdbIndexAtPix( wxPoint p );
    void InvalidateAllQuiltPatchs( void );
    void Invalidate( void )
    {
        m_bcomposed = false;
        m_vp_quilt.Invalidate();
        m_zout_dbindex = -1;
    }
    void AdjustQuiltVP( ViewPort &vp_last, ViewPort &vp_proposed );

    OCPNRegion &GetFullQuiltRegion( void ) {
        return m_covered_region;
    }
    OCPNRegion &GetFullQuiltRenderedRegion( void ) {
        return m_rendered_region;
    }
    bool IsChartSmallestScale( int dbIndex );

    int AdjustRefOnZoomOut( double proposed_scale_onscreen );
    int AdjustRefOnZoomIn( double proposed_scale_onscreen );
    int AdjustRefOnZoom( bool b_zin, ChartFamilyEnum family, ChartTypeEnum type, double proposed_scale_onscreen );
    
    void SetHiliteIndex( int index ) {
        m_nHiLiteIndex = index;
    }
    void SetReferenceChart( int dbIndex ) {
        m_refchart_dbIndex = dbIndex;
    }
    int GetRefChartdbIndex( void ) {
        return m_refchart_dbIndex;
    }
    int GetQuiltProj( void )
    {
        return m_quilt_proj;
    }
    double GetMaxErrorFactor()
    {
        return m_max_error_factor;
    }
    double GetRefScale()
    {
        return m_reference_scale;
    }
    double GetRefNativeScale();

    ArrayOfInts GetCandidatedbIndexArray( bool from_ref_chart, bool exclude_user_hidden );
    ArrayOfInts GetExtendedStackIndexArray()
    {
        return m_extended_stack_array;
    }
    ArrayOfInts GetEclipsedStackIndexArray()
    {
        return m_eclipsed_stack_array;
    }

    unsigned long GetXStackHash() {
        return m_xa_hash;
    }

    bool IsBusy()
    {
        return m_bbusy;
    }
    QuiltPatch *GetCurrentPatch();
    bool IsChartInQuilt( ChartBase *pc );
    bool IsChartInQuilt( wxString &full_path);
    
    bool IsQuiltVector( void );
    OCPNRegion GetHiliteRegion( ViewPort &vp );
    static OCPNRegion GetChartQuiltRegion( const ChartTableEntry &cte, ViewPort &vp );
    
private:
    wxRect GetChartQuiltBoundingRect( const ChartTableEntry &cte, ViewPort &vp );
    
    void EmptyCandidateArray( void );
    void SubstituteClearDC( wxMemoryDC &dc, ViewPort &vp );
    int GetNewRefChart( void );

    int GetNomScaleMin(int scale, ChartTypeEnum type, ChartFamilyEnum family);
    int GetNomScaleMax(int scale, ChartTypeEnum type, ChartFamilyEnum family);
    
    bool IsChartS57Overlay( int db_index );
    
    OCPNRegion m_covered_region;
    OCPNRegion m_rendered_region;

    PatchList m_PatchList;
    wxBitmap *m_pBM;

    bool m_bcomposed;
    wxPatchListNode *cnode;
    bool m_bbusy;
    int m_quilt_proj;

    ArrayOfSortedQuiltCandidates *m_pcandidate_array;
    ArrayOfInts m_last_index_array;
    ArrayOfInts m_index_array;
    ArrayOfInts m_extended_stack_array;
    ArrayOfInts m_eclipsed_stack_array;

    ViewPort m_vp_quilt;
    ViewPort m_vp_rendered;          // last VP rendered

    int m_nHiLiteIndex;
    int m_refchart_dbIndex;
    int m_reference_scale;
    int m_reference_type;
    int m_reference_family;
    bool m_bneed_clear;
    OCPNRegion m_back_region;
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
};

#endif
