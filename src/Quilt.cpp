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

#include "wx/wxprec.h"

#include "Quilt.h"
#include "chartdb.h"
#include "s52plib.h"
#include "chcanv.h"
#include "ocpn_pixel.h"                         // for ocpnUSE_DIBSECTION

#ifdef USE_S57
#include "s57chart.h"
#endif

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( PatchList );

extern ChartDB *ChartData;
extern ArrayOfInts g_quilt_noshow_index_array;
extern s52plib *ps52plib;
extern ChartStack *pCurrentStack;
extern ChartCanvas *cc1;
extern int g_GroupIndex;
extern ColorScheme global_color_scheme;
extern int g_chart_zoom_modifier;
extern bool g_fog_overzoom;
extern double  g_overzoom_emphasis_base;
extern bool g_bopengl;

//      We define and use this one Macro in this module
//      Reason:  some compilers refuse to inline "GetChartTableEntry()"
//      and so this leads to a push/call sequence for this heavily utilized but short function
//      Note also that in the macor expansion there is no bounds checking on the parameter (i),
//      So it is probably better to confine the macro use to one module, and scrub carefully.
//      Anyway, makes a significant difference with Windows MSVC compiler builds.

#define GetChartTableEntry(i) GetChartTable()[i]

static int CompareScales( QuiltCandidate *qc1, QuiltCandidate *qc2 )
{
    if( !ChartData ) return 0;

    const ChartTableEntry &cte1 = ChartData->GetChartTableEntry( qc1->dbIndex );
    const ChartTableEntry &cte2 = ChartData->GetChartTableEntry( qc2->dbIndex );

    if( cte1.GetScale() == cte2.GetScale() )          // same scales, so sort on dbIndex
        return qc1->dbIndex - qc2->dbIndex;
    else
        return cte1.GetScale() - cte2.GetScale();
}

const LLRegion &QuiltCandidate::GetCandidateRegion()
{
    const ChartTableEntry &cte = ChartData->GetChartTableEntry( dbIndex );
    LLRegion &candidate_region = const_cast<LLRegion &>(cte.quilt_candidate_region);

    if( !candidate_region.Empty() )
        return candidate_region;

    LLRegion world_region(-90, -180, 90, 180);

    // for cm93 charts use their valid canvas region (should this apply to all vector charts?)
    if(ChartData->GetDBChartType( dbIndex ) == CHART_TYPE_CM93COMP) {
        double cm93_ll_bounds[8] = {-80, -180, -80, 180, 80, 180, 80, -180};
        candidate_region = LLRegion(4, cm93_ll_bounds);
        return candidate_region;
    }

    //    If the chart has an aux ply table, use it for finer region precision
    int nAuxPlyEntries = cte.GetnAuxPlyEntries();
    if( nAuxPlyEntries >= 1 ) {
        candidate_region.Clear();
        for( int ip = 0; ip < nAuxPlyEntries; ip++ ) {
            float *pfp = cte.GetpAuxPlyTableEntry( ip );
            int nAuxPly = cte.GetAuxCntTableEntry( ip );

            candidate_region.Union(LLRegion( nAuxPly, pfp ));

        }
    } else {
        int n_ply_entries = cte.GetnPlyEntries();
        float *pfp = cte.GetpPlyTable();

        if( n_ply_entries >= 3 ) // could happen with old database and some charts, e.g. SHOM 2381.kap
            candidate_region = LLRegion( n_ply_entries, pfp );
        else
            candidate_region = world_region;
    }

    //  Remove the NoCovr regions
    if( !candidate_region.Empty() ){              // don't bother if the region is already empty
        int nNoCovrPlyEntries = cte.GetnNoCovrPlyEntries();
        if( nNoCovrPlyEntries ) {
            for( int ip = 0; ip < nNoCovrPlyEntries; ip++ ) {
                float *pfp = cte.GetpNoCovrPlyTableEntry( ip );
                int nNoCovrPly = cte.GetNoCovrCntTableEntry( ip );

                LLRegion t_region = LLRegion( nNoCovrPly, pfp );

                //  We do a test removal of the NoCovr region.
                //  If the result iz empty, it must be that the NoCovr region is
                //  the full extent M_COVR(CATCOV=2) feature found in NOAA ENCs.
                //  We ignore it.

                if(!t_region.Empty()) {
                    LLRegion test_region = candidate_region;
                    test_region.Subtract( t_region );

                    if( !test_region.Empty())
                        candidate_region = test_region;
                }
            }
        }
    }

    //    Another superbad hack....
    //    Super small scale raster charts like bluemarble.kap usually cross the prime meridian
    //    and Plypoints georef is problematic......
    //    So, force full screen coverage in the quilt
    if( (cte.GetScale() > 90000000) && (cte.GetChartFamily() == CHART_FAMILY_RASTER) )
        candidate_region = world_region;

    return candidate_region;
}

LLRegion &QuiltCandidate::GetReducedCandidateRegion(double factor)
{
    if(factor != last_factor) {
        reduced_candidate_region = GetCandidateRegion();
        reduced_candidate_region.Reduce(factor);
        last_factor = factor;
    }

    return reduced_candidate_region;
}

Quilt::Quilt()
{
//      m_bEnableRaster = true;
//      m_bEnableVector = false;;
//      m_bEnableCM93 = false;

    m_reference_scale = 1;
    m_refchart_dbIndex = -1;
    m_reference_type = CHART_TYPE_UNKNOWN;
    m_reference_family = CHART_FAMILY_UNKNOWN;
    m_quilt_proj = PROJECTION_UNKNOWN;

    m_lost_refchart_dbIndex = -1;

    cnode = NULL;

    m_pBM = NULL;
    m_bcomposed = false;
    m_bbusy = false;
    m_b_hidef = false;

    m_pcandidate_array = new ArrayOfSortedQuiltCandidates( CompareScales );
    m_nHiLiteIndex = -1;

    m_zout_family = -1;
    m_zout_type = -1;

    //  Quilting of skewed raster charts is allowed for OpenGL only
    m_bquiltskew = g_bopengl;
    //  Quilting of different projections is allowed for OpenGL only
    m_bquiltanyproj = g_bopengl;
}

Quilt::~Quilt()
{
    m_PatchList.DeleteContents( true );
    m_PatchList.Clear();

    EmptyCandidateArray();
    delete m_pcandidate_array;

    m_extended_stack_array.Clear();

    delete m_pBM;
}

bool Quilt::IsVPBlittable( ViewPort &VPoint, int dx, int dy, bool b_allow_vector )
{
    if( !m_vp_rendered.IsValid() )
        return false;

    wxPoint2DDouble p1 = VPoint.GetDoublePixFromLL( m_vp_rendered.clat, m_vp_rendered.clon );
    wxPoint2DDouble p2 = VPoint.GetDoublePixFromLL( VPoint.clat, VPoint.clon );
    double deltax = p2.m_x - p1.m_x;
    double deltay = p2.m_y - p1.m_y;

    if( ( fabs( deltax - dx ) > 1e-2 ) || ( fabs( deltay - dy ) > 1e-2 ) )
        return false;

    return true;
}

bool Quilt::IsChartS57Overlay( int db_index )
{
#ifdef USE_S57

    if( db_index < 0 )
        return false;

    const ChartTableEntry &cte = ChartData->GetChartTableEntry( db_index );
    if( CHART_TYPE_S57 == cte.GetChartType() ){
        return  s57chart::IsCellOverlayType( cte.GetpFullPath() );
    }
    else
#endif
        return false;
}


bool Quilt::IsChartQuiltableRef( int db_index )
{
    if( db_index < 0 ) return false;

    //    Is the chart targeted by db_index useable as a quilt reference chart?
    const ChartTableEntry &ctei = ChartData->GetChartTableEntry( db_index );

    bool bproj_match = true;                  // Accept all projections

    double skew_norm = ctei.GetChartSkew();
    if( skew_norm > 180. ) skew_norm -= 360.;

    bool skew_match = fabs( skew_norm ) < 1.;  // Only un-skewed charts are acceptable for quilt
    if(m_bquiltskew)
        skew_match = true;

    //    In noshow array?
    bool b_noshow = false;
    for( unsigned int i = 0; i < g_quilt_noshow_index_array.GetCount(); i++ ) {
        if( g_quilt_noshow_index_array.Item( i ) == db_index )        // chart is in the noshow list
        {
            b_noshow = true;
            break;
        }
    }

    return ( bproj_match & skew_match & !b_noshow );
}

bool Quilt::IsChartInQuilt( ChartBase *pc )
{
    //    Iterate thru the quilt
    for( unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( ( pqc->b_include ) && ( !pqc->b_eclipsed ) ) {
            if( ChartData->OpenChartFromDB( pqc->dbIndex, FULL_INIT ) == pc ) return true;
        }
    }
    return false;
}

bool Quilt::IsChartInQuilt( wxString &full_path)
{
    //    Iterate thru the quilt
    for( unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( ( pqc->b_include ) && ( !pqc->b_eclipsed ) ) {
            ChartTableEntry *pcte = ChartData->GetpChartTableEntry(pqc->dbIndex);
            if(pcte->GetpsFullPath()->IsSameAs(full_path))
                return true;
        }
    }
    return false;
}


ArrayOfInts Quilt::GetCandidatedbIndexArray( bool from_ref_chart, bool exclude_user_hidden )
{
    ArrayOfInts ret;
    for( unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( from_ref_chart )                     // only add entries of smaller scale than ref scale
        {
            if( pqc->ChartScale >= m_reference_scale ) {
                // Search the no-show array
                if( exclude_user_hidden ) {
                    bool b_noshow = false;
                    for( unsigned int i = 0; i < g_quilt_noshow_index_array.GetCount(); i++ ) {
                        if( g_quilt_noshow_index_array.Item( i ) == pqc->dbIndex ) // chart is in the noshow list
                        {
                            b_noshow = true;
                            break;
                        }
                    }
                    if( !b_noshow ) ret.Add( pqc->dbIndex );
                } else {
                    ret.Add( pqc->dbIndex );
                }
            }
        } else
            ret.Add( pqc->dbIndex );

    }
    return ret;
}

QuiltPatch *Quilt::GetCurrentPatch()
{
    if( cnode ) return ( cnode->GetData() );
    else
        return NULL;
}

void Quilt::EmptyCandidateArray( void )
{
    for( unsigned int i = 0; i < m_pcandidate_array->GetCount(); i++ ) {
        delete m_pcandidate_array->Item( i );
    }

    m_pcandidate_array->Clear();

}

ChartBase *Quilt::GetFirstChart()
{
    if( !ChartData ) return NULL;

    if( !ChartData->IsValid() ) // This could happen during yield recursion from progress dialog during databse update
        return NULL;

    if( !m_bcomposed ) return NULL;

    if( m_bbusy )
        return NULL;

    m_bbusy = true;
    ChartBase *pret = NULL;
    cnode = m_PatchList.GetFirst();
    while( cnode && !cnode->GetData()->b_Valid )
        cnode = cnode->GetNext();
    if( cnode && cnode->GetData()->b_Valid ) pret = ChartData->OpenChartFromDB(
                    cnode->GetData()->dbIndex, FULL_INIT );

    m_bbusy = false;
    return pret;
}

ChartBase *Quilt::GetNextChart()
{
    if( !ChartData ) return NULL;

    if( !ChartData->IsValid() ) return NULL;

    if( m_bbusy )
        return NULL;

    m_bbusy = true;
    ChartBase *pret = NULL;
    if( cnode ) {
        cnode = cnode->GetNext();
        while( cnode && !cnode->GetData()->b_Valid )
            cnode = cnode->GetNext();
        if( cnode && cnode->GetData()->b_Valid ) pret = ChartData->OpenChartFromDB(
                        cnode->GetData()->dbIndex, FULL_INIT );
    }

    m_bbusy = false;
    return pret;
}

ChartBase *Quilt::GetLargestScaleChart()
{
    if( !ChartData ) return NULL;

    if( m_bbusy )
        return NULL;

    m_bbusy = true;
    ChartBase *pret = NULL;
    cnode = m_PatchList.GetLast();
    if( cnode ) pret = ChartData->OpenChartFromDB( cnode->GetData()->dbIndex, FULL_INIT );

    m_bbusy = false;
    return pret;
}

LLRegion Quilt::GetChartQuiltRegion( const ChartTableEntry &cte, ViewPort &vp )
{
    LLRegion chart_region;
    LLRegion screen_region( vp.GetBBox() );

    // Special case for charts which extend around the world, or near to it
    //  Mostly this means cm93....
    //  Take the whole screen, clipped at +/- 80 degrees lat
    if(fabs(cte.GetLonMax() - cte.GetLonMin()) > 180.) {
/*
        int n_ply_entries = 4;
        float ply[8];
        ply[0] = 80.;
        ply[1] = vp.GetBBox().GetMinX();
        ply[2] = 80.;
        ply[3] = vp.GetBBox().GetMaxX();
        ply[4] = -80.;
        ply[5] = vp.GetBBox().GetMaxX();
        ply[6] = -80.;
        ply[7] = vp.GetBBox().GetMinX();


        OCPNRegion t_region = vp.GetVPRegionIntersect( screen_region, 4, &ply[0],
                                                     cte.GetScale() );
        return t_region;
*/
        return LLRegion(-80, vp.GetBBox().GetMinLon(), 80, vp.GetBBox().GetMaxLon());
    }

    //    If the chart has an aux ply table, use it for finer region precision
    int nAuxPlyEntries = cte.GetnAuxPlyEntries();
    if( nAuxPlyEntries >= 1 ) {
        for( int ip = 0; ip < nAuxPlyEntries; ip++ ) {
            float *pfp = cte.GetpAuxPlyTableEntry( ip );
            int nAuxPly = cte.GetAuxCntTableEntry( ip );

            LLRegion t_region(nAuxPly, pfp);
            t_region.Intersect(screen_region);
//            OCPNRegion t_region = vp.GetVPRegionIntersect( screen_region, nAuxPly, pfp,
//                                cte.GetScale() );
            if( !t_region.Empty() )
                chart_region.Union( t_region );
        }
    }

    else {
        int n_ply_entries = cte.GetnPlyEntries();
        float *pfp = cte.GetpPlyTable();

        if( n_ply_entries >= 3 ) // could happen with old database and some charts, e.g. SHOM 2381.kap
        {
            LLRegion t_region(n_ply_entries, pfp);
            t_region.Intersect(screen_region);
//            const OCPNRegion t_region = vp.GetVPRegionIntersect( screen_region, n_ply_entries, pfp,
//                                cte.GetScale() );
            if( !t_region.Empty() )
                chart_region.Union( t_region );

        } else
            chart_region = screen_region;
    }

    //  Remove the NoCovr regions
    int nNoCovrPlyEntries = cte.GetnNoCovrPlyEntries();
    if( nNoCovrPlyEntries ) {
        for( int ip = 0; ip < nNoCovrPlyEntries; ip++ ) {
            float *pfp = cte.GetpNoCovrPlyTableEntry( ip );
            int nNoCovrPly = cte.GetNoCovrCntTableEntry( ip );

            LLRegion t_region(nNoCovrPly, pfp);
            t_region.Intersect(screen_region);
//            OCPNRegion t_region = vp.GetVPRegionIntersect( screen_region, nNoCovrPly, pfp,
//                                                         cte.GetScale() );

            //  We do a test removal of the NoCovr region.
            //  If the result iz empty, it must be that the NoCovr region is
            //  the full extent M_COVR(CATCOV=2) feature found in NOAA ENCs.
            //  We ignore it.

            if(!t_region.Empty()) {
                LLRegion test_region = chart_region;
                test_region.Subtract( t_region );

                if( !test_region.Empty())
                    chart_region = test_region;
            }
        }
    }


    //    Another superbad hack....
    //    Super small scale raster charts like bluemarble.kap usually cross the prime meridian
    //    and Plypoints georef is problematic......
    //    So, force full screen coverage in the quilt
    if( (cte.GetScale() > 90000000) && (cte.GetChartFamily() == CHART_FAMILY_RASTER) )
        chart_region = screen_region;

    //    Clip the region to the current viewport
    //chart_region.Intersect( vp.rv_rect );  already done

    return chart_region;
}






bool Quilt::IsQuiltVector( void )
{
    if( m_bbusy )
        return false;

    m_bbusy = true;

    bool ret = false;

    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        if( cnode->GetData() ) {
            QuiltPatch *pqp = cnode->GetData();

            if( ( pqp->b_Valid ) && ( !pqp->b_eclipsed ) ) {
                const ChartTableEntry &ctei = ChartData->GetChartTableEntry( pqp->dbIndex );

                if( ctei.GetChartFamily() == CHART_FAMILY_VECTOR ) {
                    ret = true;
                    break;
                }

            }
        }
        cnode = cnode->GetNext();
    }

    m_bbusy = false;
    return ret;
}

int Quilt::GetChartdbIndexAtPix( ViewPort &VPoint, wxPoint p )
{
    if( m_bbusy )
        return -1;

    m_bbusy = true;

    double lat, lon;
    VPoint.GetLLFromPix( p, &lat, &lon );

    int ret = -1;

    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        if( cnode->GetData()->ActiveRegion.Contains( lat, lon ) ) {
            ret = cnode->GetData()->dbIndex;
            break;
        } else
            cnode = cnode->GetNext();
    }

    m_bbusy = false;
    return ret;
}

ChartBase *Quilt::GetChartAtPix( ViewPort &VPoint, wxPoint p )
{
    if( m_bbusy )
        return NULL;

    m_bbusy = true;

    double lat, lon;
    VPoint.GetLLFromPix( p, &lat, &lon );

    //    The patchlist is organized from small to large scale.
    //    We generally will want the largest scale chart at this point, so
    //    walk the whole list.  The result will be the last one found, i.e. the largest scale chart.
    ChartBase *pret = NULL;
    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        QuiltPatch *pqp = cnode->GetData();
        if( !pqp->b_overlay && (pqp->ActiveRegion.Contains( lat, lon ) ) )
            if( ChartData->IsChartInCache( pqp->dbIndex ) ){
                pret = ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
            }
        cnode = cnode->GetNext();
    }

    m_bbusy = false;
    return pret;
}

ChartBase *Quilt::GetOverlayChartAtPix( ViewPort &VPoint, wxPoint p )
{
    if( m_bbusy )
        return NULL;

    m_bbusy = true;

    double lat, lon;
    VPoint.GetLLFromPix( p, &lat, &lon );

    //    The patchlist is organized from small to large scale.
    //    We generally will want the largest scale chart at this point, so
    //    walk the whole list.  The result will be the last one found, i.e. the largest scale chart.
    ChartBase *pret = NULL;
    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        QuiltPatch *pqp = cnode->GetData();
        if( pqp->b_overlay && ( pqp->ActiveRegion.Contains( lat, lon ) ) )
                pret = ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
        cnode = cnode->GetNext();
    }

    m_bbusy = false;
    return pret;
}


void Quilt::InvalidateAllQuiltPatchs( void )
{
/*
    if( m_bbusy )
        return;

    m_bbusy = true;
    m_bbusy = false;
*/
    return;
}

ArrayOfInts Quilt::GetQuiltIndexArray( void )
{
    return m_index_array;

    ArrayOfInts ret;

    if( m_bbusy )
        return ret;

    m_bbusy = true;

    wxPatchListNode *cnode = m_PatchList.GetFirst();
    while( cnode ) {
        ret.Add( cnode->GetData()->dbIndex );
        cnode = cnode->GetNext();
    }

    m_bbusy = false;

    return ret;
}

bool Quilt::IsQuiltDelta( ViewPort &vp )
{
    if( !m_vp_quilt.IsValid() || !m_bcomposed ) return true;

    if( m_vp_quilt.view_scale_ppm != vp.view_scale_ppm ) return true;

    if( m_vp_quilt.m_projection_type != vp.m_projection_type ) return true;

    //    Has the quilt shifted by more than one pixel in any direction?
    wxPoint cp_last, cp_this;

    cp_last = m_vp_quilt.GetPixFromLL( vp.clat, vp.clon );
    cp_this = vp.GetPixFromLL( vp.clat, vp.clon );

    return ( cp_last != cp_this );
}

void Quilt::AdjustQuiltVP( ViewPort &vp_last, ViewPort &vp_proposed )
{
    if( m_bbusy )
        return;

//      ChartBase *pRefChart = GetLargestScaleChart();
    ChartBase *pRefChart = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );

    if( pRefChart ) pRefChart->AdjustVP( vp_last, vp_proposed );
}

double Quilt::GetRefNativeScale()
{
    double ret_val = 1.0;
    if( ChartData ) {
        ChartBase *pc = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );
        if( pc ) ret_val = pc->GetNativeScale();
    }

    return ret_val;
}

int Quilt::GetNewRefChart( void )
{
    //    Using the current quilt, select a useable reference chart
    //    Said chart will be in the extended (possibly full-screen) stack,
    //    And will have a scale equal to or just greater than the current quilt reference scale,
    //    And will match current quilt projection type, and
    //    will have Skew=0, so as to be fully quiltable
    int new_ref_dbIndex = m_refchart_dbIndex;
    unsigned int im = m_extended_stack_array.GetCount();
    if( im > 0 ) {
        for( unsigned int is = 0; is < im; is++ ) {
            const ChartTableEntry &m = ChartData->GetChartTableEntry( m_extended_stack_array.Item( is ) );

            double skew_norm = m.GetChartSkew();
            if( skew_norm > 180. ) skew_norm -= 360.;

            if( ( m.GetScale() >= m_reference_scale )
                    && ( m_reference_family == m.GetChartFamily() )
                    && ( m_bquiltanyproj || m_quilt_proj == m.GetChartProjectionType() )
                    && ( m_bquiltskew || (fabs(skew_norm) < 1.0) ) ){
                new_ref_dbIndex = m_extended_stack_array.Item( is );
                break;
            }
        }
    }
    return new_ref_dbIndex;
}


int Quilt::GetNomScaleMax(int scale, ChartTypeEnum type, ChartFamilyEnum family)
{
    switch(family){
        case CHART_FAMILY_RASTER:{
            return scale / 4;
        }

        case CHART_FAMILY_VECTOR:{
            return scale / 4;
        }

        default:{
            return scale / 2;
        }
    }
}

int Quilt::GetNomScaleMin(int scale, ChartTypeEnum type, ChartFamilyEnum family)
{
    double mod = ((double)g_chart_zoom_modifier + 5.)/5.;  // 0->2
    mod = wxMax(mod, .2);
    mod = wxMin(mod, 2.0);

    switch(family){
        case CHART_FAMILY_RASTER:{
            return scale * 3 * mod;
        }

        case CHART_FAMILY_VECTOR:{
            return scale * 4 * mod;
        }

        default:{
            return scale * 2 * mod;
        }
    }
}

int Quilt::AdjustRefOnZoom( bool b_zin, ChartFamilyEnum family,  ChartTypeEnum type, double proposed_scale_onscreen )
{
    //  Make 3 lists
    wxArrayInt nom_scale;
    wxArrayInt max_scale;
    wxArrayInt min_scale;
    wxArrayInt index_array;

    //  For Vector charts, we can switch to any chart that is on screen.
    //  Otherwise, we can only switch to charts contining the VP center point
    bool b_allow_fullscreen_ref = (family == CHART_FAMILY_VECTOR);

    //  Walk the extended chart array, capturing data
    int i_first = 0;
    for(size_t i=0 ; i < m_extended_stack_array.GetCount() ; i++){
        int test_db_index = m_extended_stack_array.Item( i );

        if( b_allow_fullscreen_ref || pCurrentStack->DoesStackContaindbIndex( test_db_index ) ) {
            if( ( family == ChartData->GetDBChartFamily( test_db_index ) )
                && IsChartQuiltableRef( test_db_index )
                && !IsChartS57Overlay( test_db_index ) ){

                index_array.Add(test_db_index);
                int nscale = ChartData->GetDBChartScale(test_db_index);
                nom_scale.Add(nscale);

                int nmax_scale = GetNomScaleMax(nscale, type, family);

                //  For the largest scale chart, allow essentially infinite overzoom.
                //  The range will be clipped later
                if(0 == i_first)
                    nmax_scale = 1;
                max_scale.Add(nmax_scale);

                int nmin_scale = GetNomScaleMin(nscale, type, family);
                min_scale.Add(nmin_scale);

                i_first ++;
            }
        }
    }

    //  If showing Vector charts,
    //  Find the smallest scale chart of the target type (i.e. skipping cm93)
    //  and make sure that its min scale is at least
    //  small enough to allow reasonable zoomout.
    if(CHART_FAMILY_VECTOR == family){
        for(size_t i = index_array.GetCount() ; i ; i--){
            int test_db_index = index_array.Item( i-1 );
            if( type == ChartData->GetDBChartType( test_db_index ) ){
                int smallest_min_scale = min_scale.Item(i-1);
                min_scale.Item(i-1) = smallest_min_scale * 80; //wxMax(smallest_min_scale, 200000);
                break;
            }
        }
    }


    // Traverse the list, making sure that the allowable scale ranges overlap so as to make no "holes"
    // in the coverage.
    // We do this by extending upward the range of larger scale charts, so that they overlap
    // the next smaller scale chart.  Makes a nicer image...
    // However, we don't want excessive underzoom, for performance reasons.
    // So make sure any adjusted min_scale is not more than twice the already established value
    if(index_array.GetCount() > 1){
        for(size_t i=0 ; i < index_array.GetCount()-1 ; i++){
            int min_scale_test = wxMax(min_scale.Item(i), max_scale.Item(i+1) + 1);
            min_scale_test = wxMin(min_scale_test, min_scale.Item(i) * 2 );
            min_scale.Item(i) = min_scale_test;
//              min_scale.Item(i) = wxMax(min_scale.Item(i), max_scale.Item(i+1) + 1);
        }
    }

    // There may still be holes...
    // Traverse the list again, from smaller to larger scale, filling in any holes by
    // increasing the max_scale of smaller scale charts.
    // Skip cm93 if present
    if(index_array.GetCount() > 2){
        for(size_t i=index_array.GetCount()-2 ; i >= 1 ; i--){
              max_scale.Item(i) = wxMin(max_scale.Item(i), min_scale.Item(i-1) - 1);
        }
    }




    int new_ref_dbIndex = -1;

    // Search for the largest scale chart whose scale limits contain the requested scale.
    for(size_t i=0 ; i < index_array.GetCount() ; i++){
        int a = min_scale.Item(i);
        int b = max_scale.Item(i);

        if( ( proposed_scale_onscreen < min_scale.Item(i) * 1.05) &&   // 5 percent leeway to allow for roundoff errors
            (proposed_scale_onscreen > max_scale.Item(i)) ) {
            new_ref_dbIndex = index_array.Item(i);
            break;
        }
    }


    return new_ref_dbIndex;
}

int Quilt::AdjustRefOnZoomOut( double proposed_scale_onscreen )
{
    //  Reset "lost" chart logic
    m_lost_refchart_dbIndex = -1;

    int current_db_index = m_refchart_dbIndex;
    int current_family =   m_reference_family;
    ChartTypeEnum current_type = (ChartTypeEnum)m_reference_type;

    if(m_refchart_dbIndex >= 0){
        const ChartTableEntry &cte = ChartData->GetChartTableEntry( m_refchart_dbIndex );
        current_family =  cte.GetChartFamily();
        current_type = (ChartTypeEnum) cte.GetChartType();
    }

    if( current_type == CHART_TYPE_CM93COMP )
            return current_db_index;

    int proposed_ref_index = AdjustRefOnZoom( false, (ChartFamilyEnum)current_family, current_type, proposed_scale_onscreen );

    m_zout_family = -1;
    if(proposed_ref_index < 0){
        m_zout_family = current_family;      // save it
        m_zout_type = current_type;
        m_zout_dbindex = current_db_index;
    }


    SetReferenceChart( proposed_ref_index );

    return proposed_ref_index;

}

int Quilt::AdjustRefOnZoomIn( double proposed_scale_onscreen )
{
        //  Reset "lost" chart logic
    m_lost_refchart_dbIndex = -1;

    int current_db_index = m_refchart_dbIndex;
    int current_family = m_reference_family;
    ChartTypeEnum current_type = (ChartTypeEnum) m_reference_type;

    if(m_zout_family >= 0){
        current_type = (ChartTypeEnum)m_zout_type;
        current_family = m_zout_family;
    }

        //  If the current reference chart is cm93, and it became so due to a zout from another family,
        //  detect this case and allow switch to save chart index family
        if( current_type == CHART_TYPE_CM93COMP ){
            if( m_zout_family >= 0) {
                current_family = ChartData->GetDBChartFamily( m_zout_dbindex );
            }
            else                            // cm93 (selected) does not shift charts
                return current_db_index;
        }

    if(( -1 == m_refchart_dbIndex) && (m_zout_dbindex >= 0))
        BuildExtendedChartStackAndCandidateArray(true, m_zout_dbindex, m_vp_quilt);


    int proposed_ref_index = AdjustRefOnZoom( true, (ChartFamilyEnum)current_family, current_type, proposed_scale_onscreen );

    SetReferenceChart( proposed_ref_index );

    return proposed_ref_index;
}

bool Quilt::IsChartSmallestScale( int dbIndex )
{
    if(!ChartData)
        return false;

    // find the smallest scale chart of the specified type on the extended stack array
    int specified_type = ChartData->GetDBChartType( dbIndex );
    int target_dbindex = -1;

    unsigned int target_stack_index = 0;
    if( m_extended_stack_array.GetCount() ) {
        while( ( target_stack_index <= ( m_extended_stack_array.GetCount() - 1 ) ) ) {
            int test_db_index = m_extended_stack_array.Item( target_stack_index );

            if( specified_type == ChartData->GetDBChartType( test_db_index ) ) target_dbindex =
                    test_db_index;

            target_stack_index++;
        }
    }
    return ( dbIndex == target_dbindex );
}

LLRegion Quilt::GetHiliteRegion()
{
    LLRegion r;
    if( m_nHiLiteIndex >= 0 ) {
        // Walk the PatchList, looking for the target hilite index
        for( unsigned int i = 0; i < m_PatchList.GetCount(); i++ ) {
            wxPatchListNode *pcinode = m_PatchList.Item( i );
            QuiltPatch *piqp = pcinode->GetData();
            if( ( m_nHiLiteIndex == piqp->dbIndex ) && ( piqp->b_Valid ) ) // found it
            {
                r = piqp->ActiveRegion;
                break;
            }
        }

        // If not in the patchlist, look in the full chartbar
        if( r.Empty() ) {
            for( unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
                QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
                if( m_nHiLiteIndex == pqc->dbIndex ) {
                    LLRegion chart_region = pqc->GetCandidateRegion();
                    if( !chart_region.Empty() ) {
                        // Do not highlite fully eclipsed charts
                        bool b_eclipsed = false;
                        for( unsigned int ir = 0; ir < m_eclipsed_stack_array.GetCount(); ir++ ) {
                            if( m_nHiLiteIndex == m_eclipsed_stack_array.Item( ir ) ) {
                                b_eclipsed = true;
                                break;
                            }
                        }

                        if( !b_eclipsed )
                            r = chart_region;
                        break;
                    }
                }
            }
        }
    }
    return r;
}

bool Quilt::BuildExtendedChartStackAndCandidateArray(bool b_fullscreen, int ref_db_index, ViewPort &vp_in)
{
    EmptyCandidateArray();
    m_extended_stack_array.Clear();

    int reference_scale = 1.;
    int reference_type = -1;
    int reference_family;
    int quilt_proj = m_bquiltanyproj ? vp_in.m_projection_type : PROJECTION_UNKNOWN;

    if( ref_db_index >= 0 ) {
        const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry( ref_db_index );
        reference_scale = cte_ref.GetScale();
        reference_type = cte_ref.GetChartType();
        if(!m_bquiltanyproj)
            quilt_proj = ChartData->GetDBChartProj( ref_db_index );
        reference_family = cte_ref.GetChartFamily();
    }

    bool b_need_resort = false;

    ViewPort vp_local = vp_in;          // non-const copy

    if( !pCurrentStack ) {
        pCurrentStack = new ChartStack;
        ChartData->BuildChartStack( pCurrentStack, vp_local.clat, vp_local.clon );
    }

    int n_charts = 0;
    if( pCurrentStack ) {
        n_charts = pCurrentStack->nEntry;

        //    Walk the current ChartStack...
        //    Building the quilt candidate array
        for( int ics = 0; ics < n_charts; ics++ ) {
            int i = pCurrentStack->GetDBIndex( ics );
            m_extended_stack_array.Add( i );

            const ChartTableEntry &cte = ChartData->GetChartTableEntry( i );

            double skew_norm = cte.GetChartSkew();
            if( skew_norm > 180. ) skew_norm -= 360.;

            // only charts of the proper projection and type may be quilted....
            // Also, only unskewed charts if so directed
            // and we avoid adding CM93 Composite until later
            if( ( reference_type == cte.GetChartType() )
            && ( m_bquiltskew ? 1: fabs( skew_norm ) < 1.0 )
            && ( m_bquiltanyproj || cte.GetChartProjectionType() == quilt_proj )
            && ( cte.GetChartType() != CHART_TYPE_CM93COMP ) ) {
                QuiltCandidate *qcnew = new QuiltCandidate;
                qcnew->dbIndex = i;
                qcnew->ChartScale = cte.GetScale();

                m_pcandidate_array->Add( qcnew );               // auto-sorted on scale

            }
        }
    }

    if( b_fullscreen ) {
        //    Search the entire database, potentially adding all charts
        //    which intersect the ViewPort in any way
        //    .AND. other requirements.
        //    Again, skipping cm93 for now
        int n_all_charts = ChartData->GetChartTableEntries();

        LLBBox viewbox = vp_local.GetBBox();
        int sure_index = -1;
        int sure_index_scale = 0;

        for( int i = 0; i < n_all_charts; i++ ) {
            //    We can eliminate some charts immediately
            //    Try to make these tests in some sensible order....

            const ChartTableEntry &cte = ChartData->GetChartTableEntry( i );

            int type = cte.GetChartType();

            if( reference_type != type ) continue;

            if( type == CHART_TYPE_CM93COMP ) continue;

            const LLBBox &chart_box = cte.GetBBox();
            if( ( viewbox.IntersectOut( chart_box ) ) ) continue;

            if( ( g_GroupIndex > 0 ) && ( !ChartData->IsChartInGroup( i, g_GroupIndex ) ) ) continue;

            if( !m_bquiltanyproj && quilt_proj != cte.GetChartProjectionType() ) continue;

            double skew_norm = cte.GetChartSkew();
            if( skew_norm > 180. ) skew_norm -= 360.;

             if( !m_bquiltskew && fabs( skew_norm ) > 1.0 )
                continue;

            //    Calculate zoom factor for this chart
            double candidate_chart_scale = cte.GetScale();
            double chart_native_ppm = m_canvas_scale_factor / candidate_chart_scale;
            double zoom_factor = vp_in.view_scale_ppm / chart_native_ppm;

            //  Try to guarantee that there is one chart added with scale larger than reference scale
            //    Take note here, and keep track of the smallest scale chart that is larger scale than reference....
            if( candidate_chart_scale < reference_scale ) {
                if( candidate_chart_scale > sure_index_scale ) {
                    sure_index = i;
                    sure_index_scale = candidate_chart_scale;
                }
            }

            //    At this point, the candidate is the right type, skew, and projection, and is on-screen somewhere....
            //    Now  add the candidate if its scale is smaller than the reference scale, or is not excessively underzoomed.

            if( ( candidate_chart_scale >= reference_scale ) || ( zoom_factor > .2 ) ) {
                bool b_add = true;

                //    Special case for S57 ENC
                //    Add the chart only if the chart's fractional area exceeds n%
                if( CHART_TYPE_S57 == reference_type ) {
                    //Get the fractional area of this chart
                    //                    double chart_fractional_area = 0.;
//                    double quilt_area = vp_local.pix_width * vp_local.pix_height;

                    //                  LLRegion cell_region = GetChartQuiltRegion( cte, vp_local );

//                    if( !cell_region.Empty() ) {
//                        chart_fractional_area = ( cell_rect.GetWidth() * cell_rect.GetHeight() )
                        //                                                / quilt_area;
//                    } else
//                        b_add = false;  // this chart has no actual overlap on screen
                    // probably because it has a concave outline
                    // or lots of NoCovr regions.  US3EC04.000 is a good example
                    // i.e the full bboxes overlap, but the actual vp intersect is null.

//                    if( chart_fractional_area < .05 ) {
 //                       b_add = false;
 //                   }

                    //  Allow S57 charts that are near normal zoom, no matter what their fractional area coverage
                    if(( zoom_factor > 0.1)/* && ( chart_fractional_area > .001 )*/ )
                        b_add = true;
                }

                if( ref_db_index == i)
                    b_add = true;

                if( b_add ) {
                    // Check to see if this chart is already in the stack array
                    // by virtue of being under the Viewport center point....
                    bool b_exists = false;
                    for( unsigned int ir = 0; ir < m_extended_stack_array.GetCount(); ir++ ) {
                        if( i == m_extended_stack_array.Item( ir ) ) {
                            b_exists = true;
                            break;
                        }
                    }

                    if( !b_exists ) {
                        //      Check to be sure that this chart has not already been added
                        //    i.e. charts that have exactly the same file name and nearly the same mod time
                        //    These charts can be in the database due to having the exact same chart in different directories,
                        //    as may be desired for some grouping schemes
                        bool b_noadd = false;
                        ChartTableEntry *pn = ChartData->GetpChartTableEntry( i );
                        for( unsigned int id = 0; id < m_extended_stack_array.GetCount() ; id++ ) {
                            if( m_extended_stack_array.Item( id ) != -1 ) {
                                ChartTableEntry *pm = ChartData->GetpChartTableEntry( m_extended_stack_array.Item( id ) );
                                if( pm->GetFileTime() && pn->GetFileTime()) {
                                    if( labs(pm->GetFileTime() - pn->GetFileTime()) < 60 ) {           // simple test
                                        if( pn->GetpFileName()->IsSameAs( *( pm->GetpFileName() ) ) )
                                            b_noadd = true;
                                    }
                                }
                            }
                        }

                        if(!b_noadd) {
                            m_extended_stack_array.Add( i );

                            QuiltCandidate *qcnew = new QuiltCandidate;
                            qcnew->dbIndex = i;
                            qcnew->ChartScale = candidate_chart_scale; //ChartData->GetDBChartScale( i );

                            m_pcandidate_array->Add( qcnew );               // auto-sorted on scale

                            b_need_resort = true;
                        }
                    }
                }
            }
        }               // for all charts

        //    Check to be sure that at least one chart was added that is larger scale than reference scale
        if( -1 != sure_index ) {
            // check to see if it is already in
            bool sure_exists = false;
            for( unsigned int ir = 0; ir < m_extended_stack_array.GetCount(); ir++ ) {
                if( sure_index == m_extended_stack_array.Item(ir) ) {
                    sure_exists = true;
                    break;
                }
            }

            //    If not already added, do so now
            if( !sure_exists ) {
                m_extended_stack_array.Add( sure_index );

                QuiltCandidate *qcnew = new QuiltCandidate;
                qcnew->dbIndex = sure_index;
                qcnew->ChartScale = ChartData->GetDBChartScale( sure_index );
                m_pcandidate_array->Add( qcnew );               // auto-sorted on scale

                b_need_resort = true;
            }
        }
    }   // fullscreen

    // Re sort the extended stack array on scale
    if( b_need_resort && m_extended_stack_array.GetCount() > 1 ) {
        int swap = 1;
        int ti;
        while( swap == 1 ) {
            swap = 0;
            for( unsigned int is = 0; is < m_extended_stack_array.GetCount() - 1; is++ ) {
                const ChartTableEntry &m = ChartData->GetChartTableEntry(
                                               m_extended_stack_array.Item( is ) );
                const ChartTableEntry &n = ChartData->GetChartTableEntry(
                                               m_extended_stack_array.Item( is + 1 ) );

                if( n.GetScale() < m.GetScale() ) {
                    ti = m_extended_stack_array.Item( is );
                    m_extended_stack_array.RemoveAt( is );
                    m_extended_stack_array.Insert( ti, is + 1 );
                    swap = 1;
                }
            }
        }
    }
    return true;
}

double Quilt::GetBestStartScale(int dbi_ref_hint, const ViewPort &vp_in)
{
    if( !ChartData )
        return false;

    if(ChartData->IsBusy())             // This prevent recursion on chart loads that Yeild()
        return false;

    ViewPort vp_local = vp_in;                   // need a non-const copy

    //    Validate Reference Chart hint
    int tentative_ref_index = dbi_ref_hint;
    if( dbi_ref_hint < 0 ) {
        //arbitrarily select reference chart as largest scale on current stack
        if( !pCurrentStack ) {
            pCurrentStack = new ChartStack;
            ChartData->BuildChartStack( pCurrentStack, vp_local.clat, vp_local.clon );
        }
        tentative_ref_index = pCurrentStack->GetDBIndex(0);
    }

    //    As ChartdB data is always in rectilinear space, region calculations need to be done with no VP rotation
    double saved_vp_rotation = vp_local.rotation;                      // save a copy
    vp_local.SetRotationAngle( 0. );

    bool bfull = vp_in.b_FullScreenQuilt;
    BuildExtendedChartStackAndCandidateArray(bfull, tentative_ref_index, vp_local);

    //  tentative choice might not be in the extended stack....
    bool bf = false;
    for( unsigned int i = 0; i < m_pcandidate_array->GetCount(); i++ ) {
        QuiltCandidate *qc = m_pcandidate_array->Item( i );
        if( qc->dbIndex == tentative_ref_index ) {
            bf = true;
            break;
        }
    }

    if( !bf && m_pcandidate_array->GetCount() ) {
        tentative_ref_index = GetNewRefChart();
        BuildExtendedChartStackAndCandidateArray(bfull, tentative_ref_index, vp_local);
    }

    double proposed_scale_onscreen = vp_in.chart_scale;

    if(m_pcandidate_array->GetCount()){
        m_refchart_dbIndex = tentative_ref_index;
    }
    else{
        //    Need to choose some chart, find a quiltable candidate
        bool bfq = false;
        for( unsigned int i = 0; i < m_pcandidate_array->GetCount(); i++ ) {
            QuiltCandidate *qc = m_pcandidate_array->Item( i );
            if( IsChartQuiltableRef(qc->dbIndex) ){
                m_refchart_dbIndex = qc->dbIndex;
                bfq = true;
                break;
            }
        }

        if(!bfq)        // fallback to first chart in stack
            m_refchart_dbIndex = pCurrentStack->GetDBIndex(0);
    }

    if(m_refchart_dbIndex >= 0) {
        // Suggest a scale so that the largest scale candidate is "nominally" scaled,
        // meaning not overzoomed, and not underzoomed
        ChartBase *pc = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );
        if( pc ) {
            double min_ref_scale = pc->GetNormalScaleMin( cc1->GetCanvasScaleFactor(), false );
            double max_ref_scale = pc->GetNormalScaleMax( cc1->GetCanvasScaleFactor(), m_canvas_width );

            proposed_scale_onscreen = wxMin(proposed_scale_onscreen, max_ref_scale);
            proposed_scale_onscreen = wxMax(proposed_scale_onscreen, min_ref_scale);
        }
    }
    return cc1->GetCanvasScaleFactor() / proposed_scale_onscreen;
}

void Quilt::UnlockQuilt()
{
    wxASSERT(m_bbusy == false);
    ChartData->UnLockCache();
    // unlocked only charts owned by the Quilt
    for(unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if (pqc->b_locked == true) {
            ChartData->UnLockCacheChart(pqc->dbIndex);
            pqc->b_locked = false;
        }
    }
}

bool Quilt::Compose( const ViewPort &vp_in )
{
    if( !ChartData )
        return false;

    if(ChartData->IsBusy())             // This prevent recursion on chart loads that Yeild()
        return false;

    if( m_bbusy )
        return false;

    // XXX call before setting m_bbusy for wxASSERT in UnlockQuilt
    UnlockQuilt();
    m_bbusy = true;

    ViewPort vp_local = vp_in;                   // need a non-const copy

    //    Get Reference Chart parameters
    if( m_refchart_dbIndex >= 0 ) {
        const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry( m_refchart_dbIndex );
        m_reference_scale = cte_ref.GetScale();
        m_reference_type = cte_ref.GetChartType();
        if(!m_bquiltanyproj)
            m_quilt_proj = ChartData->GetDBChartProj( m_refchart_dbIndex );
        m_reference_family = cte_ref.GetChartFamily();
    }

    //    Set up the viewport projection type
    if(!m_bquiltanyproj)
        vp_local.SetProjectionType( m_quilt_proj );

    //    As ChartdB data is always in rectilinear space, region calculations need to be done with no VP rotation
//    double saved_vp_rotation = vp_local.rotation;                      // save a copy
//    vp_local.SetRotationAngle( 0. );

    bool bfull = vp_in.b_FullScreenQuilt;
    BuildExtendedChartStackAndCandidateArray(bfull, m_refchart_dbIndex, vp_local);

    //    It is possible that the reference chart is not really part of the visible quilt
    //    This can happen when the reference chart is panned
    //    off-screen in full screen quilt mode
    //    If this situation occurs, we need to immediately select a new reference chart
    //    And rebuild the Candidate Array
    //
    //    We also save the dbIndex of the "lost" chart, and try to recover it
    //    on subsequent quilts, typically as the user pans the "lost" chart back on-screen.
    //    The "lost" chart logic is reset on any zoom operations.
    //    See FS#1221
    //
    //    A special case occurs with cm93 composite chart set as the reference chart:
    //    It is not at this point a candidate, so won't be found by the search
    //    This case is indicated if the candidate count is zero.
    //    If so, do not invalidate the ref chart
    bool bf = false;
    for( unsigned int i = 0; i < m_pcandidate_array->GetCount(); i++ ) {
        QuiltCandidate *qc = m_pcandidate_array->Item( i );
        if( qc->dbIndex == m_refchart_dbIndex ) {
            bf = true;
            break;
        }
    }

    if( !bf && m_pcandidate_array->GetCount() ) {
        m_lost_refchart_dbIndex = m_refchart_dbIndex;    // save for later
        m_refchart_dbIndex = GetNewRefChart();
        BuildExtendedChartStackAndCandidateArray(bfull, m_refchart_dbIndex, vp_local);
    }

    if((-1 != m_lost_refchart_dbIndex) && ( m_lost_refchart_dbIndex != m_refchart_dbIndex )) {

        //      Is the lost chart in the extended stack ?
        //      If so, build a new Cnadidate array based upon the lost chart
        for( unsigned int ir = 0; ir < m_extended_stack_array.GetCount(); ir++ ) {
            if( m_lost_refchart_dbIndex == m_extended_stack_array.Item( ir ) ) {
                m_refchart_dbIndex = m_lost_refchart_dbIndex;
                BuildExtendedChartStackAndCandidateArray(bfull, m_refchart_dbIndex, vp_local);
                m_lost_refchart_dbIndex = -1;
                break;
            }
        }
    }

    bool b_has_overlays = false;
#ifdef USE_S57

    //  If this is an S57 quilt, we need to know if there are overlays in it
    if(  CHART_TYPE_S57 == m_reference_type ) {
        for( unsigned int ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
            QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqc->dbIndex );

            if(s57chart::IsCellOverlayType(cte.GetpFullPath() )){
                b_has_overlays = true;
                break;;
            }
        }
    }
#endif

    //    Using Region logic, and starting from the largest scale chart
    //    figuratively "draw" charts until the ViewPort window is completely quilted over
    //    Add only those charts whose scale is smaller than the "reference scale"
    const LLRegion cvp_region = vp_local.GetLLRegion(wxRect(0, 0, vp_local.pix_width, vp_local.pix_height));
    LLRegion vp_region = cvp_region;
    unsigned int ir;

    //    "Draw" the reference chart first, since it is special in that it controls the fine vpscale setting
    QuiltCandidate *pqc_ref = NULL;
    for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ )       // find ref chart entry
    {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( pqc->dbIndex == m_refchart_dbIndex ) {
            pqc_ref = pqc;
            break;
        }
    }

    // Quilted regions can be simplified to reduce the cost of region operations, in this case
    // allow a maximum error of 8 pixels (the rendered display is much better, this is only for composing the quilt)
    const double z = 111274.96299695622; ////WGS84_semimajor_axis_meters * mercator_k0 * DEGREE;
    double factor = 8.0 / (vp_local.view_scale_ppm * z);
    
    if( pqc_ref ) {
        const ChartTableEntry &cte_ref = ChartData->GetChartTableEntry( m_refchart_dbIndex );

        LLRegion vpu_region( cvp_region );

        //LLRegion chart_region = pqc_ref->GetCandidateRegion();
        LLRegion &chart_region = pqc_ref->GetReducedCandidateRegion(factor);
        
        if( !chart_region.Empty() ){
            vpu_region.Intersect( chart_region );

            if( vpu_region.Empty() )
                pqc_ref->b_include = false;   // skip this chart, no true overlap
            else {
                pqc_ref->b_include = true;
                vp_region.Subtract( chart_region );          // adding this chart
            }
        }
        else
            pqc_ref->b_include = false;   // skip this chart, empty region
    }

    //    Now the rest of the candidates
    if( !vp_region.Empty() ) {
        for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
            QuiltCandidate *pqc = m_pcandidate_array->Item( ir );

            if( pqc->dbIndex == m_refchart_dbIndex )
                continue;               // already did this one

             const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqc->dbIndex );

            //  Skip overlays on this pass, so that they do not subtract from quilt and thus displace
            //  a geographical cell with the same extents.
            //  Overlays will be picked up in the next pass, if any are found
#ifdef USE_S57
            if(  CHART_TYPE_S57 == m_reference_type ) {
                if(s57chart::IsCellOverlayType(cte.GetpFullPath() )){
                    continue;
                }
            }
#endif
            if( cte.GetScale() >= m_reference_scale ) {
                //  If this chart appears in the no-show array, then simply include it, but
                //  don't subtract its region when determining the smaller scale charts to include.....
                bool b_in_noshow = false;
                for( unsigned int ins = 0; ins < g_quilt_noshow_index_array.GetCount(); ins++ ) {
                    if( g_quilt_noshow_index_array.Item( ins ) == pqc->dbIndex ) // chart is in the noshow list
                    {
                        b_in_noshow = true;
                        break;
                    }
                }

                if( !b_in_noshow ) {
                    //    Check intersection
                    LLRegion vpu_region( cvp_region );

                    //LLRegion chart_region = pqc->GetCandidateRegion( );  //quilt_region;
                    LLRegion &chart_region = pqc->GetReducedCandidateRegion(factor);
                    
                    if( !chart_region.Empty() ) {
                        vpu_region.Intersect( chart_region );

                        if( vpu_region.Empty() )
                            pqc->b_include = false; // skip this chart, no true overlap
                        else {
                            pqc->b_include = true;
                            vp_region.Subtract( chart_region );          // adding this chart
                        }
                    } else
                        pqc->b_include = false;   // skip this chart, empty region
                } else {
                    pqc->b_include = true;
                }

            } else {
                pqc->b_include = false;                       // skip this chart, scale is too large
            }

            if( vp_region.Empty() )                   // normal stop condition, quilt is full
                break;
        }
    }

    //  For S57 quilts, walk the list again to identify overlay cells found previously,
    //  and make sure they are always included and not eclipsed
    if( b_has_overlays && (CHART_TYPE_S57 == m_reference_type) ) {
        for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
            QuiltCandidate *pqc = m_pcandidate_array->Item( ir );

            if( pqc->dbIndex == m_refchart_dbIndex )
                continue;               // already did this one

            const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqc->dbIndex );

            if( cte.GetScale() >= m_reference_scale ) {
                bool b_in_noshow = false;
                for( unsigned int ins = 0; ins < g_quilt_noshow_index_array.GetCount(); ins++ ) {
                    if( g_quilt_noshow_index_array.Item( ins ) == pqc->dbIndex ) // chart is in the noshow list
                    {
                        b_in_noshow = true;
                        break;
                    }
                }

                if( !b_in_noshow ) {
                    //    Check intersection
                    LLRegion vpu_region( cvp_region );

                    //LLRegion chart_region = pqc->GetCandidateRegion( );
                    LLRegion &chart_region = pqc->GetReducedCandidateRegion(factor);
                    
                    if( !chart_region.Empty() )
                        vpu_region.Intersect( chart_region );

                    if( vpu_region.Empty() )
                        pqc->b_include = false; // skip this chart, no true overlap
#ifdef USE_S57
                    else {
                        bool b_overlay = s57chart::IsCellOverlayType(cte.GetpFullPath() );
                        if( b_overlay )
                            pqc->b_include = true;
                    }
#endif
                }
            }
        }
    }


    //    Walk the candidate list again, marking "eclipsed" charts
    //    which at this point are the ones with b_include == false .AND. whose scale is strictly smaller than the ref scale
    //    Also, maintain the member list of same

    m_eclipsed_stack_array.Clear();

    for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );

        if( !pqc->b_include ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqc->dbIndex );
            if( cte.GetScale() >= m_reference_scale ) {
                m_eclipsed_stack_array.Add( pqc->dbIndex );
                pqc->b_eclipsed = true;
            }
        }
    }

    //    Potentially add cm93 to the candidate array if the region is not yet fully covered
    if( ( (m_bquiltanyproj || m_quilt_proj == PROJECTION_MERCATOR) ) && !vp_region.Empty() ) {
        bool b_must_add_cm93 = true;
#if 0
        //    Check the remaining unpainted region.
        //    It may contain very small "slivers" of empty space, due to mixing of very small scale charts
        //    with the quilt.  If this is the case, do not waste time loading cm93....

        OCPNRegionIterator updd( vp_region );
        while( updd .HaveRects()) {
            wxRect rect = updd.GetRect();
            if( ( rect.width > 2 ) && ( rect.height > 2 ) ) {
                b_must_add_cm93 = true;
                break;
            }
            updd.NextRect();
        }
#endif

        if( b_must_add_cm93 ) {
            for( int ics = 0; ics < pCurrentStack->nEntry; ics++ ) {
                int i = pCurrentStack->GetDBIndex( ics );
                if( CHART_TYPE_CM93COMP == ChartData->GetDBChartType( i ) ) {
                    QuiltCandidate *qcnew = new QuiltCandidate;
                    qcnew->dbIndex = i;
                    qcnew->ChartScale = ChartData->GetDBChartScale( i );

                    m_pcandidate_array->Add( qcnew );
                }
            }
        }
    }

    //    Check the list...if no charts are visible due to all being smaller than reference_scale,
    //    then make sure the smallest scale chart which has any true region intersection is visible anyway
    //    Also enable any other charts which are the same scale as the first one added
    bool b_vis = false;
    for( unsigned int i = 0; i < m_pcandidate_array->GetCount(); i++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( i );
        if( pqc->b_include ) {
            b_vis = true;
            break;
        }
    }

    if( !b_vis && m_pcandidate_array->GetCount() ) {
        int add_scale = 0;

        for( int i = m_pcandidate_array->GetCount() - 1; i >= 0; i-- ) {
            QuiltCandidate *pqc = m_pcandidate_array->Item( i );
            const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqc->dbIndex );

            //    Don't add cm93 yet, it is always covering the quilt...
            if( cte.GetChartType() == CHART_TYPE_CM93COMP ) continue;

            //    Check intersection
            LLRegion vpck_region( vp_local.GetBBox() );

            //LLRegion chart_region = pqc->GetCandidateRegion();
            LLRegion &chart_region = pqc->GetReducedCandidateRegion(factor);
            
            if( !chart_region.Empty() ) vpck_region.Intersect( chart_region );

            if( !vpck_region.Empty() ) {
                if( add_scale ) {
                    if( add_scale == cte.GetScale() ) pqc->b_include = true;
                    ;
                } else {
                    pqc->b_include = true;
                    add_scale = cte.GetScale();
                }
            }
        }
    }

    //    Finally, build a list of "patches" for the quilt.
    //    Smallest scale first, as this will be the natural drawing order

    m_PatchList.DeleteContents( true );
    m_PatchList.Clear();

    if( m_pcandidate_array->GetCount() ) {
        for( int i = m_pcandidate_array->GetCount() - 1; i >= 0; i-- ) {
            QuiltCandidate *pqc = m_pcandidate_array->Item( i );

            //    cm93 add has been deferred until here
            //    so that it would not displace possible raster or ENCs of larger scale
            const ChartTableEntry &m = ChartData->GetChartTableEntry( pqc->dbIndex );

            if( m.GetChartType() == CHART_TYPE_CM93COMP ) pqc->b_include = true; // force acceptance of this chart in quilt
            // would not be in candidate array if not elected

            if( pqc->b_include ) {
                QuiltPatch *pqp = new QuiltPatch;
                pqp->dbIndex = pqc->dbIndex;
                pqp->ProjType = m.GetChartProjectionType();
                //pqp->quilt_region = pqc->GetCandidateRegion();
                pqp->quilt_region = pqc->GetReducedCandidateRegion(factor);
                
                pqp->b_Valid = true;

                m_PatchList.Append( pqp );
            }
        }
    }
    //    From here on out, the PatchList is usable...

#ifdef QUILT_TYPE_1
    if(!m_bquiltanyproj) {
        //    Establish the quilt projection type
        m_quilt_proj = PROJECTION_MERCATOR;// default
        ChartBase *ppc = GetLargestScaleChart();
        if(ppc)
            m_quilt_proj = ppc->GetChartProjectionType();
    }
#endif

    if(!m_bquiltanyproj) {
        //    Walk the PatchList, marking any entries whose projection does not match the determined quilt projection
        for( unsigned int i = 0; i < m_PatchList.GetCount(); i++ ) {
            wxPatchListNode *pcinode = m_PatchList.Item( i );
            QuiltPatch *piqp = pcinode->GetData();
            if( ( piqp->ProjType != m_quilt_proj ) && ( piqp->ProjType != PROJECTION_UNKNOWN ) )
                piqp->b_Valid = false;
        }
    }

    //    Walk the PatchList, marking any entries which appear in the noshow array
    for( unsigned int i = 0; i < m_PatchList.GetCount(); i++ ) {
        wxPatchListNode *pcinode = m_PatchList.Item( i );
        QuiltPatch *piqp = pcinode->GetData();
        for( unsigned int ins = 0; ins < g_quilt_noshow_index_array.GetCount(); ins++ ) {
            if( g_quilt_noshow_index_array.Item( ins ) == piqp->dbIndex ) // chart is in the noshow list
            {
                piqp->b_Valid = false;
                break;
            }
        }
    }

    //    Generate the final render regions for the patches, one by one, smallest to largest scale
//    LLRegion unrendered_region( vp_local.GetLLRegion(wxRect(0, 0, vp_local.pix_width, vp_local.pix_height)) );

    m_covered_region.Clear();
#if 1 // this does the same as before with a lot less operations if there are many charts
    for( int i = m_PatchList.GetCount()-1; i >=0; i-- ) {
        wxPatchListNode *pcinode = m_PatchList.Item( i );
        QuiltPatch *piqp = pcinode->GetData();
        if( !piqp->b_Valid )                         // skip invalid entries
            continue;

        //    Start with the chart's full region coverage.
        piqp->ActiveRegion = piqp->quilt_region;
        // this operation becomes expensive with lots of charts
        if(!b_has_overlays && m_PatchList.GetCount() < 25)
            piqp->ActiveRegion.Subtract(m_covered_region);

        piqp->ActiveRegion.Intersect(cvp_region);

        //    Could happen that a larger scale chart covers completely a smaller scale chart
        if( piqp->ActiveRegion.Empty() )
            piqp->b_eclipsed = true;

        //    Update the next pass full region to remove the region just allocated
        //    Maintain the present full quilt coverage region
        m_covered_region.Union( piqp->quilt_region );
    }
#else
    // this is the old algorithm does the same thing in n^2/2 operations instead of 2*n-1
    for( unsigned int i = 0; i < m_PatchList.GetCount(); i++ ) {
        wxPatchListNode *pcinode = m_PatchList.Item( i );
        QuiltPatch *piqp = pcinode->GetData();

        if( !piqp->b_Valid )                         // skip invalid entries
            continue;

        const ChartTableEntry &ctei = ChartData->GetChartTableEntry( piqp->dbIndex );

        //    Start with the chart's full region coverage.
        LLRegion vpr_region = piqp->quilt_region;

        // This clause should be moved into the rendering routine for quilts so that
        // the actual region logic need only be applied to the render region
#if 1       // This clause went away with full-screen quilting
        // ...and came back with OpenGL....

        //fetch and subtract regions for all larger scale charts
        for( unsigned int k = i + 1; k < m_PatchList.GetCount(); k++ ) {
            wxPatchListNode *pnode = m_PatchList.Item( k );
            QuiltPatch *pqp = pnode->GetData();

            if( !pqp->b_Valid )                         // skip invalid entries
                continue;

/// In S57ENC quilts, do not subtract larger scale regions from smaller.
/// This does two things:
/// 1. This allows co-incident or overlayed chart regions to both be included
///    thus covering the case found in layered Euro(Austrian) IENC cells
/// 2. This make quilted S57 ENC renders much faster, as the larger scale charts are not rendered
///     until the canvas is zoomed sufficiently.

/// Above logic does not apply to cm93 composites

/// However, we now find that if we have an OpenGL FBO rendering surface, it pays to calculate the exact render
/// region for each patch.  Cannot do this with overlays present, though.
            //if( ( CHART_TYPE_S57 != ctei.GetChartType() ))
            if(!b_has_overlays)
            {

                if( !vpr_region.Empty() ) {
                    const ChartTableEntry &cte = ChartData->GetChartTableEntry( pqp->dbIndex );
                    LLRegion larger_scale_chart_region = pqp->quilt_region; //GetChartQuiltRegion( cte, vp_local );

                    vpr_region.Subtract( larger_scale_chart_region );
                }
            }

        }
#endif

        //    Whatever is left in the vpr region and has not been yet rendered must belong to the current target chart

        wxPatchListNode *pinode = m_PatchList.Item( i );
        QuiltPatch *pqpi = pinode->GetData();
        pqpi->ActiveRegion = vpr_region;

        //    Move the active region so that upper left is 0,0 in final render region
//        pqpi->ActiveRegion.Offset( -vp_local.rv_rect.x, -vp_local.rv_rect.y );

        //    Could happen that a larger scale chart covers completely a smaller scale chart
        if( pqpi->ActiveRegion.Empty() )
            pqpi->b_eclipsed = true;

        //    Update the next pass full region to remove the region just allocated
//        if( !vpr_region.Empty() )
//            unrendered_region.Subtract( vpr_region );

        //    Maintain the present full quilt coverage region
//        if( !pqpi->ActiveRegion.Empty() )
            m_covered_region.Union( pqpi->ActiveRegion );
    }
#endif
    //    Restore temporary VP Rotation
    //  vp_local.SetRotationAngle( saved_vp_rotation );

    //    Walk the list again, removing any entries marked as eclipsed....
    unsigned int il = 0;
    while( il < m_PatchList.GetCount() ) {
        wxPatchListNode *pcinode = m_PatchList.Item( il );
        QuiltPatch *piqp = pcinode->GetData();
        if( piqp->b_eclipsed ) {
            //    Make sure that this chart appears in the eclipsed list...
            //    This can happen when....
            bool b_noadd = false;
            for( unsigned int ir = 0; ir < m_eclipsed_stack_array.GetCount(); ir++ ) {
                if( piqp->dbIndex == m_eclipsed_stack_array.Item( ir ) ) {
                    b_noadd = true;
                    break;
                }
            }
            if( !b_noadd ) m_eclipsed_stack_array.Add( piqp->dbIndex );

            m_PatchList.DeleteNode( pcinode );
            il = 0;           // restart the list walk
        }

        else
            il++;
    }
    //    Mark the quilt to indicate need for background clear if the region is not fully covered
//    m_bneed_clear = !unrendered_region.Empty();
//    m_back_region = unrendered_region;

    //    Finally, iterate thru the quilt and preload all of the required charts.
    //    For dynamic S57 SENC creation, this is where SENC creation happens first.....

    //  Stop (temporarily) canvas paint events, since some chart loads mught Yield(),
    //  thus causing performance loss on recursion
    //  We will (always??) get a refresh on the new Quilt anyway...
    cc1->EnablePaint(false);

    //  first lock charts already in the cache
    //  otherwise under memory pressure if chart1 and chart2
    //  are in the quilt loading chart1 could evict chart2
    //
    for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( ( pqc->b_include ) && ( !pqc->b_eclipsed ) )
            pqc->b_locked = ChartData->LockCacheChart( pqc->dbIndex );
    }

    // open charts not in the cache
    for( ir = 0; ir < m_pcandidate_array->GetCount(); ir++ ) {
        QuiltCandidate *pqc = m_pcandidate_array->Item( ir );
        if( ( pqc->b_include ) && ( !pqc->b_eclipsed ) ) {
//         I am fairly certain this test can now be removed
//            with improved smooth movement logic
//            if( !ChartData->IsChartInCache( pqc->dbIndex ) )
//                b_stop_movement = true;
            // only lock chart if not already locked
            if (ChartData->OpenChartFromDBAndLock( pqc->dbIndex, FULL_INIT, !pqc->b_locked ))
                pqc->b_locked = true;
        }
    }

    cc1->EnablePaint(true);
    //    Build and maintain the array of indexes in this quilt

    m_last_index_array = m_index_array;       //save the last one for delta checks

    m_index_array.Clear();

    //    The index array is to be built in reverse, largest scale first
    unsigned int kl = m_PatchList.GetCount();
    for( unsigned int k = 0; k < kl; k++ ) {
        wxPatchListNode *cnode = m_PatchList.Item( ( kl - k ) - 1 );
        m_index_array.Add( cnode->GetData()->dbIndex );
        cnode = cnode->GetNext();
    }

    //    Walk the patch list again, checking the depth units
    //    If they are all the same, then the value is usable

    m_quilt_depth_unit = _T("");
    ChartBase *pc = ChartData->OpenChartFromDB( m_refchart_dbIndex, FULL_INIT );
    if( pc ) {
        m_quilt_depth_unit = pc->GetDepthUnits();

#ifdef USE_S57
        if( pc->GetChartFamily() == CHART_FAMILY_VECTOR ) {
            int units = ps52plib->m_nDepthUnitDisplay;
            switch( units ) {
            case 0:
                m_quilt_depth_unit = _T("Feet");
                break;
            case 1:
                m_quilt_depth_unit = _T("Meters");
                break;
            case 2:
                m_quilt_depth_unit = _T("Fathoms");
                break;
            }
        }
#endif
    }

    for( unsigned int k = 0; k < m_PatchList.GetCount(); k++ ) {
        wxPatchListNode *pnode = m_PatchList.Item( k );
        QuiltPatch *pqp = pnode->GetData();

        if( !pqp->b_Valid )                         // skip invalid entries
            continue;

        ChartBase *pc = ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
        if( pc ) {
            wxString du = pc->GetDepthUnits();
#ifdef USE_S57
            if( pc->GetChartFamily() == CHART_FAMILY_VECTOR ) {
                int units = ps52plib->m_nDepthUnitDisplay;
                switch( units ) {
                case 0:
                    du = _T("Feet");
                    break;
                case 1:
                    du = _T("Meters");
                    break;
                case 2:
                    du = _T("Fathoms");
                    break;
                }
            }
#endif
            wxString dul = du.Lower();
            wxString ml = m_quilt_depth_unit.Lower();

            if( dul != ml ) {
                //    Try all the odd cases
                if( dul.StartsWith( _T("meters") ) && ml.StartsWith( _T("meters") ) ) continue;
                else if( dul.StartsWith( _T("metres") ) && ml.StartsWith( _T("metres") ) ) continue;
                else if( dul.StartsWith( _T("fathoms") ) && ml.StartsWith( _T("fathoms") ) ) continue;
                else if( dul.StartsWith( _T("met") ) && ml.StartsWith( _T("met") ) ) continue;

                //    They really are different
                m_quilt_depth_unit = _T("");
                break;
            }
        }
    }

    //    And try to prove that all required charts are in the cache
    //    If one is missing, try to load it
    //    If still missing, remove its patch from the quilt
    //    This will probably leave a "black hole" in the quilt...
    for( unsigned int k = 0; k < m_PatchList.GetCount(); k++ ) {
        wxPatchListNode *pnode = m_PatchList.Item( k );
        QuiltPatch *pqp = pnode->GetData();

        if( pqp->b_Valid ) {
            if( !ChartData->IsChartInCache( pqp->dbIndex ) ) {
                wxLogMessage( _T("   Quilt Compose cache miss...") );
                ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
                if( !ChartData->IsChartInCache( pqp->dbIndex ) ) {
                    wxLogMessage( _T("    Oops, removing from quilt...") );
                    pqp->b_Valid = false;
                }
            }
        }
    }

    //    Make sure the reference chart is in the cache
    if( !ChartData->IsChartInCache( m_refchart_dbIndex ) ) ChartData->OpenChartFromDB(
            m_refchart_dbIndex, FULL_INIT );

    //    Walk the patch list again, checking the error factor
    //    Also, directly mark the patch to indicate if it should be treated as an overlay
    //    as seen in Austrian Inland series

    m_bquilt_has_overlays = false;
    m_max_error_factor = 0.;
    for( unsigned int k = 0; k < m_PatchList.GetCount(); k++ ) {
        wxPatchListNode *pnode = m_PatchList.Item( k );
        QuiltPatch *pqp = pnode->GetData();

        if( !pqp->b_Valid )                         // skip invalid entries
            continue;

        ChartBase *pc = ChartData->OpenChartFromDB( pqp->dbIndex, FULL_INIT );
        if( pc ) {
            m_max_error_factor = wxMax(m_max_error_factor, pc->GetChart_Error_Factor());
#ifdef USE_S57
            if( pc->GetChartType() == CHART_TYPE_S57 ) {
                s57chart *ps57 = dynamic_cast<s57chart *>( pc );
                if( ps57 ){
                    pqp->b_overlay = ( ps57->GetUsageChar() == 'L' || ps57->GetUsageChar() == 'A' );
                    if( pqp->b_overlay )
                        m_bquilt_has_overlays = true;
                }
            }
#endif
        }
    }

    m_bcomposed = true;

    m_vp_quilt = vp_in;                 // save the corresponding ViewPort locally

    ChartData->LockCache();

    //  Create and store a hash value representing the contents of the m_extended_stack_array
    unsigned long xa_hash = 5381;
    for(unsigned int im=0 ; im < m_extended_stack_array.GetCount() ; im++) {
        int dbindex = m_extended_stack_array.Item(im);
        xa_hash = ((xa_hash << 5) + xa_hash) + dbindex; /* hash * 33 + dbindex */
    }

    m_xa_hash = xa_hash;

    m_bbusy = false;
    return true;
}



//      Compute and update the member quilt render region, considering all scale factors, group exclusions, etc.
void Quilt::ComputeRenderRegion( ViewPort &vp, OCPNRegion &chart_region )
{
    if( !m_bcomposed ) return;

    OCPNRegion rendered_region;

    if( GetnCharts() && !m_bbusy && !chart_region.Empty() ) {
        //  Walk the quilt, considering each chart from smallest scale to largest

        ChartBase *chart = GetFirstChart();

        while( chart ) {
            if( !(chart->GetChartProjectionType() != PROJECTION_MERCATOR && vp.b_MercatorProjectionOverride ) ) {
                QuiltPatch *pqp = GetCurrentPatch();
                if( pqp->b_Valid  ) {
                    OCPNRegion get_screen_region = vp.GetVPRegionIntersect(chart_region, pqp->ActiveRegion, chart->GetNativeScale());
                    if( !get_screen_region.Empty() )
                        rendered_region.Union(get_screen_region);
                }
            }
            chart = GetNextChart();
        }
    }
    //  Record the region actually rendered
    m_rendered_region = rendered_region;
}



int g_render;

bool Quilt::RenderQuiltRegionViewOnDC( wxMemoryDC &dc, ViewPort &vp, OCPNRegion &chart_region )
{

#ifdef ocpnUSE_DIBSECTION
    ocpnMemDC tmp_dc;
#else
    wxMemoryDC tmp_dc;
#endif

    if( !m_bcomposed ) return false;

    OCPNRegion rendered_region;

//    double scale_onscreen = vp.view_scale_ppm;
//    double max_allowed_scale = 4. * cc1->GetAbsoluteMinScalePpm();

    if( GetnCharts() && !m_bbusy ) {

        OCPNRegion screen_region = chart_region;

        //  Walk the quilt, drawing each chart from smallest scale to largest
        //  Render the quilt's charts onto a temp dc
        //  and blit the active region rectangles to to target dc, one-by-one

        ChartBase *chart = GetFirstChart();
        int chartsDrawn = 0;

        if( !chart_region.Empty() ) {
            while( chart ) {
                bool okToRender = true;//cc1->IsChartLargeEnoughToRender( chart, vp );

                if( chart->GetChartProjectionType() != PROJECTION_MERCATOR && vp.b_MercatorProjectionOverride )
                    okToRender = false;

                if( ! okToRender ) {
                    chart = GetNextChart();
                    continue;
                }
                QuiltPatch *pqp = GetCurrentPatch();
                if( pqp->b_Valid  ) {
                    bool b_chart_rendered = false;
                    LLRegion get_region = pqp->ActiveRegion;

                    OCPNRegion get_screen_region = vp.GetVPRegionIntersect(chart_region, get_region,
                                                                           chart->GetNativeScale());
                    if( !get_screen_region.Empty() ) {

                        if( !pqp->b_overlay ) {
                            b_chart_rendered = chart->RenderRegionViewOnDC( tmp_dc, vp, get_screen_region );
                            if( chart->GetChartType() != CHART_TYPE_CM93COMP )
                                b_chart_rendered = true;
                            screen_region.Subtract( get_screen_region );
                        }
                    }

                    OCPNRegionIterator upd( get_screen_region );
                    while( upd.HaveRects() ) {
                        wxRect rect = upd.GetRect();
                        dc.Blit( rect.x, rect.y, rect.width, rect.height, &tmp_dc, rect.x, rect.y,
                                wxCOPY, true );
                        upd.NextRect();
                    }

                    tmp_dc.SelectObject( wxNullBitmap );

                    if(b_chart_rendered)
                        rendered_region.Union(get_screen_region);
                }

                chartsDrawn++;
                chart = GetNextChart();
            }
        }

        if( ! chartsDrawn ) cc1->GetVP().SetProjectionType( PROJECTION_MERCATOR );


        //    Render any Overlay patches for s57 charts(cells)
        if( m_bquilt_has_overlays && !chart_region.Empty() ) {
            chart = GetFirstChart();
            while( chart ) {
                QuiltPatch *pqp = GetCurrentPatch();
                if( pqp->b_Valid ) {
                    if( pqp->b_overlay ) {
                        LLRegion get_region = pqp->ActiveRegion;
                        OCPNRegion get_screen_region = vp.GetVPRegionIntersect(chart_region, get_region,
                                                                               chart->GetNativeScale());
                        if( !get_region.Empty() ) {
#ifdef USE_S57
                            s57chart *Chs57 = dynamic_cast<s57chart*>( chart );
                            Chs57->RenderOverlayRegionViewOnDC( tmp_dc, vp, get_screen_region );
#endif
                            OCPNRegionIterator upd( get_screen_region );
                            while( upd.HaveRects() ) {
                                wxRect rect = upd.GetRect();
                                dc.Blit( rect.x, rect.y, rect.width, rect.height, &tmp_dc, rect.x,
                                      rect.y, wxCOPY, true );
                                upd.NextRect();
                            }
                            tmp_dc.SelectObject( wxNullBitmap );
                        }
                     }
                }

                chart = GetNextChart();
            }
        }

        //    Any part of the chart region that was not rendered in the loop needs to be cleared
        OCPNRegionIterator clrit( screen_region );
        while( clrit.HaveRects() ) {
            wxRect rect = clrit.GetRect();
#ifdef __WXOSX__
            dc.SetPen(*wxBLACK_PEN);
            dc.SetBrush(*wxBLACK_BRUSH);
            dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
#else
            dc.Blit( rect.x, rect.y, rect.width, rect.height, &dc, rect.x, rect.y, wxCLEAR );
#endif
            clrit.NextRect();
        }

        //    Highlighting....
        if( m_nHiLiteIndex >= 0 ) {
            OCPNRegion hiregion = vp.GetVPRegionIntersect(chart_region, GetHiliteRegion(), 1);
            wxRect box = hiregion.GetBox();

            if( !box.IsEmpty() ) {
                //    Is scratch member bitmap OK?
                if( m_pBM ) {
                    if( ( m_pBM->GetWidth() != vp.rv_rect.width )
                            || ( m_pBM->GetHeight() != vp.rv_rect.height ) ) {
                        delete m_pBM;
                        m_pBM = NULL;
                    }
                }

                if( NULL == m_pBM ) m_pBM = new wxBitmap( vp.rv_rect.width, vp.rv_rect.height );

                //    Copy the entire quilt to my scratch bm
                wxMemoryDC q_dc;
                q_dc.SelectObject( *m_pBM );
                q_dc.Blit( 0, 0, vp.rv_rect.width, vp.rv_rect.height, &dc, 0, 0 );
                q_dc.SelectObject( wxNullBitmap );

                //    Create a "mask" bitmap from the chart's region
                //    WxGTK has an error in this method....Creates a color bitmap, not usable for mask creation
                //    So, I clone with correction
                wxBitmap hl_mask_bm( vp.rv_rect.width, vp.rv_rect.height, 1 );
                wxMemoryDC mdc;
                mdc.SelectObject( hl_mask_bm );
                mdc.SetBackground( *wxBLACK_BRUSH );
                mdc.Clear();
                mdc.SetClippingRegion( box );
                mdc.SetBackground( *wxWHITE_BRUSH );
                mdc.Clear();
                mdc.SelectObject( wxNullBitmap );

                if( hl_mask_bm.IsOk() ) {
                    wxMask *phl_mask = new wxMask( hl_mask_bm );
                    m_pBM->SetMask( phl_mask );
                    q_dc.SelectObject( *m_pBM );

                    // Create another mask, dc and bitmap for red-out
                    wxBitmap rbm( vp.rv_rect.width, vp.rv_rect.height );
                    wxMask *pr_mask = new wxMask( hl_mask_bm );
                    wxMemoryDC rdc;
                    rbm.SetMask( pr_mask );
                    rdc.SelectObject( rbm );
                    unsigned char hlcolor = 255;
                    switch( global_color_scheme ) {
                    case GLOBAL_COLOR_SCHEME_DAY:
                        hlcolor = 255;
                        break;
                    case GLOBAL_COLOR_SCHEME_DUSK:
                        hlcolor = 64;
                        break;
                    case GLOBAL_COLOR_SCHEME_NIGHT:
                        hlcolor = 16;
                        break;
                    default:
                        hlcolor = 255;
                        break;
                    }

                    rdc.SetBackground( wxBrush( wxColour( hlcolor, 0, 0 ) ) );
                    rdc.Clear();


                    OCPNRegionIterator upd ( hiregion );
                    while ( upd.HaveRects() )
                    {
                        wxRect rect = upd.GetRect();
                        rdc.Blit( rect.x, rect.y, rect.width, rect.height, &q_dc, rect.x, rect.y, wxOR,
                                  true );
                        upd.NextRect();
                    }

                    OCPNRegionIterator updq ( hiregion );
                    while ( updq.HaveRects() )
                    {
                        wxRect rect = updq.GetRect();
                        q_dc.Blit( rect.x, rect.y, rect.width, rect.height, &rdc, rect.x, rect.y, wxCOPY,
                                   true );
                        updq.NextRect();
                    }


                    q_dc.SelectObject( wxNullBitmap );
                    m_pBM->SetMask( NULL );

                    //    Select the scratch BM as the return dc contents
                    dc.SelectObject( *m_pBM );

                    //    Clear the rdc
                    rdc.SelectObject( wxNullBitmap );
                }
            }  // box not empty
        }     // m_nHiLiteIndex

        //    Fogging....
        if( g_fog_overzoom ) {
            double scale_factor = vp.ref_scale/vp.chart_scale;

            if(scale_factor > g_overzoom_emphasis_base){
                float fog = ((scale_factor - g_overzoom_emphasis_base) * 255.) / 20.;
                fog = wxMin(fog, 200.);         // Don't fog out completely

                //    Is scratch member bitmap OK?
                if( m_pBM ) {
                    if( ( m_pBM->GetWidth() != vp.rv_rect.width )
                        || ( m_pBM->GetHeight() != vp.rv_rect.height ) ) {
                        delete m_pBM;
                    m_pBM = NULL;
                        }
                }

                if( NULL == m_pBM )
                    m_pBM = new wxBitmap( vp.rv_rect.width, vp.rv_rect.height );

                //    Copy the entire quilt to my scratch bm
                wxMemoryDC q_dc;
                q_dc.SelectObject( *m_pBM );
                q_dc.Blit( 0, 0, vp.rv_rect.width, vp.rv_rect.height, &dc, 0, 0 );
                q_dc.SelectObject( wxNullBitmap );


                wxImage src = m_pBM->ConvertToImage();
#if 1
                int blur_factor = wxMin((scale_factor-g_overzoom_emphasis_base)/4, 4);
                if(src.IsOk()){
                    wxImage dest = src.Blur( blur_factor );
#endif


#if 0           // this is fogging effect
                unsigned char *bg = src.GetData();
                wxColour color = cc1->GetFogColor();

                float transparency = fog;

                // destination image
                wxImage dest(vp.rv_rect.width, vp.rv_rect.height);
                unsigned char *dest_data = (unsigned char *) malloc( vp.rv_rect.width * vp.rv_rect.height * 3 * sizeof(unsigned char) );
                unsigned char *d = dest_data;

                float alpha = 1.0 - (float)transparency / 255.0;
                int sb = vp.rv_rect.width * vp.rv_rect.height;
                for( int i = 0; i < sb; i++ ) {
                    float a = alpha;

                    int r = ( ( *bg++ ) * a ) + (1.0-a) * color.Red();
                    *d++ = r;
                    int g = ( ( *bg++ ) * a ) + (1.0-a) * color.Green();
                    *d++ = g;
                    int b = ( ( *bg++ ) * a ) + (1.0-a) * color.Blue();
                    *d++ = b;
                }

                dest.SetData( dest_data );
#endif


                    wxBitmap dim(dest);
                    wxMemoryDC ddc;
                    ddc.SelectObject( dim );

                    q_dc.SelectObject( *m_pBM );
                    OCPNRegionIterator upd ( rendered_region );
                    while ( upd.HaveRects() )
                    {
                        wxRect rect = upd.GetRect();
                        q_dc.Blit( rect.x, rect.y, rect.width, rect.height, &ddc, rect.x, rect.y );
                        upd.NextRect();
                    }

                    ddc.SelectObject( wxNullBitmap );
                    q_dc.SelectObject( wxNullBitmap );

                //    Select the scratch BM as the return dc contents
                    dc.SelectObject( *m_pBM );
                }
            }
         }     // overzoom


        if( !dc.IsOk() )          // some error, probably bad charts, to be disabled on next compose
        {
            SubstituteClearDC( dc, vp );
        }

    } else {             // no charts yet, or busy....
        SubstituteClearDC( dc, vp );
    }

    //  Record the region actually rendered
    m_rendered_region = rendered_region;

    m_vp_rendered = vp;
    return true;
}

void Quilt::SubstituteClearDC( wxMemoryDC &dc, ViewPort &vp )
{
    if( m_pBM ) {
        if( ( m_pBM->GetWidth() != vp.rv_rect.width )
                || ( m_pBM->GetHeight() != vp.rv_rect.height ) ) {
            delete m_pBM;
            m_pBM = NULL;
        }
    }

    if( NULL == m_pBM ) {
        m_pBM = new wxBitmap( vp.rv_rect.width, vp.rv_rect.height );
    }

    dc.SelectObject( wxNullBitmap );
    dc.SelectObject( *m_pBM );
    dc.SetBackground( *wxBLACK_BRUSH );
    dc.Clear();
    m_covered_region.Clear();

}

